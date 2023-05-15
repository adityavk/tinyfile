#include "service/TFClient.h"
#include <iostream>

using namespace std;

TFClient::TFClient(pid_t pid, int clientPrivateQueueId): pid(pid), sendMessageQueue(clientPrivateQueueId) {}


pid_t TFClient::getPid() const {
    return pid;
}

TFMessageQueue& TFClient::getSendQueue() {
    return sendMessageQueue;
}