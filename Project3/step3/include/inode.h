#ifndef INODE_H
#define INODE_H

#include <stdio.h>
#include <stdint.h>

// define inode  把inode设置为32Byte,设计如下
//single_indirect_inode 对应一个256Byte的block，256B/16b=8*16=128
//所以一个文件最大为256B*（8+128）=34KB，符合条件
typedef struct inode
{
    // uint8_t uid;                 //owner id
    uint8_t mode;                   // 0代表empty，1代表文件，2代表目录
    uint8_t link_count;             // block数量
    uint16_t uid;                   // owner id
    uint16_t size;                  // 文件大小
    uint16_t index;                 // inode 编号
    uint32_t time;                  // 记录文件修改时间
    uint16_t parent_inode;          // parent_inode编号
    uint16_t direct_block[8];       // 8个直接块,直接连接block
    uint16_t single_indirect; // 一个间接块,间接块仍然连接inode

} inode;



#endif