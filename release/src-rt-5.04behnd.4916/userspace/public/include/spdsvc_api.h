/***********************************************************************
 *
 * Copyright (c) 2015  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2015:DUAL/GPL:standard
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
 * :>
 *
 ************************************************************************/

#ifndef _SPDSVC_API_H_
#define _SPDSVC_API_H_

#include <inttypes.h>
#include "spdsvc_defs.h"

int spdsvc_enable(spdsvc_config_t *config_p);
int spdsvc_getOverhead(spdsvc_config_t *config_p, uint32_t *overhead_p);
int spdsvc_getResult(spdsvc_config_t *config_p, spdsvc_result_t *result_p);
int spdsvc_disable(spdsvc_config_t *config_p);

static inline spdsvc_tr471_rx_queue_entry_t *
spdsvc_tr471_rx_queue_read(spdsvc_tr471_rx_queue_t *spdsvc_tr471_rx_queue_p)
{
    bcm_async_queue_t *queue_p = &spdsvc_tr471_rx_queue_p->async_queue;

//    printf("\t%u entries\n", bcm_async_queue_avail_entries(queue_p));

    if(bcm_async_queue_not_empty(queue_p))
    {
        return (spdsvc_tr471_rx_queue_entry_t *)
            bcm_async_queue_entry_read(queue_p);
    }

    return NULL;
}

static inline void
spdsvc_tr471_rx_queue_post(spdsvc_tr471_rx_queue_t *spdsvc_tr471_rx_queue_p)
{
    bcm_async_queue_t *queue_p = &spdsvc_tr471_rx_queue_p->async_queue;

    bcm_async_queue_entry_dequeue(queue_p);

    queue_p->stats.reads++;
}

static inline void spdsvc_tr471_rx_queue_stats(spdsvc_tr471_rx_queue_t *spdsvc_tr471_rx_queue_p)
{
    bcm_async_queue_t *queue_p = &spdsvc_tr471_rx_queue_p->async_queue;

    printf("Level %u/%u, Writes %u, Write Bytes %"PRIu64", Reads %u, Discards %u, Discard Bytes %"PRIu64"\n",
           queue_p->depth - bcm_async_queue_free_entries(queue_p), queue_p->depth,
           queue_p->stats.writes, queue_p->stats.write_bytes,
           queue_p->stats.reads, queue_p->stats.discards,
           queue_p->stats.discard_bytes);
}

static inline void spdsvc_tr471_rx_queue_init(spdsvc_tr471_rx_queue_t *spdsvc_tr471_rx_queue_p)
{
    bcm_async_queue_t *queue_p = &spdsvc_tr471_rx_queue_p->async_queue;

    queue_p->mmap_alloc_p = (uint8_t *)spdsvc_tr471_rx_queue_p->entry;
}

#endif /* _SPDSVC_API_H_ */
