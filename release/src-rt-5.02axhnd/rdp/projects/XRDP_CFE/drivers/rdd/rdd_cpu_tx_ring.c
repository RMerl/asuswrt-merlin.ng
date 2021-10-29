/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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
 * cpu_tx_ring.c
 */

#include "rdd.h"
#include "rdd_defs.h"
#include "rdd_cpu_tx_ring.h"
#include "bdmf_errno.h"

#include "bdmf_system.h"
#include "bdmf_session.h"

#ifdef CPU_TX_RING

#ifdef _CFE_

#include "cfe_iocb.h"
#include "lib_malloc.h"

extern void cfe_usleep(int);
#define XRDP_USLEEP(_a)         cfe_usleep(_a)
#define XRDP_ERR_MSG(args...)   printf(args)
extern void _cfe_flushcache ( int, uint8_t *, uint8_t * );
#define FLUSH_RANGE(s,l)        \
    do { \
        __asm__ __volatile__ ("dsb    sy");\
        _cfe_flushcache(CFE_CACHE_FLUSH_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l));\
    } while (0)
#else
#define XRDP_USLEEP(_a)         bdmf_usleep(_a)
#define XRDP_ERR_MSG(args...)   BDMF_TRACE_ERR(args)
#undef FLUSH_RANGE
#define FLUSH_RANGE(s,l)        \
    do { \
        __asm__ __volatile__ ("dsb    sy");\
        bdmf_dcache_flush((unsigned long)s, l); \
    } while (0)
#endif

#ifdef CPU_TX_RING_COPY

static RDD_CPU_TX_RING_TABLE_DTS *tx_pd_table;
static int tx_pd_idx;

#define RDD_CPU_TX_MAX_ITERS    2
#define RDD_CPU_TX_ITER_DELAY   1000
#define QM_QUEUE_INDEX_DS_FIRST QM_QUEUE_DS_START

#define RDD_CPU_TX_MAX_BUF_SIZE 2048

static uint8_t *cpu_tx_ring_buf_array[RDD_CPU_TX_RING_TABLE_SIZE];

static int cpu_tx_ring_init(void)
{
#ifndef _CFE_
    int alloc_cnt;
    alloc_cnt = bdmf_sysb_databuf_alloc((void **)cpu_tx_ring_buf_array, RDD_CPU_TX_RING_TABLE_SIZE, 0);
    if (alloc_cnt != RDD_CPU_TX_RING_TABLE_SIZE)
    {
        bdmf_trace("%s: can't allocate sysb. allocated %d\n", __FUNCTION__, alloc_cnt);
        return BDMF_ERR_NOMEM;
    }
#else
    int i;
    for (i = 0; i < RDD_CPU_TX_RING_TABLE_SIZE; i++)
    {
        cpu_tx_ring_buf_array[i] = KMALLOC(RDD_CPU_TX_MAX_BUF_SIZE, 64);
        if (!cpu_tx_ring_buf_array[i])
        {
            bdmf_trace("%s: can't allocate TX buffer. allocated %d\n", __FUNCTION__, i);
            return BDMF_ERR_NOMEM;
        }
    }
#endif
    bdmf_trace("%s: TX ring initialized. PD ring at %p\n", __FUNCTION__, RDD_CPU_TX_RING_TABLE_PTR(get_runner_idx(cpu_tx_runner_image)));
    return BDMF_ERR_OK;
}

int cpu_tx_ring_copy_and_transmit(void *buffer, uint32_t length, uint8_t tx_port)
{
    volatile RDD_CPU_TX_DESCRIPTOR_DTS *tx_pd;
    uint8_t *tx_buf;
    uintptr_t data_phys_addr;
    int valid;
    int iter;
    int rc;

    if (length >= RDD_CPU_TX_MAX_BUF_SIZE)
    {
        XRDP_ERR_MSG("ERR: can't transmit buffer with length %u longer than %d\n", length, RDD_CPU_TX_MAX_BUF_SIZE);
        return BDMF_ERR_PARM;
    }

    if (!tx_pd_table)  /* First call ? */
    {
        tx_pd_table = RDD_CPU_TX_RING_TABLE_PTR(get_runner_idx(cpu_tx_runner_image));
        rc = cpu_tx_ring_init();
        if (rc)
            return rc;
    }
    tx_buf = cpu_tx_ring_buf_array[tx_pd_idx];
    tx_pd = (volatile RDD_CPU_TX_DESCRIPTOR_DTS *)&tx_pd_table->entry[tx_pd_idx];
    /* Wait until TX_PD becomes available */
    RDD_CPU_TX_DESCRIPTOR_VALID_READ(valid, tx_pd);
    for (iter = 0; valid && iter < RDD_CPU_TX_MAX_ITERS; iter++)
    {
        XRDP_USLEEP(RDD_CPU_TX_ITER_DELAY);
        RDD_CPU_TX_DESCRIPTOR_VALID_READ(valid, tx_pd);
    }
    if (iter == RDD_CPU_TX_MAX_ITERS)
        return BDMF_ERR_INTERNAL;

    /* Copy buffer and transmit */
    memcpy(tx_buf, buffer, length);
    FLUSH_RANGE(tx_buf, length);
    data_phys_addr = RDD_RSV_VIRT_TO_PHYS(tx_buf);

    memset((void *)tx_pd, 0, sizeof(*tx_pd));
    RDD_CPU_TX_DESCRIPTOR_ABS_WRITE(1, tx_pd);
    RDD_CPU_TX_DESCRIPTOR_PACKET_LENGTH_WRITE(length, tx_pd);
    RDD_CPU_TX_DESCRIPTOR_FIRST_LEVEL_Q_WRITE(QM_QUEUE_INDEX_DS_FIRST + tx_port, tx_pd);
    RDD_CPU_TX_DESCRIPTOR_DO_NOT_RECYCLE_WRITE(1, tx_pd);
    RDD_CPU_TX_DESCRIPTOR_LAN_WRITE(1, tx_pd);
    RDD_CPU_TX_DESCRIPTOR_ABS_DATA0_WRITE((data_phys_addr) & 0x3FFFF, tx_pd);
    RDD_CPU_TX_DESCRIPTOR_ABS_DATA1_WRITE(((data_phys_addr) >> 18) & 0x3FFFFF, tx_pd);
    RDD_CPU_TX_DESCRIPTOR_FPM_FALLBACK_WRITE(0, tx_pd);
    RDD_CPU_TX_DESCRIPTOR_SBPM_COPY_WRITE(1, tx_pd);
    RDD_CPU_TX_DESCRIPTOR_VALID_WRITE(1, tx_pd);

    if (tx_pd_idx == RDD_CPU_TX_RING_TABLE_SIZE - 1)
        tx_pd_idx = 0;
    else
        ++tx_pd_idx;

    return BDMF_ERR_OK;
}

#endif /* #ifdef CPU_TX_RING_COPY */

#endif /* #ifdef CPU_TX_RING */
