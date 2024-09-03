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
