#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include "tcp_utils.h"
#include "utils.h"
#include "inode.h"
#include "superblock.h"
#include "dinode.h"


#define false 0
#define true 1

#define INODE_COUNT_PER_BLOCK=8     //256/32=8



// 定义变量
int cur_dir = 114514;       //随便取的
char cur_dir_str[1000];
_Bool format = 0;           // 是否进行格式化

super_block superb;
int superblock_init()
{
    
    supb.used_inodes_count = 0;
    supb.used_blocks_count = 65; // 被使用的块，supb+inode table
    supb.free_inodes_count = MAX_INODE_COUNT;
    supb.free_blocks_count = MAX_BLOCK_COUNT - USED_BLOCK;

    memset(supb.inode_map, 0, sizeof(supb.inode_map));
    memset(supb.block_map, 0, sizeof(supb.block_map));
    for (int i = 0; i < 2; i++) // 这是64个
        supb.block_map[i] = (0xffffffff);
    supb.block_map[2] = (0x80000000); // 再补一个

    char buf[BLOCK_SIZE];
    memset(buf, 0, sizeof(buf)); // 将spb经由spb写入内存块0
    memcpy(buf, &supb, sizeof(supb));
    printf("%d\n", sizeof(supb));
    write_block(0, buf, client);
    return 0;
};

int rootinode_init()
{
    inode root;

    root.mode = 1;
    root.uid = 0;
    root.link_count = 0;
    root.size = 0;
    root.index = 0;
    root.time = time(NULL);
    root.parent_inode = 114514; // 表示没有父节点
    memset(root.direct_block, 0, sizeof(root.direct_block));
    root.single_indirect = 0;
    inode_table[0] = root;
    supb.inode_map[0] |= (1 << 31); // superblock 记录root

    return 0;
};



//目录中查找文件,返回文件所在的位置
int dir_search(inode *node, char *name, int type)
{
    printf("name:%s type:%d\n", name, type);
    if (!node->mode) // 要是dir才能找
        return -1;
    char buf[BLOCK_SIZE];
    dinode dir_items[8];
    printf("%d\n", sizeof(dinode));
    for (int i = 0; i < 8; i++)
    {
        printf("node->direct_block:%d\n", node->direct_block[i]);
        if (node->direct_block[i] == 0)
            continue;
        if (read_block(node->direct_block[i], buf,client) < 0)
            continue;
            //当有连接块时，读取
        memcpy(&dir_items, buf, BLOCK_SIZE);
        for (int j = 0; j < 8; j++)
        {
            if (dir_items[j].valid == 0)//无效
                continue;
            printf("dir_items.name:%s type:%d\n", dir_items[j].name,dir_items[j].type);
            if (strcmp(dir_items[j].name, name) == 0 && dir_items[j].type == type)
            {
                return dir_items[j].inode_id;
            }
        }
    }
    return -1;
}


