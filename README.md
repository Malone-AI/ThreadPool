# Examples
```cpp
#include "ThreadPool.h"
#include <iostream>

int main() {
    ThreadPool<4> pool; // 模板参数指定线程数
    auto fut = pool.submit([]() { return 42; }); // 返回future对象
    std::cout << fut.get() << std::endl;
}
```
# 测试

=== 线程池测试开始 ===

[OK] 单任务测试通过

[OK] 多任务计数测试通过 (20 个任务)

[OK] 捕获到任务内异常

[OK] 异常后线程池仍然可用

[OK] 单线程版本按提交顺序执行

=== 测试结束 ===