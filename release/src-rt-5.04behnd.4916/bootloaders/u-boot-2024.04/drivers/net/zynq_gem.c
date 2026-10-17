// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011 Michal Simek
 *
 * Michal SIMEK <monstr@monstr.eu>
 *
 * Based on Xilinx gmac driver:
 * (C) Copyright 2011 Xilinx
 */

#include <clk.h>
#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <generic-phy.h>
#include <log.h>
#include <net.h>
#include <netdev.h>
#include <config.h>
#include <console.h>
#include <malloc.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <phy.h>
#include <reset.h>
#include <miiphy.h>
#include <wait_bit.h>
#include <watchdog.h>
#include <asm/system.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <eth_phy.h>
#include <zynqmp_firmware.h>

/* Bit/mask specification */
#define ZYNQ_GEM_PHYMNTNC_OP_MASK	0x40020000 /* operation mask bits */
#define ZYNQ_GEM_PHYMNTNC_OP_R_MASK	0x20000000 /* read operation */
#define ZYNQ_GEM_PHYMNTNC_OP_W_MASK	0x10000000 /* write operation */
#define ZYNQ_GEM_PHYMNTNC_PHYAD_SHIFT_MASK	23 /* Shift bits for PHYAD */
#define ZYNQ_GEM_PHYMNTNC_PHREG_SHIFT_MASK	18 /* Shift bits for PHREG */

#define ZYNQ_GEM_RXBUF_EOF_MASK		0x00008000 /* End of frame. */
#define ZYNQ_GEM_RXBUF_SOF_MASK		0x00004000 /* Start of frame. */
#define ZYNQ_GEM_RXBUF_LEN_MASK		0x00003FFF /* Mask for length field */

#define ZYNQ_GEM_RXBUF_WRAP_MASK	0x00000002 /* Wrap bit, last BD */
#define ZYNQ_GEM_RXBUF_NEW_MASK		0x00000001 /* Used bit.. */
#define ZYNQ_GEM_RXBUF_ADD_MASK		0xFFFFFFFC /* Mask for address */

/* Wrap bit, last descriptor */
#define ZYNQ_GEM_TXBUF_WRAP_MASK	0x40000000
#define ZYNQ_GEM_TXBUF_LAST_MASK	0x00008000 /* Last buffer */
#define ZYNQ_GEM_TXBUF_USED_MASK	0x80000000 /* Used by Hw */

#define ZYNQ_GEM_NWCTRL_TXEN_MASK	0x00000008 /* Enable transmit */
#define ZYNQ_GEM_NWCTRL_RXEN_MASK	0x00000004 /* Enable receive */
#define ZYNQ_GEM_NWCTRL_MDEN_MASK	0x00000010 /* Enable MDIO port */
#define ZYNQ_GEM_NWCTRL_STARTTX_MASK	0x00000200 /* Start tx (tx_go) */

#define ZYNQ_GEM_NWCFG_SPEED100		0x00000001 /* 100 Mbps operation */
#define ZYNQ_GEM_NWCFG_SPEED1000	0x00000400 /* 1Gbps operation */
#define ZYNQ_GEM_NWCFG_FDEN		0x00000002 /* Full Duplex mode */
#define ZYNQ_GEM_NWCFG_FSREM		0x00020000 /* FCS removal */
#define ZYNQ_GEM_NWCFG_SGMII_ENBL	0x08000000 /* SGMII Enable */
#define ZYNQ_GEM_NWCFG_PCS_SEL		0x00000800 /* PCS select */

#ifdef CONFIG_ARM64
# define ZYNQ_GEM_DBUS_WIDTH	(1 << 21) /* 64 bit bus */
#else
# define ZYNQ_GEM_DBUS_WIDTH	(0 << 21) /* 32 bit bus */
#endif

#define ZYNQ_GEM_NWCFG_INIT		(ZYNQ_GEM_DBUS_WIDTH | \
					ZYNQ_GEM_NWCFG_FDEN | \
					ZYNQ_GEM_NWCFG_FSREM)

#define ZYNQ_GEM_NWSR_MDIOIDLE_MASK	0x00000004 /* PHY management idle */

#define ZYNQ_GEM_DMACR_BLENGTH		0x00000004 /* INCR4 AHB bursts */
/* Use full configured addressable space (8 Kb) */
#define ZYNQ_GEM_DMACR_RXSIZE		0x00000300
/* Use full configured addressable space (4 Kb) */
#define ZYNQ_GEM_DMACR_TXSIZE		0x00000400
/* Set with binary 00011000 to use 1536 byte(1*max length frame/buffer) */
#define ZYNQ_GEM_DMACR_RXBUF		0x00180000

#if defined(CONFIG_PHYS_64BIT)
# define ZYNQ_GEM_DMA_BUS_WIDTH		BIT(30) /* 64 bit bus */
#else
# define ZYNQ_GEM_DMA_BUS_WIDTH		(0 << 30) /* 32 bit bus */
#endif

