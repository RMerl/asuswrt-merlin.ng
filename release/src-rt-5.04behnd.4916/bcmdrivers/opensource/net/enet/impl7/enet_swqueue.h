/*
   <:copyright-BRCM:2022:DUAL/GPL:standard
   
      Copyright (c) 2022 Broadcom 
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

#ifndef __ENET_SWQUEUE_H
#define __ENET_SWQUEUE_H

#include "bcm_pktfwd.h" /* BCM_PKTFWD && BCM_PKTLIST && BCM_PKTQUEUE */

#if defined(BCM_PKTFWD) && defined(BCM_PKTQUEUE) && !defined(CONFIG_BCM_PON)
#define ENET_SWQUEUE
#else
/* Stubs for when ENET_SWQUEUE is not compiled */
#define enet_swqueue_init(swq_size) 0
#define enet_swqueue_fini(void)
#define enet_sys_dump(void)
#define enet_kthread_init()
#endif 

#if defined(ENET_SWQUEUE)

/** ENET SW queue drop threshold */
#define ENET_SWQUEUE_MIN_SIZE           (64)
#define ENET_SWQUEUE_MAX_SIZE           (1024)

/** ENET_SWQUEUE construction/destruction/debug_dump : LOCKLESS */
int enet_swqueue_init(uint32_t swq_size);
void enet_swqueue_fini(void);
void enet_sys_dump(void);
int enet_kthread_init(void);

#endif /* ENET_SWQUEUE */
#endif
