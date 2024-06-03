// SPDX-License-Identifier: GPL-2.0+
/*
 * LPC32xx Ethernet MAC interface driver
 *
 * (C) Copyright 2014  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD - 3ADEV <albert.aribaud@3adev.fr>
 */

#include <common.h>
#include <net.h>
#include <malloc.h>
#include <miiphy.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/types.h>
#include <asm/system.h>
#include <asm/byteorder.h>
#include <asm/arch/cpu.h>
#include <asm/arch/config.h>

/*
 * Notes:
 *
 * 1. Unless specified otherwise, all references to tables or paragraphs
 *    are to UM10326, "LPC32x0 and LPC32x0/01 User manual".
 *
 * 2. Only bitfield masks/values which are actually used by the driver
 *    are defined.
 */

/* a single RX descriptor. The controller has an array of these */
struct lpc32xx_eth_rxdesc {
	u32 packet;		/* Receive packet pointer */
	u32 control;		/* Descriptor command status */
};

#define LPC32XX_ETH_RX_DESC_SIZE (sizeof(struct lpc32xx_eth_rxdesc))

/* RX control bitfields/masks (see Table 330) */
#define LPC32XX_ETH_RX_CTRL_SIZE_MASK 0x000007FF
#define LPC32XX_ETH_RX_CTRL_UNUSED    0x7FFFF800
#define LPC32XX_ETH_RX_CTRL_INTERRUPT 0x80000000

/* a single RX status. The controller has an array of these */
struct lpc32xx_eth_rxstat {
	u32 statusinfo;		/* Transmit Descriptor status */
	u32 statushashcrc;	/* Transmit Descriptor CRCs */
};

#define LPC32XX_ETH_RX_STAT_SIZE (sizeof(struct lpc32xx_eth_rxstat))

/* RX statusinfo bitfields/masks (see Table 333) */
#define RX_STAT_RXSIZE 0x000007FF
/* Helper: OR of all errors except RANGE */
#define RX_STAT_ERRORS 0x1B800000

/* a single TX descriptor. The controller has an array of these */
struct lpc32xx_eth_txdesc {
	u32 packet;		/* Transmit packet pointer */
	u32 control;		/* Descriptor control */
};

#define LPC32XX_ETH_TX_DESC_SIZE (sizeof(struct lpc32xx_eth_txdesc))

/* TX control bitfields/masks (see Table 335) */
#define TX_CTRL_TXSIZE    0x000007FF
#define TX_CTRL_LAST      0x40000000

/* a single TX status. The controller has an array of these */
struct lpc32xx_eth_txstat {
	u32 statusinfo;		/* Transmit Descriptor status */
};

#define LPC32XX_ETH_TX_STAT_SIZE (sizeof(struct lpc32xx_eth_txstat))

/* Ethernet MAC interface registers (see Table 283) */
struct lpc32xx_eth_registers {
	/* MAC registers - 0x3106_0000 to 0x3106_01FC */
	u32 mac1;		/* MAC configuration register 1 */
	u32 mac2;		/* MAC configuration register 2 */
	u32 ipgt;		/* Back-to-back Inter-Packet Gap reg. */
	u32 ipgr;		/* Non-back-to-back IPG register */
	u32 clrt;		/* Collision Window / Retry register */
	u32 maxf;		/* Maximum Frame register */
	u32 supp;		/* Phy Support register */
	u32 test;
	u32 mcfg;		/* MII management configuration reg. */
	u32 mcmd;		/* MII management command register */
	u32 madr;		/* MII management address register */
	u32 mwtd;		/* MII management wite data register */
	u32 mrdd;		/* MII management read data register */
	u32 mind;		/* MII management indicators register */
	u32 reserved1[2];
	u32 sa0;		/* Station address register 0 */
	u32 sa1;		/* Station address register 1 */
	u32 sa2;		/* Station address register 2 */
	u32 reserved2[45];
	/* Control registers */
	u32 command;
	u32 status;
	u32 rxdescriptor;
	u32 rxstatus;
	u32 rxdescriptornumber;	/* actually, number MINUS ONE */
	u32 rxproduceindex;	/* head of rx desc fifo */
	u32 rxconsumeindex;	/* tail of rx desc fifo */
	u32 txdescriptor;
	u32 txstatus;
	u32 txdescriptornumber;	/* actually, number MINUS ONE */
	u32 txproduceindex;	/* head of rx desc fifo */
	u32 txconsumeindex;	/* tail of rx desc fifo */
	u32 reserved3[10];
	u32 tsv0;		/* Transmit status vector register 0 */
	u32 tsv1;		/* Transmit status vector register 1 */
	u32 rsv;		/* Receive status vector register */
	u32 reserved4[3];
	u32 flowcontrolcounter;
	u32 flowcontrolstatus;
	u32 reserved5[34];
	/* RX filter registers - 0x3106_0200 to 0x3106_0FDC */
	u32 rxfilterctrl;
	u32 rxfilterwolstatus;
	u32 rxfilterwolclear;
	u32 reserved6;
	u32 hashfilterl;
	u32 hashfilterh;
	u32 reserved7[882];
	/* Module control registers - 0x3106_0FE0 to 0x3106_0FF8 */
	u32 intstatus;		/* Interrupt status register */
	u32 intenable;
	u32 intclear;
	u32 intset;
	u32 reserved8;
	u32 powerdown;
	u32 reserved9;
};

