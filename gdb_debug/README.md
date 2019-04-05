# 使用gdb进行源码级调试
为了保留使用`--enable-disasm --enable-debugger`作为配置参数编译出来的Bochs所具备某些特性，需要在虚拟机中安装支持gdb远程调试的Bochs，需要时才通过虚拟机里进行源码调试.

要想进行gdb源码级调试，需要在虚拟机中使用`--enable-gdb-stub`进行配置(**只需要这一个选项**)，然后`make`,`make install`.`bochsrc`文件中加上

```
gdbstub: enabled=1, port=1234, text_base=0, data_base=0, bss_base=0
```

然后将不带符号信息和调试信息的`kernel.bin`写入`a.img`, 并把它放到虚拟机里, 用这个镜像启动Bochs，此时Bochs会等待gdb的远程连接.

现在需要一份带有符号信息和调试信息的`kernel.bin`. 修改Makefile, `ld`链接时去掉`-s`参数，`gcc`则加上`-g3`参数. 把这个重新编译的`kernel.bin`作为gdb的输入文件.启动gdb后输入如下命令连接到虚拟机中的Bochs:

```
target remote <ip>:1234
```

OK! 但是单步时你并不知道执行到源码中的哪条语句了，如果使用的是集成了`peda`的gdb，那么可以通过反汇编进行大致猜测.
