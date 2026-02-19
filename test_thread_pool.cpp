#include <iostream>

#include "ThreadPool.h"

int main() {
    ThreadPool<4> tp;
    auto fut = tp.submit([]() { return 42; });
    std::cout << fut.get() << std::endl;
    return 0;
}