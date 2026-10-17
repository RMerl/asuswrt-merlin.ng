// SPDX-License-Identifier: GPL-2.0+

/*
 * Copyright (C) 2020 Cortina Access Inc.
 * Author: Aaron Tseng <aaron.tseng@cortina-access.com>
 *
 * Ethernet MAC Driver for all supported CAxxxx SoCs
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <net.h>
#include <miiphy.h>
#include <env.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <u-boot/crc.h>
#include <led.h>

#include "cortina_ni.h"

#define HEADER_A_SIZE	8

enum ca_led_state_t {
	CA_LED_OFF = 0,
	CA_LED_ON = 1,
};

enum ca_port_t {
	NI_PORT_0 = 0,
	NI_PORT_1,
	NI_PORT_2,
	NI_PORT_3,
	NI_PORT_4,
	NI_PORT_5,
	NI_PORT_MAX,
};

static struct udevice *curr_dev;

static u32 *ca_rdwrptr_adv_one(u32 *x, unsigned long base, unsigned long max)
{
	if (x + 1 >= (u32 *)max)
		return (u32 *)base;
	else
		return (x + 1);
}

static void ca_reg_read(void *reg, u64 base, u64 offset)
{
	u32 *val = (u32 *)reg;

	*val = readl(KSEG1_ATU_XLAT(base + offset));
}

static void ca_reg_write(void *reg, u64 base, u64 offset)
{
	u32 val = *(u32 *)reg;

	writel(val, KSEG1_ATU_XLAT(base + offset));
}

static int ca_mdio_write_rgmii(u32 addr, u32 offset, u16 data)
{
	/* up to 10000 cycles*/
	u32 loop_wait = __MDIO_ACCESS_TIMEOUT;
	struct PER_MDIO_ADDR_t mdio_addr;
	struct PER_MDIO_CTRL_t mdio_ctrl;
	struct cortina_ni_priv *priv = dev_get_priv(curr_dev);

	memset(&mdio_addr, 0, sizeof(mdio_addr));
	mdio_addr.mdio_addr = addr;
	mdio_addr.mdio_offset = offset;
	mdio_addr.mdio_rd_wr = __MDIO_WR_FLAG;
	ca_reg_write(&mdio_addr, (u64)priv->per_mdio_base_addr,
		     PER_MDIO_ADDR_OFFSET);
	ca_reg_write(&data, (u64)priv->per_mdio_base_addr,
		     PER_MDIO_WRDATA_OFFSET);

	memset(&mdio_ctrl, 0, sizeof(mdio_ctrl));
	mdio_ctrl.mdiostart = 1;
	ca_reg_write(&mdio_ctrl, (u64)priv->per_mdio_base_addr,
		     PER_MDIO_CTRL_OFFSET);

	debug("%s: phy_addr=%d, offset=%d, data=0x%x\n",
	      __func__, addr, offset, data);

	do {
		ca_reg_read(&mdio_ctrl, (u64)priv->per_mdio_base_addr,
			    PER_MDIO_CTRL_OFFSET);
		if (mdio_ctrl.mdiodone) {
			ca_reg_write(&mdio_ctrl, (u64)priv->per_mdio_base_addr,
				     PER_MDIO_CTRL_OFFSET);
			return 0;
		}
	} while (--loop_wait);

	printf("CA NI %s: PHY write timeout!!!\n", __func__);
	return -ETIMEDOUT;
}

int ca_mdio_write(u32 addr, u32 offset, u16 data)
{
	u32 reg_addr, reg_val;
	struct NI_MDIO_OPER_T mdio_oper;

	/* support range: 1~31*/
	if (addr < CA_MDIO_ADDR_MIN || addr > CA_MDIO_ADDR_MAX)
		return -EINVAL;

	/* the phy addr 5 is connect to RGMII */
	if (addr >= 5)
		return ca_mdio_write_rgmii(addr, offset, data);

	memset(&mdio_oper, 0, sizeof(mdio_oper));
	mdio_oper.reg_off = offset;
	mdio_oper.phy_addr = addr;
	mdio_oper.reg_base = CA_NI_MDIO_REG_BASE;
	reg_val = data;
	memcpy(&reg_addr, &mdio_oper, sizeof(reg_addr));
	ca_reg_write(&reg_val, (u64)reg_addr, 0);

	return 0;
}

static int ca_mdio_read_rgmii(u32 addr, u32 offset, u16 *data)
{
	u32 loop_wait = __MDIO_ACCESS_TIMEOUT;
	struct PER_MDIO_ADDR_t mdio_addr;
	struct PER_MDIO_CTRL_t mdio_ctrl;
	struct PER_MDIO_RDDATA_t read_data;
	struct cortina_ni_priv *priv = dev_get_priv(curr_dev);

	memset(&mdio_addr, 0, sizeof(mdio_addr));
	mdio_addr.mdio_addr = addr;
	mdio_addr.mdio_offset = offset;
	mdio_addr.mdio_rd_wr = __MDIO_RD_FLAG;
	ca_reg_write(&mdio_addr, (u64)priv->per_mdio_base_addr,
		     PER_MDIO_ADDR_OFFSET);

	memset(&mdio_ctrl, 0, sizeof(mdio_ctrl));
	mdio_ctrl.mdiostart = 1;
	ca_reg_write(&mdio_ctrl, (u64)priv->per_mdio_base_addr,
		     PER_MDIO_CTRL_OFFSET);

	do {
		ca_reg_read(&mdio_ctrl, (u64)priv->per_mdio_base_addr,
			    PER_MDIO_CTRL_OFFSET);
		if (mdio_ctrl.mdiodone) {
			ca_reg_write(&mdio_ctrl, (u64)priv->per_mdio_base_addr,
				     PER_MDIO_CTRL_OFFSET);
			ca_reg_read(&read_data, (u64)priv->per_mdio_base_addr,
				    PER_MDIO_RDDATA_OFFSET);
			*data = read_data.mdio_rddata;
			return 0;
		}
	} while (--loop_wait);

	printf("CA NI %s: TIMEOUT!!\n", __func__);
	return -ETIMEDOUT;
}

