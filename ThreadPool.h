#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

template <const int num_workers=6>
class ThreadPool {
public:
    ThreadPool() : workers(num_workers), is_alive(true) {
        for (int i = 0; i < num_workers; i++) {
            workers.emplace_back(
                std::thread([&]() {
                    for ( ; ; ) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lck(_mtx);
                            _cv.wait(lck, [&](){ return !tasks.empty() || !is_alive; });
                            if (tasks.empty()) {
                                continue;
                            }
                            if (!is_alive) {
                                break;
                            }
                            task = tasks.back();
                            tasks.pop_back();
                        }
                        task();
                    }
                })
            );
        }
    }
    ~ThreadPool() {
        {
            std::unique_lock lck(_mtx);
            is_alive = false;
        }
        _cv.notify_all();
        for (auto& t: workers) {
            t.join();
        }
    }
    template<typename F, typename... Args>
    auto enqueue(F&& func, Args&&... args) -> std::future<std::result_of_t<F(Args...)>>{
        using return_type = std::result_of_t<F(Args...)>;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(func), std::forward<Args>(args)...)
        );
        std::future<return_type> result = task->get_future();
        {
            std::unique_lock<std::mutex> lck(_mtx);
            tasks.emplace_back(
                [task]() {
                    (*task)();
                }
            );
        }
        _cv.notify_one();
        return result;
    }
private:
    std::vector<std::thread> workers;
    std::vector<std::function<void()>> tasks;
    std::mutex _mtx;
    std::condition_variable _cv;
    bool is_alive;
};

#endif