// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Toradex, Inc.
 */

#include <common.h>
#include <environment.h>
#include <g_dnl.h>
#include <linux/libfdt.h>

#include "tdx-cfg-block.h"
#include <asm/setup.h>
#include "tdx-common.h"

#define TORADEX_OUI 0x00142dUL

#ifdef CONFIG_TDX_CFG_BLOCK
static char tdx_serial_str[9];
static char tdx_board_rev_str[6];

#ifdef CONFIG_REVISION_TAG
u32 get_board_rev(void)
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

int show_board_info(void)
{
	unsigned char ethaddr[6];

	if (read_tdx_cfg_block()) {
		printf("MISSING TORADEX CONFIG BLOCK\n");
		tdx_eth_addr.oui = htonl(TORADEX_OUI << 8);
		tdx_eth_addr.nic = htonl(tdx_serial << 8);
		checkboard();
	} else {
		sprintf(tdx_serial_str, "%08u", tdx_serial);
		sprintf(tdx_board_rev_str, "V%1d.%1d%c",
			tdx_hw_tag.ver_major,
			tdx_hw_tag.ver_minor,
			(char)tdx_hw_tag.ver_assembly + 'A');

		env_set("serial#", tdx_serial_str);

		printf("Model: Toradex %s %s, Serial# %s\n",
		       toradex_modules[tdx_hw_tag.prodid],
		       tdx_board_rev_str,
		       tdx_serial_str);
	}

	/*
	 * Check if environment contains a valid MAC address,
	 * set the one from config block if not
	 */
	if (!eth_env_get_enetaddr("ethaddr", ethaddr))
		eth_env_set_enetaddr("ethaddr", (u8 *)&tdx_eth_addr);

#ifdef CONFIG_TDX_CFG_BLOCK_2ND_ETHADDR
	if (!eth_env_get_enetaddr("eth1addr", ethaddr)) {
		/*
		 * Secondary MAC address is allocated from block
		 * 0x100000 higher then the first MAC address
		 */
		memcpy(ethaddr, &tdx_eth_addr, 6);
		ethaddr[3] += 0x10;
		eth_env_set_enetaddr("eth1addr", ethaddr);
	}
#endif

	return 0;
}

#ifdef CONFIG_USB_GADGET_DOWNLOAD
int g_dnl_bind_fixup(struct usb_device_descriptor *dev, const char *name)
{
	unsigned short usb_pid;

	usb_pid = TORADEX_USB_PRODUCT_NUM_OFFSET + tdx_hw_tag.prodid;
	put_unaligned(usb_pid, &dev->idProduct);

	return 0;
}
#endif

#if defined(CONFIG_OF_LIBFDT)
int ft_common_board_setup(void *blob, bd_t *bd)
{
	if (tdx_serial) {
		fdt_setprop(blob, 0, "serial-number", tdx_serial_str,
			    strlen(tdx_serial_str) + 1);
	}

	if (tdx_hw_tag.ver_major) {
		char prod_id[5];

		sprintf(prod_id, "%04u", tdx_hw_tag.prodid);
		fdt_setprop(blob, 0, "toradex,product-id", prod_id, 5);

		fdt_setprop(blob, 0, "toradex,board-rev", tdx_board_rev_str,
			    strlen(tdx_board_rev_str) + 1);
	}

	return 0;
}
#endif

#else /* CONFIG_TDX_CFG_BLOCK */

#ifdef CONFIG_REVISION_TAG
u32 get_board_rev(void)
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

int ft_common_board_setup(void *blob, bd_t *bd)
{
	return 0;
}

#endif /* CONFIG_TDX_CFG_BLOCK */
