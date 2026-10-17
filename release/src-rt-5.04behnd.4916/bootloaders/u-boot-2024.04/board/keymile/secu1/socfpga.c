// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017-2020 Hitachi Power Grids
 */
#include <common.h>
#include <i2c.h>
#include <asm/gpio.h>

#include "../common/common.h"

/*
 * For FU1, the MAC address associated with the mgmt port should
 * be the base address (as read from the IVM) + 4, and for FU2 it
 * is + 10
 */
#define MAC_ADDRESS_OFFSET_FU1	4
#define MAC_ADDRESS_OFFSET_FU2	10

/*
 * This function reads the state of GPIO40 and returns true (non-zero)
 * if it is '1' and false(0) otherwise.
 *
 * This pin is routed to a pull-up on FU2 and a pull-down on
 */
#define GPIO_FU_DETECTION	40

int secu1_is_fu2(void)
{
	int value;
	int ret = gpio_request(GPIO_FU_DETECTION, "secu");

	if (ret) {
		printf("gpio: failed to request pin for FU  detection\n");
		return 1;
	}
	gpio_direction_input(GPIO_FU_DETECTION);
	value = gpio_get_value(GPIO_FU_DETECTION);

	if (value == 1)
		printf("FU2 detected\n");
	else
		printf("FU1 detected\n");

	return value;
}

static uchar ivm_content[CONFIG_SYS_IVM_EEPROM_MAX_LEN];

#if defined(CONFIG_HUSH_INIT_VAR)
int hush_init_var(void)
{
	ivm_analyze_eeprom(ivm_content, CONFIG_SYS_IVM_EEPROM_MAX_LEN);
	return 0;
}
#endif

int misc_init_r(void)
{
	if (secu1_is_fu2())
		ivm_read_eeprom(ivm_content, CONFIG_SYS_IVM_EEPROM_MAX_LEN,
				MAC_ADDRESS_OFFSET_FU2);
	else
		ivm_read_eeprom(ivm_content, CONFIG_SYS_IVM_EEPROM_MAX_LEN,
				MAC_ADDRESS_OFFSET_FU1);

	return 0;
}
