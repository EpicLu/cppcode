#include "thread_test.h"
#include <functional>

int main(int argc, char **argv)
{
    std::promise<int> myprom;
    ThreadTest tt;

    std::packaged_task<int(std::promise<int> &, int)> func(std::bind(&ThreadTest::getThread, &tt, std::placeholders::_1, std::placeholders::_2));

    std::thread thread1(std::ref(func), std::ref(myprom), 120);
    thread1.join();

    int value = func.get_future().get();
    std::cout << "return value " << value << std::endl;
    std::cout << "get value is " << myprom.get_future().get() << std::endl;

    return 0;
}