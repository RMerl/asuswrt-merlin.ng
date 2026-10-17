// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Microchip Technology Inc. All rights reserved.
 */

#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <miiphy.h>
#include <memalign.h>
#include <net.h>
#include <usb.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include "usb_ether.h"
#include "lan7x.h"

/*
 * Lan7x infrastructure commands
 */
int lan7x_write_reg(struct usb_device *udev, u32 index, u32 data)
{
	int len;
	ALLOC_CACHE_ALIGN_BUFFER(u32, tmpbuf, 1);

	cpu_to_le32s(&data);
	tmpbuf[0] = data;

	len = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			      USB_VENDOR_REQUEST_WRITE_REGISTER,
			      USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			      0, index, tmpbuf, sizeof(data),
			      USB_CTRL_SET_TIMEOUT_MS);
	if (len != sizeof(data)) {
		debug("%s failed: index=%d, data=%d, len=%d",
		      __func__, index, data, len);
		return -EIO;
	}
	return 0;
}

int lan7x_read_reg(struct usb_device *udev, u32 index, u32 *data)
{
	int len;
	ALLOC_CACHE_ALIGN_BUFFER(u32, tmpbuf, 1);

	len = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			      USB_VENDOR_REQUEST_READ_REGISTER,
			      USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			      0, index, tmpbuf, sizeof(*data),
			      USB_CTRL_GET_TIMEOUT_MS);
	*data = tmpbuf[0];
	if (len != sizeof(*data)) {
		debug("%s failed: index=%d, len=%d", __func__, index, len);
		return -EIO;
	}

	le32_to_cpus(data);
	return 0;
}

static int lan7x_phy_wait_not_busy(struct usb_device *udev)
{
	return lan7x_wait_for_bit(udev, __func__,
				  MII_ACC, MII_ACC_MII_BUSY,
				  false, 100, 0);
}

int lan7x_mdio_read(struct usb_device *udev, int phy_id, int idx)
{
	u32 val, addr;

	/* confirm MII not busy */
	if (lan7x_phy_wait_not_busy(udev)) {
		debug("MII is busy in %s\n", __func__);
		return -ETIMEDOUT;
	}

	/* set the address, index & direction (read from PHY) */
	addr = (phy_id << 11) | (idx << 6) |
		MII_ACC_MII_READ | MII_ACC_MII_BUSY;
	lan7x_write_reg(udev, MII_ACC, addr);

	if (lan7x_phy_wait_not_busy(udev)) {
		debug("Timed out reading MII reg %02X\n", idx);
		return -ETIMEDOUT;
	}

	lan7x_read_reg(udev, MII_DATA, &val);

	return val & 0xFFFF;
}

void lan7x_mdio_write(struct usb_device *udev, int phy_id, int idx, int regval)
{
	u32 addr;

	/* confirm MII not busy */
	if (lan7x_phy_wait_not_busy(udev)) {
		debug("MII is busy in %s\n", __func__);
		return;
	}

	lan7x_write_reg(udev, MII_DATA, regval);

	/* set the address, index & direction (write to PHY) */
	addr = (phy_id << 11) | (idx << 6) |
		MII_ACC_MII_WRITE | MII_ACC_MII_BUSY;
	lan7x_write_reg(udev, MII_ACC, addr);

	if (lan7x_phy_wait_not_busy(udev))
		debug("Timed out writing MII reg %02X\n", idx);
}

/*
 * Lan7x phylib wrappers
 */
static int lan7x_phylib_mdio_read(struct mii_dev *bus,
				  int addr, int devad, int reg)
{
	struct usb_device *udev = dev_get_parent_priv(bus->priv);

	return lan7x_mdio_read(udev, addr, reg);
}

static int lan7x_phylib_mdio_write(struct mii_dev *bus,
				   int addr, int devad, int reg, u16 val)
{
	struct usb_device *udev = dev_get_parent_priv(bus->priv);

	lan7x_mdio_write(udev, addr, reg, (int)val);

	return 0;
}

/*
 * Lan7x eeprom functions
 */
