#include "thread_test.h"

int main(int argc, char **argv)
{
    ThreadTest tt;

    std::thread tc(&ThreadTest::wakeCondition, &tt);

    for (int i = 0; i < 5; i++)
    {
        std::thread t(&ThreadTest::getThread, &tt);
        t.detach();
    }

    tc.join();
    sleep(1);
    return 0;
}