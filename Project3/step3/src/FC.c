#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "tcp_utils.h"
#include "utils.h"
//File system client
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <ServerAddr> <Port>", argv[0]);
        exit(EXIT_FAILURE);
        return -1;
    }
    //FS的port
    int port = atoi(argv[2]);
    tcp_client client = client_init("localhost", port);
    // printf(argc);

    // 每次都获取一下当前所在目录
    // 我新定义了一个命令dir
    char *command = "dir";
    char dir[100] = "";
    char username[100];
    static char buf[4096];
    int n;
    // 处理用户名
    printf("请输入你的用户名：");
    scanf("%s", username);
    char tmp = getchar();//吃掉回车，scanf会把回车留在缓存区
    sprintf(buf, "user %s", username);

    client_send(client, buf, strlen(buf) + 1);
    n=client_recv(client, buf, sizeof(buf));
    buf[n] = 0;
    printf("%s\n", buf);

    while (1)
    {
        client_send(client, command, strlen(command) + 1);
        n = client_recv(client, dir, sizeof(dir));
        dir[n] = 0;
        printf("%s$ ", dir);

        //读取
        fgets(buf, sizeof(buf), stdin);
        if (feof(stdin))
            break;
        client_send(client, buf, strlen(buf) + 1);
        int n = client_recv(client, buf, sizeof(buf));
        buf[n] = 0;
        printf("%s\n", buf);
        if (strcmp(buf, "Bye!") == 0)
            break;
    }
    client_destroy(client);
}
