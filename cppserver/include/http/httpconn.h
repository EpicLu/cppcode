/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:38:37
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-05-05 21:50:24
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

private:
    int m_fd;
    int m_iov_count;
    bool m_close;

    struct sockaddr_in m_addr;
    struct iovec m_iov[2];

    Buffer m_readbuf;
    Buffer m_writebuf;
};

#endif // __HTTPCONN_H__