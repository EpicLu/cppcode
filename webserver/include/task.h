#ifndef _TASK_H_
#define _TASK_H_

#include <functional>

class Task
{
public:
    Task() = default;
    template <typename F, typename... Args>
    Task(F &&f, Args &&...args) // 初始化任务 模板函数不可分文件定义
    {
        // 使用std::bind和std::forward将函数和参数绑定为一个std::function对象
        func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    }

    void run(); // 运行函数

private:
    std::function<void()> func; // 包装工作函数
};

#endif // _TASK_H_