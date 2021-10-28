/*
    Copyright (c) 2018 Broadcom
    All Rights Reserved

    <:label-BRCM:2018:DUAL/GPL:standard
    
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

#ifndef _bcm_spdsvc_h_
#define _bcm_spdsvc_h_

#include <linux/bcm_log.h>
#include "spdsvc_defs.h"

/* FIXME: Using Ethernet PHY Overhead for now */
#define WL_SPDSVC_IFG        20 /* bytes */
#define WL_SPDSVC_CRC_LEN    4  /* bytes */
#define WL_SPDSVC_OVERHEAD   (WL_SPDSVC_CRC_LEN + WL_SPDSVC_IFG) /* bytes */

#if defined(BCM_ROUTER_DHD)
extern void dhd_spdsvc_init(void);
extern int dhd_spdsvc_rx(struct sk_buff *skb);
extern int dhd_spdsvc_tx(dhd_pub_t *dhdp, void **pktbuf);
#else /* NIC */
extern void wl_spdsvc_init(void);
extern int wl_spdsvc_rx(struct sk_buff *skb);
extern int wl_spdsvc_tx(struct sk_buff *skb, struct net_device *dev);
#endif /* BCM_ROUTER_DHD */

#endif /* _bcm_spdsvc_h_ */
