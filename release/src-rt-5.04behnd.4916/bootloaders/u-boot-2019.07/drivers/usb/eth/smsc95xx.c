// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Copyright (c) 2011 The Chromium OS Authors.
 * Copyright (C) 2009 NVIDIA, Corporation
 * Copyright (C) 2007-2008 SMSC (Steve Glendinning)
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <memalign.h>
#include <usb.h>
#include <asm/unaligned.h>
#include <linux/mii.h>
#include "usb_ether.h"

/* SMSC LAN95xx based USB 2.0 Ethernet Devices */

/* LED defines */
#define LED_GPIO_CFG			(0x24)
#define LED_GPIO_CFG_SPD_LED		(0x01000000)
#define LED_GPIO_CFG_LNK_LED		(0x00100000)
#define LED_GPIO_CFG_FDX_LED		(0x00010000)

/* Tx command words */
#define TX_CMD_A_FIRST_SEG_		0x00002000
#define TX_CMD_A_LAST_SEG_		0x00001000

/* Rx status word */
#define RX_STS_FL_			0x3FFF0000	/* Frame Length */
#define RX_STS_ES_			0x00008000	/* Error Summary */

/* SCSRs */
#define ID_REV				0x00

#define INT_STS				0x08

#define TX_CFG				0x10
#define TX_CFG_ON_			0x00000004

#define HW_CFG				0x14
#define HW_CFG_BIR_			0x00001000
#define HW_CFG_RXDOFF_			0x00000600
#define HW_CFG_MEF_			0x00000020
#define HW_CFG_BCE_			0x00000002
#define HW_CFG_LRST_			0x00000008

#define PM_CTRL				0x20
#define PM_CTL_PHY_RST_			0x00000010

#define AFC_CFG				0x2C

/*
 * Hi watermark = 15.5Kb (~10 mtu pkts)
 * low watermark = 3k (~2 mtu pkts)
 * backpressure duration = ~ 350us
 * Apply FC on any frame.
 */
#define AFC_CFG_DEFAULT			0x00F830A1

#define E2P_CMD				0x30
#define E2P_CMD_BUSY_			0x80000000
#define E2P_CMD_READ_			0x00000000
#define E2P_CMD_TIMEOUT_		0x00000400
#define E2P_CMD_LOADED_			0x00000200
#define E2P_CMD_ADDR_			0x000001FF

#define E2P_DATA			0x34

#define BURST_CAP			0x38

#define INT_EP_CTL			0x68
#define INT_EP_CTL_PHY_INT_		0x00008000

#define BULK_IN_DLY			0x6C

/* MAC CSRs */
#define MAC_CR				0x100
#define MAC_CR_MCPAS_			0x00080000
#define MAC_CR_PRMS_			0x00040000
#define MAC_CR_HPFILT_			0x00002000
#define MAC_CR_TXEN_			0x00000008
#define MAC_CR_RXEN_			0x00000004

#define ADDRH				0x104

#define ADDRL				0x108

#define MII_ADDR			0x114
#define MII_WRITE_			0x02
#define MII_BUSY_			0x01
#define MII_READ_			0x00 /* ~of MII Write bit */

#define MII_DATA			0x118

#define FLOW				0x11C

#define VLAN1				0x120

#define COE_CR				0x130
#define Tx_COE_EN_			0x00010000
#define Rx_COE_EN_			0x00000001

/* Vendor-specific PHY Definitions */
#define PHY_INT_SRC			29

#define PHY_INT_MASK			30
#define PHY_INT_MASK_ANEG_COMP_		((u16)0x0040)
#define PHY_INT_MASK_LINK_DOWN_		((u16)0x0010)
#define PHY_INT_MASK_DEFAULT_		(PHY_INT_MASK_ANEG_COMP_ | \
					 PHY_INT_MASK_LINK_DOWN_)

/* USB Vendor Requests */
#define USB_VENDOR_REQUEST_WRITE_REGISTER	0xA0
#define USB_VENDOR_REQUEST_READ_REGISTER	0xA1

/* Some extra defines */
#define HS_USB_PKT_SIZE			512
#define FS_USB_PKT_SIZE			64
/* 5/33 is lower limit for BURST_CAP to work */
#define DEFAULT_HS_BURST_CAP_SIZE	(5 * HS_USB_PKT_SIZE)
#define DEFAULT_FS_BURST_CAP_SIZE	(33 * FS_USB_PKT_SIZE)
#define DEFAULT_BULK_IN_DELAY		0x00002000
#define MAX_SINGLE_PACKET_SIZE		2048
#define EEPROM_MAC_OFFSET		0x01
#define SMSC95XX_INTERNAL_PHY_ID	1
#define ETH_P_8021Q	0x8100          /* 802.1Q VLAN Extended Header  */

