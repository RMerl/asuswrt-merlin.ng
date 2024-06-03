/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Ilya Yanok, Emcraft Systems Ltd <yanok@emcraft.com>
 * (C) Copyright 2008 Armadeus Systems, nc
 * (C) Copyright 2008 Eric Jarrige <eric.jarrige@armadeus.org>
 * (C) Copyright 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 * (C) Copyright 2007 Pengutronix, Juergen Beisert <j.beisert@pengutronix.de>
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * This file is based on mpc4200fec.h
 * (C) Copyright Motorola, Inc., 2000
 */

#ifndef __FEC_MXC_H
#define __FEC_MXC_H

#include <clk.h>

/* Layout description of the FEC */
struct ethernet_regs {
	/* [10:2]addr = 00 */

	/*  Control and status Registers (offset 000-1FF) */
	uint32_t res0[1];		/* MBAR_ETH + 0x000 */
	uint32_t ievent;		/* MBAR_ETH + 0x004 */
	uint32_t imask;			/* MBAR_ETH + 0x008 */

	uint32_t res1[1];		/* MBAR_ETH + 0x00C */
	uint32_t r_des_active;		/* MBAR_ETH + 0x010 */
	uint32_t x_des_active;		/* MBAR_ETH + 0x014 */
	uint32_t res2[3];		/* MBAR_ETH + 0x018-20 */
	uint32_t ecntrl;		/* MBAR_ETH + 0x024 */

	uint32_t res3[6];		/* MBAR_ETH + 0x028-03C */
	uint32_t mii_data;		/* MBAR_ETH + 0x040 */
	uint32_t mii_speed;		/* MBAR_ETH + 0x044 */
	uint32_t res4[7];		/* MBAR_ETH + 0x048-60 */
	uint32_t mib_control;		/* MBAR_ETH + 0x064 */

	uint32_t res5[7];		/* MBAR_ETH + 0x068-80 */
	uint32_t r_cntrl;		/* MBAR_ETH + 0x084 */
	uint32_t res6[15];		/* MBAR_ETH + 0x088-C0 */
	uint32_t x_cntrl;		/* MBAR_ETH + 0x0C4 */
	uint32_t res7[7];		/* MBAR_ETH + 0x0C8-E0 */
	uint32_t paddr1;		/* MBAR_ETH + 0x0E4 */
	uint32_t paddr2;		/* MBAR_ETH + 0x0E8 */
	uint32_t op_pause;		/* MBAR_ETH + 0x0EC */

	uint32_t res8[10];		/* MBAR_ETH + 0x0F0-114 */
	uint32_t iaddr1;		/* MBAR_ETH + 0x118 */
	uint32_t iaddr2;		/* MBAR_ETH + 0x11C */
	uint32_t gaddr1;		/* MBAR_ETH + 0x120 */
	uint32_t gaddr2;		/* MBAR_ETH + 0x124 */
	uint32_t res9[7];		/* MBAR_ETH + 0x128-140 */

	uint32_t x_wmrk;		/* MBAR_ETH + 0x144 */
	uint32_t res10[1];		/* MBAR_ETH + 0x148 */
	uint32_t r_bound;		/* MBAR_ETH + 0x14C */
	uint32_t r_fstart;		/* MBAR_ETH + 0x150 */
	uint32_t res11[11];		/* MBAR_ETH + 0x154-17C */
	uint32_t erdsr;			/* MBAR_ETH + 0x180 */
	uint32_t etdsr;			/* MBAR_ETH + 0x184 */
	uint32_t emrbr;			/* MBAR_ETH + 0x188 */
	uint32_t res12[29];		/* MBAR_ETH + 0x18C-1FC */

