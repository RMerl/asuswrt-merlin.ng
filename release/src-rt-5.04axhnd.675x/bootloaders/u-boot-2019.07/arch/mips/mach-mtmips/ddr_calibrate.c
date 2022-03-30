// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Stefan Roese <sr@denx.de>
 *
 * This code is mostly based on the code extracted from this MediaTek
 * github repository:
 *
 * https://github.com/MediaTek-Labs/linkit-smart-uboot.git
 *
 * I was not able to find a specific license or other developers
 * copyrights here, so I can't add them here.
 *
 * Most functions in this file are copied from the MediaTek U-Boot
 * repository. Without any documentation, it was impossible to really
 * implement this differently. So its mostly a cleaned-up version of
 * the original code, with only support for the MT7628 / MT7688 SoC.
 */

#include <common.h>
#include <linux/io.h>
#include <asm/cacheops.h>
#include <asm/io.h>
#include "mt76xx.h"

#define NUM_OF_CACHELINE	128
#define MIN_START		6
#define MIN_FINE_START		0xf
#define MAX_START		7
#define MAX_FINE_START		0x0

#define CPU_FRAC_DIV		1

#if defined(CONFIG_ONBOARD_DDR2_SIZE_256MBIT)
#define DRAM_BUTTOM 0x02000000
#endif
#if defined(CONFIG_ONBOARD_DDR2_SIZE_512MBIT)
#define DRAM_BUTTOM 0x04000000
#endif
#if defined(CONFIG_ONBOARD_DDR2_SIZE_1024MBIT)
#define DRAM_BUTTOM 0x08000000
#endif
#if defined(CONFIG_ONBOARD_DDR2_SIZE_2048MBIT)
#define DRAM_BUTTOM 0x10000000
#endif

static inline void cal_memcpy(void *src, void *dst, u32 size)
{
	u8 *psrc = (u8 *)src;
	u8 *pdst = (u8 *)dst;
	int i;

	for (i = 0; i < size; i++, psrc++, pdst++)
		*pdst = *psrc;
}

static inline void cal_memset(void *src, u8 pat, u32 size)
{
	u8 *psrc = (u8 *)src;
	int i;

	for (i = 0; i < size; i++, psrc++)
		*psrc = pat;
}

#define pref_op(hint, addr)						\
	__asm__ __volatile__(						\
		".set	push\n"						\
		".set	noreorder\n"					\
		"pref	%0, %1\n"					\
		".set	pop\n"						\
		:							\
		: "i" (hint), "R" (*(u8 *)(addr)))

static inline void cal_patgen(u32 start_addr, u32 size, u32 bias)
{
	u32 *addr = (u32 *)start_addr;
	int i;

	for (i = 0; i < size; i++)
		addr[i] = start_addr + i + bias;
}

static inline int test_loop(int k, int dqs, u32 test_dqs, u32 *coarse_dqs,
			    u32 offs, u32 pat, u32 val)
{
	u32 nc_addr;
	u32 *c_addr;
	int i;

	for (nc_addr = 0xa0000000;
	     nc_addr < (0xa0000000 + DRAM_BUTTOM - NUM_OF_CACHELINE * 32);
	     nc_addr += (DRAM_BUTTOM >> 6) + offs) {
		writel(0x00007474, (void *)MT76XX_MEMCTRL_BASE + 0x64);
		wmb();		/* Make sure store if finished */

		c_addr = (u32 *)(nc_addr & 0xdfffffff);
		cal_memset(((u8 *)c_addr), 0x1F, NUM_OF_CACHELINE * 32);
		cal_patgen(nc_addr, NUM_OF_CACHELINE * 8, pat);

		if (dqs > 0)
			writel(0x00000074 |
			       (((k == 1) ? coarse_dqs[dqs] : test_dqs) << 12) |
			       (((k == 0) ? val : test_dqs) << 8),
			       (void *)MT76XX_MEMCTRL_BASE + 0x64);
		else
			writel(0x00007400 |
			       (((k == 1) ? coarse_dqs[dqs] : test_dqs) << 4) |
			       (((k == 0) ? val : test_dqs) << 0),
			       (void *)MT76XX_MEMCTRL_BASE + 0x64);
		wmb();		/* Make sure store if finished */

		invalidate_dcache_range((u32)c_addr,
					(u32)c_addr +
					NUM_OF_CACHELINE * 32);
		wmb();		/* Make sure store if finished */

		for (i = 0; i < NUM_OF_CACHELINE * 8; i++) {
			if (i % 8 == 0)
				pref_op(0, &c_addr[i]);
		}

		for (i = 0; i < NUM_OF_CACHELINE * 8; i++) {
			if (c_addr[i] != nc_addr + i + pat)
				return -1;
		}
	}

	return 0;
}

