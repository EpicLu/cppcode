#include "thread_test.h"

int main(int argc, char **argv)
{
    ThreadTest tt;

    for (int i = 0; i < 5; i++)
    {
        std::thread t(&ThreadTest::getThread, &tt);
        t.detach();
    }

    sleep(10);
    return 0;
}