// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 Ilya Yanok, Emcraft Systems Ltd <yanok@emcraft.com>
 * (C) Copyright 2008,2009 Eric Jarrige <eric.jarrige@armadeus.org>
 * (C) Copyright 2008 Armadeus Systems nc
 * (C) Copyright 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 * (C) Copyright 2007 Pengutronix, Juergen Beisert <j.beisert@pengutronix.de>
 */

#include <common.h>
#include <dm.h>
#include <environment.h>
#include <malloc.h>
#include <memalign.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <power/regulator.h>

#include <asm/io.h>
#include <linux/errno.h>
#include <linux/compiler.h>

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/sys_proto.h>
#include <asm-generic/gpio.h>

#include "fec_mxc.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Timeout the transfer after 5 mS. This is usually a bit more, since
 * the code in the tightloops this timeout is used in adds some overhead.
 */
#define FEC_XFER_TIMEOUT	5000

/*
 * The standard 32-byte DMA alignment does not work on mx6solox, which requires
 * 64-byte alignment in the DMA RX FEC buffer.
 * Introduce the FEC_DMA_RX_MINALIGN which can cover mx6solox needs and also
 * satisfies the alignment on other SoCs (32-bytes)
 */
#define FEC_DMA_RX_MINALIGN	64

#ifndef CONFIG_MII
#error "CONFIG_MII has to be defined!"
#endif

#ifndef CONFIG_FEC_XCV_TYPE
#define CONFIG_FEC_XCV_TYPE MII100
#endif

/*
 * The i.MX28 operates with packets in big endian. We need to swap them before
 * sending and after receiving.
 */
#ifdef CONFIG_MX28
#define CONFIG_FEC_MXC_SWAP_PACKET
#endif

#define RXDESC_PER_CACHELINE (ARCH_DMA_MINALIGN/sizeof(struct fec_bd))

/* Check various alignment issues at compile time */
#if ((ARCH_DMA_MINALIGN < 16) || (ARCH_DMA_MINALIGN % 16 != 0))
#error "ARCH_DMA_MINALIGN must be multiple of 16!"
#endif

#if ((PKTALIGN < ARCH_DMA_MINALIGN) || \
	(PKTALIGN % ARCH_DMA_MINALIGN != 0))
#error "PKTALIGN must be multiple of ARCH_DMA_MINALIGN!"
#endif

#undef DEBUG

#ifdef CONFIG_FEC_MXC_SWAP_PACKET
static void swap_packet(uint32_t *packet, int length)
{
	int i;

	for (i = 0; i < DIV_ROUND_UP(length, 4); i++)
		packet[i] = __swab32(packet[i]);
}
#endif

/* MII-interface related functions */
static int fec_mdio_read(struct ethernet_regs *eth, uint8_t phyaddr,
		uint8_t regaddr)
{
	uint32_t reg;		/* convenient holder for the PHY register */
	uint32_t phy;		/* convenient holder for the PHY */
	uint32_t start;
	int val;

	/*
	 * reading from any PHY's register is done by properly
	 * programming the FEC's MII data register.
	 */
	writel(FEC_IEVENT_MII, &eth->ievent);
	reg = regaddr << FEC_MII_DATA_RA_SHIFT;
	phy = phyaddr << FEC_MII_DATA_PA_SHIFT;

	writel(FEC_MII_DATA_ST | FEC_MII_DATA_OP_RD | FEC_MII_DATA_TA |
			phy | reg, &eth->mii_data);

	/* wait for the related interrupt */
	start = get_timer(0);
	while (!(readl(&eth->ievent) & FEC_IEVENT_MII)) {
		if (get_timer(start) > (CONFIG_SYS_HZ / 1000)) {
			printf("Read MDIO failed...\n");
			return -1;
		}
	}

	/* clear mii interrupt bit */
	writel(FEC_IEVENT_MII, &eth->ievent);

	/* it's now safe to read the PHY's register */
	val = (unsigned short)readl(&eth->mii_data);
	debug("%s: phy: %02x reg:%02x val:%#x\n", __func__, phyaddr,
	      regaddr, val);
	return val;
}

static int fec_get_clk_rate(void *udev, int idx)
{
#if IS_ENABLED(CONFIG_IMX8)
	struct fec_priv *fec;
	struct udevice *dev;
	int ret;

	dev = udev;
	if (!dev) {
		ret = uclass_get_device(UCLASS_ETH, idx, &dev);
		if (ret < 0) {
			debug("Can't get FEC udev: %d\n", ret);
			return ret;
		}
	}

	fec = dev_get_priv(dev);
	if (fec)
		return fec->clk_rate;

	return -EINVAL;
#else
	return imx_get_fecclk();
#endif
}

static void fec_mii_setspeed(struct ethernet_regs *eth)
{
	/*
	 * Set MII_SPEED = (1/(mii_speed * 2)) * System Clock
	 * and do not drop the Preamble.
	 *
	 * The i.MX28 and i.MX6 types have another field in the MSCR (aka
	 * MII_SPEED) register that defines the MDIO output hold time. Earlier
	 * versions are RAZ there, so just ignore the difference and write the
	 * register always.
	 * The minimal hold time according to IEE802.3 (clause 22) is 10 ns.
	 * HOLDTIME + 1 is the number of clk cycles the fec is holding the
	 * output.
	 * The HOLDTIME bitfield takes values between 0 and 7 (inclusive).
	 * Given that ceil(clkrate / 5000000) <= 64, the calculation for
	 * holdtime cannot result in a value greater than 3.
	 */
	u32 pclk;
	u32 speed;
	u32 hold;
	int ret;

	ret = fec_get_clk_rate(NULL, 0);
	if (ret < 0) {
		printf("Can't find FEC0 clk rate: %d\n", ret);
		return;
	}
	pclk = ret;
	speed = DIV_ROUND_UP(pclk, 5000000);
	hold = DIV_ROUND_UP(pclk, 100000000) - 1;

#ifdef FEC_QUIRK_ENET_MAC
	speed--;
#endif
	writel(speed << 1 | hold << 8, &eth->mii_speed);
	debug("%s: mii_speed %08x\n", __func__, readl(&eth->mii_speed));
}

