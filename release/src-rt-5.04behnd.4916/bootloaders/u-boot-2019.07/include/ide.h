/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef	_IDE_H
#define _IDE_H

#include <blk.h>

#define IDE_BUS(dev)	(dev / (CONFIG_SYS_IDE_MAXDEVICE / CONFIG_SYS_IDE_MAXBUS))

#define	ATA_CURR_BASE(dev)	(CONFIG_SYS_ATA_BASE_ADDR+ide_bus_offset[IDE_BUS(dev)])
extern ulong ide_bus_offset[];

/*
 * Function Prototypes
 */

void ide_init(void);
struct blk_desc;
struct udevice;
#ifdef CONFIG_BLK
ulong ide_read(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
	       void *buffer);
ulong ide_write(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
		const void *buffer);
#else
ulong ide_read(struct blk_desc *block_dev, lbaint_t blknr, lbaint_t blkcnt,
	       void *buffer);
ulong ide_write(struct blk_desc *block_dev, lbaint_t blknr, lbaint_t blkcnt,
		const void *buffer);
#endif

#ifdef CONFIG_IDE_PREINIT
int ide_preinit(void);
#endif

#if defined(CONFIG_OF_IDE_FIXUP)
int ide_device_present(int dev);
#endif

#if defined(CONFIG_IDE_AHB)
unsigned char ide_read_register(int dev, unsigned int port);
void ide_write_register(int dev, unsigned int port, unsigned char val);
void ide_read_data(int dev, ulong *sect_buf, int words);
void ide_write_data(int dev, const ulong *sect_buf, int words);
#endif

/*
 * I/O function overrides
 */
unsigned char ide_inb(int dev, int port);
void ide_outb(int dev, int port, unsigned char val);
void ide_input_swap_data(int dev, ulong *sect_buf, int words);
void ide_input_data(int dev, ulong *sect_buf, int words);
void ide_output_data(int dev, const ulong *sect_buf, int words);
void ide_input_data_shorts(int dev, ushort *sect_buf, int shorts);
void ide_output_data_shorts(int dev, ushort *sect_buf, int shorts);

void ide_led(uchar led, uchar status);

/**
 * board_start_ide() - Start up the board IDE interfac
 *
 * @return 0 if ok
 */
int board_start_ide(void);

#endif /* _IDE_H */
