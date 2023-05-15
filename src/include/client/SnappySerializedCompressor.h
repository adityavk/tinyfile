#ifndef SNAPPY_SERIALIZED_COMPRESSOR_H
#define SNAPPY_SERIALIZED_COMPRESSOR_H

#include <string>
#include <vector>
#include "TFSnappyManager.h"

class SnappySerializedCompressor {
private:
    uint16_t buffSize;
    TFSnappyManager snappyManager;
public:
    SnappySerializedCompressor(uint16_t shmSize);
    std::vector<char> compress(const std::vector<char>& input);
};


#endif //SNAPPY_SERIALIZED_COMPRESSOR_H
