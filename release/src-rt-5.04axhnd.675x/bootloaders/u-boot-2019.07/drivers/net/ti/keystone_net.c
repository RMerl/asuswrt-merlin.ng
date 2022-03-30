// SPDX-License-Identifier: GPL-2.0+
/*
 * Ethernet driver for TI K2HK EVM.
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */
#include <common.h>
#include <command.h>
#include <console.h>

#include <dm.h>
#include <dm/lists.h>

#include <net.h>
#include <phy.h>
#include <errno.h>
#include <miiphy.h>
#include <malloc.h>
#include <asm/ti-common/keystone_nav.h>
#include <asm/ti-common/keystone_net.h>
#include <asm/ti-common/keystone_serdes.h>
#include <asm/arch/psc_defs.h>

#include "cpsw_mdio.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef KEYSTONE2_EMAC_GIG_ENABLE
#define emac_gigabit_enable(x)	keystone2_eth_gigabit_enable(x)
#else
#define emac_gigabit_enable(x)	/* no gigabit to enable */
#endif

#define RX_BUFF_NUMS	24
#define RX_BUFF_LEN	1520
#define MAX_SIZE_STREAM_BUFFER RX_BUFF_LEN
#define SGMII_ANEG_TIMEOUT		4000

static u8 rx_buffs[RX_BUFF_NUMS * RX_BUFF_LEN] __aligned(16);

enum link_type {
	LINK_TYPE_SGMII_MAC_TO_MAC_AUTO		= 0,
	LINK_TYPE_SGMII_MAC_TO_PHY_MODE		= 1,
	LINK_TYPE_SGMII_MAC_TO_MAC_FORCED_MODE	= 2,
	LINK_TYPE_SGMII_MAC_TO_FIBRE_MODE	= 3,
	LINK_TYPE_SGMII_MAC_TO_PHY_NO_MDIO_MODE	= 4,
	LINK_TYPE_RGMII_LINK_MAC_PHY		= 5,
	LINK_TYPE_RGMII_LINK_MAC_MAC_FORCED	= 6,
	LINK_TYPE_RGMII_LINK_MAC_PHY_NO_MDIO	= 7,
	LINK_TYPE_10G_MAC_TO_PHY_MODE		= 10,
	LINK_TYPE_10G_MAC_TO_MAC_FORCED_MODE	= 11,
};

#define mac_hi(mac)     (((mac)[0] << 0) | ((mac)[1] << 8) |    \
			 ((mac)[2] << 16) | ((mac)[3] << 24))
#define mac_lo(mac)     (((mac)[4] << 0) | ((mac)[5] << 8))

#ifdef CONFIG_KSNET_NETCP_V1_0

#define EMAC_EMACSW_BASE_OFS		0x90800
#define EMAC_EMACSW_PORT_BASE_OFS	(EMAC_EMACSW_BASE_OFS + 0x60)

/* CPSW Switch slave registers */
#define CPGMACSL_REG_SA_LO		0x10
#define CPGMACSL_REG_SA_HI		0x14

#define DEVICE_EMACSW_BASE(base, x)	((base) + EMAC_EMACSW_PORT_BASE_OFS +  \
					 (x) * 0x30)

#elif defined(CONFIG_KSNET_NETCP_V1_5)

#define EMAC_EMACSW_PORT_BASE_OFS	0x222000

/* CPSW Switch slave registers */
#define CPGMACSL_REG_SA_LO		0x308
#define CPGMACSL_REG_SA_HI		0x30c

#define DEVICE_EMACSW_BASE(base, x)	((base) + EMAC_EMACSW_PORT_BASE_OFS +  \
					 (x) * 0x1000)

#endif


struct ks2_eth_priv {
	struct udevice			*dev;
	struct phy_device		*phydev;
	struct mii_dev			*mdio_bus;
	int				phy_addr;
	phy_interface_t			phy_if;
	int				phy_of_handle;
	int				sgmii_link_type;
	void				*mdio_base;
	struct rx_buff_desc		net_rx_buffs;
	struct pktdma_cfg		*netcp_pktdma;
	void				*hd;
	int				slave_port;
	enum link_type			link_type;
	bool				emac_open;
	bool				has_mdio;
};

