// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Gerhard Sittig <gsi@denx.de>
 * based on the U-Boot Asix driver as well as information
 * from the Linux Moschip driver
 */

/*
 * MOSCHIP MCS7830 based (7730/7830/7832) USB 2.0 Ethernet Devices
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <net.h>
#include <linux/delay.h>
#include <linux/mii.h>
#include <malloc.h>
#include <memalign.h>
#include <usb.h>
#include <linux/printk.h>

#include "usb_ether.h"

#define MCS7830_BASE_NAME	"mcs"

#define USBCALL_TIMEOUT		1000
#define LINKSTATUS_TIMEOUT	5000	/* link status, connect timeout */
#define LINKSTATUS_TIMEOUT_RES	50	/* link status, resolution in msec */

#define MCS7830_RX_URB_SIZE	2048

/* command opcodes */
#define MCS7830_WR_BREQ		0x0d
#define MCS7830_RD_BREQ		0x0e

/* register layout, numerical offset specs for USB API calls */
struct mcs7830_regs {
	uint8_t multicast_hashes[8];
	uint8_t packet_gap[2];
	uint8_t phy_data[2];
	uint8_t phy_command[2];
	uint8_t configuration;
	uint8_t ether_address[6];
	uint8_t frame_drop_count;
	uint8_t pause_threshold;
};
#define REG_MULTICAST_HASH	offsetof(struct mcs7830_regs, multicast_hashes)
#define REG_PHY_DATA		offsetof(struct mcs7830_regs, phy_data)
#define REG_PHY_CMD		offsetof(struct mcs7830_regs, phy_command)
#define REG_CONFIG		offsetof(struct mcs7830_regs, configuration)
#define REG_ETHER_ADDR		offsetof(struct mcs7830_regs, ether_address)
#define REG_FRAME_DROP_COUNTER	offsetof(struct mcs7830_regs, frame_drop_count)
#define REG_PAUSE_THRESHOLD	offsetof(struct mcs7830_regs, pause_threshold)

/* bit masks and default values for the above registers */
#define PHY_CMD1_READ		0x40
#define PHY_CMD1_WRITE		0x20
#define PHY_CMD1_PHYADDR	0x01

#define PHY_CMD2_PEND		0x80
#define PHY_CMD2_READY		0x40

#define CONF_CFG		0x80
#define CONF_SPEED100		0x40
#define CONF_FDX_ENABLE		0x20
#define CONF_RXENABLE		0x10
#define CONF_TXENABLE		0x08
#define CONF_SLEEPMODE		0x04
#define CONF_ALLMULTICAST	0x02
#define CONF_PROMISCUOUS	0x01

#define PAUSE_THRESHOLD_DEFAULT	0

/* bit masks for the status byte which follows received ethernet frames */
#define STAT_RX_FRAME_CORRECT	0x20
#define STAT_RX_LARGE_FRAME	0x10
#define STAT_RX_CRC_ERROR	0x08
#define STAT_RX_ALIGNMENT_ERROR	0x04
#define STAT_RX_LENGTH_ERROR	0x02
#define STAT_RX_SHORT_FRAME	0x01

/*
 * struct mcs7830_private - private driver data for an individual adapter
 * @config:	shadow for the network adapter's configuration register
 * @mchash:	shadow for the network adapter's multicast hash registers
 */
struct mcs7830_private {
	uint8_t rx_buf[MCS7830_RX_URB_SIZE];
	struct ueth_data ueth;
	uint8_t config;
	uint8_t mchash[8];
};

/*
 * mcs7830_read_reg() - read a register of the network adapter
 * @udev:	network device to read from
 * @idx:	index of the register to start reading from
 * @size:	number of bytes to read
 * @data:	buffer to read into
 * Return: zero upon success, negative upon error
 */
static int mcs7830_read_reg(struct usb_device *udev, uint8_t idx,
			    uint16_t size, void *data)
{
	int len;
	ALLOC_CACHE_ALIGN_BUFFER(uint8_t, buf, size);

	debug("%s() idx=0x%04X sz=%d\n", __func__, idx, size);

	len = usb_control_msg(udev,
			      usb_rcvctrlpipe(udev, 0),
			      MCS7830_RD_BREQ,
			      USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			      0, idx, buf, size,
			      USBCALL_TIMEOUT);
	if (len != size) {
		debug("%s() len=%d != sz=%d\n", __func__, len, size);
		return -EIO;
	}
	memcpy(data, buf, size);
	return 0;
}

