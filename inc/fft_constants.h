#ifndef FFT_CONSTANTS_H
#define FFT_CONSTANTS_H

#define DATA_LEN       512U
#define DATA_LEN_HALF  DATA_LEN/2
#define FFT_DATA_SIZE  DATA_LEN_HALF/4
// must be DATA_LEN/8 (or DATA_LEN_HALF/4)
// (2 adjacent 16-bits are converted into 32-bit)
// other 2 16-bits of data are ignored e.g. L or R channel
// which is 1/4 and then 1/2 that due to DMA double buffering

#endif /* FFT_CONSTANTS_H */
