/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

#include "config.h"
#include "asm/types.h"

/* Architecture-specific global data */
struct arch_global_data {
#if defined(CONFIG_FSL_ESDHC)
	u32 sdhc_clk;
#if defined(CONFIG_FSL_ESDHC_ADAPTER_IDENT)
	u8 sdhc_adapter;
#endif
#endif
#if defined(CONFIG_MPC8xx)
	unsigned long brg_clk;
#endif
#if defined(CONFIG_CPM2)
	/* There are many clocks on the MPC8260 - see page 9-5 */
	unsigned long vco_out;
	unsigned long cpm_clk;
	unsigned long scc_clk;
	unsigned long brg_clk;
#endif
	/* TODO: sjg@chromium.org: Should these be unslgned long? */
#if defined(CONFIG_MPC83xx)
#ifdef CONFIG_CLK_MPC83XX
	u32 core_clk;
#else
	/* There are other clocks in the MPC83XX */
	u32 csb_clk;
# if defined(CONFIG_ARCH_MPC8308) || defined(CONFIG_ARCH_MPC831X) || \
	defined(CONFIG_ARCH_MPC834X) || defined(CONFIG_ARCH_MPC837X)
	u32 tsec1_clk;
	u32 tsec2_clk;
	u32 usbdr_clk;
# elif defined(CONFIG_ARCH_MPC8309)
	u32 usbdr_clk;
# endif
# if defined(CONFIG_ARCH_MPC834X)
	u32 usbmph_clk;
# endif /* CONFIG_ARCH_MPC834X */
# if defined(CONFIG_ARCH_MPC8315)
	u32 tdm_clk;
# endif
	u32 core_clk;
	u32 enc_clk;
	u32 lbiu_clk;
	u32 lclk_clk;
# if defined(CONFIG_ARCH_MPC8308) || defined(CONFIG_ARCH_MPC831X) || \
	defined(CONFIG_ARCH_MPC837X)
	u32 pciexp1_clk;
	u32 pciexp2_clk;
# endif
# if defined(CONFIG_ARCH_MPC837X) || defined(CONFIG_ARCH_MPC8315)
	u32 sata_clk;
# endif
# if defined(CONFIG_ARCH_MPC8360)
	u32 mem_sec_clk;
# endif /* CONFIG_ARCH_MPC8360 */
#endif
#endif
#if defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
	u32 lbc_clk;
	void *cpu;
#endif /* CONFIG_MPC85xx || CONFIG_MPC86xx */
#if defined(CONFIG_MPC83xx) || defined(CONFIG_MPC85xx) || \
		defined(CONFIG_MPC86xx)
	u32 i2c1_clk;
	u32 i2c2_clk;
#endif
#if defined(CONFIG_QE)
	u32 qe_clk;
	u32 brg_clk;
	uint mp_alloc_base;
	uint mp_alloc_top;
#endif /* CONFIG_QE */
#if defined(CONFIG_FSL_LAW)
	u32 used_laws;
#endif
#if defined(CONFIG_E500)
	u32 used_tlb_cams[(CONFIG_SYS_NUM_TLBCAMS+31)/32];
#endif
	unsigned long reset_status;	/* reset status register at boot */
#if defined(CONFIG_MPC83xx)
	unsigned long arbiter_event_attributes;
	unsigned long arbiter_event_address;
#endif
#if defined(CONFIG_CPM2)
	unsigned int dp_alloc_base;
	unsigned int dp_alloc_top;
#endif
#ifdef CONFIG_SYS_FPGA_COUNT
	unsigned fpga_state[CONFIG_SYS_FPGA_COUNT];
#endif
#if defined(CONFIG_WD_MAX_RATE)
	unsigned long long wdt_last;	/* trace watch-dog triggering rate */
#endif
#if defined(CONFIG_LWMON5)
	unsigned long kbd_status;
#endif
};

#include <asm-generic/global_data.h>

#if 1
#define DECLARE_GLOBAL_DATA_PTR     register volatile gd_t *gd asm ("r2")
#else /* We could use plain global data, but the resulting code is bigger */
#define XTRN_DECLARE_GLOBAL_DATA_PTR	extern
#define DECLARE_GLOBAL_DATA_PTR     XTRN_DECLARE_GLOBAL_DATA_PTR \
				    gd_t *gd
#endif

#endif /* __ASM_GBL_DATA_H */
