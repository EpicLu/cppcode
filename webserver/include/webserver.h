#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

#include "threadpool.h"
#include "reactor.h"

class WebServer
{
public:
    WebServer(int min_threads, int max_threads, int max_tasks); // 实例化reactor和线程池

    int initSocket();   // 完成socket，bind，listen 返回lfd
    void listenEvent(); // 监听客户端 将监听到的客户端设置为读事件 挂到红黑树上
    void startServer(); // 启动服务器 让epoll监听事件

    ~WebServer(); // 销毁两个实例化对象 关闭lfd

private:
    ThreadPool *m_pool;
    Reactor *m_reactor;
};

#endif // _WEBSERVER_H_