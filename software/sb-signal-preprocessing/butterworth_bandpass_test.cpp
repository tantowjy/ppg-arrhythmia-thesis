#include "butterworth_bandpass.h"

int main() {
    // Input signal
    std::vector<double> input_signal = {
        6308.0, 7147.0, 6308.0, 5854.0, 6259.0, 6488.0, 6211.0, 6037.0, 6211.0, 6315.0,
        6211.0, 6144.0, 6211.0, 6240.0, 6211.0, 6213.0, 6259.0, 6267.0, 6259.0, 6273.0,
        6308.0, 6311.0, 6308.0, 6295.0, 6308.0, 6307.0, 6308.0, 6296.0, 6308.0, 6310.0,
        6308.0, 6276.0, 6259.0, 6249.0, 6259.0, 6239.0, 6211.0, 6170.0, 6162.0, 6157.0,
        6162.0, 6151.0, 6162.0, 6165.0, 6162.0, 6127.0, 6114.0, 6109.0, 6114.0, 6084.0
    };

    // Sampling frequency
    double fs = 50.0;

    // Generate SOS with butterworth_bandpass_sos.py
    // SOS sections for fs = 50, lowcut = 0.5, highcut = 5.0, order = 2
    std::vector<std::vector<double>> sos = {
        {0.05644846, 0.11289692, 0.05644846, 1.0, -1.24532221, 0.49035942},
        {1.0, -2.0, 1.0, 1.0, -1.91414109, 0.91860259}
    };

    // Bandpass Filter
    BandpassFilter filter(fs, sos);
    std::vector<double> filtered = filter.apply(input_signal);

    std::cout << "Filtered signal:\n";
    for (double v : filtered) {
        std::cout << v << " ";
    }
    std::cout << std::endl;

    return 0;
}