/*
 * mcs7830_write_reg() - write a register of the network adapter
 * @udev:	network device to write to
 * @idx:	index of the register to start writing to
 * @size:	number of bytes to write
 * @data:	buffer holding the data to write
 * Return: zero upon success, negative upon error
 */
static int mcs7830_write_reg(struct usb_device *udev, uint8_t idx,
			     uint16_t size, void *data)
{
	int len;
	ALLOC_CACHE_ALIGN_BUFFER(uint8_t, buf, size);

	debug("%s() idx=0x%04X sz=%d\n", __func__, idx, size);

	memcpy(buf, data, size);
	len = usb_control_msg(udev,
			      usb_sndctrlpipe(udev, 0),
			      MCS7830_WR_BREQ,
			      USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			      0, idx, buf, size,
			      USBCALL_TIMEOUT);
	if (len != size) {
		debug("%s() len=%d != sz=%d\n", __func__, len, size);
		return -EIO;
	}
	return 0;
}

/*
 * mcs7830_phy_emit_wait() - emit PHY read/write access, wait for its execution
 * @udev:	network device to talk to
 * @rwflag:	PHY_CMD1_READ or PHY_CMD1_WRITE opcode
 * @index:	number of the PHY register to read or write
 * Return: zero upon success, negative upon error
 */
static int mcs7830_phy_emit_wait(struct usb_device *udev,
				 uint8_t rwflag, uint8_t index)
{
	int rc;
	int retry;
	uint8_t cmd[2];

	/* send the PHY read/write request */
	cmd[0] = rwflag | PHY_CMD1_PHYADDR;
	cmd[1] = PHY_CMD2_PEND | (index & 0x1f);
	rc = mcs7830_write_reg(udev, REG_PHY_CMD, sizeof(cmd), cmd);
	if (rc < 0)
		return rc;

	/* wait for the response to become available (usually < 1ms) */
	retry = 10;
	do {
		rc = mcs7830_read_reg(udev, REG_PHY_CMD, sizeof(cmd), cmd);
		if (rc < 0)
			return rc;
		if (cmd[1] & PHY_CMD2_READY)
			return 0;
		if (!retry--)
			return -ETIMEDOUT;
		mdelay(1);
	} while (1);
	/* UNREACH */
}

/*
 * mcs7830_read_phy() - read a PHY register of the network adapter
 * @udev:	network device to read from
 * @index:	index of the PHY register to read from
 * Return: non-negative 16bit register content, negative upon error
 */
static int mcs7830_read_phy(struct usb_device *udev, uint8_t index)
{
	int rc;
	uint16_t val;

	/* issue the PHY read request and wait for its execution */
	rc = mcs7830_phy_emit_wait(udev, PHY_CMD1_READ, index);
	if (rc < 0)
		return rc;

	/* fetch the PHY data which was read */
	rc = mcs7830_read_reg(udev, REG_PHY_DATA, sizeof(val), &val);
	if (rc < 0)
		return rc;
	rc = le16_to_cpu(val);
	debug("%s(%d) => 0x%04X\n", __func__, index, rc);
	return rc;
}

/*
 * mcs7830_write_phy() - write a PHY register of the network adapter
 * @udev:	network device to write to
 * @index:	index of the PHY register to write to
 * @val:	value to write to the PHY register
 * Return: zero upon success, negative upon error
 */
static int mcs7830_write_phy(struct usb_device *udev, uint8_t index,
			     uint16_t val)
{
	int rc;

	debug("%s(%d, 0x%04X)\n", __func__, index, val);

	/* setup the PHY data which is to get written */
	val = cpu_to_le16(val);
	rc = mcs7830_write_reg(udev, REG_PHY_DATA, sizeof(val), &val);
	if (rc < 0)
		return rc;

	/* issue the PHY write request and wait for its execution */
	rc = mcs7830_phy_emit_wait(udev, PHY_CMD1_WRITE, index);
	if (rc < 0)
		return rc;

	return 0;
}

/*
 * mcs7830_write_config() - write to the network adapter's config register
 * @udev:	network device to write to
 * @priv:	private data
 * Return: zero upon success, negative upon error
 *
 * the data which gets written is taken from the shadow config register
 * within the device driver's private data
 */
