// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Toradex, Inc.
 */

#include <dm.h>
#include <common.h>
#include <env.h>
#include <g_dnl.h>
#include <init.h>
#include <linux/libfdt.h>
#include <sysinfo.h>

#ifdef CONFIG_VIDEO
#include <bmp_logo.h>
#include <splash.h>
#include <video.h>
#endif

#include "tdx-cfg-block.h"
#include <asm/setup.h>
#include "tdx-common.h"

#define SERIAL_STR_LEN 8
#define MODULE_VER_STR_LEN 4 // V1.1
#define MODULE_REV_STR_LEN 3 // [A-Z] or #[26-99]

#ifdef CONFIG_TDX_CFG_BLOCK
static char tdx_serial_str[SERIAL_STR_LEN + 1];
static char tdx_board_rev_str[MODULE_VER_STR_LEN + MODULE_REV_STR_LEN + 1];

#ifdef CONFIG_TDX_CFG_BLOCK_EXTRA
static char tdx_car_serial_str[SERIAL_STR_LEN + 1];
static char tdx_car_rev_str[MODULE_VER_STR_LEN + MODULE_REV_STR_LEN + 1];
static const char *tdx_carrier_board_name;
#endif

#if defined(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)
u32 get_board_revision(void)
{
	/* Check validity */
	if (!tdx_hw_tag.ver_major)
		return 0;

	return ((tdx_hw_tag.ver_major & 0xff) << 8) |
		((tdx_hw_tag.ver_minor & 0xf) << 4) |
		((tdx_hw_tag.ver_assembly & 0xf) + 0xa);
}
#endif /* CONFIG_TDX_CFG_BLOCK */

#ifdef CONFIG_SERIAL_TAG
void get_board_serial(struct tag_serialnr *serialnr)
{
	int array[8];
	unsigned int serial = tdx_serial;
	int i;

	serialnr->low = 0;
	serialnr->high = 0;

	/* Check validity */
	if (serial) {
		/*
		 * Convert to Linux serial number format (hexadecimal coded
		 * decimal)
		 */
		i = 7;
		while (serial) {
			array[i--] = serial % 10;
			serial /= 10;
		}
		while (i >= 0)
			array[i--] = 0;
		serial = array[0];
		for (i = 1; i < 8; i++) {
			serial *= 16;
			serial += array[i];
		}

		serialnr->low = serial;
	}
}
#endif /* CONFIG_SERIAL_TAG */

static const char *get_board_assembly(u16 ver_assembly)
{
	static char ver_name[MODULE_REV_STR_LEN + 1];

	if (ver_assembly < 26) {
		ver_name[0] = (char)ver_assembly + 'A';
		ver_name[1] = '\0';
	} else {
		snprintf(ver_name, sizeof(ver_name),
			 "#%u", ver_assembly);
	}

	return ver_name;
}

__weak int print_bootinfo(void)
{
	return 0;
}

int checkboard(void)
{
	if (valid_cfgblock)
		printf("Serial#: %s\n", tdx_serial_str);

#ifdef CONFIG_TDX_CFG_BLOCK_EXTRA
	if (tdx_carrier_board_name)
		printf("Carrier: Toradex %s %s, Serial# %s\n",
		       tdx_carrier_board_name,
		       tdx_car_rev_str,
		       tdx_car_serial_str);
#endif

	print_bootinfo();

	return 0;
}

