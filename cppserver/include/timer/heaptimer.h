/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:39:33
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-05-05 18:56:56
 */

#ifndef _HEAPTIMER_H_
#define _HEAPTIMER_H_

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h>
#include <functional>
#include <assert.h>
#include <chrono>
#include "../log/log.h"

using TimeoutCallBack = std::function<void()>;
using Clock = std::chrono::high_resolution_clock;
using MS = std::chrono::milliseconds;
using TimeStamp = Clock::time_point;

struct TimerNode
{
    int id;             // 节点id
    TimeStamp expires;  // 时间戳
    TimeoutCallBack cb; // 回调函数
    bool operator<(const TimerNode &t)
    {
        return expires < t.expires;
    }
};

class HeapTimer // 最小堆: 此类所有TimerNode的比较都是基于时间戳的大小
{
public:
    HeapTimer();
    ~HeapTimer() = default; // 无指针成员 默认

    void add(int id, int timeout, const TimeoutCallBack &cb); // 添加节点
    void adjust(int id, int newExpires);                      // 给指定节点换新时间戳并调整堆
    void work(int id);                                        // 处理节点
    void clear();                                             // 清空m_heap m_ref
    void tick();                                              // 处理超时节点
    void pop();                                               // 弹出一个节点
    int getNextTick();                                        // 获取下一个节点的剩余时间

private:
    void swapNode(size_t i, size_t j);     // 交换节点
    void del(size_t i);                    // 删除节点
    void siftup(size_t i);                 // 节点向上移动 直到大于父节点
    bool siftdown(size_t index, size_t n); // 节点向下移动 直到小于子节点

    std::vector<TimerNode> m_heap;         // 堆节点的一维数组
    std::unordered_map<int, size_t> m_ref; // 存放TimerNode一维数组中对应的索引 其中node的id为key
};

#endif // _HEAPTIMER_H_