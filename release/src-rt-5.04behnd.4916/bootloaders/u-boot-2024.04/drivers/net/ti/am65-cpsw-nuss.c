// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments K3 AM65 Ethernet Switch SubSystem Driver
 *
 * Copyright (C) 2019, Texas Instruments, Incorporated
 *
 */

#include <common.h>
#include <malloc.h>
#include <asm/cache.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <dma-uclass.h>
#include <dm/of_access.h>
#include <miiphy.h>
#include <net.h>
#include <phy.h>
#include <power-domain.h>
#include <regmap.h>
#include <soc.h>
#include <syscon.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/soc/ti/ti-udma.h>

#include "cpsw_mdio.h"

#define AM65_CPSW_CPSWNU_MAX_PORTS 9

#define AM65_CPSW_SS_BASE		0x0
#define AM65_CPSW_SGMII_BASE	0x100
#define AM65_CPSW_MDIO_BASE	0xf00
#define AM65_CPSW_XGMII_BASE	0x2100
#define AM65_CPSW_CPSW_NU_BASE	0x20000
#define AM65_CPSW_CPSW_NU_ALE_BASE 0x1e000

#define AM65_CPSW_CPSW_NU_PORTS_OFFSET	0x1000
#define AM65_CPSW_CPSW_NU_PORT_MACSL_OFFSET	0x330

#define AM65_CPSW_MDIO_BUS_FREQ_DEF 1000000

#define AM65_CPSW_CTL_REG			0x4
#define AM65_CPSW_STAT_PORT_EN_REG	0x14
#define AM65_CPSW_PTYPE_REG		0x18

#define AM65_CPSW_CTL_REG_P0_ENABLE			BIT(2)
#define AM65_CPSW_CTL_REG_P0_TX_CRC_REMOVE		BIT(13)
#define AM65_CPSW_CTL_REG_P0_RX_PAD			BIT(14)

#define AM65_CPSW_P0_FLOW_ID_REG			0x8
#define AM65_CPSW_PN_RX_MAXLEN_REG		0x24
#define AM65_CPSW_PN_REG_SA_L			0x308
#define AM65_CPSW_PN_REG_SA_H			0x30c

#define AM65_CPSW_SGMII_CONTROL_REG             0x010
#define AM65_CPSW_SGMII_MR_ADV_ABILITY_REG      0x018
#define AM65_CPSW_SGMII_CONTROL_MR_AN_ENABLE    BIT(0)

#define ADVERTISE_SGMII				0x1

#define AM65_CPSW_ALE_CTL_REG			0x8
#define AM65_CPSW_ALE_CTL_REG_ENABLE		BIT(31)
#define AM65_CPSW_ALE_CTL_REG_RESET_TBL		BIT(30)
#define AM65_CPSW_ALE_CTL_REG_BYPASS		BIT(4)
#define AM65_CPSW_ALE_PN_CTL_REG(x)		(0x40 + (x) * 4)
#define AM65_CPSW_ALE_PN_CTL_REG_MODE_FORWARD	0x3
#define AM65_CPSW_ALE_PN_CTL_REG_MAC_ONLY	BIT(11)

#define AM65_CPSW_ALE_THREADMAPDEF_REG		0x134
#define AM65_CPSW_ALE_DEFTHREAD_EN		BIT(15)

#define AM65_CPSW_MACSL_CTL_REG			0x0
#define AM65_CPSW_MACSL_CTL_REG_IFCTL_A		BIT(15)
#define AM65_CPSW_MACSL_CTL_EXT_EN		BIT(18)
#define AM65_CPSW_MACSL_CTL_REG_GIG		BIT(7)
#define AM65_CPSW_MACSL_CTL_REG_GMII_EN		BIT(5)
#define AM65_CPSW_MACSL_CTL_REG_LOOPBACK	BIT(1)
#define AM65_CPSW_MACSL_CTL_REG_FULL_DUPLEX	BIT(0)
#define AM65_CPSW_MACSL_RESET_REG		0x8
#define AM65_CPSW_MACSL_RESET_REG_RESET		BIT(0)
#define AM65_CPSW_MACSL_STATUS_REG		0x4
#define AM65_CPSW_MACSL_RESET_REG_PN_IDLE	BIT(31)
#define AM65_CPSW_MACSL_RESET_REG_PN_E_IDLE	BIT(30)
#define AM65_CPSW_MACSL_RESET_REG_PN_P_IDLE	BIT(29)
#define AM65_CPSW_MACSL_RESET_REG_PN_TX_IDLE	BIT(28)
#define AM65_CPSW_MACSL_RESET_REG_IDLE_MASK \
	(AM65_CPSW_MACSL_RESET_REG_PN_IDLE | \
	 AM65_CPSW_MACSL_RESET_REG_PN_E_IDLE | \
	 AM65_CPSW_MACSL_RESET_REG_PN_P_IDLE | \
	 AM65_CPSW_MACSL_RESET_REG_PN_TX_IDLE)