#define ZYNQ_GEM_DMACR_INIT		(ZYNQ_GEM_DMACR_BLENGTH | \
					ZYNQ_GEM_DMACR_RXSIZE | \
					ZYNQ_GEM_DMACR_TXSIZE | \
					ZYNQ_GEM_DMACR_RXBUF | \
					ZYNQ_GEM_DMA_BUS_WIDTH)

#define ZYNQ_GEM_TSR_DONE		0x00000020 /* Tx done mask */

#define ZYNQ_GEM_PCS_CTL_ANEG_ENBL	0x1000

#define ZYNQ_GEM_DCFG_DBG6_DMA_64B	BIT(23)

#define MDIO_IDLE_TIMEOUT_MS		100

/* Use MII register 1 (MII status register) to detect PHY */
#define PHY_DETECT_REG  1

/* Mask used to verify certain PHY features (or register contents)
 * in the register above:
 *  0x1000: 10Mbps full duplex support
 *  0x0800: 10Mbps half duplex support
 *  0x0008: Auto-negotiation support
 */
#define PHY_DETECT_MASK 0x1808

/* PCS (SGMII) Link Status */
#define ZYNQ_GEM_PCSSTATUS_LINK		BIT(2)
#define ZYNQ_GEM_PCSSTATUS_ANEG_COMPL	BIT(5)

/* TX BD status masks */
#define ZYNQ_GEM_TXBUF_FRMLEN_MASK	0x000007ff
#define ZYNQ_GEM_TXBUF_EXHAUSTED	0x08000000
#define ZYNQ_GEM_TXBUF_UNDERRUN		0x10000000

/* Clock frequencies for different speeds */
#define ZYNQ_GEM_FREQUENCY_10	2500000UL
#define ZYNQ_GEM_FREQUENCY_100	25000000UL
#define ZYNQ_GEM_FREQUENCY_1000	125000000UL

#define RXCLK_EN		BIT(0)

/* GEM specific constants for CLK. */
#define GEM_CLK_DIV8		0
#define GEM_CLK_DIV16		1
#define GEM_CLK_DIV32		2
#define GEM_CLK_DIV48		3
#define GEM_CLK_DIV64		4
#define GEM_CLK_DIV96		5
#define GEM_CLK_DIV128		6
#define GEM_CLK_DIV224		7

#define GEM_MDC_SET(val)	FIELD_PREP(GENMASK(20, 18), val)

/* Device registers */
struct zynq_gem_regs {
	u32 nwctrl; /* 0x0 - Network Control reg */
	u32 nwcfg; /* 0x4 - Network Config reg */
	u32 nwsr; /* 0x8 - Network Status reg */
	u32 reserved1;
	u32 dmacr; /* 0x10 - DMA Control reg */
	u32 txsr; /* 0x14 - TX Status reg */
	u32 rxqbase; /* 0x18 - RX Q Base address reg */
	u32 txqbase; /* 0x1c - TX Q Base address reg */
	u32 rxsr; /* 0x20 - RX Status reg */
	u32 reserved2[2];
	u32 idr; /* 0x2c - Interrupt Disable reg */
	u32 reserved3;
	u32 phymntnc; /* 0x34 - Phy Maintaince reg */
	u32 reserved4[18];
	u32 hashl; /* 0x80 - Hash Low address reg */
	u32 hashh; /* 0x84 - Hash High address reg */
#define LADDR_LOW	0
#define LADDR_HIGH	1
	u32 laddr[4][LADDR_HIGH + 1]; /* 0x8c - Specific1 addr low/high reg */
	u32 match[4]; /* 0xa8 - Type ID1 Match reg */
	u32 reserved6[18];
#define STAT_SIZE	44
	u32 stat[STAT_SIZE]; /* 0x100 - Octects transmitted Low reg */
	u32 reserved9[20];
	u32 pcscntrl;
	u32 pcsstatus;
	u32 rserved12[35];
	u32 dcfg6; /* 0x294 Design config reg6 */
	u32 reserved7[106];
	u32 transmit_q1_ptr; /* 0x440 - Transmit priority queue 1 */
	u32 reserved8[15];
	u32 receive_q1_ptr; /* 0x480 - Receive priority queue 1 */
	u32 reserved10[17];
	u32 upper_txqbase; /* 0x4C8 - Upper tx_q base addr */
	u32 reserved11[2];
	u32 upper_rxqbase; /* 0x4D4 - Upper rx_q base addr */
};

/* BD descriptors */
struct emac_bd {
	u32 addr; /* Next descriptor pointer */
	u32 status;
#if defined(CONFIG_PHYS_64BIT)
	u32 addr_hi;
	u32 reserved;
#endif
};

/* Reduce amount of BUFs if you have limited amount of memory */
#define RX_BUF 32
/* Page table entries are set to 1MB, or multiples of 1MB
 * (not < 1MB). driver uses less bd's so use 1MB bdspace.
 */
#define BD_SPACE	0x100000
/* BD separation space */
#define BD_SEPRN_SPACE	(RX_BUF * sizeof(struct emac_bd))

/* Setup the first free TX descriptor */
#define TX_FREE_DESC	2

