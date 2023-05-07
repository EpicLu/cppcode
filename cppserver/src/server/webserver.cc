/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:40:21
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-05-07 21:19:56
 */

#include "server/webserver.h"

WebServer::WebServer(
    int port, int trigMode, int timeoutMS, bool OptLinger,
    int sqlPort, const char *sqlUser, const char *sqlPwd, const char *dbName,
    int connPoolNum, int threadNum, bool openLog, int logLevel, int logQueSize)
    : m_port(port), m_open_linger(OptLinger), m_timeout(timeoutMS), m_close(false)
{
    m_timer = std::make_unique<HeapTimer>();
    m_pool = std::make_unique<ThreadPool>(threadNum, 8);
    m_epoller = std::make_unique<Epoller>();
    m_users.reserve(128);

    m_src_dir = getcwd(nullptr, 256); // 传的是nullptr 所以是malloc出来的字符串
    assert(m_src_dir);
    strncat(m_src_dir, "/resources/", 16);
    HttpConn::setUseCount(0);
    HttpConn::setSrcDir(m_src_dir);
    SqlPool::getInstance()->init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);

    initEventMode(trigMode);
    if (!initSocket())
        m_close = true;

    if (openLog)
    {
        Log::getInstance()->init(logLevel, "./webserverlog", ".log", logQueSize);
        if (m_close)
        {
            LOG_ERROR("========== Server init error!==========");
        } // !(m_close)
        else
        {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", m_port, OptLinger ? "true" : "false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                     (m_listenEvent & EPOLLET ? "ET" : "LT"),
                     (m_connEvent & EPOLLET ? "ET" : "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::getSrcDir());
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }
}

WebServer::~WebServer()
{
    close(m_listen_fd);
    m_close = true;
    free(m_src_dir);
    SqlPool::getInstance()->close();
}

void WebServer::initEventMode(int trigMode)
{
    m_listenEvent = EPOLLRDHUP;              // 对端关闭触发
    m_connEvent = EPOLLONESHOT | EPOLLRDHUP; // EPOLLONESHOT 防止多线程处理同一个fd
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        m_connEvent |= EPOLLET;
        break;
    case 2:
        m_listenEvent |= EPOLLET;
        break;
    case 3:
        m_listenEvent |= EPOLLET;
        m_connEvent |= EPOLLET;
        break;
    default:
        m_listenEvent |= EPOLLET;
        m_connEvent |= EPOLLET;
        break;
    }

    HttpConn::setET(m_connEvent & EPOLLET);
}

