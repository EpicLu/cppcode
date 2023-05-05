/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:39:23
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-05-05 17:33:10
 */

#ifndef _EPOLLER_H_
#define _EPOLLER_H_

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <errno.h>

class Epoller
{
public:
    static constexpr size_t MAX_EVENTS = 4096;

public:
    explicit Epoller(int max_events = MAX_EVENTS);
    ~Epoller() = default;

    // 以下都是对epollctl的封装
    bool addFd(int fd, uint32_t events);
    bool modFd(int fd, uint32_t events);
    bool delFd(int fd);

    int wait(int timeout = -1);             // epollwait的封装
    int getFd(size_t index) const;          // 获取事件对应的fd
    uint32_t getEvents(size_t index) const; // 获取事件的读写属性

private:
    int m_epoll_fd; // 红黑数根节点

    std::vector<struct epoll_event> m_events; // epoll_wait传入的数组
};

#endif // _EPOLLER_H_