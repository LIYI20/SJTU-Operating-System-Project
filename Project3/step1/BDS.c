#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include "tcp_utils.h"

// Block size in bytes
#define BLOCKSIZE 256
int ncyl, nsec, ttd;
char *diskfile;//文件映射的首指针



// return a negative value to exit
int cmd_i(tcp_buffer *write_buf, char *args, int len) {
    static char buf[64];
    sprintf(buf, "%d %d", ncyl, nsec);

    // send to buffer, including the null terminator
    send_to_buffer(write_buf, buf, strlen(buf) + 1);
    return 0;
}

int cmd_r(tcp_buffer *write_buf, char *args, int len) {
    // 首先获得R后面的参数数，有cylinder c sector s
    //但是我们也不能让参数过多，如果只是多一点我们可以直接忽略，但是在W操作中，我们输入的数据中
    //可能有多个空格导致数据被更改，所以不如直接设置一下后面参数的数量。
    int arg_num = 2;
    int count = 0;
    char *beh_args[2];
    char* p = strtok(args, " ");
    while (p && count < arg_num)
    {
        beh_args[count] = p;
        count++;
        p = strtok(NULL, " ");
    }
   
    if(count<arg_num){
        printf("Error: R command args is wrong\n");
        send_to_buffer(write_buf, "No", 3);
    }
    else{

        int c = atoi(beh_args[0]);
        beh_args[1] = strtok(beh_args[1], "\n");
        int s = atoi(beh_args[1]);
        if(c<0||s<0||c>=ncyl||s>=nsec){
            printf("The block does not exist\n");
            send_to_buffer(write_buf, "No", 3);
        }
        else{//返回数据
            // send_to_buffer(write_buf, "Yes", 4);
            char *data = diskfile + (c * nsec + s) * BLOCKSIZE;
            char buf[261];
            buf[0] = 'Y';
            buf[1] = 'e';
            buf[2] = 's';
            buf[3] = ' ';
            int j = 4;
            buf[260] = '\0';
            printf("%s", buf);
            for (int i = 0; i < BLOCKSIZE;i++)
                buf[j++] = data[i];
            buf[260] = '\0';
            // buf[j] = '\0';
            // memcpy(buf, &diskfile[BLOCKSIZE * (c * nsec + s)], BLOCKSIZE);
            // memcpy(buf, &data, BLOCKSIZE);
            send_to_buffer(write_buf,buf , 261);
        }
    }
    return 0;
}

int cmd_w(tcp_buffer *write_buf, char *args, int len) {
    int arg_num = 4;
    int count = 0;
    char *beh_args[4];
    char *p = strtok(args, " ");
    while (p && count < arg_num)
    {
        printf("%s\n",p);
        beh_args[count] = p;
        count++;
        p = strtok(NULL, " ");
    }
    if (count < arg_num)
    {
        printf("Error: W command args is wrong\n");
        printf("%d",count);
        send_to_buffer(write_buf, "No", 3);
    }
    else
    {
        int c = atoi(beh_args[0]);
        int s = atoi(beh_args[1]);
        int l = atoi(beh_args[2]);
        char *data = beh_args[3];
        data[l] = '\0';
        printf("%s\n", data);
        if (c < 0 || s < 0 || c >= ncyl || s >= nsec)
        {
            printf("The block does not exist\n");
            send_to_buffer(write_buf, "No", 3);
        }
        else
        { // 写入数据
            for (int i = 0; i < strlen(data);i++)
                printf("%d\n", (int)data[i]);
            send_to_buffer(write_buf, "Yes", 4);
            char *write_start = diskfile + (c * nsec + s) * BLOCKSIZE;
            if (l < BLOCKSIZE)
            {
                int i = 0;
                for (i; i < strlen(data);i++)
                    write_start[i] = data[i];
                for (i; i < BLOCKSIZE;i++)
                    write_start[i] = 0;
            }
            else{
                for (int i = 0; i < BLOCKSIZE;i++)
                    write_start[i] = data[i];
            }
        }
    }
    return 0;
}

int cmd_e(tcp_buffer *write_buf, char *args, int len) {
    send_to_buffer(write_buf, "Bye!", 5);
    return -1;
}

static struct {
    const char *name;
    int (*handler)(tcp_buffer *write_buf, char *, int);
} cmd_table[] = {
    {"I", cmd_i},
    {"R", cmd_r},
    {"W", cmd_w},
    {"E", cmd_e},
};

#define NCMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

void add_client(int id) {
    // some code that are executed when a new client is connected
    // you don't need this in step1
}

int handle_client(int id, tcp_buffer *write_buf, char *msg, int len) {
    char *p = strtok(msg, " \r\n");
    printf("p的值为%s\n",p);
    int ret = 1;
    for (int i = 0; i < NCMD; i++)
        if (strcmp(p, cmd_table[i].name) == 0) {
            ret = cmd_table[i].handler(write_buf, p + strlen(p) + 1, len - strlen(p) - 1);
            break;
        }
    if (ret == 1) {
        static char unk[] = "Unknown command";
        send_to_buffer(write_buf, unk, sizeof(unk));
    }
    if (ret < 0) {
        return -1;
    }
}

void clear_client(int id) {
    // some code that are executed when a client is disconnected
    // you don't need this in step2
}

int main(int argc, char *argv[]) {
    if (argc !=6) {
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
    if(fd<0){
        printf("Error: Could not open file '%s'.\n", diskfname);
        exit(-1);
    }

    // stretch the file
    long FILESIZE = BLOCKSIZE * nsec * ncyl;//磁盘文件大小
    int result = lseek(fd, FILESIZE - 1, SEEK_SET);
    if (result == -1)
    {
        perror("Error calling lseek() to 'stretch' the file");
        close(fd);
        exit(-1);
    }

    result = write(fd, "", 1);//结尾插入空字符
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
    tcp_server server = server_init(port, 1, add_client, handle_client, clear_client);
    server_loop(server);
}
