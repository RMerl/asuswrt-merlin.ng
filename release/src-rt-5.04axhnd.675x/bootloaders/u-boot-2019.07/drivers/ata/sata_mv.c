// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) Excito Elektronik i Skåne AB, 2010.
 * Author: Tor Krill <tor@excito.com>
 *
 * Copyright (C) 2015, 2019 Stefan Roese <sr@denx.de>
 */

/*
 * This driver supports the SATA controller of some Mavell SoC's.
 * Here a (most likely incomplete) list of the supported SoC's:
 * - Kirkwood
 * - Armada 370
 * - Armada XP
 *
 * This driver implementation is an alternative to the already available
 * driver via the "ide" commands interface (drivers/block/mvsata_ide.c).
 * But this driver only supports PIO mode and as this new driver also
 * supports transfer via DMA, its much faster.
 *
 * Please note, that the newer SoC's (e.g. Armada 38x) are not supported
 * by this driver. As they have an AHCI compatible SATA controller
 * integrated.
 */

/*
 * TODO:
 * Better error recovery
 * No support for using PRDs (Thus max 64KB transfers)
 * No NCQ support
 * No port multiplier support
 */

#include <common.h>
#include <ahci.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <fis.h>
#include <libata.h>
#include <malloc.h>
#include <sata.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <linux/mbus.h>

#include <asm/arch/soc.h>
#if defined(CONFIG_KIRKWOOD)
#define SATAHC_BASE		KW_SATA_BASE
#else
#define SATAHC_BASE		MVEBU_AXP_SATA_BASE
#endif

#define SATA0_BASE		(SATAHC_BASE + 0x2000)
#define SATA1_BASE		(SATAHC_BASE + 0x4000)

/* EDMA registers */
#define EDMA_CFG		0x000
#define EDMA_CFG_NCQ		(1 << 5)
#define EDMA_CFG_EQUE		(1 << 9)
#define EDMA_TIMER		0x004
#define EDMA_IECR		0x008
#define EDMA_IEMR		0x00c
#define EDMA_RQBA_HI		0x010
#define EDMA_RQIPR		0x014
#define EDMA_RQIPR_IPMASK	(0x1f << 5)
#define EDMA_RQIPR_IPSHIFT	5
#define EDMA_RQOPR		0x018
#define EDMA_RQOPR_OPMASK	(0x1f << 5)
#define EDMA_RQOPR_OPSHIFT	5
#define EDMA_RSBA_HI		0x01c
#define EDMA_RSIPR		0x020
#define EDMA_RSIPR_IPMASK	(0x1f << 3)
#define EDMA_RSIPR_IPSHIFT	3
#define	EDMA_RSOPR		0x024
#define EDMA_RSOPR_OPMASK	(0x1f << 3)
#define EDMA_RSOPR_OPSHIFT	3
#define EDMA_CMD		0x028
#define EDMA_CMD_ENEDMA		(0x01 << 0)
#define EDMA_CMD_DISEDMA	(0x01 << 1)
#define EDMA_CMD_ATARST		(0x01 << 2)
#define EDMA_CMD_FREEZE		(0x01 << 4)
#define EDMA_TEST_CTL		0x02c
#define EDMA_STATUS		0x030
#define EDMA_IORTO		0x034
#define EDMA_CDTR		0x040
#define EDMA_HLTCND		0x060
#define EDMA_NTSR		0x094

/* Basic DMA registers */
#define BDMA_CMD		0x224
#define BDMA_STATUS		0x228
#define BDMA_DTLB		0x22c
#define BDMA_DTHB		0x230
#define BDMA_DRL		0x234
#define BDMA_DRH		0x238

/* SATA Interface registers */
#define SIR_ICFG		0x050
#define SIR_CFG_GEN2EN		(0x1 << 7)
#define SIR_PLL_CFG		0x054
#define SIR_SSTATUS		0x300
#define SSTATUS_DET_MASK	(0x0f << 0)
#define SIR_SERROR		0x304
#define SIR_SCONTROL		0x308
#define SIR_SCONTROL_DETEN	(0x01 << 0)
#define SIR_LTMODE		0x30c
#define SIR_LTMODE_NELBE	(0x01 << 7)
#define SIR_PHYMODE3		0x310
#define SIR_PHYMODE4		0x314
#define SIR_PHYMODE1		0x32c
#define SIR_PHYMODE2		0x330
#define SIR_BIST_CTRL		0x334
#define SIR_BIST_DW1		0x338
#define SIR_BIST_DW2		0x33c
#define SIR_SERR_IRQ_MASK	0x340
#define SIR_SATA_IFCTRL		0x344
#define SIR_SATA_TESTCTRL	0x348
#define SIR_SATA_IFSTATUS	0x34c
#define SIR_VEND_UNIQ		0x35c
#define SIR_FIS_CFG		0x360
#define SIR_FIS_IRQ_CAUSE	0x364
#define SIR_FIS_IRQ_MASK	0x368
#define SIR_FIS_DWORD0		0x370
#define SIR_FIS_DWORD1		0x374
#define SIR_FIS_DWORD2		0x378
#define SIR_FIS_DWORD3		0x37c
#define SIR_FIS_DWORD4		0x380
#define SIR_FIS_DWORD5		0x384
#define SIR_FIS_DWORD6		0x388
#define SIR_PHYM9_GEN2		0x398
#define SIR_PHYM9_GEN1		0x39c
#define SIR_PHY_CFG		0x3a0
#define SIR_PHYCTL		0x3a4
#define SIR_PHYM10		0x3a8
#define SIR_PHYM12		0x3b0

