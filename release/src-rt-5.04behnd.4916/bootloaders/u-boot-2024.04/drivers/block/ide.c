// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2011
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#define LOG_CATEGORY UCLASS_IDE

#include <common.h>
#include <ata.h>
#include <blk.h>
#include <bootdev.h>
#include <dm.h>
#include <ide.h>
#include <log.h>
#include <part.h>
#include <watchdog.h>
#include <asm/io.h>
#include <linux/delay.h>

#ifdef __PPC__
# define EIEIO		__asm__ volatile ("eieio")
# define SYNC		__asm__ volatile ("sync")
#else
# define EIEIO		/* nothing */
# define SYNC		/* nothing */
#endif

/* Current offset for IDE0 / IDE1 bus access	*/
ulong ide_bus_offset[CONFIG_SYS_IDE_MAXBUS] = {
#if defined(CONFIG_SYS_ATA_IDE0_OFFSET)
	CONFIG_SYS_ATA_IDE0_OFFSET,
#endif
#if defined(CONFIG_SYS_ATA_IDE1_OFFSET) && (CONFIG_SYS_IDE_MAXBUS > 1)
	CONFIG_SYS_ATA_IDE1_OFFSET,
#endif
};

#define ATA_CURR_BASE(dev)	(CONFIG_SYS_ATA_BASE_ADDR + \
		ide_bus_offset[IDE_BUS(dev)])

#define IDE_TIME_OUT	2000	/* 2 sec timeout */

#define ATAPI_TIME_OUT	7000	/* 7 sec timeout (5 sec seems to work...) */

#define IDE_SPIN_UP_TIME_OUT 5000 /* 5 sec spin-up timeout */

static void ide_reset(void)
{
	if (IS_ENABLED(CONFIG_IDE_RESET)) {
		/* assert reset */
		ide_set_reset(1);

		/* the reset signal shall be asserted for et least 25 us */
		udelay(25);

		schedule();

		/* de-assert RESET signal */
		ide_set_reset(0);

		mdelay(250);
	}
}

static void ide_outb(int dev, int port, u8 val)
{
	log_debug("(dev= %d, port= %#x, val= 0x%02x) : @ 0x%08lx\n",
		  dev, port, val, ATA_CURR_BASE(dev) + port);

	outb(val, ATA_CURR_BASE(dev) + port);
}

static u8 ide_inb(int dev, int port)
{
	uchar val;

	val = inb(ATA_CURR_BASE(dev) + port);

	log_debug("(dev= %d, port= %#x) : @ 0x%08lx -> 0x%02x\n",
		  dev, port, ATA_CURR_BASE(dev) + port, val);
	return val;
}

static void ide_input_swap_data(int dev, ulong *sect_buf, int words)
{
	uintptr_t paddr = (ATA_CURR_BASE(dev) + ATA_DATA_REG);
	ushort *dbuf = (ushort *)sect_buf;

	log_debug("in input swap data base for read is %p\n", (void *)paddr);

	while (words--) {
		EIEIO;
		*dbuf++ = be16_to_cpu(inw(paddr));
		EIEIO;
		*dbuf++ = be16_to_cpu(inw(paddr));
	}
}

/*
 * Wait until Busy bit is off, or timeout (in ms)
 * Return last status
 */
static uchar ide_wait(int dev, ulong t)
{
	ulong delay = 10 * t;	/* poll every 100 us */
	uchar c;

	while ((c = ide_inb(dev, ATA_STATUS)) & ATA_STAT_BUSY) {
		udelay(100);
		if (!delay--)
			break;
	}
	return c;
}

/*
 * copy src to dest, skipping leading and trailing blanks and null
 * terminate the string
 * "len" is the size of available memory including the terminating '\0'
 */
static void ident_cpy(u8 *dst, u8 *src, uint len)
{
	u8 *end, *last;

	last = dst;
	end = src + len - 1;

	/* reserve space for '\0' */
	if (len < 2)
		goto OUT;

	/* skip leading white space */
	while ((*src) && (src < end) && (*src == ' '))
		++src;

	/* copy string, omitting trailing white space */
	while ((*src) && (src < end)) {
		*dst++ = *src;
		if (*src++ != ' ')
			last = dst;
	}
OUT:
	*last = '\0';
}

/****************************************************************************
 * ATAPI Support
 */

/* since ATAPI may use commands with not 4 bytes alligned length
 * we have our own transfer functions, 2 bytes alligned */
static void ide_output_data_shorts(int dev, ushort *sect_buf, int shorts)
{
	uintptr_t paddr = ATA_CURR_BASE(dev) + ATA_DATA_REG;
	ushort *dbuf;

	dbuf = (ushort *)sect_buf;

	log_debug("in output data shorts base for read is %p\n", (void *)paddr);

	while (shorts--) {
		EIEIO;
		outw(cpu_to_le16(*dbuf++), paddr);
	}
}

