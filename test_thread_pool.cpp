#include "ThreadPool.h"
#include <thread>
#include <iostream>

int main() {
    // for (int threads: {1, 2, 4, 8, std::thread::hardware_concurrency()}) {
    // }
    ThreadPool<4> tp;
    auto it = tp.enqueue([]{ return 4;});
    std::cout << it.get() << std::endl;
}