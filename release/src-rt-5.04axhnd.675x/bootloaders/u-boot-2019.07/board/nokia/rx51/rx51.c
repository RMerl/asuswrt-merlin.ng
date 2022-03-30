// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012
 * Ивайло Димитров <freemangordon@abv.bg>
 *
 * (C) Copyright 2011-2012
 * Pali Rohár <pali.rohar@gmail.com>
 *
 * (C) Copyright 2010
 * Alistair Buxton <a.j.buxton@gmail.com>
 *
 * Derived from Beagle Board and 3430 SDP code:
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Sunil Kumar <sunilsaini05@gmail.com>
 *	Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 */

#include <common.h>
#include <watchdog.h>
#include <malloc.h>
#include <twl4030.h>
#include <i2c.h>
#include <video_fb.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <asm/bitops.h>
#include <asm/mach-types.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>

#include "rx51.h"
#include "tag_omap.h"

DECLARE_GLOBAL_DATA_PTR;

GraphicDevice gdev;

const omap3_sysinfo sysinfo = {
	DDR_STACKED,
	"Nokia RX-51",
	"OneNAND"
};

/* This structure contains default omap tags needed for booting Maemo 5 */
static struct tag_omap omap[] = {
	OMAP_TAG_UART_CONFIG(0x04),
	OMAP_TAG_SERIAL_CONSOLE_CONFIG(0x03, 0x01C200),
	OMAP_TAG_LCD_CONFIG("acx565akm", "internal", 90, 0x18),
	OMAP_TAG_GPIO_SWITCH_CONFIG("cam_focus", 0x44, 0x1, 0x2, 0x0),
	OMAP_TAG_GPIO_SWITCH_CONFIG("cam_launch", 0x45, 0x1, 0x2, 0x0),
	OMAP_TAG_GPIO_SWITCH_CONFIG("cam_shutter", 0x6e, 0x1, 0x0, 0x0),
	OMAP_TAG_GPIO_SWITCH_CONFIG("cmt_apeslpx", 0x46, 0x2, 0x2, 0x0),
	OMAP_TAG_GPIO_SWITCH_CONFIG("cmt_bsi", 0x9d, 0x2, 0x2, 0x0),
	OMAP_TAG_GPIO_SWITCH_CONFIG("cmt_en", 0x4a, 0x2, 0x2, 0x0),
	OMAP_TAG_GPIO_SWITCH_CONFIG("cmt_rst", 0x4b, 0x6, 0x2, 0x0),
	OMAP_TAG_GPIO_SWITCH_CONFIG("cmt_rst_rq", 0x49, 0x6, 0x2, 0x0),
	OMAP_TAG_GPIO_SWITCH_CONFIG("cmt_wddis", 0x0d, 0x2, 0x2, 0x0),
	OMAP_TAG_GPIO_SWITCH_CONFIG("headphone", 0xb1, 0x1, 0x1, 0x0),
	OMAP_TAG_GPIO_SWITCH_CONFIG("kb_lock", 0x71, 0x1, 0x0, 0x0),
	OMAP_TAG_GPIO_SWITCH_CONFIG("proximity", 0x59, 0x0, 0x0, 0x0),
	OMAP_TAG_GPIO_SWITCH_CONFIG("sleep_ind", 0xa2, 0x2, 0x2, 0x0),
	OMAP_TAG_GPIO_SWITCH_CONFIG("slide", GPIO_SLIDE, 0x0, 0x0, 0x0),
	OMAP_TAG_WLAN_CX3110X_CONFIG(0x25, 0xff, 87, 42, -1),
	OMAP_TAG_PARTITION_CONFIG(PART1_NAME, PART1_SIZE * PART1_MULL,
			PART1_OFFS, PART1_MASK),
	OMAP_TAG_PARTITION_CONFIG(PART2_NAME, PART2_SIZE * PART2_MULL,
			PART2_OFFS, PART2_MASK),
	OMAP_TAG_PARTITION_CONFIG(PART3_NAME, PART3_SIZE * PART3_MULL,
			PART3_OFFS, PART3_MASK),
	OMAP_TAG_PARTITION_CONFIG(PART4_NAME, PART4_SIZE * PART4_MULL,
			PART4_OFFS, PART4_MASK),
	OMAP_TAG_PARTITION_CONFIG(PART5_NAME, PART5_SIZE * PART5_MULL,
			PART5_OFFS, PART5_MASK),
	OMAP_TAG_PARTITION_CONFIG(PART6_NAME, PART6_SIZE * PART6_MULL,
			PART6_OFFS, PART6_MASK),
	OMAP_TAG_BOOT_REASON_CONFIG("pwr_key"),
	OMAP_TAG_VERSION_STR_CONFIG("product", "RX-51"),
	OMAP_TAG_VERSION_STR_CONFIG("hw-build", "2101"),
	OMAP_TAG_VERSION_STR_CONFIG("nolo", "1.4.14"),
	OMAP_TAG_VERSION_STR_CONFIG("boot-mode", "normal"),
	{ }
};

