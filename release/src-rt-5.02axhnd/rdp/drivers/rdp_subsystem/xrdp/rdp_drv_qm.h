/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/


#ifndef DRV_QM_H_INCLUDED
#define DRV_QM_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#include "xrdp_drv_qm_ag.h"
#include "rdp_subsystem_common.h"
#if defined(RDP_SIM) && !defined(XRDP_EMULATION)
#include "rdp_cpu_sim.h"
#endif
#ifdef CONFIG_DHD_RUNNER
#include "dhd_defs.h"
#include "data_path_init.h"
#ifndef _CFE_
#include "rdd_dhd_helper.h"
#endif
#endif
#include "rdp_drv_fpm.h"

/*
 * Constants
 * ToDo: possibly belong elsewhere
 */

#define QM_PD_CPU_INTERFACE_HIGH_THRESHOLD  (15000)
#define QM_PD_CPU_INTERFACE_SPECIFIC_THRESHOLD  (15000-32)
#define QM_PD_CPU_INTERFACE_LOW_THRESHOLD   (14500)

/* Queue context bits */
#define QM_Q_CTX_WRED_PROFILE_S         0
#define QM_Q_CTX_WRED_PROFILE_W         4
#define QM_Q_CTX_CD_PROFILE_S           4
#define QM_Q_CTX_CD_PROFILE_W           3
#define QM_Q_CTX_COPY_TO_DDR_S          7
#define QM_Q_CTX_COPY_TO_DDR_W          1
#define QM_Q_CTX_NO_COPY_TO_DDR_S       8
#define QM_Q_CTX_NO_COPY_TO_DDR_W       1
#define QM_Q_CTX_NO_AGGREGATION_S       9
#define QM_Q_CTX_NO_AGGREGATION_W       1
#define QM_Q_CTX_FPM_UG_S               10
#define QM_Q_CTX_FPM_UG_W               2
#define QM_Q_CTX_EXCLUSIVE_PRTY_S       12
#define QM_Q_CTX_EXCLUSIVE_PRTY_W       1
#define QM_Q_CTX_802_1AE_S              13
#define QM_Q_CTX_802_1AE_W              1
#define QM_Q_CTX_SCI_S                  14
#define QM_Q_CTX_SCI_W                  1
#define QM_Q_CTX_FEC_ENABLE_S           15
#define QM_Q_CTX_FEC_ENABLE_W           1

/* UBUS SLAVE configuration */
#define QM_UBUS_SLV_APB_BASE    0x82100000
#define QM_UBUS_SLV_APB_MASK    0x000c0000
#define QM_UBUS_SLV_VPB_BASE    0x82140000
#define QM_UBUS_SLV_VPB_MASK    0x000c0000
#define QM_UBUS_SLV_DQM_BASE    0x82180000
#define QM_UBUS_SLV_DQM_MASK    0x000c0000

/* BBH TX configuration */
#define QM_BBH_TX_IDX           5       /* <--- !!! */
#define QM_BBH_TX_BBCFG_1_TX    0x002f1a00
#define QM_BBH_TX_SDMACFG_TX    0x003f0fc0
#if defined(BCM6856) || defined(BCM6846)
#define QM_BBH_TX_DFIFOCTRL     0x0000017f
#else
#define QM_BBH_TX_DFIFOCTRL     0x00000120
#endif
#define QM_BBH_TX_LANCFG_PDSIZE 0x0000001f

/* DMA configuration */
#define QM_DMA_SOURCE           0x0000001a
#define QM_DMA_NUM_READS        0x0000003f
#define QM_DMA_NUM_WRITES       0x0000001c
#if !defined(BCM6858)
#define QM_DMA_PERIPH_RX_SOURCE 0x0
#define QM_DMA_PERIPH_TX_SOURCE 0x1c
#else
#define QM_DMA_PERIPH_SOURCE    0x00001c00
#endif
#define QM_DMA_PERIPH_MAX_OTF   0x0000003f

#define QM_EPON_IPG_DEFAULT_LENGTH 0xa

#define QM_WRED_PROFILE_MAX_VAL ((1 << 22) - 1)

/* qcfg_g bits*/
#define QM_CNFG_G_DISABLE_COPY_TO_DDR_S 0
#define QM_CNFG_G_FORCE_COPY_TO_DDR_S   1
#define QM_CNFG_G_DISABLE_AGGREGATION_S 2

#define NUM_OF_FPM_UG           4
#define NUM_OF_FPM_POOLS        4

#define FPM_RES_XEPON           0
#define FPM_RES_WLAN            1
#define FPM_RES_MBR             2
#define FPM_RES_NONE            3

#define IMIX_AVERAGE_PACKET_SIZE            366         /* IMIX 384 - 68(58.33%),598(33.33%),1522(8.33%) */
#define RDD_MBR_BUFFER_SIZE_QUANTUM         512         /* res_thr 512bytes resolution */

/* minimum buffer reservation */
#if defined(BCM6856)
#define QM_MBR_PROFILE__NUM_OF              4           /* Number of QM minimum buffer reservation profiles 
                                                          available for allocation. Profile 0 should remain 0*/ 