static int fec_mdio_write(struct ethernet_regs *eth, uint8_t phyaddr,
		uint8_t regaddr, uint16_t data)
{
	uint32_t reg;		/* convenient holder for the PHY register */
	uint32_t phy;		/* convenient holder for the PHY */
	uint32_t start;

	reg = regaddr << FEC_MII_DATA_RA_SHIFT;
	phy = phyaddr << FEC_MII_DATA_PA_SHIFT;

	writel(FEC_MII_DATA_ST | FEC_MII_DATA_OP_WR |
		FEC_MII_DATA_TA | phy | reg | data, &eth->mii_data);

	/* wait for the MII interrupt */
	start = get_timer(0);
	while (!(readl(&eth->ievent) & FEC_IEVENT_MII)) {
		if (get_timer(start) > (CONFIG_SYS_HZ / 1000)) {
			printf("Write MDIO failed...\n");
			return -1;
		}
	}

	/* clear MII interrupt bit */
	writel(FEC_IEVENT_MII, &eth->ievent);
	debug("%s: phy: %02x reg:%02x val:%#x\n", __func__, phyaddr,
	      regaddr, data);

	return 0;
}

static int fec_phy_read(struct mii_dev *bus, int phyaddr, int dev_addr,
			int regaddr)
{
	return fec_mdio_read(bus->priv, phyaddr, regaddr);
}

static int fec_phy_write(struct mii_dev *bus, int phyaddr, int dev_addr,
			 int regaddr, u16 data)
{
	return fec_mdio_write(bus->priv, phyaddr, regaddr, data);
}

#ifndef CONFIG_PHYLIB
static int miiphy_restart_aneg(struct eth_device *dev)
{
	int ret = 0;
#if !defined(CONFIG_FEC_MXC_NO_ANEG)
	struct fec_priv *fec = (struct fec_priv *)dev->priv;
	struct ethernet_regs *eth = fec->bus->priv;

	/*
	 * Wake up from sleep if necessary
	 * Reset PHY, then delay 300ns
	 */
#ifdef CONFIG_MX27
	fec_mdio_write(eth, fec->phy_id, MII_DCOUNTER, 0x00FF);
#endif
	fec_mdio_write(eth, fec->phy_id, MII_BMCR, BMCR_RESET);
	udelay(1000);

	/* Set the auto-negotiation advertisement register bits */
	fec_mdio_write(eth, fec->phy_id, MII_ADVERTISE,
		       LPA_100FULL | LPA_100HALF | LPA_10FULL |
		       LPA_10HALF | PHY_ANLPAR_PSB_802_3);
	fec_mdio_write(eth, fec->phy_id, MII_BMCR,
		       BMCR_ANENABLE | BMCR_ANRESTART);

	if (fec->mii_postcall)
		ret = fec->mii_postcall(fec->phy_id);

#endif
	return ret;
}

#ifndef CONFIG_FEC_FIXED_SPEED
static int miiphy_wait_aneg(struct eth_device *dev)
{
	uint32_t start;
	int status;
	struct fec_priv *fec = (struct fec_priv *)dev->priv;
	struct ethernet_regs *eth = fec->bus->priv;

	/* Wait for AN completion */
	start = get_timer(0);
	do {
		if (get_timer(start) > (CONFIG_SYS_HZ * 5)) {
			printf("%s: Autonegotiation timeout\n", dev->name);
			return -1;
		}

		status = fec_mdio_read(eth, fec->phy_id, MII_BMSR);
		if (status < 0) {
			printf("%s: Autonegotiation failed. status: %d\n",
			       dev->name, status);
			return -1;
		}
	} while (!(status & BMSR_LSTATUS));

	return 0;
}
#endif /* CONFIG_FEC_FIXED_SPEED */
#endif

static int fec_rx_task_enable(struct fec_priv *fec)
{
	writel(FEC_R_DES_ACTIVE_RDAR, &fec->eth->r_des_active);
	return 0;
}

static int fec_rx_task_disable(struct fec_priv *fec)
{
	return 0;
}

static int fec_tx_task_enable(struct fec_priv *fec)
{
	writel(FEC_X_DES_ACTIVE_TDAR, &fec->eth->x_des_active);
	return 0;
}

static int fec_tx_task_disable(struct fec_priv *fec)
{
	return 0;
}

/**
 * Initialize receive task's buffer descriptors
 * @param[in] fec all we know about the device yet
 * @param[in] count receive buffer count to be allocated
 * @param[in] dsize desired size of each receive buffer
 * @return 0 on success
 *
 * Init all RX descriptors to default values.
 */
static void fec_rbd_init(struct fec_priv *fec, int count, int dsize)
{
	uint32_t size;
	ulong data;
	int i;

	/*
	 * Reload the RX descriptors with default values and wipe
	 * the RX buffers.
	 */
	size = roundup(dsize, ARCH_DMA_MINALIGN);
	for (i = 0; i < count; i++) {
		data = fec->rbd_base[i].data_pointer;
		memset((void *)data, 0, dsize);
		flush_dcache_range(data, data + size);

		fec->rbd_base[i].status = FEC_RBD_EMPTY;
		fec->rbd_base[i].data_length = 0;
	}

	/* Mark the last RBD to close the ring. */
	fec->rbd_base[i - 1].status = FEC_RBD_WRAP | FEC_RBD_EMPTY;
	fec->rbd_index = 0;

	flush_dcache_range((ulong)fec->rbd_base,
			   (ulong)fec->rbd_base + size);
}

/**
 * Initialize transmit task's buffer descriptors
 * @param[in] fec all we know about the device yet
 *
 * Transmit buffers are created externally. We only have to init the BDs here.\n
 * Note: There is a race condition in the hardware. When only one BD is in
 * use it must be marked with the WRAP bit to use it for every transmitt.
 * This bit in combination with the READY bit results into double transmit
 * of each data buffer. It seems the state machine checks READY earlier then
 * resetting it after the first transfer.
 * Using two BDs solves this issue.
 */
static void fec_tbd_init(struct fec_priv *fec)
{
	ulong addr = (ulong)fec->tbd_base;
	unsigned size = roundup(2 * sizeof(struct fec_bd),
				ARCH_DMA_MINALIGN);

	memset(fec->tbd_base, 0, size);
	fec->tbd_base[0].status = 0;
	fec->tbd_base[1].status = FEC_TBD_WRAP;
	fec->tbd_index = 0;
	flush_dcache_range(addr, addr + size);
}

/**
 * Mark the given read buffer descriptor as free
 * @param[in] last 1 if this is the last buffer descriptor in the chain, else 0
 * @param[in] prbd buffer descriptor to mark free again
 */
