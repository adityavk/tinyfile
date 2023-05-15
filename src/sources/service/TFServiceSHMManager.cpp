#include <iostream>
#include <sys/shm.h>
#include "common/TFIPC.h"
#include "common/TFSemOpUtils.h"
#include "common/TFShmUtils.h"
#include "service/TFServiceSHMManager.h"

using namespace std;

constexpr int SHM_SERVICE_SEM_PROJ_ID = 1;
constexpr int SHM_ClIENT_SEM_PROJ_ID = 2;
constexpr int CLIENT_CONTENTION_SEM_PROJ_ID = 3;
constexpr int SHM_PROJ_ID_OFFSET = 4;

union Semun {
	int             val;            /* value for SETVAL */
	struct semid_ds *buf;           /* buffer for IPC_STAT & IPC_SET */
	unsigned short  *array;         /* array for GETALL & SETALL */
};

size_t getShmSize(int shmId) {
    shmid_ds buff;
    if ((shmId = shmctl(shmId, IPC_STAT, &buff)) == -1) {
        perror("shmctl failed while getting shm size");
        return 0;
    }
    return buff.shm_segsz;
}

void TFServiceSHMManager::initialize(uint8_t shmCount, uint16_t shmSize) {
    wasInitialized = true;
    this->shmSize = shmSize;
    if (shmCount > MAX_NUM_SHM_SEGMENTS) {
        shmCount = MAX_NUM_SHM_SEGMENTS;
    }
    shmData.reserve(shmCount);
    for (uint8_t i = 0; i < shmCount; ++i) {
        auto shm = createShm(i, shmSize);
        if (shm.second != nullptr) {
            shmData.push_back(shm);
        }
    }
    this->shmSize = getShmSize(shmData[0].first);
    this->shmCount = shmData.size();
    vector<ushort> semInitValues(shmCount, 0);
    shmServiceSemId = createAndInitSem(SHM_SERVICE_SEM_PROJ_ID, shmCount, SEM_DEFAULT_PERMS | IPC_CREAT, semInitValues);

    for (auto& x: semInitValues) {
        x = 1;
    }
    shmClientSemId = createAndInitSem(SHM_ClIENT_SEM_PROJ_ID, shmCount, SEM_DEFAULT_PERMS | IPC_CREAT, semInitValues);
    clientContentionSemId = createAndInitSem(CLIENT_CONTENTION_SEM_PROJ_ID, shmCount, SEM_DEFAULT_PERMS | IPC_CREAT, semInitValues);
}

TFServiceSHMManager::SHMData TFServiceSHMManager::createShm(uint8_t id, uint16_t size) {
    key_t key;
    int shmId = -1;
    if ((key = ftok(SHM_PATHNAME, id + SHM_PROJ_ID_OFFSET)) == -1) {
        perror("ftok failed while creating shm key");
        return {shmId, nullptr};
    }
    if ((shmId = shmget(key, size, SHM_DEFAULT_PERMS | IPC_CREAT)) == -1) {
        perror("shmget failed while getting shm id");
        return {shmId, nullptr};
    }
#if DEBUG
    cerr<<"Created shm object with id "<<shmId<<"\n";
#endif
    void* shmData = nullptr;
    if ((shmData = shmat(shmId, NULL, 0)) == (void *)-1) {
        perror("shmat failed while attaching shm");
    }
#if DEBUG
    cerr<<"Attached shm object with id "<<shmId<<"\n";
#endif
    return {shmId, shmData};
}

int TFServiceSHMManager::createAndInitSem(uint32_t projId, uint32_t numSems, uint32_t semFlags, vector<ushort>& initValues) {
    key_t key;
    int semId = -1;
    if ((key = ftok(SEM_PATHNAME, projId)) == -1) {
        perror("ftok failed while creating sem key");
        return semId;
    }
    if ((semId = semget(key, numSems, semFlags)) == -1) {
        perror("semget failed while getting sem id");
        return semId;
    }
#if DEBUG
    cerr<<"Created sem set with id "<<semId<<"\n";
#endif

    Semun setParams;
    setParams.array = initValues.data();
    if (semctl(semId, 0, SETALL, setParams) == -1) {
        perror("semctl failed while setting ");
    }
#if DEBUG
    cerr<<"Initialized sem set with id "<<semId<<"\n";
#endif
    return semId;
}

int TFServiceSHMManager::getShmCount() {
    return shmCount;
}

int TFServiceSHMManager::getShmActualSize() {
    return shmSize;
}

int TFServiceSHMManager::getShmServiceSemId() {
    return shmServiceSemId;
}

int TFServiceSHMManager::getShmClientSemId() {
    return shmClientSemId;
}

int TFServiceSHMManager::getClientContentionSemId() {
    return clientContentionSemId;
}

vector<int> TFServiceSHMManager::getShmIds() {
    vector<int> shmIds;
    shmIds.reserve(shmCount);
    for (const auto&data: shmData) {
        shmIds.push_back(data.first);
    }
    return shmIds;
}

vector<char> TFServiceSHMManager::waitAndReadShmSegment(uint8_t shmNumber) {
    decrementSemSet(shmServiceSemId, shmNumber);
    return readShm(shmData[shmNumber].second);
}

void TFServiceSHMManager::writeToShmSegmentAndSignal(uint8_t shmNumber, const vector<char>& input) {
    writeToShm(shmData[shmNumber].second, input);
    incrementSemSet(shmClientSemId, shmNumber);
}

TFServiceSHMManager::~TFServiceSHMManager() {
    if (wasInitialized) {
#if DEBUG
        cerr<<"Deleting shmServiceSemId set with id "<<shmServiceSemId<<"\n";
#endif
        if (semctl(shmServiceSemId, 0, IPC_RMID) == -1) {
            perror("failed to delete server-client semaphore");
        }
#if DEBUG
        cerr<<"Deleting shmClientSemId set with id "<<shmClientSemId<<"\n";
#endif
        if (semctl(shmClientSemId, 0, IPC_RMID) == -1) {
            perror("failed to delete server-client semaphore");
        }
#if DEBUG
        cerr<<"Deleting clientContentionSemId set with id "<<clientContentionSemId<<"\n";
#endif
        if (semctl(clientContentionSemId, 0, IPC_RMID) == -1) {
            perror("failed to delete client-contention semaphore");
        }
        for (auto& data: shmData) {
            if (data.second != nullptr) {
#if DEBUG
                cerr<<"Deleting shm object with id "<<data.first<<"\n";
#endif
                if (shmdt(data.second) == -1) {
                    perror("shmdt failed");
                }
                if (shmctl(data.first, IPC_RMID, NULL) == -1) {
                    perror("failed to delete created shm segment");
                }
            }
        }
    }
}