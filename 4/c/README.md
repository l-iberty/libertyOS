# c. 区分用户进程和任务
将终端任务`Task_tty`放在 ring1, 三个用户进程放在 ring3.

## 步骤
### 1. 修改/添加变量定义(include/sysconst.h)
![sysconst](screenshot/sysconst.png)

### 2. 在所有用到`NR_TASKS`的地方进行修改:
- `include/proc.h`

![proc_table](screenshot/proc_table.png)

用户进程和任务还是统一放到`proc_table`里.

- `kernel/main.c` 初始化进程表时区分用户进程和任务

![kernel_main1](screenshot/kernel_main1.png)
![kernel_main2](screenshot/kernel_main2.png)

- `kernel/clock.c` 进程调度相关

![schedule](screenshot/schedule.png)

### 3. 将`Task_tty()`函数单独放进`kernel/tty.c`
![tty](screenshot/tty.png)

## 运行结果
![c](screenshot/c.png)