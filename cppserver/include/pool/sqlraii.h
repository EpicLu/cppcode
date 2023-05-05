/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:39:14
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-04-24 03:43:34
 */

#ifndef _SQLRAII_H_
#define _SQLRAII_H_

#include "sqlpool.h"

class SqlRAII
{
public:
    SqlRAII() = default;
    SqlRAII(MYSQL **sql, SqlPool *sql_pool); //
    ~SqlRAII();

private:
    MYSQL *m_sql;
    SqlPool *m_sql_pool; // 单例 无需手动删除
};

#endif // _SQLRAII_H_