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


/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/

#include "rdp_common.h"
#include "rdp_drv_qm.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "rdp_drv_dqm.h"
#include "rdd_ghost_reporting.h"
#include "rdd_data_structures_auto.h"
#include "data_path_init.h"

extern dpi_params_t *p_dpi_cfg;

typedef struct
{
    int queue_us_epon_start;
    int queue_us_epon_end;
    int queue_us_start;
    int queue_us_end;
    int queue_ds_start;
    int queue_ds_end;
    int queue_service_start;
    int queue_service_end;
} queue_locations_s;

static queue_locations_s g_queue_locations = {};

void drv_qm_update_queue_tables(void)
{
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_US_START_WRITE_G(g_queue_locations.queue_us_start, RDD_PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_US_START_WRITE_G(g_queue_locations.queue_us_start, RDD_GENERAL_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);

    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_US_END_WRITE_G(g_queue_locations.queue_us_end, RDD_PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_US_END_WRITE_G(g_queue_locations.queue_us_end, RDD_GENERAL_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);

    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_DS_START_WRITE_G(g_queue_locations.queue_ds_start, RDD_PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_DS_START_WRITE_G(g_queue_locations.queue_ds_start, RDD_GENERAL_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);

    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_DS_END_WRITE_G(g_queue_locations.queue_ds_end, RDD_PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_DS_END_WRITE_G(g_queue_locations.queue_ds_end, RDD_GENERAL_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);

    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_EPON_START_WRITE_G(g_queue_locations.queue_us_epon_start, RDD_PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_EPON_START_WRITE_G(g_queue_locations.queue_us_epon_start, RDD_GENERAL_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);

    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_SQ_START_WRITE_G(g_queue_locations.queue_service_start, RDD_PROCESSING_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);
    RDD_QUEUE_DYNAMIC_MNG_ENTRY_QM_QUEUE_SQ_START_WRITE_G(g_queue_locations.queue_service_start, RDD_GENERAL_QUEUE_DYNAMIC_MNG_TABLE_ADDRESS_ARR, 0);
}

bdmf_error_t drv_qm_system_init(dpi_params_t *p_dpi_cfg)
{
    int last_index;
    int total_required_size;
#ifdef _CFE_
    p_dpi_cfg->number_of_ds_queues = QM_QUEUE_DS_DEFAULT_QUANTITY;
    p_dpi_cfg->number_of_us_queues = QM_QUEUE_US_DEFAULT_QUANTITY;
    p_dpi_cfg->number_of_service_queues = 0;
#endif
    last_index = 0;
    total_required_size = p_dpi_cfg->number_of_ds_queues + p_dpi_cfg->number_of_us_queues + p_dpi_cfg->number_of_service_queues;
    if ((total_required_size> QM_QUEUE_MAX_DYNAMIC_QUANTITY) || (p_dpi_cfg->number_of_ds_queues > RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE) || (p_dpi_cfg->number_of_us_queues > RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE))
    {
        return BDMF_ERR_NORES;
    }

    //first set all queus to be empty
    g_queue_locations.queue_us_epon_start = QM_ILLEGAL_QUEUE;
    g_queue_locations.queue_us_epon_end = QM_ILLEGAL_QUEUE;
    g_queue_locations.queue_us_start = QM_ILLEGAL_QUEUE;
    g_queue_locations.queue_us_end = QM_ILLEGAL_QUEUE;
    g_queue_locations.queue_ds_start = QM_ILLEGAL_QUEUE;
    g_queue_locations.queue_ds_end = QM_ILLEGAL_QUEUE;
    g_queue_locations.queue_service_start = QM_ILLEGAL_QUEUE;
    g_queue_locations.queue_service_end = QM_ILLEGAL_QUEUE;

    //init queue according to system configuration
    //us and DS will have at least one Q

    /* US */
    g_queue_locations.queue_us_start = last_index;
    g_queue_locations.queue_us_end = last_index + p_dpi_cfg->number_of_us_queues -1;
    last_index = g_queue_locations.queue_us_end + 1;
#ifndef _CFE_
    /* EPON */
    if (total_required_size + QM_QUEUE_DYNAMIC_EPON <= QM_QUEUE_MAX_DYNAMIC_QUANTITY)
    {
        //we have also enough space for EPON
        g_queue_locations.queue_us_epon_start = last_index;
        g_queue_locations.queue_us_epon_end = last_index + QM_QUEUE_DYNAMIC_EPON -1;
        last_index = g_queue_locations.queue_us_epon_end + 1;
    }
#endif

    /* DS */
    g_queue_locations.queue_ds_start = last_index;
    g_queue_locations.queue_ds_end = last_index + p_dpi_cfg->number_of_ds_queues -1;
    last_index = g_queue_locations.queue_ds_end + 1;

    /* SERVICE */
    if (p_dpi_cfg->number_of_service_queues)
    {
        g_queue_locations.queue_service_start = last_index;
        g_queue_locations.queue_service_end = last_index + p_dpi_cfg->number_of_service_queues -1;
        last_index = g_queue_locations.queue_service_end + 1;
    }

    return BDMF_ERR_OK;
}

int drv_qm_get_max_dynamic_queue_number(void)
{
    return QM_QUEUE_MAX_DYNAMIC_QUANTITY;
}
EXPORT_SYMBOL(drv_qm_get_max_dynamic_queue_number);

int drv_qm_get_us_epon_start(void)
{
    return g_queue_locations.queue_us_epon_start;
}

int drv_qm_get_us_epon_end(void)
{
    return g_queue_locations.queue_us_epon_end;
}

int drv_qm_get_us_start(void)
{
    return g_queue_locations.queue_us_start;
}

int drv_qm_get_us_end(void)
{
    return g_queue_locations.queue_us_end;
}

int drv_qm_get_ds_start(void)
{
    return g_queue_locations.queue_ds_start;
}

int drv_qm_get_ds_end(void)
{
    return g_queue_locations.queue_ds_end;
}

int drv_qm_get_sq_start(void)
{
    return g_queue_locations.queue_service_start ;
}

int drv_qm_get_sq_end(void)
{
    return g_queue_locations.queue_service_end ;
}

#ifndef _CFE_
bdmf_fastlock   qm_engine_lock[2];
#endif /* _CFE_ */

#define QM_ERR_RETURN(err, fmt, args...) \
    do { \
        if (err) \
        { \
            bdmf_trace("ERR: QM: %s#%d: " fmt, __FUNCTION__, __LINE__, ## args);\
            return err; \
        } \
    } while (0)

#define RU_REQ_WRITE_CONST(i,b,r,v) \
    do { \
        uint32_t _v = v; \
        RU_REG_WRITE(i, b, r, _v);\
    } while (0)

/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* Macros definitions                                                         */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/* Global variables definitions                                               */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/* API functions implementations                                              */
/*                                                                            */
/******************************************************************************/

static inline void drv_qm_drop_counter_set(rdp_qm_queue_idx_t q_idx, uint32_t * data)
{
    uint32_t reg_drop_counter_counter = 0;
    RU_REG_RAM_WRITE(0, q_idx, QM, DROP_COUNTER_COUNTER, reg_drop_counter_counter);
}

static bdmf_boolean qm_initialized = 0;

/* Initialize and enable QM */
bdmf_error_t drv_qm_init(const qm_init_cfg *init_cfg)
{
    bdmf_error_t err;
    uint32_t i;
    bdmf_boolean fpm_pool_bp_enable, fpm_congestion_bp_enable;
    uint8_t fpm_prefetch_pending_req_limit, buff_size;
    qm_fpm_pool_thr fpm_pool_thr = {};

    if (qm_initialized)
        return BDMF_ERR_ALREADY;


    /* Init UBUS slave */
    {
        qm_ubus_slave ubus_slave =
        {
            .vpb_base = QM_UBUS_SLV_VPB_BASE,
            .vpb_mask = QM_UBUS_SLV_VPB_MASK,
            .apb_base = QM_UBUS_SLV_APB_BASE,
            .apb_mask = QM_UBUS_SLV_APB_MASK,
            .dqm_base = QM_UBUS_SLV_DQM_BASE,
            .dqm_mask = QM_UBUS_SLV_DQM_MASK,
        };

        err = ag_drv_qm_ubus_slave_set(&ubus_slave);
        QM_ERR_RETURN(err, "ag_drv_qm_ubus_slave_set()\n");
    }

    /* Init BBH_TX */
    RU_REQ_WRITE_CONST(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, QM_BBH_TX_BBCFG_1_TX);
    RU_REQ_WRITE_CONST(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, QM_BBH_TX_SDMACFG_TX);
    RU_REQ_WRITE_CONST(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL, QM_BBH_TX_DFIFOCTRL);
    RU_REQ_WRITE_CONST(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE, QM_BBH_TX_LANCFG_PDSIZE);

    /* Init DMA */
#if !defined(BCM6858)
    RU_REQ_WRITE_CONST(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, QM_BBH_TX_SDMACFG_TX);
    for (i = 0; i < 8; i++)
    {
        ag_drv_qm_dma_qm_dma_config_num_of_reads_set(i, QM_DMA_NUM_READS);
        ag_drv_qm_dma_qm_dma_config_num_of_writes_set(i, QM_DMA_NUM_WRITES);
    }
    ag_drv_qm_dma_qm_dma_config_periph_source_set(0, QM_DMA_PERIPH_RX_SOURCE, QM_DMA_PERIPH_TX_SOURCE);
    ag_drv_qm_dma_qm_dma_config_max_otf_set(QM_DMA_PERIPH_MAX_OTF);
#else
    RU_REQ_WRITE_CONST(0, QM, DMA_QM_DMA_CONFIG_NUM_OF_READS, QM_DMA_NUM_READS);
    RU_REQ_WRITE_CONST(0, QM, DMA_QM_DMA_CONFIG_PERIPH_SOURCE, QM_DMA_PERIPH_SOURCE);
    RU_REQ_WRITE_CONST(0, QM, DMA_QM_DMA_CONFIG_MAX_OTF, QM_DMA_PERIPH_MAX_OTF);
#endif

    /*
     * Init QM
     */
    /* We leave most registers at their reset values
     * - FPM_PREFETCH_CONTROL
     * - DDR_CONGESTION_CONTROL
     * - FPM_POOLS
     * - [FPM_USR_GRP]
     * - FPM_RUNNER_GRP - all groups are disabled
     * - CPU_GRP - all groups are disabled
     * - WRED profile[0-13] - pass all
     * - COPY_DECISION_PROFILE
     */
     
    /* Set UG thresholds to default */
    err = set_fpm_budget(FPM_RES_NONE, 0, 0);
    QM_ERR_RETURN(err, "ag_drv_qm_fpm_ug_thr_set()\n");

    /* Drop counters behavior */
    {
        qm_drop_counters_ctrl drop_ctrl = {};

        drop_ctrl.exclusive_dont_drop = 1;
        drop_ctrl.exclusive_dont_drop_bp_en = 1;
        drop_ctrl.qm_preserve_pd_with_fpm = 0;   /* should be 0 drop according to precedence not if fpm is already allocated */
        drop_ctrl.fpm_buffer_global_res_enable = 0; /* 6858/46: Must be set to zero at all times. 6856: MBR activation flag will */
                                                    /* be activated upon feature configuration.                                  */
#if defined(BCM6858) || defined(BCM6856)
        drop_ctrl.qm_residue_per_queue = 1;
#endif        
#if !defined(BCM6846) && !defined(BCM6856)
        drop_ctrl.close_aggregation_on_timeout_d = init_cfg->is_close_agg_disable;
#else
        drop_ctrl.close_aggregation_on_timeout_disable = init_cfg->is_close_agg_disable;
#endif

        if (init_cfg->is_counters_read_clear)
        {
            drop_ctrl.read_clear_bytes = 1;
            drop_ctrl.read_clear_pkts = 1;
        }
        if (!init_cfg->is_drop_counters_enable)
        {
            drop_ctrl.disable_wrap_around_bytes = 1;
            drop_ctrl.disable_wrap_around_pkts = 1;
        }
        err = ag_drv_qm_drop_counters_ctrl_set(&drop_ctrl);
        QM_ERR_RETURN(err, "ag_drv_qm_drop_counters_ctrl_set()\n");
    }

    fpm_pool_thr.higher_thr = 2;
    fpm_pool_thr.lower_thr = 2;

    err = ag_drv_qm_fpm_pool_thr_set(0, &fpm_pool_thr);
    QM_ERR_RETURN(err, "ag_drv_qm_fpm_pool_thr_set()\n");
    err = ag_drv_qm_fpm_pool_thr_set(1, &fpm_pool_thr);
    QM_ERR_RETURN(err, "ag_drv_qm_fpm_pool_thr_set()\n");
    err = ag_drv_qm_fpm_pool_thr_set(2, &fpm_pool_thr);
    QM_ERR_RETURN(err, "ag_drv_qm_fpm_pool_thr_set()\n");
    err = ag_drv_qm_fpm_pool_thr_set(3, &fpm_pool_thr);
    QM_ERR_RETURN(err, "ag_drv_qm_fpm_pool_thr_set()\n");

    err = ag_drv_qm_fpm_base_addr_set(init_cfg->fpm_base);
    QM_ERR_RETURN(err, "ag_drv_qm_fpm_base_addr_set()\n");

    err =  ag_drv_qm_fpm_ctrl_get(&fpm_pool_bp_enable, &fpm_congestion_bp_enable, &buff_size, &fpm_prefetch_pending_req_limit);
    
    /* pool backpressure enable */
    fpm_pool_bp_enable = 1;
    QM_ERR_RETURN(err, "ag_drv_qm_fpm_ctrl_get()\n");
    err = ag_drv_qm_fpm_ctrl_set(fpm_pool_bp_enable, fpm_congestion_bp_enable, init_cfg->fpm_buf_size, fpm_prefetch_pending_req_limit);
    QM_ERR_RETURN(err, "ag_drv_qm_fpm_ctrl_set()\n");

    err = ag_drv_qm_ddr_sop_offset_set(init_cfg->ddr_sop_offset[0], init_cfg->ddr_sop_offset[1]);
    QM_ERR_RETURN(err, "ag_drv_qm_ddr_sop_offset_set()\n");

    /* Configure DROP_ALL WRED and CPU_RX profiles */
    {
        /* DROP_ALL WRED profile */
        qm_wred_profile_cfg wred_cfg = {};
        err = ag_drv_qm_wred_profile_cfg_set(QM_WRED_PROFILE_DROP_ALL, &wred_cfg);
        QM_ERR_RETURN(err, "ag_drv_qm_wred_profile_cfg_set()\n");
        /* CPU_RX WRED profile */
        wred_cfg.min_thr0 = QM_WRED_PROFILE_CPU_RX_THR_MIN;
        wred_cfg.min_thr1 = QM_WRED_PROFILE_CPU_RX_THR_MAX;
        wred_cfg.max_thr0 = QM_WRED_PROFILE_CPU_RX_THR_MIN;
        wred_cfg.max_thr1 = QM_WRED_PROFILE_CPU_RX_THR_MAX;
        err = ag_drv_qm_wred_profile_cfg_set(QM_WRED_PROFILE_CPU_RX, &wred_cfg);
        QM_ERR_RETURN(err, "ag_drv_qm_wred_profile_cfg_set()\n");
    }

    /* Clear Q context SRAM */
    {
        qm_q_context ctx = {.wred_profile = QM_WRED_PROFILE_DROP_ALL};
        for (i = 0; i < QM_NUM_QUEUES; i++)
        {
            err = ag_drv_qm_q_context_set(i, &ctx);
        }
    }

#ifdef QM_TOTAL_VALID_COUNTER_LEGACY_STRUCTURE
    for (i = 0; i < QM_NUM_QUEUES*2; i++)
    {
        ag_drv_qm_total_valid_cnt_set(i, 0);
        ag_drv_qm_dqm_valid_cnt_set(i, 0);
        drv_qm_drop_counter_set(i, 0);
    }
#else
    for (i = 0; i < QM_NUM_QUEUES*4; i++)
    {
        ag_drv_qm_total_valid_cnt_set(i, 0);
    }
    for (i = 0; i < QM_NUM_QUEUES*2; i++)
    {
        ag_drv_qm_dqm_valid_cnt_set(i, 0);
        drv_qm_drop_counter_set(i, 0);
    }
#endif
    /* each queue has: counter - 4 bytes ; overhead - 4 bytes*/
    for (i = 0; i < QM_NUM_REPORTED_QUEUES*2; i++)
    {
        ag_drv_qm_epon_q_byte_cnt_set(i, 0);
    }

#ifndef _CFE_    
    /* init qm engine locks*/
    bdmf_fastlock_init(&qm_engine_lock[0]);
    bdmf_fastlock_init(&qm_engine_lock[1]);
#endif

    qm_initialized = 1;

    return BDMF_ERR_OK;
}

void drv_qm_drop_counter_clr(void)
{
    uint32_t i;

    for (i = 0; i < QM_NUM_QUEUES*2; i++)
    {
        drv_qm_drop_counter_set(i, 0);
    }
}

bdmf_error_t drv_qm_fpm_min_pool_get(uint16_t* fpm_min_pool_size)
{
    qm_fpm_ctrl_cfg fpm_ctrl_cfg = {};
    bdmf_error_t rc = ag_drv_qm_fpm_ctrl_get(&(fpm_ctrl_cfg.fpm_pool_bp_enable),
            &(fpm_ctrl_cfg.fpm_congestion_bp_enable),
            &(fpm_ctrl_cfg.fpm_prefetch_min_pool_size),
            &(fpm_ctrl_cfg.fpm_prefetch_pending_req_limit));
    if (!rc)
    {
        switch(fpm_ctrl_cfg.fpm_prefetch_min_pool_size)
        {
        case 0:
            *fpm_min_pool_size = 256;
            break;
        case 1:
            *fpm_min_pool_size = 512;
            break;
        case 2:
            *fpm_min_pool_size = 1024;
            break;
        case 3:
            *fpm_min_pool_size = 2048;
            break;
        default:
            QM_ERR_RETURN(BDMF_ERR_PARM, "drv_qm_fpm_min_pool_get() illegal fpm_prefetch_min_pool_size=%d\n",
                fpm_ctrl_cfg.fpm_prefetch_min_pool_size);
        }
    }
    return rc;
}

bdmf_error_t drv_qm_exit(void)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

/* Configure queue */
bdmf_error_t drv_qm_queue_config(rdp_qm_queue_idx_t q_idx, const qm_q_context *cfg)
{
    bdmf_error_t err = BDMF_ERR_OK;

    /* Setup queue profile */
    err = ag_drv_qm_q_context_set(q_idx, cfg);

    return err;
}

/* Get Configure queue */
bdmf_error_t drv_qm_queue_get_config(rdp_qm_queue_idx_t q_idx, qm_q_context *cfg)
{
    bdmf_error_t err = BDMF_ERR_OK;

    /* Setup queue profile */
    err = ag_drv_qm_q_context_get(q_idx, cfg);

    return err;
}

/* Flush and disable queue */
bdmf_error_t drv_qm_queue_disable(rdp_qm_queue_idx_t q_idx, bdmf_boolean flush)
{
    bdmf_error_t err;

    /* Disable in DQM */
    err = ag_drv_dqm_dqmol_cfgb_set(q_idx, 0);

    /* Flush if necessary */
    if (flush)
    {
        /* ToDo: */
    }

    return err;
}

/* Enable queue. Must be configured first */
bdmf_error_t drv_qm_queue_enable(rdp_qm_queue_idx_t q_idx)
{
    bdmf_error_t rc;

    rc = ag_drv_dqm_dqmol_cfgb_set(q_idx, 1);

    return rc;
}

bdmf_error_t force_copy_ddr_on_queue(rdp_qm_queue_idx_t q_idx, bdmf_boolean copy_to_ddr)
{
    qm_q_context q_cfg = {};
    bdmf_error_t rc = drv_qm_queue_disable(q_idx, 1);

    rc = rc ? rc : drv_qm_queue_get_config(q_idx, &q_cfg);

    q_cfg.copy_to_ddr = copy_to_ddr;

    /* If copy to DDR set, reset ddr_copy_disable*/
    q_cfg.ddr_copy_disable = copy_to_ddr ? 0 : q_cfg.ddr_copy_disable;
    rc = rc ? rc : drv_qm_queue_config(q_idx, &q_cfg);
    rc = rc ? rc : drv_qm_queue_enable(q_idx);

    return rc;
}


int drv_qm_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val)
{
     qm_clk_gate_clk_gate_cntrl qm_ctrl;

     ag_drv_qm_clk_gate_clk_gate_cntrl_get(&qm_ctrl);
     qm_ctrl.bypass_clk_gate = auto_gate ? 0 : 1;
     qm_ctrl.timer_val = timer_val;
     ag_drv_qm_clk_gate_clk_gate_cntrl_set(&qm_ctrl);
     return 0;
}

/***************************************************************
 *  drv_qm_drop_stat_get - get QM statistics from HW
 *  INPUT PARAMS:
 *      qm_drop_stat - pointer to structure of QM drop statistics
 ***********************************************************/
int drv_qm_drop_stat_get(drv_qm_drop_cntr_t *qm_drop_stat)
{
    int rc, i;

    memset(qm_drop_stat, 0, sizeof(drv_qm_drop_cntr_t));
    rc = ag_drv_qm_wred_drop_cnt_get(&qm_drop_stat->qm_wred_drop);

    for (i = 0; i < NUM_OF_FPM_UG && !rc; i++)
        rc = ag_drv_qm_fpm_grp_drop_cnt_get(i, &qm_drop_stat->qm_fpm_grp_drop[i]);

    for (i = 0; i < NUM_OF_FPM_POOLS && !rc; i++)
        rc = rc ? rc : ag_drv_qm_fpm_pool_drop_cnt_get(i, &qm_drop_stat->qm_fpm_prefetch_drop[i]);

    rc = rc ? rc : ag_drv_qm_fpm_congestion_drop_cnt_get(&qm_drop_stat->qm_fpm_cong_drop);
    rc = rc ? rc : ag_drv_qm_ddr_pd_congestion_drop_cnt_get(&qm_drop_stat->qm_ddr_pd_cong_drop);
    rc = rc ? rc : ag_drv_qm_qm_pd_congestion_drop_cnt_get(&qm_drop_stat->qm_pd_cong_drop);
    rc = rc ? rc : ag_drv_qm_psram_egress_cong_drp_cnt_get(&qm_drop_stat->qm_psram_egress_cong_drop);
    return rc;
}

bdmf_error_t drv_qm_fpm_buffer_reservation_profile_cfg(uint8_t profile_id, uint16_t token_threshold)
{
    bdmf_error_t err;
    
#ifdef BCM6856
    /* for 6856 there is a 16 bit dedicated register for each profile threshold */
    /* but only bits [11:4] are used. verify threshold is not lower than min resolution*/
    if ((token_threshold != 0) & (token_threshold < QM_MBR_PROFILE_RESOLUTION))
        token_threshold = QM_MBR_PROFILE_RESOLUTION;
    
    err = ag_drv_qm_fpm_buffer_reservation_data_set(profile_id, token_threshold);
#else
    /* for all platforms but 6856 there are 2 registers, 8bit per profile. starting from profile 0 to 7*/
    uint8_t reg_idx = profile_id / 4; /* register address offset */
    uint8_t int_idx = profile_id & 0x3; /* internal location profile_id%4 */
    uint8_t res_thr[4];
    uint32_t range_check;

    err = ag_drv_qm_global_cfg_qm_fpm_buffer_grp_res_get(reg_idx, &res_thr[0], &res_thr[1], &res_thr[2], &res_thr[3]);
    
    /* calculate KB threshold. We can't take a worst case of single PD per token*/
    range_check = ((uint32_t)token_threshold) * p_dpi_cfg->fpm_buf_size / RDD_MBR_BUFFER_SIZE_QUANTUM;
    
    /* Test uint_8 compliance */
    if (range_check >= (1U<<8))
    {
        BDMF_TRACE_ERR("%d packet buffers cannot be configured with packet_buffer_size=%d. Total reserved must be lower then 128Kb\n",
            token_threshold, p_dpi_cfg->fpm_buf_size);
        return BDMF_ERR_RANGE;
    }
    res_thr[int_idx] = (uint8_t)range_check;
    
    err = err ? err : ag_drv_qm_global_cfg_qm_fpm_buffer_grp_res_set(reg_idx, res_thr[0], res_thr[1], res_thr[2], res_thr[3]);
#endif
    
    return err;
}

#if defined(BCM6856)
static bdmf_error_t drv_qm_fpm_buffer_global_res_set(bdmf_boolean enable)
{
    int rc;
    qm_drop_counters_ctrl drop_ctrl;
    
    rc = ag_drv_qm_drop_counters_ctrl_get(&drop_ctrl);
    drop_ctrl.fpm_buffer_global_res_enable = enable;
    rc = rc ? rc : ag_drv_qm_drop_counters_ctrl_set(&drop_ctrl);
    
    return rc;
}
#endif

/* This function can be called after UG thresholds were modified by minimum buffer reservation */
bdmf_error_t set_fpm_budget(int resource_num, int add, uint32_t reserved_packet_buffer)
{
#if defined(CONFIG_DHD_RUNNER) && !defined(_CFE_)    
    static int xepon_port_on = 0;
    uint32_t low_prio_budget, high_prio_budget, excl_prio_budget;
    uint16_t fpm_token_size;
#endif
    static int num_of_wlan_radios = 0;
    int rc;
    int dhd_rx_post_fpm = 0;
    unsigned int fpm_thr[NUM_OF_FPM_UG];
    qm_fpm_ug_thr fpm_ug_thr;

#if defined(BCM6856)
    /* This section is not necessary here but we can keep it in case queue minimum buffer allocation will
       once again auto set the UG thresholds */
    static uint32_t us_mbr = 0, ds_mbr = 0;
    if (resource_num == FPM_RES_MBR)
    {
        /* In case minimum buffer reservation is allocated just save the numbers add is used as direction*/
        if (add)
            us_mbr = reserved_packet_buffer;
        else
            ds_mbr = reserved_packet_buffer;
        
        rc = drv_qm_fpm_buffer_global_res_set((ds_mbr != 0) || (us_mbr != 0));
    }
#else
    if (resource_num == FPM_RES_MBR)
    {
        return 0; /* No use for MBR besides 6856 */
    }
#endif

    if (resource_num == FPM_RES_WLAN)
    {
        if (add)
          num_of_wlan_radios++;
        else
          num_of_wlan_radios--;
    }

#if defined(CONFIG_DHD_RUNNER) && !defined(_CFE_)
    BUG_ON(num_of_wlan_radios < 0);

    if (resource_num == FPM_RES_XEPON)
    {
        if (add)
          xepon_port_on = 1;
        else
          xepon_port_on = 0;
    }   
    rc = drv_qm_fpm_min_pool_get(&fpm_token_size);
    if (rc)
        return rc;
    fpm_token_size >>= 8;
    /* number of 256 byte fpm = 8*(DHD_RX_POST_FLOW_RING_SIZE-1), dhd_rx_post_fpm(fpm_token_size_asr_8) is a multiples of 256 */
    dhd_rx_post_fpm = (num_of_wlan_radios*8*(DHD_RX_POST_FLOW_RING_SIZE-1))/fpm_token_size;
    
#endif

    /* User defined token allocation. US test only since both US and DS were tested for non zero allocation at configuration */
    if (p_dpi_cfg->us_fpm_tokens_allocation.total_fpm_tokens)
    {
        fpm_thr[FPM_DS_UG] = p_dpi_cfg->ds_fpm_tokens_allocation.total_fpm_tokens;
        fpm_thr[FPM_US_UG] = p_dpi_cfg->us_fpm_tokens_allocation.total_fpm_tokens;
        BUG_ON(p_dpi_cfg->wlan_fpm_tokens_allocation.total_fpm_tokens < dhd_rx_post_fpm);
        fpm_thr[FPM_WLAN_UG] = p_dpi_cfg->wlan_fpm_tokens_allocation.total_fpm_tokens - dhd_rx_post_fpm;
    }
    else /* Default configuration will apply */
    {
        fpm_thr[FPM_DS_UG] = ((TOTAL_DYNAMIC_FPM - fpm_get_dqm_extra_fpm_tokens())/100) * DS_FPM_UG_DEFAULT_PERCENTAGE;
        fpm_thr[FPM_US_UG] = ((TOTAL_DYNAMIC_FPM - fpm_get_dqm_extra_fpm_tokens())/100) * US_FPM_UG_DEFAULT_PERCENTAGE;
        fpm_thr[FPM_WLAN_UG] = ((TOTAL_DYNAMIC_FPM - fpm_get_dqm_extra_fpm_tokens())/100) * WLAN_FPM_UG_DEFAULT_PERCENTAGE;

#if defined(CONFIG_DHD_RUNNER) && !defined(_CFE_)
        /* in case WLAN default configuration will apply */
        if (num_of_wlan_radios)
        {
            if (xepon_port_on)
            {
                fpm_thr[FPM_DS_UG] = ((TOTAL_DYNAMIC_FPM - fpm_get_dqm_extra_fpm_tokens())/100) * DS_FPM_UG_XEPON_PERCENTAGE;
                fpm_thr[FPM_US_UG] = ((TOTAL_DYNAMIC_FPM - fpm_get_dqm_extra_fpm_tokens())/100) * US_FPM_UG_XEPON_PERCENTAGE;
                BUG_ON((((TOTAL_DYNAMIC_FPM - fpm_get_dqm_extra_fpm_tokens())/100) * WLAN_FPM_UG_XEPON_PERCENTAGE) < dhd_rx_post_fpm);
                fpm_thr[FPM_WLAN_UG] = (((TOTAL_DYNAMIC_FPM - fpm_get_dqm_extra_fpm_tokens())/100) * WLAN_FPM_UG_XEPON_PERCENTAGE) - dhd_rx_post_fpm;
            }
            else
            {
                fpm_thr[FPM_DS_UG] = ((TOTAL_DYNAMIC_FPM - fpm_get_dqm_extra_fpm_tokens())/100) * DS_FPM_UG_NO_XEPON_PERCENTAGE;
                fpm_thr[FPM_US_UG] = ((TOTAL_DYNAMIC_FPM - fpm_get_dqm_extra_fpm_tokens())/100) * US_FPM_UG_NO_XEPON_PERCENTAGE;
                BUG_ON((((TOTAL_DYNAMIC_FPM - fpm_get_dqm_extra_fpm_tokens())/100) * WLAN_FPM_UG_NO_XEPON_PERCENTAGE) < dhd_rx_post_fpm);
                fpm_thr[FPM_WLAN_UG] = (((TOTAL_DYNAMIC_FPM - fpm_get_dqm_extra_fpm_tokens())/100) * WLAN_FPM_UG_NO_XEPON_PERCENTAGE) - dhd_rx_post_fpm;
            }
        }
#endif
    }

    /* Update DS UG thresholds */
#if defined(BCM6856)
    fpm_ug_thr.higher_thr = fpm_thr[FPM_DS_UG] - (fpm_thr[FPM_DS_UG] * p_dpi_cfg->ds_fpm_tokens_allocation.excl_prio_rsv_percent) / 100;
    /* check if total FPM allocated for all queues exceeds the global allocation */
    if (ds_mbr > (fpm_thr[FPM_DS_UG] * p_dpi_cfg->ds_fpm_tokens_allocation.excl_prio_rsv_percent) / 100)
    {
        BDMF_TRACE_ERR("Not enough packet buffers for DS minimum buffer reservation. DS allocation=%d Total queues requirement=%d\n",
            ((fpm_thr[FPM_DS_UG] * p_dpi_cfg->ds_fpm_tokens_allocation.excl_prio_rsv_percent) / 100), ds_mbr);
        return BDMF_ERR_RANGE;
    }
#else
    fpm_ug_thr.higher_thr = fpm_thr[FPM_DS_UG];
#endif
    fpm_ug_thr.mid_thr = fpm_thr[FPM_DS_UG] - (fpm_thr[FPM_DS_UG] * p_dpi_cfg->ds_fpm_tokens_allocation.excl_prio_rsv_percent) / 100;
    /* Certain percent from total allocation are reserved for high priority traffic */
    fpm_ug_thr.lower_thr = fpm_ug_thr.mid_thr - (fpm_thr[FPM_DS_UG] * p_dpi_cfg->ds_fpm_tokens_allocation.high_prio_rsv_percent) / 100;
    /* Check for wrap around issues */ 
    if ((fpm_ug_thr.lower_thr > fpm_ug_thr.higher_thr) || (fpm_ug_thr.lower_thr == 0))
    {
        BDMF_TRACE_ERR("Illegal lower UG %d thresholds high=%d med=%d low=%d\n",
            FPM_DS_UG, fpm_ug_thr.higher_thr, fpm_ug_thr.mid_thr, fpm_ug_thr.lower_thr);
        return BDMF_ERR_PARM;
    }
    
    rc = ag_drv_qm_fpm_ug_thr_set(FPM_DS_UG, &fpm_ug_thr);
    if (rc)
        return rc;
    
    /* Update US UG thresholds */
#if defined(BCM6856)
    fpm_ug_thr.higher_thr = fpm_thr[FPM_US_UG] - (fpm_thr[FPM_US_UG] * p_dpi_cfg->us_fpm_tokens_allocation.excl_prio_rsv_percent) / 100;
    /* check if total FPM allocated for all queues exceeds the global allocation */
    if (us_mbr > (fpm_thr[FPM_US_UG] * p_dpi_cfg->us_fpm_tokens_allocation.excl_prio_rsv_percent) / 100)
    {
        BDMF_TRACE_ERR("Not enough packet buffers for US minimum buffer reservation. US allocation=%d Total queues requirement=%d\n",
            ((fpm_thr[FPM_DS_UG] * p_dpi_cfg->ds_fpm_tokens_allocation.excl_prio_rsv_percent) / 100), us_mbr);
        return BDMF_ERR_RANGE;
    }
#else
    fpm_ug_thr.higher_thr = fpm_thr[FPM_US_UG];
#endif
    fpm_ug_thr.mid_thr = fpm_thr[FPM_US_UG] - (fpm_thr[FPM_US_UG] * p_dpi_cfg->us_fpm_tokens_allocation.excl_prio_rsv_percent) / 100;
    /* Certain percent from total allocation are reserved for high priority traffic */
    fpm_ug_thr.lower_thr = fpm_ug_thr.mid_thr - (fpm_thr[FPM_US_UG] * p_dpi_cfg->us_fpm_tokens_allocation.high_prio_rsv_percent) / 100;
    /* Check for wrap around issues */ 
    if ((fpm_ug_thr.lower_thr > fpm_ug_thr.higher_thr) || (fpm_ug_thr.lower_thr == 0))
    {
        BDMF_TRACE_ERR("Illegal UG %d thresholds high=%d med=%d low=%d\n",
            FPM_US_UG, fpm_ug_thr.higher_thr, fpm_ug_thr.mid_thr, fpm_ug_thr.lower_thr);
        return BDMF_ERR_PARM;
    }
    rc = ag_drv_qm_fpm_ug_thr_set(FPM_US_UG, &fpm_ug_thr);
    if (rc)
        return rc;
    
    /* Update WLAN UG thresholds */
    fpm_ug_thr.higher_thr = fpm_thr[FPM_WLAN_UG];
    fpm_ug_thr.mid_thr = fpm_ug_thr.higher_thr;
    fpm_ug_thr.lower_thr = fpm_ug_thr.higher_thr;
    rc = ag_drv_qm_fpm_ug_thr_set(FPM_WLAN_UG, &fpm_ug_thr);
    if (rc)
        return rc;
    
    /* last group is temporary all pass - does not change*/
    /* All pass group - for CPU and WLAN in short term solution to exploit the UG counters. Can be moved to UG 7*/
    fpm_ug_thr.higher_thr = (TOTAL_FPM_TOKENS-1);
    fpm_ug_thr.mid_thr    = (TOTAL_FPM_TOKENS-1);
    fpm_ug_thr.lower_thr  = (TOTAL_FPM_TOKENS-1);
    rc = ag_drv_qm_fpm_ug_thr_set(FPM_ALL_PASS_UG, &fpm_ug_thr);
	
#if defined(CONFIG_DHD_RUNNER) && !defined(_CFE_)
    if ((resource_num == FPM_RES_WLAN) || (resource_num == FPM_RES_XEPON) || ((resource_num == FPM_RES_NONE) && (num_of_wlan_radios)))
    {
        /* set budget fpm */
        excl_prio_budget = fpm_thr[FPM_WLAN_UG];
        high_prio_budget = excl_prio_budget - (fpm_thr[FPM_WLAN_UG] * p_dpi_cfg->wlan_fpm_tokens_allocation.excl_prio_rsv_percent) / 100;
        low_prio_budget = high_prio_budget - (fpm_thr[FPM_WLAN_UG] * p_dpi_cfg->wlan_fpm_tokens_allocation.high_prio_rsv_percent) / 100;
        
        rc = rc ? rc : rdd_dhd_helper_fpm_thresholds_set(low_prio_budget, high_prio_budget, excl_prio_budget);

        bdmf_trace("FPM group configuration set:  DS = %u, US = %u, WLAN_Tx = (low:%u, high:%u, excl:%u) for %d radios\n", 
            fpm_thr[FPM_DS_UG], fpm_thr[FPM_US_UG], low_prio_budget, high_prio_budget, excl_prio_budget, num_of_wlan_radios);
    }

#endif
    return rc;
}


bdmf_boolean is_qm_queue_aggregation_context_valid(uint16_t qm_queue)
{
    uint32_t reg_idx, bit_idx, context_valid;
    int rc = 0;

    reg_idx = qm_queue / 32;
    bit_idx = qm_queue % 32;
    rc = ag_drv_qm_aggr_context_get(reg_idx, &context_valid);
    if (rc)
    {
        BDMF_TRACE_ERR("ag_drv_qm_aggr_context_get returned %d for reg_idx=%d\n", rc, reg_idx);
        return 0;
    }
    if (context_valid & (1 << bit_idx))
        return 1;
    
    return 0;
}

#ifdef USE_BDMF_SHELL

/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/

static bdmfmon_handle_t qm_dir;

/* "init" handler
 * BDMFMON_MAKE_PARM( "fpm_base", "FPM_BASE", BDMFMON_PARM_HEX, 0),
 * BDMFMON_MAKE_PARM_RANGE( "ddr_sop0", "DDR SoP 0", BDMFMON_PARM_NUMBER, 0, 0, 255),
 * BDMFMON_MAKE_PARM_RANGE( "ddr_sop1", "DDR SoP 1", BDMFMON_PARM_NUMBER, 0, 0, 255),
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "clear_on_read", "Clear counters on read", bdmfmon_enum_bool_table, 0, "no"),
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "wrap_around", "Counters can wrap around", bdmfmon_enum_bool_table, 0, "yes") );
 */
static int _qm_cli_init_handler(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    qm_init_cfg cfg = {};

    cfg.fpm_base = parm[0].value.unumber;
    cfg.ddr_sop_offset[0] = parm[1].value.unumber;
    cfg.ddr_sop_offset[1] = parm[2].value.unumber;
    cfg.is_counters_read_clear = (bdmf_boolean)parm[3].value.number;
    cfg.is_drop_counters_enable = (bdmf_boolean)parm[4].value.number;

    return drv_qm_init(&cfg);
}

/* "qcfg" handler
 * BDMFMON_MAKE_PARM_RANGE( "idx", "Queue index", BDMFMON_PARM_NUMBER, 0, 0, 287),
 * BDMFMON_MAKE_PARM_RANGE( "wred", "WRED profile", BDMFMON_PARM_NUMBER, 0, 0, 15),
 * BDMFMON_MAKE_PARM_RANGE( "ug", "UG profile", BDMFMON_PARM_NUMBER, 0, 0, 3),
 * BDMFMON_MAKE_PARM_RANGE( "cd", "CD profile", BDMFMON_PARM_NUMBER, 0, 0, 7),
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "disable_copy", "Disable copy to DDR", bdmfmon_enum_bool_table, 0, "no"),
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "force_copy", "Force copy to DDR", bdmfmon_enum_bool_table, 0, "no"),
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "disable_aggregation", "Disable aggregation", bdmfmon_enum_bool_table, 0, "no"),
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "exclusive", "Exclusive priority", bdmfmon_enum_bool_table, 0, "no"),
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "802_1ae", "802_1ae support", bdmfmon_enum_bool_table, 0, "no"),
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "sci", "SCI support", bdmfmon_enum_bool_table, 0, "no"),
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "fec", "Enable FEC", bdmfmon_enum_bool_table, 0, "no"),
 * BDMFMON_MAKE_PARM_RANGE( "res_profile", "Min Buffer Reservation profile", 0, 0, 7));
 */
static int _qm_cli_qcfg_handler(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    qm_q_context cfg = {};
    uint16_t q_idx = (uint16_t)parm[0].value.number;
    cfg.wred_profile = parm[1].value.number;
    cfg.fpm_ug = parm[2].value.number;
    cfg.copy_dec_profile = parm[3].value.number;
    cfg.ddr_copy_disable = (bdmf_boolean)parm[4].value.number;
    cfg.copy_to_ddr = (bdmf_boolean)parm[5].value.number;
    cfg.aggregation_disable = (bdmf_boolean)parm[6].value.number;
    cfg.exclusive_priority = (bdmf_boolean)parm[7].value.number;
    cfg.q_802_1ae = (bdmf_boolean)parm[8].value.number;
    cfg.sci = (bdmf_boolean)parm[9].value.number;
    cfg.fec_enable = (bdmf_boolean)parm[10].value.number;
    cfg.res_profile = (bdmf_boolean)parm[11].value.number;

    return drv_qm_queue_config(q_idx, &cfg);
}


/* "qcfg_g_helper" function handler
 *  this function read modify write the Queues of start to end address
 *  makeaction is bit array to decide which action to do 
 *  QM_CNFG_G_DISABLE_COPY_TO_DDR_S 0
 *  QM_CNFG_G_FORCE_COPY_TO_DDR_S   1
 *  QM_CNFG_G_DISABLE_AGGREGATION_S 2
 *
*/
static int _qm_cli_qcfg_g_helper(uint16_t startAddr, uint16_t endAddr, bdmf_boolean disableCopy, bdmf_boolean forceCopy, bdmf_boolean aggregation_disable, const uint16_t makeaction)
{
    bdmf_error_t err = BDMF_ERR_OK;
    qm_q_context cfg = {};
    uint16_t q_idx = 0;
    for (q_idx = startAddr; q_idx <= endAddr; q_idx++)
    {
        err = drv_qm_queue_get_config(q_idx, &cfg);
        if (err)
            return err;
        
        cfg.ddr_copy_disable = (makeaction & (1<<QM_CNFG_G_DISABLE_COPY_TO_DDR_S))? disableCopy:cfg.ddr_copy_disable;
        cfg.copy_to_ddr = (makeaction & (1<<QM_CNFG_G_FORCE_COPY_TO_DDR_S))? forceCopy:cfg.copy_to_ddr;
        cfg.aggregation_disable = (makeaction & (1<<QM_CNFG_G_DISABLE_AGGREGATION_S))? aggregation_disable:cfg.aggregation_disable;
        err = drv_qm_queue_config(q_idx, &cfg);
        if (err)
            return err;
    }
    return BDMF_ERR_OK;
}

/* "qcfg_g_disable_aggregation" handler
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "US", "up stream queues", bdmfmon_enum_bool_table, 0, "no"),
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "DS", "dp stream queues", bdmfmon_enum_bool_table, 0, "no"),
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "disable_aggregation", "Disable aggregation", bdmfmon_enum_bool_table, 0, "no"));
*/
static int _qm_cli_qcfg_g_disable_aggregation_handler(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_boolean us = (bdmf_boolean)parm[0].value.number;
    bdmf_boolean ds = (bdmf_boolean)parm[1].value.number;
    bdmf_error_t err = BDMF_ERR_OK ;
    if (us)
    {
        err = _qm_cli_qcfg_g_helper(drv_qm_get_us_start(), drv_qm_get_us_end(), 0,0, (bdmf_boolean)parm[2].value.number, 1<<QM_CNFG_G_DISABLE_AGGREGATION_S);
        if (err)
            return err;
    }

    if (ds)
    {
        err = _qm_cli_qcfg_g_helper(drv_qm_get_ds_start(), drv_qm_get_ds_end(), 0, 0, (bdmf_boolean)parm[2].value.number, 1<<QM_CNFG_G_DISABLE_AGGREGATION_S);
        if (err)
            return err;
    }
    return BDMF_ERR_OK;
}

/* "qcfg_g_copy_to_ddr" handler
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "US", "up stream queues", bdmfmon_enum_bool_table, 0, "no"),
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "DS", "dp stream queues", bdmfmon_enum_bool_table, 0, "no"),
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "disable_copy", "Disable copy to DDR", bdmfmon_enum_bool_table, 0, "no"),
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "force_copy", "Force copy to DDR", bdmfmon_enum_bool_table, 0, "no"),
*/
static int _qm_cli_qcfg_g_copy_to_ddr_handler(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{

    bdmf_boolean us = (bdmf_boolean)parm[0].value.number;
    bdmf_boolean ds = (bdmf_boolean)parm[1].value.number;
    bdmf_error_t err = BDMF_ERR_OK ;
    if (us)
    {
        err = _qm_cli_qcfg_g_helper(drv_qm_get_us_start(), drv_qm_get_us_end(), (bdmf_boolean)parm[2].value.number, (bdmf_boolean)parm[3].value.number, 0, ((1<< QM_CNFG_G_DISABLE_COPY_TO_DDR_S) | (1<< QM_CNFG_G_FORCE_COPY_TO_DDR_S)));
        if (err)
            return err;
    }
    
    if (ds)
    {
        err = _qm_cli_qcfg_g_helper(drv_qm_get_ds_start(), drv_qm_get_ds_end(), (bdmf_boolean)parm[2].value.number, (bdmf_boolean)parm[3].value.number, 0, ((1<< QM_CNFG_G_DISABLE_COPY_TO_DDR_S) | (1<< QM_CNFG_G_FORCE_COPY_TO_DDR_S)));
        if (err)
            return err;
    }
    return BDMF_ERR_OK;
}

/* "qdisable" handler
 * BDMFMON_MAKE_PARM_RANGE( "idx", "Queue index", BDMFMON_PARM_NUMBER, 0, 0, 287),
 * BDMFMON_MAKE_PARM_ENUM( "flush", "Disable copy to DDR", bdmfmon_enum_bool_table, 0));
 */
static int _qm_cli_qdisable_handler(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint16_t q_idx = (uint16_t)parm[0].value.number;
    bdmf_boolean flush = (bdmf_boolean)parm[1].value.number;
    return drv_qm_queue_disable(q_idx, flush);
}

static int _drv_qm_cli_wred_profile_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    static uint32_t wred_pofile[] = {cli_qm_wred_profile_cfg};

    /* get qm wred profile cfg */
    bdmf_session_print(session, "\nQM wred profile configurations:\n");
    HAL_CLI_PRINT_NUM_OF_LIST(session, qm, wred_pofile, QM_MAX_NUM_OF_WRED_PROFILES);

    return BDMF_ERR_OK;
}

static int _drv_qm_cli_queue_cfg_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    static uint32_t queue_cfg[] = {cli_qm_q_context};

    /* get queue cfg */
    bdmf_session_print(session, "\nQM queue configurations:\n");
    HAL_CLI_PRINT_NUM_OF_LIST(session, qm, queue_cfg, QM_NUM_QUEUES);

    return BDMF_ERR_OK;
}

static int _drv_qm_cli_copy_decision_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    static uint32_t copy_profile[] = {cli_qm_copy_decision_profile};

    /* get copy decision profile */
    bdmf_session_print(session, "\nQM copy decision profile configurations:\n");
    HAL_CLI_PRINT_NUM_OF_LIST(session, qm, copy_profile, QM_MAX_COPY_DECISION_PROFILE);

    return BDMF_ERR_OK;
}

static int _drv_qm_cli_rnr_grp_cfg_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    static uint32_t rnr_grp[] = {cli_qm_rnr_group_cfg};

    /* get qm rnr grp cfg */
    bdmf_session_print(session, "\nQM runner group configurations:\n");
    HAL_CLI_PRINT_NUM_OF_LIST(session, qm, rnr_grp, qm_rnr_group_num);

    return BDMF_ERR_OK;
}

static int _drv_qm_cli_print_not_empty_queues(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t idx;
    uint32_t packets = 0;
    uint32_t bytes = 0;
    uint32_t open_aggr;
    bdmf_error_t rc = BDMF_ERR_OK ;
    bdmf_session_print(session, "\nprint non empty queues:\n");
    for (idx = 0; idx < QM_NUM_QUEUES; idx++)
    {
        open_aggr = 0;
        if (is_qm_queue_aggregation_context_valid(idx))
        {
            open_aggr = 1;
            BDMF_TRACE_DBG("NON ZERO AGG CONTEXT - for queue %d\n", idx);
        }

#ifdef BCM6856
        rc = rc ? rc : ag_drv_qm_total_valid_cnt_get(((int)idx * 4), &(packets));
        rc = rc ? rc : ag_drv_qm_total_valid_cnt_get(((int)idx * 4) + 1, &(bytes));
#else
        rc = rc ? rc : ag_drv_qm_total_valid_cnt_get(((int)idx * 2), &(packets));
        rc = rc ? rc : ag_drv_qm_total_valid_cnt_get(((int)idx * 2) + 1, &(bytes));
#endif
        packets += open_aggr;
        if (packets || bytes)
        {
            bdmf_session_print(session, "index=%d,packet=%d,bytes=%d:\n", idx,packets,bytes);
        }
    }
    return rc;
}

int drv_qm_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    static uint32_t qm_cfg[] = {cli_qm_enable_ctrl, cli_qm_drop_counters_ctrl, cli_qm_global_cfg_aggregation_ctrl, cli_qm_ddr_sop_offset,
                                cli_qm_epon_overhead_ctrl, cli_qm_ddr_cong_ctrl, cli_qm_qm_pd_cong_ctrl, cli_qm_global_cfg_abs_drop_queue,
                                cli_qm_global_cfg_aggregation_ctrl};
    static uint32_t fpm_ug_cfg[] = {cli_qm_fpm_ug_thr, cli_qm_fpm_pool_thr};
    static uint32_t fpm_cfg[] = {cli_qm_fpm_base_addr, cli_qm_fpm_ctrl, cli_qm_global_cfg_fpm_coherent_base_addr};

    bdmf_boolean get_all_cfg = (bdmf_boolean)parm[0].value.number;

    /* get qm global cfg */
    bdmf_session_print(session, "\nQM global configurations:\n");
    HAL_CLI_PRINT_LIST(session, qm, qm_cfg);

    /* get fpm cfg */
    bdmf_session_print(session, "\nQM fpm configurations:\n");
    HAL_CLI_PRINT_NUM_OF_LIST(session, qm, fpm_ug_cfg, NUM_OF_FPM_UG);
    HAL_CLI_PRINT_LIST(session, qm, fpm_cfg);

    if (get_all_cfg)
    {
        _drv_qm_cli_wred_profile_get(session, parm, n_parms);
        _drv_qm_cli_queue_cfg_get(session, parm, n_parms);
        _drv_qm_cli_copy_decision_get(session, parm, n_parms);
        _drv_qm_cli_rnr_grp_cfg_get(session, parm, n_parms);
    }

    return 0;
}

int drv_qm_cli_debug_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    static uint32_t qm_debug[] = {cli_qm_debug_counters, cli_qm_debug_info, cli_qm_good_lvl1_pkts_cnt, cli_qm_good_lvl1_bytes_cnt, cli_qm_good_lvl2_pkts_cnt,
                                  cli_qm_good_lvl2_bytes_cnt, cli_qm_copied_pkts_cnt, cli_qm_copied_bytes_cnt, cli_qm_agg_pkts_cnt, cli_qm_agg_bytes_cnt,
                                  cli_qm_agg_1_pkts_cnt, cli_qm_agg_2_pkts_cnt, cli_qm_agg_3_pkts_cnt, cli_qm_agg_4_pkts_cnt,cli_qm_wred_drop_cnt,
                                  cli_qm_fpm_pool_drop_cnt, cli_qm_fpm_congestion_drop_cnt, cli_qm_fpm_grp_drop_cnt, cli_qm_ddr_pd_congestion_drop_cnt,
                                  cli_qm_ddr_byte_congestion_drop_cnt, cli_qm_qm_pd_congestion_drop_cnt, cli_qm_qm_abs_requeue_cnt, cli_qm_ingress_stat, 
                                  cli_qm_egress_stat, cli_qm_cm_stat, cli_qm_fpm_prefetch_stat, cli_qm_qm_connect_ack_counter, cli_qm_qm_ddr_wr_reply_counter,
                                  cli_qm_qm_ddr_pipe_byte_counter, cli_qm_qm_abs_requeue_valid_counter};

    /* get qm debug */
    bdmf_session_print(session, "\nQM debug information:\n");
    HAL_CLI_PRINT_LIST(session, qm, qm_debug);

    return drv_qm_cli_sanity_check(session, parm, n_parms);
}