static void ide_input_data_shorts(int dev, ushort *sect_buf, int shorts)
{
	uintptr_t paddr = ATA_CURR_BASE(dev) + ATA_DATA_REG;
	ushort *dbuf;

	dbuf = (ushort *)sect_buf;

	log_debug("in input data shorts base for read is %p\n", (void *)paddr);

	while (shorts--) {
		EIEIO;
		*dbuf++ = le16_to_cpu(inw(paddr));
	}
}

/*
 * Wait until (Status & mask) == res, or timeout (in ms)
 * Return last status
 * This is used since some ATAPI CD ROMs clears their Busy Bit first
 * and then they set their DRQ Bit
 */
static uchar atapi_wait_mask(int dev, ulong t, uchar mask, uchar res)
{
	ulong delay = 10 * t;	/* poll every 100 us */
	uchar c;

	/* prevents to read the status before valid */
	c = ide_inb(dev, ATA_DEV_CTL);

	while (c = ide_inb(dev, ATA_STATUS) & mask, c != res) {
		/* break if error occurs (doesn't make sense to wait more) */
		if ((c & ATA_STAT_ERR) == ATA_STAT_ERR)
			break;
		udelay(100);
		if (!delay--)
			break;
	}
	return c;
}

/*
 * issue an atapi command
 */
static u8 atapi_issue(int device, u8 *ccb, int ccblen, u8 *buffer, int buflen)
{
	u8 c, err, mask, res;
	int n;

	/* Select device
	 */
	mask = ATA_STAT_BUSY | ATA_STAT_DRQ;
	res = 0;
	ide_outb(device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));
	c = atapi_wait_mask(device, ATAPI_TIME_OUT, mask, res);
	if ((c & mask) != res) {
		printf("ATAPI_ISSUE: device %d not ready status %x\n", device,
		       c);
		err = 0xff;
		goto AI_OUT;
	}
	/* write taskfile */
	ide_outb(device, ATA_ERROR_REG, 0);	/* no DMA, no overlaped */
	ide_outb(device, ATA_SECT_CNT, 0);
	ide_outb(device, ATA_SECT_NUM, 0);
	ide_outb(device, ATA_CYL_LOW, (u8)(buflen & 0xff));
	ide_outb(device, ATA_CYL_HIGH, (u8)((buflen >> 8) & 0xff));
	ide_outb(device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));

	ide_outb(device, ATA_COMMAND, ATA_CMD_PACKET);
	udelay(50);

	mask = ATA_STAT_DRQ | ATA_STAT_BUSY | ATA_STAT_ERR;
	res = ATA_STAT_DRQ;
	c = atapi_wait_mask(device, ATAPI_TIME_OUT, mask, res);

	if ((c & mask) != res) {	/* DRQ must be 1, BSY 0 */
		printf("ATAPI_ISSUE: Error (no IRQ) before sending ccb dev %d status %#02x\n",
		       device, c);
		err = 0xff;
		goto AI_OUT;
	}

	/* write command block */
	ide_output_data_shorts(device, (ushort *)ccb, ccblen / 2);

	/* ATAPI Command written wait for completition */
	mdelay(5);		/* device must set bsy */

	mask = ATA_STAT_DRQ | ATA_STAT_BUSY | ATA_STAT_ERR;
	/*
	 * if no data wait for DRQ = 0 BSY = 0
	 * if data wait for DRQ = 1 BSY = 0
	 */
	res = 0;
	if (buflen)
		res = ATA_STAT_DRQ;
	c = atapi_wait_mask(device, ATAPI_TIME_OUT, mask, res);
	if ((c & mask) != res) {
		if (c & ATA_STAT_ERR) {
			err = (ide_inb(device, ATA_ERROR_REG)) >> 4;
			log_debug("1 returned sense key %x status %02x\n",
				  err, c);
		} else {
			printf("ATAPI_ISSUE: (no DRQ) after sending ccb (%x)  status %#02x\n",
			       ccb[0], c);
			err = 0xff;
		}
		goto AI_OUT;
	}
	n = ide_inb(device, ATA_CYL_HIGH);
	n <<= 8;
	n += ide_inb(device, ATA_CYL_LOW);
	if (n > buflen) {
		printf("ERROR, transfer bytes %d requested only %d\n", n,
		       buflen);
		err = 0xff;
		goto AI_OUT;
	}
	if (!n && buflen < 0) {
		printf("ERROR, transfer bytes %d requested %d\n", n, buflen);
		err = 0xff;
		goto AI_OUT;
	}
	if (n != buflen) {
		log_debug("WARNING, transfer bytes %d not equal with requested %d\n",
			  n, buflen);
	}
	if (n) {		/* data transfer */
		log_debug("ATAPI_ISSUE: %d Bytes to transfer\n", n);
		/* we transfer shorts */
		n >>= 1;
		/* ok now decide if it is an in or output */
		if (!(ide_inb(device, ATA_SECT_CNT) & 0x02)) {
			log_debug("Write to device\n");
			ide_output_data_shorts(device, (ushort *)buffer, n);
		} else {
			log_debug("Read from device @ %p shorts %d\n", buffer,
				  n);
			ide_input_data_shorts(device, (ushort *)buffer, n);
		}
	}
	mdelay(5);		/* seems that some CD ROMs need this... */
	mask = ATA_STAT_BUSY | ATA_STAT_ERR;
	res = 0;
	c = atapi_wait_mask(device, ATAPI_TIME_OUT, mask, res);
	if ((c & ATA_STAT_ERR) == ATA_STAT_ERR) {
		err = (ide_inb(device, ATA_ERROR_REG) >> 4);
		log_debug("2 returned sense key %x status %x\n", err, c);
	} else {
		err = 0;
	}
