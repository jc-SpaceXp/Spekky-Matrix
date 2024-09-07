#include <stdbool.h>
#include "mic_data_processing.h"

void dma_i2s_halfword_to_word_complex_conversion(const int16_t* src, float* dst
                                                , unsigned int input_bytes
                                                , enum MicChannel channel)
{
	// data is: L L R R L L R R ....
	// data is transmitted MSB first
	int src_offset = 0;
	if (channel == R) { src_offset = 2; }
	while (src_offset < (int) input_bytes) {
		int dst_offset = src_offset / 2;
		if (channel == R) { dst_offset -= 1; }
		uint32_t msb_byte = (uint32_t) (*(src + src_offset)) << 16U;
		uint16_t lsb_byte = *(src + src_offset + 1);
		*(dst + dst_offset) = (float) ((int32_t) msb_byte | lsb_byte);
		*(dst + dst_offset + 1) = 0.0f; // complex value
		src_offset += 4; // skip next channel data
		dst_offset += 2;
	}
}
