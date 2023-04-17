#include "httphandler.h"

HTTPHandler::HTTPHandler(ThreadPool *pool)
{
    m_pool = pool;
}

void HTTPHandler::handleEvent(int fd, uint32_t events)
{
    if (events & EPOLLIN)
    {
        // 将读回调函数加入线程池的工作线程
        m_pool->addTask(recvEvent, fd);
    }
    if (events & EPOLLOUT)
    {
        // 将写回调函数加入线程池的工作线程
        m_pool->addTask(sendFile, fd);
    }
    if (events & (EPOLLERR | EPOLLHUP))
    {
        // 关闭连接并处理
    }
}