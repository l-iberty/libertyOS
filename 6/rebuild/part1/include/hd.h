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

typedef struct {
	u8	features;
	u8	nr_sectors;
	u8	lba_low;
	u8	lba_mid;
	u8	lba_high;
	u8	device;
	u8	command;
} HD_CMD;


#define	MAKE_DEVICE_REG(lba,drv,lba_highest) (((lba) << 6) | \
					     ((drv) << 4)  | \
					     (lba_highest & 0xF) | 0xA0)

void init_hd();
void hd_cmd_out(HD_CMD* cmd);
void get_hd_info(int drive);
void disp_hd_info();
void hd_write(int device, int sector, void* buf, int len);

#endif /* Hd_H */
