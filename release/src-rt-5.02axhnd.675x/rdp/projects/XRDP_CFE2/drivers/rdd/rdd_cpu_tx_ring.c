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
#include "bdmf_system.h"
#include "bdmf_session.h"
#include "cfe_iocb.h"
#include "lib_malloc.h"
#include "xrdp_drv_rnr_regs_ag.h"

extern void cfe_usleep(int);
#define XRDP_USLEEP(_a)         cfe_usleep(_a)
#define XRDP_ERR_MSG(args...)   printf(args)
extern void _cfe_flushcache ( int, uint8_t *, uint8_t * );
#define FLUSH_RANGE(s,l)        \
    do { \
        __asm__ __volatile__ ("dsb    sy");\
        _cfe_flushcache(CFE_CACHE_FLUSH_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l));\
    } while (0)


static RDD_BBH_TX_RING_TABLE_DTS *bbh_pd_table;
static RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_DTS *counters_table;
static RDD_BBH_TX_BB_DESTINATION_TABLE_DTS *bb_dest_table;
static int tx_pd_idx;
static uint8_t bbh_ingress_counter[8];


#define RDD_CPU_TX_MAX_ITERS    2
#define RDD_CPU_TX_ITER_DELAY   1000
#define QM_QUEUE_INDEX_DS_FIRST QM_QUEUE_DS_START

#define RDD_CPU_TX_MAX_BUF_SIZE 2048


int rdd_cpu_tx(uint32_t fpm_bn, uint32_t length, uint8_t tx_port)
{
    //printf("try to transmit fpm=0x%x, len=%d to port=%d, pd_idx = %d\n", fpm_bn, length, tx_port, tx_pd_idx);
    volatile RDD_CPU_TX_DESCRIPTOR_DTS *tx_pd;
    volatile RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS *counters_id;
    volatile RDD_BB_DESTINATION_ENTRY_DTS *bb_dest;
    int old_packet_size;
    int iter;
    int egress_counter_val;
    uint8_t *p;

    if (length >= RDD_CPU_TX_MAX_BUF_SIZE)
    {
       XRDP_ERR_MSG("ERR: can't transmit buffer with length %u longer than %d\n", length, RDD_CPU_TX_MAX_BUF_SIZE);
       return BDMF_ERR_PARM;
    }

    if (!bbh_pd_table)  /* First call ? */
    {
        bbh_pd_table = RDD_BBH_TX_RING_TABLE_PTR(get_runner_idx(cfe_core_runner_image));
        counters_table = RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_PTR(get_runner_idx(cfe_core_runner_image));
        bb_dest_table = RDD_BBH_TX_BB_DESTINATION_TABLE_PTR(get_runner_idx(cfe_core_runner_image));
    }
    tx_pd = (volatile RDD_CPU_TX_DESCRIPTOR_DTS *)&bbh_pd_table->entry;
    counters_id = (volatile RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS *)&counters_table->entry[0];
    bb_dest = (volatile RDD_BB_DESTINATION_ENTRY_DTS *)&bb_dest_table->entry;

    /* Wait until TX_PD becomes available */
    RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_READ(old_packet_size, tx_pd);

    /* Wait until BBH is  available*/
    p = (uint8_t*)counters_id;
    p = p + tx_port;
    RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_READ(egress_counter_val,p);
    //XRDP_ERR_MSG("egress 0 = %d\n", egress_counter_val);

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

    /*transmit */
    memset((void *)tx_pd, 0, sizeof(*tx_pd));
    RDD_BBH_TX_DESCRIPTOR_SOF_WRITE(0, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_LAST_WRITE(1, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_AGG_PD_WRITE(0, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_ABS_WRITE(0, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_SOP_WRITE(0, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_BN0_FIRST_WRITE(fpm_bn, tx_pd);
    RDD_BBH_TX_DESCRIPTOR_BN1_FIRST_WRITE(0, tx_pd);

    RDD_BB_DESTINATION_ENTRY_BB_DESTINATION_WRITE(BB_ID_TX_LAN + (tx_port<<6),bb_dest);
    /*  must be latest write */
    RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_WRITE(length, tx_pd);


    /* update ingress counter */
    bbh_ingress_counter[tx_port]+=1;
    //XRDP_ERR_MSG("success to transmit index %d bbh -(ingress=%d,egress=%d,diff=%d)\n", tx_pd_idx, bbh_ingress_counter[tx_port], egress_counter_val, bbh_ingress_counter[tx_port] - egress_counter_val);
    ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(cfe_core_runner_image), IMAGE_0_CFE_CORE_CPU_TX_THREAD_NUMBER);
    /*
    if (tx_pd_idx == RDD_BBH_TX_RING_TABLE_SIZE - 1)
       tx_pd_idx = 0;
    else
       ++tx_pd_idx;
    */

    return BDMF_ERR_OK;
}