/* Initialized, rxbd_current, rx_first_buf must be 0 after init */
struct zynq_gem_priv {
	struct emac_bd *tx_bd;
	struct emac_bd *rx_bd;
	char *rxbuffers;
	u32 rxbd_current;
	u32 rx_first_buf;
	int phyaddr;
	int init;
	struct zynq_gem_regs *iobase;
	struct zynq_gem_regs *mdiobase;
	phy_interface_t interface;
	struct phy_device *phydev;
	ofnode phy_of_node;
	struct mii_dev *bus;
	struct clk rx_clk;
	struct clk tx_clk;
	struct clk pclk;
	u32 max_speed;
	bool int_pcs;
	bool dma_64bit;
	u32 clk_en_info;
	struct reset_ctl_bulk resets;
};

static int phy_setup_op(struct zynq_gem_priv *priv, u32 phy_addr, u32 regnum,
			u32 op, u16 *data)
{
	u32 mgtcr;
	struct zynq_gem_regs *regs = priv->mdiobase;
	int err;

	err = wait_for_bit_le32(&regs->nwsr, ZYNQ_GEM_NWSR_MDIOIDLE_MASK,
				true, MDIO_IDLE_TIMEOUT_MS, false);
	if (err)
		return err;

	/* Construct mgtcr mask for the operation */
	mgtcr = ZYNQ_GEM_PHYMNTNC_OP_MASK | op |
		(phy_addr << ZYNQ_GEM_PHYMNTNC_PHYAD_SHIFT_MASK) |
		(regnum << ZYNQ_GEM_PHYMNTNC_PHREG_SHIFT_MASK) | *data;

	/* Write mgtcr and wait for completion */
	writel(mgtcr, &regs->phymntnc);

	err = wait_for_bit_le32(&regs->nwsr, ZYNQ_GEM_NWSR_MDIOIDLE_MASK,
				true, MDIO_IDLE_TIMEOUT_MS, false);
	if (err)
		return err;

	if (op == ZYNQ_GEM_PHYMNTNC_OP_R_MASK)
		*data = readl(&regs->phymntnc);

	return 0;
}

static int phyread(struct zynq_gem_priv *priv, u32 phy_addr,
		   u32 regnum, u16 *val)
{
	int ret;

	ret = phy_setup_op(priv, phy_addr, regnum,
			   ZYNQ_GEM_PHYMNTNC_OP_R_MASK, val);

	if (!ret)
		debug("%s: phy_addr %d, regnum 0x%x, val 0x%x\n", __func__,
		      phy_addr, regnum, *val);

	return ret;
}

static int phywrite(struct zynq_gem_priv *priv, u32 phy_addr,
		    u32 regnum, u16 data)
{
	debug("%s: phy_addr %d, regnum 0x%x, data 0x%x\n", __func__, phy_addr,
	      regnum, data);

	return phy_setup_op(priv, phy_addr, regnum,
			    ZYNQ_GEM_PHYMNTNC_OP_W_MASK, &data);
}

static int zynq_gem_setup_mac(struct udevice *dev)
{
	u32 i, macaddrlow, macaddrhigh;
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct zynq_gem_priv *priv = dev_get_priv(dev);
	struct zynq_gem_regs *regs = priv->iobase;

	/* Set the MAC bits [31:0] in BOT */
	macaddrlow = pdata->enetaddr[0];
	macaddrlow |= pdata->enetaddr[1] << 8;
	macaddrlow |= pdata->enetaddr[2] << 16;
	macaddrlow |= pdata->enetaddr[3] << 24;

	/* Set MAC bits [47:32] in TOP */
	macaddrhigh = pdata->enetaddr[4];
	macaddrhigh |= pdata->enetaddr[5] << 8;

	for (i = 0; i < 4; i++) {
		writel(0, &regs->laddr[i][LADDR_LOW]);
		writel(0, &regs->laddr[i][LADDR_HIGH]);
		/* Do not use MATCHx register */
		writel(0, &regs->match[i]);
	}

	writel(macaddrlow, &regs->laddr[0][LADDR_LOW]);
	writel(macaddrhigh, &regs->laddr[0][LADDR_HIGH]);

	return 0;
}

static u32 gem_mdc_clk_div(struct zynq_gem_priv *priv)
{
	u32 config;
	unsigned long pclk_hz;

	pclk_hz = clk_get_rate(&priv->pclk);
	if (pclk_hz <= 20000000)
		config = GEM_MDC_SET(GEM_CLK_DIV8);
	else if (pclk_hz <= 40000000)
		config = GEM_MDC_SET(GEM_CLK_DIV16);
	else if (pclk_hz <= 80000000)
		config = GEM_MDC_SET(GEM_CLK_DIV32);
	else if (pclk_hz <= 120000000)
		config = GEM_MDC_SET(GEM_CLK_DIV48);
	else if (pclk_hz <= 160000000)
		config = GEM_MDC_SET(GEM_CLK_DIV64);
	else if (pclk_hz <= 240000000)
		config = GEM_MDC_SET(GEM_CLK_DIV96);
	else if (pclk_hz <= 320000000)
		config = GEM_MDC_SET(GEM_CLK_DIV128);
	else
		config = GEM_MDC_SET(GEM_CLK_DIV224);

	return config;
}