int ca_mdio_read(u32 addr, u32 offset, u16 *data)
{
	u32 reg_addr, reg_val;
	struct NI_MDIO_OPER_T mdio_oper;

	if (!data)
		return -EINVAL;

	/* support range: 1~31*/
	if (addr < CA_MDIO_ADDR_MIN || addr > CA_MDIO_ADDR_MAX)
		return -EINVAL;

	/* the phy addr 5 is connect to RGMII */
	if (addr >= 5)
		return ca_mdio_read_rgmii(addr, offset, data);

	memset(&mdio_oper, 0, sizeof(mdio_oper));
	mdio_oper.reg_off = offset;
	mdio_oper.phy_addr = addr;
	mdio_oper.reg_base = CA_NI_MDIO_REG_BASE;
	reg_val = *data;
	memcpy(&reg_addr, &mdio_oper, sizeof(reg_addr));
	ca_reg_read(&reg_val, (u64)reg_addr, 0);
	*data = reg_val;
	return 0;
}

int ca_miiphy_read(const char *devname, u8 addr, u8 reg, u16 *value)
{
	return ca_mdio_read(addr, reg, value);
}

int ca_miiphy_write(const char *devname, u8 addr, u8 reg, u16 value)
{
	return ca_mdio_write(addr, reg, value);
}

static int cortina_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	u16 data;

	ca_mdio_read(addr, reg, &data);
	return data;
}

static int cortina_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			      u16 val)
{
	return ca_mdio_write(addr, reg, val);
}

static void ca_ni_setup_mac_addr(void)
{
	u8 mac[6];
	struct NI_HV_GLB_MAC_ADDR_CFG0_t mac_addr_cfg0;
	struct NI_HV_GLB_MAC_ADDR_CFG1_t mac_addr_cfg1;
	struct NI_HV_PT_PORT_STATIC_CFG_t port_static_cfg;
	struct NI_HV_XRAM_CPUXRAM_CFG_t cpuxram_cfg;
	struct cortina_ni_priv *priv = dev_get_priv(curr_dev);

	/* parsing ethaddr and set to NI registers. */
	if (eth_env_get_enetaddr("ethaddr", mac)) {
		/* The complete MAC address consists of
		 * {MAC_ADDR0_mac_addr0[0-3], MAC_ADDR1_mac_addr1[4],
		 * PT_PORT_STATIC_CFG_mac_addr6[5]}.
		 */
		mac_addr_cfg0.mac_addr0 = (mac[0] << 24) + (mac[1] << 16) +
					  (mac[2] << 8) + mac[3];
		ca_reg_write(&mac_addr_cfg0, (u64)priv->ni_hv_base_addr,
			     NI_HV_GLB_MAC_ADDR_CFG0_OFFSET);

		memset(&mac_addr_cfg1, 0, sizeof(mac_addr_cfg1));
		mac_addr_cfg1.mac_addr1 = mac[4];
		ca_reg_write(&mac_addr_cfg1, (u64)priv->ni_hv_base_addr,
			     NI_HV_GLB_MAC_ADDR_CFG1_OFFSET);

		ca_reg_read(&port_static_cfg, (u64)priv->ni_hv_base_addr,
			    NI_HV_PT_PORT_STATIC_CFG_OFFSET +
			    (APB0_NI_HV_PT_STRIDE * priv->active_port));

		port_static_cfg.mac_addr6 = mac[5];
		ca_reg_write(&port_static_cfg, (u64)priv->ni_hv_base_addr,
			     NI_HV_PT_PORT_STATIC_CFG_OFFSET +
			     (APB0_NI_HV_PT_STRIDE * priv->active_port));

		/* received only Broadcast and Address matched packets */
		ca_reg_read(&cpuxram_cfg, (u64)priv->ni_hv_base_addr,
			    NI_HV_XRAM_CPUXRAM_CFG_OFFSET);
		cpuxram_cfg.xram_mgmt_promisc_mode = 0;
		cpuxram_cfg.rx_0_cpu_pkt_dis = 0;
		cpuxram_cfg.tx_0_cpu_pkt_dis = 0;
		ca_reg_write(&cpuxram_cfg, (u64)priv->ni_hv_base_addr,
			     NI_HV_XRAM_CPUXRAM_CFG_OFFSET);
	} else {
		/* received all packets(promiscuous mode) */
		ca_reg_read(&cpuxram_cfg, (u64)priv->ni_hv_base_addr,
			    NI_HV_XRAM_CPUXRAM_CFG_OFFSET);
		cpuxram_cfg.xram_mgmt_promisc_mode = 3;
		cpuxram_cfg.rx_0_cpu_pkt_dis = 0;
		cpuxram_cfg.tx_0_cpu_pkt_dis = 0;
		ca_reg_write(&cpuxram_cfg, (u64)priv->ni_hv_base_addr,
			     NI_HV_XRAM_CPUXRAM_CFG_OFFSET);
	}
}

static void ca_ni_enable_tx_rx(void)
{
	struct NI_HV_PT_RXMAC_CFG_t rxmac_cfg;
	struct NI_HV_PT_TXMAC_CFG_t txmac_cfg;
	struct cortina_ni_priv *priv = dev_get_priv(curr_dev);

	/* Enable TX and RX functions */
	ca_reg_read(&rxmac_cfg, (u64)priv->ni_hv_base_addr,
		    NI_HV_PT_RXMAC_CFG_OFFSET +
		    (APB0_NI_HV_PT_STRIDE * priv->active_port));
	rxmac_cfg.rx_en = 1;
	ca_reg_write(&rxmac_cfg, (u64)priv->ni_hv_base_addr,
		     NI_HV_PT_RXMAC_CFG_OFFSET +
		     (APB0_NI_HV_PT_STRIDE * priv->active_port));

	ca_reg_read(&txmac_cfg, (u64)priv->ni_hv_base_addr,
		    NI_HV_PT_TXMAC_CFG_OFFSET +
		    (APB0_NI_HV_PT_STRIDE * priv->active_port));
	txmac_cfg.tx_en = 1;
	ca_reg_write(&txmac_cfg, (u64)priv->ni_hv_base_addr,
		     NI_HV_PT_TXMAC_CFG_OFFSET +
		     (APB0_NI_HV_PT_STRIDE * priv->active_port));
}