	/*  MIB COUNTERS (Offset 200-2FF) */
	uint32_t rmon_t_drop;		/* MBAR_ETH + 0x200 */
	uint32_t rmon_t_packets;	/* MBAR_ETH + 0x204 */
	uint32_t rmon_t_bc_pkt;		/* MBAR_ETH + 0x208 */
	uint32_t rmon_t_mc_pkt;		/* MBAR_ETH + 0x20C */
	uint32_t rmon_t_crc_align;	/* MBAR_ETH + 0x210 */
	uint32_t rmon_t_undersize;	/* MBAR_ETH + 0x214 */
	uint32_t rmon_t_oversize;	/* MBAR_ETH + 0x218 */
	uint32_t rmon_t_frag;		/* MBAR_ETH + 0x21C */
	uint32_t rmon_t_jab;		/* MBAR_ETH + 0x220 */
	uint32_t rmon_t_col;		/* MBAR_ETH + 0x224 */
	uint32_t rmon_t_p64;		/* MBAR_ETH + 0x228 */
	uint32_t rmon_t_p65to127;	/* MBAR_ETH + 0x22C */
	uint32_t rmon_t_p128to255;	/* MBAR_ETH + 0x230 */
	uint32_t rmon_t_p256to511;	/* MBAR_ETH + 0x234 */
	uint32_t rmon_t_p512to1023;	/* MBAR_ETH + 0x238 */
	uint32_t rmon_t_p1024to2047;	/* MBAR_ETH + 0x23C */
	uint32_t rmon_t_p_gte2048;	/* MBAR_ETH + 0x240 */
	uint32_t rmon_t_octets;		/* MBAR_ETH + 0x244 */
	uint32_t ieee_t_drop;		/* MBAR_ETH + 0x248 */
	uint32_t ieee_t_frame_ok;	/* MBAR_ETH + 0x24C */
	uint32_t ieee_t_1col;		/* MBAR_ETH + 0x250 */
	uint32_t ieee_t_mcol;		/* MBAR_ETH + 0x254 */
	uint32_t ieee_t_def;		/* MBAR_ETH + 0x258 */
	uint32_t ieee_t_lcol;		/* MBAR_ETH + 0x25C */
	uint32_t ieee_t_excol;		/* MBAR_ETH + 0x260 */
	uint32_t ieee_t_macerr;		/* MBAR_ETH + 0x264 */
	uint32_t ieee_t_cserr;		/* MBAR_ETH + 0x268 */
	uint32_t ieee_t_sqe;		/* MBAR_ETH + 0x26C */
	uint32_t t_fdxfc;		/* MBAR_ETH + 0x270 */
	uint32_t ieee_t_octets_ok;	/* MBAR_ETH + 0x274 */

	uint32_t res13[2];		/* MBAR_ETH + 0x278-27C */
	uint32_t rmon_r_drop;		/* MBAR_ETH + 0x280 */
	uint32_t rmon_r_packets;	/* MBAR_ETH + 0x284 */
	uint32_t rmon_r_bc_pkt;		/* MBAR_ETH + 0x288 */
	uint32_t rmon_r_mc_pkt;		/* MBAR_ETH + 0x28C */
	uint32_t rmon_r_crc_align;	/* MBAR_ETH + 0x290 */
	uint32_t rmon_r_undersize;	/* MBAR_ETH + 0x294 */
	uint32_t rmon_r_oversize;	/* MBAR_ETH + 0x298 */
	uint32_t rmon_r_frag;		/* MBAR_ETH + 0x29C */
	uint32_t rmon_r_jab;		/* MBAR_ETH + 0x2A0 */

	uint32_t rmon_r_resvd_0;	/* MBAR_ETH + 0x2A4 */

	uint32_t rmon_r_p64;		/* MBAR_ETH + 0x2A8 */
	uint32_t rmon_r_p65to127;	/* MBAR_ETH + 0x2AC */
	uint32_t rmon_r_p128to255;	/* MBAR_ETH + 0x2B0 */
	uint32_t rmon_r_p256to511;	/* MBAR_ETH + 0x2B4 */
	uint32_t rmon_r_p512to1023;	/* MBAR_ETH + 0x2B8 */
	uint32_t rmon_r_p1024to2047;	/* MBAR_ETH + 0x2BC */
	uint32_t rmon_r_p_gte2048;	/* MBAR_ETH + 0x2C0 */
	uint32_t rmon_r_octets;		/* MBAR_ETH + 0x2C4 */
	uint32_t ieee_r_drop;		/* MBAR_ETH + 0x2C8 */
	uint32_t ieee_r_frame_ok;	/* MBAR_ETH + 0x2CC */
	uint32_t ieee_r_crc;		/* MBAR_ETH + 0x2D0 */
	uint32_t ieee_r_align;		/* MBAR_ETH + 0x2D4 */
	uint32_t r_macerr;		/* MBAR_ETH + 0x2D8 */
	uint32_t r_fdxfc;		/* MBAR_ETH + 0x2DC */
	uint32_t ieee_r_octets_ok;	/* MBAR_ETH + 0x2E0 */

	uint32_t res14[7];		/* MBAR_ETH + 0x2E4-2FC */

#if defined(CONFIG_MX25) || defined(CONFIG_MX53) || defined(CONFIG_MX6SL)
	uint16_t miigsk_cfgr;		/* MBAR_ETH + 0x300 */
	uint16_t res15[3];		/* MBAR_ETH + 0x302-306 */
	uint16_t miigsk_enr;		/* MBAR_ETH + 0x308 */
	uint16_t res16[3];		/* MBAR_ETH + 0x30a-30e */
	uint32_t res17[60];		/* MBAR_ETH + 0x300-3FF */
#else
	uint32_t res15[64];		/* MBAR_ETH + 0x300-3FF */
#endif
};