/* local defines */
#define SMSC95XX_BASE_NAME "sms"
#define USB_CTRL_SET_TIMEOUT 5000
#define USB_CTRL_GET_TIMEOUT 5000
#define USB_BULK_SEND_TIMEOUT 5000
#define USB_BULK_RECV_TIMEOUT 5000

#define RX_URB_SIZE DEFAULT_HS_BURST_CAP_SIZE
#define PHY_CONNECT_TIMEOUT 5000

#define TURBO_MODE

#ifndef CONFIG_DM_ETH
/* local vars */
static int curr_eth_dev; /* index for name of next device detected */
#endif

/* driver private */
struct smsc95xx_private {
#ifdef CONFIG_DM_ETH
	struct ueth_data ueth;
#endif
	size_t rx_urb_size;  /* maximum USB URB size */
	u32 mac_cr;  /* MAC control register value */
	int have_hwaddr;  /* 1 if we have a hardware MAC address */
};

/*
 * Smsc95xx infrastructure commands
 */
static int smsc95xx_write_reg(struct usb_device *udev, u32 index, u32 data)
{
	int len;
	ALLOC_CACHE_ALIGN_BUFFER(u32, tmpbuf, 1);

	cpu_to_le32s(&data);
	tmpbuf[0] = data;

	len = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			      USB_VENDOR_REQUEST_WRITE_REGISTER,
			      USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			      0, index, tmpbuf, sizeof(data),
			      USB_CTRL_SET_TIMEOUT);
	if (len != sizeof(data)) {
		debug("smsc95xx_write_reg failed: index=%d, data=%d, len=%d",
		      index, data, len);
		return -EIO;
	}
	return 0;
}

static int smsc95xx_read_reg(struct usb_device *udev, u32 index, u32 *data)
{
	int len;
	ALLOC_CACHE_ALIGN_BUFFER(u32, tmpbuf, 1);

	len = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			      USB_VENDOR_REQUEST_READ_REGISTER,
			      USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			      0, index, tmpbuf, sizeof(*data),
			      USB_CTRL_GET_TIMEOUT);
	*data = tmpbuf[0];
	if (len != sizeof(*data)) {
		debug("smsc95xx_read_reg failed: index=%d, len=%d",
		      index, len);
		return -EIO;
	}

	le32_to_cpus(data);
	return 0;
}

/* Loop until the read is completed with timeout */
static int smsc95xx_phy_wait_not_busy(struct usb_device *udev)
{
	unsigned long start_time = get_timer(0);
	u32 val;

	do {
		smsc95xx_read_reg(udev, MII_ADDR, &val);
		if (!(val & MII_BUSY_))
			return 0;
	} while (get_timer(start_time) < 1000);

	return -ETIMEDOUT;
}

static int smsc95xx_mdio_read(struct usb_device *udev, int phy_id, int idx)
{
	u32 val, addr;

	/* confirm MII not busy */
	if (smsc95xx_phy_wait_not_busy(udev)) {
		debug("MII is busy in smsc95xx_mdio_read\n");
		return -ETIMEDOUT;
	}

	/* set the address, index & direction (read from PHY) */
	addr = (phy_id << 11) | (idx << 6) | MII_READ_;
	smsc95xx_write_reg(udev, MII_ADDR, addr);

	if (smsc95xx_phy_wait_not_busy(udev)) {
		debug("Timed out reading MII reg %02X\n", idx);
		return -ETIMEDOUT;
	}

	smsc95xx_read_reg(udev, MII_DATA, &val);

	return (u16)(val & 0xFFFF);
}

static void smsc95xx_mdio_write(struct usb_device *udev, int phy_id, int idx,
				int regval)
{
	u32 val, addr;

	/* confirm MII not busy */
	if (smsc95xx_phy_wait_not_busy(udev)) {
		debug("MII is busy in smsc95xx_mdio_write\n");
		return;
	}

	val = regval;
	smsc95xx_write_reg(udev, MII_DATA, val);

	/* set the address, index & direction (write to PHY) */
	addr = (phy_id << 11) | (idx << 6) | MII_WRITE_;
	smsc95xx_write_reg(udev, MII_ADDR, addr);

	if (smsc95xx_phy_wait_not_busy(udev))
		debug("Timed out writing MII reg %02X\n", idx);
}

