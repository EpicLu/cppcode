/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:40:31
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-04-23 00:06:44
 */

#include "server/webserver.h"
#include "log/log.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>

int brithDay()
{
    return 1206;
}

int main(int argc, char const *argv[])
{
    Log::getInstance()->init(0, "./log", ".log", 1024);
    for (size_t i = 0; i < 77777; i++)
        LOG_INFO("lpc %d", brithDay());

    sleep(3);

    return 0;
}
