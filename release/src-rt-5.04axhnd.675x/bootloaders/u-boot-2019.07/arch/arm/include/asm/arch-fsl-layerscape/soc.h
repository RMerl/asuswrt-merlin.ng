/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 * Copyright 2015 Freescale Semiconductor
 */

#ifndef _ASM_ARMV8_FSL_LAYERSCAPE_SOC_H_
#define _ASM_ARMV8_FSL_LAYERSCAPE_SOC_H_

#ifndef __ASSEMBLY__
#include <linux/types.h>
#ifdef CONFIG_FSL_LSCH2
#include <asm/arch/immap_lsch2.h>
#endif
#ifdef CONFIG_FSL_LSCH3
#include <asm/arch/immap_lsch3.h>
#endif
#endif

#ifdef CONFIG_SYS_FSL_CCSR_GUR_LE
#define gur_in32(a)       in_le32(a)
#define gur_out32(a, v)   out_le32(a, v)
#elif defined(CONFIG_SYS_FSL_CCSR_GUR_BE)
#define gur_in32(a)       in_be32(a)
#define gur_out32(a, v)   out_be32(a, v)
#endif

#ifdef CONFIG_SYS_FSL_CCSR_SCFG_LE
#define scfg_in32(a)       in_le32(a)
#define scfg_out32(a, v)   out_le32(a, v)
#define scfg_clrbits32(addr, clear) clrbits_le32(addr, clear)
#define scfg_clrsetbits32(addr, clear, set) clrsetbits_le32(addr, clear, set)
#elif defined(CONFIG_SYS_FSL_CCSR_SCFG_BE)
#define scfg_in32(a)       in_be32(a)
#define scfg_out32(a, v)   out_be32(a, v)
#define scfg_clrbits32(addr, clear) clrbits_be32(addr, clear)
#define scfg_clrsetbits32(addr, clear, set) clrsetbits_be32(addr, clear, set)
#endif

#ifdef CONFIG_SYS_FSL_PEX_LUT_LE
#define pex_lut_in32(a)       in_le32(a)
#define pex_lut_out32(a, v)   out_le32(a, v)
#elif defined(CONFIG_SYS_FSL_PEX_LUT_BE)
#define pex_lut_in32(a)       in_be32(a)
#define pex_lut_out32(a, v)   out_be32(a, v)
#endif
#ifndef __ASSEMBLY__
struct cpu_type {
	char name[15];
	u32 soc_ver;
	u32 num_cores;
};

#define CPU_TYPE_ENTRY(n, v, nc) \
	{ .name = #n, .soc_ver = SVR_##v, .num_cores = (nc)}

#ifdef CONFIG_TFABOOT
#define SMC_DRAM_BANK_INFO (0xC200FF12)
#define SIP_SVC_RCW	0xC200FF18

phys_size_t tfa_get_dram_size(void);

enum boot_src {
	BOOT_SOURCE_RESERVED = 0,
	BOOT_SOURCE_IFC_NOR,
	BOOT_SOURCE_IFC_NAND,
	BOOT_SOURCE_QSPI_NOR,
	BOOT_SOURCE_QSPI_NAND,
	BOOT_SOURCE_XSPI_NOR,
	BOOT_SOURCE_XSPI_NAND,
	BOOT_SOURCE_SD_MMC,
	BOOT_SOURCE_SD_MMC2,
	BOOT_SOURCE_I2C1_EXTENDED,
};

enum boot_src get_boot_src(void);
#endif
#endif
#define SVR_WO_E		0xFFFFFE
#define SVR_LS1012A		0x870400
#define SVR_LS1043A		0x879200
#define SVR_LS1023A		0x879208
/* LS1043A/LS1023A 23x23 package silicon has different value of VAR_PER */
#define SVR_LS1043A_P23		0x879202
#define SVR_LS1023A_P23		0x87920A
#define SVR_LS1028A		0x870B00
#define SVR_LS1046A		0x870700
#define SVR_LS1026A		0x870708
#define SVR_LS1048A		0x870320
#define SVR_LS1084A		0x870302
#define SVR_LS1088A		0x870300
#define SVR_LS1044A		0x870322
#define SVR_LS2045A		0x870120
#define SVR_LS2080A		0x870110
#define SVR_LS2085A		0x870100
#define SVR_LS2040A		0x870130
#define SVR_LS2088A		0x870900
#define SVR_LS2084A		0x870910
#define SVR_LS2048A		0x870920
#define SVR_LS2044A		0x870930
#define SVR_LS2081A		0x870918
#define SVR_LS2041A		0x870914
#define SVR_LX2160A		0x873601
#define SVR_LX2120A		0x873621
#define SVR_LX2080A		0x873603

#define SVR_MAJ(svr)		(((svr) >> 4) & 0xf)
#define SVR_MIN(svr)		(((svr) >> 0) & 0xf)
#define SVR_REV(svr)		(((svr) >> 0) & 0xff)
#define SVR_SOC_VER(svr)	(((svr) >> 8) & SVR_WO_E)
#define IS_E_PROCESSOR(svr)	(!((svr >> 8) & 0x1))
#ifdef CONFIG_ARCH_LX2160A
#define IS_C_PROCESSOR(svr)	(!((svr >> 12) & 0x1))
#endif
#define IS_SVR_REV(svr, maj, min) \
		((SVR_MAJ(svr) == (maj)) && (SVR_MIN(svr) == (min)))
#define SVR_DEV(svr)		((svr) >> 8)
#define IS_SVR_DEV(svr, dev)	(((svr) >> 16) == (dev))

#ifndef __ASSEMBLY__
#ifdef CONFIG_FSL_LSCH3
void fsl_lsch3_early_init_f(void);
int get_core_volt_from_fuse(void);
#elif defined(CONFIG_FSL_LSCH2)
void fsl_lsch2_early_init_f(void);
int setup_chip_volt(void);
/* Setup core vdd in unit mV */
int board_setup_core_volt(u32 vdd);
#ifdef CONFIG_FSL_PFE
void init_pfe_scfg_dcfg_regs(void);
#endif
#endif
#ifdef CONFIG_QSPI_AHB_INIT
int qspi_ahb_init(void);
#endif

void cpu_name(char *name);
#ifdef CONFIG_SYS_FSL_ERRATUM_A009635
void erratum_a009635(void);
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_A010315
void erratum_a010315(void);
#endif

bool soc_has_dp_ddr(void);
bool soc_has_aiop(void);
#endif

#endif /* _ASM_ARMV8_FSL_LAYERSCAPE_SOC_H_ */
