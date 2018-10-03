# 修改与补充
- `init_mm`不再拷贝`NR_PROCS`份页表，子进程的页表由`fork`拷贝

# problem remained
目前的系统可以正常运行，但存在一些隐性BUG:
## `sys_write_process_memory`
- 注释部分的代码应该加上，但加上后会出问题，导致个别进程通信出故障

## `fork`
- 子进程调用`sendrecv`异常