#ifndef TF_COMPRESSOR_H
#define TF_COMPRESSOR_H

#include "TFService.h"
#include <string>
#include <thread>
#include <vector>

class TFCompressor {
private:
    TFService& service;
    uint16_t buffSize;
    uint8_t shmCount;
    std::vector<std::thread> processingThreads;
    
    void joinThreads();
public:
    TFCompressor(TFService& service);

    std::vector<char> compress(const std::vector<char>& input);

    TFCompressor(const TFCompressor&) = delete;
    TFCompressor& operator=(const TFCompressor&) = delete;

    ~TFCompressor();
};

#endif //TF_COMPRESSOR_H
