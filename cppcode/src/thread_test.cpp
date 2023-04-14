#include "thread_test.h"

ThreadTest::ThreadTest()
{
    m_data = 6;
    std::cout << "ThreadTest() data is " << m_data << std::endl;
}

void ThreadTest::getThread()
{
    std::cout << "this is " << std::this_thread::get_id() << std::endl;
    testMutex.lock();
    m_data++;
    std::cout << "now data is " << m_data << std::endl;
    testMutex.unlock();
}