static int mcs7830_write_config(struct usb_device *udev,
				struct mcs7830_private *priv)
{
	int rc;

	debug("%s()\n", __func__);

	rc = mcs7830_write_reg(udev, REG_CONFIG,
			       sizeof(priv->config), &priv->config);
	if (rc < 0) {
		debug("writing config to adapter failed\n");
		return rc;
	}

	return 0;
}

/*
 * mcs7830_write_mchash() - write the network adapter's multicast filter
 * @udev:	network device to write to
 * @priv:	private data
 * Return: zero upon success, negative upon error
 *
 * the data which gets written is taken from the shadow multicast hashes
 * within the device driver's private data
 */
static int mcs7830_write_mchash(struct usb_device *udev,
				struct mcs7830_private *priv)
{
	int rc;

	debug("%s()\n", __func__);

	rc = mcs7830_write_reg(udev, REG_MULTICAST_HASH,
			       sizeof(priv->mchash), &priv->mchash);
	if (rc < 0) {
		debug("writing multicast hash to adapter failed\n");
		return rc;
	}

	return 0;
}

/*
 * mcs7830_set_autoneg() - setup and trigger ethernet link autonegotiation
 * @udev:	network device to run link negotiation on
 * Return: zero upon success, negative upon error
 *
 * the routine advertises available media and starts autonegotiation
 */
static int mcs7830_set_autoneg(struct usb_device *udev)
{
	int adv, flg;
	int rc;

	debug("%s()\n", __func__);

	/*
	 * algorithm taken from the Linux driver, which took it from
	 * "the original mcs7830 version 1.4 driver":
	 *
	 * enable all media, reset BMCR, enable auto neg, restart
	 * auto neg while keeping the enable auto neg flag set
	 */

	adv = ADVERTISE_PAUSE_CAP | ADVERTISE_ALL | ADVERTISE_CSMA;
	rc = mcs7830_write_phy(udev, MII_ADVERTISE, adv);

	flg = 0;
	if (!rc)
		rc = mcs7830_write_phy(udev, MII_BMCR, flg);

	flg |= BMCR_ANENABLE;
	if (!rc)
		rc = mcs7830_write_phy(udev, MII_BMCR, flg);

	flg |= BMCR_ANRESTART;
	if (!rc)
		rc = mcs7830_write_phy(udev, MII_BMCR, flg);

	return rc;
}

/*
 * mcs7830_get_rev() - identify a network adapter's chip revision
 * @udev:	network device to identify
 * Return: non-negative number, reflecting the revision number
 *
 * currently, only "rev C and higher" and "below rev C" are needed, so
 * the return value is #1 for "below rev C", and #2 for "rev C and above"
 */
static int mcs7830_get_rev(struct usb_device *udev)
{
	uint8_t buf[2];
	int rc;
	int rev;

	/* register 22 is readable in rev C and higher */
	rc = mcs7830_read_reg(udev, REG_FRAME_DROP_COUNTER, sizeof(buf), buf);
	if (rc < 0)
		rev = 1;
	else
		rev = 2;
	debug("%s() rc=%d, rev=%d\n", __func__, rc, rev);
	return rev;
}

/*
 * mcs7830_apply_fixup() - identify an adapter and potentially apply fixups
 * @udev:	network device to identify and apply fixups to
 * Return: zero upon success (no errors emitted from here)
 *
 * this routine identifies the network adapter's chip revision, and applies
 * fixups for known issues
 */
static int mcs7830_apply_fixup(struct usb_device *udev)
{
	int rev;
	int i;
	uint8_t thr;

	rev = mcs7830_get_rev(udev);
	debug("%s() rev=%d\n", __func__, rev);

	/*
	 * rev C requires setting the pause threshold (the Linux driver
	 * is inconsistent, the implementation does it for "rev C
	 * exactly", the introductory comment says "rev C and above")
	 */
	if (rev == 2) {
		debug("%s: applying rev C fixup\n", __func__);
		thr = PAUSE_THRESHOLD_DEFAULT;
		for (i = 0; i < 2; i++) {
			(void)mcs7830_write_reg(udev, REG_PAUSE_THRESHOLD,
						sizeof(thr), &thr);
			mdelay(1);
		}
	}

	return 0;
}

