#ifndef RESHAPE_FOR_CNN_H
#define RESHAPE_FOR_CNN_H

#include <vector>
#include <cstddef>

// Shape metadata wrapper
struct ColumnVector {
    float* data;
    size_t timesteps;
    size_t features;
};

// Overload for std::vector<float>
ColumnVector reshape_for_cnn(std::vector<float>& arr) {
    ColumnVector col;
    col.data = arr.data();
    col.timesteps = arr.size();
    col.features = 1;
    return col;
}

// Helper: convert std::vector<double> -> std::vector<float>
std::vector<float> to_float_vector(const std::vector<double>& src) {
    std::vector<float> dst;
    dst.reserve(src.size());
    for (double val : src) {
        dst.push_back(static_cast<float>(val));
    }
    return dst;
}

#endif // RESHAPE_FOR_CNN_H