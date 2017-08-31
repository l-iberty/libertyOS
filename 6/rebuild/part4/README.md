# 重构

## 7. 编码创建文件系统
- `task_hd`

![task_hd](screenshot/task_hd.png)

- `task_fs`

![task_fs](screenshot/task_fs.png)

`mkfs()`、`hd_write()`等核心函数未改动，只是修改了两个进程的框架结构.

- 运行结果

![part4](screenshot/part4.png)

`hd.img`的文件内容还是和`libertyOS/6/d/README.md`描述的一样.
