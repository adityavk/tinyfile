#include <sys/ipc.h>
#include <sys/msg.h>
#include <iostream>

#include "common/TFIPC.h"
#include "common/TFMessageQueue.h"
#include "common/TFThreadPool.h"
#include "lib/TinyFile.h"
#include "lib-private/TFService.h"
#include "lib-private/TFCompressor.h"

using namespace std;

constexpr uint8_t ASYNC_REQUESTS_NUM_THREADS = 1;

TFService service;
TFThreadPool asyncRequestsPool(ASYNC_REQUESTS_NUM_THREADS);

bool tf_initialize(string relativeServicePath) {
    return service.registerClient(relativeServicePath);
}

bool tf_terminate() {
    return service.unregisterClient();
}

vector<char> tf_compress(vector<char> inputData) {

    TFCompressor fileProcessor(service);
    return fileProcessor.compress(inputData);
}

void tf_compress_async(vector<char> inputData, TFCompressionCallback callback) {
    asyncRequestsPool.queue([inputData = move(inputData), callback = move(callback)] {
        callback(tf_compress(inputData));
    });
}

void tf_wait_all_async_requests() {
    asyncRequestsPool.waitForTasks();
}