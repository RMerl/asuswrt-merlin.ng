/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014-2017 Broadcom.
 */

#ifndef _BCM_SF2_ETH_H_
#define _BCM_SF2_ETH_H_

#include <phy.h>

#define RX_BUF_SIZE	2048
/* RX_BUF_NUM must be power of 2 */
#define RX_BUF_NUM	32

#define TX_BUF_SIZE	2048
/* TX_BUF_NUM must be power of 2 */
#define TX_BUF_NUM	2

/* Support 2 Ethernet ports now */
#define BCM_ETH_MAX_PORT_NUM	2

enum {
	MAC_DMA_TX = 1,
	MAC_DMA_RX = 2
};

struct eth_dma {
	void *tx_desc_aligned;
	void *rx_desc_aligned;

	uint8_t *tx_buf;
	uint8_t *rx_buf;

	int cur_tx_index;
	int cur_rx_index;

	int (*tx_packet)(struct eth_dma *dma, void *packet, int length);
	bool (*check_tx_done)(struct eth_dma *dma);

	int (*check_rx_done)(struct eth_dma *dma, uint8_t *buf);

	int (*enable_dma)(struct eth_dma *dma, int dir);
	int (*disable_dma)(struct eth_dma *dma, int dir);
};

struct eth_info {
	struct eth_dma dma;
	phy_interface_t phy_interface;
	struct phy_device *port[BCM_ETH_MAX_PORT_NUM];
	int port_num;

	int (*miiphy_read)(struct mii_dev *bus, int phyaddr, int devad,
			   int reg);
	int (*miiphy_write)(struct mii_dev *bus, int phyaddr, int devad,
			    int reg, u16 value);

	int (*mac_init)(struct eth_device *dev);
	int (*enable_mac)(void);
	int (*disable_mac)(void);
	int (*set_mac_addr)(unsigned char *mac);
	int (*set_mac_speed)(int speed, int duplex);

};

#endif /* _BCM_SF2_ETH_H_ */
