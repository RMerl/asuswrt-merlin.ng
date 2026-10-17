/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom.
 */

#ifndef __DT_BINDINGS_PINCTRL_BRCM_STINGRAY_H__
#define __DT_BINDINGS_PINCTRL_BRCM_STINGRAY_H__

/* Alternate functions available in MUX controller */
#define MODE_NITRO				0
#define MODE_NAND				1
#define MODE_PNOR				2
#define MODE_GPIO				3

/* Pad configuration attribute */
#define PAD_SLEW_RATE_ENA			BIT(0)
#define PAD_SLEW_RATE_ENA_MASK			BIT(0)

#define PAD_DRIVE_STRENGTH_2_MA			(0 << 1)
#define PAD_DRIVE_STRENGTH_4_MA			BIT(1)
#define PAD_DRIVE_STRENGTH_6_MA			(2 << 1)
#define PAD_DRIVE_STRENGTH_8_MA			(3 << 1)
#define PAD_DRIVE_STRENGTH_10_MA		(4 << 1)
#define PAD_DRIVE_STRENGTH_12_MA		(5 << 1)
#define PAD_DRIVE_STRENGTH_14_MA		(6 << 1)
#define PAD_DRIVE_STRENGTH_16_MA		(7 << 1)
#define PAD_DRIVE_STRENGTH_MASK			(7 << 1)

#define PAD_PULL_UP_ENA				BIT(4)
#define PAD_PULL_UP_ENA_MASK			BIT(4)

#define PAD_PULL_DOWN_ENA			BIT(5)
#define PAD_PULL_DOWN_ENA_MASK			BIT(5)

#define PAD_INPUT_PATH_DIS			BIT(6)
#define PAD_INPUT_PATH_DIS_MASK			BIT(6)

#define PAD_HYSTERESIS_ENA			BIT(7)
#define PAD_HYSTERESIS_ENA_MASK			BIT(7)

#endif
