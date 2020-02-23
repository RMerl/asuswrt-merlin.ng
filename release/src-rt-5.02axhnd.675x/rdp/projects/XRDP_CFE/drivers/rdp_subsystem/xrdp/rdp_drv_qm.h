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
#if defined(BCM6836) || defined(BCM6846)
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
    uint32_t num_fpm_ug;                        /* Number of valid elements in fpm_ug_thr array */
    qm_fpm_ug_thr fpm_ug_thr[4];                /* FPM UG threshold configuration */
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
#define QM_WRED_PROFILE_EPON            14

#define QM_MAX_NUM_OF_WRED_PROFILES     16
#define QM_MAX_COPY_DECISION_PROFILE    8
#define QM_MAX_FPM_USER_GRP_CFG         4

/******************************************************************************/
/*                                                                            */
/* Functions prototypes                                                       */
/*                                                                            */
/******************************************************************************/

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
        ag_drv_qm_total_valid_cnt_get((qm_queue_num * 2), &qm_pd_cnt[qm_q_cnt_index]);

    /* reserved 32 slots for specific packets (1588, PON, ) with highest priority*/
    if ((qm_pd_cnt[qm_q_cnt_index] > QM_PD_CPU_INTERFACE_SPECIFIC_THRESHOLD) && !exclusive)
        return BDMF_ERR_NORES;

    if (qm_pd_cnt[qm_q_cnt_index] > QM_PD_CPU_INTERFACE_HIGH_THRESHOLD)
        return BDMF_ERR_NORES;

#if !defined(RDP_SIM) || defined(XRDP_EMULATION)
#ifndef _CFE_
    /*Use exclusive test and change as atomic operation*/
#ifndef XRDP_EMULATION
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

int drv_qm_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val);

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
