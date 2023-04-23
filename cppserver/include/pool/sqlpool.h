/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:39:10
 * @Last Modified by:   EpicLu
 * @Last Modified time: 2023-04-22 18:39:10
 */

#ifndef _SQLPOOL_H_
#define _SQLPOOL_H_

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include "log/log.h"

class SqlPool
{
public:
    // 单例模式删除拷贝构造
    SqlPool(const SqlPool &pool) = delete;
    SqlPool &operator=(const SqlPool &pool) = delete;

    static SqlPool *getInstance(); // 单例

    // 初始化
    void init(const char *host, int port,
              const char *user, const char *pwd,
              const char *db, int connsize = 16);

    MYSQL *connect();             // 从池中获取一个sql连接
    void disconnect(MYSQL *conn); // 关闭一个sql连接（送回池中）
    int free_count();             // 空闲连接的数量

    void close(); // 关闭sql连接池

private:
    SqlPool() = default;
    ~SqlPool() = default;

    std::queue<MYSQL *> m_conns; // sql连接队列（池）
    std::mutex m_locker;
    sem_t m_sem_id; // 信号量 当所有的sql连接都在忙 阻塞
};

#endif // _SQLPOOL_H_