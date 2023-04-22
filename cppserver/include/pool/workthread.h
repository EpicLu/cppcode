/*
 * @Author: EpicLu
 * @Date: 2023-04-22 19:00:02
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-04-22 19:48:33
 */

#ifndef __WORKTHREAD_H__
#define __WORKTHREAD_H__

#include <condition_variable>
#include <functional>
#include <iostream>
#include <assert.h>
#include <cstdio>
#include <atomic>
#include <memory>
#include <thread>
#include <queue>
#include <mutex>

class WorkThread
{
public:
    // 状态
    constexpr static int STATE_WAIT = 1;
    constexpr static int STATE_WORK = 2;
    constexpr static int STATE_EXIT = 3;

    WorkThread() = default;
    WorkThread(std::queue<std::function<void()>> &queue);
    ~WorkThread();

    int getState() const;            // 获取线程状态
    void finish();                   // 标记当前线程为finish状态
    std::thread::id getTid();        // 获取线程ID
    std::thread &getCurrentThread(); // 获取当前线程
    void notify(const int &choice);  // 唤醒阻塞在条件变量上的线程

private:
    void work();

    std::queue<std::function<void()>> &m_queue; // 任务队列
    std::atomic_int m_state;                    // 线程状态
    std::atomic_bool m_finish;                  // 结束标志
    std::thread m_thread;                       // 当前线程地址

    // 所有工作线程共用一个互斥量和条件变量 c++17中 可在类中用inline来初始化静态成员变量
    static inline std::mutex m_locker;            // 条件变量用到的互斥量
    static inline std::condition_variable m_cond; // 条件变量 所有等待中的任务会阻塞在这个条件变量上
};

using ThreadPtr = std::shared_ptr<WorkThread>;

#endif // __WORKTHREAD_H__