#ifndef TF_SERVICE_H
#define TF_SERVICE_H

#include <unistd.h>
#include <thread>
#include "common/TFMessageQueue.h"
#include "TFClientSHMManager.h"

class TFService {
private:
    TFMessageQueue receiveQueue;
    TFMessageQueue publicSendQueue;
    pid_t pid;
    TFClientSHMManager shmManager;
    std::vector<std::thread> shmThreads;
    uint16_t shmSize;
    uint8_t shmCount;
    bool unregisterInvoked = false;
public:
    TFService() = default;

    bool registerClient(const std::string& relativeServicePath);
    bool unregisterClient();

    uint16_t getShmSize() const;
    uint8_t getShmCount() const;
    TFClientSHMManager& getShmManager();

    TFService(const TFService&) = delete;
    TFService& operator=(const TFService&) = delete;
    ~TFService() = default;
};


#endif //TF_SERVICE_H
