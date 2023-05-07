/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:39:28
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-05-07 21:05:06
 */

#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "epoller.h"
#include "pool/threadpool.hpp"
#include "pool/sqlraii.h"
#include "timer/heaptimer.h"
#include "http/httpconn.h"

class WebServer
{
public:
    explicit WebServer(
        int port, int trigMode, int timeoutMS, bool OptLinger,
        int sqlPort, const char *sqlUser, const char *sqlPwd,
        const char *dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize);

    ~WebServer();

    void start();

private:
    static constexpr int MAX_FD = 65536;

private:
    static int setFdNonblock(int fd);

private:
    bool initSocket();
    void initEventMode(int trigMode);
    void addClient(int fd, sockaddr_in addr);

    void dealListen();
    void dealWrite(HttpConn *client);
    void dealRead(HttpConn *client);

    void sendError(int fd, const char *info);
    void extentTime(HttpConn *client);
    void disconnect(HttpConn *client);

    void onRead(HttpConn *client);
    void onWrite(HttpConn *client);
    void onProcess(HttpConn *client);

private:
    int m_port;
    bool m_open_linger;
    int m_timeout;
    bool m_close;
    int m_listen_fd;
    char *m_src_dir;

    uint32_t m_listenEvent;
    uint32_t m_connEvent;

    std::unique_ptr<HeapTimer> m_timer;
    std::unique_ptr<ThreadPool> m_pool;
    std::unique_ptr<Epoller> m_epoller;
    std::unordered_map<int, HttpConn> m_users;
};

#endif // _WEBSERVER_H_