static void fec_rbd_clean(int last, struct fec_bd *prbd)
{
	unsigned short flags = FEC_RBD_EMPTY;
	if (last)
		flags |= FEC_RBD_WRAP;
	writew(flags, &prbd->status);
	writew(0, &prbd->data_length);
}

static int fec_get_hwaddr(int dev_id, unsigned char *mac)
{
	imx_get_mac_from_fuse(dev_id, mac);
	return !is_valid_ethaddr(mac);
}

#ifdef CONFIG_DM_ETH
static int fecmxc_set_hwaddr(struct udevice *dev)
#else
static int fec_set_hwaddr(struct eth_device *dev)
#endif
{
#ifdef CONFIG_DM_ETH
	struct fec_priv *fec = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	uchar *mac = pdata->enetaddr;
#else
	uchar *mac = dev->enetaddr;
	struct fec_priv *fec = (struct fec_priv *)dev->priv;
#endif

	writel(0, &fec->eth->iaddr1);
	writel(0, &fec->eth->iaddr2);
	writel(0, &fec->eth->gaddr1);
	writel(0, &fec->eth->gaddr2);

	/* Set physical address */
	writel((mac[0] << 24) + (mac[1] << 16) + (mac[2] << 8) + mac[3],
	       &fec->eth->paddr1);
	writel((mac[4] << 24) + (mac[5] << 16) + 0x8808, &fec->eth->paddr2);

	return 0;
}

/* Do initial configuration of the FEC registers */
static void fec_reg_setup(struct fec_priv *fec)
{
	uint32_t rcntrl;

	/* Set interrupt mask register */
	writel(0x00000000, &fec->eth->imask);

	/* Clear FEC-Lite interrupt event register(IEVENT) */
	writel(0xffffffff, &fec->eth->ievent);

	/* Set FEC-Lite receive control register(R_CNTRL): */

	/* Start with frame length = 1518, common for all modes. */
	rcntrl = PKTSIZE << FEC_RCNTRL_MAX_FL_SHIFT;
	if (fec->xcv_type != SEVENWIRE)		/* xMII modes */
		rcntrl |= FEC_RCNTRL_FCE | FEC_RCNTRL_MII_MODE;
	if (fec->xcv_type == RGMII)
		rcntrl |= FEC_RCNTRL_RGMII;
	else if (fec->xcv_type == RMII)
		rcntrl |= FEC_RCNTRL_RMII;

	writel(rcntrl, &fec->eth->r_cntrl);
}

/**
 * Start the FEC engine
 * @param[in] dev Our device to handle
 */
#ifdef CONFIG_DM_ETH
static int fec_open(struct udevice *dev)
#else
static int fec_open(struct eth_device *edev)
#endif
{
#ifdef CONFIG_DM_ETH
	struct fec_priv *fec = dev_get_priv(dev);
#else
	struct fec_priv *fec = (struct fec_priv *)edev->priv;
#endif
	int speed;
	ulong addr, size;
	int i;

	debug("fec_open: fec_open(dev)\n");
	/* full-duplex, heartbeat disabled */
	writel(1 << 2, &fec->eth->x_cntrl);
	fec->rbd_index = 0;

	/* Invalidate all descriptors */
	for (i = 0; i < FEC_RBD_NUM - 1; i++)
		fec_rbd_clean(0, &fec->rbd_base[i]);
	fec_rbd_clean(1, &fec->rbd_base[i]);

	/* Flush the descriptors into RAM */
	size = roundup(FEC_RBD_NUM * sizeof(struct fec_bd),
			ARCH_DMA_MINALIGN);
	addr = (ulong)fec->rbd_base;
	flush_dcache_range(addr, addr + size);

#ifdef FEC_QUIRK_ENET_MAC
	/* Enable ENET HW endian SWAP */
	writel(readl(&fec->eth->ecntrl) | FEC_ECNTRL_DBSWAP,
	       &fec->eth->ecntrl);
	/* Enable ENET store and forward mode */
	writel(readl(&fec->eth->x_wmrk) | FEC_X_WMRK_STRFWD,
	       &fec->eth->x_wmrk);
#endif
	/* Enable FEC-Lite controller */
	writel(readl(&fec->eth->ecntrl) | FEC_ECNTRL_ETHER_EN,
	       &fec->eth->ecntrl);

#if defined(CONFIG_MX25) || defined(CONFIG_MX53) || defined(CONFIG_MX6SL)
	udelay(100);

	/* setup the MII gasket for RMII mode */
	/* disable the gasket */
	writew(0, &fec->eth->miigsk_enr);

	/* wait for the gasket to be disabled */
	while (readw(&fec->eth->miigsk_enr) & MIIGSK_ENR_READY)
		udelay(2);

	/* configure gasket for RMII, 50 MHz, no loopback, and no echo */
	writew(MIIGSK_CFGR_IF_MODE_RMII, &fec->eth->miigsk_cfgr);

	/* re-enable the gasket */
	writew(MIIGSK_ENR_EN, &fec->eth->miigsk_enr);

	/* wait until MII gasket is ready */
	int max_loops = 10;
	while ((readw(&fec->eth->miigsk_enr) & MIIGSK_ENR_READY) == 0) {
		if (--max_loops <= 0) {
			printf("WAIT for MII Gasket ready timed out\n");
			break;
		}
	}
#endif

#ifdef CONFIG_PHYLIB
	{
		/* Start up the PHY */
		int ret = phy_startup(fec->phydev);

		if (ret) {
			printf("Could not initialize PHY %s\n",
			       fec->phydev->dev->name);
			return ret;
		}
		speed = fec->phydev->speed;
	}
#elif CONFIG_FEC_FIXED_SPEED
	speed = CONFIG_FEC_FIXED_SPEED;
#else
	miiphy_wait_aneg(edev);
	speed = miiphy_speed(edev->name, fec->phy_id);
	miiphy_duplex(edev->name, fec->phy_id);
#endif

#ifdef FEC_QUIRK_ENET_MAC
	{
		u32 ecr = readl(&fec->eth->ecntrl) & ~FEC_ECNTRL_SPEED;
		u32 rcr = readl(&fec->eth->r_cntrl) & ~FEC_RCNTRL_RMII_10T;
		if (speed == _1000BASET)
			ecr |= FEC_ECNTRL_SPEED;
		else if (speed != _100BASET)
			rcr |= FEC_RCNTRL_RMII_10T;
		writel(ecr, &fec->eth->ecntrl);
		writel(rcr, &fec->eth->r_cntrl);
	}
#endif
	debug("%s:Speed=%i\n", __func__, speed);

	/* Enable SmartDMA receive task */
	fec_rx_task_enable(fec);

	udelay(100000);
	return 0;
}

