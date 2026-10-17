/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Most of the following information was derived from the document
 * "Information Technology - AT Attachment-3 Interface (ATA-3)",
 * ANSI X3.298-1997.
 */

#ifndef	_ATA_H
#define _ATA_H

#include <libata.h>

/* Register addressing depends on the hardware design; for instance,
 * 8-bit (register) and 16-bit (data) accesses might use different
 * address spaces. This is implemented by the following definitions.
 */

#define ATA_IO_DATA(x)	(CONFIG_SYS_ATA_DATA_OFFSET+((x) * CONFIG_SYS_ATA_STRIDE))
#define ATA_IO_REG(x)	(CONFIG_SYS_ATA_REG_OFFSET +((x) * CONFIG_SYS_ATA_STRIDE))
#define ATA_IO_ALT(x)	(CONFIG_SYS_ATA_ALT_OFFSET +((x) * CONFIG_SYS_ATA_STRIDE))

/*
 * I/O Register Descriptions
 */
#define ATA_DATA_REG	ATA_IO_DATA(0)
#define ATA_ERROR_REG	ATA_IO_REG(1)
#define ATA_SECT_CNT	ATA_IO_REG(2)
#define ATA_SECT_NUM	ATA_IO_REG(3)
#define ATA_CYL_LOW	ATA_IO_REG(4)
#define ATA_CYL_HIGH	ATA_IO_REG(5)
#define ATA_DEV_HD	ATA_IO_REG(6)
#define ATA_COMMAND	ATA_IO_REG(7)
#define ATA_DATA_EVEN	ATA_IO_REG(8)
#define ATA_DATA_ODD	ATA_IO_REG(9)
#define ATA_STATUS	ATA_COMMAND
#define ATA_DEV_CTL	ATA_IO_ALT(6)
#define ATA_LBA_LOW	ATA_SECT_NUM
#define ATA_LBA_MID	ATA_CYL_LOW
#define ATA_LBA_HIGH	ATA_CYL_HIGH
#define ATA_LBA_SEL	ATA_DEV_CTL

/*
 * Status register bits
 */
#define ATA_STAT_BUSY	0x80	/* Device Busy			*/
#define ATA_STAT_READY	0x40	/* Device Ready			*/
#define ATA_STAT_FAULT	0x20	/* Device Fault			*/
#define ATA_STAT_SEEK	0x10	/* Device Seek Complete		*/
#define ATA_STAT_DRQ	0x08	/* Data Request (ready)		*/
#define ATA_STAT_CORR	0x04	/* Corrected Data Error		*/
#define ATA_STAT_INDEX	0x02	/* Vendor specific		*/
#define ATA_STAT_ERR	0x01	/* Error			*/

/*
 * Device / Head Register Bits
 */
#ifndef ATA_DEVICE
#define ATA_DEVICE(x)	((x & 1)<<4)
#endif /* ATA_DEVICE */
#define ATA_LBA		0xE0

/*
 * ATAPI Commands
 */
#define ATAPI_CMD_INQUIRY 0x12
#define ATAPI_CMD_REQ_SENSE 0x03
#define ATAPI_CMD_READ_CAP 0x25
#define ATAPI_CMD_START_STOP 0x1B
#define ATAPI_CMD_READ_12 0xA8


#define ATA_GET_ERR()	inb(ATA_STATUS)
#define ATA_GET_STAT()	inb(ATA_STATUS)
#define ATA_OK_STAT(stat,good,bad)	(((stat)&((good)|(bad)))==(good))
#define ATA_BAD_R_STAT	(ATA_STAT_BUSY	| ATA_STAT_ERR)
#define ATA_BAD_W_STAT	(ATA_BAD_R_STAT	| ATA_STAT_FAULT)
#define ATA_BAD_STAT	(ATA_BAD_R_STAT	| ATA_STAT_DRQ)
#define ATA_DRIVE_READY	(ATA_READY_STAT	| ATA_STAT_SEEK)
#define ATA_DATA_READY	(ATA_STAT_DRQ)

#define ATA_BLOCKSIZE	512	/* bytes */
#define ATA_BLOCKSHIFT	9	/* 2 ^ ATA_BLOCKSIZESHIFT = 512 */
#define ATA_SECTORWORDS	(512 / sizeof(uint32_t))

#ifndef ATA_RESET_TIME
#define ATA_RESET_TIME	60	/* spec allows up to 31 seconds */
#endif

/* ------------------------------------------------------------------------- */

/*
 * structure returned by ATA_CMD_IDENT, as per ANSI ATA2 rev.2f spec
 */
