/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2012-2017 Altera Corporation <www.altera.com>
 * All rights reserved.
 */

#ifndef _FPGA_MANAGER_GEN5_H_
#define _FPGA_MANAGER_GEN5_H_

#define FPGAMGRREGS_STAT_MODE_MASK		0x7
#define FPGAMGRREGS_STAT_MSEL_MASK		0xf8
#define FPGAMGRREGS_STAT_MSEL_LSB		3

#define FPGAMGRREGS_CTRL_CFGWDTH_MASK		BIT(9)
#define FPGAMGRREGS_CTRL_AXICFGEN_MASK		BIT(8)
#define FPGAMGRREGS_CTRL_NCONFIGPULL_MASK	BIT(2)
#define FPGAMGRREGS_CTRL_NCE_MASK		BIT(1)
#define FPGAMGRREGS_CTRL_EN_MASK		BIT(0)
#define FPGAMGRREGS_CTRL_CDRATIO_LSB		6

#define FPGAMGRREGS_MON_GPIO_EXT_PORTA_CRC_MASK	BIT(3)
#define FPGAMGRREGS_MON_GPIO_EXT_PORTA_ID_MASK	BIT(2)
#define FPGAMGRREGS_MON_GPIO_EXT_PORTA_CD_MASK	BIT(1)
#define FPGAMGRREGS_MON_GPIO_EXT_PORTA_NS_MASK	BIT(0)

/* FPGA Mode */
#define FPGAMGRREGS_MODE_FPGAOFF		0x0
#define FPGAMGRREGS_MODE_RESETPHASE		0x1
#define FPGAMGRREGS_MODE_CFGPHASE		0x2
#define FPGAMGRREGS_MODE_INITPHASE		0x3
#define FPGAMGRREGS_MODE_USERMODE		0x4
#define FPGAMGRREGS_MODE_UNKNOWN		0x5

#ifndef __ASSEMBLY__

struct socfpga_fpga_manager {
	/* FPGA Manager Module */
	u32	stat;			/* 0x00 */
	u32	ctrl;
	u32	dclkcnt;
	u32	dclkstat;
	u32	gpo;			/* 0x10 */
	u32	gpi;
	u32	misci;			/* 0x18 */
	u32	_pad_0x1c_0x82c[517];

	/* Configuration Monitor (MON) Registers */
	u32	gpio_inten;		/* 0x830 */
	u32	gpio_intmask;
	u32	gpio_inttype_level;
	u32	gpio_int_polarity;
	u32	gpio_intstatus;		/* 0x840 */
	u32	gpio_raw_intstatus;
	u32	_pad_0x848;
	u32	gpio_porta_eoi;
	u32	gpio_ext_porta;		/* 0x850 */
	u32	_pad_0x854_0x85c[3];
	u32	gpio_1s_sync;		/* 0x860 */
	u32	_pad_0x864_0x868[2];
	u32	gpio_ver_id_code;
	u32	gpio_config_reg2;	/* 0x870 */
	u32	gpio_config_reg1;
};

#endif /* __ASSEMBLY__ */

#endif /* _FPGA_MANAGER_GEN5_H_ */
