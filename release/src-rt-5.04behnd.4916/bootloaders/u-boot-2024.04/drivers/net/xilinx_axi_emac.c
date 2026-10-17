// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Waymo LLC
 * Copyright (C) 2011 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2011 PetaLogix
 * Copyright (C) 2010 Xilinx, Inc. All rights reserved.
 */

#include <config.h>
#include <common.h>
#include <cpu_func.h>
#include <display_options.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <log.h>
#include <net.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <phy.h>
#include <miiphy.h>
#include <wait_bit.h>
#include <linux/delay.h>
#include <eth_phy.h>

DECLARE_GLOBAL_DATA_PTR;

/* Link setup */
#define XAE_EMMC_LINKSPEED_MASK	0xC0000000 /* Link speed */
#define XAE_EMMC_LINKSPD_10	0x00000000 /* Link Speed mask for 10 Mbit */
#define XAE_EMMC_LINKSPD_100	0x40000000 /* Link Speed mask for 100 Mbit */
#define XAE_EMMC_LINKSPD_1000	0x80000000 /* Link Speed mask for 1000 Mbit */

/* Interrupt Status/Enable/Mask Registers bit definitions */
#define XAE_INT_RXRJECT_MASK	0x00000008 /* Rx frame rejected */
#define XAE_INT_MGTRDY_MASK	0x00000080 /* MGT clock Lock */

/* Receive Configuration Word 1 (RCW1) Register bit definitions */
#define XAE_RCW1_RX_MASK	0x10000000 /* Receiver enable */

/* Transmitter Configuration (TC) Register bit definitions */
#define XAE_TC_TX_MASK		0x10000000 /* Transmitter enable */

#define XAE_UAW1_UNICASTADDR_MASK	0x0000FFFF

/* MDIO Management Configuration (MC) Register bit definitions */
#define XAE_MDIO_MC_MDIOEN_MASK		0x00000040 /* MII management enable*/

/* MDIO Management Control Register (MCR) Register bit definitions */
#define XAE_MDIO_MCR_PHYAD_MASK		0x1F000000 /* Phy Address Mask */
#define XAE_MDIO_MCR_PHYAD_SHIFT	24	   /* Phy Address Shift */
#define XAE_MDIO_MCR_REGAD_MASK		0x001F0000 /* Reg Address Mask */
#define XAE_MDIO_MCR_REGAD_SHIFT	16	   /* Reg Address Shift */
#define XAE_MDIO_MCR_OP_READ_MASK	0x00008000 /* Op Code Read Mask */
#define XAE_MDIO_MCR_OP_WRITE_MASK	0x00004000 /* Op Code Write Mask */
#define XAE_MDIO_MCR_INITIATE_MASK	0x00000800 /* Ready Mask */
#define XAE_MDIO_MCR_READY_MASK		0x00000080 /* Ready Mask */

#define XAE_MDIO_DIV_DFT	29	/* Default MDIO clock divisor */

#define XAXIDMA_BD_STS_ACTUAL_LEN_MASK	0x007FFFFF /* Actual len */

/* DMA macros */
/* Bitmasks of XAXIDMA_CR_OFFSET register */
#define XAXIDMA_CR_RUNSTOP_MASK	0x00000001 /* Start/stop DMA channel */
#define XAXIDMA_CR_RESET_MASK	0x00000004 /* Reset DMA engine */

/* Bitmasks of XAXIDMA_SR_OFFSET register */
#define XAXIDMA_HALTED_MASK	0x00000001  /* DMA channel halted */

/* Bitmask for interrupts */
#define XAXIDMA_IRQ_IOC_MASK	0x00001000 /* Completion intr */
#define XAXIDMA_IRQ_DELAY_MASK	0x00002000 /* Delay interrupt */
#define XAXIDMA_IRQ_ALL_MASK	0x00007000 /* All interrupts */

/* Bitmasks of XAXIDMA_BD_CTRL_OFFSET register */
#define XAXIDMA_BD_CTRL_TXSOF_MASK	0x08000000 /* First tx packet */
#define XAXIDMA_BD_CTRL_TXEOF_MASK	0x04000000 /* Last tx packet */

/* Bitmasks for XXV Ethernet MAC */
#define XXV_TC_TX_MASK		0x00000001
#define XXV_TC_FCS_MASK		0x00000002
#define XXV_RCW1_RX_MASK	0x00000001
#define XXV_RCW1_FCS_MASK	0x00000002

#define DMAALIGN		128
#define XXV_MIN_PKT_SIZE	60

