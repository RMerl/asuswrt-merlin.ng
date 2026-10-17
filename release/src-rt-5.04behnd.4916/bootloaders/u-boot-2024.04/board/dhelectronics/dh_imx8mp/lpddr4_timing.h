/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#ifndef __LPDDR4_TIMING_H__
#define __LPDDR4_TIMING_H__

extern struct dram_timing_info dh_imx8mp_dhcom_dram_timing_16g_x32;
extern struct dram_timing_info dh_imx8mp_dhcom_dram_timing_32g_x32;

typedef void (*scrub_func_t)(void);
extern void dh_imx8mp_dhcom_dram_scrub_16g_x32(void);
extern void dh_imx8mp_dhcom_dram_scrub_32g_x32(void);

u8 dh_get_memcfg(void);

#define DDRC_ECCCFG0_ECC_MODE_MASK	0x7

#endif /* __LPDDR4_TIMING_H__ */