#ifdef CONFIG_DM_ETH
static int fecmxc_init(struct udevice *dev)
#else
static int fec_init(struct eth_device *dev, bd_t *bd)
#endif
{
#ifdef CONFIG_DM_ETH
	struct fec_priv *fec = dev_get_priv(dev);
#else
	struct fec_priv *fec = (struct fec_priv *)dev->priv;
#endif
	u8 *mib_ptr = (uint8_t *)&fec->eth->rmon_t_drop;
	u8 *i;
	ulong addr;

	/* Initialize MAC address */
#ifdef CONFIG_DM_ETH
	fecmxc_set_hwaddr(dev);
#else
	fec_set_hwaddr(dev);
#endif

	/* Setup transmit descriptors, there are two in total. */
	fec_tbd_init(fec);

	/* Setup receive descriptors. */
	fec_rbd_init(fec, FEC_RBD_NUM, FEC_MAX_PKT_SIZE);

	fec_reg_setup(fec);

	if (fec->xcv_type != SEVENWIRE)
		fec_mii_setspeed(fec->bus->priv);

	/* Set Opcode/Pause Duration Register */
	writel(0x00010020, &fec->eth->op_pause);	/* FIXME 0xffff0020; */
	writel(0x2, &fec->eth->x_wmrk);

	/* Set multicast address filter */
	writel(0x00000000, &fec->eth->gaddr1);
	writel(0x00000000, &fec->eth->gaddr2);

	/* Do not access reserved register */
	if (!is_mx6ul() && !is_mx6ull() && !is_imx8() && !is_imx8m()) {
		/* clear MIB RAM */
		for (i = mib_ptr; i <= mib_ptr + 0xfc; i += 4)
			writel(0, i);

		/* FIFO receive start register */
		writel(0x520, &fec->eth->r_fstart);
	}

	/* size and address of each buffer */
	writel(FEC_MAX_PKT_SIZE, &fec->eth->emrbr);

	addr = (ulong)fec->tbd_base;
	writel((uint32_t)addr, &fec->eth->etdsr);

	addr = (ulong)fec->rbd_base;
	writel((uint32_t)addr, &fec->eth->erdsr);

#ifndef CONFIG_PHYLIB
	if (fec->xcv_type != SEVENWIRE)
		miiphy_restart_aneg(dev);
#endif
	fec_open(dev);
	return 0;
}

/**
 * Halt the FEC engine
 * @param[in] dev Our device to handle
 */
#ifdef CONFIG_DM_ETH
static void fecmxc_halt(struct udevice *dev)
#else
static void fec_halt(struct eth_device *dev)
#endif
{
#ifdef CONFIG_DM_ETH
	struct fec_priv *fec = dev_get_priv(dev);
#else
	struct fec_priv *fec = (struct fec_priv *)dev->priv;
#endif
	int counter = 0xffff;

	/* issue graceful stop command to the FEC transmitter if necessary */
	writel(FEC_TCNTRL_GTS | readl(&fec->eth->x_cntrl),
	       &fec->eth->x_cntrl);

	debug("eth_halt: wait for stop regs\n");
	/* wait for graceful stop to register */
	while ((counter--) && (!(readl(&fec->eth->ievent) & FEC_IEVENT_GRA)))
		udelay(1);

	/* Disable SmartDMA tasks */
	fec_tx_task_disable(fec);
	fec_rx_task_disable(fec);

	/*
	 * Disable the Ethernet Controller
	 * Note: this will also reset the BD index counter!
	 */
	writel(readl(&fec->eth->ecntrl) & ~FEC_ECNTRL_ETHER_EN,
	       &fec->eth->ecntrl);
	fec->rbd_index = 0;
	fec->tbd_index = 0;
	debug("eth_halt: done\n");
}

/**
 * Transmit one frame
 * @param[in] dev Our ethernet device to handle
 * @param[in] packet Pointer to the data to be transmitted
 * @param[in] length Data count in bytes
 * @return 0 on success
 */