static u8 rxframe[PKTSIZE_ALIGN] __attribute((aligned(DMAALIGN)));
static u8 txminframe[XXV_MIN_PKT_SIZE] __attribute((aligned(DMAALIGN)));

enum emac_variant {
	EMAC_1G = 0,
	EMAC_10G_25G = 1,
};

/* Reflect dma offsets */
struct axidma_reg {
	u32 control; /* DMACR */
	u32 status; /* DMASR */
	u32 current; /* CURDESC low 32 bit */
	u32 current_hi; /* CURDESC high 32 bit */
	u32 tail; /* TAILDESC low 32 bit */
	u32 tail_hi; /* TAILDESC high 32 bit */
};

/* Platform data structures */
struct axidma_plat {
	struct eth_pdata eth_pdata;
	struct axidma_reg *dmatx;
	struct axidma_reg *dmarx;
	int pcsaddr;
	int phyaddr;
	u8 eth_hasnobuf;
	ofnode phynode;
	enum emac_variant mactype;
};

/* Private driver structures */
struct axidma_priv {
	struct axidma_reg *dmatx;
	struct axidma_reg *dmarx;
	int pcsaddr;
	int phyaddr;
	struct axi_regs *iobase;
	phy_interface_t interface;
	struct phy_device *phydev;
	struct mii_dev *bus;
	u8 eth_hasnobuf;
	ofnode phynode;
	enum emac_variant mactype;
};

/* BD descriptors */
struct axidma_bd {
	u32 next_desc;	/* Next descriptor pointer */
	u32 next_desc_msb;
	u32 buf_addr;	/* Buffer address */
	u32 buf_addr_msb;
	u32 reserved3;
	u32 reserved4;
	u32 cntrl;	/* Control */
	u32 status;	/* Status */
	u32 app0;
	u32 app1;	/* TX start << 16 | insert */
	u32 app2;	/* TX csum seed */
	u32 app3;
	u32 app4;
	u32 sw_id_offset;
	u32 reserved5;
	u32 reserved6;
};

/* Static BDs - driver uses only one BD */
static struct axidma_bd tx_bd __attribute((aligned(DMAALIGN)));
static struct axidma_bd rx_bd __attribute((aligned(DMAALIGN)));

struct axi_regs {
	u32 reserved[3];
	u32 is; /* 0xC: Interrupt status */
	u32 reserved2;
	u32 ie; /* 0x14: Interrupt enable */
	u32 reserved3[251];
	u32 rcw1; /* 0x404: Rx Configuration Word 1 */
	u32 tc; /* 0x408: Tx Configuration */
	u32 reserved4;
	u32 emmc; /* 0x410: EMAC mode configuration */
	u32 reserved5[59];
	u32 mdio_mc; /* 0x500: MII Management Config */
	u32 mdio_mcr; /* 0x504: MII Management Control */
	u32 mdio_mwd; /* 0x508: MII Management Write Data */
	u32 mdio_mrd; /* 0x50C: MII Management Read Data */
	u32 reserved6[124];
	u32 uaw0; /* 0x700: Unicast address word 0 */
	u32 uaw1; /* 0x704: Unicast address word 1 */
};

struct xxv_axi_regs {
	u32 gt_reset;	/* 0x0 */
	u32 reserved[2];
	u32 tc;		/* 0xC: Tx Configuration */
	u32 reserved2;
	u32 rcw1;	/* 0x14: Rx Configuration Word 1 */
};

/* Use MII register 1 (MII status register) to detect PHY */
#define PHY_DETECT_REG  1

/*
 * Mask used to verify certain PHY features (or register contents)
 * in the register above:
 *  0x1000: 10Mbps full duplex support
 *  0x0800: 10Mbps half duplex support
 *  0x0008: Auto-negotiation support
 */
#define PHY_DETECT_MASK 0x1808

static inline int mdio_wait(struct axi_regs *regs)
{
	u32 timeout = 200;

	/* Wait till MDIO interface is ready to accept a new transaction. */
	while (timeout && (!(readl(&regs->mdio_mcr)
						& XAE_MDIO_MCR_READY_MASK))) {
		timeout--;
		udelay(1);
	}
	if (!timeout) {
		printf("%s: Timeout\n", __func__);
		return 1;
	}
	return 0;
}

/**
 * axienet_dma_write -	Memory mapped Axi DMA register Buffer Descriptor write.
 * @bd:		pointer to BD descriptor structure
 * @desc:	Address offset of DMA descriptors
 *
 * This function writes the value into the corresponding Axi DMA register.
 */
