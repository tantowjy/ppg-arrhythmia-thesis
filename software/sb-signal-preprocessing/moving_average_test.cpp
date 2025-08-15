#include <iostream>
#include <vector>
#include "moving_average.h"

int main() {
    std::vector<double> signal = {
        6308.0, 7147.0, 6308.0, 5854.0, 6259.0, 6488.0, 6211.0, 6037.0, 6211.0, 6315.0,
        6211.0, 6144.0, 6211.0, 6240.0, 6211.0, 6213.0, 6259.0, 6267.0, 6259.0, 6273.0,
        6308.0, 6311.0, 6308.0, 6295.0, 6308.0, 6307.0, 6308.0, 6296.0, 6308.0, 6310.0,
        6308.0, 6276.0, 6259.0, 6249.0, 6259.0, 6239.0, 6211.0, 6170.0, 6162.0, 6157.0,
        6162.0, 6151.0, 6162.0, 6165.0, 6162.0, 6127.0, 6114.0, 6109.0, 6114.0, 6084.0
    };

    int window_size = 5;

    std::vector<double> result = moving_average_filter(signal, window_size);

    std::cout << "Filtered signal: \n";
    for (double val : result) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    return 0;
}