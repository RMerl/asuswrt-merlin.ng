/*
   <:copyright-BRCM:2021:DUAL/GPL:standard
   
      Copyright (c) 2021 Broadcom 
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
* File Name  : archer_gpl.h
*
* Description: Archer GPL Driver
*
*******************************************************************************
*/

#ifndef __ARCHER_GPL_H_INCLUDED__
#define __ARCHER_GPL_H_INCLUDED__

#include <linux/irqreturn.h>
#include "archer_cpu_queues.h"

#if defined(CONFIG_BCM947622)
#define ARCHER_GPL_SYSPORT_IRQ_MAX  6
#include "47622_sysport.h"
#endif
#if defined(CONFIG_BCM963178)
#define ARCHER_GPL_SYSPORT_IRQ_MAX  8
#include "63178_sysport.h"
#include "bcm_iudma.h"
#endif
#if defined(CONFIG_BCM96756)
#define ARCHER_GPL_SYSPORT_IRQ_MAX  6
#include "6756_sysport.h"
#endif
#if defined(CONFIG_BCM96765)
#define ARCHER_GPL_SYSPORT_IRQ_MAX  10
#include "6765_crossbow.h"
#endif
#if defined(CONFIG_BCM96766)
#define ARCHER_GPL_SYSPORT_IRQ_MAX  12
#include "6766_crossbow.h"
#endif
#if defined(CONFIG_BCM96764)
#define ARCHER_GPL_SYSPORT_IRQ_MAX  6
#include "6764_crossbow.h"
#endif

#define ARCHER_GPL_SAR_IUDMA_IRQ_MAX    20

#define ARCHER_GPL_INTR_MAX             12

#define ARCHER_GPL_CPU_ID_INVALID      -1

#define ARCHER_GPL_INTF_INDEX_INVALID  -1

typedef enum {
    ARCHER_GPL_IRQ_TYPE_RXQ,
    ARCHER_GPL_IRQ_TYPE_TXQ,
    ARCHER_GPL_IRQ_TYPE_WOL,
    ARCHER_GPL_IRQ_TYPE_USR,
    ARCHER_GPL_IRQ_TYPE_DSL,
    ARCHER_GPL_IRQ_TYPE_SOCKET,
    ARCHER_GPL_IRQ_TYPE_MAILBOX,
    ARCHER_GPL_IRQ_TYPE_TIMER,
    ARCHER_GPL_IRQ_TYPE_MAX
} archer_gpl_irq_type_t;

typedef struct {
    int intf_index;
    int intc;
    irq_handler_t isr;
    void *param;
    int cpu_id;
    archer_gpl_irq_type_t type;
    int type_index;
} archer_gpl_intr_t;

typedef struct {
    bcmSysport_Config_t *sysport_p;
    archer_gpl_intr_t intr[ARCHER_GPL_INTR_MAX];
} archer_gpl_enet_config_t;

typedef struct {
    volatile uint32_t *cnp_p;
    volatile uint32_t *natc_p;
    volatile uint32_t *cm7_p;
} archer_gpl_crossbow_t;

#if defined(CONFIG_ARM64)
#define CC_ARCHER_PHYS_ADDR_40BIT
#endif

typedef uint64_t archer_phys_addr_t;

volatile void *archer_coherent_mem_alloc(int size, archer_phys_addr_t *phys_addr_p);

void archer_coherent_mem_free(int size, archer_phys_addr_t phys_addr, volatile void *p);

#define archer_virt_to_phys virt_to_phys

#if defined(CONFIG_BCM_ARCHER_GSO)
void archer_gso(pNBuff_t pNBuff, int nbuff_max, pNBuff_t *pNBuff_list, int *nbuff_count_p);
#else
static inline void archer_gso(pNBuff_t pNBuff, int nbuff_max, pNBuff_t *pNBuff_list, int *nbuff_count_p)
{
    pNBuff_list[0] = pNBuff;

    *nbuff_count_p = 1;
}
#endif

int archer_gpl_enet_bind(archer_host_hooks_t *hooks_p);

int archer_gpl_enet_config(bcmSysport_Config_t *config_p);

volatile SYSTEMPORT_INTRL2 *
archer_gpl_intc_to_intrl2(volatile sysport *sysport_p, int intc);

volatile SYSTEMPORT_INTC *
archer_gpl_intc_to_intrl2_intc(volatile sysport *sysport_p, int intc);

int archer_gpl_sysport_get(int intf_index, volatile sysport **sysport_p,
                           archer_phys_addr_t *phys_addr_p);

void *archer_gpl_sar_iudma_get(void);

void archer_gpl_crossbow_get(archer_gpl_crossbow_t *crossbow_p);

#endif  /* __ARCHER_GPL_H_INCLUDED__ */
