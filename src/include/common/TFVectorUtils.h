#ifndef TF_VECTOR_UTILS_H
#define TF_VECTOR_UTILS_H

#include <vector>
#include <numeric>

// Code adapted from https://stackoverflow.com/a/35291919 to join a vector of vectors into a single vector
template<typename T>
inline std::vector<T> joinedVector(const std::vector<std::vector<T>>& input)
{
    std::vector<T> result;
    result.reserve(std::accumulate(input.begin(), input.end(), std::size_t{}, [](auto size, auto const& inner) {
        return size + inner.size();
    }));
    for (auto const& vec : input) {
        result.insert(result.end(), vec.begin(), vec.end());
    }
    return result;
}

#endif // TF_VECTOR_UTILS_H
