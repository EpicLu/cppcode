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
    // 等待所有线程结束
    for (auto &t : threads)
    {
        if (t.joinable())
            t.join();
    }
}

template <typename F, typename... Args>
bool ThreadPool::addTask(F &&f, Args &&...args)
{
    // 创建一个任务对象
    Task task(std::forward<F>(f), std::forward<Args>(args)...);
    // 上锁，保护任务队列和线程数目
    std::unique_lock<std::mutex> lock(mutex);
    // 判断任务队列是否已满
    if (tasks.size() >= max_tasks)
        return false; // 任务队列已满，添加失败
    tasks.push(std::move(task));
    // 判断是否需要创建新的线程
    if (idle_threads == 0 && cur_threads < max_threads)
        create_thread(); // 创建新的线程
    // 唤醒一个等待的线程
    lock.unlock();
    cond_task.notify_one();
    return true; // 添加成功
}

void ThreadPool::createThread()
{
    // 创建一个线程对象，传入work函数作为线程函数
    std::thread t(&ThreadPool::work, this);
    // 将线程对象加入列表
    threads.push_back(std::move(t));
    // 更新当前线程数目和空闲线程数目
    m_cur_threads++;
    m_idle_threads++;
}

void ThreadPool::work()
{
    while (1)
    {
        // 定义一个任务对象
        Task task;
        // 上锁，保护任务队列和线程数目
        std::unique_lock<std::mutex> lock(mutex);
        // 判断线程池是否关闭
        if (m_stop)
            break;
        if (tasks.empty())
        {
            // 等待任务
            cond_task.wait(lock);
            if (m_stop)
                break;
        }
        // 判断是否需要销毁多余的线程
        if (m_cur_threads > m_min_threads)
        {
            // 减少当前线程数目和空闲线程数目
            m_cur_threads--;
            m_idle_threads--;
            lock.unlock();
            // 当前线程结束
            break;
        }
        // 取出任务
        task = std::move(tasks.front());
        tasks.pop();
        // 解锁，释放任务队列
        lock.unlock();
        // 执行任务
        task.run();
    }
}