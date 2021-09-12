/*
 * Chip related low power flags
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
 * $Id: lpflags.h 592839 2015-10-14 14:19:09Z $
 */
#ifndef _lpflags_h_
#define _lpflags_h_

/* Chip related low power flags (lpflags) */
#define LPFLAGS_SI_GLOBAL_DISABLE		(1 << 0)
#define LPFLAGS_SI_MEM_STDBY_DISABLE		(1 << 1)
#define LPFLAGS_SI_SFLASH_DISABLE		(1 << 2)
#define LPFLAGS_SI_BTLDO3P3_DISABLE		(1 << 3)
#define LPFLAGS_SI_GCI_FORCE_REGCLK_DISABLE	(1 << 4)
#define LPFLAGS_SI_FORCE_PWM_WHEN_RADIO_ON	(1 << 5)
#define LPFLAGS_SI_DS0_SLEEP_PDA_DISABLE	(1 << 6)
#define LPFLAGS_SI_DS1_SLEEP_PDA_DISABLE	(1 << 7)
#define LPFLAGS_PHY_GLOBAL_DISABLE		(1 << 16)
#define LPFLAGS_PHY_LP_DISABLE			(1 << 17)
#define LPFLAGS_PSM_PHY_CTL			(1 << 18)

#endif /* _lpflags_h_ */
