/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:39:00
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-04-23 03:44:30
 */

#ifndef _BLOCKQUEUE_H_
#define _BLOCKQUEUE_H_

#include <mutex>
#include <deque>
#include <atomic>
#include <sys/time.h>
#include <condition_variable>
#include "log.h"

template <class T>
class BlockQueue
{
public:
    BlockQueue(const size_t &capacity = 1000);
    ~BlockQueue() = default;

    T front();                      // 返回队首元素
    T back();                       // 返回队尾元素
    size_t size();                  // 元素数量
    size_t capacity();              // 队列大小
    void clear();                   // 清空队列
    void push_back(const T &item);  // 尾插
    void push_front(const T &item); // 头插
    void flush();                   // 通知
    void close();                   // 关闭队列
    bool pop(T &item);              // 头部先出
    bool pop(T &item, int timeout); // 可设置超时的pop
    bool empty();                   // 队列空返回true
    bool full();                    // 队列满返回true

private:
    std::deque<T> m_deque;     // 队列
    size_t m_capacity;         // 队列大小
    std::atomic_bool m_finish; // 结束标志

    std::mutex m_locker;                      // 互斥锁
    std::condition_variable m_cond_consumer;  // 条件变量 队列为空阻塞
    std::condition_variable m_cond_productor; // 条件变量 队列满则阻塞
};

template <class T>
BlockQueue<T>::BlockQueue(const size_t &capacity) : m_finish(false)
{
    assert(capacity > 0);
}

template <class T>
void BlockQueue<T>::close()
{
    {
        std::lock_guard<std::mutex> locker(m_locker);
        m_deque.clear();
        m_finish = true;
    }

    m_cond_productor.notify_all();
    m_cond_consumer.notify_all();
};

template <class T>
T BlockQueue<T>::front()
{
    std::lock_guard<std::mutex> locker(m_locker);
    return m_deque.front();
}

template <class T>
T BlockQueue<T>::back()
{
    std::lock_guard<std::mutex> locker(m_locker);
    return m_deque.back();
}

template <class T>
size_t BlockQueue<T>::size()
{
    std::lock_guard<std::mutex> locker(m_locker);
    return m_deque.size();
}

template <class T>
size_t BlockQueue<T>::capacity()
{
    std::lock_guard<std::mutex> locker(m_locker);
    return m_capacity;
}

template <class T>
void BlockQueue<T>::clear()
{
    std::lock_guard<std::mutex> locker(m_locker);
    m_deque.clear();
}

template <class T>
void BlockQueue<T>::push_back(const T &item)
{

    std::unique_lock<std::mutex> locker(m_locker);
    // 当队列关闭时或队列没满可继续执行
    m_cond_productor.wait(locker, [this]() -> bool
                          { return (m_deque.size() < m_capacity) || m_finish; });

    locker.unlock();
    if (m_finish)
        return;

    m_deque.emplace_back(item);

    m_cond_consumer.notify_one();
}

template <class T>
void BlockQueue<T>::push_front(const T &item)
{

    std::unique_lock<std::mutex> locker(m_locker);
    // 当队列关闭时或队列没满可继续执行
    m_cond_productor.wait(locker, [this]() -> bool
                          { return (m_deque.size() < m_capacity) || m_finish; });

    locker.unlock();
    if (m_finish)
        return;

    m_deque.emplace_front(item);

    m_cond_consumer.notify_one();
}

template <class T>
void BlockQueue<T>::flush()
{
    m_cond_consumer.notify_one();
};

template <class T>
bool BlockQueue<T>::empty()
{
    std::lock_guard<std::mutex> locker(m_locker);
    return m_deque.empty();
}

template <class T>
bool BlockQueue<T>::full()
{
    std::lock_guard<std::mutex> locker(m_locker);
    return m_deque.size() >= m_capacity;
}

template <class T>
bool BlockQueue<T>::pop(T &item)
{

    std::unique_lock<std::mutex> locker(m_locker);
    // 当队列关闭时或队列非空可继续执行
    m_cond_consumer.wait(locker, [this]() -> bool
                         { return (!(m_deque.empty())) || m_finish; });
    locker.unlock();
    if (m_finish)
        return;

    item = m_deque.front();
    m_deque.pop_front();

    m_cond_productor.notify_one();
    return true;
}

template <class T>
bool BlockQueue<T>::pop(T &item, int timeout)
{

    std::unique_lock<std::mutex> locker(m_locker);
    // 当队列关闭时或队列非空可继续执行 超时则关闭
    if (m_cond_consumer.wait_for(locker, std::chrono::seconds(timeout), [this]() -> bool
                                 { return (!(m_deque.empty())) || m_finish; }) == std::cv_status::timeout)
        return false;

    locker.unlock();
    if (m_finish)
        return false;

    item = m_deque.front();
    m_deque.pop_front();

    m_cond_productor.notify_one();
    return true;
}

#endif // _BLOCKQUEUE_H_