#define AUTO_SCAN_TIMEOUT 3000 /* 3 seconds */
static int ca_ni_auto_scan_active_port(struct cortina_ni_priv *priv)
{
	u8 i;
	u16 data;
	u32 start_time;

	start_time = get_timer(0);
	while (get_timer(start_time) < AUTO_SCAN_TIMEOUT) {
		for (i = 0; i < priv->valid_port_num; i++) {
			if (!priv->port_map[i].phy_addr)
				continue;

			ca_mdio_read(priv->port_map[i].phy_addr, 1, &data);
			if (data & 0x04) {
				priv->active_port = priv->port_map[i].port;
				return 0;
			}
		}
	}

	printf("CA NI %s: auto scan active_port timeout.\n", __func__);
	return -1;
}

static void ca_ni_led(int port, int status)
{
	char label[10];
	struct udevice *led_dev;

	if (IS_ENABLED(CONFIG_LED_CORTINA)) {
		snprintf(label, sizeof(label), "led%d", port);
		debug("%s: set port %d led %s.\n",
		      __func__, port, status ? "on" : "off");
		led_get_by_label(label, &led_dev);
		led_set_state(led_dev, status);
	}
}

static void ca_ni_reset(void)
{
	int i;
	struct NI_HV_GLB_INIT_DONE_t init_done;
	struct NI_HV_GLB_INTF_RST_CONFIG_t intf_rst_config;
	struct NI_HV_GLB_STATIC_CFG_t static_cfg;
	struct GLOBAL_BLOCK_RESET_t glb_blk_reset;
	struct cortina_ni_priv *priv = dev_get_priv(curr_dev);

	/* NI global resets */
	ca_reg_read(&glb_blk_reset, (u64)priv->glb_base_addr,
		    GLOBAL_BLOCK_RESET_OFFSET);
	glb_blk_reset.reset_ni = 1;
	ca_reg_write(&glb_blk_reset, (u64)priv->glb_base_addr,
		     GLOBAL_BLOCK_RESET_OFFSET);
	/* Remove resets */
	glb_blk_reset.reset_ni = 0;
	ca_reg_write(&glb_blk_reset, (u64)priv->glb_base_addr,
		     GLOBAL_BLOCK_RESET_OFFSET);

	/* check the ready bit of NI module */
	for (i = 0; i < NI_READ_POLL_COUNT; i++) {
		ca_reg_read(&init_done, (u64)priv->ni_hv_base_addr,
			    NI_HV_GLB_INIT_DONE_OFFSET);
		if (init_done.ni_init_done)
			break;
	}
	if (i == NI_READ_POLL_COUNT) {
		printf("CA NI %s: NI init done not ready, init_done=0x%x!!!\n",
		       __func__, init_done.ni_init_done);
	}

	ca_reg_read(&intf_rst_config, (u64)priv->ni_hv_base_addr,
		    NI_HV_GLB_INTF_RST_CONFIG_OFFSET);
	switch (priv->active_port) {
	case NI_PORT_0:
		intf_rst_config.intf_rst_p0 = 0;
		intf_rst_config.mac_rx_rst_p0 = 0;
		intf_rst_config.mac_tx_rst_p0 = 0;
		break;
	case NI_PORT_1:
		intf_rst_config.intf_rst_p1 = 0;
		intf_rst_config.mac_rx_rst_p1 = 0;
		intf_rst_config.mac_tx_rst_p1 = 0;
		break;
	case NI_PORT_2:
		intf_rst_config.intf_rst_p2 = 0;
		intf_rst_config.mac_rx_rst_p2 = 0;
		intf_rst_config.mac_tx_rst_p2 = 0;
		break;
	case NI_PORT_3:
		intf_rst_config.intf_rst_p3 = 0;
		intf_rst_config.mac_tx_rst_p3 = 0;
		intf_rst_config.mac_rx_rst_p3 = 0;
		break;
	case NI_PORT_4:
		intf_rst_config.intf_rst_p4 = 0;
		intf_rst_config.mac_tx_rst_p4 = 0;
		intf_rst_config.mac_rx_rst_p4 = 0;
		break;
	}

	ca_reg_write(&intf_rst_config, (u64)priv->ni_hv_base_addr,
		     NI_HV_GLB_INTF_RST_CONFIG_OFFSET);

	/* Only one GMAC can connect to CPU */
	ca_reg_read(&static_cfg, (u64)priv->ni_hv_base_addr,
		    NI_HV_GLB_STATIC_CFG_OFFSET);
	static_cfg.port_to_cpu = priv->active_port;
	static_cfg.txmib_mode = 1;
	static_cfg.rxmib_mode = 1;

	ca_reg_write(&static_cfg, (u64)priv->ni_hv_base_addr,
		     NI_HV_GLB_STATIC_CFG_OFFSET);
}

static void ca_internal_gphy_cal(struct cortina_ni_priv *priv)
{
	int i, port, num;
	u32 reg_off, value;

	num = priv->gphy_num;
	for (port = 0; port < 4; port++) {
		for (i = 0; i < num; i++) {
			reg_off = priv->gphy_values[i].reg_off + (port * 0x80);
			value = priv->gphy_values[i].value;
			ca_reg_write(&value, reg_off, 0);
			mdelay(50);
		}
	}
}

static int ca_mdio_register(struct udevice *dev)
{
	int ret;
	struct cortina_ni_priv *priv = dev_get_priv(dev);
	struct mii_dev *mdio_bus = mdio_alloc();

	if (!mdio_bus)
		return -ENOMEM;

	mdio_bus->read = cortina_mdio_read;
	mdio_bus->write = cortina_mdio_write;
	snprintf(mdio_bus->name, sizeof(mdio_bus->name), dev->name);

	mdio_bus->priv = (void *)priv;

	ret = mdio_register(mdio_bus);
	if (ret)
		return ret;

	priv->mdio_bus = mdio_bus;
	return 0;
}

