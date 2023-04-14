#include "thread_test.h"

void ThreadTest::getThread()
{
    std::cout << "this is " << std::this_thread::get_id() << std::endl;
}