# 遗留的问题
目前的系统可以正常运行，但存在一些隐性BUG:

# `sys_write_process_memory`
- 注释部分的代码应该加上，但加上后会出问题，导致个别进程通信出故障

# `fork`
- 拷贝新进程的页表最好在`fork`中完成，但这会出BUG，所以只好将`NR_PROCS`个进程的页表在`init_mm`里全部拷贝好
- 子进程调用`sendrecv`异常