AI_OUT:
	return err;
}

/*
 * sending the command to atapi_issue. If an status other than good
 * returns, an request_sense will be issued
 */

#define ATAPI_DRIVE_NOT_READY	100
#define ATAPI_UNIT_ATTN		10

static u8 atapi_issue_autoreq(int device, u8 *ccb, int ccblen, u8 *buffer,
			      int buflen)
{
	u8 sense_data[18], sense_ccb[12];
	u8 res, key, asc, ascq;
	int notready, unitattn;

	unitattn = ATAPI_UNIT_ATTN;
	notready = ATAPI_DRIVE_NOT_READY;

retry:
	res = atapi_issue(device, ccb, ccblen, buffer, buflen);
	if (!res)
		return 0;	/* Ok */

	if (res == 0xff)
		return 0xff;	/* error */

	log_debug("(auto_req)atapi_issue returned sense key %x\n", res);

	memset(sense_ccb, 0, sizeof(sense_ccb));
	memset(sense_data, 0, sizeof(sense_data));
	sense_ccb[0] = ATAPI_CMD_REQ_SENSE;
	sense_ccb[4] = 18;	/* allocation Length */

	res = atapi_issue(device, sense_ccb, 12, sense_data, 18);
	key = (sense_data[2] & 0xf);
	asc = (sense_data[12]);
	ascq = (sense_data[13]);

	log_debug("ATAPI_CMD_REQ_SENSE returned %x\n", res);
	log_debug(" Sense page: %02X key %02X ASC %02X ASCQ %02X\n",
		  sense_data[0], key, asc, ascq);

	if (!key)
		return 0;	/* ok device ready */

	if (key == 6 || asc == 0x29 || asc == 0x28) { /* Unit Attention */
		if (unitattn-- > 0) {
			mdelay(200);
			goto retry;
		}
		printf("Unit Attention, tried %d\n", ATAPI_UNIT_ATTN);
		goto error;
	}
	if (asc == 0x4 && ascq == 0x1) {
		/* not ready, but will be ready soon */
		if (notready-- > 0) {
			mdelay(200);
			goto retry;
		}
		printf("Drive not ready, tried %d times\n",
		       ATAPI_DRIVE_NOT_READY);
		goto error;
	}
	if (asc == 0x3a) {
		log_debug("Media not present\n");
		goto error;
	}

	printf("ERROR: Unknown Sense key %02X ASC %02X ASCQ %02X\n", key, asc,
	       ascq);
error:
	log_debug("ERROR Sense key %02X ASC %02X ASCQ %02X\n", key, asc, ascq);
	return 0xff;
}

/*
 * atapi_read:
 * we transfer only one block per command, since the multiple DRQ per
 * command is not yet implemented
 */
#define ATAPI_READ_MAX_BYTES	2048	/* we read max 2kbytes */
#define ATAPI_READ_BLOCK_SIZE	2048	/* assuming CD part */
#define ATAPI_READ_MAX_BLOCK	(ATAPI_READ_MAX_BYTES/ATAPI_READ_BLOCK_SIZE)

