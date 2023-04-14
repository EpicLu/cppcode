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

private:
    int m_data;
    std::mutex m_mutex1;
    std::mutex m_mutex2;
    std::recursive_mutex m_rmutex; // 递归锁
};

#endif