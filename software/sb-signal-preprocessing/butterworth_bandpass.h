/*
    IIR Filter - Second Order Section Band Pass Filter

    This version allows the SOS coefficients to be provided as input,
    so the filter is not restricted to a fixed bandpass frequency.
*/

#ifndef BUTTERWORTH_BANDPASSFILTER_H
#define BUTTERWORTH_BANDPASSFILTER_H

#include <iostream>
#include <vector>
#include <stdexcept>

class BandpassFilter {
public:
    // Constructor: fs = sampling frequency, sos_input = vector of SOS sections
    BandpassFilter(double fs, const std::vector<std::vector<double>>& sos_input);

    // Apply the filter to a signal
    std::vector<double> apply(const std::vector<double>& signal);

private:
    std::vector<std::vector<double>> sos_sections;
    std::vector<std::vector<double>> zi;
    double fs;

    std::vector<double> filter_section(const std::vector<double>& signal,
                                       const std::vector<double>& sos,
                                       std::vector<double>& z);
};

// Constructor
BandpassFilter::BandpassFilter(double fs, const std::vector<std::vector<double>>& sos_input)
    : fs(fs), sos_sections(sos_input) 
{
    if (fs <= 0) {
        throw std::invalid_argument("Sampling frequency must be positive.");
    }
    if (sos_sections.empty()) {
        throw std::invalid_argument("SOS sections cannot be empty.");
    }

    // Initialize filter states
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

// Apply one SOS section
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

#endif // BUTTERWORTH_BANDPASSFILTER_H