#ifndef TF_IPC_TYPES_H
#define TF_IPC_TYPES_H

#include <thread>

constexpr const char* MSG_Q_PATHNAME = ".";
constexpr int SERVER_QUEUE_PROJ_ID = 1;
constexpr int MSGQ_PERMS = 0660;

constexpr const char* SHM_PATHNAME = "..";
constexpr int SHM_DEFAULT_PERMS = 0660;

constexpr const char* SEM_PATHNAME = "..";
constexpr int SEM_DEFAULT_PERMS = 0660;

constexpr uint8_t MAX_NUM_SHM_SEGMENTS = 32;
constexpr uint16_t MAX_SHM_SIZE = UINT16_MAX;

template<typename T> struct TFQueueMessageT {
    long message_type;
    T params;
};

enum class TFServiceRequestType: short {
    registerClient,
    unregisterClient,
    stopListeningThread_Internal
};

struct TFServiceRequestParams {
    TFServiceRequestType type;
    pid_t pid;
    int privateQueueId;
};

using TFServiceRequestMessage = TFQueueMessageT<TFServiceRequestParams>;

enum class TFServiceResponseStatus: short {
    success,
    clientExists,
    failure
};

struct TFServiceResponseParams {
    TFServiceResponseStatus status;
    uint16_t shmSize;
    int shmServiceSemId;
    int shmClientSemId;
    int clientContentionSemId;
    int shmIds[MAX_NUM_SHM_SEGMENTS];
};

using TFServiceResponseMessage = TFQueueMessageT<TFServiceResponseParams>;


enum class TFServicePrivateRequestType: short {
    test1,
    test2,
    stopListeningThread_Internal
};

struct TFServicePrivateRequestParams {
    TFServicePrivateRequestType type;
};

using TFServicePrivateRequestMessage = TFQueueMessageT<TFServicePrivateRequestParams>;

#endif //TF_IPC_TYPES_H