/* Shadow registers */
#define	PIO_DATA		0x100
#define PIO_ERR_FEATURES	0x104
#define PIO_SECTOR_COUNT	0x108
#define PIO_LBA_LOW		0x10c
#define PIO_LBA_MID		0x110
#define PIO_LBA_HI		0x114
#define PIO_DEVICE		0x118
#define PIO_CMD_STATUS		0x11c
#define PIO_STATUS_ERR		(0x01 << 0)
#define PIO_STATUS_DRQ		(0x01 << 3)
#define PIO_STATUS_DF		(0x01 << 5)
#define PIO_STATUS_DRDY		(0x01 << 6)
#define PIO_STATUS_BSY		(0x01 << 7)
#define PIO_CTRL_ALTSTAT	0x120

/* SATAHC arbiter registers */
#define SATAHC_CFG		0x000
#define SATAHC_RQOP		0x004
#define SATAHC_RQIP		0x008
#define SATAHC_ICT		0x00c
#define SATAHC_ITT		0x010
#define SATAHC_ICR		0x014
#define SATAHC_ICR_PORT0	(0x01 << 0)
#define SATAHC_ICR_PORT1	(0x01 << 1)
#define SATAHC_MIC		0x020
#define SATAHC_MIM		0x024
#define SATAHC_LED_CFG		0x02c

#define REQUEST_QUEUE_SIZE	32
#define RESPONSE_QUEUE_SIZE	REQUEST_QUEUE_SIZE

struct crqb {
	u32 dtb_low;		/* DW0 */
	u32 dtb_high;		/* DW1 */
	u32 control_flags;	/* DW2 */
	u32 drb_count;		/* DW3 */
	u32 ata_cmd_feat;	/* DW4 */
	u32 ata_addr;		/* DW5 */
	u32 ata_addr_exp;	/* DW6 */
	u32 ata_sect_count;	/* DW7 */
};

#define CRQB_ALIGN			0x400

#define CRQB_CNTRLFLAGS_DIR		(0x01 << 0)
#define CRQB_CNTRLFLAGS_DQTAGMASK	(0x1f << 1)
#define CRQB_CNTRLFLAGS_DQTAGSHIFT	1
#define CRQB_CNTRLFLAGS_PMPORTMASK	(0x0f << 12)
#define CRQB_CNTRLFLAGS_PMPORTSHIFT	12
#define CRQB_CNTRLFLAGS_PRDMODE		(0x01 << 16)
#define CRQB_CNTRLFLAGS_HQTAGMASK	(0x1f << 17)
#define CRQB_CNTRLFLAGS_HQTAGSHIFT	17

#define CRQB_CMDFEAT_CMDMASK		(0xff << 16)
#define CRQB_CMDFEAT_CMDSHIFT		16
#define CRQB_CMDFEAT_FEATMASK		(0xff << 16)
#define CRQB_CMDFEAT_FEATSHIFT		24

#define CRQB_ADDR_LBA_LOWMASK		(0xff << 0)
#define CRQB_ADDR_LBA_LOWSHIFT		0
#define CRQB_ADDR_LBA_MIDMASK		(0xff << 8)
#define CRQB_ADDR_LBA_MIDSHIFT		8
#define CRQB_ADDR_LBA_HIGHMASK		(0xff << 16)
#define CRQB_ADDR_LBA_HIGHSHIFT		16
#define CRQB_ADDR_DEVICE_MASK		(0xff << 24)
#define CRQB_ADDR_DEVICE_SHIFT		24

#define CRQB_ADDR_LBA_LOW_EXP_MASK	(0xff << 0)
#define CRQB_ADDR_LBA_LOW_EXP_SHIFT	0
#define CRQB_ADDR_LBA_MID_EXP_MASK	(0xff << 8)
#define CRQB_ADDR_LBA_MID_EXP_SHIFT	8
#define CRQB_ADDR_LBA_HIGH_EXP_MASK	(0xff << 16)
#define CRQB_ADDR_LBA_HIGH_EXP_SHIFT	16
#define CRQB_ADDR_FEATURE_EXP_MASK	(0xff << 24)
#define CRQB_ADDR_FEATURE_EXP_SHIFT	24

#define CRQB_SECTCOUNT_COUNT_MASK	(0xff << 0)
#define CRQB_SECTCOUNT_COUNT_SHIFT	0
#define CRQB_SECTCOUNT_COUNT_EXP_MASK	(0xff << 8)
#define CRQB_SECTCOUNT_COUNT_EXP_SHIFT	8

#define MVSATA_WIN_CONTROL(w)	(SATAHC_BASE + 0x30 + ((w) << 4))
#define MVSATA_WIN_BASE(w)	(SATAHC_BASE + 0x34 + ((w) << 4))