static int settings_r(void)
{
	unsigned char ethaddr[6];

	if (read_tdx_cfg_block()) {
		printf("MISSING TORADEX CONFIG BLOCK\n");

		/* Board can run even if config block is not present */
		return 0;
	}

	snprintf(tdx_serial_str, sizeof(tdx_serial_str),
		 "%08u", tdx_serial);
	snprintf(tdx_board_rev_str, sizeof(tdx_board_rev_str),
		 "V%1d.%1d%s",
		 tdx_hw_tag.ver_major,
		 tdx_hw_tag.ver_minor,
		 get_board_assembly(tdx_hw_tag.ver_assembly));

	env_set("serial#", tdx_serial_str);

#ifdef CONFIG_TDX_CFG_BLOCK_EXTRA
	if (read_tdx_cfg_block_carrier()) {
		printf("MISSING TORADEX CARRIER CONFIG BLOCKS\n");
		try_migrate_tdx_cfg_block_carrier();
	} else {
		tdx_carrier_board_name =
			get_toradex_carrier_boards(tdx_car_hw_tag.prodid);

		snprintf(tdx_car_serial_str, sizeof(tdx_car_serial_str),
			 "%08u", tdx_car_serial);
		snprintf(tdx_car_rev_str, sizeof(tdx_car_rev_str),
			 "V%1d.%1d%s",
			 tdx_car_hw_tag.ver_major,
			 tdx_car_hw_tag.ver_minor,
			 get_board_assembly(tdx_car_hw_tag.ver_assembly));

		env_set("carrier_serial#", tdx_car_serial_str);
	}
#endif

	/*
	 * Check if environment contains a valid MAC address,
	 * set the one from config block if not
	 */
	if (!eth_env_get_enetaddr("ethaddr", ethaddr))
		eth_env_set_enetaddr("ethaddr", (u8 *)&tdx_eth_addr);

	if (IS_ENABLED(CONFIG_TDX_CFG_BLOCK_2ND_ETHADDR) &&
	    !eth_env_get_enetaddr("eth1addr", ethaddr)) {
		/*
		 * Secondary MAC address is allocated from block
		 * 0x100000 higher then the first MAC address
		 */
		memcpy(ethaddr, &tdx_eth_addr, 6);
		ethaddr[3] += 0x10;
		eth_env_set_enetaddr("eth1addr", ethaddr);
	}

	return 0;
}
EVENT_SPY_SIMPLE(EVT_SETTINGS_R, settings_r);

static int tdx_detect(struct udevice *dev)
{
	return valid_cfgblock ? 0 : -EINVAL;
}

static int tdx_get_str(struct udevice *dev, int id, size_t size, char *val)
{
	int ret = -ENOTSUPP;

	switch (id) {
	case SYSINFO_ID_BOARD_MODEL:
		snprintf(val, size,
			 "Toradex %04d %s %s",
			 tdx_hw_tag.prodid,
			 toradex_modules[tdx_hw_tag.prodid].name,
			 tdx_board_rev_str);

		ret = 0;
	}

	return ret;
}

static const struct udevice_id sysinfo_tdx_ids[] = {
	{ .compatible = "toradex,sysinfo" },
	{ /* sentinel */ }
};

static const struct sysinfo_ops sysinfo_tdx_ops = {
	.detect		= tdx_detect,
	.get_str	= tdx_get_str,
};

U_BOOT_DRIVER(sysinfo_toradex) = {
	.name		= "sysinfo_toradex",
	.id		= UCLASS_SYSINFO,
	.of_match	= sysinfo_tdx_ids,
	.ops		= &sysinfo_tdx_ops,
};

#ifdef CONFIG_TDX_CFG_BLOCK_USB_GADGET_PID
int g_dnl_bind_fixup(struct usb_device_descriptor *dev, const char *name)
{
	unsigned short usb_pid;

	usb_pid = TORADEX_USB_PRODUCT_NUM_OFFSET + tdx_hw_tag.prodid;
	put_unaligned(usb_pid, &dev->idProduct);

	return 0;
}
#endif

#if defined(CONFIG_OF_LIBFDT)
int ft_common_board_setup(void *blob, struct bd_info *bd)
{
	if (tdx_serial) {
		fdt_setprop(blob, 0, "serial-number", tdx_serial_str,
			    strlen(tdx_serial_str) + 1);
	}

	if (tdx_hw_tag.ver_major) {
		char prod_id[5];

		snprintf(prod_id, sizeof(prod_id), "%04u", tdx_hw_tag.prodid);
		fdt_setprop(blob, 0, "toradex,product-id", prod_id, 5);

		fdt_setprop(blob, 0, "toradex,board-rev", tdx_board_rev_str,
			    strlen(tdx_board_rev_str) + 1);
	}

	return 0;
}
#endif

#else /* CONFIG_TDX_CFG_BLOCK */

#if defined(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)
u32 get_board_revision(void)
{
	return 0;
}
#endif /* CONFIG_REVISION_TAG */

#ifdef CONFIG_SERIAL_TAG
u32 get_board_serial(void)
{
	return 0;
}
#endif /* CONFIG_SERIAL_TAG */

int ft_common_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}

#endif /* CONFIG_TDX_CFG_BLOCK */
