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
    ThreadPool(const ThreadPool &pool) = delete;
    ThreadPool(ThreadPool &&pool) = delete;
    ThreadPool(int min_threads, int max_threads, int min_tasks); // 初始化线程池

    template <typename F, typename... Args>
    bool addTask(F &&f, Args &&...args); // 添加任务到队列中

    ~ThreadPool(); // 回收所有线程销毁线程池

private:
    void createThread(); // 创建线程
    void work();         // 线程工作函数

    int m_min_threads; // 最小线程数
    int m_max_threads; // 最大线程数
    int m_max_tasks;   // 最大任务数
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