#define AM65_CPSW_CPPI_PKT_TYPE			0x7

#define DEFAULT_GPIO_RESET_DELAY		10

struct am65_cpsw_port {
	fdt_addr_t	port_base;
	fdt_addr_t	port_sgmii_base;
	fdt_addr_t	macsl_base;
	bool		disabled;
	u32		mac_control;
};

struct am65_cpsw_common {
	struct udevice		*dev;
	fdt_addr_t		ss_base;
	fdt_addr_t		cpsw_base;
	fdt_addr_t		mdio_base;
	fdt_addr_t		ale_base;

	struct clk		fclk;
	struct power_domain	pwrdmn;

	u32			port_num;
	struct am65_cpsw_port	ports[AM65_CPSW_CPSWNU_MAX_PORTS];

	struct mii_dev		*bus;
	u32			bus_freq;

	struct gpio_desc	mdio_gpio_reset;
	u32			reset_delay_us;
	u32			reset_post_delay_us;

	struct dma		dma_tx;
	struct dma		dma_rx;
	u32			rx_next;
	u32			rx_pend;
	bool			started;
};

struct am65_cpsw_priv {
	struct udevice		*dev;
	struct am65_cpsw_common	*cpsw_common;
	u32			port_id;

	struct phy_device	*phydev;
	bool			has_phy;
	ofnode			phy_node;
	u32			phy_addr;

	bool			mdio_manual_mode;
};

#ifdef PKTSIZE_ALIGN
#define UDMA_RX_BUF_SIZE PKTSIZE_ALIGN
#else
#define UDMA_RX_BUF_SIZE ALIGN(1522, ARCH_DMA_MINALIGN)
#endif

#ifdef PKTBUFSRX
#define UDMA_RX_DESC_NUM PKTBUFSRX
#else
#define UDMA_RX_DESC_NUM 4
#endif

#define mac_hi(mac)	(((mac)[0] << 0) | ((mac)[1] << 8) |    \
			 ((mac)[2] << 16) | ((mac)[3] << 24))
#define mac_lo(mac)	(((mac)[4] << 0) | ((mac)[5] << 8))

static void am65_cpsw_set_sl_mac(struct am65_cpsw_port *slave,
				 unsigned char *addr)
{
	writel(mac_hi(addr),
	       slave->port_base + AM65_CPSW_PN_REG_SA_H);
	writel(mac_lo(addr),
	       slave->port_base + AM65_CPSW_PN_REG_SA_L);
}

int am65_cpsw_macsl_reset(struct am65_cpsw_port *slave)
{
	u32 i = 100;

	/* Set the soft reset bit */
	writel(AM65_CPSW_MACSL_RESET_REG_RESET,
	       slave->macsl_base + AM65_CPSW_MACSL_RESET_REG);

	while ((readl(slave->macsl_base + AM65_CPSW_MACSL_RESET_REG) &
		AM65_CPSW_MACSL_RESET_REG_RESET) && i--)
		cpu_relax();

	/* Timeout on the reset */
	return i;
}

static int am65_cpsw_macsl_wait_for_idle(struct am65_cpsw_port *slave)
{
	u32 i = 100;

	while ((readl(slave->macsl_base + AM65_CPSW_MACSL_STATUS_REG) &
		AM65_CPSW_MACSL_RESET_REG_IDLE_MASK) && i--)
		cpu_relax();

	return i;
}

