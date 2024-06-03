/***********************************************************************
 *
 * Copyright (c) 2015  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2015:DUAL/GPL:standard
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
 * :>
 *
 ************************************************************************/

#ifndef _SPDSVC_API_H_
#define _SPDSVC_API_H_

#include "spdsvc_defs.h"

int spdsvc_enable(spdsvc_config_t *config_p);
int spdsvc_getOverhead(uint32_t *overhead_p);
int spdsvc_getResult(spdsvc_result_t *result_p);
int spdsvc_disable(void);

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

    printf("Level %u/%u, Writes %u, Write Bytes %llu, Reads %u, Discards %u, Discard Bytes %llu\n",
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