struct eprd {
	u32 phyaddr_low;
	u32 bytecount_eot;
	u32 phyaddr_hi;
	u32 reserved;
};

#define EPRD_PHYADDR_MASK	0xfffffffe
#define EPRD_BYTECOUNT_MASK	0x0000ffff
#define EPRD_EOT		(0x01 << 31)

struct crpb {
	u32 id;
	u32 flags;
	u32 timestamp;
};

#define CRPB_ALIGN		0x100

#define READ_CMD		0
#define WRITE_CMD		1

/*
 * Since we don't use PRDs yet max transfer size
 * is 64KB
 */
#define MV_ATA_MAX_SECTORS	(65535 / ATA_SECT_SIZE)

/* Keep track if hw is initialized or not */
static u32 hw_init;

struct mv_priv {
	char name[12];
	u32 link;
	u32 regbase;
	u32 queue_depth;
	u16 pio;
	u16 mwdma;
	u16 udma;
	int dev_nr;

	void *crqb_alloc;
	struct crqb *request;

	void *crpb_alloc;
	struct crpb *response;
};

static int ata_wait_register(u32 *addr, u32 mask, u32 val, u32 timeout_msec)
{
	ulong start;

	start = get_timer(0);
	do {
		if ((in_le32(addr) & mask) == val)
			return 0;
	} while (get_timer(start) < timeout_msec);

	return -ETIMEDOUT;
}

/* Cut from sata_mv in linux kernel */
static int mv_stop_edma_engine(struct udevice *dev, int port)
{
	struct mv_priv *priv = dev_get_platdata(dev);
	int i;

	/* Disable eDMA. The disable bit auto clears. */
	out_le32(priv->regbase + EDMA_CMD, EDMA_CMD_DISEDMA);

	/* Wait for the chip to confirm eDMA is off. */
	for (i = 10000; i > 0; i--) {
		u32 reg = in_le32(priv->regbase + EDMA_CMD);
		if (!(reg & EDMA_CMD_ENEDMA)) {
			debug("EDMA stop on port %d succesful\n", port);
			return 0;
		}
		udelay(10);
	}
	debug("EDMA stop on port %d failed\n", port);
	return -1;
}

static int mv_start_edma_engine(struct udevice *dev, int port)
{
	struct mv_priv *priv = dev_get_platdata(dev);
	u32 tmp;

	/* Check preconditions */
	tmp = in_le32(priv->regbase + SIR_SSTATUS);
	if ((tmp & SSTATUS_DET_MASK) != 0x03) {
		printf("Device error on port: %d\n", port);
		return -1;
	}

	tmp = in_le32(priv->regbase + PIO_CMD_STATUS);
	if (tmp & (ATA_BUSY | ATA_DRQ)) {
		printf("Device not ready on port: %d\n", port);
		return -1;
	}

	/* Clear interrupt cause */
	out_le32(priv->regbase + EDMA_IECR, 0x0);

	tmp = in_le32(SATAHC_BASE + SATAHC_ICR);
	tmp &= ~(port == 0 ? SATAHC_ICR_PORT0 : SATAHC_ICR_PORT1);
	out_le32(SATAHC_BASE + SATAHC_ICR, tmp);

	/* Configure edma operation */
	tmp = in_le32(priv->regbase + EDMA_CFG);
	tmp &= ~EDMA_CFG_NCQ;	/* No NCQ */
	tmp &= ~EDMA_CFG_EQUE;	/* Dont queue operations */
	out_le32(priv->regbase + EDMA_CFG, tmp);

	out_le32(priv->regbase + SIR_FIS_IRQ_CAUSE, 0x0);

	/* Configure fis, set all to no-wait for now */
	out_le32(priv->regbase + SIR_FIS_CFG, 0x0);

	/* Setup request queue */
	out_le32(priv->regbase + EDMA_RQBA_HI, 0x0);
	out_le32(priv->regbase + EDMA_RQIPR, priv->request);
	out_le32(priv->regbase + EDMA_RQOPR, 0x0);

	/* Setup response queue */
	out_le32(priv->regbase + EDMA_RSBA_HI, 0x0);
	out_le32(priv->regbase + EDMA_RSOPR, priv->response);
	out_le32(priv->regbase + EDMA_RSIPR, 0x0);

	/* Start edma */
	out_le32(priv->regbase + EDMA_CMD, EDMA_CMD_ENEDMA);

	return 0;
}

static int mv_reset_channel(struct udevice *dev, int port)
{
	struct mv_priv *priv = dev_get_platdata(dev);

	/* Make sure edma is stopped  */
	mv_stop_edma_engine(dev, port);

	out_le32(priv->regbase + EDMA_CMD, EDMA_CMD_ATARST);
	udelay(25);		/* allow reset propagation */
	out_le32(priv->regbase + EDMA_CMD, 0);
	mdelay(10);

	return 0;
}

