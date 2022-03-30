// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Rene Griessl <rgriessl@cit-ec.uni-bielefeld.de>
 * based on the U-Boot Asix driver as well as information
 * from the Linux AX88179_178a driver
 */

#include <common.h>
#include <dm.h>
#include <usb.h>
#include <net.h>
#include <linux/mii.h>
#include "usb_ether.h"
#include <malloc.h>
#include <memalign.h>
#include <errno.h>

/* ASIX AX88179 based USB 3.0 Ethernet Devices */
#define AX88179_PHY_ID				0x03
#define AX_EEPROM_LEN				0x100
#define AX88179_EEPROM_MAGIC			0x17900b95
#define AX_MCAST_FLTSIZE			8
#define AX_MAX_MCAST				64
#define AX_INT_PPLS_LINK			(1 << 16)
#define AX_RXHDR_L4_TYPE_MASK			0x1c
#define AX_RXHDR_L4_TYPE_UDP			4
#define AX_RXHDR_L4_TYPE_TCP			16
#define AX_RXHDR_L3CSUM_ERR			2
#define AX_RXHDR_L4CSUM_ERR			1
#define AX_RXHDR_CRC_ERR			(1 << 29)
#define AX_RXHDR_DROP_ERR			(1 << 31)
#define AX_ENDPOINT_INT				0x01
#define AX_ENDPOINT_IN				0x02
#define AX_ENDPOINT_OUT				0x03
#define AX_ACCESS_MAC				0x01
#define AX_ACCESS_PHY				0x02
#define AX_ACCESS_EEPROM			0x04
#define AX_ACCESS_EFUS				0x05
#define AX_PAUSE_WATERLVL_HIGH			0x54
#define AX_PAUSE_WATERLVL_LOW			0x55

#define PHYSICAL_LINK_STATUS			0x02
	#define	AX_USB_SS		(1 << 2)
	#define	AX_USB_HS		(1 << 1)

#define GENERAL_STATUS				0x03
	#define	AX_SECLD		(1 << 2)

#define AX_SROM_ADDR				0x07
#define AX_SROM_CMD				0x0a
	#define EEP_RD			(1 << 2)
	#define EEP_BUSY		(1 << 4)

#define AX_SROM_DATA_LOW			0x08
#define AX_SROM_DATA_HIGH			0x09

#define AX_RX_CTL				0x0b
	#define AX_RX_CTL_DROPCRCERR	(1 << 8)
	#define AX_RX_CTL_IPE		(1 << 9)
	#define AX_RX_CTL_START		(1 << 7)
	#define AX_RX_CTL_AP		(1 << 5)
	#define AX_RX_CTL_AM		(1 << 4)
	#define AX_RX_CTL_AB		(1 << 3)
	#define AX_RX_CTL_AMALL		(1 << 1)
	#define AX_RX_CTL_PRO		(1 << 0)
	#define AX_RX_CTL_STOP		0

#define AX_NODE_ID				0x10
#define AX_MULFLTARY				0x16

#define AX_MEDIUM_STATUS_MODE			0x22
	#define AX_MEDIUM_GIGAMODE	(1 << 0)
	#define AX_MEDIUM_FULL_DUPLEX	(1 << 1)
	#define AX_MEDIUM_EN_125MHZ	(1 << 3)
	#define AX_MEDIUM_RXFLOW_CTRLEN	(1 << 4)
	#define AX_MEDIUM_TXFLOW_CTRLEN	(1 << 5)
	#define AX_MEDIUM_RECEIVE_EN	(1 << 8)
	#define AX_MEDIUM_PS		(1 << 9)
	#define AX_MEDIUM_JUMBO_EN	0x8040

#define AX_MONITOR_MOD				0x24
	#define AX_MONITOR_MODE_RWLC	(1 << 1)
	#define AX_MONITOR_MODE_RWMP	(1 << 2)
	#define AX_MONITOR_MODE_PMEPOL	(1 << 5)
	#define AX_MONITOR_MODE_PMETYPE	(1 << 6)

#define AX_GPIO_CTRL				0x25
	#define AX_GPIO_CTRL_GPIO3EN	(1 << 7)
	#define AX_GPIO_CTRL_GPIO2EN	(1 << 6)
	#define AX_GPIO_CTRL_GPIO1EN	(1 << 5)

