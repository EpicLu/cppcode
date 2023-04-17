#include "task.h"

template <typename F, typename... Args>
Task::Task(F &&f, Args &&...args)
{
    // 使用std::bind和std::forward将函数和参数绑定为一个std::function对象
    func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
}

void Task::run()
{
    func();
}