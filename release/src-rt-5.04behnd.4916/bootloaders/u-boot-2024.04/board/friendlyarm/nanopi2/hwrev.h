/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright (C) Guangzhou FriendlyARM Computer Tech. Co., Ltd.
 * (http://www.friendlyarm.com)
 */

#ifndef __BD_HW_REV_H__
#define __BD_HW_REV_H__

extern void bd_hwrev_init(void);
extern void bd_base_rev_init(void);
extern u32 get_board_revision(void);
extern const char *get_board_name(void);

#endif /* __BD_HW_REV_H__ */