static int zynq_phy_init(struct udevice *dev)
{
	int ret, val;
	struct zynq_gem_priv *priv = dev_get_priv(dev);
	struct zynq_gem_regs *regs_mdio = priv->mdiobase;
	struct zynq_gem_regs *regs = priv->iobase;
	const u32 supported = SUPPORTED_10baseT_Half |
			SUPPORTED_10baseT_Full |
			SUPPORTED_100baseT_Half |
			SUPPORTED_100baseT_Full |
			SUPPORTED_1000baseT_Half |
			SUPPORTED_1000baseT_Full;

	val = gem_mdc_clk_div(priv);
	if (val)
		writel(val, &regs->nwcfg);

	/* Enable only MDIO bus */
	writel(ZYNQ_GEM_NWCTRL_MDEN_MASK, &regs_mdio->nwctrl);

	if (IS_ENABLED(CONFIG_DM_ETH_PHY))
		priv->phyaddr = eth_phy_get_addr(dev);

	priv->phydev = phy_connect(priv->bus, priv->phyaddr, dev,
				   priv->interface);
	if (IS_ERR_OR_NULL(priv->phydev))
		return -ENODEV;

	if (priv->max_speed) {
		ret = phy_set_supported(priv->phydev, priv->max_speed);
		if (ret)
			return ret;
	}

	priv->phydev->supported &= supported | ADVERTISED_Pause |
				  ADVERTISED_Asym_Pause;

	priv->phydev->advertising = priv->phydev->supported;
	if (!ofnode_valid(priv->phydev->node))
		priv->phydev->node = priv->phy_of_node;

	return phy_config(priv->phydev);
}