int drv_qm_cli_sanity_check(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    qm_intr_ctrl_isr intr_ctrl_isr = {};
    int rc;

    rc = ag_drv_qm_intr_ctrl_isr_get(&intr_ctrl_isr);

    if (!rc)
    {
        if (intr_ctrl_isr.qm_dqm_pop_on_empty)
            bdmf_session_print(session, "QM: qm_dqm_pop_on_empty\n");
        if (intr_ctrl_isr.qm_dqm_push_on_full)
            bdmf_session_print(session, "QM: qm_dqm_push_on_full\n");
        if (intr_ctrl_isr.qm_cpu_pop_on_empty)
            bdmf_session_print(session, "QM: qm_cpu_pop_on_empty\n");
        if (intr_ctrl_isr.qm_cpu_push_on_full)
            bdmf_session_print(session, "QM: qm_cpu_push_on_full\n");
        if (intr_ctrl_isr.qm_normal_queue_pd_no_credit)
            bdmf_session_print(session, "QM: qm_normal_queue_pd_no_credit\n");
#if !defined(BCM6846) && !defined(BCM6856)
        if (intr_ctrl_isr.qm_non_delayed_queue_pd_no_cre)
            bdmf_session_print(session, "QM: qm_non_delayed_queue_pd_no_cre\n");
#if !defined(XRDP)        
/* temporary disabled as shows up on QM copy descriptors which is normal*/
        if (intr_ctrl_isr.qm_target_mem_abs_contradictio)
            bdmf_session_print(session, "QM: qm_target_mem_abs_contradictio\n");
#endif            
        if (intr_ctrl_isr.qm_1588_multicast_contradictio)
            bdmf_session_print(session, "QM: qm_1588_multicast_contradictio\n");
#endif
        if (intr_ctrl_isr.qm_agg_coherent_inconsistency)
            bdmf_session_print(session, "QM: qm_agg_coherent_inconsistency\n");
        if (intr_ctrl_isr.qm_force_copy_on_non_delayed)
            bdmf_session_print(session, "QM: qm_force_copy_on_non_delayed\n");
        if (intr_ctrl_isr.qm_fpm_pool_size_nonexistent)
            bdmf_session_print(session, "QM: qm_fpm_pool_size_nonexistent\n");

        if (intr_ctrl_isr.qm_byte_drop_cnt_overrun)
            bdmf_session_print(session, "QM: qm_byte_drop_cnt_overrun\n");
        if (intr_ctrl_isr.qm_pkt_drop_cnt_overrun)
            bdmf_session_print(session, "QM: qm_pkt_drop_cnt_overrun\n");
        if (intr_ctrl_isr.qm_total_byte_cnt_underrun)
            bdmf_session_print(session, "QM: qm_total_byte_cnt_underrun\n");
        if (intr_ctrl_isr.qm_total_pkt_cnt_underrun)
            bdmf_session_print(session, "QM: qm_total_pkt_cnt_underrun\n");
        if (intr_ctrl_isr.qm_fpm_ug0_underrun)
            bdmf_session_print(session, "QM: qm_fpm_ug0_underrun\n");
        if (intr_ctrl_isr.qm_fpm_ug1_underrun)
            bdmf_session_print(session, "QM: qm_fpm_ug1_underrun\n");
        if (intr_ctrl_isr.qm_fpm_ug2_underrun)
            bdmf_session_print(session, "QM: qm_fpm_ug2_underrun\n");
        if (intr_ctrl_isr.qm_fpm_ug3_underrun)
            bdmf_session_print(session, "QM: qm_fpm_ug3_underrun\n");
    }

    return rc;
}