static void ca_rgmii_init(struct cortina_ni_priv *priv)
{
	struct GLOBAL_GLOBAL_CONFIG_t	glb_config;
	struct GLOBAL_IO_DRIVE_CONTROL_t io_drive_control;

	/* Generating 25Mhz reference clock for switch */
	ca_reg_read(&glb_config, (u64)priv->glb_base_addr,
		    GLOBAL_GLOBAL_CONFIG_OFFSET);
	glb_config.refclk_sel = 0x01;
	glb_config.ext_reset = 0x01;
	ca_reg_write(&glb_config, (u64)priv->glb_base_addr,
		     GLOBAL_GLOBAL_CONFIG_OFFSET);

	mdelay(20);

	/* Do external reset */
	ca_reg_read(&glb_config, (u64)priv->glb_base_addr,
		    GLOBAL_GLOBAL_CONFIG_OFFSET);
	glb_config.ext_reset = 0x0;
	ca_reg_write(&glb_config, (u64)priv->glb_base_addr,
		     GLOBAL_GLOBAL_CONFIG_OFFSET);

	ca_reg_read(&io_drive_control, (u64)priv->glb_base_addr,
		    GLOBAL_IO_DRIVE_CONTROL_OFFSET);
	io_drive_control.gmac_mode = 2;
	io_drive_control.gmac_dn = 1;
	io_drive_control.gmac_dp = 1;
	ca_reg_write(&io_drive_control, (u64)priv->glb_base_addr,
		     GLOBAL_IO_DRIVE_CONTROL_OFFSET);
}

static int ca_phy_probe(struct udevice *dev)
{
	int auto_scan_active_port = 0, tmp_port;
	char *buf;
	struct cortina_ni_priv *priv = dev_get_priv(dev);
	struct phy_device *int_phydev, *ext_phydev;

	/* Initialize internal phy device */
	int_phydev = phy_connect(priv->mdio_bus,
				 priv->port_map[NI_PORT_3].phy_addr,
				 dev, priv->phy_interface);
	if (int_phydev) {
		int_phydev->supported &= PHY_GBIT_FEATURES;
		int_phydev->advertising = int_phydev->supported;
		phy_config(int_phydev);
	} else {
		printf("CA NI %s: There is no internal phy device\n", __func__);
	}

	/* Initialize external phy device */
	ext_phydev = phy_connect(priv->mdio_bus,
				 priv->port_map[NI_PORT_4].phy_addr,
				 dev, priv->phy_interface);
	if (ext_phydev) {
		ext_phydev->supported &= PHY_GBIT_FEATURES;
		ext_phydev->advertising = int_phydev->supported;
		phy_config(ext_phydev);
	} else {
		printf("CA NI %s: There is no external phy device\n", __func__);
	}

	/* auto scan the first link up port as active_port */
	buf = env_get("auto_scan_active_port");
	if (buf != 0) {
		auto_scan_active_port = simple_strtoul(buf, NULL, 0);
		printf("CA NI %s: auto_scan_active_port=%d\n", __func__,
		       auto_scan_active_port);
	}

	if (auto_scan_active_port) {
		ca_ni_auto_scan_active_port(priv);
	} else {
		buf = env_get("active_port");
		if (buf != 0) {
			tmp_port = simple_strtoul(buf, NULL, 0);
			if (tmp_port < 0 &&
			    !(priv->valid_port_map && BIT(tmp_port))) {
				printf("CA NI ERROR: not support this port.");
				free(dev);
				free(priv);
				return 1;
			}

			priv->active_port = tmp_port;
		}
	}

	printf("CA NI %s: active_port=%d\n", __func__, priv->active_port);
	if (priv->active_port == NI_PORT_4)
		priv->phydev = ext_phydev;
	else
		priv->phydev = int_phydev;

	return 0;
}