static char *boot_reason_ptr;
static char *hw_build_ptr;
static char *nolo_version_ptr;
static char *boot_mode_ptr;

/*
 * Routine: init_omap_tags
 * Description: Initialize pointers to values in tag_omap
 */
static void init_omap_tags(void)
{
	char *component;
	char *version;
	int i = 0;
	while (omap[i].hdr.tag) {
		switch (omap[i].hdr.tag) {
		case OMAP_TAG_BOOT_REASON:
			boot_reason_ptr = omap[i].u.boot_reason.reason_str;
			break;
		case OMAP_TAG_VERSION_STR:
			component = omap[i].u.version.component;
			version = omap[i].u.version.version;
			if (strcmp(component, "hw-build") == 0)
				hw_build_ptr = version;
			else if (strcmp(component, "nolo") == 0)
				nolo_version_ptr = version;
			else if (strcmp(component, "boot-mode") == 0)
				boot_mode_ptr = version;
			break;
		default:
			break;
		}
		i++;
	}
}

static void reuse_omap_atags(struct tag_omap *t)
{
	char *component;
	char *version;
	while (t->hdr.tag) {
		switch (t->hdr.tag) {
		case OMAP_TAG_BOOT_REASON:
			memset(boot_reason_ptr, 0, 12);
			strcpy(boot_reason_ptr, t->u.boot_reason.reason_str);
			break;
		case OMAP_TAG_VERSION_STR:
			component = t->u.version.component;
			version = t->u.version.version;
			if (strcmp(component, "hw-build") == 0) {
				memset(hw_build_ptr, 0, 12);
				strcpy(hw_build_ptr, version);
			} else if (strcmp(component, "nolo") == 0) {
				memset(nolo_version_ptr, 0, 12);
				strcpy(nolo_version_ptr, version);
			} else if (strcmp(component, "boot-mode") == 0) {
				memset(boot_mode_ptr, 0, 12);
				strcpy(boot_mode_ptr, version);
			}
			break;
		default:
			break;
		}
		t = tag_omap_next(t);
	}
}

/*
 * Routine: reuse_atags
 * Description: Reuse atags from previous bootloader.
 *              Reuse only only HW build, boot reason, boot mode and nolo
 */
static void reuse_atags(void)
{
	struct tag *t = (struct tag *)gd->bd->bi_boot_params;

	/* First tag must be ATAG_CORE */
	if (t->hdr.tag != ATAG_CORE)
		return;

	if (!boot_reason_ptr || !hw_build_ptr)
		return;

	/* Last tag must be ATAG_NONE */
	while (t->hdr.tag != ATAG_NONE) {
		switch (t->hdr.tag) {
		case ATAG_REVISION:
			memset(hw_build_ptr, 0, 12);
			sprintf(hw_build_ptr, "%x", t->u.revision.rev);
			break;
		case ATAG_BOARD:
			reuse_omap_atags((struct tag_omap *)&t->u);
			break;
		default:
			break;
		}
		t = tag_next(t);
	}
}

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	/* in SRAM or SDRAM, finish GPMC */
	gpmc_init();
	/* boot param addr */
	gd->bd->bi_boot_params = OMAP34XX_SDRC_CS0 + 0x100;
	return 0;
}

