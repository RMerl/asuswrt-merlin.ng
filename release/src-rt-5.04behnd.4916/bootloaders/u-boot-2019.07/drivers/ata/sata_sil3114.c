// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) Excito Elektronik i Skåne AB, All rights reserved.
 * Author: Tor Krill <tor@excito.com>
 *
 * This is a driver for Silicon Image sil3114 sata chip modelled on
 * the ata_piix driver
 */

#include <common.h>
#include <pci.h>
#include <command.h>
#include <config.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <ide.h>
#include <sata.h>
#include <libata.h>
#include "sata_sil3114.h"

/* Convert sectorsize to wordsize */
#define ATA_SECTOR_WORDS (ATA_SECT_SIZE/2)

/* Forwards */
u8 sil3114_spin_up (int num);
u8 sil3114_spin_down (int num);
static int sata_bus_softreset (int num);
static void sata_identify (int num, int dev);
static u8 check_power_mode (int num);
static void sata_port (struct sata_ioports *ioport);
static void set_Feature_cmd (int num, int dev);
static u8 sata_busy_wait (struct sata_ioports *ioaddr, int bits,
			  unsigned int max, u8 usealtstatus);
static u8 sata_chk_status (struct sata_ioports *ioaddr, u8 usealtstatus);
static void msleep (int count);

static u32 iobase[6] = { 0, 0, 0, 0, 0, 0};	/* PCI BAR registers for device */

static struct sata_port port[CONFIG_SYS_SATA_MAX_DEVICE];

static void output_data (struct sata_ioports *ioaddr, u16 * sect_buf, int words)
{
	while (words--) {
		__raw_writew (*sect_buf++, (void *)ioaddr->data_addr);
	}
}

static int input_data (struct sata_ioports *ioaddr, u16 * sect_buf, int words)
{
	while (words--) {
		*sect_buf++ = __raw_readw ((void *)ioaddr->data_addr);
	}
	return 0;
}

static int sata_bus_softreset (int num)
{
	u8 status = 0;

	port[num].dev_mask = 1;

	port[num].ctl_reg = 0x08;	/*Default value of control reg */
	writeb (port[num].ctl_reg, port[num].ioaddr.ctl_addr);
	udelay (10);
	writeb (port[num].ctl_reg | ATA_SRST, port[num].ioaddr.ctl_addr);
	udelay (10);
	writeb (port[num].ctl_reg, port[num].ioaddr.ctl_addr);

	/* spec mandates ">= 2ms" before checking status.
	 * We wait 150ms, because that was the magic delay used for
	 * ATAPI devices in Hale Landis's ATADRVR, for the period of time
	 * between when the ATA command register is written, and then
	 * status is checked.  Because waiting for "a while" before
	 * checking status is fine, post SRST, we perform this magic
	 * delay here as well.
	 */
	msleep (150);
	status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 300, 0);
	while ((status & ATA_BUSY)) {
		msleep (100);
		status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 3, 0);
	}

	if (status & ATA_BUSY) {
		printf ("ata%u is slow to respond,plz be patient\n", num);
	}

	while ((status & ATA_BUSY)) {
		msleep (100);
		status = sata_chk_status (&port[num].ioaddr, 0);
	}

	if (status & ATA_BUSY) {
		printf ("ata%u failed to respond : ", num);
		printf ("bus reset failed\n");
		port[num].dev_mask = 0;
		return 1;
	}
	return 0;
}

