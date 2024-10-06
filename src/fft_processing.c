#include <math.h>
#include <limits.h>

#include "fft_processing.h"

void real_fft_to_db_fs(const float* src, float* dst, unsigned int real_fft_size)
{
	// 20log10(sqrt(real^2 + imag^2)) == 10log10(real^2 + imag^2)
	// Then FFTdb = 20log10(sqrt(real^2 + imag^2) / max_ref)
	// where max_ref == maximum magnitude value of mic output
	unsigned int fft_max_ref = (1 << 24) - 1; // SD is 24 bits wide
	// typically scale factor is N/2 however real_fft_size is already N/2
	unsigned int adjusted_fft_max_ref = fft_max_ref * real_fft_size;
	for (int i = 0; i < (int) real_fft_size; ++i) {
		float complex_mag = src[i];
		// dB FS conversion
		if (!complex_mag) {
			complex_mag = 0.8;
		}
		dst[i] = 20 * log10(complex_mag / adjusted_fft_max_ref) + 3.0f;
	}
}


void average_bin_2d_array(unsigned int total_arrays, unsigned int length
                         , const float (*src)[length], float* dst)
{
	float sum[length];
	for (int bin = 0; bin < (int) length; ++bin) {
		sum[bin] = 0;
	}

	for (int buffer = 0; buffer < (int) total_arrays; ++buffer) {
		for (int bin = 0; bin < (int) length; ++bin) {
			sum[bin] += src[buffer][bin];
		}
	}

	for (int bin = 0; bin < (int) length; ++bin) {
		dst[bin] = sum[bin] / total_arrays;
	}
}
