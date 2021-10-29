#ifdef CONFIG_BCM_KF_MHI
/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom 
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
/*
 * File: mhdp.h
 *
 * Modem-Host Interface (MHI) - MHDP kernel interface
 */


#ifndef __NET_MHI_MHDP_H
#define __NET_MHI_MHDP_H

struct mhdp_tunnel_parm {
	char name[IFNAMSIZ];
	char master[IFNAMSIZ];
	int  pdn_id;
	int  sim_id;
};

struct mhdp_udp_filter {

	unsigned short port_id;
	unsigned char active;
};

#define SIOCADDPDNID	(SIOCDEVPRIVATE + 1)
#define SIOCDELPDNID	(SIOCDEVPRIVATE + 2)
#define SIOCRESETMHDP	(SIOCDEVPRIVATE + 3)
#define SIOSETUDPFILTER	(SIOCDEVPRIVATE + 4)

struct net_device *mhdp_get_netdev_by_pdn_id(struct net_device *dev, int pdn_id);

#endif /* __NET_MHI_MHDP_H */
#endif /* CONFIG_BCM_KF_MHI */
