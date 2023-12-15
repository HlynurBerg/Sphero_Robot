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
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_var_;

public:
    void PushFrame(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
        cond_var_.notify_one();
    }

    void WaitAndPopFrame(T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [this]{ return !queue_.empty(); });
        value = std::move(queue_.front());
        queue_.pop();
    }

    //TODO: Maybe utilize TryPop for non-blocking pop if time?
};

#endif//SPHERO_ROBOT_THREAD_SAFE_QUEUE_HPP
