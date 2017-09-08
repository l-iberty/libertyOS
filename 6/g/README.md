# g. 初步实现`open()`

## 测试代码
![taskc](screenshot/taskc.png)

## 测试结果
### 输出:
![g](screenshot/g.png)

### 硬盘映像:
### 运行后的硬盘映像如下:
- `inode_map`

![inode_map1](screenshot/inode_map1.png)

- `sector_map`

![sector_map1](screenshot/sector_map1.png)

- `inode_array`

![inode_array1](screenshot/inode_array1.png)

- `root_dir`

![root_dir1](screenshot/root_dir1.png)

#### 将`TaskC`的测试代码注释掉, 初始硬盘映像如下
- `inode_map`

![inode_map2](screenshot/inode_map2.png)

- `sector_map`

![sector_map2](screenshot/sector_map2.png)

- `inode_array`

![inode_array2](screenshot/inode_array2.png)

- `root_dir`

![root_dir2](screenshot/root_dir2.png)

### 对比后发现，`open()`对文件系统各部分数据结构的操作暂时是正确的. 以下是几点说明:
- 第一个创建的文件`/blah`的`inode`结构里的`i_start_sector`(文件数据所在的第一个扇区号)为`0x88`，该扇区紧随根目录占据的那个扇区. `include/fs.h`定义了常量`FIRST_SECTOR`，其值即为`0x88`.
- 目前仅能创建文件，不能向文件写入数据，那么暂且认为文件数据占据 0 个扇区，即，将`i_size`和`i_nr_sectors`定为 0, `i_start_sector`从`FIRST_SECTOR`开始依次递增，每次创建文件时仅在`sector_map`里分配 1 个 bit.

> 文件系统部分暂时告一段落，后续还将在`do_open()`内添加: <1>如果打开字符设备该如何处理? <2>添加对`O_RDWR`的处理，目前仅支持`O_CREAT`.
