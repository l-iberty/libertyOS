# 遗留的问题
目前的系统可以正常运行，但存在一些隐性BUG:

# `sys_write_process_memory`
- 注释部分的代码应该加上，但加上后会出问题，导致个别进程通信出故障

# `fork`
- 子进程调用`sendrecv`异常