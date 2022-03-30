// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */

#include <common.h>
#include <lcd.h>
#include <libtizen.h>
#include <samsung/misc.h>
#include <errno.h>
#include <version.h>
#include <malloc.h>
#include <memalign.h>
#include <linux/sizes.h>
#include <asm/arch/cpu.h>
#include <asm/gpio.h>
#include <linux/input.h>
#include <dm.h>
/*
 * Use #ifdef to work around conflicting headers while we wait for this to be
 * converted to driver model.
 */
#ifdef CONFIG_DM_PMIC_MAX77686
#include <power/max77686_pmic.h>
#endif
#ifdef CONFIG_DM_PMIC_MAX8998
#include <power/max8998_pmic.h>
#endif
#ifdef CONFIG_PMIC_MAX8997
#include <power/max8997_pmic.h>
#endif
#include <power/pmic.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SET_DFU_ALT_INFO
void set_dfu_alt_info(char *interface, char *devstr)
{
	size_t buf_size = CONFIG_SET_DFU_ALT_BUF_LEN;
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, buf_size);
	char *alt_info = "Settings not found!";
	char *status = "error!\n";
	char *alt_setting;
	char *alt_sep;
	int offset = 0;

	puts("DFU alt info setting: ");

	alt_setting = get_dfu_alt_boot(interface, devstr);
	if (alt_setting) {
		env_set("dfu_alt_boot", alt_setting);
		offset = snprintf(buf, buf_size, "%s", alt_setting);
	}

	alt_setting = get_dfu_alt_system(interface, devstr);
	if (alt_setting) {
		if (offset)
			alt_sep = ";";
		else
			alt_sep = "";

		offset += snprintf(buf + offset, buf_size - offset,
				    "%s%s", alt_sep, alt_setting);
	}

	if (offset) {
		alt_info = buf;
		status = "done\n";
	}

	env_set("dfu_alt_info", alt_info);
	puts(status);
}
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
void set_board_info(void)
{
	char info[64];

	snprintf(info, ARRAY_SIZE(info), "%u.%u", (s5p_cpu_rev & 0xf0) >> 4,
		 s5p_cpu_rev & 0xf);
	env_set("soc_rev", info);

	snprintf(info, ARRAY_SIZE(info), "%x", s5p_cpu_id);
	env_set("soc_id", info);

#ifdef CONFIG_REVISION_TAG
	snprintf(info, ARRAY_SIZE(info), "%x", get_board_rev());
	env_set("board_rev", info);
#endif
#ifdef CONFIG_OF_LIBFDT
	const char *bdtype = "";
	const char *bdname = CONFIG_SYS_BOARD;

#ifdef CONFIG_BOARD_TYPES
	bdtype = get_board_type();
	if (!bdtype)
		bdtype = "";

	sprintf(info, "%s%s", bdname, bdtype);
	env_set("board_name", info);
#endif
	snprintf(info, ARRAY_SIZE(info),  "%s%x-%s%s.dtb",
		 CONFIG_SYS_SOC, s5p_cpu_id, bdname, bdtype);
	env_set("fdtfile", info);
#endif
}
#endif /* CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG */

#ifdef CONFIG_LCD_MENU
static int power_key_pressed(u32 reg)
{
#ifndef CONFIG_DM_I2C /* TODO(maintainer): Convert to driver model */
	struct pmic *pmic;
	u32 status;
	u32 mask;

	pmic = pmic_get(KEY_PWR_PMIC_NAME);
	if (!pmic) {
		printf("%s: Not found\n", KEY_PWR_PMIC_NAME);
		return 0;
	}

	if (pmic_probe(pmic))
		return 0;

	if (reg == KEY_PWR_STATUS_REG)
		mask = KEY_PWR_STATUS_MASK;
	else
		mask = KEY_PWR_INTERRUPT_MASK;

	if (pmic_reg_read(pmic, reg, &status))
		return 0;

	return !!(status & mask);
#else
	return 0;
#endif
}

static int key_pressed(int key)
{
	int value;

	switch (key) {
	case KEY_POWER:
		value = power_key_pressed(KEY_PWR_INTERRUPT_REG);
		break;
	case KEY_VOLUMEUP:
		value = !gpio_get_value(KEY_VOL_UP_GPIO);
		break;
	case KEY_VOLUMEDOWN:
		value = !gpio_get_value(KEY_VOL_DOWN_GPIO);
		break;
	default:
		value = 0;
		break;
	}

	return value;
}

