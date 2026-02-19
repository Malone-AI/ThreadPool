#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <future>

template <const int num_worker=6>
class ThreadPool {
public:
    ThreadPool() : workers(num_worker), is_alive(true) {
        for (int i = 0; i < num_worker; i++) {
            workers[i] = std::thread(
                [this]() {
                    while (true) {
                        std::function<void()> t;
                        {
                            std::unique_lock<std::mutex> lck(_mtx);
                            _cv.wait(lck, [this](){ return !is_alive || !tasks.empty(); });
                            if (!is_alive || tasks.empty()) {
                                // It will return when is_alive is false and tasks is empty.
                                return ;
                            }
                            t = std::move(tasks.front());
                            tasks.pop();
                        }
                        t();
                    }
                }
            );
        }
    }
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lck(_mtx);
            is_alive = false;
        }
        _cv.notify_all();
        for (auto& w: workers) {
            w.join();
        }
    }
    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<std::result_of_t<F(Args...)>> {
        using return_type = std::result_of_t<F(Args...)>;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        auto fut = task->get_future();
        {
            std::unique_lock<std::mutex> lck(_mtx);
            tasks.emplace([task]() { (*task)(); });
        }
        _cv.notify_one();
        return fut;
    }
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex _mtx;
    std::condition_variable _cv;
    bool is_alive;
};

#endif