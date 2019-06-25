# 实现`malloc` & `free()`
## 概述
`malloc()`和`free()`的实现参考博文[张洋的博文《如何实现一个malloc》](http://blog.codinglabs.org/articles/a-malloc-tutorial.html). 数据结构和算法与之类似，只是细节实现上略由不同，比如原文定义的全局变量`first_block`，显然无法在我的系统中使用，因为会有多个进程调用`malloc()`，所以每个进程都需要在PCB中维护一个指针`first_chunk`, 参见[include/proc.h](./include/proc.h)；另外，对于`first_chunk`的访问也设置了getter和setter的系统调用. 具体的实现细节参见[mm/malloc/malloc.c](./mm/malloc/malloc.c).

## 测试
### 测试内容
在`TaskA`和`TaskD`中进行测试. 测试内容包括:

1. `malloc()`的返回值
2. chunk链的连接情况，以及每个chunk的元数据
3. `free()`的各种情形:边界上的chunk、空闲chunk的合并、`program break`的调整，等
4. 不同进程调用分配内存时的地址空间独立性

### 测试结果
结果符合预期：

![](screenshot/out1.png)

![](screenshot/out2.png)

## More
### 针对`free()`的错误测试将在后面给出
### BUG修复——中断重入导致进程调度上的BUG
详见[kernel/proc.c=>schedule()的注释](./kernel/proc.c)
