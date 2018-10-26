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

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/

#include "rdp_common.h"
#include "rdp_drv_qm.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_qm_ag.h"
#include "rdp_drv_dqm.h"

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
    for (i = 0; i < init_cfg->num_fpm_ug; i++)
    {
        err = ag_drv_qm_fpm_ug_thr_set(i, &init_cfg->fpm_ug_thr[i]);
        QM_ERR_RETURN(err, "ag_drv_qm_fpm_ug_thr_set()\n");
    }

    /* Drop counters behaviour */
    {
        qm_drop_counters_ctrl drop_ctrl = {};

#if !defined(BCM6836)
        drop_ctrl.exclusive_dont_drop = 1;
#endif
        drop_ctrl.qm_preserve_pd_with_fpm = 1;
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

    err = ag_drv_qm_fpm_base_addr_set(init_cfg->fpm_base);
    QM_ERR_RETURN(err, "ag_drv_qm_fpm_base_addr_set()\n");

    err =  ag_drv_qm_fpm_ctrl_get(&fpm_pool_bp_enable, &fpm_congestion_bp_enable, &buff_size, &fpm_prefetch_pending_req_limit);
    QM_ERR_RETURN(err, "ag_drv_qm_fpm_ctrl_get()\n");
    err = ag_drv_qm_fpm_ctrl_set(fpm_pool_bp_enable, fpm_congestion_bp_enable, init_cfg->fpm_buf_size, fpm_prefetch_pending_req_limit);
    QM_ERR_RETURN(err, "ag_drv_qm_fpm_ctrl_set()\n");

    err = ag_drv_qm_ddr_sop_offset_set(init_cfg->ddr_sop_offset[0], init_cfg->ddr_sop_offset[1]);
    QM_ERR_RETURN(err, "ag_drv_qm_ddr_sop_offset_set()\n");

    /* DROP_ALL WRED profile */
    {
        qm_wred_profile_cfg wred_cfg = {};
        err = ag_drv_qm_wred_profile_cfg_set(QM_WRED_PROFILE_DROP_ALL, &wred_cfg);
        QM_ERR_RETURN(err, "ag_drv_qm_wred_profile_cfg_set()\n");
    }

    /* EPON second level queue WRED profile */
    {
        qm_wred_profile_cfg wred_cfg = {};
        wred_cfg.min_thr0 = QM_WRED_PROFILE_MAX_VAL;
        wred_cfg.min_thr1 = QM_WRED_PROFILE_MAX_VAL;
        wred_cfg.max_thr0 = QM_WRED_PROFILE_MAX_VAL;
        wred_cfg.max_thr1 = QM_WRED_PROFILE_MAX_VAL;
        err = ag_drv_qm_wred_profile_cfg_set(QM_WRED_PROFILE_EPON, &wred_cfg);
        QM_ERR_RETURN(err, "ag_drv_qm_wred_profile_cfg_set()\n");
    }

    /* Clear Q context SRAM */
    for (i = 0; i < QM_NUM_QUEUES; i++)
    {
        qm_q_context ctx = {.wred_profile = QM_WRED_PROFILE_DROP_ALL};
        err = ag_drv_qm_q_context_set(i, &ctx);
    }

#if defined(BCM6858) || defined(BCM6846) || defined(BCM6836) || defined(BCM63158)
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

#ifndef _CFE_    
    /* init qm engine locks*/
    bdmf_fastlock_init(&qm_engine_lock[0]);
    bdmf_fastlock_init(&qm_engine_lock[1]);
#endif

    qm_initialized = 1;

    return BDMF_ERR_OK;
}
bdmf_error_t drv_qm_fpm_min_pool_get(uint16_t* fpm_min_pool_size)
{
    qm_fpm_ctrl_cfg fpm_ctrl_cfg = {};
    bdmf_error_t rc =  ag_drv_qm_fpm_ctrl_get(&(fpm_ctrl_cfg.fpm_pool_bp_enable),
            &(fpm_ctrl_cfg.fpm_congestion_bp_enable),
            &(fpm_ctrl_cfg.fpm_prefetch_min_pool_size),
            &(fpm_ctrl_cfg.fpm_prefetch_pending_req_limit));
    if (!rc)
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
    q_cfg.ddr_copy_disable = copy_to_ddr ? copy_to_ddr : q_cfg.ddr_copy_disable;
    rc = rc ? rc : drv_qm_queue_config(q_idx, &q_cfg);
    rc = rc ? rc : drv_qm_queue_enable(q_idx);

    return rc;
}


int drv_qm_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val)
{
#ifndef BCM6836 /*not supported for 6836*/
     qm_clk_gate_clk_gate_cntrl qm_ctrl;

     ag_drv_qm_clk_gate_clk_gate_cntrl_get(&qm_ctrl);
     qm_ctrl.bypass_clk_gate = auto_gate ? 0 : 1;
     qm_ctrl.timer_val = timer_val;
     ag_drv_qm_clk_gate_clk_gate_cntrl_set(&qm_ctrl);
#endif
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
 * BDMFMON_MAKE_PARM_ENUM_DEFVAL( "fec", "Enable FEC", bdmfmon_enum_bool_table, 0, "no"));
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
        err = _qm_cli_qcfg_g_helper(QM_QUEUE_US_START, QM_QUEUE_US_END, 0,0, (bdmf_boolean)parm[2].value.number, 1<<QM_CNFG_G_DISABLE_AGGREGATION_S);
        if (err)
            return err;
    }

    if (ds)
    {
        err = _qm_cli_qcfg_g_helper(QM_QUEUE_DS_START, QM_QUEUE_DS_END, 0, 0, (bdmf_boolean)parm[2].value.number, 1<<QM_CNFG_G_DISABLE_AGGREGATION_S);
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
        err = _qm_cli_qcfg_g_helper(QM_QUEUE_US_START, QM_QUEUE_US_END, (bdmf_boolean)parm[2].value.number, (bdmf_boolean)parm[3].value.number, 0, ((1<< QM_CNFG_G_DISABLE_COPY_TO_DDR_S) | (1<< QM_CNFG_G_FORCE_COPY_TO_DDR_S)));
        if (err)
            return err;
    }
    
    if (ds)
    {
        err = _qm_cli_qcfg_g_helper(QM_QUEUE_DS_START, QM_QUEUE_DS_END, (bdmf_boolean)parm[2].value.number, (bdmf_boolean)parm[3].value.number, 0, ((1<< QM_CNFG_G_DISABLE_COPY_TO_DDR_S) | (1<< QM_CNFG_G_FORCE_COPY_TO_DDR_S)));
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
    HAL_CLI_PRINT_NUM_OF_LIST(session, qm, fpm_ug_cfg, QM_MAX_FPM_USER_GRP_CFG);
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
        BDMFMON_MAKE_PARM_ENUM_DEFVAL( "fec", "Enable FEC", bdmfmon_enum_bool_table, 0, "no"));

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

