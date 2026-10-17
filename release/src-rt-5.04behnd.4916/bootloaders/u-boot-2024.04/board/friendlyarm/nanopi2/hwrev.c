// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) Guangzhou FriendlyARM Computer Tech. Co., Ltd.
 * (http://www.friendlyarm.com)
 */

#include <config.h>
#include <common.h>
#include <i2c.h>
#include <asm/io.h>

#include <asm/arch/nexell.h>
#include <asm/arch/nx_gpio.h>

/* Board revision list: <PCB3 | PCB2 | PCB1>
 *  0b000 - NanoPi 2
 *  0b001 - NanoPC-T2
 *  0b010 - NanoPi S2
 *  0b011 - Smart4418
 *  0b100 - NanoPi Fire 2A
 *  0b111 - NanoPi M2A
 *
 * Extented revision:
 *  0b001 - Smart4418-SDK
 */
#define __IO_GRP		2	/* GPIO_C */
#define __IO_PCB1		26
#define __IO_PCB2		27
#define __IO_PCB3		25

static int pcb_rev = -1;
static int base_rev;

static void bd_hwrev_config_gpio(void)
{
	int gpios[3][2] = {
		{ __IO_PCB1, 1 },
		{ __IO_PCB2, 1 },
		{ __IO_PCB3, 1 },
	};
	int i;

	/* gpio input mode, pull-down */
	for (i = 0; i < 3; i++) {
		nx_gpio_set_pad_function(__IO_GRP, gpios[i][0], gpios[i][1]);
		nx_gpio_set_output_enable(__IO_GRP, gpios[i][0], 0);
		nx_gpio_set_pull_mode(__IO_GRP, gpios[i][0], 0);
	}
}

void bd_hwrev_init(void)
{
	if (pcb_rev >= 0)
		return;

	bd_hwrev_config_gpio();

	pcb_rev  = nx_gpio_get_input_value(__IO_GRP, __IO_PCB1);
	pcb_rev |= nx_gpio_get_input_value(__IO_GRP, __IO_PCB2) << 1;
	pcb_rev |= nx_gpio_get_input_value(__IO_GRP, __IO_PCB3) << 2;
}

/* Get extended revision for SmartXX18 */
void bd_base_rev_init(void)
{
	struct udevice *dev;
	u8 val = 0;

	if (pcb_rev != 0x3)
		return;

#define PCA9536_I2C_BUS     2
#define PCA9636_I2C_ADDR    0x41
	if (i2c_get_chip_for_busnum
				(PCA9536_I2C_BUS, PCA9636_I2C_ADDR, 1, &dev))
		return;

	if (!dm_i2c_read(dev, 0, &val, 1))
		base_rev = (val & 0xf);
}

/* To override __weak symbols */
u32 get_board_revision(void)
{
	return (base_rev << 8) | pcb_rev;
}

#ifdef CONFIG_REVISION_TAG
u32 get_board_rev(void)
{
	return get_board_revision();
}
#endif

const char *get_board_name(void)
{
	bd_hwrev_init();

	switch (pcb_rev) {
	case 0:
		return "NanoPi 2";
	case 1:
		return "NanoPC-T2";
	case 2:
		return "NanoPi S2";
	case 3:
		return "Smart4418";
	case 4:
		return "NanoPi Fire 2A";
	case 7:
		return "NanoPi M2A";
	default:
		return "s5p4418-X";
	}
}