static int am65_cpsw_update_link(struct am65_cpsw_priv *priv)
{
	struct am65_cpsw_common	*common = priv->cpsw_common;
	struct am65_cpsw_port *port = &common->ports[priv->port_id];
	struct phy_device *phy = priv->phydev;
	u32 mac_control = 0;

	if (phy->link) { /* link up */
		mac_control = /*AM65_CPSW_MACSL_CTL_REG_LOOPBACK |*/
			      AM65_CPSW_MACSL_CTL_REG_GMII_EN;
		if (phy->speed == 1000)
			mac_control |= AM65_CPSW_MACSL_CTL_REG_GIG;
		if (phy->speed == 10 && phy_interface_is_rgmii(phy))
			/* Can be used with in band mode only */
			mac_control |= AM65_CPSW_MACSL_CTL_EXT_EN;
		if (phy->duplex == DUPLEX_FULL)
			mac_control |= AM65_CPSW_MACSL_CTL_REG_FULL_DUPLEX;
		if (phy->speed == 100)
			mac_control |= AM65_CPSW_MACSL_CTL_REG_IFCTL_A;
		if (phy->interface == PHY_INTERFACE_MODE_SGMII)
			mac_control |= AM65_CPSW_MACSL_CTL_EXT_EN;
	}

	if (mac_control == port->mac_control)
		goto out;

	if (mac_control) {
		printf("link up on port %d, speed %d, %s duplex\n",
		       priv->port_id, phy->speed,
		       (phy->duplex == DUPLEX_FULL) ? "full" : "half");
	} else {
		printf("link down on port %d\n", priv->port_id);
	}

	writel(mac_control, port->macsl_base + AM65_CPSW_MACSL_CTL_REG);
	port->mac_control = mac_control;

out:
	return phy->link;
}

#define AM65_GMII_SEL_PORT_OFFS(x)	(0x4 * ((x) - 1))

#define AM65_GMII_SEL_MODE_MII		0
#define AM65_GMII_SEL_MODE_RMII		1
#define AM65_GMII_SEL_MODE_RGMII	2
#define AM65_GMII_SEL_MODE_SGMII	3

#define AM65_GMII_SEL_RGMII_IDMODE	BIT(4)

static int am65_cpsw_gmii_sel_k3(struct am65_cpsw_priv *priv,
				 phy_interface_t phy_mode)
{
	struct udevice *dev = priv->dev;
	u32 offset, reg, phandle;
	bool rgmii_id = false;
	fdt_addr_t gmii_sel;
	u32 mode = 0;
	ofnode node;
	int ret;

	ret = ofnode_read_u32(dev_ofnode(dev), "phys", &phandle);
	if (ret)
		return ret;

	ret = ofnode_read_u32_index(dev_ofnode(dev), "phys", 1, &offset);
	if (ret)
		return ret;

	node = ofnode_get_by_phandle(phandle);
	if (!ofnode_valid(node))
		return -ENODEV;

	gmii_sel = ofnode_get_addr(node);
	if (gmii_sel == FDT_ADDR_T_NONE)
		return -ENODEV;

	gmii_sel += AM65_GMII_SEL_PORT_OFFS(offset);
	reg = readl(gmii_sel);

	dev_dbg(dev, "old gmii_sel: %08x\n", reg);

	switch (phy_mode) {
	case PHY_INTERFACE_MODE_RMII:
		mode = AM65_GMII_SEL_MODE_RMII;
		break;

	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_RXID:
		mode = AM65_GMII_SEL_MODE_RGMII;
		break;

	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		mode = AM65_GMII_SEL_MODE_RGMII;
		rgmii_id = true;
		break;

	case PHY_INTERFACE_MODE_SGMII:
		mode = AM65_GMII_SEL_MODE_SGMII;
		break;

	default:
		dev_warn(dev,
			 "Unsupported PHY mode: %u. Defaulting to MII.\n",
			 phy_mode);
		/* fallthrough */
	case PHY_INTERFACE_MODE_MII:
		mode = AM65_GMII_SEL_MODE_MII;
		break;
	};

	if (rgmii_id)
		mode |= AM65_GMII_SEL_RGMII_IDMODE;

	reg = mode;
	dev_dbg(dev, "gmii_sel PHY mode: %u, new gmii_sel: %08x\n",
		phy_mode, reg);
	writel(reg, gmii_sel);

	reg = readl(gmii_sel);
	if (reg != mode) {
		dev_err(dev,
			"gmii_sel PHY mode NOT SET!: requested: %08x, gmii_sel: %08x\n",
			mode, reg);
		return 0;
	}

	return 0;
}

