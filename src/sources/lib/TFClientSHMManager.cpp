#include <iostream>
#include <sys/shm.h>
#include <sys/sem.h>
#include "common/TFIPC.h"
#include "common/TFSemOpUtils.h"
#include "common/TFShmUtils.h"
#include "lib-private/TFClientSHMManager.h"

using namespace std;

void TFClientSHMManager::initialize(uint16_t shmSize, int shmServiceSemId, int shmClientSemId, int clientContentionSemId, const vector<int>& shmIds) {
    this->shmSize = shmSize;
    this->shmServiceSemId = shmServiceSemId;
    this->shmClientSemId = shmClientSemId;
    this->clientContentionSemId = clientContentionSemId;
    shmData.reserve(shmIds.size());
    for (auto& id: shmIds) {
        shmData.emplace_back(id, attachShm(id));
    }
}

void* TFClientSHMManager::attachShm(int shmId) {
    void* shmData;
    if ((shmData = shmat(shmId, NULL, 0)) == (void *)-1) {
        perror("shmat failed while attaching shm");
    }
#if DEBUG
    cerr<<"Attached shm object with id "<<shmId<<"\n";
#endif
    return shmData;
}

TFClientSHMManager::~TFClientSHMManager() {
    for (auto& data: shmData) {
        if (data.second != nullptr) {
#if DEBUG
            cerr<<"Detaching shm object with id "<<data.first<<"\n";
#endif
            if (shmdt(data.second) == -1) {
                perror("shmdt failed");
            }
        }
    }
}

vector<char> TFClientSHMManager::writeToShmSegmentAndRead(uint8_t shmNumber, const vector<char>& input) {
    decrementSemSet(clientContentionSemId, shmNumber);
    // cerr<<"Waiting for shmClientSemId to start writing to segment "<<shmNumber<<"...\n";
    decrementSemSet(shmClientSemId, shmNumber);
    // cerr<<"Acquired shmClientSemId, writing to segment "<<shmNumber<<"...\n";
    writeToShm(shmData[shmNumber].second, input);
    // cerr<<"Written, incrementing service sem for segment "<<shmNumber<<"...\n";
    incrementSemSet(shmServiceSemId, shmNumber);
    // cerr<<"Incremented service sem, waiting for client sem to be signalled for segment "<<shmNumber<<"...\n";
    decrementSemSet(shmClientSemId, shmNumber);
    // cerr<<"Client sem signalled, trying to read shm for segment "<<shmNumber<<"...\n";
    vector<char> result = readShm(shmData[shmNumber].second);
    // cerr<<"Read shm, signalling client sem set for next run to segment "<<shmNumber<<"...\n";
    incrementSemSet(shmClientSemId, shmNumber);
    // cerr<<"Signalled client sem set for next run, returning for segment "<<shmNumber<<"...\n";
    incrementSemSet(clientContentionSemId, shmNumber);
    return result;
}