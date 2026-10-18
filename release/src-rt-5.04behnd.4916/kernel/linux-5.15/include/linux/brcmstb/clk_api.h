/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2018 Broadcom */
#ifndef _ASM_BRCMSTB_CLK_API_H
#define _ASM_BRCMSTB_CLK_API_H

#define BCLK_NULL		0xffffffff
#define BCLK_SW_OFFSET		0x5000

/*
 * IF YOU ARE ADDING A PMAP CORE THEN READ THE FOLLOWING:
 *
 * o These constants must be kept in sync with the clk_names_stb[]
 *   array names found in drivers/soc/bcm/brcmstb/nexus/clk.c.
 * o Thse constants must be kept in sync with the constants
 *   in include/linux/brcmstb/reset_api.h.
 * o These constants must be kept in sync with the pmap_cores[]
 *   array names found in drivers/soc/bcm/brcmstb/nexus/dvfs.c.
 * o These constants must be kept in sync with the @a_linux_core_regexps
 *   array in Pmap.pm (from the stbgit-scripts clkgen.pl code).
 * o The following list are Pmap cores.  They may additionally have
 *   the role of SW clock as well.
 * o The following constants are permanent and cannot be changed,
 *   or moved to the 2nd and 3rd lists.
 * o New cores may be added to the end of the following list.
 *   If so, be sure to update the definition of BCLK_SW_NUM_CORES.
 * o Constants from the second list may be "promoted" to the first
 *   list, at the end of course, and Nexus must be recompiled.
 */
#define BCLK_SW_CPU0		(BCLK_SW_OFFSET + 0x0)
#define BCLK_SW_V3D		(BCLK_SW_OFFSET + 0x1)
#define BCLK_SW_SYSIF		(BCLK_SW_OFFSET + 0x2)
#define BCLK_SW_SCB		(BCLK_SW_OFFSET + 0x3)
#define BCLK_SW_HVD0		(BCLK_SW_OFFSET + 0x4)
#define BCLK_SW_RAAGA0		(BCLK_SW_OFFSET + 0x5)
#define BCLK_SW_VICE0		(BCLK_SW_OFFSET + 0x6)
#define BCLK_SW_VICE0_PSS	(BCLK_SW_OFFSET + 0x7)
#define BCLK_SW_VICE1		(BCLK_SW_OFFSET + 0x8)
#define BCLK_SW_VICE1_PSS	(BCLK_SW_OFFSET + 0x9)
#define BCLK_SW_XPT		(BCLK_SW_OFFSET + 0xa)
#define BCLK_SW_M2MC0		(BCLK_SW_OFFSET + 0xb)
#define BCLK_SW_M2MC1		(BCLK_SW_OFFSET + 0xc)
#define BCLK_SW_MIPMAP0		(BCLK_SW_OFFSET + 0xd)
#define BCLK_SW_TSX0		(BCLK_SW_OFFSET + 0xe)
#define BCLK_SW_SMARTCARD0	(BCLK_SW_OFFSET + 0xf)
#define BCLK_SW_SMARTCARD1	(BCLK_SW_OFFSET + 0x10)
#define BCLK_SW_VPU0		(BCLK_SW_OFFSET + 0x11)
#define BCLK_SW_BNE0		(BCLK_SW_OFFSET + 0x12)
#define BCLK_SW_ASP0		(BCLK_SW_OFFSET + 0x13)
#define BCLK_SW_HVD_CABAC0	(BCLK_SW_OFFSET + 0x14)
#define BCLK_SW_AXI0		(BCLK_SW_OFFSET + 0x15)
#define BCLK_SW_BSTM0		(BCLK_SW_OFFSET + 0x16)
#define BCLK_SW_CPU1		(BCLK_SW_OFFSET + 0x17)
#define BCLK_SW_CPU2		(BCLK_SW_OFFSET + 0x18)
#define BCLK_SW_CPU3		(BCLK_SW_OFFSET + 0x19)
/* Do not use as clock; use BCLK_SW_DVPHT instead  */
#define BCLK_SW_DVPHT_CORE	(BCLK_SW_OFFSET + 0x1a)
#define BCLK_SW_HVS0		(BCLK_SW_OFFSET + 0x1b)
/*
 * IF YOU ADD A CLK/CORE ABOVE, PLEASE UPDATE BELOW AND
 * FOLLOW THE BULLET LIST ABOVE.
 */
#define BCLK_SW_NUM_CORES	(BCLK_SW_HVS0 - BCLK_SW_OFFSET + 1)

/* Keep some space reserved for future cores.  */

/*
 * o Entries from the following lists may be promoted
 *   to the core list above if they also become a core.
 * o If you add to this list, or the SRAM list below,
 *   be sure to enter the clock's name in the
 *   clk_names_stb[] array in
 *       drivers/soc/bcm/brcmstb/nexus/clk.c.
 */
