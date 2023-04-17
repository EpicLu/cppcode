#include "httphandler.h"

HTTPHandler::HTTPHandler(ThreadPool *pool)
{
    m_pool = pool;
    m_filename = "";
}

void HTTPHandler::handleEvent(int fd, uint32_t events)
{
    if (events & EPOLLIN)
    {
        // 将读回调函数加入线程池的工作线程
        m_pool->addTask(&HTTPHandler::recvEvent, this, fd);
    }
    if (events & EPOLLOUT)
    {
        // 将写回调函数加入线程池的工作线程
        m_pool->addTask(&HTTPHandler::sendFile, this, fd);
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

void HTTPHandler::sendMessage(int fd, int no, std::string status, u_long size)
{
    std::string msg = "";

    // 应答报文首行
    msg += "HTTP/1.1 ";
    msg += std::to_string(no);
    msg += " ";
    msg += status;
    msg += "\r\n";
    // 应答报文第二行
    msg += "Content-Length: ";
    msg += std::to_string(size);
    msg += "\r\n";
    // 第三行
    msg += "Connection: close\r\n";
    // 末行为空行
    msg += "\r\n";

    int ret = send(fd, msg.data(), msg.size(), 0);
}

void HTTPHandler::recvEvent(int fd)
{
}

void HTTPHandler::sendFile(int fd)
{
}
