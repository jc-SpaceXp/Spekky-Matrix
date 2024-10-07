#include "fft_rtos.h"
#include "extern_i2s_dma_data.h"
#include "mic_data_processing.h"
#include "fft_processing.h"

#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"
#include "transform_functions.h"
#include "statistics_functions.h"
#include "basic_math_functions.h"

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

const float32_t window_func[FFT_DATA_SIZE * 2] = {
	1, 1, 1, 1,   1, 1, 1, 1
	, 1, 1, 1, 1, 1, 1, 1, 1
	, 1, 1, 1, 1, 1, 1, 1, 1
	, 1, 1, 1, 1, 1, 1, 1, 1
	, 1, 1, 1, 1, 1, 1, 1, 1
	, 1, 1, 1, 1, 1, 1, 1, 1
	, 1, 1, 1, 1, 1, 1, 1, 1
	, 1, 1, 1, 1, 1, 1, 1, 1
	, 1, 1, 1, 1, 1, 1, 1, 1
	, 1, 1, 1, 1, 1, 1, 1, 1
	, 1, 1, 1, 1, 1, 1, 1, 1
	, 1, 1, 1, 1, 1, 1, 1, 1
	, 1, 1, 1, 1, 1, 1, 1, 1
	, 1, 1, 1, 1, 1, 1, 1, 1
	, 1, 1, 1, 1, 1, 1, 1, 1
	, 1, 1, 1, 1, 1, 1, 1, 1
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


		arm_mult_f32(data_buffer, window_func, fft_input, FFT_DATA_SIZE * 2);

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