//向目录中添加文件
int dir_add_inode(inode *node, char *name, uint8_t type)
{
    if (!node->mode)
        return -1; // 不是目录结点
    char buf[BLOCK_SIZE];
    dinode dir_items[8];

    //如果在现有的block中就有空的dinode
    for (int i = 0; i < 8; i++)
    {
        if (node->direct_block[i] == 0)
            continue;
        if (read_block(node->direct_block[i], buf,client) < 0)
            continue;
        memcpy(&dir_items, buf, BLOCK_SIZE);
        for (int j = 0; j < 8; j++)
        { 
            // 找空dinode
            if (dir_items[j].valid == 0)//空
            {
                int id = alloc_inode();//调用alloc_inode来获得一个新的inode给dinode
                printf("new inode:%d\n", id);
                if (id >= 0)
                {
                    //初始化
                    init_inode(&inode_table[id], type,0,0,0,id, node->index);

                    // dir_items[j].valid = 1;
                    // strcpy(dir_items[j].name, name);
                    // dir_items[j].inode_id =id;
                    // dir_items[j].type = type;
                    init_dinode(&dir_items[j], 1, name, id, type);

                    memcpy(buf, &dir_items, BLOCK_SIZE);
                    if (write_block(node->direct_block[i], buf, client) < 0)
                        return -1;

                    node->time = time(NULL);
                    if (write_inode_to_disk(node, node->index,client) < 0)
                        return -1;
                    return 0;
                }
                else
                    return -1;
            }
        }
    }
    // 如果上述过程无结果且link<8，则新分配block
    if (node->link_count < 8)
    {
        int k; // 寻找尚未link的direct编号
        for (k = 0; k < 8; k++)
            if (node->direct_block[k] == 0)
                break;
        printf("空block:%d\n", k);
        int new_block_id = alloc_block();
        printf("new_block_id:%d\n", new_block_id);
        if (new_block_id >= 0)
        {
            int new_inode_id = alloc_inode();
            printf("new_inode_id:%d\n", new_inode_id);
            if (new_inode_id >= 0)
            {
                node->direct_block[k] = new_block_id; // 分配block
                node->link_count += 1;

                for (int i = 0; i < 8; i++)
                {
                    dir_items[i].valid = 0;//0是有效
                    dir_items[i].inode_id = 0;
                }
                init_inode(&inode_table[new_inode_id],type,0,0,0, new_inode_id,node->index);

                // dir_items[0].valid = 1;
                // strcpy(dir_items[0].name, name);
                // dir_items[0].inode_id = new_inode_id;
                // dir_items[0].type = type;
                init_dinode(&dir_items[0], 1, name, new_inode_id, type);

                memcpy(buf, &dir_items, BLOCK_SIZE);
                if (write_block(node->direct_block[k], buf, client) < 0)
                    return -1;


                // //test
                // read_block(node->direct_block[k], buf, client);
                // printf("buffer\n");
                // for (int i = 0; i < 256; i++)
                //     printf("%d", buf[i]);
                // printf("\n");
                // memcpy(&dir_items, buf, BLOCK_SIZE);
                // printf("after:%d\n", dir_items[0].valid);
                // for (int j = 0; j < 8; j++)
                // {
                //     if (dir_items[j].valid == 0) // 无效
                //         continue;
                //     else printf("dir_items.name:%s\n", dir_items[j].name);
                //     // if (strcmp(dir_items[j].name, name) == 0 && dir_items[j].type == type)
                //     // {
                //     //     return dir_items[j].inode_id;
                //     // }
                // }

                node->time = time(NULL);
                if (write_inode_to_disk(node, node->index, client) < 0)
                    return -1;
                return 0;
            }
            else
                return -1;
        }
        else
            return -1;
    }
    return -1;
}


//向目录中删除文件
int dir_remove_inode(inode* node, char *name,int type){
    if (!node->mode)
        return -1;
    char buf[BLOCK_SIZE];
    dinode dir_items[8];
    for (int i = 0; i < 8; i++)
    {
        if (node->direct_block[i] == 0)
            continue;
        if (read_block(node->direct_block[i], buf,client) < 0)
            continue;
        memcpy(&dir_items, buf, BLOCK_SIZE);
        for (int j = 0; j < 8; j++)
        {
            if (dir_items[j].valid == 0)
                continue;
            //应该要区分rmfile和rmdir，因为rmdir的时候可能删除的不止它一个，那么对superblock的维护就比较困难
            //但是我没写完rmdir，应该进去再调用rmfile和rm_dir函数，直到文件夹为空，先这样吧。
            if (strcmp(dir_items[j].name, name) == 0 && dir_items[j].type == type)
            {
                dir_items[j].valid = 0;
                free_inode(&inode_table[dir_items[j].inode_id]);
                //把删完之后的dinode再写回block中
                memcpy(buf, &dir_items, BLOCK_SIZE);
                if (write_block(node->direct_block[i], buf, client) < 0)
                    return -1;

                node->time = time(NULL);
                if (write_inode_to_disk(node, node->index,client) < 0)
                    return -1;
                return 0;
            }
        }
    }
    return -1;
}

