# Test run of Project 1

##  Build

### Usage

> Input "make" in the command line to build.

You will see

![image-20240401112057850](C:\Users\w\AppData\Roaming\Typora\typora-user-images\image-20240401112057850.png)

## Copy

### Usage

```shell
./<CopyFunction> <InputFile> <OutputFile> 
```

- This command can copy InputFile to OutputFile.

- The buffer_size is set by the program.

For example, we can use the following command to write a file `src.text` with the same content as `target.text`.

```shell
./MyCopy src.text target.text
./ForkCopy src.text target.text
./PipeCopy src.text target.text
```

If executed successfully, the program will output the execution time such as:

```shell
Time used: 0.354000 milliseconds.
```

###  Test

>  I test the Copy function by "text" and "jpg"

**command**

```shell
./MyCopy src.text target.text
./ForkCopy src.text target.text
./PipeCopy src.text target.text

./MyCopy src.jpg target.jpg
./ForkCopy src.jpg target.jpg
./PipeCopy src.jpg target.jpg
```

**result**

- The "target.text" and "target.jpg" are created and the contents are same.

![image-20240331222751583](C:\Users\w\AppData\Roaming\Typora\typora-user-images\image-20240331222751583.png)

![image-20240331222535725](C:\Users\w\AppData\Roaming\Typora\typora-user-images\image-20240331222535725.png)

![image-20240331223757724](C:\Users\w\AppData\Roaming\Typora\typora-user-images\image-20240331223757724.png)



### Error 

* Wrong number of command line arguments.

  ```shell
  Error: argc's value is not allowed
  ```
  
* Fail to malloc

  ```shell
  Error: Fail to create buffers
  ```

* Fail to fork the sub process.

  ```shell
  Error: Failed to fork.
  ```

* Fail to create pipe

  ```shell
  Error: Pipe failed.
  ```

* Fail to open file

  ```shell
  "Error: Could not open file %s\n",src_path
  ```

  

## Shell

### Usage

#### Run the server

```shell
./shell <Port>
```

This command can run the shell server on Port. If executed successfully, the server will output the message.

```
The server is running...
```

#### Client connect to the server

When the server is running, we can use the following command to connect to server. The server support multiple clients to connect at the same time.

```bash
telnet localhost <Port>
```

If executed successfully, the client will be shown as a system shell. For example:

```shell
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
Connect sucessfully!Welcome to my shell!
liyi@liyi-VirtualBox:/home/liyi/homework/Project1/Shell$
```

### Test

#### Command with pipes

You can use the client terminal as a  shell. For example, input:

> Note that there are space between '|' and other character.

```shell
ls -l | wc
```

The server terminal will output :

```shell
request: ls -l
command_num:4		#it include '|'
108 115
45 108
124					#'|'
119 99
pipe_id:2
pipe_id:-1
```

The client terminal will output the result of the command:

```shell
      3     20     111
```

#### CD 

You can use `cd` command in my shell just as the system shell. Different client can have different working directories.

input:

```shell
liyi@liyi-VirtualBox:/home/liyi/homework/Project1/Shell$cd .. 
```

output:

```shell
liyi@liyi-VirtualBox:/home/liyi/homework/Project1$ 
```

#### Exit

You can input `exit` command in the client to close the connection.

input:

```shell
liyi@liyi-VirtualBox:/home/liyi/homework/Project1/Shell$exit 
```

output:

```shell
Connection closed by foreign host.
liyi@liyi-VirtualBox:~$
```



### Error 

* Wrong number of command line arguments.

  ```shell
  Error: argc's value is not allowed
  ```

* Fail to connect between server and client.

  ```shell
  Unable to connect to remote host: Connection refused
  ```

* Fail to fork

  ```shell
  Error: Failed to execute the commmand
  ```

* Fail to change directory

  ```shell
  Error: can't change directory
  ```

* Invalid commands or invalid arguments of the command.

  ```shell
  Error: Failed to execute the commmand
  ```
  
* Fail to create pipe

  ```shell
  Error: Pipe failed.
  ```


## Sort

### Usage

There are three ways to run this task：

```shell
./MergesortSingle # run in the single thread mode
./MergesortMulti <max_thread_num> # run in the multi thread mode, read input from stdin
```

If executed successfully, the program will output the execution time such as:

```shell
Time used: 0.652000 millisecond
```

### Test

#### MergesortSingle

**input**

```
./MergesortSingle
请输入将要排序数组的长度
10 
请输入数组，以空格分割
10 9 8 7 6 5 4 3 2 1
```

**output**

```shell
排序后的数组为：
1 2 3 4 5 6 7 8 9 10 Time used: 0.104000 millisecond
```



#### MergesortMulti

**input**

```
./MergesortMulti 16
请输入将要排序数组的长度
10 
请输入数组，以空格分割
10 9 8 7 6 5 4 3 2 1
```

**output**

```shell
排序后的数组为：
1 2 3 4 5 6 7 8 10 9 Time used: 6.956000 millisecond
```

### Error 

The error prompt is similar to Copy.