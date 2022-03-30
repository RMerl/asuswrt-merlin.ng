/*
 * Copyright 2003 Motorola,Inc.
 * Xianghua Xiao(x.xiao@motorola.com)
 */

#ifndef	__E500_H__
#define __E500_H__

#ifndef __ASSEMBLY__

typedef struct
{
	unsigned long freq_processor[CONFIG_MAX_CPUS];
#ifdef CONFIG_HETROGENOUS_CLUSTERS
	unsigned long freq_processor_dsp[CONFIG_MAX_DSP_CPUS];
#endif
	unsigned long freq_systembus;
	unsigned long freq_ddrbus;
	unsigned long freq_localbus;
	unsigned long freq_qe;
	unsigned long freq_sdhc;
#ifdef CONFIG_SYS_DPAA_FMAN
	unsigned long freq_fman[CONFIG_SYS_NUM_FMAN];
#endif
#ifdef CONFIG_SYS_DPAA_QBMAN
	unsigned long freq_qman;
#endif
#ifdef CONFIG_SYS_DPAA_PME
	unsigned long freq_pme;
#endif
#ifdef CONFIG_SYS_CPRI
	unsigned long freq_cpri;
#endif
#ifdef CONFIG_SYS_MAPLE
	unsigned long freq_maple;
	unsigned long freq_maple_ulb;
	unsigned long freq_maple_etvpe;
#endif
#ifdef CONFIG_SYS_FSL_SINGLE_SOURCE_CLK
	unsigned char diff_sysclk;
#endif
} MPC85xx_SYS_INFO;

#endif  /* _ASMLANGUAGE */

#define RESET_VECTOR	0xfffffffc

#endif	/* __E500_H__ */