#define AX_PHYPWR_RSTCTL			0x26
	#define AX_PHYPWR_RSTCTL_BZ	(1 << 4)
	#define AX_PHYPWR_RSTCTL_IPRL	(1 << 5)
	#define AX_PHYPWR_RSTCTL_AT	(1 << 12)

#define AX_RX_BULKIN_QCTRL			0x2e
#define AX_CLK_SELECT				0x33
	#define AX_CLK_SELECT_BCS	(1 << 0)
	#define AX_CLK_SELECT_ACS	(1 << 1)
	#define AX_CLK_SELECT_ULR	(1 << 3)

#define AX_RXCOE_CTL				0x34
	#define AX_RXCOE_IP		(1 << 0)
	#define AX_RXCOE_TCP		(1 << 1)
	#define AX_RXCOE_UDP		(1 << 2)
	#define AX_RXCOE_TCPV6		(1 << 5)
	#define AX_RXCOE_UDPV6		(1 << 6)

#define AX_TXCOE_CTL				0x35
	#define AX_TXCOE_IP		(1 << 0)
	#define AX_TXCOE_TCP		(1 << 1)
	#define AX_TXCOE_UDP		(1 << 2)
	#define AX_TXCOE_TCPV6		(1 << 5)
	#define AX_TXCOE_UDPV6		(1 << 6)

#define AX_LEDCTRL				0x73

#define GMII_PHY_PHYSR				0x11
	#define GMII_PHY_PHYSR_SMASK	0xc000
	#define GMII_PHY_PHYSR_GIGA	(1 << 15)
	#define GMII_PHY_PHYSR_100	(1 << 14)
	#define GMII_PHY_PHYSR_FULL	(1 << 13)
	#define GMII_PHY_PHYSR_LINK	(1 << 10)

#define GMII_LED_ACT				0x1a
	#define	GMII_LED_ACTIVE_MASK	0xff8f
	#define	GMII_LED0_ACTIVE	(1 << 4)
	#define	GMII_LED1_ACTIVE	(1 << 5)
	#define	GMII_LED2_ACTIVE	(1 << 6)

#define GMII_LED_LINK				0x1c
	#define	GMII_LED_LINK_MASK	0xf888
	#define	GMII_LED0_LINK_10	(1 << 0)
	#define	GMII_LED0_LINK_100	(1 << 1)
	#define	GMII_LED0_LINK_1000	(1 << 2)
	#define	GMII_LED1_LINK_10	(1 << 4)
	#define	GMII_LED1_LINK_100	(1 << 5)
	#define	GMII_LED1_LINK_1000	(1 << 6)
	#define	GMII_LED2_LINK_10	(1 << 8)
	#define	GMII_LED2_LINK_100	(1 << 9)
	#define	GMII_LED2_LINK_1000	(1 << 10)
	#define	LED0_ACTIVE		(1 << 0)
	#define	LED0_LINK_10		(1 << 1)
	#define	LED0_LINK_100		(1 << 2)
	#define	LED0_LINK_1000		(1 << 3)
	#define	LED0_FD			(1 << 4)
	#define	LED0_USB3_MASK		0x001f
	#define	LED1_ACTIVE		(1 << 5)
	#define	LED1_LINK_10		(1 << 6)
	#define	LED1_LINK_100		(1 << 7)
	#define	LED1_LINK_1000		(1 << 8)
	#define	LED1_FD			(1 << 9)
	#define	LED1_USB3_MASK		0x03e0
	#define	LED2_ACTIVE		(1 << 10)
	#define	LED2_LINK_1000		(1 << 13)
	#define	LED2_LINK_100		(1 << 12)
	#define	LED2_LINK_10		(1 << 11)
	#define	LED2_FD			(1 << 14)
	#define	LED_VALID		(1 << 15)
	#define	LED2_USB3_MASK		0x7c00

#define GMII_PHYPAGE				0x1e
#define GMII_PHY_PAGE_SELECT			0x1f
	#define GMII_PHY_PGSEL_EXT	0x0007
	#define GMII_PHY_PGSEL_PAGE0	0x0000

/* local defines */
#define ASIX_BASE_NAME "axg"
#define USB_CTRL_SET_TIMEOUT 5000
#define USB_CTRL_GET_TIMEOUT 5000
#define USB_BULK_SEND_TIMEOUT 5000
#define USB_BULK_RECV_TIMEOUT 5000

#define AX_RX_URB_SIZE 1024 * 0x12
#define BLK_FRAME_SIZE 0x200
#define PHY_CONNECT_TIMEOUT 5000