static void sata_identify (int num, int dev)
{
	u8 cmd = 0, status = 0, devno = num;
	u16 iobuf[ATA_SECTOR_WORDS];
	u64 n_sectors = 0;

	memset (iobuf, 0, sizeof (iobuf));

	if (!(port[num].dev_mask & 0x01)) {
		printf ("dev%d is not present on port#%d\n", dev, num);
		return;
	}

	debug ("port=%d dev=%d\n", num, dev);

	status = 0;
	cmd = ATA_CMD_ID_ATA;	/*Device Identify Command */
	writeb (cmd, port[num].ioaddr.command_addr);
	readb (port[num].ioaddr.altstatus_addr);
	udelay (10);

	status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 1000, 0);
	if (status & ATA_ERR) {
		printf ("\ndevice not responding\n");
		port[num].dev_mask &= ~0x01;
		return;
	}

	input_data (&port[num].ioaddr, iobuf, ATA_SECTOR_WORDS);

	ata_swap_buf_le16 (iobuf, ATA_SECTOR_WORDS);

	debug ("Specific config: %x\n", iobuf[2]);

	/* we require LBA and DMA support (bits 8 & 9 of word 49) */
	if (!ata_id_has_dma (iobuf) || !ata_id_has_lba (iobuf)) {
		debug ("ata%u: no dma/lba\n", num);
	}
#ifdef DEBUG
	ata_dump_id (iobuf);
#endif
	n_sectors = ata_id_n_sectors (iobuf);

	if (n_sectors == 0) {
		port[num].dev_mask &= ~0x01;
		return;
	}
	ata_id_c_string (iobuf, (unsigned char *)sata_dev_desc[devno].revision,
			 ATA_ID_FW_REV, sizeof (sata_dev_desc[devno].revision));
	ata_id_c_string (iobuf, (unsigned char *)sata_dev_desc[devno].vendor,
			 ATA_ID_PROD, sizeof (sata_dev_desc[devno].vendor));
	ata_id_c_string (iobuf, (unsigned char *)sata_dev_desc[devno].product,
			 ATA_ID_SERNO, sizeof (sata_dev_desc[devno].product));

	/* TODO - atm we asume harddisk ie not removable */
	sata_dev_desc[devno].removable = 0;

	sata_dev_desc[devno].lba = (u32) n_sectors;
	debug("lba=0x%lx\n", sata_dev_desc[devno].lba);

#ifdef CONFIG_LBA48
	if (iobuf[83] & (1 << 10)) {
		sata_dev_desc[devno].lba48 = 1;
	} else {
		sata_dev_desc[devno].lba48 = 0;
	}
#endif

	/* assuming HD */
	sata_dev_desc[devno].type = DEV_TYPE_HARDDISK;
	sata_dev_desc[devno].blksz = ATA_SECT_SIZE;
	sata_dev_desc[devno].lun = 0;	/* just to fill something in... */
}

static void set_Feature_cmd (int num, int dev)
{
	u8 status = 0;

	if (!(port[num].dev_mask & 0x01)) {
		debug ("dev%d is not present on port#%d\n", dev, num);
		return;
	}

	writeb (SETFEATURES_XFER, port[num].ioaddr.feature_addr);
	writeb (XFER_PIO_4, port[num].ioaddr.nsect_addr);
	writeb (0, port[num].ioaddr.lbal_addr);
	writeb (0, port[num].ioaddr.lbam_addr);
	writeb (0, port[num].ioaddr.lbah_addr);

	writeb (ATA_DEVICE_OBS, port[num].ioaddr.device_addr);
	writeb (ATA_CMD_SET_FEATURES, port[num].ioaddr.command_addr);

	udelay (50);
	msleep (150);

	status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 5000, 0);
	if ((status & (ATA_BUSY | ATA_ERR))) {
		printf ("Error  : status 0x%02x\n", status);
		port[num].dev_mask &= ~0x01;
	}
}

u8 sil3114_spin_down (int num)
{
	u8 status = 0;

	debug ("Spin down disk\n");

	if (!(port[num].dev_mask & 0x01)) {
		debug ("Device ata%d is not present\n", num);
		return 1;
	}

	if ((status = check_power_mode (num)) == 0x00) {
		debug ("Already in standby\n");
		return 0;
	}

	if (status == 0x01) {
		printf ("Failed to check power mode on ata%d\n", num);
		return 1;
	}

	if (!((status = sata_chk_status (&port[num].ioaddr, 0)) & ATA_DRDY)) {
		printf ("Device ata%d not ready\n", num);
		return 1;
	}

	writeb (0x00, port[num].ioaddr.feature_addr);

	writeb (0x00, port[num].ioaddr.nsect_addr);
	writeb (0x00, port[num].ioaddr.lbal_addr);
	writeb (0x00, port[num].ioaddr.lbam_addr);
	writeb (0x00, port[num].ioaddr.lbah_addr);

	writeb (ATA_DEVICE_OBS, port[num].ioaddr.device_addr);
	writeb (ATA_CMD_STANDBY, port[num].ioaddr.command_addr);

	status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 30000, 0);
	if ((status & (ATA_BUSY | ATA_ERR))) {
		printf ("Error waiting for disk spin down: status 0x%02x\n",
			status);
		port[num].dev_mask &= ~0x01;
		return 1;
	}
	return 0;
}