#define QM_MBR_PROFILE_RESOLUTION           16          /* HW uses bits [11:4] from profile */
#else
#define QM_MBR_PROFILE__NUM_OF              8           /* Number of QM minimum buffer reservation profiles 
                                                          available for allocation. Profile 0 should remain 0*/ 
#define QM_MBR_PROFILE_RESOLUTION           1
#endif

/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/

/* QM initialization parameters */
typedef struct qm_init_cfg
{
    uint32_t fpm_base;
    uint8_t fpm_buf_size;
    bdmf_boolean is_counters_read_clear;        /* TRUE=read-clear counter access mode */
    bdmf_boolean is_drop_counters_enable;       /* TRUE=drop counters enable. FALSE=max occupancy holder */
    uint16_t ddr_sop_offset[2];                 /* DDR SoP offsets */
    bdmf_boolean is_close_agg_disable;          /* TRUE=aggregations are not closed automatically */
} qm_init_cfg;

typedef struct qm_fpm_ctrl_cfg
{
    bdmf_boolean fpm_pool_bp_enable;
    uint8_t fpm_prefetch_min_pool_size;
    bdmf_boolean fpm_congestion_bp_enable;
    uint8_t fpm_prefetch_pending_req_limit;
} qm_fpm_ctrl_cfg;

typedef struct
{
    uint32_t qm_wred_drop;
    uint32_t qm_fpm_prefetch_drop[NUM_OF_FPM_POOLS];
    uint32_t qm_fpm_cong_drop;
    uint32_t qm_fpm_grp_drop[NUM_OF_FPM_UG];
    uint32_t qm_ddr_pd_cong_drop;
    uint32_t qm_pd_cong_drop;
    uint32_t qm_psram_egress_cong_drop;
} drv_qm_drop_cntr_t;

/* PD fifo - prefetch size */
typedef enum qm_pd_fifo_size_e
{
    qm_pd_fifo_size_2 = 0,
    qm_pd_fifo_size_4 = 1,
    qm_pd_fifo_size_8 = 2
} qm_pd_fifo_size_t;

/* Update fifo - fifo size */
typedef enum qm_update_fifo_size_e
{
    qm_update_fifo_size_8 = 0,
    qm_update_fifo_size_16 = 1,
    qm_update_fifo_size_32 = 2,
    qm_update_fifo_size_64 = 3
} qm_update_fifo_size_t;

/* Update fifo - group id */
typedef enum qm_rnr_group_e
{
    qm_rnr_group_0 = 0,
    qm_rnr_group_1 = 1,
    qm_rnr_group_2 = 2,
    qm_rnr_group_3 = 3,
    qm_rnr_group_4 = 4,
    qm_rnr_group_5 = 5,
    qm_rnr_group_6 = 6,
    qm_rnr_group_7 = 7,
    qm_rnr_group_8 = 8,
    qm_rnr_group_9 = 9,
    qm_rnr_group_10 = 10,
    qm_rnr_group_11 = 11,
    qm_rnr_group_12 = 12,
    qm_rnr_group_13 = 13,
    qm_rnr_group_14 = 14,
    qm_rnr_group_15 = 15,
    qm_rnr_group_num = 16

} qm_rnr_group_t;

typedef enum qm_cpu_indr_cmd_e
{
    qm_cpu_indr_cmd_nothing = 0,
    qm_cpu_indr_cmd_write = 1,
    qm_cpu_indr_cmd_read = 2
} qm_cpu_indr_cmd_t;

/* Index of WRED profile that is reserved for disabled queues - DROP ALL */
#define QM_WRED_PROFILE_DROP_ALL        15
#define QM_WRED_PROFILE_CPU_RX          14

#define QM_WRED_PROFILE_CPU_RX_THR_MIN  8000
#define QM_WRED_PROFILE_CPU_RX_THR_MAX  24000

#define QM_MAX_NUM_OF_WRED_PROFILES     16
#define QM_MAX_COPY_DECISION_PROFILE    8

#define QM_ILLEGAL_QUEUE                -1
/******************************************************************************/
/*                                                                            */
/* Functions prototypes                                                       */
/*                                                                            */
/******************************************************************************/

bdmf_error_t drv_qm_system_init(dpi_params_t *p_dpi_cfg);
int drv_qm_get_us_epon_start(void);
int drv_qm_get_us_epon_end(void);
int drv_qm_get_us_start(void);
int drv_qm_get_us_end(void);
int drv_qm_get_ds_start(void);
int drv_qm_get_ds_end(void);
void drv_qm_drop_counter_clr(void);
int drv_qm_get_sq_start(void);
int drv_qm_get_sq_end(void);
void drv_qm_update_queue_tables(void);
bdmf_error_t drv_qm_init(const qm_init_cfg *init_cfg);
bdmf_error_t drv_qm_fpm_min_pool_get(uint16_t* fpm_min_pool_size);
bdmf_error_t drv_qm_exit(void);

