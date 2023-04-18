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
    // 置空智能指针让其销毁创建的对象
    auto it = m_handlers.begin();
    for (; it != m_handlers.end(); it++)
        (it->second).reset();
    close(epoll_fd); // 关闭红黑树
}

void Reactor::addHandler(std::unique_ptr<EventHandler> &&handler, int fd, uint32_t events)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) // 将事件挂到红黑书上
    {
        std::cerr << "Failed to add file descriptor to epoll\n";
        exit(1);
    }
    m_handlers[fd] = std::move(handler);
}

void Reactor::delHandler(int fd)
{
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
    {
        std::cerr << "Failed to remove file descriptor from epoll\n";
        exit(1);
    }
    m_handlers.erase(fd);
}

void Reactor::handleEvent()
{
    while (1)
    {
        int ret = epoll_wait(epoll_fd, m_events, MAX_EVENTS, 0);
        if (ret == -1)
        {
            std::cerr << "Failed to wait for epoll events\n";
            exit(1);
        }
        for (int i = 0; i < ret; i++)
        {
            int fd = m_events[i].data.fd;
            uint32_t events = m_events[i].events;
            // auto handler = m_handlers[fd];
            if (m_handlers[fd])
                m_handlers[fd]->handleEvent(fd, events); // 调用EventHandler类中的函数，此函数调用对应事件的回调函数
        }
    }
}