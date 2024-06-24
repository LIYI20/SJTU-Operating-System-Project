#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "tcp_utils.h"
#include "command.h"
// Block size in bytes
#define BLOCKSIZE 256
#define MAX_BLOCK_COUNT 1024

int ncyl, nsec, ttd;
char *diskfile; // 文件映射的首指针
char response[256] = "";

// return a negative value to exit
int cmd_i(cmd *client_cmd)
{
    // 写入response
    sprintf(response, "%d %d", ncyl, nsec);
    return 0;
}

// 读操作
int cmd_r(cmd *client_cmd)
{
    int block_id = client_cmd->block_id;
    if (block_id < 0 || block_id >= MAX_BLOCK_COUNT)
    {
        printf("The block does not exist\n");
        // send_to_buffer(write_buf, "No", 3);
        return 0;
    }
    char buffer[BLOCKSIZE] = "";
    char *sector_data = diskfile + block_id * BLOCKSIZE;
    memcpy(response, sector_data, BLOCKSIZE);
    printf("Read OK\n");
    return 0;
}

// 写操作
int cmd_w(cmd *client_cmd)
{
    int block_id = client_cmd->block_id;
    printf("%d\n", block_id);
    if (block_id < 0 || block_id >= MAX_BLOCK_COUNT)
    {
        printf("The block does not exist\n");
        // send_to_buffer(write_buf, "No", 3);
        return 0;
    }
    char *block_data = diskfile + block_id * BLOCKSIZE;
    for (int i = 0; i < 256; i++)
        printf("%d", client_cmd->data[i]);
    printf("\n");

    memcpy(block_data, client_cmd->data, BLOCKSIZE);
    printf("Write OK\n");
    return 0;
}

int cmd_e(cmd *client_cmd)
{
    sprintf(response, "Bye!", 5);
    return -1;
}

// static struct
// {
//     const char *name;
//     int (*handler)(tcp_buffer *write_buf, cmd* client_cmd);
// } cmd_table[] = {
//     {"I", cmd_i},
//     {"R", cmd_r},
//     {"W", cmd_w},
//     {"E", cmd_e},
// };

#define NCMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

void add_client(int id)
{
    // some code that are executed when a new client is connected
    // you don't need this in step1
}

// 这个函数是助教提供的，处理第二步很麻烦，传个msg然后切切切，在后面的传输很困难，
// 但是这一改又非常麻烦
// int handle_client(int id, tcp_buffer *write_buf, cmd* client_cmd)
// {
//     char p = client_cmd->type;
//     int ret = 1;
//     if (strcmp(p, 'I') == 0)
//     {
//         ret = cmd_i(&client_cmd);
//     }
//     else if (strcmp(p, 'R') == 0)
//     {

//         ret = cmd_r(write_buf, client_cmd);
//     }
//     else if (strcmp(p, 'W') == 0)
//     {

//         ret = cmd_w(write_buf, client_cmd);
//     }
//     else if (strcmp(p, 'E') == 0)
//     {
//         ret = cmd_e(write_buf,client_cmd);
//     }
//     if (ret == 1)
//     {
//         static char unk[] = "Unknown command";
//         send_to_buffer(write_buf, unk, sizeof(unk));
//     }
//     if (ret < 0)
//     {
//         return -1;
//     }
// }

void clear_client(int id)
{
    // some code that are executed when a client is disconnected
    // you don't need this in step2
}

// 处理client函数
handle_fs_client(int client_socket)
{
    cmd raw_command;
    char cmdtype;
    int ret;

    while (1)
    {
        memset(response, 0, sizeof(response));
        memset(&raw_command, 0, sizeof(raw_command));
        ssize_t ret = recv(client_socket, &raw_command, sizeof(raw_command), 0);

        if (ret <= 0)
        {
            printf("Failed to read from fs client, disconnecting.\n");
            break;
        }
        if (strlen(raw_command.data) == 1 && raw_command.data[0] == '\n')
            continue; // 继续下一次循环

        printf("%c %d\n", raw_command.type, raw_command.block_id);

        // int handled = 0;//未处理
        cmdtype = raw_command.type;

        int res = 1;
        if (cmdtype == 'I')
        {
            res = cmd_i(&raw_command);
        }
        else if (cmdtype == 'R')
        {

            res = cmd_r(&raw_command);
        }
        else if (cmdtype == 'W')
        {

            res = cmd_w(&raw_command);
        }
        else if (cmdtype == 'E')
        {
            res = cmd_e(&raw_command);
        }
        if (res == 1)
        {
            sprintf(response, "Unknown command", 16);
        }
        if (res < 0)
        {
            return -1;
        }
        printf("Response:");
        for (int i = 0; i < 256; i++)
            printf("%c", response[i]);
        printf("\n");

        if (send(client_socket, response, sizeof(response), 0) < 0)
        {
            perror("Failed to send response to client");
        }
    }
};

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        fprintf(stderr,
                "Usage: %s <disk file name> <cylinders> <sector per cylinder> "
                "<track-to-track delay> <port>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }
    // args
    char *diskfname = argv[1];
    ncyl = atoi(argv[2]);
    nsec = atoi(argv[3]);
    ttd = atoi(argv[4]); // ms delay时间track-to-track time
    int port = atoi(argv[5]);

    // open file
    //	O_RDWR以可读写方式打开文件
    // O_CREAT若欲打开的文件不存在则自动建立该文件.
    int fd = open(diskfname, O_RDWR | O_CREAT, 0);
    if (fd < 0)
    {
        printf("Error: Could not open file '%s'.\n", diskfname);
        exit(-1);
    }

    // stretch the file
    long FILESIZE = BLOCKSIZE * nsec * ncyl; // 磁盘文件大小
    int result = lseek(fd, FILESIZE - 1, SEEK_SET);
    if (result == -1)
    {
        perror("Error calling lseek() to 'stretch' the file");
        close(fd);
        exit(-1);
    }

    result = write(fd, "", 1); // 结尾插入空字符
    if (result != 1)
    {
        perror("Error writing last byte of the file");
        close(fd);
        exit(-1);
    }
    // mmap
    // char *diskfile;
    diskfile = (char *)mmap(NULL, FILESIZE,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
    if (diskfile == MAP_FAILED)
    {
        close(fd);
        printf("Error: Could not map file.\n");
        exit(-1);
    }

    // command
    // tcp_server server = server_init(port, 1, add_client, handle_client, clear_client);
    // server_loop(server);

    // 创建新socket，因为助教写的tpc工具只能传char*，我也不知道为什么当传的不是字符串的时候，
    // 写入和读出的值不同，所以BDS只能自己新写socket和处理函数了。
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    int client_socket;

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Failed to create socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Failed to bind socket");
        return 1;
    }
    if (listen(server_socket, 1) < 0)
    {
        perror("Failed to listen on socket");
        return 1;
    }

    printf("BDS server started. Listening on port %d\n", port);

    while (1)
    {
        client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0)
        {
            perror("Failed to accept connection");
            continue;
        }

        printf("New FS client connected. Client socket: %d\n", client_socket);
        handle_fs_client(client_socket);
        printf("FS client disconnected. Client socket: %d\n", client_socket);
        close(client_socket);
    }

    close(server_socket);
}