static ulong atapi_read(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
			void *buffer)
{
	struct blk_desc *desc = dev_get_uclass_plat(dev);
	int device = desc->devnum;
	ulong n = 0;
	u8 ccb[12];	/* Command descriptor block */
	ulong cnt;

	log_debug("%d start " LBAF " blocks " LBAF " buffer at %lx\n", device,
		  blknr, blkcnt, (ulong)buffer);

	do {
		if (blkcnt > ATAPI_READ_MAX_BLOCK)
			cnt = ATAPI_READ_MAX_BLOCK;
		else
			cnt = blkcnt;

		ccb[0] = ATAPI_CMD_READ_12;
		ccb[1] = 0;	/* reserved */
		ccb[2] = (u8)(blknr >> 24) & 0xff;	/* MSB Block */
		ccb[3] = (u8)(blknr >> 16) & 0xff;	/*  */
		ccb[4] = (u8)(blknr >> 8) & 0xff;
		ccb[5] = (u8)blknr & 0xff;	/* LSB Block */
		ccb[6] = (u8)(cnt >> 24) & 0xff; /* MSB Block cnt */
		ccb[7] = (u8)(cnt >> 16) & 0xff;
		ccb[8] = (u8)(cnt >> 8) & 0xff;
		ccb[9] = (u8)cnt & 0xff;	/* LSB Block */
		ccb[10] = 0;	/* reserved */
		ccb[11] = 0;	/* reserved */

		if (atapi_issue_autoreq(device, ccb, 12,
					(u8 *)buffer,
					cnt * ATAPI_READ_BLOCK_SIZE) == 0xff)
			return n;
		n += cnt;
		blkcnt -= cnt;
		blknr += cnt;
		buffer += cnt * ATAPI_READ_BLOCK_SIZE;
	} while (blkcnt > 0);
	return n;
}

static void atapi_inquiry(struct blk_desc *desc)
{
	u8 ccb[12];	/* Command descriptor block */
	u8 iobuf[64];	/* temp buf */
	u8 c;
	int device;

	device = desc->devnum;
	desc->type = DEV_TYPE_UNKNOWN;	/* not yet valid */

	memset(ccb, 0, sizeof(ccb));
	memset(iobuf, 0, sizeof(iobuf));

	ccb[0] = ATAPI_CMD_INQUIRY;
	ccb[4] = 40;		/* allocation Legnth */
	c = atapi_issue_autoreq(device, ccb, 12, (u8 *)iobuf, 40);

	log_debug("ATAPI_CMD_INQUIRY returned %x\n", c);
	if (c)
		return;

	/* copy device ident strings */
	ident_cpy((u8 *)desc->vendor, &iobuf[8], 8);
	ident_cpy((u8 *)desc->product, &iobuf[16], 16);
	ident_cpy((u8 *)desc->revision, &iobuf[32], 5);

	desc->lun = 0;
	desc->lba = 0;
	desc->blksz = 0;
	desc->log2blksz = LOG2_INVALID(typeof(desc->log2blksz));
	desc->type = iobuf[0] & 0x1f;

	if (iobuf[1] & 0x80)
		desc->removable = 1;
	else
		desc->removable = 0;

	memset(ccb, 0, sizeof(ccb));
	memset(iobuf, 0, sizeof(iobuf));
	ccb[0] = ATAPI_CMD_START_STOP;
	ccb[4] = 0x03;		/* start */

	c = atapi_issue_autoreq(device, ccb, 12, (u8 *)iobuf, 0);

	log_debug("ATAPI_CMD_START_STOP returned %x\n", c);
	if (c)
		return;

	memset(ccb, 0, sizeof(ccb));
	memset(iobuf, 0, sizeof(iobuf));
	c = atapi_issue_autoreq(device, ccb, 12, (u8 *)iobuf, 0);

	log_debug("ATAPI_CMD_UNIT_TEST_READY returned %x\n", c);
	if (c)
		return;

	memset(ccb, 0, sizeof(ccb));
	memset(iobuf, 0, sizeof(iobuf));
	ccb[0] = ATAPI_CMD_READ_CAP;
	c = atapi_issue_autoreq(device, ccb, 12, (u8 *)iobuf, 8);
	log_debug("ATAPI_CMD_READ_CAP returned %x\n", c);
	if (c)
		return;

	log_debug("Read Cap: LBA %02X%02X%02X%02X blksize %02X%02X%02X%02X\n",
		  iobuf[0], iobuf[1], iobuf[2], iobuf[3],
		  iobuf[4], iobuf[5], iobuf[6], iobuf[7]);

	desc->lba = (ulong)iobuf[0] << 24 | (ulong)iobuf[1] << 16 |
		(ulong)iobuf[2] << 8 | (ulong)iobuf[3];
	desc->blksz = (ulong)iobuf[4] << 24 | (ulong)iobuf[5] << 16 |
		(ulong)iobuf[6] << 8 | (ulong)iobuf[7];
	desc->log2blksz = LOG2(desc->blksz);

	/* ATAPI devices cannot use 48bit addressing (ATA/ATAPI v7) */
	desc->lba48 = false;
}

/**
 * ide_ident() - Identify an IDE device
 *
 * @device: Device number to use
 * @desc: Block descriptor to fill in
 * Returns: 0 if OK, -ENOENT if no device is found
 */