static inline void axienet_dma_write(struct axidma_bd *bd, u32 *desc)
{
#if defined(CONFIG_PHYS_64BIT)
	writeq((unsigned long)bd, desc);
#else
	writel((u32)bd, desc);
#endif
}

static u32 phyread(struct axidma_priv *priv, u32 phyaddress, u32 registernum,
		   u16 *val)
{
	struct axi_regs *regs = priv->iobase;
	u32 mdioctrlreg = 0;

	if (mdio_wait(regs))
		return 1;

	mdioctrlreg = ((phyaddress << XAE_MDIO_MCR_PHYAD_SHIFT) &
			XAE_MDIO_MCR_PHYAD_MASK) |
			((registernum << XAE_MDIO_MCR_REGAD_SHIFT)
			& XAE_MDIO_MCR_REGAD_MASK) |
			XAE_MDIO_MCR_INITIATE_MASK |
			XAE_MDIO_MCR_OP_READ_MASK;

	writel(mdioctrlreg, &regs->mdio_mcr);

	if (mdio_wait(regs))
		return 1;

	/* Read data */
	*val = readl(&regs->mdio_mrd);
	return 0;
}

static u32 phywrite(struct axidma_priv *priv, u32 phyaddress, u32 registernum,
		    u32 data)
{
	struct axi_regs *regs = priv->iobase;
	u32 mdioctrlreg = 0;

	if (mdio_wait(regs))
		return 1;

	mdioctrlreg = ((phyaddress << XAE_MDIO_MCR_PHYAD_SHIFT) &
			XAE_MDIO_MCR_PHYAD_MASK) |
			((registernum << XAE_MDIO_MCR_REGAD_SHIFT)
			& XAE_MDIO_MCR_REGAD_MASK) |
			XAE_MDIO_MCR_INITIATE_MASK |
			XAE_MDIO_MCR_OP_WRITE_MASK;

	/* Write data */
	writel(data, &regs->mdio_mwd);

	writel(mdioctrlreg, &regs->mdio_mcr);

	if (mdio_wait(regs))
		return 1;

	return 0;
}

static int axiemac_phy_init(struct udevice *dev)
{
	u16 phyreg;
	int i;
	u32 ret;
	struct axidma_priv *priv = dev_get_priv(dev);
	struct axi_regs *regs = priv->iobase;
	struct phy_device *phydev;

	u32 supported = SUPPORTED_10baseT_Half |
			SUPPORTED_10baseT_Full |
			SUPPORTED_100baseT_Half |
			SUPPORTED_100baseT_Full |
			SUPPORTED_1000baseT_Half |
			SUPPORTED_1000baseT_Full;

	/* Set default MDIO divisor */
	writel(XAE_MDIO_DIV_DFT | XAE_MDIO_MC_MDIOEN_MASK, &regs->mdio_mc);

	if (IS_ENABLED(CONFIG_DM_ETH_PHY))
		priv->phyaddr = eth_phy_get_addr(dev);

	/*
	 * Set address of PCS/PMA PHY to the one pointed by phy-handle for
	 * backward compatibility.
	 */
	if (priv->phyaddr != -1 && priv->pcsaddr == 0)
		priv->pcsaddr = priv->phyaddr;

	if (priv->phyaddr == -1) {
		/* Detect the PHY address */
		for (i = 31; i >= 0; i--) {
			ret = phyread(priv, i, PHY_DETECT_REG, &phyreg);
			if (!ret && (phyreg != 0xFFFF) &&
			((phyreg & PHY_DETECT_MASK) == PHY_DETECT_MASK)) {
				/* Found a valid PHY address */
				priv->phyaddr = i;
				debug("axiemac: Found valid phy address, %x\n",
				      i);
				break;
			}
		}
	}

	/* Interface - look at tsec */
	phydev = phy_connect(priv->bus, priv->phyaddr, dev, priv->interface);
	if (IS_ERR_OR_NULL(phydev)) {
		dev_err(dev, "phy_connect() failed\n");
		return -ENODEV;
	}

	phydev->supported &= supported;
	phydev->advertising = phydev->supported;
	priv->phydev = phydev;
	if (ofnode_valid(priv->phynode))
		priv->phydev->node = priv->phynode;
	phy_config(phydev);

	return 0;
}