static void mv_reset_port(struct udevice *dev, int port)
{
	struct mv_priv *priv = dev_get_platdata(dev);

	mv_reset_channel(dev, port);

	out_le32(priv->regbase + EDMA_CMD, 0x0);
	out_le32(priv->regbase + EDMA_CFG, 0x101f);
	out_le32(priv->regbase + EDMA_IECR, 0x0);
	out_le32(priv->regbase + EDMA_IEMR, 0x0);
	out_le32(priv->regbase + EDMA_RQBA_HI, 0x0);
	out_le32(priv->regbase + EDMA_RQIPR, 0x0);
	out_le32(priv->regbase + EDMA_RQOPR, 0x0);
	out_le32(priv->regbase + EDMA_RSBA_HI, 0x0);
	out_le32(priv->regbase + EDMA_RSIPR, 0x0);
	out_le32(priv->regbase + EDMA_RSOPR, 0x0);
	out_le32(priv->regbase + EDMA_IORTO, 0xfa);
}

static void mv_reset_one_hc(void)
{
	out_le32(SATAHC_BASE + SATAHC_ICT, 0x00);
	out_le32(SATAHC_BASE + SATAHC_ITT, 0x00);
	out_le32(SATAHC_BASE + SATAHC_ICR, 0x00);
}

static int probe_port(struct udevice *dev, int port)
{
	struct mv_priv *priv = dev_get_platdata(dev);
	int tries, tries2, set15 = 0;
	u32 tmp;

	debug("Probe port: %d\n", port);

	for (tries = 0; tries < 2; tries++) {
		/* Clear SError */
		out_le32(priv->regbase + SIR_SERROR, 0x0);

		/* trigger com-init */
		tmp = in_le32(priv->regbase + SIR_SCONTROL);
		tmp = (tmp & 0x0f0) | 0x300 | SIR_SCONTROL_DETEN;
		out_le32(priv->regbase + SIR_SCONTROL, tmp);

		mdelay(1);

		tmp = in_le32(priv->regbase + SIR_SCONTROL);
		tries2 = 5;
		do {
			tmp = (tmp & 0x0f0) | 0x300;
			out_le32(priv->regbase + SIR_SCONTROL, tmp);
			mdelay(10);
			tmp = in_le32(priv->regbase + SIR_SCONTROL);
		} while ((tmp & 0xf0f) != 0x300 && tries2--);

		mdelay(10);

		for (tries2 = 0; tries2 < 200; tries2++) {
			tmp = in_le32(priv->regbase + SIR_SSTATUS);
			if ((tmp & SSTATUS_DET_MASK) == 0x03) {
				debug("Found device on port\n");
				return 0;
			}
			mdelay(1);
		}

		if ((tmp & SSTATUS_DET_MASK) == 0) {
			debug("No device attached on port %d\n", port);
			return -ENODEV;
		}

		if (!set15) {
			/* Try on 1.5Gb/S */
			debug("Try 1.5Gb link\n");
			set15 = 1;
			out_le32(priv->regbase + SIR_SCONTROL, 0x304);

			tmp = in_le32(priv->regbase + SIR_ICFG);
			tmp &= ~SIR_CFG_GEN2EN;
			out_le32(priv->regbase + SIR_ICFG, tmp);

			mv_reset_channel(dev, port);
		}
	}

	debug("Failed to probe port\n");
	return -1;
}

/* Get request queue in pointer */
static int get_reqip(struct udevice *dev, int port)
{
	struct mv_priv *priv = dev_get_platdata(dev);
	u32 tmp;

	tmp = in_le32(priv->regbase + EDMA_RQIPR) & EDMA_RQIPR_IPMASK;
	tmp = tmp >> EDMA_RQIPR_IPSHIFT;

	return tmp;
}

static void set_reqip(struct udevice *dev, int port, int reqin)
{
	struct mv_priv *priv = dev_get_platdata(dev);
	u32 tmp;

	tmp = in_le32(priv->regbase + EDMA_RQIPR) & ~EDMA_RQIPR_IPMASK;
	tmp |= ((reqin << EDMA_RQIPR_IPSHIFT) & EDMA_RQIPR_IPMASK);
	out_le32(priv->regbase + EDMA_RQIPR, tmp);
}

/* Get next available slot, ignoring possible overwrite */
static int get_next_reqip(struct udevice *dev, int port)
{
	int slot = get_reqip(dev, port);
	slot = (slot + 1) % REQUEST_QUEUE_SIZE;
	return slot;
}

/* Get response queue in pointer */
static int get_rspip(struct udevice *dev, int port)
{
	struct mv_priv *priv = dev_get_platdata(dev);
	u32 tmp;

	tmp = in_le32(priv->regbase + EDMA_RSIPR) & EDMA_RSIPR_IPMASK;
	tmp = tmp >> EDMA_RSIPR_IPSHIFT;

	return tmp;
}

/* Get response queue out pointer */
static int get_rspop(struct udevice *dev, int port)
{
	struct mv_priv *priv = dev_get_platdata(dev);
	u32 tmp;

	tmp = in_le32(priv->regbase + EDMA_RSOPR) & EDMA_RSOPR_OPMASK;
	tmp = tmp >> EDMA_RSOPR_OPSHIFT;
	return tmp;
}

/* Get next response queue pointer  */
static int get_next_rspop(struct udevice *dev, int port)
{
	return (get_rspop(dev, port) + 1) % RESPONSE_QUEUE_SIZE;
}

