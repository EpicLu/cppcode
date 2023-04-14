#include "thread_test.h"

ThreadTest::ThreadTest()
{
    m_data = 6;
    std::cout << "ThreadTest() data is " << m_data << std::endl;
}

void ThreadTest::getThread()
{
    std::cout << std::this_thread::get_id() << " got lock\n";
    std::call_once(test_flag, &ThreadTest::test, this);
    // m_data++;
    // std::cout << "now data is " << m_data << std::endl;
}