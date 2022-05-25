/*
    <:copyright-BRCM:2015:DUAL/GPL:standard

       Copyright (c) 2015 Broadcom
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

/*
 * cpu_tx_ring.c
 */

#include "rdd.h"
#include "rdd_defs.h"
#include "rdd_cpu_tx_ring.h"
#include "bdmf_errno.h"
#include "xrdp_drv_rnr_regs_ag.h"

#if defined(_CFE_)
extern void cfe_usleep(int);
#define XRDP_USLEEP(_a)         cfe_usleep(_a)
#define XRDP_ERR_MSG(args...)   printf(args)
extern void _cfe_flushcache ( int, uint8_t *, uint8_t * );
#define FLUSH_RANGE(s,l)        \
    do { \
        __asm__ __volatile__ ("dsb    sy");\
        _cfe_flushcache(CFE_CACHE_FLUSH_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l));\
    } while (0)
#elif defined(__UBOOT__)
#include "linux/delay.h"
#include "string.h"
#include "stdio.h"

#define XRDP_USLEEP(_a)         udelay(_a)
#define XRDP_ERR_MSG(args...)   printf(args)
#define FLUSH_RANGE(_addr, _size)       flush_dcache_range((void *)_addr, (uintptr_t)_addr + _size)
#define INV_RANGE(_addr, _size)         invalidate_dcache_range((void *)_addr, (uintptr_t)_addr + _size)
#define DMA_CACHE_LINE                  dma_get_cache_alignment()
#endif


static BBH_TX_RING_TABLE_STRUCT *bbh_pd_table = NULL;
static DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_STRUCT *counters_table = NULL;
static BBH_TX_BB_DESTINATION_TABLE_STRUCT *bb_dest_table = NULL;
static int tx_pd_idx;
static uint8_t bbh_ingress_counter[8];


#define RDD_CPU_TX_MAX_ITERS    2
#define RDD_CPU_TX_ITER_DELAY   1000
#define QM_QUEUE_INDEX_DS_FIRST QM_QUEUE_DS_START

#define RDD_CPU_TX_MAX_BUF_SIZE 2048

static int __rdd_cpu_tx_poll(uint8_t tx_port)
{
    volatile CPU_TX_DESCRIPTOR_STRUCT *tx_pd;
    volatile BBH_TX_EGRESS_COUNTER_ENTRY_STRUCT *counters_id;
    int old_packet_size;
    int iter;
    int egress_counter_val;
    uint8_t *p;

    if (!bbh_pd_table)  /* First call ? */
    {
        bbh_pd_table = RDD_BBH_TX_RING_TABLE_PTR(get_runner_idx(cfe_core_runner_image));
        bb_dest_table = RDD_BBH_TX_BB_DESTINATION_TABLE_PTR(get_runner_idx(cfe_core_runner_image));
    }
    tx_pd = (volatile CPU_TX_DESCRIPTOR_STRUCT *)&bbh_pd_table->entry;
#if CONFIG_BCM94912
    if (tx_port > 5)
    {
        counters_table = RDD_US_TM_BBH_TX_EGRESS_COUNTER_TABLE_PTR(get_runner_idx(cfe_core_runner_image));
        counters_id = (volatile BBH_TX_EGRESS_COUNTER_ENTRY_STRUCT *)&counters_table->entry[0];
        /* Wait until BBH is available*/
        p = (uint8_t *)counters_id;
        p = p + (tx_port - 6);
    }
    else
#endif
    {
        counters_table = RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_PTR(get_runner_idx(cfe_core_runner_image));
        counters_id = (volatile BBH_TX_EGRESS_COUNTER_ENTRY_STRUCT *)&counters_table->entry[0];

        /* Wait until BBH is available*/
        p = (uint8_t *)counters_id;

        /* in 6858 (XRDP_BBH_PER_LAN_PORT)egress counters are in 64 bit alignment), in other 1 byte */
#if XRDP_BBH_PER_LAN_PORT
        p = p + (tx_port * EGRESS_COUNTER_SIZE);
#else
        p = p + tx_port;
#endif
    }

    /* Wait until TX_PD becomes available */
    RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_READ(old_packet_size, tx_pd);
    RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_READ(egress_counter_val, p);

    for (iter = 0; ((old_packet_size!=0) || ((bbh_ingress_counter[tx_port] - egress_counter_val) >= 8)/*BBH_TX_FIFO_SIZE*/) && (iter < RDD_CPU_TX_MAX_ITERS); iter++)
    {
       XRDP_USLEEP(RDD_CPU_TX_ITER_DELAY);
       RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_READ(old_packet_size, tx_pd);
       RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_READ(egress_counter_val,p);
    }
    if (iter == RDD_CPU_TX_MAX_ITERS)
    {
       XRDP_ERR_MSG("non empty at index %d or bbh full(ingress=%d,egress=%d,diff=%d)\n", tx_pd_idx, bbh_ingress_counter[tx_port], egress_counter_val, bbh_ingress_counter[tx_port] - egress_counter_val);
       return BDMF_ERR_INTERNAL;
    }

    return BDMF_ERR_OK;
}