#ifdef CONFIG_LCD
static int check_keys(void)
{
	int keys = 0;

	if (key_pressed(KEY_POWER))
		keys += KEY_POWER;
	if (key_pressed(KEY_VOLUMEUP))
		keys += KEY_VOLUMEUP;
	if (key_pressed(KEY_VOLUMEDOWN))
		keys += KEY_VOLUMEDOWN;

	return keys;
}

/*
 * 0 BOOT_MODE_INFO
 * 1 BOOT_MODE_THOR
 * 2 BOOT_MODE_UMS
 * 3 BOOT_MODE_DFU
 * 4 BOOT_MODE_EXIT
 */
static char *
mode_name[BOOT_MODE_EXIT + 1][2] = {
	{"DEVICE", ""},
	{"THOR", "thor"},
	{"UMS", "ums"},
	{"DFU", "dfu"},
	{"GPT", "gpt"},
	{"ENV", "env"},
	{"EXIT", ""},
};

static char *
mode_info[BOOT_MODE_EXIT + 1] = {
	"info",
	"downloader",
	"mass storage",
	"firmware update",
	"restore",
	"default",
	"and run normal boot"
};

static char *
mode_cmd[BOOT_MODE_EXIT + 1] = {
	"",
	"thor 0 mmc 0",
	"ums 0 mmc 0",
	"dfu 0 mmc 0",
	"gpt write mmc 0 $partitions",
	"env default -a; saveenv",
	"",
};

static void display_board_info(void)
{
#ifdef CONFIG_MMC
	struct mmc *mmc = find_mmc_device(0);
#endif
	vidinfo_t *vid = &panel_info;

	lcd_position_cursor(4, 4);

	lcd_printf("%s\n\t", U_BOOT_VERSION);
	lcd_puts("\n\t\tBoard Info:\n");
#ifdef CONFIG_SYS_BOARD
	lcd_printf("\tBoard name: %s\n", CONFIG_SYS_BOARD);
#endif
#ifdef CONFIG_REVISION_TAG
	lcd_printf("\tBoard rev: %u\n", get_board_rev());
#endif
	lcd_printf("\tDRAM banks: %u\n", CONFIG_NR_DRAM_BANKS);
	lcd_printf("\tDRAM size: %u MB\n", gd->ram_size / SZ_1M);

#ifdef CONFIG_MMC
	if (mmc) {
		if (!mmc->capacity)
			mmc_init(mmc);

		lcd_printf("\teMMC size: %llu MB\n", mmc->capacity / SZ_1M);
	}
#endif
	if (vid)
		lcd_printf("\tDisplay resolution: %u x % u\n",
			   vid->vl_col, vid->vl_row);

	lcd_printf("\tDisplay BPP: %u\n", 1 << vid->vl_bpix);
}
#endif

static int mode_leave_menu(int mode)
{
#ifdef CONFIG_LCD
	char *exit_option;
	char *exit_reset = "reset";
	char *exit_back = "back";
	cmd_tbl_t *cmd;
	int cmd_result;
	int leave;

	lcd_clear();

	switch (mode) {
	case BOOT_MODE_EXIT:
		return 1;
	case BOOT_MODE_INFO:
		display_board_info();
		exit_option = exit_back;
		leave = 0;
		break;
	default:
		cmd = find_cmd(mode_name[mode][1]);
		if (cmd) {
			printf("Enter: %s %s\n", mode_name[mode][0],
			       mode_info[mode]);
			lcd_printf("\n\n\t%s %s\n", mode_name[mode][0],
				   mode_info[mode]);
			lcd_puts("\n\tDo not turn off device before finish!\n");

			cmd_result = run_command(mode_cmd[mode], 0);

			if (cmd_result == CMD_RET_SUCCESS) {
				printf("Command finished\n");
				lcd_clear();
				lcd_printf("\n\n\t%s finished\n",
					   mode_name[mode][0]);

				exit_option = exit_reset;
				leave = 1;
			} else {
				printf("Command error\n");
				lcd_clear();
				lcd_printf("\n\n\t%s command error\n",
					   mode_name[mode][0]);

				exit_option = exit_back;
				leave = 0;
			}
		} else {
			lcd_puts("\n\n\tThis mode is not supported.\n");
			exit_option = exit_back;
			leave = 0;
		}
	}

	lcd_printf("\n\n\tPress POWER KEY to %s\n", exit_option);

	/* Clear PWR button Rising edge interrupt status flag */
	power_key_pressed(KEY_PWR_INTERRUPT_REG);

	/* Wait for PWR key */
	while (!key_pressed(KEY_POWER))
		mdelay(1);

	lcd_clear();
	return leave;
#else
	return 0;
#endif
}