static int lan7x_eeprom_confirm_not_busy(struct usb_device *udev)
{
	return lan7x_wait_for_bit(udev, __func__,
				  E2P_CMD, E2P_CMD_EPC_BUSY,
				  false, 100, 0);
}

static int lan7x_wait_eeprom(struct usb_device *udev)
{
	return lan7x_wait_for_bit(udev, __func__,
				  E2P_CMD,
				  (E2P_CMD_EPC_BUSY | E2P_CMD_EPC_TIMEOUT),
				  false, 100, 0);
}

static int lan7x_read_eeprom(struct usb_device *udev,
			     u32 offset, u32 length, u8 *data)
{
	u32 val;
	int i, ret;

	ret = lan7x_eeprom_confirm_not_busy(udev);
	if (ret)
		return ret;

	for (i = 0; i < length; i++) {
		val = E2P_CMD_EPC_BUSY | E2P_CMD_EPC_CMD_READ |
			(offset & E2P_CMD_EPC_ADDR_MASK);
		lan7x_write_reg(udev, E2P_CMD, val);

		ret = lan7x_wait_eeprom(udev);
		if (ret)
			return ret;

		lan7x_read_reg(udev, E2P_DATA, &val);
		data[i] = val & 0xFF;
		offset++;
	}
	return ret;
}

/*
 * Lan7x phylib functions
 */
int lan7x_phylib_register(struct udevice *udev)
{
	struct usb_device *usbdev = dev_get_parent_priv(udev);
	struct lan7x_private *priv = dev_get_priv(udev);
	int ret;

	priv->mdiobus = mdio_alloc();
	if (!priv->mdiobus) {
		printf("mdio_alloc failed\n");
		return -ENOMEM;
	}
	priv->mdiobus->read = lan7x_phylib_mdio_read;
	priv->mdiobus->write = lan7x_phylib_mdio_write;
	sprintf(priv->mdiobus->name,
		"lan7x_mdiobus-d%hu-p%hu", usbdev->devnum, usbdev->portnr);
	priv->mdiobus->priv = (void *)udev;

	ret = mdio_register(priv->mdiobus);
	if (ret) {
		printf("mdio_register failed\n");
		free(priv->mdiobus);
		return -ENOMEM;
	}

	return 0;
}

int lan7x_eth_phylib_connect(struct udevice *udev, struct ueth_data *dev)
{
	struct lan7x_private *priv = dev_get_priv(udev);

	priv->phydev = phy_connect(priv->mdiobus, dev->phy_id,
			     udev, PHY_INTERFACE_MODE_MII);

	if (!priv->phydev) {
		printf("phy_connect failed\n");
		return -ENODEV;
	}
	return 0;
}

int lan7x_eth_phylib_config_start(struct udevice *udev)
{
	struct lan7x_private *priv = dev_get_priv(udev);
	int ret;

	/* configure supported modes */
	priv->phydev->supported = PHY_BASIC_FEATURES |
				  SUPPORTED_1000baseT_Full |
				  SUPPORTED_Pause |
				  SUPPORTED_Asym_Pause;

	priv->phydev->advertising = ADVERTISED_10baseT_Half |
				    ADVERTISED_10baseT_Full |
				    ADVERTISED_100baseT_Half |
				    ADVERTISED_100baseT_Full |
				    ADVERTISED_1000baseT_Full |
				    ADVERTISED_Pause |
				    ADVERTISED_Asym_Pause |
				    ADVERTISED_Autoneg;

	priv->phydev->autoneg = AUTONEG_ENABLE;

	ret = genphy_config_aneg(priv->phydev);
	if (ret) {
		printf("genphy_config_aneg failed\n");
		return ret;
	}
	ret = phy_startup(priv->phydev);
	if (ret) {
		printf("phy_startup failed\n");
		return ret;
	}

	debug("** %s() speed %i duplex %i adv %X supp %X\n", __func__,
	      priv->phydev->speed, priv->phydev->duplex,
	      priv->phydev->advertising, priv->phydev->supported);

	return 0;
}

