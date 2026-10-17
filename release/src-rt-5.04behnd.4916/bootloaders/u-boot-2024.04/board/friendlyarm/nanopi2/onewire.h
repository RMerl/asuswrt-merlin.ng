/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright (C) Guangzhou FriendlyARM Computer Tech. Co., Ltd.
 * (http://www.friendlyarm.com)
 */

#ifndef __ONE_WIRE_H__
#define __ONE_WIRE_H__

extern void onewire_init(void);
extern int  onewire_get_info(unsigned char *lcd, unsigned short *fw_ver);
extern int  onewire_get_lcd_id(void);
extern int  onewire_set_backlight(int brightness);

#endif /* __ONE_WIRE_H__ */
