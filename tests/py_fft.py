import numpy as np
import matplotlib.pyplot as plt

def dump_lists_to_file(fft_input, fft_output):
    expected_input = "py_sine_input_test.txt"
    outfile = open(expected_input, 'w')
    outfile.writelines('\n'.join(str(i).ljust(23) for i in fft_input))
    outfile.close()
    expected_output = "py_sine_output_fft_test.txt"
    outfile = open(expected_output, 'w')
    outfile.writelines('\n'.join(f'{c.real:0.09f}'.ljust(18) + f' {c.imag:+0.09f}j' for c in fft_output))
    return None

def periodically_sampled_waveform(integer, freq, sampling_freq, total_samples, show_samples, show_plot):
    freq = freq
    fsamp = sampling_freq
    tsamp = 1.0/fsamp
    w = 2*np.pi*freq
    N = total_samples  # making N say 256 only works when tweaking fsamp and f so that most of the wave is captured
    tlen = N/fsamp
    t = np.arange(0, tlen, tsamp) # 1sec worth of samples == fsamples total
    tend = N/fsamp

    # Round to a value which stm32 can replicate
    int32_max = np.power(2, 31) - 1
    x = np.float32(np.sin(w*t))
    if integer:
        x = (int32_max * np.sin(w*t)).astype(np.int32)

    if show_samples:
        print(x)

    if show_plot:
        plt.plot(t, x)
        plt.xlabel('Time, seconds (s)')
        plt.ylabel('Amplitude')
        plt.show()
    return x

def fft_ifft_conversion(sampled_waveform, sampling_freq, total_samples, show_samples, show_plot):
    tsamp = 1.0/fsamp
    n = np.arange(total_samples)
    T = N/sampling_freq

    X = np.singlecomplex(np.fft.fft(sampled_waveform))

    if show_samples:
        print(X)

    if show_plot:
        plt.subplot(121)
        plt.stem((n/T), np.abs(X))
        plt.xlabel('Freq (Hz)')
        plt.ylabel('FFT Amplitude')
        plt.subplot(122)
        plt.plot(tsamp * n, np.fft.ifft(X).real)
        plt.xlabel('Time, seconds (s)')
        plt.show()
    return X


freq = 125  # 64th bin with no spectral leakage
fsamp = 2000
N = 1024 # fft size
print_sampled_fft_input = False
plot_sampled_fft_input = False
print_fft_output = False
plot_fft_output = True
integer = False
x = periodically_sampled_waveform(integer, freq, fsamp, N, print_sampled_fft_input, plot_sampled_fft_input)
fft_out = fft_ifft_conversion(x, fsamp, N, print_fft_output, plot_fft_output)

#dump_lists_to_file(x, fft_out)