int lan7x_update_flowcontrol(struct usb_device *udev,
			     struct ueth_data *dev,
			     uint32_t *flow, uint32_t *fct_flow)
{
	uint32_t lcladv, rmtadv;
	u8 cap = 0;
	struct lan7x_private *priv = dev_get_priv(udev->dev);

	debug("** %s()\n", __func__);
	debug("** %s() priv->phydev->speed %i duplex %i\n", __func__,
	      priv->phydev->speed, priv->phydev->duplex);

	if (priv->phydev->duplex == DUPLEX_FULL) {
		lcladv = lan7x_mdio_read(udev, dev->phy_id, MII_ADVERTISE);
		rmtadv = lan7x_mdio_read(udev, dev->phy_id, MII_LPA);
		cap = mii_resolve_flowctrl_fdx(lcladv, rmtadv);

		debug("TX Flow ");
		if (cap & FLOW_CTRL_TX) {
			*flow = (FLOW_CR_TX_FCEN | 0xFFFF);
			/* set fct_flow thresholds to 20% and 80% */
			*fct_flow = ((MAX_RX_FIFO_SIZE * 2) / (10 * 512))
					& 0x7FUL;
			*fct_flow <<= 8UL;
			*fct_flow |= ((MAX_RX_FIFO_SIZE * 8) / (10 * 512))
					& 0x7FUL;
			debug("EN ");
		} else {
			debug("DIS ");
		}
		debug("RX Flow ");
		if (cap & FLOW_CTRL_RX) {
			*flow |= FLOW_CR_RX_FCEN;
			debug("EN");
		} else {
			debug("DIS");
		}
	}
	debug("\n");
	return 0;
}

int lan7x_read_eeprom_mac(unsigned char *enetaddr, struct usb_device *udev)
{
	int ret;

	memset(enetaddr, 0, 6);

	ret = lan7x_read_eeprom(udev, 0, 1, enetaddr);

	if ((ret == 0) && (enetaddr[0] == EEPROM_INDICATOR)) {
		ret = lan7x_read_eeprom(udev,
					EEPROM_MAC_OFFSET, ETH_ALEN,
					enetaddr);
		if ((ret == 0) && is_valid_ethaddr(enetaddr)) {
			/* eeprom values are valid so use them */
			debug("MAC address read from EEPROM %pM\n",
			      enetaddr);
			return 0;
		}
	}
	debug("MAC address read from EEPROM invalid %pM\n", enetaddr);

	memset(enetaddr, 0, 6);
	return -EINVAL;
}

int lan7x_pmt_phy_reset(struct usb_device *udev,
			struct ueth_data *dev)
{
	int ret;
	u32 data;

	ret = lan7x_read_reg(udev, PMT_CTL, &data);
	if (ret)
		return ret;
	ret = lan7x_write_reg(udev, PMT_CTL, data | PMT_CTL_PHY_RST);
	if (ret)
		return ret;

	/* for LAN7x, we need to check PMT_CTL_READY asserted */
	ret = lan7x_wait_for_bit(udev, "PMT_CTL_PHY_RST",
				 PMT_CTL, PMT_CTL_PHY_RST,
				 false, 1000, 0); /* could take over 125mS */
	if (ret)
		return ret;

	return lan7x_wait_for_bit(udev, "PMT_CTL_READY",
				 PMT_CTL, PMT_CTL_READY,
				 true, 1000, 0);
}

int lan7x_basic_reset(struct usb_device *udev,
		      struct ueth_data *dev)
{
	int ret;

	dev->phy_id = LAN7X_INTERNAL_PHY_ID; /* fixed phy id */

	ret = lan7x_write_reg(udev, HW_CFG, HW_CFG_LRST);
	if (ret)
		return ret;

	ret = lan7x_wait_for_bit(udev, "HW_CFG_LRST",
				 HW_CFG, HW_CFG_LRST,
				 false, 1000, 0);
	if (ret)
		return ret;

	debug("USB devnum %d portnr %d\n", udev->devnum, udev->portnr);

	return lan7x_pmt_phy_reset(udev, dev);
}

void lan7x_eth_stop(struct udevice *dev)
{
	debug("** %s()\n", __func__);
}

