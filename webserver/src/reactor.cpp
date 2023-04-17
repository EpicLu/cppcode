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

Reactor::~Reactor()
{
    close(epoll_fd); // 关闭红黑树
}

void Reactor::addHandler(EventHandler &handler, int fd, uint32_t events)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) // 将事件挂到红黑书上
    {
        std::cerr << "Failed to add file descriptor to epoll\n";
        exit(1);
    }
    handlers[fd] = handler;
}