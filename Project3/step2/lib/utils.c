#include <stdint.h>
#include "utils.h"
#include "inode.h"
#include "superblock.h"
#include "tcp_utils.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"
#include "dinode.h"
#include "command.h"

super_block supb;
inode inode_table[512];
int  client;//FS的sock
dinode dir_items[8]; // 256bytes


// init
int init_inode(inode *node, uint8_t mode, uint8_t link_count, uint16_t uid, uint16_t size, uint16_t index, uint16_t parent)
{
    node->mode = mode;
    node->link_count;
    node->uid = uid;
    node->size = size;
    node->index = index;
    node->parent_inode = parent;
    node->time = time(NULL);
    memset(node->direct_block, 0, sizeof(node->direct_block));
    node->single_indirect = 0;
    // 把inode写入到disk中
    write_inode_to_disk(node, index, client);
    return 0;
};

// dinode init 目录文件初始化
int init_dinode(dinode *dnode, uint8_t valid, char *name, uint16_t id, uint8_t type)
{
    dnode->valid = 1;
    strcpy(dnode->name, name);
    dnode->inode_id = id;
    dnode->type = type;
    return 0;
};

// 和disk交互
//  向disk中写block
int write_block(int block_id, char *buf, int client)
{
    if (block_id < 0 || block_id >= MAX_BLOCK_COUNT)
        return -1;

    cmd client_cmd;
    client_cmd.block_id = block_id;
    client_cmd.type = 'W';
    memcpy(client_cmd.data, buf, BLOCK_SIZE);
    for (int i = 0; i < 256; i++)
        printf("%d", client_cmd.data[i]);
    printf("\n");

    if (send(client, &client_cmd, sizeof(client_cmd), 0) < 0)
    {
        perror("Failed to send command to disk server");
        return -1;
    }
    char tmp[256];
    memset(tmp, 0, sizeof(tmp));
    int ret = recv(client, tmp, BLOCK_SIZE, 0);
    if (ret <= 0)
    {
        perror("Failed to receive response from disk server");
        return -1;
    }
    return 0;
}

// 向disk中读block
int read_block(int block_id, char *buf, int client)
{
    if (block_id < 0 || block_id >= MAX_BLOCK_COUNT)
        return -1;

    // 发送R请求,获得block到buffer
    // 读
    cmd client_cmd;
    client_cmd.block_id = block_id;
    client_cmd.type = 'R';

    if (send(client, &client_cmd, sizeof(client_cmd), 0) < 0)
    {
        perror("Failed to send command to disk server");
        return -1;
    }

    int ret = recv(client, buf, BLOCK_SIZE, 0);
    if (ret <= 0)
    {
        perror("Failed to receive response from disk server");
        return -1;
    }

    return 0;
}

int alloc_inode() {
    if (!supb.free_inodes_count)
        return -1;
    for (int i = 0; i < INODE_MAP; i++)
    {
        uint32_t node = supb.inode_map[i];
        for (int j = 0; j < 32; j++)
        {
            if ((node >> (31 - j)) & 1) // 当该bit为1代表不空闲
                continue; 
            else
            {
                supb.inode_map[i] |= 1 << (31 - j);
                supb.free_inodes_count--;
                supb.used_inodes_count++;
                
                char buf[BLOCK_SIZE]; // 将修改后的spb经由spb写入内存块1-3
                memset(buf, 0, sizeof(buf));
                memcpy(buf, &supb, sizeof(supb));
                if (write_block(0, buf, client) < 0)
                    return -1;
                return i * 32 + j;
            }
        }
    }
    return -1;
};

