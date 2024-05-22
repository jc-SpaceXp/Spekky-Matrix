#include "fft_rtos.h"

#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"
#include "transform_functions.h"
#include "statistics_functions.h"
#include "hw_verification/py_sine_125hz_input_test.h"


static float32_t bin_mags[1024];

struct FftBinPeak {
	float32_t magnitude;
	uint32_t bin_index;
};

void fft_oneshot_callback(xTimerHandle pxTimer)
{
	(void) pxTimer;

	uint8_t inverse_fft = 0;
	uint8_t bit_reverse = 1;
	arm_cfft_instance_f32 arm_cfft;
	arm_status c_status = arm_cfft_init_1024_f32(&arm_cfft);
	(void) c_status; // suppress compiler warning
	assert_param(c_status == ARM_MATH_SUCCESS);

	arm_cfft_f32(&arm_cfft, sine32c_125hz_2048, inverse_fft, bit_reverse);
	arm_cmplx_mag_f32(sine32c_125hz_2048, bin_mags, 1024);

	struct FftBinPeak fft_bin_peak = { 0, 0 };
	arm_max_f32(bin_mags, 1024, &fft_bin_peak.magnitude, &fft_bin_peak.bin_index);
}
