#ifndef SPHERO_ROBOT_THREAD_SAFE_QUEUE_HPP
#define SPHERO_ROBOT_THREAD_SAFE_QUEUE_HPP

//reference: https://stackoverflow.com/questions/15278343/c11-thread-safe-queue
// https://www.codeproject.com/Articles/5281878/Producer-Consumer-Queues-in-Cplusplus

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue;
    mutable std::mutex mutex;
    std::condition_variable cond_var;

public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(std::move(value));
        cond_var.notify_one();
    }
    //Remove this function if not needed later - robert sjekker
    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(mutex);
        if (queue.empty()) {
            return false;
        }
        value = std::move(queue.front());
        queue.pop();
        return true;
    }

    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lock(mutex);
        cond_var.wait(lock, [this]{ return !queue.empty(); });
        value = std::move(queue.front());
        queue.pop();
    }

};

#endif//SPHERO_ROBOT_THREAD_SAFE_QUEUE_HPP
