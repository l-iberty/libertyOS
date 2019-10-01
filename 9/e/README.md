# 解决`Init`父子进程的通信问题
抛弃了原来的`write_process_memory`，改用物理内存的直接读写`phymemcpy`. 在消息传递的过程中对`message`结构的读写都需要直接读写物理内存.