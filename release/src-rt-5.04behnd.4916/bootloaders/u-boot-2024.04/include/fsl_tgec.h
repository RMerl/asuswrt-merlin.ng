/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 *	Dave Liu <daveliu@freescale.com>
 */

#ifndef __TGEC_H__
#define __TGEC_H__

#include <phy.h>

struct tgec {
	/* 10GEC general control and status registers */
	u32	tgec_id;	/* Controller ID register */
	u32	res0;
	u32	command_config;	/* Control and configuration register */
	u32	mac_addr_0;	/* Lower 32 bits of 48-bit MAC address */
	u32	mac_addr_1;	/* Upper 16 bits of 48-bit MAC address */
	u32	maxfrm;		/* Maximum frame length register */
	u32	pause_quant;	/* Pause quanta register */
	u32	res1[4];
	u32	hashtable_ctrl;	/* Hash table control register */
	u32	res2[4];
	u32	status;		/* MAC status register */
	u32	tx_ipg_length;	/* Transmitter inter-packet-gap register */
	u32	mac_addr_2;	/* Lower 32 bits of the 2nd 48-bit MAC addr */
	u32	mac_addr_3;	/* Upper 16 bits of the 2nd 48-bit MAC addr */
	u32	res3[4];
	u32	imask;		/* Interrupt mask register */
	u32	ievent;		/* Interrupt event register */
	u32	res4[6];
	/* 10GEC statistics counter registers */
	u32	tx_frame_u;	/* Tx frame counter upper */
	u32	tx_frame_l;	/* Tx frame counter lower */
	u32	rx_frame_u;	/* Rx frame counter upper */
	u32	rx_frame_l;	/* Rx frame counter lower */
	u32	rx_frame_crc_err_u; /* Rx frame check sequence error upper */
	u32	rx_frame_crc_err_l; /* Rx frame check sequence error lower */
	u32	rx_align_err_u;	/* Rx alignment error upper */
	u32	rx_align_err_l;	/* Rx alignment error lower */
	u32	tx_pause_frame_u; /* Tx valid pause frame upper */
	u32	tx_pause_frame_l; /* Tx valid pause frame lower */
	u32	rx_pause_frame_u; /* Rx valid pause frame upper */
	u32	rx_pause_frame_l; /* Rx valid pause frame upper */
	u32	rx_long_err_u;	/* Rx too long frame error upper */
	u32	rx_long_err_l;	/* Rx too long frame error lower */
	u32	rx_frame_err_u;	/* Rx frame length error upper */
	u32	rx_frame_err_l;	/* Rx frame length error lower */
	u32	tx_vlan_u;	/* Tx VLAN frame upper */
	u32	tx_vlan_l;	/* Tx VLAN frame lower */
	u32	rx_vlan_u;	/* Rx VLAN frame upper */
	u32	rx_vlan_l;	/* Rx VLAN frame lower */
	u32	tx_oct_u;	/* Tx octets upper */
	u32	tx_oct_l;	/* Tx octets lower */
	u32	rx_oct_u;	/* Rx octets upper */
	u32	rx_oct_l;	/* Rx octets lower */
	u32	rx_uni_u;	/* Rx unicast frame upper */
	u32	rx_uni_l;	/* Rx unicast frame lower */
	u32	rx_multi_u;	/* Rx multicast frame upper */
	u32	rx_multi_l;	/* Rx multicast frame lower */
	u32	rx_brd_u;	/* Rx broadcast frame upper */
	u32	rx_brd_l;	/* Rx broadcast frame lower */
	u32	tx_frame_err_u;	/* Tx frame error upper */
	u32	tx_frame_err_l;	/* Tx frame error lower */
	u32	tx_uni_u;	/* Tx unicast frame upper */
	u32	tx_uni_l;	/* Tx unicast frame lower */
	u32	tx_multi_u;	/* Tx multicast frame upper */
	u32	tx_multi_l;	/* Tx multicast frame lower */
	u32	tx_brd_u;	/* Tx broadcast frame upper */
	u32	tx_brd_l;	/* Tx broadcast frame lower */
	u32	rx_drop_u;	/* Rx dropped packets upper */
	u32	rx_drop_l;	/* Rx dropped packets lower */
	u32	rx_eoct_u;	/* Rx ethernet octets upper */
	u32	rx_eoct_l;	/* Rx ethernet octets lower */
	u32	rx_pkt_u;	/* Rx packets upper */
	u32	rx_pkt_l;	/* Rx packets lower */
	u32	tx_undsz_u;	/* Undersized packet upper */
	u32	tx_undsz_l;	/* Undersized packet lower */
	u32	rx_64_u;	/* Rx 64 oct packet upper */
	u32	rx_64_l;	/* Rx 64 oct packet lower */
	u32	rx_127_u;	/* Rx 65 to 127 oct packet upper */
	u32	rx_127_l;	/* Rx 65 to 127 oct packet lower */
	u32	rx_255_u;	/* Rx 128 to 255 oct packet upper */
	u32	rx_255_l;	/* Rx 128 to 255 oct packet lower */
	u32	rx_511_u;	/* Rx 256 to 511 oct packet upper */
	u32	rx_511_l;	/* Rx 256 to 511 oct packet lower */
	u32	rx_1023_u;	/* Rx 512 to 1023 oct packet upper */
	u32	rx_1023_l;	/* Rx 512 to 1023 oct packet lower */
	u32	rx_1518_u;	/* Rx 1024 to 1518 oct packet upper */
	u32	rx_1518_l;	/* Rx 1024 to 1518 oct packet lower */
	u32	rx_1519_u;	/* Rx 1519 to max oct packet upper */
	u32	rx_1519_l;	/* Rx 1519 to max oct packet lower */
	u32	tx_oversz_u;	/* oversized packet upper */
	u32	tx_oversz_l;	/* oversized packet lower */
	u32	tx_jabber_u;	/* Jabber packet upper */
	u32	tx_jabber_l;	/* Jabber packet lower */
	u32	tx_frag_u;	/* Fragment packet upper */
	u32	tx_frag_l;	/* Fragment packet lower */
	u32	rx_err_u;	/* Rx frame error upper */
	u32	rx_err_l;	/* Rx frame error lower */
	u32	res5[0x39a];
};

