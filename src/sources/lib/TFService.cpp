#include <iostream>
#include "lib-private/TFService.h"

using namespace std;

bool TFService::registerClient(const string& relativeServicePath) {
    receiveQueue = TFMessageQueue(IPC_PRIVATE, MSGQ_PERMS);
    publicSendQueue = TFMessageQueue((relativeServicePath + MSG_Q_PATHNAME).c_str(), SERVER_QUEUE_PROJ_ID, 0, false);
    pid = getpid();
    if (publicSendQueue.getId() == -1 || receiveQueue.getId() == -1) {
        cerr<<"Unable to connect to the service, check if it is started\n";
        return false;
    }
    TFServiceRequestMessage request;
    TFServiceResponseMessage response;

    request.message_type = 1;
    request.params.pid = pid;
    request.params.type = TFServiceRequestType::registerClient;
    request.params.privateQueueId = receiveQueue.getId();
    if (!publicSendQueue.send(request)) {
        cerr<<"Failed to send registration message to the service\n";
        return false;
    }

    if (!receiveQueue.receive(response) || response.params.status != TFServiceResponseStatus::success) {
        cerr<<"Failed to receive registration response from the service\n";
        return false;
    }

    TFServiceResponseParams responseParams = response.params;
    this->shmSize = responseParams.shmSize;
    vector<int> shmIds;
    shmIds.reserve(MAX_NUM_SHM_SEGMENTS);
    for (int i = 0; i < MAX_NUM_SHM_SEGMENTS; ++i) {
        if (responseParams.shmIds[i] == -1) {
            break;
        }
        shmIds.push_back(responseParams.shmIds[i]);
    }
    shmManager.initialize(responseParams.shmSize, responseParams.shmServiceSemId, responseParams.shmClientSemId, responseParams.clientContentionSemId, shmIds);
    
    this->shmCount = shmIds.size();
    return true;
}

bool TFService::unregisterClient() {
    unregisterInvoked = true;
    TFServiceRequestMessage request;
    TFServiceResponseMessage response;

    request.message_type = 1;
    request.params.type = TFServiceRequestType::unregisterClient;
    request.params.pid = pid;
    if (!publicSendQueue.send(request)) {
        cerr<<"Failed to send deregistration message to the service\n";
        return false;
    }

    if (!receiveQueue.receive(response)) {
        cerr<<"Failed to receive deregistration confirmation from the service\n";
        return false;
    }
    for (auto& t: shmThreads) {
        t.join();
    }
    return true;
}

uint16_t TFService::getShmSize() const {
    return shmSize;
}

uint8_t TFService::getShmCount() const {
    return shmCount;
}

TFClientSHMManager& TFService::getShmManager() {
    return shmManager;
}