static int am65_cpsw_start(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct am65_cpsw_priv *priv = dev_get_priv(dev);
	struct am65_cpsw_common	*common = priv->cpsw_common;
	struct am65_cpsw_port *port = &common->ports[priv->port_id];
	struct am65_cpsw_port *port0 = &common->ports[0];
	struct ti_udma_drv_chan_cfg_data *dma_rx_cfg_data;
	int ret, i;

	ret = power_domain_on(&common->pwrdmn);
	if (ret) {
		dev_err(dev, "power_domain_on() failed %d\n", ret);
		goto out;
	}

	ret = clk_enable(&common->fclk);
	if (ret) {
		dev_err(dev, "clk enabled failed %d\n", ret);
		goto err_off_pwrdm;
	}

	common->rx_next = 0;
	common->rx_pend = 0;
	ret = dma_get_by_name(common->dev, "tx0", &common->dma_tx);
	if (ret) {
		dev_err(dev, "TX dma get failed %d\n", ret);
		goto err_off_clk;
	}
	ret = dma_get_by_name(common->dev, "rx", &common->dma_rx);
	if (ret) {
		dev_err(dev, "RX dma get failed %d\n", ret);
		goto err_free_tx;
	}

	for (i = 0; i < UDMA_RX_DESC_NUM; i++) {
		ret = dma_prepare_rcv_buf(&common->dma_rx,
					  net_rx_packets[i],
					  UDMA_RX_BUF_SIZE);
		if (ret) {
			dev_err(dev, "RX dma add buf failed %d\n", ret);
			goto err_free_tx;
		}
	}

	ret = dma_enable(&common->dma_tx);
	if (ret) {
		dev_err(dev, "TX dma_enable failed %d\n", ret);
		goto err_free_rx;
	}
	ret = dma_enable(&common->dma_rx);
	if (ret) {
		dev_err(dev, "RX dma_enable failed %d\n", ret);
		goto err_dis_tx;
	}

	/* Control register */
	writel(AM65_CPSW_CTL_REG_P0_ENABLE |
	       AM65_CPSW_CTL_REG_P0_TX_CRC_REMOVE |
	       AM65_CPSW_CTL_REG_P0_RX_PAD,
	       common->cpsw_base + AM65_CPSW_CTL_REG);

	/* disable priority elevation */
	writel(0, common->cpsw_base + AM65_CPSW_PTYPE_REG);

	/* enable statistics */
	writel(BIT(0) | BIT(priv->port_id),
	       common->cpsw_base + AM65_CPSW_STAT_PORT_EN_REG);

	/* Port 0  length register */
	writel(PKTSIZE_ALIGN, port0->port_base + AM65_CPSW_PN_RX_MAXLEN_REG);

	/* set base flow_id */
	dma_get_cfg(&common->dma_rx, 0, (void **)&dma_rx_cfg_data);
	writel(dma_rx_cfg_data->flow_id_base,
	       port0->port_base + AM65_CPSW_P0_FLOW_ID_REG);
	dev_info(dev, "K3 CPSW: rflow_id_base: %u\n",
		 dma_rx_cfg_data->flow_id_base);

	/* Reset and enable the ALE */
	writel(AM65_CPSW_ALE_CTL_REG_ENABLE | AM65_CPSW_ALE_CTL_REG_RESET_TBL |
	       AM65_CPSW_ALE_CTL_REG_BYPASS,
	       common->ale_base + AM65_CPSW_ALE_CTL_REG);

	/* port 0 put into forward mode */
	writel(AM65_CPSW_ALE_PN_CTL_REG_MODE_FORWARD,
	       common->ale_base + AM65_CPSW_ALE_PN_CTL_REG(0));

	writel(AM65_CPSW_ALE_DEFTHREAD_EN,
	       common->ale_base + AM65_CPSW_ALE_THREADMAPDEF_REG);

	/* PORT x configuration */

	/* Port x Max length register */
	writel(PKTSIZE_ALIGN, port->port_base + AM65_CPSW_PN_RX_MAXLEN_REG);

	/* Port x set mac */
	am65_cpsw_set_sl_mac(port, pdata->enetaddr);

	/* Port x ALE: mac_only, Forwarding */
	writel(AM65_CPSW_ALE_PN_CTL_REG_MAC_ONLY |
	       AM65_CPSW_ALE_PN_CTL_REG_MODE_FORWARD,
	       common->ale_base + AM65_CPSW_ALE_PN_CTL_REG(priv->port_id));

	port->mac_control = 0;
	if (!am65_cpsw_macsl_reset(port)) {
		dev_err(dev, "mac_sl reset failed\n");
		ret = -EFAULT;
		goto err_dis_rx;
	}

	if (priv->phydev->interface == PHY_INTERFACE_MODE_SGMII) {
		writel(ADVERTISE_SGMII,
		       port->port_sgmii_base + AM65_CPSW_SGMII_MR_ADV_ABILITY_REG);
		writel(AM65_CPSW_SGMII_CONTROL_MR_AN_ENABLE,
		       port->port_sgmii_base + AM65_CPSW_SGMII_CONTROL_REG);
	}

	ret = phy_startup(priv->phydev);
	if (ret) {
		dev_err(dev, "phy_startup failed\n");
		goto err_dis_rx;
	}

	ret = am65_cpsw_update_link(priv);
	if (!ret) {
		ret = -ENODEV;
		goto err_phy_shutdown;
	}

	common->started = true;

	return 0;

err_phy_shutdown:
	phy_shutdown(priv->phydev);
err_dis_rx:
	/* disable ports */
	writel(0, common->ale_base + AM65_CPSW_ALE_PN_CTL_REG(priv->port_id));
	writel(0, common->ale_base + AM65_CPSW_ALE_PN_CTL_REG(0));
	if (!am65_cpsw_macsl_wait_for_idle(port))
		dev_err(dev, "mac_sl idle timeout\n");
	writel(0, port->macsl_base + AM65_CPSW_MACSL_CTL_REG);
	writel(0, common->ale_base + AM65_CPSW_ALE_CTL_REG);
	writel(0, common->cpsw_base + AM65_CPSW_CTL_REG);

	dma_disable(&common->dma_rx);
err_dis_tx:
	dma_disable(&common->dma_tx);
err_free_rx:
	dma_free(&common->dma_rx);
err_free_tx:
	dma_free(&common->dma_tx);
err_off_clk:
	clk_disable(&common->fclk);
err_off_pwrdm:
	power_domain_off(&common->pwrdmn);
out:
	dev_err(dev, "%s end error\n", __func__);

	return ret;
}

