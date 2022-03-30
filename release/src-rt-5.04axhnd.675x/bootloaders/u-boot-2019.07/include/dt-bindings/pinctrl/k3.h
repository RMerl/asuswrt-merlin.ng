/* SPDX-License-Identifier: GPL-2.0 */
/*
 * This header provides constants for TI K3-AM65 pinctrl bindings.
 *
 * Copyright (C) 2018 Texas Instruments
 */
#ifndef _DT_BINDINGS_PINCTRL_TI_K3_AM65_H
#define _DT_BINDINGS_PINCTRL_TI_K3_AM65_H

#define PULL_DISABLE		(1 << 16)
#define PULL_UP			(1 << 17)
#define INPUT_EN		(1 << 18)
#define SLEWCTRL_200MHZ		0
#define SLEWCTRL_150MHZ		(1 << 19)
#define SLEWCTRL_100MHZ		(2 << 19)
#define SLEWCTRL_50MHZ		(3 << 19)
#define TX_DIS			(1 << 21)
#define ISO_OVR			(1 << 22)
#define ISO_BYPASS		(1 << 23)
#define DS_EN			(1 << 24)
#define DS_INPUT		(1 << 25)
#define DS_FORCE_OUT_HIGH	(1 << 26)
#define DS_PULL_UP_DOWN_EN	0
#define DS_PULL_UP_DOWN_DIS	(1 << 27)
#define DS_PULL_UP_SEL		(1 << 28)
#define WAKEUP_ENABLE		(1 << 29)

#define PIN_OUTPUT		(PULL_DISABLE)
#define PIN_OUTPUT_PULLUP	(PULL_UP)
#define PIN_OUTPUT_PULLDOWN	0
#define PIN_INPUT		(INPUT_EN | PULL_DISABLE)
#define PIN_INPUT_PULLUP	(INPUT_EN | PULL_UP)
#define PIN_INPUT_PULLDOWN	(INPUT_EN)

#define AM65X_IOPAD(pa, val, muxmode)		(((pa) & 0x1fff)) ((val) | (muxmode))
#define AM65X_WKUP_IOPAD(pa, val, muxmode)	(((pa) & 0x1fff)) ((val) | (muxmode))

#endif
