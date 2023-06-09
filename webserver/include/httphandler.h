#ifndef _HTTPHANDLER_H_
#define _HTTPHANDLER_H_

#include <fstream>
#include "eventhandler.h"
#include "threadpool.h"
#include "reactor.h"

class Reactor;

class HTTPHandler : public EventHandler
{
public:
    HTTPHandler(ThreadPool *pool, Reactor *ptr);

    void handleEvent(int &fd, uint32_t &events) override; // 将对应的回调函数添加到线程池中
    std::string getLine(int &fd);                         // 获取HTTP请求报文的行
    std::string getType(const std::string filename);
    void sendMessage(int &fd, int no, std::string status, u_long size); // 生成HTTP应答报文
    void sendErr(int &fd);                                              // 发送404页面
    void recvEvent(int &fd);                                            // 读事件的回调函数
    void sendFile(int &fd);                                             // 写事件的回调函数
    void acceptConn();

    ~HTTPHandler();

private:
    ThreadPool *m_pool;
    Reactor *m_reactor;
    std::string m_filename; // 要发送的文件名
    static int listen_fd;
};

#endif // _HTTPHANDLER_H_