typedef struct hd_driveid {
	unsigned short	config;		/* lots of obsolete bit flags */
	unsigned short	cyls;		/* "physical" cyls */
	unsigned short	reserved2;	/* reserved (word 2) */
	unsigned short	heads;		/* "physical" heads */
	unsigned short	track_bytes;	/* unformatted bytes per track */
	unsigned short	sector_bytes;	/* unformatted bytes per sector */
	unsigned short	sectors;	/* "physical" sectors per track */
	unsigned short	vendor0;	/* vendor unique */
	unsigned short	vendor1;	/* vendor unique */
	unsigned short	vendor2;	/* vendor unique */
	unsigned char	serial_no[20];	/* 0 = not_specified */
	unsigned short	buf_type;
	unsigned short	buf_size;	/* 512 byte increments; 0 = not_specified */
	unsigned short	ecc_bytes;	/* for r/w long cmds; 0 = not_specified */
	unsigned char	fw_rev[8];	/* 0 = not_specified */
	unsigned char	model[40];	/* 0 = not_specified */
	unsigned char	max_multsect;	/* 0=not_implemented */
	unsigned char	vendor3;	/* vendor unique */
	unsigned short	dword_io;	/* 0=not_implemented; 1=implemented */
	unsigned char	vendor4;	/* vendor unique */
	unsigned char	capability;	/* bits 0:DMA 1:LBA 2:IORDYsw 3:IORDYsup*/
	unsigned short	reserved50;	/* reserved (word 50) */
	unsigned char	vendor5;	/* vendor unique */
	unsigned char	tPIO;		/* 0=slow, 1=medium, 2=fast */
	unsigned char	vendor6;	/* vendor unique */
	unsigned char	tDMA;		/* 0=slow, 1=medium, 2=fast */
	unsigned short	field_valid;	/* bits 0:cur_ok 1:eide_ok */
	unsigned short	cur_cyls;	/* logical cylinders */
	unsigned short	cur_heads;	/* logical heads */
	unsigned short	cur_sectors;	/* logical sectors per track */
	unsigned short	cur_capacity0;	/* logical total sectors on drive */
	unsigned short	cur_capacity1;	/*  (2 words, misaligned int)     */
	unsigned char	multsect;	/* current multiple sector count */
	unsigned char	multsect_valid;	/* when (bit0==1) multsect is ok */
	unsigned short  lba_capacity[2];/* two words containing total number of sectors */
	unsigned short	dma_1word;	/* single-word dma info */
	unsigned short	dma_mword;	/* multiple-word dma info */
	unsigned short  eide_pio_modes; /* bits 0:mode3 1:mode4 */
	unsigned short  eide_dma_min;	/* min mword dma cycle time (ns) */
	unsigned short  eide_dma_time;	/* recommended mword dma cycle time (ns) */
	unsigned short  eide_pio;       /* min cycle time (ns), no IORDY  */
	unsigned short  eide_pio_iordy; /* min cycle time (ns), with IORDY */
	unsigned short	words69_70[2];	/* reserved words 69-70 */
	unsigned short	words71_74[4];	/* reserved words 71-74 */
	unsigned short  queue_depth;	/*  */
	unsigned short  words76_79[4];	/* reserved words 76-79 */
	unsigned short  major_rev_num;	/*  */
	unsigned short  minor_rev_num;	/*  */
	unsigned short  command_set_1;	/* bits 0:Smart 1:Security 2:Removable 3:PM */
	unsigned short	command_set_2;	/* bits 14:Smart Enabled 13:0 zero 10:lba48 support*/
	unsigned short  cfsse;		/* command set-feature supported extensions */
	unsigned short  cfs_enable_1;	/* command set-feature enabled */
	unsigned short  cfs_enable_2;	/* command set-feature enabled */
	unsigned short  csf_default;	/* command set-feature default */
	unsigned short  dma_ultra;	/*  */
	unsigned short	word89;		/* reserved (word 89) */
	unsigned short	word90;		/* reserved (word 90) */
	unsigned short	CurAPMvalues;	/* current APM values */
	unsigned short	word92;		/* reserved (word 92) */
	unsigned short	hw_config;	/* hardware config */
	unsigned short	words94_99[6];/* reserved words 94-99 */
	/*unsigned long long  lba48_capacity; /--* 4 16bit values containing lba 48 total number of sectors */
	unsigned short	lba48_capacity[4]; /* 4 16bit values containing lba 48 total number of sectors */
	unsigned short	words104_125[22];/* reserved words 104-125 */
	unsigned short	last_lun;	/* reserved (word 126) */
	unsigned short	word127;	/* reserved (word 127) */
	unsigned short	dlf;		/* device lock function
					 * 15:9	reserved
					 * 8	security level 1:max 0:high
					 * 7:6	reserved
					 * 5	enhanced erase
					 * 4	expire
					 * 3	frozen
					 * 2	locked
					 * 1	en/disabled
					 * 0	capability
					 */
	unsigned short  csfo;		/* current set features options
					 * 15:4	reserved
					 * 3	auto reassign
					 * 2	reverting
					 * 1	read-look-ahead
					 * 0	write cache
					 */
	unsigned short	words130_155[26];/* reserved vendor words 130-155 */
	unsigned short	word156;
	unsigned short	words157_159[3];/* reserved vendor words 157-159 */
	unsigned short	words160_162[3];/* reserved words 160-162 */
	unsigned short	cf_advanced_caps;
	unsigned short	words164_255[92];/* reserved words 164-255 */
} hd_driveid_t;


/*
 * PIO Mode Configuration
 *
 * See ATA-3 (AT Attachment-3 Interface) documentation, Figure 14 / Table 21
 */

typedef struct {
	unsigned int	t_setup;	/* Setup  Time in [ns] or clocks	*/
	unsigned int	t_length;	/* Length Time in [ns] or clocks	*/
	unsigned int	t_hold;		/* Hold   Time in [ns] or clocks	*/
}
pio_config_t;

#define	IDE_MAX_PIO_MODE	4	/* max suppurted PIO mode		*/

/* ------------------------------------------------------------------------- */

#endif /* _ATA_H */
