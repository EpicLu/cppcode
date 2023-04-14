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
    std::lock_guard<std::mutex>(mutex2, std::adopt_lock); // 这样这里就不会在上锁一次
    std::lock_guard<std::mutex>(mutex1, std::adopt_lock);
    std::cout << std::this_thread::get_id() << " got lock\n";
    m_data++;
    std::cout << "now data is " << m_data << std::endl;
}