/*************************************************************************
 *
 * ivi_rule6.h :
 *
 * This file is the header file for the 'ivi_rule6.c' file.
 *
 * Copyright (C) 2013 CERNET Network Center
 * All rights reserved.
 * 
 * Design and coding: 
 *   Xing Li <xing@cernet.edu.cn> 
 *	 Congxiao Bao <congxiao@cernet.edu.cn>
 *   Wentao Shang <wentaoshang@gmail.com>
 * 	 Yuncheng Zhu <haoyu@cernet.edu.cn> 
 * 	 Guoliang Han <bupthgl@gmail.com>
 * 
 * Contributions:
 *
 * This file is part of MAP-T/MAP-E Kernel Module.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * You should have received a copy of the GNU General Public License 
 * along with MAP-T/MAP-E Kernel Module. If not, see 
 * <http://www.gnu.org/licenses/>.
 *
 * For more versions, please send an email to <bupthgl@gmail.com> to
 * obtain an password to access the svn server.
 *
 * LIC: GPLv2
 *
 ************************************************************************/


#ifndef IVI_RULE6_H
#define IVI_RULE6_H

#include <linux/module.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <net/ip.h>
#include <net/ipv6.h>

#include "ivi_config.h"
#include "ivi_rule.h"

extern u8 u_byte;

static inline int ubyte_adjust(int pos) {
	if (!u_byte)
		return pos;

	if (pos < 8)
		return pos;
	else
		return pos + 1;
}

static inline int ubyte_adjust_bit(int pos) {
	if (!u_byte)
		return pos;

	if (pos < 64)
		return pos;
	else
		return pos + 8;
}

extern int ivi_rule6_insert(struct rule_info *rule);
extern int ivi_rule6_lookup(struct in6_addr *addr, int *plen, u32 *prefix4, int *plen4, u16 *ratio, u16 *adjacent, u8 *fmt);
extern int ivi_rule6_delete(struct rule_info *rule);
extern void ivi_rule6_flush(void);

extern int ivi_rule6_init(void);
extern void ivi_rule6_exit(void);

#endif
