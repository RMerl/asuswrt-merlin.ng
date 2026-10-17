// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * Graeme Russ, <graeme.russ@gmail.com>
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>
#include <linux/errno.h>
#include <asm/mtrr.h>

DECLARE_GLOBAL_DATA_PTR;

int init_cache_f_r(void)
{
	bool do_mtrr = CONFIG_IS_ENABLED(X86_32BIT_INIT) ||
		 IS_ENABLED(CONFIG_FSP_VERSION2) ||
		 (IS_ENABLED(CONFIG_TPL) && IS_ENABLED(CONFIG_HAVE_MRC));
	int ret;

	/*
	 * Supported configurations:
	 *
	 * booting from slimbootloader - MTRRs are already set up
	 * booting with FSPv1 - MTRRs are already set up
	 * booting with FSPv2 or MRC - MTRRs must be set here
	 * booting from coreboot - in this case there is no SPL, so we set up
	 *	the MTRRs here
	 */
	do_mtrr &= !IS_ENABLED(CONFIG_FSP_VERSION1) &&
		!IS_ENABLED(CONFIG_SYS_SLIMBOOTLOADER);

	if (do_mtrr) {
		ret = mtrr_commit(false);
		/*
		 * If MTRR MSR is not implemented by the processor, just ignore
		 * it
		 */
		if (ret && ret != -ENOSYS)
			return ret;
	}

	/* Initialise the CPU cache(s) */
	return init_cache();
}