static int smsc95xx_eeprom_confirm_not_busy(struct usb_device *udev)
{
	unsigned long start_time = get_timer(0);
	u32 val;

	do {
		smsc95xx_read_reg(udev, E2P_CMD, &val);
		if (!(val & E2P_CMD_BUSY_))
			return 0;
		udelay(40);
	} while (get_timer(start_time) < 1 * 1000 * 1000);

	debug("EEPROM is busy\n");
	return -ETIMEDOUT;
}

static int smsc95xx_wait_eeprom(struct usb_device *udev)
{
	unsigned long start_time = get_timer(0);
	u32 val;

	do {
		smsc95xx_read_reg(udev, E2P_CMD, &val);
		if (!(val & E2P_CMD_BUSY_) || (val & E2P_CMD_TIMEOUT_))
			break;
		udelay(40);
	} while (get_timer(start_time) < 1 * 1000 * 1000);

	if (val & (E2P_CMD_TIMEOUT_ | E2P_CMD_BUSY_)) {
		debug("EEPROM read operation timeout\n");
		return -ETIMEDOUT;
	}
	return 0;
}

static int smsc95xx_read_eeprom(struct usb_device *udev, u32 offset, u32 length,
				u8 *data)
{
	u32 val;
	int i, ret;

	ret = smsc95xx_eeprom_confirm_not_busy(udev);
	if (ret)
		return ret;

	for (i = 0; i < length; i++) {
		val = E2P_CMD_BUSY_ | E2P_CMD_READ_ | (offset & E2P_CMD_ADDR_);
		smsc95xx_write_reg(udev, E2P_CMD, val);

		ret = smsc95xx_wait_eeprom(udev);
		if (ret < 0)
			return ret;

		smsc95xx_read_reg(udev, E2P_DATA, &val);
		data[i] = val & 0xFF;
		offset++;
	}
	return 0;
}

/*
 * mii_nway_restart - restart NWay (autonegotiation) for this interface
 *
 * Returns 0 on success, negative on error.
 */
static int mii_nway_restart(struct usb_device *udev, struct ueth_data *dev)
{
	int bmcr;
	int r = -1;

	/* if autoneg is off, it's an error */
	bmcr = smsc95xx_mdio_read(udev, dev->phy_id, MII_BMCR);

	if (bmcr & BMCR_ANENABLE) {
		bmcr |= BMCR_ANRESTART;
		smsc95xx_mdio_write(udev, dev->phy_id, MII_BMCR, bmcr);
		r = 0;
	}
	return r;
}

static int smsc95xx_phy_initialize(struct usb_device *udev,
				   struct ueth_data *dev)
{
	smsc95xx_mdio_write(udev, dev->phy_id, MII_BMCR, BMCR_RESET);
	smsc95xx_mdio_write(udev, dev->phy_id, MII_ADVERTISE,
			    ADVERTISE_ALL | ADVERTISE_CSMA |
			    ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);

	/* read to clear */
	smsc95xx_mdio_read(udev, dev->phy_id, PHY_INT_SRC);

	smsc95xx_mdio_write(udev, dev->phy_id, PHY_INT_MASK,
			    PHY_INT_MASK_DEFAULT_);
	mii_nway_restart(udev, dev);

	debug("phy initialised succesfully\n");
	return 0;
}

static int smsc95xx_init_mac_address(unsigned char *enetaddr,
				     struct usb_device *udev)
{
	int ret;

	/* try reading mac address from EEPROM */
	ret = smsc95xx_read_eeprom(udev, EEPROM_MAC_OFFSET, ETH_ALEN, enetaddr);
	if (ret)
		return ret;

	if (is_valid_ethaddr(enetaddr)) {
		/* eeprom values are valid so use them */
		debug("MAC address read from EEPROM\n");
		return 0;
	}

	/*
	 * No eeprom, or eeprom values are invalid. Generating a random MAC
	 * address is not safe. Just return an error.
	 */
	debug("Invalid MAC address read from EEPROM\n");

	return -ENXIO;
}

static int smsc95xx_write_hwaddr_common(struct usb_device *udev,
					struct smsc95xx_private *priv,
					unsigned char *enetaddr)
{
	u32 addr_lo = get_unaligned_le32(&enetaddr[0]);
	u32 addr_hi = get_unaligned_le16(&enetaddr[4]);
	int ret;

	/* set hardware address */
	debug("** %s()\n", __func__);
	ret = smsc95xx_write_reg(udev, ADDRL, addr_lo);
	if (ret < 0)
		return ret;

	ret = smsc95xx_write_reg(udev, ADDRH, addr_hi);
	if (ret < 0)
		return ret;

	debug("MAC %pM\n", enetaddr);
	priv->have_hwaddr = 1;

	return 0;
}

