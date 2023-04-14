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

    std::unique_lock<std::mutex> uni_lock(m_mutex1);
    std::cout << "this is wakeCondition!\n";
    sleep(1);
    // unique_lock配合wait 如果定义在循环外且比要唤醒的进程先执行 需要先解锁一次
    // 因为此线程先用唯一锁锁上了互斥量 其他线程会阻塞在初始化unique_lock的互斥量lock上
    // 否则第一次唤醒其他线程是从unlock后初始化unique_lock开始 而不是notify唤醒wait
    // 且循环内需要自行重新上锁
    // 如果嫌麻烦可将定义写在循环内 每次循环都会自行初始化 但要保证循环第一次执行在其他线程阻塞wait时
    uni_lock.unlock();

    for (int i = 0; i < 5; i++)
    {
        sleep(2);
        uni_lock.lock();
        std::cout << "wakeCondition condition.notify_one()\n";
        uni_lock.unlock();
        condition.notify_one();
    }
}