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
		{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f }
		, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f }
	};
	float dst[9] = { 0.0f };
	float expected[9] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f };

	average_bin_2d_array(2, 9, src, dst);

	for (int i = 0; i < 9; ++i) {
		ASSERT_IN_RANGE(expected[i], dst[i], 0.3f);
	}
	PASS();
}

TEST average_buffered_bin_test(void)
{
	float src[4][9] = {
		{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f }
		, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f }
		, {5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f }
		, {5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f }
	};
	float dst[9] = { 0.0f };
	float expected[9] = { 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f };
	float (*src_offset)[9] = &src[2];

	average_bin_2d_array(2, 9, src_offset, dst);

	for (int i = 0; i < 9; ++i) {
		ASSERT_IN_RANGE(expected[i], dst[i], 0.3f);
	}
	PASS();
}

TEST average_buffered_bin_test_with_zeros(void)
{
	float src[4][9] = {
		{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f }
		, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f }
		, {5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 0.0f, 13.0f }
		, {5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 0.0f, 13.0f }
	};
	float dst[9] = { 0.0f };
	float expected[9] = { 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 0.0f, 13.0f };
	float (*src_offset)[9] = &src[2];

	average_bin_2d_array(2, 9, src_offset, dst);

	for (int i = 0; i < 9; ++i) {
		ASSERT_IN_RANGE(expected[i], dst[i], 0.3f);
	}
	PASS();
}


SUITE(fft_suite)
{
	RUN_TEST(max_amp_square_wave_to_db_fs);
	RUN_TEST(basic_average_bin_test);
	RUN_TEST(average_buffered_bin_test);
	RUN_TEST(average_buffered_bin_test_with_zeros);
}

