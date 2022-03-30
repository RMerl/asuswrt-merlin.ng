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
#include <linux/mii.h>
#include <malloc.h>
#include <memalign.h>
#include <usb.h>

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
#ifdef CONFIG_DM_ETH
	uint8_t rx_buf[MCS7830_RX_URB_SIZE];
	struct ueth_data ueth;
#endif
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

#ifndef CONFIG_DM_ETH
/*
 * mcs7830_init() - network interface's init callback
 * @udev:	network device to initialize
 * @bd:		board information
 * Return: zero upon success, negative upon error
 *
 * after initial setup during probe() and get_info(), this init() callback
 * ensures that the link is up and subsequent send() and recv() calls can
 * exchange ethernet frames
 */
static int mcs7830_init(struct eth_device *eth, bd_t *bd)
{
	struct ueth_data *dev = eth->priv;

	return mcs7830_init_common(dev->pusb_dev);
}

/*
 * mcs7830_send() - network interface's send callback
 * @eth:	network device to send the frame from
 * @packet:	ethernet frame content
 * @length:	ethernet frame length
 * Return: zero upon success, negative upon error
 *
 * this routine send an ethernet frame out of the network interface
 */
static int mcs7830_send(struct eth_device *eth, void *packet, int length)
{
	struct ueth_data *dev = eth->priv;

	return mcs7830_send_common(dev, packet, length);
}

/*
 * mcs7830_recv() - network interface's recv callback
 * @eth:	network device to receive frames from
 * Return: zero upon success, negative upon error
 *
 * this routine checks for available ethernet frames that the network
 * interface might have received, and notifies the network stack
 */
static int mcs7830_recv(struct eth_device *eth)
{
	ALLOC_CACHE_ALIGN_BUFFER(uint8_t, buf, MCS7830_RX_URB_SIZE);
	struct ueth_data *ueth = eth->priv;
	int len;

	len = mcs7830_recv_common(ueth, buf);
	if (len >= 0) {
		net_process_received_packet(buf, len);
		return 0;
	}

	return len;
}

/*
 * mcs7830_halt() - network interface's halt callback
 * @eth:	network device to cease operation of
 * Return: none
 *
 * this routine is supposed to undo the effect of previous initialization and
 * ethernet frames exchange; in this implementation it's a NOP
 */
static void mcs7830_halt(struct eth_device *eth)
{
	debug("%s()\n", __func__);
}

/*
 * mcs7830_write_mac() - write an ethernet adapter's MAC address
 * @eth:	network device to write to
 * Return: zero upon success, negative upon error
 *
 * this routine takes the MAC address from the ethernet interface's data
 * structure, and writes it into the ethernet adapter such that subsequent
 * exchange of ethernet frames uses this address
 */
static int mcs7830_write_mac(struct eth_device *eth)
{
	struct ueth_data *ueth = eth->priv;

	return mcs7830_write_mac_common(ueth->pusb_dev, eth->enetaddr);
}

/*
 * mcs7830_iface_idx - index of detected network interfaces
 *
 * this counter keeps track of identified supported interfaces,
 * to assign unique names as more interfaces are found
 */
static int mcs7830_iface_idx;

/*
 * mcs7830_eth_before_probe() - network driver's before_probe callback
 * Return: none
 *
 * this routine initializes driver's internal data in preparation of
 * subsequent probe callbacks
 */
void mcs7830_eth_before_probe(void)
{
	mcs7830_iface_idx = 0;
}

/*
 * struct mcs7830_dongle - description of a supported Moschip ethernet dongle
 * @vendor:	16bit USB vendor identification
 * @product:	16bit USB product identification
 *
 * this structure describes a supported USB ethernet dongle by means of the
 * vendor and product codes found during USB enumeration; no flags are held
 * here since all supported dongles have identical behaviour, and required
 * fixups get determined at runtime, such that no manual configuration is
 * needed
 */
struct mcs7830_dongle {
	uint16_t vendor;
	uint16_t product;
};

/*
 * mcs7830_dongles - the list of supported Moschip based USB ethernet dongles
 */
static const struct mcs7830_dongle mcs7830_dongles[] = {
	{ 0x9710, 0x7832, },	/* Moschip 7832 */
	{ 0x9710, 0x7830, },	/* Moschip 7830 */
	{ 0x9710, 0x7730, },	/* Moschip 7730 */
	{ 0x0df6, 0x0021, },	/* Sitecom LN 30 */
};

/*
 * mcs7830_eth_probe() - network driver's probe callback
 * @dev:	detected USB device to check
 * @ifnum:	detected USB interface to check
 * @ss:		USB ethernet data structure to fill in upon match
 * Return: #1 upon match, #0 upon mismatch or error
 *
 * this routine checks whether the found USB device is supported by
 * this ethernet driver, and upon match fills in the USB ethernet
 * data structure which later is passed to the get_info callback
 */
