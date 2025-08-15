#ifndef DWTRANSFORM_DB1_H
#define DWTRANSFORM_DB1_H

#include <vector>
#include <cmath>

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

// Main function: cD1...cDn, cAn
void get_dwt_coeffs(const std::vector<double>& signal,
                    int levels,
                    std::vector<std::vector<double>>& detail_levels, // cD1..cDn
                    std::vector<double>& final_approx)               // cAn
{
    std::vector<std::vector<double>> approx_levels;
    dwt_db1_multi(signal, levels, approx_levels, detail_levels);
    final_approx = approx_levels.back();
}

#endif // DWTRANSFORM_DB1_H