int lan7x_eth_send(struct udevice *dev, void *packet, int length)
{
	struct lan7x_private *priv = dev_get_priv(dev);
	struct ueth_data *ueth = &priv->ueth;
	int err;
	int actual_len;
	u32 tx_cmd_a;
	u32 tx_cmd_b;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, msg,
				 PKTSIZE + sizeof(tx_cmd_a) + sizeof(tx_cmd_b));

	debug("** %s(), len %d, buf %#x\n", __func__, length,
	      (unsigned int)(ulong) msg);
	if (length > PKTSIZE)
		return -ENOSPC;

	/* LAN7x disable all TX offload features for u-boot */
	tx_cmd_a = (u32) (length & TX_CMD_A_LEN_MASK) | TX_CMD_A_FCS;
	tx_cmd_b = 0;
	cpu_to_le32s(&tx_cmd_a);
	cpu_to_le32s(&tx_cmd_b);

	/* prepend cmd_a and cmd_b */
	memcpy(msg, &tx_cmd_a, sizeof(tx_cmd_a));
	memcpy(msg + sizeof(tx_cmd_a), &tx_cmd_b, sizeof(tx_cmd_b));
	memcpy(msg + sizeof(tx_cmd_a) + sizeof(tx_cmd_b), (void *)packet,
	       length);
	err = usb_bulk_msg(ueth->pusb_dev,
			   usb_sndbulkpipe(ueth->pusb_dev, ueth->ep_out),
			   (void *)msg,
			   length + sizeof(tx_cmd_a) +
			   sizeof(tx_cmd_b),
			   &actual_len, USB_BULK_SEND_TIMEOUT_MS);
	debug("Tx: len = %u, actual = %u, err = %d\n",
	      (unsigned int)(length + sizeof(tx_cmd_a) + sizeof(tx_cmd_b)),
	      (unsigned int)actual_len, err);

	return err;
}

int lan7x_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct lan7x_private *priv = dev_get_priv(dev);
	struct ueth_data *ueth = &priv->ueth;
	uint8_t *ptr;
	int ret, len;
	u32 packet_len = 0;
	u32 rx_cmd_a = 0;

	len = usb_ether_get_rx_bytes(ueth, &ptr);
	debug("%s: first try, len=%d\n", __func__, len);
	if (!len) {
		if (!(flags & ETH_RECV_CHECK_DEVICE))
			return -EAGAIN;
		ret = usb_ether_receive(ueth, RX_URB_SIZE);
		if (ret == -EAGAIN)
			return ret;

		len = usb_ether_get_rx_bytes(ueth, &ptr);
		debug("%s: second try, len=%d\n", __func__, len);
	}

	/*
	 * 1st 4 bytes contain the length of the actual data plus error info.
	 * Extract data length.
	 */
	if (len < sizeof(packet_len)) {
		debug("Rx: incomplete packet length\n");
		goto err;
	}
	memcpy(&rx_cmd_a, ptr, sizeof(rx_cmd_a));
	le32_to_cpus(&rx_cmd_a);
	if (rx_cmd_a & RX_CMD_A_RXE) {
		debug("Rx: Error header=%#x", rx_cmd_a);
		goto err;
	}
	packet_len = (u16) (rx_cmd_a & RX_CMD_A_LEN_MASK);

	if (packet_len > len - sizeof(packet_len)) {
		debug("Rx: too large packet: %d\n", packet_len);
		goto err;
	}

	/*
	 * For LAN7x, the length in command A does not
	 * include command A, B, and C length.
	 * So use it as is.
	 */

	*packetp = ptr + 10;
	return packet_len;

err:
	usb_ether_advance_rxbuf(ueth, -1);
	return -EINVAL;
}

int lan7x_free_pkt(struct udevice *dev, uchar *packet, int packet_len)
{
	struct lan7x_private *priv = dev_get_priv(dev);

	packet_len = ALIGN(packet_len, 4);
	usb_ether_advance_rxbuf(&priv->ueth, sizeof(u32) + packet_len);

	return 0;
}

int lan7x_eth_remove(struct udevice *dev)
{
	struct lan7x_private *priv = dev_get_priv(dev);

	debug("** %s()\n", __func__);
	free(priv->phydev);
	mdio_unregister(priv->mdiobus);
	mdio_free(priv->mdiobus);

	return 0;
}
