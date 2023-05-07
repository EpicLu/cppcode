/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:39:56
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-05-07 19:02:10
 */

#include "http/httpconn.h"

const char *HttpConn::m_src_dir;
std::atomic_int HttpConn::m_use_count;
bool HttpConn::m_ET;

HttpConn::HttpConn()
    : m_fd(-1), m_close(true), m_iov_count(0)
{
    m_readbuf = nullptr;
    m_writebuf = nullptr;
    m_request = nullptr;
    m_response = nullptr;
    m_addr = {0};
}

HttpConn::~HttpConn()
{
    disconncet();
};

void HttpConn::init(int fd, const sockaddr_in &addr)
{
    assert(fd > 0);
    m_use_count++;
    m_addr = addr;
    m_fd = fd;
    m_writebuf->retrieveAll();
    m_readbuf->retrieveAll();
    m_close = false;
    LOG_INFO("Client[%d](%s:%d) in, use_count:%d", m_fd, getIP(), getPort(), (int)m_use_count);
}

void HttpConn::disconncet()
{
    munmap(m_response->file(), m_response->fileSize());
    if (m_close == false)
    {
        m_close = true;
        m_use_count--;
        close(m_fd);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", m_fd, getIP(), getPort(), (int)m_use_count);
    }
}

ssize_t HttpConn::read(int *saveErrno)
{
    ssize_t len = -1;
    do
    {
        len = m_readbuf->readFd(m_fd, saveErrno);
        if (len <= 0)
            break;

    } while (m_ET);

    return len;
}

ssize_t HttpConn::write(int *saveErrno)
{
    ssize_t len = -1;
    do
    {
        len = writev(m_fd, m_iov, m_iov_count);
        if (len <= 0)
        {
            *saveErrno = errno;
            break;
        }

        if (m_iov[0].iov_len + m_iov[1].iov_len == 0)
        {
            break;
        } // !(m_iov[0].iov_len + m_iov[1].iov_len == 0)
        else if (static_cast<size_t>(len) > m_iov[0].iov_len)
        {
            // 进入此判断的条件是写入的数据比m_iov[0]大 说明写到m_iov[1]了
            m_iov[1].iov_base = (uint8_t *)m_iov[1].iov_base + (len - m_iov[0].iov_len);
            m_iov[1].iov_len -= (len - m_iov[0].iov_len);
            if (m_iov[0].iov_len) // 进入此判断 下一次循环开始时 m_iov[0]已空
            {
                m_writebuf->retrieveAll();
                m_iov[0].iov_len = 0;
            } // !(m_iov[0].iov_len)
        }
        else
        {
            m_iov[0].iov_base = (uint8_t *)m_iov[0].iov_base + len; // 调整指针位置
            m_iov[0].iov_len -= len;
            m_writebuf->retrieve(len);
        }
    } while (m_ET || toWriteBytes() > 10240);

    return len;
}

bool HttpConn::process()
{
    m_request->init();
    if (m_readbuf->readableBytes() <= 0)
    {
        return false;
    }
    else if (m_request->parse(m_readbuf))
    {
        LOG_DEBUG("%s", m_request->path().c_str());
        m_response->init(m_src_dir, m_request->path(), m_request->isKeepAlive(), 200);
    }
    else
    {
        m_response->init(m_src_dir, m_request->path(), false, 400);
    }

    m_response->makeResponse(m_writebuf);
    /* 响应头 */
    m_iov[0].iov_base = const_cast<char *>(m_writebuf->peek());
    m_iov[0].iov_len = m_writebuf->readableBytes();
    m_iov_count = 1;

    /* 文件 */
    if (m_response->fileSize() > 0 && m_response->file())
    {
        m_iov[1].iov_base = m_response->file();
        m_iov[1].iov_len = m_response->fileSize();
        m_iov_count = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", m_response->fileSize(), m_iov_count, toWriteBytes());

    return true;
}