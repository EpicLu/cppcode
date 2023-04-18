#include "webserver.h"

WebServer::WebServer(int min_threads, int max_threads, int max_tasks)
{
    m_pool = new ThreadPool(min_threads, max_threads, max_tasks);
    if (m_pool == nullptr)
    {
        std::cerr << "pool init error!\n";
        exit(1);
    }
    m_reactor = new Reactor();
    if (m_reactor == nullptr)
    {
        std::cerr << "reactor init error!\n";
        exit(1);
    }
}

WebServer::~WebServer()
{
    if (m_pool != nullptr)
    {
        delete m_pool;
        m_pool = nullptr;
    }
    if (m_reactor != nullptr)
    {
        delete m_reactor;
        m_reactor = nullptr;
    }
}

void WebServer::initSocket(int port)
{
    int ret, i;
    int opt = 1;
    struct sockaddr_in sin = {0};

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
    {
        std::cerr << "socket error!\n";
        exit(1);
    }
    ret = fcntl(lfd, F_SETFL, O_NONBLOCK); // 将socket设为非阻塞
    if (ret == -1)
    {
        std::cerr << "fcntl error!\n";
        exit(1);
    }

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // 端口复用

    ret = bind(lfd, (struct sockaddr *)&sin, sizeof(sin));
    if (ret == -1)
    {
        std::cerr << "bind error!\n";
        exit(1);
    }

    ret = listen(lfd, MAX_EVENTS);
    if (ret == -1)
    {
        std::cerr << "listen error!\n";
        exit(1);
    }

    std::unique_ptr<HTTPHandler> handler(new HTTPHandler(m_pool, m_reactor));
    m_reactor->addHandler(std::move(handler), lfd, EPOLLIN | EPOLLET);
}