static int pcs_pma_startup(struct axidma_priv *priv)
{
	u32 rc, retry_cnt = 0;
	u16 mii_reg;

	rc = phyread(priv, priv->pcsaddr, MII_BMCR, &mii_reg);
	if (rc)
		goto failed_mdio;

	if (!(mii_reg & BMCR_ANENABLE)) {
		mii_reg |= BMCR_ANENABLE;
		if (phywrite(priv, priv->pcsaddr, MII_BMCR, mii_reg))
			goto failed_mdio;
	}

	/*
	 * Check the internal PHY status and warn user if the link between it
	 * and the external PHY is not obtained.
	 */
	debug("axiemac: waiting for link status of the PCS/PMA PHY");
	while (retry_cnt * 10 < PHY_ANEG_TIMEOUT) {
		rc = phyread(priv, priv->pcsaddr, MII_BMSR, &mii_reg);
		if ((mii_reg & BMSR_LSTATUS) && mii_reg != 0xffff && !rc) {
			debug(".Done\n");
			return 0;
		}
		if ((retry_cnt++ % 10) == 0)
			debug(".");
		mdelay(10);
	}
	debug("\n");
	printf("axiemac: Warning, PCS/PMA PHY@%d is not ready, link is down\n",
	       priv->pcsaddr);
	return 1;
failed_mdio:
	printf("axiemac: MDIO to the PCS/PMA PHY has failed\n");
	return 1;
}

/* Setting axi emac and phy to proper setting */
static int setup_phy(struct udevice *dev)
{
	u16 temp;
	u32 speed, emmc_reg, ret;
	struct axidma_priv *priv = dev_get_priv(dev);
	struct axi_regs *regs = priv->iobase;
	struct phy_device *phydev = priv->phydev;

	if (priv->interface == PHY_INTERFACE_MODE_SGMII) {
		/*
		 * In SGMII cases the isolate bit might set
		 * after DMA and ethernet resets and hence
		 * check and clear if set.
		 */
		ret = phyread(priv, priv->pcsaddr, MII_BMCR, &temp);
		if (ret)
			return 0;
		if (temp & BMCR_ISOLATE) {
			temp &= ~BMCR_ISOLATE;
			ret = phywrite(priv, priv->pcsaddr, MII_BMCR, temp);
			if (ret)
				return 0;
		}
	}

	if (phy_startup(phydev)) {
		printf("axiemac: could not initialize PHY %s\n",
		       phydev->dev->name);
		return 0;
	}
	if (priv->interface == PHY_INTERFACE_MODE_SGMII ||
	    priv->interface == PHY_INTERFACE_MODE_1000BASEX) {
		if (pcs_pma_startup(priv))
			return 0;
	}
	if (!phydev->link) {
		printf("%s: No link.\n", phydev->dev->name);
		return 0;
	}

	switch (phydev->speed) {
	case 1000:
		speed = XAE_EMMC_LINKSPD_1000;
		break;
	case 100:
		speed = XAE_EMMC_LINKSPD_100;
		break;
	case 10:
		speed = XAE_EMMC_LINKSPD_10;
		break;
	default:
		return 0;
	}

	/* Setup the emac for the phy speed */
	emmc_reg = readl(&regs->emmc);
	emmc_reg &= ~XAE_EMMC_LINKSPEED_MASK;
	emmc_reg |= speed;

	/* Write new speed setting out to Axi Ethernet */
	writel(emmc_reg, &regs->emmc);

	/*
	* Setting the operating speed of the MAC needs a delay. There
	* doesn't seem to be register to poll, so please consider this
	* during your application design.
	*/
	udelay(1);

	return 1;
}

/* STOP DMA transfers */
static void axiemac_stop(struct udevice *dev)
{
	struct axidma_priv *priv = dev_get_priv(dev);
	u32 temp;

	/* Stop the hardware */
	temp = readl(&priv->dmatx->control);
	temp &= ~XAXIDMA_CR_RUNSTOP_MASK;
	writel(temp, &priv->dmatx->control);

	temp = readl(&priv->dmarx->control);
	temp &= ~XAXIDMA_CR_RUNSTOP_MASK;
	writel(temp, &priv->dmarx->control);

	debug("axiemac: Halted\n");
}

static int xxv_axi_ethernet_init(struct axidma_priv *priv)
{
	struct xxv_axi_regs *regs = (struct xxv_axi_regs *)priv->iobase;

	writel(readl(&regs->rcw1) | XXV_RCW1_FCS_MASK, &regs->rcw1);
	writel(readl(&regs->tc) | XXV_TC_FCS_MASK, &regs->tc);
	writel(readl(&regs->tc) | XXV_TC_TX_MASK, &regs->tc);
	writel(readl(&regs->rcw1) | XXV_RCW1_RX_MASK, &regs->rcw1);

	return 0;
}