static int ide_ident(int device, struct blk_desc *desc)
{
	hd_driveid_t iop;
	bool is_atapi = false;
	int tries = 1;
	u8 c;

	memset(desc, '\0', sizeof(*desc));
	desc->devnum = device;
	desc->type = DEV_TYPE_UNKNOWN;
	desc->uclass_id = UCLASS_IDE;
	desc->log2blksz = LOG2_INVALID(typeof(desc->log2blksz));
	printf("  Device %d: ", device);

	/* Select device
	 */
	ide_outb(device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));
	if (IS_ENABLED(CONFIG_ATAPI))
		tries = 2;

	while (tries) {
		/* check signature */
		if (IS_ENABLED(CONFIG_ATAPI) &&
		    ide_inb(device, ATA_SECT_CNT) == 0x01 &&
		    ide_inb(device, ATA_SECT_NUM) == 0x01 &&
		    ide_inb(device, ATA_CYL_LOW) == 0x14 &&
		    ide_inb(device, ATA_CYL_HIGH) == 0xeb) {
			/* ATAPI Signature found */
			is_atapi = true;
			/*
			 * Start Ident Command
			 */
			ide_outb(device, ATA_COMMAND, ATA_CMD_ID_ATAPI);
			/*
			 * Wait for completion - ATAPI devices need more time
			 * to become ready
			 */
			c = ide_wait(device, ATAPI_TIME_OUT);
		} else {
			/*
			 * Start Ident Command
			 */
			ide_outb(device, ATA_COMMAND, ATA_CMD_ID_ATA);

			/*
			 * Wait for completion
			 */
			c = ide_wait(device, IDE_TIME_OUT);
		}

		if ((c & ATA_STAT_DRQ) &&
		    !(c & (ATA_STAT_FAULT | ATA_STAT_ERR))) {
			break;
		} else if (IS_ENABLED(CONFIG_ATAPI)) {
			/*
			 * Need to soft reset the device
			 * in case it's an ATAPI...
			 */
			log_debug("Retrying...\n");
			ide_outb(device, ATA_DEV_HD,
				 ATA_LBA | ATA_DEVICE(device));
			mdelay(100);
			ide_outb(device, ATA_COMMAND, 0x08);
			mdelay(500);
			/* Select device */
			ide_outb(device, ATA_DEV_HD,
				 ATA_LBA | ATA_DEVICE(device));
		}
		tries--;
	}

	if (!tries)	/* Not found */
		return -ENOENT;

	ide_input_swap_data(device, (ulong *)&iop, ATA_SECTORWORDS);

	ident_cpy((u8 *)desc->revision, iop.fw_rev, sizeof(desc->revision));
	ident_cpy((u8 *)desc->vendor, iop.model, sizeof(desc->vendor));
	ident_cpy((u8 *)desc->product, iop.serial_no, sizeof(desc->product));

	if (iop.config & 0x0080)
		desc->removable = 1;
	else
		desc->removable = 0;

	if (IS_ENABLED(CONFIG_ATAPI) && is_atapi) {
		desc->atapi = true;
		atapi_inquiry(desc);
		return 0;
	}

	iop.lba_capacity[0] = be16_to_cpu(iop.lba_capacity[0]);
	iop.lba_capacity[1] = be16_to_cpu(iop.lba_capacity[1]);
	desc->lba = (ulong)iop.lba_capacity[0] |
		(ulong)iop.lba_capacity[1] << 16;

	if (IS_ENABLED(CONFIG_LBA48) && (iop.command_set_2 & 0x0400)) {
		/* LBA 48 support */
		desc->lba48 = true;
		for (int i = 0; i < 4; i++)
			iop.lba48_capacity[i] = be16_to_cpu(iop.lba48_capacity[i]);
		desc->lba = (unsigned long long)iop.lba48_capacity[0] |
			(unsigned long long)iop.lba48_capacity[1] << 16 |
			(unsigned long long)iop.lba48_capacity[2] << 32 |
			(unsigned long long)iop.lba48_capacity[3] << 48;
	} else {
		desc->lba48 = false;
	}

	/* assuming HD */
	desc->type = DEV_TYPE_HARDDISK;
	desc->blksz = ATA_BLOCKSIZE;
	desc->log2blksz = LOG2(desc->blksz);
	desc->lun = 0;	/* just to fill something in... */

#if 0				/* only used to test the powersaving mode,
				 * if enabled, the drive goes after 5 sec
				 * in standby mode */
	ide_outb(device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));
	c = ide_wait(device, IDE_TIME_OUT);
	ide_outb(device, ATA_SECT_CNT, 1);
	ide_outb(device, ATA_LBA_LOW, 0);
	ide_outb(device, ATA_LBA_MID, 0);
	ide_outb(device, ATA_LBA_HIGH, 0);
	ide_outb(device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));
	ide_outb(device, ATA_COMMAND, 0xe3);
	udelay(50);
	c = ide_wait(device, IDE_TIME_OUT);	/* can't take over 500 ms */
