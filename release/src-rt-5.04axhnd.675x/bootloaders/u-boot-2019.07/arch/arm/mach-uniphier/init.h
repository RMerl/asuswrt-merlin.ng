/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef __MACH_INIT_H
#define __MACH_INIT_H

#include <linux/types.h>

#define UNIPHIER_MAX_NR_DRAM_CH		3

struct uniphier_dram_ch {
	unsigned long size;
	unsigned int width;
};

struct uniphier_board_data {
	unsigned int dram_freq;
	struct uniphier_dram_ch dram_ch[UNIPHIER_MAX_NR_DRAM_CH];
	unsigned int flags;

#define UNIPHIER_BD_DRAM_SPARSE			BIT(9)
#define UNIPHIER_BD_DDR3PLUS			BIT(8)
};

const struct uniphier_board_data *uniphier_get_board_param(void);

int uniphier_ld4_init(const struct uniphier_board_data *bd);
int uniphier_pro4_init(const struct uniphier_board_data *bd);
int uniphier_sld8_init(const struct uniphier_board_data *bd);
int uniphier_pro5_init(const struct uniphier_board_data *bd);
int uniphier_pxs2_init(const struct uniphier_board_data *bd);

#if defined(CONFIG_MICRO_SUPPORT_CARD)
void uniphier_sbc_init_admulti(void);
void uniphier_sbc_init_savepin(void);
void uniphier_ld4_sbc_init(void);
void uniphier_pxs2_sbc_init(void);
void uniphier_ld11_sbc_init(void);
#else
static inline void uniphier_sbc_init_admulti(void)
{
}

static inline void uniphier_sbc_init_savepin(void)
{
}

static inline void uniphier_ld4_sbc_init(void)
{
}

static inline void uniphier_pxs2_sbc_init(void)
{
}

static inline void uniphier_ld11_sbc_init(void)
{
}
#endif

void uniphier_ld4_bcu_init(const struct uniphier_board_data *bd);

int uniphier_memconf_2ch_init(const struct uniphier_board_data *bd);
int uniphier_memconf_3ch_init(const struct uniphier_board_data *bd);

int uniphier_ld4_dpll_init(const struct uniphier_board_data *bd);
int uniphier_pro4_dpll_init(const struct uniphier_board_data *bd);
int uniphier_sld8_dpll_init(const struct uniphier_board_data *bd);
int uniphier_pro5_dpll_init(const struct uniphier_board_data *bd);
int uniphier_pxs2_dpll_init(const struct uniphier_board_data *bd);

void uniphier_ld4_early_clk_init(void);

void uniphier_ld4_dram_clk_init(void);
void uniphier_pro5_dram_clk_init(void);
void uniphier_pxs2_dram_clk_init(void);

int uniphier_ld4_umc_init(const struct uniphier_board_data *bd);
int uniphier_pro4_umc_init(const struct uniphier_board_data *bd);
int uniphier_sld8_umc_init(const struct uniphier_board_data *bd);
int uniphier_pro5_umc_init(const struct uniphier_board_data *bd);
int uniphier_pxs2_umc_init(const struct uniphier_board_data *bd);

void uniphier_ld4_pll_init(void);
void uniphier_pro4_pll_init(void);
void uniphier_ld11_pll_init(void);
void uniphier_ld20_pll_init(void);
void uniphier_pxs3_pll_init(void);

void uniphier_ld4_clk_init(void);
void uniphier_pro4_clk_init(void);
void uniphier_pro5_clk_init(void);
void uniphier_pxs2_clk_init(void);
void uniphier_ld11_clk_init(void);
void uniphier_ld20_clk_init(void);
void uniphier_pxs3_clk_init(void);

unsigned int uniphier_boot_device_raw(void);
int uniphier_have_internal_stm(void);
int uniphier_boot_from_backend(void);
int uniphier_pin_init(const char *pinconfig_name);

#endif /* __MACH_INIT_H */
