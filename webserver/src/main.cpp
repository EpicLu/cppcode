#include <unistd.h>
#include "webserver.h"

int main(int argc, char **argv)
{
    // 检查运行程序传入的参数
    if (argc < 3)
        printf("./main port path\n");

    int port = atoi(argv[1]);

    int ret = chdir(argv[2]); // 把工作目录改到web目录下
    if (ret == -1)
    {
        printf("chdir error!");
        exit(-1);
    }

    WebServer server(8, 16, 8);
    server.startServer(port);

    return 0;
}