#ifndef _TASK_H_
#define _TASK_H_

#include <functional>

class Task
{
public:
    template <typename F, typename... Args>
    Task(F &&f, Args &&...args); // 初始化任务

    void run(); // 运行函数

private:
    std::function<void()> func; // 包装工作函数
};

#endif // _TASK_H_