//向目录中删除文件夹
// int dir_remove_dir(inode* node, char *name){
//     if (!node->mode)//目录
//         return -1;
//     char buf[BLOCK_SIZE];
//     dinode dir_items[8];
//     for (int i = 0; i < 8; i++)
//     {
//         if (node->direct_block[i] == 0)
//             continue;
//         if (read_block(node->direct_block[i], buf,client) < 0)
//             continue;
//         memcpy(&dir_items, buf, BLOCK_SIZE);
//         for (int j = 0; j < 8; j++)
//         {
//             if (dir_items[j].valid == 0)
//                 continue;
//             if (strcmp(dir_items[j].name, name) == 0 && dir_items[j].type == 1)
//             { // type=1
//                 //对文件夹的删除还要删除文件夹中的文件
//                 inode *dir_node = &inode_table[dir_items[j].inode_id];
//                 for (int i = 0; i < 8; i++)
//                 {
//                     if (dir_node->direct_block[i] == 0)
//                         continue;
//                     if (read_block(dir_node->direct_block[i], buf,client) < 0)
//                         continue;
//                     memcpy(&dir_items, buf, BLOCK_SIZE);
//                     for (int j = 0; j < 8; j++)
//                     {
//                         if (dir_items[j].valid)
//                         {
//                             //文件直接删除
//                             if(dir_items[j].type==0){

//                             }
//                             else{

//                             }
//                         }
//                     }
//                     free_block(dir_node->i_direct[i]);
//                     dir_node->i_direct[i] = 0;
//                     dir_node->i_link_count--;
//                 }

//                 //删除文件夹
//                 free_inode(&inode_table[dir_items[j].inode_id]);
//                 dir_items[j].valid = 0;
//                 memcpy(buf, &dir_items, BLOCK_SIZE);
//                 if (write_block(node->direct_block[i], buf, client) < 0)
//                     return -1;

//                 node->time = time(NULL);
//                 if (write_inode_to_disk(node, node->index,client) < 0)
//                     return -1;
//                 return 0;
//             }
//         }
//     }
//     return -1;
// }

int read_file(inode *node, char *ret)
{
    if (node->mode)
        return -1; 
    
    char buf[BLOCK_SIZE];
    memset(ret, 0, sizeof(&ret));

    int total_blocks = node->link_count;
    // int last_block_size = node->size % BLOCK_SIZE;
    // if (total_blocks > 0 && last_block_size == 0)
    //     last_block_size = BLOCK_SIZE;

    printf("total_blocks:%d", total_blocks);
    if (total_blocks <= 8)
        for (int i = 0; i < total_blocks; i++)
        {
            if (node->direct_block[i] == 0)
                return -1;
            memset(buf, 0, BLOCK_SIZE);
            if (read_block(node->direct_block[i], buf,client) < 0)
                return -1;
            // int data_size = (i == total_blocks - 1) ? last_block_size + 1 : BLOCK_SIZE;
            memcpy(ret + i * BLOCK_SIZE, buf, BLOCK_SIZE);
            // printf("%d %ld\n", i, strlen(ret));
        }
    else
    {
        for (int i = 0; i < 8; i++)
        {
            if (node->direct_block[i] == 0)
                return -1;
            memset(buf, 0, BLOCK_SIZE);
            if (read_block(node->direct_block[i], buf,client) < 0)
                return -1;
            memcpy(ret + i * BLOCK_SIZE, buf, BLOCK_SIZE);
        }
        if (node->single_indirect == 0)
            return -1;
        if (read_block(node->single_indirect, buf,client) < 0)
            return -1;
        uint16_t indirect_blocks[128];
        memcpy(&indirect_blocks, buf, BLOCK_SIZE);
        for (int i = 0; i < total_blocks - 8; i++)
        {
            if (indirect_blocks[i] == 0)
                return -1;
            if (read_block(indirect_blocks[i], buf,client) < 0)
                return -1;
            // int data_size = (i == total_blocks - 8 - 1) ? last_block_size : BLOCK_SIZE;
            memcpy(ret + (i + 8) * BLOCK_SIZE, buf, BLOCK_SIZE);
        }
    }
    return 0;
}