/* MAC1 register bitfields/masks and offsets (see Table 283) */
#define MAC1_RECV_ENABLE        0x00000001
#define MAC1_PASS_ALL_RX_FRAMES 0x00000002
#define MAC1_SOFT_RESET         0x00008000
/* Helper: general reset */
#define MAC1_RESETS             0x0000CF00

/* MAC2 register bitfields/masks and offsets (see Table 284) */
#define MAC2_FULL_DUPLEX    0x00000001
#define MAC2_CRC_ENABLE     0x00000010
#define MAC2_PAD_CRC_ENABLE 0x00000020

/* SUPP register bitfields/masks and offsets (see Table 290) */
#define SUPP_SPEED 0x00000100

/* MCFG register bitfields/masks and offsets (see Table 292) */
#define MCFG_RESET_MII_MGMT     0x00008000
/* divide clock by 28 (see Table 293) */
#define MCFG_CLOCK_SELECT_DIV28 0x0000001C

/* MADR register bitfields/masks and offsets (see Table 295) */
#define MADR_REG_MASK   0x0000001F
#define MADR_PHY_MASK   0x00001F00
#define MADR_REG_OFFSET 0
#define MADR_PHY_OFFSET 8

/* MIND register bitfields/masks (see Table 298) */
#define MIND_BUSY      0x00000001

/* COMMAND register bitfields/masks and offsets (see Table 283) */
#define COMMAND_RXENABLE      0x00000001
#define COMMAND_TXENABLE      0x00000002
#define COMMAND_PASSRUNTFRAME 0x00000040
#define COMMAND_RMII          0x00000200
#define COMMAND_FULL_DUPLEX   0x00000400
/* Helper: general reset */
#define COMMAND_RESETS        0x00000038

/* STATUS register bitfields/masks and offsets (see Table 283) */
#define STATUS_RXSTATUS 0x00000001
#define STATUS_TXSTATUS 0x00000002

/* RXFILTERCTRL register bitfields/masks (see Table 319) */
#define RXFILTERCTRL_ACCEPTBROADCAST 0x00000002
#define RXFILTERCTRL_ACCEPTPERFECT   0x00000020

/* Buffers and descriptors */

#define ATTRS(n) __aligned(n)

#define TX_BUF_COUNT 4
#define RX_BUF_COUNT 4

struct lpc32xx_eth_buffers {
	ATTRS(4) struct lpc32xx_eth_txdesc tx_desc[TX_BUF_COUNT];
	ATTRS(4) struct lpc32xx_eth_txstat tx_stat[TX_BUF_COUNT];
	ATTRS(PKTALIGN) u8 tx_buf[TX_BUF_COUNT*PKTSIZE_ALIGN];
	ATTRS(4) struct lpc32xx_eth_rxdesc rx_desc[RX_BUF_COUNT];
	ATTRS(8) struct lpc32xx_eth_rxstat rx_stat[RX_BUF_COUNT];
	ATTRS(PKTALIGN) u8 rx_buf[RX_BUF_COUNT*PKTSIZE_ALIGN];
};

/* port device data struct */
struct lpc32xx_eth_device {
	struct eth_device dev;
	struct lpc32xx_eth_registers *regs;
	struct lpc32xx_eth_buffers *bufs;
	bool phy_rmii;
};

