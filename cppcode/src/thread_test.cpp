#include "thread_test.h"

ThreadTest::ThreadTest()
{
    m_data = 6;
    std::cout << "ThreadTest() data is " << m_data << std::endl;
}

int ThreadTest::getThread(std::promise<int> &pm, int val)
{
    pm.set_value(val);
    return m_data;
}

void ThreadTest::wakeCondition()
{
    sleep(1);
    std::unique_lock<std::mutex> uni_lock(m_mutex1);
    std::cout << "this is wakeCondition!\n";
    sleep(1);
    std::cout << "wakeCondition condition.notify_all()\n";
    uni_lock.unlock();
    condition.notify_all();
}