/* Enable or disable Tx & Rx checksum offload engines */
static int smsc95xx_set_csums(struct usb_device *udev, int use_tx_csum,
			      int use_rx_csum)
{
	u32 read_buf;
	int ret = smsc95xx_read_reg(udev, COE_CR, &read_buf);
	if (ret < 0)
		return ret;

	if (use_tx_csum)
		read_buf |= Tx_COE_EN_;
	else
		read_buf &= ~Tx_COE_EN_;

	if (use_rx_csum)
		read_buf |= Rx_COE_EN_;
	else
		read_buf &= ~Rx_COE_EN_;

	ret = smsc95xx_write_reg(udev, COE_CR, read_buf);
	if (ret < 0)
		return ret;

	debug("COE_CR = 0x%08x\n", read_buf);
	return 0;
}

static void smsc95xx_set_multicast(struct smsc95xx_private *priv)
{
	/* No multicast in u-boot */
	priv->mac_cr &= ~(MAC_CR_PRMS_ | MAC_CR_MCPAS_ | MAC_CR_HPFILT_);
}

/* starts the TX path */
static void smsc95xx_start_tx_path(struct usb_device *udev,
				   struct smsc95xx_private *priv)
{
	u32 reg_val;

	/* Enable Tx at MAC */
	priv->mac_cr |= MAC_CR_TXEN_;

	smsc95xx_write_reg(udev, MAC_CR, priv->mac_cr);

	/* Enable Tx at SCSRs */
	reg_val = TX_CFG_ON_;
	smsc95xx_write_reg(udev, TX_CFG, reg_val);
}

/* Starts the Receive path */
static void smsc95xx_start_rx_path(struct usb_device *udev,
				   struct smsc95xx_private *priv)
{
	priv->mac_cr |= MAC_CR_RXEN_;
	smsc95xx_write_reg(udev, MAC_CR, priv->mac_cr);
}

