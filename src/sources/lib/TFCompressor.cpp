#include <iostream>
#include "lib-private/TFCompressor.h"
#include "common/TFShmUtils.h"
#include "common/TFVectorUtils.h"

using namespace std;

TFCompressor::TFCompressor(TFService& service): service(service) {
    buffSize = MAX_BUFFER_SIZE(service.getShmSize());
    shmCount = service.getShmCount();
    processingThreads = vector<thread>(shmCount);
}

vector<char> TFCompressor::compress(const vector<char>& input) {
    auto inputSize = input.size();
    int blocks = (inputSize + buffSize - 1) / buffSize;
    
    vector<vector<char>> results(blocks, vector<char>());
    
    int startIndex;
    int shmNumber;
    for (int i = 0; i < blocks; ++i) {
        shmNumber = i % shmCount;
        startIndex = buffSize * i;
        auto buffer = vector<char>(input.begin() + startIndex, i != (blocks - 1) ? (input.begin() + startIndex + buffSize) : input.end());
        processingThreads[shmNumber] = thread([this, i, shmNumber, buffer = move(buffer), &results] {
            results.at(i) = service.getShmManager().writeToShmSegmentAndRead(shmNumber, buffer);
        });
        if (i % shmCount == 0) {
            joinThreads();
        }
    }
    if (shmNumber != shmCount - 1) {
        joinThreads();
    }
    return joinedVector(results);
}

void TFCompressor::joinThreads() {
    for (auto& t: processingThreads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

TFCompressor::~TFCompressor() {
    joinThreads();
}