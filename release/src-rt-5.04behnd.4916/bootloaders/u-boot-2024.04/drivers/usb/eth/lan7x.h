/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Microchip Technology Inc. All rights reserved.
 */

#include <console.h>
#include <time.h>
#include <watchdog.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/errno.h>

/* USB Vendor Requests */
#define USB_VENDOR_REQUEST_WRITE_REGISTER	0xA0
#define USB_VENDOR_REQUEST_READ_REGISTER	0xA1
#define USB_VENDOR_REQUEST_GET_STATS		0xA2

/* Tx Command A */
#define TX_CMD_A_FCS			BIT(22)
#define TX_CMD_A_LEN_MASK		0x000FFFFF

/* Rx Command A */
#define RX_CMD_A_RXE			BIT(18)
#define RX_CMD_A_LEN_MASK		0x00003FFF

/* SCSRs */
#define ID_REV				0x00
#define ID_REV_CHIP_ID_MASK		0xFFFF0000
#define ID_REV_CHIP_ID_7500		0x7500
#define ID_REV_CHIP_ID_7800		0x7800
#define ID_REV_CHIP_ID_7850		0x7850

#define INT_STS				0x0C

#define HW_CFG				0x010
#define HW_CFG_LRST			BIT(1)

#define PMT_CTL				0x014
#define PMT_CTL_PHY_PWRUP		BIT(10)
#define PMT_CTL_READY			BIT(7)
#define PMT_CTL_PHY_RST			BIT(4)

#define E2P_CMD				0x040
#define E2P_CMD_EPC_BUSY		BIT(31)
#define E2P_CMD_EPC_CMD_READ		0x00000000
#define E2P_CMD_EPC_TIMEOUT		BIT(10)
#define E2P_CMD_EPC_ADDR_MASK		0x000001FF

#define E2P_DATA			0x044

#define RFE_CTL_BCAST_EN		BIT(10)
#define RFE_CTL_DA_PERFECT		BIT(1)

#define FCT_RX_CTL_EN			BIT(31)

#define FCT_TX_CTL_EN			BIT(31)

#define MAC_CR				0x100
#define MAC_CR_ADP			BIT(13)
#define MAC_CR_AUTO_DUPLEX		BIT(12)
#define MAC_CR_AUTO_SPEED		BIT(11)

#define MAC_RX				0x104
#define MAC_RX_FCS_STRIP		BIT(4)
#define MAC_RX_RXEN			BIT(0)

#define MAC_TX				0x108
#define MAC_TX_TXEN			BIT(0)

#define FLOW				0x10C
#define FLOW_CR_TX_FCEN			BIT(30)
#define FLOW_CR_RX_FCEN			BIT(29)

#define RX_ADDRH			0x118
#define RX_ADDRL			0x11C

#define MII_ACC				0x120
#define MII_ACC_MII_READ		0x00000000
#define MII_ACC_MII_WRITE		0x00000002
#define MII_ACC_MII_BUSY		BIT(0)

#define MII_DATA			0x124

#define SS_USB_PKT_SIZE			1024
#define HS_USB_PKT_SIZE			512
#define FS_USB_PKT_SIZE			64

#define MAX_RX_FIFO_SIZE		(12 * 1024)
#define MAX_TX_FIFO_SIZE		(12 * 1024)
#define DEFAULT_BULK_IN_DELAY		0x0800

#define EEPROM_INDICATOR		0xA5
#define EEPROM_MAC_OFFSET		0x01

/* Some extra defines */
#define LAN7X_INTERNAL_PHY_ID		1

#define LAN7X_MAC_RX_MAX_SIZE(mtu) \
	((mtu) << 16)			/* Max frame size */
#define LAN7X_MAC_RX_MAX_SIZE_DEFAULT \
	LAN7X_MAC_RX_MAX_SIZE(PKTSIZE_ALIGN + 4 /* VLAN */ + 4 /* CRC */)

