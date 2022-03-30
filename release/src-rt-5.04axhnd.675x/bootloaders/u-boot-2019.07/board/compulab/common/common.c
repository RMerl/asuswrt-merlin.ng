// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#include <common.h>
#include <asm/bootm.h>
#include <asm/gpio.h>
#include <asm/setup.h>

#include "common.h"
#include "eeprom.h"

void cl_print_pcb_info(void)
{
	u32 board_rev = get_board_rev();
	u32 rev_major = board_rev / 100;
	u32 rev_minor = board_rev - (rev_major * 100);

	if ((rev_minor / 10) * 10 == rev_minor)
		rev_minor = rev_minor / 10;

	printf("PCB:   %u.%u\n", rev_major, rev_minor);
}

#ifdef CONFIG_SERIAL_TAG
void __weak get_board_serial(struct tag_serialnr *serialnr)
{
	/*
	 * This corresponds to what happens when we can communicate with the
	 * eeprom but don't get a valid board serial value.
	 */
	serialnr->low = 0;
	serialnr->high = 0;
};
#endif

#ifdef CONFIG_CMD_USB
int cl_usb_hub_init(int gpio, const char *label)
{
	if (gpio_request(gpio, label)) {
		printf("Error: can't obtain GPIO%d for %s", gpio, label);
		return -1;
	}

	gpio_direction_output(gpio, 0);
	udelay(10);
	gpio_set_value(gpio, 1);
	udelay(1000);
	return 0;
}

void cl_usb_hub_deinit(int gpio)
{
	gpio_free(gpio);
}
#endif