static int am65_cpsw_send(struct udevice *dev, void *packet, int length)
{
	struct am65_cpsw_priv *priv = dev_get_priv(dev);
	struct am65_cpsw_common	*common = priv->cpsw_common;
	struct ti_udma_drv_packet_data packet_data;
	int ret;

	packet_data.pkt_type = AM65_CPSW_CPPI_PKT_TYPE;
	packet_data.dest_tag = priv->port_id;
	ret = dma_send(&common->dma_tx, packet, length, &packet_data);
	if (ret) {
		dev_err(dev, "TX dma_send failed %d\n", ret);
		return ret;
	}

	return 0;
}

static int am65_cpsw_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct am65_cpsw_priv *priv = dev_get_priv(dev);
	struct am65_cpsw_common	*common = priv->cpsw_common;

	/* try to receive a new packet */
	return dma_receive(&common->dma_rx, (void **)packetp, NULL);
}

static int am65_cpsw_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct am65_cpsw_priv *priv = dev_get_priv(dev);
	struct am65_cpsw_common	*common = priv->cpsw_common;
	int ret;

	if (length > 0) {
		u32 pkt = common->rx_next % UDMA_RX_DESC_NUM;

		ret = dma_prepare_rcv_buf(&common->dma_rx,
					  net_rx_packets[pkt],
					  UDMA_RX_BUF_SIZE);
		if (ret)
			dev_err(dev, "RX dma free_pkt failed %d\n", ret);
		common->rx_next++;
	}

	return 0;
}

static void am65_cpsw_stop(struct udevice *dev)
{
	struct am65_cpsw_priv *priv = dev_get_priv(dev);
	struct am65_cpsw_common *common = priv->cpsw_common;
	struct am65_cpsw_port *port = &common->ports[priv->port_id];

	if (!common->started)
		return;

	phy_shutdown(priv->phydev);

	writel(0, common->ale_base + AM65_CPSW_ALE_PN_CTL_REG(priv->port_id));
	writel(0, common->ale_base + AM65_CPSW_ALE_PN_CTL_REG(0));
	if (!am65_cpsw_macsl_wait_for_idle(port))
		dev_err(dev, "mac_sl idle timeout\n");
	writel(0, port->macsl_base + AM65_CPSW_MACSL_CTL_REG);
	writel(0, common->ale_base + AM65_CPSW_ALE_CTL_REG);
	writel(0, common->cpsw_base + AM65_CPSW_CTL_REG);

	dma_disable(&common->dma_tx);
	dma_free(&common->dma_tx);

	dma_disable(&common->dma_rx);
	dma_free(&common->dma_rx);

	common->started = false;
}

static int am65_cpsw_am654_get_efuse_macid(struct udevice *dev,
					   int slave, u8 *mac_addr)
{
	u32 mac_lo, mac_hi, offset;
	struct regmap *syscon;
	int ret;

	syscon = syscon_regmap_lookup_by_phandle(dev, "ti,syscon-efuse");
	if (IS_ERR(syscon)) {
		if (PTR_ERR(syscon) == -ENODEV)
			return 0;
		return PTR_ERR(syscon);
	}

	ret = dev_read_u32_index(dev, "ti,syscon-efuse", 1, &offset);
	if (ret)
		return ret;

	regmap_read(syscon, offset, &mac_lo);
	regmap_read(syscon, offset + 4, &mac_hi);

	mac_addr[0] = (mac_hi >> 8) & 0xff;
	mac_addr[1] = mac_hi & 0xff;
	mac_addr[2] = (mac_lo >> 24) & 0xff;
	mac_addr[3] = (mac_lo >> 16) & 0xff;
	mac_addr[4] = (mac_lo >> 8) & 0xff;
	mac_addr[5] = mac_lo & 0xff;

	return 0;
}

