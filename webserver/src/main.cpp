#include "webserver.h"

void print()
{
    std::cout << "this is " << std::this_thread::get_id() << std::endl;
}

void runpool()
{
    ThreadPool t(4, 8, 4);
    for (int i = 1; i <= 3; i++)
        t.addTask(print);
}

int main(int argc, char **argv)
{
    runpool();

    return 0;
}