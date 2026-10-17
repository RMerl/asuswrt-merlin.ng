/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Texas Instruments K3 Device Platform Data
 *
 * Copyright (C) 2020 Texas Instruments Incorporated - https://www.ti.com/
 */
#ifndef __K3_DEV_H__
#define __K3_DEV_H__

#include <asm/io.h>
#include <linux/types.h>
#include <stdint.h>

#define LPSC_MODULE_EXISTS      BIT(0)
#define LPSC_NO_CLOCK_GATING    BIT(1)
#define LPSC_DEPENDS            BIT(2)
#define LPSC_HAS_RESET_ISO      BIT(3)
#define LPSC_HAS_LOCAL_RESET    BIT(4)
#define LPSC_NO_MODULE_RESET    BIT(5)

#define PSC_PD_EXISTS           BIT(0)
#define PSC_PD_ALWAYSON         BIT(1)
#define PSC_PD_DEPENDS          BIT(2)

#define MDSTAT_STATE_MASK		0x3f
#define MDSTAT_BUSY_MASK		0x30
#define MDSTAT_STATE_SWRSTDISABLE	0x0
#define MDSTAT_STATE_ENABLE		0x3

struct ti_psc {
	int id;
	void __iomem *base;
};

struct ti_pd;

struct ti_pd {
	int id;
	int usecount;
	struct ti_psc *psc;
	struct ti_pd *depend;
};

struct ti_lpsc;

struct ti_lpsc {
	int id;
	int usecount;
	struct ti_psc *psc;
	struct ti_pd *pd;
	struct ti_lpsc *depend;
};

struct ti_dev {
	struct ti_lpsc *lpsc;
	int id;
};

/**
 * struct ti_k3_pd_platdata - pm domain controller information structure
 */
struct ti_k3_pd_platdata {
	struct ti_psc *psc;
	struct ti_pd *pd;
	struct ti_lpsc *lpsc;
	struct ti_dev *devs;
	int num_psc;
	int num_pd;
	int num_lpsc;
	int num_devs;
};

#define PSC(_id, _base) { .id = _id, .base = (void *)_base, }
#define PSC_PD(_id, _psc, _depend) { .id = _id, .psc = _psc, .depend = _depend }
#define PSC_LPSC(_id, _psc, _pd, _depend) { .id = _id, .psc = _psc, .pd = _pd, .depend = _depend }
#define PSC_DEV(_id, _lpsc) { .id = _id, .lpsc = _lpsc }

extern const struct ti_k3_pd_platdata j721e_pd_platdata;
extern const struct ti_k3_pd_platdata j7200_pd_platdata;
extern const struct ti_k3_pd_platdata j721s2_pd_platdata;
extern const struct ti_k3_pd_platdata am62x_pd_platdata;
extern const struct ti_k3_pd_platdata am62ax_pd_platdata;

u8 ti_pd_state(struct ti_pd *pd);
u8 lpsc_get_state(struct ti_lpsc *lpsc);
int ti_lpsc_transition(struct ti_lpsc *lpsc, u8 state);

#endif
