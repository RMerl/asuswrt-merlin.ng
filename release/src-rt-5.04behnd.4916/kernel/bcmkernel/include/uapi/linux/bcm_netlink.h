#ifndef __BCM_NETLINK_H_INCLUDED__
#define __BCM_NETLINK_H_INCLUDED__
/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
#define NETLINK_WLCT            24
#define NETLINK_BRCM_MONITOR	25 /*send events to userspace monitor task(broadcom specific)*/
#define NETLINK_BRCM_EPON       26
#define NETLINK_NDI		27	/* network identifier driver */
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
