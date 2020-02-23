#if defined(CONFIG_BCM_KF_RUNNER)
/*
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; 
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef BR_FP_HOOKS_H
#define BR_FP_HOOKS_H

#include <linux/device.h>
#include "br_private.h"

extern struct br_fp_data *fp_hooks; 

#undef BR_FP_DEBUG_SET
#ifdef BR_FP_DEBUG_SET
#define BR_FP_DEBUG_LEVEL 4
#define BR_FP_START_DEBUG(n) do { if (n<BR_FP_DEBUG_LEVEL)
#define BR_FP_END_DEBUG      } while (0)
#define BR_FP_DEBUG(n, args...)			\
	BR_FP_START_DEBUG(n)			\
		printk(KERN_INFO args);		\
	BR_FP_END_DEBUG
#else
#define BR_FP_DEBUG(n, args...)
#endif

static inline int br_fp_hook(int cmd, void *in, void *out)
{
    if (!fp_hooks)
        return 0;
    return fp_hooks->rdpa_hook(cmd, in, out);
}

#endif /* BR_FP_HOOKS_H */
#endif
