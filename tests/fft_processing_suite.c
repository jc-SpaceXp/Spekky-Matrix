#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "greatest.h"
#include "fft_processing_suite.h"

#include "hw_verification/fft_bin_mags_max_amp_square_wave_24bit.h"

#include "fft_processing.h"

TEST max_amp_square_wave_to_db_fs(void)
{
	float dst[32] = { 0 };
	real_fft_to_db_fs(real_bin_mags_max_square_wave24, dst, 32);

	ASSERT_LT(-6.0f, dst[0]); // 1st harmonic should be above -6 dB FS
	float prev_odd_harmonic = dst[0];
	for (int i = 0; i < 16; ++i) {
		float odd_harmonic = dst[i * 2];
		float even_harmonic = dst[(i * 2) + 1];
		ASSERT_GT(-150.0f, even_harmonic); // even harmonics should be below -150 dB FS
		if (i) {
			ASSERT_GT(prev_odd_harmonic, odd_harmonic); // prev > new (1st > 3rd etc.)
		}
		prev_odd_harmonic = odd_harmonic;
	}
	PASS();
}

TEST basic_average_bin_test(void)
{
	float src[2][9] = {
		{1, 2, 3, 4, 5, 6, 7, 8, 9 }
		, {1, 2, 3, 4, 5, 6, 7, 8, 9 }
	};
	float dst[9] = { 0 };
	float expected[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9 };

	average_bin_2d_array(2, 9, src, dst);

	ASSERT_MEM_EQ(expected, dst, 9);
	PASS();
}

TEST average_buffered_bin_test(void)
{
	float src[4][10] = {
		{1, 2, 3, 4, 5, 6, 7, 8, 9 }
		, {1, 2, 3, 4, 5, 6, 7, 8, 9 }
		, {5, 6, 7, 8, 9, 10, 11, 12, 13 }
		, {5, 6, 7, 8, 9, 10, 11, 12, 13 }
	};
	float dst[10] = { 0 };
	float expected[10] = {5, 6, 7, 8, 9, 10, 11, 12, 13, };
	float (*src_offset)[10] = &src[2];

	average_bin_2d_array(2, 10, src_offset, dst);

	ASSERT_MEM_EQ(expected, dst, 10);
	PASS();
}


SUITE(fft_suite)
{
	RUN_TEST(max_amp_square_wave_to_db_fs);
	RUN_TEST(basic_average_bin_test);
	RUN_TEST(average_buffered_bin_test);
}

