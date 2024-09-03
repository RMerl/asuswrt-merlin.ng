/*
  <:copyright-BRCM:2023:DUAL/GPL:standard
  
     Copyright (c) 2023 Broadcom 
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
*******************************************************************************
*
* File Name  : crossbow_gpl.c
*
* Description: Crossbow GPL Driver
*
*******************************************************************************
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/bcm_log.h>
#include <linux/nbuff.h>
#include "bcm_rsvmem.h"
#include "bcm_timer.h"

#include "crossbow_gpl.h"

#define isLogDebug bcmLog_logIsEnabled(BCM_LOG_ID_ARCHER, BCM_LOG_LEVEL_DEBUG)
#define __logDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_ARCHER, fmt, ##arg)
#define __logInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_ARCHER, fmt, ##arg)
#define __logNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_ARCHER, fmt, ##arg)
#define __logError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_ARCHER, fmt, ##arg)

crossbow_enet_hooks_t crossbow_gpl_hooks_g;
EXPORT_SYMBOL(crossbow_gpl_hooks_g);

#if defined(CC_CROSSBOW_TIMER)
crossbow_timer_hooks_t crossbow_gpl_timer_hooks_g;
EXPORT_SYMBOL(crossbow_gpl_timer_hooks_g);
#endif

int archer_gpl_cpu_affinity_set(struct task_struct *task_p);

int crossbow_gpl_enet_bind(crossbow_enet_hooks_t *hooks_p)
{
    bcmFun_t *archer_host_bind = bcmFun_get(BCM_FUN_ID_ARCHER_HOST_BIND);
    int ret;

    if(!archer_host_bind)
    {
        __logError("Archer binding is not available");

        return -1;
    }

    crossbow_gpl_hooks_g.rxq_isr = hooks_p->rxq_isr;
    crossbow_gpl_hooks_g.rxq_isr_cpu_id = hooks_p->rxq_isr_cpu_id;
    crossbow_gpl_hooks_g.rcq_isr = hooks_p->rcq_isr;
    crossbow_gpl_hooks_g.rcq_isr_cpu_id = hooks_p->rcq_isr_cpu_id;
    crossbow_gpl_hooks_g.stats_dump = hooks_p->stats_dump;

    ret = archer_host_bind(&crossbow_gpl_hooks_g);
    if(ret)
    {
        __logError("Could not archer_host_bind");

        return ret;
    }

    if(crossbow_gpl_hooks_g.archer_task_p)
    {
        archer_gpl_cpu_affinity_set(crossbow_gpl_hooks_g.archer_task_p);
    }

    return 0;
}
EXPORT_SYMBOL(crossbow_gpl_enet_bind);

#if defined(CC_CROSSBOW_TIMER)

int crossbow_gpl_timer_bind(crossbow_timer_hooks_t *hooks_p)
{
    crossbow_gpl_timer_hooks_g = *hooks_p;

    return bcm_timer_hw_bind(hooks_p);
}
EXPORT_SYMBOL(crossbow_gpl_timer_bind);

#endif /* CC_CROSSBOW_TIMER */

int crossbow_gpl_flow_memory_get(crossbow_gpl_flow_memory_t *flow_mem_p)
{
    int ret;

    ret = BcmMemReserveGetByName(RNRMEM_BASE_ADDR_STR, &flow_mem_p->virt_p,
                                 &flow_mem_p->phys_addr, &flow_mem_p->size);
    if(ret)
    {
        __logError("Could not BcmMemReserveGetByName (%s)", RNRMEM_BASE_ADDR_STR);

        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(crossbow_gpl_flow_memory_get);

int crossbow_gpl_construct(void)
{
    memset(&crossbow_gpl_hooks_g, 0, sizeof(crossbow_enet_hooks_t));

    return 0;
}
