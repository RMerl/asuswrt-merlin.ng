/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2019 Google LLC
 *
 * Taken from coreboot file of the same name
 */

#ifndef BASEBOARD_GPIO_H
#define BASEBOARD_GPIO_H

#include <asm/arch/gpio.h>
#include <ec_commands.h>

/*
 * GPIO_11 for SCI is routed to GPE0_DW1 and maps to group GPIO_GPE_N_31_0
 * which is North community
 */
#define EC_SCI_GPI	GPE0_DW1_11

/* EC SMI */
#define EC_SMI_GPI	GPIO_49

/*
 * On lidopen/lidclose GPIO_22 from North Community gets toggled and
 * is used in _PRW to wake up device from sleep. GPIO_22 maps to
 * group GPIO_GPE_N_31_0 and the pad is configured as SCI with
 * EDGE_SINGLE and INVERT.
 */
#define GPE_EC_WAKE	GPE0_DW1_22

/* Write Protect and indication if EC is in RW code. */
#define GPIO_PCH_WP	GPIO_75
#define GPIO_EC_IN_RW	GPIO_41
/* Determine if board is in final shipping mode. */
#define GPIO_SHIP_MODE	GPIO_10

/* DMIC_CONFIG_PIN: High for 1-DMIC and low for 4-DMIC's */
#define DMIC_CONFIG_PIN	GPIO_17

#ifndef __ASSEMBLY__

enum cros_gpio_t {
	CROS_GPIO_REC = 1,	 /* Recovery */

	/* Developer; * deprecated (chromium:942901) */
	CROS_GPIO_DEPRECATED_DEV = 2,
	CROS_GPIO_WP = 3,	/* Write Protect */
	CROS_GPIO_PE = 4,	/* Phase enforcement for final product */

	CROS_GPIO_ACTIVE_LOW = 0,
	CROS_GPIO_ACTIVE_HIGH = 1,

	CROS_GPIO_VIRTUAL = -1,
};
#endif

#endif /* BASEBOARD_GPIO_H */
