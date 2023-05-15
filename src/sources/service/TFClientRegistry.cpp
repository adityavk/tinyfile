#include "common/TFSemOpUtils.h"
#include "service/TFClientRegistry.h"
#include "service/TFServiceSHMManager.h"
#include <iostream>

using namespace std;

void TFClientRegistry::initialize(uint8_t shmCount, uint16_t shmSize) {
    listenerQueue = TFMessageQueue(MSG_Q_PATHNAME, SERVER_QUEUE_PROJ_ID, IPC_CREAT | MSGQ_PERMS);
    this->shmSize = shmSize;
    shmManager.initialize(shmCount, shmSize);
    this->shmCount = shmManager.getShmCount();
}

bool TFClientRegistry::registerClient(pid_t pid, int clientPrivateQueueId) {
    bool success = false;
    decltype(clients)::iterator it;
    if (clients.find(pid) == clients.end()) {
        it = clients.emplace(piecewise_construct, forward_as_tuple(pid), forward_as_tuple(pid, clientPrivateQueueId)).first;
        success = true;
    }
    TFServiceResponseMessage response;
    response.message_type = 1;
    response.params.status = success ? TFServiceResponseStatus::success : TFServiceResponseStatus::clientExists;
    response.params.shmSize = shmManager.getShmActualSize();
    response.params.shmServiceSemId = shmManager.getShmServiceSemId();
    response.params.shmClientSemId = shmManager.getShmClientSemId();
    response.params.clientContentionSemId = shmManager.getClientContentionSemId();
    const vector<int> shmIds = shmManager.getShmIds();
    uint8_t i;
    for (i = 0; i < shmIds.size(); ++i) {
        response.params.shmIds[i] = shmIds[i];
    }
    for (; i < MAX_NUM_SHM_SEGMENTS; ++i) {
        response.params.shmIds[i] = -1;
    }
    if (!TFMessageQueue(clientPrivateQueueId).send(response)) {
        perror("msgsnd failed while responding back to client's registration");
        exit(1);
    }
    return false;
}

void TFClientRegistry::unregisterClient(pid_t pid) {
    const auto it = clients.find(pid);
    if (it != clients.end()) {
        TFMessageQueue clientSendQueue = move(it->second.getSendQueue());
        clients.erase(it);
        
        TFServiceResponseMessage response;
        response.message_type = 1;
        response.params.status = TFServiceResponseStatus::success;
        if (!clientSendQueue.send(response)) {
            perror("msgsnd failed while responding back to client's deregistration");
            exit(1);
        }
    }
}

TFServiceSHMManager& TFClientRegistry::getShmManager() {
    return shmManager;
}

void TFClientRegistry::process_clients() {
    TFServiceRequestMessage message;
    while (!needToStopClientsListenerThread) {
        if (!listenerQueue.receive(message)) {
            exit (1);
        }

        switch (message.params.type) {
            case TFServiceRequestType::registerClient: {
                if (needToStopClientsListenerThread) { break; }
                cout<<"Register client message received from pid "<<message.params.pid<<"\n";
                registerClient(message.params.pid, message.params.privateQueueId);
            }
                break;
            case TFServiceRequestType::unregisterClient:
                cout<<"Unregister client message received from pid "<<message.params.pid<<"\n";
                unregisterClient(message.params.pid);
                break;
            case TFServiceRequestType::stopListeningThread_Internal:
                break;
        }
    }
}

void TFClientRegistry::startClientsListenerThread() {
    clientsListenerThread = thread([this] { process_clients(); });
}

void TFClientRegistry::joinClientsListenerThread() {
    // Send a kill message so that process_clients() thread can break out of the while loop
    needToStopClientsListenerThread = true;

    TFServiceRequestMessage killMessage;
    killMessage.message_type = 1;
    killMessage.params.type = TFServiceRequestType::stopListeningThread_Internal;
    TFMessageQueue(MSG_Q_PATHNAME, SERVER_QUEUE_PROJ_ID, MSGQ_PERMS, false).send(killMessage);

    clientsListenerThread.join();
}

void TFClientRegistry::startShmThreads() {
    if (shmThreads.size() > 0) {
        cerr<<"startShmThreads called more than once!\n";
        return;
    }
    shmThreads.reserve(shmCount);
    for (int i = 0; i < shmCount; ++i) {
        shmThreads.emplace_back([i, this] {
            while (!needToStopShmThreads) {
                auto requestData = shmManager.waitAndReadShmSegment(i);
#if DEBUG
                cout<<"Read string from shm segment "<<i<<"\n";
#endif
                if (needToStopShmThreads) {
                    break;
                }
#if DEBUG
                cout<<"Writing back to the segment...\n";
#endif
                auto compressedData = snappyManager.compress(requestData);
                shmManager.writeToShmSegmentAndSignal(i, compressedData);
#if DEBUG
                cout<<"Wrote string to shm segment "<<i<<"and signalled\n";
#endif
            }
        });
    }
}

void TFClientRegistry::joinShmThreads() {
    needToStopShmThreads = true;
    incrementAllInSemSet(shmManager.getShmServiceSemId(), shmCount);
    for (int i = 0; i < shmCount; ++i) {
        shmThreads[i].join();
    }
}

bool TFClientRegistry::hasClient(pid_t pid) {
    return clients.find(pid) != clients.end();
}

TFClient& TFClientRegistry::getClient(pid_t pid) {
    return clients.at(pid);
}