void ddr_calibrate(void)
{
	u32 min_coarse_dqs[2];
	u32 max_coarse_dqs[2];
	u32 min_fine_dqs[2];
	u32 max_fine_dqs[2];
	u32 coarse_dqs[2];
	u32 fine_dqs[2];
	int reg = 0, ddr_cfg2_reg;
	int flag;
	int i, k;
	int dqs = 0;
	u32 min_coarse_dqs_bnd, min_fine_dqs_bnd, coarse_dqs_dll, fine_dqs_dll;
	u32 val;
	u32 fdiv = 0, frac = 0;

	/* Setup clock to run at full speed */
	val = readl((void *)MT76XX_DYN_CFG0_REG);
	fdiv = (u32)((val >> 8) & 0x0F);
	if (CPU_FRAC_DIV < 1 || CPU_FRAC_DIV > 10)
		frac = val & 0x0f;
	else
		frac = CPU_FRAC_DIV;

	while (frac < fdiv) {
		val = readl((void *)MT76XX_DYN_CFG0_REG);
		fdiv = (val >> 8) & 0x0f;
		fdiv--;
		val &= ~(0x0f << 8);
		val |= (fdiv << 8);
		writel(val, (void *)MT76XX_DYN_CFG0_REG);
		udelay(500);
		val = readl((void *)MT76XX_DYN_CFG0_REG);
		fdiv = (val >> 8) & 0x0f;
	}

	clrbits_le32((void *)MT76XX_MEMCTRL_BASE + 0x10, BIT(4));
	ddr_cfg2_reg = readl((void *)MT76XX_MEMCTRL_BASE + 0x48);
	clrbits_le32((void *)MT76XX_MEMCTRL_BASE + 0x48,
		     (0x3 << 28) | (0x3 << 26));

	min_coarse_dqs[0] = MIN_START;
	min_coarse_dqs[1] = MIN_START;
	min_fine_dqs[0] = MIN_FINE_START;
	min_fine_dqs[1] = MIN_FINE_START;
	max_coarse_dqs[0] = MAX_START;
	max_coarse_dqs[1] = MAX_START;
	max_fine_dqs[0] = MAX_FINE_START;
	max_fine_dqs[1] = MAX_FINE_START;
	dqs = 0;

	/* Add by KP, DQS MIN boundary */
	reg = readl((void *)MT76XX_MEMCTRL_BASE + 0x20);
	coarse_dqs_dll = (reg & 0xf00) >> 8;
	fine_dqs_dll = (reg & 0xf0) >> 4;
	if (coarse_dqs_dll <= 8)
		min_coarse_dqs_bnd = 8 - coarse_dqs_dll;
	else
		min_coarse_dqs_bnd = 0;

	if (fine_dqs_dll <= 8)
		min_fine_dqs_bnd = 8 - fine_dqs_dll;
	else
		min_fine_dqs_bnd = 0;
	/* DQS MIN boundary */

DQS_CAL:

	for (k = 0; k < 2; k++) {
		u32 test_dqs;

		if (k == 0)
			test_dqs = MAX_START;
		else
			test_dqs = MAX_FINE_START;

		do {
			flag = test_loop(k, dqs, test_dqs, max_coarse_dqs,
					 0x400, 0x3, 0xf);
			if (flag == -1)
				break;

			test_dqs++;
		} while (test_dqs <= 0xf);

		if (k == 0) {
			max_coarse_dqs[dqs] = test_dqs;
		} else {
			test_dqs--;

			if (test_dqs == MAX_FINE_START - 1) {
				max_coarse_dqs[dqs]--;
				max_fine_dqs[dqs] = 0xf;
			} else {
				max_fine_dqs[dqs] = test_dqs;
			}
		}
	}

	for (k = 0; k < 2; k++) {
		u32 test_dqs;

		if (k == 0)
			test_dqs = MIN_START;
		else
			test_dqs = MIN_FINE_START;

		do {
			flag = test_loop(k, dqs, test_dqs, min_coarse_dqs,
					 0x480, 0x1, 0x0);
			if (k == 0) {
				if (flag == -1 ||
				    test_dqs == min_coarse_dqs_bnd)
					break;

				test_dqs--;

				if (test_dqs < min_coarse_dqs_bnd)
					break;
			} else {
				if (flag == -1) {
					test_dqs++;
					break;
				} else if (test_dqs == min_fine_dqs_bnd) {
					break;
				}

				test_dqs--;

				if (test_dqs < min_fine_dqs_bnd)
					break;
			}
		} while (test_dqs >= 0);

		if (k == 0) {
			min_coarse_dqs[dqs] = test_dqs;
		} else {
			if (test_dqs == MIN_FINE_START + 1) {
				min_coarse_dqs[dqs]++;
				min_fine_dqs[dqs] = 0x0;
			} else {
				min_fine_dqs[dqs] = test_dqs;
			}
		}
	}

	if (dqs == 0) {
		dqs = 1;
		goto DQS_CAL;
	}

	for (i = 0; i < 2; i++) {
		u32 temp;

		coarse_dqs[i] = (max_coarse_dqs[i] + min_coarse_dqs[i]) >> 1;
		temp =
		    (((max_coarse_dqs[i] + min_coarse_dqs[i]) % 2) * 4) +
		    ((max_fine_dqs[i] + min_fine_dqs[i]) >> 1);
		if (temp >= 0x10) {
			coarse_dqs[i]++;
			fine_dqs[i] = (temp - 0x10) + 0x8;
		} else {
			fine_dqs[i] = temp;
		}
	}
	reg = (coarse_dqs[1] << 12) | (fine_dqs[1] << 8) |
		(coarse_dqs[0] << 4) | fine_dqs[0];

	clrbits_le32((void *)MT76XX_MEMCTRL_BASE + 0x10, BIT(4));
	writel(reg, (void *)MT76XX_MEMCTRL_BASE + 0x64);
	writel(ddr_cfg2_reg, (void *)MT76XX_MEMCTRL_BASE + 0x48);
	setbits_le32((void *)MT76XX_MEMCTRL_BASE + 0x10, BIT(4));

	for (i = 0; i < 2; i++)
		debug("[%02X%02X%02X%02X]", min_coarse_dqs[i],
		      min_fine_dqs[i], max_coarse_dqs[i], max_fine_dqs[i]);
	debug("\nDDR Calibration DQS reg = %08X\n", reg);
}
