/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 Toradex, Inc.
 */

#ifndef _TDX_COMMON_H
#define _TDX_COMMON_H

#define TORADEX_USB_PRODUCT_NUM_OFFSET	0x4000
#define TDX_USB_VID			0x1B67

int ft_common_board_setup(void *blob, struct bd_info *bd);
u32 get_board_revision(void);

#endif /* _TDX_COMMON_H */
