#include "common/TFThreadPool.h"

#if DEBUG
#include <iostream>
#endif

using namespace std;

TFThreadPool::TFThreadPool(uint8_t num_threads) {
    threads.reserve(num_threads);
    for (uint8_t i = 0; i < num_threads; ++i) {
        threads.push_back(thread([this] { threadHandler(); }));
    }
}

void TFThreadPool::queue(const Task& task) {
#if DEBUG
    fprintf(stderr, "Queuing task %x on the pool\n", &task);
#endif
    {
        lock_guard<mutex> taskQueueLock(taskQueueMutex);
        taskQueue.push(task);
    }
#if DEBUG
    fprintf(stderr, "Queued task %x on the pool, notifying a thread...\n", &task);
#endif
    stopPoolCV.notify_one();
}

void TFThreadPool::threadHandler() {
    while (true) {
#if DEBUG
        fprintf(stderr, "Inside thread runloop, waiting for condvar...\n");
#endif
        unique_lock<mutex> taskQueueLock(taskQueueMutex);
        stopPoolCV.wait(taskQueueLock, [this] { return !taskQueue.empty() || shouldStop; });
#if DEBUG
        fprintf(stderr, "Condvar wait over, shouldStop: %d, queue empty?: %d...\n", shouldStop, taskQueue.empty());
#endif
        if (!taskQueue.empty()) {
            Task task = taskQueue.front();
            taskQueue.pop();
            ++tasksInProgress;
            taskQueueLock.unlock();

#if DEBUG
            fprintf(stderr, "Picked task %x from the pool, executing...\n", &task);
#endif
            task();

#if DEBUG
            fprintf(stderr, "Finished picked task %x from the pool, decrementing tasksInProgress and notifying...\n", &task);
#endif
            taskQueueLock.lock();
            --tasksInProgress;
            tasksWaitCV.notify_one();
        } else if (shouldStop) {
#if DEBUG
            fprintf(stderr, "Need to stop, exiting runloop\n");
#endif
            break;
        }
    }
}

void TFThreadPool::waitForTasks() {
    unique_lock<mutex> taskQueueLock(taskQueueMutex);
    tasksWaitCV.wait(taskQueueLock, [this] { return taskQueue.empty() && tasksInProgress == 0; });
}

void TFThreadPool::stopProcessing() {
#if DEBUG
        fprintf(stderr, "stopProcessing() invoked for TFThreadPool, asking threads to stop...\n");
#endif
    {
        lock_guard<mutex> taskQueueLock(taskQueueMutex);
        shouldStop = true;
    }
    stopPoolCV.notify_all();
}

TFThreadPool::~TFThreadPool() noexcept {
    stopProcessing();
#if DEBUG
        fprintf(stderr, "Asked all threads to stop, waiting for them to join...\n");
#endif
    for (auto& thread: threads) {
        thread.join();
    }
#if DEBUG
        fprintf(stderr, "Threads joined, Dtor complete!\n");
#endif
}