static int cortina_eth_start(struct udevice *dev)
{
	int ret;
	struct NI_HV_XRAM_CPUXRAM_ADRCFG_RX_t cpuxram_adrcfg_rx;
	struct NI_HV_XRAM_CPUXRAM_ADRCFG_TX_0_t cpuxram_adrcfg_tx;
	struct NI_HV_XRAM_CPUXRAM_CFG_t	cpuxram_cfg;
	struct NI_HV_PT_PORT_STATIC_CFG_t port_static_cfg;
	struct NI_HV_PT_PORT_GLB_CFG_t port_glb_cfg;
	struct cortina_ni_priv *priv = dev_get_priv(dev);
	struct phy_device *phydev = priv->phydev;

	ret = phy_startup(priv->phydev);
	if (ret) {
		ca_ni_led(priv->active_port, CA_LED_OFF);
		printf("CA NI Could not initialize PHY %s, active_port=%d\n",
		       priv->phydev->dev->name, priv->active_port);
		return ret;
	}

	if (!priv->phydev->link) {
		printf("CA NI %s: link down.\n", priv->phydev->dev->name);
		return 0;
	}

	ca_ni_led(priv->active_port, CA_LED_ON);
	printf("CA NI PHY ID 0x%08X %dMbps %s duplex\n",
	       phydev->phy_id, phydev->speed,
	       phydev->duplex == DUPLEX_HALF ? "half" : "full");

	/* RX XRAM ADDRESS CONFIG (start and end address) */
	memset(&cpuxram_adrcfg_rx, 0, sizeof(cpuxram_adrcfg_rx));
	cpuxram_adrcfg_rx.rx_top_addr = RX_TOP_ADDR;
	cpuxram_adrcfg_rx.rx_base_addr = RX_BASE_ADDR;
	ca_reg_write(&cpuxram_adrcfg_rx, (u64)priv->ni_hv_base_addr,
		     NI_HV_XRAM_CPUXRAM_ADRCFG_RX_OFFSET);

	/* TX XRAM ADDRESS CONFIG (start and end address) */
	memset(&cpuxram_adrcfg_tx, 0, sizeof(cpuxram_adrcfg_tx));
	cpuxram_adrcfg_tx.tx_top_addr = TX_TOP_ADDR;
	cpuxram_adrcfg_tx.tx_base_addr = TX_BASE_ADDR;
	ca_reg_write(&cpuxram_adrcfg_tx, (u64)priv->ni_hv_base_addr,
		     NI_HV_XRAM_CPUXRAM_ADRCFG_TX_0_OFFSET);

	/*
	 * Configuration for Management Ethernet Interface:
	 * - RGMII 1000 mode or RGMII 100 mode
	 * - MAC mode
	 */
	ca_reg_read(&port_static_cfg, (u64)priv->ni_hv_base_addr,
		    NI_HV_PT_PORT_STATIC_CFG_OFFSET +
		    (APB0_NI_HV_PT_STRIDE * priv->active_port));
	if (phydev->speed == SPEED_1000) {
		/* port 4 connects to RGMII PHY */
		if (phydev->addr == 5)
			port_static_cfg.int_cfg = GE_MAC_INTF_RGMII_1000;
		else
			port_static_cfg.int_cfg = GE_MAC_INTF_GMII;
	} else {
		/* port 4 connects to RGMII PHY */
		if (phydev->addr == 5)
			port_static_cfg.int_cfg = GE_MAC_INTF_RGMII_100;
		else
			port_static_cfg.int_cfg = GE_MAC_INTF_MII;
	}

	ca_reg_write(&port_static_cfg, (u64)priv->ni_hv_base_addr,
		     NI_HV_PT_PORT_STATIC_CFG_OFFSET +
		     (APB0_NI_HV_PT_STRIDE * priv->active_port));

	ca_reg_read(&port_glb_cfg, (u64)priv->ni_hv_base_addr,
		    NI_HV_PT_PORT_GLB_CFG_OFFSET +
		    (APB0_NI_HV_PT_STRIDE * priv->active_port));
	port_glb_cfg.speed = phydev->speed == SPEED_10 ? 1 : 0;
	port_glb_cfg.duplex = phydev->duplex == DUPLEX_HALF ? 1 : 0;
	ca_reg_write(&port_glb_cfg, (u64)priv->ni_hv_base_addr,
		     NI_HV_PT_PORT_GLB_CFG_OFFSET +
		     (APB0_NI_HV_PT_STRIDE * priv->active_port));

	/* Need to toggle the tx and rx cpu_pkt_dis bit */
	/* after changing Address config register.      */
	ca_reg_read(&cpuxram_cfg, (u64)priv->ni_hv_base_addr,
		    NI_HV_XRAM_CPUXRAM_CFG_OFFSET);
	cpuxram_cfg.rx_0_cpu_pkt_dis = 1;
	cpuxram_cfg.tx_0_cpu_pkt_dis = 1;
	ca_reg_write(&cpuxram_cfg, (u64)priv->ni_hv_base_addr,
		     NI_HV_XRAM_CPUXRAM_CFG_OFFSET);

	ca_reg_read(&cpuxram_cfg, (u64)priv->ni_hv_base_addr,
		    NI_HV_XRAM_CPUXRAM_CFG_OFFSET);
	cpuxram_cfg.rx_0_cpu_pkt_dis = 0;
	cpuxram_cfg.tx_0_cpu_pkt_dis = 0;
	ca_reg_write(&cpuxram_cfg, (u64)priv->ni_hv_base_addr,
		     NI_HV_XRAM_CPUXRAM_CFG_OFFSET);

	ca_ni_enable_tx_rx();

	return 0;
}

/*********************************************
 * Packet receive routine from Management FE
 * Expects a previously allocated buffer and
 * fills the length
 * Retruns 0 on success -1 on failure
 *******************************************/
