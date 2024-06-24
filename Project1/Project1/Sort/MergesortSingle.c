//sigle thread to do the mergesort

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
void MergeSort(int *array, int left, int right);
void Merge(int *array, int left, int mid, int right);

int main(int argc, char *argv[])
{
    int n;
    printf("请输入将要排序数组的长度\n");
    scanf("%d", &n);
    printf("请输入数组，以空格分割\n");
    int *array = (int *)malloc(sizeof(int) * n);
    for (int i = 0; i < n;i++)
        scanf("%d", &array[i]);


    clock_t start, end;
    double elapsed;
    start = clock();
    MergeSort(array, 0, n - 1);
    //打印函数
    printf("排序后的数组为：\n");
    for (int i = 0; i < n;i++)
        printf("%d ", array[i]);

    end = clock();
    elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    printf("Time used: %f millisecond\n", elapsed);
    
    free(array);
    return 0;
}
void MergeSort(int *array,int left,int right){
    if(left>=right)return;
    int mid = (left+right)>>1;
    MergeSort(array, left, mid);
    MergeSort(array, mid + 1, right);
    Merge(array, left, mid, right);
}

void Merge(int *array, int left, int mid, int right){
    int len = right - left + 1;
    int *res = (int *)malloc(sizeof(int) * len);
    int i = left;
    int j = mid + 1;
    int k = 0;
    while(i<=mid&&j<=right){
        if(array[i]<array[j]){
            res[k++] = array[i++];
        }
        else
            res[k++] = array[j++];
    }

    if (i == mid + 1){
        while(j<=right){
            res[k++] = array[j++];
        }
    }

    if(j==right+1){
        while(i<=mid){
            res[k++] = array[i++];
        }
    }

    for (int i = 0; i < len;i++)
        array[i + left] = res[i];
    free(res);
}