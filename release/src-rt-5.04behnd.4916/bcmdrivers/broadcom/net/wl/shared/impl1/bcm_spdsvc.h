/*
    Copyright (c) 2018 Broadcom
    All Rights Reserved

    <:label-BRCM:2018:DUAL/GPL:standard

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

#ifndef _bcm_spdsvc_h_
#define _bcm_spdsvc_h_

#include <linux/bcm_log.h>
#include "spdsvc_defs.h"

/* FIXME: Using Ethernet PHY Overhead for now */
#define WL_SPDSVC_IFG        20 /* bytes */
#define WL_SPDSVC_CRC_LEN    4  /* bytes */
#define WL_SPDSVC_OVERHEAD   (WL_SPDSVC_CRC_LEN + WL_SPDSVC_IFG) /* bytes */

#if defined(BCMDONGLEHOST)
extern void dhd_spdsvc_init(void);
extern int dhd_spdsvc_rx(struct sk_buff *skb);
extern int dhd_spdsvc_tx(dhd_pub_t *dhdp, void **pktbuf);
#else /* NIC */
extern void wl_spdsvc_init(void);
extern int wl_spdsvc_rx(struct sk_buff *skb);
extern int wl_spdsvc_tx(struct sk_buff *skb, struct net_device *dev);
#endif /* BCMDONGLEHOST */

#endif /* _bcm_spdsvc_h_ */