#ifdef CONFIG_DM_ETH
static int fecmxc_send(struct udevice *dev, void *packet, int length)
#else
static int fec_send(struct eth_device *dev, void *packet, int length)
#endif
{
	unsigned int status;
	u32 size;
	ulong addr, end;
	int timeout = FEC_XFER_TIMEOUT;
	int ret = 0;

	/*
	 * This routine transmits one frame.  This routine only accepts
	 * 6-byte Ethernet addresses.
	 */
#ifdef CONFIG_DM_ETH
	struct fec_priv *fec = dev_get_priv(dev);
#else
	struct fec_priv *fec = (struct fec_priv *)dev->priv;
#endif

	/*
	 * Check for valid length of data.
	 */
	if ((length > 1500) || (length <= 0)) {
		printf("Payload (%d) too large\n", length);
		return -1;
	}

	/*
	 * Setup the transmit buffer. We are always using the first buffer for
	 * transmission, the second will be empty and only used to stop the DMA
	 * engine. We also flush the packet to RAM here to avoid cache trouble.
	 */
#ifdef CONFIG_FEC_MXC_SWAP_PACKET
	swap_packet((uint32_t *)packet, length);
#endif

	addr = (ulong)packet;
	end = roundup(addr + length, ARCH_DMA_MINALIGN);
	addr &= ~(ARCH_DMA_MINALIGN - 1);
	flush_dcache_range(addr, end);

	writew(length, &fec->tbd_base[fec->tbd_index].data_length);
	writel((uint32_t)addr, &fec->tbd_base[fec->tbd_index].data_pointer);

	/*
	 * update BD's status now
	 * This block:
	 * - is always the last in a chain (means no chain)
	 * - should transmitt the CRC
	 * - might be the last BD in the list, so the address counter should
	 *   wrap (-> keep the WRAP flag)
	 */
	status = readw(&fec->tbd_base[fec->tbd_index].status) & FEC_TBD_WRAP;
	status |= FEC_TBD_LAST | FEC_TBD_TC | FEC_TBD_READY;
	writew(status, &fec->tbd_base[fec->tbd_index].status);

	/*
	 * Flush data cache. This code flushes both TX descriptors to RAM.
	 * After this code, the descriptors will be safely in RAM and we
	 * can start DMA.
	 */
	size = roundup(2 * sizeof(struct fec_bd), ARCH_DMA_MINALIGN);
	addr = (ulong)fec->tbd_base;
	flush_dcache_range(addr, addr + size);

	/*
	 * Below we read the DMA descriptor's last four bytes back from the
	 * DRAM. This is important in order to make sure that all WRITE
	 * operations on the bus that were triggered by previous cache FLUSH
	 * have completed.
	 *
	 * Otherwise, on MX28, it is possible to observe a corruption of the
	 * DMA descriptors. Please refer to schematic "Figure 1-2" in MX28RM
	 * for the bus structure of MX28. The scenario is as follows:
	 *
	 * 1) ARM core triggers a series of WRITEs on the AHB_ARB2 bus going
	 *    to DRAM due to flush_dcache_range()
	 * 2) ARM core writes the FEC registers via AHB_ARB2
	 * 3) FEC DMA starts reading/writing from/to DRAM via AHB_ARB3
	 *
	 * Note that 2) does sometimes finish before 1) due to reordering of
	 * WRITE accesses on the AHB bus, therefore triggering 3) before the
	 * DMA descriptor is fully written into DRAM. This results in occasional
	 * corruption of the DMA descriptor.
	 */
	readl(addr + size - 4);

	/* Enable SmartDMA transmit task */
	fec_tx_task_enable(fec);

	/*
	 * Wait until frame is sent. On each turn of the wait cycle, we must
	 * invalidate data cache to see what's really in RAM. Also, we need
	 * barrier here.
	 */
	while (--timeout) {
		if (!(readl(&fec->eth->x_des_active) & FEC_X_DES_ACTIVE_TDAR))
			break;
	}

	if (!timeout) {
		ret = -EINVAL;
		goto out;
	}

	/*
	 * The TDAR bit is cleared when the descriptors are all out from TX
	 * but on mx6solox we noticed that the READY bit is still not cleared
	 * right after TDAR.
	 * These are two distinct signals, and in IC simulation, we found that
	 * TDAR always gets cleared prior than the READY bit of last BD becomes
	 * cleared.
	 * In mx6solox, we use a later version of FEC IP. It looks like that
	 * this intrinsic behaviour of TDAR bit has changed in this newer FEC
	 * version.
	 *
	 * Fix this by polling the READY bit of BD after the TDAR polling,
	 * which covers the mx6solox case and does not harm the other SoCs.
	 */
	timeout = FEC_XFER_TIMEOUT;
	while (--timeout) {
		invalidate_dcache_range(addr, addr + size);
		if (!(readw(&fec->tbd_base[fec->tbd_index].status) &
		    FEC_TBD_READY))
			break;
	}

	if (!timeout)
		ret = -EINVAL;

out:
	debug("fec_send: status 0x%x index %d ret %i\n",
	      readw(&fec->tbd_base[fec->tbd_index].status),
	      fec->tbd_index, ret);
	/* for next transmission use the other buffer */
	if (fec->tbd_index)
		fec->tbd_index = 0;
	else
		fec->tbd_index = 1;

	return ret;
}

/**
 * Pull one frame from the card
 * @param[in] dev Our ethernet device to handle
 * @return Length of packet read
 */
#ifdef CONFIG_DM_ETH
static int fecmxc_recv(struct udevice *dev, int flags, uchar **packetp)
#else
static int fec_recv(struct eth_device *dev)
#endif
{
#ifdef CONFIG_DM_ETH
	struct fec_priv *fec = dev_get_priv(dev);
#else
	struct fec_priv *fec = (struct fec_priv *)dev->priv;
#endif
	struct fec_bd *rbd = &fec->rbd_base[fec->rbd_index];
	unsigned long ievent;
	int frame_length, len = 0;
	uint16_t bd_status;
	ulong addr, size, end;
	int i;

#ifdef CONFIG_DM_ETH
	*packetp = memalign(ARCH_DMA_MINALIGN, FEC_MAX_PKT_SIZE);
	if (*packetp == 0) {
		printf("%s: error allocating packetp\n", __func__);
		return -ENOMEM;
	}
#else
	ALLOC_CACHE_ALIGN_BUFFER(uchar, buff, FEC_MAX_PKT_SIZE);
#endif

	/* Check if any critical events have happened */
	ievent = readl(&fec->eth->ievent);
	writel(ievent, &fec->eth->ievent);
	debug("fec_recv: ievent 0x%lx\n", ievent);
	if (ievent & FEC_IEVENT_BABR) {
#ifdef CONFIG_DM_ETH
		fecmxc_halt(dev);
		fecmxc_init(dev);
#else
		fec_halt(dev);
		fec_init(dev, fec->bd);
#endif
		printf("some error: 0x%08lx\n", ievent);
		return 0;
	}
	if (ievent & FEC_IEVENT_HBERR) {
		/* Heartbeat error */
		writel(0x00000001 | readl(&fec->eth->x_cntrl),
		       &fec->eth->x_cntrl);
	}
	if (ievent & FEC_IEVENT_GRA) {
		/* Graceful stop complete */
		if (readl(&fec->eth->x_cntrl) & 0x00000001) {
#ifdef CONFIG_DM_ETH
			fecmxc_halt(dev);
#else
			fec_halt(dev);
#endif
			writel(~0x00000001 & readl(&fec->eth->x_cntrl),
			       &fec->eth->x_cntrl);
#ifdef CONFIG_DM_ETH
			fecmxc_init(dev);
#else
			fec_init(dev, fec->bd);
#endif
		}
	}

	/*
	 * Read the buffer status. Before the status can be read, the data cache
	 * must be invalidated, because the data in RAM might have been changed
	 * by DMA. The descriptors are properly aligned to cachelines so there's
	 * no need to worry they'd overlap.
	 *
	 * WARNING: By invalidating the descriptor here, we also invalidate
	 * the descriptors surrounding this one. Therefore we can NOT change the
	 * contents of this descriptor nor the surrounding ones. The problem is
	 * that in order to mark the descriptor as processed, we need to change
	 * the descriptor. The solution is to mark the whole cache line when all
	 * descriptors in the cache line are processed.
	 */
	addr = (ulong)rbd;
	addr &= ~(ARCH_DMA_MINALIGN - 1);
	size = roundup(sizeof(struct fec_bd), ARCH_DMA_MINALIGN);
	invalidate_dcache_range(addr, addr + size);

	bd_status = readw(&rbd->status);
	debug("fec_recv: status 0x%x\n", bd_status);

	if (!(bd_status & FEC_RBD_EMPTY)) {
		if ((bd_status & FEC_RBD_LAST) && !(bd_status & FEC_RBD_ERR) &&
		    ((readw(&rbd->data_length) - 4) > 14)) {
			/* Get buffer address and size */
			addr = readl(&rbd->data_pointer);
			frame_length = readw(&rbd->data_length) - 4;
			/* Invalidate data cache over the buffer */
			end = roundup(addr + frame_length, ARCH_DMA_MINALIGN);
			addr &= ~(ARCH_DMA_MINALIGN - 1);
			invalidate_dcache_range(addr, end);

			/* Fill the buffer and pass it to upper layers */
#ifdef CONFIG_FEC_MXC_SWAP_PACKET
			swap_packet((uint32_t *)addr, frame_length);
#endif

#ifdef CONFIG_DM_ETH
			memcpy(*packetp, (char *)addr, frame_length);
#else
			memcpy(buff, (char *)addr, frame_length);
			net_process_received_packet(buff, frame_length);
#endif
			len = frame_length;
		} else {
			if (bd_status & FEC_RBD_ERR)
				debug("error frame: 0x%08lx 0x%08x\n",
				      addr, bd_status);
		}

		/*
		 * Free the current buffer, restart the engine and move forward
		 * to the next buffer. Here we check if the whole cacheline of
		 * descriptors was already processed and if so, we mark it free
		 * as whole.
		 */
		size = RXDESC_PER_CACHELINE - 1;
		if ((fec->rbd_index & size) == size) {
			i = fec->rbd_index - size;
			addr = (ulong)&fec->rbd_base[i];
			for (; i <= fec->rbd_index ; i++) {
				fec_rbd_clean(i == (FEC_RBD_NUM - 1),
					      &fec->rbd_base[i]);
			}
			flush_dcache_range(addr,
					   addr + ARCH_DMA_MINALIGN);
		}

		fec_rx_task_enable(fec);
		fec->rbd_index = (fec->rbd_index + 1) % FEC_RBD_NUM;
	}
	debug("fec_recv: stop\n");

	return len;
}

