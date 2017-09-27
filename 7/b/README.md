# b. 几点补充

## 1.在`Loader`的实模式下通过`15h`中断获得内存信息，在保护模式下通过`DispMemInfo`函数打印出来:
![b](screenshot/b.png)

## 2.整理`Loader`的数据段
![loaderdata](screenshot/loaderdata.png)

## 3.使用`左Shift`和`右Shift`实现向上/向下滚屏

## 特别注意:
由于`loader.bin`扩充后，`loader.bin`和`kernel.bin`加载到内存中后发生重叠，导致奇怪的异常，比如打印字符串出现乱码. 要解决此问题只需调整二者的加载地址.