static int axi_ethernet_init(struct axidma_priv *priv)
{
	struct axi_regs *regs = priv->iobase;
	int err;

	/*
	 * Check the status of the MgtRdy bit in the interrupt status
	 * registers. This must be done to allow the MGT clock to become stable
	 * for the Sgmii and 1000BaseX PHY interfaces. No other register reads
	 * will be valid until this bit is valid.
	 * The bit is always a 1 for all other PHY interfaces.
	 * Interrupt status and enable registers are not available in non
	 * processor mode and hence bypass in this mode
	 */
	if (!priv->eth_hasnobuf) {
		err = wait_for_bit_le32(&regs->is, XAE_INT_MGTRDY_MASK,
					true, 200, false);
		if (err) {
			printf("%s: Timeout\n", __func__);
			return 1;
		}

		/*
		 * Stop the device and reset HW
		 * Disable interrupts
		 */
		writel(0, &regs->ie);
	}

	/* Disable the receiver */
	writel(readl(&regs->rcw1) & ~XAE_RCW1_RX_MASK, &regs->rcw1);

	/*
	 * Stopping the receiver in mid-packet causes a dropped packet
	 * indication from HW. Clear it.
	 */
	if (!priv->eth_hasnobuf) {
		/* Set the interrupt status register to clear the interrupt */
		writel(XAE_INT_RXRJECT_MASK, &regs->is);
	}

	/* Setup HW */
	/* Set default MDIO divisor */
	writel(XAE_MDIO_DIV_DFT | XAE_MDIO_MC_MDIOEN_MASK, &regs->mdio_mc);

	debug("axiemac: InitHw done\n");
	return 0;
}

static int axiemac_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct axidma_priv *priv = dev_get_priv(dev);
	struct axi_regs *regs = priv->iobase;

	if (priv->mactype != EMAC_1G)
		return 0;

	/* Set the MAC address */
	int val = ((pdata->enetaddr[3] << 24) | (pdata->enetaddr[2] << 16) |
		(pdata->enetaddr[1] << 8) | (pdata->enetaddr[0]));
	writel(val, &regs->uaw0);

	val = (pdata->enetaddr[5] << 8) | pdata->enetaddr[4];
	val |= readl(&regs->uaw1) & ~XAE_UAW1_UNICASTADDR_MASK;
	writel(val, &regs->uaw1);
	return 0;
}

/* Reset DMA engine */
static void axi_dma_init(struct axidma_priv *priv)
{
	u32 timeout = 500;

	/* Reset the engine so the hardware starts from a known state */
	writel(XAXIDMA_CR_RESET_MASK, &priv->dmatx->control);
	writel(XAXIDMA_CR_RESET_MASK, &priv->dmarx->control);

	/* At the initialization time, hardware should finish reset quickly */
	while (timeout--) {
		/* Check transmit/receive channel */
		/* Reset is done when the reset bit is low */
		if (!((readl(&priv->dmatx->control) |
				readl(&priv->dmarx->control))
						& XAXIDMA_CR_RESET_MASK)) {
			break;
		}
	}
	if (!timeout)
		printf("%s: Timeout\n", __func__);
}

