/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef SC_PAD_API_H
#define SC_PAD_API_H

/* Defines for sc_pad_config_t */
#define SC_PAD_CONFIG_NORMAL	0U	/* Normal */
#define SC_PAD_CONFIG_OD	1U	/* Open Drain */
#define SC_PAD_CONFIG_OD_IN	2U	/* Open Drain and input */
#define SC_PAD_CONFIG_OUT_IN	3U	/* Output and input */

/* Defines for sc_pad_iso_t */
#define SC_PAD_ISO_OFF		0U	/* ISO latch is transparent */
#define SC_PAD_ISO_EARLY	1U	/* Follow EARLY_ISO */
#define SC_PAD_ISO_LATE		2U	/* Follow LATE_ISO */
#define SC_PAD_ISO_ON		3U	/* ISO latched data is held */

/* Defines for sc_pad_28fdsoi_dse_t */
#define SC_PAD_28FDSOI_DSE_18V_1MA	0U /* Drive strength of 1mA for 1.8v */
#define SC_PAD_28FDSOI_DSE_18V_2MA	1U /* Drive strength of 2mA for 1.8v */
#define SC_PAD_28FDSOI_DSE_18V_4MA	2U /* Drive strength of 4mA for 1.8v */
#define SC_PAD_28FDSOI_DSE_18V_6MA	3U /* Drive strength of 6mA for 1.8v */
#define SC_PAD_28FDSOI_DSE_18V_8MA	4U /* Drive strength of 8mA for 1.8v */
#define SC_PAD_28FDSOI_DSE_18V_10MA	5U /* Drive strength of 10mA for 1.8v */
#define SC_PAD_28FDSOI_DSE_18V_12MA	6U /* Drive strength of 12mA for 1.8v */
#define SC_PAD_28FDSOI_DSE_18V_HS	7U /* High-speed for 1.8v */
#define SC_PAD_28FDSOI_DSE_33V_2MA	0U /* Drive strength of 2mA for 3.3v */
#define SC_PAD_28FDSOI_DSE_33V_4MA	1U /* Drive strength of 4mA for 3.3v */
#define SC_PAD_28FDSOI_DSE_33V_8MA	2U /* Drive strength of 8mA for 3.3v */
#define SC_PAD_28FDSOI_DSE_33V_12MA	3U /* Drive strength of 12mA for 3.3v */
#define SC_PAD_28FDSOI_DSE_DV_HIGH	0U /* High drive strength dual volt */
#define SC_PAD_28FDSOI_DSE_DV_LOW	1U /* Low drive strength  dual volt */

/* Defines for sc_pad_28fdsoi_ps_t */
#define SC_PAD_28FDSOI_PS_KEEPER 0U /* Bus-keeper (only valid for 1.8v) */
#define SC_PAD_28FDSOI_PS_PU	1U /* Pull-up */
#define SC_PAD_28FDSOI_PS_PD	2U /* Pull-down */
#define SC_PAD_28FDSOI_PS_NONE	3U /* No pull (disabled) */

/* Defines for sc_pad_28fdsoi_pus_t */
#define SC_PAD_28FDSOI_PUS_30K_PD	0U /* 30K pull-down */
#define SC_PAD_28FDSOI_PUS_100K_PU	1U /* 100K pull-up */
#define SC_PAD_28FDSOI_PUS_3K_PU	2U /* 3K pull-up */
#define SC_PAD_28FDSOI_PUS_30K_PU	3U /* 30K pull-up */

/* Defines for sc_pad_wakeup_t */
#define SC_PAD_WAKEUP_OFF	0U /* Off */
#define SC_PAD_WAKEUP_CLEAR	1U /* Clears pending flag */
#define SC_PAD_WAKEUP_LOW_LVL	4U /* Low level */
#define SC_PAD_WAKEUP_FALL_EDGE	5U /* Falling edge */
#define SC_PAD_WAKEUP_RISE_EDGE	6U /* Rising edge */
#define SC_PAD_WAKEUP_HIGH_LVL	7U /* High-level */

#endif /* SC_PAD_API_H */
