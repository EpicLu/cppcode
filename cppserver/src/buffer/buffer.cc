/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:39:52
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-04-23 00:01:35
 */

#include "buffer/buffer.h"

Buffer::Buffer(const int &bufsize) : m_buffer(bufsize), m_readpos(0), m_writepos(0) {}

size_t Buffer::readableBytes() const
{
    return m_writepos - m_readpos;
}

size_t Buffer::writableBytes() const
{
    return m_buffer.size() - m_writepos;
}

size_t Buffer::prependableBytes() const
{
    return m_readpos;
}

char *Buffer::peek()
{
    return begin() + m_readpos;
}

const char *Buffer::peek() const
{
    return begin() + m_readpos;
}

void Buffer::retrieve(const size_t &len)
{
    assert(len <= readableBytes());

    m_readpos += len;
}

void Buffer::retrieveUntil(const char *end)
{
    assert(peek() <= end);

    retrieve(end - peek());
}

void Buffer::retrieveAll()
{
    bzero(&m_buffer[0], m_buffer.size());
    m_readpos = 0;
    m_writepos = 0;
}

std::string Buffer::to_string()
{
    size_t readable = readableBytes();
    std::string str(peek(), readable);
    // retrieveAll();

    return str;
}

char *Buffer::beginWrite()
{
    return begin() + m_writepos;
}

const char *Buffer::beginWrite() const
{
    return begin() + m_writepos;
}

void Buffer::hasWritten(const size_t &len)
{
    m_writepos += len;
}

void Buffer::ensureWritable(const size_t &len)
{
    // 如果剩余写缓冲小于len 扩容
    if (writableBytes() < len)
        makeSpace(len);

    assert(writableBytes() >= len);
}

void Buffer::append(const char *data, const size_t &len)
{
    assert(data);
    ensureWritable(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
}

void Buffer::append(const std::string &data)
{
    append(data.data(), data.length());
}

void Buffer::append(const void *data, const size_t &len)
{
    assert(data);
    append(static_cast<const char *>(data), len);
}

void Buffer::append(const Buffer &data)
{
    append(data.peek(), data.readableBytes());
}

ssize_t Buffer::readFd(const int &fd, int *errorno)
{
    char buf[65536];
    struct iovec iov[2]; // sys/uio.h
    const size_t writable = writableBytes();

    // Buffer缓冲
    iov[0].iov_base = begin() + m_writepos;
    iov[0].iov_len = writable;
    // 临时buf缓冲
    iov[1].iov_base = buf;
    iov[1].iov_len = sizeof(buf);

    const ssize_t len = readv(fd, iov, 2);
    if (len < 0)
        *errorno = errno; // 错误号给参数保存
    else if (static_cast<size_t>(len) <= writable)
        hasWritten(len); // 更新writepos
    else
    {
        // 写缓冲不够大 剩余数据在readv中被存入临时buf 需要将临时buf中的数据再写入Buffer的写缓冲
        m_writepos = m_buffer.size();
        append(buf, len - writable);
    }

    return len;
}

ssize_t Buffer::writeFd(const int &fd, int *errorno)
{
    const size_t readable = readableBytes();

    ssize_t len = write(fd, peek(), readable);
    if (len < 0)
    {
        *errorno = errno; // 错误号给参数保存
        return len;       // -1
    }
    retrieve(len);

    return len;
}

char *Buffer::begin()
{
    // 先*取出原内容 再&取此内容的地址
    return &*m_buffer.begin();
}

const char *Buffer::begin() const
{
    // 先*取出原内容 再&取此内容的地址
    return &*m_buffer.begin();
}

void Buffer::makeSpace(const size_t &len)
{
    if (writableBytes() + prependableBytes() < len)
        m_buffer.resize(m_writepos + len + 1); // 比原来大len+1
    else
    {
        const size_t readable = readableBytes();
        std::copy(peek(), beginWrite(), begin()); // 预留空间登场
        m_readpos = 0;
        m_writepos = m_readpos + readable;

        assert(readable == readableBytes());
    }
}