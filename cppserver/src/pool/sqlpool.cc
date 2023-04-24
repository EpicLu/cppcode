/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:40:11
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-04-24 18:53:04
 */

#include "pool/sqlpool.h"

SqlPool *SqlPool::getInstance()
{
    static SqlPool s_pool;
    return &s_pool;
}

void SqlPool::init(const char *host, int port,
                   const char *user, const char *pwd,
                   const char *db, int connsize)
{
    assert(connsize > 0);

    for (size_t i = 0; i < connsize; i++)
    {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);

        if (!sql)
        {
            LOG_ERROR("mysql init error");
            assert(sql);
        }
        // else
        sql = mysql_real_connect(sql, host, user, pwd, db, port, nullptr, 0);

        if (!sql)
            LOG_ERROR("mysql connect error");
        // else
        m_conns.emplace(sql);
    }
    // 信号量初始化
    sem_init(&m_sem_id, 0, connsize);
}

MYSQL *SqlPool::connect()
{
    MYSQL *sql = nullptr;
    if (m_conns.empty())
    {
        LOG_WARN("SqlPool busy");
        return nullptr;
    }
    sem_wait(&m_sem_id); // conn_count--;
    {
        std::lock_guard<std::mutex> locker(m_locker);
        sql = m_conns.front();
        m_conns.pop();
    }

    return sql;
}

void SqlPool::disconnect(MYSQL *sql)
{
    assert(sql);
    std::lock_guard<std::mutex> locker(m_locker);
    m_conns.emplace(sql);
    sem_post(&m_sem_id); // conn_count++;
}

void SqlPool::close()
{
    std::lock_guard<std::mutex> locker(m_locker);
    while (!m_conns.empty())
    {
        auto conn = m_conns.front();
        m_conns.pop();
        mysql_close(conn); // 释放堆区申请的mysql类内存
    }

    mysql_library_end();
}

int SqlPool::free_count()
{
    return m_conns.size();
}