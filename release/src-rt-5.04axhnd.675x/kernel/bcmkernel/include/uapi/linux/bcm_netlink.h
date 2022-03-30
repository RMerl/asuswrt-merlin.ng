#ifndef __BCM_NETLINK_H_INCLUDED__
#define __BCM_NETLINK_H_INCLUDED__
/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

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
#define NETLINK_WLCT            24
#define NETLINK_BRCM_MONITOR	25 /*send events to userspace monitor task(broadcom specific)*/
#define NETLINK_BRCM_EPON       26
#define NETLINK_DPI		27	/* dpicore driver */
#define NETLINK_DPI_QOS		28	/* DPI QoS */
#define NETLINK_IGSC		29	/* for WIFI multicast igs sdb listing */
#define NETLINK_BCM_MCAST	30	/* for multicast */
#define NETLINK_WLCSM		31	/* for brcm wireless cfg[nvram]/statics/management extention */

/* Note that MAX netlink message ids is 32. Defined by MAX_LINKS
 * macro in kernel/linux-<ver>/include/uapi/linux/netlink.h.
 * Max netlink message ids have been reached and any new
 * netlink message requirements must look at using NETLINK_GENERIC
 * with a sub-type
 */

#endif /* __BCM_NETLINK_H_INCLUDED__ */
