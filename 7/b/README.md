# b. 几点补充

## 1.在`Loader`的实模式下通过`15h`中断获得内存信息，在保护模式下通过`DispMemInfo`函数打印出来:
![b](screenshot/b.png)

## 2.整理`Loader`的数据段
![loaderdata](screenshot/loaderdata.png)

## 3.使用`左Shift`和`右Shift`实现向上/向下滚屏

## 特别注意:
由于`loader`的扩充，需要修改与加载kernel相关的地址数据(`boot/include/load.inc`, `BaseOfKernel`和`KernelEntryPointPAddr`需要同时增加0x200)，并修改`Makefile`. 对该部分的不当处理导致`c/`中打印字符串时出现乱码.**如何发现错误的:** 字符串常量作为参数传递给函数时，函数将接收到该字符串的地址，于是想到，乱码的来源可能与地址有关，进一步说就是重定位的问题.
