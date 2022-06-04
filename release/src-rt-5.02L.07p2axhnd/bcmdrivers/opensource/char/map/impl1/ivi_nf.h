/*************************************************************************
 *
 * ivi_nf.h :
 *
 * This file is the header file for the 'ivi_nf.c' file.
 *
 * Copyright (C) 2013 CERNET Network Center
 * All rights reserved.
 * 
 * Design and coding: 
 *   Xing Li <xing@cernet.edu.cn> 
 *	 Congxiao Bao <congxiao@cernet.edu.cn>
 * 	 Yuncheng Zhu <haoyu@cernet.edu.cn>
 * 	 Wentao Shang <wentaoshang@gmail.com>
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


#ifndef IVI_NF_H
#define IVI_NF_H

#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netdevice.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/route.h>

#include "ivi_config.h"
#include "ivi_map.h"
#include "ivi_xmit.h"

extern int nf_getv4dev(struct net_device *dev);
extern int nf_getv6dev(struct net_device *dev);
extern int nf_running(const int run);

extern int ivi_nf_init(void);
extern void ivi_nf_exit(void);

extern struct net_device *v4_dev, *v6_dev;

#endif /* IVI_NF_H */
