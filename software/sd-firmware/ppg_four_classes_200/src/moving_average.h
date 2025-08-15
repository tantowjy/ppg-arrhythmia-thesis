/*
    Moving Average Filter with Zero-Padding

    Function:
        std::vector<double> moving_average_filter(const std::vector<double>& signal, int window_size);

        Parameters:
        - signal      : input signal (vector of doubles)
        - window_size : size of the averaging window (must be > 0)

        Returns:
        - filtered signal (vector of doubles) with the same size as input
*/

#ifndef MOVING_AVERAGE_FILTER_H
#define MOVING_AVERAGE_FILTER_H

#include <vector>

// Applies a moving average filter with zero-padding ('same' mode).
inline std::vector<double> moving_average_filter(const std::vector<double>& signal, int window_size) {
    int n = signal.size();
    std::vector<double> filtered(n, 0.0);

    std::vector<double> window(window_size, 1.0 / window_size);
    int half_window = window_size / 2;

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;

        for (int j = 0; j < window_size; ++j) {
            int idx = i - half_window + j;
            if (idx >= 0 && idx < n) {
                sum += signal[idx] * window[j];
            }
        }
        filtered[i] = sum;
    }

    return filtered;
}

#endif // MOVING_AVERAGE_FILTER_H