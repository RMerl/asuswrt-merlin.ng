/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 *	Roy Zang <tie-fei.zang@freescale.com>
 */

#ifndef __MEMAC_H__
#define __MEMAC_H__

#include <phy.h>

struct memac {
	/* memac general control and status registers */
	u32	res_0[2];
	u32	command_config;	/* Control and configuration register */
	u32	mac_addr_0;	/* Lower 32 bits of 48-bit MAC address */
	u32	mac_addr_1;	/* Upper 16 bits of 48-bit MAC address */
	u32	maxfrm;		/* Maximum frame length register */
	u32	res_18[5];
	u32	hashtable_ctrl;	/* Hash table control register */
	u32	res_30[4];
	u32	ievent;		/* Interrupt event register */
	u32	tx_ipg_length;	/* Transmitter inter-packet-gap register */
	u32	res_48;
	u32	imask;		/* interrupt mask register */
	u32	res_50;
	u32	cl_pause_quanta[4]; /* CL01-CL67 pause quanta register */
	u32	cl_pause_thresh[4]; /* CL01-CL67 pause thresh register */
	u32	rx_pause_status;	/* Receive pause status register */
	u32	res_78[2];
	u32	mac_addr[14];	/* MAC address */
	u32	lpwake_timer;	/* EEE low power wakeup timer register */
	u32	sleep_timer;	/* Transmit EEE Low Power Timer register */
	u32	res_c0[8];
	u32	statn_config;	/* Statistics configuration register */
	u32	res_e4[7];

	/* memac statistics counter registers */
	u32	rx_eoct_l;	/* Rx ethernet octests lower */
	u32	rx_eoct_u;	/* Rx ethernet octests upper */
	u32	rx_oct_l;	/* Rx octests lower */
	u32	rx_oct_u;	/* Rx octests upper */
	u32	rx_align_err_l;	/* Rx alignment error lower */
	u32	rx_align_err_u;	/* Rx alignment error upper */
	u32	rx_pause_frame_l; /* Rx valid pause frame upper */
	u32	rx_pause_frame_u; /* Rx valid pause frame upper */
	u32	rx_frame_l;	/* Rx frame counter lower */
	u32	rx_frame_u;	/* Rx frame counter upper */
	u32	rx_frame_crc_err_l; /* Rx frame check sequence error lower */
	u32	rx_frame_crc_err_u; /* Rx frame check sequence error upper */
	u32	rx_vlan_l;	/* Rx VLAN frame lower */
	u32	rx_vlan_u;	/* Rx VLAN frame upper */
	u32	rx_err_l;	/* Rx frame error lower */
	u32	rx_err_u;	/* Rx frame error upper */
	u32	rx_uni_l;	/* Rx unicast frame lower */
	u32	rx_uni_u;	/* Rx unicast frame upper */
	u32	rx_multi_l;	/* Rx multicast frame lower */
	u32	rx_multi_u;	/* Rx multicast frame upper */
	u32	rx_brd_l;	/* Rx broadcast frame lower */
	u32	rx_brd_u;	/* Rx broadcast frame upper */
	u32	rx_drop_l;	/* Rx dropped packets lower */
	u32	rx_drop_u;	/* Rx dropped packets upper */
	u32	rx_pkt_l;	/* Rx packets lower */
	u32	rx_pkt_u;	/* Rx packets upper */
	u32	rx_undsz_l;	/* Rx undersized packet lower */
	u32	rx_undsz_u;	/* Rx undersized packet upper */
	u32	rx_64_l;	/* Rx 64 oct packet lower */
	u32	rx_64_u;	/* Rx 64 oct packet upper */
	u32	rx_127_l;	/* Rx 65 to 127 oct packet lower */
	u32	rx_127_u;	/* Rx 65 to 127 oct packet upper */
	u32	rx_255_l;	/* Rx 128 to 255 oct packet lower */
	u32	rx_255_u;	/* Rx 128 to 255 oct packet upper */
	u32	rx_511_l;	/* Rx 256 to 511 oct packet lower */
	u32	rx_511_u;	/* Rx 256 to 511 oct packet upper */
	u32	rx_1023_l;	/* Rx 512 to 1023 oct packet lower */
	u32	rx_1023_u;	/* Rx 512 to 1023 oct packet upper */
	u32	rx_1518_l;	/* Rx 1024 to 1518 oct packet lower */
	u32	rx_1518_u;	/* Rx 1024 to 1518 oct packet upper */
	u32	rx_1519_l;	/* Rx 1519 to max oct packet lower */
	u32	rx_1519_u;	/* Rx 1519 to max oct packet upper */
	u32	rx_oversz_l;	/* Rx oversized packet lower */
	u32	rx_oversz_u;	/* Rx oversized packet upper */
	u32	rx_jabber_l;	/* Rx Jabber packet lower */
	u32	rx_jabber_u;	/* Rx Jabber packet upper */
	u32	rx_frag_l;	/* Rx Fragment packet lower */
	u32	rx_frag_u;	/* Rx Fragment packet upper */
	u32	rx_cnp_l;	/* Rx control packet lower */
	u32	rx_cnp_u;	/* Rx control packet upper */
	u32	rx_drntp_l;	/* Rx dripped not truncated packet lower */
	u32	rx_drntp_u;	/* Rx dripped not truncated packet upper */
	u32	res_1d0[0xc];

