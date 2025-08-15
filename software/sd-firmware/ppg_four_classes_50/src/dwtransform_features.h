#ifndef DWTRANSFORM_FEATURES_H
#define DWTRANSFORM_FEATURES_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>

// Perform DWT up to level 4
void dwt_db1_level4(const std::vector<double>& signal,
                    std::vector<std::vector<double>>& approx_levels,
                    std::vector<std::vector<double>>& detail_levels) {
    std::vector<double> current = signal;
    for (int level = 0; level < 4; ++level) {
        std::vector<double> approx, detail;
        dwt_db1(current, approx, detail);
        approx_levels.push_back(approx);
        detail_levels.push_back(detail);
        current = approx;
    }
}

double median(std::vector<double> vec) {
    if (vec.empty()) return 0.0;
    size_t n = vec.size();
    sort(vec.begin(), vec.end());
    return (n % 2 == 0) ? (vec[n / 2 - 1] + vec[n / 2]) / 2.0 : vec[n / 2];
}

// Extract features from DWT coefficients: mean, std, max, min, median
void extract_features_from_dwt(const std::vector<std::vector<double>>& approx_levels,
                               const std::vector<std::vector<double>>& detail_levels,
                               std::vector<double>& features) {
    features.clear();

    auto extract_stats = [](const std::vector<double>& coeffs, std::vector<double>& out) {
        if (coeffs.empty()) {
            out.insert(out.end(), {0, 0, 0, 0, 0});
            return;
        }
        double sum = accumulate(coeffs.begin(), coeffs.end(), 0.0);
        double mean = sum / coeffs.size();

        double sq_sum = 0.0;
        for (double val : coeffs) {
            sq_sum += (val - mean) * (val - mean);
        }
        double std_dev = sqrt(sq_sum / coeffs.size());

        double max_val = *max_element(coeffs.begin(), coeffs.end());
        double min_val = *min_element(coeffs.begin(), coeffs.end());
        double med = median(coeffs);

        out.insert(out.end(), {mean, std_dev, max_val, min_val, med});
    };

    // cA4
    extract_stats(approx_levels.back(), features);

    // cD4 → cD1
    for (int i = 3; i >= 0; --i) {
        extract_stats(detail_levels[i], features);
    }
}

void dwtransform(const std::vector<double>& signal, std::vector<double>& output_features) {
    std::vector<std::vector<double>> approx_levels;
    std::vector<std::vector<double>> detail_levels;

    dwt_db1_level4(signal, approx_levels, detail_levels);

    extract_features_from_dwt(approx_levels, detail_levels, output_features);
}

#endif // DWTRANSFORM_H