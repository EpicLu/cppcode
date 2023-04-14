#include "thread_test.h"

ThreadTest::ThreadTest()
{
    m_data = 6;
    std::cout << "ThreadTest() data is " << m_data << std::endl;
}

void ThreadTest::getThread()
{
    std::unique_lock<std::mutex> uni_lock(mutex1, std::defer_lock);
    uni_lock.lock();
    std::cout << std::this_thread::get_id() << " got lock\n";
    m_data++;
    std::cout << "now data is " << m_data << std::endl;
    uni_lock.release(); // 释放后管理权全部归还给mutex1
                        //  uni_lock.unlock(); // 释放后不起作用
                        // uni_lock.release()->unlock(); //不能两次释放，释放后必须->unlock()
    mutex1.unlock();    // 可用
}