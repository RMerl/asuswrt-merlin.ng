// SPDX-License-Identifier: GPL-2.0+
/*
 * AXP313(a) driver
 *
 * (C) Copyright 2023 Arm Ltd.
 *
 * Based on axp305.c
 * (C) Copyright 2020 Jernej Skrabec <jernej.skrabec@siol.net>
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 * (C) Copyright 2013 Oliver Schinagl <oliver@schinagl.nl>
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <asm/arch/pmic_bus.h>
#include <axp_pmic.h>

enum axp313_reg {
	AXP313_CHIP_VERSION	= 0x03,
	AXP313_OUTPUT_CTRL	= 0x10,
	AXP313_DCDC1_CTRL	= 0x13,
	AXP313_SHUTDOWN		= 0x1a,
};

#define AXP313_CHIP_VERSION_MASK	0xcf
#define AXP313_CHIP_VERSION_AXP1530	0x48
#define AXP313_CHIP_VERSION_AXP313A	0x4b
#define AXP313_CHIP_VERSION_AXP313B	0x4c

#define AXP313_DCDC_SPLIT_OFFSET	71
#define AXP313_DCDC_SPLIT_MVOLT		1200

#define AXP313_POWEROFF			BIT(7)

static u8 mvolt_to_cfg(int mvolt, int min, int max, int div)
{
	if (mvolt < min)
		mvolt = min;
	else if (mvolt > max)
		mvolt = max;

	return (mvolt - min) / div;
}

static int axp_set_dcdc(int dcdc_num, unsigned int mvolt)
{
	int ret;
	u8 cfg, enable_mask = 1U << (dcdc_num - 1);
	int volt_reg = AXP313_DCDC1_CTRL + dcdc_num - 1;
	int max_mV;

	switch (dcdc_num) {
	case 1:
	case 2:
		max_mV	= 1540;
		break;
	case 3:
		/*
		 * The manual defines a different split point, but tests
		 * show that it's the same 1200mV as for DCDC1/2.
		 */
		max_mV	= 1840;
		break;
	default:
		return -EINVAL;
	}

	if (mvolt > AXP313_DCDC_SPLIT_MVOLT)
		cfg = AXP313_DCDC_SPLIT_OFFSET + mvolt_to_cfg(mvolt,
				AXP313_DCDC_SPLIT_MVOLT + 20, max_mV, 20);
	else
		cfg = mvolt_to_cfg(mvolt, 500, AXP313_DCDC_SPLIT_MVOLT, 10);

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP313_OUTPUT_CTRL, enable_mask);

	debug("DCDC%d: writing 0x%x to reg 0x%x\n", dcdc_num, cfg, volt_reg);
	ret = pmic_bus_write(volt_reg, cfg);
	if (ret)
		return ret;

	return pmic_bus_setbits(AXP313_OUTPUT_CTRL, enable_mask);
}

int axp_set_dcdc2(unsigned int mvolt)
{
	return axp_set_dcdc(2, mvolt);
}

int axp_set_dcdc3(unsigned int mvolt)
{
	return axp_set_dcdc(3, mvolt);
}

int axp_init(void)
{
	u8 axp_chip_id;
	int ret;

	ret = pmic_bus_init();
	if (ret)
		return ret;

	ret = pmic_bus_read(AXP313_CHIP_VERSION, &axp_chip_id);
	if (ret)
		return ret;

	axp_chip_id &= AXP313_CHIP_VERSION_MASK;
	switch (axp_chip_id) {
	case AXP313_CHIP_VERSION_AXP1530:
	case AXP313_CHIP_VERSION_AXP313A:
	case AXP313_CHIP_VERSION_AXP313B:
		break;
	default:
		debug("unknown PMIC: 0x%x\n", axp_chip_id);
		return -EINVAL;
	}

	return ret;
}

#if !CONFIG_IS_ENABLED(ARM_PSCI_FW) && !IS_ENABLED(CONFIG_SYSRESET_CMD_POWEROFF)
int do_poweroff(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	pmic_bus_write(AXP313_SHUTDOWN, AXP313_POWEROFF);

	/* infinite loop during shutdown */
	while (1) {}

	/* not reached */
	return 0;
}
#endif
