#ifndef _SUPERBLOCK_
#define _SUPERBLOCK_

#include <stdio.h>
#include <stdint.h>

//在super_block中会记录freeinode和block，
//在我的设计中，16 个uint_32即可记录512个inode，32个uint_32可以记录1024个block
#define INODE_MAP 16
#define BLOCK_MAP 32

// 212B 给super_block1个block就够用了
typedef struct super_block
{
    uint32_t used_inodes_count;//使用的
    uint32_t used_blocks_count;
    uint32_t free_inodes_count;//空闲的
    uint32_t free_blocks_count;
    uint32_t root;
    uint32_t inode_map[INODE_MAP]; // 按位记录inode是否空闲
    uint32_t block_map[BLOCK_MAP]; // 按位记录block是否空闲
    
} super_block;  


#endif