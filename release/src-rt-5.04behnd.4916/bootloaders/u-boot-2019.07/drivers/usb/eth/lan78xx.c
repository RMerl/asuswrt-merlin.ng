// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Microchip Technology Inc. All rights reserved.
 */

#include <dm.h>
#include <usb.h>
#include "usb_ether.h"
#include "lan7x.h"

/* LAN78xx specific register/bit defines */
#define LAN78XX_HW_CFG_LED1_EN		BIT(21) /* Muxed with EEDO */
#define LAN78XX_HW_CFG_LED0_EN		BIT(20) /* Muxed with EECLK */

#define LAN78XX_USB_CFG0		0x080
#define LAN78XX_USB_CFG0_BIR		BIT(6)

#define LAN78XX_BURST_CAP		0x090

#define LAN78XX_BULK_IN_DLY		0x094

#define LAN78XX_RFE_CTL			0x0B0

#define LAN78XX_FCT_RX_CTL		0x0C0

#define LAN78XX_FCT_TX_CTL		0x0C4

#define LAN78XX_FCT_RX_FIFO_END		0x0C8

#define LAN78XX_FCT_TX_FIFO_END		0x0CC

#define LAN78XX_FCT_FLOW		0x0D0

#define LAN78XX_MAF_BASE		0x400
#define LAN78XX_MAF_HIX			0x00
#define LAN78XX_MAF_LOX			0x04
#define LAN78XX_MAF_HI_BEGIN		(LAN78XX_MAF_BASE + LAN78XX_MAF_HIX)
#define LAN78XX_MAF_LO_BEGIN		(LAN78XX_MAF_BASE + LAN78XX_MAF_LOX)
#define LAN78XX_MAF_HI(index)		(LAN78XX_MAF_BASE + (8 * (index)) + \
					LAN78XX_MAF_HIX)
#define LAN78XX_MAF_LO(index)		(LAN78XX_MAF_BASE + (8 * (index)) + \
					LAN78XX_MAF_LOX)
#define LAN78XX_MAF_HI_VALID		BIT(31)

/* OTP registers */
#define LAN78XX_OTP_BASE_ADDR		0x00001000

#define LAN78XX_OTP_PWR_DN		(LAN78XX_OTP_BASE_ADDR + 4 * 0x00)
#define LAN78XX_OTP_PWR_DN_PWRDN_N	BIT(0)

#define LAN78XX_OTP_ADDR1		(LAN78XX_OTP_BASE_ADDR + 4 * 0x01)
#define LAN78XX_OTP_ADDR1_15_11		0x1F

#define LAN78XX_OTP_ADDR2		(LAN78XX_OTP_BASE_ADDR + 4 * 0x02)
#define LAN78XX_OTP_ADDR2_10_3		0xFF

#define LAN78XX_OTP_RD_DATA		(LAN78XX_OTP_BASE_ADDR + 4 * 0x06)

#define LAN78XX_OTP_FUNC_CMD		(LAN78XX_OTP_BASE_ADDR + 4 * 0x08)
#define LAN78XX_OTP_FUNC_CMD_READ	BIT(0)

#define LAN78XX_OTP_CMD_GO		(LAN78XX_OTP_BASE_ADDR + 4 * 0x0A)
#define LAN78XX_OTP_CMD_GO_GO		BIT(0)

#define LAN78XX_OTP_STATUS		(LAN78XX_OTP_BASE_ADDR + 4 * 0x0C)
#define LAN78XX_OTP_STATUS_BUSY		BIT(0)

#define LAN78XX_OTP_INDICATOR_1		0xF3
#define LAN78XX_OTP_INDICATOR_2		0xF7

/*
 * Lan78xx infrastructure commands
 */
