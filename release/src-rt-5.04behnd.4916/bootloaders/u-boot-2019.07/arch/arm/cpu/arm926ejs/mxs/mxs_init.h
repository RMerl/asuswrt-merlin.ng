/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale i.MX28 SPL functions
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */

#ifndef	__M28_INIT_H__
#define	__M28_INIT_H__

void early_delay(int delay);

void mxs_power_init(void);

#ifdef	CONFIG_SPL_MXS_PSWITCH_WAIT
void mxs_power_wait_pswitch(void);
#else
static inline void mxs_power_wait_pswitch(void) { }
#endif

void mxs_mem_init(void);
uint32_t mxs_mem_get_size(void);

void mxs_lradc_init(void);
void mxs_lradc_enable_batt_measurement(void);

#endif	/* __M28_INIT_H__ */