u8 sil3114_spin_up (int num)
{
	u8 status = 0;

	debug ("Spin up disk\n");

	if (!(port[num].dev_mask & 0x01)) {
		debug ("Device ata%d is not present\n", num);
		return 1;
	}

	if ((status = check_power_mode (num)) != 0x00) {
		if (status == 0x01) {
			printf ("Failed to check power mode on ata%d\n", num);
			return 1;
		} else {
			/* should be up and running already */
			return 0;
		}
	}

	if (!((status = sata_chk_status (&port[num].ioaddr, 0)) & ATA_DRDY)) {
		printf ("Device ata%d not ready\n", num);
		return 1;
	}

	debug ("Stautus of device check: %d\n", status);

	writeb (0x00, port[num].ioaddr.feature_addr);

	writeb (0x00, port[num].ioaddr.nsect_addr);
	writeb (0x00, port[num].ioaddr.lbal_addr);
	writeb (0x00, port[num].ioaddr.lbam_addr);
	writeb (0x00, port[num].ioaddr.lbah_addr);

	writeb (ATA_DEVICE_OBS, port[num].ioaddr.device_addr);
	writeb (ATA_CMD_IDLE, port[num].ioaddr.command_addr);

	status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 30000, 0);
	if ((status & (ATA_BUSY | ATA_ERR))) {
		printf ("Error waiting for disk spin up: status 0x%02x\n",
			status);
		port[num].dev_mask &= ~0x01;
		return 1;
	}

	/* Wait for disk to enter Active state */
	do {
		msleep (10);
		status = check_power_mode (num);
	} while ((status == 0x00) || (status == 0x80));

	if (status == 0x01) {
		printf ("Falied waiting for disk to spin up\n");
		return 1;
	}

	return 0;
}

/* Return value is not the usual here
 * 0x00 - Device stand by
 * 0x01 - Operation failed
 * 0x80 - Device idle
 * 0xff - Device active
*/
static u8 check_power_mode (int num)
{
	u8 status = 0;
	u8 res = 0;
	if (!(port[num].dev_mask & 0x01)) {
		debug ("Device ata%d is not present\n", num);
		return 1;
	}

	if (!(sata_chk_status (&port[num].ioaddr, 0) & ATA_DRDY)) {
		printf ("Device ata%d not ready\n", num);
		return 1;
	}

	writeb (0, port[num].ioaddr.feature_addr);
	writeb (0, port[num].ioaddr.nsect_addr);
	writeb (0, port[num].ioaddr.lbal_addr);
	writeb (0, port[num].ioaddr.lbam_addr);
	writeb (0, port[num].ioaddr.lbah_addr);

	writeb (ATA_DEVICE_OBS, port[num].ioaddr.device_addr);
	writeb (ATA_CMD_CHK_POWER, port[num].ioaddr.command_addr);

	status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 5000, 0);
	if ((status & (ATA_BUSY | ATA_ERR))) {
		printf
		    ("Error waiting for check power mode complete  : status 0x%02x\n",
		     status);
		port[num].dev_mask &= ~0x01;
		return 1;
	}
	res = readb (port[num].ioaddr.nsect_addr);
	debug ("Check powermode: %d\n", res);
	return res;

}

