#include <iostream>
#include "dwtransform_db1_features.h"

int main() {
    std::vector<double> features;

    // Example
    std::vector<double> signal = {
        6308.0, 7147.0, 6308.0, 5854.0, 6259.0, 6488.0, 6211.0, 6037.0, 6211.0, 6315.0,
        6211.0, 6144.0, 6211.0, 6240.0, 6211.0, 6213.0, 6259.0, 6267.0, 6259.0, 6273.0,
        6308.0, 6311.0, 6308.0, 6295.0, 6308.0, 6307.0, 6308.0, 6296.0, 6308.0, 6310.0,
        6308.0, 6276.0, 6259.0, 6249.0, 6259.0, 6239.0, 6211.0, 6170.0, 6162.0, 6157.0,
        6162.0, 6151.0, 6162.0, 6165.0, 6162.0, 6127.0, 6114.0, 6109.0, 6114.0, 6084.0
    };

    int level = 4;

    // Run transform and feature extraction
    get_dwt_features(signal, level, features);

    std::cout << "\nExtracted Features:\n";
    for (double f : features) {
        std::cout << f << " ";
    }
    std::cout << "\n\n";

    return 0;
}
