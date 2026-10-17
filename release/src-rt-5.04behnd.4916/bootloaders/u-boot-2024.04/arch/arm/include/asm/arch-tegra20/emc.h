/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2010,2011 NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _ARCH_EMC_H_
#define _ARCH_EMC_H_

#include <asm/types.h>

#define TEGRA_EMC_NUM_REGS	46

/* EMC Registers */
struct emc_ctlr {
	u32 cfg;		/* 0x00: EMC_CFG */
	u32 reserved0[3];	/* 0x04 ~ 0x0C */
	u32 adr_cfg;		/* 0x10: EMC_ADR_CFG */
	u32 adr_cfg1;		/* 0x14: EMC_ADR_CFG_1 */
	u32 reserved1[2];	/* 0x18 ~ 0x18 */
	u32 refresh_ctrl;	/* 0x20: EMC_REFCTRL */
	u32 pin;		/* 0x24: EMC_PIN */
	u32 timing_ctrl;	/* 0x28: EMC_TIMING_CONTROL */
	u32 rc;			/* 0x2C: EMC_RC */
	u32 rfc;		/* 0x30: EMC_RFC */
	u32 ras;		/* 0x34: EMC_RAS */
	u32 rp;			/* 0x38: EMC_RP */
	u32 r2w;		/* 0x3C: EMC_R2W */
	u32 w2r;		/* 0x40: EMC_W2R */
	u32 r2p;		/* 0x44: EMC_R2P */
	u32 w2p;		/* 0x48: EMC_W2P */
	u32 rd_rcd;		/* 0x4C: EMC_RD_RCD */
	u32 wd_rcd;		/* 0x50: EMC_WD_RCD */
	u32 rrd;		/* 0x54: EMC_RRD */
	u32 rext;		/* 0x58: EMC_REXT */
	u32 wdv;		/* 0x5C: EMC_WDV */
	u32 quse;		/* 0x60: EMC_QUSE */
	u32 qrst;		/* 0x64: EMC_QRST */
	u32 qsafe;		/* 0x68: EMC_QSAFE */
	u32 rdv;		/* 0x6C: EMC_RDV */
	u32 refresh;		/* 0x70: EMC_REFRESH */
	u32 burst_refresh_num;	/* 0x74: EMC_BURST_REFRESH_NUM */
	u32 pdex2wr;		/* 0x78: EMC_PDEX2WR */
	u32 pdex2rd;		/* 0x7c: EMC_PDEX2RD */
	u32 pchg2pden;		/* 0x80: EMC_PCHG2PDEN */
	u32 act2pden;		/* 0x84: EMC_ACT2PDEN */
	u32 ar2pden;		/* 0x88: EMC_AR2PDEN */
	u32 rw2pden;		/* 0x8C: EMC_RW2PDEN */
	u32 txsr;		/* 0x90: EMC_TXSR */
	u32 tcke;		/* 0x94: EMC_TCKE */
	u32 tfaw;		/* 0x98: EMC_TFAW */
	u32 trpab;		/* 0x9C: EMC_TRPAB */
	u32 tclkstable;		/* 0xA0: EMC_TCLKSTABLE */
	u32 tclkstop;		/* 0xA4: EMC_TCLKSTOP */
	u32 trefbw;		/* 0xA8: EMC_TREFBW */
	u32 quse_extra;		/* 0xAC: EMC_QUSE_EXTRA */
	u32 odt_write;		/* 0xB0: EMC_ODT_WRITE */
	u32 odt_read;		/* 0xB4: EMC_ODT_READ */
	u32 reserved2[5];	/* 0xB8 ~ 0xC8 */
	u32 mrs;		/* 0xCC: EMC_MRS */
	u32 emrs;		/* 0xD0: EMC_EMRS */
	u32 ref;		/* 0xD4: EMC_REF */
	u32 pre;		/* 0xD8: EMC_PRE */
	u32 nop;		/* 0xDC: EMC_NOP */
	u32 self_ref;		/* 0xE0: EMC_SELF_REF */
	u32 dpd;		/* 0xE4: EMC_DPD */
	u32 mrw;		/* 0xE8: EMC_MRW */
	u32 mrr;		/* 0xEC: EMC_MRR */
	u32 reserved3;		/* 0xF0: */
	u32 fbio_cfg1;		/* 0xF4: EMC_FBIO_CFG1 */
	u32 fbio_dqsib_dly;	/* 0xF8: EMC_FBIO_DQSIB_DLY */
	u32 fbio_dqsib_dly_msb;	/* 0xFC: EMC_FBIO_DQSIB_DLY_MSG */
	u32 fbio_spare;		/* 0x100: SBIO_SPARE */
				/* There are more registers ... */
};

/**
 * Set up the EMC for the given rate. The timing parameters are retrieved
 * from the device tree "nvidia,tegra20-emc" node and its
 * "nvidia,tegra20-emc-table" sub-nodes.
 *
 * @param blob	Device tree blob
 * @param rate	Clock speed of memory controller in Hz (=2x memory bus rate)
 * Return: 0 if ok, else -ve error code (look in emc.c to decode it)
 */
int tegra_set_emc(const void *blob, unsigned rate);

/**
 * Get a pointer to the EMC controller from the device tree.
 *
 * @param blob	Device tree blob
 * Return: pointer to EMC controller
 */
struct emc_ctlr *emc_get_controller(const void *blob);

#endif