static void  __attribute__((unused))
	keystone2_eth_gigabit_enable(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);

	/*
	 * Check if link detected is giga-bit
	 * If Gigabit mode detected, enable gigbit in MAC
	 */
	if (priv->has_mdio) {
		if (priv->phydev->speed != 1000)
			return;
	}

	writel(readl(DEVICE_EMACSL_BASE(priv->slave_port - 1) +
		     CPGMACSL_REG_CTL) |
	       EMAC_MACCONTROL_GIGFORCE | EMAC_MACCONTROL_GIGABIT_ENABLE,
	       DEVICE_EMACSL_BASE(priv->slave_port - 1) + CPGMACSL_REG_CTL);
}

#ifdef CONFIG_SOC_K2G
int keystone_rgmii_config(struct phy_device *phy_dev)
{
	unsigned int i, status;

	i = 0;
	do {
		if (i > SGMII_ANEG_TIMEOUT) {
			puts(" TIMEOUT !\n");
			phy_dev->link = 0;
			return 0;
		}

		if (ctrlc()) {
			puts("user interrupt!\n");
			phy_dev->link = 0;
			return -EINTR;
		}

		if ((i++ % 500) == 0)
			printf(".");

		udelay(1000);   /* 1 ms */
		status = readl(RGMII_STATUS_REG);
	} while (!(status & RGMII_REG_STATUS_LINK));

	puts(" done\n");

	return 0;
}
#else
int keystone_sgmii_config(struct phy_device *phy_dev, int port, int interface)
{
	unsigned int i, status, mask;
	unsigned int mr_adv_ability, control;

	switch (interface) {
	case SGMII_LINK_MAC_MAC_AUTONEG:
		mr_adv_ability	= (SGMII_REG_MR_ADV_ENABLE |
				   SGMII_REG_MR_ADV_LINK |
				   SGMII_REG_MR_ADV_FULL_DUPLEX |
				   SGMII_REG_MR_ADV_GIG_MODE);
		control		= (SGMII_REG_CONTROL_MASTER |
				   SGMII_REG_CONTROL_AUTONEG);

		break;
	case SGMII_LINK_MAC_PHY:
	case SGMII_LINK_MAC_PHY_FORCED:
		mr_adv_ability	= SGMII_REG_MR_ADV_ENABLE;
		control		= SGMII_REG_CONTROL_AUTONEG;

		break;
	case SGMII_LINK_MAC_MAC_FORCED:
		mr_adv_ability	= (SGMII_REG_MR_ADV_ENABLE |
				   SGMII_REG_MR_ADV_LINK |
				   SGMII_REG_MR_ADV_FULL_DUPLEX |
				   SGMII_REG_MR_ADV_GIG_MODE);
		control		= SGMII_REG_CONTROL_MASTER;

		break;
	case SGMII_LINK_MAC_FIBER:
		mr_adv_ability	= 0x20;
		control		= SGMII_REG_CONTROL_AUTONEG;

		break;
	default:
		mr_adv_ability	= SGMII_REG_MR_ADV_ENABLE;
		control		= SGMII_REG_CONTROL_AUTONEG;
	}

	__raw_writel(0, SGMII_CTL_REG(port));

	/*
	 * Wait for the SerDes pll to lock,
	 * but don't trap if lock is never read
	 */
	for (i = 0; i < 1000; i++)  {
		udelay(2000);
		status = __raw_readl(SGMII_STATUS_REG(port));
		if ((status & SGMII_REG_STATUS_LOCK) != 0)
			break;
	}

	__raw_writel(mr_adv_ability, SGMII_MRADV_REG(port));
	__raw_writel(control, SGMII_CTL_REG(port));


	mask = SGMII_REG_STATUS_LINK;

	if (control & SGMII_REG_CONTROL_AUTONEG)
		mask |= SGMII_REG_STATUS_AUTONEG;

	status = __raw_readl(SGMII_STATUS_REG(port));
	if ((status & mask) == mask)
		return 0;

	printf("\n%s Waiting for SGMII auto negotiation to complete",
	       phy_dev->dev->name);
	while ((status & mask) != mask) {
		/*
		 * Timeout reached ?
		 */
		if (i > SGMII_ANEG_TIMEOUT) {
			puts(" TIMEOUT !\n");
			phy_dev->link = 0;
			return 0;
		}

		if (ctrlc()) {
			puts("user interrupt!\n");
			phy_dev->link = 0;
			return -EINTR;
		}

		if ((i++ % 500) == 0)
			printf(".");

		udelay(1000);   /* 1 ms */
		status = __raw_readl(SGMII_STATUS_REG(port));
	}
	puts(" done\n");

	return 0;
}
#endif