/* Set response queue pointer */
static void set_rspop(struct udevice *dev, int port, int reqin)
{
	struct mv_priv *priv = dev_get_platdata(dev);
	u32 tmp;

	tmp = in_le32(priv->regbase + EDMA_RSOPR) & ~EDMA_RSOPR_OPMASK;
	tmp |= ((reqin << EDMA_RSOPR_OPSHIFT) & EDMA_RSOPR_OPMASK);

	out_le32(priv->regbase + EDMA_RSOPR, tmp);
}

static int wait_dma_completion(struct udevice *dev, int port, int index,
			       u32 timeout_msec)
{
	u32 tmp, res;

	tmp = port == 0 ? SATAHC_ICR_PORT0 : SATAHC_ICR_PORT1;
	res = ata_wait_register((u32 *)(SATAHC_BASE + SATAHC_ICR), tmp,
				tmp, timeout_msec);
	if (res)
		printf("Failed to wait for completion on port %d\n", port);

	return res;
}

static void process_responses(struct udevice *dev, int port)
{
#ifdef DEBUG
	struct mv_priv *priv = dev_get_platdata(dev);
#endif
	u32 tmp;
	u32 outind = get_rspop(dev, port);

	/* Ack interrupts */
	tmp = in_le32(SATAHC_BASE + SATAHC_ICR);
	if (port == 0)
		tmp &= ~(BIT(0) | BIT(8));
	else
		tmp &= ~(BIT(1) | BIT(9));
	tmp &= ~(BIT(4));
	out_le32(SATAHC_BASE + SATAHC_ICR, tmp);

	while (get_rspip(dev, port) != outind) {
#ifdef DEBUG
		debug("Response index %d flags %08x on port %d\n", outind,
		      priv->response[outind].flags, port);
#endif
		outind = get_next_rspop(dev, port);
		set_rspop(dev, port, outind);
	}
}

static int mv_ata_exec_ata_cmd(struct udevice *dev, int port,
			       struct sata_fis_h2d *cfis,
			       u8 *buffer, u32 len, u32 iswrite)
{
	struct mv_priv *priv = dev_get_platdata(dev);
	struct crqb *req;
	int slot;
	u32 start;

	if (len >= 64 * 1024) {
		printf("We only support <64K transfers for now\n");
		return -1;
	}

	/* Initialize request */
	slot = get_reqip(dev, port);
	memset(&priv->request[slot], 0, sizeof(struct crqb));
	req = &priv->request[slot];

	req->dtb_low = (u32)buffer;

	/* Dont use PRDs */
	req->control_flags = CRQB_CNTRLFLAGS_PRDMODE;
	req->control_flags |= iswrite ? 0 : CRQB_CNTRLFLAGS_DIR;
	req->control_flags |=
	    ((cfis->pm_port_c << CRQB_CNTRLFLAGS_PMPORTSHIFT)
	     & CRQB_CNTRLFLAGS_PMPORTMASK);

	req->drb_count = len;

	req->ata_cmd_feat = (cfis->command << CRQB_CMDFEAT_CMDSHIFT) &
		CRQB_CMDFEAT_CMDMASK;
	req->ata_cmd_feat |= (cfis->features << CRQB_CMDFEAT_FEATSHIFT) &
		CRQB_CMDFEAT_FEATMASK;

	req->ata_addr = (cfis->lba_low << CRQB_ADDR_LBA_LOWSHIFT) &
		CRQB_ADDR_LBA_LOWMASK;
	req->ata_addr |= (cfis->lba_mid << CRQB_ADDR_LBA_MIDSHIFT) &
		CRQB_ADDR_LBA_MIDMASK;
	req->ata_addr |= (cfis->lba_high << CRQB_ADDR_LBA_HIGHSHIFT) &
		CRQB_ADDR_LBA_HIGHMASK;
	req->ata_addr |= (cfis->device << CRQB_ADDR_DEVICE_SHIFT) &
		CRQB_ADDR_DEVICE_MASK;

	req->ata_addr_exp = (cfis->lba_low_exp << CRQB_ADDR_LBA_LOW_EXP_SHIFT) &
		CRQB_ADDR_LBA_LOW_EXP_MASK;
	req->ata_addr_exp |=
		(cfis->lba_mid_exp << CRQB_ADDR_LBA_MID_EXP_SHIFT) &
		CRQB_ADDR_LBA_MID_EXP_MASK;
	req->ata_addr_exp |=
		(cfis->lba_high_exp << CRQB_ADDR_LBA_HIGH_EXP_SHIFT) &
		CRQB_ADDR_LBA_HIGH_EXP_MASK;
	req->ata_addr_exp |=
		(cfis->features_exp << CRQB_ADDR_FEATURE_EXP_SHIFT) &
		CRQB_ADDR_FEATURE_EXP_MASK;

	req->ata_sect_count =
		(cfis->sector_count << CRQB_SECTCOUNT_COUNT_SHIFT) &
		CRQB_SECTCOUNT_COUNT_MASK;
	req->ata_sect_count |=
		(cfis->sector_count_exp << CRQB_SECTCOUNT_COUNT_EXP_SHIFT) &
		CRQB_SECTCOUNT_COUNT_EXP_MASK;

	/* Flush data */
	start = (u32)req & ~(ARCH_DMA_MINALIGN - 1);
	flush_dcache_range(start,
			   start + ALIGN(sizeof(*req), ARCH_DMA_MINALIGN));

	/* Trigger operation */
	slot = get_next_reqip(dev, port);
	set_reqip(dev, port, slot);

	/* Wait for completion */
	if (wait_dma_completion(dev, port, slot, 10000)) {
		printf("ATA operation timed out\n");
		return -1;
	}

	process_responses(dev, port);

	/* Invalidate data on read */
	if (buffer && len) {
		start = (u32)buffer & ~(ARCH_DMA_MINALIGN - 1);
		invalidate_dcache_range(start,
					start + ALIGN(len, ARCH_DMA_MINALIGN));
	}

	return len;
}