#ifdef CONFIG_LCD
static void display_download_menu(int mode)
{
	char *selection[BOOT_MODE_EXIT + 1];
	int i;

	for (i = 0; i <= BOOT_MODE_EXIT; i++)
		selection[i] = "[  ]";

	selection[mode] = "[=>]";

	lcd_clear();
	lcd_printf("\n\n\t\tDownload Mode Menu\n\n");

	for (i = 0; i <= BOOT_MODE_EXIT; i++)
		lcd_printf("\t%s  %s - %s\n\n", selection[i],
			   mode_name[i][0], mode_info[i]);
}
#endif

static void download_menu(void)
{
#ifdef CONFIG_LCD
	int mode = 0;
	int last_mode = 0;
	int run;
	int key = 0;
	int timeout = 15; /* sec */
	int i;

	display_download_menu(mode);

	lcd_puts("\n");

	/* Start count if no key is pressed */
	while (check_keys())
		continue;

	while (timeout--) {
		lcd_printf("\r\tNormal boot will start in: %2.d seconds.",
			   timeout);

		/* about 1000 ms in for loop */
		for (i = 0; i < 10; i++) {
			mdelay(100);
			key = check_keys();
			if (key)
				break;
		}
		if (key)
			break;
	}

	if (!key) {
		lcd_clear();
		return;
	}

	while (1) {
		run = 0;

		if (mode != last_mode)
			display_download_menu(mode);

		last_mode = mode;
		mdelay(200);

		key = check_keys();
		switch (key) {
		case KEY_POWER:
			run = 1;
			break;
		case KEY_VOLUMEUP:
			if (mode > 0)
				mode--;
			break;
		case KEY_VOLUMEDOWN:
			if (mode < BOOT_MODE_EXIT)
				mode++;
			break;
		default:
			break;
		}

		if (run) {
			if (mode_leave_menu(mode))
				run_command("reset", 0);

			display_download_menu(mode);
		}
	}

	lcd_clear();
#endif
}

void check_boot_mode(void)
{
	int pwr_key;

	pwr_key = power_key_pressed(KEY_PWR_STATUS_REG);
	if (!pwr_key)
		return;

	/* Clear PWR button Rising edge interrupt status flag */
	power_key_pressed(KEY_PWR_INTERRUPT_REG);

	if (key_pressed(KEY_VOLUMEUP))
		download_menu();
	else if (key_pressed(KEY_VOLUMEDOWN))
		mode_leave_menu(BOOT_MODE_THOR);
}

void keys_init(void)
{
	/* Set direction to input */
	gpio_request(KEY_VOL_UP_GPIO, "volume-up");
	gpio_request(KEY_VOL_DOWN_GPIO, "volume-down");
	gpio_direction_input(KEY_VOL_UP_GPIO);
	gpio_direction_input(KEY_VOL_DOWN_GPIO);
}
#endif /* CONFIG_LCD_MENU */

#ifdef CONFIG_CMD_BMP
void draw_logo(void)
{
	int x, y;
	ulong addr;

	addr = panel_info.logo_addr;
	if (!addr) {
		pr_err("There is no logo data.\n");
		return;
	}

	if (panel_info.vl_width >= panel_info.logo_width) {
		x = ((panel_info.vl_width - panel_info.logo_width) >> 1);
		x += panel_info.logo_x_offset; /* For X center align */
	} else {
		x = 0;
		printf("Warning: image width is bigger than display width\n");
	}

	if (panel_info.vl_height >= panel_info.logo_height) {
		y = ((panel_info.vl_height - panel_info.logo_height) >> 1);
		y += panel_info.logo_y_offset; /* For Y center align */
	} else {
		y = 0;
		printf("Warning: image height is bigger than display height\n");
	}

	bmp_display(addr, x, y);
}
#endif /* CONFIG_CMD_BMP */