int free_inode(inode *node) {

    //  root
    if (node->index == 0)
        return -1; 

    //在superblock的map中去掉该inode
    int i = node->index / 32;
    int j = node->index % 32;
    // 如果本身就不在disk中，直接返回就行
    if (((supb.inode_map[i] >> (31 - j)) & 1) == 0)
        return 1; 
    else
    {
        // 清空direct_block
        for (int k = 0; k < 8; k++) 
            if (node->direct_block[k] != 0)
                free_block(node->direct_block[k]);
        if (node->single_indirect != 0)
        { // 清空一级indirect
            char buf[BLOCK_SIZE];
            uint16_t indirect_blocks[128];
            if (read_block(node->single_indirect, buf,client) < 0)
                return -1;
            memcpy(&indirect_blocks, buf, BLOCK_SIZE);
            for (int k = 0; k < 127; k++)
                if (indirect_blocks[k] != 0)
                    free_block(indirect_blocks[k]);
            free_block(node->single_indirect);
        }
        if (init_inode(node,0,0,0,0, node->index, 114514) < 0)
            return -1; //  还原此inode

        supb.inode_map[i] ^= 1 << (31 - j); // 修改inode table
        supb.free_inodes_count++;
        supb.used_inodes_count--;
        // if (supb.free_blocks_count > MAX_INODE_COUNT)
        //     return -1;

        char buf[BLOCK_SIZE]; // 将修改后的spb经由spb写入内存块1-3
        memset(buf, 0, sizeof(buf));
        memcpy(buf, &supb, sizeof(supb));
        if (write_block(0, buf, client) < 0)
            return -1;
        return 0;
    }
};

int alloc_block() {
    if (!supb.free_blocks_count)
        return -1;
    for (int i = 2; i < BLOCK_MAP; i++)//map从2开始，在superblock的init中，前两个已经用满了。
    {
        uint32_t block = supb.block_map[i];
        for (int j = 0; j < 32; j++)//对每一位进行寻找
        {
            if ((block >> (31 - j)) & 1)//当该bit为1代表不空闲
                continue; 
            else
            {
                supb.block_map[i] |= 1 << (31 - j);
                supb.free_blocks_count--;
                supb.used_blocks_count++;
               
                char buf[BLOCK_SIZE]; // 将修改后的spb经由spb写入内存块1-3
                memset(buf, 0, sizeof(buf));
                memcpy(buf, &supb, sizeof(supb));
                if (write_block(0, buf, client) < 0)
                    return -1;
                return i * 32 + j;
            }
        }
    }
    return -1;
};

int free_block(uint16_t index) {
    // 前65个block是放superblock和inode table的，不能free
    if (index < 65)
        return -1; 
    int i = index / 32;
    int j = index % 32;
    if (((supb.block_map[i] >> (31 - j)) & 1) == 0)
        return 1; 
    else
    {
        supb.block_map[i] ^= 1 << (31 - j);
        supb.free_blocks_count++;
        supb.used_blocks_count--;

        char buf[BLOCK_SIZE]; // 清空对应块
        memset(buf, 0, sizeof(buf));
        if (write_block(index, buf, client) < 0)
            return -1;

        char buf_2[BLOCK_SIZE]; // 将修改后的spb经由spb写入内存块1-3
        memset(buf_2, 0, sizeof(buf_2));
        memcpy(buf_2, &supb, sizeof(supb));
        if (write_block(0, buf_2, client) < 0)
            return -1;
        return 0;
    }
    return -1;
};

// 把inode写入到disk中
int write_inode_to_disk(inode *node, uint16_t index, int client)
{
    // block id
    int bid = index / INODE_PER_BLOCK + START_OFFSET;
    int offset = index % INODE_PER_BLOCK;
    // 写入block的起始位置
    int start = offset * sizeof(inode);
    // 因为BDS的写入是直接对块执行对的，不能从offset开始，所以需要把node扩充到block之后再进行写入
    static char buf[BLOCK_SIZE];
    // 发送R请求,获得block到buffer
    read_block(bid, buf, client);

    // 在现有的block上进行修改
    printf("%d\n", sizeof(inode));
    memcpy(buf + start, node, sizeof(inode));

    // 发送写请求
    write_block(bid, buf, client);
    return 0;

    // int block_id = INODE_TABLE_START_BLOCK + index / INODE_PER_BLOCK;
    // int inode_start = sizeof(inode) * (index % INODE_PER_BLOCK);
    // char buf[BLOCK_SIZE];
    // if (read_block(block_id, buf) < 0)
    //     return -1;
    // memcpy(buf + inode_start, node, sizeof(inode));
    // write_block(block_id, buf, 1);
    // return 0;
}


