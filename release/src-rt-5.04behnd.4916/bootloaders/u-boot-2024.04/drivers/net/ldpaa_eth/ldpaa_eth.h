/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#ifndef __LDPAA_ETH_H
#define __LDPAA_ETH_H

#include <linux/netdevice.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/fsl_dpaa_fd.h>
#include <fsl-mc/fsl_dprc.h>
#include <fsl-mc/fsl_dpni.h>
#include <fsl-mc/fsl_dpbp.h>
#include <fsl-mc/fsl_dpio.h>
#include <fsl-mc/fsl_qbman_portal.h>
#include <fsl-mc/fsl_mc_private.h>


enum ldpaa_eth_type {
	LDPAA_ETH_1G_E,
	LDPAA_ETH_10G_E,
};

/* Arbitrary values for now, but we'll need to tune */
#define LDPAA_ETH_NUM_BUFS		(7 * 7)
#define LDPAA_ETH_REFILL_THRESH		(LDPAA_ETH_NUM_BUFS/2)
#define LDPAA_ETH_RX_BUFFER_SIZE	2048

/* Hardware requires alignment for buffer address and length: 256-byte
 * for ingress, 64-byte for egress. Using 256 for both.
 */
#define LDPAA_ETH_BUF_ALIGN		256

/* So far we're only accomodating a skb backpointer in the frame's
 * software annotation, but the hardware options are either 0 or 64.
 */
#define LDPAA_ETH_SWA_SIZE		64

/* Annotation valid bits in FD FRC */
#define LDPAA_FD_FRC_FASV		0x8000
#define LDPAA_FD_FRC_FAEADV		0x4000
#define LDPAA_FD_FRC_FAPRV		0x2000
#define LDPAA_FD_FRC_FAIADV		0x1000
#define LDPAA_FD_FRC_FASWOV		0x0800
#define LDPAA_FD_FRC_FAICFDV		0x0400

/* Annotation bits in FD CTRL */
#define LDPAA_FD_CTRL_ASAL		0x00020000	/* ASAL = 128 */
#define LDPAA_FD_CTRL_PTA		0x00800000
#define LDPAA_FD_CTRL_PTV1		0x00400000

/* TODO: we may want to move this and other WRIOP related defines
 * to a separate header
 */
/* Frame annotation status */
struct ldpaa_fas {
	u8 reserved;
	u8 ppid;
	__le16 ifpid;
	__le32 status;
} __packed;

/* Debug frame, otherwise supposed to be discarded */
#define LDPAA_ETH_FAS_DISC		0x80000000
/* MACSEC frame */
#define LDPAA_ETH_FAS_MS		0x40000000
#define LDPAA_ETH_FAS_PTP		0x08000000
/* Ethernet multicast frame */
#define LDPAA_ETH_FAS_MC		0x04000000
/* Ethernet broadcast frame */
#define LDPAA_ETH_FAS_BC		0x02000000
#define LDPAA_ETH_FAS_KSE		0x00040000
#define LDPAA_ETH_FAS_EOFHE		0x00020000
#define LDPAA_ETH_FAS_MNLE		0x00010000
#define LDPAA_ETH_FAS_TIDE		0x00008000
#define LDPAA_ETH_FAS_PIEE		0x00004000
/* Frame length error */
#define LDPAA_ETH_FAS_FLE		0x00002000
/* Frame physical error; our favourite pastime */
#define LDPAA_ETH_FAS_FPE		0x00001000
#define LDPAA_ETH_FAS_PTE		0x00000080
#define LDPAA_ETH_FAS_ISP		0x00000040
#define LDPAA_ETH_FAS_PHE		0x00000020
#define LDPAA_ETH_FAS_BLE		0x00000010
/* L3 csum validation performed */
#define LDPAA_ETH_FAS_L3CV		0x00000008
/* L3 csum error */
#define LDPAA_ETH_FAS_L3CE		0x00000004
/* L4 csum validation performed */
#define LDPAA_ETH_FAS_L4CV		0x00000002
/* L4 csum error */
#define LDPAA_ETH_FAS_L4CE		0x00000001
/* These bits always signal errors */
#define LDPAA_ETH_RX_ERR_MASK		(LDPAA_ETH_FAS_DISC	| \
					 LDPAA_ETH_FAS_KSE	| \
					 LDPAA_ETH_FAS_EOFHE	| \
					 LDPAA_ETH_FAS_MNLE	| \
					 LDPAA_ETH_FAS_TIDE	| \
					 LDPAA_ETH_FAS_PIEE	| \
					 LDPAA_ETH_FAS_FLE	| \
					 LDPAA_ETH_FAS_FPE	| \
					 LDPAA_ETH_FAS_PTE	| \
					 LDPAA_ETH_FAS_ISP	| \
					 LDPAA_ETH_FAS_PHE	| \
					 LDPAA_ETH_FAS_BLE	| \
					 LDPAA_ETH_FAS_L3CE	| \
					 LDPAA_ETH_FAS_L4CE)
/* Unsupported features in the ingress */
#define LDPAA_ETH_RX_UNSUPP_MASK	LDPAA_ETH_FAS_MS
/* TODO trim down the bitmask; not all of them apply to Tx-confirm */
#define LDPAA_ETH_TXCONF_ERR_MASK	(LDPAA_ETH_FAS_KSE	| \
					 LDPAA_ETH_FAS_EOFHE	| \
					 LDPAA_ETH_FAS_MNLE	| \
					 LDPAA_ETH_FAS_TIDE)