static int zynq_gem_init(struct udevice *dev)
{
	u32 i, nwconfig, nwcfg;
	int ret;
	unsigned long clk_rate = 0;
	struct zynq_gem_priv *priv = dev_get_priv(dev);
	struct zynq_gem_regs *regs = priv->iobase;
	struct zynq_gem_regs *regs_mdio = priv->mdiobase;
	struct emac_bd *dummy_tx_bd = &priv->tx_bd[TX_FREE_DESC];
	struct emac_bd *dummy_rx_bd = &priv->tx_bd[TX_FREE_DESC + 2];

	if (readl(&regs->dcfg6) & ZYNQ_GEM_DCFG_DBG6_DMA_64B)
		priv->dma_64bit = true;
	else
		priv->dma_64bit = false;

#if defined(CONFIG_PHYS_64BIT)
	if (!priv->dma_64bit) {
		printf("ERR: %s: Using 64-bit DMA but HW doesn't support it\n",
		       __func__);
		return -EINVAL;
	}
#else
	if (priv->dma_64bit)
		debug("WARN: %s: Not using 64-bit dma even HW supports it\n",
		      __func__);
#endif

	if (!priv->init) {
		/* Disable all interrupts */
		writel(0xFFFFFFFF, &regs->idr);

		/* Disable the receiver & transmitter */
		writel(0, &regs->nwctrl);
		writel(0, &regs->txsr);
		writel(0, &regs->rxsr);
		writel(0, &regs->phymntnc);

		/* Clear the Hash registers for the mac address
		 * pointed by AddressPtr
		 */
		writel(0x0, &regs->hashl);
		/* Write bits [63:32] in TOP */
		writel(0x0, &regs->hashh);

		/* Clear all counters */
		for (i = 0; i < STAT_SIZE; i++)
			readl(&regs->stat[i]);

		/* Setup RxBD space */
		memset(priv->rx_bd, 0, RX_BUF * sizeof(struct emac_bd));

		for (i = 0; i < RX_BUF; i++) {
			priv->rx_bd[i].status = 0xF0000000;
			priv->rx_bd[i].addr =
					(lower_32_bits((ulong)(priv->rxbuffers)
							+ (i * PKTSIZE_ALIGN)));
#if defined(CONFIG_PHYS_64BIT)
			priv->rx_bd[i].addr_hi =
					(upper_32_bits((ulong)(priv->rxbuffers)
							+ (i * PKTSIZE_ALIGN)));
#endif
	}
		/* WRAP bit to last BD */
		priv->rx_bd[--i].addr |= ZYNQ_GEM_RXBUF_WRAP_MASK;
		/* Write RxBDs to IP */
		writel(lower_32_bits((ulong)priv->rx_bd), &regs->rxqbase);
#if defined(CONFIG_PHYS_64BIT)
		writel(upper_32_bits((ulong)priv->rx_bd), &regs->upper_rxqbase);
#endif

		/* Setup for DMA Configuration register */
		writel(ZYNQ_GEM_DMACR_INIT, &regs->dmacr);

		/* Setup for Network Control register, MDIO, Rx and Tx enable */
		setbits_le32(&regs_mdio->nwctrl, ZYNQ_GEM_NWCTRL_MDEN_MASK);

		/* Disable the second priority queue */
		dummy_tx_bd->addr = 0;
#if defined(CONFIG_PHYS_64BIT)
		dummy_tx_bd->addr_hi = 0;
#endif
		dummy_tx_bd->status = ZYNQ_GEM_TXBUF_WRAP_MASK |
				ZYNQ_GEM_TXBUF_LAST_MASK|
				ZYNQ_GEM_TXBUF_USED_MASK;

		dummy_rx_bd->addr = ZYNQ_GEM_RXBUF_WRAP_MASK |
				ZYNQ_GEM_RXBUF_NEW_MASK;
#if defined(CONFIG_PHYS_64BIT)
		dummy_rx_bd->addr_hi = 0;
#endif
		dummy_rx_bd->status = 0;

		writel((ulong)dummy_tx_bd, &regs->transmit_q1_ptr);
		writel((ulong)dummy_rx_bd, &regs->receive_q1_ptr);

		priv->init++;
	}

	ret = phy_startup(priv->phydev);
	if (ret)
		return ret;

	if (!priv->phydev->link) {
		printf("%s: No link.\n", priv->phydev->dev->name);
		return -1;
	}

	nwconfig = ZYNQ_GEM_NWCFG_INIT;

	/*
	 * Set SGMII enable PCS selection only if internal PCS/PMA
	 * core is used and interface is SGMII.
	 */
	if (priv->interface == PHY_INTERFACE_MODE_SGMII &&
	    priv->int_pcs) {
		nwconfig |= ZYNQ_GEM_NWCFG_SGMII_ENBL |
			    ZYNQ_GEM_NWCFG_PCS_SEL;
	}

	switch (priv->phydev->speed) {
	case SPEED_1000:
		nwconfig |= ZYNQ_GEM_NWCFG_SPEED1000;
		clk_rate = ZYNQ_GEM_FREQUENCY_1000;
		break;
	case SPEED_100:
		nwconfig |= ZYNQ_GEM_NWCFG_SPEED100;
		clk_rate = ZYNQ_GEM_FREQUENCY_100;
		break;
	case SPEED_10:
		clk_rate = ZYNQ_GEM_FREQUENCY_10;
		break;
	}
	nwcfg = readl(&regs->nwcfg);
	nwcfg |= nwconfig;
	if (nwcfg)
		writel(nwcfg, &regs->nwcfg);

#ifdef CONFIG_ARM64
	if (priv->interface == PHY_INTERFACE_MODE_SGMII &&
	    priv->int_pcs) {
		/*
		 * Disable AN for fixed link configuration, enable otherwise.
		 * Must be written after PCS_SEL is set in nwconfig,
		 * otherwise writes will not take effect.
		 */
		if (priv->phydev->phy_id != PHY_FIXED_ID) {
			writel(readl(&regs->pcscntrl) | ZYNQ_GEM_PCS_CTL_ANEG_ENBL,
			       &regs->pcscntrl);
			/*
			 * When the PHY link is already up, the PCS link needs
			 * to get re-checked
			 */
			if (priv->phydev->link) {
				u32 pcsstatus;

				pcsstatus = ZYNQ_GEM_PCSSTATUS_LINK |
					ZYNQ_GEM_PCSSTATUS_ANEG_COMPL;
				ret = wait_for_bit_le32(&regs->pcsstatus,
							pcsstatus,
							true, 5000, true);
				if (ret) {
					dev_warn(dev,
						 "no PCS (SGMII) link\n");
				} else {
					/*
					 * Some additional minimal delay seems
					 * to be needed so that the first
					 * packet will be sent correctly
					 */
					mdelay(1);
				}
			}
		} else {
			writel(readl(&regs->pcscntrl) & ~ZYNQ_GEM_PCS_CTL_ANEG_ENBL,
			       &regs->pcscntrl);
		}
	}
#endif

	ret = clk_get_rate(&priv->tx_clk);
	if (ret != clk_rate) {
		ret = clk_set_rate(&priv->tx_clk, clk_rate);
		if (IS_ERR_VALUE(ret)) {
			dev_err(dev, "failed to set tx clock rate %ld\n", clk_rate);
			return ret;
		}
	}

	ret = clk_enable(&priv->tx_clk);
	if (ret) {
		dev_err(dev, "failed to enable tx clock\n");
		return ret;
	}

	if (priv->clk_en_info & RXCLK_EN) {
		ret = clk_enable(&priv->rx_clk);
		if (ret) {
			dev_err(dev, "failed to enable rx clock\n");
			return ret;
		}
	}
	setbits_le32(&regs->nwctrl, ZYNQ_GEM_NWCTRL_RXEN_MASK |
					ZYNQ_GEM_NWCTRL_TXEN_MASK);

	return 0;
}