#define BCLK_SW_AIO		(BCLK_SW_OFFSET + 0x30)
#define BCLK_SW_BVN		(BCLK_SW_OFFSET + 0x31)
#define BCLK_SW_DVPHR		(BCLK_SW_OFFSET + 0x32)
/* Do not use BCLK_SW_DVPHT as a core; use BCLK_SW_DVPHT_CORE instead  */
#define BCLK_SW_DVPHT		(BCLK_SW_OFFSET + 0x33)
#define BCLK_SW_GENET0		(BCLK_SW_OFFSET + 0x34)
#define BCLK_SW_GENETWOL0	(BCLK_SW_OFFSET + 0x35)
#define BCLK_SW_HVD0_CPU	(BCLK_SW_OFFSET + 0x36)
#define BCLK_SW_ITU656		(BCLK_SW_OFFSET + 0x37)
#define BCLK_SW_MMM2MC0		(BCLK_SW_OFFSET + 0x38)
#define BCLK_SW_PCIE0		(BCLK_SW_OFFSET + 0x39)
#define BCLK_SW_PCIE1		(BCLK_SW_OFFSET + 0x3a)
#define BCLK_SW_POTP		(BCLK_SW_OFFSET + 0x3b)
#define BCLK_SW_RAAGA0_CPU	(BCLK_SW_OFFSET + 0x3c)
#define BCLK_SW_SATA3		(BCLK_SW_OFFSET + 0x3d)
#define BCLK_SW_SDIO0		(BCLK_SW_OFFSET + 0x3e)
#define BCLK_SW_SDIO1		(BCLK_SW_OFFSET + 0x3f)
#define BCLK_SW_SID		(BCLK_SW_OFFSET + 0x40)
#define BCLK_SW_V3D_CPU		(BCLK_SW_OFFSET + 0x41)
#define BCLK_SW_VEC		(BCLK_SW_OFFSET + 0x42)
#define BCLK_SW_XPT_WAKEUP	(BCLK_SW_OFFSET + 0x43)
#define BCLK_SW_TSIO		(BCLK_SW_OFFSET + 0x44)
#define BCLK_SW_MBVN		(BCLK_SW_OFFSET + 0x45)
#define BCLK_SW_RAAGA0_WAKEUP	(BCLK_SW_OFFSET + 0x46)

/* Let's keep the SRAM clocks in their own group */
#define BCLK_SW_AIO_SRAM	(BCLK_SW_OFFSET + 0x60)
#define BCLK_SW_BVN_SRAM	(BCLK_SW_OFFSET + 0x61)
#define BCLK_SW_DVPHR_SRAM	(BCLK_SW_OFFSET + 0x62)
#define BCLK_SW_HVD0_SRAM	(BCLK_SW_OFFSET + 0x63)
#define BCLK_SW_M2MC0_SRAM	(BCLK_SW_OFFSET + 0x64)
#define BCLK_SW_M2MC1_SRAM	(BCLK_SW_OFFSET + 0x65)
#define BCLK_SW_MMM2MC0_SRAM	(BCLK_SW_OFFSET + 0x66)
#define BCLK_SW_RAAGA0_SRAM	(BCLK_SW_OFFSET + 0x67)
#define BCLK_SW_V3D_SRAM	(BCLK_SW_OFFSET + 0x68)
#define BCLK_SW_VEC_SRAM	(BCLK_SW_OFFSET + 0x69)
#define BCLK_SW_VICE0_SRAM	(BCLK_SW_OFFSET + 0x6a)
#define BCLK_SW_VICE1_SRAM	(BCLK_SW_OFFSET + 0x6b)
#define BCLK_SW_XPT_SRAM	(BCLK_SW_OFFSET + 0x6c)

#if IS_ENABLED(CONFIG_BRCMSTB_NEXUS_CLK_API)
int brcm_clk_prepare_enable(unsigned int clk_id);
void  brcm_clk_disable_unprepare(unsigned int clk_id);
int brcm_clk_get_rate(unsigned int clk_id, u64 *rate);

int brcm_pmap_show(void);
int brcm_pmap_num_pstates(unsigned int core_id, unsigned int *num_pstates);
int brcm_pmap_get_pstate(unsigned int core_id, unsigned int *pstate);
int brcm_pmap_set_pstate(unsigned int core_id, unsigned int pstate);
/* Returns the frequencies in kHz of pstates [0..N-1] */
int brcm_pmap_get_pstate_freqs(unsigned int core_id, unsigned int *freqs);

#else
static inline int brcm_clk_prepare_enable(unsigned int clk_id)
{
	return -ENOTSUPP;
}
static inline void  brcm_clk_disable_unprepare(unsigned int clk_id)
{
}

static inline int brcm_pmap_show(void)
{
	return -ENOTSUPP;
}

static inline int brcm_pmap_num_pstates(unsigned int core_id,
					unsigned int *num_pstates)
{
	return -ENOTSUPP;
}

static inline int brcm_pmap_get_pstate(unsigned int core_id,
				       unsigned int *pstate)
{
	return -ENOTSUPP;
}

static inline int brcm_pmap_set_pstate(unsigned int core_id,
				       unsigned int pstate)
{
	return -ENOTSUPP;
}

static inline int brcm_pmap_get_pstate_freqs(unsigned int core_id,
					     uint32_t *freqs)
{
	return -ENOTSUPP;
}
#endif
#endif
