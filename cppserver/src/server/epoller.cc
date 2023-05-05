/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:40:17
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-05-05 17:30:39
 */

#include "server/epoller.h"

Epoller::Epoller(int max_events) // 判断参数是否大于内定好的MAX_EVENTS 如果不大于就默认MAX_EVENTS
    : m_epoll_fd(epoll_create(1)), m_events(max_events > MAX_EVENTS ? max_events : MAX_EVENTS)
{
    assert(m_epoll_fd > 0);
}

bool Epoller::addFd(int fd, uint32_t events)
{
    if (fd < 0)
        return false;

    struct epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &ev) == 0;
}

bool Epoller::modFd(int fd, uint32_t events)
{
    if (fd < 0)
        return false;

    struct epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &ev) == 0;
}

bool Epoller::delFd(int fd)
{
    if (fd < 0)
        return false;

    struct epoll_event ev = {0};
    return epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, &ev) == 0;
}

int Epoller::wait(int timeout)
{
    return epoll_wait(m_epoll_fd, &m_events[0], static_cast<int>(m_events.size()), timeout);
}

int Epoller::getFd(size_t index) const
{
    return m_events[index].data.fd;
}

uint32_t Epoller::getEvents(size_t index) const
{
    return m_events[index].events;
}