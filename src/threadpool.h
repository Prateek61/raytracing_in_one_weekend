#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class thread_pool
{
private:
    bool should_terminate_;
    std::mutex queue_mutex_;
    std::condition_variable mutex_condition_;
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    static thread_pool* instance_;

    void thread_loop();

    thread_pool() = default;

public:
    static unsigned int num_threads;

    static thread_pool& initialize();
    static thread_pool& get_instance();
    static bool is_initialized();
    void queue_job(std::function<void()> job);
    bool busy();
    void stop();
};

void thread_pool::thread_loop()
{
    while (true)
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            mutex_condition_.wait(lock, [this] { return !tasks_.empty() || should_terminate_; });

            if (should_terminate_ && tasks_.empty())
                return;

            task = tasks_.front();
            tasks_.pop();
        }

        task();
    }
}

thread_pool& thread_pool::initialize()
{
    if (instance_ != nullptr)
    {
        return *instance_;
    }

    instance_ = new thread_pool();

    const auto thread_count = std::thread::hardware_concurrency() * 2;
    num_threads = thread_count;
    std::clog << "Creating thread pool with " << thread_count << " threads\n";
    for (size_t i = 0; i < thread_count; i++)
    {
        instance_->threads_.emplace_back(&thread_pool::thread_loop, instance_);
    }

    return *instance_;
}

thread_pool& thread_pool::get_instance()
{
    return *instance_;
}

bool thread_pool::is_initialized()
{
    return instance_ != nullptr;
}

void thread_pool::queue_job(std::function<void()> job)
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        tasks_.push(job);
    }

    mutex_condition_.notify_one();
}

bool thread_pool::busy()
{
    bool pool_busy;
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        pool_busy = !tasks_.empty();
    }
    return pool_busy;
}

void thread_pool::stop()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        should_terminate_ = true;
    }

    mutex_condition_.notify_all();

    for (auto& thread : threads_)
    {
        thread.join();
    }

    threads_.clear();
    delete instance_;
}