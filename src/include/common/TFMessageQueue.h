#ifndef TF_MESSAGE_QUEUE_H
#define TF_MESSAGE_QUEUE_H

#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include "TFIPC.h"

class TFMessageQueue {
private:
    int messageQueueId;
    bool manageQueue;
public:
    TFMessageQueue(): messageQueueId(-1), manageQueue(false) {}

    TFMessageQueue(const char* pathName, int projectId, int permissions, bool manageQueue = true): manageQueue(manageQueue) {
        key_t key;
        if ((key = ftok(pathName, projectId)) == -1) {
            perror("ftok failed while creating key");
        }

        if ((messageQueueId = msgget(key, permissions)) == -1) {
            perror("msgget failed while getting message queue id");
        }
#if DEBUG
        fprintf(stderr, "Created queue object  with id: %d\n", messageQueueId);
#endif
    }

    TFMessageQueue(key_t key, int permissions, bool manageQueue = true): manageQueue(manageQueue) {
        if ((messageQueueId = msgget(key, permissions)) == -1) {
            perror("msgget failed while getting message queue id");
        }
#if DEBUG
        fprintf(stderr, "Created queue object with id: %d\n", messageQueueId);
#endif
    }

    TFMessageQueue(int messageQueueId): messageQueueId(messageQueueId), manageQueue(false) {}

    int getId() const {
        return messageQueueId;
    }

    template<typename T>
    bool send(const TFQueueMessageT<T>& message) const {
        if (msgsnd(messageQueueId, &message, sizeof(T), 0) == -1) {
            perror("msgsnd failed");
            return false;
        }
        return true;
    }

    template<typename T>
    bool receive(TFQueueMessageT<T>& receivedMessage) const {
        if (msgrcv(messageQueueId, &receivedMessage, sizeof(T), 0, 0) == -1) {
            perror("msgrcv failed");
            return false;
        }
        return true;
    }

    TFMessageQueue(TFMessageQueue&& other) noexcept :
        messageQueueId(std::exchange(other.messageQueueId, -1)),
        manageQueue(std::exchange(other.manageQueue, false)) {}

    TFMessageQueue& operator=(TFMessageQueue&& other) noexcept {
        if(this != &other) {
            messageQueueId = std::exchange(other.messageQueueId, -1);
            manageQueue = std::exchange(other.manageQueue, false);
        }
        return *this;
    }

    ~TFMessageQueue() noexcept {
#if DEBUG
        fprintf(stderr, "Dtor called for queue with id: %d, manageQueue: %d\n", messageQueueId, manageQueue);
#endif
        if (manageQueue && messageQueueId != -1) {
#if DEBUG
            fprintf(stderr, "Removing queue with id: %d\n", messageQueueId);
#endif
            if (msgctl(messageQueueId, IPC_RMID, NULL) == -1) {
                perror("msgctl failed");
                exit(1);
            }
        }
    }
};


#endif //TF_MESSAGE_QUEUE_H
