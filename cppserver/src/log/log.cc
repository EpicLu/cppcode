/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:40:07
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-04-23 00:41:07
 */

#include "log/log.h"

Log::Log()
{
    m_line_count = 0;
    m_today = 0;
    m_is_async = false;
    m_buf = nullptr;
    m_queue = nullptr;
    m_thread = nullptr;
    // 其余成员在init中初始化
}

Log::~Log()
{
    if (m_thread && m_thread->joinable())
    {
        while (!m_queue->empty())
            m_queue->flush(); // 先完成剩余工作
        m_queue->close();
        m_thread->join();
    }
    if (m_fp)
    {
        std::lock_guard<std::mutex> locker(m_locker);
        flush();
        fclose(m_fp);
    }
}

int Log::getLevel()
{
    std::lock_guard<std::mutex> locker(m_locker);
    return m_level;
}

void Log::setLevel(int level)
{
    std::lock_guard<std::mutex> locker(m_locker);
    m_level = level;
}

void Log::init(int level = 1, const char *path, const char *suffix, int capacity)
{
    assert(capacity > 0);
    m_is_open = true;
    m_level = level;

    if (capacity > 0)
    {
        m_is_async = true;
        if (!m_queue)
        {
            m_queue = std::make_unique<BlockQueue<std::string>>(capacity);
            m_thread = std::make_unique<std::thread>(flushThread);
            m_buf = std::make_unique<Buffer>(BUFSIZ);
        }
    }
    else
    {
        m_is_async = false;
    }

    m_line_count = 0;

    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;
    m_path = path;
    m_suffix = suffix;
    char filename[LOG_NAME_LEN] = {0};
    // 设置日志文件名 年_月_日.log
    snprintf(filename, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s",
             m_path, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, m_suffix);
    m_today = t.tm_mday;

    {
        std::lock_guard<std::mutex> locker(m_locker);
        m_buf->retrieveAll();
        if (m_fp)
        {
            flush();
            fclose(m_fp);
        }

        m_fp = fopen(filename, "a");
        if (m_fp == nullptr)
        {
            mkdir(m_path, 0777);
            m_fp = fopen(filename, "a");
        }
        assert(m_fp != nullptr);
    } // lock_guard
}

void Log::write(int level, const char *format, ...)
{
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;

    if ((m_today != t.tm_mday) || (m_line_count && (m_line_count % MAX_LINES == 0)))
    {
        char newfile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        if (m_today != t.tm_mday) // 进入下一天 新建一份日志
        {
            snprintf(newfile, LOG_NAME_LEN - 72, "%s/%s%s", m_path, tail, m_suffix);
            m_today = t.tm_mday;
            m_line_count = 0;
        }
        else // 日志满了 重开一份
        {
            snprintf(newfile, LOG_NAME_LEN - 72, "%s/%s-%d%s", m_path, tail, (m_line_count / MAX_LINES), m_suffix);
        }

        std::lock_guard<std::mutex> locker(m_locker);
        flush();
        fclose(m_fp);
        m_fp = fopen(newfile, "a");
        assert(m_fp != nullptr);
    }

    // 日志内容
    {
        std::lock_guard<std::mutex> locker(m_locker);
        m_line_count++;
        // 事件时间
        int n = snprintf(m_buf->beginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                         t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                         t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);

        m_buf->hasWritten(n);
        appendTitle(level);

        // 事件内容
        va_start(vaList, format);
        int m = vsnprintf(m_buf->beginWrite(), m_buf->writableBytes(), format, vaList);
        va_end(vaList);

        m_buf->hasWritten(m);
        m_buf->append("\n\0", 2); // 换行和结束标志

        if (m_is_async && m_queue && !m_queue->full())
        {
            m_queue->push_back(m_buf->to_string()); // 写入阻塞队列
        }
        else
        {
            fputs(m_buf->peek(), m_fp);
        }
        m_buf->retrieveAll();
    }
}

bool Log::isOpen()
{
    return m_is_open;
}

void Log::appendTitle(int level)
{
    switch (level)
    {
    case 0:
        m_buf->append("[debug]: ", 9);
        break;
    case 1:
        m_buf->append("[info] : ", 9);
        break;
    case 2:
        m_buf->append("[warn] : ", 9);
        break;
    case 3:
        m_buf->append("[error]: ", 9);
        break;
    default:
        m_buf->append("[info] : ", 9);
        break;
    }
}

void Log::flush()
{
    if (m_is_async)
        m_queue->flush();
    fflush(m_fp);
}

void Log::asyncWrite()
{
    std::string str = "";
    while (m_queue->pop(str))
    {
        std::lock_guard<std::mutex> locker(m_locker);
        fputs(str.c_str(), m_fp);
    }
}

Log *Log::getInstance()
{
    static Log s_log;
    return &s_log;
}

void Log::flushThread()
{
    Log::getInstance()->asyncWrite();
}