#define LPC32XX_ETH_DEVICE_SIZE (sizeof(struct lpc32xx_eth_device))

/* generic macros */
#define to_lpc32xx_eth(_d) container_of(_d, struct lpc32xx_eth_device, dev)

/* timeout for MII polling */
#define MII_TIMEOUT 10000000

/* limits for PHY and register addresses */
#define MII_MAX_REG (MADR_REG_MASK >> MADR_REG_OFFSET)

#define MII_MAX_PHY (MADR_PHY_MASK >> MADR_PHY_OFFSET)

#if defined(CONFIG_PHYLIB) || defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
/*
 * mii_reg_read - miiphy_read callback function.
 *
 * Returns 16bit phy register value, or 0xffff on error
 */
static int mii_reg_read(struct mii_dev *bus, int phy_adr, int devad,
			int reg_ofs)
{
	u16 data = 0;
	struct eth_device *dev = eth_get_dev_by_name(bus->name);
	struct lpc32xx_eth_device *dlpc32xx_eth = to_lpc32xx_eth(dev);
	struct lpc32xx_eth_registers *regs = dlpc32xx_eth->regs;
	u32 mind_reg;
	u32 timeout;

	/* check parameters */
	if (phy_adr > MII_MAX_PHY) {
		printf("%s:%u: Invalid PHY address %d\n",
		       __func__, __LINE__, phy_adr);
		return -EFAULT;
	}
	if (reg_ofs > MII_MAX_REG) {
		printf("%s:%u: Invalid register offset %d\n",
		       __func__, __LINE__, reg_ofs);
		return -EFAULT;
	}

	/* write the phy and reg addressse into the MII address reg */
	writel((phy_adr << MADR_PHY_OFFSET) | (reg_ofs << MADR_REG_OFFSET),
	       &regs->madr);

	/* write 1 to the MII command register to cause a read */
	writel(1, &regs->mcmd);

	/* wait till the MII is not busy */
	timeout = MII_TIMEOUT;
	do {
		/* read MII indicators register */
		mind_reg = readl(&regs->mind);
		if (--timeout == 0)
			break;
	} while (mind_reg & MIND_BUSY);

	/* write 0 to the MII command register to finish the read */
	writel(0, &regs->mcmd);

	if (timeout == 0) {
		printf("%s:%u: MII busy timeout\n", __func__, __LINE__);
		return -EFAULT;
	}

	data = (u16) readl(&regs->mrdd);

	debug("%s:(adr %d, off %d) => %04x\n", __func__, phy_adr,
	      reg_ofs, data);

	return data;
}

/*
 * mii_reg_write - imiiphy_write callback function.
 *
 * Returns 0 if write succeed, -EINVAL on bad parameters
 * -ETIME on timeout
 */
static int mii_reg_write(struct mii_dev *bus, int phy_adr, int devad,
			 int reg_ofs, u16 data)
{
	struct eth_device *dev = eth_get_dev_by_name(bus->name);
	struct lpc32xx_eth_device *dlpc32xx_eth = to_lpc32xx_eth(dev);
	struct lpc32xx_eth_registers *regs = dlpc32xx_eth->regs;
	u32 mind_reg;
	u32 timeout;

	/* check parameters */
	if (phy_adr > MII_MAX_PHY) {
		printf("%s:%u: Invalid PHY address %d\n",
		       __func__, __LINE__, phy_adr);
		return -EFAULT;
	}
	if (reg_ofs > MII_MAX_REG) {
		printf("%s:%u: Invalid register offset %d\n",
		       __func__, __LINE__, reg_ofs);
		return -EFAULT;
	}

	/* write the phy and reg addressse into the MII address reg */
	writel((phy_adr << MADR_PHY_OFFSET) | (reg_ofs << MADR_REG_OFFSET),
	       &regs->madr);

	/* write data to the MII write register */
	writel(data, &regs->mwtd);

	/* wait till the MII is not busy */
	timeout = MII_TIMEOUT;
	do {
		/* read MII indicators register */
		mind_reg = readl(&regs->mind);
		if (--timeout == 0)
			break;
	} while (mind_reg & MIND_BUSY);

	if (timeout == 0) {
		printf("%s:%u: MII busy timeout\n", __func__,
		       __LINE__);
		return -EFAULT;
	}

	/*debug("%s:(adr %d, off %d) <= %04x\n", __func__, phy_adr,
		reg_ofs, data);*/

	return 0;
}
#endif