static void fec_set_dev_name(char *dest, int dev_id)
{
	sprintf(dest, (dev_id == -1) ? "FEC" : "FEC%i", dev_id);
}

static int fec_alloc_descs(struct fec_priv *fec)
{
	unsigned int size;
	int i;
	uint8_t *data;
	ulong addr;

	/* Allocate TX descriptors. */
	size = roundup(2 * sizeof(struct fec_bd), ARCH_DMA_MINALIGN);
	fec->tbd_base = memalign(ARCH_DMA_MINALIGN, size);
	if (!fec->tbd_base)
		goto err_tx;

	/* Allocate RX descriptors. */
	size = roundup(FEC_RBD_NUM * sizeof(struct fec_bd), ARCH_DMA_MINALIGN);
	fec->rbd_base = memalign(ARCH_DMA_MINALIGN, size);
	if (!fec->rbd_base)
		goto err_rx;

	memset(fec->rbd_base, 0, size);

	/* Allocate RX buffers. */

	/* Maximum RX buffer size. */
	size = roundup(FEC_MAX_PKT_SIZE, FEC_DMA_RX_MINALIGN);
	for (i = 0; i < FEC_RBD_NUM; i++) {
		data = memalign(FEC_DMA_RX_MINALIGN, size);
		if (!data) {
			printf("%s: error allocating rxbuf %d\n", __func__, i);
			goto err_ring;
		}

		memset(data, 0, size);

		addr = (ulong)data;
		fec->rbd_base[i].data_pointer = (uint32_t)addr;
		fec->rbd_base[i].status = FEC_RBD_EMPTY;
		fec->rbd_base[i].data_length = 0;
		/* Flush the buffer to memory. */
		flush_dcache_range(addr, addr + size);
	}

	/* Mark the last RBD to close the ring. */
	fec->rbd_base[i - 1].status = FEC_RBD_WRAP | FEC_RBD_EMPTY;

	fec->rbd_index = 0;
	fec->tbd_index = 0;

	return 0;

err_ring:
	for (; i >= 0; i--) {
		addr = fec->rbd_base[i].data_pointer;
		free((void *)addr);
	}
	free(fec->rbd_base);
err_rx:
	free(fec->tbd_base);
err_tx:
	return -ENOMEM;
}

static void fec_free_descs(struct fec_priv *fec)
{
	int i;
	ulong addr;

	for (i = 0; i < FEC_RBD_NUM; i++) {
		addr = fec->rbd_base[i].data_pointer;
		free((void *)addr);
	}
	free(fec->rbd_base);
	free(fec->tbd_base);
}

struct mii_dev *fec_get_miibus(ulong base_addr, int dev_id)
{
	struct ethernet_regs *eth = (struct ethernet_regs *)base_addr;
	struct mii_dev *bus;
	int ret;

	bus = mdio_alloc();
	if (!bus) {
		printf("mdio_alloc failed\n");
		return NULL;
	}
	bus->read = fec_phy_read;
	bus->write = fec_phy_write;
	bus->priv = eth;
	fec_set_dev_name(bus->name, dev_id);

	ret = mdio_register(bus);
	if (ret) {
		printf("mdio_register failed\n");
		free(bus);
		return NULL;
	}
	fec_mii_setspeed(eth);
	return bus;
}

#ifndef CONFIG_DM_ETH
#ifdef CONFIG_PHYLIB
int fec_probe(bd_t *bd, int dev_id, uint32_t base_addr,
		struct mii_dev *bus, struct phy_device *phydev)
#else
static int fec_probe(bd_t *bd, int dev_id, uint32_t base_addr,
		struct mii_dev *bus, int phy_id)