static int cortina_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	u8 *ptr;
	u32 next_link, pktlen = 0;
	u32 sw_rx_rd_ptr, hw_rx_wr_ptr, *rx_xram_ptr, *data_ptr;
	int loop, index = 0, blk_num;
	struct cortina_ni_priv *priv = dev_get_priv(dev);
	struct NI_HEADER_X_T header_x;
	struct NI_PACKET_STATUS packet_status;
	struct NI_HV_XRAM_CPUXRAM_CPU_STA_RX_0_t cpuxram_cpu_sta_rx;
	struct NI_HV_XRAM_CPUXRAM_CPU_CFG_RX_0_t cpuxram_cpu_cfg_rx;

	/* get the hw write pointer */
	memset(&cpuxram_cpu_sta_rx, 0, sizeof(cpuxram_cpu_sta_rx));
	ca_reg_read(&cpuxram_cpu_sta_rx, (u64)priv->ni_hv_base_addr,
		    NI_HV_XRAM_CPUXRAM_CPU_STA_RX_0_OFFSET);
	hw_rx_wr_ptr = cpuxram_cpu_sta_rx.pkt_wr_ptr;

	/* get the sw read pointer */
	memset(&cpuxram_cpu_cfg_rx, 0, sizeof(cpuxram_cpu_cfg_rx));
	ca_reg_read(&cpuxram_cpu_cfg_rx, (u64)priv->ni_hv_base_addr,
		    NI_HV_XRAM_CPUXRAM_CPU_CFG_RX_0_OFFSET);
	sw_rx_rd_ptr = cpuxram_cpu_cfg_rx.pkt_rd_ptr;

	debug("%s: NI_HV_XRAM_CPUXRAM_CPU_STA_RX_0 = 0x%p, ", __func__,
	      priv->ni_hv_base_addr + NI_HV_XRAM_CPUXRAM_CPU_STA_RX_0_OFFSET);
	debug("NI_HV_XRAM_CPUXRAM_CPU_CFG_RX_0 = 0x%p\n",
	      priv->ni_hv_base_addr + NI_HV_XRAM_CPUXRAM_CPU_CFG_RX_0_OFFSET);
	debug("%s : RX hw_wr_ptr = %d, sw_rd_ptr = %d\n",
	      __func__, hw_rx_wr_ptr, sw_rx_rd_ptr);

	while (sw_rx_rd_ptr != hw_rx_wr_ptr) {
		/* Point to the absolute memory address of XRAM
		 * where read pointer is
		 */
		rx_xram_ptr = (u32 *)
			      ((unsigned long)priv->ni_xram_base
			       + sw_rx_rd_ptr * 8);

		/* Wrap around if required */
		if (rx_xram_ptr >= (u32 *)(unsigned long)priv->rx_xram_end_adr)
			rx_xram_ptr = (u32 *)
				      (unsigned long)priv->rx_xram_base_adr;

		/* Checking header XR. Do not update the read pointer yet */
		/* skip unused 32-bit in Header XR */
		rx_xram_ptr = ca_rdwrptr_adv_one(rx_xram_ptr,
						 priv->rx_xram_base_adr,
						 priv->rx_xram_end_adr);

		memcpy(&header_x, rx_xram_ptr, sizeof(header_x));
		next_link = header_x.next_link;
		/* Header XR [31:0] */

		if (*rx_xram_ptr == 0xffffffff)
			printf("CA NI %s: XRAM Error !\n", __func__);

		debug("%s : RX next link 0x%x\n", __func__, next_link);
		debug("%s : bytes_valid %x\n", __func__, header_x.bytes_valid);

		if (header_x.ownership == 0) {
			/* point to Packet status [31:0] */
			rx_xram_ptr = ca_rdwrptr_adv_one(rx_xram_ptr,
							 priv->rx_xram_base_adr,
							 priv->rx_xram_end_adr);

			memcpy(&packet_status, rx_xram_ptr,
			       sizeof(*rx_xram_ptr));
			if (packet_status.valid == 0) {
				debug("%s: Invalid Packet !!, ", __func__);
				debug("next_link=%d\n", next_link);

				/* Update the software read pointer */
				ca_reg_write(&next_link,
					     (u64)priv->ni_hv_base_addr,
					NI_HV_XRAM_CPUXRAM_CPU_CFG_RX_0_OFFSET);
				return 0;
			}

			if (packet_status.drop ||
			    packet_status.runt ||
			    packet_status.oversize ||
			    packet_status.jabber ||
			    packet_status.crc_error ||
			    packet_status.jumbo) {
				debug("%s: Error Packet!!, ", __func__);
				debug("next_link=%d\n", next_link);

				/* Update the software read pointer */
				ca_reg_write(&next_link,
					     (u64)priv->ni_hv_base_addr,
					NI_HV_XRAM_CPUXRAM_CPU_CFG_RX_0_OFFSET);
				return 0;
			}

			/* check whether packet size is larger than 1514 */
			if (packet_status.packet_size > 1518) {
				debug("%s: Error Packet !! Packet size=%d, ",
				      __func__, packet_status.packet_size);
				debug("larger than 1518, next_link=%d\n",
				      next_link);

				/* Update the software read pointer */
				ca_reg_write(&next_link,
					     (u64)priv->ni_hv_base_addr,
					NI_HV_XRAM_CPUXRAM_CPU_CFG_RX_0_OFFSET);
				return 0;
			}

			rx_xram_ptr = ca_rdwrptr_adv_one(rx_xram_ptr,
							 priv->rx_xram_base_adr,
							 priv->rx_xram_end_adr);

			pktlen = packet_status.packet_size;

			debug("%s : rx packet length = %d\n",
			      __func__, packet_status.packet_size);

			rx_xram_ptr = ca_rdwrptr_adv_one(rx_xram_ptr,
							 priv->rx_xram_base_adr,
							 priv->rx_xram_end_adr);

			data_ptr = (u32 *)net_rx_packets[index];

			/* Read out the packet */
			/* Data is in little endian form in the XRAM */

			/* Send the packet to upper layer */

			debug("%s: packet data[]=", __func__);

			for (loop = 0; loop <= pktlen / 4; loop++) {
				ptr = (u8 *)rx_xram_ptr;
				if (loop < 10)
					debug("[0x%x]-[0x%x]-[0x%x]-[0x%x]",
					      ptr[0], ptr[1], ptr[2], ptr[3]);
				*data_ptr++ = *rx_xram_ptr++;
				/* Wrap around if required */
				if (rx_xram_ptr >= (u32 *)
				    (unsigned long)priv->rx_xram_end_adr) {
					rx_xram_ptr = (u32 *)(unsigned long)
						       (priv->rx_xram_base_adr);
				}
			}

			debug("\n");
			net_process_received_packet(net_rx_packets[index],
						    pktlen);
			if (++index >= PKTBUFSRX)
				index = 0;
			blk_num = net_rx_packets[index][0x2c] * 255 +
				net_rx_packets[index][0x2d];
			debug("%s: tftp block number=%d\n", __func__, blk_num);

			/* Update the software read pointer */
			ca_reg_write(&next_link,
				     (u64)priv->ni_hv_base_addr,
				     NI_HV_XRAM_CPUXRAM_CPU_CFG_RX_0_OFFSET);
		}

		/* get the hw write pointer */
		ca_reg_read(&cpuxram_cpu_sta_rx, (u64)priv->ni_hv_base_addr,
			    NI_HV_XRAM_CPUXRAM_CPU_STA_RX_0_OFFSET);
		hw_rx_wr_ptr = cpuxram_cpu_sta_rx.pkt_wr_ptr;

		/* get the sw read pointer */
		ca_reg_read(&sw_rx_rd_ptr, (u64)priv->ni_hv_base_addr,
			    NI_HV_XRAM_CPUXRAM_CPU_CFG_RX_0_OFFSET);
	}
	return 0;
}

