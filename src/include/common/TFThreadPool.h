#ifndef TF_THREAD_POOL_H
#define TF_THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>

class TFThreadPool {
public:
    using Task = std::function<void()>;
private:
    std::vector<std::thread> threads;

    std::queue<Task> taskQueue;
    std::mutex taskQueueMutex;
    
    std::condition_variable stopPoolCV;
    std::condition_variable tasksWaitCV;
    bool shouldStop = false;
    unsigned int tasksInProgress = 0;

    void threadHandler();
public:
    TFThreadPool(uint8_t num_threads);

    void queue(const Task& task);

    void stopProcessing();

    void waitForTasks();

    TFThreadPool(const TFThreadPool&) = delete;
    TFThreadPool& operator=(const TFThreadPool&) = delete;

    ~TFThreadPool() noexcept;
};


#endif //TF_THREAD_POOL_H
