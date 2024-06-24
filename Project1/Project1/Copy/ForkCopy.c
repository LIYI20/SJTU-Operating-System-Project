#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    // 创建子进程
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        printf("Error: Failed to fork.\n");
        exit(-1);
    }
    else if (pid == 0)
    {
        execl("./MyCopy", argv[0],argv[1],argv[2],NULL) ;
    }
    // 父进程
    else if (pid > 0)
    {
        wait(NULL);
        return 0;
    }
}
