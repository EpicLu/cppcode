#include "threadpool.h"

ThreadPool::ThreadPool(int min_threads, int max_threads, int max_tasks)
{
    // 参数检查
    if (min_threads <= 0 || max_threads <= 0 || max_tasks <= 0)
        throw std::invalid_argument("Invalid arguments");
    if (min_threads > max_threads)
        throw std::invalid_argument("min_threads should not be greater than max_threads");
    // 初始化成员
    m_min_threads = min_threads;
    m_max_threads = max_threads;
    m_max_tasks = max_tasks;
    m_cur_threads = 0;
    m_idle_threads = 0;
    m_stop = false;
    // 创建最小线程
    for (int i = 0; i < min_threads; i++)
        createThread();
}

ThreadPool::~ThreadPool()
{
    m_stop = true;
    // 唤醒所有等待的线程
    cond_task.notify_all();
    cond_thread.notify_all();
    // 等待所有线程结束
    for (auto &t : threads)
    {
        if (t.joinable())
            t.join();
    }
}