static int lan78xx_read_raw_otp(struct usb_device *udev, u32 offset,
				u32 length, u8 *data)
{
	int i;
	int ret;
	u32 buf;

	ret = lan7x_read_reg(udev, LAN78XX_OTP_PWR_DN, &buf);
	if (ret)
		return ret;

	if (buf & LAN78XX_OTP_PWR_DN_PWRDN_N) {
		/* clear it and wait to be cleared */
		ret = lan7x_write_reg(udev, LAN78XX_OTP_PWR_DN, 0);
		if (ret)
			return ret;

		ret = lan7x_wait_for_bit(udev, "LAN78XX_OTP_PWR_DN_PWRDN_N",
					 LAN78XX_OTP_PWR_DN,
					 LAN78XX_OTP_PWR_DN_PWRDN_N,
					 false, 1000, 0);
		if (ret)
			return ret;
	}

	for (i = 0; i < length; i++) {
		ret = lan7x_write_reg(udev, LAN78XX_OTP_ADDR1,
				      ((offset + i) >> 8) &
				      LAN78XX_OTP_ADDR1_15_11);
		if (ret)
			return ret;
		ret = lan7x_write_reg(udev, LAN78XX_OTP_ADDR2,
				      ((offset + i) & LAN78XX_OTP_ADDR2_10_3));
		if (ret)
			return ret;

		ret = lan7x_write_reg(udev, LAN78XX_OTP_FUNC_CMD,
				      LAN78XX_OTP_FUNC_CMD_READ);
		if (ret)
			return ret;
		ret = lan7x_write_reg(udev, LAN78XX_OTP_CMD_GO,
				      LAN78XX_OTP_CMD_GO_GO);

		if (ret)
			return ret;

		ret = lan7x_wait_for_bit(udev, "LAN78XX_OTP_STATUS_BUSY",
					 LAN78XX_OTP_STATUS,
					 LAN78XX_OTP_STATUS_BUSY,
					 false, 1000, 0);
		if (ret)
			return ret;

		ret = lan7x_read_reg(udev, LAN78XX_OTP_RD_DATA, &buf);
		if (ret)
			return ret;

		data[i] = (u8)(buf & 0xFF);
	}

	return 0;
}

static int lan78xx_read_otp(struct usb_device *udev, u32 offset,
			    u32 length, u8 *data)
{
	u8 sig;
	int ret;

	ret = lan78xx_read_raw_otp(udev, 0, 1, &sig);

	if (!ret) {
		if (sig == LAN78XX_OTP_INDICATOR_1)
			offset = offset;
		else if (sig == LAN78XX_OTP_INDICATOR_2)
			offset += 0x100;
		else
			return -EINVAL;
		ret = lan78xx_read_raw_otp(udev, offset, length, data);
		if (ret)
			return ret;
	}
	debug("LAN78x: MAC address from OTP = %pM\n", data);

	return ret;
}

static int lan78xx_read_otp_mac(unsigned char *enetaddr,
				struct usb_device *udev)
{
	int ret;

	memset(enetaddr, 0, 6);

	ret = lan78xx_read_otp(udev,
			       EEPROM_MAC_OFFSET,
			       ETH_ALEN,
			       enetaddr);
	if (!ret && is_valid_ethaddr(enetaddr)) {
		/* eeprom values are valid so use them */
		debug("MAC address read from OTP %pM\n", enetaddr);
		return 0;
	}
	debug("MAC address read from OTP invalid %pM\n", enetaddr);

	memset(enetaddr, 0, 6);
	return -EINVAL;
}

static int lan78xx_update_flowcontrol(struct usb_device *udev,
				      struct ueth_data *dev)
{
	uint32_t flow = 0, fct_flow = 0;
	int ret;

	ret = lan7x_update_flowcontrol(udev, dev, &flow, &fct_flow);
	if (ret)
		return ret;

	ret = lan7x_write_reg(udev, LAN78XX_FCT_FLOW, fct_flow);
	if (ret)
		return ret;
	return lan7x_write_reg(udev, FLOW, flow);
}

