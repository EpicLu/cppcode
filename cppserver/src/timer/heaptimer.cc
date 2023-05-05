/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:40:26
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-05-05 18:49:06
 */

#include "timer/heaptimer.h"

HeapTimer::HeapTimer()
{
    m_heap.reserve(64);
}

void HeapTimer::swapNode(size_t i, size_t j)
{
    assert(i >= 0 && i < m_heap.size());
    assert(j >= 0 && j < m_heap.size());
    std::swap(m_heap[i], m_heap[j]);
    m_ref[m_heap[i].id] = i;
    m_ref[m_heap[j].id] = j;
}

void HeapTimer::siftup(size_t i)
{
    assert(i >= 0 && i < m_heap.size());
    size_t j = (i - 1) / 2;
    while (j >= 0)
    {
        if (m_heap[j] < m_heap[i])
            break;
        // 如果小于父节点 与父节点交换
        swapNode(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}

bool HeapTimer::siftdown(size_t index, size_t n)
{
    assert(index >= 0 && index < m_heap.size());
    assert(n >= 0 && n <= m_heap.size());
    size_t i = index;
    size_t j = i * 2 + 1;

    while (j < n)
    {
        if (j + 1 < n && m_heap[j + 1] < m_heap[j]) // 始终与最小的子结点作比较
            j++;
        if (m_heap[i] < m_heap[j])
            break;
        // 如果大于子节点 与子节点交换
        swapNode(i, j);
        i = j;
        j = i * 2 + 1;
    }

    return i > index; // 当i == index时说明没发生交换
}

void HeapTimer::del(size_t index)
{
    // 删除指定位置的结点
    assert(!m_heap.empty() && index >= 0 && index < m_heap.size());
    // 将要删除的结点换到队尾，然后调整堆
    size_t i = index;
    size_t n = m_heap.size() - 1;
    assert(i <= n);

    if (i < n)
    {
        swapNode(i, n);

        if (!siftdown(i, n))
            siftup(i);
    }
    // 尾元素删除
    m_ref.erase(m_heap.back().id);
    m_heap.pop_back();
}

void HeapTimer::add(int id, int timeout, const TimeoutCallBack &cb)
{
    assert(id >= 0);
    size_t i;

    if (m_ref.count(id) == 0)
    {
        // 新节点：堆尾插入，调整堆
        i = m_heap.size();
        m_ref[id] = i;
        m_heap.push_back({id, Clock::now() + MS(timeout), cb});

        siftup(i);
    }
    else
    {
        // 已有结点：调整堆
        i = m_ref[id];
        m_heap[i].expires = Clock::now() + MS(timeout);
        m_heap[i].cb = cb;

        if (!siftdown(i, m_heap.size()))
            siftup(i);
    }
}

void HeapTimer::work(int id)
{
    // 触发回调函数, 工作完后删除指定id结点
    if (m_heap.empty() || m_ref.count(id) == 0)
    {
        return;
    }
    size_t i = m_ref[id];
    TimerNode &node = m_heap[i];
    node.cb();

    del(i);
}

void HeapTimer::adjust(int id, int timeout)
{
    // 调整指定id的结点
    assert(!m_heap.empty() && m_ref.count(id) > 0);
    m_heap[m_ref[id]].expires = Clock::now() + MS(timeout);

    siftdown(m_ref[id], m_heap.size());
}

void HeapTimer::tick()
{
    // 清除超时结点
    if (m_heap.empty())
        return;

    while (!m_heap.empty())
    {
        TimerNode &node = m_heap.front();
        // 如果时间戳 - 当前时间 > 0 说明没有超时
        if (std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0)
            break;

        node.cb();
        pop();
    }
}

void HeapTimer::pop()
{
    assert(!m_heap.empty());
    del(0);
}

void HeapTimer::clear()
{
    m_ref.clear();
    m_heap.clear();
}

int HeapTimer::getNextTick()
{
    tick();
    size_t res = -1;
    if (!m_heap.empty())
    {
        res = std::chrono::duration_cast<MS>(m_heap.front().expires - Clock::now()).count();

        if (res < 0)
            res = 0;
    }
    return res;
}