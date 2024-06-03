// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Microchip
 *		      Wenyou Yang <wenyou.yang@microchip.com>
 */

#include <common.h>
#include <atmel_lcd.h>
#include <dm.h>
#include <nand.h>
#include <version.h>
#include <video.h>
#include <video_console.h>
#include <asm/io.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;

int at91_video_show_board_info(void)
{
	struct vidconsole_priv *priv;
	ulong dram_size, nand_size;
	int i;
	u32 len = 0;
	char buf[255];
	char *corp = "2017 Microchip Technology Inc.\n";
	char temp[32];
	struct udevice *dev, *con;
	const char *s;
	vidinfo_t logo_info;
	int ret;

	len += sprintf(&buf[len], "%s\n", U_BOOT_VERSION);
	memcpy(&buf[len], corp, strlen(corp));
	len += strlen(corp);
	len += sprintf(&buf[len], "%s CPU at %s MHz\n", get_cpu_name(),
			strmhz(temp, get_cpu_clk_rate()));

	dram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		dram_size += gd->bd->bi_dram[i].size;

	nand_size = 0;
#ifdef CONFIG_NAND_ATMEL
	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
		nand_size += get_nand_dev_by_index(i)->size;
#endif

	len += sprintf(&buf[len], "%ld MB SDRAM, %ld MB NAND\n",
		       dram_size >> 20, nand_size >> 20);

	ret = uclass_get_device(UCLASS_VIDEO, 0, &dev);
	if (ret)
		return ret;

	microchip_logo_info(&logo_info);
	ret = video_bmp_display(dev, logo_info.logo_addr,
				logo_info.logo_x_offset,
				logo_info.logo_y_offset, false);
	if (ret)
		return ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &con);
	if (ret)
		return ret;

	priv = dev_get_uclass_priv(con);
	vidconsole_position_cursor(con, 0, (logo_info.logo_height +
				   priv->y_charsize - 1) / priv->y_charsize);
	for (s = buf, i = 0; i < len; s++, i++)
		vidconsole_put_char(con, *s);

	return 0;
}
