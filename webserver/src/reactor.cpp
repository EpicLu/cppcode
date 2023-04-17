#include "reactor.h"

Reactor::Reactor()
{
    // 创建一个epoll实例
    epoll_fd = epoll_create(1);
    if (epoll_fd == -1)
    {
        std::cerr << "Failed to create epoll file descriptor\n";
        exit(1);
    }
}