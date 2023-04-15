#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <queue>
#include <thread>
#include <vector>
#include <iostream>
#include <condition_variable>
#include "task.h"

class ThreadPool
{
public:
    ThreadPool() = default;
    ThreadPool(const ThreadPool &pool) = delete;
    ThreadPool(ThreadPool &&pool) = delete;
    ThreadPool(int min_threads, int max_threads, int min_tasks);

    template <typename F, typename... Args>
    bool addTask(F &&f, Args &&...args);

    ~ThreadPool();

private:
    void createThread();
    void work();

    int m_min_threads;
    int m_max_threads;
    int m_min_tasks;
    bool m_stop;
    int m_cur_threads;                   // 当前线程数目
    int m_idle_threads;                  // 空闲线程数目
    std::queue<Task> tasks;              // 任务队列
    std::vector<std::thread> threads;    // 线程列表
    std::mutex mutex;                    // 互斥锁，保护任务队列和线程数目
    std::condition_variable cond_task;   // 条件变量，用于通知有新的任务到来
    std::condition_variable cond_thread; // 条件变量，用于通知有多余的线程可以退出
};

#endif // _THREADPOOL_H_