static int axiemac_start(struct udevice *dev)
{
	struct axidma_priv *priv = dev_get_priv(dev);
	u32 temp;

	debug("axiemac: Init started\n");
	/*
	 * Initialize AXIDMA engine. AXIDMA engine must be initialized before
	 * AxiEthernet. During AXIDMA engine initialization, AXIDMA hardware is
	 * reset, and since AXIDMA reset line is connected to AxiEthernet, this
	 * would ensure a reset of AxiEthernet.
	 */
	axi_dma_init(priv);

	/* Initialize AxiEthernet hardware. */
	if (priv->mactype == EMAC_1G) {
		if (axi_ethernet_init(priv))
			return -1;
	} else {
		if (xxv_axi_ethernet_init(priv))
			return -1;
	}

	/* Disable all RX interrupts before RxBD space setup */
	temp = readl(&priv->dmarx->control);
	temp &= ~XAXIDMA_IRQ_ALL_MASK;
	writel(temp, &priv->dmarx->control);

	/* Start DMA RX channel. Now it's ready to receive data.*/
	axienet_dma_write(&rx_bd, &priv->dmarx->current);

	/* Setup the BD. */
	memset(&rx_bd, 0, sizeof(rx_bd));
	rx_bd.next_desc = lower_32_bits((unsigned long)&rx_bd);
	rx_bd.buf_addr = lower_32_bits((unsigned long)&rxframe);
#if defined(CONFIG_PHYS_64BIT)
	rx_bd.next_desc_msb = upper_32_bits((unsigned long)&rx_bd);
	rx_bd.buf_addr_msb = upper_32_bits((unsigned long)&rxframe);
#endif
	rx_bd.cntrl = sizeof(rxframe);
	/* Flush the last BD so DMA core could see the updates */
	flush_cache((phys_addr_t)&rx_bd, sizeof(rx_bd));

	/* It is necessary to flush rxframe because if you don't do it
	 * then cache can contain uninitialized data */
	flush_cache((phys_addr_t)&rxframe, sizeof(rxframe));

	/* Start the hardware */
	temp = readl(&priv->dmarx->control);
	temp |= XAXIDMA_CR_RUNSTOP_MASK;
	writel(temp, &priv->dmarx->control);

	/* Rx BD is ready - start */
	axienet_dma_write(&rx_bd, &priv->dmarx->tail);

	if (priv->mactype == EMAC_1G) {
		struct axi_regs *regs = priv->iobase;
		/* Enable TX */
		writel(XAE_TC_TX_MASK, &regs->tc);
		/* Enable RX */
		writel(XAE_RCW1_RX_MASK, &regs->rcw1);

		/* PHY setup */
		if (!setup_phy(dev)) {
			axiemac_stop(dev);
			return -1;
		}
	} else {
		struct xxv_axi_regs *regs = (struct xxv_axi_regs *)priv->iobase;
		/* Enable TX */
		writel(readl(&regs->tc) | XXV_TC_TX_MASK, &regs->tc);

		/* Enable RX */
		writel(readl(&regs->rcw1) | XXV_RCW1_RX_MASK, &regs->rcw1);
	}

	debug("axiemac: Init complete\n");
	return 0;
}

static int axiemac_send(struct udevice *dev, void *ptr, int len)
{
	struct axidma_priv *priv = dev_get_priv(dev);
	u32 timeout;

	if (len > PKTSIZE_ALIGN)
		len = PKTSIZE_ALIGN;

	/* If size is less than min packet size, pad to min size */
	if (priv->mactype == EMAC_10G_25G && len < XXV_MIN_PKT_SIZE) {
		memset(txminframe, 0, XXV_MIN_PKT_SIZE);
		memcpy(txminframe, ptr, len);
		len = XXV_MIN_PKT_SIZE;
		ptr = txminframe;
	}

	/* Flush packet to main memory to be trasfered by DMA */
	flush_cache((phys_addr_t)ptr, len);

	/* Setup Tx BD */
	memset(&tx_bd, 0, sizeof(tx_bd));
	/* At the end of the ring, link the last BD back to the top */
	tx_bd.next_desc = lower_32_bits((unsigned long)&tx_bd);
	tx_bd.buf_addr = lower_32_bits((unsigned long)ptr);
#if defined(CONFIG_PHYS_64BIT)
	tx_bd.next_desc_msb = upper_32_bits((unsigned long)&tx_bd);
	tx_bd.buf_addr_msb = upper_32_bits((unsigned long)ptr);
#endif
	/* Save len */
	tx_bd.cntrl = len | XAXIDMA_BD_CTRL_TXSOF_MASK |
						XAXIDMA_BD_CTRL_TXEOF_MASK;

	/* Flush the last BD so DMA core could see the updates */
	flush_cache((phys_addr_t)&tx_bd, sizeof(tx_bd));

	if (readl(&priv->dmatx->status) & XAXIDMA_HALTED_MASK) {
		u32 temp;
		axienet_dma_write(&tx_bd, &priv->dmatx->current);
		/* Start the hardware */
		temp = readl(&priv->dmatx->control);
		temp |= XAXIDMA_CR_RUNSTOP_MASK;
		writel(temp, &priv->dmatx->control);
	}

	/* Start transfer */
	axienet_dma_write(&tx_bd, &priv->dmatx->tail);

	/* Wait for transmission to complete */
	debug("axiemac: Waiting for tx to be done\n");
	timeout = 200;
	while (timeout && (!(readl(&priv->dmatx->status) &
			(XAXIDMA_IRQ_DELAY_MASK | XAXIDMA_IRQ_IOC_MASK)))) {
		timeout--;
		udelay(1);
	}
	if (!timeout) {
		printf("%s: Timeout\n", __func__);
		return 1;
	}

	debug("axiemac: Sending complete\n");
	return 0;
}