static void sata_port (struct sata_ioports *ioport)
{
	ioport->data_addr = ioport->cmd_addr + ATA_REG_DATA;
	ioport->error_addr = ioport->cmd_addr + ATA_REG_ERR;
	ioport->feature_addr = ioport->cmd_addr + ATA_REG_FEATURE;
	ioport->nsect_addr = ioport->cmd_addr + ATA_REG_NSECT;
	ioport->lbal_addr = ioport->cmd_addr + ATA_REG_LBAL;
	ioport->lbam_addr = ioport->cmd_addr + ATA_REG_LBAM;
	ioport->lbah_addr = ioport->cmd_addr + ATA_REG_LBAH;
	ioport->device_addr = ioport->cmd_addr + ATA_REG_DEVICE;
	ioport->status_addr = ioport->cmd_addr + ATA_REG_STATUS;
	ioport->command_addr = ioport->cmd_addr + ATA_REG_CMD;
}

static u8 wait_for_irq (int num, unsigned int max)
{

	u32 port = iobase[5];
	switch (num) {
	case 0:
		port += VND_TF_CNST_CH0;
		break;
	case 1:
		port += VND_TF_CNST_CH1;
		break;
	case 2:
		port += VND_TF_CNST_CH2;
		break;
	case 3:
		port += VND_TF_CNST_CH3;
		break;
	default:
		return 1;
	}

	do {
		if (readl (port) & VND_TF_CNST_INTST) {
			break;
		}
		udelay (1000);
		max--;
	} while ((max > 0));

	return (max == 0);
}

static u8 sata_busy_wait (struct sata_ioports *ioaddr, int bits,
			  unsigned int max, u8 usealtstatus)
{
	u8 status;

	do {
		if (!((status = sata_chk_status (ioaddr, usealtstatus)) & bits)) {
			break;
		}
		udelay (1000);
		max--;
	} while ((status & bits) && (max > 0));

	return status;
}

static u8 sata_chk_status (struct sata_ioports *ioaddr, u8 usealtstatus)
{
	if (!usealtstatus) {
		return readb (ioaddr->status_addr);
	} else {
		return readb (ioaddr->altstatus_addr);
	}
}

static void msleep (int count)
{
	int i;

	for (i = 0; i < count; i++)
		udelay (1000);
}

/* Read up to 255 sectors
 *
 * Returns sectors read
*/
static u8 do_one_read (int device, ulong block, u8 blkcnt, u16 * buff,
		       uchar lba48)
{

	u8 sr = 0;
	u8 status;
	u64 blknr = (u64) block;

	if (!(sata_chk_status (&port[device].ioaddr, 0) & ATA_DRDY)) {
		printf ("Device ata%d not ready\n", device);
		return 0;
	}

	/* Set up transfer */
#ifdef CONFIG_LBA48
	if (lba48) {
		/* write high bits */
		writeb (0, port[device].ioaddr.nsect_addr);
		writeb ((blknr >> 24) & 0xFF, port[device].ioaddr.lbal_addr);
		writeb ((blknr >> 32) & 0xFF, port[device].ioaddr.lbam_addr);
		writeb ((blknr >> 40) & 0xFF, port[device].ioaddr.lbah_addr);
	}
#endif
	writeb (blkcnt, port[device].ioaddr.nsect_addr);
	writeb (((blknr) >> 0) & 0xFF, port[device].ioaddr.lbal_addr);
	writeb ((blknr >> 8) & 0xFF, port[device].ioaddr.lbam_addr);
	writeb ((blknr >> 16) & 0xFF, port[device].ioaddr.lbah_addr);

#ifdef CONFIG_LBA48
	if (lba48) {
		writeb (ATA_LBA, port[device].ioaddr.device_addr);
		writeb (ATA_CMD_PIO_READ_EXT, port[device].ioaddr.command_addr);
	} else
#endif
	{
		writeb (ATA_LBA | ((blknr >> 24) & 0xF),
			port[device].ioaddr.device_addr);
		writeb (ATA_CMD_PIO_READ, port[device].ioaddr.command_addr);
	}

	status = sata_busy_wait (&port[device].ioaddr, ATA_BUSY, 10000, 1);

	if (status & ATA_BUSY) {
		u8 err = 0;

		printf ("Device %d not responding status %d\n", device, status);
		err = readb (port[device].ioaddr.error_addr);
		printf ("Error reg = 0x%x\n", err);

		return (sr);
	}
	while (blkcnt--) {

		if (wait_for_irq (device, 500)) {
			printf ("ata%u irq failed\n", device);
			return sr;
		}

		status = sata_chk_status (&port[device].ioaddr, 0);
		if (status & ATA_ERR) {
			printf ("ata%u error %d\n", device,
				readb (port[device].ioaddr.error_addr));
			return sr;
		}
		/* Read one sector */
		input_data (&port[device].ioaddr, buff, ATA_SECTOR_WORDS);
		buff += ATA_SECTOR_WORDS;
		sr++;

	}
	return sr;
}