/*
 * Provide default Ethernet buffers base address if target did not.
 * Locate buffers in SRAM at 0x00001000 to avoid cache issues and
 * maximize throughput.
 */
#if !defined(CONFIG_LPC32XX_ETH_BUFS_BASE)
#define CONFIG_LPC32XX_ETH_BUFS_BASE 0x00001000
#endif

static struct lpc32xx_eth_device lpc32xx_eth = {
	.regs = (struct lpc32xx_eth_registers *)LPC32XX_ETH_BASE,
	.bufs = (struct lpc32xx_eth_buffers *)CONFIG_LPC32XX_ETH_BUFS_BASE,
#if defined(CONFIG_RMII)
	.phy_rmii = true,
#endif
};

#define TX_TIMEOUT 10000

static int lpc32xx_eth_send(struct eth_device *dev, void *dataptr, int datasize)
{
	struct lpc32xx_eth_device *lpc32xx_eth_device =
		container_of(dev, struct lpc32xx_eth_device, dev);
	struct lpc32xx_eth_registers *regs = lpc32xx_eth_device->regs;
	struct lpc32xx_eth_buffers *bufs = lpc32xx_eth_device->bufs;
	int timeout, tx_index;

	/* time out if transmit descriptor array remains full too long */
	timeout = TX_TIMEOUT;
	while ((readl(&regs->status) & STATUS_TXSTATUS) &&
	       (readl(&regs->txconsumeindex)
	       == readl(&regs->txproduceindex))) {
		if (timeout-- == 0)
			return -1;
	}

	/* determine next transmit packet index to use */
	tx_index = readl(&regs->txproduceindex);

	/* set up transmit packet */
	memcpy((void *)&bufs->tx_buf[tx_index * PKTSIZE_ALIGN],
	       (void *)dataptr, datasize);
	writel(TX_CTRL_LAST | ((datasize - 1) & TX_CTRL_TXSIZE),
	       &bufs->tx_desc[tx_index].control);
	writel(0, &bufs->tx_stat[tx_index].statusinfo);

	/* pass transmit packet to DMA engine */
	tx_index = (tx_index + 1) % TX_BUF_COUNT;
	writel(tx_index, &regs->txproduceindex);

	/* transmission succeeded */
	return 0;
}

#define RX_TIMEOUT 1000000

static int lpc32xx_eth_recv(struct eth_device *dev)
{
	struct lpc32xx_eth_device *lpc32xx_eth_device =
		container_of(dev, struct lpc32xx_eth_device, dev);
	struct lpc32xx_eth_registers *regs = lpc32xx_eth_device->regs;
	struct lpc32xx_eth_buffers *bufs = lpc32xx_eth_device->bufs;
	int timeout, rx_index;

	/* time out if receive descriptor array remains empty too long */
	timeout = RX_TIMEOUT;
	while (readl(&regs->rxproduceindex) == readl(&regs->rxconsumeindex)) {
		if (timeout-- == 0)
			return -1;
	}

	/* determine next receive packet index to use */
	rx_index = readl(&regs->rxconsumeindex);

	/* if data was valid, pass it on */
	if (!(bufs->rx_stat[rx_index].statusinfo & RX_STAT_ERRORS)) {
		net_process_received_packet(
			&(bufs->rx_buf[rx_index * PKTSIZE_ALIGN]),
			(bufs->rx_stat[rx_index].statusinfo
			 & RX_STAT_RXSIZE) + 1);
	}

	/* pass receive slot back to DMA engine */
	rx_index = (rx_index + 1) % RX_BUF_COUNT;
	writel(rx_index, &regs->rxconsumeindex);

	/* reception successful */
	return 0;
}

static int lpc32xx_eth_write_hwaddr(struct eth_device *dev)
{
	struct lpc32xx_eth_device *lpc32xx_eth_device =
		container_of(dev, struct lpc32xx_eth_device, dev);
	struct lpc32xx_eth_registers *regs = lpc32xx_eth_device->regs;

	/* Save station address */
	writel((unsigned long) (dev->enetaddr[0] |
		(dev->enetaddr[1] << 8)), &regs->sa2);
	writel((unsigned long) (dev->enetaddr[2] |
		(dev->enetaddr[3] << 8)), &regs->sa1);
	writel((unsigned long) (dev->enetaddr[4] |
		(dev->enetaddr[5] << 8)), &regs->sa0);

	return 0;
}

