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
    bool addTask(F &&f, Args &&...args) // 添加任务到队列中 模板函数不可份文件定义
    {
        // 创建一个任务对象
        Task task(std::forward<F>(f), std::forward<Args>(args)...);
        // 上锁，保护任务队列和线程数目
        std::unique_lock<std::mutex> lock(mutex);
        // 判断任务队列是否已满
        if (tasks.size() >= m_max_tasks)
            return false; // 任务队列已满，添加失败
        tasks.push(std::move(task));
        // 判断是否需要创建新的线程
        if (m_idle_threads == 0 && m_cur_threads < m_max_threads)
            createThread(); // 创建新的线程
        // 唤醒一个等待的线程
        lock.unlock();
        cond_task.notify_one();
        return true; // 添加成功
    }

    ~ThreadPool(); // 回收所有线程销毁线程池

private:
    void createThread(); // 创建线程
    void work();         // 线程工作函数

    int m_min_threads; // 最小线程数
    int m_max_threads; // 最大线程数
    int m_max_tasks;   // 最大任务数
    bool m_stop;
    int m_cur_threads;                 // 当前线程数目
    int m_idle_threads;                // 空闲线程数目
    std::queue<Task> tasks;            // 任务队列
    std::vector<std::thread> threads;  // 线程列表
    std::mutex mutex;                  // 互斥锁，保护任务队列和线程数目
    std::condition_variable cond_task; // 条件变量，用于通知有新的任务到来
};

#endif // _THREADPOOL_H_