int write_file(inode *node, char *data)
{
    if (node->mode)
        return -1;

    uint16_t indirect_blocks[128];
    memset(indirect_blocks, 0, sizeof(indirect_blocks));
    int new_link_count = (strlen(data) + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int pre_link_count = node->link_count;

    char tmp[new_link_count * BLOCK_SIZE];
    memset(tmp, 0, sizeof(tmp));
    memcpy(tmp, data, strlen(data) + 1);
    char buf[BLOCK_SIZE];
    printf("new_link_count:%d pre_link_count:%d\n", new_link_count, pre_link_count);

    // 原始的block数已经够new file的使用了。
    if (new_link_count <= pre_link_count)
    {
        if (new_link_count <= 8)
        {
            for (int i = 0; i < new_link_count; i++)
            {
                memcpy(buf, tmp + i * BLOCK_SIZE, BLOCK_SIZE);
                write_block(node->direct_block[i], buf, client);
            }
        }

        else
        {
            memcpy(&indirect_blocks, buf, BLOCK_SIZE);
            for (int i = 0; i < (new_link_count - 8); i++)
            {
                memcpy(buf, tmp + (i + 8) * BLOCK_SIZE, BLOCK_SIZE);
                write_block(indirect_blocks[i], buf, client);
            }
        }

        // 归还多余block
        if (pre_link_count <= 8)
        {
            for (int i = new_link_count; i < pre_link_count; i++)
            {
                free_block(node->direct_block[i]);
                node->direct_block[i] = 0;
            }
        }

        else
        {
            for (int i = (new_link_count < 8 ? 0 : new_link_count - 8); i < pre_link_count - 8; i++)
            {
                free_block(indirect_blocks[i]);
                indirect_blocks[i] = 0;
            }
            if (new_link_count <= 8)
            {
                free_block(node->single_indirect);
                node->single_indirect = 0;
            }
            else
            {
                memcpy(buf, &indirect_blocks, BLOCK_SIZE);
                write_block(node->single_indirect, buf, client);
            }
        }
    }
    else
    {
        // 申请不足的新结点
        for (int i = pre_link_count; i < (new_link_count > 8 ? 8 : new_link_count); i++)
        {
            uint16_t new_block_id = alloc_block();
            if (new_block_id < 0)
                return -1;
            node->direct_block[i] = new_block_id;
        }

        if (new_link_count > 8)
        {
            if (pre_link_count <= 8 && node->single_indirect == 0)
            {
                uint16_t new_block_id = alloc_block();
                if (new_block_id < 0)
                    return -1;
                node->single_indirect = new_block_id;
                memset(indirect_blocks, 0, sizeof(indirect_blocks));
            }
            else
            {
                if (read_block(node->single_indirect, buf, client) < 0)
                    return -1;
                memcpy(&indirect_blocks, buf, BLOCK_SIZE);
            }
            for (int i = (pre_link_count < 8 ? 0 : pre_link_count - 8); i < new_link_count - 8; i++)
            {
                uint16_t new_block_id = alloc_block();
                if (new_block_id < 0)
                    return -1;
                indirect_blocks[i] = new_block_id;
            }
            memcpy(buf, &indirect_blocks, BLOCK_SIZE);
            write_block(node->single_indirect, buf, client);
        }
        // 写入
        for (int i = 0; i < (new_link_count >= 8 ? 8 : new_link_count); i++)
        {
            memcpy(buf, tmp + i * BLOCK_SIZE, BLOCK_SIZE);
            write_block(node->direct_block[i], buf, client);
        }
        if (read_block(node->single_indirect, buf, client) < 0)
            return -1;
        memcpy(&indirect_blocks, buf, BLOCK_SIZE);
        for (int i = 0; i < (new_link_count - 8); i++)
        {
            memcpy(buf, tmp + (i + 8) * BLOCK_SIZE, BLOCK_SIZE);
            write_block(indirect_blocks[i], buf, client);
        }
    }
    // 更新
    printf("data:%s data_len:%d time:%d\n", data, strlen(data), time(NULL));
    node->size = strlen(data);
    node->link_count = new_link_count;
    node->time = time(NULL);
    write_inode_to_disk(node, node->index, client);
    return 0;
}

// format
int cmd_f(tcp_buffer *write_buf, char *args, int len){
    //init
    superblock_init();
    rootinode_init();
    cur_dir = 0;//0 是root inode
    strcpy(cur_dir_str, "/");
    for (int i = 1; i < MAX_INODE_COUNT; i++)
    { 
        if (init_inode(&inode_table[i], 0,0,0,0,i,114514) < 0)
        {
            printf("Init inode failed.\n");
            return 0;
        }
    }


    format = true;
    send_to_buffer(write_buf, "Format is done", 15);
    return 0;
}

//create a file
int cmd_mk(tcp_buffer *write_buf, char *args, int len)
{
    if(!format){
        printf("Don't  format\n");
        send_to_buffer(write_buf, "Please format first", 20);
        return 0;
    }
    // for (int i = 0; i < strlen(args);i++)
    //     printf("%d\n", args[i]);
    char *name = strtok(args, " \r\n");
    // printf("name:%s\n", name);

    //在文件夹内判断是否重名
    if (dir_search(&inode_table[cur_dir], name,0)>=0)
    {
        printf("The name has existed");
        send_to_buffer(write_buf, "The name has existed", 21);
        return 0;
    }

    //在dir中添加文件，每个dir有8个直接块，每个块又有8个inode，也就是说一个dir可以有64个文件（包括子文件夹）
    //在添加文件的时候，要搜索所有块，因为空缺块的位置不确定
    if (dir_add_inode(&inode_table[cur_dir], name, 0) < 0)
    {
        printf("mk failed\n");
        send_to_buffer(write_buf, "mk failed", 10);
        return 0;
    }
    else{
        printf("mk successful\n");
        send_to_buffer(write_buf, "mk successful", 14);
        return 0;
    }
    return 0;
}

//create a directory
int cmd_mkdir(tcp_buffer *write_buf, char *args, int len)
{
    //format check
    if (!format)
    {
        printf("Don't  format\n");
        send_to_buffer(write_buf, "Please format first", 20);
        return 0;
    }
    //name
    char *name = strtok(args, " \r\n");

    if (dir_search(&inode_table[cur_dir], name, 1) >= 0)
    { // 重名
        printf("Directory name has existed");
        send_to_buffer(write_buf, "Directory name has existed", 27);
        return 0;
    }
    if (dir_add_inode(&inode_table[cur_dir], name, 1) < 0)
    {
        printf("mkdir failed\n");
        send_to_buffer(write_buf, "mkdir failed", 13);
        return 0;
    }
    else
    {
        printf("mkdir successful\n");
        send_to_buffer(write_buf, "mkdir successful", 17);
        return 0;
    }
    return 0;

}

// remove a file
int cmd_rm(tcp_buffer *write_buf, char *args, int len)
{
    // format check
    if (!format)
    {
        printf("Don't  format\n");
        send_to_buffer(write_buf, "Please format first", 20);
        return 0;
    }
    // name
    char *name = strtok(args, " \r\n");

    if (dir_remove_inode(&inode_table[cur_dir],name,0) < 0){ 
            printf("rm failed\n");
            send_to_buffer(write_buf, "rm failed", 10);
            return 0;
    }
    else{
        printf("rm successful\n");
        send_to_buffer(write_buf, "rm successful", 14);
        return 0;
    }
    return 0;
}

// cd command change directory
int cmd_cd(tcp_buffer *write_buf, char *args, int len)
{
    // format check
    if (!format)
    {
        printf("Don't  format\n");
        send_to_buffer(write_buf, "Please format first", 20);
        return 0;
    }
    // name
    char *name = strtok(args, " \r\n");
    // 先保存当前dir
    uint16_t pre_dir = cur_dir; 
    char pre_dir_str[1000];
    strcpy(pre_dir_str, cur_dir_str);

    char *path = strtok(name, "/");
    while (path != NULL)
    {
        if (strcmp(path, "..") == 0)
        { // 回到上一级
            uint16_t parent = inode_table[cur_dir].parent_inode;
            if (parent != 114514)
            { 
                cur_dir = parent;

                if (strcmp(cur_dir_str, "/") != 0)
                {
                    char *temp = strrchr(cur_dir_str, '/');
                    *temp = '\0';
                }
                if (parent == 0)
                    strcpy(cur_dir_str, "/");
            }
        }
        else 
        { 
            int ret = dir_search(&inode_table[cur_dir], path, 1);
            if (ret == -1)
            {
                printf("NO directory\n");
                send_to_buffer(write_buf, "NO directory", 13);
                cur_dir = pre_dir; // 没有对应文件夹
                strcpy(cur_dir_str, pre_dir_str);
                return 0;
            }
            else{
                if (cur_dir != 0)
                    strcat(cur_dir_str, "/");
                cur_dir = ret;
                strcat(cur_dir_str, path);
            }
           
        }
        path = strtok(NULL, "/");
    }
    char tmp[100];
    sprintf(tmp,"cur_dir:%d  cur_dir_str:%s\n", cur_dir, cur_dir_str);
    printf("%s\n", tmp);
    send_to_buffer(write_buf,tmp,strlen(tmp+1));//一定要send出去，不然会报错
    return 0;
}

// remove directory
int cmd_rmdir(tcp_buffer *write_buf, char *args, int len)
{
    // format check
    if (!format)
    {
        printf("Don't  format\n");
        send_to_buffer(write_buf, "Please format first", 20);
        return 0;
    }
    // name
    char *name = strtok(args, " \r\n");

    if (dir_remove_inode(&inode_table[cur_dir], name,1) < 0)
    {
        printf("rmdir failed\n");
        send_to_buffer(write_buf, "rmdir failed", 13);
        return 0;
    }
    else
    {
        printf("rmdir successful\n");
        send_to_buffer(write_buf, "rmdir successful", 17);
        return 0;
    }
    return 0;
}

int cmp(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b);
}
// ls  Directory listing
int cmd_ls(tcp_buffer *write_buf, char *args, int len)
{
    // format check
    if (!format)
    {
        printf("Don't  format\n");
        send_to_buffer(write_buf, "Please format first", 20);
        return 0;
    }


    _Bool flag = false; // 文件夹是否为空

    char file_name[100][100];
    uint16_t inode_list[100];//存放文件的inode_id
    int file_count = 0, dir_count = 0, count=0;

    dinode dir_items[8];
    inode *node = &inode_table[cur_dir];
    char buf[BLOCK_SIZE];
    char list[10000] = "";
    for (int i = 0; i < 8; i++)
    {
        if (node->direct_block[i] == 0)
            continue;
        if (read_block(node->direct_block[i], buf,client) < 0)
            continue;
        memcpy(&dir_items, buf, BLOCK_SIZE);
        for (int j = 0; j < 8; j++)
        {
            if (dir_items[j].valid)
            {
                printf("%d\n", dir_items[j].inode_id);
                 inode_list[count] = dir_items[j].inode_id;
                strcpy(file_name[count++], dir_items[j].name);
                if (dir_items[j].type == 0){
                    file_count++;
                }

                else{
                    dir_count++;
                }
                 flag = true;
            }
        }
    }
    // 排序
    qsort(file_name, count, sizeof(file_name[0]), cmp);
    
    for (int i = 0; i < count; i++)
    {
        strcat(list, file_name[i]);
        strcat(list, "      ");
        inode *tmp = &inode_table[inode_list[i]];
        char ctmp[100];
        sprintf(ctmp, "type:%d size:%d time:%d\n",tmp->mode, tmp->size, tmp->time);
        strcat(list, ctmp);
    }
    if(flag==false){
        send_to_buffer(write_buf, "dir_num:0\nfile_num:0",21);
    }

    else{
        char ctmp[100];
        sprintf(ctmp, "dir_num:%d\nfile_num:%d", dir_count, file_count);
        strcat(list, ctmp);
        send_to_buffer(write_buf, list, strlen(list) + 1);
    }

    return 0;
}

