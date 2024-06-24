// Write a copy function to do the actual copying
// Use blocks with a suitable size
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

//buffer_size
const int buffer_size = 5;

int main(int argc, char *argv[])
{
    // argc 是argv传入的个数,包含了src文件路径和target文件路径
    if (argc != 3)
    {
        printf("Error: argc's value is not allowed\n");
        exit(-1);
    }
    char *src_path, *target_path;
    src_path = argv[1];
    target_path = argv[2];

    clock_t start, end;
    double elapsed;
    start = clock();
    //src
    FILE *src = fopen(src_path, "r");
    if (src == NULL)
    {
        printf("Error: Could not open file %s\n", src_path);
        exit(-1);
    }

    //target
    FILE *target = fopen(target_path, "w");
    if (target == NULL)
    {
        printf("Error: Could not open file %s\n", target_path);
        exit(-1);
    }

    //buffer
    char *buffer = malloc(buffer_size);
    size_t len = 0;
    while ((len = fread(buffer, 1, buffer_size, src)) > 0)
    {
        fwrite( buffer,1,len,target);
    }

    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    printf("Time used: %f millisecond\n", elapsed);
    return 0;
}
