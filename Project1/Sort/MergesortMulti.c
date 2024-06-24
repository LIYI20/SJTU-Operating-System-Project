//mutil thread to do the mergesort
//We need to set the max_thread

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
void *MergeSort(void *arg);
void Merge(int *array, int left, int mid, int right);

//define argemnts
struct arguments{
    int *array;
    int left;
    int right;
};
int max_thread;
int used_thread = 0; // 被使用的线程数

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Error: argc's value is not allowed\n");
        exit(-1);
    }
    max_thread= atoi(argv[1]);
    int n;
    printf("请输入将要排序数组的长度\n");
    scanf("%d", &n);
    printf("请输入数组，以空格分割\n");
    int *array = (int *)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++)
        scanf("%d", &array[i]);

    clock_t start, end;
    double elapsed;
    start = clock();

    struct arguments args = {array, 0, n - 1};
    pthread_t t0;
    pthread_create(&t0, NULL, MergeSort, &args);
    pthread_join(t0, NULL);
    // 打印函数
    printf("排序后的数组为：\n");
    for (int i = 0; i < n; i++){
        printf("%d ", array[i]);
    }

    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    printf("Time used: %f millisecond\n", elapsed);
    
    free(array);
    return 0;
}
void* MergeSort(void*arg)
{
    struct arguments* my_arg = (struct arguments *)arg;
    int *array = my_arg->array;
    int left = my_arg->left;
    int right = my_arg->right;

    if (left >= right)
        pthread_exit(0);
    int mid = (left + right) >> 1;
    struct arguments my_arguments[2];
    //构造arguments
    my_arguments[0].array = array;
    my_arguments[0].left = left;
    my_arguments[0].right = mid;

    my_arguments[1].array = array;
    my_arguments[1].left = mid + 1;
    my_arguments[1].right = right;
    //创建线程来执行子问题
    if(used_thread<max_thread){
        pthread_t t1,t2;
        used_thread += 2;
        int rc1 = pthread_create(&t1, NULL, MergeSort, &my_arguments[0]);
        if(rc1)
        {
            printf("ERROR; return code from pthread_create(t1) is %d\n", rc1);
            exit(-1);
        }

        int rc2 = pthread_create(&t2, NULL, MergeSort, &my_arguments[1]);
        if (rc2)
        {
            printf("ERROR; return code from pthread_create(t1) is %d\n", rc2);
            exit(-1);
        }

        int rc3 = pthread_join(t1, NULL);
        if (rc3)
        {
            printf("Error: return code from pthread_join(t1) is %d\n", rc3);
            exit(1);
        }
        used_thread--;
        int rc4 = pthread_join(t2, NULL);
        if (rc4)
        {
            printf("Error: return code from pthread_join(t2) is %d\n", rc4);
            exit(1);
        }
        used_thread--;
        Merge(array, left, mid, right);
    }

    else {
        MergeSort(&my_arguments[0]);
        MergeSort(&my_arguments[1]);
        Merge(array, left, mid, right);
    }
    pthread_exit(0);
}

void Merge(int *array, int left, int mid, int right)
{
    int len = right - left + 1;
    int *res = (int *)malloc(sizeof(int) * len);
    int i = left;
    int j = mid + 1;
    int k = 0;
    while (i <= mid && j <= right)
    {
        if (array[i] < array[j])
        {
            res[k++] = array[i++];
        }
        else
            res[k++] = array[j++];
    }

    if (i == mid + 1)
    {
        while (j <= right)
        {
            res[k++] = array[j++];
        }
    }

    if (j == right + 1)
    {
        while (i <= mid)
        {
            res[k++] = array[i++];
        }
    }

    for (int i = 0; i < len; i++)
        array[i + left] = res[i];
    free(res);
}