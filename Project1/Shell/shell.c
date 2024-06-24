#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

char temp[1000]; // 通用临时字符串
int parseLine(char *line, char *command_array[]);
void execute_command(char *command_array[], int n, int client_sock);
int main(int argc, char *argv[])
{
    if(argc!=2){
        printf("Error: argc's value is not allowed\n");
        exit(-1);
    }
    int port = atoi(argv[1]);//port
    // 两个命令
    const char *cmd_cd = "cd";
    const char *cmd_exit = "exit";

    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
    bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(serv_sock, 5);
    int client_sock;
    struct sockaddr_in client_addr;
    printf("The server is running...\n");
    while (1)
    {
        socklen_t client_addr_len = sizeof(client_addr);
        client_sock = accept(serv_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        pid_t pid;
        pid = fork();
        if (pid < 0)
        {
            printf("Error: Failed to execute the commmand.\n");
            continue;
        }
        else if (pid == 0)
        {
            close(serv_sock);
            char str[] = "Connect successfully! Welcome to my shell!\n";
            write(client_sock, str, strlen(str));
            char req_msg[100]; // 接收客户端发送的信息
            char *command_array[100];
            int command_num=0;
            ssize_t len;

            while(1){
            //因为要用cd指令，还需要有个导航栏
            char user_path[1000];
            char hostname[100];
            char dir_path[100];
            strcpy(user_path, getlogin());
            gethostname(hostname, 100);
            strcat(user_path, "@");
            strcat(user_path, hostname);
            strcat(user_path, ":");
            getcwd(dir_path, 100);
            strcat(dir_path, "$ ");
            write(client_sock, user_path, strlen(user_path));
            write(client_sock, dir_path, strlen(dir_path));
            //开始读指令
            len = read(client_sock, req_msg, sizeof(req_msg)-1);
            if(len<=0){
                printf("Can't read the command from client");
                continue;
            }
            //经过多次输出发现，read会读回车为\r\n，多两个字符,导致无法匹配
            req_msg[len-2] = '\0';
            printf("requset: %s\n",req_msg);
            command_num=parseLine(req_msg,command_array);
            printf("command_num:%d\n", command_num);
            for (int i = 0; i < command_num;i++){
                for (int j = 0; j < strlen(command_array[i]);j++)
                    printf("%d\t", command_array[i][j]);
                printf("\n");
            }
                if (command_num == 0)
                    continue;
                else if (strcmp(command_array[0], cmd_exit) == 0)
                    break;
                // 处理cd指令
                else if (strcmp(command_array[0], cmd_cd) == 0)
                {
                    int res = chdir(command_array[1]);
                    if (res != 0)
                    {
                       strcpy(temp,"Error: can't change directory\n");
                       write(client_sock, temp, strlen(temp));
                       continue;
                    }
                }
                else
                    execute_command(command_array, command_num, client_sock);
        }
            close(client_sock);
            printf("exit\n");
            exit(0);
        }
        else
            close(client_sock);
    }
}
//执行命令
void execute_command(char *command_array[], int n, int client_sock)
{
    int pipe_id = -1;
    for (int i = 0; i < n; i++)
    {
        if(strcmp(command_array[i],"|")==0){
            pipe_id = i;
            break;
        }   
    }
    printf("pipe_id:%d\n", pipe_id);
    //执行命令
    //错误情况
    if(pipe_id==0||pipe_id==n-1){
        strcpy(temp, "Error: error near unexpected token '|'\n");
        write(client_sock, temp, strlen(temp));
        return;
    }
    //no pipes
    else if(pipe_id==-1){
        // 经过多次测试发现，execvp函数执行成功后会自动结束该进程，通过查询可知，execvp函数会在原有进程上执行一个可执行文件来代替当前进程，
        // 不会有返回值，不同于一般的函数调用，所以要用fork一个子进程来保留当前进程
        pid_t pid;
        pid = fork();
        if (pid < 0)
        {
            strcpy(temp, "Error: Failed to execute the commmand.\n");
            write(client_sock, temp, strlen(temp));
        }
        else if (pid == 0){
            dup2(client_sock, STDOUT_FILENO);
            if (execvp(command_array[0], command_array) == -1)
            {
                printf("Error: running the command ");
                for (int i = 0; i < n; i++)
                    printf("%s ", command_array[i]);
                printf("error\n");
            }
            exit(-1);
        }
        else{
            //等子进程运行完后再return
            wait(NULL);
            return;
        }
    }
        //有pipe时

    else{
        int my_pipes[2];
        if(pipe(my_pipes)==-1){
            strcpy(temp, "Error: Pipe failed.\n");
            write(client_sock, temp, strlen(temp));
        };
        //执行'|'前的指令
        pid_t pid1;
        pid1 = fork();
        if (pid1 < 0)
        {
            strcpy(temp, "Error: Failed to execute the commmand.\n");
            write(client_sock, temp, strlen(temp));
        }
        else if (pid1 == 0)
        {
            close(my_pipes[0]);
            dup2(my_pipes[1], STDOUT_FILENO);
            close(my_pipes[1]);
            char *command_arrayA[100];
            for (int i = 0; i < pipe_id; i++)
                command_arrayA[i] = command_array[i];
            command_arrayA[pipe_id] = NULL;
            if (execvp(command_arrayA[0], command_arrayA) == -1)
            {
                printf("Error: running the command ");
                for (int i = 0; i < n; i++)
                    printf("%s ", command_array[i]);
                printf("error\n");
            }
            exit(-1);
        }
        else
        {
            // 等子进程1运行完后
            wait(NULL);
            pid_t pid2;
            pid2 = fork();
            if (pid2 < 0)
            {
                strcpy(temp, "Error: Failed to execute the commmand.\n");
                write(client_sock, temp, strlen(temp));
            }
            else if (pid2 == 0)
            {
                close(my_pipes[1]);
                close(0);
                dup2(my_pipes[0], STDIN_FILENO);
                close(my_pipes[0]);
                char *command_arrayB[100];
                for (int i = pipe_id + 1; i < n; i++)
                    command_arrayB[i-pipe_id-1] = command_array[i];
                command_arrayB[n-pipe_id-1] = NULL;
                execute_command(command_arrayB, n - pipe_id - 1, client_sock);
                exit(-1);
            }
            else{
                close(my_pipes[0]);
                close(my_pipes[1]);
                wait(NULL);
                return;
            }
        }
    }


}

    // parse function
int parseLine(char *line, char *command_array[])
{
    char *p;
    int count = 0;
    p = strtok(line, " ");
    while (p)
    {
        command_array[count] = p;
        count++;
        p = strtok(NULL, " ");
    }
    //标记字符串数组结束
    command_array[count] = NULL;
    return count;
}