ulong sata_read (int device, ulong block, lbaint_t blkcnt, void *buff)
{
	ulong n = 0, sread;
	u16 *buffer = (u16 *) buff;
	u8 status = 0;
	u64 blknr = (u64) block;
	unsigned char lba48 = 0;

#ifdef CONFIG_LBA48
	if (blknr > 0xfffffff) {
		if (!sata_dev_desc[device].lba48) {
			printf ("Drive doesn't support 48-bit addressing\n");
			return 0;
		}
		/* more than 28 bits used, use 48bit mode */
		lba48 = 1;
	}
#endif

	while (blkcnt > 0) {

		if (blkcnt > 255) {
			sread = 255;
		} else {
			sread = blkcnt;
		}

		status = do_one_read (device, blknr, sread, buffer, lba48);
		if (status != sread) {
			printf ("Read failed\n");
			return n;
		}

		blkcnt -= sread;
		blknr += sread;
		n += sread;
		buffer += sread * ATA_SECTOR_WORDS;
	}
	return n;
}

ulong sata_write (int device, ulong block, lbaint_t blkcnt, const void *buff)
{
	ulong n = 0;
	u16 *buffer = (u16 *) buff;
	unsigned char status = 0, num = 0;
	u64 blknr = (u64) block;
#ifdef CONFIG_LBA48
	unsigned char lba48 = 0;

	if (blknr > 0xfffffff) {
		if (!sata_dev_desc[device].lba48) {
			printf ("Drive doesn't support 48-bit addressing\n");
			return 0;
		}
		/* more than 28 bits used, use 48bit mode */
		lba48 = 1;
	}
#endif
	/*Port Number */
	num = device;

	while (blkcnt-- > 0) {
		status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 500, 0);
		if (status & ATA_BUSY) {
			printf ("ata%u failed to respond\n", port[num].port_no);
			return n;
		}
#ifdef CONFIG_LBA48
		if (lba48) {
			/* write high bits */
			writeb (0, port[num].ioaddr.nsect_addr);
			writeb ((blknr >> 24) & 0xFF,
				port[num].ioaddr.lbal_addr);
			writeb ((blknr >> 32) & 0xFF,
				port[num].ioaddr.lbam_addr);
			writeb ((blknr >> 40) & 0xFF,
				port[num].ioaddr.lbah_addr);
		}
#endif
		writeb (1, port[num].ioaddr.nsect_addr);
		writeb ((blknr >> 0) & 0xFF, port[num].ioaddr.lbal_addr);
		writeb ((blknr >> 8) & 0xFF, port[num].ioaddr.lbam_addr);
		writeb ((blknr >> 16) & 0xFF, port[num].ioaddr.lbah_addr);
#ifdef CONFIG_LBA48
		if (lba48) {
			writeb (ATA_LBA, port[num].ioaddr.device_addr);
			writeb (ATA_CMD_PIO_WRITE_EXT, port[num].ioaddr.command_addr);
		} else
#endif
		{
			writeb (ATA_LBA | ((blknr >> 24) & 0xF),
				port[num].ioaddr.device_addr);
			writeb (ATA_CMD_PIO_WRITE, port[num].ioaddr.command_addr);
		}

		msleep (50);
		/*may take up to 4 sec */
		status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 4000, 0);
		if ((status & (ATA_DRQ | ATA_BUSY | ATA_ERR)) != ATA_DRQ) {
			printf ("Error no DRQ dev %d blk %ld: sts 0x%02x\n",
				device, (ulong) blknr, status);
			return (n);
		}

		output_data (&port[num].ioaddr, buffer, ATA_SECTOR_WORDS);
		readb (port[num].ioaddr.altstatus_addr);
		udelay (50);

		++n;
		++blknr;
		buffer += ATA_SECTOR_WORDS;
	}
	return n;
}

