/*
* <:copyright-BRCM:2022:DUAL/GPL:standard
* 
*    Copyright (c) 2022 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
:>
*/

#include "sw_gso_hook.h"
#include <linux/spinlock_types.h>

extern int sw_gso_log_init(void*);

int sw_gso_hook_init(void)
{
    sw_gso_log_init((void *)bcmLog_logIsEnabled(BCM_LOG_ID_LOG, BCM_LOG_LEVEL_ERROR));

    return 0;
}

void* pkt_get_queue(void* buf)
{
    return nbuff_get_queue(buf);
}

void pkt_set_queue(void* buf,void *queue)
{
    nbuff_set_queue(buf,queue);
}

int sw_gso_get_wait_queue_head_t_sz(void)
{
    return (int)sizeof(wait_queue_head_t);
}

void* sw_gso_alloc_spinlock(void)
{
    spinlock_t *lock=NULL;

    lock = kmalloc(sizeof(spinlock_t), GFP_ATOMIC);

    if(lock != NULL)
    {
       spin_lock_init(lock);
    }

    return (void*)lock;
}

void sw_gso_free_spinlock(void* lock)
{
    if(lock != NULL)
    {
       kfree((void*)lock);
    }

    return;
}

void sw_gso_spin_lock_bh(void* lock)
{
    spin_lock_bh((spinlock_t*)lock);
}

void sw_gso_spin_unlock_bh(void* lock)
{
    spin_unlock_bh((spinlock_t*)lock);
}