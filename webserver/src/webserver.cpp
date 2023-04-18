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