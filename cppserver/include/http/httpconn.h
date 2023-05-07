/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:38:37
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-05-07 18:52:14
 */

#ifndef __HTTPCONN_H__
#define __HTTPCONN_H__

#include <sys/types.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include "log/log.h"
#include "pool/sqlraii.h"
#include "buffer/buffer.h"
#include "httprequest.h"
#include "httpresponse.h"

class HttpConn
{
public:
    HttpConn();
    ~HttpConn();

    void init(int sockFd, const sockaddr_in &addr);
    void disconncet();
    ssize_t read(int *saveErrno);
    ssize_t write(int *saveErrno);
    bool process();

    inline int toWriteBytes() { return m_iov[0].iov_len + m_iov[1].iov_len; }
    inline bool isKeepAlive() const { return m_request->isKeepAlive(); }
    inline int getFd() const { return m_fd; }
    inline int getPort() const { return m_addr.sin_port; }
    inline const char *getIP() const { return inet_ntoa(m_addr.sin_addr); }
    inline struct sockaddr_in getAddr() const { return m_addr; }
    inline int use_count() const { return m_use_count; }
    inline bool isET() const { return m_ET; }
    inline const char *getSrcDir() const { return m_src_dir; }

private:
    static std::atomic_int m_use_count;
    static bool m_ET;
    static const char *m_src_dir;

private:
    int m_fd;
    int m_iov_count;
    bool m_close;

    struct sockaddr_in m_addr;
    struct iovec m_iov[2];

    std::shared_ptr<Buffer> m_readbuf;
    std::shared_ptr<Buffer> m_writebuf;

    std::unique_ptr<HttpRequest> m_request;
    std::unique_ptr<HttpResponse> m_response;
};

#endif // __HTTPCONN_H__