static int lan78xx_read_mac(unsigned char *enetaddr,
			    struct usb_device *udev,
			    struct lan7x_private *priv)
{
	u32 val;
	int ret;
	int saved = 0, done = 0;

	/*
	 * Depends on chip, some EEPROM pins are muxed with LED function.
	 * disable & restore LED function to access EEPROM.
	 */
	if ((priv->chipid == ID_REV_CHIP_ID_7800) ||
	    (priv->chipid == ID_REV_CHIP_ID_7850)) {
		ret = lan7x_read_reg(udev, HW_CFG, &val);
		if (ret)
			return ret;
		saved = val;
		val &= ~(LAN78XX_HW_CFG_LED1_EN | LAN78XX_HW_CFG_LED0_EN);
		ret = lan7x_write_reg(udev, HW_CFG, val);
		if (ret)
			goto restore;
	}

	/*
	 * Refer to the doc/README.enetaddr and doc/README.usb for
	 * the U-Boot MAC address policy
	 */
	/* try reading mac address from EEPROM, then from OTP */
	ret = lan7x_read_eeprom_mac(enetaddr, udev);
	if (!ret)
		done = 1;

restore:
	if ((priv->chipid == ID_REV_CHIP_ID_7800) ||
	    (priv->chipid == ID_REV_CHIP_ID_7850)) {
		ret = lan7x_write_reg(udev, HW_CFG, saved);
		if (ret)
			return ret;
	}
	/* if the EEPROM mac address is good, then exit */
	if (done)
		return 0;

	/* try reading mac address from OTP if the device is LAN78xx */
	return lan78xx_read_otp_mac(enetaddr, udev);
}

static int lan78xx_set_receive_filter(struct usb_device *udev)
{
	/* No multicast in u-boot for now */
	return lan7x_write_reg(udev, LAN78XX_RFE_CTL,
			       RFE_CTL_BCAST_EN | RFE_CTL_DA_PERFECT);
}

/* starts the TX path */
static void lan78xx_start_tx_path(struct usb_device *udev)
{
	/* Enable Tx at MAC */
	lan7x_write_reg(udev, MAC_TX, MAC_TX_TXEN);

	/* Enable Tx at SCSRs */
	lan7x_write_reg(udev, LAN78XX_FCT_TX_CTL, FCT_TX_CTL_EN);
}

/* Starts the Receive path */
static void lan78xx_start_rx_path(struct usb_device *udev)
{
	/* Enable Rx at MAC */
	lan7x_write_reg(udev, MAC_RX,
			LAN7X_MAC_RX_MAX_SIZE_DEFAULT |
			MAC_RX_FCS_STRIP | MAC_RX_RXEN);

	/* Enable Rx at SCSRs */
	lan7x_write_reg(udev, LAN78XX_FCT_RX_CTL, FCT_RX_CTL_EN);
}

static int lan78xx_basic_reset(struct usb_device *udev,
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
	debug("LAN78xx ID_REV = 0x%08x\n", val);

	priv->chipid = (val & ID_REV_CHIP_ID_MASK) >> 16;

	/* Respond to the IN token with a NAK */
	ret = lan7x_read_reg(udev, LAN78XX_USB_CFG0, &val);
	if (ret)
		return ret;
	val &= ~LAN78XX_USB_CFG0_BIR;
	return lan7x_write_reg(udev, LAN78XX_USB_CFG0, val);
}

int lan78xx_write_hwaddr(struct udevice *dev)
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

	ret = lan7x_write_reg(udev, LAN78XX_MAF_LO(0), addr_lo);
	if (ret)
		return ret;

	ret = lan7x_write_reg(udev, LAN78XX_MAF_HI(0),
			      addr_hi | LAN78XX_MAF_HI_VALID);
	if (ret)
		return ret;

	debug("MAC addr %pM written\n", enetaddr);

	return 0;
}

