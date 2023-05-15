#include "tiny-file/TinyFile.h"
#include <iostream>
#include <string>
#include <vector>
#include "SnappySerializedCompressor.h"
#include "TFVectorUtils.h"

using namespace std;

SnappySerializedCompressor::SnappySerializedCompressor(uint16_t shmSize) {
    if (shmSize <= 34) {
        buffSize = 0;
    }
    buffSize = (6 * (shmSize - 34)) / 7;
}

vector<char> SnappySerializedCompressor::compress(const vector<char>& input) {
    auto inputSize = input.size();
    int blocks = (inputSize + buffSize - 1) / buffSize;
    
    vector<vector<char>> results(blocks, vector<char>());
    
    int startIndex;
    for (int i = 0; i < blocks; ++i) {
        startIndex = buffSize * i;
        auto buffer = vector<char>(input.begin() + startIndex, i != (blocks - 1) ? (input.begin() + startIndex + buffSize) : input.end());
        results.at(i) = snappyManager.compress(buffer);
    }
    return joinedVector(results);
}