#endif
{
	struct eth_device *edev;
	struct fec_priv *fec;
	unsigned char ethaddr[6];
	char mac[16];
	uint32_t start;
	int ret = 0;

	/* create and fill edev struct */
	edev = (struct eth_device *)malloc(sizeof(struct eth_device));
	if (!edev) {
		puts("fec_mxc: not enough malloc memory for eth_device\n");
		ret = -ENOMEM;
		goto err1;
	}

	fec = (struct fec_priv *)malloc(sizeof(struct fec_priv));
	if (!fec) {
		puts("fec_mxc: not enough malloc memory for fec_priv\n");
		ret = -ENOMEM;
		goto err2;
	}

	memset(edev, 0, sizeof(*edev));
	memset(fec, 0, sizeof(*fec));

	ret = fec_alloc_descs(fec);
	if (ret)
		goto err3;

	edev->priv = fec;
	edev->init = fec_init;
	edev->send = fec_send;
	edev->recv = fec_recv;
	edev->halt = fec_halt;
	edev->write_hwaddr = fec_set_hwaddr;

	fec->eth = (struct ethernet_regs *)(ulong)base_addr;
	fec->bd = bd;

	fec->xcv_type = CONFIG_FEC_XCV_TYPE;

	/* Reset chip. */
	writel(readl(&fec->eth->ecntrl) | FEC_ECNTRL_RESET, &fec->eth->ecntrl);
	start = get_timer(0);
	while (readl(&fec->eth->ecntrl) & FEC_ECNTRL_RESET) {
		if (get_timer(start) > (CONFIG_SYS_HZ * 5)) {
			printf("FEC MXC: Timeout resetting chip\n");
			goto err4;
		}
		udelay(10);
	}

	fec_reg_setup(fec);
	fec_set_dev_name(edev->name, dev_id);
	fec->dev_id = (dev_id == -1) ? 0 : dev_id;
	fec->bus = bus;
	fec_mii_setspeed(bus->priv);
#ifdef CONFIG_PHYLIB
	fec->phydev = phydev;
	phy_connect_dev(phydev, edev);
	/* Configure phy */
	phy_config(phydev);
#else
	fec->phy_id = phy_id;
#endif
	eth_register(edev);
	/* only support one eth device, the index number pointed by dev_id */
	edev->index = fec->dev_id;

	if (fec_get_hwaddr(fec->dev_id, ethaddr) == 0) {
		debug("got MAC%d address from fuse: %pM\n", fec->dev_id, ethaddr);
		memcpy(edev->enetaddr, ethaddr, 6);
		if (fec->dev_id)
			sprintf(mac, "eth%daddr", fec->dev_id);
		else
			strcpy(mac, "ethaddr");
		if (!env_get(mac))
			eth_env_set_enetaddr(mac, ethaddr);
	}
	return ret;
err4:
	fec_free_descs(fec);
err3:
	free(fec);
err2:
	free(edev);
err1:
	return ret;
}

int fecmxc_initialize_multi(bd_t *bd, int dev_id, int phy_id, uint32_t addr)
{
	uint32_t base_mii;
	struct mii_dev *bus = NULL;
#ifdef CONFIG_PHYLIB
	struct phy_device *phydev = NULL;
#endif
	int ret;

#ifdef CONFIG_FEC_MXC_MDIO_BASE
	/*
	 * The i.MX28 has two ethernet interfaces, but they are not equal.
	 * Only the first one can access the MDIO bus.
	 */
	base_mii = CONFIG_FEC_MXC_MDIO_BASE;
#else
	base_mii = addr;
#endif
	debug("eth_init: fec_probe(bd, %i, %i) @ %08x\n", dev_id, phy_id, addr);
	bus = fec_get_miibus(base_mii, dev_id);
	if (!bus)
		return -ENOMEM;
#ifdef CONFIG_PHYLIB
	phydev = phy_find_by_mask(bus, 1 << phy_id, PHY_INTERFACE_MODE_RGMII);
	if (!phydev) {
		mdio_unregister(bus);
		free(bus);
		return -ENOMEM;
	}
	ret = fec_probe(bd, dev_id, addr, bus, phydev);
#else
	ret = fec_probe(bd, dev_id, addr, bus, phy_id);
#endif
	if (ret) {
#ifdef CONFIG_PHYLIB
		free(phydev);
#endif
		mdio_unregister(bus);
		free(bus);
	}
	return ret;
}

#ifdef CONFIG_FEC_MXC_PHYADDR
int fecmxc_initialize(bd_t *bd)
{
	return fecmxc_initialize_multi(bd, -1, CONFIG_FEC_MXC_PHYADDR,
			IMX_FEC_BASE);
}
#endif

#ifndef CONFIG_PHYLIB
int fecmxc_register_mii_postcall(struct eth_device *dev, int (*cb)(int))
{
	struct fec_priv *fec = (struct fec_priv *)dev->priv;
	fec->mii_postcall = cb;
	return 0;
}
#endif

#else

static int fecmxc_read_rom_hwaddr(struct udevice *dev)
{
	struct fec_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);

	return fec_get_hwaddr(priv->dev_id, pdata->enetaddr);
}

static int fecmxc_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	if (packet)
		free(packet);

	return 0;
}

static const struct eth_ops fecmxc_ops = {
	.start			= fecmxc_init,
	.send			= fecmxc_send,
	.recv			= fecmxc_recv,
	.free_pkt		= fecmxc_free_pkt,
	.stop			= fecmxc_halt,
	.write_hwaddr		= fecmxc_set_hwaddr,
	.read_rom_hwaddr	= fecmxc_read_rom_hwaddr,
};

static int device_get_phy_addr(struct udevice *dev)
{
	struct ofnode_phandle_args phandle_args;
	int reg;

	if (dev_read_phandle_with_args(dev, "phy-handle", NULL, 0, 0,
				       &phandle_args)) {
		debug("Failed to find phy-handle");
		return -ENODEV;
	}

	reg = ofnode_read_u32_default(phandle_args.node, "reg", 0);

	return reg;
}

static int fec_phy_init(struct fec_priv *priv, struct udevice *dev)
{
	struct phy_device *phydev;
	int addr;

	addr = device_get_phy_addr(dev);
#ifdef CONFIG_FEC_MXC_PHYADDR
	addr = CONFIG_FEC_MXC_PHYADDR;
#endif

	phydev = phy_connect(priv->bus, addr, dev, priv->interface);
	if (!phydev)
		return -ENODEV;

	priv->phydev = phydev;
	phy_config(phydev);

	return 0;
}

