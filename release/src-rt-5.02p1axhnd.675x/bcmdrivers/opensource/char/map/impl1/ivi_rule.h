/*************************************************************************
 *
 * ivi_rule.h :
 *
 * This file is the header file for the 'ivi_rule.c' file.
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


#ifndef IVI_RULE_H
#define IVI_RULE_H

#include <linux/module.h>
#include <linux/list.h>
//#include "list.h"
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <linux/inetdevice.h>

#include "ivi_config.h"

extern int ivi_rule_lookup(u32 key, struct in6_addr *prefix6, int *plen4, int *plen6, u16 *ratio, u16 *adjacent, u8 *fmt, u8 *transpt, u8 *ext6);
extern int ivi_rule_insert(struct rule_info *rule);
extern int ivi_rule_delete(struct rule_info *rule);
extern void ivi_rule_flush(void);

extern int ivi_rule_init(void);
extern void ivi_rule_exit(void);

#endif /* IVI_RULE_H */
