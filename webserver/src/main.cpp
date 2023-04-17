#include "threadpool.h"

void print(int val)
{
    std::cout << "this is " << val << std::endl;
}

int main(int argc, char **argv)
{
    ThreadPool t(8, 16, 8);
    for (int i; i < 3; i++)
        t.addTask(print, i);

    return 0;
}