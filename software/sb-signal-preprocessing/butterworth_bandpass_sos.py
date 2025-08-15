from scipy.signal import butter, sosfilt

def bandpass_sos(fs, lowcut, highcut, order):
    nyquist = 0.5 * fs
    low = lowcut / nyquist
    high = highcut / nyquist
    sos = butter(order, [low, high], btype='bandpass', output='sos')
    return sos

## Example
fs = 50
lowcut = 0.5
highcut = 5.0
order = 2

print("SOS Sections: \n", bandpass_sos(fs, lowcut, highcut, order))