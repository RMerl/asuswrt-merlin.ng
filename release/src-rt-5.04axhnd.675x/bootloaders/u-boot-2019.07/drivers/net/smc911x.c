// SPDX-License-Identifier: GPL-2.0+
/*
 * SMSC LAN9[12]1[567] Network driver
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <net.h>
#include <miiphy.h>

#include "smc911x.h"

u32 pkt_data_pull(struct eth_device *dev, u32 addr) \
	__attribute__ ((weak, alias ("smc911x_reg_read")));
void pkt_data_push(struct eth_device *dev, u32 addr, u32 val) \
	__attribute__ ((weak, alias ("smc911x_reg_write")));

static void smc911x_handle_mac_address(struct eth_device *dev)
{
	unsigned long addrh, addrl;
	uchar *m = dev->enetaddr;

	addrl = m[0] | (m[1] << 8) | (m[2] << 16) | (m[3] << 24);
	addrh = m[4] | (m[5] << 8);
	smc911x_set_mac_csr(dev, ADDRL, addrl);
	smc911x_set_mac_csr(dev, ADDRH, addrh);

	printf(DRIVERNAME ": MAC %pM\n", m);
}

static int smc911x_eth_phy_read(struct eth_device *dev,
				u8 phy, u8 reg, u16 *val)
{
	while (smc911x_get_mac_csr(dev, MII_ACC) & MII_ACC_MII_BUSY)
		;

	smc911x_set_mac_csr(dev, MII_ACC, phy << 11 | reg << 6 |
				MII_ACC_MII_BUSY);

	while (smc911x_get_mac_csr(dev, MII_ACC) & MII_ACC_MII_BUSY)
		;

	*val = smc911x_get_mac_csr(dev, MII_DATA);

	return 0;
}

static int smc911x_eth_phy_write(struct eth_device *dev,
				u8 phy, u8 reg, u16  val)
{
	while (smc911x_get_mac_csr(dev, MII_ACC) & MII_ACC_MII_BUSY)
		;

	smc911x_set_mac_csr(dev, MII_DATA, val);
	smc911x_set_mac_csr(dev, MII_ACC,
		phy << 11 | reg << 6 | MII_ACC_MII_BUSY | MII_ACC_MII_WRITE);

	while (smc911x_get_mac_csr(dev, MII_ACC) & MII_ACC_MII_BUSY)
		;
	return 0;
}

static int smc911x_phy_reset(struct eth_device *dev)
{
	u32 reg;

	reg = smc911x_reg_read(dev, PMT_CTRL);
	reg &= ~0xfffff030;
	reg |= PMT_CTRL_PHY_RST;
	smc911x_reg_write(dev, PMT_CTRL, reg);

	mdelay(100);

	return 0;
}

static void smc911x_phy_configure(struct eth_device *dev)
{
	int timeout;
	u16 status;

	smc911x_phy_reset(dev);

	smc911x_eth_phy_write(dev, 1, MII_BMCR, BMCR_RESET);
	mdelay(1);
	smc911x_eth_phy_write(dev, 1, MII_ADVERTISE, 0x01e1);
	smc911x_eth_phy_write(dev, 1, MII_BMCR, BMCR_ANENABLE |
				BMCR_ANRESTART);

	timeout = 5000;
	do {
		mdelay(1);
		if ((timeout--) == 0)
			goto err_out;

		if (smc911x_eth_phy_read(dev, 1, MII_BMSR, &status) != 0)
			goto err_out;
	} while (!(status & BMSR_LSTATUS));

	printf(DRIVERNAME ": phy initialized\n");

	return;

err_out:
	printf(DRIVERNAME ": autonegotiation timed out\n");
}

static void smc911x_enable(struct eth_device *dev)
{
	/* Enable TX */
	smc911x_reg_write(dev, HW_CFG, 8 << 16 | HW_CFG_SF);

	smc911x_reg_write(dev, GPT_CFG, GPT_CFG_TIMER_EN | 10000);

	smc911x_reg_write(dev, TX_CFG, TX_CFG_TX_ON);

	/* no padding to start of packets */
	smc911x_reg_write(dev, RX_CFG, 0);

	smc911x_set_mac_csr(dev, MAC_CR, MAC_CR_TXEN | MAC_CR_RXEN |
				MAC_CR_HBDIS);

}

static int smc911x_init(struct eth_device *dev, bd_t * bd)
{
	struct chip_id *id = dev->priv;

	printf(DRIVERNAME ": detected %s controller\n", id->name);

	smc911x_reset(dev);

	/* Configure the PHY, initialize the link state */
	smc911x_phy_configure(dev);

	smc911x_handle_mac_address(dev);

	/* Turn on Tx + Rx */
	smc911x_enable(dev);

	return 0;
}