static int isrxready(struct axidma_priv *priv)
{
	u32 status;

	/* Read pending interrupts */
	status = readl(&priv->dmarx->status);

	/* Acknowledge pending interrupts */
	writel(status & XAXIDMA_IRQ_ALL_MASK, &priv->dmarx->status);

	/*
	 * If Reception done interrupt is asserted, call RX call back function
	 * to handle the processed BDs and then raise the according flag.
	 */
	if ((status & (XAXIDMA_IRQ_DELAY_MASK | XAXIDMA_IRQ_IOC_MASK)))
		return 1;

	return 0;
}

static int axiemac_recv(struct udevice *dev, int flags, uchar **packetp)
{
	u32 length;
	struct axidma_priv *priv = dev_get_priv(dev);
	u32 temp;

	/* Wait for an incoming packet */
	if (!isrxready(priv))
		return -EAGAIN;

	debug("axiemac: RX data ready\n");

	/* Disable IRQ for a moment till packet is handled */
	temp = readl(&priv->dmarx->control);
	temp &= ~XAXIDMA_IRQ_ALL_MASK;
	writel(temp, &priv->dmarx->control);
	if (!priv->eth_hasnobuf  && priv->mactype == EMAC_1G)
		length = rx_bd.app4 & 0xFFFF; /* max length mask */
	else
		length = rx_bd.status & XAXIDMA_BD_STS_ACTUAL_LEN_MASK;

#ifdef DEBUG
	print_buffer(&rxframe, &rxframe[0], 1, length, 16);
#endif

	*packetp = rxframe;
	return length;
}

static int axiemac_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct axidma_priv *priv = dev_get_priv(dev);

#ifdef DEBUG
	/* It is useful to clear buffer to be sure that it is consistent */
	memset(rxframe, 0, sizeof(rxframe));
#endif
	/* Setup RxBD */
	/* Clear the whole buffer and setup it again - all flags are cleared */
	memset(&rx_bd, 0, sizeof(rx_bd));
	rx_bd.next_desc = lower_32_bits((unsigned long)&rx_bd);
	rx_bd.buf_addr = lower_32_bits((unsigned long)&rxframe);
#if defined(CONFIG_PHYS_64BIT)
	rx_bd.next_desc_msb = upper_32_bits((unsigned long)&rx_bd);
	rx_bd.buf_addr_msb = upper_32_bits((unsigned long)&rxframe);
#endif
	rx_bd.cntrl = sizeof(rxframe);

	/* Write bd to HW */
	flush_cache((phys_addr_t)&rx_bd, sizeof(rx_bd));

	/* It is necessary to flush rxframe because if you don't do it
	 * then cache will contain previous packet */
	flush_cache((phys_addr_t)&rxframe, sizeof(rxframe));

	/* Rx BD is ready - start again */
	axienet_dma_write(&rx_bd, &priv->dmarx->tail);

	debug("axiemac: RX completed, framelength = %d\n", length);

	return 0;
}

static int axiemac_miiphy_read(struct mii_dev *bus, int addr,
			       int devad, int reg)
{
	int ret;
	u16 value;

	ret = phyread(bus->priv, addr, reg, &value);
	debug("axiemac: Read MII 0x%x, 0x%x, 0x%x, %d\n", addr, reg,
	      value, ret);
	return value;
}

static int axiemac_miiphy_write(struct mii_dev *bus, int addr, int devad,
				int reg, u16 value)
{
	debug("axiemac: Write MII 0x%x, 0x%x, 0x%x\n", addr, reg, value);
	return phywrite(bus->priv, addr, reg, value);
}

