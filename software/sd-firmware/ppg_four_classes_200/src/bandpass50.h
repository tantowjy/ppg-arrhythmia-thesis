/*
    IIR Filter - Second Order Section Band Pass Filter

    Main Components:
    - `BandpassFilter` class
        - Constructor : initializes the filter with sampling frequency and order
        - `apply()`   : applies all SOS filter sections to an input signal
        - `filter_section()` : applies a single SOS section

    Important Notes for SOS:
    sampling rate : 200
    lowcut        : 0.5
    highcut       : 5.0
    order         : 2

    Example Usage:
        BandpassFilter filter(50.0);  // Sampling frequency = 50 Hz
        std::vector<double> raw_signal = { ... };
        std::vector<double> filtered = filter.apply(raw_signal);
*/

#ifndef BANDPASSFILTER50_H
#define BANDPASSFILTER50_H

#include <iostream>
#include <vector>
#include <stdexcept>

class BandpassFilter {
public:
    BandpassFilter(double fs, int order = 2);
    std::vector<double> apply(const std::vector<double>& signal);

private:
    std::vector<std::vector<double>> sos_sections;
    std::vector<std::vector<double>> zi;

    void init_filter();
    std::vector<double> filter_section(const std::vector<double>& signal,
                                       const std::vector<double>& sos,
                                       std::vector<double>& z);

    double fs;
    int order;
};

// Constructor
BandpassFilter::BandpassFilter(double fs, int order)
    : fs(fs), order(order) {
    if (fs <= 0) {
        throw std::invalid_argument("Sampling frequency must be positive.");
    }
    init_filter();
}

// Initialize the SOS filter structure
void BandpassFilter::init_filter() {
    // Replace with SOS array
    sos_sections = {
        {0.00453622, 0.00907244, 0.00453622, 1.0, -1.81717983, 0.83630344},
        {1.0, -2.0, 1.0, 1.0, -1.97876951, 0.97905842}
    };
    zi.resize(sos_sections.size(), std::vector<double>(2, 0.0));
}

// Apply the filter to the signal
std::vector<double> BandpassFilter::apply(const std::vector<double>& signal) {
    std::vector<double> output = signal;

    for (size_t i = 0; i < sos_sections.size(); ++i) {
        output = filter_section(output, sos_sections[i], zi[i]);
    }

    return output;
}

// Apply one section of the SOS filter
std::vector<double> BandpassFilter::filter_section(const std::vector<double>& signal,
                                                   const std::vector<double>& sos,
                                                   std::vector<double>& z) {
    std::vector<double> output(signal.size());
    double b0 = sos[0], b1 = sos[1], b2 = sos[2];
    double a0 = sos[3], a1 = sos[4], a2 = sos[5];

    for (size_t n = 0; n < signal.size(); ++n) {
        double x = signal[n];
        double y = b0 / a0 * x + z[0];
        z[0] = b1 / a0 * x - a1 / a0 * y + z[1];
        z[1] = b2 / a0 * x - a2 / a0 * y;
        output[n] = y;
    }
    return output;
}

#endif // BANDPASSFILTER50_H