static int smsc95xx_init_common(struct usb_device *udev, struct ueth_data *dev,
				struct smsc95xx_private *priv,
				unsigned char *enetaddr)
{
	int ret;
	u32 write_buf;
	u32 read_buf;
	u32 burst_cap;
	int timeout;
#define TIMEOUT_RESOLUTION 50	/* ms */
	int link_detected;

	debug("** %s()\n", __func__);
	dev->phy_id = SMSC95XX_INTERNAL_PHY_ID; /* fixed phy id */

	write_buf = HW_CFG_LRST_;
	ret = smsc95xx_write_reg(udev, HW_CFG, write_buf);
	if (ret < 0)
		return ret;

	timeout = 0;
	do {
		ret = smsc95xx_read_reg(udev, HW_CFG, &read_buf);
		if (ret < 0)
			return ret;
		udelay(10 * 1000);
		timeout++;
	} while ((read_buf & HW_CFG_LRST_) && (timeout < 100));

	if (timeout >= 100) {
		debug("timeout waiting for completion of Lite Reset\n");
		return -ETIMEDOUT;
	}

	write_buf = PM_CTL_PHY_RST_;
	ret = smsc95xx_write_reg(udev, PM_CTRL, write_buf);
	if (ret < 0)
		return ret;

	timeout = 0;
	do {
		ret = smsc95xx_read_reg(udev, PM_CTRL, &read_buf);
		if (ret < 0)
			return ret;
		udelay(10 * 1000);
		timeout++;
	} while ((read_buf & PM_CTL_PHY_RST_) && (timeout < 100));
	if (timeout >= 100) {
		debug("timeout waiting for PHY Reset\n");
		return -ETIMEDOUT;
	}
#ifndef CONFIG_DM_ETH
	if (!priv->have_hwaddr && smsc95xx_init_mac_address(enetaddr, udev) ==
			0)
		priv->have_hwaddr = 1;
#endif
	if (!priv->have_hwaddr) {
		puts("Error: SMSC95xx: No MAC address set - set usbethaddr\n");
		return -EADDRNOTAVAIL;
	}
	ret = smsc95xx_write_hwaddr_common(udev, priv, enetaddr);
	if (ret < 0)
		return ret;

#ifdef TURBO_MODE
	if (dev->pusb_dev->speed == USB_SPEED_HIGH) {
		burst_cap = DEFAULT_HS_BURST_CAP_SIZE / HS_USB_PKT_SIZE;
		priv->rx_urb_size = DEFAULT_HS_BURST_CAP_SIZE;
	} else {
		burst_cap = DEFAULT_FS_BURST_CAP_SIZE / FS_USB_PKT_SIZE;
		priv->rx_urb_size = DEFAULT_FS_BURST_CAP_SIZE;
	}
#else
	burst_cap = 0;
	priv->rx_urb_size = MAX_SINGLE_PACKET_SIZE;
#endif
	debug("rx_urb_size=%ld\n", (ulong)priv->rx_urb_size);

	ret = smsc95xx_write_reg(udev, BURST_CAP, burst_cap);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(udev, BURST_CAP, &read_buf);
	if (ret < 0)
		return ret;
	debug("Read Value from BURST_CAP after writing: 0x%08x\n", read_buf);

	read_buf = DEFAULT_BULK_IN_DELAY;
	ret = smsc95xx_write_reg(udev, BULK_IN_DLY, read_buf);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(udev, BULK_IN_DLY, &read_buf);
	if (ret < 0)
		return ret;
	debug("Read Value from BULK_IN_DLY after writing: "
			"0x%08x\n", read_buf);

	ret = smsc95xx_read_reg(udev, HW_CFG, &read_buf);
	if (ret < 0)
		return ret;
	debug("Read Value from HW_CFG: 0x%08x\n", read_buf);

#ifdef TURBO_MODE
	read_buf |= (HW_CFG_MEF_ | HW_CFG_BCE_);
#endif
	read_buf &= ~HW_CFG_RXDOFF_;

#define NET_IP_ALIGN 0
	read_buf |= NET_IP_ALIGN << 9;

	ret = smsc95xx_write_reg(udev, HW_CFG, read_buf);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(udev, HW_CFG, &read_buf);
	if (ret < 0)
		return ret;
	debug("Read Value from HW_CFG after writing: 0x%08x\n", read_buf);

	write_buf = 0xFFFFFFFF;
	ret = smsc95xx_write_reg(udev, INT_STS, write_buf);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(udev, ID_REV, &read_buf);
	if (ret < 0)
		return ret;
	debug("ID_REV = 0x%08x\n", read_buf);

	/* Configure GPIO pins as LED outputs */
	write_buf = LED_GPIO_CFG_SPD_LED | LED_GPIO_CFG_LNK_LED |
		LED_GPIO_CFG_FDX_LED;
	ret = smsc95xx_write_reg(udev, LED_GPIO_CFG, write_buf);
	if (ret < 0)
		return ret;
	debug("LED_GPIO_CFG set\n");

	/* Init Tx */
	write_buf = 0;
	ret = smsc95xx_write_reg(udev, FLOW, write_buf);
	if (ret < 0)
		return ret;

	read_buf = AFC_CFG_DEFAULT;
	ret = smsc95xx_write_reg(udev, AFC_CFG, read_buf);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(udev, MAC_CR, &priv->mac_cr);
	if (ret < 0)
		return ret;

	/* Init Rx. Set Vlan */
	write_buf = (u32)ETH_P_8021Q;
	ret = smsc95xx_write_reg(udev, VLAN1, write_buf);
	if (ret < 0)
		return ret;

	/* Disable checksum offload engines */
	ret = smsc95xx_set_csums(udev, 0, 0);
	if (ret < 0) {
		debug("Failed to set csum offload: %d\n", ret);
		return ret;
	}
	smsc95xx_set_multicast(priv);

	ret = smsc95xx_phy_initialize(udev, dev);
	if (ret < 0)
		return ret;
	ret = smsc95xx_read_reg(udev, INT_EP_CTL, &read_buf);
	if (ret < 0)
		return ret;

	/* enable PHY interrupts */
	read_buf |= INT_EP_CTL_PHY_INT_;

	ret = smsc95xx_write_reg(udev, INT_EP_CTL, read_buf);
	if (ret < 0)
		return ret;

	smsc95xx_start_tx_path(udev, priv);
	smsc95xx_start_rx_path(udev, priv);

	timeout = 0;
	do {
		link_detected = smsc95xx_mdio_read(udev, dev->phy_id, MII_BMSR)
			& BMSR_LSTATUS;
		if (!link_detected) {
			if (timeout == 0)
				printf("Waiting for Ethernet connection... ");
			udelay(TIMEOUT_RESOLUTION * 1000);
			timeout += TIMEOUT_RESOLUTION;
		}
	} while (!link_detected && timeout < PHY_CONNECT_TIMEOUT);
	if (link_detected) {
		if (timeout != 0)
			printf("done.\n");
	} else {
		printf("unable to connect.\n");
		return -EIO;
	}
	return 0;
}