#ifdef CONFIG_DM_GPIO
/* FEC GPIO reset */
static void fec_gpio_reset(struct fec_priv *priv)
{
	debug("fec_gpio_reset: fec_gpio_reset(dev)\n");
	if (dm_gpio_is_valid(&priv->phy_reset_gpio)) {
		dm_gpio_set_value(&priv->phy_reset_gpio, 1);
		mdelay(priv->reset_delay);
		dm_gpio_set_value(&priv->phy_reset_gpio, 0);
		if (priv->reset_post_delay)
			mdelay(priv->reset_post_delay);
	}
}
#endif

static int fecmxc_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct fec_priv *priv = dev_get_priv(dev);
	struct mii_dev *bus = NULL;
	uint32_t start;
	int ret;

	if (IS_ENABLED(CONFIG_IMX8)) {
		ret = clk_get_by_name(dev, "ipg", &priv->ipg_clk);
		if (ret < 0) {
			debug("Can't get FEC ipg clk: %d\n", ret);
			return ret;
		}
		ret = clk_enable(&priv->ipg_clk);
		if (ret < 0) {
			debug("Can't enable FEC ipg clk: %d\n", ret);
			return ret;
		}

		priv->clk_rate = clk_get_rate(&priv->ipg_clk);
	}

	ret = fec_alloc_descs(priv);
	if (ret)
		return ret;

#ifdef CONFIG_DM_REGULATOR
	if (priv->phy_supply) {
		ret = regulator_set_enable(priv->phy_supply, true);
		if (ret) {
			printf("%s: Error enabling phy supply\n", dev->name);
			return ret;
		}
	}
#endif

#ifdef CONFIG_DM_GPIO
	fec_gpio_reset(priv);
#endif
	/* Reset chip. */
	writel(readl(&priv->eth->ecntrl) | FEC_ECNTRL_RESET,
	       &priv->eth->ecntrl);
	start = get_timer(0);
	while (readl(&priv->eth->ecntrl) & FEC_ECNTRL_RESET) {
		if (get_timer(start) > (CONFIG_SYS_HZ * 5)) {
			printf("FEC MXC: Timeout reseting chip\n");
			goto err_timeout;
		}
		udelay(10);
	}

	fec_reg_setup(priv);

	priv->dev_id = dev->seq;
#ifdef CONFIG_FEC_MXC_MDIO_BASE
	bus = fec_get_miibus((ulong)CONFIG_FEC_MXC_MDIO_BASE, dev->seq);
#else
	bus = fec_get_miibus((ulong)priv->eth, dev->seq);
#endif
	if (!bus) {
		ret = -ENOMEM;
		goto err_mii;
	}

	priv->bus = bus;
	priv->interface = pdata->phy_interface;
	switch (priv->interface) {
	case PHY_INTERFACE_MODE_MII:
		priv->xcv_type = MII100;
		break;
	case PHY_INTERFACE_MODE_RMII:
		priv->xcv_type = RMII;
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		priv->xcv_type = RGMII;
		break;
	default:
		priv->xcv_type = CONFIG_FEC_XCV_TYPE;
		printf("Unsupported interface type %d defaulting to %d\n",
		       priv->interface, priv->xcv_type);
		break;
	}

	ret = fec_phy_init(priv, dev);
	if (ret)
		goto err_phy;

	return 0;

err_phy:
	mdio_unregister(bus);
	free(bus);
err_mii:
err_timeout:
	fec_free_descs(priv);
	return ret;
}

static int fecmxc_remove(struct udevice *dev)
{
	struct fec_priv *priv = dev_get_priv(dev);

	free(priv->phydev);
	fec_free_descs(priv);
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);

#ifdef CONFIG_DM_REGULATOR
	if (priv->phy_supply)
		regulator_set_enable(priv->phy_supply, false);
#endif

	return 0;
}

static int fecmxc_ofdata_to_platdata(struct udevice *dev)
{
	int ret = 0;
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct fec_priv *priv = dev_get_priv(dev);
	const char *phy_mode;

	pdata->iobase = (phys_addr_t)devfdt_get_addr(dev);
	priv->eth = (struct ethernet_regs *)pdata->iobase;

	pdata->phy_interface = -1;
	phy_mode = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "phy-mode",
			       NULL);
	if (phy_mode)
		pdata->phy_interface = phy_get_interface_by_name(phy_mode);
	if (pdata->phy_interface == -1) {
		debug("%s: Invalid PHY interface '%s'\n", __func__, phy_mode);
		return -EINVAL;
	}

#ifdef CONFIG_DM_REGULATOR
	device_get_supply_regulator(dev, "phy-supply", &priv->phy_supply);
#endif

#ifdef CONFIG_DM_GPIO
	ret = gpio_request_by_name(dev, "phy-reset-gpios", 0,
				   &priv->phy_reset_gpio, GPIOD_IS_OUT);
	if (ret < 0)
		return 0; /* property is optional, don't return error! */

	priv->reset_delay = dev_read_u32_default(dev, "phy-reset-duration", 1);
	if (priv->reset_delay > 1000) {
		printf("FEC MXC: phy reset duration should be <= 1000ms\n");
		/* property value wrong, use default value */
		priv->reset_delay = 1;
	}

	priv->reset_post_delay = dev_read_u32_default(dev,
						      "phy-reset-post-delay",
						      0);
	if (priv->reset_post_delay > 1000) {
		printf("FEC MXC: phy reset post delay should be <= 1000ms\n");
		/* property value wrong, use default value */
		priv->reset_post_delay = 0;
	}
#endif

	return 0;
}

static const struct udevice_id fecmxc_ids[] = {
	{ .compatible = "fsl,imx6q-fec" },
	{ .compatible = "fsl,imx6sl-fec" },
	{ .compatible = "fsl,imx6sx-fec" },
	{ .compatible = "fsl,imx6ul-fec" },
	{ .compatible = "fsl,imx53-fec" },
	{ .compatible = "fsl,imx7d-fec" },
	{ .compatible = "fsl,mvf600-fec" },
	{ }
};

U_BOOT_DRIVER(fecmxc_gem) = {
	.name	= "fecmxc",
	.id	= UCLASS_ETH,
	.of_match = fecmxc_ids,
	.ofdata_to_platdata = fecmxc_ofdata_to_platdata,
	.probe	= fecmxc_probe,
	.remove	= fecmxc_remove,
	.ops	= &fecmxc_ops,
	.priv_auto_alloc_size = sizeof(struct fec_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
#endif