#endif

	return 0;
}

/**
 * ide_init_one() - Init one IDE device
 *
 * @bus: Bus to use
 * Return: 0 iuf OK, -EIO if not available, -ETIMEDOUT if timed out
 */
static int ide_init_one(int bus)
{
	int dev = bus * CONFIG_SYS_IDE_MAXDEVICE / CONFIG_SYS_IDE_MAXBUS;
	int i;
	u8 c;

	printf("Bus %d: ", bus);

	/* Select device */
	mdelay(100);
	ide_outb(dev, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(dev));
	mdelay(100);
	i = 0;
	do {
		mdelay(10);

		c = ide_inb(dev, ATA_STATUS);
		i++;
		if (i > (ATA_RESET_TIME * 100)) {
			puts("** Timeout **\n");
			return -ETIMEDOUT;
		}
		if (i >= 100 && !(i % 100))
			putc('.');
	} while (c & ATA_STAT_BUSY);

	if (c & (ATA_STAT_BUSY | ATA_STAT_FAULT)) {
		puts("not available  ");
		log_debug("Status = %#02X ", c);
		return -EIO;
	} else if (IS_ENABLED(CONFIG_ATAPI) && !(c & ATA_STAT_READY)) {
		/* ATAPI Devices do not set DRDY */
		puts("not available  ");
		log_debug("Status = %#02X ", c);
		return -EIO;
	}
	puts("OK ");

	return 0;
}

static void ide_output_data(int dev, const ulong *sect_buf, int words)
{
	uintptr_t paddr = (ATA_CURR_BASE(dev) + ATA_DATA_REG);
	ushort *dbuf;

	dbuf = (ushort *)sect_buf;
	while (words--) {
		EIEIO;
		outw(cpu_to_le16(*dbuf++), paddr);
		EIEIO;
		outw(cpu_to_le16(*dbuf++), paddr);
	}
}

static void ide_input_data(int dev, ulong *sect_buf, int words)
{
	uintptr_t paddr = (ATA_CURR_BASE(dev) + ATA_DATA_REG);
	ushort *dbuf;

	dbuf = (ushort *)sect_buf;

	log_debug("in input data base for read is %p\n", (void *)paddr);

	while (words--) {
		EIEIO;
		*dbuf++ = le16_to_cpu(inw(paddr));
		EIEIO;
		*dbuf++ = le16_to_cpu(inw(paddr));
	}
}

static ulong ide_read(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
		      void *buffer)
{
	struct blk_desc *desc = dev_get_uclass_plat(dev);
	int device = desc->devnum;
	bool lba48 = false;
	ulong n = 0;
	u8 pwrsave = 0;	/* power save */
	u8 c;

	if (IS_ENABLED(CONFIG_LBA48) && (blknr & 0x0000fffff0000000ULL)) {
		/* more than 28 bits used, use 48bit mode */
		lba48 = true;
	}

	log_debug("dev %d start " LBAF ", blocks " LBAF " buffer at %lx\n",
		  device, blknr, blkcnt, (ulong)buffer);

	/* Select device
	 */
	ide_outb(device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));
	c = ide_wait(device, IDE_TIME_OUT);

	if (c & ATA_STAT_BUSY) {
		printf("IDE read: device %d not ready\n", device);
		goto IDE_READ_E;
	}

	/* first check if the drive is in Powersaving mode, if yes,
	 * increase the timeout value */
	ide_outb(device, ATA_COMMAND, ATA_CMD_CHK_POWER);
	udelay(50);

	c = ide_wait(device, IDE_TIME_OUT);	/* can't take over 500 ms */

	if (c & ATA_STAT_BUSY) {
		printf("IDE read: device %d not ready\n", device);
		goto IDE_READ_E;
	}
	if ((c & ATA_STAT_ERR) == ATA_STAT_ERR) {
		printf("No Powersaving mode %x\n", c);
	} else {
		c = ide_inb(device, ATA_SECT_CNT);
		log_debug("Powersaving %02X\n", c);
		if (!c)
			pwrsave = 1;
	}


	while (blkcnt-- > 0) {
		c = ide_wait(device, IDE_TIME_OUT);

		if (c & ATA_STAT_BUSY) {
			printf("IDE read: device %d not ready\n", device);
			break;
		}
		if (IS_ENABLED(CONFIG_LBA48) && lba48) {
			/* write high bits */
			ide_outb(device, ATA_SECT_CNT, 0);
			ide_outb(device, ATA_LBA_LOW, (blknr >> 24) & 0xff);
#ifdef CONFIG_SYS_64BIT_LBA
			ide_outb(device, ATA_LBA_MID, (blknr >> 32) & 0xff);
			ide_outb(device, ATA_LBA_HIGH, (blknr >> 40) & 0xff);
#else
			ide_outb(device, ATA_LBA_MID, 0);
			ide_outb(device, ATA_LBA_HIGH, 0);
#endif
		}
		ide_outb(device, ATA_SECT_CNT, 1);
		ide_outb(device, ATA_LBA_LOW, (blknr >> 0) & 0xff);
		ide_outb(device, ATA_LBA_MID, (blknr >> 8) & 0xff);
		ide_outb(device, ATA_LBA_HIGH, (blknr >> 16) & 0xff);

		if (IS_ENABLED(CONFIG_LBA48) && lba48) {
			ide_outb(device, ATA_DEV_HD,
				 ATA_LBA | ATA_DEVICE(device));
			ide_outb(device, ATA_COMMAND, ATA_CMD_PIO_READ_EXT);

		} else {
			ide_outb(device, ATA_DEV_HD, ATA_LBA |
				 ATA_DEVICE(device) | ((blknr >> 24) & 0xf));
			ide_outb(device, ATA_COMMAND, ATA_CMD_PIO_READ);
		}

		udelay(50);

		if (pwrsave) {
			/* may take up to 4 sec */
			c = ide_wait(device, IDE_SPIN_UP_TIME_OUT);
			pwrsave = 0;
		} else {
			/* can't take over 500 ms */
			c = ide_wait(device, IDE_TIME_OUT);
		}

		if ((c & (ATA_STAT_DRQ | ATA_STAT_BUSY | ATA_STAT_ERR)) !=
		    ATA_STAT_DRQ) {
			printf("Error (no IRQ) dev %d blk " LBAF
			       ": status %#02x\n", device, blknr, c);
			break;
		}

		ide_input_data(device, buffer, ATA_SECTORWORDS);
		(void) ide_inb(device, ATA_STATUS);	/* clear IRQ */

		++n;
		++blknr;
		buffer += ATA_BLOCKSIZE;
	}