static int cortina_eth_send(struct udevice *dev, void *packet, int length)
{
	u32 hw_tx_rd_ptr = 0, sw_tx_wr_ptr = 0;
	u32 loop, new_pkt_len, ca_crc32;
	u32 *tx_xram_ptr, *data_ptr;
	u16 next_link = 0;
	u8 *ptr, *pkt_buf_ptr, valid_bytes = 0;
	int pad = 0;
	static u8 pkt_buf[2048];
	struct NI_HEADER_X_T hdr_xt;
	struct NI_HV_XRAM_CPUXRAM_CPU_CFG_TX_0_t cpuxram_cpu_cfg_tx;
	struct cortina_ni_priv *priv = dev_get_priv(dev);

	if (!packet || length > 2032)
		return -1;

	/* Get the hardware read pointer */
	ca_reg_read(&hw_tx_rd_ptr, (u64)priv->ni_hv_base_addr,
		    NI_HV_XRAM_CPUXRAM_CPU_STAT_TX_0_OFFSET);

	/* Get the software write pointer */
	ca_reg_read(&sw_tx_wr_ptr, (u64)priv->ni_hv_base_addr,
		    NI_HV_XRAM_CPUXRAM_CPU_CFG_TX_0_OFFSET);

	debug("%s: NI_HV_XRAM_CPUXRAM_CPU_STAT_TX_0=0x%p, ",
	      __func__,
	      KSEG1_ATU_XLAT(priv->ni_hv_base_addr +
			     NI_HV_XRAM_CPUXRAM_CPU_STAT_TX_0_OFFSET));
	debug("NI_HV_XRAM_CPUXRAM_CPU_CFG_TX_0=0x%p\n",
	      KSEG1_ATU_XLAT(priv->ni_hv_base_addr +
			     NI_HV_XRAM_CPUXRAM_CPU_CFG_TX_0_OFFSET));
	debug("%s : hw_tx_rd_ptr = %d\n", __func__, hw_tx_rd_ptr);
	debug("%s : sw_tx_wr_ptr = %d\n", __func__, sw_tx_wr_ptr);

	if (hw_tx_rd_ptr != sw_tx_wr_ptr) {
		printf("CA NI %s: Tx FIFO is not available!\n", __func__);
		return 1;
	}

	/* a workaround on 2015/10/01
	 * the packet size+CRC should be 8-byte alignment
	 */
	if (((length + 4) % 8) != 0)
		length += (8 - ((length + 4) % 8));

	memset(pkt_buf, 0x00, sizeof(pkt_buf));

	/* add 8-byte header_A at the beginning of packet */
	memcpy(&pkt_buf[HEADER_A_SIZE], (const void *)packet, length);

	pad = 64 - (length + 4);	/* if packet length < 60 */
	pad = (pad < 0) ? 0 : pad;

	debug("%s: length=%d, pad=%d\n", __func__, length, pad);

	new_pkt_len = length + pad;	/* new packet length */

	pkt_buf_ptr = (u8 *)pkt_buf;

	/* Calculate the CRC32, skip 8-byte header_A */
	ca_crc32 = crc32(0, (u8 *)(pkt_buf_ptr + HEADER_A_SIZE), new_pkt_len);

	debug("%s: crc32 is 0x%x\n", __func__, ca_crc32);
	debug("%s: ~crc32 is 0x%x\n", __func__, ~ca_crc32);
	debug("%s: pkt len %d\n", __func__, new_pkt_len);
	/* should add 8-byte header_! */
	/* CRC will re-calculated by hardware */
	memcpy((pkt_buf_ptr + new_pkt_len + HEADER_A_SIZE),
	       (u8 *)(&ca_crc32), sizeof(ca_crc32));
	new_pkt_len = new_pkt_len + 4;	/* add CRC */

	valid_bytes = new_pkt_len % 8;
	valid_bytes = valid_bytes ? valid_bytes : 0;
	debug("%s: valid_bytes %d\n", __func__, valid_bytes);

	/* should add 8-byte headerA */
	next_link = sw_tx_wr_ptr +
		(new_pkt_len + 7 + HEADER_A_SIZE) / 8; /* for headr XT */
	/* add header */
	next_link = next_link + 1;
	/* Wrap around if required */
	if (next_link > priv->tx_xram_end) {
		next_link = priv->tx_xram_start +
			(next_link - (priv->tx_xram_end + 1));
	}

	debug("%s: TX next_link %x\n", __func__, next_link);
	memset(&hdr_xt, 0, sizeof(hdr_xt));
	hdr_xt.ownership = 1;
	hdr_xt.bytes_valid = valid_bytes;
	hdr_xt.next_link = next_link;

	tx_xram_ptr = (u32 *)((unsigned long)priv->ni_xram_base
		      + sw_tx_wr_ptr * 8);

	/* Wrap around if required */
	if (tx_xram_ptr >= (u32 *)(unsigned long)priv->tx_xram_end_adr)
		tx_xram_ptr = (u32 *)(unsigned long)priv->tx_xram_base_adr;

	tx_xram_ptr = ca_rdwrptr_adv_one(tx_xram_ptr,
					 priv->tx_xram_base_adr,
					 priv->tx_xram_end_adr);

	memcpy(tx_xram_ptr, &hdr_xt, sizeof(*tx_xram_ptr));

	tx_xram_ptr = ca_rdwrptr_adv_one(tx_xram_ptr,
					 priv->tx_xram_base_adr,
					 priv->tx_xram_end_adr);

	/* Now to copy the data. The first byte on the line goes first */
	data_ptr = (u32 *)pkt_buf_ptr;
	debug("%s: packet data[]=", __func__);

	/* copy header_A to XRAM */
	for (loop = 0; loop <= (new_pkt_len + HEADER_A_SIZE) / 4; loop++) {
		ptr = (u8 *)data_ptr;
		if ((loop % 4) == 0)
			debug("\n");
		debug("[0x%x]-[0x%x]-[0x%x]-[0x%x]-",
		      ptr[0], ptr[1], ptr[2], ptr[3]);

		*tx_xram_ptr = *data_ptr++;
		tx_xram_ptr = ca_rdwrptr_adv_one(tx_xram_ptr,
						 priv->tx_xram_base_adr,
						 priv->tx_xram_end_adr);
	}
	debug("\n");

	/* Publish the software write pointer */
	cpuxram_cpu_cfg_tx.pkt_wr_ptr = next_link;
	ca_reg_write(&cpuxram_cpu_cfg_tx,
		     (u64)priv->ni_hv_base_addr,
		     NI_HV_XRAM_CPUXRAM_CPU_CFG_TX_0_OFFSET);

	return 0;
}

static void cortina_eth_stop(struct udevice *netdev)
{
	/* Nothing to do for now. */
}

