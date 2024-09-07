#ifndef MIC_DATA_PROCESSING_H
#define MIC_DATA_PROCESSING_H

#include <stdint.h>

enum MicChannel { L, R };

void dma_i2s_halfword_to_word_complex_conversion(const int16_t* src, float* dst
                                                , unsigned int input_bytes
                                                , enum MicChannel channel);

#endif /* MIC_DATA_PROCESSING_H */
