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
    char dir_name[1000] = "";
    

    static char buf[4096];
    while (1)
    {
        client_send(client, command, strlen(command) + 1);
        int n1 = client_recv(client, dir_name, sizeof(dir_name));
        printf("%s$ ", dir_name);

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
