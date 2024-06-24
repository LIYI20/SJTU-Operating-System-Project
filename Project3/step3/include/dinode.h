#ifndef _DINODE_
#define _DINODE_

#include <stdint.h>

//本来觉得directory和file应该一样，但是好像又不太一样，还是应该为directory单独创建一个inode
//因为directory还要
typedef struct dinode
{
    uint16_t inode_id;        // 当前目录项表示的文件/目录的对应inode
    uint8_t valid;            // 当前目录项是否有效
    uint8_t type;             // 当前目录项类型（文件/目录）
    char name[28];            //凑成32byte和inode一样，方便管理 
} dinode;                   

extern dinode dir_items[8]; // 256bytes

#endif
