#include "thread_test.h"

ThreadTest::ThreadTest()
{
    m_data = 6;
    std::cout << "ThreadTest() data is " << m_data << std::endl;
}

void ThreadTest::getThread()
{
    std::cout << std::this_thread::get_id() << " waiting for lock\n";
    std::lock(mutex1, mutex2);
    std::cout << std::this_thread::get_id() << " got lock\n";
    m_data++;
    std::cout << "now data is " << m_data << std::endl;
    mutex1.unlock();
    sleep(1);
    mutex2.unlock();
}