static int am65_cpsw_read_rom_hwaddr(struct udevice *dev)
{
	struct am65_cpsw_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);

	am65_cpsw_am654_get_efuse_macid(dev,
					priv->port_id,
					pdata->enetaddr);

	return 0;
}

static const struct eth_ops am65_cpsw_ops = {
	.start		= am65_cpsw_start,
	.send		= am65_cpsw_send,
	.recv		= am65_cpsw_recv,
	.free_pkt	= am65_cpsw_free_pkt,
	.stop		= am65_cpsw_stop,
	.read_rom_hwaddr = am65_cpsw_read_rom_hwaddr,
};

static const struct soc_attr k3_mdio_soc_data[] = {
	{ .family = "AM62X", .revision = "SR1.0" },
	{ .family = "AM64X", .revision = "SR1.0" },
	{ .family = "AM64X", .revision = "SR2.0" },
	{ .family = "AM65X", .revision = "SR1.0" },
	{ .family = "AM65X", .revision = "SR2.0" },
	{ .family = "J7200", .revision = "SR1.0" },
	{ .family = "J7200", .revision = "SR2.0" },
	{ .family = "J721E", .revision = "SR1.0" },
	{ .family = "J721E", .revision = "SR1.1" },
	{ .family = "J721S2", .revision = "SR1.0" },
	{ /* sentinel */ },
};

static ofnode am65_cpsw_find_mdio(ofnode parent)
{
	ofnode node;

	ofnode_for_each_subnode(node, parent)
		if (ofnode_device_is_compatible(node, "ti,cpsw-mdio"))
			return node;

	return ofnode_null();
}

static int am65_cpsw_mdio_setup(struct udevice *dev)
{
	struct am65_cpsw_priv *priv = dev_get_priv(dev);
	struct am65_cpsw_common	*cpsw_common = priv->cpsw_common;
	struct udevice *mdio_dev;
	ofnode mdio;
	int ret;

	mdio = am65_cpsw_find_mdio(dev_ofnode(cpsw_common->dev));
	if (!ofnode_valid(mdio))
		return 0;

	/*
	 * The MDIO controller is represented in the DT binding by a
	 * subnode of the MAC controller.
	 *
	 * We don't have a DM driver for the MDIO device yet, and thus any
	 * pinctrl setting on its node will be ignored.
	 *
	 * However, we do need to make sure the pins states tied to the
	 * MDIO node are configured properly. Fortunately, the core DM
	 * does that for use when we get a device, so we can work around
	 * that whole issue by just requesting a dummy MDIO driver to
	 * probe, and our pins will get muxed.
	 */
	ret = uclass_get_device_by_ofnode(UCLASS_MDIO, mdio, &mdio_dev);
	if (ret)
		return ret;

	return 0;
}

static int am65_cpsw_mdio_init(struct udevice *dev)
{
	struct am65_cpsw_priv *priv = dev_get_priv(dev);
	struct am65_cpsw_common	*cpsw_common = priv->cpsw_common;
	int ret;

	if (!priv->has_phy || cpsw_common->bus)
		return 0;

	if (IS_ENABLED(CONFIG_DM_GPIO)) {
		if (dm_gpio_is_valid(&cpsw_common->mdio_gpio_reset)) {
			dm_gpio_set_value(&cpsw_common->mdio_gpio_reset, 1);
			udelay(cpsw_common->reset_delay_us);
			dm_gpio_set_value(&cpsw_common->mdio_gpio_reset, 0);
			if (cpsw_common->reset_post_delay_us > 0)
				udelay(cpsw_common->reset_post_delay_us);
		}
	}

	ret = am65_cpsw_mdio_setup(dev);
	if (ret)
		return ret;

	cpsw_common->bus = cpsw_mdio_init(dev->name,
					  cpsw_common->mdio_base,
					  cpsw_common->bus_freq,
					  clk_get_rate(&cpsw_common->fclk),
					  priv->mdio_manual_mode);
	if (!cpsw_common->bus)
		return -EFAULT;

	return 0;
}

