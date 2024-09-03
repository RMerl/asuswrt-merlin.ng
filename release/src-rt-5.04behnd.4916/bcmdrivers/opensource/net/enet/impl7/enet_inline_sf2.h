/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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
#ifndef _ENET_INLINE_SF2_H_
#define _ENET_INLINE_SF2_H_

#if defined(SF2_DEVICE)

#ifdef CONFIG_NET_SWITCHDEV

#include <linux/etherdevice.h>

inline int enetxapi_offload_should_mark(struct net_device *dev, FkBuff_t *fkb, uint32_t reason)
{
    /* Set offload flag if the switch has already handled this packet.
       Unicast packets are ARL forwarded in which case the host won't see it.
       Multicast packets are not handled by the switch.
       Broadcast packets are handled by the switch but the host would still
       see it from the IMP port. So set the flag for broadcast pkt so that 
       the linux bridge does not forward it */
    return is_netdev_hw_switch(dev) && is_broadcast_ether_addr(fkb->data);
}
#endif /* CONFIG_NET_SWITCHDEV */

#endif /* SF2_DEVICE */

#endif /* _ENET_INLINE_SF2_H_ */
