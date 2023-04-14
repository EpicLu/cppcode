#ifndef __THREAD_TEST_H__
#define __THREAD_TEST_H__

#include <unistd.h>
#include <iostream>
#include <thread>
#include <mutex>

class ThreadTest
{
public:
    ThreadTest();
    void getThread();
    void test() { std::cout << "test\n"; };

private:
    int m_data;
    std::mutex m_mutex1;
    std::mutex m_mutex2;
    std::recursive_mutex m_rmutex; // 递归锁
    std::timed_mutex m_tmutex;     // 时间锁
    std::once_flag test_flag;
};

#endif