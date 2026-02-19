#include <iostream>

#include "ThreadPool.h"

int main() {
    std::cout << "=== 线程池测试开始 ===\n\n";

    // ---------------- 测试 1：单任务 ----------------
    {
        ThreadPool<4> pool;
        int result = 0;

        auto fut = pool.submit([&result]() {
            result = 10086;
        });

        fut.wait();  // 等待任务完成

        if (result == 10086) {
            std::cout << "[OK] 单任务测试通过\n";
        } else {
            std::cout << "[FAIL] 单任务测试失败，result = " << result << "\n";
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300)); // 给点时间让线程池析构

    // ---------------- 测试 2：多任务 + 计数 ----------------
    {
        ThreadPool<4> pool;
        std::atomic<int> counter{0};
        constexpr int TASK_COUNT = 20;

        std::vector<std::future<void>> futures;
        for (int i = 0; i < TASK_COUNT; ++i) {
            futures.push_back(pool.submit([&counter]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                counter++;
            }));
        }

        for (auto& f : futures) {
            f.wait();
        }

        if (counter == TASK_COUNT) {
            std::cout << "[OK] 多任务计数测试通过 (" << TASK_COUNT << " 个任务)\n";
        } else {
            std::cout << "[FAIL] 计数不对，预期 " << TASK_COUNT << "，实际 " << counter << "\n";
        }
    }

    // ---------------- 测试 3：异常是否会弄死线程池 ----------------
    {
        ThreadPool<2> pool;
        bool second_task_ran = false;

        auto fut1 = pool.submit([]() {
            throw std::runtime_error("故意抛异常");
        });

        auto fut2 = pool.submit([&second_task_ran]() {
            second_task_ran = true;
        });

        try {
            fut1.get();
        } catch (...) {
            std::cout << "[OK] 捕获到任务内异常\n";
        }

        fut2.wait();

        if (second_task_ran) {
            std::cout << "[OK] 异常后线程池仍然可用\n";
        } else {
            std::cout << "[FAIL] 异常导致线程池挂掉，后续任务没执行\n";
        }
    }

    // ---------------- 测试 4：N=1 时是否退化为串行 ----------------
    {
        ThreadPool<1> pool;
        std::vector<int> order;
        std::mutex mtx;

        auto task = [&](int id) {
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
            std::lock_guard<std::mutex> lk(mtx);
            order.push_back(id);
        };

        for (int i = 1; i <= 5; ++i) {
            pool.submit([i, &task]() { task(i); });
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(600));

        bool is_sequential = true;
        for (size_t i = 1; i < order.size(); ++i) {
            if (order[i] <= order[i-1]) {
                is_sequential = false;
                break;
            }
        }

        if (is_sequential && order.size() == 5) {
            std::cout << "[OK] 单线程版本按提交顺序执行\n";
        } else {
            std::cout << "[FAIL] 单线程未按顺序执行 或 任务丢失\n";
            std::cout << "实际顺序: ";
            for (int x : order) std::cout << x << " ";
            std::cout << "\n";
        }
    }

    std::cout << "\n=== 测试结束 ===\n";
    return 0;
}