#define TIMEOUT_RESOLUTION 50	/* ms */

#define FLAG_NONE			0
#define FLAG_TYPE_AX88179	(1U << 0)
#define FLAG_TYPE_AX88178a	(1U << 1)
#define FLAG_TYPE_DLINK_DUB1312	(1U << 2)
#define FLAG_TYPE_SITECOM	(1U << 3)
#define FLAG_TYPE_SAMSUNG	(1U << 4)
#define FLAG_TYPE_LENOVO	(1U << 5)
#define FLAG_TYPE_GX3		(1U << 6)

/* local vars */
static const struct {
	unsigned char ctrl, timer_l, timer_h, size, ifg;
} AX88179_BULKIN_SIZE[] =	{
	{7, 0x4f, 0,	0x02, 0xff},
	{7, 0x20, 3,	0x03, 0xff},
	{7, 0xae, 7,	0x04, 0xff},
	{7, 0xcc, 0x4c, 0x04, 8},
};

#ifndef CONFIG_DM_ETH
static int curr_eth_dev; /* index for name of next device detected */
#endif

/* driver private */
struct asix_private {
#ifdef CONFIG_DM_ETH
	struct ueth_data ueth;
	unsigned pkt_cnt;
	uint8_t *pkt_data;
	uint32_t *pkt_hdr;
#endif
	int flags;
	int rx_urb_size;
	int maxpacketsize;
};

/*
 * Asix infrastructure commands
 */
static int asix_write_cmd(struct ueth_data *dev, u8 cmd, u16 value, u16 index,
			     u16 size, void *data)
{
	int len;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buf, size);

	debug("asix_write_cmd() cmd=0x%02x value=0x%04x index=0x%04x size=%d\n",
	      cmd, value, index, size);

	memcpy(buf, data, size);

	len = usb_control_msg(
		dev->pusb_dev,
		usb_sndctrlpipe(dev->pusb_dev, 0),
		cmd,
		USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		value,
		index,
		buf,
		size,
		USB_CTRL_SET_TIMEOUT);

	return len == size ? 0 : ECOMM;
}

static int asix_read_cmd(struct ueth_data *dev, u8 cmd, u16 value, u16 index,
			    u16 size, void *data)
{
	int len;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buf, size);

	debug("asix_read_cmd() cmd=0x%02x value=0x%04x index=0x%04x size=%d\n",
	      cmd, value, index, size);

	len = usb_control_msg(
		dev->pusb_dev,
		usb_rcvctrlpipe(dev->pusb_dev, 0),
		cmd,
		USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		value,
		index,
		buf,
		size,
		USB_CTRL_GET_TIMEOUT);

	memcpy(data, buf, size);

	return len == size ? 0 : ECOMM;
}

static int asix_read_mac(struct ueth_data *dev, uint8_t *enetaddr)
{
	int ret;

	ret = asix_read_cmd(dev, AX_ACCESS_MAC, AX_NODE_ID, 6, 6, enetaddr);
	if (ret < 0)
		debug("Failed to read MAC address: %02x\n", ret);

	return ret;
}

static int asix_write_mac(struct ueth_data *dev, uint8_t *enetaddr)
{
	int ret;

	ret = asix_write_cmd(dev, AX_ACCESS_MAC, AX_NODE_ID, ETH_ALEN,
				 ETH_ALEN, enetaddr);
	if (ret < 0)
		debug("Failed to set MAC address: %02x\n", ret);

	return ret;
}

static int asix_basic_reset(struct ueth_data *dev,
			struct asix_private *dev_priv)
{
	u8 buf[5];
	u16 *tmp16;
	u8 *tmp;

	tmp16 = (u16 *)buf;
	tmp = (u8 *)buf;

	/* Power up ethernet PHY */
	*tmp16 = 0;
	asix_write_cmd(dev, AX_ACCESS_MAC, AX_PHYPWR_RSTCTL, 2, 2, tmp16);

	*tmp16 = AX_PHYPWR_RSTCTL_IPRL;
	asix_write_cmd(dev, AX_ACCESS_MAC, AX_PHYPWR_RSTCTL, 2, 2, tmp16);
	mdelay(200);

	*tmp = AX_CLK_SELECT_ACS | AX_CLK_SELECT_BCS;
	asix_write_cmd(dev, AX_ACCESS_MAC, AX_CLK_SELECT, 1, 1, tmp);
	mdelay(200);