//  Catch file
int cmd_cat(tcp_buffer *write_buf, char *args, int len)
{
    // format check
    if (!format)
    {
        printf("Don't  format\n");
        send_to_buffer(write_buf, "Please format first", 20);
        return 0;
    }
    // name
    char *name = strtok(args, " \r\n");


    char buf[60000]="";
    int id;
    //没有该文件
    if ((id = dir_search(&inode_table[cur_dir], name, 0)) < 0)
    { 
        printf("File does not exist\n");
        send_to_buffer(write_buf, "File does not exist",20);
        return 0;
    }

    if (read_file(&inode_table[id], buf) == 0)
    {
        printf("%s\n", buf);
        send_to_buffer(write_buf, buf, strlen(buf) + 1);
    }
    else
        send_to_buffer(write_buf, "Can't catch the file",21);
    return 0;
}

//  write file
int cmd_w(tcp_buffer *write_buf, char *args, int length)
{
    // format check
    if (!format)
    {
        printf("Don't  format\n");
        send_to_buffer(write_buf, "Please format first", 20);
        return 0;
    }
    // name
    char* name = strtok(args, " \r\n");//name
    int len = atoi(strtok(NULL, " \r\n"));//len
    char *data = strtok(NULL, " \r\n");//data
    printf("name:%s len:%d data:%s\n", name, len, data);

    int id;
    // 没有该文件
    if ((id = dir_search(&inode_table[cur_dir], name, 0)) < 0)
    {
        printf("File does not exist\n");
        send_to_buffer(write_buf, "File does not exist", 20);
        return 0;
    }
   
    if (write_file(&inode_table[id], data) == 0){
        printf("Write data successful\n");
        send_to_buffer(write_buf, "Write data successful", 22);
    }

    else
    {
        printf("Write data failed\n");
        send_to_buffer(write_buf, "Write data failed", 18);
    }
    return 0;
}

