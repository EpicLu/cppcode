#include "thread_test.h"

ThreadTest::ThreadTest()
{
    m_data = 6;
    std::cout << "ThreadTest() data is " << m_data << std::endl;
}

void ThreadTest::getThread()
{
    if (m_tmutex.try_lock_for(std::chrono::seconds(2)))
    {
        std::cout << std::this_thread::get_id() << " got lock\n";
        m_data++;
        std::cout << "now data is " << m_data << std::endl;
        sleep(1);
        m_tmutex.unlock();
    }
    else
    {
        std::cout << std::this_thread::get_id() << " not got lock\n";
        std::cout << "now data is " << m_data << std::endl;
    }
}