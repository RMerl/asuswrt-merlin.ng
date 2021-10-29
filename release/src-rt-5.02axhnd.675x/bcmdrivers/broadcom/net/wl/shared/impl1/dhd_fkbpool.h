/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard
    
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

#ifndef __DHD_FKBPOOL_H__
#define __DHD_FKBPOOL_H__

#if defined(BCM_NBUFF)

#include <linux/nbuff.h>
#ifdef WLCSM_DEBUG
#include <wlcsm_linux.h>
#endif

#define  DHD_FKBPOOL_MAX_SIZE (1600)
#define  DHD_FKBPOOL_DEFAULT_SIZE (1280)
#define  DHD_FKB_DATA_MAXLEN (2048)
#define DHD_FKB_MAXLEN (BCM_DCACHE_ALIGN(BCM_FKB_INPLACE   + \
                             BCM_PKT_HEADROOM        + \
                             DHD_FKB_DATA_MAXLEN + \
                             BCM_SKB_TAILROOM)       + \
                             BCM_SKB_SHAREDINFO)

#if BCM_MAX_PKT_LEN > DHD_FKB_DATA_MAXLEN
#define DHD_FKB_TXBUFSZ  DHD_FKB_MAXLEN 
#else
#define DHD_FKB_TXBUFSZ  BCM_PKTBUF_SIZE
#endif

typedef struct dhd_fkb_pool_entry {
    char fkb[DHD_FKB_TXBUFSZ];
} DHD_FKB_POOL_ENTRY;

#define  DHD_FKBPOOL_ENTRY_SIZE	(sizeof(DHD_FKB_POOL_ENTRY))

#ifdef WLCSM_DEBUG
FkBuff_t * dhd_fkb_pool_clone2unicast(osl_t *osh,FkBuff_t *fkb_p,void *mac,const char *file,int line);
#define DHD_FKB_CLONE2UNICAST(osh,fkb_p,mac) dhd_fkb_pool_clone2unicast(osh,fkb_p,mac,__FUNCTION__,__LINE__)
#else
FkBuff_t * dhd_fkb_pool_clone2unicast(osl_t *osh,FkBuff_t *fkb_p,void *mac);
#define DHD_FKB_CLONE2UNICAST(osh,fkb_p,mac) dhd_fkb_pool_clone2unicast(osh,fkb_p,mac)
#endif
void dhd_fkb_pool_put(void *fkb, unsigned long context,uint32_t flags);
void dhd_fkb_pool_free( void );
int dhd_fkb_pool_init( void );
extern uint g_dhd_fkbpool_size;
extern uint g_dhd_fkb_pool_free;
extern FkBuff_t *g_dhd_fkb_pool_free_p;
extern FkBuff_t *g_dhd_fkb_pool_p;
extern spinlock_t  g_dhd_fkb_pool_lock;
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define DHD_FKB_POOL_LOCK() spin_lock_irqsave(&g_dhd_fkb_pool_lock,lock_flags)
#define DHD_FKB_POOL_UNLOCK() spin_unlock_irqrestore(&g_dhd_fkb_pool_lock,lock_flags)
#else
#define DHD_FKB_POOL_LOCK()  local_irq_disable()
#define DHD_FKB_POOL_UNLOC() local_irq_enable()
#endif




#endif /* BCM_NBUFF */
#endif /* __DHD_FKBPOOL_H__ */