/*
 * mcs7830_basic_reset() - bring the network adapter into a known first state
 * @eth:	network device to act upon
 * Return: zero upon success, negative upon error
 *
 * this routine initializes the network adapter such that subsequent invocations
 * of the interface callbacks can exchange ethernet frames; link negotiation is
 * triggered from here already and continues in background
 */
static int mcs7830_basic_reset(struct usb_device *udev,
			       struct mcs7830_private *priv)
{
	int rc;

	debug("%s()\n", __func__);

	/*
	 * comment from the respective Linux driver, which
	 * unconditionally sets the ALLMULTICAST flag as well:
	 * should not be needed, but does not work otherwise
	 */
	priv->config = CONF_TXENABLE;
	priv->config |= CONF_ALLMULTICAST;

	rc = mcs7830_set_autoneg(udev);
	if (rc < 0) {
		pr_err("setting autoneg failed\n");
		return rc;
	}

	rc = mcs7830_write_mchash(udev, priv);
	if (rc < 0) {
		pr_err("failed to set multicast hash\n");
		return rc;
	}

	rc = mcs7830_write_config(udev, priv);
	if (rc < 0) {
		pr_err("failed to set configuration\n");
		return rc;
	}

	rc = mcs7830_apply_fixup(udev);
	if (rc < 0) {
		pr_err("fixup application failed\n");
		return rc;
	}

	return 0;
}

/*
 * mcs7830_read_mac() - read an ethernet adapter's MAC address
 * @udev:	network device to read from
 * @enetaddr:	place to put ethernet MAC address
 * Return: zero upon success, negative upon error
 *
 * this routine fetches the MAC address stored within the ethernet adapter,
 * and stores it in the ethernet interface's data structure
 */
static int mcs7830_read_mac(struct usb_device *udev, unsigned char enetaddr[])
{
	int rc;
	uint8_t buf[ETH_ALEN];

	debug("%s()\n", __func__);

	rc = mcs7830_read_reg(udev, REG_ETHER_ADDR, ETH_ALEN, buf);
	if (rc < 0) {
		debug("reading MAC from adapter failed\n");
		return rc;
	}

	memcpy(enetaddr, buf, ETH_ALEN);
	return 0;
}

static int mcs7830_write_mac_common(struct usb_device *udev,
				    unsigned char enetaddr[])
{
	int rc;

	debug("%s()\n", __func__);

	rc = mcs7830_write_reg(udev, REG_ETHER_ADDR, ETH_ALEN, enetaddr);
	if (rc < 0) {
		debug("writing MAC to adapter failed\n");
		return rc;
	}
	return 0;
}

static int mcs7830_init_common(struct usb_device *udev)
{
	int timeout;
	int have_link;

	debug("%s()\n", __func__);

	timeout = 0;
	do {
		have_link = mcs7830_read_phy(udev, MII_BMSR) & BMSR_LSTATUS;
		if (have_link)
			break;
		udelay(LINKSTATUS_TIMEOUT_RES * 1000);
		timeout += LINKSTATUS_TIMEOUT_RES;
	} while (timeout < LINKSTATUS_TIMEOUT);
	if (!have_link) {
		debug("ethernet link is down\n");
		return -ETIMEDOUT;
	}
	return 0;
}

static int mcs7830_send_common(struct ueth_data *ueth, void *packet,
			       int length)
{
	struct usb_device *udev = ueth->pusb_dev;
	int rc;
	int gotlen;
	/* there is a status byte after the ethernet frame */
	ALLOC_CACHE_ALIGN_BUFFER(uint8_t, buf, PKTSIZE + sizeof(uint8_t));

	memcpy(buf, packet, length);
	rc = usb_bulk_msg(udev,
			  usb_sndbulkpipe(udev, ueth->ep_out),
			  &buf[0], length, &gotlen,
			  USBCALL_TIMEOUT);
	debug("%s() TX want len %d, got len %d, rc %d\n",
	      __func__, length, gotlen, rc);
	return rc;
}