static int zynq_gem_send(struct udevice *dev, void *ptr, int len)
{
	dma_addr_t addr;
	u32 size;
	struct zynq_gem_priv *priv = dev_get_priv(dev);
	struct zynq_gem_regs *regs = priv->iobase;
	struct emac_bd *current_bd = &priv->tx_bd[1];

	/* Setup Tx BD */
	memset(priv->tx_bd, 0, sizeof(struct emac_bd));

	priv->tx_bd->addr = lower_32_bits((ulong)ptr);
#if defined(CONFIG_PHYS_64BIT)
	priv->tx_bd->addr_hi = upper_32_bits((ulong)ptr);
#endif
	priv->tx_bd->status = (len & ZYNQ_GEM_TXBUF_FRMLEN_MASK) |
			       ZYNQ_GEM_TXBUF_LAST_MASK;
	/* Dummy descriptor to mark it as the last in descriptor chain */
	current_bd->addr = 0x0;
#if defined(CONFIG_PHYS_64BIT)
	current_bd->addr_hi = 0x0;
#endif
	current_bd->status = ZYNQ_GEM_TXBUF_WRAP_MASK |
			     ZYNQ_GEM_TXBUF_LAST_MASK|
			     ZYNQ_GEM_TXBUF_USED_MASK;

	/* setup BD */
	writel(lower_32_bits((ulong)priv->tx_bd), &regs->txqbase);
#if defined(CONFIG_PHYS_64BIT)
	writel(upper_32_bits((ulong)priv->tx_bd), &regs->upper_txqbase);
#endif

	addr = (ulong) ptr;
	addr &= ~(ARCH_DMA_MINALIGN - 1);
	size = roundup(len, ARCH_DMA_MINALIGN);
	flush_dcache_range(addr, addr + size);
	barrier();

	/* Start transmit */
	setbits_le32(&regs->nwctrl, ZYNQ_GEM_NWCTRL_STARTTX_MASK);

	/* Read TX BD status */
	if (priv->tx_bd->status & ZYNQ_GEM_TXBUF_EXHAUSTED)
		printf("TX buffers exhausted in mid frame\n");

	return wait_for_bit_le32(&regs->txsr, ZYNQ_GEM_TSR_DONE,
				 true, 20000, true);
}

/* Do not check frame_recd flag in rx_status register 0x20 - just poll BD */
static int zynq_gem_recv(struct udevice *dev, int flags, uchar **packetp)
{
	int frame_len;
	dma_addr_t addr;
	struct zynq_gem_priv *priv = dev_get_priv(dev);
	struct emac_bd *current_bd = &priv->rx_bd[priv->rxbd_current];

	if (!(current_bd->addr & ZYNQ_GEM_RXBUF_NEW_MASK))
		return -1;

	if (!(current_bd->status &
			(ZYNQ_GEM_RXBUF_SOF_MASK | ZYNQ_GEM_RXBUF_EOF_MASK))) {
		printf("GEM: SOF or EOF not set for last buffer received!\n");
		return -1;
	}

	frame_len = current_bd->status & ZYNQ_GEM_RXBUF_LEN_MASK;
	if (!frame_len) {
		printf("%s: Zero size packet?\n", __func__);
		return -1;
	}

#if defined(CONFIG_PHYS_64BIT)
	addr = (dma_addr_t)((current_bd->addr & ZYNQ_GEM_RXBUF_ADD_MASK)
		      | ((dma_addr_t)current_bd->addr_hi << 32));
#else
	addr = current_bd->addr & ZYNQ_GEM_RXBUF_ADD_MASK;
#endif
	addr &= ~(ARCH_DMA_MINALIGN - 1);

	*packetp = (uchar *)(uintptr_t)addr;

	invalidate_dcache_range(addr, addr + roundup(PKTSIZE_ALIGN, ARCH_DMA_MINALIGN));
	barrier();

	return frame_len;
}

static int zynq_gem_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct zynq_gem_priv *priv = dev_get_priv(dev);
	struct emac_bd *current_bd = &priv->rx_bd[priv->rxbd_current];
	struct emac_bd *first_bd;
	dma_addr_t addr;

	if (current_bd->status & ZYNQ_GEM_RXBUF_SOF_MASK) {
		priv->rx_first_buf = priv->rxbd_current;
	} else {
		current_bd->addr &= ~ZYNQ_GEM_RXBUF_NEW_MASK;
		current_bd->status = 0xF0000000; /* FIXME */
	}

	if (current_bd->status & ZYNQ_GEM_RXBUF_EOF_MASK) {
		first_bd = &priv->rx_bd[priv->rx_first_buf];
		first_bd->addr &= ~ZYNQ_GEM_RXBUF_NEW_MASK;
		first_bd->status = 0xF0000000;
	}

	/* Flush the cache for the packet as well */
#if defined(CONFIG_PHYS_64BIT)
	addr = (dma_addr_t)((current_bd->addr & ZYNQ_GEM_RXBUF_ADD_MASK)
		| ((dma_addr_t)current_bd->addr_hi << 32));
#else
	addr = current_bd->addr & ZYNQ_GEM_RXBUF_ADD_MASK;
#endif
	flush_dcache_range(addr, addr + roundup(PKTSIZE_ALIGN,
						ARCH_DMA_MINALIGN));
	barrier();

	if ((++priv->rxbd_current) >= RX_BUF)
		priv->rxbd_current = 0;

	return 0;
}

