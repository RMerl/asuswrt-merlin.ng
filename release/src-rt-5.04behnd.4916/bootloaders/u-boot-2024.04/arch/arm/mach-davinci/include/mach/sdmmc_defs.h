/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Davinci MMC Controller Defines - Based on Linux davinci_mmc.c
 *
 * Copyright (C) 2010 Texas Instruments Incorporated
 */

#ifndef _SDMMC_DEFS_H_
#define _SDMMC_DEFS_H_

#include <asm/arch/hardware.h>

/* MMC Control Reg fields */
#define MMCCTL_DATRST		(1 << 0)
#define MMCCTL_CMDRST		(1 << 1)
#define MMCCTL_WIDTH_4_BIT	(1 << 2)
#define MMCCTL_DATEG_DISABLED	(0 << 6)
#define MMCCTL_DATEG_RISING	(1 << 6)
#define MMCCTL_DATEG_FALLING	(2 << 6)
#define MMCCTL_DATEG_BOTH	(3 << 6)
#define MMCCTL_PERMDR_LE	(0 << 9)
#define MMCCTL_PERMDR_BE	(1 << 9)
#define MMCCTL_PERMDX_LE	(0 << 10)
#define MMCCTL_PERMDX_BE	(1 << 10)

/* MMC Clock Control Reg fields */
#define MMCCLK_CLKEN		(1 << 8)
#define MMCCLK_CLKRT_MASK	(0xFF << 0)

/* MMC Status Reg0 fields */
#define MMCST0_DATDNE		(1 << 0)
#define MMCST0_BSYDNE		(1 << 1)
#define MMCST0_RSPDNE		(1 << 2)
#define MMCST0_TOUTRD		(1 << 3)
#define MMCST0_TOUTRS		(1 << 4)
#define MMCST0_CRCWR		(1 << 5)
#define MMCST0_CRCRD		(1 << 6)
#define MMCST0_CRCRS		(1 << 7)
#define MMCST0_DXRDY		(1 << 9)
#define MMCST0_DRRDY		(1 << 10)
#define MMCST0_DATED		(1 << 11)
#define MMCST0_TRNDNE		(1 << 12)

#define MMCST0_ERR_MASK		(0x00F8)

/* MMC Status Reg1 fields */
#define MMCST1_BUSY		(1 << 0)
#define MMCST1_CLKSTP		(1 << 1)
#define MMCST1_DXEMP		(1 << 2)
#define MMCST1_DRFUL		(1 << 3)
#define MMCST1_DAT3ST		(1 << 4)
#define MMCST1_FIFOEMP		(1 << 5)
#define MMCST1_FIFOFUL		(1 << 6)

/* MMC INT Mask Reg fields */
#define MMCIM_EDATDNE		(1 << 0)
#define MMCIM_EBSYDNE		(1 << 1)
#define MMCIM_ERSPDNE		(1 << 2)
#define MMCIM_ETOUTRD		(1 << 3)
#define MMCIM_ETOUTRS		(1 << 4)
#define MMCIM_ECRCWR		(1 << 5)
#define MMCIM_ECRCRD		(1 << 6)
#define MMCIM_ECRCRS		(1 << 7)
#define MMCIM_EDXRDY		(1 << 9)
#define MMCIM_EDRRDY		(1 << 10)
#define MMCIM_EDATED		(1 << 11)
#define MMCIM_ETRNDNE		(1 << 12)

#define MMCIM_MASKALL		(0xFFFFFFFF)

/* MMC Resp Tout Reg fields */
#define MMCTOR_TOR_MASK		(0xFF)		/* dont write to reg, | it */
#define MMCTOR_TOD_20_16_SHIFT  (8)

/* MMC Data Read Tout Reg fields */
#define MMCTOD_TOD_0_15_MASK	(0xFFFF)

/* MMC Block len Reg fields */
#define MMCBLEN_BLEN_MASK	(0xFFF)

/* MMC Num Blocks Reg fields */
#define MMCNBLK_NBLK_MASK	(0xFFFF)
#define MMCNBLK_NBLK_MAX	(0xFFFF)

/* MMC Num Blocks Counter Reg fields */
#define MMCNBLC_NBLC_MASK	(0xFFFF)

/* MMC Cmd Reg fields */
#define MMCCMD_CMD_MASK		(0x3F)
#define MMCCMD_PPLEN		(1 << 7)
#define MMCCMD_BSYEXP		(1 << 8)
#define MMCCMD_RSPFMT_NONE	(0 << 9)
#define MMCCMD_RSPFMT_R1567	(1 << 9)
#define MMCCMD_RSPFMT_R2	(2 << 9)
#define MMCCMD_RSPFMT_R3	(3 << 9)
#define MMCCMD_DTRW		(1 << 11)
#define MMCCMD_STRMTP		(1 << 12)
#define MMCCMD_WDATX		(1 << 13)
#define MMCCMD_INITCK		(1 << 14)
#define MMCCMD_DCLR		(1 << 15)
#define MMCCMD_DMATRIG		(1 << 16)

/* FIFO control Reg fields */
#define MMCFIFOCTL_FIFORST	(1 << 0)
#define MMCFIFOCTL_FIFODIR	(1 << 1)
#define MMCFIFOCTL_FIFOLEV	(1 << 2)
#define MMCFIFOCTL_ACCWD_4	(0 << 3)	/* access width of 4 bytes */
#define MMCFIFOCTL_ACCWD_3	(1 << 3)	/* access width of 3 bytes */
#define MMCFIFOCTL_ACCWD_2	(2 << 3)	/* access width of 2 bytes */
#define MMCFIFOCTL_ACCWD_1	(3 << 3)	/* access width of 1 byte */

/* Davinci MMC Register definitions */
struct davinci_mmc_regs {
	dv_reg mmcctl;
	dv_reg mmcclk;
	dv_reg mmcst0;
	dv_reg mmcst1;
	dv_reg mmcim;
	dv_reg mmctor;
	dv_reg mmctod;
	dv_reg mmcblen;
	dv_reg mmcnblk;
	dv_reg mmcnblc;
	dv_reg mmcdrr;
	dv_reg mmcdxr;
	dv_reg mmccmd;
	dv_reg mmcarghl;
	dv_reg mmcrsp01;
	dv_reg mmcrsp23;
	dv_reg mmcrsp45;
	dv_reg mmcrsp67;
	dv_reg mmcdrsp;
	dv_reg mmcetok;
	dv_reg mmccidx;
	dv_reg mmcckc;
	dv_reg mmctorc;
	dv_reg mmctodc;
	dv_reg mmcblnc;
	dv_reg sdioctl;
	dv_reg sdiost0;
	dv_reg sdioien;
	dv_reg sdioist;
	dv_reg mmcfifoctl;
};

/* Davinci MMC board definitions */
struct davinci_mmc {
	struct davinci_mmc_regs *reg_base;	/* Register base address */
	uint input_clk;		/* Input clock to MMC controller */
	uint host_caps;		/* Host capabilities */
	uint voltages;		/* Host supported voltages */
	struct mmc_config cfg;
};

#define DAVINCI_MAX_BLOCKS	(32)
struct davinci_mmc_plat {
	struct davinci_mmc_regs *reg_base;	/* Register base address */
	struct mmc_config cfg;
	struct mmc mmc;
};

int davinci_mmc_init(struct bd_info *bis, struct davinci_mmc *host);

#endif /* _SDMMC_DEFS_H */
