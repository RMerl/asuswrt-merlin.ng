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

#ifndef _ENET_INLINE_RUNNER_H_
#define _ENET_INLINE_RUNNER_H_

#ifdef RUNNER

#include <rdpa_api.h>

#ifdef CONFIG_NET_SWITCHDEV

inline int enetxapi_offload_should_mark(struct net_device *dev, FkBuff_t *fkb, uint32_t reason)
{
#if defined(CONFIG_ONU_TYPE_SFU) && defined(CONFIG_BCM_RUNNER_FLOODING)
    /* The RUNNER firmware in PRV mode sends one copy of a flooded packet to the CPU with special reason
     * unknown_da_flood. Packets with this reason should  be marked as offloaded t oLinux in order to avoid double
     * flooding
     */
    return reason == rdpa_cpu_rx_reason_unknown_da_flood;
#endif
    return 0;
}

#endif /* CONFIG_NET_SWITCHDEV */

#endif /* RUNNER */

#endif /* _ENET_INLINE_RUNNER_H_ */
