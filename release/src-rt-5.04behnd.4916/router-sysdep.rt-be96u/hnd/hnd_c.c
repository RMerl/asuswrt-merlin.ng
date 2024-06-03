
/*
 * HND SRC OBJECT FILE: These functions handle the src object files in 4908
 * and the emf, igs, wl, dhd drivers.
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
#include <linux/version.h>
#include <linux/init.h>
#include <linux/notifier.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
#include <linux/panic_notifier.h>
#endif /* KERNEL >= 5.14 */
#include <typedefs.h>
#include <bcmutils.h>
#if defined(HND_LINUX)
#include <hnd_linux.h>
#endif /* HND_LINUX */

MODULE_LICENSE("GPL and additional rights");

#define SUCCESS                     0
#define FAILURE                     -1
#define HND_DEBUG(fmt, args...)	printf(fmt, ##args)

#if defined(MLO_IPC)
char *wl_mlo_config = "-1 -1 -1 -1";
module_param(wl_mlo_config, charp, 0);
#endif

static int
hnd_panic_callback(struct notifier_block *this, unsigned long code, void *ptr)
{
#if defined(BCM_ROUTER)
	bcm_radio_panic_hook();
#endif /* BCM_ROUTER */
	return NOTIFY_DONE;
}

static struct notifier_block hnd_panic_notifier = {
	.notifier_call = hnd_panic_callback,
};

static int32 __init
hnd_module_init(void)
{
	int32 ret = SUCCESS;

	HND_DEBUG("Loading HND Module\n");
#if defined(HND_LINUX)
	ret = hnd_linux_init();
#endif /* HND_LINUX */
	atomic_notifier_chain_register(&panic_notifier_list, &hnd_panic_notifier);

	return ret;
}

static void __exit
hnd_module_exit(void)
{

	atomic_notifier_chain_unregister(&panic_notifier_list, &hnd_panic_notifier);
#if defined(HND_LINUX)
	hnd_linux_exit();
#endif /* HND_LINUX */

	HND_DEBUG("Exiting HND Module\n");
	return;
}

module_init(hnd_module_init);
module_exit(hnd_module_exit);
