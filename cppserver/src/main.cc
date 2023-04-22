/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:40:31
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-04-22 20:46:34
 */

#include "server/webserver.h"
#include "pool/threadpool.hpp"
#include <iostream>
#include <unistd.h>

void test(int val)
{
    printf("val = %d\n", val);
}

int main(int argc, char const *argv[])
{
    auto pool = ThreadPool::getInstance();

    sleep(1);

    pool->addTask(test, 7);

    sleep(1);

    pool->shutdown();

    return 0;
}
