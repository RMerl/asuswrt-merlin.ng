/*
<:copyright-BRCM:2002:GPL/GPL:standard

   Copyright (c) 2002 Broadcom 
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
/***************************************************************************
* File Name  : bmu.c
*
* Description: This file contains kernel level support for the 
*              battery management unit
*
***************************************************************************/

/* Includes. */
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <bcmtypes.h>
#include <bcm_intr.h>
#include "bcm_map.h"
#include "boardparms.h"
#include "bmu.h"

/* Typedefs. */
typedef struct _CB_BMU__LIST
{
    struct list_head list;
    char name[DEVNAMSIZ];
    cb_bmu_t cb_bmu_fn;
    void *context;
} CB_BMU_LIST, *PCB_BMU_LIST;

static DEFINE_MUTEX(bmuMutex);
static CB_BMU_LIST *g_cb_bmu_list_head = NULL;

struct tasklet_struct bcmBmuTasklet;
unsigned long bmu_tasklet_active_cnt = 0;
static spinlock_t bmu_irq_lock;

/***************************************************************************
* Battery Management Unit ISR and functions.
* Triggers when switching from Utility power to Battery mode.
* Note: BMUD application reports when switching from Battery back to Utility.
***************************************************************************/

static void bcmBmuTaskletFunc(unsigned long data)
{
    struct list_head *pos;
    CB_BMU_LIST *tmp = NULL;
    unsigned long flags;

    /* Invoke each registered function */
    list_for_each(pos, &g_cb_bmu_list_head->list) {
        tmp = list_entry(pos, CB_BMU_LIST, list);
        (tmp->cb_bmu_fn)(tmp->context);
    }

    spin_lock_irqsave(&bmu_irq_lock, flags);
    bmu_tasklet_active_cnt = 0;
    spin_unlock_irqrestore(&bmu_irq_lock, flags);
}

static irqreturn_t bcmBmuIsr(int irq, void * dev_id)
{
    unsigned long flags;

    if (APM->apm_dev_irq_pend & DEV_BMU_IRQ) {
        /* Clear the BMU specific interrupt */
        APM->apm_dev_irq_pend |= DEV_BMU_IRQ;

        spin_lock_irqsave(&bmu_irq_lock, flags);
        bmu_tasklet_active_cnt++;
        spin_unlock_irqrestore(&bmu_irq_lock, flags);

        if (bmu_tasklet_active_cnt == 1)
            tasklet_hi_schedule(&bcmBmuTasklet);
        return( IRQ_HANDLED );
    }
    return IRQ_NONE;
}

void __init bcmInitBmuHandler( void )
{
    CB_BMU_LIST *new_node;
    unsigned short bmuen;

    if (BpGetBatteryEnable(&bmuen) != BP_SUCCESS) {
        return;
    }
    if (!bmuen) {
        return;
    }

    if( g_cb_bmu_list_head != NULL) {
        printk("Error: bcmInitBmuHandler: list head is not null\n");
        return;
    }
    new_node = (CB_BMU_LIST *)kmalloc(sizeof(CB_BMU_LIST), GFP_KERNEL);
    memset(new_node, 0x00, sizeof(CB_BMU_LIST));
    INIT_LIST_HEAD(&new_node->list);
    g_cb_bmu_list_head = new_node;

    spin_lock_init(&bmu_irq_lock);
    tasklet_init(&bcmBmuTasklet, bcmBmuTaskletFunc, 0);

    BcmHalMapInterrupt((FN_HANDLER)bcmBmuIsr, (void*)0, INTERRUPT_ID_PCMC);
#if !defined(CONFIG_ARM)
    BcmHalInterruptEnable(INTERRUPT_ID_PCMC);
#endif

    /* Enable the BMU specific interrupt */
    APM->apm_dev_irq_mask |= DEV_BMU_IRQ;

} /* bcmInitBmuHandler */

void __exit bcmDeinitBmuHandler( void )
{
    struct list_head *pos;
    CB_BMU_LIST *tmp;

    if(g_cb_bmu_list_head == NULL)
        return;

    list_for_each(pos, &g_cb_bmu_list_head->list) {
        tmp = list_entry(pos, CB_BMU_LIST, list);
        list_del(pos);
        kfree(tmp);
    }

    kfree(g_cb_bmu_list_head);
    g_cb_bmu_list_head = NULL;
} /* bcmDeinitBmuHandler */

void bcmRegisterBmuHandler(char *devname, void *cbfn, void *context)
{
    CB_BMU_LIST *new_node;

    // do all the stuff that can be done without the lock first
    if( devname == NULL || cbfn == NULL ) {
        printk("Error: bcmRegisterBmuHandler: register info not enough (%s,%x,%x)\n", devname, (unsigned int)cbfn, (unsigned int)context);
        return;
    }

    if (strlen(devname) > (DEVNAMSIZ - 1)) {
        printk("Warning: bcmRegisterBmuHandler: devname too long, will be truncated\n");
    }

    new_node= (CB_BMU_LIST *)kmalloc(sizeof(CB_BMU_LIST), GFP_KERNEL);
    memset(new_node, 0x00, sizeof(CB_BMU_LIST));
    INIT_LIST_HEAD(&new_node->list);
    strncpy(new_node->name, devname, DEVNAMSIZ-1);
    new_node->cb_bmu_fn = (cb_bmu_t)cbfn;
    new_node->context = context;

    // OK, now acquire the lock and insert into list
    mutex_lock(&bmuMutex);
    if( g_cb_bmu_list_head == NULL) {
        printk("Error: bcmRegisterBmuHandler: list head is null\n");
        kfree(new_node);
    } else {
        list_add(&new_node->list, &g_cb_bmu_list_head->list);
        printk("bmu: bcmRegisterBmuHandler: %s registered \n", devname);
    }
    mutex_unlock(&bmuMutex);

    return;
} /* bcmRegisterBmuHandler */

void bcmDeregisterBmuHandler(char *devname)
{
    struct list_head *pos;
    CB_BMU_LIST *tmp;
    int found=0;

    if(devname == NULL) {
        printk("Error: bcmDeregisterBmuHandler: devname is null\n");
        return;
    }

    printk("bcmDeregisterBmuHandler: %s is deregistering\n", devname);

    mutex_lock(&bmuMutex);
    if(g_cb_bmu_list_head == NULL) {
        printk("Error: bcmDeregisterBmuHandler: list head is null\n");
    } else {
        list_for_each(pos, &g_cb_bmu_list_head->list) {
            tmp = list_entry(pos, CB_BMU_LIST, list);
            if(!strcmp(tmp->name, devname)) {
                list_del(pos);
                kfree(tmp);
                found = 1;
                printk("bcmDeregisterBmuHandler: %s is deregistered\n", devname);
                break;
            }
        }
        if (!found)
            printk("bcmDeregisterBmuHandler: %s not (de)registered\n", devname);
    }
    mutex_unlock(&bmuMutex);

    return;
} /* bcmDeregisterBmuHandler */

EXPORT_SYMBOL(bcmRegisterBmuHandler);
EXPORT_SYMBOL(bcmDeregisterBmuHandler);