static int smsc95xx_send_common(struct ueth_data *dev, void *packet, int length)
{
	int err;
	int actual_len;
	u32 tx_cmd_a;
	u32 tx_cmd_b;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, msg,
				 PKTSIZE + sizeof(tx_cmd_a) + sizeof(tx_cmd_b));

	debug("** %s(), len %d, buf %#x\n", __func__, length,
	      (unsigned int)(ulong)msg);
	if (length > PKTSIZE)
		return -ENOSPC;

	tx_cmd_a = (u32)length | TX_CMD_A_FIRST_SEG_ | TX_CMD_A_LAST_SEG_;
	tx_cmd_b = (u32)length;
	cpu_to_le32s(&tx_cmd_a);
	cpu_to_le32s(&tx_cmd_b);

	/* prepend cmd_a and cmd_b */
	memcpy(msg, &tx_cmd_a, sizeof(tx_cmd_a));
	memcpy(msg + sizeof(tx_cmd_a), &tx_cmd_b, sizeof(tx_cmd_b));
	memcpy(msg + sizeof(tx_cmd_a) + sizeof(tx_cmd_b), (void *)packet,
	       length);
	err = usb_bulk_msg(dev->pusb_dev,
				usb_sndbulkpipe(dev->pusb_dev, dev->ep_out),
				(void *)msg,
				length + sizeof(tx_cmd_a) + sizeof(tx_cmd_b),
				&actual_len,
				USB_BULK_SEND_TIMEOUT);
	debug("Tx: len = %u, actual = %u, err = %d\n",
	      (unsigned int)(length + sizeof(tx_cmd_a) + sizeof(tx_cmd_b)),
	      (unsigned int)actual_len, err);

	return err;
}

#ifndef CONFIG_DM_ETH
/*
 * Smsc95xx callbacks
 */
static int smsc95xx_init(struct eth_device *eth, bd_t *bd)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	struct usb_device *udev = dev->pusb_dev;
	struct smsc95xx_private *priv =
		(struct smsc95xx_private *)dev->dev_priv;

	return smsc95xx_init_common(udev, dev, priv, eth->enetaddr);
}

static int smsc95xx_send(struct eth_device *eth, void *packet, int length)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;

	return smsc95xx_send_common(dev, packet, length);
}

static int smsc95xx_recv(struct eth_device *eth)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	DEFINE_CACHE_ALIGN_BUFFER(unsigned char, recv_buf, RX_URB_SIZE);
	unsigned char *buf_ptr;
	int err;
	int actual_len;
	u32 packet_len;
	int cur_buf_align;

	debug("** %s()\n", __func__);
	err = usb_bulk_msg(dev->pusb_dev,
			   usb_rcvbulkpipe(dev->pusb_dev, dev->ep_in),
			   (void *)recv_buf, RX_URB_SIZE, &actual_len,
			   USB_BULK_RECV_TIMEOUT);
	debug("Rx: len = %u, actual = %u, err = %d\n", RX_URB_SIZE,
	      actual_len, err);
	if (err != 0) {
		debug("Rx: failed to receive\n");
		return -err;
	}
	if (actual_len > RX_URB_SIZE) {
		debug("Rx: received too many bytes %d\n", actual_len);
		return -ENOSPC;
	}

	buf_ptr = recv_buf;
	while (actual_len > 0) {
		/*
		 * 1st 4 bytes contain the length of the actual data plus error
		 * info. Extract data length.
		 */
		if (actual_len < sizeof(packet_len)) {
			debug("Rx: incomplete packet length\n");
			return -EIO;
		}
		memcpy(&packet_len, buf_ptr, sizeof(packet_len));
		le32_to_cpus(&packet_len);
		if (packet_len & RX_STS_ES_) {
			debug("Rx: Error header=%#x", packet_len);
			return -EIO;
		}
		packet_len = ((packet_len & RX_STS_FL_) >> 16);

		if (packet_len > actual_len - sizeof(packet_len)) {
			debug("Rx: too large packet: %d\n", packet_len);
			return -EIO;
		}

		/* Notify net stack */
		net_process_received_packet(buf_ptr + sizeof(packet_len),
					    packet_len - 4);

		/* Adjust for next iteration */
		actual_len -= sizeof(packet_len) + packet_len;
		buf_ptr += sizeof(packet_len) + packet_len;
		cur_buf_align = (ulong)buf_ptr - (ulong)recv_buf;

		if (cur_buf_align & 0x03) {
			int align = 4 - (cur_buf_align & 0x03);

			actual_len -= align;
			buf_ptr += align;
		}
	}
	return err;
}

