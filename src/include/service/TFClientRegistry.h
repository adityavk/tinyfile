#ifndef TF_CLIENT_REGISTRY_H
#define TF_CLIENT_REGISTRY_H

#include <thread>
#include <unordered_map>
#include "TFClient.h"
#include "TFServiceSHMManager.h"
#include "common/TFMessageQueue.h"
#include "common/TFSnappyManager.h"

class TFClientRegistry {
private:
    uint8_t shmCount;
    uint16_t shmSize;
    std::unordered_map<pid_t, TFClient> clients;
    TFMessageQueue listenerQueue;
    TFServiceSHMManager shmManager;
    TFSnappyManager snappyManager;
    std::vector<std::thread> shmThreads;
    std::thread clientsListenerThread;
    bool needToStopShmThreads = false;
    bool needToStopClientsListenerThread = false;

    void process_clients();
public:
    TFClientRegistry() = default;
    void initialize(uint8_t shmCount, uint16_t shmSize);
    bool registerClient(pid_t pid, int clientPrivateQueueId);
    void unregisterClient(pid_t pid);
    bool hasClient(pid_t pid);

    TFServiceSHMManager& getShmManager();

    void startShmThreads();
    void startClientsListenerThread();
    void joinShmThreads();
    void joinClientsListenerThread();

    TFClientRegistry(const TFClientRegistry&) = delete;
    TFClientRegistry& operator=(const TFClientRegistry&) = delete;

    TFClient& getClient(pid_t pid);
};


#endif //TF_CLIENT_REGISTRY_H