//  insert to a file
int cmd_i(tcp_buffer *write_buf, char *args, int length)
{
    if (!format)
    {
        printf("Don't  format");
        send_to_buffer(write_buf, "Please format first", 20);
        return 0;
    }
    // name
    char *name = strtok(args, " \r\n"); // name
    int pos = atoi(strtok(NULL, " \r\n"));    //pos
    int len = atoi(strtok(NULL, " \r\n"));    // len
    char *data = strtok(NULL, " \r\n");       // data
    printf("name:%s pos:%d len:%d data:%s\n", name, pos, len, data);

    char buf[10000];

    int id = 0;
    if(len<=0)
        return 0;

    // 文件不存在
    if ((id = dir_search(&inode_table[cur_dir], name, 0)) < 0)
    {
        printf("File does not exist\n");
        send_to_buffer(write_buf, "File does not exist", 20);
        return 0;
    }


    int file_size = inode_table[id].size; // 获取文件当前的大小
    if (pos > file_size)
        pos = file_size;//insert到末尾

    int new_size = file_size + len;

    memset(buf, 0, 10000);
    int ret = read_file(&inode_table[id], buf);

    // 在指定位置插入数据
    //memove将src开始的N位移动到dst的位置
    memmove(buf + pos + len, buf + pos, file_size - pos); 
    memcpy(buf + pos, data, len);                        
    buf[new_size] = '\0';

    if (write_file(&inode_table[id], buf) == 0){
        printf("Insert data successful\n");
        send_to_buffer(write_buf, "Insert data successful", 23);
    }

    else
    {
        printf("Insert data failed\n");
        send_to_buffer(write_buf, "Insert data failed", 19);
    }
    return 0;
}