static void smsc95xx_halt(struct eth_device *eth)
{
	debug("** %s()\n", __func__);
}

static int smsc95xx_write_hwaddr(struct eth_device *eth)
{
	struct ueth_data *dev = eth->priv;
	struct usb_device *udev = dev->pusb_dev;
	struct smsc95xx_private *priv = dev->dev_priv;

	return smsc95xx_write_hwaddr_common(udev, priv, eth->enetaddr);
}

/*
 * SMSC probing functions
 */
void smsc95xx_eth_before_probe(void)
{
	curr_eth_dev = 0;
}

struct smsc95xx_dongle {
	unsigned short vendor;
	unsigned short product;
};

static const struct smsc95xx_dongle smsc95xx_dongles[] = {
	{ 0x0424, 0xec00 },	/* LAN9512/LAN9514 Ethernet */
	{ 0x0424, 0x9500 },	/* LAN9500 Ethernet */
	{ 0x0424, 0x9730 },	/* LAN9730 Ethernet (HSIC) */
	{ 0x0424, 0x9900 },	/* SMSC9500 USB Ethernet Device (SAL10) */
	{ 0x0424, 0x9e00 },	/* LAN9500A Ethernet */
	{ 0x0000, 0x0000 }	/* END - Do not remove */
};

/* Probe to see if a new device is actually an SMSC device */
int smsc95xx_eth_probe(struct usb_device *dev, unsigned int ifnum,
		      struct ueth_data *ss)
{
	struct usb_interface *iface;
	struct usb_interface_descriptor *iface_desc;
	int i;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];
	iface_desc = &dev->config.if_desc[ifnum].desc;

	for (i = 0; smsc95xx_dongles[i].vendor != 0; i++) {
		if (dev->descriptor.idVendor == smsc95xx_dongles[i].vendor &&
		    dev->descriptor.idProduct == smsc95xx_dongles[i].product)
			/* Found a supported dongle */
			break;
	}
	if (smsc95xx_dongles[i].vendor == 0)
		return 0;

	/* At this point, we know we've got a live one */
	debug("\n\nUSB Ethernet device detected\n");
	memset(ss, '\0', sizeof(struct ueth_data));

	/* Initialize the ueth_data structure with some useful info */
	ss->ifnum = ifnum;
	ss->pusb_dev = dev;
	ss->subclass = iface_desc->bInterfaceSubClass;
	ss->protocol = iface_desc->bInterfaceProtocol;

	/*
	 * We are expecting a minimum of 3 endpoints - in, out (bulk), and int.
	 * We will ignore any others.
	 */
	for (i = 0; i < iface_desc->bNumEndpoints; i++) {
		/* is it an BULK endpoint? */
		if ((iface->ep_desc[i].bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
			if (iface->ep_desc[i].bEndpointAddress & USB_DIR_IN)
				ss->ep_in =
					iface->ep_desc[i].bEndpointAddress &
					USB_ENDPOINT_NUMBER_MASK;
			else
				ss->ep_out =
					iface->ep_desc[i].bEndpointAddress &
					USB_ENDPOINT_NUMBER_MASK;
		}

		/* is it an interrupt endpoint? */
		if ((iface->ep_desc[i].bmAttributes &
		    USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = iface->ep_desc[i].bEndpointAddress &
				USB_ENDPOINT_NUMBER_MASK;
			ss->irqinterval = iface->ep_desc[i].bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
		  ss->ep_in, ss->ep_out, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	if (usb_set_interface(dev, iface_desc->bInterfaceNumber, 0) ||
	    !ss->ep_in || !ss->ep_out || !ss->ep_int) {
		debug("Problems with device\n");
		return 0;
	}
	dev->privptr = (void *)ss;

	/* alloc driver private */
	ss->dev_priv = calloc(1, sizeof(struct smsc95xx_private));
	if (!ss->dev_priv)
		return 0;

	return 1;
}

int smsc95xx_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
				struct eth_device *eth)
{
	debug("** %s()\n", __func__);
	if (!eth) {
		debug("%s: missing parameter.\n", __func__);
		return 0;
	}
	sprintf(eth->name, "%s%d", SMSC95XX_BASE_NAME, curr_eth_dev++);
	eth->init = smsc95xx_init;
	eth->send = smsc95xx_send;
	eth->recv = smsc95xx_recv;
	eth->halt = smsc95xx_halt;
	eth->write_hwaddr = smsc95xx_write_hwaddr;
	eth->priv = ss;
	return 1;
}
#endif /* !CONFIG_DM_ETH */

#ifdef CONFIG_DM_ETH
static int smsc95xx_eth_start(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);
	struct smsc95xx_private *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);

	/* Driver-model Ethernet ensures we have this */
	priv->have_hwaddr = 1;

	return smsc95xx_init_common(udev, &priv->ueth, priv, pdata->enetaddr);
}

