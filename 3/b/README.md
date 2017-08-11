# b. 第二步：丰富时钟中断例程
* 之前在中断例程中没有考虑到现场的保存与恢复，现在添加进去：

![clockhandler](screenshot/clockhandler.png)

另外，还加入了打印字符串的代码，使其呈现如下效果:

![b](screenshot/b.png)

可以在`kernel/main.c`里修改`delay(...)`的参数来控制打印"A."的时间间隔.