static u32 mv_sata_rw_cmd_ext(struct udevice *dev, int port, lbaint_t start,
			      u32 blkcnt,
			      u8 *buffer, int is_write)
{
	struct sata_fis_h2d cfis;
	u32 res;
	u64 block;

	block = (u64)start;

	memset(&cfis, 0, sizeof(struct sata_fis_h2d));

	cfis.fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis.command = (is_write) ? ATA_CMD_WRITE_EXT : ATA_CMD_READ_EXT;

	cfis.lba_high_exp = (block >> 40) & 0xff;
	cfis.lba_mid_exp = (block >> 32) & 0xff;
	cfis.lba_low_exp = (block >> 24) & 0xff;
	cfis.lba_high = (block >> 16) & 0xff;
	cfis.lba_mid = (block >> 8) & 0xff;
	cfis.lba_low = block & 0xff;
	cfis.device = ATA_LBA;
	cfis.sector_count_exp = (blkcnt >> 8) & 0xff;
	cfis.sector_count = blkcnt & 0xff;

	res = mv_ata_exec_ata_cmd(dev, port, &cfis, buffer,
				  ATA_SECT_SIZE * blkcnt, is_write);

	return res >= 0 ? blkcnt : res;
}

static u32 mv_sata_rw_cmd(struct udevice *dev, int port, lbaint_t start,
			  u32 blkcnt, u8 *buffer, int is_write)
{
	struct sata_fis_h2d cfis;
	lbaint_t block;
	u32 res;

	block = start;

	memset(&cfis, 0, sizeof(struct sata_fis_h2d));

	cfis.fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis.command = (is_write) ? ATA_CMD_WRITE : ATA_CMD_READ;
	cfis.device = ATA_LBA;

	cfis.device |= (block >> 24) & 0xf;
	cfis.lba_high = (block >> 16) & 0xff;
	cfis.lba_mid = (block >> 8) & 0xff;
	cfis.lba_low = block & 0xff;
	cfis.sector_count = (u8)(blkcnt & 0xff);

	res = mv_ata_exec_ata_cmd(dev, port, &cfis, buffer,
				  ATA_SECT_SIZE * blkcnt, is_write);

	return res >= 0 ? blkcnt : res;
}

static u32 ata_low_level_rw(struct udevice *dev, int port, lbaint_t blknr,
			    lbaint_t blkcnt, void *buffer, int is_write)
{
	struct blk_desc *desc = dev_get_uclass_platdata(dev);
	lbaint_t start, blks;
	u8 *addr;
	int max_blks;

	debug("%s: " LBAFU " " LBAFU "\n", __func__, blknr, blkcnt);

	start = blknr;
	blks = blkcnt;
	addr = (u8 *)buffer;

	max_blks = MV_ATA_MAX_SECTORS;
	do {
		if (blks > max_blks) {
			if (desc->lba48) {
				mv_sata_rw_cmd_ext(dev, port, start, max_blks,
						   addr, is_write);
			} else {
				mv_sata_rw_cmd(dev, port, start, max_blks,
					       addr, is_write);
			}
			start += max_blks;
			blks -= max_blks;
			addr += ATA_SECT_SIZE * max_blks;
		} else {
			if (desc->lba48) {
				mv_sata_rw_cmd_ext(dev, port, start, blks, addr,
						   is_write);
			} else {
				mv_sata_rw_cmd(dev, port, start, blks, addr,
					       is_write);
			}
			start += blks;
			blks = 0;
			addr += ATA_SECT_SIZE * blks;
		}
	} while (blks != 0);

	return blkcnt;
}

static int mv_ata_exec_ata_cmd_nondma(struct udevice *dev, int port,
				      struct sata_fis_h2d *cfis, u8 *buffer,
				      u32 len, u32 iswrite)
{
	struct mv_priv *priv = dev_get_platdata(dev);
	int i;
	u16 *tp;

	debug("%s\n", __func__);

	out_le32(priv->regbase + PIO_SECTOR_COUNT, cfis->sector_count);
	out_le32(priv->regbase + PIO_LBA_HI, cfis->lba_high);
	out_le32(priv->regbase + PIO_LBA_MID, cfis->lba_mid);
	out_le32(priv->regbase + PIO_LBA_LOW, cfis->lba_low);
	out_le32(priv->regbase + PIO_ERR_FEATURES, cfis->features);
	out_le32(priv->regbase + PIO_DEVICE, cfis->device);
	out_le32(priv->regbase + PIO_CMD_STATUS, cfis->command);

	if (ata_wait_register((u32 *)(priv->regbase + PIO_CMD_STATUS),
			      ATA_BUSY, 0x0, 10000)) {
		debug("Failed to wait for completion\n");
		return -1;
	}

	if (len > 0) {
		tp = (u16 *)buffer;
		for (i = 0; i < len / 2; i++) {
			if (iswrite)
				out_le16(priv->regbase + PIO_DATA, *tp++);
			else
				*tp++ = in_le16(priv->regbase + PIO_DATA);
		}
	}

	return len;
}

