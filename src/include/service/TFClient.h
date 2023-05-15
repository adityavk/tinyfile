#ifndef TF_CLIENT_H
#define TF_CLIENT_H

#include "common/TFMessageQueue.h"

class TFClient {
private:
    pid_t pid;
    TFMessageQueue sendMessageQueue;
public:
    TFClient(pid_t pid, int clientPrivateQueueId);

    pid_t getPid() const;
    TFMessageQueue& getSendQueue();

    TFClient(TFClient&& other) noexcept :
        pid(std::exchange(other.pid, -1)),
        sendMessageQueue(std::exchange(other.sendMessageQueue, -1)) {}

    TFClient& operator=(TFClient&& other) noexcept {
        if(this != &other) {
            pid = std::exchange(other.pid, -1);
            sendMessageQueue = std::exchange(other.sendMessageQueue, -1);
        }
        return *this;
    }

    ~TFClient() = default;
};


#endif //TF_CLIENT_H