	u32	tx_eoct_l;	/* Tx ethernet octests lower */
	u32	tx_eoct_u;	/* Tx ethernet octests upper */
	u32	tx_oct_l;	/* Tx octests lower */
	u32	tx_oct_u;	/* Tx octests upper */
	u32	res_210[0x2];
	u32	tx_pause_frame_l; /* Tx valid pause frame lower */
	u32	tx_pause_frame_u; /* Tx valid pause frame upper */
	u32	tx_frame_l;	/* Tx frame counter lower */
	u32	tx_frame_u;	/* Tx frame counter upper */
	u32	tx_frame_crc_err_l; /* Tx frame check sequence error lower */
	u32	tx_frame_crc_err_u; /* Tx frame check sequence error upper */
	u32	tx_vlan_l;	/* Tx VLAN frame lower */
	u32	tx_vlan_u;	/* Tx VLAN frame upper */
	u32	tx_frame_err_l;	/* Tx frame error lower */
	u32	tx_frame_err_u;	/* Tx frame error upper */
	u32	tx_uni_l;	/* Tx unicast frame lower */
	u32	tx_uni_u;	/* Tx unicast frame upper */
	u32	tx_multi_l;	/* Tx multicast frame lower */
	u32	tx_multi_u;	/* Tx multicast frame upper */
	u32	tx_brd_l;	/* Tx broadcast frame lower */
	u32	tx_brd_u;	/* Tx broadcast frame upper */
	u32	res_258[0x2];
	u32	tx_pkt_l;	/* Tx packets lower */
	u32	tx_pkt_u;	/* Tx packets upper */
	u32	tx_undsz_l;	/* Tx undersized packet lower */
	u32	tx_undsz_u;	/* Tx undersized packet upper */
	u32	tx_64_l;	/* Tx 64 oct packet lower */
	u32	tx_64_u;	/* Tx 64 oct packet upper */
	u32	tx_127_l;	/* Tx 65 to 127 oct packet lower */
	u32	tx_127_u;	/* Tx 65 to 127 oct packet upper */
	u32	tx_255_l;	/* Tx 128 to 255 oct packet lower */
	u32	tx_255_u;	/* Tx 128 to 255 oct packet upper */
	u32	tx_511_l;	/* Tx 256 to 511 oct packet lower */
	u32	tx_511_u;	/* Tx 256 to 511 oct packet upper */
	u32	tx_1023_l;	/* Tx 512 to 1023 oct packet lower */
	u32	tx_1023_u;	/* Tx 512 to 1023 oct packet upper */
	u32	tx_1518_l;	/* Tx 1024 to 1518 oct packet lower */
	u32	tx_1518_u;	/* Tx 1024 to 1518 oct packet upper */
	u32	tx_1519_l;	/* Tx 1519 to max oct packet lower */
	u32	tx_1519_u;	/* Tx 1519 to max oct packet upper */
	u32	res_2a8[0x6];
	u32	tx_cnp_l;	/* Tx control packet lower */
	u32	tx_cnp_u;	/* Tx control packet upper */
	u32	res_2c8[0xe];

