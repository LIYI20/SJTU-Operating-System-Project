#ifndef _UTILS_
#define _UTILS_

#include<stdio.h>
#include<stdint.h>
#include "inode.h"
#include "superblock.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "tcp_utils.h"
// 封装对block和inode分配、回收、读写等方法。

#define INODE_PER_BLOCK 8
#define START_OFFSET 1 // disk存放superblock后内存的起始位置
#define ncyl 16
#define nsec 64
#define BLOCK_SIZE 256
#define MAX_BLOCK_COUNT
#define USED_BLOCK 65        // 1个给superblock，64个给inode table
#define MAX_INODE_COUNT 512  // 我打算给inode 64个block，那么就有64*8=512个inode
#define MAX_BLOCK_COUNT 1024 // 定义1024个block，16*64 可以定义ncly=16，nsec=64，刚好用完


// 用extern防止编译出错，因为这个.h文件会被多个.c文件所使用，链接时会出错
extern super_block supb;
// inode table，在utils.c文件中记录了MAX_INODE_COUNT,我设计的是512个inode
extern inode inode_table[512];

// 创建disk的客户端
extern int client;


//alloc and  free
int alloc_inode();
int free_inode(inode *node);
int alloc_block();
int free_block(uint16_t index);

//
int write_inode_to_disk(inode *node, uint16_t index,int client);
int write_block(int block_id, char *buf, int client);
int read_block(int block_id, char *buf, int client);


#endif