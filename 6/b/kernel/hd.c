#include "hd.h"
#include "proc.h"

void _printf(const char* fmt, ...);
void println(char* sz);
u8 in_byte(u16 port);
void out_byte(u16 port, u8 byte);
void port_read(u16 port, void* buf, int len);
void _strcpy(char* dst, const char* src);
u32 _strlen(char* s);
void _memset(void* pDst, u8 value, int len);

int sendrecv(int func_type, int pid, MESSAGE* p_msg); /* system call */

extern int is_current_proc_done;

#define BUFSIZE 512

u16 hd_buf[BUFSIZE];
/**
 * port_read 必须使用 `insw` 以字为单位读取, 缓冲区 hd_buf 必须是 u16 数组.
 * 读写规则使用小端序, 因此对于 ASCII 字符串, 需重新组织字节顺序.
 */

/* Ring1 */
void Task_hd()
{
	is_current_proc_done = 0;
	
	_printf("-----Task_hd-----");
	
	MESSAGE msg;
	int nr_msg_processed = 0;
	
	while (!isEmpty_msg_queue(p_current_proc))
	{
		if (!sendrecv(RECEIVE, 1, &msg))
		{
			if (msg.value == DEV_OPEN)
			{
				init_hd();
				get_hd_info(0); /* 0 = Master, 1 = Slave */
				port_read(REG_DATA, hd_buf, BUFSIZE);
				disp_hd_info();
				
				nr_msg_processed++;		
			}
			else
			{
				_printf("Unknown message");
			}
		}
		else
		{
			_printf("error: sendrecv [pid: %.4x]", p_current_proc->pid);
		}
	}
	_printf("nr_msg_processed: 0x%.2x", nr_msg_processed);
	if (nr_msg_processed > 0)
		while(1) {}
	
	is_current_proc_done = 1;
	while(1) {}
}

void init_hd()
{
	u8* pNrDrives = (u8*) (0x475);
	_printf("NrDrives: %.2x", *pNrDrives);
}

void hd_cmd_out(HD_CMD* cmd)
{
	/* 只有当 Status Register 的 BSY 位为 0, 才能继续 */
	while ((in_byte(REG_STATUS) & 0x80)) {}

	/* 通过 Control Block Register 打开中断 */
	out_byte(REG_DEV_CTRL, 0);
	
	/* 写入命令参数 */
	out_byte(REG_FEATURES, cmd->features);
	out_byte(REG_NRSECTOR, cmd->nr_sector);
	out_byte(REG_LBA_LOW, cmd->lba_low);
	out_byte(REG_LBA_MID, cmd->lba_mid);
	out_byte(REG_LBA_HIGH, cmd->lba_high);
	out_byte(REG_DEVICE, cmd->device);
	/* 发送命令 */
	out_byte(REG_CMD, cmd->command);
}

void get_hd_info(int drive)
{
	HD_CMD cmd;
	_memset(&cmd, 0, sizeof(cmd));
	cmd.device = MAKE_DEVICE_REG(0, drive, 0);
	cmd.command = ATA_IDENTIFY;
	hd_cmd_out(&cmd);
}

void disp_hd_info()
{
	int i, j;
	char sz[64];
	struct hd_info {
		int offset;
		int len;
		char *desc;
	};
	
	struct hd_info info[] = {{10, 20, "Serial Number"}, {27, 40, "Model Number"}};
	
	for (i = 0; i < sizeof(info) / sizeof(info[0]); i++)
	{
		char *p;
		p = (char*) (hd_buf + info[i].offset);
		/* 重组字符串 */
		for (j = 0; j < info[i].len / 2; j++)
		{
			sz[j*2+1] = *p++;
			sz[j*2] = *p++;
		}
		sz[j] = 0;
		_printf("%s: %s", info[i].desc, sz);
	}
	
	u16 cap = hd_buf[49];
	_printf("Capabilities: 0x%.4x (LBA supported: %s)",
		cap, (cap & 0x0200) ? "YES" : "NO");

	u16 cmd_set_supported = hd_buf[83];
	_printf("LBA48 supported: %s",
		(cmd_set_supported & 0x0400) ? "YES" : "NO");
}

/* Ring0 */
void hd_handler()
{
	_printf("#hd_handler#");
}

