#ifndef FFT_PROCESSING_H
#define FFT_PROCESSING_H

void real_fft_to_db_fs(const float* src, float* dst, unsigned int real_fft_size);
void average_bin_2d_array(unsigned int total_arrays, unsigned int length
                         , const float (*src)[length], float* dst);

#endif /* FFT_PROCESSING_H */
