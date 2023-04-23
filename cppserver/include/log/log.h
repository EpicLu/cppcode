/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:39:05
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-04-23 00:27:00
 */

#ifndef _LOG_H_
#define _LOG_H_

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/stat.h>
#include "blockqueue.hpp"
#include "buffer/buffer.h"

class Log
{
public:
    // 单例模式删除拷贝构造
    Log(const Log &log) = delete;
    Log &operator=(const Log &log) = delete;

    // 初始化日志
    void init(int level, const char *path = "./log",
              const char *suffix = ".log",
              int capacity = 1024);

    static Log *getInstance(); // 单例
    static void flushThread(); // 刷新线程

    void write(int level, const char *format, ...); // 写日志
    void flush();                                   // 刷新日志缓冲区

    int getLevel();           // 获取日志内容的级别
    void setLevel(int level); // 设置日志内容级别
    bool isOpen();            // 日志是否打开

private:
    Log();                       // 初始化成员
    virtual ~Log();              // fclose()
    void appendTitle(int level); // 根据级别定级
    void asyncWrite();           // 异步写日志
    // #define XXX
    static constexpr int LOG_PATH_LEN = 256;
    static constexpr int LOG_NAME_LEN = 256;
    static constexpr int MAX_LINES = 77777;

    const char *m_path;   // 文件路径
    const char *m_suffix; // 后缀

    int m_line_count; // 行计数
    int m_today;      // 当天日期
    int m_level;      // 级别 0 1 2 3分别对应debug info(默认) warn error
    bool m_is_open;   // 打开标志
    bool m_is_async;  // 异步标志

    FILE *m_fp; // 指向日志文件
    // fstream m_fp; // stdio的更高效
    std::unique_ptr<Buffer> m_buf;                    // 缓冲区
    std::unique_ptr<BlockQueue<std::string>> m_queue; // 阻塞队列
    std::unique_ptr<std::thread> m_thread;            // 异步线程
    std::mutex m_locker;                              // 互斥锁
};

#define LOG_BASE(level, format, ...)                   \
    do                                                 \
    {                                                  \
        Log *log = Log::getInstance();                 \
        if (log->isOpen() && log->getLevel() <= level) \
        {                                              \
            log->write(level, format, ##__VA_ARGS__);  \
            log->flush();                              \
        }                                              \
    } while (0);

#define LOG_DEBUG(format, ...)             \
    do                                     \
    {                                      \
        LOG_BASE(0, format, ##__VA_ARGS__) \
    } while (0);
#define LOG_INFO(format, ...)              \
    do                                     \
    {                                      \
        LOG_BASE(1, format, ##__VA_ARGS__) \
    } while (0);
#define LOG_WARN(format, ...)              \
    do                                     \
    {                                      \
        LOG_BASE(2, format, ##__VA_ARGS__) \
    } while (0);
#define LOG_ERROR(format, ...)             \
    do                                     \
    {                                      \
        LOG_BASE(3, format, ##__VA_ARGS__) \
    } while (0);

#endif // _LOG_H_