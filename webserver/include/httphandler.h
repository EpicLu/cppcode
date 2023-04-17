#ifndef _HTTPHANDLER_H_
#define _HTTPHANDLER_H_

#include <fstream>
#include "eventhandler.h"
#include "threadpool.h"
#include "reactor.h"

class HTTPHandler : public EventHandler
{
public:
    HTTPHandler(ThreadPool *pool);

    void handleEvent(int fd, uint32_t events) override; // 将对应的回调函数添加到线程池中
    std::string getLine(int fd);                        // 获取HTTP请求报文的行
    std::string createMessage();                        // 生成HTTP应答报文
    void sendMessage(std::string msg);                  // 发送HTTP应答报文
    void recvEvent(int fd);                             // 读事件的回调函数
    void sendFile(int fd);                              // 写事件的回调函数

private:
    ThreadPool *m_pool;
    std::string m_filename; // 要发送的文件名
};

#endif // _HTTPHANDLER_H_