	/* RX bulk configuration */
	memcpy(tmp, &AX88179_BULKIN_SIZE[0], 5);
	asix_write_cmd(dev, AX_ACCESS_MAC, AX_RX_BULKIN_QCTRL, 5, 5, tmp);

	dev_priv->rx_urb_size = 128 * 20;

	/* Water Level configuration */
	*tmp = 0x34;
	asix_write_cmd(dev, AX_ACCESS_MAC, AX_PAUSE_WATERLVL_LOW, 1, 1, tmp);

	*tmp = 0x52;
	asix_write_cmd(dev, AX_ACCESS_MAC, AX_PAUSE_WATERLVL_HIGH, 1, 1, tmp);

	/* Enable checksum offload */
	*tmp = AX_RXCOE_IP | AX_RXCOE_TCP | AX_RXCOE_UDP |
	       AX_RXCOE_TCPV6 | AX_RXCOE_UDPV6;
	asix_write_cmd(dev, AX_ACCESS_MAC, AX_RXCOE_CTL, 1, 1, tmp);

	*tmp = AX_TXCOE_IP | AX_TXCOE_TCP | AX_TXCOE_UDP |
	       AX_TXCOE_TCPV6 | AX_TXCOE_UDPV6;
	asix_write_cmd(dev, AX_ACCESS_MAC, AX_TXCOE_CTL, 1, 1, tmp);

	/* Configure RX control register => start operation */
	*tmp16 = AX_RX_CTL_DROPCRCERR | AX_RX_CTL_IPE | AX_RX_CTL_START |
		 AX_RX_CTL_AP | AX_RX_CTL_AMALL | AX_RX_CTL_AB;
	asix_write_cmd(dev, AX_ACCESS_MAC, AX_RX_CTL, 2, 2, tmp16);

	*tmp = AX_MONITOR_MODE_PMETYPE | AX_MONITOR_MODE_PMEPOL |
	       AX_MONITOR_MODE_RWMP;
	asix_write_cmd(dev, AX_ACCESS_MAC, AX_MONITOR_MOD, 1, 1, tmp);

	/* Configure default medium type => giga */
	*tmp16 = AX_MEDIUM_RECEIVE_EN | AX_MEDIUM_TXFLOW_CTRLEN |
		 AX_MEDIUM_RXFLOW_CTRLEN | AX_MEDIUM_FULL_DUPLEX |
		 AX_MEDIUM_GIGAMODE | AX_MEDIUM_JUMBO_EN;
	asix_write_cmd(dev, AX_ACCESS_MAC, AX_MEDIUM_STATUS_MODE, 2, 2, tmp16);

	u16 adv = 0;
	adv = ADVERTISE_ALL | ADVERTISE_CSMA | ADVERTISE_LPACK |
	      ADVERTISE_NPAGE | ADVERTISE_PAUSE_ASYM | ADVERTISE_PAUSE_CAP;
	asix_write_cmd(dev, AX_ACCESS_PHY, 0x03, MII_ADVERTISE, 2, &adv);

	adv = ADVERTISE_1000FULL;
	asix_write_cmd(dev, AX_ACCESS_PHY, 0x03, MII_CTRL1000, 2, &adv);

	return 0;
}

static int asix_wait_link(struct ueth_data *dev)
{
	int timeout = 0;
	int link_detected;
	u8 buf[2];
	u16 *tmp16;

	tmp16 = (u16 *)buf;

	do {
		asix_read_cmd(dev, AX_ACCESS_PHY, AX88179_PHY_ID,
			      MII_BMSR, 2, buf);
		link_detected = *tmp16 & BMSR_LSTATUS;
		if (!link_detected) {
			if (timeout == 0)
				printf("Waiting for Ethernet connection... ");
			mdelay(TIMEOUT_RESOLUTION);
			timeout += TIMEOUT_RESOLUTION;
		}
	} while (!link_detected && timeout < PHY_CONNECT_TIMEOUT);

	if (link_detected) {
		if (timeout > 0)
			printf("done.\n");
		return 0;
	} else {
		printf("unable to connect.\n");
		return -ENETUNREACH;
	}
}

static int asix_init_common(struct ueth_data *dev,
			struct asix_private *dev_priv)
{
	u8 buf[2], tmp[5], link_sts;
	u16 *tmp16, mode;


	tmp16 = (u16 *)buf;

	debug("** %s()\n", __func__);

