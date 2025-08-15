/*
    Min-Max Normalization

    Function:
        std::vector<double> normalize_signal(const std::vector<double>& signal);

        Parameters:
        - signal : vector of input signal values

        Returns:
        - normalized signal in the range [0, 1]
*/

#ifndef NORMALIZE_MINMAX_H
#define NORMALIZE_MINMAX_H

#include <vector>
#include <algorithm>

std::vector<double> normalize_signal(const std::vector<double>& signal) {
    if (signal.empty()) return {};

    double min_val = *std::min_element(signal.begin(), signal.end());
    double max_val = *std::max_element(signal.begin(), signal.end());
    double epsilon = 1e-6f;

    std::vector<double> normalized;
    normalized.reserve(signal.size());

    for (double val : signal) {
        normalized.push_back((val - min_val) / (max_val - min_val + epsilon));
    }

    return normalized;
}

#endif // NORMALIZE_MINMAX_H