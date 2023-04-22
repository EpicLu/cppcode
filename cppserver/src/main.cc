/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:40:31
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-04-23 00:06:44
 */

#include "server/webserver.h"
#include "buffer/buffer.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>

void test(int val)
{
    printf("val = %d\n", val);
}

int main(int argc, char const *argv[])
{
    Buffer buf(BUFSIZ);

    int fd = open("README.md", O_RDONLY);
    int err = 0;

    buf.readFd(fd, &err);
    if (err == -1)
        perror("read error! ");

    std::string str = buf.to_string();
    std::cout << str << std::endl;

    return 0;
}