static int smc911x_send(struct eth_device *dev, void *packet, int length)
{
	u32 *data = (u32*)packet;
	u32 tmplen;
	u32 status;

	smc911x_reg_write(dev, TX_DATA_FIFO, TX_CMD_A_INT_FIRST_SEG |
				TX_CMD_A_INT_LAST_SEG | length);
	smc911x_reg_write(dev, TX_DATA_FIFO, length);

	tmplen = (length + 3) / 4;

	while (tmplen--)
		pkt_data_push(dev, TX_DATA_FIFO, *data++);

	/* wait for transmission */
	while (!((smc911x_reg_read(dev, TX_FIFO_INF) &
					TX_FIFO_INF_TSUSED) >> 16));

	/* get status. Ignore 'no carrier' error, it has no meaning for
	 * full duplex operation
	 */
	status = smc911x_reg_read(dev, TX_STATUS_FIFO) &
			(TX_STS_LOC | TX_STS_LATE_COLL | TX_STS_MANY_COLL |
			TX_STS_MANY_DEFER | TX_STS_UNDERRUN);

	if (!status)
		return 0;

	printf(DRIVERNAME ": failed to send packet: %s%s%s%s%s\n",
		status & TX_STS_LOC ? "TX_STS_LOC " : "",
		status & TX_STS_LATE_COLL ? "TX_STS_LATE_COLL " : "",
		status & TX_STS_MANY_COLL ? "TX_STS_MANY_COLL " : "",
		status & TX_STS_MANY_DEFER ? "TX_STS_MANY_DEFER " : "",
		status & TX_STS_UNDERRUN ? "TX_STS_UNDERRUN" : "");

	return -1;
}

static void smc911x_halt(struct eth_device *dev)
{
	smc911x_reset(dev);
	smc911x_handle_mac_address(dev);
}

static int smc911x_rx(struct eth_device *dev)
{
	u32 *data = (u32 *)net_rx_packets[0];
	u32 pktlen, tmplen;
	u32 status;

	if ((smc911x_reg_read(dev, RX_FIFO_INF) & RX_FIFO_INF_RXSUSED) >> 16) {
		status = smc911x_reg_read(dev, RX_STATUS_FIFO);
		pktlen = (status & RX_STS_PKT_LEN) >> 16;

		smc911x_reg_write(dev, RX_CFG, 0);

		tmplen = (pktlen + 3) / 4;
		while (tmplen--)
			*data++ = pkt_data_pull(dev, RX_DATA_FIFO);

		if (status & RX_STS_ES)
			printf(DRIVERNAME
				": dropped bad packet. Status: 0x%08x\n",
				status);
		else
			net_process_received_packet(net_rx_packets[0], pktlen);
	}

	return 0;
}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
/* wrapper for smc911x_eth_phy_read */
static int smc911x_miiphy_read(struct mii_dev *bus, int phy, int devad,
			       int reg)
{
	u16 val = 0;
	struct eth_device *dev = eth_get_dev_by_name(bus->name);
	if (dev) {
		int retval = smc911x_eth_phy_read(dev, phy, reg, &val);
		if (retval < 0)
			return retval;
		return val;
	}
	return -ENODEV;
}
/* wrapper for smc911x_eth_phy_write */
static int smc911x_miiphy_write(struct mii_dev *bus, int phy, int devad,
				int reg, u16 val)
{
	struct eth_device *dev = eth_get_dev_by_name(bus->name);
	if (dev)
		return smc911x_eth_phy_write(dev, phy, reg, val);
	return -ENODEV;
}
#endif

int smc911x_initialize(u8 dev_num, int base_addr)
{
	unsigned long addrl, addrh;
	struct eth_device *dev;

	dev = malloc(sizeof(*dev));
	if (!dev) {
		return -1;
	}
	memset(dev, 0, sizeof(*dev));

	dev->iobase = base_addr;

	/* Try to detect chip. Will fail if not present. */
	if (smc911x_detect_chip(dev)) {
		free(dev);
		return 0;
	}

	addrh = smc911x_get_mac_csr(dev, ADDRH);
	addrl = smc911x_get_mac_csr(dev, ADDRL);
	if (!(addrl == 0xffffffff && addrh == 0x0000ffff)) {
		/* address is obtained from optional eeprom */
		dev->enetaddr[0] = addrl;
		dev->enetaddr[1] = addrl >>  8;
		dev->enetaddr[2] = addrl >> 16;
		dev->enetaddr[3] = addrl >> 24;
		dev->enetaddr[4] = addrh;
		dev->enetaddr[5] = addrh >> 8;
	}

	dev->init = smc911x_init;
	dev->halt = smc911x_halt;
	dev->send = smc911x_send;
	dev->recv = smc911x_rx;
	sprintf(dev->name, "%s-%hu", DRIVERNAME, dev_num);

	eth_register(dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	int retval;
	struct mii_dev *mdiodev = mdio_alloc();
	if (!mdiodev)
		return -ENOMEM;
	strncpy(mdiodev->name, dev->name, MDIO_NAME_LEN);
	mdiodev->read = smc911x_miiphy_read;
	mdiodev->write = smc911x_miiphy_write;

	retval = mdio_register(mdiodev);
	if (retval < 0)
		return retval;
#endif

	return 1;
}