int mac_sl_reset(u32 port)
{
	u32 i, v;

	if (port >= DEVICE_N_GMACSL_PORTS)
		return GMACSL_RET_INVALID_PORT;

	/* Set the soft reset bit */
	writel(CPGMAC_REG_RESET_VAL_RESET,
	       DEVICE_EMACSL_BASE(port) + CPGMACSL_REG_RESET);

	/* Wait for the bit to clear */
	for (i = 0; i < DEVICE_EMACSL_RESET_POLL_COUNT; i++) {
		v = readl(DEVICE_EMACSL_BASE(port) + CPGMACSL_REG_RESET);
		if ((v & CPGMAC_REG_RESET_VAL_RESET_MASK) !=
		    CPGMAC_REG_RESET_VAL_RESET)
			return GMACSL_RET_OK;
	}

	/* Timeout on the reset */
	return GMACSL_RET_WARN_RESET_INCOMPLETE;
}

int mac_sl_config(u_int16_t port, struct mac_sl_cfg *cfg)
{
	u32 v, i;
	int ret = GMACSL_RET_OK;

	if (port >= DEVICE_N_GMACSL_PORTS)
		return GMACSL_RET_INVALID_PORT;

	if (cfg->max_rx_len > CPGMAC_REG_MAXLEN_LEN) {
		cfg->max_rx_len = CPGMAC_REG_MAXLEN_LEN;
		ret = GMACSL_RET_WARN_MAXLEN_TOO_BIG;
	}

	/* Must wait if the device is undergoing reset */
	for (i = 0; i < DEVICE_EMACSL_RESET_POLL_COUNT; i++) {
		v = readl(DEVICE_EMACSL_BASE(port) + CPGMACSL_REG_RESET);
		if ((v & CPGMAC_REG_RESET_VAL_RESET_MASK) !=
		    CPGMAC_REG_RESET_VAL_RESET)
			break;
	}

	if (i == DEVICE_EMACSL_RESET_POLL_COUNT)
		return GMACSL_RET_CONFIG_FAIL_RESET_ACTIVE;

	writel(cfg->max_rx_len, DEVICE_EMACSL_BASE(port) + CPGMACSL_REG_MAXLEN);
	writel(cfg->ctl, DEVICE_EMACSL_BASE(port) + CPGMACSL_REG_CTL);

#ifndef CONFIG_SOC_K2HK
	/* Map RX packet flow priority to 0 */
	writel(0, DEVICE_EMACSL_BASE(port) + CPGMACSL_REG_RX_PRI_MAP);
#endif

	return ret;
}

int ethss_config(u32 ctl, u32 max_pkt_size)
{
	u32 i;

	/* Max length register */
	writel(max_pkt_size, DEVICE_CPSW_BASE + CPSW_REG_MAXLEN);

	/* Control register */
	writel(ctl, DEVICE_CPSW_BASE + CPSW_REG_CTL);

	/* All statistics enabled by default */
	writel(CPSW_REG_VAL_STAT_ENABLE_ALL,
	       DEVICE_CPSW_BASE + CPSW_REG_STAT_PORT_EN);

	/* Reset and enable the ALE */
	writel(CPSW_REG_VAL_ALE_CTL_RESET_AND_ENABLE |
	       CPSW_REG_VAL_ALE_CTL_BYPASS,
	       DEVICE_CPSW_BASE + CPSW_REG_ALE_CONTROL);

	/* All ports put into forward mode */
	for (i = 0; i < DEVICE_CPSW_NUM_PORTS; i++)
		writel(CPSW_REG_VAL_PORTCTL_FORWARD_MODE,
		       DEVICE_CPSW_BASE + CPSW_REG_ALE_PORTCTL(i));

	return 0;
}