static int am65_cpsw_phy_init(struct udevice *dev)
{
	struct am65_cpsw_priv *priv = dev_get_priv(dev);
	struct am65_cpsw_common *cpsw_common = priv->cpsw_common;
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct phy_device *phydev;
	u32 supported = PHY_GBIT_FEATURES;
	int ret;

	phydev = phy_connect(cpsw_common->bus,
			     priv->phy_addr,
			     priv->dev,
			     pdata->phy_interface);

	if (!phydev) {
		dev_err(dev, "phy_connect() failed\n");
		return -ENODEV;
	}

	phydev->supported &= supported;
	if (pdata->max_speed) {
		ret = phy_set_supported(phydev, pdata->max_speed);
		if (ret)
			return ret;
	}
	phydev->advertising = phydev->supported;

	if (ofnode_valid(priv->phy_node))
		phydev->node = priv->phy_node;

	priv->phydev = phydev;
	ret = phy_config(phydev);
	if (ret < 0)
		pr_err("phy_config() failed: %d", ret);

	return ret;
}

static int am65_cpsw_ofdata_parse_phy(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct am65_cpsw_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args out_args;
	int ret = 0;

	dev_read_u32(dev, "reg", &priv->port_id);

	pdata->phy_interface = dev_read_phy_mode(dev);
	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA) {
		dev_err(dev, "Invalid PHY mode, port %u\n", priv->port_id);
		return -EINVAL;
	}

	dev_read_u32(dev, "max-speed", (u32 *)&pdata->max_speed);
	if (pdata->max_speed)
		dev_err(dev, "Port %u speed froced to %uMbit\n",
			priv->port_id, pdata->max_speed);

	priv->has_phy  = true;
	ret = ofnode_parse_phandle_with_args(dev_ofnode(dev), "phy-handle",
					     NULL, 0, 0, &out_args);
	if (ret) {
		dev_err(dev, "can't parse phy-handle port %u (%d)\n",
			priv->port_id, ret);
		priv->has_phy  = false;
		ret = 0;
	}

	priv->phy_node = out_args.node;
	if (priv->has_phy) {
		ret = ofnode_read_u32(priv->phy_node, "reg", &priv->phy_addr);
		if (ret) {
			dev_err(dev, "failed to get phy_addr port %u (%d)\n",
				priv->port_id, ret);
			goto out;
		}
	}

out:
	return ret;
}

static int am65_cpsw_port_probe(struct udevice *dev)
{
	struct am65_cpsw_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct am65_cpsw_common *cpsw_common;
	char portname[15];
	int ret;

	priv->dev = dev;

	cpsw_common = dev_get_priv(dev->parent);
	priv->cpsw_common = cpsw_common;

	sprintf(portname, "%s%s", dev->parent->name, dev->name);
	device_set_name(dev, portname);

	priv->mdio_manual_mode = false;
	if (soc_device_match(k3_mdio_soc_data))
		priv->mdio_manual_mode = true;

	ret = am65_cpsw_ofdata_parse_phy(dev);
	if (ret)
		goto out;

	ret = am65_cpsw_gmii_sel_k3(priv, pdata->phy_interface);
	if (ret)
		goto out;

	ret = am65_cpsw_mdio_init(dev);
	if (ret)
		goto out;

	ret = am65_cpsw_phy_init(dev);
	if (ret)
		goto out;
out:
	return ret;
}

