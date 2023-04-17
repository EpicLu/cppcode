#ifndef _REACTOR_H_
#define _REACTOR_H_

#include <iostream>
#include <vector>
#include <map>
#include <functional>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "httphandler.h"

class Reactor
{
public:
    Reactor(); // 创建红黑书 初始化红黑树根

    void addHandler(EventHandler *handler, int fd, uint32_t events); // 将事件挂到树上
    void delHandler(int fd);                                         // 将事件从红黑树中摘除
    void handleEvent();                                              // 监听事件

    ~Reactor();

private:
    int epoll_fd;                             // epoll实例的文件描述符
    std::map<int, EventHandler *> m_handlers; // 文件描述符到事件处理器的映射
    static const int MAX_EVENTS = 16;         // 每次等待的最大事件数
    struct epoll_event m_events[MAX_EVENTS];  // 用于存储等待的事件
};

#endif // _REACTOR_H_