static const char ldpaa_eth_dpni_stat_strings[][ETH_GSTRING_LEN] = {
	"[dpni ] rx frames",
	"[dpni ] rx bytes",
	"[dpni ] rx mcast frames",
	"[dpni ] rx mcast bytes",
	"[dpni ] rx bcast frames",
	"[dpni ] rx bcast bytes",
	"[dpni ] tx frames",
	"[dpni ] tx bytes",
	"[dpni ] tx mcast frames",
	"[dpni ] tx mcast bytes",
	"[dpni ] tx bcast frames",
	"[dpni ] tx bcast bytes",
	"[dpni ] rx filtered frames",
	"[dpni ] rx discarded frames",
	"[dpni ] rx nobuffer discards",
	"[dpni ] tx discarded frames",
	"[dpni ] tx confirmed frames",
	"[dpni ] tx dequeued bytes",
	"[dpni ] tx dequeued frames",
	"[dpni ] tx rejected bytes",
	"[dpni ] tx rejected frames",
	"[dpni ] tx pending frames",
};

#define LDPAA_ETH_DPNI_NUM_STATS	ARRAY_SIZE(ldpaa_eth_dpni_stat_strings)

static const char ldpaa_eth_dpmac_stat_strings[][ETH_GSTRING_LEN] = {
	[DPMAC_CNT_ING_ALL_FRAME]		= "[mac] rx all frames",
	[DPMAC_CNT_ING_GOOD_FRAME]		= "[mac] rx frames ok",
	[DPMAC_CNT_ING_ERR_FRAME]		= "[mac] rx frame errors",
	[DPMAC_CNT_ING_FRAME_DISCARD]		= "[mac] rx frame discards",
	[DPMAC_CNT_ING_UCAST_FRAME]		= "[mac] rx u-cast",
	[DPMAC_CNT_ING_BCAST_FRAME]		= "[mac] rx b-cast",
	[DPMAC_CNT_ING_MCAST_FRAME]		= "[mac] rx m-cast",
	[DPMAC_CNT_ING_FRAME_64]		= "[mac] rx 64 bytes",
	[DPMAC_CNT_ING_FRAME_127]		= "[mac] rx 65-127 bytes",
	[DPMAC_CNT_ING_FRAME_255]		= "[mac] rx 128-255 bytes",
	[DPMAC_CNT_ING_FRAME_511]		= "[mac] rx 256-511 bytes",
	[DPMAC_CNT_ING_FRAME_1023]		= "[mac] rx 512-1023 bytes",
	[DPMAC_CNT_ING_FRAME_1518]		= "[mac] rx 1024-1518 bytes",
	[DPMAC_CNT_ING_FRAME_1519_MAX]		= "[mac] rx 1519-max bytes",
	[DPMAC_CNT_ING_FRAG]			= "[mac] rx frags",
	[DPMAC_CNT_ING_JABBER]			= "[mac] rx jabber",
	[DPMAC_CNT_ING_ALIGN_ERR]		= "[mac] rx align errors",
	[DPMAC_CNT_ING_OVERSIZED]		= "[mac] rx oversized",
	[DPMAC_CNT_ING_VALID_PAUSE_FRAME]	= "[mac] rx pause",
	[DPMAC_CNT_ING_BYTE]			= "[mac] rx bytes",
	[DPMAC_CNT_EGR_GOOD_FRAME]		= "[mac] tx frames ok",
	[DPMAC_CNT_EGR_UCAST_FRAME]		= "[mac] tx u-cast",
	[DPMAC_CNT_EGR_MCAST_FRAME]		= "[mac] tx m-cast",
	[DPMAC_CNT_EGR_BCAST_FRAME]		= "[mac] tx b-cast",
	[DPMAC_CNT_EGR_ERR_FRAME]		= "[mac] tx frame errors",
	[DPMAC_CNT_EGR_UNDERSIZED]		= "[mac] tx undersized",
	[DPMAC_CNT_EGR_VALID_PAUSE_FRAME]	= "[mac] tx b-pause",
	[DPMAC_CNT_EGR_BYTE]			= "[mac] tx bytes",
};

#define LDPAA_ETH_DPMAC_NUM_STATS	ARRAY_SIZE(ldpaa_eth_dpmac_stat_strings)

struct ldpaa_eth_priv {
	struct phy_device *phy;
	int phy_mode;
	bool started;
	uint32_t dpmac_id;
	uint16_t dpmac_handle;

	uint16_t tx_data_offset;

	uint32_t rx_dflt_fqid;
	uint16_t tx_qdid;
	uint16_t tx_flow_id;

	enum ldpaa_eth_type type;	/* 1G or 10G ethernet */

	/* SW kept statistics */
	u64 dpni_stats[LDPAA_ETH_DPNI_NUM_STATS];
	u64 dpmac_stats[LDPAA_ETH_DPMAC_NUM_STATS];
};

struct dprc_endpoint dpmac_endpoint;
struct dprc_endpoint dpni_endpoint;

extern struct fsl_mc_io *dflt_mc_io;
extern struct fsl_dpbp_obj *dflt_dpbp;
extern struct fsl_dpio_obj *dflt_dpio;
extern struct fsl_dpni_obj *dflt_dpni;
extern uint16_t dflt_dprc_handle;

static void ldpaa_dpbp_drain_cnt(int count);
static void ldpaa_dpbp_drain(void);
static int ldpaa_dpbp_seed(uint16_t bpid);
static void ldpaa_dpbp_free(void);
static int ldpaa_dpni_setup(struct ldpaa_eth_priv *priv);
static int ldpaa_dpbp_setup(void);
static int ldpaa_dpni_bind(struct ldpaa_eth_priv *priv);
static int ldpaa_dpmac_setup(struct ldpaa_eth_priv *priv);
static int ldpaa_dpmac_bind(struct ldpaa_eth_priv *priv);
#endif	/* __LDPAA_H */