//  delete in the file
int cmd_d(tcp_buffer *write_buf, char *args, int length)
{
    if (!format)
    {
        printf("Don't  format\n");
        send_to_buffer(write_buf, "Please format first", 20);
        return 0;
    }
    // name
    char *name = strtok(args, " \r\n");    // name
    int pos = atoi(strtok(NULL, " \r\n")); // pos
    int len = atoi(strtok(NULL, " \r\n")); // len
    printf("name:%s pos:%d len:%d\n", name, pos, len);

    int id;
    if ((id = dir_search(&inode_table[cur_dir], name, 0)) < 0)
    { // 文件不存在
        printf("File does not exist\n");
        send_to_buffer(write_buf, "File does not exist", 20);
        return 0;
    }

    int file_size = inode_table[id].size; 
    if (pos > file_size)
    {
        printf("Pos can't larger than file_size\n");
        send_to_buffer(write_buf, "Pos can't larger than file_size", 32);
        return 0;
    }

    //当删除长度过长
    if (file_size < pos + len)
        len = file_size - pos;

    char buf[10000];
    memset(buf, 0, sizeof(buf));
    int ret = read_file(&inode_table[id], buf);
   
    // 在指定位置删除数据
    //memmove(buf+pos, buf+pos+len, file_size-pos-len);
    char tmp[10000];
    memcpy(tmp, buf, pos);
    memcpy(tmp + pos, buf + pos + len, file_size - pos - len);
    tmp[file_size - len] = '\0';

    if (write_file(&inode_table[id], tmp) == 0){
        printf("Delete data successful\n");
        send_to_buffer(write_buf, "Delete data successful", 23);
    }
    else
    {
        printf("Delete data failed\n");
        send_to_buffer(write_buf, "Delete data failed", 19);
    }
    return 0;
}

