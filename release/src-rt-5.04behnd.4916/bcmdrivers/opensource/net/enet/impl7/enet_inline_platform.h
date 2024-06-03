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
/*
 * Platform specific implementations of inline API calls
 * from the data path should be placed here.
 */
#ifndef _ENET_INLINE_PLATFORM_H_
#define _ENET_INLINE_PLATFORM_H_

/* INLINE API */

#ifdef CONFIG_NET_SWITCHDEV

#include <linux/nbuff.h>
#include <linux/netdevice.h>
/* Check if SKB should be marked as offloaded */
inline int enetxapi_offload_should_mark(struct net_device *dev, FkBuff_t *fkb, uint32_t reason) __attribute__((always_inline));

#endif /* CONFIG_NET_SWITCHDEV */

#if defined(RUNNER)
#include "enet_inline_runner.h"
#elif defined(SF2_DEVICE)
#include "enet_inline_sf2.h"
#else
/* Unimplemented APIs */

#define enetxapi_offload_should_mark(...) 0

#endif

#endif /* _ENET_INLINE_PLATFORM_H_ */
