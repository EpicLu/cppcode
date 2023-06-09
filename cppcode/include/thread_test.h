#ifndef __THREAD_TEST_H__
#define __THREAD_TEST_H__

#include <unistd.h>
#include <condition_variable>
#include <iostream>
#include <future>
#include <thread>
#include <mutex>

class ThreadTest
{
public:
    ThreadTest();
    int getThread(std::promise<int> &pm, int val);
    void wakeCondition();

private:
    int m_data;
    std::mutex m_mutex1;
    std::mutex m_mutex2;
    std::recursive_mutex m_rmutex; // 递归锁
    std::timed_mutex m_tmutex;     // 时间锁
    std::once_flag test_flag;
    std::condition_variable condition; // 条件变量
};

#endif