void smsc95xx_eth_stop(struct udevice *dev)
{
	debug("** %s()\n", __func__);
}

int smsc95xx_eth_send(struct udevice *dev, void *packet, int length)
{
	struct smsc95xx_private *priv = dev_get_priv(dev);

	return smsc95xx_send_common(&priv->ueth, packet, length);
}

int smsc95xx_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct smsc95xx_private *priv = dev_get_priv(dev);
	struct ueth_data *ueth = &priv->ueth;
	uint8_t *ptr;
	int ret, len;
	u32 packet_len;

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
	memcpy(&packet_len, ptr, sizeof(packet_len));
	le32_to_cpus(&packet_len);
	if (packet_len & RX_STS_ES_) {
		debug("Rx: Error header=%#x", packet_len);
		goto err;
	}
	packet_len = ((packet_len & RX_STS_FL_) >> 16);

	if (packet_len > len - sizeof(packet_len)) {
		debug("Rx: too large packet: %d\n", packet_len);
		goto err;
	}

	*packetp = ptr + sizeof(packet_len);
	return packet_len - 4;

err:
	usb_ether_advance_rxbuf(ueth, -1);
	return -EINVAL;
}

static int smsc95xx_free_pkt(struct udevice *dev, uchar *packet, int packet_len)
{
	struct smsc95xx_private *priv = dev_get_priv(dev);

	packet_len = ALIGN(packet_len + sizeof(u32), 4);
	usb_ether_advance_rxbuf(&priv->ueth, sizeof(u32) + packet_len);

	return 0;
}

int smsc95xx_write_hwaddr(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct smsc95xx_private *priv = dev_get_priv(dev);

	return smsc95xx_write_hwaddr_common(udev, priv, pdata->enetaddr);
}

int smsc95xx_read_rom_hwaddr(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	int ret;

	ret = smsc95xx_init_mac_address(pdata->enetaddr, udev);
	if (ret)
		memset(pdata->enetaddr, 0, 6);

	return 0;
}

static int smsc95xx_eth_probe(struct udevice *dev)
{
	struct smsc95xx_private *priv = dev_get_priv(dev);
	struct ueth_data *ueth = &priv->ueth;

	return usb_ether_register(dev, ueth, RX_URB_SIZE);
}

static const struct eth_ops smsc95xx_eth_ops = {
	.start	= smsc95xx_eth_start,
	.send	= smsc95xx_eth_send,
	.recv	= smsc95xx_eth_recv,
	.free_pkt = smsc95xx_free_pkt,
	.stop	= smsc95xx_eth_stop,
	.write_hwaddr = smsc95xx_write_hwaddr,
	.read_rom_hwaddr = smsc95xx_read_rom_hwaddr,
};

U_BOOT_DRIVER(smsc95xx_eth) = {
	.name	= "smsc95xx_eth",
	.id	= UCLASS_ETH,
	.probe = smsc95xx_eth_probe,
	.ops	= &smsc95xx_eth_ops,
	.priv_auto_alloc_size = sizeof(struct smsc95xx_private),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};

static const struct usb_device_id smsc95xx_eth_id_table[] = {
	{ USB_DEVICE(0x05ac, 0x1402) },
	{ USB_DEVICE(0x0424, 0xec00) },	/* LAN9512/LAN9514 Ethernet */
	{ USB_DEVICE(0x0424, 0x9500) },	/* LAN9500 Ethernet */
	{ USB_DEVICE(0x0424, 0x9730) },	/* LAN9730 Ethernet (HSIC) */
	{ USB_DEVICE(0x0424, 0x9900) },	/* SMSC9500 USB Ethernet (SAL10) */
	{ USB_DEVICE(0x0424, 0x9e00) },	/* LAN9500A Ethernet */
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(smsc95xx_eth, smsc95xx_eth_id_table);
#endif
