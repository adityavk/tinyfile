#include <iostream>
#include <thread>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include "common/TFIPC.h"
#include "common/CommandLineUtils.h"
#include "common/TFMessageQueue.h"
#include "service/TFClientRegistry.h"

using namespace std;

constexpr char NUM_SHM_SEGMENTS_ARG_NAME[] = "n_sms";
constexpr char SHM_SEGMENT_SIZE_ARG_NAME[] = "sms_size";

// Used for registering new clients
bool hasStarted = false;
bool hasExited = false;
TFClientRegistry clientRegistry;

void exit_service() {
    if (!hasStarted || hasExited) {
        return;
    }
    cout<<"Stopping service...\n";
    hasExited = true;

    cout<<"Joining clients listener thread\n";
    clientRegistry.joinClientsListenerThread();
    cout<<"Joining SHM handler thread\n";
    clientRegistry.joinShmThreads();
    cout<<"Service stopped!\n";
}

void init_service(uint8_t shmCount, uint16_t shmSize) {
    clientRegistry.initialize(shmCount, shmSize);
    clientRegistry.startShmThreads();
    clientRegistry.startClientsListenerThread();
}

int main(int argc, char* argv[]) {
    const auto args = parseArgs(argc, argv);
    uint16_t shmSize;
    uint8_t shmCount;

    if (!parseUnsignedLongArg(args, NUM_SHM_SEGMENTS_ARG_NAME, shmCount)) {
        exit(1);
    }
    if (!parseUnsignedLongArg(args, SHM_SEGMENT_SIZE_ARG_NAME, shmSize)) {
        exit(1);
    }
    
    hasStarted = true;
    atexit(exit_service);
    init_service(shmCount, shmSize);

    cout<<"Service started, press Enter to stop:\n";
    cin.ignore();

    exit_service();

    cout<<"Service stopped!\n";
    return 0;
}