/*
 * Routine: get_board_revision
 * Description: Return board revision.
 */
u32 get_board_rev(void)
{
	return simple_strtol(hw_build_ptr, NULL, 16);
}

/*
 * Routine: setup_board_tags
 * Description: Append board specific boot tags.
 */
void setup_board_tags(struct tag **in_params)
{
	int setup_console_atag;
	char *setup_boot_reason_atag;
	char *setup_boot_mode_atag;
	char *str;
	int i;
	int size;
	int total_size;
	struct tag *params;
	struct tag_omap *t;

	params = (struct tag *)gd->bd->bi_boot_params;

	params->u.core.flags = 0x0;
	params->u.core.pagesize = 0x1000;
	params->u.core.rootdev = 0x0;

	/* append omap atag only if env setup_omap_atag is set to 1 */
	str = env_get("setup_omap_atag");
	if (!str || str[0] != '1')
		return;

	str = env_get("setup_console_atag");
	if (str && str[0] == '1')
		setup_console_atag = 1;
	else
		setup_console_atag = 0;

	setup_boot_reason_atag = env_get("setup_boot_reason_atag");
	setup_boot_mode_atag = env_get("setup_boot_mode_atag");

	params = *in_params;
	t = (struct tag_omap *)&params->u;
	total_size = sizeof(struct tag_header);

	for (i = 0; omap[i].hdr.tag; i++) {

		/* skip serial console tag */
		if (!setup_console_atag &&
			omap[i].hdr.tag == OMAP_TAG_SERIAL_CONSOLE)
			continue;

		size = omap[i].hdr.size + sizeof(struct tag_omap_header);
		memcpy(t, &omap[i], size);

		/* set uart tag to 0 - disable serial console */
		if (!setup_console_atag && omap[i].hdr.tag == OMAP_TAG_UART)
			t->u.uart.enabled_uarts = 0;

		/* change boot reason */
		if (setup_boot_reason_atag &&
			omap[i].hdr.tag == OMAP_TAG_BOOT_REASON) {
			memset(t->u.boot_reason.reason_str, 0, 12);
			strcpy(t->u.boot_reason.reason_str,
				setup_boot_reason_atag);
		}

		/* change boot mode */
		if (setup_boot_mode_atag &&
			omap[i].hdr.tag == OMAP_TAG_VERSION_STR &&
			strcmp(omap[i].u.version.component, "boot-mode") == 0) {
			memset(t->u.version.version, 0, 12);
			strcpy(t->u.version.version, setup_boot_mode_atag);
		}

		total_size += size;
		t = tag_omap_next(t);

	}

	params->hdr.tag = ATAG_BOARD;
	params->hdr.size = total_size >> 2;
	params = tag_next(params);

	*in_params = params;
}

/*
 * Routine: video_hw_init
 * Description: Set up the GraphicDevice depending on sys_boot.
 */
void *video_hw_init(void)
{
	/* fill in Graphic Device */
	gdev.frameAdrs = 0x8f9c0000;
	gdev.winSizeX = 800;
	gdev.winSizeY = 480;
	gdev.gdfBytesPP = 2;
	gdev.gdfIndex = GDF_16BIT_565RGB;
	memset((void *)gdev.frameAdrs, 0, 0xbb800);
	return (void *) &gdev;
}

/*
 * Routine: twl4030_regulator_set_mode
 * Description: Set twl4030 regulator mode over i2c powerbus.
 */
static void twl4030_regulator_set_mode(u8 id, u8 mode)
{
	u16 msg = MSG_SINGULAR(DEV_GRP_P1, id, mode);
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER,
			     TWL4030_PM_MASTER_PB_WORD_MSB, msg >> 8);
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER,
			     TWL4030_PM_MASTER_PB_WORD_LSB, msg & 0xff);
}