IDE_READ_E:
	return n;
}

static ulong ide_write(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
		       const void *buffer)
{
	struct blk_desc *desc = dev_get_uclass_plat(dev);
	int device = desc->devnum;
	ulong n = 0;
	bool lba48 = false;
	u8 c;

	if (IS_ENABLED(CONFIG_LBA48) && (blknr & 0x0000fffff0000000ULL)) {
		/* more than 28 bits used, use 48bit mode */
		lba48 = true;
	}

	/* Select device
	 */
	ide_outb(device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));

	while (blkcnt-- > 0) {
		c = ide_wait(device, IDE_TIME_OUT);

		if (c & ATA_STAT_BUSY) {
			printf("IDE read: device %d not ready\n", device);
			goto WR_OUT;
		}
		if (IS_ENABLED(CONFIG_LBA48) && lba48) {
			/* write high bits */
			ide_outb(device, ATA_SECT_CNT, 0);
			ide_outb(device, ATA_LBA_LOW, (blknr >> 24) & 0xff);
#ifdef CONFIG_SYS_64BIT_LBA
			ide_outb(device, ATA_LBA_MID, (blknr >> 32) & 0xff);
			ide_outb(device, ATA_LBA_HIGH, (blknr >> 40) & 0xff);
#else
			ide_outb(device, ATA_LBA_MID, 0);
			ide_outb(device, ATA_LBA_HIGH, 0);
#endif
		}
		ide_outb(device, ATA_SECT_CNT, 1);
		ide_outb(device, ATA_LBA_LOW, (blknr >> 0) & 0xff);
		ide_outb(device, ATA_LBA_MID, (blknr >> 8) & 0xff);
		ide_outb(device, ATA_LBA_HIGH, (blknr >> 16) & 0xff);

		if (IS_ENABLED(CONFIG_LBA48) && lba48) {
			ide_outb(device, ATA_DEV_HD,
				 ATA_LBA | ATA_DEVICE(device));
			ide_outb(device, ATA_COMMAND, ATA_CMD_PIO_WRITE_EXT);

		} else {
			ide_outb(device, ATA_DEV_HD, ATA_LBA |
				 ATA_DEVICE(device) | ((blknr >> 24) & 0xf));
			ide_outb(device, ATA_COMMAND, ATA_CMD_PIO_WRITE);
		}

		udelay(50);

		/* can't take over 500 ms */
		c = ide_wait(device, IDE_TIME_OUT);

		if ((c & (ATA_STAT_DRQ | ATA_STAT_BUSY | ATA_STAT_ERR)) !=
		    ATA_STAT_DRQ) {
			printf("Error (no IRQ) dev %d blk " LBAF
			       ": status %#02x\n", device, blknr, c);
			goto WR_OUT;
		}

		ide_output_data(device, buffer, ATA_SECTORWORDS);
		c = ide_inb(device, ATA_STATUS);	/* clear IRQ */
		++n;
		++blknr;
		buffer += ATA_BLOCKSIZE;
	}