static int mv_sata_identify(struct udevice *dev, int port, u16 *id)
{
	struct sata_fis_h2d h2d;

	memset(&h2d, 0, sizeof(struct sata_fis_h2d));

	h2d.fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	h2d.command = ATA_CMD_ID_ATA;

	/* Give device time to get operational */
	mdelay(10);

	return mv_ata_exec_ata_cmd_nondma(dev, port, &h2d, (u8 *)id,
					  ATA_ID_WORDS * 2, READ_CMD);
}

static void mv_sata_xfer_mode(struct udevice *dev, int port, u16 *id)
{
	struct mv_priv *priv = dev_get_platdata(dev);

	priv->pio = id[ATA_ID_PIO_MODES];
	priv->mwdma = id[ATA_ID_MWDMA_MODES];
	priv->udma = id[ATA_ID_UDMA_MODES];
	debug("pio %04x, mwdma %04x, udma %04x\n", priv->pio, priv->mwdma,
	      priv->udma);
}

static void mv_sata_set_features(struct udevice *dev, int port)
{
	struct mv_priv *priv = dev_get_platdata(dev);
	struct sata_fis_h2d cfis;
	u8 udma_cap;

	memset(&cfis, 0, sizeof(struct sata_fis_h2d));

	cfis.fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis.command = ATA_CMD_SET_FEATURES;
	cfis.features = SETFEATURES_XFER;

	/* First check the device capablity */
	udma_cap = (u8) (priv->udma & 0xff);

	if (udma_cap == ATA_UDMA6)
		cfis.sector_count = XFER_UDMA_6;
	if (udma_cap == ATA_UDMA5)
		cfis.sector_count = XFER_UDMA_5;
	if (udma_cap == ATA_UDMA4)
		cfis.sector_count = XFER_UDMA_4;
	if (udma_cap == ATA_UDMA3)
		cfis.sector_count = XFER_UDMA_3;

	mv_ata_exec_ata_cmd_nondma(dev, port, &cfis, NULL, 0, READ_CMD);
}

/*
 * Initialize SATA memory windows
 */
static void mvsata_ide_conf_mbus_windows(void)
{
	const struct mbus_dram_target_info *dram;
	int i;

	dram = mvebu_mbus_dram_info();

	/* Disable windows, Set Size/Base to 0  */
	for (i = 0; i < 4; i++) {
		writel(0, MVSATA_WIN_CONTROL(i));
		writel(0, MVSATA_WIN_BASE(i));
	}

	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;
		writel(((cs->size - 1) & 0xffff0000) | (cs->mbus_attr << 8) |
		       (dram->mbus_dram_target_id << 4) | 1,
		       MVSATA_WIN_CONTROL(i));
		writel(cs->base & 0xffff0000, MVSATA_WIN_BASE(i));
	}
}

static int sata_mv_init_sata(struct udevice *dev, int port)
{
	struct mv_priv *priv = dev_get_platdata(dev);

	debug("Initialize sata dev: %d\n", port);

	if (port < 0 || port >= CONFIG_SYS_SATA_MAX_DEVICE) {
		printf("Invalid sata device %d\n", port);
		return -1;
	}

	/* Allocate and align request buffer */
	priv->crqb_alloc = malloc(sizeof(struct crqb) * REQUEST_QUEUE_SIZE +
				  CRQB_ALIGN);
	if (!priv->crqb_alloc) {
		printf("Unable to allocate memory for request queue\n");
		return -ENOMEM;
	}
	memset(priv->crqb_alloc, 0,
	       sizeof(struct crqb) * REQUEST_QUEUE_SIZE + CRQB_ALIGN);
	priv->request = (struct crqb *)(((u32) priv->crqb_alloc + CRQB_ALIGN) &
					~(CRQB_ALIGN - 1));

	/* Allocate and align response buffer */
	priv->crpb_alloc = malloc(sizeof(struct crpb) * REQUEST_QUEUE_SIZE +
				  CRPB_ALIGN);
	if (!priv->crpb_alloc) {
		printf("Unable to allocate memory for response queue\n");
		return -ENOMEM;
	}
	memset(priv->crpb_alloc, 0,
	       sizeof(struct crpb) * REQUEST_QUEUE_SIZE + CRPB_ALIGN);
	priv->response = (struct crpb *)(((u32) priv->crpb_alloc + CRPB_ALIGN) &
					 ~(CRPB_ALIGN - 1));

	sprintf(priv->name, "SATA%d", port);

	priv->regbase = port == 0 ? SATA0_BASE : SATA1_BASE;

	if (!hw_init) {
		debug("Initialize sata hw\n");
		hw_init = 1;
		mv_reset_one_hc();
		mvsata_ide_conf_mbus_windows();
	}

	mv_reset_port(dev, port);

	if (probe_port(dev, port)) {
		priv->link = 0;
		return -ENODEV;
	}
	priv->link = 1;

	return 0;
}

