#ifndef HD_C
#define HD_C

#include "type.h"

/* Command Block Registers, Primary */
#define	REG_DATA	0x1F0
#define	REG_ERROR	0x1F1
#define	REG_FEATURES	REG_ERROR
#define	REG_NRSECTOR	0x1F2
#define	REG_LBA_LOW	0x1F3
#define	REG_LBA_MID	0x1F4
#define	REG_LBA_HIGH	0x1F5
#define	REG_DEVICE	0x1F6
#define	REG_STATUS	0x1F7
#define	REG_CMD		REG_STATUS

/* Control Block Registers */
#define REG_DEV_CTRL	0x3F6

/* command */
#define	ATA_IDENTIFY	0xEC
#define ATA_READ	0x20
#define ATA_WRITE	0x30

#define	MAKE_DEVICE_REG(lba,drv,lba_highest) (((lba) << 6) | \
					     ((drv) << 4)  | \
					     ((lba_highest) & 0xF) | 0xA0)


#define READ_HD(sector, buf, len)	hd_read(0, sector, buf, len)
#define WRITE_HD(sector, buf, len)	hd_write(0, sector, buf, len)

struct hd_cmd
{
	uint8_t	features;
	uint8_t	nr_sectors;
	uint8_t	lba_low;
	uint8_t	lba_mid;
	uint8_t	lba_high;
	uint8_t	device;
	uint8_t	command;
};

extern struct message hd_msg;

void init_hd();
void hd_open();
void hd_write(int drive, int sector, void* buf, int len);
void hd_read(int drive, int sector, void* buf, int len);
void do_hd_open();
void do_hd_write();
void do_hd_read();

#endif /* HD_H */
