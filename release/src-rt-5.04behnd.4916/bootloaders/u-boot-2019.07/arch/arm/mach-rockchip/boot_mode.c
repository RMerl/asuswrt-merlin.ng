// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <adc.h>
#include <asm/io.h>
#include <asm/arch-rockchip/boot_mode.h>

#if (CONFIG_ROCKCHIP_BOOT_MODE_REG == 0)

int setup_boot_mode(void)
{
	return 0;
}

#else

void set_back_to_bootrom_dnl_flag(void)
{
	writel(BOOT_BROM_DOWNLOAD, CONFIG_ROCKCHIP_BOOT_MODE_REG);
}

/*
 * detect download key status by adc, most rockchip
 * based boards use adc sample the download key status,
 * but there are also some use gpio. So it's better to
 * make this a weak function that can be override by
 * some special boards.
 */
#define KEY_DOWN_MIN_VAL	0
#define KEY_DOWN_MAX_VAL	30

__weak int rockchip_dnl_key_pressed(void)
{
	unsigned int val;

	if (adc_channel_single_shot("saradc", 1, &val)) {
		pr_err("%s: adc_channel_single_shot fail!\n", __func__);
		return false;
	}

	if ((val >= KEY_DOWN_MIN_VAL) && (val <= KEY_DOWN_MAX_VAL))
		return true;
	else
		return false;
}

void rockchip_dnl_mode_check(void)
{
	if (rockchip_dnl_key_pressed()) {
		printf("download key pressed, entering download mode...");
		set_back_to_bootrom_dnl_flag();
		do_reset(NULL, 0, 0, NULL);
	}
}

int setup_boot_mode(void)
{
	void *reg = (void *)CONFIG_ROCKCHIP_BOOT_MODE_REG;
	int boot_mode = readl(reg);

	rockchip_dnl_mode_check();

	boot_mode = readl(reg);
	debug("%s: boot mode 0x%08x\n", __func__, boot_mode);

	/* Clear boot mode */
	writel(BOOT_NORMAL, reg);

	switch (boot_mode) {
	case BOOT_FASTBOOT:
		debug("%s: enter fastboot!\n", __func__);
		env_set("preboot", "setenv preboot; fastboot usb0");
		break;
	case BOOT_UMS:
		debug("%s: enter UMS!\n", __func__);
		env_set("preboot", "setenv preboot; ums mmc 0");
		break;
	}

	return 0;
}

#endif