/* Timeouts */
#define USB_CTRL_SET_TIMEOUT_MS		5000
#define USB_CTRL_GET_TIMEOUT_MS		5000
#define USB_BULK_SEND_TIMEOUT_MS	5000
#define USB_BULK_RECV_TIMEOUT_MS	5000
#define TIMEOUT_RESOLUTION_MS		50
#define PHY_CONNECT_TIMEOUT_MS		5000

#define RX_URB_SIZE	2048

/* driver private */
struct lan7x_private {
	struct ueth_data ueth;
	u32 chipid;		/* Chip or device ID */
	struct mii_dev *mdiobus;
	struct phy_device *phydev;
};

/*
 * Lan7x infrastructure commands
 */

int lan7x_write_reg(struct usb_device *udev, u32 index, u32 data);

int lan7x_read_reg(struct usb_device *udev, u32 index, u32 *data);

/*
 * FIXME: Code should not be in header files. Nive this to a file common to
 * the two drivers.
 */
static inline int lan7x_wait_for_bit(struct usb_device *udev,
				     const char *prefix, const u32 reg,
				     const u32 mask, const bool set,
				     const unsigned int timeout_ms,
				     const bool breakable)
{
	u32 val;
	unsigned long start = get_timer(0);

	while (1) {
		lan7x_read_reg(udev, reg, &val);

		if (!set)
			val = ~val;

		if ((val & mask) == mask)
			return 0;

		if (get_timer(start) > timeout_ms)
			break;

		if (breakable && ctrlc()) {
			puts("Abort\n");
			return -EINTR;
		}

		udelay(1);
		schedule();
	}

	debug("%s: Timeout (reg=0x%x mask=%08x wait_set=%i)\n", prefix, reg,
	      mask, set);

	return -ETIMEDOUT;
}

int lan7x_mdio_read(struct usb_device *udev, int phy_id, int idx);

void lan7x_mdio_write(struct usb_device *udev, int phy_id, int idx,
		      int regval);

static inline int lan7x_mdio_wait_for_bit(struct usb_device *udev,
					  const char *prefix,
					  int phy_id, const u32 reg,
					  const u32 mask, const bool set,
					  const unsigned int timeout_ms,
					  const bool breakable)
{
	u32 val;
	unsigned long start = get_timer(0);

	while (1) {
		val = lan7x_mdio_read(udev, phy_id, reg);

		if (!set)
			val = ~val;

		if ((val & mask) == mask)
			return 0;

		if (get_timer(start) > timeout_ms)
			break;

		if (breakable && ctrlc()) {
			puts("Abort\n");
			return -EINTR;
		}

		udelay(1);
		schedule();
	}

	debug("%s: Timeout (reg=0x%x mask=%08x wait_set=%i)\n", prefix, reg,
	      mask, set);

	return -ETIMEDOUT;
}

int lan7x_phylib_register(struct udevice *udev);

int lan7x_eth_phylib_connect(struct udevice *udev, struct ueth_data *dev);

int lan7x_eth_phylib_config_start(struct udevice *udev);

int lan7x_pmt_phy_reset(struct usb_device *udev,
			struct ueth_data *dev);

int lan7x_update_flowcontrol(struct usb_device *udev,
			     struct ueth_data *dev,
			     uint32_t *flow, uint32_t *fct_flow);

int lan7x_read_eeprom_mac(unsigned char *enetaddr, struct usb_device *udev);

int lan7x_basic_reset(struct usb_device *udev,
		      struct ueth_data *dev);

void lan7x_eth_stop(struct udevice *dev);

int lan7x_eth_send(struct udevice *dev, void *packet, int length);

int lan7x_eth_recv(struct udevice *dev, int flags, uchar **packetp);

int lan7x_free_pkt(struct udevice *dev, uchar *packet, int packet_len);

int lan7x_eth_remove(struct udevice *dev);