int mcs7830_eth_probe(struct usb_device *dev, unsigned int ifnum,
		      struct ueth_data *ss)
{
	struct usb_interface *iface;
	struct usb_interface_descriptor *iface_desc;
	int i;
	struct mcs7830_private *priv;
	int ep_in_found, ep_out_found, ep_intr_found;

	debug("%s()\n", __func__);

	/* iterate the list of supported dongles */
	iface = &dev->config.if_desc[ifnum];
	iface_desc = &iface->desc;
	for (i = 0; i < ARRAY_SIZE(mcs7830_dongles); i++) {
		if (dev->descriptor.idVendor == mcs7830_dongles[i].vendor &&
		    dev->descriptor.idProduct == mcs7830_dongles[i].product)
			break;
	}
	if (i == ARRAY_SIZE(mcs7830_dongles))
		return 0;
	debug("detected USB ethernet device: %04X:%04X\n",
	      dev->descriptor.idVendor, dev->descriptor.idProduct);

	/* fill in driver private data */
	priv = calloc(1, sizeof(*priv));
	if (!priv)
		return 0;

	/* fill in the ueth_data structure, attach private data */
	memset(ss, 0, sizeof(*ss));
	ss->ifnum = ifnum;
	ss->pusb_dev = dev;
	ss->subclass = iface_desc->bInterfaceSubClass;
	ss->protocol = iface_desc->bInterfaceProtocol;
	ss->dev_priv = priv;

	/*
	 * a minimum of three endpoints is expected: in (bulk),
	 * out (bulk), and interrupt; ignore all others
	 */
	ep_in_found = ep_out_found = ep_intr_found = 0;
	for (i = 0; i < iface_desc->bNumEndpoints; i++) {
		uint8_t eptype, epaddr;
		bool is_input;

		eptype = iface->ep_desc[i].bmAttributes;
		eptype &= USB_ENDPOINT_XFERTYPE_MASK;

		epaddr = iface->ep_desc[i].bEndpointAddress;
		is_input = epaddr & USB_DIR_IN;
		epaddr &= USB_ENDPOINT_NUMBER_MASK;

		if (eptype == USB_ENDPOINT_XFER_BULK) {
			if (is_input && !ep_in_found) {
				ss->ep_in = epaddr;
				ep_in_found++;
			}
			if (!is_input && !ep_out_found) {
				ss->ep_out = epaddr;
				ep_out_found++;
			}
		}

		if (eptype == USB_ENDPOINT_XFER_INT) {
			if (is_input && !ep_intr_found) {
				ss->ep_int = epaddr;
				ss->irqinterval = iface->ep_desc[i].bInterval;
				ep_intr_found++;
			}
		}
	}
	debug("endpoints: in %d, out %d, intr %d\n",
	      ss->ep_in, ss->ep_out, ss->ep_int);

	/* apply basic sanity checks */
	if (usb_set_interface(dev, iface_desc->bInterfaceNumber, 0) ||
	    !ss->ep_in || !ss->ep_out || !ss->ep_int) {
		debug("device probe incomplete\n");
		return 0;
	}

	dev->privptr = ss;
	return 1;
}

/*
 * mcs7830_eth_get_info() - network driver's get_info callback
 * @dev:	detected USB device
 * @ss:		USB ethernet data structure filled in at probe()
 * @eth:	ethernet interface data structure to fill in
 * Return: #1 upon success, #0 upon error
 *
 * this routine registers the mandatory init(), send(), recv(), and
 * halt() callbacks with the ethernet interface, can register the
 * optional write_hwaddr() callback with the ethernet interface,
 * and initiates configuration of the interface such that subsequent
 * calls to those callbacks results in network communication
 */
int mcs7830_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
			 struct eth_device *eth)
{
	debug("%s()\n", __func__);
	if (!eth) {
		debug("%s: missing parameter.\n", __func__);
		return 0;
	}

	snprintf(eth->name, sizeof(eth->name), "%s%d",
		 MCS7830_BASE_NAME, mcs7830_iface_idx++);
	eth->init = mcs7830_init;
	eth->send = mcs7830_send;
	eth->recv = mcs7830_recv;
	eth->halt = mcs7830_halt;
	eth->write_hwaddr = mcs7830_write_mac;
	eth->priv = ss;

	if (mcs7830_basic_reset(ss->pusb_dev, ss->dev_priv))
		return 0;

	if (mcs7830_read_mac(ss->pusb_dev, eth->enetaddr))
		return 0;
	debug("MAC %pM\n", eth->enetaddr);

	return 1;
}
#endif


#ifdef CONFIG_DM_ETH
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
	struct eth_pdata *pdata = dev_get_platdata(dev);

	return mcs7830_write_mac_common(udev, pdata->enetaddr);
}

static int mcs7830_eth_probe(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);
	struct mcs7830_private *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
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
	.priv_auto_alloc_size = sizeof(struct mcs7830_private),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
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
#endif