static int am65_cpsw_probe_nuss(struct udevice *dev)
{
	struct am65_cpsw_common *cpsw_common = dev_get_priv(dev);
	ofnode ports_np, node, mdio_np;
	int ret, i;
	struct udevice *port_dev;

	cpsw_common->dev = dev;
	cpsw_common->ss_base = dev_read_addr(dev);
	if (cpsw_common->ss_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = power_domain_get_by_index(dev, &cpsw_common->pwrdmn, 0);
	if (ret) {
		dev_err(dev, "failed to get pwrdmn: %d\n", ret);
		return ret;
	}

	ret = clk_get_by_name(dev, "fck", &cpsw_common->fclk);
	if (ret) {
		power_domain_free(&cpsw_common->pwrdmn);
		dev_err(dev, "failed to get clock %d\n", ret);
		return ret;
	}

	cpsw_common->cpsw_base = cpsw_common->ss_base + AM65_CPSW_CPSW_NU_BASE;
	cpsw_common->ale_base = cpsw_common->cpsw_base +
				AM65_CPSW_CPSW_NU_ALE_BASE;
	cpsw_common->mdio_base = cpsw_common->ss_base + AM65_CPSW_MDIO_BASE;

	if (IS_ENABLED(CONFIG_DM_GPIO)) {
		/* get bus level PHY reset GPIO details */
		mdio_np = dev_read_subnode(dev, "mdio");
		if (!ofnode_valid(mdio_np)) {
			ret = -ENOENT;
			goto out;
		}

		cpsw_common->reset_delay_us = ofnode_read_u32_default(mdio_np, "reset-delay-us",
								      DEFAULT_GPIO_RESET_DELAY);
		cpsw_common->reset_post_delay_us = ofnode_read_u32_default(mdio_np,
									   "reset-post-delay-us",
									   0);
		ret = gpio_request_by_name_nodev(mdio_np, "reset-gpios", 0,
						 &cpsw_common->mdio_gpio_reset,
						 GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	}

	ports_np = dev_read_subnode(dev, "ethernet-ports");
	if (!ofnode_valid(ports_np)) {
		ret = -ENOENT;
		goto out;
	}

	ofnode_for_each_subnode(node, ports_np) {
		const char *node_name;
		u32 port_id;
		bool disabled;

		node_name = ofnode_get_name(node);

		disabled = !ofnode_is_enabled(node);

		ret = ofnode_read_u32(node, "reg", &port_id);
		if (ret) {
			dev_err(dev, "%s: failed to get port_id (%d)\n",
				node_name, ret);
			goto out;
		}

		if (port_id >= AM65_CPSW_CPSWNU_MAX_PORTS) {
			dev_err(dev, "%s: invalid port_id (%d)\n",
				node_name, port_id);
			ret = -EINVAL;
			goto out;
		}
		cpsw_common->port_num++;

		if (!port_id)
			continue;

		cpsw_common->ports[port_id].disabled = disabled;
		if (disabled)
			continue;

		ret = device_bind_driver_to_node(dev, "am65_cpsw_nuss_port", ofnode_get_name(node), node, &port_dev);
		if (ret)
			dev_err(dev, "Failed to bind to %s node\n", ofnode_get_name(node));
	}

	for (i = 0; i < AM65_CPSW_CPSWNU_MAX_PORTS; i++) {
		struct am65_cpsw_port *port = &cpsw_common->ports[i];

		port->port_base = cpsw_common->cpsw_base +
				  AM65_CPSW_CPSW_NU_PORTS_OFFSET +
				  (i * AM65_CPSW_CPSW_NU_PORTS_OFFSET);
		port->port_sgmii_base = cpsw_common->ss_base +
					(i * AM65_CPSW_SGMII_BASE);
		port->macsl_base = port->port_base +
				   AM65_CPSW_CPSW_NU_PORT_MACSL_OFFSET;
	}

	cpsw_common->bus_freq =
			dev_read_u32_default(dev, "bus_freq",
					     AM65_CPSW_MDIO_BUS_FREQ_DEF);

	dev_info(dev, "K3 CPSW: nuss_ver: 0x%08X cpsw_ver: 0x%08X ale_ver: 0x%08X Ports:%u mdio_freq:%u\n",
		 readl(cpsw_common->ss_base),
		 readl(cpsw_common->cpsw_base),
		 readl(cpsw_common->ale_base),
		 cpsw_common->port_num,
		 cpsw_common->bus_freq);

out:
	power_domain_free(&cpsw_common->pwrdmn);
	return ret;
}

static const struct udevice_id am65_cpsw_nuss_ids[] = {
	{ .compatible = "ti,am654-cpsw-nuss" },
	{ .compatible = "ti,j721e-cpsw-nuss" },
	{ .compatible = "ti,am642-cpsw-nuss" },
	{ }
};

U_BOOT_DRIVER(am65_cpsw_nuss) = {
	.name	= "am65_cpsw_nuss",
	.id	= UCLASS_MISC,
	.of_match = am65_cpsw_nuss_ids,
	.probe	= am65_cpsw_probe_nuss,
	.priv_auto = sizeof(struct am65_cpsw_common),
};

U_BOOT_DRIVER(am65_cpsw_nuss_port) = {
	.name	= "am65_cpsw_nuss_port",
	.id	= UCLASS_ETH,
	.probe	= am65_cpsw_port_probe,
	.ops	= &am65_cpsw_ops,
	.priv_auto	= sizeof(struct am65_cpsw_priv),
	.plat_auto	= sizeof(struct eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA | DM_FLAG_OS_PREPARE,
};

static const struct udevice_id am65_cpsw_mdio_ids[] = {
	{ .compatible = "ti,cpsw-mdio" },
	{ }
};

U_BOOT_DRIVER(am65_cpsw_mdio) = {
	.name		= "am65_cpsw_mdio",
	.id		= UCLASS_MDIO,
	.of_match	= am65_cpsw_mdio_ids,
};
