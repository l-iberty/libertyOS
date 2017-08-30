#include "hd.h"
#include "proc.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "global.h"

#define SECTOR_SIZE 512

u16 hd_buf[SECTOR_SIZE];
/**
 * port_read 必须使用 `insw` 以字为单位读取, 缓冲区 hd_buf 必须是 u16 数组,
 * 或长度为 SECTOR_SIZE*2 的 u8 数组.
 * 读写规则使用小端序, 因此对于 ASCII 字符串, 需重新组织字节顺序.
 */

/* Ring1 */
void Task_hd()
{
	
	_printf("\n-----Task_hd-----");
	
	MESSAGE msg;
	
	init_hd();
	
	/* 主循环 */
	for (;;)
	{
		sendrecv(RECEIVE, PID_TASK_FS, &msg);
		switch (msg.value)
		{
			case DEV_OPEN:
			{
				get_hd_info(0);
				port_read(REG_DATA, hd_buf, SECTOR_SIZE);
				disp_hd_info();
				break;
			}
			default:
			{
				_printf("{unknown message value}");
				dump_msg(&msg);
			}
		}
		//sendrecv(SEND, msg.src_pid, &msg); /* 制造死锁 */
		sendrecv(SEND, PID_TASK_A, &msg); /* 制造死锁 */
	}
}

void init_hd()
{
	u8* pNrDrives = (u8*) (0x475);
	_printf("\nNrDrives: %.2x", *pNrDrives);
	assert(*pNrDrives);
}

void hd_cmd_out(HD_CMD* cmd)
{
	/* 只有当 Status Register 的 BSY 位为 0, 才能继续 */
	while ((in_byte(REG_STATUS) & 0x80)) {}

	/* 通过 Control Block Register 打开中断 */
	out_byte(REG_DEV_CTRL, 0);
	
	/* 写入命令参数 */
	out_byte(REG_FEATURES, cmd->features);
	out_byte(REG_NRSECTOR, cmd->nr_sectors);
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
		_printf("\n%s: %s", info[i].desc, sz);
	}
	
	u16 cap = hd_buf[49];
	_printf("\nCapabilities: 0x%.4x (LBA supported: %s)",
		cap, (cap & 0x0200) ? "YES" : "NO");

	u16 cmd_set_supported = hd_buf[83];
	_printf("\nLBA48 supported: %s",
		(cmd_set_supported & 0x0400) ? "YES" : "NO");
}

/**
 * @param	drive	0 = Master, 1 = Slave
 * @param	sector	扇区号
 * @param	buf	缓冲区
 * @param	len	字节数
 */
void hd_write(int drive, int sector, void* buf, int len)
{
	HD_CMD cmd;
	cmd.features	= 0;
	cmd.nr_sectors	= (len + SECTOR_SIZE - 1) / SECTOR_SIZE;
	cmd.lba_low	= sector & 0xFF;
	cmd.lba_mid	= (sector >> 8) & 0xFF;
	cmd.lba_high	= (sector >> 16) & 0xFF;
	cmd.device	= MAKE_DEVICE_REG(1, 0, (sector >> 24));
	cmd.command	= ATA_WRITE;
	
	hd_cmd_out(&cmd);
	
	port_write(REG_DATA, buf, len >> 1);
}

/* Ring0 */
void hd_handler()
{
	//_printf("#hd_handler#");
}