static void zynq_gem_halt(struct udevice *dev)
{
	struct zynq_gem_priv *priv = dev_get_priv(dev);
	struct zynq_gem_regs *regs = priv->iobase;

	clrsetbits_le32(&regs->nwctrl, ZYNQ_GEM_NWCTRL_RXEN_MASK |
						ZYNQ_GEM_NWCTRL_TXEN_MASK, 0);
}

static int zynq_gem_miiphy_read(struct mii_dev *bus, int addr,
				int devad, int reg)
{
	struct zynq_gem_priv *priv = bus->priv;
	int ret;
	u16 val = 0;

	ret = phyread(priv, addr, reg, &val);
	debug("%s 0x%x, 0x%x, 0x%x, 0x%x\n", __func__, addr, reg, val, ret);
	return val;
}

static int zynq_gem_miiphy_write(struct mii_dev *bus, int addr, int devad,
				 int reg, u16 value)
{
	struct zynq_gem_priv *priv = bus->priv;

	debug("%s 0x%x, 0x%x, 0x%x\n", __func__, addr, reg, value);
	return phywrite(priv, addr, reg, value);
}

static int zynq_gem_reset_init(struct udevice *dev)
{
	struct zynq_gem_priv *priv = dev_get_priv(dev);
	int ret;

	ret = reset_get_bulk(dev, &priv->resets);
	if (ret == -ENOTSUPP || ret == -ENOENT)
		return 0;
	else if (ret)
		return ret;

	ret = reset_deassert_bulk(&priv->resets);
	if (ret) {
		reset_release_bulk(&priv->resets);
		return ret;
	}

	return 0;
}

static int gem_zynqmp_set_dynamic_config(struct udevice *dev)
{
	u32 pm_info[2];
	int ret;

	if (IS_ENABLED(CONFIG_ARCH_ZYNQMP) && IS_ENABLED(CONFIG_ZYNQMP_FIRMWARE)) {
		if (!zynqmp_pm_is_function_supported(PM_IOCTL,
						     IOCTL_SET_GEM_CONFIG)) {
			ret = ofnode_read_u32_array(dev_ofnode(dev),
						    "power-domains",
						    pm_info,
						    ARRAY_SIZE(pm_info));
			if (ret) {
				dev_err(dev,
					"Failed to read power-domains info\n");
				return ret;
			}

			ret = zynqmp_pm_set_gem_config(pm_info[1],
						       GEM_CONFIG_FIXED, 0);
			if (ret)
				return ret;

			ret = zynqmp_pm_set_gem_config(pm_info[1],
						       GEM_CONFIG_SGMII_MODE,
						       1);
			if (ret)
				return ret;
		}
	}

	return 0;
}

static int zynq_gem_probe(struct udevice *dev)
{
	void *bd_space;
	struct zynq_gem_priv *priv = dev_get_priv(dev);
	int ret;
	struct phy phy;

	if (priv->interface == PHY_INTERFACE_MODE_SGMII) {
		ret = generic_phy_get_by_index(dev, 0, &phy);
		if (!ret) {
			ret = generic_phy_init(&phy);
			if (ret)
				return ret;
		} else if (ret != -ENOENT) {
			debug("could not get phy (err %d)\n", ret);
			return ret;
		}
	}

	ret = zynq_gem_reset_init(dev);
	if (ret)
		return ret;

	/* Align rxbuffers to ARCH_DMA_MINALIGN */
	priv->rxbuffers = memalign(ARCH_DMA_MINALIGN, RX_BUF * PKTSIZE_ALIGN);
	if (!priv->rxbuffers)
		return -ENOMEM;

	memset(priv->rxbuffers, 0, RX_BUF * PKTSIZE_ALIGN);
	ulong addr = (ulong)priv->rxbuffers;
	flush_dcache_range(addr, addr + roundup(RX_BUF * PKTSIZE_ALIGN, ARCH_DMA_MINALIGN));
	barrier();

	/* Align bd_space to MMU_SECTION_SHIFT */
	bd_space = memalign(1 << MMU_SECTION_SHIFT, BD_SPACE);
	if (!bd_space) {
		ret = -ENOMEM;
		goto err1;
	}

	mmu_set_region_dcache_behaviour((phys_addr_t)bd_space,
					BD_SPACE, DCACHE_OFF);

	/* Initialize the bd spaces for tx and rx bd's */
	priv->tx_bd = (struct emac_bd *)bd_space;
	priv->rx_bd = (struct emac_bd *)((ulong)bd_space + BD_SEPRN_SPACE);

	ret = clk_get_by_name(dev, "tx_clk", &priv->tx_clk);
	if (ret < 0) {
		dev_err(dev, "failed to get tx_clock\n");
		goto err2;
	}

	if (priv->clk_en_info & RXCLK_EN) {
		ret = clk_get_by_name(dev, "rx_clk", &priv->rx_clk);
		if (ret < 0) {
			dev_err(dev, "failed to get rx_clock\n");
			goto err2;
		}
	}

	ret = clk_get_by_name(dev, "pclk", &priv->pclk);
	if (ret < 0) {
		dev_err(dev, "failed to get pclk clock\n");
		goto err2;
	}

	if (IS_ENABLED(CONFIG_DM_ETH_PHY))
		priv->bus = eth_phy_get_mdio_bus(dev);

	if (!priv->bus) {
		priv->bus = mdio_alloc();
		priv->bus->read = zynq_gem_miiphy_read;
		priv->bus->write = zynq_gem_miiphy_write;
		priv->bus->priv = priv;

		ret = mdio_register_seq(priv->bus, dev_seq(dev));
		if (ret)
			goto err2;
	}

	if (IS_ENABLED(CONFIG_DM_ETH_PHY))
		eth_phy_set_mdio_bus(dev, priv->bus);

	ret = zynq_phy_init(dev);
	if (ret)
		goto err3;

	if (priv->interface == PHY_INTERFACE_MODE_SGMII &&
	    generic_phy_valid(&phy)) {
		if (IS_ENABLED(CONFIG_DM_ETH_PHY)) {
			if (device_is_compatible(dev, "cdns,zynqmp-gem") ||
			    device_is_compatible(dev, "xlnx,zynqmp-gem")) {
				ret = gem_zynqmp_set_dynamic_config(dev);
				if (ret) {
					dev_err
					(dev,
					 "Failed to set gem dynamic config\n");
					return ret;
				}
			}
		}
		ret = generic_phy_power_on(&phy);
		if (ret)
			return ret;
	}

	printf("\nZYNQ GEM: %lx, mdio bus %lx, phyaddr %d, interface %s\n",
	       (ulong)priv->iobase, (ulong)priv->mdiobase, priv->phydev->addr,
	       phy_string_for_interface(priv->interface));

	return ret;

err3:
	mdio_unregister(priv->bus);
err2:
	free(priv->tx_bd);
err1:
	free(priv->rxbuffers);
	return ret;
}

