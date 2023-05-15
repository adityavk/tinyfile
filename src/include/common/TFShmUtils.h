#ifndef TF_SHM_UTILS_H
#define TF_SHM_UTILS_H

#include <stdint.h>
#include <vector>

// We use first 2 bytes (uint16_t) of SHM to indicate the size of buffer in the SHM
constexpr int SIZEOF_BUFFSIZE_VAR = sizeof(uint16_t);

/**
 * Since snappy's compressed buffer can take upto 32 + input + input/6 bytes,
 * we can only fill SHM upto 6/7 (SHM_SIZE - 2 - 32) bytes, where first 2 bytes are for storing buff size
 */ 
inline uint16_t MAX_BUFFER_SIZE(uint16_t shmSize) {
    if (shmSize <= 32 + SIZEOF_BUFFSIZE_VAR) {
        return 0;
    }
    return (6 * (shmSize - 32 - SIZEOF_BUFFSIZE_VAR)) / 7;
}

inline std::vector<char> readShm(void* shm) {
    uint16_t* sizePtr = (uint16_t*)shm;
    uint16_t size = *sizePtr;
#if DEBUG
    fprintf(stderr, "Used shm memory is %d bytes\n", size);
#endif
    char* buff = static_cast<char*>(static_cast<void*>(sizePtr + 1));
    std::vector<char> data(buff, buff + size);
    return data;
}

inline void writeToShm(void* shm, const std::vector<char>& data) {
    uint16_t* sizePtr = (uint16_t*)shm;
    *sizePtr = data.size();
    char* buff = static_cast<char*>(static_cast<void*>(sizePtr + 1));
    std::copy(data.begin(), data.end(), buff);
}

#endif //TF_SHM_UTILS_H