	/* Configure RX control register => start operation */
	*tmp16 = AX_RX_CTL_DROPCRCERR | AX_RX_CTL_IPE | AX_RX_CTL_START |
		 AX_RX_CTL_AP | AX_RX_CTL_AMALL | AX_RX_CTL_AB;
	if (asix_write_cmd(dev, AX_ACCESS_MAC, AX_RX_CTL, 2, 2, tmp16) != 0)
		goto out_err;

	if (asix_wait_link(dev) != 0) {
		/*reset device and try again*/
		printf("Reset Ethernet Device\n");
		asix_basic_reset(dev, dev_priv);
		if (asix_wait_link(dev) != 0)
			goto out_err;
	}

	/* Configure link */
	mode = AX_MEDIUM_RECEIVE_EN | AX_MEDIUM_TXFLOW_CTRLEN |
	       AX_MEDIUM_RXFLOW_CTRLEN;

	asix_read_cmd(dev, AX_ACCESS_MAC, PHYSICAL_LINK_STATUS,
		      1, 1, &link_sts);

	asix_read_cmd(dev, AX_ACCESS_PHY, AX88179_PHY_ID,
		      GMII_PHY_PHYSR, 2, tmp16);

	if (!(*tmp16 & GMII_PHY_PHYSR_LINK)) {
		return 0;
	} else if (GMII_PHY_PHYSR_GIGA == (*tmp16 & GMII_PHY_PHYSR_SMASK)) {
		mode |= AX_MEDIUM_GIGAMODE | AX_MEDIUM_EN_125MHZ |
			AX_MEDIUM_JUMBO_EN;

		if (link_sts & AX_USB_SS)
			memcpy(tmp, &AX88179_BULKIN_SIZE[0], 5);
		else if (link_sts & AX_USB_HS)
			memcpy(tmp, &AX88179_BULKIN_SIZE[1], 5);
		else
			memcpy(tmp, &AX88179_BULKIN_SIZE[3], 5);
	} else if (GMII_PHY_PHYSR_100 == (*tmp16 & GMII_PHY_PHYSR_SMASK)) {
		mode |= AX_MEDIUM_PS;

		if (link_sts & (AX_USB_SS | AX_USB_HS))
			memcpy(tmp, &AX88179_BULKIN_SIZE[2], 5);
		else
			memcpy(tmp, &AX88179_BULKIN_SIZE[3], 5);
	} else {
		memcpy(tmp, &AX88179_BULKIN_SIZE[3], 5);
	}

	/* RX bulk configuration */
	asix_write_cmd(dev, AX_ACCESS_MAC, AX_RX_BULKIN_QCTRL, 5, 5, tmp);

	dev_priv->rx_urb_size = (1024 * (tmp[3] + 2));
	if (*tmp16 & GMII_PHY_PHYSR_FULL)
		mode |= AX_MEDIUM_FULL_DUPLEX;
	asix_write_cmd(dev, AX_ACCESS_MAC, AX_MEDIUM_STATUS_MODE,
		       2, 2, &mode);

	return 0;
out_err:
	return -1;
}

static int asix_send_common(struct ueth_data *dev,
			struct asix_private *dev_priv,
			void *packet, int length)
{
	int err;
	u32 packet_len, tx_hdr2;
	int actual_len, framesize;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, msg,
				 PKTSIZE + (2 * sizeof(packet_len)));

	debug("** %s(), len %d\n", __func__, length);

	packet_len = length;
	cpu_to_le32s(&packet_len);

	memcpy(msg, &packet_len, sizeof(packet_len));
	framesize = dev_priv->maxpacketsize;
	tx_hdr2 = 0;
	if (((length + 8) % framesize) == 0)
		tx_hdr2 |= 0x80008000;	/* Enable padding */

	cpu_to_le32s(&tx_hdr2);

	memcpy(msg + sizeof(packet_len), &tx_hdr2, sizeof(tx_hdr2));

	memcpy(msg + sizeof(packet_len) + sizeof(tx_hdr2),
	       (void *)packet, length);

	err = usb_bulk_msg(dev->pusb_dev,
				usb_sndbulkpipe(dev->pusb_dev, dev->ep_out),
				(void *)msg,
				length + sizeof(packet_len) + sizeof(tx_hdr2),
				&actual_len,
				USB_BULK_SEND_TIMEOUT);
	debug("Tx: len = %zu, actual = %u, err = %d\n",
	      length + sizeof(packet_len), actual_len, err);

	return err;
}