int ethss_start(void)
{
	int i;
	struct mac_sl_cfg cfg;

	cfg.max_rx_len	= MAX_SIZE_STREAM_BUFFER;
	cfg.ctl		= GMACSL_ENABLE | GMACSL_RX_ENABLE_EXT_CTL;

	for (i = 0; i < DEVICE_N_GMACSL_PORTS; i++) {
		mac_sl_reset(i);
		mac_sl_config(i, &cfg);
	}

	return 0;
}

int ethss_stop(void)
{
	int i;

	for (i = 0; i < DEVICE_N_GMACSL_PORTS; i++)
		mac_sl_reset(i);

	return 0;
}

struct ks2_serdes ks2_serdes_sgmii_156p25mhz = {
	.clk = SERDES_CLOCK_156P25M,
	.rate = SERDES_RATE_5G,
	.rate_mode = SERDES_QUARTER_RATE,
	.intf = SERDES_PHY_SGMII,
	.loopback = 0,
};

#ifndef CONFIG_SOC_K2G
static void keystone2_net_serdes_setup(void)
{
	ks2_serdes_init(CONFIG_KSNET_SERDES_SGMII_BASE,
			&ks2_serdes_sgmii_156p25mhz,
			CONFIG_KSNET_SERDES_LANES_PER_SGMII);

#if defined(CONFIG_SOC_K2E) || defined(CONFIG_SOC_K2L)
	ks2_serdes_init(CONFIG_KSNET_SERDES_SGMII2_BASE,
			&ks2_serdes_sgmii_156p25mhz,
			CONFIG_KSNET_SERDES_LANES_PER_SGMII);
#endif

	/* wait till setup */
	udelay(5000);
}
#endif

static int ks2_eth_start(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);

#ifdef CONFIG_SOC_K2G
	keystone_rgmii_config(priv->phydev);
#else
	keystone_sgmii_config(priv->phydev, priv->slave_port - 1,
			      priv->sgmii_link_type);
#endif

	udelay(10000);

	/* On chip switch configuration */
	ethss_config(target_get_switch_ctl(), SWITCH_MAX_PKT_SIZE);

	qm_init();

	if (ksnav_init(priv->netcp_pktdma, &priv->net_rx_buffs)) {
		pr_err("ksnav_init failed\n");
		goto err_knav_init;
	}

	/*
	 * Streaming switch configuration. If not present this
	 * statement is defined to void in target.h.
	 * If present this is usually defined to a series of register writes
	 */
	hw_config_streaming_switch();

	if (priv->has_mdio) {
		phy_startup(priv->phydev);
		if (priv->phydev->link == 0) {
			pr_err("phy startup failed\n");
			goto err_phy_start;
		}
	}

	emac_gigabit_enable(dev);

	ethss_start();

	priv->emac_open = true;

	return 0;

err_phy_start:
	ksnav_close(priv->netcp_pktdma);
err_knav_init:
	qm_close();

	return -EFAULT;
}

static int ks2_eth_send(struct udevice *dev, void *packet, int length)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);

	genphy_update_link(priv->phydev);
	if (priv->phydev->link == 0)
		return -1;

	if (length < EMAC_MIN_ETHERNET_PKT_SIZE)
		length = EMAC_MIN_ETHERNET_PKT_SIZE;

	return ksnav_send(priv->netcp_pktdma, (u32 *)packet,
			  length, (priv->slave_port) << 16);
}

