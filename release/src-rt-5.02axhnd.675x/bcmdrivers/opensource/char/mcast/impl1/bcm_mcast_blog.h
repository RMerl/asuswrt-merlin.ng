/*
*    Copyright (c) 2015 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2015:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#ifndef _BCM_MCAST_BLOG_H_
#define _BCM_MCAST_BLOG_H_

int bcm_mcast_blog_get_rep_info(struct net_device *repDev, unsigned char *repMac, uint32_t *info);
void bcm_mcast_blog_release(int proto, void *mc_fdb);
int bcm_mcast_blog_process(bcm_mcast_ifdata *pif, void *mc_fdb, int proto, struct hlist_head *headMcHash);

__init int bcm_mcast_blog_init(void);
void bcm_mcast_blog_deinit(void);

#endif /* _BCM_MCAST_BLOG_H_ */