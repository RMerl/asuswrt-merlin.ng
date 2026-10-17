// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_BOARD

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <power/pmic.h>
#include <power/stpmic1.h>

int board_ddr_power_init(enum ddr_type ddr_type)
{
	struct udevice *dev;
	bool buck3_at_1800000v = false;
	int ret;
	u32 buck2;

	ret = uclass_get_device_by_driver(UCLASS_PMIC,
					  DM_DRIVER_GET(pmic_stpmic1), &dev);
	if (ret)
		/* No PMIC on board */
		return 0;

	switch (ddr_type) {
	case STM32MP_DDR3:
		/* VTT = Set LDO3 to sync mode */
		ret = pmic_reg_read(dev, STPMIC1_LDOX_MAIN_CR(STPMIC1_LDO3));
		if (ret < 0)
			return ret;

		ret &= ~STPMIC1_LDO3_MODE;
		ret &= ~STPMIC1_LDO12356_VOUT_MASK;
		ret |= STPMIC1_LDO_VOUT(STPMIC1_LDO3_DDR_SEL);

		ret = pmic_reg_write(dev, STPMIC1_LDOX_MAIN_CR(STPMIC1_LDO3),
				     ret);
		if (ret < 0)
			return ret;

		/* VDD_DDR = Set BUCK2 to 1.35V */
		ret = pmic_clrsetbits(dev,
				      STPMIC1_BUCKX_MAIN_CR(STPMIC1_BUCK2),
				      STPMIC1_BUCK_VOUT_MASK,
				      STPMIC1_BUCK2_1350000V);
		if (ret < 0)
			return ret;

		/* Enable VDD_DDR = BUCK2 */
		ret = pmic_clrsetbits(dev,
				      STPMIC1_BUCKX_MAIN_CR(STPMIC1_BUCK2),
				      STPMIC1_BUCK_ENA, STPMIC1_BUCK_ENA);
		if (ret < 0)
			return ret;

		mdelay(STPMIC1_DEFAULT_START_UP_DELAY_MS);

		/* Enable VREF */
		ret = pmic_clrsetbits(dev, STPMIC1_REFDDR_MAIN_CR,
				      STPMIC1_VREF_ENA, STPMIC1_VREF_ENA);
		if (ret < 0)
			return ret;

		mdelay(STPMIC1_DEFAULT_START_UP_DELAY_MS);

		/* Enable VTT = LDO3 */
		ret = pmic_clrsetbits(dev,
				      STPMIC1_LDOX_MAIN_CR(STPMIC1_LDO3),
				      STPMIC1_LDO_ENA, STPMIC1_LDO_ENA);
		if (ret < 0)
			return ret;

		mdelay(STPMIC1_DEFAULT_START_UP_DELAY_MS);

		break;

	case STM32MP_LPDDR2_16:
	case STM32MP_LPDDR2_32:
	case STM32MP_LPDDR3_16:
	case STM32MP_LPDDR3_32:
		/*
		 * configure VDD_DDR1 = LDO3
		 * Set LDO3 to 1.8V
		 * + bypass mode if BUCK3 = 1.8V
		 * + normal mode if BUCK3 != 1.8V
		 */
		ret = pmic_reg_read(dev,
				    STPMIC1_BUCKX_MAIN_CR(STPMIC1_BUCK3));
		if (ret < 0)
			return ret;

		if ((ret & STPMIC1_BUCK3_1800000V) == STPMIC1_BUCK3_1800000V)
			buck3_at_1800000v = true;

		ret = pmic_reg_read(dev, STPMIC1_LDOX_MAIN_CR(STPMIC1_LDO3));
		if (ret < 0)
			return ret;

		ret &= ~STPMIC1_LDO3_MODE;
		ret &= ~STPMIC1_LDO12356_VOUT_MASK;
		ret |= STPMIC1_LDO3_1800000;
		if (buck3_at_1800000v)
			ret |= STPMIC1_LDO3_MODE;

		ret = pmic_reg_write(dev, STPMIC1_LDOX_MAIN_CR(STPMIC1_LDO3),
				     ret);
		if (ret < 0)
			return ret;

		/* VDD_DDR2 : Set BUCK2 to 1.2V (16bits) or 1.25V (32 bits)*/
		switch (ddr_type) {
		case STM32MP_LPDDR2_32:
		case STM32MP_LPDDR3_32:
			buck2 = STPMIC1_BUCK2_1250000V;
			break;
		default:
		case STM32MP_LPDDR2_16:
		case STM32MP_LPDDR3_16:
			buck2 = STPMIC1_BUCK2_1200000V;
			break;
		}

		ret = pmic_clrsetbits(dev,
				      STPMIC1_BUCKX_MAIN_CR(STPMIC1_BUCK2),
				      STPMIC1_BUCK_VOUT_MASK,
				      buck2);
		if (ret < 0)
			return ret;

		/* Enable VDD_DDR1 = LDO3 */
		ret = pmic_clrsetbits(dev, STPMIC1_LDOX_MAIN_CR(STPMIC1_LDO3),
				      STPMIC1_LDO_ENA, STPMIC1_LDO_ENA);
		if (ret < 0)
			return ret;

		mdelay(STPMIC1_DEFAULT_START_UP_DELAY_MS);

		/* Enable VDD_DDR2 =BUCK2 */
		ret = pmic_clrsetbits(dev,
				      STPMIC1_BUCKX_MAIN_CR(STPMIC1_BUCK2),
				      STPMIC1_BUCK_ENA, STPMIC1_BUCK_ENA);
		if (ret < 0)
			return ret;

		mdelay(STPMIC1_DEFAULT_START_UP_DELAY_MS);

		/* Enable VREF */
		ret = pmic_clrsetbits(dev, STPMIC1_REFDDR_MAIN_CR,
				      STPMIC1_VREF_ENA, STPMIC1_VREF_ENA);
		if (ret < 0)
			return ret;

		mdelay(STPMIC1_DEFAULT_START_UP_DELAY_MS);

		break;

	default:
		break;
	};

	return 0;
}

static int stmpic_buck1_set(struct udevice *dev, u32 voltage_mv)
{
	u32 value;

	/* VDDCORE= STMPCI1 BUCK1 ramp=+25mV, 5 => 725mV, 36 => 1500mV */
	value = ((voltage_mv - 725) / 25) + 5;
	if (value < 5)
		value = 5;
	if (value > 36)
		value = 36;

	return pmic_clrsetbits(dev,
			       STPMIC1_BUCKX_MAIN_CR(STPMIC1_BUCK1),
			       STPMIC1_BUCK_VOUT_MASK,
			       STPMIC1_BUCK_VOUT(value));
}

/* early init of PMIC */
struct udevice *stpmic1_init(u32 voltage_mv)
{
	struct udevice *dev;

	if (uclass_get_device_by_driver(UCLASS_PMIC,
					DM_DRIVER_GET(pmic_stpmic1), &dev))
		return NULL;

	/* update VDDCORE = BUCK1 */
	if (voltage_mv)
		stmpic_buck1_set(dev, voltage_mv);

	return dev;
}
