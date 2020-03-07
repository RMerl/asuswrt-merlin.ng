
/*
 * HND SRC OBJECT FILE: These functions handle the src object files in 4908
 * and the emf, igs, wl, dhd drivers.
 *
 * Copyright (C) 2019, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: hnd_c.c 692718 2017-03-29 03:25:07Z $
 */
#include <linux/module.h>
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>
#include <typedefs.h>

MODULE_LICENSE("GPL and additional rights");

#define SUCCESS                     0
#define FAILURE                     -1
#define printf(fmt, args...)    printk(fmt , ## args)
#define HND_DEBUG(fmt, args...)	printf(fmt, ##args)

static int32 __init
hnd_module_init(void)
{
	HND_DEBUG("Loading HND Module\n");
	return (SUCCESS);
}

static void __exit
hnd_module_exit(void)
{
	HND_DEBUG("Exiting HND Module\n");
	return;
}

module_init(hnd_module_init);
module_exit(hnd_module_exit);