/* Config and enable queue */
bdmf_error_t drv_qm_queue_config(rdp_qm_queue_idx_t q_idx, const qm_q_context *cfg);
/* Get Queue Configuration */
bdmf_error_t drv_qm_queue_get_config(rdp_qm_queue_idx_t q_idx, qm_q_context *cfg);

/* Enable queue. Must be configured first */
bdmf_error_t drv_qm_queue_enable(rdp_qm_queue_idx_t q_idx);

/* Disable and flush queue */
bdmf_error_t drv_qm_queue_disable(rdp_qm_queue_idx_t q_idx, bdmf_boolean flush);

/* Set/Reset force copy to DDR on queue */
bdmf_error_t force_copy_ddr_on_queue(rdp_qm_queue_idx_t q_idx, bdmf_boolean force_copy_to_ddr);

#ifndef _CFE_
extern bdmf_fastlock  qm_engine_lock[2];
#endif /* _CFE_ */

static inline bdmf_error_t drv_qm_cpu_tx(uint32_t *cpu_tx_descriptor, uint16_t qm_queue_num, uint8_t no_lock, uint8_t exclusive)
{
#if !defined(RDP_SIM) || defined(XRDP_EMULATION)
    /*atomic binary operation must be on unsigned long aligned pointers*/
    static uint64_t qm_interface __attribute__((aligned(8))) = 0;
    uint8_t local_qm_interface = qm_interface;
#endif
    // counter for each Queue, egress/ingress
    static uint32_t qm_pd_cnt[2];
    /*qm_queue_num can be 256 or 257*/
    uint8_t qm_q_cnt_index = qm_queue_num & 1;

    /* read counter from QM if bigger then low treshold*/
    if (qm_pd_cnt[qm_q_cnt_index] > QM_PD_CPU_INTERFACE_LOW_THRESHOLD)
        ag_drv_qm_total_valid_cnt_get((qm_queue_num * (sizeof(RDD_QM_QUEUE_COUNTER_DATA_DTS) / 4)), &qm_pd_cnt[qm_q_cnt_index]);

    /* reserved 32 slots for specific packets (1588, PON, ) with highest priority*/
    if ((qm_pd_cnt[qm_q_cnt_index] > QM_PD_CPU_INTERFACE_SPECIFIC_THRESHOLD) && !exclusive)
        return BDMF_ERR_NORES;

    if (qm_pd_cnt[qm_q_cnt_index] > QM_PD_CPU_INTERFACE_HIGH_THRESHOLD)
        return BDMF_ERR_NORES;

#if !defined(RDP_SIM) || defined(XRDP_EMULATION)
#ifndef _CFE_
    /*Use exclusive test and change as atomic operation*/
#ifndef XRDP_EMULATION

#ifdef CONFIG_BCM_CACHE_COHERENCY
    /*Must invoke memory barrier here to prevent cache coherency mistakes between host and Runner's DMA*/
    dma_wmb();
#endif

#ifdef BCM6858
    local_qm_interface = 0;
#else
    local_qm_interface = test_and_change_bit(0, (volatile void *)&qm_interface);
#endif
#endif
    if (likely(!no_lock))
    {
        bdmf_fastlock_lock(&qm_engine_lock[local_qm_interface]);

        /* write PD to QM */
        ag_drv_qm_cpu_pd_indirect_wr_data_set(local_qm_interface, *cpu_tx_descriptor, *(cpu_tx_descriptor + 1),
            *(cpu_tx_descriptor + 2), *(cpu_tx_descriptor + 3));
        ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set(local_qm_interface, qm_queue_num, qm_cpu_indr_cmd_write, 0, 0);

        bdmf_fastlock_unlock(&qm_engine_lock[local_qm_interface]);
    }
    else
#endif /* _CFE_ */
    {
        /* write PD to QM */
        ag_drv_qm_cpu_pd_indirect_wr_data_set(local_qm_interface, *cpu_tx_descriptor, *(cpu_tx_descriptor + 1),
            *(cpu_tx_descriptor + 2), *(cpu_tx_descriptor + 3));
        ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set(local_qm_interface, qm_queue_num, qm_cpu_indr_cmd_write, 0, 0);
    }

    ++qm_pd_cnt[qm_q_cnt_index];
#else
    rdp_cpu_qm_req(cpu_tx_descriptor, qm_queue_num);
#endif
    return BDMF_ERR_OK;
}

int drv_qm_drop_stat_get(drv_qm_drop_cntr_t *qm_drop_stat);
int drv_qm_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val);

bdmf_error_t drv_qm_fpm_buffer_reservation_profile_cfg(uint8_t profile_id, uint16_t token_threshold);
bdmf_error_t set_fpm_budget(int resource_num, int add, uint32_t reserved_packet_buffer);
bdmf_boolean is_qm_queue_aggregation_context_valid(uint16_t qm_queue);

#ifdef USE_BDMF_SHELL
int drv_qm_cli_debug_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_qm_cli_sanity_check(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_qm_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
void drv_qm_cli_init(bdmfmon_handle_t driver_dir);
void drv_qm_cli_exit(bdmfmon_handle_t driver_dir);
#endif


#ifdef __cplusplus
}
#endif

#endif
