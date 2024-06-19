import numpy as np
import matplotlib.pyplot as plt
import matplotlib.pyplot as mtick
from fermion_mic_check import l_channel_list

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
        fig, (ax1, ax2) = plt.subplots(1, 2)
        fig.suptitle('FFT and iFFT')

        ax1.stem((n/T), np.abs(X))
        ax1.set_xlabel('Freq (Hz)')
        ax1.set_ylabel('FFT Amplitude')
        secax1 = ax1.twiny()
        secax1.stem(n, np.abs(X))
        secax1.tick_params(axis='x', which='major', pad=15)
        secax1.set_xlabel('FFT Bins')

        ax2.plot(tsamp * n, np.fft.ifft(X).real)
        ax2.set_xlabel('Time, seconds (s)')

        ax1.yaxis.set_major_formatter(mtick.ScalarFormatter(useMathText=True))
        ax2.yaxis.set_major_formatter(mtick.ScalarFormatter(useMathText=True))

        plt.show()
    return X


# Compare theoretical (i2s_debug == False) with actual (i2s_debug == True)
freq = 1000
fsamp = 1.0/60E-6 # fsamp is one period of WS (~i2s_samp / 2) (measured in PulseView)
N = 1024 # fft size (or sample count)
print_sampled_fft_input = False
plot_sampled_fft_input = False
print_fft_output = False
plot_fft_output = True
integer = True
i2s_debug = True
dump_results_to_file = False

x = periodically_sampled_waveform(integer, freq, fsamp, N, print_sampled_fft_input, plot_sampled_fft_input)
if i2s_debug:
    offset = 0
    x = l_channel_list[offset:N+offset]

fft_out = fft_ifft_conversion(x, fsamp, N, print_fft_output, plot_fft_output)

if dump_results_to_file:
    dump_lists_to_file(x, fft_out)
