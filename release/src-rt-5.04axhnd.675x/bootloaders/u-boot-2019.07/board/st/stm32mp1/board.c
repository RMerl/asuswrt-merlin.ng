// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <power/pmic.h>
#include <power/stpmic1.h>

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
#if (CONFIG_DEBUG_UART_BASE == STM32_UART4_BASE)

#define RCC_MP_APB1ENSETR (STM32_RCC_BASE + 0x0A00)
#define RCC_MP_AHB4ENSETR (STM32_RCC_BASE + 0x0A28)

	/* UART4 clock enable */
	setbits_le32(RCC_MP_APB1ENSETR, BIT(16));

#define GPIOG_BASE 0x50008000
	/* GPIOG clock enable */
	writel(BIT(6), RCC_MP_AHB4ENSETR);
	/* GPIO configuration for EVAL board
	 * => Uart4 TX = G11
	 */
	writel(0xffbfffff, GPIOG_BASE + 0x00);
	writel(0x00006000, GPIOG_BASE + 0x24);
#else

#error("CONFIG_DEBUG_UART_BASE: not supported value")

#endif
}
#endif

#ifdef CONFIG_PMIC_STPMIC1
int board_ddr_power_init(enum ddr_type ddr_type)
{
	struct udevice *dev;
	bool buck3_at_1800000v = false;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_PMIC,
					  DM_GET_DRIVER(pmic_stpmic1), &dev);
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

	case STM32MP_LPDDR2:
	case STM32MP_LPDDR3:
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

		/* VDD_DDR2 : Set BUCK2 to 1.2V */
		ret = pmic_clrsetbits(dev,
				      STPMIC1_BUCKX_MAIN_CR(STPMIC1_BUCK2),
				      STPMIC1_BUCK_VOUT_MASK,
				      STPMIC1_BUCK2_1200000V);
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
#endif