void WebServer::start()
{
    int timeMS = -1;
    if (!m_close)
        LOG_INFO("========== Server start ==========");

    while (!m_close)
    {
        if (m_timeout > 0)
            timeMS = m_timer->getNextTick();

        int eventCnt = m_epoller->wait(timeMS);
        for (int i = 0; i < eventCnt; i++)
        {
            /* 处理事件 */
            int fd = m_epoller->getFd(i);
            uint32_t events = m_epoller->getEvents(i);

            if (fd == m_listen_fd) // 监听客户端连接
            {
                dealListen();
            }
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) // 处理错误
            {
                assert(m_users.count(fd) > 0);
                disconnect(&m_users[fd]);
            }
            else if (events & EPOLLIN) // 处理读
            {
                assert(m_users.count(fd) > 0);
                dealRead(&m_users[fd]);
            }
            else if (events & EPOLLOUT) // 处理写
            {
                assert(m_users.count(fd) > 0);
                dealWrite(&m_users[fd]);
            }
            else
            {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void WebServer::sendError(int fd, const char *info)
{
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);

    if (ret < 0)
        LOG_WARN("send error to client[%d] error!", fd);

    close(fd);
}

void WebServer::disconnect(HttpConn *client)
{
    assert(client);
    LOG_INFO("Client[%d] quit!", client->getFd());
    m_epoller->delFd(client->getFd());
    client->disconnect();
}

void WebServer::addClient(int fd, sockaddr_in addr)
{
    assert(fd > 0);
    m_users[fd].init(fd, addr);
    if (m_timeout > 0)
        m_timer->add(fd, m_timeout, std::bind(&WebServer::disconnect, this, &m_users[fd]));

    m_epoller->addFd(fd, EPOLLIN | m_connEvent);
    setFdNonblock(fd);

    LOG_INFO("Client[%d] in!", m_users[fd].getFd());
}

void WebServer::dealListen()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    int fd = accept(m_listen_fd, (struct sockaddr *)&addr, &len);
    if (fd <= 0)
    {
        return;
    }
    else if (HttpConn::use_count() >= MAX_FD)
    {
        sendError(fd, "Server busy");
        LOG_WARN("Clients is full");
        return;
    }

    addClient(fd, addr);
}

void WebServer::dealRead(HttpConn *client)
{
    assert(client);
    extentTime(client);
    m_pool->addTask(&WebServer::onRead, this, client);
}

void WebServer::dealWrite(HttpConn *client)
{
    assert(client);
    extentTime(client);
    m_pool->addTask(&WebServer::onWrite, this, client);
}

void WebServer::extentTime(HttpConn *client)
{
    assert(client);
    if (m_timeout > 0)
        m_timer->adjust(client->getFd(), m_timeout);
}

void WebServer::onRead(HttpConn *client)
{
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if (ret <= 0 && readErrno != EAGAIN)
    {
        disconnect(client);
        return;
    }

    onProcess(client);
}

void WebServer::onProcess(HttpConn *client)
{
    // ET模式下 当从EPOLLIN转换到EPOLLOUT时会立即触发一次写事件
    // 读事件触发是从无数据到有数据 从不可读到可读 即状态发生改变
    // 写事件也是当写状态发生改变才触发
    if (client->process())
    {
        m_epoller->modFd(client->getFd(), m_connEvent | EPOLLOUT);
    }
    else
    {
        m_epoller->modFd(client->getFd(), m_connEvent | EPOLLIN);
    }
}

void WebServer::onWrite(HttpConn *client)
{
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);

    if (client->toWriteBytes() == 0)
    {
        /* 传输完成 */
        if (client->isKeepAlive())
        {
            onProcess(client);
            return;
        } // !(client->isKeepAlive())
    }     // !(client->toWriteBytes() == 0)
    else if (ret < 0)
    {
        if (writeErrno == EAGAIN)
        {
            /* 继续传输 */
            m_epoller->modFd(client->getFd(), m_connEvent | EPOLLOUT);
            return;
        } // !(writeErrno == EAGAIN)
    }     // !(ret < 0)
    disconnect(client);
}

bool WebServer::initSocket()
{
    if (m_port > 65535 || m_port < 1024)
    {
        LOG_ERROR("Port:%d error!", m_port);
        return false;
    }

    int ret;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(m_port);

    struct linger optLinger = {0};
    if (m_open_linger)
    {
        // 直到所剩数据发送完毕或超时返回close
        optLinger.l_onoff = 1;
        optLinger.l_linger = 7;
    }

    m_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listen_fd < 0)
    {
        LOG_ERROR("Create socket error!", m_port);
        return false;
    }

    ret = setsockopt(m_listen_fd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if (ret < 0)
    {
        close(m_listen_fd);
        LOG_ERROR("Init linger error!", m_port);
        return false;
    }

    int optval = 1;
    // 端口复用 最后一个lfd才能接收连接请求
    ret = setsockopt(m_listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
    if (ret == -1)
    {
        LOG_ERROR("set socket setsockopt error !");
        close(m_listen_fd);
        return false;
    }

    ret = bind(m_listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        LOG_ERROR("Bind Port:%d error!", m_port);
        close(m_listen_fd);
        return false;
    }

    ret = listen(m_listen_fd, 6);
    if (ret < 0)
    {
        LOG_ERROR("Listen port:%d error!", m_port);
        close(m_listen_fd);
        return false;
    }
    ret = m_epoller->addFd(m_listen_fd, m_listenEvent | EPOLLIN);
    if (ret == 0)
    {
        LOG_ERROR("Add listen error!");
        close(m_listen_fd);
        return false;
    }

    setFdNonblock(m_listen_fd);
    LOG_INFO("Server port:%d", m_port);
    return true;
}

int WebServer::setFdNonblock(int fd)
{
    assert(fd > 0);
    int flag = fcntl(fd, F_GETFL, 0);
    flag |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flag);
}
