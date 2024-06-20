import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as mtick
import matplotlib.animation as animation
from functools import partial
from fermion_mic_check import l_channel_list

def dump_lists_to_file(fft_input, fft_output):
    expected_input = "py_sine_input_test.txt"
    outfile = open(expected_input, 'w')
    outfile.writelines('\n'.join(str(i).ljust(23) for i in fft_input))
    outfile.close()
    expected_output = "py_sine_output_fft_test.txt"
    outfile = open(expected_output, 'w')
    outfile.writelines('\n'.join(f'{c.real:0.09f}'.ljust(18) + f' {c.imag:+0.09f}j'
                                for c in fft_output))
    return None

def periodically_sampled_waveform(integer, freq, sampling_freq, total_samples, show_samples
                                 , show_plot):
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

def fft_ifft_conversion(sampled_waveform, sampling_freq, total_samples, show_samples):
    tsamp = 1.0/sampling_freq
    n = np.arange(total_samples)
    T = N/sampling_freq

    X = np.singlecomplex(np.fft.fft(sampled_waveform))

    if show_samples:
        print(X)

    return X

def plot_fft_ifft_results(fft_results, sampling_freq, total_samples):
    tsamp = 1.0/sampling_freq
    n = np.arange(total_samples)
    T = N/sampling_freq

    fig.suptitle('FFT and iFFT')

    ax1.stem((n/T), np.abs(fft_results))
    ax1.set_xlabel('Freq (Hz)')
    ax1.set_ylabel('FFT Amplitude')
    secax1.stem(n, np.abs(fft_results))
    secax1.tick_params(axis='x', which='major', pad=15)
    secax1.set_xlabel('FFT Bins')
    secax1.xaxis.set_label_position('top')

    ax2.plot(tsamp * n, np.fft.ifft(fft_results).real)
    ax2.set_xlabel('Time, seconds (s)')

    ax1.yaxis.set_major_formatter(mtick.ScalarFormatter(useMathText=True))
    ax2.yaxis.set_major_formatter(mtick.ScalarFormatter(useMathText=True))

    return None

def gen_raw_data_iter():
    offset = 0
    fft_size = 64
    raw_data = l_channel_list
    while (offset + fft_size) < len(raw_data):
        yield offset
        offset += fft_size

def animate(frame, fft_fig, ifft_fig, fft_bins_fig, sampling_freq, fft_size):
    fft_fig.clear()
    fft_bins_fig.clear()
    ifft_fig.clear()
    x = l_channel_list[frame:fft_size+frame]
    fft_out = fft_ifft_conversion(x, sampling_freq, fft_size, False)
    plot_fft_ifft_results(fft_out, sampling_freq, fft_size)
    return None

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
animate_fft = True

# Make global so animate and plot functions can both use
fig, (ax1, ax2) = plt.subplots(1, 2)
secax1 = ax1.twiny()

x = periodically_sampled_waveform(integer, freq, fsamp, N, print_sampled_fft_input
                                 , plot_sampled_fft_input)
if i2s_debug:
    offset = 0
    x = l_channel_list[offset:N+offset]

fft_out = fft_ifft_conversion(x, fsamp, N, print_fft_output)
if plot_fft_output:
    if animate_fft:
        ani = animation.FuncAnimation(fig, partial(animate, fft_fig=ax1, ifft_fig=ax2
                                                  , fft_bins_fig=secax1
                                                  , sampling_freq=fsamp, fft_size=64)
                                     , frames=gen_raw_data_iter, interval=400, save_count=1
                                     , repeat_delay=3000)
    else:
        plot_fft_ifft_results(fft_out, fsamp, N)
    plt.show()

if dump_results_to_file:
    dump_lists_to_file(x, fft_out)
