/*
   <:copyright-BRCM:2022:DUAL/GPL:standard

      Copyright (c) 2022 Broadcom 
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
