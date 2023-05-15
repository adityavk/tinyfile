#ifndef TF_SNAPPY_MANAGER_H
#define TF_SNAPPY_MANAGER_H

#include <vector>
#if DEBUG
#include <iostream>
#include <string>
#endif

extern "C" {
    #include "snappy-c/snappy.h"
}

class TFSnappyManager {
private:
    snappy_env snappyEnv;
#if DEBUG
    uint64_t totalInput;
    uint64_t totalOutput;
#endif
public:
    TFSnappyManager() {
        snappy_init_env(&snappyEnv);
    }
    
    std::vector<char> compress(const std::vector<char>& input) {
        const auto inputSize = input.size();
        auto length = snappy_max_compressed_length(inputSize);
        std::vector<char> buff(length, 0);
        snappy_compress(&snappyEnv, input.data(), inputSize, buff.data(), &length);
        buff.resize(length);

#if DEBUG
        totalInput += inputSize;
        totalOutput += length;
        std::string output = "[DEBUG]: " + std::to_string(inputSize) + ": " + std::to_string(length) + "\n";
        if (length > inputSize) {
            output += "[DEBUG]: size increase is " + std::to_string(length - inputSize) + "\n";
        }
        std::cout<<output;
#endif
        return buff;
    }

    ~TFSnappyManager() noexcept {
        snappy_free_env(&snappyEnv);
#if DEBUG
        if (totalInput > 0) {
            double sizeDecreasePercent = 100.0 * (((double)totalInput) - totalOutput) / totalInput;
            fprintf(stdout,"Total input: %llu, total output: %llu, size decrease: %lf%%\n", totalInput, totalOutput, sizeDecreasePercent);
        }
#endif
    }
};


#endif //TF_SNAPPY_MANAGER_H
