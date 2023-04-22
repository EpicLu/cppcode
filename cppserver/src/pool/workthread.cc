/*
 * @Author: EpicLu
 * @Date: 2023-04-22 20:21:02
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-04-23 03:45:11
 */

#include "pool/workthread.h"

WorkThread::WorkThread(std::queue<std::function<void()>> &queue) : m_queue(queue), m_state(STATE_WAIT), m_finish(false)
{
    m_thread = std::thread(&WorkThread::work, this);
}

WorkThread::~WorkThread()
{
    // std::cout << "thread " << getTid() << " finished!\n";

    if (m_thread.joinable())
        m_thread.join();
}

void WorkThread::work()
{
    // std::cout << "thread " << std::this_thread::get_id() << " is working!\n";

    while (!m_finish)
    {
        m_state = STATE_WAIT; // 线程每次开始工作都是默认为阻塞状态

        std::unique_lock<std::mutex> unilock(m_locker);

        m_cond.wait(unilock); // 任务队列为空 等待条件变量

        if (m_finish)
            return;

        // 执行任务
        auto task = m_queue.front();
        m_queue.pop();
        unilock.unlock();

        if (task != nullptr)
        {
            m_state = STATE_WORK;
            // std::cout << "thread " << std::this_thread::get_id() << " is runing task!\n";
            task();
        }
        else
            std::cout << "thread " << std::this_thread::get_id() << " task is null!\n";
    }
    // 跳出循环 说明线程结束
    m_state = STATE_EXIT;
}

int WorkThread::getState() const
{
    return m_state;
}

std::thread::id WorkThread::getTid() const
{
    // return std::this_thread::get_id(); // 返回的是当前线程的id 不是工作线程的
    return m_thread.get_id();
}

const std::thread &WorkThread::getCurrentThread()
{
    return m_thread;
}

void WorkThread::finish()
{
    if (!m_finish)
        m_finish = true;
}

void WorkThread::notify(const int &choice)
{
    // 断言防止出现乱给参数
    assert(choice == 1 || choice == 2);

    if (choice == 1)
        m_cond.notify_one();
    if (choice == 2)
        m_cond.notify_all();
}
