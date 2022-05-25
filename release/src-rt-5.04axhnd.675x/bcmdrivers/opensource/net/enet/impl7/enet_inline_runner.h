/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom
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
