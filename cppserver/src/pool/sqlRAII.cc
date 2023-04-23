/*
 * @Author: EpicLu
 * @Date: 2023-04-24 03:30:30
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-04-24 03:36:43
 */

#include "pool/sqlRAII.h"

SqlRAII::SqlRAII(MYSQL **sql, SqlPool *sql_pool)
{
    assert(sql_pool);
    *sql = sql_pool->connect();
    m_sql = *sql;
    m_sql_pool = sql_pool;
}

SqlRAII::~SqlRAII()
{
    if (m_sql)
    {
        m_sql_pool->disconnect(m_sql);
    }
}