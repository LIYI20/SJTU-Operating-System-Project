#ifndef _CMD_
#define _CMD_
#include<stdio.h>
typedef struct cmd
{
    char type;
    int block_id;
    char data[256];
} cmd;
#endif