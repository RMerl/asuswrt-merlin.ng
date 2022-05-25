/*
 * Routine to access external NVRAM for STB
 *
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
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
 * $Id: bcmsromio.c $
 */

#ifdef WLC_LOW
#ifndef LINUX_VERSION_CODE
#include <linuxver.h>
#endif

#define MAX_SROM_FILE_SIZE SROM_MAX

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <osl.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#include <typedefs.h>
#include <bcmdevs.h>
#include "bcmsrom_fmt.h"
#include "siutils.h"
#include "bcmutils.h"

int BCMATTACHFN(init_sromvars_map)(si_t *sih, uint chipId, void *buf, uint nbytes);

int BCMATTACHFN(init_sromvars_map)(si_t *sih, uint chipId, void *buf, uint nbytes)
{
	void *fp = NULL;
	char fname[32];
	int ret = 0;

	sprintf(fname, "/etc/wlan/bcm%x_vars.bin", chipId);

	fp = (void*)osl_os_open_image(fname);
	if (fp != NULL) {
		while (osl_os_get_image_block(buf, nbytes, fp));
		osl_os_close_image(fp);
	}
	else {
		printk("Could not open %s file\n", fname);
		ret = -1;
	}

	return ret;
}
#else
/* no longer maintained for linux 2.4, compare above */
#error "kernel version not supported"

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0) */
#endif /* WLC_LOW */