/* Driver implementation */
static u8 sil_get_device_cache_line (pci_dev_t pdev)
{
	u8 cache_line = 0;
	pci_read_config_byte (pdev, PCI_CACHE_LINE_SIZE, &cache_line);
	return cache_line;
}

int init_sata (int dev)
{
	static u8 init_done = 0;
	static int res = 1;
	pci_dev_t devno;
	u8 cls = 0;
	u16 cmd = 0;
	u32 sconf = 0;

	if (init_done) {
		return res;
	}

	init_done = 1;

	if ((devno = pci_find_device (SIL_VEND_ID, SIL3114_DEVICE_ID, 0)) == -1) {
		res = 1;
		return res;
	}

	/* Read out all BARs, even though we only use MMIO from BAR5 */
	pci_read_config_dword (devno, PCI_BASE_ADDRESS_0, &iobase[0]);
	pci_read_config_dword (devno, PCI_BASE_ADDRESS_1, &iobase[1]);
	pci_read_config_dword (devno, PCI_BASE_ADDRESS_2, &iobase[2]);
	pci_read_config_dword (devno, PCI_BASE_ADDRESS_3, &iobase[3]);
	pci_read_config_dword (devno, PCI_BASE_ADDRESS_4, &iobase[4]);
	pci_read_config_dword (devno, PCI_BASE_ADDRESS_5, &iobase[5]);

	if ((iobase[0] == 0xFFFFFFFF) || (iobase[1] == 0xFFFFFFFF) ||
	    (iobase[2] == 0xFFFFFFFF) || (iobase[3] == 0xFFFFFFFF) ||
	    (iobase[4] == 0xFFFFFFFF) || (iobase[5] == 0xFFFFFFFF)) {
		printf ("Error no base addr for SATA controller\n");
		res = 1;
		return res;
	}

	/* mask off unused bits */
	iobase[0] &= 0xfffffffc;
	iobase[1] &= 0xfffffff8;
	iobase[2] &= 0xfffffffc;
	iobase[3] &= 0xfffffff8;
	iobase[4] &= 0xfffffff0;
	iobase[5] &= 0xfffffc00;

	/* from sata_sil in Linux kernel */
	cls = sil_get_device_cache_line (devno);
	if (cls) {
		cls >>= 3;
		cls++;		/* cls = (line_size/8)+1 */
		writel (cls << 8 | cls, iobase[5] + VND_FIFOCFG_CH0);
		writel (cls << 8 | cls, iobase[5] + VND_FIFOCFG_CH1);
		writel (cls << 8 | cls, iobase[5] + VND_FIFOCFG_CH2);
		writel (cls << 8 | cls, iobase[5] + VND_FIFOCFG_CH3);
	} else {
		printf ("Cache line not set. Driver may not function\n");
	}

	/* Enable operation */
	pci_read_config_word (devno, PCI_COMMAND, &cmd);
	cmd |= PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
	pci_write_config_word (devno, PCI_COMMAND, cmd);

	/* Disable interrupt usage */
	pci_read_config_dword (devno, VND_SYSCONFSTAT, &sconf);
	sconf |= (VND_SYSCONFSTAT_CHN_0_INTBLOCK | VND_SYSCONFSTAT_CHN_1_INTBLOCK);
	pci_write_config_dword (devno, VND_SYSCONFSTAT, sconf);

	res = 0;
	return res;
}

int reset_sata(int dev)
{
	return 0;
}

