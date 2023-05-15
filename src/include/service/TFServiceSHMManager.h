#ifndef TF_SERVICE_SHM_MANAGER_H
#define TF_SERVICE_SHM_MANAGER_H

#include <utility>
#include <vector>
#include <sys/types.h>
#include <sys/sem.h>

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
class TFServiceSHMManager {
private:
    using SHMData = std::pair<int, void*>;
    uint16_t shmSize;
    uint8_t shmCount;
    std::vector<SHMData> shmData;
    
    // Semaphore set used by the service for each shm segment's synchronization, details in the comment above
    int shmServiceSemId;
    // Semaphore set used by the clients for each shm segment's synchronization, details in the comment above
    int shmClientSemId;
    // Semaphore set used to ensure that only one client has access to one shm segment at a time
    int clientContentionSemId;

    bool wasInitialized = false;

    SHMData createShm(uint8_t id, uint16_t size);
    int createAndInitSem(uint32_t projId, uint32_t numSems, uint32_t semFlags, std::vector<ushort>& initValues);
public:
    TFServiceSHMManager() = default;
    void initialize(uint8_t shmCount, uint16_t shmSize);

    int getShmCount();
    int getShmActualSize();
    int getShmServiceSemId();
    int getShmClientSemId();
    int getClientContentionSemId();
    std::vector<int> getShmIds();

    std::vector<char> waitAndReadShmSegment(uint8_t shmNumber);
    void writeToShmSegmentAndSignal(uint8_t shmNumber, const std::vector<char>& input);

    TFServiceSHMManager(const TFServiceSHMManager&) = delete;
    TFServiceSHMManager& operator=(const TFServiceSHMManager&) = delete;

    ~TFServiceSHMManager();
};

#endif //TF_SERVICE_SHM_MANAGER_H
