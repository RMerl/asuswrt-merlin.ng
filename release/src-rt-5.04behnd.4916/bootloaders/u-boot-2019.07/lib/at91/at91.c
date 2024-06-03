// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Microchip
 *		 Wenyou.Yang <wenyou.yang@microchip.com>
 */

#include <common.h>
#include <atmel_lcd.h>

#include "atmel_logo_8bpp.h"
#include "microchip_logo_8bpp.h"

void atmel_logo_info(vidinfo_t *info)
{
	info->logo_width = ATMEL_LOGO_8BPP_WIDTH;
	info->logo_height = ATMEL_LOGO_8BPP_HEIGHT;
	info->logo_x_offset = ATMEL_LOGO_8BPP_X_OFFSET;
	info->logo_y_offset = ATMEL_LOGO_8BPP_X_OFFSET;
	info->logo_addr = (u_long)atmel_logo_8bpp;
}

void microchip_logo_info(vidinfo_t *info)
{
	info->logo_width = MICROCHIP_LOGO_8BPP_WIDTH;
	info->logo_height = MICROCHIP_LOGO_8BPP_HEIGHT;
	info->logo_x_offset = MICROCHIP_LOGO_8BPP_X_OFFSET;
	info->logo_y_offset = MICROCHIP_LOGO_8BPP_X_OFFSET;
	info->logo_addr = (u_long)microchip_logo_8bpp;
}
