#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// buffer_size
const int buffer_size = 5;

int main(int argc, char *argv[])
{
	//argc 是argv传入的个数,包含了src文件路径和target文件路径，buffer大小
	if(argc!=3){
		printf("Error: argc's value is not allowed\n");
		exit(-1);
		}
	char *src_path, *target_path;
	src_path = argv[1];
	target_path = argv[2];

	char *rbuffer = malloc(buffer_size);//设定buffer
	char *wbuffer = malloc(buffer_size);
	if(!rbuffer||!wbuffer){
		printf("Error: Fail to create buffers");
	}
	//创建管道
	int mypipe[2];
	if (pipe(mypipe))
	{
		fprintf(stderr, "Pipe failed.\n");
		return -1;
	}

	clock_t start, end;
	double elapsed;
	start = clock();
	
	// 创建子进程
	pid_t pid;
	pid=fork();
	if(pid<0){
		printf("Error: Failed to fork.\n");
		exit(-1);
	}
	else if(pid==0){
		FILE *src=fopen(src_path,"r");
		if(src==NULL){
			printf("Error: Could not open file %s\n",src_path);
			exit(-1);
		}
		close(mypipe[0]); //要向pipe中写
		size_t len=0;
		while ((len = fread(rbuffer, 1, buffer_size, src)) > 0)
		{
			write(mypipe[1], rbuffer, len);//向pipe中写
		}
		close(mypipe[1]);
		fclose(src);
		free(rbuffer);
	}
	//父进程
	else if(pid>0){
		FILE *target = fopen(target_path, "w");
		if (target == NULL)
		{
			printf("Error: Could not open file %s\n", target_path);
			exit(-1);
		}
		close(mypipe[1]);
		size_t len = 0;
		while((len=read(mypipe[0],wbuffer,buffer_size))>0){
			fwrite(wbuffer, 1, len, target);
		}
		close(mypipe[0]);
		fclose(target);
		free(wbuffer);
		//time
		end = clock();
		elapsed = ((double)(end - start)) /CLOCKS_PER_SEC * 1000;
		printf("Time used: %f millisecond\n", elapsed);
	}
}