/* EC10G_ID - 10-gigabit ethernet MAC controller ID */
#define EC10G_ID_VER_MASK	0x0000ff00
#define EC10G_ID_VER_SHIFT	8
#define EC10G_ID_REV_MASK	0x000000ff

/* COMMAND_CONFIG - command and configuration register */
#define TGEC_CMD_CFG_EN_TIMESTAMP	0x00100000 /* enable IEEE1588 */
#define TGEC_CMD_CFG_TX_ADDR_INS_SEL	0x00080000 /* Tx mac addr w/ second */
#define TGEC_CMD_CFG_NO_LEN_CHK		0x00020000 /* payload len chk disable */
#define TGEC_CMD_CFG_SEND_IDLE		0x00010000 /* send XGMII idle seqs */
#define TGEC_CMD_CFG_RX_ER_DISC		0x00004000 /* Rx err frm discard enb */
#define TGEC_CMD_CFG_CMD_FRM_EN		0x00002000 /* CMD frame RX enable */
#define TGEC_CMD_CFG_STAT_CLR		0x00001000 /* clear stats */
#define TGEC_CMD_CFG_TX_ADDR_INS	0x00000200 /* overwrite src MAC addr */
#define TGEC_CMD_CFG_PAUSE_IGNORE	0x00000100 /* ignore pause frames */
#define TGEC_CMD_CFG_PAUSE_FWD		0x00000080 /* fwd pause frames */
#define TGEC_CMD_CFG_CRC_FWD		0x00000040 /* fwd Rx CRC frames */
#define TGEC_CMD_CFG_PAD_EN		0x00000020 /* MAC remove Rx padding */
#define TGEC_CMD_CFG_PROM_EN		0x00000010 /* promiscuous mode enable */
#define TGEC_CMD_CFG_WAN_MODE		0x00000008 /* WAN mode enable */
#define TGEC_CMD_CFG_RX_EN		0x00000002 /* MAC Rx path enable */
#define TGEC_CMD_CFG_TX_EN		0x00000001 /* MAC Tx path enable */
#define TGEC_CMD_CFG_RXTX_EN	(TGEC_CMD_CFG_RX_EN | TGEC_CMD_CFG_TX_EN)

/* HASHTABLE_CTRL - Hashtable control register */
#define HASHTABLE_CTRL_MCAST_EN	0x00000200 /* enable mulitcast Rx hash */
#define HASHTABLE_CTRL_ADDR_MASK	0x000001ff

/* TX_IPG_LENGTH - Transmit inter-packet gap length register */
#define TX_IPG_LENGTH_IPG_LEN_MASK	0x000003ff

