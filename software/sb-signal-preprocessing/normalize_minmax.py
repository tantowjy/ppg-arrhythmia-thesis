"""
    This script provides a function to normalize signal data using different methods.
    Supported methods:
    1. 'minmax' : Scales the signal values into the range [0, 1].
    2. 'zscore' : Standardizes the signal to have mean 0 and standard deviation 1.

    The function automatically handles numerical stability by adding a small epsilon 
    (1e-6) to the denominator to avoid division by zero.

"""

import numpy as np

def normalize_signal(signal, method):
    signal = np.array(signal).astype(np.float32)
    if method == 'minmax':
        return ((signal - np.min(signal)) / (np.max(signal) - np.min(signal) + 1e-6))
    elif method == 'zscore':
        return ((signal - np.mean(signal)) / (np.std(signal) + 1e-6))
    else:
        raise ValueError("Normalization method not supported")
    

## Example 
signal_data = [
    6308.0, 7147.0, 6308.0, 5854.0, 6259.0, 6488.0, 6211.0, 6037.0, 6211.0, 6315.0,
    6211.0, 6144.0, 6211.0, 6240.0, 6211.0, 6213.0, 6259.0, 6267.0, 6259.0, 6273.0,
    6308.0, 6311.0, 6308.0, 6295.0, 6308.0, 6307.0, 6308.0, 6296.0, 6308.0, 6310.0,
    6308.0, 6276.0, 6259.0, 6249.0, 6259.0, 6239.0, 6211.0, 6170.0, 6162.0, 6157.0,
    6162.0, 6151.0, 6162.0, 6165.0, 6162.0, 6127.0, 6114.0, 6109.0, 6114.0, 6084.0]

normalized_signal = normalize_signal(signal_data, method='minmax')
print("Signal Length: ", len(normalized_signal))
print("Normalized Signal: \n", normalized_signal)