#ifndef _TASK_H_
#define _TASK_H_

#include <functional>

class Task
{
public:
    Task() = default;
    Task(const Task &t) = default;
    Task(Task &&t) = default;
    template <typename F, typename... Args>
    Task(F &&f, Args &&...args);

    void run();

    ~Task() = default;

private:
    std::function<void()> func;
};

#endif // _TASK_H_