static void omap3_emu_romcode_call(u32 service_id, u32 *parameters)
{
	u32 i, num_params = *parameters;
	u32 *sram_scratch_space = (u32 *)OMAP3_PUBLIC_SRAM_SCRATCH_AREA;

	/*
	 * copy the parameters to an un-cached area to avoid coherency
	 * issues
	 */
	for (i = 0; i < num_params; i++) {
		__raw_writel(*parameters, sram_scratch_space);
		parameters++;
		sram_scratch_space++;
	}

	/* Now make the PPA call */
	do_omap3_emu_romcode_call(service_id, OMAP3_PUBLIC_SRAM_SCRATCH_AREA);
}

void omap3_set_aux_cr_secure(u32 acr)
{
	struct emu_hal_params_rx51 emu_romcode_params = { 0, };

	emu_romcode_params.num_params = 2;
	emu_romcode_params.param1 = acr;

	omap3_emu_romcode_call(OMAP3_EMU_HAL_API_WRITE_ACR,
			       (u32 *)&emu_romcode_params);
}

/*
 * Routine: omap3_update_aux_cr_secure_rx51
 * Description: Modify the contents Auxiliary Control Register.
 * Parameters:
 *   set_bits - bits to set in ACR
 *   clr_bits - bits to clear in ACR
 */
static void omap3_update_aux_cr_secure_rx51(u32 set_bits, u32 clear_bits)
{
	u32 acr;

	/* Read ACR */
	asm volatile ("mrc p15, 0, %0, c1, c0, 1" : "=r" (acr));
	acr &= ~clear_bits;
	acr |= set_bits;
	omap3_set_aux_cr_secure(acr);
}

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts.
 */
int misc_init_r(void)
{
	char buf[12];
	u8 state;

	/* reset lp5523 led */
	i2c_set_bus_num(1);
	state = 0xff;
	i2c_write(0x32, 0x3d, 1, &state, 1);
	i2c_set_bus_num(0);

	/* initialize twl4030 power managment */
	twl4030_power_init();

	/* set VSIM to 1.8V */
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VSIM_DEDICATED,
				TWL4030_PM_RECEIVER_VSIM_VSEL_18,
				TWL4030_PM_RECEIVER_VSIM_DEV_GRP,
				TWL4030_PM_RECEIVER_DEV_GRP_P1);

	/* store I2C access state */
	twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER, TWL4030_PM_MASTER_PB_CFG,
			    &state);

	/* enable I2C access to powerbus (needed for twl4030 regulator) */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER, TWL4030_PM_MASTER_PB_CFG,
			     0x02);

	/* set VAUX3, VSIM and VMMC1 state to active - enable eMMC memory */
	twl4030_regulator_set_mode(RES_VAUX3, RES_STATE_ACTIVE);
	twl4030_regulator_set_mode(RES_VSIM, RES_STATE_ACTIVE);
	twl4030_regulator_set_mode(RES_VMMC1, RES_STATE_ACTIVE);

	/* restore I2C access state */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER, TWL4030_PM_MASTER_PB_CFG,
			     state);

	/* set env variable attkernaddr for relocated kernel */
	sprintf(buf, "%#x", KERNEL_ADDRESS);
	env_set("attkernaddr", buf);

	/* initialize omap tags */
	init_omap_tags();

	/* reuse atags from previous bootloader */
	reuse_atags();

	omap_die_id_display();
	print_cpuinfo();

	/*
	 * Cortex-A8(r1p0..r1p2) errata 430973 workaround
	 * Set IBE bit in Auxiliary Control Register
	 *
	 * Call this routine only on real secure device
	 * Qemu does not implement secure PPA and crash
	 */
	if (get_device_type() == HS_DEVICE)
		omap3_update_aux_cr_secure_rx51(1 << 6, 0);

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_RX51();
}

static unsigned long int twl_wd_time; /* last time of watchdog reset */
static unsigned long int twl_i2c_lock;

