#include <sys/stat.h>
#include <cstring>
#include "httphandler.h"

HTTPHandler::HTTPHandler(ThreadPool *pool, Reactor *ptr)
{
    m_pool = pool;
    m_reactor = ptr;
    m_filename = "";
}

HTTPHandler::~HTTPHandler()
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

void HTTPHandler::handleEvent(int &fd, uint32_t &events)
{
    if (events & EPOLLIN)
    {
        // 如果此函数第一次运行 必定是服务器接受客户端请求 所以此时fd就是lfd
        static bool run_once = true; // 只会在第一次运行时初始化为true 第二次运行编译器会跳过此语句
        if (__builtin_expect(run_once, 0))
        {
            run_once = false;
            listen_fd = fd;
        }
        // 将读回调函数加入线程池的工作线程
        if (fd != listen_fd)
            m_pool->addTask(&HTTPHandler::recvEvent, this, fd);
        else
            m_pool->addTask(&HTTPHandler::acceptConn, this);
    }
    if (events & EPOLLOUT)
    {
        // 将写回调函数加入线程池的工作线程
        m_pool->addTask(&HTTPHandler::sendFile, this, fd);
    }
    if (events & (EPOLLERR | EPOLLHUP))
    {
        // 关闭连接并处理
        // delete this
    }
}

std::string HTTPHandler::getLine(int &fd)
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
        else if (n == 0)
            c = '\n';
        else
            return "";
    }
    line += "\0";

    return line;
}

void HTTPHandler::sendMessage(int &fd, int no, std::string status, u_long size)
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
    if (ret == -1)
    {
        std::cerr << "Failed to recv msg\n";
        close(fd);
        // delete this
        return;
    }
}

void HTTPHandler::recvEvent(int &fd)
{
    m_reactor->delHandler(fd); // 先不监听此事件

    std::string first = getLine(fd);
    std::string methos = "";

    while (getLine(fd) != "\n") // 把缓冲区数据读完
        ;

    if (first == "")
    {
        std::cerr << "Failed to recv msg\n";
        close(fd);
        // delete this
        return;
    }

    size_t pos = first.find_first_of(" ");
    methos = first.substr(0, pos); // 第一个空格前
    if (methos == "GET")
    {
        // GET事件处理
        m_filename = first.substr(pos + 1, first.find(" ", pos + 1) - pos - 1); // 第一个空格与第二个空格间
        m_filename = m_filename.substr(m_filename.find_first_of('/') + 1);      // 去掉开头的斜杠
        std::cout << "methos = " << methos << " file = " << m_filename << std::endl;
        std::unique_ptr<HTTPHandler> handler(new HTTPHandler(m_pool, m_reactor));
        m_reactor->addHandler(std::move(handler), fd, EPOLLOUT | EPOLLET); // 改成写事件重新挂回树上监听
    }
    else if (methos == "POST")
    {
        // POST事件处理
    }
}

void HTTPHandler::sendFile(int &fd)
{
    m_reactor->delHandler(fd); // 不监听此事件

    struct stat sbuf;
    int ret = stat(m_filename.c_str(), &sbuf);
    if (ret == -1)
    {
        std::cerr << "stat file error\n";
        // 发送404页面
        // sendMessage(fd, 404, "Not Found", sbuf.st_size);
        // sendFile(m_filename);
        close(fd);
        // delete this
        return;
    }
    sendMessage(fd, 200, "OK", sbuf.st_size);

    std::ifstream ifs;
    ifs.open(m_filename, std::ios::in);

    char buf[BUFSIZ] = {0};
    while (ifs.read(buf, sizeof(buf)))
    {
    tryagain:
        ret = send(fd, buf, strlen(buf), 0);
        std::cout << "send file size: " << ret << std::endl;
        if (ret == -1)
        {
            if (ret == EAGAIN || ret == EINTR) // 阻塞状态不算发送错误
                goto tryagain;
            else
            {
                std::cerr << "send file error\n";
                break;
            }
        }
    }

    ifs.close();
    close(fd);
}

void HTTPHandler::acceptConn()
{
    struct sockaddr_in cin;
    socklen_t len = sizeof(cin);

    int cfd, i;
    if ((cfd = accept(listen_fd, (struct sockaddr *)&cin, &len)) == -1)
    {
        if (errno != EAGAIN && errno != EINTR)
        {
            printf("%s:accept,%s\n", __func__, strerror(errno));
            return;
        }
    }
    do
    {
        if (i == MAX_EVENTS) // 超出连接数上限
        {
            printf("%s: max connect limit[%d]\n", __func__, MAX_EVENTS);
            break;
        }
        int flag = 0;
        if ((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0) // 将cfd也设置为非阻塞
        {
            printf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
            break;
        }

        long opt = 16 * 1204 * 1204;
        socklen_t optlen = sizeof(int);
        setsockopt(cfd, SOL_SOCKET, SO_SNDBUF, &opt, sizeof(opt));
        getsockopt(cfd, SOL_SOCKET, SO_SNDBUF, &opt, &optlen);
        printf("===============================================================sendbufsize = %ld\n", opt);

        std::unique_ptr<HTTPHandler> handler(new HTTPHandler(m_pool, m_reactor));
        m_reactor->addHandler(std::move(handler), listen_fd, EPOLLIN | EPOLLET); // 设置为读事件挂树上监听
    } while (0);

    // printf("new connect[%s:%d],[time:%ld],pos[%d]\n\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), g_hev[i].mev.last_active, i);
    return;
}
