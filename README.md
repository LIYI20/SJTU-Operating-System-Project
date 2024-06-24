## 操作系统课程设计项目

> 基于linux系统下的设计

### Project 1

- 分别包括copy，shell，sort三部分

- copy利用三种不同方式实现copy
- shell要求写一个通过sock通信的服务端，来模拟shell，不过并没有重写底层的命令执行，而是直接调用，侧重于多线程的调用
- sort是归并排序来比较单线程和多线程对程序的性能影响

### Project 2

- project 2是解决两个线程冲突的问题，加强对锁和信号量的理解和在避免线程冲突中的使用

### Project 3

- project模拟一个操作系统的文件系统，需要三个部分，客户端、file_system server 、disk存储 server。
- 要求两两通过socket通信。
- step 1 写一个简单的disk存储服务端，同时模拟磁盘的寻块过程。

- step 2 要写一个file_system server当disk serve的client，同时再创建一个fs serve的client。file_system需要设计block、inode 、inode table，同时设计好inode内字长和在block中的储存设计。
- step 3在step 2的基础上加一个fs和fc的交互，多用户处理。需要考虑多线程问题。