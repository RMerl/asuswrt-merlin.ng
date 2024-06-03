/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015
 * Texas Instruments Incorporated
 *
 * Lokesh Vutla <lokeshvutla@ti.com>
 */

#ifndef _DRA7_IODELAY_H_
#define _DRA7_IODELAY_H_

#include <common.h>
#include <asm/arch/sys_proto.h>

/* CONFIG_REG_0 */
#define CFG_REG_0_OFFSET		0xC
#define CFG_REG_ROM_READ_SHIFT		1
#define CFG_REG_ROM_READ_MASK		(1 << 1)
#define CFG_REG_CALIB_STRT_SHIFT	0
#define CFG_REG_CALIB_STRT_MASK		(1 << 0)
#define CFG_REG_CALIB_STRT		1
#define CFG_REG_CALIB_END		0
#define CFG_REG_ROM_READ_START		(1 << 1)
#define CFG_REG_ROM_READ_END		(0 << 1)

/* CONFIG_REG_2 */
#define CFG_REG_2_OFFSET		0x14
#define CFG_REG_REFCLK_PERIOD_SHIFT	0
#define CFG_REG_REFCLK_PERIOD_MASK	(0xFFFF << 0)
#define CFG_REG_REFCLK_PERIOD		0x2EF

/* CONFIG_REG_8 */
#define CFG_REG_8_OFFSET		0x2C
#define CFG_IODELAY_UNLOCK_KEY		0x0000AAAA
#define CFG_IODELAY_LOCK_KEY		0x0000AAAB

/* CONFIG_REG_3/4 */
#define CFG_REG_3_OFFSET	0x18
#define CFG_REG_4_OFFSET	0x1C
#define CFG_REG_DLY_CNT_SHIFT	16
#define CFG_REG_DLY_CNT_MASK	(0xFFFF << 16)
#define CFG_REG_REF_CNT_SHIFT	0
#define CFG_REG_REF_CNT_MASK	(0xFFFF << 0)

/* CTRL_CORE_SMA_SW_0 */
#define CTRL_ISOLATE_SHIFT		2
#define CTRL_ISOLATE_MASK		(1 << 2)
#define ISOLATE_IO			1
#define DEISOLATE_IO			0

/* CTRL_CORE_SMA_SW_1 */
#define RGMII2_ID_MODE_N_MASK		(1 << 26)
#define RGMII1_ID_MODE_N_MASK		(1 << 25)

/* PRM_IO_PMCTRL */
#define PMCTRL_ISOCLK_OVERRIDE_SHIFT	0
#define PMCTRL_ISOCLK_OVERRIDE_MASK	(1 << 0)
#define PMCTRL_ISOCLK_STATUS_SHIFT	1
#define PMCTRL_ISOCLK_STATUS_MASK	(1 << 1)
#define PMCTRL_ISOCLK_OVERRIDE_CTRL	1
#define PMCTRL_ISOCLK_NOT_OVERRIDE_CTRL	0

#define ERR_CALIBRATE_IODELAY		0x1
#define ERR_DEISOLATE_IO		0x2
#define ERR_ISOLATE_IO			0x4
#define ERR_UPDATE_DELAY		0x8
#define ERR_CPDE			0x3
#define ERR_FPDE			0x5

/* CFG_XXX */
#define CFG_X_SIGNATURE_SHIFT		12
#define CFG_X_SIGNATURE_MASK		(0x3F << 12)
#define CFG_X_LOCK_SHIFT		10
#define CFG_X_LOCK_MASK			(0x1 << 10)
#define CFG_X_COARSE_DLY_SHIFT		5
#define CFG_X_COARSE_DLY_MASK		(0x1F << 5)
#define CFG_X_FINE_DLY_SHIFT		0
#define CFG_X_FINE_DLY_MASK		(0x1F << 0)
#define CFG_X_SIGNATURE			0x29
#define CFG_X_LOCK			1

void __recalibrate_iodelay(struct pad_conf_entry const *pad, int npads,
			   struct iodelay_cfg_entry const *iodelay,
			   int niodelays);
void late_recalibrate_iodelay(struct pad_conf_entry const *pad, int npads,
			      struct iodelay_cfg_entry const *iodelay,
			      int niodelays);
int __recalibrate_iodelay_start(void);
void __recalibrate_iodelay_end(int ret);

int do_set_iodelay(u32 base, struct iodelay_cfg_entry const *array,
		   int niodelays);
#endif
