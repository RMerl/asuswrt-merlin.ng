/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#ifndef __PFE_DRIVER_H__
#define __PFE_DRIVER_H__

#include <net/pfe_eth/pfe/pfe_hw.h>
#include <dm/platform_data/pfe_dm_eth.h>

#define HIF_RX_DESC_NT		64
#define	HIF_TX_DESC_NT		64

#define RX_BD_BASEADDR		(HIF_DESC_BASEADDR)
#define TX_BD_BASEADDR		(HIF_DESC_BASEADDR + HIF_TX_DESC_SIZE)

#define MIN_PKT_SIZE		56
#define MAX_FRAME_SIZE		2048

struct __packed hif_header_s {
	u8	port_no; /* Carries input port no for host rx packets and
			  * output port no for tx pkts
			  */
	u8 reserved0;
	u32 reserved2;
};

struct __packed buf_desc {
	u32 ctrl;
	u32 status;
	u32 data;
	u32 next;
};

struct rx_desc_s {
	struct buf_desc *rx_base;
	unsigned int rx_base_pa;
	int rx_to_read;
	int rx_ring_size;
};

struct tx_desc_s {
	struct buf_desc *tx_base;
	unsigned int tx_base_pa;
	int tx_to_send;
	int tx_ring_size;
};

int pfe_send(int phy_port, void *data, int length);
int pfe_recv(uchar **pkt_ptr, int *phy_port);
int pfe_tx_done(void);
int pfe_eth_free_pkt(struct udevice *dev, uchar *packet, int length);
int pfe_drv_init(struct pfe_ddr_address  *pfe_addr);
int pfe_eth_remove(struct udevice *dev);

#endif
