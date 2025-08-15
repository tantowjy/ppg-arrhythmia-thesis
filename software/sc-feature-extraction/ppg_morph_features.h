/*
    PPG Morphological Feature Extraction

    Features Extracted:
    - Morphological:
        * sa : Systolic amplitude (peak)
        * Da : Diastolic amplitude (valley)
        * SA : Area under the peak
        * DA : Remaining area of the signal
        * St : Time from start to peak (systolic time)
        * Dt : Time from peak to end (diastolic time)
    - Frequency-domain:
        * Dominant_frequency : Main frequency component within a window
    - Statistical:
        * Mean, Median, Standard deviation
        * Min_value, Max_value, Variance
        * Amplitude_ratio (sa/Da), Area_ratio (SA/Da)

    Key Functions:
    - trapezoidal_integral     : Calculates area under the curve using trapezoidal rule
    - compute_median           : Computes median value from a vector
    - compute_dft              : Performs naive DFT to find dominant frequency
    - find_peaks               : Detects peaks above a threshold in the PPG signal
    - extract_morph_features   : Main function to compute feature sets around each peak
    - morphological_features   : Returns vectorized feature output (for ML input)
*/

#ifndef PPG_MORPH_FEATURES_H
#define PPG_MORPH_FEATURES_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>

struct FeatureSet {
    double sa, Da, SA, DA, St, Dt;
    double Mean, Median, Standard_deviation;
    double Min_value, Max_value, Variance;
    double Amplitude_ratio, Area_ratio;
};

// Trapezoidal integration
double trapezoidal_integral(const std::vector<double>& signal, double fs) {
    double area = 0.0;
    for (size_t i = 1; i < signal.size(); ++i) {
        area += 0.5 * (signal[i] + signal[i - 1]) * (1.0 / fs);
    }
    return area;
}

// Median
double compute_median(std::vector<double> data) {
    std::sort(data.begin(), data.end());
    size_t n = data.size();
    if (n % 2 == 0)
        return (data[n / 2 - 1] + data[n / 2]) / 2.0;
    else
        return data[n / 2];
}

// Naive DFT magnitude spectrum and frequencies
void compute_dft(const std::vector<double>& signal, double fs, std::vector<double>& magnitude, std::vector<double>& frequencies) {
    size_t N = signal.size();
    magnitude.resize(N);
    frequencies.resize(N);
    for (size_t k = 0; k < N; ++k) {
        double real = 0.0, imag = 0.0;
        for (size_t n = 0; n < N; ++n) {
            double angle = 2.0 * M_PI * k * n / N;
            real += signal[n] * cos(angle);
            imag -= signal[n] * sin(angle);
        }
        magnitude[k] = std::sqrt(real * real + imag * imag);
        frequencies[k] = (k < N / 2) ? k * fs / N : (static_cast<int>(k) - static_cast<int>(N)) * fs / N;
    }
}

// Peak detection
std::vector<int> find_peaks(const std::vector<double>& signal, double height_threshold) {
    std::vector<int> peaks;
    for (size_t i = 1; i < signal.size() - 1; ++i) {
        if (signal[i] > height_threshold && signal[i] > signal[i - 1] && signal[i] > signal[i + 1]) {
            peaks.push_back(i);
        }
    }
    return peaks;
}

// Feature extraction
std::vector<FeatureSet> extract_morph_features(const std::vector<double>& ppg_signal, double fs, std::vector<int>& out_peaks) {
    double peak_height_threshold = 0.55;
    std::vector<int> peaks = find_peaks(ppg_signal, peak_height_threshold);
    out_peaks = peaks;

    std::vector<FeatureSet> peak_features;
    int signal_len = ppg_signal.size();

    for (int peak_index : peaks) {
        int window_size = static_cast<int>(fs * 2);
        int start = std::max(0, peak_index - window_size);
        int end = std::min(signal_len, peak_index + window_size);
        std::vector<double> peak_window(ppg_signal.begin() + start, ppg_signal.begin() + end);
        size_t window_len = peak_window.size();

        // Morphological
        double sa = *std::max_element(peak_window.begin(), peak_window.end());
        double Da = *std::min_element(peak_window.begin(), peak_window.end());
        double SA = trapezoidal_integral(peak_window, fs);
        double full_area = trapezoidal_integral(ppg_signal, fs);
        double DA = full_area - SA;
        double St = static_cast<double>(peak_index) / fs;
        double Dt = static_cast<double>(signal_len - peak_index) / fs;

        // Statistical
        double sum = std::accumulate(peak_window.begin(), peak_window.end(), 0.0);
        double mean = sum / window_len;
        double median = compute_median(peak_window);
        double sq_sum = std::inner_product(peak_window.begin(), peak_window.end(), peak_window.begin(), 0.0);
        double std_deviation = std::sqrt(sq_sum / window_len - mean * mean);
        double variance = std_deviation * std_deviation;
        double min_value = *std::min_element(peak_window.begin(), peak_window.end());
        double max_value = *std::max_element(peak_window.begin(), peak_window.end());

        double amplitude_ratio = sa / (Da + 1e-6);
        double area_ratio = SA / (Da + 1e-6);

        FeatureSet features = {
            sa, Da, SA, DA, St, Dt,
            mean, median, std_deviation,
            min_value, max_value,
            variance,
            amplitude_ratio, area_ratio
        };
        peak_features.push_back(features);
    }

    return peak_features;
}

// MF Features
double safe_value(double x) {
    return std::isnan(x) ? 0.0 : x;
}

// // All Features
// std::vector<double> morphological_features(const std::vector<FeatureSet>& features) {
//     std::vector<double> output;
//     for (const auto& f : features) {
//         output.push_back(safe_value(f.sa));
//         output.push_back(safe_value(f.Da));
//         output.push_back(safe_value(f.SA));
//         output.push_back(safe_value(f.DA));
//         output.push_back(safe_value(f.St));
//         output.push_back(safe_value(f.Dt));
//         output.push_back(safe_value(f.Mean));
//         output.push_back(safe_value(f.Median));
//         output.push_back(safe_value(f.Standard_deviation));
//         output.push_back(safe_value(f.Min_value));
//         output.push_back(safe_value(f.Max_value));
//         output.push_back(safe_value(f.Variance));
//         output.push_back(safe_value(f.Amplitude_ratio));
//         output.push_back(safe_value(f.Area_ratio));
//     }
//     return output;
// }

// Only first peak
std::vector<double> morphological_features(const std::vector<FeatureSet>& features) {
    std::vector<double> output;
    if (features.empty()) return output;

    const auto& f = features[0];  // First peak only
    output.push_back(safe_value(f.sa));
    output.push_back(safe_value(f.Da));
    output.push_back(safe_value(f.SA));
    output.push_back(safe_value(f.DA));
    output.push_back(safe_value(f.St));
    output.push_back(safe_value(f.Dt));
    output.push_back(safe_value(f.Mean));
    output.push_back(safe_value(f.Median));
    output.push_back(safe_value(f.Standard_deviation));
    output.push_back(safe_value(f.Min_value));
    output.push_back(safe_value(f.Max_value));
    output.push_back(safe_value(f.Variance));
    output.push_back(safe_value(f.Amplitude_ratio));
    output.push_back(safe_value(f.Area_ratio));

    return output;
}

#endif // PPG_MORPH_FEATURES_H