#define FEC_IEVENT_HBERR		0x80000000
#define FEC_IEVENT_BABR			0x40000000
#define FEC_IEVENT_BABT			0x20000000
#define FEC_IEVENT_GRA			0x10000000
#define FEC_IEVENT_TXF			0x08000000
#define FEC_IEVENT_TXB			0x04000000
#define FEC_IEVENT_RXF			0x02000000
#define FEC_IEVENT_RXB			0x01000000
#define FEC_IEVENT_MII			0x00800000
#define FEC_IEVENT_EBERR		0x00400000
#define FEC_IEVENT_LC			0x00200000
#define FEC_IEVENT_RL			0x00100000
#define FEC_IEVENT_UN			0x00080000

#define FEC_IMASK_HBERR			0x80000000
#define FEC_IMASK_BABR			0x40000000
#define FEC_IMASKT_BABT			0x20000000
#define FEC_IMASK_GRA			0x10000000
#define FEC_IMASKT_TXF			0x08000000
#define FEC_IMASK_TXB			0x04000000
#define FEC_IMASKT_RXF			0x02000000
#define FEC_IMASK_RXB			0x01000000
#define FEC_IMASK_MII			0x00800000
#define FEC_IMASK_EBERR			0x00400000
#define FEC_IMASK_LC			0x00200000
#define FEC_IMASKT_RL			0x00100000
#define FEC_IMASK_UN			0x00080000

#define FEC_RCNTRL_MAX_FL_SHIFT		16
#define FEC_RCNTRL_LOOP			0x00000001
#define FEC_RCNTRL_DRT			0x00000002
#define FEC_RCNTRL_MII_MODE		0x00000004
#define FEC_RCNTRL_PROM			0x00000008
#define FEC_RCNTRL_BC_REJ		0x00000010
#define FEC_RCNTRL_FCE			0x00000020
#define FEC_RCNTRL_RGMII		0x00000040
#define FEC_RCNTRL_RMII			0x00000100
#define FEC_RCNTRL_RMII_10T		0x00000200

#define FEC_TCNTRL_GTS			0x00000001
#define FEC_TCNTRL_HBC			0x00000002
#define FEC_TCNTRL_FDEN			0x00000004
#define FEC_TCNTRL_TFC_PAUSE		0x00000008
#define FEC_TCNTRL_RFC_PAUSE		0x00000010

#define FEC_ECNTRL_RESET		0x00000001	/* reset the FEC */
#define FEC_ECNTRL_ETHER_EN		0x00000002	/* enable the FEC */
#define FEC_ECNTRL_SPEED		0x00000020
#define FEC_ECNTRL_DBSWAP		0x00000100

#define FEC_X_WMRK_STRFWD		0x00000100

#define FEC_X_DES_ACTIVE_TDAR		0x01000000
#define FEC_R_DES_ACTIVE_RDAR		0x01000000

#if defined(CONFIG_MX25) || defined(CONFIG_MX53) || defined(CONFIG_MX6SL)
/* defines for MIIGSK */
/* RMII frequency control: 0=50MHz, 1=5MHz */
#define MIIGSK_CFGR_FRCONT		(1 << 6)
/* loopback mode */
#define MIIGSK_CFGR_LBMODE		(1 << 4)
/* echo mode */
#define MIIGSK_CFGR_EMODE		(1 << 3)
/* MII gasket mode field */
#define MIIGSK_CFGR_IF_MODE_MASK	(3 << 0)
/* MMI/7-Wire mode */
#define MIIGSK_CFGR_IF_MODE_MII		(0 << 0)
/* RMII mode */
#define MIIGSK_CFGR_IF_MODE_RMII	(1 << 0)
/* reflects MIIGSK Enable bit (RO) */
#define MIIGSK_ENR_READY		(1 << 2)
/* enable MIGSK (set by default) */
#define MIIGSK_ENR_EN			(1 << 1)
#endif

/**
 * @brief Receive & Transmit Buffer Descriptor definitions
 *
 * Note: The first BD must be aligned (see DB_ALIGNMENT)
 */
struct fec_bd {
	uint16_t data_length;		/* payload's length in bytes */
	uint16_t status;		/* BD's staus (see datasheet) */
	uint32_t data_pointer;		/* payload's buffer address */
};