/* Check if device is connected to port */
int sata_bus_probe (int portno)
{
	u32 port = iobase[5];
	u32 val;
	switch (portno) {
	case 0:
		port += VND_SSTATUS_CH0;
		break;
	case 1:
		port += VND_SSTATUS_CH1;
		break;
	case 2:
		port += VND_SSTATUS_CH2;
		break;
	case 3:
		port += VND_SSTATUS_CH3;
		break;
	default:
		return 0;
	}
	val = readl (port);
	if ((val & SATA_DET_PRES) == SATA_DET_PRES) {
		return 1;
	} else {
		return 0;
	}
}

int sata_phy_reset (int portno)
{
	u32 port = iobase[5];
	u32 val;
	switch (portno) {
	case 0:
		port += VND_SCONTROL_CH0;
		break;
	case 1:
		port += VND_SCONTROL_CH1;
		break;
	case 2:
		port += VND_SCONTROL_CH2;
		break;
	case 3:
		port += VND_SCONTROL_CH3;
		break;
	default:
		return 0;
	}
	val = readl (port);
	writel (val | SATA_SC_DET_RST, port);
	msleep (150);
	writel (val & ~SATA_SC_DET_RST, port);
	return 0;
}

int scan_sata (int dev)
{
	/* A bit brain dead, but the code has a legacy */
	switch (dev) {
	case 0:
		port[0].port_no = 0;
		port[0].ioaddr.cmd_addr = iobase[5] + VND_TF0_CH0;
		port[0].ioaddr.altstatus_addr = port[0].ioaddr.ctl_addr =
		    (iobase[5] + VND_TF2_CH0) | ATA_PCI_CTL_OFS;
		port[0].ioaddr.bmdma_addr = iobase[5] + VND_BMDMA_CH0;
		break;
#if (CONFIG_SYS_SATA_MAX_DEVICE >= 1)
	case 1:
		port[1].port_no = 0;
		port[1].ioaddr.cmd_addr = iobase[5] + VND_TF0_CH1;
		port[1].ioaddr.altstatus_addr = port[1].ioaddr.ctl_addr =
		    (iobase[5] + VND_TF2_CH1) | ATA_PCI_CTL_OFS;
		port[1].ioaddr.bmdma_addr = iobase[5] + VND_BMDMA_CH1;
		break;
#elif (CONFIG_SYS_SATA_MAX_DEVICE >= 2)
	case 2:
		port[2].port_no = 0;
		port[2].ioaddr.cmd_addr = iobase[5] + VND_TF0_CH2;
		port[2].ioaddr.altstatus_addr = port[2].ioaddr.ctl_addr =
		    (iobase[5] + VND_TF2_CH2) | ATA_PCI_CTL_OFS;
		port[2].ioaddr.bmdma_addr = iobase[5] + VND_BMDMA_CH2;
		break;
#elif (CONFIG_SYS_SATA_MAX_DEVICE >= 3)
	case 3:
		port[3].port_no = 0;
		port[3].ioaddr.cmd_addr = iobase[5] + VND_TF0_CH3;
		port[3].ioaddr.altstatus_addr = port[3].ioaddr.ctl_addr =
		    (iobase[5] + VND_TF2_CH3) | ATA_PCI_CTL_OFS;
		port[3].ioaddr.bmdma_addr = iobase[5] + VND_BMDMA_CH3;
		break;
#endif
	default:
		printf ("Tried to scan unknown port: ata%d\n", dev);
		return 1;
	}

	/* Initialize other registers */
	sata_port (&port[dev].ioaddr);

	/* Check for attached device */
	if (!sata_bus_probe (dev)) {
		port[dev].port_state = 0;
		debug ("SATA#%d port is not present\n", dev);
	} else {
		debug ("SATA#%d port is present\n", dev);
		if (sata_bus_softreset (dev)) {
			/* soft reset failed, try a hard one */
			sata_phy_reset (dev);
			if (sata_bus_softreset (dev)) {
				port[dev].port_state = 0;
			} else {
				port[dev].port_state = 1;
			}
		} else {
			port[dev].port_state = 1;
		}
	}
	if (port[dev].port_state == 1) {
		/* Probe device and set xfer mode */
		sata_identify (dev, 0);
		set_Feature_cmd (dev, 0);
	}

	return 0;
}