static int cortina_eth_probe(struct udevice *dev)
{
	int ret, reg_value;
	struct cortina_ni_priv *priv;

	priv = dev_get_priv(dev);
	priv->rx_xram_base_adr	= priv->ni_xram_base + (RX_BASE_ADDR * 8);
	priv->rx_xram_end_adr	= priv->ni_xram_base + ((RX_TOP_ADDR + 1) * 8);
	priv->rx_xram_start	= RX_BASE_ADDR;
	priv->rx_xram_end	= RX_TOP_ADDR;
	priv->tx_xram_base_adr	= priv->ni_xram_base + (TX_BASE_ADDR * 8);
	priv->tx_xram_end_adr	= priv->ni_xram_base + ((TX_TOP_ADDR + 1) * 8);
	priv->tx_xram_start	= TX_BASE_ADDR;
	priv->tx_xram_end	= TX_TOP_ADDR;

	curr_dev = dev;
	debug("%s: rx_base_addr:%x\t rx_top_addr %x\n",
	      __func__, priv->rx_xram_start, priv->rx_xram_end);
	debug("%s: tx_base_addr:%x\t tx_top_addr %x\n",
	      __func__, priv->tx_xram_start, priv->tx_xram_end);
	debug("%s: rx physical start address = %x end address = %x\n",
	      __func__, priv->rx_xram_base_adr, priv->rx_xram_end_adr);
	debug("%s: tx physical start address = %x end address = %x\n",
	      __func__, priv->tx_xram_base_adr, priv->tx_xram_end_adr);

	/* MDIO register */
	ret = ca_mdio_register(dev);
	if (ret)
		return ret;

	/* set MDIO pre-scale value */
	ca_reg_read(&reg_value, (u64)priv->per_mdio_base_addr,
		    PER_MDIO_CFG_OFFSET);
	reg_value = reg_value | 0x00280000;
	ca_reg_write(&reg_value, (u64)priv->per_mdio_base_addr,
		     PER_MDIO_CFG_OFFSET);

	ca_phy_probe(dev);
	priv->phydev->addr = priv->port_map[priv->active_port].phy_addr;

	ca_ni_led(priv->active_port, CA_LED_ON);

	ca_ni_reset();

	printf("CA NI %s: active_port=%d, phy_addr=%d\n",
	       __func__, priv->active_port, priv->phydev->addr);
	printf("CA NI %s: phy_id=0x%x, phy_id & PHY_ID_MASK=0x%x\n", __func__,
	       priv->phydev->phy_id, priv->phydev->phy_id & 0xFFFFFFF0);

	/* parsing ethaddr and set to NI registers. */
	ca_ni_setup_mac_addr();

#ifdef MIIPHY_REGISTER
	/* the phy_read and phy_write
	 * should meet the proto type of miiphy_register
	 */
	miiphy_register(dev->name, ca_miiphy_read, ca_miiphy_write);
#endif

	if (priv->init_rgmii) {
		/* hardware settings for RGMII port */
		ca_rgmii_init(priv);
	}

	if (priv->gphy_num > 0) {
		/* do internal gphy calibration */
		ca_internal_gphy_cal(priv);
	}
	return 0;
}

static int ca_ni_of_to_plat(struct udevice *dev)
{
	int i, ret;
	struct cortina_ni_priv *priv = dev_get_priv(dev);

	memset(priv, 0, sizeof(struct cortina_ni_priv));
	priv->glb_base_addr = dev_remap_addr_index(dev, 0);
	if (!priv->glb_base_addr)
		return -ENOENT;
	printf("CA NI %s: priv->glb_base_addr for index 0 is 0x%p\n",
	       __func__, priv->glb_base_addr);

	priv->per_mdio_base_addr = dev_remap_addr_index(dev, 1);
	if (!priv->per_mdio_base_addr)
		return -ENOENT;
	printf("CA NI %s: priv->per_mdio_base_addr for index 1 is 0x%p\n",
	       __func__, priv->per_mdio_base_addr);

	priv->ni_hv_base_addr = dev_remap_addr_index(dev, 2);
	if (!priv->ni_hv_base_addr)
		return -ENOENT;
	printf("CA NI %s: priv->ni_hv_base_addr for index 2 is 0x%p\n",
	       __func__, priv->ni_hv_base_addr);

	priv->valid_port_map = dev_read_u32_default(dev, "valid-port-map", 1);
	priv->valid_port_num = dev_read_u32_default(dev, "valid-port-num", 1);

	for (i = 0; i < priv->valid_port_num; i++) {
		ret = dev_read_u32_index(dev, "valid-ports", i * 2,
					 &priv->port_map[i].phy_addr);
		ret = dev_read_u32_index(dev, "valid-ports", (i * 2) + 1,
					 &priv->port_map[i].port);
	}

	priv->gphy_num = dev_read_u32_default(dev, "inter-gphy-num", 1);
	for (i = 0; i < priv->gphy_num; i++) {
		ret = dev_read_u32_index(dev, "inter-gphy-val", i * 2,
					 &priv->gphy_values[i].reg_off);
		ret = dev_read_u32_index(dev, "inter-gphy-val", (i * 2) + 1,
					 &priv->gphy_values[i].value);
	}

	priv->active_port = dev_read_u32_default(dev, "def-active-port", 1);
	priv->init_rgmii = dev_read_u32_default(dev, "init-rgmii", 1);
	priv->ni_xram_base = dev_read_u32_default(dev, "ni-xram-base", 1);
	return 0;
}

static const struct eth_ops cortina_eth_ops = {
	.start = cortina_eth_start,
	.send = cortina_eth_send,
	.recv = cortina_eth_recv,
	.stop = cortina_eth_stop,
};

static const struct udevice_id cortina_eth_ids[] = {
	{ .compatible = "eth_cortina" },
	{ }
};

U_BOOT_DRIVER(eth_cortina) = {
	.name = "eth_cortina",
	.id = UCLASS_ETH,
	.of_match = cortina_eth_ids,
	.probe = cortina_eth_probe,
	.ops = &cortina_eth_ops,
	.priv_auto = sizeof(struct cortina_ni_priv),
	.plat_auto = sizeof(struct eth_pdata),
	.of_to_plat = ca_ni_of_to_plat,
};
