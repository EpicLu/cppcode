/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:39:18
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-04-22 20:51:14
 */

#ifndef __THREADPOOL_HPP__
#define __THREADPOOL_HPP__

#include <map>
#include "workthread.h"

class ThreadPool
{
public:
    ThreadPool() = delete;                       // 不允许默认构造
    ThreadPool(const ThreadPool &pool) = delete; // 不允许拷贝操作
    ThreadPool(ThreadPool &&pool) = default;
    explicit ThreadPool(int min, int max);
    ~ThreadPool();

    ThreadPool &operator=(const ThreadPool &pool) = delete; // 不允许赋值操作

    static std::shared_ptr<ThreadPool> getInstance(); // 返回一个线程池实例 且每次返回都是同一个
    void shutdown();                                  // 结束线程池

    template <typename F, typename... Args>
    void addTask(F &&f, Args &&...args); // 将任务添加进队列

private:
    void addThread(); // 添加工作线程
    void delThread(); // 删除工作线程
    void manager();   // 管理者线程

    int m_min;                                                // 最小线程数
    int m_max;                                                // 最大线程数
    std::thread m_manager;                                    // 管理线程
    std::mutex m_locker;                                      // 互斥锁
    std::condition_variable m_cond;                           // 条件变量
    std::atomic_bool m_finish;                                // 结束标志
    std::unordered_map<std::thread::id, ThreadPtr> m_threads; // 工作线程按id存在map中
    std::queue<std::function<void()>> m_tasks;                // 包装函数的任务队列

    static inline std::shared_ptr<ThreadPool> m_pool = nullptr; // 线程池实例 使用inline可类内初始化
};

ThreadPool::ThreadPool(int min, int max) : m_min(min), m_max(max), m_finish(false)
{
    assert(max > min);

    printf("=======pool start=======\n");

    for (unsigned int i = 0; i < m_min; i++)
        addThread(); // 创建最小数量的工作线程

    // 创建管理者线程
    m_manager = std::thread(&ThreadPool::manager, this);
}

ThreadPool::~ThreadPool()
{
    // 回收管理者线程
    if (m_manager.joinable())
        m_manager.join();
    // m_queue和m_threads均有智能指针管理 无需手动销毁

    // 静态成员变量销毁不适合在析构函数中销毁 在shutdown函数中实现
}

void ThreadPool::manager()
{
    int cnt = 0;
    while (!m_finish)
    {
        // 每次运行都申请锁 防止其他函数改变了工作线程的状态
        {
            std::lock_guard<std::mutex> lock(m_locker);
            // 若满足 当前任务数 > 2倍当前线程数 且 当前线程数 < 最大线程数 则认为线程数不够 需添加线程
            if ((m_tasks.size() > 2 * m_threads.size()) && (m_threads.size() < m_max))
                addThread();
            else
            {
                cnt = 0;
                // 使用const auto&减小开销
                for (const auto &thread : m_threads)
                {
                    // 计算空闲线程
                    if ((thread.second)->getState() == WorkThread::STATE_WAIT)
                        ++cnt;
                }
                // 若满足 当前空闲线程 > 2倍当前线程数 且 当前线程数 > 最小线程数 则认为空闲线程过多 删除线程
                if ((cnt > 2 * m_tasks.size()) && (m_threads.size() > m_min))
                    delThread();
            }
        } // lock_guard的作用域
        // 每隔一定时间管理线程才运行一次 减少资源开销 设定为6秒
        std::this_thread::sleep_for(std::chrono::seconds(6));
    }
}

void ThreadPool::addThread()
{
    // 使用make_shared创建对象只需调用一次构造函数
    // std::cout << "add thread " << tid << std::endl;
    auto thread = std::make_shared<WorkThread>(m_tasks);

    m_threads[thread->getTid()] = thread;
}

void ThreadPool::delThread()
{
    std::thread::id tid;

    // 找出状态是STATE_WAIT的线程
    // 使用const auto&减小开销
    for (const auto &thread : m_threads)
    {
        if ((thread.second)->getState() == WorkThread::STATE_WAIT)
        {

            (thread.second)->finish(); // 结束线程
            {
                std::lock_guard<std::mutex> lock(m_locker);
                (thread.second)->notify(1); // 通知此线程
            }
            // 记录遍历到的线程的tid
            tid = thread.first;
            break;
        }
    }

    // std::cout << "del thread " << tid << std::endl;
    // 从map中删除 智能指针自动析构WorkThread对象 析构函数中join回收线程
    m_threads.erase(tid);
}

std::shared_ptr<ThreadPool> ThreadPool::getInstance()
{
    // 如果实例已存在 直接返回
    if (m_pool)
        return m_pool;
    // 如果没创建过实例 则创建一个
    // 使用make_shared创建对象只需调用一次构造函数
    m_pool = std::make_shared<ThreadPool>(8, 16);

    return m_pool;
}

void ThreadPool::shutdown()
{
    printf("pool will finish in seconds...\n");
    // 如果是调用的getInstance来创建的对象 且引用计数不止两个（类静态成员变量一个，外部调用一个）
    // 则不关闭当前线程池 直接返回
    // 直接实例化的对象跳过判断直接shutdown
    if ((this == m_pool.get()) && (m_pool.use_count() > 2))
    {
        printf("pool is used by others, so pool will finish by the other one...\n");
        return;
    }

    std::lock_guard<std::mutex> lock(m_locker); // 锁住 不在允许其他成员函数运行

    m_finish = true;

    // 关闭工作线程
    // 除了线程回收需要干预终止循环 并且join 其余成员变量均可在对象生命周期结束后自行销毁
    for (auto &thread : m_threads)
    {
        (thread.second)->finish();
        // pool销毁map会自动销毁 此时erase反而会导致此对象提前销毁而导致访问空指针报错
        // m_threads.erase(thread.first);
    }
    (m_threads.cbegin())->second->notify(2); // ==notify_all 其中一个线程通知所有线程结束工作
    m_threads.clear();                       // 让容器里的任务对象调用析构函数

    // 如果判断为真 则说明外部调用getInstance的最后一个对象正在执行shutdown
    if ((this == m_pool.get()) && (m_pool.use_count() == 2))
        m_pool.reset(); // 置空静态成员变量 当外界的智能指针生命周期结束后 静态成员变量正式销毁
    // 给6秒钟时间来回收线程
    std::this_thread::sleep_for(std::chrono::milliseconds(6000));
    printf("=======pool finished=======\n");
}

template <typename F, typename... Args>
void ThreadPool::addTask(F &&f, Args &&...args)
{
    assert(f); // 判断任务是否为空

    std::lock_guard<std::mutex> lock(m_locker);
    m_tasks.emplace(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    m_threads.begin()->second->notify(1);
}

#endif // __THREADPOOL_HPP__