static int axi_emac_probe(struct udevice *dev)
{
	struct axidma_plat *plat = dev_get_plat(dev);
	struct eth_pdata *pdata = &plat->eth_pdata;
	struct axidma_priv *priv = dev_get_priv(dev);
	int ret;

	priv->iobase = (struct axi_regs *)pdata->iobase;
	priv->dmatx = plat->dmatx;
	/* RX channel offset is 0x30 */
	priv->dmarx = (struct axidma_reg *)((phys_addr_t)priv->dmatx + 0x30);
	priv->mactype = plat->mactype;

	if (priv->mactype == EMAC_1G) {
		priv->eth_hasnobuf = plat->eth_hasnobuf;
		priv->pcsaddr = plat->pcsaddr;
		priv->phyaddr = plat->phyaddr;
		priv->phynode = plat->phynode;
		priv->interface = pdata->phy_interface;

		if (IS_ENABLED(CONFIG_DM_ETH_PHY))
			priv->bus = eth_phy_get_mdio_bus(dev);

		if (!priv->bus) {
			priv->bus = mdio_alloc();
			priv->bus->read = axiemac_miiphy_read;
			priv->bus->write = axiemac_miiphy_write;
			priv->bus->priv = priv;

			ret = mdio_register_seq(priv->bus, dev_seq(dev));
			if (ret)
				return ret;
		}

		if (IS_ENABLED(CONFIG_DM_ETH_PHY))
			eth_phy_set_mdio_bus(dev, priv->bus);

		axiemac_phy_init(dev);
	}

	printf("AXI EMAC: %lx, phyaddr %d, interface %s\n", (ulong)pdata->iobase,
	       priv->phyaddr, phy_string_for_interface(pdata->phy_interface));

	return 0;
}

static int axi_emac_remove(struct udevice *dev)
{
	struct axidma_priv *priv = dev_get_priv(dev);

	if (priv->mactype == EMAC_1G) {
		free(priv->phydev);
		mdio_unregister(priv->bus);
		mdio_free(priv->bus);
	}

	return 0;
}

static const struct eth_ops axi_emac_ops = {
	.start			= axiemac_start,
	.send			= axiemac_send,
	.recv			= axiemac_recv,
	.free_pkt		= axiemac_free_pkt,
	.stop			= axiemac_stop,
	.write_hwaddr		= axiemac_write_hwaddr,
};

static int axi_emac_of_to_plat(struct udevice *dev)
{
	struct axidma_plat *plat = dev_get_plat(dev);
	struct eth_pdata *pdata = &plat->eth_pdata;
	struct ofnode_phandle_args pcs_node, axistream_node;
	ofnode phynode;
	int ret;

	pdata->iobase = dev_read_addr(dev);
	plat->mactype = dev_get_driver_data(dev);

	ret = dev_read_phandle_with_args(dev, "axistream-connected", NULL, 0, 0,
					 &axistream_node);
	if (!ret)
		plat->dmatx = (struct axidma_reg *)ofnode_get_addr(axistream_node.node);
	else
		plat->dmatx = (struct axidma_reg *)dev_read_addr_index(dev, 1);

	if (!plat->dmatx) {
		printf("%s: axi_dma register space not found\n", __func__);
		return -EINVAL;
	}

	if (plat->mactype == EMAC_1G) {
		plat->phyaddr = -1;
		/* PHYAD 0 always redirects to the PCS/PMA PHY */
		plat->pcsaddr = 0;

		phynode = dev_get_phy_node(dev);
		if (ofnode_valid(phynode)) {
			if (!(IS_ENABLED(CONFIG_DM_ETH_PHY)))
				plat->phyaddr = ofnode_read_u32_default(phynode,
									"reg", -1);
			plat->phynode = phynode;
		}

		pdata->phy_interface = dev_read_phy_mode(dev);
		if (pdata->phy_interface == PHY_INTERFACE_MODE_NA)
			return -EINVAL;

		plat->eth_hasnobuf = dev_read_bool(dev, "xlnx,eth-hasnobuf");

		if (pdata->phy_interface == PHY_INTERFACE_MODE_SGMII ||
		    pdata->phy_interface == PHY_INTERFACE_MODE_1000BASEX) {
			ret = dev_read_phandle_with_args(dev, "pcs-handle", NULL, 0, 0,
							 &pcs_node);
			if (!ret) {
				plat->pcsaddr = ofnode_read_u32_default(pcs_node.node,
									"reg", -1);
			}
		}
	}

	return 0;
}

static const struct udevice_id axi_emac_ids[] = {
	{ .compatible = "xlnx,axi-ethernet-1.00.a", .data = (uintptr_t)EMAC_1G },
	{ .compatible = "xlnx,xxv-ethernet-1.0", .data = (uintptr_t)EMAC_10G_25G },
	{ }
};

U_BOOT_DRIVER(axi_emac) = {
	.name	= "axi_emac",
	.id	= UCLASS_ETH,
	.of_match = axi_emac_ids,
	.of_to_plat = axi_emac_of_to_plat,
	.probe	= axi_emac_probe,
	.remove	= axi_emac_remove,
	.ops	= &axi_emac_ops,
	.priv_auto	= sizeof(struct axidma_priv),
	.plat_auto	= sizeof(struct axidma_plat),
};
