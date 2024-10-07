#include "fft_rtos.h"
#include "extern_i2s_dma_data.h"
#include "mic_data_processing.h"
#include "fft_processing.h"

#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"
#include "transform_functions.h"
#include "statistics_functions.h"
#include "basic_math_functions.h"
#include "window_functions.h"

#include "FreeRTOS.h"
#include "queue.h"

#include "fft_constants.h"

extern QueueHandle_t xDmaFlagQueue;
extern QueueHandle_t xFftCompleteFlagQueue;

float32_t bin_mags[FFT_AVERAGE * 2][FFT_DATA_SIZE/2];
float32_t average_bin_mags[FFT_DATA_SIZE/2];
float32_t db_bin_mags[FFT_DATA_SIZE/2];
static float32_t data_buffer1[FFT_DATA_SIZE * 2];
static float32_t data_buffer2[FFT_DATA_SIZE * 2];
static float32_t fft_input[FFT_DATA_SIZE * 2];

// via: arm_blackman_harris_92db_f32(window_func, FFT_DATA_SIZE); gdb output
// adjusted for complex data e.g. FFT_DATA_SIZE blackman harris interleaved with 0's
// b[i], 0, b[i+1], 0 format
const float32_t complex_window_func[FFT_DATA_SIZE * 2] = {
	5.99687919e-05,  0, 0.000199495815, 0, 0.000656468794, 0, 0.00154587673,  0
	, 0.00305913622, 0, 0.00546273123,  0, 0.0090958653,   0, 0.014364982,    0
	, 0.0217358209,  0, 0.0317205638,   0, 0.0448606685,   0, 0.0617044829,   0
	, 0.0827803537,  0, 0.108565629,    0, 0.139452726,    0, 0.175714359,    0
	, 0.21747002,    0, 0.264654964,    0, 0.316995889,    0, 0.373994321,    0
	, 0.434919626,   0, 0.498813659,    0, 0.564508379,    0, 0.630654693,    0
	, 0.695764124,   0, 0.758259773,    0, 0.816535234,    0, 0.869019151,    0
	, 0.914240956,   0, 0.950894117,    0, 0.977894962,    0, 0.994431198,    0
	, 1,             0, 0.994431138,    0, 0.977894843,    0, 0.950894058,    0
	, 0.914240956,   0, 0.869019091,    0, 0.816535056,    0, 0.758259594,    0
	, 0.695764065,   0, 0.630654752,    0, 0.56450808,     0, 0.49881354,     0
	, 0.434919268,   0, 0.373994231,    0, 0.316995889,    0, 0.264654785,    0
	, 0.21746999,    0, 0.175714269,    0, 0.139452651,    0, 0.108565524,    0
	, 0.0827803165,  0, 0.061704468,    0, 0.0448606201,   0, 0.0317205489,   0
	, 0.021735793,   0, 0.0143649699,   0, 0.00909586065,  0, 0.0054627629,   0
	, 0.00305913808, 0, 0.00154588651,  0, 0.00065645203,  0, 0.000199492089, 0
};


void fft_task_processing(void* pvParameters)
{
	(void) pvParameters;

	static int fft_counter = 0;

	uint8_t inverse_fft = 0;
	uint8_t bit_reverse = 1;
	arm_cfft_instance_f32 arm_cfft;
	arm_status c_status = arm_cfft_init_64_f32(&arm_cfft); // init_XX_f32, XX = FFT_SIZE
	(void) c_status; // suppress compiler warning
	assert_param(c_status == ARM_MATH_SUCCESS);

	int fft_section = 0;
	float32_t* data_buffer = &data_buffer1[0];
	for (;;) {
		while (!xQueueReceive(xDmaFlagQueue, &fft_section, portMAX_DELAY)) {
			// delay/block until data is ready
		}

		if (fft_section == 1) {
			data_buffer = &data_buffer1[0];
			dma_i2s_halfword_to_word_complex_conversion(&i2s_dma_data[0], data_buffer
		                                               , DATA_LEN_HALF, L);
		} else if (fft_section == 2) {
			data_buffer = &data_buffer2[0];
			dma_i2s_halfword_to_word_complex_conversion(&i2s_dma_data[DATA_LEN_HALF], data_buffer
		                                               , DATA_LEN_HALF, L);
		}

		float32_t dc_signal = 0.0f;
		arm_mean_f32(data_buffer, FFT_DATA_SIZE * 2, &dc_signal);
		dc_signal *= 2.0f; // data is complex { a, 0, b, 0 .. }, double to adjust for 0j's
		for (int i = 0; i < (int) FFT_DATA_SIZE; ++i) {
			data_buffer[i * 2] = data_buffer[i * 2] - dc_signal;
		}

		arm_mult_f32(data_buffer, complex_window_func, fft_input, FFT_DATA_SIZE * 2);

		arm_cfft_f32(&arm_cfft, fft_input, inverse_fft, bit_reverse);
		// ignore DC component, any gather real frequencies and Nyquist
		arm_cmplx_mag_f32(&fft_input[2], &bin_mags[fft_counter][0], FFT_DATA_SIZE/2);

		fft_counter += 1;
		if (fft_counter == (int) (FFT_AVERAGE * 2)) {
			average_bin_2d_array(FFT_AVERAGE, FFT_DATA_SIZE/2, &bin_mags[FFT_AVERAGE]
			                    , average_bin_mags);
			real_fft_to_db_fs(average_bin_mags, db_bin_mags, FFT_DATA_SIZE/2);

			xQueueSendToFront(xFftCompleteFlagQueue, &fft_counter, pdMS_TO_TICKS(2));
			fft_counter = 0;
		} else if (fft_counter == (int) FFT_AVERAGE) {
			average_bin_2d_array(FFT_AVERAGE, FFT_DATA_SIZE/2, bin_mags, average_bin_mags);
			real_fft_to_db_fs(average_bin_mags, db_bin_mags, FFT_DATA_SIZE/2);

			xQueueSendToFront(xFftCompleteFlagQueue, &fft_counter, pdMS_TO_TICKS(2));
		}
	}
}
