/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:38:25
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-04-23 03:08:02
 */

#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/uio.h>
#include <vector>
#include <atomic>
#include <assert.h>

class Buffer
{
    // Buffer的结构是([预留空间]+[读缓冲]+[写缓冲])组成的一个大的缓冲空间
public:
    Buffer(const int &bufsize = 4096); // 缺省4096
    ~Buffer() = default;

    size_t writableBytes() const;    // 写缓冲大小
    size_t readableBytes() const;    // 读缓冲大小
    size_t prependableBytes() const; // 预留空间大小

    // readpos的所在位置 对外界来说就是起始位置
    char *peek();             // 可写
    const char *peek() const; // 只读

    // writepos的所在位置
    char *beginWrite();
    const char *beginWrite() const;

    void retrieve(const size_t &len);    // 更新读数据的起始pos（回收空间）
    void retrieveUntil(const char *end); // 更新pos到指定位置（回收空间）
    void retrieveAll();                  // 回收所有空间 pos置0
    void hasWritten(const size_t &len);  // 更新写数据的起始pos

    std::string to_string(); // 数据内容转string

    // 向往写缓冲加data && 它的重载
    void append(const std::string &data);
    void append(const char *data, const size_t &len);
    void append(const void *data, const size_t &len);
    void append(const Buffer &data);

    // 句柄操作
    ssize_t readFd(const int &fd, int *errorno);
    ssize_t writeFd(const int &fd, int *errorno);

private:
    // 起始位置
    char *begin();
    const char *begin() const;

    void ensureWritable(const size_t &len); // 确保写缓冲够大
    void makeSpace(const size_t &len);      // 扩容

    std::vector<char> m_buffer;
    std::atomic_size_t m_readpos;
    std::atomic_size_t m_writepos;
};

#endif // _BUFFER_H_