/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010 Samsung Electronics
 * Naveen Krishna Ch <ch.naveen@samsung.com>
 *
 * Note: This file contains the register description for Memory subsystem
 * 	 (SROM, NAND Flash, OneNand, DDR, OneDRAM) on S5PC1XX.
 *
 * 	 Only SROMC is defined as of now
 */

#ifndef __ASM_ARCH_SROMC_H_
#define __ASM_ARCH_SROMC_H_

#define SMC_DATA16_WIDTH(x)    (1<<((x*4)+0))
#define SMC_BYTE_ADDR_MODE(x)  (1<<((x*4)+1))  /* 0-> Half-word base address*/
						/* 1-> Byte base address*/
#define SMC_WAIT_ENABLE(x)     (1<<((x*4)+2))
#define SMC_BYTE_ENABLE(x)     (1<<((x*4)+3))

#define SMC_BC_TACS(x) (x << 28) /* 0clk     address set-up */
#define SMC_BC_TCOS(x) (x << 24) /* 4clk     chip selection set-up */
#define SMC_BC_TACC(x) (x << 16) /* 14clk    access cycle */
#define SMC_BC_TCOH(x) (x << 12) /* 1clk     chip selection hold */
#define SMC_BC_TAH(x)  (x << 8)  /* 4clk     address holding time */
#define SMC_BC_TACP(x) (x << 4)  /* 6clk     page mode access cycle */
#define SMC_BC_PMC(x)  (x << 0)  /* normal(1data)page mode configuration */

#ifndef __ASSEMBLY__
struct s5p_sromc {
	unsigned int	bw;
	unsigned int	bc[6];
};
#endif	/* __ASSEMBLY__ */

/* Configure the Band Width and Bank Control Regs for required SROMC Bank */
void s5p_config_sromc(u32 srom_bank, u32 srom_bw_conf, u32 srom_bc_conf);

#endif /* __ASM_ARCH_SMC_H_ */
