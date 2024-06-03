// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Microchip Technology Inc. All rights reserved.
 */

#include <dm.h>
#include <usb.h>
#include <linux/mii.h>
#include "usb_ether.h"
#include "lan7x.h"

/* LAN75xx specific register/bit defines */
#define LAN75XX_HW_CFG_BIR		BIT(7)

#define LAN75XX_BURST_CAP		0x034

#define LAN75XX_BULK_IN_DLY		0x03C

#define LAN75XX_RFE_CTL			0x060

#define LAN75XX_FCT_RX_CTL		0x090

#define LAN75XX_FCT_TX_CTL		0x094

#define LAN75XX_FCT_RX_FIFO_END		0x098

#define LAN75XX_FCT_TX_FIFO_END		0x09C

#define LAN75XX_FCT_FLOW		0x0A0

/* MAC ADDRESS PERFECT FILTER For LAN75xx */
#define LAN75XX_ADDR_FILTX		0x300
#define LAN75XX_ADDR_FILTX_FB_VALID	BIT(31)

/*
 * Lan75xx infrastructure commands
 */
static int lan75xx_phy_gig_workaround(struct usb_device *udev,
				      struct ueth_data *dev)
{
	int ret = 0;

	/* Only internal phy */
	/* Set the phy in Gig loopback */
	lan7x_mdio_write(udev, dev->phy_id, MII_BMCR,
			 (BMCR_LOOPBACK | BMCR_SPEED1000));

	/* Wait for the link up */
	ret = lan7x_mdio_wait_for_bit(udev, "BMSR_LSTATUS",
				      dev->phy_id, MII_BMSR, BMSR_LSTATUS,
				      true, PHY_CONNECT_TIMEOUT_MS, 1);
	if (ret)
		return ret;

	/* phy reset */
	return lan7x_pmt_phy_reset(udev, dev);
}

static int lan75xx_update_flowcontrol(struct usb_device *udev,
				      struct ueth_data *dev)
{
	uint32_t flow = 0, fct_flow = 0;
	int ret;

	ret = lan7x_update_flowcontrol(udev, dev, &flow, &fct_flow);
	if (ret)
		return ret;

	ret = lan7x_write_reg(udev, LAN75XX_FCT_FLOW, fct_flow);
	if (ret)
		return ret;
	return lan7x_write_reg(udev, FLOW, flow);
}

static int lan75xx_set_receive_filter(struct usb_device *udev)
{
	/* No multicast in u-boot */
	return lan7x_write_reg(udev, LAN75XX_RFE_CTL,
			       RFE_CTL_BCAST_EN | RFE_CTL_DA_PERFECT);
}

/* starts the TX path */
static void lan75xx_start_tx_path(struct usb_device *udev)
{
	/* Enable Tx at MAC */
	lan7x_write_reg(udev, MAC_TX, MAC_TX_TXEN);

	/* Enable Tx at SCSRs */
	lan7x_write_reg(udev, LAN75XX_FCT_TX_CTL, FCT_TX_CTL_EN);
}

/* Starts the Receive path */
static void lan75xx_start_rx_path(struct usb_device *udev)
{
	/* Enable Rx at MAC */
	lan7x_write_reg(udev, MAC_RX,
			LAN7X_MAC_RX_MAX_SIZE_DEFAULT |
			MAC_RX_FCS_STRIP | MAC_RX_RXEN);

	/* Enable Rx at SCSRs */
	lan7x_write_reg(udev, LAN75XX_FCT_RX_CTL, FCT_RX_CTL_EN);
}

static int lan75xx_basic_reset(struct usb_device *udev,
			       struct ueth_data *dev,
			       struct lan7x_private *priv)
{
	int ret;
	u32 val;

	ret = lan7x_basic_reset(udev, dev);
	if (ret)
		return ret;

	/* Keep the chip ID */
	ret = lan7x_read_reg(udev, ID_REV, &val);
	if (ret)
		return ret;
	debug("LAN75xx ID_REV = 0x%08x\n", val);

	priv->chipid = (val & ID_REV_CHIP_ID_MASK) >> 16;

	/* Respond to the IN token with a NAK */
	ret = lan7x_read_reg(udev, HW_CFG, &val);
	if (ret)
		return ret;
	val |= LAN75XX_HW_CFG_BIR;
	return lan7x_write_reg(udev, HW_CFG, val);
}

int lan75xx_write_hwaddr(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	unsigned char *enetaddr = pdata->enetaddr;
	u32 addr_lo = get_unaligned_le32(&enetaddr[0]);
	u32 addr_hi = (u32)get_unaligned_le16(&enetaddr[4]);
	int ret;

	/* set hardware address */
	ret = lan7x_write_reg(udev, RX_ADDRL, addr_lo);
	if (ret)
		return ret;

	ret = lan7x_write_reg(udev, RX_ADDRH, addr_hi);
	if (ret)
		return ret;

	ret = lan7x_write_reg(udev, LAN75XX_ADDR_FILTX + 4, addr_lo);
	if (ret)
		return ret;

	addr_hi |= LAN75XX_ADDR_FILTX_FB_VALID;
	ret = lan7x_write_reg(udev, LAN75XX_ADDR_FILTX, addr_hi);
	if (ret)
		return ret;

	debug("MAC addr %pM written\n", enetaddr);

	return 0;
}

