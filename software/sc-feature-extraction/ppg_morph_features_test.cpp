#include <iostream>
#include <chrono>
#include "ppg_morph_features.h"

int main() {
    std::vector<double> segment_test = {
        6308.0, 7147.0, 6308.0, 5854.0, 6259.0, 6488.0, 6211.0, 6037.0, 6211.0, 6315.0,
        6211.0, 6144.0, 6211.0, 6240.0, 6211.0, 6213.0, 6259.0, 6267.0, 6259.0, 6273.0,
        6308.0, 6311.0, 6308.0, 6295.0, 6308.0, 6307.0, 6308.0, 6296.0, 6308.0, 6310.0,
        6308.0, 6276.0, 6259.0, 6249.0, 6259.0, 6239.0, 6211.0, 6170.0, 6162.0, 6157.0,
        6162.0, 6151.0, 6162.0, 6165.0, 6162.0, 6127.0, 6114.0, 6109.0, 6114.0, 6084.0
    };

    auto start = std::chrono::high_resolution_clock::now();

    double frequency = 50.0;
    std::vector<int> peaks;
    std::vector<FeatureSet> mf_features = extract_morph_features(segment_test, frequency, peaks);

    // Vector first peak
    std::vector<double> morph_features = morphological_features(mf_features);

    std::cout << "\nExtracted Features:\n";
    for (double f : morph_features) {
        std::cout << f << " ";
    }
    std::cout << "\n";

    std::cout << "\nTotal number of features: " << morph_features.size() << "\n \n";

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Elapsed time: " << duration.count() << " ms" << std::endl;

    return 0;
}