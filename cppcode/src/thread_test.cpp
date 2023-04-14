#include "thread_test.h"

ThreadTest::ThreadTest()
{
    m_data = 6;
    std::cout << "ThreadTest() data is " << m_data << std::endl;
}

void ThreadTest::getThread()
{
    std::unique_lock<std::mutex> uni_lock(m_mutex1);
    std::cout << "this is " << std::this_thread::get_id() << std::endl;
    // std::lock_guard<std::mutex> lock1(m_mutex1); // wait只能用唯一锁
    // 如果不加函数判断 则需要另一个线程来唤醒 第一次执行wait默认false直接阻塞并解锁互斥量
    condition.wait(uni_lock);
    // 如果wait加了函数 那么必须要得到一个true的返回值才能继续往下执行
    /*condition.wait(uni_lock, [this]() -> bool
                   {
                    if (m_data == 6)
                    {
                        sleep(1);
                        return true;
                    }
                    else if (m_data > 6)
                        return true;
                    return false; });*/
    std::cout << std::this_thread::get_id() << " got lock\n";
    m_data++;
    std::cout << "now data is " << m_data << std::endl;
    uni_lock.unlock();
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