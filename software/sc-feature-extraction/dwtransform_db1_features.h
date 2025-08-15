#ifndef DWTRANSFORM_DB1_FEATURES_H
#define DWTRANSFORM_DB1_FEATURES_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>

// Haar wavelet (db1) single-level DWT
void dwt_db1(const std::vector<double>& input,
             std::vector<double>& approx,
             std::vector<double>& detail) {
    size_t n = (input.size() + 1) / 2; // ceil for n odd 
    approx.resize(n);
    detail.resize(n);

    for (size_t i = 0; i < n; ++i) {
        double x0 = input[2 * i];
        double x1 = (2 * i + 1 < input.size()) ? input[2 * i + 1] : input[2 * i];
        approx[i] = (x0 + x1) / std::sqrt(2.0);
        detail[i] = (x0 - x1) / std::sqrt(2.0);
    }
}

// DWT perform for specific level
void dwt_db1_multi(const std::vector<double>& signal,
                   int levels,
                   std::vector<std::vector<double>>& approx_levels,
                   std::vector<std::vector<double>>& detail_levels) {
    std::vector<double> current = signal;
    for (int level = 0; level < levels; ++level) {
        std::vector<double> approx, detail;
        dwt_db1(current, approx, detail);
        approx_levels.push_back(approx);
        detail_levels.push_back(detail);
        current = approx;
    }
}

// Compute median value of a vector
double median(std::vector<double> vec) {
    if (vec.empty()) return 0.0;
    size_t n = vec.size();
    std::sort(vec.begin(), vec.end());
    return (n % 2 == 0) ? (vec[n / 2 - 1] + vec[n / 2]) / 2.0 : vec[n / 2];
}

// Extract statistical features from DWT coefficients:
// mean, std deviation, max, min, median
void extract_features_from_dwt(const std::vector<std::vector<double>>& approx_levels,
                               const std::vector<std::vector<double>>& detail_levels,
                               std::vector<double>& features) {
    features.clear();

    auto extract_stats = [](const std::vector<double>& coeffs, std::vector<double>& out) {
        if (coeffs.empty()) {
            out.insert(out.end(), {0, 0, 0, 0, 0});
            return;
        }

        double sum = std::accumulate(coeffs.begin(), coeffs.end(), 0.0);
        double mean = sum / coeffs.size();

        double sq_sum = 0.0;
        for (double val : coeffs) {
            sq_sum += (val - mean) * (val - mean);
        }
        double std_dev = std::sqrt(sq_sum / coeffs.size());

        double max_val = *std::max_element(coeffs.begin(), coeffs.end());
        double min_val = *std::min_element(coeffs.begin(), coeffs.end());
        double med = median(coeffs);

        out.insert(out.end(), {mean, std_dev, max_val, min_val, med});
    };

    // Extract features from final approximation (cA_L)
    extract_stats(approx_levels.back(), features);

    // Extract features from detail coefficients (cD_L ... cD1)
    for (int i = (int)detail_levels.size() - 1; i >= 0; --i) {
        extract_stats(detail_levels[i], features);
    }
}

// Full DWT transform + feature extraction
void get_dwt_features(const std::vector<double>& signal,
                      int levels,
                      std::vector<double>& output_features) {
    std::vector<std::vector<double>> approx_levels;
    std::vector<std::vector<double>> detail_levels;

    dwt_db1_multi(signal, levels, approx_levels, detail_levels);
    extract_features_from_dwt(approx_levels, detail_levels, output_features);
}

#endif // DWTRANSFORM_DB1_FEATURES_H