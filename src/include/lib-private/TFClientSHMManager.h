#ifndef TF_CLIENT_SHM_MANAGER_H
#define TF_CLIENT_SHM_MANAGER_H

#include <utility>
#include <vector>
#include <unordered_set>
#include <sys/types.h>

/**
 * SHM synchronization: for each shm segment, we have three semaphores: one in each of shmServiceSemId,
 * shmClientSemId and clientContentionSemId sem sets. Initially all:
 *   - shmServiceSemId sems are 0
 *   - shmClientSemId sems are 1
 *   - clientContentionSemId sems are 1
 * The server threads are waiting on shmServiceSemId sems. When a client (C1) decreases (and acquires)
 * one of the clientContentionSemId sems, no other client can acquire the sems for that shm segment.
 * Then:
 *   1. C1 decrements shmClientSemId, writes to the corresponding shm segment, signals the corresponding
 *      shmServiceSemId and decrements shmClientSemId, which then waits for shmClientSemId to be signalled.
 *   2. Server thread waiting on shmServiceSemId wakes up, reads the shm data, compresses it and writes
 *      back the result. Then it signals shmClientSemId for that segment and decrements shmServiceSemId.
 *   3. Client thread waiting for shmClientSemId wakes up, reads the result, saves it and increments shmClientSemId
 *      again for the next server-client interaction. Then it signals clientContentionSemId also, for the next waiting
 *      client for that shm segment.
*/
class TFClientSHMManager {
private:
    using SHMData = std::pair<int, void*>;
    uint16_t shmSize;
    std::vector<SHMData> shmData;
    
    // Semaphore set used by the service for each shm segment's synchronization, details in the comment above
    int shmServiceSemId = -1;
    // Semaphore set used by the clients for each shm segment's synchronization, details in the comment above
    int shmClientSemId = -1;
    // Semaphore set used to ensure that only one client has access to one shm segment at a time
    int clientContentionSemId = -1;

    void* attachShm(int shmId);
public:
    TFClientSHMManager() = default;
    void initialize(uint16_t shmSize, int shmServiceSemId, int shmClientSemId, int clientContentionSemId, const std::vector<int>& shmIds);

    std::vector<char> writeToShmSegmentAndRead(uint8_t shmNumber, const std::vector<char>& input);

    TFClientSHMManager(const TFClientSHMManager&) = delete;
    TFClientSHMManager& operator=(const TFClientSHMManager&) = delete;

    ~TFClientSHMManager();
};

#endif //TF_CLIENT_SHM_MANAGER_H