static int mcs7830_recv_common(struct ueth_data *ueth, uint8_t *buf)
{
	int rc, wantlen, gotlen;
	uint8_t sts;

	debug("%s()\n", __func__);

	/* fetch input data from the adapter */
	wantlen = MCS7830_RX_URB_SIZE;
	rc = usb_bulk_msg(ueth->pusb_dev,
			  usb_rcvbulkpipe(ueth->pusb_dev, ueth->ep_in),
			  &buf[0], wantlen, &gotlen,
			  USBCALL_TIMEOUT);
	debug("%s() RX want len %d, got len %d, rc %d\n",
	      __func__, wantlen, gotlen, rc);
	if (rc != 0) {
		pr_err("RX: failed to receive\n");
		return rc;
	}
	if (gotlen > wantlen) {
		pr_err("RX: got too many bytes (%d)\n", gotlen);
		return -EIO;
	}

	/*
	 * the bulk message that we received from USB contains exactly
	 * one ethernet frame and a trailing status byte
	 */
	if (gotlen < sizeof(sts))
		return -EIO;
	gotlen -= sizeof(sts);
	sts = buf[gotlen];

	if (sts == STAT_RX_FRAME_CORRECT) {
		debug("%s() got a frame, len=%d\n", __func__, gotlen);
		return gotlen;
	}

	debug("RX: frame error (sts 0x%02X, %s %s %s %s %s)\n",
	      sts,
	      (sts & STAT_RX_LARGE_FRAME) ? "large" : "-",
	      (sts & STAT_RX_LENGTH_ERROR) ?  "length" : "-",
	      (sts & STAT_RX_SHORT_FRAME) ? "short" : "-",
	      (sts & STAT_RX_CRC_ERROR) ? "crc" : "-",
	      (sts & STAT_RX_ALIGNMENT_ERROR) ?  "align" : "-");
	return -EIO;
}

static int mcs7830_eth_start(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);

	return mcs7830_init_common(udev);
}

void mcs7830_eth_stop(struct udevice *dev)
{
	debug("** %s()\n", __func__);
}

int mcs7830_eth_send(struct udevice *dev, void *packet, int length)
{
	struct mcs7830_private *priv = dev_get_priv(dev);
	struct ueth_data *ueth = &priv->ueth;

	return mcs7830_send_common(ueth, packet, length);
}

int mcs7830_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct mcs7830_private *priv = dev_get_priv(dev);
	struct ueth_data *ueth = &priv->ueth;
	int len;

	len = mcs7830_recv_common(ueth, priv->rx_buf);
	*packetp = priv->rx_buf;

	return len;
}

static int mcs7830_free_pkt(struct udevice *dev, uchar *packet, int packet_len)
{
	struct mcs7830_private *priv = dev_get_priv(dev);

	packet_len = ALIGN(packet_len, 4);
	usb_ether_advance_rxbuf(&priv->ueth, sizeof(u32) + packet_len);

	return 0;
}

int mcs7830_write_hwaddr(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);

	return mcs7830_write_mac_common(udev, pdata->enetaddr);
}

static int mcs7830_eth_probe(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);
	struct mcs7830_private *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct ueth_data *ueth = &priv->ueth;

	if (mcs7830_basic_reset(udev, priv))
		return 0;

	if (mcs7830_read_mac(udev, pdata->enetaddr))
		return 0;

	return usb_ether_register(dev, ueth, MCS7830_RX_URB_SIZE);
}

static const struct eth_ops mcs7830_eth_ops = {
	.start	= mcs7830_eth_start,
	.send	= mcs7830_eth_send,
	.recv	= mcs7830_eth_recv,
	.free_pkt = mcs7830_free_pkt,
	.stop	= mcs7830_eth_stop,
	.write_hwaddr = mcs7830_write_hwaddr,
};

U_BOOT_DRIVER(mcs7830_eth) = {
	.name	= "mcs7830_eth",
	.id	= UCLASS_ETH,
	.probe = mcs7830_eth_probe,
	.ops	= &mcs7830_eth_ops,
	.priv_auto	= sizeof(struct mcs7830_private),
	.plat_auto	= sizeof(struct eth_pdata),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

static const struct usb_device_id mcs7830_eth_id_table[] = {
	{ USB_DEVICE(0x9710, 0x7832) },		/* Moschip 7832 */
	{ USB_DEVICE(0x9710, 0x7830), },	/* Moschip 7830 */
	{ USB_DEVICE(0x9710, 0x7730), },	/* Moschip 7730 */
	{ USB_DEVICE(0x0df6, 0x0021), },	/* Sitecom LN 30 */
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(mcs7830_eth, mcs7830_eth_id_table);
