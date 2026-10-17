// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) Guangzhou FriendlyARM Computer Tech. Co., Ltd.
 * (http://www.friendlyarm.com)
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <fdt_support.h>
#include <log.h>
#ifdef CONFIG_PWM_NX
#include <pwm.h>
#endif
#include <asm/global_data.h>
#include <asm/io.h>

#include <asm/arch/nexell.h>
#include <asm/arch/nx_gpio.h>
#include <asm/arch/display.h>
#include <asm/arch/display_dev.h>

#include <u-boot/md5.h>

#include <linux/stringify.h>

#include "hwrev.h"
#include "onewire.h"
#include "nxp-fb.h"

#include <env_internal.h>	/* for env_save() */
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

enum gpio_group {
	gpio_a, gpio_b, gpio_c, gpio_d, gpio_e,
};

#ifdef CONFIG_PWM_NX
struct pwm_device {
	int grp;
	int bit;
	int io_fn;
};

static inline void bd_pwm_config_gpio(int ch)
{
	struct pwm_device pwm_dev[] = {
		[0] = { .grp = gpio_d, .bit = 1,  .io_fn = 0 },
		[1] = { .grp = gpio_c, .bit = 13, .io_fn = 1 },
		[2] = { .grp = gpio_c, .bit = 14, .io_fn = 1 },
		[3] = { .grp = gpio_d, .bit = 0,  .io_fn = 0 },
	};

	int gp = pwm_dev[ch].grp;
	int io = pwm_dev[ch].bit;

	/* pwm backlight OFF: HIGH, ON: LOW */
	nx_gpio_set_pad_function(gp, io, pwm_dev[ch].io_fn);
	nx_gpio_set_output_value(gp, io, 1);
	nx_gpio_set_output_enable(gp, io, 1);
}
#endif

static void bd_backlight_off(void)
{
#ifdef CONFIG_S5P4418_ONEWIRE
	onewire_set_backlight(0);

#elif defined(BACKLIGHT_CH)
	bd_pwm_config_gpio(BACKLIGHT_CH);
#endif
}

static void bd_backlight_on(void)
{
#ifdef CONFIG_S5P4418_ONEWIRE
	onewire_set_backlight(127);

#elif defined(BACKLIGHT_CH)
	/* pwm backlight ON: HIGH, ON: LOW */
	s5p_pwm_init(BACKLIGHT_CH,
		 BACKLIGHT_DIV, BACKLIGHT_INV);
	s5p_pwm_config(BACKLIGHT_CH,
		   TO_DUTY_NS(BACKLIGHT_DUTY, BACKLIGHT_HZ),
		   TO_PERIOD_NS(BACKLIGHT_HZ));
#endif
}

static void bd_lcd_config_gpio(void)
{
	int i;

	for (i = 0; i < 28; i++) {
		nx_gpio_set_pad_function(gpio_a, i, 1);
		nx_gpio_set_drive_strength(gpio_a, i, 0);
		nx_gpio_set_pull_mode(gpio_a, i, 2);
	}

	nx_gpio_set_drive_strength(gpio_a, 0, 1);
}

/* DEFAULT mmc dev for eMMC boot (dwmmc.2) */
static int mmc_boot_dev;

int board_mmc_bootdev(void)
{
	return mmc_boot_dev;
}

/* call from common/env_mmc.c */
int mmc_get_env_dev(void)
{
	return mmc_boot_dev;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("Board: %s\n", get_board_name());

	return 0;
}
#endif

int nx_display_fixup_dp(struct nx_display_dev *dp)
{
	struct nxp_lcd *lcd = bd_get_lcd();
	enum lcd_format fmt = bd_get_lcd_format();
	struct nxp_lcd_timing *timing = &lcd->timing;
	struct dp_sync_info *sync = &dp->sync;
	struct dp_plane_info *plane = &dp->planes[0];
	int i;
	u32 clk = 800000000;
	u32 div;

	sync->h_active_len = lcd->width;
	sync->h_sync_width = timing->h_sw;
	sync->h_back_porch = timing->h_bp;
	sync->h_front_porch = timing->h_fp;
	sync->h_sync_invert = !lcd->polarity.inv_hsync;

	sync->v_active_len = lcd->height;
	sync->v_sync_width = timing->v_sw;
	sync->v_back_porch = timing->v_bp;
	sync->v_front_porch = timing->v_fp;
	sync->v_sync_invert = !lcd->polarity.inv_vsync;

	/* calculates pixel clock */
	div  = timing->h_sw + timing->h_bp + timing->h_fp + lcd->width;
	div *= timing->v_sw + timing->v_bp + timing->v_fp + lcd->height;
	div *= lcd->freq ? : 60;
	clk /= div;

	dp->ctrl.clk_div_lv0 = clk;
	dp->ctrl.clk_inv_lv0 = lcd->polarity.rise_vclk;

	dp->top.screen_width = lcd->width;
	dp->top.screen_height = lcd->height;

	for (i = 0; i < dp->top.plane_num; i++, plane++) {
		if (plane->enable) {
			plane->width = lcd->width;
			plane->height = lcd->height;
		}
	}

	/* initialize display device type */
	if (fmt == LCD_RGB) {
		dp->dev_type = DP_DEVICE_RGBLCD;

	} else if (fmt == LCD_HDMI) {
		struct dp_hdmi_dev *dev = (struct dp_hdmi_dev *)dp->device;

		dp->dev_type = DP_DEVICE_HDMI;
		if (lcd->width == 1920 && lcd->height == 1080)
			dev->preset = 1;
		else
			dev->preset = 0;

	} else {
		struct dp_lvds_dev *dev = (struct dp_lvds_dev *)dp->device;

		dp->dev_type = DP_DEVICE_LVDS;
		dev->lvds_format = (fmt & 0x3);
	}

	return 0;
}

/* --------------------------------------------------------------------------
 * initialize board status.
 */

#define	MMC_BOOT_CH0		(0)
#define	MMC_BOOT_CH1		(1 <<  3)
#define	MMC_BOOT_CH2		(1 << 19)

static void bd_bootdev_init(void)
{
	unsigned int rst = readl(PHY_BASEADDR_CLKPWR + SYSRSTCONFIG);

	rst &= (1 << 19) | (1 << 3);
	if (rst == MMC_BOOT_CH0) {
		/* mmc dev 1 for SD boot */
		mmc_boot_dev = 1;
	}
}

#ifdef CONFIG_S5P4418_ONEWIRE
static void bd_onewire_init(void)
{
	unsigned char lcd;
	unsigned short fw_ver;

	onewire_init();
	onewire_get_info(&lcd, &fw_ver);
}
#endif

static void bd_lcd_init(void)
{
	struct nxp_lcd *cfg;
	int id = -1;
	int ret;

#ifdef CONFIG_S5P4418_ONEWIRE
	id = onewire_get_lcd_id();
	/* -1: onwire probe failed
	 *  0: bad
	 * >0: identified
	 */
#endif
	ret = bd_setup_lcd_by_id(id);
	if (id <= 0 || ret != id) {
		printf("Panel: N/A (%d)\n", id);
		bd_setup_lcd_by_name("HDMI720P60");

	} else {
		printf("Panel: %s\n", bd_get_lcd_name());

		cfg = bd_get_lcd();
		if (cfg->gpio_init)
			cfg->gpio_init();
	}
}

static int mac_read_from_generic_eeprom(u8 *addr)
{
	return -1;
}

static void make_ether_addr(u8 *addr)
{
	u32 hash[20];

#define ETHER_MAC_TAG  "ethmac"
	memset(hash, 0, sizeof(hash));
	memcpy(hash + 12, ETHER_MAC_TAG, sizeof(ETHER_MAC_TAG));

	hash[4] = readl(PHY_BASEADDR_ECID + 0x00);
	hash[5] = readl(PHY_BASEADDR_ECID + 0x04);
	hash[6] = readl(PHY_BASEADDR_ECID + 0x08);
	hash[7] = readl(PHY_BASEADDR_ECID + 0x0c);

	md5((unsigned char *)&hash[4], 64, (unsigned char *)hash);

	hash[0] ^= hash[2];
	hash[1] ^= hash[3];

	memcpy(addr, (char *)hash, 6);
	addr[0] &= 0xfe;	/* clear multicast bit */
	addr[0] |= 0x02;
}

static void set_ether_addr(void)
{
	unsigned char mac[6];
	char ethaddr[20];
	int ret;

	if (env_get("ethaddr"))
		return;

	ret = mac_read_from_generic_eeprom(mac);
	if (ret < 0)
		make_ether_addr(mac);

	sprintf(ethaddr, "%02x:%02x:%02x:%02x:%02x:%02x",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	if (!ret)
		printf("MAC:  [%s]\n", ethaddr);

	env_set("ethaddr", ethaddr);
}

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
static void set_board_rev(void)
{
	char info[64] = {0, };

	snprintf(info, ARRAY_SIZE(info), "%02x", get_board_revision());
	env_set("board_rev", info);
}
#endif

static void set_dtb_name(void)
{
	char info[64] = {0, };

	snprintf(info, ARRAY_SIZE(info),
		 "s5p4418-nanopi2-rev%02x.dtb", get_board_revision());
	env_set("dtb_name", info);
}

static void bd_update_env(void)
{
	char *lcdtype = env_get("lcdtype");
	char *lcddpi = env_get("lcddpi");
	char *bootargs = env_get("bootargs");
	const char *name;
	char *p = NULL;
	int rootdev = board_mmc_bootdev();
	int need_save = 0;

#define CMDLINE_LCD		" lcd="
	char cmdline[CONFIG_SYS_CBSIZE];
	int n = 1;

	if (rootdev != CONFIG_ROOT_DEV && !env_get("firstboot")) {
		env_set_ulong("rootdev", rootdev);
		env_set("firstboot", "0");
		need_save = 1;
	}

	if (lcdtype) {
		/* Setup again as user specified LCD in env */
		bd_setup_lcd_by_name(lcdtype);
	}

	name = bd_get_lcd_name();

	if (bootargs)
		n = strlen(bootargs);	/* isn't 0 for NULL */
	else
		cmdline[0] = '\0';

	if ((n + strlen(name) + sizeof(CMDLINE_LCD)) > sizeof(cmdline)) {
		printf("Error: `bootargs' is too large (%d)\n", n);
		goto __exit;
	}

	if (bootargs) {
		p = strstr(bootargs, CMDLINE_LCD);
		if (p) {
			n = (p - bootargs);
			p += strlen(CMDLINE_LCD);
		}
		strncpy(cmdline, bootargs, n);
	}

	/* add `lcd=NAME,NUMdpi' */
	strncpy(cmdline + n, CMDLINE_LCD, strlen(CMDLINE_LCD));
	n += strlen(CMDLINE_LCD);

	strcpy(cmdline + n, name);
	n += strlen(name);

	if (lcddpi) {
		n += sprintf(cmdline + n, ",%sdpi", lcddpi);
	} else {
		int dpi = bd_get_lcd_density();

		if (dpi > 0 && dpi < 600)
			n += sprintf(cmdline + n, ",%ddpi", dpi);
	}

	/* copy remaining of bootargs */
	if (p) {
		p = strstr(p, " ");
		if (p) {
			strcpy(cmdline + n, p);
			n += strlen(p);
		}
	}

	/* append `bootdev=2' */
#define CMDLINE_BDEV	" bootdev="
	if (rootdev > 0 && !strstr(cmdline, CMDLINE_BDEV))
		n += sprintf(cmdline + n, "%s2", CMDLINE_BDEV);

	/* finally, let's update uboot env & save it */
	if (bootargs && strncmp(cmdline, bootargs, sizeof(cmdline))) {
		env_set("bootargs", cmdline);
		need_save = 1;
	}

__exit:
	if (need_save)
		env_save();
}

/* --------------------------------------------------------------------------
 * call from u-boot
 */

int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	bd_hwrev_init();
	bd_base_rev_init();

	bd_bootdev_init();
#ifdef CONFIG_S5P4418_ONEWIRE
	bd_onewire_init();
#endif

	bd_backlight_off();

	bd_lcd_config_gpio();
	bd_lcd_init();

	if (IS_ENABLED(CONFIG_SILENT_CONSOLE))
		gd->flags |= GD_FLG_SILENT;

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	bd_update_env();

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	set_board_rev();
#endif
	set_dtb_name();

	set_ether_addr();

	if (IS_ENABLED(CONFIG_SILENT_CONSOLE))
		gd->flags &= ~GD_FLG_SILENT;

	bd_backlight_on();
	printf("\n");

	return 0;
}
#endif

#ifdef CONFIG_SPLASH_SOURCE
#include <splash.h>
static struct splash_location splash_locations[] = {
	{
	.name = "mmc_fs",
	.storage = SPLASH_STORAGE_MMC,
	.flags = SPLASH_STORAGE_FS,
	.devpart = __stringify(CONFIG_ROOT_DEV) ":"
		   __stringify(CONFIG_BOOT_PART),
	},
};

int splash_screen_prepare(void)
{
	int err;
	char *env_cmd = env_get("load_splash");

	debug("%s()\n", __func__);

	if (env_cmd) {
		err = run_command(env_cmd, 0);

	} else {
		char devpart[64] = { 0, };
		int bootpart = env_get_ulong("bootpart", 0, CONFIG_BOOT_PART);
		int rootdev;

		if (env_get("firstboot"))
			rootdev = env_get_ulong("rootdev", 0, CONFIG_ROOT_DEV);
		else
			rootdev = board_mmc_bootdev();

		snprintf(devpart, ARRAY_SIZE(devpart), "%d:%d", rootdev,
			 bootpart);
		splash_locations[0].devpart = devpart;

		err = splash_source_load(splash_locations,
					 ARRAY_SIZE(splash_locations));
	}

	if (!err) {
		char addr[64];

		sprintf(addr, "0x%lx", gd->fb_base);
		env_set("fb_addr", addr);
	}

	return err;
}
#endif

/* u-boot dram initialize */
int dram_init(void)
{
	gd->ram_size = CFG_SYS_SDRAM_SIZE;
	return 0;
}

/* u-boot dram board specific */
int dram_init_banksize(void)
{
#define SCR_USER_SIG6_READ		(SCR_ALIVE_BASE + 0x0F0)
	unsigned int reg_val = readl(SCR_USER_SIG6_READ);

	/* set global data memory */
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x00000100;

	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size  = CFG_SYS_SDRAM_SIZE;

	/* Number of Row: 14 bits */
	if ((reg_val >> 28) == 14)
		gd->bd->bi_dram[0].size -= 0x20000000;

	/* Number of Memory Chips */
	if ((reg_val & 0x3) > 1) {
		gd->bd->bi_dram[1].start = 0x80000000;
		gd->bd->bi_dram[1].size  = 0x40000000;
	}
	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	int nodeoff;
	unsigned int rootdev;
	unsigned int fb_addr;

	if (board_mmc_bootdev() > 0) {
		rootdev = fdt_getprop_u32_default(blob, "/board", "sdidx", 2);
		if (rootdev) {
			/* find or create "/chosen" node. */
			nodeoff = fdt_find_or_add_subnode(blob, 0, "chosen");
			if (nodeoff >= 0)
				fdt_setprop_u32(blob, nodeoff, "linux,rootdev",
						rootdev);
		}
	}

	fb_addr = env_get_ulong("fb_addr", 0, 0);
	if (fb_addr) {
		nodeoff = fdt_path_offset(blob, "/reserved-memory");
		if (nodeoff < 0)
			return nodeoff;

		nodeoff = fdt_add_subnode(blob, nodeoff, "display_reserved");
		if (nodeoff >= 0) {
			fdt32_t cells[2];

			cells[0] = cpu_to_fdt32(fb_addr);
			cells[1] = cpu_to_fdt32(0x800000);

			fdt_setprop(blob, nodeoff, "reg", cells,
				    sizeof(cells[0]) * 2);
		}
	}

	return 0;
}
#endif
