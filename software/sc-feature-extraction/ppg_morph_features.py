import numpy as np
from scipy.signal import find_peaks

def extract_morph_features(ppg_signal, fs):
    # Peak Detection with higher threshold
    peak_height_threshold = 0.55  # Might have to adjust accordingly
    peaks, _ = find_peaks(ppg_signal, height=peak_height_threshold)

    peak_features = []

    for peak_index in peaks:

        window_size = int(fs * 2)  # Example: 0.5 seconds window
        peak_window = ppg_signal[max(0, peak_index - window_size):min(len(ppg_signal), peak_index + window_size)]

        # Morphological Features
        sa = np.max(peak_window)
        Da = np.min(peak_window)
        SA = np.trapezoid(peak_window, dx=1/fs)
        DA = np.trapezoid(ppg_signal, dx=1/fs) - SA
        St = peak_index / fs
        Dt = (len(ppg_signal) - peak_index) / fs

        # Statistical Features
        mean = np.mean(peak_window)
        median = np.median(peak_window)
        std_deviation = np.std(peak_window)
        min_value = np.min(peak_window)
        max_value = np.max(peak_window)
        variance = np.var(peak_window)
        try:
            amplitude_ratio = sa / (Da + 1e-6)
            area_ratio = SA / (Da + 1e-6)
        except ZeroDivisionError:
            amplitude_ratio = 0
            area_ratio = 0

        # Append all features to the list
        peak_features.append({
            'sa': sa, 'Da': Da, 'SA': SA, 'DA': DA, 'St': St, 'Dt': Dt,
            'Mean': mean,
            'Median': median,
            'Standard_deviation': std_deviation,
            'Min_value': min_value,
            'Max_value': max_value,
            'Variance': variance,
            'Amplitude_ratio': amplitude_ratio,
            'Area_ratio': area_ratio
        })

    return peak_features, peaks

## Example
signal_data = [
    6308.0, 7147.0, 6308.0, 5854.0, 6259.0, 6488.0, 6211.0, 6037.0, 6211.0, 6315.0,
    6211.0, 6144.0, 6211.0, 6240.0, 6211.0, 6213.0, 6259.0, 6267.0, 6259.0, 6273.0,
    6308.0, 6311.0, 6308.0, 6295.0, 6308.0, 6307.0, 6308.0, 6296.0, 6308.0, 6310.0,
    6308.0, 6276.0, 6259.0, 6249.0, 6259.0, 6239.0, 6211.0, 6170.0, 6162.0, 6157.0,
    6162.0, 6151.0, 6162.0, 6165.0, 6162.0, 6127.0, 6114.0, 6109.0, 6114.0, 6084.0]

fs = 50

features, peaks = extract_morph_features(signal_data, fs)

morp_features = []

for feature in features:
    peak_array = [feature[key] for key in feature.keys()]

    # Check for NaN values
    if not any(np.isnan(val) for val in peak_array):
        morp_features.extend(peak_array)
        break

print("\nExtracted Features:\n", morp_features)
print("\nTotal number of features: ", len(morp_features))