/*
 * Routine: hw_watchdog_reset
 * Description: Reset timeout of twl4030 watchdog.
 */
void hw_watchdog_reset(void)
{
	u8 timeout = 0;

	/* do not reset watchdog too often - max every 4s */
	if (get_timer(twl_wd_time) < 4 * CONFIG_SYS_HZ)
		return;

	/* localy lock twl4030 i2c bus */
	if (test_and_set_bit(0, &twl_i2c_lock))
		return;

	/* read actual watchdog timeout */
	twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER,
			    TWL4030_PM_RECEIVER_WATCHDOG_CFG, &timeout);

	/* timeout 0 means watchdog is disabled */
	/* reset watchdog timeout to 31s (maximum) */
	if (timeout != 0)
		twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER,
				     TWL4030_PM_RECEIVER_WATCHDOG_CFG, 31);

	/* store last watchdog reset time */
	twl_wd_time = get_timer(0);

	/* localy unlock twl4030 i2c bus */
	test_and_clear_bit(0, &twl_i2c_lock);
}

/*
 * TWL4030 keypad handler for cfb_console
 */

static const char keymap[] = {
	/* normal */
	'q',  'o',  'p',  ',', '\b',    0,  'a',  's',
	'w',  'd',  'f',  'g',  'h',  'j',  'k',  'l',
	'e',  '.',    0,  '\r',   0,  'z',  'x',  'c',
	'r',  'v',  'b',  'n',  'm',  ' ',  ' ',    0,
	't',    0,    0,    0,    0,    0,    0,    0,
	'y',    0,    0,    0,    0,    0,    0,    0,
	'u',    0,    0,    0,    0,    0,    0,    0,
	'i',    5,    6,    0,    0,    0,    0,    0,
	/* fn */
	'1',  '9',  '0',  '=', '\b',    0,  '*',  '+',
	'2',  '#',  '-',  '_',  '(',  ')',  '&',  '!',
	'3',  '?',  '^', '\r',    0,  156,  '$',  238,
	'4',  '/', '\\',  '"', '\'',  '@',    0,  '<',
	'5',  '|',  '>',    0,    0,    0,    0,    0,
	'6',    0,    0,    0,    0,    0,    0,    0,
	'7',    0,    0,    0,    0,    0,    0,    0,
	'8',   16,   17,    0,    0,    0,    0,    0,
};

static u8 keys[8];
static u8 old_keys[8] = {0, 0, 0, 0, 0, 0, 0, 0};
#define KEYBUF_SIZE 32
static u8 keybuf[KEYBUF_SIZE];
static u8 keybuf_head;
static u8 keybuf_tail;

/*
 * Routine: rx51_kp_init
 * Description: Initialize HW keyboard.
 */
int rx51_kp_init(void)
{
	int ret = 0;
	u8 ctrl;
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_KEYPAD,
				  TWL4030_KEYPAD_KEYP_CTRL_REG, &ctrl);

	if (ret)
		return ret;

	/* turn on keyboard and use hardware scanning */
	ctrl |= TWL4030_KEYPAD_CTRL_KBD_ON;
	ctrl |= TWL4030_KEYPAD_CTRL_SOFT_NRST;
	ctrl |= TWL4030_KEYPAD_CTRL_SOFTMODEN;
	ret |= twl4030_i2c_write_u8(TWL4030_CHIP_KEYPAD,
				    TWL4030_KEYPAD_KEYP_CTRL_REG, ctrl);
	/* enable key event status */
	ret |= twl4030_i2c_write_u8(TWL4030_CHIP_KEYPAD,
				    TWL4030_KEYPAD_KEYP_IMR1, 0xfe);
	/* enable interrupt generation on rising and falling */
	/* this is a workaround for qemu twl4030 emulation */
	ret |= twl4030_i2c_write_u8(TWL4030_CHIP_KEYPAD,
				    TWL4030_KEYPAD_KEYP_EDR, 0x57);
	/* enable ISR clear on read */
	ret |= twl4030_i2c_write_u8(TWL4030_CHIP_KEYPAD,
				    TWL4030_KEYPAD_KEYP_SIH_CTRL, 0x05);
	return 0;
}

