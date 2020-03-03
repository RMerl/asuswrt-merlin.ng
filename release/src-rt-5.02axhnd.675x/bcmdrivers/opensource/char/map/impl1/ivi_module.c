/*************************************************************************
 *
 * ivi_module.c :
 *
 * MAP-T/MAP-E kernel module initiation and parameters set.
 *
 * Copyright (C) 2013 CERNET Network Center
 * All rights reserved.
 * 
 * Design and coding: 
 *   Xing Li      <xing@cernet.edu.cn> 
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

#include <linux/module.h>

#include "ivi_rule.h"
#include "ivi_rule6.h"
#include "ivi_map.h"
#if 0
#include "ivi_map_tcp.h"
#endif
#include "ivi_portmap.h"
#include "ivi_nf.h"
#include "ivi_ioctl.h"

static int __init ivi_module_init(void) {
	int retval = 0;
	if ((retval = ivi_rule_init()) < 0) {
		return retval;
	}
	if ((retval = ivi_rule6_init()) < 0) {
		return retval;
	}
	if ((retval = ivi_map_init()) < 0) {
		return retval;
	}
#if 0
	if ((retval = ivi_map_tcp_init()) < 0) {
		return retval;
	}
#endif
	if ((retval = init_mapportmap_list()) < 0) {
		return retval;
	}
	if ((retval = ivi_nf_init()) < 0) {
		return retval;
	}
	if ((retval = ivi_ioctl_init()) < 0) {
		return retval;
	}
	return 0;
}
module_init(ivi_module_init);

static void __exit ivi_module_exit(void) {
	ivi_ioctl_exit();
	ivi_nf_exit();
#if 0
	ivi_map_tcp_exit();
#endif
	ivi_map_exit();
	ivi_rule6_exit();
	ivi_rule_exit();
}
module_exit(ivi_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHU Yuncheng <haoyu@cernet.edu.cn>");
MODULE_AUTHOR("Wentao Shang <wentaoshang@gmail.com>");
MODULE_AUTHOR("Guoliang Han <bupthgl@gmail.com>");
MODULE_DESCRIPTION("MAP-T/MAP-E Kernel Module");
