/*
 * HND SiliconBackplane PMU support.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: hndpmu.h 546588 2015-04-13 09:24:52Z $
 */

#ifndef _hndlhl_h_
#define _hndlhl_h_

enum {
	LHL_MAC_TIMER = 0,
	LHL_ARM_TIMER = 1
};

typedef struct {
	uint16 offset;
	uint32 mask;
	uint32 val;
} lhl_reg_set_t;

#define LHL_REG_OFF(reg) OFFSETOF(gciregs_t, reg)

extern void si_lhl_timer_config(si_t *sih, osl_t *osh, int timer_type);
extern void si_lhl_timer_enable(si_t *sih);

extern void si_lhl_setup(si_t *sih, osl_t *osh);
extern void si_lhl_enable(si_t *sih, osl_t *osh, bool enable);
extern void si_lhl_ilp_config(si_t *sih, osl_t *osh, uint32 ilp_period);
extern void si_lhl_enable_sdio_wakeup(si_t *sih, osl_t *osh);
extern void si_lhl_disable_sdio_wakeup(si_t *sih);
extern int si_lhl_set_lpoclk(si_t *sih, osl_t *osh, uint32 lpo_force);
extern void si_set_lv_sleep_mode_lhl_config_4369(si_t *sih);

#define HIB_EXT_WAKEUP_CAP(sih)  (BCM4347_CHIP(sih->chip))

#define LHL_IS_PSMODE_0(sih)  (si_lhl_ps_mode(sih) == LHL_PS_MODE_0)
#define LHL_IS_PSMODE_1(sih)  (si_lhl_ps_mode(sih) == LHL_PS_MODE_1)
#endif /* _hndlhl_h_ */