static int lpc32xx_eth_init(struct eth_device *dev)
{
	struct lpc32xx_eth_device *lpc32xx_eth_device =
		container_of(dev, struct lpc32xx_eth_device, dev);
	struct lpc32xx_eth_registers *regs = lpc32xx_eth_device->regs;
	struct lpc32xx_eth_buffers *bufs = lpc32xx_eth_device->bufs;
	int index;

	/* Initial MAC initialization */
	writel(MAC1_PASS_ALL_RX_FRAMES, &regs->mac1);
	writel(MAC2_PAD_CRC_ENABLE | MAC2_CRC_ENABLE, &regs->mac2);
	writel(PKTSIZE_ALIGN, &regs->maxf);

	/* Retries: 15 (0xF). Collision window: 57 (0x37). */
	writel(0x370F, &regs->clrt);

	/* Set IP gap pt 2 to default 0x12 but pt 1 to non-default 0 */
	writel(0x0012, &regs->ipgr);

	/* pass runt (smaller than 64 bytes) frames */
	if (lpc32xx_eth_device->phy_rmii)
		writel(COMMAND_PASSRUNTFRAME | COMMAND_RMII, &regs->command);
	else
		writel(COMMAND_PASSRUNTFRAME, &regs->command);

	/* Configure Full/Half Duplex mode */
	if (miiphy_duplex(dev->name, CONFIG_PHY_ADDR) == FULL) {
		setbits_le32(&regs->mac2, MAC2_FULL_DUPLEX);
		setbits_le32(&regs->command, COMMAND_FULL_DUPLEX);
		writel(0x15, &regs->ipgt);
	} else {
		writel(0x12, &regs->ipgt);
	}

	/* Configure 100MBit/10MBit mode */
	if (miiphy_speed(dev->name, CONFIG_PHY_ADDR) == _100BASET)
		writel(SUPP_SPEED, &regs->supp);
	else
		writel(0, &regs->supp);

	/* Save station address */
	writel((unsigned long) (dev->enetaddr[0] |
		(dev->enetaddr[1] << 8)), &regs->sa2);
	writel((unsigned long) (dev->enetaddr[2] |
		(dev->enetaddr[3] << 8)), &regs->sa1);
	writel((unsigned long) (dev->enetaddr[4] |
		(dev->enetaddr[5] << 8)), &regs->sa0);

	/* set up transmit buffers */
	for (index = 0; index < TX_BUF_COUNT; index++) {
		bufs->tx_desc[index].control = 0;
		bufs->tx_stat[index].statusinfo = 0;
	}
	writel((u32)(&bufs->tx_desc), (u32 *)&regs->txdescriptor);
	writel((u32)(&bufs->tx_stat), &regs->txstatus);
	writel(TX_BUF_COUNT-1, &regs->txdescriptornumber);

	/* set up receive buffers */
	for (index = 0; index < RX_BUF_COUNT; index++) {
		bufs->rx_desc[index].packet =
			(u32) (bufs->rx_buf+index*PKTSIZE_ALIGN);
		bufs->rx_desc[index].control = PKTSIZE_ALIGN - 1;
		bufs->rx_stat[index].statusinfo = 0;
		bufs->rx_stat[index].statushashcrc = 0;
	}
	writel((u32)(&bufs->rx_desc), &regs->rxdescriptor);
	writel((u32)(&bufs->rx_stat), &regs->rxstatus);
	writel(RX_BUF_COUNT-1, &regs->rxdescriptornumber);

	/* set up transmit buffers */
	for (index = 0; index < TX_BUF_COUNT; index++)
		bufs->tx_desc[index].packet =
			(u32)(bufs->tx_buf + index * PKTSIZE_ALIGN);

	/* Enable broadcast and matching address packets */
	writel(RXFILTERCTRL_ACCEPTBROADCAST |
		RXFILTERCTRL_ACCEPTPERFECT, &regs->rxfilterctrl);

	/* Clear and disable interrupts */
	writel(0xFFFF, &regs->intclear);
	writel(0, &regs->intenable);

	/* Enable receive and transmit mode of MAC ethernet core */
	setbits_le32(&regs->command, COMMAND_RXENABLE | COMMAND_TXENABLE);
	setbits_le32(&regs->mac1, MAC1_RECV_ENABLE);

	/*
	 * Perform a 'dummy' first send to work around Ethernet.1
	 * erratum (see ES_LPC3250 rev. 9 dated 1 June 2011).
	 * Use zeroed "index" variable as the dummy.
	 */

	index = 0;
	lpc32xx_eth_send(dev, &index, 4);

	return 0;
}