//exit the file system
int cmd_e(tcp_buffer *write_buf, char *args, int len)
{
    send_to_buffer(write_buf, "Bye!", 5);
    return -1;
}

//返回cur_dir
int cmd_dir(tcp_buffer *write_buf, char *args, int len)
{
    send_to_buffer(write_buf,cur_dir_str,strlen(cur_dir_str)+1);
    return 0;
}

// 和BDS类似，也有comand_table
static struct{
    const char *name;
    int (*handler)(tcp_buffer *write_buf, char *, int);
} cmd_table[] = {
    {"f", cmd_f},
    {"mk", cmd_mk},
    {"mkdir", cmd_mkdir},
    {"rm", cmd_rm},
    {"cd", cmd_cd},
    {"rmdir", cmd_rmdir},
    {"d", cmd_d},
    {"ls", cmd_ls},
    {"cat", cmd_cat},
    {"w", cmd_w},
    {"i", cmd_i},
    {"d", cmd_d},
    {"e", cmd_e},
    {"dir", cmd_dir}

};

#define NCMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

void add_client(int id)
{
    // some code that are executed when a new client is connected
    // you don't need this in step1
}

//处理client函数
int handle_client(int id, tcp_buffer *write_buf, char *msg, int len)
{

    // 获得command的头
    char *p = strtok(msg, " \r\n");
    printf("p的值为%s\n", p);
    int ret = 1;
    for (int i = 0; i < NCMD; i++)
        if (strcmp(p, cmd_table[i].name) == 0)
        {
            //处理响应的command
            ret = cmd_table[i].handler(write_buf, p + strlen(p) + 1, len - strlen(p) - 1);
            break;
        }
    if (ret == 1)
    {
        static char unk[] = "Unknown command";
        send_to_buffer(write_buf, unk, sizeof(unk));
    }
    if (ret < 0)
    {
        return -1;
    }
}

void clear_client(int id)
{
    // some code that are executed when a client is disconnected
    // you don't need this in step2
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr,
                "Usage: %s  <DiskServerAddress> <BDSPort> <FSPort>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }
    // args
    int BDS_port= atoi(argv[2]);
    int FS_port= atoi(argv[3]);

    // disk的客户端初始化
    //  client = client_init("localhost", BDS_port);
    // 创建客户端
    client = socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0)
    {
        perror("Failed to create disk server socket");
        return 1;
    }
    struct sockaddr_in disk_server_addr; // disk的地址
    memset(&disk_server_addr, 0, sizeof(disk_server_addr));
    disk_server_addr.sin_family = AF_INET;
    disk_server_addr.sin_addr.s_addr = INADDR_ANY;
    disk_server_addr.sin_port = htons(BDS_port);
    if (connect(client, (struct sockaddr *)&disk_server_addr, sizeof(disk_server_addr)) < 0)
    {
        perror("Failed to connect to disk server");
        return 1;
    }
    printf("Connected to disk server\n");

    //创建file_system服务端
    tcp_server server = server_init(FS_port, 1, add_client, handle_client, clear_client);
    server_loop(server);
}
