import numpy as np
import pywt

def extract_dwt_coeffs(signal, wavelet='db1', level=4):
    coeffs = pywt.wavedec(signal, wavelet, level=level)
    # cA4, cD4, cD3, cD2, cD1 = coeffs
    
    for i in range (0, level):
        print(f"cD{i+1}: ", coeffs[level - i])

        if (i + 1 == level):
            print(f"cA{i+1}: ", coeffs[0])
    
    # return np.concatenate([cA4])

## Example
signal_data = [
    6308.0, 7147.0, 6308.0, 5854.0, 6259.0, 6488.0, 6211.0, 6037.0, 6211.0, 6315.0,
    6211.0, 6144.0, 6211.0, 6240.0, 6211.0, 6213.0, 6259.0, 6267.0, 6259.0, 6273.0,
    6308.0, 6311.0, 6308.0, 6295.0, 6308.0, 6307.0, 6308.0, 6296.0, 6308.0, 6310.0,
    6308.0, 6276.0, 6259.0, 6249.0, 6259.0, 6239.0, 6211.0, 6170.0, 6162.0, 6157.0,
    6162.0, 6151.0, 6162.0, 6165.0, 6162.0, 6127.0, 6114.0, 6109.0, 6114.0, 6084.0]

extract_dwt_coeffs(signal_data)
# print("DWT Coefficients: \n", extract_dwt_coeffs(signal_data))