WR_OUT:
	return n;
}

ulong ide_or_atapi_read(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
			void *buffer)
{
	struct blk_desc *desc = dev_get_uclass_plat(dev);

	if (IS_ENABLED(CONFIG_ATAPI) && desc->atapi)
		return atapi_read(dev, blknr, blkcnt, buffer);

	return ide_read(dev, blknr, blkcnt, buffer);
}

static const struct blk_ops ide_blk_ops = {
	.read	= ide_or_atapi_read,
	.write	= ide_write,
};

U_BOOT_DRIVER(ide_blk) = {
	.name		= "ide_blk",
	.id		= UCLASS_BLK,
	.ops		= &ide_blk_ops,
};

static int ide_bootdev_bind(struct udevice *dev)
{
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

	ucp->prio = BOOTDEVP_5_SCAN_SLOW;

	return 0;
}

static int ide_bootdev_hunt(struct bootdev_hunter *info, bool show)
{
	struct udevice *dev;

	uclass_first_device(UCLASS_IDE, &dev);

	return 0;
}

struct bootdev_ops ide_bootdev_ops = {
};

static const struct udevice_id ide_bootdev_ids[] = {
	{ .compatible = "u-boot,bootdev-ide" },
	{ }
};

U_BOOT_DRIVER(ide_bootdev) = {
	.name		= "ide_bootdev",
	.id		= UCLASS_BOOTDEV,
	.ops		= &ide_bootdev_ops,
	.bind		= ide_bootdev_bind,
	.of_match	= ide_bootdev_ids,
};

BOOTDEV_HUNTER(ide_bootdev_hunter) = {
	.prio		= BOOTDEVP_5_SCAN_SLOW,
	.uclass		= UCLASS_IDE,
	.hunt		= ide_bootdev_hunt,
	.drv		= DM_DRIVER_REF(ide_bootdev),
};

static int ide_probe(struct udevice *udev)
{
	bool bus_ok[CONFIG_SYS_IDE_MAXBUS];
	int i, bus;

	schedule();

	/* ATAPI Drives seems to need a proper IDE Reset */
	ide_reset();

	/*
	 * Wait for IDE to get ready.
	 * According to spec, this can take up to 31 seconds!
	 */
	for (bus = 0; bus < CONFIG_SYS_IDE_MAXBUS; ++bus) {
		bus_ok[bus] = !ide_init_one(bus);
		schedule();
	}

	putc('\n');

	schedule();

	for (i = 0; i < CONFIG_SYS_IDE_MAXDEVICE; i++) {
		struct blk_desc *desc, pdesc;
		struct udevice *blk;
		char name[20];
		int ret;

		if (!bus_ok[IDE_BUS(i)])
			continue;

		ret = ide_ident(i, &pdesc);
		dev_print(&pdesc);

		if (ret)
			continue;

		sprintf(name, "blk#%d", i);

		/*
		 * With CDROM, if there is no CD inserted, blksz will
		 * be zero, don't bother to create IDE block device.
		 */
		if (!pdesc.blksz)
			continue;
		ret = blk_create_devicef(udev, "ide_blk", name, UCLASS_IDE, i,
					 pdesc.blksz, pdesc.lba, &blk);
		if (ret)
			return ret;

		ret = blk_probe_or_unbind(blk);
		if (ret)
			return ret;

		/* fill in device vendor/product/rev strings */
		desc = dev_get_uclass_plat(blk);
		strlcpy(desc->vendor, pdesc.vendor, BLK_VEN_SIZE);
		strlcpy(desc->product, pdesc.product, BLK_PRD_SIZE);
		strlcpy(desc->revision, pdesc.revision, BLK_REV_SIZE);
		desc->removable = pdesc.removable;
		desc->atapi = pdesc.atapi;
		desc->lba48 = pdesc.lba48;
		desc->type = pdesc.type;

		ret = bootdev_setup_for_sibling_blk(blk, "ide_bootdev");
		if (ret)
			return log_msg_ret("bd", ret);
	}

	return 0;
}

U_BOOT_DRIVER(ide) = {
	.name		= "ide",
	.id		= UCLASS_IDE,
	.probe		= ide_probe,
};

struct pci_device_id ide_supported[] = {
	{ PCI_DEVICE_CLASS(PCI_CLASS_STORAGE_IDE << 8, 0xffff00) },
	{ }
};

U_BOOT_PCI_DEVICE(ide, ide_supported);

UCLASS_DRIVER(ide) = {
	.name		= "ide",
	.id		= UCLASS_IDE,
};
