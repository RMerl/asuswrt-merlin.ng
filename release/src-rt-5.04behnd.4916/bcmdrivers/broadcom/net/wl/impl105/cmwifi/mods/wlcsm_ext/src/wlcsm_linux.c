/*
 * The wlcsm kernel module
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * <<Broadcom-WL-IPTag/Open:>
 *
 * $Id: wlcsm_linux.c 832801 2023-11-13 20:14:38Z $
 */

#include <linux/module.h>

extern void wlcsm_module_exit(void);
extern int wlcsm_module_init(void);

static int __init
wl_module_init(void) {
	return 	wlcsm_module_init();
}

static void __exit
wl_module_exit(void)
{
	wlcsm_module_exit();
}

module_init(wl_module_init);
module_exit(wl_module_exit);
MODULE_LICENSE("Dual MIT/GPL");
