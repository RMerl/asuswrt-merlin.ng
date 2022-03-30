/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010 Samsung Electronics
 * Naveen Krishna Ch <ch.naveen@samsung.com>
 *
 * Note: This file contains the register description for SROMC
 */

#ifndef __ASM_ARCH_SROMC_H_
#define __ASM_ARCH_SROMC_H_

#define SROMC_DATA16_WIDTH(x)    (1<<((x*4)+0))
#define SROMC_BYTE_ADDR_MODE(x)  (1<<((x*4)+1))  /* 0-> Half-word base address*/
						/* 1-> Byte base address*/
#define SROMC_WAIT_ENABLE(x)     (1<<((x*4)+2))
#define SROMC_BYTE_ENABLE(x)     (1<<((x*4)+3))

#define SROMC_BC_TACS(x) (x << 28) /* address set-up */
#define SROMC_BC_TCOS(x) (x << 24) /* chip selection set-up */
#define SROMC_BC_TACC(x) (x << 16) /* access cycle */
#define SROMC_BC_TCOH(x) (x << 12) /* chip selection hold */
#define SROMC_BC_TAH(x)  (x << 8)  /* address holding time */
#define SROMC_BC_TACP(x) (x << 4)  /* page mode access cycle */
#define SROMC_BC_PMC(x)  (x << 0)  /* normal(1data)page mode configuration */

#ifndef __ASSEMBLY__
struct s5p_sromc {
	unsigned int	bw;
	unsigned int	bc[4];
};
#endif	/* __ASSEMBLY__ */

/* Configure the Band Width and Bank Control Regs for required SROMC Bank */
void s5p_config_sromc(u32 srom_bank, u32 srom_bw_conf, u32 srom_bc_conf);

enum {
	FDT_SROM_PMC,
	FDT_SROM_TACP,
	FDT_SROM_TAH,
	FDT_SROM_TCOH,
	FDT_SROM_TACC,
	FDT_SROM_TCOS,
	FDT_SROM_TACS,

	FDT_SROM_TIMING_COUNT,
};

struct fdt_sromc {
	u8 bank;	/* srom bank number */
	u8 width;	/* bus width in bytes */
	unsigned int timing[FDT_SROM_TIMING_COUNT]; /* timing parameters */
};

#endif /* __ASM_ARCH_SROMC_H_ */