/* IMASK - interrupt mask register */
#define IMASK_MDIO_SCAN_EVENT	0x00010000 /* MDIO scan event mask */
#define IMASK_MDIO_CMD_CMPL	0x00008000 /* MDIO cmd completion mask */
#define IMASK_REM_FAULT		0x00004000 /* remote fault mask */
#define IMASK_LOC_FAULT		0x00002000 /* local fault mask */
#define IMASK_TX_ECC_ER		0x00001000 /* Tx frame ECC error mask */
#define IMASK_TX_FIFO_UNFL	0x00000800 /* Tx FIFO underflow mask */
#define IMASK_TX_ER		0x00000200 /* Tx frame error mask */
#define IMASK_RX_FIFO_OVFL	0x00000100 /* Rx FIFO overflow mask */
#define IMASK_RX_ECC_ER		0x00000080 /* Rx frame ECC error mask */
#define IMASK_RX_JAB_FRM	0x00000040 /* Rx jabber frame mask */
#define IMASK_RX_OVRSZ_FRM	0x00000020 /* Rx oversized frame mask */
#define IMASK_RX_RUNT_FRM	0x00000010 /* Rx runt frame mask */
#define IMASK_RX_FRAG_FRM	0x00000008 /* Rx fragment frame mask */
#define IMASK_RX_LEN_ER		0x00000004 /* Rx payload length error mask */
#define IMASK_RX_CRC_ER		0x00000002 /* Rx CRC error mask */
#define IMASK_RX_ALIGN_ER	0x00000001 /* Rx alignment error mask */

#define IMASK_MASK_ALL		0x00000000

/* IEVENT - interrupt event register */
#define IEVENT_MDIO_SCAN_EVENT	0x00010000 /* MDIO scan event */
#define IEVENT_MDIO_CMD_CMPL	0x00008000 /* MDIO cmd completion */
#define IEVENT_REM_FAULT	0x00004000 /* remote fault */
#define IEVENT_LOC_FAULT	0x00002000 /* local fault */
#define IEVENT_TX_ECC_ER	0x00001000 /* Tx frame ECC error */
#define IEVENT_TX_FIFO_UNFL	0x00000800 /* Tx FIFO underflow */
#define IEVENT_TX_ER		0x00000200 /* Tx frame error */
#define IEVENT_RX_FIFO_OVFL	0x00000100 /* Rx FIFO overflow */
#define IEVENT_RX_ECC_ER	0x00000080 /* Rx frame ECC error */
#define IEVENT_RX_JAB_FRM	0x00000040 /* Rx jabber frame */
#define IEVENT_RX_OVRSZ_FRM	0x00000020 /* Rx oversized frame */
#define IEVENT_RX_RUNT_FRM	0x00000010 /* Rx runt frame */
#define IEVENT_RX_FRAG_FRM	0x00000008 /* Rx fragment frame */
#define IEVENT_RX_LEN_ER	0x00000004 /* Rx payload length error */
#define IEVENT_RX_CRC_ER	0x00000002 /* Rx CRC error */
#define IEVENT_RX_ALIGN_ER	0x00000001 /* Rx alignment error */

#define IEVENT_CLEAR_ALL	0xffffffff

struct tgec_mdio_controller {
	u32	res0[0xc];
	u32	mdio_stat;	/* MDIO configuration and status */
	u32	mdio_ctl;	/* MDIO control */
	u32	mdio_data;	/* MDIO data */
	u32	mdio_addr;	/* MDIO address */
};

#define MDIO_STAT_CLKDIV(x)	(((x>>1) & 0xff) << 8)
#define MDIO_STAT_BSY		(1 << 0)
#define MDIO_STAT_RD_ER		(1 << 1)
#define MDIO_CTL_DEV_ADDR(x)	(x & 0x1f)
#define MDIO_CTL_PORT_ADDR(x)	((x & 0x1f) << 5)
#define MDIO_CTL_PRE_DIS	(1 << 10)
#define MDIO_CTL_SCAN_EN	(1 << 11)
#define MDIO_CTL_POST_INC	(1 << 14)
#define MDIO_CTL_READ		(1 << 15)

#define MDIO_DATA(x)		(x & 0xffff)
#define MDIO_DATA_BSY		(1 << 31)

struct fsl_enet_mac;

void init_tgec(struct fsl_enet_mac *mac, void *base, void *phyregs,
		int max_rx_len);

#endif
