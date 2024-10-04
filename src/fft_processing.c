#include <math.h>
#include <limits.h>

#include "fft_processing.h"

void real_fft_to_db_fs(const float* src, float* dst, unsigned int real_fft_size)
{
	// 20log10(sqrt(real^2 + imag^2)) == 10log10(real^2 + imag^2)
	// Then FFTdb = 20log10(sqrt(real^2 + imag^2) / max_ref)
	// where max_ref == maximum magnitude value of mic output
	// e.g. all bits set on SD out (plus one for 0 value too)
	unsigned int fft_max_ref = (1 << 24) - 1; // SD is 24 bits wide
	// typically scale factor is N/2 however real_fft_size is already N/2
	unsigned int adjusted_fft_max_ref = fft_max_ref * real_fft_size;
	for (int i = 0; i < (int) real_fft_size; ++i) {
		float complex_mag = src[i];
		// dB FS conversion
		if (!complex_mag) {
			complex_mag = 0.8;
		}
		dst[i] = 20 * log10(complex_mag / adjusted_fft_max_ref);
	}
}
