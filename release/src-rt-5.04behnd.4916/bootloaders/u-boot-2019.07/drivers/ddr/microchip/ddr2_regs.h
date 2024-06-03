/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (c) 2015 Purna Chandra Mandal <purna.mandal@microchip.com>
 *
 */

#ifndef __MICROCHIP_DDR2_REGS_H
#define __MICROCHIP_DDR2_REGS_H

#include <linux/bitops.h>

/* DDR2 Controller */
struct ddr2_ctrl_regs {
	u32 tsel;
	u32 minlim;
	u32 reqprd;
	u32 mincmd;
	u32 memcon;
	u32 memcfg0;
	u32 memcfg1;
	u32 memcfg2;
	u32 memcfg3;
	u32 memcfg4;
	u32 refcfg;
	u32 pwrcfg;
	u32 dlycfg0;
	u32 dlycfg1;
	u32 dlycfg2;
	u32 dlycfg3;
	u32 odtcfg;
	u32 xfercfg;
	u32 cmdissue;
	u32 odtencfg;
	u32 memwidth;
	u32 unused[11];
	u32 cmd10[16];
	u32 cmd20[16];
};

/* Arbiter Config */
#define MIN_LIM_WIDTH		5
#define RQST_PERIOD_WIDTH	8
#define MIN_CMDACPT_WIDTH	8

/* Refresh Config */
#define REFCNT_CLK(x)		(x)
#define REFDLY_CLK(x)		((x) << 16)
#define MAX_PEND_REF(x)		((x) << 24)

/* Power Config */
#define PRECH_PWR_DN_ONLY(x)	((x) << 22)
#define SELF_REF_DLY(x)		((x) << 12)
#define PWR_DN_DLY(x)		((x) << 4)
#define EN_AUTO_SELF_REF(x)	((x) << 3)
#define EN_AUTO_PWR_DN(x)	((x) << 2)
#define ERR_CORR_EN(x)		((x) << 1)
#define ECC_EN(x)		(x)

/* Memory Width */
#define HALF_RATE_MODE		BIT(3)

/* Delay Config */
#define ODTWLEN(x)	((x) << 20)
#define ODTRLEN(x)	((x) << 16)
#define ODTWDLY(x)	((x) << 12)
#define ODTRDLY(x)	((x) << 8)

/* Xfer Config */
#define BIG_ENDIAN(x)	((x) << 31)
#define MAX_BURST(x)	((x) << 24)
#define RDATENDLY(x)	((x) << 16)
#define NXDATAVDLY(x)	((x) << 4)
#define NXTDATRQDLY(x)	((x) << 0)

/* Host Commands */
#define IDLE_NOP	0x00ffffff
#define PRECH_ALL_CMD	0x00fff401
#define REF_CMD		0x00fff801
#define LOAD_MODE_CMD	0x00fff001
#define CKE_LOW		0x00ffeffe

#define NUM_HOST_CMDS	12

/* Host CMD Issue */
#define CMD_VALID	BIT(4)
#define NUMHOSTCMD(x)	(x)

/* Memory Control */
#define INIT_DONE	BIT(1)
#define INIT_START	BIT(0)

/* Address Control */
#define EN_AUTO_PRECH		0
#define SB_PRI			1

/* DDR2 Phy Register */
struct ddr2_phy_regs {
	u32 scl_start;
	u32 unused1[2];
	u32 scl_latency;
	u32 unused2[2];
	u32 scl_config_1;
	u32 scl_config_2;
	u32 pad_ctrl;
	u32 dll_recalib;
};

/* PHY PAD CONTROL */
#define ODT_SEL			BIT(0)
#define ODT_EN			BIT(1)
#define DRIVE_SEL(x)		((x) << 2)
#define ODT_PULLDOWN(x)		((x) << 4)
#define ODT_PULLUP(x)		((x) << 6)
#define EXTRA_OEN_CLK(x)	((x) << 8)
#define NOEXT_DLL		BIT(9)
#define DLR_DFT_WRCMD		BIT(13)
#define HALF_RATE		BIT(14)
#define DRVSTR_PFET(x)		((x) << 16)
#define DRVSTR_NFET(x)		((x) << 20)
#define RCVR_EN			BIT(28)
#define PREAMBLE_DLY(x)		((x) << 29)

/* PHY DLL RECALIBRATE */
#define RECALIB_CNT(x)		((x) << 8)
#define DISABLE_RECALIB(x)	((x) << 26)
#define DELAY_START_VAL(x)	((x) << 28)

/* PHY SCL CONFIG1 */
#define SCL_BURST8		BIT(0)
#define SCL_DDR_CONNECTED		BIT(1)
#define SCL_RCAS_LAT(x)		((x) << 4)
#define SCL_ODTCSWW		BIT(24)

/* PHY SCL CONFIG2 */
#define SCL_CSEN		BIT(0)
#define SCL_WCAS_LAT(x)		((x) << 8)

/* PHY SCL Latency */
#define SCL_CAPCLKDLY(x)	((x) << 0)
#define SCL_DDRCLKDLY(x)	((x) << 4)

/* PHY SCL START */
#define SCL_START		BIT(28)
#define SCL_EN			BIT(26)
#define SCL_LUBPASS		(BIT(1) | BIT(0))

#endif	/* __MICROCHIP_DDR2_REGS_H */
