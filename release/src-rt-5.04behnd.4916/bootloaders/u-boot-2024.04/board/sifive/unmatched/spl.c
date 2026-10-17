// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020-2021 SiFive, Inc
 *
 * Authors:
 *   Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#include <init.h>
#include <spl.h>
#include <misc.h>
#include <log.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/spl.h>
#include <linux/io.h>
#include <asm/arch/eeprom.h>

struct pwm_sifive_regs {
	unsigned int cfg;       /* PWM configuration register */
	unsigned int pad0;      /* Reserved */
	unsigned int cnt;       /* PWM count register */
	unsigned int pad1;      /* Reserved */
	unsigned int pwms;      /* Scaled PWM count register */
	unsigned int pad2;      /* Reserved */
	unsigned int pad3;      /* Reserved */
	unsigned int pad4;      /* Reserved */
	unsigned int cmp0;      /* PWM 0 compare register */
	unsigned int cmp1;      /* PWM 1 compare register */
	unsigned int cmp2;      /* PWM 2 compare register */
	unsigned int cmp3;      /* PWM 3 compare register */
};

#define PWM0_BASE               0x10020000
#define PWM1_BASE               0x10021000
#define PWM_CFG_INIT            0x1000
#define PWM_CMP_ENABLE_VAL      0x0
#define PWM_CMP_DISABLE_VAL     0xffff

#define UBRDG_RESET	SIFIVE_GENERIC_GPIO_NR(0, 7)
#define ULPI_RESET	SIFIVE_GENERIC_GPIO_NR(0, 9)
#define UHUB_RESET	SIFIVE_GENERIC_GPIO_NR(0, 11)
#define GEM_PHY_RESET	SIFIVE_GENERIC_GPIO_NR(0, 12)

#define MODE_SELECT_REG		0x1000
#define MODE_SELECT_SPI		0x6
#define MODE_SELECT_SD		0xb
#define MODE_SELECT_MASK	GENMASK(3, 0)

void spl_pwm_device_init(void)
{
	struct pwm_sifive_regs *pwm0, *pwm1;

	pwm0 = (struct pwm_sifive_regs *)PWM0_BASE;
	pwm1 = (struct pwm_sifive_regs *)PWM1_BASE;
	writel(PWM_CMP_DISABLE_VAL, (void *)&pwm0->cmp0);

	/* Set the 3-color PWM LEDs to yellow in SPL */
	writel(PWM_CMP_ENABLE_VAL, (void *)&pwm0->cmp1);
	writel(PWM_CMP_ENABLE_VAL, (void *)&pwm0->cmp2);
	writel(PWM_CMP_DISABLE_VAL, (void *)&pwm0->cmp3);
	writel(PWM_CFG_INIT, (void *)&pwm0->cfg);

	/* Turn on all the fans, (J21), (J23) and (J24), on the unmatched board */
	/* The SoC fan(J21) on the rev3 board cannot be controlled by PWM_COMP0,
	 *            so here sets the initial value of PWM_COMP0 as DISABLE */
	if (get_pcb_revision_from_eeprom() == PCB_REVISION_REV3)
		writel(PWM_CMP_DISABLE_VAL, (void *)&pwm1->cmp1);
	else
		writel(PWM_CMP_ENABLE_VAL, (void *)&pwm1->cmp1);

	writel(PWM_CMP_ENABLE_VAL, (void *)&pwm1->cmp2);
	writel(PWM_CMP_ENABLE_VAL, (void *)&pwm1->cmp3);
	writel(PWM_CFG_INIT, (void *)&pwm1->cfg);
}

static inline int spl_reset_device_by_gpio(const char *label, int pin, int low_width)
{
	int ret;

	ret = gpio_request(pin, label);
	if (ret) {
		debug("%s gpio request failed: %d\n", label, ret);
		return ret;
	}

	ret = gpio_direction_output(pin, 1);
	if (ret) {
		debug("%s gpio direction set failed: %d\n", label, ret);
		return ret;
	}

	udelay(1);

	gpio_set_value(pin, 0);
	udelay(low_width);
	gpio_set_value(pin, 1);

	return ret;
}

static inline int spl_gemgxl_init(void)
{
	int ret;
	/*
	 * GEMGXL init VSC8541 PHY reset sequence;
	 * leave pull-down active for 2ms
	 */
	udelay(2000);
	ret = spl_reset_device_by_gpio("gem_phy_reset", GEM_PHY_RESET, 1);
	mdelay(15);

	return ret;
}

static inline int spl_usb_pcie_bridge_init(void)
{
	return spl_reset_device_by_gpio("usb_pcie_bridge_reset", UBRDG_RESET, 3000);
}

static inline int spl_usb_hub_init(void)
{
	return spl_reset_device_by_gpio("usb_hub_reset", UHUB_RESET, 100);
}

static inline int spl_ulpi_init(void)
{
	return spl_reset_device_by_gpio("ulpi_reset", ULPI_RESET, 1);
}

int spl_board_init_f(void)
{
	int ret;

	ret = spl_soc_init();
	if (ret) {
		debug("HiFive Unmatched FU740 SPL init failed: %d\n", ret);
		goto end;
	}

	spl_pwm_device_init();

	ret = spl_gemgxl_init();
	if (ret) {
		debug("Gigabit ethernet PHY (VSC8541) init failed: %d\n", ret);
		goto end;
	}

	ret = spl_usb_pcie_bridge_init();
	if (ret) {
		debug("USB Bridge (ASM1042A) init failed: %d\n", ret);
		goto end;
	}

	ret = spl_usb_hub_init();
	if (ret) {
		debug("USB Hub (ASM1074) init failed: %d\n", ret);
		goto end;
	}

	ret = spl_ulpi_init();
	if (ret) {
		debug("USB 2.0 PHY (USB3320C) init failed: %d\n", ret);
		goto end;
	}

end:
	return ret;
}

u32 spl_boot_device(void)
{
	u32 mode_select = readl((void *)MODE_SELECT_REG);
	u32 boot_device = mode_select & MODE_SELECT_MASK;

	switch (boot_device) {
	case MODE_SELECT_SPI:
		return BOOT_DEVICE_SPI;
	case MODE_SELECT_SD:
		return BOOT_DEVICE_MMC1;
	default:
		debug("Unsupported boot device 0x%x but trying MMC1\n",
		      boot_device);
		return BOOT_DEVICE_MMC1;
	}
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* boot using first FIT config */
	return 0;
}
#endif