	/* Line interface control register */
	u32 if_mode;		/* interface mode control */
	u32 if_status;		/* interface status */
	u32 res_308[0xe];

	/* HiGig/2 Register */
	u32 hg_config;	/* HiGig2 control and configuration */
	u32 res_344[0x3];
	u32 hg_pause_quanta;	/* HiGig2 pause quanta */
	u32 res_354[0x3];
	u32 hg_pause_thresh;	/* HiGig2 pause quanta threshold */
	u32 res_364[0x3];
	u32 hgrx_pause_status;	/* HiGig2 rx pause quanta status */
	u32 hg_fifos_status;	/* HiGig2 fifos status */
	u32 rhm;	/* Rx HiGig2 message counter register */
	u32 thm;/* Tx HiGig2 message counter register */
	u32 res_380[0x320];
};

/* COMMAND_CONFIG - command and configuration register */
#define MEMAC_CMD_CFG_RX_EN		0x00000002 /* MAC Rx path enable */
#define MEMAC_CMD_CFG_TX_EN		0x00000001 /* MAC Tx path enable */
#define MEMAC_CMD_CFG_RXTX_EN	(MEMAC_CMD_CFG_RX_EN | MEMAC_CMD_CFG_TX_EN)
#define MEMAC_CMD_CFG_NO_LEN_CHK 0x20000 /* Payload length check disable */

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

/* IF_MODE - Interface Mode Register */
#define IF_MODE_EN_AUTO	0x00008000 /* 1 - Enable automatic speed selection */
#define IF_MODE_SETSP_100M	0x00000000 /* 00 - 100Mbps RGMII */
#define IF_MODE_SETSP_10M	0x00002000 /* 01 - 10Mbps RGMII */
#define IF_MODE_SETSP_1000M	0x00004000 /* 10 - 1000Mbps RGMII */
#define IF_MODE_SETSP_MASK	0x00006000 /* setsp mask bits */
#define IF_MODE_XGMII	0x00000000 /* 00- XGMII(10) interface mode */
#define IF_MODE_GMII		0x00000002 /* 10- GMII interface mode */
#define IF_MODE_MASK	0x00000003 /* mask for mode interface mode */
#define IF_MODE_RG		0x00000004 /* 1- RGMII */
#define IF_MODE_RM		0x00000008 /* 1- RGMII */

#define IF_DEFAULT	(IF_GMII)

/* Internal PHY Registers - SGMII */
#define PHY_SGMII_CR_PHY_RESET      0x8000
#define PHY_SGMII_CR_RESET_AN       0x0200
#define PHY_SGMII_CR_DEF_VAL        0x1140
#define PHY_SGMII_IF_SPEED_GIGABIT  0x0008
#define PHY_SGMII_DEV_ABILITY_SGMII 0x4001
#define PHY_SGMII_IF_MODE_AN        0x0002
#define PHY_SGMII_IF_MODE_SGMII     0x0001

struct memac_mdio_controller {
	u32	res0[0xc];
	u32	mdio_stat;	/* MDIO configuration and status */
	u32	mdio_ctl;	/* MDIO control */
	u32	mdio_data;	/* MDIO data */
	u32	mdio_addr;	/* MDIO address */
};

#define MDIO_STAT_CLKDIV(x)	(((x>>1) & 0xff) << 8)
#define MDIO_STAT_BSY		(1 << 0)
#define MDIO_STAT_RD_ER		(1 << 1)
#define MDIO_STAT_PRE		(1 << 5)
#define MDIO_STAT_ENC		(1 << 6)
#define MDIO_STAT_HOLD_15_CLK	(7 << 2)
#define MDIO_STAT_NEG		(1 << 23)

#define MDIO_CTL_DEV_ADDR(x)	(x & 0x1f)
#define MDIO_CTL_PORT_ADDR(x)	((x & 0x1f) << 5)
#define MDIO_CTL_PRE_DIS	(1 << 10)
#define MDIO_CTL_SCAN_EN	(1 << 11)
#define MDIO_CTL_POST_INC	(1 << 14)
#define MDIO_CTL_READ		(1 << 15)

#define MDIO_DATA(x)		(x & 0xffff)

struct fsl_enet_mac;

void init_memac(struct fsl_enet_mac *mac, void *base, void *phyregs,
		int max_rx_len);

#endif
