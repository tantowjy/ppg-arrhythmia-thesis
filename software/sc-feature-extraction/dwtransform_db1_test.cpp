#include <iostream>
#include "dwtransform_db1.h"

int main() {
    std::vector<double> signal = {
        6308.0, 7147.0, 6308.0, 5854.0, 6259.0, 6488.0, 6211.0, 6037.0, 6211.0, 6315.0,
        6211.0, 6144.0, 6211.0, 6240.0, 6211.0, 6213.0, 6259.0, 6267.0, 6259.0, 6273.0,
        6308.0, 6311.0, 6308.0, 6295.0, 6308.0, 6307.0, 6308.0, 6296.0, 6308.0, 6310.0,
        6308.0, 6276.0, 6259.0, 6249.0, 6259.0, 6239.0, 6211.0, 6170.0, 6162.0, 6157.0,
        6162.0, 6151.0, 6162.0, 6165.0, 6162.0, 6127.0, 6114.0, 6109.0, 6114.0, 6084.0
    };

    std::vector<std::vector<double>> cD;
    std::vector<double> cA;

    int n_level = 4;

    get_dwt_coeffs(signal, n_level, cD, cA);

    for (int i = 0; i < cD.size(); ++i) {
        std::cout << "cD" << i+1 << ": ";
        for (double val : cD[i]) std::cout << val << " ";
        std::cout << "\n";
    }

    std::cout << "cA" << n_level << ": ";
    for (double val : cA) std::cout << val << " ";
    std::cout << "\n";
}