static int ks2_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);
	int  pkt_size;
	u32 *pkt = NULL;

	priv->hd = ksnav_recv(priv->netcp_pktdma, &pkt, &pkt_size);
	if (priv->hd == NULL)
		return -EAGAIN;

	*packetp = (uchar *)pkt;

	return pkt_size;
}

static int ks2_eth_free_pkt(struct udevice *dev, uchar *packet,
				   int length)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);

	ksnav_release_rxhd(priv->netcp_pktdma, priv->hd);

	return 0;
}

static void ks2_eth_stop(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);

	if (!priv->emac_open)
		return;
	ethss_stop();

	ksnav_close(priv->netcp_pktdma);
	qm_close();
	phy_shutdown(priv->phydev);
	priv->emac_open = false;
}

int ks2_eth_read_rom_hwaddr(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	u32 maca = 0;
	u32 macb = 0;

	/* Read the e-fuse mac address */
	if (priv->slave_port == 1) {
		maca = __raw_readl(MAC_ID_BASE_ADDR);
		macb = __raw_readl(MAC_ID_BASE_ADDR + 4);
	}

	pdata->enetaddr[0] = (macb >>  8) & 0xff;
	pdata->enetaddr[1] = (macb >>  0) & 0xff;
	pdata->enetaddr[2] = (maca >> 24) & 0xff;
	pdata->enetaddr[3] = (maca >> 16) & 0xff;
	pdata->enetaddr[4] = (maca >>  8) & 0xff;
	pdata->enetaddr[5] = (maca >>  0) & 0xff;

	return 0;
}

int ks2_eth_write_hwaddr(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);

	writel(mac_hi(pdata->enetaddr),
	       DEVICE_EMACSW_BASE(pdata->iobase, priv->slave_port - 1) +
				  CPGMACSL_REG_SA_HI);
	writel(mac_lo(pdata->enetaddr),
	       DEVICE_EMACSW_BASE(pdata->iobase, priv->slave_port - 1) +
				  CPGMACSL_REG_SA_LO);

	return 0;
}

static int ks2_eth_probe(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);
	struct mii_dev *mdio_bus;

	priv->dev = dev;
	priv->emac_open = false;

	/* These clock enables has to be moved to common location */
	if (cpu_is_k2g())
		writel(KS2_ETHERNET_RGMII, KS2_ETHERNET_CFG);

	/* By default, select PA PLL clock as PA clock source */
#ifndef CONFIG_SOC_K2G
	if (psc_enable_module(KS2_LPSC_PA))
		return -EACCES;
#endif
	if (psc_enable_module(KS2_LPSC_CPGMAC))
		return -EACCES;
	if (psc_enable_module(KS2_LPSC_CRYPTO))
		return -EACCES;

	if (cpu_is_k2e() || cpu_is_k2l())
		pll_pa_clk_sel();

	priv->net_rx_buffs.buff_ptr = rx_buffs;
	priv->net_rx_buffs.num_buffs = RX_BUFF_NUMS;
	priv->net_rx_buffs.buff_len = RX_BUFF_LEN;

	if (priv->slave_port == 1) {
#ifndef CONFIG_SOC_K2G
		keystone2_net_serdes_setup();
#endif
		/*
		 * Register MDIO bus for slave 0 only, other slave have
		 * to re-use the same
		 */
		mdio_bus = cpsw_mdio_init("ethernet-mdio",
					  (u32)priv->mdio_base,
					  EMAC_MDIO_CLOCK_FREQ,
					  EMAC_MDIO_BUS_FREQ);
		if (!mdio_bus) {
			pr_err("MDIO alloc failed\n");
			return -ENOMEM;
		}
		priv->mdio_bus = mdio_bus;
	} else {
		/* Get the MDIO bus from slave 0 device */
		struct ks2_eth_priv *parent_priv;

		parent_priv = dev_get_priv(dev->parent);
		priv->mdio_bus = parent_priv->mdio_bus;
		priv->mdio_base = parent_priv->mdio_base;
	}

	priv->netcp_pktdma = &netcp_pktdma;

	if (priv->has_mdio) {
		priv->phydev = phy_connect(priv->mdio_bus, priv->phy_addr,
					   dev, priv->phy_if);
#ifdef CONFIG_DM_ETH
	if (priv->phy_of_handle)
		priv->phydev->node = offset_to_ofnode(priv->phy_of_handle);
#endif
		phy_config(priv->phydev);
	}

	return 0;
}