static int zynq_gem_remove(struct udevice *dev)
{
	struct zynq_gem_priv *priv = dev_get_priv(dev);

	free(priv->phydev);
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);

	return 0;
}

static const struct eth_ops zynq_gem_ops = {
	.start			= zynq_gem_init,
	.send			= zynq_gem_send,
	.recv			= zynq_gem_recv,
	.free_pkt		= zynq_gem_free_pkt,
	.stop			= zynq_gem_halt,
	.write_hwaddr		= zynq_gem_setup_mac,
};

static int zynq_gem_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct zynq_gem_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args phandle_args;

	pdata->iobase = (phys_addr_t)dev_read_addr(dev);
	priv->iobase = (struct zynq_gem_regs *)pdata->iobase;
	priv->mdiobase = priv->iobase;
	/* Hardcode for now */
	priv->phyaddr = -1;

	if (!dev_read_phandle_with_args(dev, "phy-handle", NULL, 0, 0,
					&phandle_args)) {
		fdt_addr_t addr;
		ofnode parent;

		debug("phy-handle does exist %s\n", dev->name);
		if (!(IS_ENABLED(CONFIG_DM_ETH_PHY)))
			priv->phyaddr = ofnode_read_u32_default
					(phandle_args.node, "reg", -1);

		priv->phy_of_node = phandle_args.node;
		priv->max_speed = ofnode_read_u32_default(phandle_args.node,
							  "max-speed",
							  SPEED_1000);

		parent = ofnode_get_parent(phandle_args.node);
		if (ofnode_name_eq(parent, "mdio"))
			parent = ofnode_get_parent(parent);

		addr = ofnode_get_addr(parent);
		if (addr != FDT_ADDR_T_NONE) {
			debug("MDIO bus not found %s\n", dev->name);
			priv->mdiobase = (struct zynq_gem_regs *)addr;
		}
	}

	pdata->phy_interface = dev_read_phy_mode(dev);
	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA)
		return -EINVAL;
	priv->interface = pdata->phy_interface;

	priv->int_pcs = dev_read_bool(dev, "is-internal-pcspma");

	priv->clk_en_info = dev_get_driver_data(dev);

	return 0;
}

static const struct udevice_id zynq_gem_ids[] = {
	{ .compatible = "xlnx,versal-gem", .data = RXCLK_EN },
	{ .compatible = "cdns,versal-gem", .data = RXCLK_EN },
	{ .compatible = "xlnx,zynqmp-gem" },
	{ .compatible = "cdns,zynqmp-gem" },
	{ .compatible = "xlnx,zynq-gem" },
	{ .compatible = "cdns,zynq-gem" },
	{ .compatible = "cdns,gem" },
	{ }
};

U_BOOT_DRIVER(zynq_gem) = {
	.name	= "zynq_gem",
	.id	= UCLASS_ETH,
	.of_match = zynq_gem_ids,
	.of_to_plat = zynq_gem_of_to_plat,
	.probe	= zynq_gem_probe,
	.remove	= zynq_gem_remove,
	.ops	= &zynq_gem_ops,
	.priv_auto	= sizeof(struct zynq_gem_priv),
	.plat_auto	= sizeof(struct eth_pdata),
};
