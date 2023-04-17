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

std::string HTTPHandler::getLine(int fd)
{
    std::string line = "";
    char c = '\0';
    int n = 0;

    while (c != '\n')
    {
        n = recv(fd, &c, 1, 0);
        if (n > 0)
        {
            if (c == '\r')
            {
                n = recv(fd, &c, 1, MSG_PEEK); // MSG_PEEK表拷贝缓冲区的内容 不直接取
                if ((n > 0) && (c == '\n'))
                    recv(fd, &c, 1, 0);
                else
                    c = '\r';
            }
            line += c;
        }
        else
            c = '\n';
    }
    line += '\0';

    return line;
}