static int lan75xx_eth_start(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);
	struct lan7x_private *priv = dev_get_priv(dev);
	struct ueth_data *ueth = &priv->ueth;
	int ret;
	u32 write_buf;

	/* Reset and read Mac addr were done in probe() */
	ret = lan75xx_write_hwaddr(dev);
	if (ret)
		return ret;

	ret = lan7x_write_reg(udev, INT_STS, 0xFFFFFFFF);
	if (ret)
		return ret;

	ret = lan7x_write_reg(udev, LAN75XX_BURST_CAP, 0);
	if (ret)
		return ret;

	ret = lan7x_write_reg(udev, LAN75XX_BULK_IN_DLY, DEFAULT_BULK_IN_DELAY);
	if (ret)
		return ret;

	/* set FIFO sizes */
	write_buf = (MAX_RX_FIFO_SIZE - 512) / 512;
	ret = lan7x_write_reg(udev, LAN75XX_FCT_RX_FIFO_END, write_buf);
	if (ret)
		return ret;

	write_buf = (MAX_TX_FIFO_SIZE - 512) / 512;
	ret = lan7x_write_reg(udev, LAN75XX_FCT_TX_FIFO_END, write_buf);
	if (ret)
		return ret;

	/* Init Tx */
	ret = lan7x_write_reg(udev, FLOW, 0);
	if (ret)
		return ret;

	/* Init Rx. Set Vlan, keep default for VLAN on 75xx */
	ret = lan75xx_set_receive_filter(udev);
	if (ret)
		return ret;

	/* phy workaround for gig link */
	ret = lan75xx_phy_gig_workaround(udev, ueth);
	if (ret)
		return ret;

	/* Init PHY, autonego, and link */
	ret = lan7x_eth_phylib_connect(dev, &priv->ueth);
	if (ret)
		return ret;
	ret = lan7x_eth_phylib_config_start(dev);
	if (ret)
		return ret;

	/*
	 * MAC_CR has to be set after PHY init.
	 * MAC will auto detect the PHY speed.
	 */
	ret = lan7x_read_reg(udev, MAC_CR, &write_buf);
	if (ret)
		return ret;
	write_buf |= MAC_CR_AUTO_DUPLEX | MAC_CR_AUTO_SPEED | MAC_CR_ADP;
	ret = lan7x_write_reg(udev, MAC_CR, write_buf);
	if (ret)
		return ret;

	lan75xx_start_tx_path(udev);
	lan75xx_start_rx_path(udev);

	return lan75xx_update_flowcontrol(udev, ueth);
}

int lan75xx_read_rom_hwaddr(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	int ret;

	/*
	 * Refer to the doc/README.enetaddr and doc/README.usb for
	 * the U-Boot MAC address policy
	 */
	ret = lan7x_read_eeprom_mac(pdata->enetaddr, udev);
	if (ret)
		memset(pdata->enetaddr, 0, 6);

	return 0;
}

static int lan75xx_eth_probe(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);
	struct lan7x_private *priv = dev_get_priv(dev);
	struct ueth_data *ueth = &priv->ueth;
	struct eth_pdata *pdata = dev_get_platdata(dev);
	int ret;

	/* Do a reset in order to get the MAC address from HW */
	if (lan75xx_basic_reset(udev, ueth, priv))
		return 0;

	/* Get the MAC address */
	/*
	 * We must set the eth->enetaddr from HW because the upper layer
	 * will force to use the environmental var (usbethaddr) or random if
	 * there is no valid MAC address in eth->enetaddr.
	 *
	 * Refer to the doc/README.enetaddr and doc/README.usb for
	 * the U-Boot MAC address policy
	 */
	lan7x_read_eeprom_mac(pdata->enetaddr, udev);
	/* Do not return 0 for not finding MAC addr in HW */

	ret = usb_ether_register(dev, ueth, RX_URB_SIZE);
	if (ret)
		return ret;

	/* Register phylib */
	return lan7x_phylib_register(dev);
}

static const struct eth_ops lan75xx_eth_ops = {
	.start	= lan75xx_eth_start,
	.send	= lan7x_eth_send,
	.recv	= lan7x_eth_recv,
	.free_pkt = lan7x_free_pkt,
	.stop	= lan7x_eth_stop,
	.write_hwaddr = lan75xx_write_hwaddr,
	.read_rom_hwaddr = lan75xx_read_rom_hwaddr,
};

U_BOOT_DRIVER(lan75xx_eth) = {
	.name	= "lan75xx_eth",
	.id	= UCLASS_ETH,
	.probe	= lan75xx_eth_probe,
	.remove	= lan7x_eth_remove,
	.ops	= &lan75xx_eth_ops,
	.priv_auto_alloc_size = sizeof(struct lan7x_private),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};

static const struct usb_device_id lan75xx_eth_id_table[] = {
	{ USB_DEVICE(0x0424, 0x7500) },	/* LAN7500 USB Ethernet */
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(lan75xx_eth, lan75xx_eth_id_table);