/* Supported phy types on this platform */
enum xceiver_type {
	SEVENWIRE,	/* 7-wire       */
	MII10,		/* MII 10Mbps   */
	MII100,		/* MII 100Mbps  */
	RMII,		/* RMII */
	RGMII,		/* RGMII */
};

/* @brief i.MX27-FEC private structure */
struct fec_priv {
	struct ethernet_regs *eth;	/* pointer to register'S base */
	enum xceiver_type xcv_type;	/* transceiver type */
	struct fec_bd *rbd_base;	/* RBD ring */
	int rbd_index;			/* next receive BD to read */
	struct fec_bd *tbd_base;	/* TBD ring */
	int tbd_index;			/* next transmit BD to write */
	bd_t *bd;
	uint8_t *tdb_ptr;
	int dev_id;
	struct mii_dev *bus;
#ifdef CONFIG_PHYLIB
	struct phy_device *phydev;
#else
	int phy_id;
	int (*mii_postcall)(int);
#endif
#ifdef CONFIG_DM_REGULATOR
	struct udevice *phy_supply;
#endif
#ifdef CONFIG_DM_GPIO
	struct gpio_desc phy_reset_gpio;
	uint32_t reset_delay;
	uint32_t reset_post_delay;
#endif
#ifdef CONFIG_DM_ETH
	u32 interface;
#endif
	struct clk ipg_clk;
	u32 clk_rate;
};

void imx_get_mac_from_fuse(int dev_id, unsigned char *mac);

/**
 * @brief Numbers of buffer descriptors for receiving
 *
 * The number defines the stocked memory buffers for the receiving task.
 * Larger values makes no sense in this limited environment.
 */
#define FEC_RBD_NUM		64

/**
 * @brief Define the ethernet packet size limit in memory
 *
 * Note: Do not shrink this number. This will force the FEC to spread larger
 * frames in more than one BD. This is nothing to worry about, but the current
 * driver can't handle it.
 */
#define FEC_MAX_PKT_SIZE	1536

/* Receive BD status bits */
#define FEC_RBD_EMPTY	0x8000	/* Receive BD status: Buffer is empty */
#define FEC_RBD_WRAP	0x2000	/* Receive BD status: Last BD in ring */
/* Receive BD status: Buffer is last in frame (useless here!) */
#define FEC_RBD_LAST	0x0800
#define FEC_RBD_MISS	0x0100	/* Receive BD status: Miss bit for prom mode */
/* Receive BD status: The received frame is broadcast frame */
#define FEC_RBD_BC	0x0080
/* Receive BD status: The received frame is multicast frame */
#define FEC_RBD_MC	0x0040
#define FEC_RBD_LG	0x0020	/* Receive BD status: Frame length violation */
#define FEC_RBD_NO	0x0010	/* Receive BD status: Nonoctet align frame */
#define FEC_RBD_CR	0x0004	/* Receive BD status: CRC error */
#define FEC_RBD_OV	0x0002	/* Receive BD status: Receive FIFO overrun */
#define FEC_RBD_TR	0x0001	/* Receive BD status: Frame is truncated */
#define FEC_RBD_ERR	(FEC_RBD_LG | FEC_RBD_NO | FEC_RBD_CR | \
			FEC_RBD_OV | FEC_RBD_TR)

/* Transmit BD status bits */
#define FEC_TBD_READY	0x8000	/* Tansmit BD status: Buffer is ready */
#define FEC_TBD_WRAP	0x2000	/* Tansmit BD status: Mark as last BD in ring */
#define FEC_TBD_LAST	0x0800	/* Tansmit BD status: Buffer is last in frame */
#define FEC_TBD_TC	0x0400	/* Tansmit BD status: Transmit the CRC */
#define FEC_TBD_ABC	0x0200	/* Tansmit BD status: Append bad CRC */

/* MII-related definitios */
#define FEC_MII_DATA_ST		0x40000000	/* Start of frame delimiter */
#define FEC_MII_DATA_OP_RD	0x20000000	/* Perform a read operation */
#define FEC_MII_DATA_OP_WR	0x10000000	/* Perform a write operation */
#define FEC_MII_DATA_PA_MSK	0x0f800000	/* PHY Address field mask */
#define FEC_MII_DATA_RA_MSK	0x007c0000	/* PHY Register field mask */
#define FEC_MII_DATA_TA		0x00020000	/* Turnaround */
#define FEC_MII_DATA_DATAMSK	0x0000ffff	/* PHY data field */

#define FEC_MII_DATA_RA_SHIFT	18	/* MII Register address bits */
#define FEC_MII_DATA_PA_SHIFT	23	/* MII PHY address bits */

#endif	/* __FEC_MXC_H */
