#include "hd.h"
#include "proc.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "global.h"
#include "irq.h"

struct message hd_msg;

uint16_t hd_buf[SECTOR_SIZE];
/**
 * port_read 必须使用 `insw` 以字为单位读取, 缓冲区 hd_buf 必须是 uint16_t 数组,
 * 或长度为 SECTOR_SIZE*2 的 uint8_t 数组.
 * 读写规则使用小端序, 因此对于 ASCII 字符串, 需重新组织字节顺序.
 */

void TaskHD()
{
	printf("\n-----TaskHD-----");
	
	init_hd();
	
	for (;;)
	{
		sendrecv(RECEIVE, PID_TASK_FS, &hd_msg);
		switch (hd_msg.value)
		{
			case DEV_OPEN:
			{
				do_hd_open();
				break;
			}
			case DEV_READ:
			{
				do_hd_read();
				break;
			}
			case DEV_WRITE:
			{
				do_hd_write();
				break;
			}
			default:
			{
				printf("\nHD-{unknown message value}");
				dump_msg(&hd_msg);
			}
		}
		sendrecv(SEND, hd_msg.source, &hd_msg);
	}
}

void init_hd()
{
	printf("\n{HD}    initializing hard-disk driver...");
	
	uint8_t* pNrDrives = (uint8_t*) (0x475);
	printf("\n{HD}    NrDrives: %.2x", *pNrDrives);
	assert(*pNrDrives);
	
	put_irq_handler(IRQ_AT, hd_handler);
	enable_irq(IRQ_CASCADE);
	enable_irq(IRQ_AT);
	
	printf("\n{HD}    hard-disk driver initializaion done");
}

void hd_cmd_out(struct hd_cmd* cmd)
{
	/* 只有当 Status Register 的 BSY 位为 0, 才能继续 */
	while ((in_byte(REG_STATUS) & 0x80)) {printf("-");}

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
	struct hd_cmd cmd;
	memset(&cmd, 0, sizeof(cmd));
	cmd.device = MAKE_DEVICE_REG(0, drive, 0);
	cmd.command = ATA_IDENTIFY;
	hd_cmd_out(&cmd);
}

void disp_hd_info()
{
	int i, j;
	char sz[64];
	struct hd_info 
	{
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
		printf("\n{HD}    %s: %s", info[i].desc, sz);
	}
	
	uint16_t cap = hd_buf[49];
	printf("\n{HD}    Capabilities: 0x%.4x (LBA supported: %s)",
		cap, (cap & 0x0200) ? "YES" : "NO");

	uint16_t cmd_set_supported = hd_buf[83];
	printf("\n{HD}    LBA48 supported: %s",
		(cmd_set_supported & 0x0400) ? "YES" : "NO");
}


void hd_open()
{
	struct message msg;
	
	msg.value = DEV_OPEN;
	sendrecv(BOTH, PID_TASK_HD, &msg);
}

/**
 * @param	drive	0 = Master, 1 = Slave
 * @param	sector	扇区号
 * @param	buf	缓冲区
 * @param	len	字节数
 */
void hd_read(int drive, int sector, void* buf, int len)
{
	struct message msg;
	
	msg.value	= DEV_READ;
	msg.DRIVE	= drive;
	msg.SECTOR	= sector;
	msg.BUF		= buf;
	msg.LEN		= len;
	
	sendrecv(BOTH, PID_TASK_HD, &msg);
}

/**
 * @param	drive	0 = Master, 1 = Slave
 * @param	sector	扇区号
 * @param	buf	缓冲区
 * @param	len	字节数
 */
void hd_write(int drive, int sector, void* buf, int len)
{
	struct message msg;
	
	msg.value	= DEV_WRITE;
	msg.DRIVE	= drive;
	msg.SECTOR	= sector;
	msg.BUF		= buf;
	msg.LEN		= len;
	
	sendrecv(BOTH, PID_TASK_HD, &msg);
}

/**
 * This is a dummy routine, it only get information
 * of hard disk and display it.
 */
void do_hd_open()
{
	get_hd_info(0);
	interrupt_wait();
	port_read(REG_DATA, hd_buf, SECTOR_SIZE);
	disp_hd_info();
}

void do_hd_read()
{
	struct hd_cmd cmd;
	
	cmd.features	= 0;
	cmd.nr_sectors	= (hd_msg.LEN + SECTOR_SIZE - 1) >> SECTOR_SIZE_SHIFT;
	cmd.lba_low	= hd_msg.SECTOR & 0xFF;
	cmd.lba_mid	= (hd_msg.SECTOR >> 8) & 0xFF;
	cmd.lba_high	= (hd_msg.SECTOR >> 16) & 0xFF;
	cmd.device	= MAKE_DEVICE_REG(1, hd_msg.DRIVE, (hd_msg.SECTOR >> 24) & 0x0F);
	cmd.command	= ATA_READ;
	
	hd_cmd_out(&cmd);
	
	/**
	 * 硬盘把数据准备好后会产生一个硬盘中断, 如果不等待中断到来
	 * 就调用 port_read(), 由于数据尚未准备好, 读取会出错.
	 */
	interrupt_wait();
	
	port_read(REG_DATA, hd_msg.BUF, hd_msg.LEN >> 1);
}

void do_hd_write()
{
	struct hd_cmd cmd;
	
	cmd.features	= 0;
	cmd.nr_sectors	= (hd_msg.LEN + SECTOR_SIZE - 1) >> SECTOR_SIZE_SHIFT;
	cmd.lba_low	= hd_msg.SECTOR & 0xFF;
	cmd.lba_mid	= (hd_msg.SECTOR >> 8) & 0xFF;
	cmd.lba_high	= (hd_msg.SECTOR >> 16) & 0xFF;
	cmd.device	= MAKE_DEVICE_REG(1, hd_msg.DRIVE, (hd_msg.SECTOR >> 24) & 0x0F);
	cmd.command	= ATA_WRITE;
	
	hd_cmd_out(&cmd);
	port_write(REG_DATA, hd_msg.BUF, hd_msg.LEN >> 1);
	
	/* 写硬盘结束后会产生一个硬盘中断, 必须等待之 */
	interrupt_wait();
}

/* Ring0 */
void hd_handler(int irq)
{
	/**
	 * Interrupts are cleared when the host
	 *   - reads the Status Register,
	 *   - issues a reset, or
	 *   - writes to the Command Register.
	 */
	uint8_t status = in_byte(REG_STATUS);
	inform_int(PID_TASK_HD);
}