static int lpc32xx_eth_halt(struct eth_device *dev)
{
	struct lpc32xx_eth_device *lpc32xx_eth_device =
		container_of(dev, struct lpc32xx_eth_device, dev);
	struct lpc32xx_eth_registers *regs = lpc32xx_eth_device->regs;

	/* Reset all MAC logic */
	writel(MAC1_RESETS, &regs->mac1);
	writel(COMMAND_RESETS, &regs->command);
	/* Let reset condition settle */
	udelay(2000);

	return 0;
}

#if defined(CONFIG_PHYLIB)
int lpc32xx_eth_phylib_init(struct eth_device *dev, int phyid)
{
	struct lpc32xx_eth_device *lpc32xx_eth_device =
		container_of(dev, struct lpc32xx_eth_device, dev);
	struct mii_dev *bus;
	struct phy_device *phydev;
	int ret;

	bus = mdio_alloc();
	if (!bus) {
		printf("mdio_alloc failed\n");
		return -ENOMEM;
	}
	bus->read = mii_reg_read;
	bus->write = mii_reg_write;
	strcpy(bus->name, dev->name);

	ret = mdio_register(bus);
	if (ret) {
		printf("mdio_register failed\n");
		free(bus);
		return -ENOMEM;
	}

	if (lpc32xx_eth_device->phy_rmii)
		phydev = phy_connect(bus, phyid, dev, PHY_INTERFACE_MODE_RMII);
	else
		phydev = phy_connect(bus, phyid, dev, PHY_INTERFACE_MODE_MII);

	if (!phydev) {
		printf("phy_connect failed\n");
		return -ENODEV;
	}

	phy_config(phydev);
	phy_startup(phydev);

	return 0;
}
#endif

int lpc32xx_eth_initialize(bd_t *bis)
{
	struct eth_device *dev = &lpc32xx_eth.dev;
	struct lpc32xx_eth_registers *regs = lpc32xx_eth.regs;

	/*
	 * Set RMII management clock rate. With HCLK at 104 MHz and
	 * a divider of 28, this will be 3.72 MHz.
	 */
	writel(MCFG_RESET_MII_MGMT, &regs->mcfg);
	writel(MCFG_CLOCK_SELECT_DIV28, &regs->mcfg);

	/* Reset all MAC logic */
	writel(MAC1_RESETS, &regs->mac1);
	writel(COMMAND_RESETS, &regs->command);

	/* wait 10 ms for the whole I/F to reset */
	udelay(10000);

	/* must be less than sizeof(dev->name) */
	strcpy(dev->name, "eth0");

	dev->init = (void *)lpc32xx_eth_init;
	dev->halt = (void *)lpc32xx_eth_halt;
	dev->send = (void *)lpc32xx_eth_send;
	dev->recv = (void *)lpc32xx_eth_recv;
	dev->write_hwaddr = (void *)lpc32xx_eth_write_hwaddr;

	/* Release SOFT reset to let MII talk to PHY */
	clrbits_le32(&regs->mac1, MAC1_SOFT_RESET);

	/* register driver before talking to phy */
	eth_register(dev);

#if defined(CONFIG_PHYLIB)
	lpc32xx_eth_phylib_init(dev, CONFIG_PHY_ADDR);
#elif defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	int retval;
	struct mii_dev *mdiodev = mdio_alloc();
	if (!mdiodev)
		return -ENOMEM;
	strncpy(mdiodev->name, dev->name, MDIO_NAME_LEN);
	mdiodev->read = mii_reg_read;
	mdiodev->write = mii_reg_write;

	retval = mdio_register(mdiodev);
	if (retval < 0)
		return retval;
#endif

	return 0;
}
