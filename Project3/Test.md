# Test run of Project 3

## Step1

### Usage

```shell
make
./BDS <DiskFileName> <#cylinders> <#sector per cylinder> <track-to-track delay> <port=10356>	
./BDC <DiskServerAddress> <port=10356>	
```

- Command test


```shell
I
W 2 2 5 12345
R 2 2
```

### Test

**Command test**

```shell
I
W 2 2 5 12345
R 2 2
```

**Running screenshot**

**BDC:**

![image-20240607201750277](C:\Users\w\AppData\Roaming\Typora\typora-user-images\image-20240607201750277.png)

**BDS:**

​		![image-20240607201839563](C:\Users\w\AppData\Roaming\Typora\typora-user-images\image-20240607201839563.png)



## Step2

### Usage

```shell
cd src
make
./BDS <DiskFileName> <#cylinders> <#sectors per cylinder> <track-to-track delay> <port=10356>
./FS <DiskServerAddress> <BDSPort=10356> <FSPort=12356>
./FC <ServerAddr> <FSPort=12356>
```

### Test

**Command** 

```shell
f
mk a
mk b
mkdir c
mkdir d
rm a
rmdir c
ls
cd d
cd ..
w b 5 12345
cat b
i b 2 2 98
cat b
d b 2 2 
cat b
e
```

**Result**

![image-20240607203003124](C:\Users\w\AppData\Roaming\Typora\typora-user-images\image-20240607203003124.png)

![image-20240607203024981](C:\Users\w\AppData\Roaming\Typora\typora-user-images\image-20240607203024981.png)

## Step3

### Usage

```shell
cd src
make
./BDS <DiskFileName> <#cylinders> <#sectors per cylinder> <track-to-track delay> <port=10356>
./FS <DiskServerAddress> <BDSPort=10356> <FSPort=12356>
./FC <ServerAddr> <FSPort=12356>

```

### Test

![image-20240607203516122](C:\Users\w\AppData\Roaming\Typora\typora-user-images\image-20240607203516122.png)

![image-20240607203548512](C:\Users\w\AppData\Roaming\Typora\typora-user-images\image-20240607203548512.png)

![image-20240607203610473](C:\Users\w\AppData\Roaming\Typora\typora-user-images\image-20240607203610473.png)