#ifndef CONFIG_DM_ETH
/*
 * Asix callbacks
 */
static int asix_init(struct eth_device *eth, bd_t *bd)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	struct asix_private *dev_priv = (struct asix_private *)dev->dev_priv;

	return asix_init_common(dev, dev_priv);
}

static int asix_write_hwaddr(struct eth_device *eth)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;

	return asix_write_mac(dev, eth->enetaddr);
}

static int asix_send(struct eth_device *eth, void *packet, int length)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	struct asix_private *dev_priv = (struct asix_private *)dev->dev_priv;

	return asix_send_common(dev, dev_priv, packet, length);
}

static int asix_recv(struct eth_device *eth)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	struct asix_private *dev_priv = (struct asix_private *)dev->dev_priv;

	u16 frame_pos;
	int err;
	int actual_len;

	int pkt_cnt;
	u32 rx_hdr;
	u16 hdr_off;
	u32 *pkt_hdr;
	ALLOC_CACHE_ALIGN_BUFFER(u8, recv_buf, dev_priv->rx_urb_size);

	actual_len = -1;

	debug("** %s()\n", __func__);

	err = usb_bulk_msg(dev->pusb_dev,
				usb_rcvbulkpipe(dev->pusb_dev, dev->ep_in),
				(void *)recv_buf,
				dev_priv->rx_urb_size,
				&actual_len,
				USB_BULK_RECV_TIMEOUT);
	debug("Rx: len = %u, actual = %u, err = %d\n", dev_priv->rx_urb_size,
	      actual_len, err);

	if (err != 0) {
		debug("Rx: failed to receive\n");
		return -ECOMM;
	}
	if (actual_len > dev_priv->rx_urb_size) {
		debug("Rx: received too many bytes %d\n", actual_len);
		return -EMSGSIZE;
	}


	rx_hdr = *(u32 *)(recv_buf + actual_len - 4);
	le32_to_cpus(&rx_hdr);

	pkt_cnt = (u16)rx_hdr;
	hdr_off = (u16)(rx_hdr >> 16);
	pkt_hdr = (u32 *)(recv_buf + hdr_off);


	frame_pos = 0;

	while (pkt_cnt--) {
		u16 pkt_len;

		le32_to_cpus(pkt_hdr);
		pkt_len = (*pkt_hdr >> 16) & 0x1fff;

		frame_pos += 2;

		net_process_received_packet(recv_buf + frame_pos, pkt_len);

		pkt_hdr++;
		frame_pos += ((pkt_len + 7) & 0xFFF8)-2;

		if (pkt_cnt == 0)
			return 0;
	}
	return err;
}

static void asix_halt(struct eth_device *eth)
{
	debug("** %s()\n", __func__);
}

/*
 * Asix probing functions
 */
void ax88179_eth_before_probe(void)
{
	curr_eth_dev = 0;
}

struct asix_dongle {
	unsigned short vendor;
	unsigned short product;
	int flags;
};

static const struct asix_dongle asix_dongles[] = {
	{ 0x0b95, 0x1790, FLAG_TYPE_AX88179 },
	{ 0x0b95, 0x178a, FLAG_TYPE_AX88178a },
	{ 0x2001, 0x4a00, FLAG_TYPE_DLINK_DUB1312 },
	{ 0x0df6, 0x0072, FLAG_TYPE_SITECOM },
	{ 0x04e8, 0xa100, FLAG_TYPE_SAMSUNG },
	{ 0x17ef, 0x304b, FLAG_TYPE_LENOVO },
	{ 0x04b4, 0x3610, FLAG_TYPE_GX3 },
	{ 0x0000, 0x0000, FLAG_NONE }	/* END - Do not remove */
};