static int sata_mv_scan_sata(struct udevice *dev, int port)
{
	struct blk_desc *desc = dev_get_uclass_platdata(dev);
	struct mv_priv *priv = dev_get_platdata(dev);
	unsigned char serial[ATA_ID_SERNO_LEN + 1];
	unsigned char firmware[ATA_ID_FW_REV_LEN + 1];
	unsigned char product[ATA_ID_PROD_LEN + 1];
	u64 n_sectors;
	u16 *id;

	if (!priv->link)
		return -ENODEV;

	id = (u16 *)malloc(ATA_ID_WORDS * 2);
	if (!id) {
		printf("Failed to malloc id data\n");
		return -ENOMEM;
	}

	mv_sata_identify(dev, port, id);
	ata_swap_buf_le16(id, ATA_ID_WORDS);
#ifdef DEBUG
	ata_dump_id(id);
#endif

	/* Serial number */
	ata_id_c_string(id, serial, ATA_ID_SERNO, sizeof(serial));
	memcpy(desc->product, serial, sizeof(serial));

	/* Firmware version */
	ata_id_c_string(id, firmware, ATA_ID_FW_REV, sizeof(firmware));
	memcpy(desc->revision, firmware, sizeof(firmware));

	/* Product model */
	ata_id_c_string(id, product, ATA_ID_PROD, sizeof(product));
	memcpy(desc->vendor, product, sizeof(product));

	/* Total sectors */
	n_sectors = ata_id_n_sectors(id);
	desc->lba = n_sectors;

	/* Check if support LBA48 */
	if (ata_id_has_lba48(id)) {
		desc->lba48 = 1;
		debug("Device support LBA48\n");
	}

	/* Get the NCQ queue depth from device */
	priv->queue_depth = ata_id_queue_depth(id);

	/* Get the xfer mode from device */
	mv_sata_xfer_mode(dev, port, id);

	/* Set the xfer mode to highest speed */
	mv_sata_set_features(dev, port);

	/* Start up */
	mv_start_edma_engine(dev, port);

	return 0;
}

static ulong sata_mv_read(struct udevice *blk, lbaint_t blknr,
			  lbaint_t blkcnt, void *buffer)
{
	struct mv_priv *priv = dev_get_platdata(blk);

	return ata_low_level_rw(blk, priv->dev_nr, blknr, blkcnt,
				buffer, READ_CMD);
}

static ulong sata_mv_write(struct udevice *blk, lbaint_t blknr,
			   lbaint_t blkcnt, const void *buffer)
{
	struct mv_priv *priv = dev_get_platdata(blk);

	return ata_low_level_rw(blk, priv->dev_nr, blknr, blkcnt,
				(void *)buffer, WRITE_CMD);
}

static const struct blk_ops sata_mv_blk_ops = {
	.read	= sata_mv_read,
	.write	= sata_mv_write,
};

U_BOOT_DRIVER(sata_mv_driver) = {
	.name = "sata_mv_blk",
	.id = UCLASS_BLK,
	.ops = &sata_mv_blk_ops,
	.platdata_auto_alloc_size = sizeof(struct mv_priv),
};

static int sata_mv_probe(struct udevice *dev)
{
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);
	struct mv_priv *priv;
	struct udevice *blk;
	int nr_ports;
	int ret;
	int i;

	/* Get number of ports of this SATA controller */
	nr_ports = min(fdtdec_get_int(blob, node, "nr-ports", -1),
		       CONFIG_SYS_SATA_MAX_DEVICE);

	for (i = 0; i < nr_ports; i++) {
		ret = blk_create_devicef(dev, "sata_mv_blk", "blk",
					 IF_TYPE_SATA, -1, 512, 0, &blk);
		if (ret) {
			debug("Can't create device\n");
			return ret;
		}

		priv = dev_get_platdata(blk);
		priv->dev_nr = i;

		/* Init SATA port */
		ret = sata_mv_init_sata(blk, i);
		if (ret) {
			debug("%s: Failed to init bus\n", __func__);
			return ret;
		}

		/* Scan SATA port */
		ret = sata_mv_scan_sata(blk, i);
		if (ret) {
			debug("%s: Failed to scan bus\n", __func__);
			return ret;
		}
	}

	return 0;
}

static int sata_mv_scan(struct udevice *dev)
{
	/* Nothing to do here */

	return 0;
}

static const struct udevice_id sata_mv_ids[] = {
	{ .compatible = "marvell,armada-370-sata" },
	{ .compatible = "marvell,orion-sata" },
	{ }
};

struct ahci_ops sata_mv_ahci_ops = {
	.scan = sata_mv_scan,
};

U_BOOT_DRIVER(sata_mv_ahci) = {
	.name = "sata_mv_ahci",
	.id = UCLASS_AHCI,
	.of_match = sata_mv_ids,
	.ops = &sata_mv_ahci_ops,
	.probe = sata_mv_probe,
};
