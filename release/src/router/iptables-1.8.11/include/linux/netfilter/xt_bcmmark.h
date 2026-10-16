/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _XT_BCMMARK_H
#define _XT_BCMMARK_H

#include <linux/types.h>

struct xt_bcmmark_tginfo2 {
	__u32 mark, mask;
};

struct xt_bcmmark_mtinfo1 {
	__u32 mark, mask;
	__u8 invert;
};

#endif /*_XT_BCMMARK_H*/