/* Probe to see if a new device is actually an asix device */
int ax88179_eth_probe(struct usb_device *dev, unsigned int ifnum,
		      struct ueth_data *ss)
{
	struct usb_interface *iface;
	struct usb_interface_descriptor *iface_desc;
	struct asix_private *dev_priv;
	int ep_in_found = 0, ep_out_found = 0;
	int i;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];
	iface_desc = &dev->config.if_desc[ifnum].desc;

	for (i = 0; asix_dongles[i].vendor != 0; i++) {
		if (dev->descriptor.idVendor == asix_dongles[i].vendor &&
		    dev->descriptor.idProduct == asix_dongles[i].product)
			/* Found a supported dongle */
			break;
	}

	if (asix_dongles[i].vendor == 0)
		return 0;

	memset(ss, 0, sizeof(struct ueth_data));

	/* At this point, we know we've got a live one */
	debug("\n\nUSB Ethernet device detected: %#04x:%#04x\n",
	      dev->descriptor.idVendor, dev->descriptor.idProduct);

	/* Initialize the ueth_data structure with some useful info */
	ss->ifnum = ifnum;
	ss->pusb_dev = dev;
	ss->subclass = iface_desc->bInterfaceSubClass;
	ss->protocol = iface_desc->bInterfaceProtocol;

	/* alloc driver private */
	ss->dev_priv = calloc(1, sizeof(struct asix_private));
	if (!ss->dev_priv)
		return 0;
	dev_priv = ss->dev_priv;
	dev_priv->flags = asix_dongles[i].flags;

	/*
	 * We are expecting a minimum of 3 endpoints - in, out (bulk), and
	 * int. We will ignore any others.
	 */
	for (i = 0; i < iface_desc->bNumEndpoints; i++) {
		/* is it an interrupt endpoint? */
		if ((iface->ep_desc[i].bmAttributes &
		    USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = iface->ep_desc[i].bEndpointAddress &
				USB_ENDPOINT_NUMBER_MASK;
			ss->irqinterval = iface->ep_desc[i].bInterval;
			continue;
		}

		/* is it an BULK endpoint? */
		if (!((iface->ep_desc[i].bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK))
			continue;

		u8 ep_addr = iface->ep_desc[i].bEndpointAddress;
		if ((ep_addr & USB_DIR_IN) && !ep_in_found) {
			ss->ep_in = ep_addr &
				USB_ENDPOINT_NUMBER_MASK;
			ep_in_found = 1;
		}
		if (!(ep_addr & USB_DIR_IN) && !ep_out_found) {
			ss->ep_out = ep_addr &
				USB_ENDPOINT_NUMBER_MASK;
			dev_priv->maxpacketsize =
				dev->epmaxpacketout[AX_ENDPOINT_OUT];
			ep_out_found = 1;
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
	return 1;
}

int ax88179_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
				struct eth_device *eth)
{
	struct asix_private *dev_priv = (struct asix_private *)ss->dev_priv;

	if (!eth) {
		debug("%s: missing parameter.\n", __func__);
		return 0;
	}
	sprintf(eth->name, "%s%d", ASIX_BASE_NAME, curr_eth_dev++);
	eth->init = asix_init;
	eth->send = asix_send;
	eth->recv = asix_recv;
	eth->halt = asix_halt;
	eth->write_hwaddr = asix_write_hwaddr;
	eth->priv = ss;

	if (asix_basic_reset(ss, dev_priv))
		return 0;

	/* Get the MAC address */
	if (asix_read_mac(ss, eth->enetaddr))
		return 0;
	debug("MAC %pM\n", eth->enetaddr);

	return 1;
}

#else /* !CONFIG_DM_ETH */

static int ax88179_eth_start(struct udevice *dev)
{
	struct asix_private *priv = dev_get_priv(dev);

	return asix_init_common(&priv->ueth, priv);
}

void ax88179_eth_stop(struct udevice *dev)
{
	struct asix_private *priv = dev_get_priv(dev);
	struct ueth_data *ueth = &priv->ueth;

	debug("** %s()\n", __func__);

	usb_ether_advance_rxbuf(ueth, -1);
	priv->pkt_cnt = 0;
	priv->pkt_data = NULL;
	priv->pkt_hdr = NULL;
}

int ax88179_eth_send(struct udevice *dev, void *packet, int length)
{
	struct asix_private *priv = dev_get_priv(dev);

	return asix_send_common(&priv->ueth, priv, packet, length);
}

int ax88179_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct asix_private *priv = dev_get_priv(dev);
	struct ueth_data *ueth = &priv->ueth;
	int ret, len;
	u16 pkt_len;

	/* No packet left, get a new one */
	if (priv->pkt_cnt == 0) {
		uint8_t *ptr;
		u16 pkt_cnt;
		u16 hdr_off;
		u32 rx_hdr;

		len = usb_ether_get_rx_bytes(ueth, &ptr);
		debug("%s: first try, len=%d\n", __func__, len);
		if (!len) {
			if (!(flags & ETH_RECV_CHECK_DEVICE))
				return -EAGAIN;

			ret = usb_ether_receive(ueth, priv->rx_urb_size);
			if (ret < 0)
				return ret;

			len = usb_ether_get_rx_bytes(ueth, &ptr);
			debug("%s: second try, len=%d\n", __func__, len);
		}

		if (len < 4) {
			usb_ether_advance_rxbuf(ueth, -1);
			return -EMSGSIZE;
		}

		rx_hdr = *(u32 *)(ptr + len - 4);
		le32_to_cpus(&rx_hdr);

		pkt_cnt = (u16)rx_hdr;
		if (pkt_cnt == 0) {
			usb_ether_advance_rxbuf(ueth, -1);
			return 0;
		}

		hdr_off = (u16)(rx_hdr >> 16);
		if (hdr_off > len - 4) {
			usb_ether_advance_rxbuf(ueth, -1);
			return -EIO;
		}

		priv->pkt_cnt = pkt_cnt;
		priv->pkt_data = ptr;
		priv->pkt_hdr = (u32 *)(ptr + hdr_off);
		debug("%s: %d packets received, pkt header at %d\n",
		      __func__, (int)priv->pkt_cnt, (int)hdr_off);
	}

	le32_to_cpus(priv->pkt_hdr);
	pkt_len = (*priv->pkt_hdr >> 16) & 0x1fff;

	*packetp = priv->pkt_data + 2;

	priv->pkt_data += (pkt_len + 7) & 0xFFF8;
	priv->pkt_cnt--;
	priv->pkt_hdr++;

	debug("%s: return packet of %d bytes (%d packets left)\n",
	      __func__, (int)pkt_len, priv->pkt_cnt);
	return pkt_len;
}

static int ax88179_free_pkt(struct udevice *dev, uchar *packet, int packet_len)
{
	struct asix_private *priv = dev_get_priv(dev);
	struct ueth_data *ueth = &priv->ueth;

	if (priv->pkt_cnt == 0)
		usb_ether_advance_rxbuf(ueth, -1);

	return 0;
}

int ax88179_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct asix_private *priv = dev_get_priv(dev);
	struct ueth_data *ueth = &priv->ueth;

	return asix_write_mac(ueth, pdata->enetaddr);
}