static int lan78xx_eth_start(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);
	struct lan7x_private *priv = dev_get_priv(dev);

	int ret;
	u32 write_buf;

	/* Reset and read Mac addr were done in probe() */
	ret = lan78xx_write_hwaddr(dev);
	if (ret)
		return ret;

	ret = lan7x_write_reg(udev, LAN78XX_BURST_CAP, 0);
	if (ret)
		return ret;

	ret = lan7x_write_reg(udev, LAN78XX_BULK_IN_DLY, DEFAULT_BULK_IN_DELAY);
	if (ret)
		return ret;

	ret = lan7x_write_reg(udev, INT_STS, 0xFFFFFFFF);
	if (ret)
		return ret;

	/* set FIFO sizes */
	ret = lan7x_write_reg(udev, LAN78XX_FCT_RX_FIFO_END,
			      (MAX_RX_FIFO_SIZE - 512) / 512);
	if (ret)
		return ret;

	ret = lan7x_write_reg(udev, LAN78XX_FCT_TX_FIFO_END,
			      (MAX_TX_FIFO_SIZE - 512) / 512);
	if (ret)
		return ret;

	/* Init Tx */
	ret = lan7x_write_reg(udev, FLOW, 0);
	if (ret)
		return ret;

	/* Init Rx. Set Vlan, keep default for VLAN on 78xx */
	ret = lan78xx_set_receive_filter(udev);
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

	lan78xx_start_tx_path(udev);
	lan78xx_start_rx_path(udev);

	return lan78xx_update_flowcontrol(udev, &priv->ueth);
}

int lan78xx_read_rom_hwaddr(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct lan7x_private *priv = dev_get_priv(dev);
	int ret;

	ret = lan78xx_read_mac(pdata->enetaddr, udev, priv);
	if (ret)
		memset(pdata->enetaddr, 0, 6);

	return 0;
}

static int lan78xx_eth_probe(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);
	struct lan7x_private *priv = dev_get_priv(dev);
	struct ueth_data *ueth = &priv->ueth;
	struct eth_pdata *pdata = dev_get_platdata(dev);
	int ret;

	/* Do a reset in order to get the MAC address from HW */
	if (lan78xx_basic_reset(udev, ueth, priv))
		return 0;

	/* Get the MAC address */
	/*
	 * We must set the eth->enetaddr from HW because the upper layer
	 * will force to use the environmental var (usbethaddr) or random if
	 * there is no valid MAC address in eth->enetaddr.
	 */
	lan78xx_read_mac(pdata->enetaddr, udev, priv);
	/* Do not return 0 for not finding MAC addr in HW */

	ret = usb_ether_register(dev, ueth, RX_URB_SIZE);
	if (ret)
		return ret;

	/* Register phylib */
	return lan7x_phylib_register(dev);
}

static const struct eth_ops lan78xx_eth_ops = {
	.start	= lan78xx_eth_start,
	.send	= lan7x_eth_send,
	.recv	= lan7x_eth_recv,
	.free_pkt = lan7x_free_pkt,
	.stop	= lan7x_eth_stop,
	.write_hwaddr = lan78xx_write_hwaddr,
	.read_rom_hwaddr = lan78xx_read_rom_hwaddr,
};

U_BOOT_DRIVER(lan78xx_eth) = {
	.name	= "lan78xx_eth",
	.id	= UCLASS_ETH,
	.probe	= lan78xx_eth_probe,
	.remove	= lan7x_eth_remove,
	.ops	= &lan78xx_eth_ops,
	.priv_auto_alloc_size = sizeof(struct lan7x_private),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};

static const struct usb_device_id lan78xx_eth_id_table[] = {
	{ USB_DEVICE(0x0424, 0x7800) },	/* LAN7800 USB Ethernet */
	{ USB_DEVICE(0x0424, 0x7850) },	/* LAN7850 USB Ethernet */
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(lan78xx_eth, lan78xx_eth_id_table);