static uint32_t __rdd_cpu_tx_get_bb_id(uint8_t tx_port)
{
#ifdef CONFIG_BCM96858

    switch (tx_port)
    {
    case 0:
        return BB_ID_TX_BBH_4;
    case 1:
        return BB_ID_TX_BBH_1;
    case 2:
        return BB_ID_TX_BBH_2;
    case 3:
        return BB_ID_TX_BBH_3;
    case 4:
        return BB_ID_TX_BBH_0;
    case 5:
        return BB_ID_TX_BBH_5;
    case 6:
        return BB_ID_TX_BBH_6;
    case 7:
        return BB_ID_TX_BBH_7;
    default:
        return BB_ID_TX_BBH_0;
    }
#elif CONFIG_BCM94912
    if (tx_port <= 5)
       return (BB_ID_TX_LAN + (tx_port << 6));
    else
       return (BB_ID_TX_LAN_1 + ((tx_port - 6) << 6));
#else
    return (BB_ID_TX_LAN + (tx_port << 6));
#endif
}

#if defined(XRDP_MANAGE_SBPM)
#define MIN_PACKET_LENGTH_WITHOUT_CRC 60
int rdd_cpu_tx_new(uint8_t *buffer, uint32_t length, uint8_t tx_port)
{
    volatile CPU_TX_DESCRIPTOR_STRUCT *tx_pd;
    volatile BB_DESTINATION_ENTRY_STRUCT *bb_dest;
    uint32_t bb_id;
    uint32_t *tx_buffer_ptr = (uint32_t *)PACKET_BUFFER_POOL_TABLE_ADDR_TX;
    int rc;

    if (length >= RDD_CPU_TX_MAX_BUF_SIZE) {
        printf("ERR: can't transmit buffer with length %u longer "
               "than %d\n", length, RDD_CPU_TX_MAX_BUF_SIZE);
        return BDMF_ERR_PARM;
    }

    rc = __rdd_cpu_tx_poll(tx_port);
    if (rc)
        return rc;

    tx_pd = (volatile CPU_TX_DESCRIPTOR_STRUCT *)&bbh_pd_table->entry;
    bb_dest = (volatile BB_DESTINATION_ENTRY_STRUCT *)&bb_dest_table->entry;

    /* copy buffer to the fix location */
    memcpy(tx_buffer_ptr, buffer, length);
    FLUSH_RANGE(tx_buffer_ptr, length);

    /* complete TX descriptor */
    RDD_BBH_TX_DESCRIPTOR_LAST_WRITE(1, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_ABS_WRITE(1, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_TARGET_MEM_0_WRITE(1, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_TARGET_MEM_1_WRITE(1, tx_pd);

    bb_id = __rdd_cpu_tx_get_bb_id(tx_port);
    RDD_BB_DESTINATION_ENTRY_BB_DESTINATION_WRITE(bb_id, bb_dest);

    /*  must be latest write */
    if (unlikely(length < MIN_PACKET_LENGTH_WITHOUT_CRC))
        length = MIN_PACKET_LENGTH_WITHOUT_CRC;
    RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_WRITE(length, tx_pd);

    WMB();

    /* update ingress counter */
    bbh_ingress_counter[tx_port] += 1;
    ag_drv_rnr_regs_cfg_cpu_wakeup_set(
            get_runner_idx(cfe_core_runner_image),
            IMAGE_0_CFE_CORE_CPU_TX_THREAD_NUMBER);

    return BDMF_ERR_OK;
}
#else
int rdd_cpu_tx(uint32_t length, uint16_t bn0, uint16_t bn1, uint8_t bns_num, uint8_t tx_port)
{
    volatile CPU_TX_DESCRIPTOR_STRUCT *tx_pd;
    volatile BB_DESTINATION_ENTRY_STRUCT *bb_dest;
    uint32_t bb_id;
    int rc;

    if (length >= RDD_CPU_TX_MAX_BUF_SIZE)
    {
       XRDP_ERR_MSG("ERR: can't transmit buffer with length %u longer than %d\n", length, RDD_CPU_TX_MAX_BUF_SIZE);
       return BDMF_ERR_PARM;
    }

    rc = __rdd_cpu_tx_poll(tx_port);
    if (rc)
        return rc;

    tx_pd = (volatile CPU_TX_DESCRIPTOR_STRUCT *)&bbh_pd_table->entry;
    bb_dest = (volatile BB_DESTINATION_ENTRY_STRUCT *)&bb_dest_table->entry;

    /* transmit */
    memset((void *)tx_pd, 0, sizeof(*tx_pd));
    RDD_BBH_TX_DESCRIPTOR_SOF_WRITE(0, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_LAST_WRITE(1, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_AGG_PD_WRITE(0, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_ABS_WRITE(0, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_SOP_WRITE(0, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_BN0_FIRST_WRITE(bn0, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_BN1_FIRST_WRITE(bn1, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_TARGET_MEM_0_WRITE(1, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_TARGET_MEM_1_WRITE(1, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_BN_NUM_WRITE(bns_num, tx_pd);

    bb_id = __rdd_cpu_tx_get_bb_id(tx_port);
    RDD_BB_DESTINATION_ENTRY_BB_DESTINATION_WRITE(bb_id, bb_dest);


    /*  must be latest write */
    RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_WRITE(length, tx_pd);


    /* update ingress counter */
    bbh_ingress_counter[tx_port]+=1;
    ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(cfe_core_runner_image), IMAGE_0_CFE_CORE_CPU_TX_THREAD_NUMBER);

    return BDMF_ERR_OK;
}
#endif

