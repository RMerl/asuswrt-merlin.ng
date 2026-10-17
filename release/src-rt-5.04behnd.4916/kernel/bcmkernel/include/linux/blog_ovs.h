/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 *
 *    Copyright (c) 2019 Broadcom
 *    All Rights Reserved
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
 * :>
 */

#ifndef BLOG_OVS_H
#define BLOG_OVS_H 1

#include <linux/blog.h>

struct sw_flow_ext {
	int output_count; /* # of output actions */
	u32 blog_key;     /* Associating megaflow to blogged flows  */
	s32 stats_adj;    /* Adjust byte stats of mega flow */
	bool is_gretap;    /* Is rx port gretap? */
	bool blog_match;  /* Is megaflow compatible with blog flow? */
	blog_fast_stats_t cumm_fast_stats; /*
					    * cummulative fast stats of all flow
					    * cache flows of a megaflow evicted
					    * from flow cache
					    */
	u64 prev_packet_count;  /*
				 * previous packet count of active flow cache
				 * flows of a megaflow
				 */
};

#endif /* BLOG_OVS_H */