int ks2_eth_remove(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);

	cpsw_mdio_free(priv->mdio_bus);

	return 0;
}

static const struct eth_ops ks2_eth_ops = {
	.start			= ks2_eth_start,
	.send			= ks2_eth_send,
	.recv			= ks2_eth_recv,
	.free_pkt		= ks2_eth_free_pkt,
	.stop			= ks2_eth_stop,
	.read_rom_hwaddr	= ks2_eth_read_rom_hwaddr,
	.write_hwaddr		= ks2_eth_write_hwaddr,
};

static int ks2_eth_bind_slaves(struct udevice *dev, int gbe, int *gbe_0)
{
	const void *fdt = gd->fdt_blob;
	struct udevice *sl_dev;
	int interfaces;
	int sec_slave;
	int slave;
	int ret;
	char *slave_name;

	interfaces = fdt_subnode_offset(fdt, gbe, "interfaces");
	fdt_for_each_subnode(slave, fdt, interfaces) {
		int slave_no;

		slave_no = fdtdec_get_int(fdt, slave, "slave-port", -ENOENT);
		if (slave_no == -ENOENT)
			continue;

		if (slave_no == 0) {
			/* This is the current eth device */
			*gbe_0 = slave;
		} else {
			/* Slave devices to be registered */
			slave_name = malloc(20);
			snprintf(slave_name, 20, "netcp@slave-%d", slave_no);
			ret = device_bind_driver_to_node(dev, "eth_ks2_sl",
					slave_name, offset_to_ofnode(slave),
					&sl_dev);
			if (ret) {
				pr_err("ks2_net - not able to bind slave interfaces\n");
				return ret;
			}
		}
	}

	sec_slave = fdt_subnode_offset(fdt, gbe, "secondary-slave-ports");
	fdt_for_each_subnode(slave, fdt, sec_slave) {
		int slave_no;

		slave_no = fdtdec_get_int(fdt, slave, "slave-port", -ENOENT);
		if (slave_no == -ENOENT)
			continue;

		/* Slave devices to be registered */
		slave_name = malloc(20);
		snprintf(slave_name, 20, "netcp@slave-%d", slave_no);
		ret = device_bind_driver_to_node(dev, "eth_ks2_sl", slave_name,
					offset_to_ofnode(slave), &sl_dev);
		if (ret) {
			pr_err("ks2_net - not able to bind slave interfaces\n");
			return ret;
		}
	}

	return 0;
}

static int ks2_eth_parse_slave_interface(int netcp, int slave,
					 struct ks2_eth_priv *priv,
					 struct eth_pdata *pdata)
{
	const void *fdt = gd->fdt_blob;
	int mdio;
	int phy;
	int dma_count;
	u32 dma_channel[8];
	const char *phy_mode;

	priv->slave_port = fdtdec_get_int(fdt, slave, "slave-port", -1);
	priv->net_rx_buffs.rx_flow = priv->slave_port * 8;

	/* U-Boot slave port number starts with 1 instead of 0 */
	priv->slave_port += 1;

	dma_count = fdtdec_get_int_array_count(fdt, netcp,
					       "ti,navigator-dmas",
					       dma_channel, 8);

	if (dma_count > (2 * priv->slave_port)) {
		int dma_idx;

		dma_idx = priv->slave_port * 2 - 1;
		priv->net_rx_buffs.rx_flow = dma_channel[dma_idx];
	}

	priv->link_type = fdtdec_get_int(fdt, slave, "link-interface", -1);

	phy = fdtdec_lookup_phandle(fdt, slave, "phy-handle");

