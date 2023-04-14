#include "thread_test.h"

int main(int argc, char **argv)
{
    std::cout << "hello world!\n";

    ThreadTest tt;

    for (int i = 0; i < 5; i++)
    {
        std::thread t(&ThreadTest::getThread, &tt);
        t.detach();
    }

    return 0;
}