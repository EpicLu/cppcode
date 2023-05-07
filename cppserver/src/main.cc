/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:40:31
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-05-07 21:16:15
 */

#include "server/webserver.h"

int main(int argc, char const *argv[])
{
    // 确保程序的工作区的子目录有resoures子目录 即网站文件夹
    int ret = chdir("/home/lpc/cppcode/cppserver");
    if (ret == -1)
    {
        perror("chdir error!");
        exit(-1);
    }

    WebServer server(
        1206, 3, 60000, false,              /* 端口 ET模式 timeout毫秒 优雅退出 */
        3306, "lpc", "126126", "webserver", /* Mysql配置 */
        16, 4, true, 0, 1024);              /* 连接池数量 线程池数量 日志开关 日志等级 日志异步队列容量 */

    server.start();

    return 0;
}
