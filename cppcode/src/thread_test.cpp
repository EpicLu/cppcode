#include "thread_test.h"

ThreadTest::ThreadTest()
{
    m_data = 6;
    std::cout << "ThreadTest() data is " << m_data << std::endl;
}

void ThreadTest::getThread()
{
    std::lock_guard<std::recursive_mutex> lock1(m_rmutex);
    std::lock_guard<std::recursive_mutex> lock2(m_rmutex);
    std::lock_guard<std::recursive_mutex> lock3(m_rmutex);
    std::lock_guard<std::recursive_mutex> lock4(m_rmutex);
    std::lock_guard<std::recursive_mutex> lock5(m_rmutex);
    std::cout << std::this_thread::get_id() << " got lock\n";
    m_data++;
    std::cout << "now data is " << m_data << std::endl;
}