static void rx51_kp_fill(u8 k, u8 mods)
{
	/* check if some cursor key without meta fn key was pressed */
	if (!(mods & 2) && (k == 18 || k == 31 || k == 33 || k == 34)) {
		keybuf[keybuf_tail++] = '\e';
		keybuf_tail %= KEYBUF_SIZE;
		keybuf[keybuf_tail++] = '[';
		keybuf_tail %= KEYBUF_SIZE;
		if (k == 18) /* up */
			keybuf[keybuf_tail++] = 'A';
		else if (k == 31) /* left */
			keybuf[keybuf_tail++] = 'D';
		else if (k == 33) /* down */
			keybuf[keybuf_tail++] = 'B';
		else if (k == 34) /* right */
			keybuf[keybuf_tail++] = 'C';
		keybuf_tail %= KEYBUF_SIZE;
		return;
	}

	if (mods & 2) { /* fn meta key was pressed */
		k = keymap[k+64];
	} else {
		k = keymap[k];
		if (mods & 1) { /* ctrl key was pressed */
			if (k >= 'a' && k <= 'z')
				k -= 'a' - 1;
		}
		if (mods & 4) { /* shift key was pressed */
			if (k >= 'a' && k <= 'z')
				k += 'A' - 'a';
			else if (k == '.')
				k = ':';
			else if (k == ',')
				k = ';';
		}
	}
	keybuf[keybuf_tail++] = k;
	keybuf_tail %= KEYBUF_SIZE;
}

/*
 * Routine: rx51_kp_tstc
 * Description: Test if key was pressed (from buffer).
 */
int rx51_kp_tstc(struct stdio_dev *sdev)
{
	u8 c, r, dk, i;
	u8 intr;
	u8 mods;

	/* localy lock twl4030 i2c bus */
	if (test_and_set_bit(0, &twl_i2c_lock))
		return 0;

	/* twl4030 remembers up to 2 events */
	for (i = 0; i < 2; i++) {

		/* check interrupt register for events */
		twl4030_i2c_read_u8(TWL4030_CHIP_KEYPAD,
				    TWL4030_KEYPAD_KEYP_ISR1 + (2 * i), &intr);

		/* no event */
		if (!(intr&1))
			continue;

		/* read the key state */
		i2c_read(TWL4030_CHIP_KEYPAD,
			TWL4030_KEYPAD_FULL_CODE_7_0, 1, keys, 8);

		/* cut out modifier keys from the keystate */
		mods = keys[4] >> 4;
		keys[4] &= 0x0f;

		for (c = 0; c < 8; c++) {

			/* get newly pressed keys only */
			dk = ((keys[c] ^ old_keys[c])&keys[c]);
			old_keys[c] = keys[c];

			/* fill the keybuf */
			for (r = 0; r < 8; r++) {
				if (dk&1)
					rx51_kp_fill((c*8)+r, mods);
				dk = dk >> 1;
			}

		}

	}

	/* localy unlock twl4030 i2c bus */
	test_and_clear_bit(0, &twl_i2c_lock);

	return (KEYBUF_SIZE + keybuf_tail - keybuf_head)%KEYBUF_SIZE;
}

/*
 * Routine: rx51_kp_getc
 * Description: Get last pressed key (from buffer).
 */
int rx51_kp_getc(struct stdio_dev *sdev)
{
	keybuf_head %= KEYBUF_SIZE;
	while (!rx51_kp_tstc(sdev))
		WATCHDOG_RESET();
	return keybuf[keybuf_head++];
}

/*
 * Routine: board_mmc_init
 * Description: Initialize mmc devices.
 */
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0, 0, 0, -1, -1);
	omap_mmc_init(1, 0, 0, -1, -1);
	return 0;
}

void board_mmc_power_init(void)
{
	twl4030_power_mmc_init(0);
	twl4030_power_mmc_init(1);
}