void drv_qm_cli_init(bdmfmon_handle_t driver_dir)
{
    ag_drv_qm_cli_init(driver_dir);

    if (!(qm_dir = bdmfmon_dir_find(driver_dir, "qm")))
        return;

    BDMFMON_MAKE_CMD_NOPARM(qm_dir, "wred", "wred profile configuration", _drv_qm_cli_wred_profile_get);
    BDMFMON_MAKE_CMD_NOPARM(qm_dir, "queue", "queue profile configuration", _drv_qm_cli_queue_cfg_get);
    BDMFMON_MAKE_CMD_NOPARM(qm_dir, "copy", "copy decision profile configuration", _drv_qm_cli_copy_decision_get);
    BDMFMON_MAKE_CMD_NOPARM(qm_dir, "rg", "runner group configuration", _drv_qm_cli_rnr_grp_cfg_get);
    BDMFMON_MAKE_CMD(qm_dir, "cfg_get", "qm configurations", (bdmfmon_cmd_cb_t)drv_qm_cli_config_get,
        BDMFMON_MAKE_PARM_ENUM_DEFVAL( "full_cfg", "Print all configurations", bdmfmon_enum_bool_table, 0, "no"));

    BDMFMON_MAKE_CMD_NOPARM(qm_dir, "debug_get", "get debug information", (bdmfmon_cmd_cb_t)drv_qm_cli_debug_get);

    BDMFMON_MAKE_CMD_NOPARM(qm_dir, "sanity", "sanity check", (bdmfmon_cmd_cb_t)drv_qm_cli_sanity_check);

    BDMFMON_MAKE_CMD(qm_dir, "init", "Initialize QM block", _qm_cli_init_handler,
        BDMFMON_MAKE_PARM( "fpm_base", "FPM_BASE", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM_RANGE( "ddr_sop0", "DDR SoP 0", BDMFMON_PARM_NUMBER, 0, 0, 255),
        BDMFMON_MAKE_PARM_RANGE( "ddr_sop1", "DDR SoP 1", BDMFMON_PARM_NUMBER, 0, 0, 255),
        BDMFMON_MAKE_PARM_ENUM_DEFVAL( "clear_on_read", "Clear counters on read", bdmfmon_enum_bool_table, 0, "no"),
        BDMFMON_MAKE_PARM_ENUM_DEFVAL( "wrap_around", "Counters can wrap around", bdmfmon_enum_bool_table, 0, "yes") );

    BDMFMON_MAKE_CMD(qm_dir, "qcfg", "Configure queue", _qm_cli_qcfg_handler,
        BDMFMON_MAKE_PARM_RANGE( "idx", "Queue index", BDMFMON_PARM_NUMBER, 0, 0, 287),
        BDMFMON_MAKE_PARM_RANGE( "wred", "WRED profile", BDMFMON_PARM_NUMBER, 0, 0, 15),
        BDMFMON_MAKE_PARM_RANGE( "ug", "UG profile", BDMFMON_PARM_NUMBER, 0, 0, 3),
        BDMFMON_MAKE_PARM_RANGE( "cd", "CD profile", BDMFMON_PARM_NUMBER, 0, 0, 7),
        BDMFMON_MAKE_PARM_ENUM_DEFVAL( "disable_copy", "Disable copy to DDR", bdmfmon_enum_bool_table, 0, "no"),
        BDMFMON_MAKE_PARM_ENUM_DEFVAL( "force_copy", "Force copy to DDR", bdmfmon_enum_bool_table, 0, "no"),
        BDMFMON_MAKE_PARM_ENUM_DEFVAL( "disable_aggregation", "Disable aggregation", bdmfmon_enum_bool_table, 0, "no"),
        BDMFMON_MAKE_PARM_ENUM_DEFVAL( "exclusive", "Exclusive priority", bdmfmon_enum_bool_table, 0, "no"),
        BDMFMON_MAKE_PARM_ENUM_DEFVAL( "802_1ae", "802_1ae support", bdmfmon_enum_bool_table, 0, "no"),
        BDMFMON_MAKE_PARM_ENUM_DEFVAL( "sci", "SCI support", bdmfmon_enum_bool_table, 0, "no"),
        BDMFMON_MAKE_PARM_ENUM_DEFVAL( "fec", "Enable FEC", bdmfmon_enum_bool_table, 0, "no"),
        BDMFMON_MAKE_PARM_RANGE( "res_profile", "Min Buffer Reservation profile", BDMFMON_PARM_NUMBER, 0, 0, 7));

    BDMFMON_MAKE_CMD(qm_dir, "qcfg_g_disable_aggregation", "Configure group of queues(US or DS) to disable (or not) aggregation", _qm_cli_qcfg_g_disable_aggregation_handler,
        BDMFMON_MAKE_PARM_ENUM( "US", "up stream queues", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM( "DS", "dp stream queues", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM( "disable_aggregation", "Disable aggregation", bdmfmon_enum_bool_table, 0));

    BDMFMON_MAKE_CMD(qm_dir, "qcfg_g_copy_to_ddr", "Configure group of queues(US or DS) to control copy to ddr", _qm_cli_qcfg_g_copy_to_ddr_handler,
        BDMFMON_MAKE_PARM_ENUM( "US", "up stream queues", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM( "DS", "dp stream queues", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM( "disable_copy", "Disable copy to DDR", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM( "force_copy", "Force copy to DDR", bdmfmon_enum_bool_table, 0));
        
    BDMFMON_MAKE_CMD(qm_dir, "qdisable", "Disable queue", _qm_cli_qdisable_handler,
        BDMFMON_MAKE_PARM_RANGE( "idx", "Queue index", BDMFMON_PARM_NUMBER, 0, 0, 287),
        BDMFMON_MAKE_PARM_ENUM( "flush", "Disable copy to DDR", bdmfmon_enum_bool_table, 0));

    BDMFMON_MAKE_CMD_NOPARM(qm_dir, "print_non_empty_queues", "print all non empty queues for debug", _drv_qm_cli_print_not_empty_queues);

}


void drv_qm_cli_exit(bdmfmon_handle_t driver_dir)
{
    if (qm_dir)
    {
        bdmfmon_token_destroy(qm_dir);
        qm_dir = NULL;
    }
}

#endif /* USE_BDMF_SHELL */

