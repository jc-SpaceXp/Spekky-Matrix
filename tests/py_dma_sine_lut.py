import numpy as np
import matplotlib.pyplot as plt

def dump_lists_to_file(fft_input):
    expected_input = "py_sine_1hz_lut_input_test.txt"
    outfile = open(expected_input, 'w')
    outfile.writelines('\n'.join(str(i).ljust(23) for i in fft_input))
    outfile.close()
    return None

def periodically_sampled_waveform(integer, freq, sampling_freq, total_samples, show_samples, show_plot):
    freq = freq
    fsamp = sampling_freq
    tsamp = 1.0/fsamp
    w = 2*np.pi*freq
    N = total_samples
    tlen = N/fsamp
    t = np.arange(0, tlen, tsamp) # 1sec worth of samples == fsamples total
    tend = N/fsamp

    # Power rating of my breadboard speaker is 0.5W
    # P = IV --> P = 0.5V --> V needs to be 1, V = 3 * x / 4096
    # 1 = 3x/4096, 4096/3 = x, x = 1395
    # nearest power of 2 is 1024
    intscale = np.power(2, 10) - 1
    x = np.float32(np.sin(w*t))
    if integer:
        x = (intscale * np.sin(w*t)).astype(np.int16)

    if show_samples:
        print(x)

    if show_plot:
        plt.plot(t, x)
        plt.xlabel('Time, seconds (s)')
        plt.ylabel('Amplitude')
        plt.show()
    return x


# generate 1 Hz sine wave lut
# timer interrupt for dma will output sine wave at this freq
freq = 1
fsamp = 5
N = 1024 # sample points
print_sampled_input = False
plot_sampled_input = False
integer = True
x = periodically_sampled_waveform(integer, freq, fsamp, N, print_sampled_input, plot_sampled_input)

dump_lists_to_file(x)