	if (phy >= 0) {
		priv->phy_of_handle = phy;
		priv->phy_addr = fdtdec_get_int(fdt, phy, "reg", -1);

		mdio = fdt_parent_offset(fdt, phy);
		if (mdio < 0) {
			pr_err("mdio dt not found\n");
			return -ENODEV;
		}
		priv->mdio_base = (void *)fdtdec_get_addr(fdt, mdio, "reg");
	}

	if (priv->link_type == LINK_TYPE_SGMII_MAC_TO_PHY_MODE) {
		priv->phy_if = PHY_INTERFACE_MODE_SGMII;
		pdata->phy_interface = priv->phy_if;
		priv->sgmii_link_type = SGMII_LINK_MAC_PHY;
		priv->has_mdio = true;
	} else if (priv->link_type == LINK_TYPE_RGMII_LINK_MAC_PHY) {
		phy_mode = fdt_getprop(fdt, slave, "phy-mode", NULL);
		if (phy_mode) {
			priv->phy_if = phy_get_interface_by_name(phy_mode);
			if (priv->phy_if != PHY_INTERFACE_MODE_RGMII &&
			    priv->phy_if != PHY_INTERFACE_MODE_RGMII_ID &&
			    priv->phy_if != PHY_INTERFACE_MODE_RGMII_RXID &&
			    priv->phy_if != PHY_INTERFACE_MODE_RGMII_TXID) {
				pr_err("invalid phy-mode\n");
				return -EINVAL;
			}
		} else {
			priv->phy_if = PHY_INTERFACE_MODE_RGMII;
		}
		pdata->phy_interface = priv->phy_if;
		priv->has_mdio = true;
	}

	return 0;
}

static int ks2_sl_eth_ofdata_to_platdata(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	const void *fdt = gd->fdt_blob;
	int slave = dev_of_offset(dev);
	int interfaces;
	int gbe;
	int netcp_devices;
	int netcp;

	interfaces = fdt_parent_offset(fdt, slave);
	gbe = fdt_parent_offset(fdt, interfaces);
	netcp_devices = fdt_parent_offset(fdt, gbe);
	netcp = fdt_parent_offset(fdt, netcp_devices);

	ks2_eth_parse_slave_interface(netcp, slave, priv, pdata);

	pdata->iobase = fdtdec_get_addr(fdt, netcp, "reg");

	return 0;
}

static int ks2_eth_ofdata_to_platdata(struct udevice *dev)
{
	struct ks2_eth_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	const void *fdt = gd->fdt_blob;
	int gbe_0 = -ENODEV;
	int netcp_devices;
	int gbe;

	netcp_devices = fdt_subnode_offset(fdt, dev_of_offset(dev),
					   "netcp-devices");
	gbe = fdt_subnode_offset(fdt, netcp_devices, "gbe");

	ks2_eth_bind_slaves(dev, gbe, &gbe_0);

	ks2_eth_parse_slave_interface(dev_of_offset(dev), gbe_0, priv, pdata);

	pdata->iobase = devfdt_get_addr(dev);

	return 0;
}

static const struct udevice_id ks2_eth_ids[] = {
	{ .compatible = "ti,netcp-1.0" },
	{ }
};

U_BOOT_DRIVER(eth_ks2_slave) = {
	.name	= "eth_ks2_sl",
	.id	= UCLASS_ETH,
	.ofdata_to_platdata = ks2_sl_eth_ofdata_to_platdata,
	.probe	= ks2_eth_probe,
	.remove	= ks2_eth_remove,
	.ops	= &ks2_eth_ops,
	.priv_auto_alloc_size = sizeof(struct ks2_eth_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};

U_BOOT_DRIVER(eth_ks2) = {
	.name	= "eth_ks2",
	.id	= UCLASS_ETH,
	.of_match = ks2_eth_ids,
	.ofdata_to_platdata = ks2_eth_ofdata_to_platdata,
	.probe	= ks2_eth_probe,
	.remove	= ks2_eth_remove,
	.ops	= &ks2_eth_ops,
	.priv_auto_alloc_size = sizeof(struct ks2_eth_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