static int ax88179_eth_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct asix_private *priv = dev_get_priv(dev);
	struct usb_device *usb_dev;
	int ret;

	priv->flags = dev->driver_data;
	ret = usb_ether_register(dev, &priv->ueth, AX_RX_URB_SIZE);
	if (ret)
		return ret;

	usb_dev = priv->ueth.pusb_dev;
	priv->maxpacketsize = usb_dev->epmaxpacketout[AX_ENDPOINT_OUT];

	/* Get the MAC address */
	ret = asix_read_mac(&priv->ueth, pdata->enetaddr);
	if (ret)
		return ret;
	debug("MAC %pM\n", pdata->enetaddr);

	return 0;
}

static const struct eth_ops ax88179_eth_ops = {
	.start = ax88179_eth_start,
	.send = ax88179_eth_send,
	.recv = ax88179_eth_recv,
	.free_pkt = ax88179_free_pkt,
	.stop = ax88179_eth_stop,
	.write_hwaddr = ax88179_write_hwaddr,
};

U_BOOT_DRIVER(ax88179_eth) = {
	.name = "ax88179_eth",
	.id = UCLASS_ETH,
	.probe = ax88179_eth_probe,
	.ops = &ax88179_eth_ops,
	.priv_auto_alloc_size = sizeof(struct asix_private),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};

static const struct usb_device_id ax88179_eth_id_table[] = {
	{ USB_DEVICE(0x0b95, 0x1790), .driver_info = FLAG_TYPE_AX88179 },
	{ USB_DEVICE(0x0b95, 0x178a), .driver_info = FLAG_TYPE_AX88178a },
	{ USB_DEVICE(0x2001, 0x4a00), .driver_info = FLAG_TYPE_DLINK_DUB1312 },
	{ USB_DEVICE(0x0df6, 0x0072), .driver_info = FLAG_TYPE_SITECOM },
	{ USB_DEVICE(0x04e8, 0xa100), .driver_info = FLAG_TYPE_SAMSUNG },
	{ USB_DEVICE(0x17ef, 0x304b), .driver_info = FLAG_TYPE_LENOVO },
	{ USB_DEVICE(0x04b4, 0x3610), .driver_info = FLAG_TYPE_GX3 },
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(ax88179_eth, ax88179_eth_id_table);
#endif /* !CONFIG_DM_ETH */
