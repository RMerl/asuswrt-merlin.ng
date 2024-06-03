/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 */

#ifndef __FM_H__
#define __FM_H__

#include <common.h>
#include <phy.h>
#include <fm_eth.h>
#include <fsl_fman.h>

/* Port ID */
#define OH_PORT_ID_BASE		0x01
#define MAX_NUM_OH_PORT		7
#define RX_PORT_1G_BASE		0x08
#define MAX_NUM_RX_PORT_1G	CONFIG_SYS_NUM_FM1_DTSEC
#define RX_PORT_10G_BASE	0x10
#define RX_PORT_10G_BASE2	0x08
#define TX_PORT_1G_BASE		0x28
#define MAX_NUM_TX_PORT_1G	CONFIG_SYS_NUM_FM1_DTSEC
#define TX_PORT_10G_BASE	0x30
#define TX_PORT_10G_BASE2	0x28
#define MIIM_TIMEOUT    0xFFFF

struct fm_muram {
	void *base;
	void *top;
	size_t size;
	void *alloc;
};
#define FM_MURAM_RES_SIZE	0x01000

/* Rx/Tx buffer descriptor */
struct fm_port_bd {
	u16 status;
	u16 len;
	u32 res0;
	u16 res1;
	u16 buf_ptr_hi;
	u32 buf_ptr_lo;
};

/* Common BD flags */
#define BD_LAST			0x0800

/* Rx BD status flags */
#define RxBD_EMPTY		0x8000
#define RxBD_LAST		BD_LAST
#define RxBD_FIRST		0x0400
#define RxBD_PHYS_ERR		0x0008
#define RxBD_SIZE_ERR		0x0004
#define RxBD_ERROR		(RxBD_PHYS_ERR | RxBD_SIZE_ERR)

/* Tx BD status flags */
#define TxBD_READY		0x8000
#define TxBD_LAST		BD_LAST

/* Rx/Tx queue descriptor */
struct fm_port_qd {
	u16 gen;
	u16 bd_ring_base_hi;
	u32 bd_ring_base_lo;
	u16 bd_ring_size;
	u16 offset_in;
	u16 offset_out;
	u16 res0;
	u32 res1[0x4];
};

/* IM global parameter RAM */
struct fm_port_global_pram {
	u32 mode;	/* independent mode register */
	u32 rxqd_ptr;	/* Rx queue descriptor pointer */
	u32 txqd_ptr;	/* Tx queue descriptor pointer */
	u16 mrblr;	/* max Rx buffer length */
	u16 rxqd_bsy_cnt;	/* RxQD busy counter, should be cleared */
	u32 res0[0x4];
	struct fm_port_qd rxqd;	/* Rx queue descriptor */
	struct fm_port_qd txqd;	/* Tx queue descriptor */
	u32 res1[0x28];
};

#define FM_PRAM_SIZE		sizeof(struct fm_port_global_pram)
#define FM_PRAM_ALIGN		256
#define PRAM_MODE_GLOBAL	0x20000000
#define PRAM_MODE_GRACEFUL_STOP	0x00800000

#if defined(CONFIG_ARCH_P1023)
#define FM_FREE_POOL_SIZE	0x2000 /* 8K bytes */
#else
#define FM_FREE_POOL_SIZE	0x20000 /* 128K bytes */
#endif
#define FM_FREE_POOL_ALIGN	256

void *fm_muram_alloc(int fm_idx, size_t size, ulong align);
void *fm_muram_base(int fm_idx);
int fm_init_common(int index, struct ccsr_fman *reg);
int fm_eth_initialize(struct ccsr_fman *reg, struct fm_eth_info *info);
phy_interface_t fman_port_enet_if(enum fm_port port);
void fman_disable_port(enum fm_port port);
void fman_enable_port(enum fm_port port);

struct fsl_enet_mac {
	void *base; /* MAC controller registers base address */
	void *phyregs;
	int max_rx_len;
	void (*init_mac)(struct fsl_enet_mac *mac);
	void (*enable_mac)(struct fsl_enet_mac *mac);
	void (*disable_mac)(struct fsl_enet_mac *mac);
	void (*set_mac_addr)(struct fsl_enet_mac *mac, u8 *mac_addr);
	void (*set_if_mode)(struct fsl_enet_mac *mac, phy_interface_t type,
				int speed);
};

/* Fman ethernet private struct */
struct fm_eth {
	int fm_index;			/* Fman index */
	u32 num;			/* 0..n-1 for give type */
	struct fm_bmi_tx_port *tx_port;
	struct fm_bmi_rx_port *rx_port;
	enum fm_eth_type type;		/* 1G or 10G ethernet */
	phy_interface_t enet_if;
	struct fsl_enet_mac *mac;	/* MAC controller */
	struct mii_dev *bus;
	struct phy_device *phydev;
	int phyaddr;
	struct eth_device *dev;
	int max_rx_len;
	struct fm_port_global_pram *rx_pram; /* Rx parameter table */
	struct fm_port_global_pram *tx_pram; /* Tx parameter table */
	void *rx_bd_ring;		/* Rx BD ring base */
	void *cur_rxbd;			/* current Rx BD */
	void *rx_buf;			/* Rx buffer base */
	void *tx_bd_ring;		/* Tx BD ring base */
	void *cur_txbd;			/* current Tx BD */
};

#define RX_BD_RING_SIZE		8
#define TX_BD_RING_SIZE		8
#define MAX_RXBUF_LOG2		11
#define MAX_RXBUF_LEN		(1 << MAX_RXBUF_LOG2)

#define PORT_IS_ENABLED(port)	(fm_port_to_index(port) == -1 ? \
	0 : fm_info[fm_port_to_index(port)].enabled)

#endif /* __FM_H__ */
