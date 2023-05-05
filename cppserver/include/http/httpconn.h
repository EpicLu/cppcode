/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:38:37
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-05-05 21:45:04
 */

#ifndef __HTTPCONN_H__
#define __HTTPCONN_H__

#include <sys/types.h>
#include <sys/uio.h>   // readv/writev
#include <arpa/inet.h> // sockaddr_in
#include <stdlib.h>    // atoi()
#include <errno.h>
#include "../log/log.h"
#include "../pool/sqlraii.h"
#include "../buffer/buffer.h"
#include "httprequest.h"
#include "httpresponse.h"

class httpconn
{
public:
    httpconn();
    ~httpconn();

private:
    /* data */
};

#endif // __HTTPCONN_H__