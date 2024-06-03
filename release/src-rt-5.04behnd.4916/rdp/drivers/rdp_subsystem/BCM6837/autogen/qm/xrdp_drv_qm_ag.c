/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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


#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_qm_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_qm_ddr_cong_ctrl_set(const qm_ddr_cong_ctrl *ddr_cong_ctrl)
{
    uint32_t reg_global_cfg_ddr_byte_congestion_control = 0;
    uint32_t reg_global_cfg_ddr_byte_congestion_lower_thr = 0;
    uint32_t reg_global_cfg_ddr_byte_congestion_mid_thr = 0;
    uint32_t reg_global_cfg_ddr_byte_congestion_higher_thr = 0;
    uint32_t reg_global_cfg_ddr_pd_congestion_control = 0;

#ifdef VALIDATE_PARMS
    if(!ddr_cong_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((ddr_cong_ctrl->ddr_byte_congestion_drop_enable >= _1BITS_MAX_VAL_) ||
       (ddr_cong_ctrl->ddr_bytes_lower_thr >= _30BITS_MAX_VAL_) ||
       (ddr_cong_ctrl->ddr_bytes_mid_thr >= _30BITS_MAX_VAL_) ||
       (ddr_cong_ctrl->ddr_bytes_higher_thr >= _30BITS_MAX_VAL_) ||
       (ddr_cong_ctrl->ddr_pd_congestion_drop_enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_ddr_byte_congestion_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL, DDR_BYTE_CONGESTION_DROP_ENABLE, reg_global_cfg_ddr_byte_congestion_control, ddr_cong_ctrl->ddr_byte_congestion_drop_enable);
    reg_global_cfg_ddr_byte_congestion_lower_thr = RU_FIELD_SET(0, QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR, DDR_BYTES_LOWER_THR, reg_global_cfg_ddr_byte_congestion_lower_thr, ddr_cong_ctrl->ddr_bytes_lower_thr);
    reg_global_cfg_ddr_byte_congestion_mid_thr = RU_FIELD_SET(0, QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR, DDR_BYTES_MID_THR, reg_global_cfg_ddr_byte_congestion_mid_thr, ddr_cong_ctrl->ddr_bytes_mid_thr);
    reg_global_cfg_ddr_byte_congestion_higher_thr = RU_FIELD_SET(0, QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR, DDR_BYTES_HIGHER_THR, reg_global_cfg_ddr_byte_congestion_higher_thr, ddr_cong_ctrl->ddr_bytes_higher_thr);
    reg_global_cfg_ddr_pd_congestion_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL, DDR_PD_CONGESTION_DROP_ENABLE, reg_global_cfg_ddr_pd_congestion_control, ddr_cong_ctrl->ddr_pd_congestion_drop_enable);
    reg_global_cfg_ddr_pd_congestion_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL, DDR_PIPE_LOWER_THR, reg_global_cfg_ddr_pd_congestion_control, ddr_cong_ctrl->ddr_pipe_lower_thr);
    reg_global_cfg_ddr_pd_congestion_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL, DDR_PIPE_HIGHER_THR, reg_global_cfg_ddr_pd_congestion_control, ddr_cong_ctrl->ddr_pipe_higher_thr);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL, reg_global_cfg_ddr_byte_congestion_control);
    RU_REG_WRITE(0, QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR, reg_global_cfg_ddr_byte_congestion_lower_thr);
    RU_REG_WRITE(0, QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR, reg_global_cfg_ddr_byte_congestion_mid_thr);
    RU_REG_WRITE(0, QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR, reg_global_cfg_ddr_byte_congestion_higher_thr);
    RU_REG_WRITE(0, QM, GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL, reg_global_cfg_ddr_pd_congestion_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_ddr_cong_ctrl_get(qm_ddr_cong_ctrl *ddr_cong_ctrl)
{
    uint32_t reg_global_cfg_ddr_byte_congestion_control;
    uint32_t reg_global_cfg_ddr_byte_congestion_lower_thr;
    uint32_t reg_global_cfg_ddr_byte_congestion_mid_thr;
    uint32_t reg_global_cfg_ddr_byte_congestion_higher_thr;
    uint32_t reg_global_cfg_ddr_pd_congestion_control;

#ifdef VALIDATE_PARMS
    if (!ddr_cong_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL, reg_global_cfg_ddr_byte_congestion_control);
    RU_REG_READ(0, QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR, reg_global_cfg_ddr_byte_congestion_lower_thr);
    RU_REG_READ(0, QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR, reg_global_cfg_ddr_byte_congestion_mid_thr);
    RU_REG_READ(0, QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR, reg_global_cfg_ddr_byte_congestion_higher_thr);
    RU_REG_READ(0, QM, GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL, reg_global_cfg_ddr_pd_congestion_control);

    ddr_cong_ctrl->ddr_byte_congestion_drop_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL, DDR_BYTE_CONGESTION_DROP_ENABLE, reg_global_cfg_ddr_byte_congestion_control);
    ddr_cong_ctrl->ddr_bytes_lower_thr = RU_FIELD_GET(0, QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR, DDR_BYTES_LOWER_THR, reg_global_cfg_ddr_byte_congestion_lower_thr);
    ddr_cong_ctrl->ddr_bytes_mid_thr = RU_FIELD_GET(0, QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR, DDR_BYTES_MID_THR, reg_global_cfg_ddr_byte_congestion_mid_thr);
    ddr_cong_ctrl->ddr_bytes_higher_thr = RU_FIELD_GET(0, QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR, DDR_BYTES_HIGHER_THR, reg_global_cfg_ddr_byte_congestion_higher_thr);
    ddr_cong_ctrl->ddr_pd_congestion_drop_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL, DDR_PD_CONGESTION_DROP_ENABLE, reg_global_cfg_ddr_pd_congestion_control);
    ddr_cong_ctrl->ddr_pipe_lower_thr = RU_FIELD_GET(0, QM, GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL, DDR_PIPE_LOWER_THR, reg_global_cfg_ddr_pd_congestion_control);
    ddr_cong_ctrl->ddr_pipe_higher_thr = RU_FIELD_GET(0, QM, GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL, DDR_PIPE_HIGHER_THR, reg_global_cfg_ddr_pd_congestion_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_is_queue_not_empty_get(uint16_t q_idx, bdmf_boolean *data)
{
    uint32_t reg_global_cfg_dqm_not_empty;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((q_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx / 32, QM, GLOBAL_CFG_DQM_NOT_EMPTY, reg_global_cfg_dqm_not_empty);

    *data = RU_FLD_VAL_GET(reg_global_cfg_dqm_not_empty, 0x1 << (q_idx % 32), q_idx % 32);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_is_queue_pop_ready_get(uint16_t q_idx, bdmf_boolean *data)
{
    uint32_t reg_global_cfg_dqm_pop_ready;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((q_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx / 32, QM, GLOBAL_CFG_DQM_POP_READY, reg_global_cfg_dqm_pop_ready);

    *data = RU_FLD_VAL_GET(reg_global_cfg_dqm_pop_ready, 0x1 << (q_idx % 32), q_idx % 32);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_is_queue_full_get(uint16_t q_idx, bdmf_boolean *data)
{
    uint32_t reg_global_cfg_dqm_full;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((q_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx / 32, QM, GLOBAL_CFG_DQM_FULL, reg_global_cfg_dqm_full);

    *data = RU_FLD_VAL_GET(reg_global_cfg_dqm_full, 0x1 << (q_idx % 32), q_idx % 32);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_ug_thr_set(uint8_t ug_grp_idx, const qm_fpm_ug_thr *fpm_ug_thr)
{
    uint32_t reg_fpm_usr_grp_lower_thr = 0;
    uint32_t reg_fpm_usr_grp_mid_thr = 0;
    uint32_t reg_fpm_usr_grp_higher_thr = 0;

#ifdef VALIDATE_PARMS
    if(!fpm_ug_thr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((ug_grp_idx >= 4) ||
       (fpm_ug_thr->lower_thr >= _18BITS_MAX_VAL_) ||
       (fpm_ug_thr->mid_thr >= _18BITS_MAX_VAL_) ||
       (fpm_ug_thr->higher_thr >= _18BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpm_usr_grp_lower_thr = RU_FIELD_SET(0, QM, FPM_USR_GRP_LOWER_THR, FPM_GRP_LOWER_THR, reg_fpm_usr_grp_lower_thr, fpm_ug_thr->lower_thr);
    reg_fpm_usr_grp_mid_thr = RU_FIELD_SET(0, QM, FPM_USR_GRP_MID_THR, FPM_GRP_MID_THR, reg_fpm_usr_grp_mid_thr, fpm_ug_thr->mid_thr);
    reg_fpm_usr_grp_higher_thr = RU_FIELD_SET(0, QM, FPM_USR_GRP_HIGHER_THR, FPM_GRP_HIGHER_THR, reg_fpm_usr_grp_higher_thr, fpm_ug_thr->higher_thr);

    RU_REG_RAM_WRITE(0, ug_grp_idx, QM, FPM_USR_GRP_LOWER_THR, reg_fpm_usr_grp_lower_thr);
    RU_REG_RAM_WRITE(0, ug_grp_idx, QM, FPM_USR_GRP_MID_THR, reg_fpm_usr_grp_mid_thr);
    RU_REG_RAM_WRITE(0, ug_grp_idx, QM, FPM_USR_GRP_HIGHER_THR, reg_fpm_usr_grp_higher_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_ug_thr_get(uint8_t ug_grp_idx, qm_fpm_ug_thr *fpm_ug_thr)
{
    uint32_t reg_fpm_usr_grp_lower_thr;
    uint32_t reg_fpm_usr_grp_mid_thr;
    uint32_t reg_fpm_usr_grp_higher_thr;

#ifdef VALIDATE_PARMS
    if (!fpm_ug_thr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((ug_grp_idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, ug_grp_idx, QM, FPM_USR_GRP_LOWER_THR, reg_fpm_usr_grp_lower_thr);
    RU_REG_RAM_READ(0, ug_grp_idx, QM, FPM_USR_GRP_MID_THR, reg_fpm_usr_grp_mid_thr);
    RU_REG_RAM_READ(0, ug_grp_idx, QM, FPM_USR_GRP_HIGHER_THR, reg_fpm_usr_grp_higher_thr);

    fpm_ug_thr->lower_thr = RU_FIELD_GET(0, QM, FPM_USR_GRP_LOWER_THR, FPM_GRP_LOWER_THR, reg_fpm_usr_grp_lower_thr);
    fpm_ug_thr->mid_thr = RU_FIELD_GET(0, QM, FPM_USR_GRP_MID_THR, FPM_GRP_MID_THR, reg_fpm_usr_grp_mid_thr);
    fpm_ug_thr->higher_thr = RU_FIELD_GET(0, QM, FPM_USR_GRP_HIGHER_THR, FPM_GRP_HIGHER_THR, reg_fpm_usr_grp_higher_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_rnr_group_cfg_set(uint8_t rnr_idx, const qm_rnr_group_cfg *rnr_group_cfg)
{
    uint32_t reg_runner_grp_queue_config = 0;
    uint32_t reg_runner_grp_pdfifo_config = 0;
    uint32_t reg_runner_grp_update_fifo_config = 0;
    uint32_t reg_runner_grp_rnr_config = 0;

#ifdef VALIDATE_PARMS
    if(!rnr_group_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_idx >= 16) ||
       (rnr_group_cfg->start_queue >= _9BITS_MAX_VAL_) ||
       (rnr_group_cfg->end_queue >= _9BITS_MAX_VAL_) ||
       (rnr_group_cfg->pd_fifo_base >= _11BITS_MAX_VAL_) ||
       (rnr_group_cfg->pd_fifo_size >= _2BITS_MAX_VAL_) ||
       (rnr_group_cfg->upd_fifo_base >= _11BITS_MAX_VAL_) ||
       (rnr_group_cfg->upd_fifo_size >= _3BITS_MAX_VAL_) ||
       (rnr_group_cfg->rnr_bb_id >= _6BITS_MAX_VAL_) ||
       (rnr_group_cfg->rnr_task >= _4BITS_MAX_VAL_) ||
       (rnr_group_cfg->rnr_enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_runner_grp_queue_config = RU_FIELD_SET(0, QM, RUNNER_GRP_QUEUE_CONFIG, START_QUEUE, reg_runner_grp_queue_config, rnr_group_cfg->start_queue);
    reg_runner_grp_queue_config = RU_FIELD_SET(0, QM, RUNNER_GRP_QUEUE_CONFIG, END_QUEUE, reg_runner_grp_queue_config, rnr_group_cfg->end_queue);
    reg_runner_grp_pdfifo_config = RU_FIELD_SET(0, QM, RUNNER_GRP_PDFIFO_CONFIG, BASE_ADDR, reg_runner_grp_pdfifo_config, rnr_group_cfg->pd_fifo_base);
    reg_runner_grp_pdfifo_config = RU_FIELD_SET(0, QM, RUNNER_GRP_PDFIFO_CONFIG, SIZE, reg_runner_grp_pdfifo_config, rnr_group_cfg->pd_fifo_size);
    reg_runner_grp_update_fifo_config = RU_FIELD_SET(0, QM, RUNNER_GRP_UPDATE_FIFO_CONFIG, BASE_ADDR, reg_runner_grp_update_fifo_config, rnr_group_cfg->upd_fifo_base);
    reg_runner_grp_update_fifo_config = RU_FIELD_SET(0, QM, RUNNER_GRP_UPDATE_FIFO_CONFIG, SIZE, reg_runner_grp_update_fifo_config, rnr_group_cfg->upd_fifo_size);
    reg_runner_grp_rnr_config = RU_FIELD_SET(0, QM, RUNNER_GRP_RNR_CONFIG, RNR_BB_ID, reg_runner_grp_rnr_config, rnr_group_cfg->rnr_bb_id);
    reg_runner_grp_rnr_config = RU_FIELD_SET(0, QM, RUNNER_GRP_RNR_CONFIG, RNR_TASK, reg_runner_grp_rnr_config, rnr_group_cfg->rnr_task);
    reg_runner_grp_rnr_config = RU_FIELD_SET(0, QM, RUNNER_GRP_RNR_CONFIG, RNR_ENABLE, reg_runner_grp_rnr_config, rnr_group_cfg->rnr_enable);

    RU_REG_RAM_WRITE(0, rnr_idx, QM, RUNNER_GRP_QUEUE_CONFIG, reg_runner_grp_queue_config);
    RU_REG_RAM_WRITE(0, rnr_idx, QM, RUNNER_GRP_PDFIFO_CONFIG, reg_runner_grp_pdfifo_config);
    RU_REG_RAM_WRITE(0, rnr_idx, QM, RUNNER_GRP_UPDATE_FIFO_CONFIG, reg_runner_grp_update_fifo_config);
    RU_REG_RAM_WRITE(0, rnr_idx, QM, RUNNER_GRP_RNR_CONFIG, reg_runner_grp_rnr_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_rnr_group_cfg_get(uint8_t rnr_idx, qm_rnr_group_cfg *rnr_group_cfg)
{
    uint32_t reg_runner_grp_queue_config;
    uint32_t reg_runner_grp_pdfifo_config;
    uint32_t reg_runner_grp_update_fifo_config;
    uint32_t reg_runner_grp_rnr_config;

#ifdef VALIDATE_PARMS
    if (!rnr_group_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((rnr_idx >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, rnr_idx, QM, RUNNER_GRP_QUEUE_CONFIG, reg_runner_grp_queue_config);
    RU_REG_RAM_READ(0, rnr_idx, QM, RUNNER_GRP_PDFIFO_CONFIG, reg_runner_grp_pdfifo_config);
    RU_REG_RAM_READ(0, rnr_idx, QM, RUNNER_GRP_UPDATE_FIFO_CONFIG, reg_runner_grp_update_fifo_config);
    RU_REG_RAM_READ(0, rnr_idx, QM, RUNNER_GRP_RNR_CONFIG, reg_runner_grp_rnr_config);

    rnr_group_cfg->start_queue = RU_FIELD_GET(0, QM, RUNNER_GRP_QUEUE_CONFIG, START_QUEUE, reg_runner_grp_queue_config);
    rnr_group_cfg->end_queue = RU_FIELD_GET(0, QM, RUNNER_GRP_QUEUE_CONFIG, END_QUEUE, reg_runner_grp_queue_config);
    rnr_group_cfg->pd_fifo_base = RU_FIELD_GET(0, QM, RUNNER_GRP_PDFIFO_CONFIG, BASE_ADDR, reg_runner_grp_pdfifo_config);
    rnr_group_cfg->pd_fifo_size = RU_FIELD_GET(0, QM, RUNNER_GRP_PDFIFO_CONFIG, SIZE, reg_runner_grp_pdfifo_config);
    rnr_group_cfg->upd_fifo_base = RU_FIELD_GET(0, QM, RUNNER_GRP_UPDATE_FIFO_CONFIG, BASE_ADDR, reg_runner_grp_update_fifo_config);
    rnr_group_cfg->upd_fifo_size = RU_FIELD_GET(0, QM, RUNNER_GRP_UPDATE_FIFO_CONFIG, SIZE, reg_runner_grp_update_fifo_config);
    rnr_group_cfg->rnr_bb_id = RU_FIELD_GET(0, QM, RUNNER_GRP_RNR_CONFIG, RNR_BB_ID, reg_runner_grp_rnr_config);
    rnr_group_cfg->rnr_task = RU_FIELD_GET(0, QM, RUNNER_GRP_RNR_CONFIG, RNR_TASK, reg_runner_grp_rnr_config);
    rnr_group_cfg->rnr_enable = RU_FIELD_GET(0, QM, RUNNER_GRP_RNR_CONFIG, RNR_ENABLE, reg_runner_grp_rnr_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_cpu_pd_indirect_wr_data_set(uint8_t indirect_grp_idx, uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3)
{
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_wr_data_0 = 0;
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_wr_data_1 = 0;
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_wr_data_2 = 0;
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_wr_data_3 = 0;

#ifdef VALIDATE_PARMS
    if ((indirect_grp_idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cpu_indr_port_cpu_pd_indirect_wr_data_0 = RU_FIELD_SET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0, DATA, reg_cpu_indr_port_cpu_pd_indirect_wr_data_0, data0);
    reg_cpu_indr_port_cpu_pd_indirect_wr_data_1 = RU_FIELD_SET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1, DATA, reg_cpu_indr_port_cpu_pd_indirect_wr_data_1, data1);
    reg_cpu_indr_port_cpu_pd_indirect_wr_data_2 = RU_FIELD_SET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2, DATA, reg_cpu_indr_port_cpu_pd_indirect_wr_data_2, data2);
    reg_cpu_indr_port_cpu_pd_indirect_wr_data_3 = RU_FIELD_SET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3, DATA, reg_cpu_indr_port_cpu_pd_indirect_wr_data_3, data3);

    RU_REG_RAM_WRITE(0, indirect_grp_idx, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0, reg_cpu_indr_port_cpu_pd_indirect_wr_data_0);
    RU_REG_RAM_WRITE(0, indirect_grp_idx, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1, reg_cpu_indr_port_cpu_pd_indirect_wr_data_1);
    RU_REG_RAM_WRITE(0, indirect_grp_idx, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2, reg_cpu_indr_port_cpu_pd_indirect_wr_data_2);
    RU_REG_RAM_WRITE(0, indirect_grp_idx, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3, reg_cpu_indr_port_cpu_pd_indirect_wr_data_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_cpu_pd_indirect_wr_data_get(uint8_t indirect_grp_idx, uint32_t *data0, uint32_t *data1, uint32_t *data2, uint32_t *data3)
{
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_wr_data_0;
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_wr_data_1;
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_wr_data_2;
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_wr_data_3;

#ifdef VALIDATE_PARMS
    if (!data0 || !data1 || !data2 || !data3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((indirect_grp_idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, indirect_grp_idx, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0, reg_cpu_indr_port_cpu_pd_indirect_wr_data_0);
    RU_REG_RAM_READ(0, indirect_grp_idx, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1, reg_cpu_indr_port_cpu_pd_indirect_wr_data_1);
    RU_REG_RAM_READ(0, indirect_grp_idx, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2, reg_cpu_indr_port_cpu_pd_indirect_wr_data_2);
    RU_REG_RAM_READ(0, indirect_grp_idx, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3, reg_cpu_indr_port_cpu_pd_indirect_wr_data_3);

    *data0 = RU_FIELD_GET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0, DATA, reg_cpu_indr_port_cpu_pd_indirect_wr_data_0);
    *data1 = RU_FIELD_GET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1, DATA, reg_cpu_indr_port_cpu_pd_indirect_wr_data_1);
    *data2 = RU_FIELD_GET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2, DATA, reg_cpu_indr_port_cpu_pd_indirect_wr_data_2);
    *data3 = RU_FIELD_GET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3, DATA, reg_cpu_indr_port_cpu_pd_indirect_wr_data_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_cpu_pd_indirect_rd_data_get(uint8_t indirect_grp_idx, uint32_t *data0, uint32_t *data1, uint32_t *data2, uint32_t *data3)
{
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_rd_data_0;
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_rd_data_1;
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_rd_data_2;
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_rd_data_3;

#ifdef VALIDATE_PARMS
    if (!data0 || !data1 || !data2 || !data3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((indirect_grp_idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, indirect_grp_idx, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0, reg_cpu_indr_port_cpu_pd_indirect_rd_data_0);
    RU_REG_RAM_READ(0, indirect_grp_idx, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1, reg_cpu_indr_port_cpu_pd_indirect_rd_data_1);
    RU_REG_RAM_READ(0, indirect_grp_idx, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2, reg_cpu_indr_port_cpu_pd_indirect_rd_data_2);
    RU_REG_RAM_READ(0, indirect_grp_idx, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3, reg_cpu_indr_port_cpu_pd_indirect_rd_data_3);

    *data0 = RU_FIELD_GET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0, DATA, reg_cpu_indr_port_cpu_pd_indirect_rd_data_0);
    *data1 = RU_FIELD_GET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1, DATA, reg_cpu_indr_port_cpu_pd_indirect_rd_data_1);
    *data2 = RU_FIELD_GET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2, DATA, reg_cpu_indr_port_cpu_pd_indirect_rd_data_2);
    *data3 = RU_FIELD_GET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3, DATA, reg_cpu_indr_port_cpu_pd_indirect_rd_data_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_aggr_context_get(uint16_t idx, uint32_t *context_valid)
{
    uint32_t reg_global_cfg_aggregation_context_valid;

#ifdef VALIDATE_PARMS
    if (!context_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 5))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, GLOBAL_CFG_AGGREGATION_CONTEXT_VALID, reg_global_cfg_aggregation_context_valid);

    *context_valid = RU_FIELD_GET(0, QM, GLOBAL_CFG_AGGREGATION_CONTEXT_VALID, CONTEXT_VALID, reg_global_cfg_aggregation_context_valid);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_wred_profile_cfg_set(uint8_t profile_idx, const qm_wred_profile_cfg *wred_profile_cfg)
{
    uint32_t reg_wred_profile_color_min_thr_0 = 0;
    uint32_t reg_wred_profile_color_min_thr_1 = 0;
    uint32_t reg_wred_profile_color_max_thr_0 = 0;
    uint32_t reg_wred_profile_color_max_thr_1 = 0;
    uint32_t reg_wred_profile_color_slope_0 = 0;
    uint32_t reg_wred_profile_color_slope_1 = 0;

#ifdef VALIDATE_PARMS
    if(!wred_profile_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((profile_idx >= 16) ||
       (wred_profile_cfg->min_thr0 >= _24BITS_MAX_VAL_) ||
       (wred_profile_cfg->flw_ctrl_en0 >= _1BITS_MAX_VAL_) ||
       (wred_profile_cfg->min_thr1 >= _24BITS_MAX_VAL_) ||
       (wred_profile_cfg->flw_ctrl_en1 >= _1BITS_MAX_VAL_) ||
       (wred_profile_cfg->max_thr0 >= _24BITS_MAX_VAL_) ||
       (wred_profile_cfg->max_thr1 >= _24BITS_MAX_VAL_) ||
       (wred_profile_cfg->slope_exp0 >= _5BITS_MAX_VAL_) ||
       (wred_profile_cfg->slope_exp1 >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wred_profile_color_min_thr_0 = RU_FIELD_SET(0, QM, WRED_PROFILE_COLOR_MIN_THR_0, MIN_THR, reg_wred_profile_color_min_thr_0, wred_profile_cfg->min_thr0);
    reg_wred_profile_color_min_thr_0 = RU_FIELD_SET(0, QM, WRED_PROFILE_COLOR_MIN_THR_0, FLW_CTRL_EN, reg_wred_profile_color_min_thr_0, wred_profile_cfg->flw_ctrl_en0);
    reg_wred_profile_color_min_thr_1 = RU_FIELD_SET(0, QM, WRED_PROFILE_COLOR_MIN_THR_1, MIN_THR, reg_wred_profile_color_min_thr_1, wred_profile_cfg->min_thr1);
    reg_wred_profile_color_min_thr_1 = RU_FIELD_SET(0, QM, WRED_PROFILE_COLOR_MIN_THR_1, FLW_CTRL_EN, reg_wred_profile_color_min_thr_1, wred_profile_cfg->flw_ctrl_en1);
    reg_wred_profile_color_max_thr_0 = RU_FIELD_SET(0, QM, WRED_PROFILE_COLOR_MAX_THR_0, MAX_THR, reg_wred_profile_color_max_thr_0, wred_profile_cfg->max_thr0);
    reg_wred_profile_color_max_thr_1 = RU_FIELD_SET(0, QM, WRED_PROFILE_COLOR_MAX_THR_1, MAX_THR, reg_wred_profile_color_max_thr_1, wred_profile_cfg->max_thr1);
    reg_wred_profile_color_slope_0 = RU_FIELD_SET(0, QM, WRED_PROFILE_COLOR_SLOPE_0, SLOPE_MANTISSA, reg_wred_profile_color_slope_0, wred_profile_cfg->slope_mantissa0);
    reg_wred_profile_color_slope_0 = RU_FIELD_SET(0, QM, WRED_PROFILE_COLOR_SLOPE_0, SLOPE_EXP, reg_wred_profile_color_slope_0, wred_profile_cfg->slope_exp0);
    reg_wred_profile_color_slope_1 = RU_FIELD_SET(0, QM, WRED_PROFILE_COLOR_SLOPE_1, SLOPE_MANTISSA, reg_wred_profile_color_slope_1, wred_profile_cfg->slope_mantissa1);
    reg_wred_profile_color_slope_1 = RU_FIELD_SET(0, QM, WRED_PROFILE_COLOR_SLOPE_1, SLOPE_EXP, reg_wred_profile_color_slope_1, wred_profile_cfg->slope_exp1);

    RU_REG_RAM_WRITE(0, profile_idx, QM, WRED_PROFILE_COLOR_MIN_THR_0, reg_wred_profile_color_min_thr_0);
    RU_REG_RAM_WRITE(0, profile_idx, QM, WRED_PROFILE_COLOR_MIN_THR_1, reg_wred_profile_color_min_thr_1);
    RU_REG_RAM_WRITE(0, profile_idx, QM, WRED_PROFILE_COLOR_MAX_THR_0, reg_wred_profile_color_max_thr_0);
    RU_REG_RAM_WRITE(0, profile_idx, QM, WRED_PROFILE_COLOR_MAX_THR_1, reg_wred_profile_color_max_thr_1);
    RU_REG_RAM_WRITE(0, profile_idx, QM, WRED_PROFILE_COLOR_SLOPE_0, reg_wred_profile_color_slope_0);
    RU_REG_RAM_WRITE(0, profile_idx, QM, WRED_PROFILE_COLOR_SLOPE_1, reg_wred_profile_color_slope_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_wred_profile_cfg_get(uint8_t profile_idx, qm_wred_profile_cfg *wred_profile_cfg)
{
    uint32_t reg_wred_profile_color_min_thr_0;
    uint32_t reg_wred_profile_color_min_thr_1;
    uint32_t reg_wred_profile_color_max_thr_0;
    uint32_t reg_wred_profile_color_max_thr_1;
    uint32_t reg_wred_profile_color_slope_0;
    uint32_t reg_wred_profile_color_slope_1;

#ifdef VALIDATE_PARMS
    if (!wred_profile_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((profile_idx >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, profile_idx, QM, WRED_PROFILE_COLOR_MIN_THR_0, reg_wred_profile_color_min_thr_0);
    RU_REG_RAM_READ(0, profile_idx, QM, WRED_PROFILE_COLOR_MIN_THR_1, reg_wred_profile_color_min_thr_1);
    RU_REG_RAM_READ(0, profile_idx, QM, WRED_PROFILE_COLOR_MAX_THR_0, reg_wred_profile_color_max_thr_0);
    RU_REG_RAM_READ(0, profile_idx, QM, WRED_PROFILE_COLOR_MAX_THR_1, reg_wred_profile_color_max_thr_1);
    RU_REG_RAM_READ(0, profile_idx, QM, WRED_PROFILE_COLOR_SLOPE_0, reg_wred_profile_color_slope_0);
    RU_REG_RAM_READ(0, profile_idx, QM, WRED_PROFILE_COLOR_SLOPE_1, reg_wred_profile_color_slope_1);

    wred_profile_cfg->min_thr0 = RU_FIELD_GET(0, QM, WRED_PROFILE_COLOR_MIN_THR_0, MIN_THR, reg_wred_profile_color_min_thr_0);
    wred_profile_cfg->flw_ctrl_en0 = RU_FIELD_GET(0, QM, WRED_PROFILE_COLOR_MIN_THR_0, FLW_CTRL_EN, reg_wred_profile_color_min_thr_0);
    wred_profile_cfg->min_thr1 = RU_FIELD_GET(0, QM, WRED_PROFILE_COLOR_MIN_THR_1, MIN_THR, reg_wred_profile_color_min_thr_1);
    wred_profile_cfg->flw_ctrl_en1 = RU_FIELD_GET(0, QM, WRED_PROFILE_COLOR_MIN_THR_1, FLW_CTRL_EN, reg_wred_profile_color_min_thr_1);
    wred_profile_cfg->max_thr0 = RU_FIELD_GET(0, QM, WRED_PROFILE_COLOR_MAX_THR_0, MAX_THR, reg_wred_profile_color_max_thr_0);
    wred_profile_cfg->max_thr1 = RU_FIELD_GET(0, QM, WRED_PROFILE_COLOR_MAX_THR_1, MAX_THR, reg_wred_profile_color_max_thr_1);
    wred_profile_cfg->slope_mantissa0 = RU_FIELD_GET(0, QM, WRED_PROFILE_COLOR_SLOPE_0, SLOPE_MANTISSA, reg_wred_profile_color_slope_0);
    wred_profile_cfg->slope_exp0 = RU_FIELD_GET(0, QM, WRED_PROFILE_COLOR_SLOPE_0, SLOPE_EXP, reg_wred_profile_color_slope_0);
    wred_profile_cfg->slope_mantissa1 = RU_FIELD_GET(0, QM, WRED_PROFILE_COLOR_SLOPE_1, SLOPE_MANTISSA, reg_wred_profile_color_slope_1);
    wred_profile_cfg->slope_exp1 = RU_FIELD_GET(0, QM, WRED_PROFILE_COLOR_SLOPE_1, SLOPE_EXP, reg_wred_profile_color_slope_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_enable_ctrl_set(const qm_enable_ctrl *enable_ctrl)
{
    uint32_t reg_global_cfg_qm_enable_ctrl = 0;

#ifdef VALIDATE_PARMS
    if(!enable_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((enable_ctrl->fpm_prefetch_enable >= _1BITS_MAX_VAL_) ||
       (enable_ctrl->reorder_credit_enable >= _1BITS_MAX_VAL_) ||
       (enable_ctrl->dqm_pop_enable >= _1BITS_MAX_VAL_) ||
       (enable_ctrl->rmt_fixed_arb_enable >= _1BITS_MAX_VAL_) ||
       (enable_ctrl->dqm_push_fixed_arb_enable >= _1BITS_MAX_VAL_) ||
       (enable_ctrl->aqm_clk_counter_enable >= _1BITS_MAX_VAL_) ||
       (enable_ctrl->aqm_timestamp_counter_enable >= _1BITS_MAX_VAL_) ||
       (enable_ctrl->aqm_timestamp_write_to_pd_enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_qm_enable_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, FPM_PREFETCH_ENABLE, reg_global_cfg_qm_enable_ctrl, enable_ctrl->fpm_prefetch_enable);
    reg_global_cfg_qm_enable_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, REORDER_CREDIT_ENABLE, reg_global_cfg_qm_enable_ctrl, enable_ctrl->reorder_credit_enable);
    reg_global_cfg_qm_enable_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, DQM_POP_ENABLE, reg_global_cfg_qm_enable_ctrl, enable_ctrl->dqm_pop_enable);
    reg_global_cfg_qm_enable_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, RMT_FIXED_ARB_ENABLE, reg_global_cfg_qm_enable_ctrl, enable_ctrl->rmt_fixed_arb_enable);
    reg_global_cfg_qm_enable_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, DQM_PUSH_FIXED_ARB_ENABLE, reg_global_cfg_qm_enable_ctrl, enable_ctrl->dqm_push_fixed_arb_enable);
    reg_global_cfg_qm_enable_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, AQM_CLK_COUNTER_ENABLE, reg_global_cfg_qm_enable_ctrl, enable_ctrl->aqm_clk_counter_enable);
    reg_global_cfg_qm_enable_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, AQM_TIMESTAMP_COUNTER_ENABLE, reg_global_cfg_qm_enable_ctrl, enable_ctrl->aqm_timestamp_counter_enable);
    reg_global_cfg_qm_enable_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, AQM_TIMESTAMP_WRITE_TO_PD_ENABLE, reg_global_cfg_qm_enable_ctrl, enable_ctrl->aqm_timestamp_write_to_pd_enable);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, reg_global_cfg_qm_enable_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_enable_ctrl_get(qm_enable_ctrl *enable_ctrl)
{
    uint32_t reg_global_cfg_qm_enable_ctrl;

#ifdef VALIDATE_PARMS
    if (!enable_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, reg_global_cfg_qm_enable_ctrl);

    enable_ctrl->fpm_prefetch_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, FPM_PREFETCH_ENABLE, reg_global_cfg_qm_enable_ctrl);
    enable_ctrl->reorder_credit_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, REORDER_CREDIT_ENABLE, reg_global_cfg_qm_enable_ctrl);
    enable_ctrl->dqm_pop_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, DQM_POP_ENABLE, reg_global_cfg_qm_enable_ctrl);
    enable_ctrl->rmt_fixed_arb_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, RMT_FIXED_ARB_ENABLE, reg_global_cfg_qm_enable_ctrl);
    enable_ctrl->dqm_push_fixed_arb_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, DQM_PUSH_FIXED_ARB_ENABLE, reg_global_cfg_qm_enable_ctrl);
    enable_ctrl->aqm_clk_counter_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, AQM_CLK_COUNTER_ENABLE, reg_global_cfg_qm_enable_ctrl);
    enable_ctrl->aqm_timestamp_counter_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, AQM_TIMESTAMP_COUNTER_ENABLE, reg_global_cfg_qm_enable_ctrl);
    enable_ctrl->aqm_timestamp_write_to_pd_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, AQM_TIMESTAMP_WRITE_TO_PD_ENABLE, reg_global_cfg_qm_enable_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_reset_ctrl_set(const qm_reset_ctrl *reset_ctrl)
{
    uint32_t reg_global_cfg_qm_sw_rst_ctrl = 0;

#ifdef VALIDATE_PARMS
    if(!reset_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((reset_ctrl->fpm_prefetch0_sw_rst >= _1BITS_MAX_VAL_) ||
       (reset_ctrl->fpm_prefetch1_sw_rst >= _1BITS_MAX_VAL_) ||
       (reset_ctrl->fpm_prefetch2_sw_rst >= _1BITS_MAX_VAL_) ||
       (reset_ctrl->fpm_prefetch3_sw_rst >= _1BITS_MAX_VAL_) ||
       (reset_ctrl->normal_rmt_sw_rst >= _1BITS_MAX_VAL_) ||
       (reset_ctrl->non_delayed_rmt_sw_rst >= _1BITS_MAX_VAL_) ||
       (reset_ctrl->pre_cm_fifo_sw_rst >= _1BITS_MAX_VAL_) ||
       (reset_ctrl->cm_rd_pd_fifo_sw_rst >= _1BITS_MAX_VAL_) ||
       (reset_ctrl->cm_wr_pd_fifo_sw_rst >= _1BITS_MAX_VAL_) ||
       (reset_ctrl->bb0_output_fifo_sw_rst >= _1BITS_MAX_VAL_) ||
       (reset_ctrl->bb1_output_fifo_sw_rst >= _1BITS_MAX_VAL_) ||
       (reset_ctrl->bb1_input_fifo_sw_rst >= _1BITS_MAX_VAL_) ||
       (reset_ctrl->tm_fifo_ptr_sw_rst >= _1BITS_MAX_VAL_) ||
       (reset_ctrl->non_delayed_out_fifo_sw_rst >= _1BITS_MAX_VAL_) ||
       (reset_ctrl->bb0_egr_msg_out_fifo_sw_rst >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_qm_sw_rst_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, FPM_PREFETCH0_SW_RST, reg_global_cfg_qm_sw_rst_ctrl, reset_ctrl->fpm_prefetch0_sw_rst);
    reg_global_cfg_qm_sw_rst_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, FPM_PREFETCH1_SW_RST, reg_global_cfg_qm_sw_rst_ctrl, reset_ctrl->fpm_prefetch1_sw_rst);
    reg_global_cfg_qm_sw_rst_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, FPM_PREFETCH2_SW_RST, reg_global_cfg_qm_sw_rst_ctrl, reset_ctrl->fpm_prefetch2_sw_rst);
    reg_global_cfg_qm_sw_rst_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, FPM_PREFETCH3_SW_RST, reg_global_cfg_qm_sw_rst_ctrl, reset_ctrl->fpm_prefetch3_sw_rst);
    reg_global_cfg_qm_sw_rst_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, NORMAL_RMT_SW_RST, reg_global_cfg_qm_sw_rst_ctrl, reset_ctrl->normal_rmt_sw_rst);
    reg_global_cfg_qm_sw_rst_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, NON_DELAYED_RMT_SW_RST, reg_global_cfg_qm_sw_rst_ctrl, reset_ctrl->non_delayed_rmt_sw_rst);
    reg_global_cfg_qm_sw_rst_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, PRE_CM_FIFO_SW_RST, reg_global_cfg_qm_sw_rst_ctrl, reset_ctrl->pre_cm_fifo_sw_rst);
    reg_global_cfg_qm_sw_rst_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, CM_RD_PD_FIFO_SW_RST, reg_global_cfg_qm_sw_rst_ctrl, reset_ctrl->cm_rd_pd_fifo_sw_rst);
    reg_global_cfg_qm_sw_rst_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, CM_WR_PD_FIFO_SW_RST, reg_global_cfg_qm_sw_rst_ctrl, reset_ctrl->cm_wr_pd_fifo_sw_rst);
    reg_global_cfg_qm_sw_rst_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, BB0_OUTPUT_FIFO_SW_RST, reg_global_cfg_qm_sw_rst_ctrl, reset_ctrl->bb0_output_fifo_sw_rst);
    reg_global_cfg_qm_sw_rst_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, BB1_OUTPUT_FIFO_SW_RST, reg_global_cfg_qm_sw_rst_ctrl, reset_ctrl->bb1_output_fifo_sw_rst);
    reg_global_cfg_qm_sw_rst_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, BB1_INPUT_FIFO_SW_RST, reg_global_cfg_qm_sw_rst_ctrl, reset_ctrl->bb1_input_fifo_sw_rst);
    reg_global_cfg_qm_sw_rst_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, TM_FIFO_PTR_SW_RST, reg_global_cfg_qm_sw_rst_ctrl, reset_ctrl->tm_fifo_ptr_sw_rst);
    reg_global_cfg_qm_sw_rst_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, NON_DELAYED_OUT_FIFO_SW_RST, reg_global_cfg_qm_sw_rst_ctrl, reset_ctrl->non_delayed_out_fifo_sw_rst);
    reg_global_cfg_qm_sw_rst_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, BB0_EGR_MSG_OUT_FIFO_SW_RST, reg_global_cfg_qm_sw_rst_ctrl, reset_ctrl->bb0_egr_msg_out_fifo_sw_rst);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, reg_global_cfg_qm_sw_rst_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_reset_ctrl_get(qm_reset_ctrl *reset_ctrl)
{
    uint32_t reg_global_cfg_qm_sw_rst_ctrl;

#ifdef VALIDATE_PARMS
    if (!reset_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, reg_global_cfg_qm_sw_rst_ctrl);

    reset_ctrl->fpm_prefetch0_sw_rst = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, FPM_PREFETCH0_SW_RST, reg_global_cfg_qm_sw_rst_ctrl);
    reset_ctrl->fpm_prefetch1_sw_rst = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, FPM_PREFETCH1_SW_RST, reg_global_cfg_qm_sw_rst_ctrl);
    reset_ctrl->fpm_prefetch2_sw_rst = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, FPM_PREFETCH2_SW_RST, reg_global_cfg_qm_sw_rst_ctrl);
    reset_ctrl->fpm_prefetch3_sw_rst = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, FPM_PREFETCH3_SW_RST, reg_global_cfg_qm_sw_rst_ctrl);
    reset_ctrl->normal_rmt_sw_rst = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, NORMAL_RMT_SW_RST, reg_global_cfg_qm_sw_rst_ctrl);
    reset_ctrl->non_delayed_rmt_sw_rst = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, NON_DELAYED_RMT_SW_RST, reg_global_cfg_qm_sw_rst_ctrl);
    reset_ctrl->pre_cm_fifo_sw_rst = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, PRE_CM_FIFO_SW_RST, reg_global_cfg_qm_sw_rst_ctrl);
    reset_ctrl->cm_rd_pd_fifo_sw_rst = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, CM_RD_PD_FIFO_SW_RST, reg_global_cfg_qm_sw_rst_ctrl);
    reset_ctrl->cm_wr_pd_fifo_sw_rst = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, CM_WR_PD_FIFO_SW_RST, reg_global_cfg_qm_sw_rst_ctrl);
    reset_ctrl->bb0_output_fifo_sw_rst = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, BB0_OUTPUT_FIFO_SW_RST, reg_global_cfg_qm_sw_rst_ctrl);
    reset_ctrl->bb1_output_fifo_sw_rst = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, BB1_OUTPUT_FIFO_SW_RST, reg_global_cfg_qm_sw_rst_ctrl);
    reset_ctrl->bb1_input_fifo_sw_rst = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, BB1_INPUT_FIFO_SW_RST, reg_global_cfg_qm_sw_rst_ctrl);
    reset_ctrl->tm_fifo_ptr_sw_rst = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, TM_FIFO_PTR_SW_RST, reg_global_cfg_qm_sw_rst_ctrl);
    reset_ctrl->non_delayed_out_fifo_sw_rst = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, NON_DELAYED_OUT_FIFO_SW_RST, reg_global_cfg_qm_sw_rst_ctrl);
    reset_ctrl->bb0_egr_msg_out_fifo_sw_rst = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, BB0_EGR_MSG_OUT_FIFO_SW_RST, reg_global_cfg_qm_sw_rst_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_drop_counters_ctrl_set(const qm_drop_counters_ctrl *drop_counters_ctrl)
{
    uint32_t reg_global_cfg_qm_general_ctrl = 0;

#ifdef VALIDATE_PARMS
    if(!drop_counters_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((drop_counters_ctrl->read_clear_pkts >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->read_clear_bytes >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->disable_wrap_around_pkts >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->disable_wrap_around_bytes >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->free_with_context_last_search >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->wred_disable >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->ddr_pd_congestion_disable >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->ddr_byte_congestion_disable >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->ddr_occupancy_disable >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->ddr_fpm_congestion_disable >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->fpm_ug_disable >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->queue_occupancy_ddr_copy_decision_disable >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->psram_occupancy_ddr_copy_decision_disable >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->dont_send_mc_bit_to_bbh >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->close_aggregation_on_timeout_disable >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->fpm_congestion_buf_release_mechanism_disable >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->fpm_buffer_global_res_enable >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->qm_preserve_pd_with_fpm >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->qm_residue_per_queue >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->ghost_rpt_update_after_close_agg_en >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->fpm_ug_flow_ctrl_disable >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->ddr_write_multi_slave_en >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->ddr_pd_congestion_agg_priority >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->psram_occupancy_drop_disable >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->qm_ddr_write_alignment >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->exclusive_dont_drop >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->dqmol_jira_973_fix_enable >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->gpon_dbr_ceil >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->drop_cnt_wred_drops >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->same_sec_lvl_bit_agg_en >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->glbl_egr_drop_cnt_read_clear_enable >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->glbl_egr_aqm_drop_cnt_read_clear_enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DROP_CNT_PKTS_READ_CLEAR_ENABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->read_clear_pkts);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DROP_CNT_BYTES_READ_CLEAR_ENABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->read_clear_bytes);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->disable_wrap_around_pkts);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->disable_wrap_around_bytes);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, FREE_WITH_CONTEXT_LAST_SEARCH, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->free_with_context_last_search);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, WRED_DISABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->wred_disable);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DDR_PD_CONGESTION_DISABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->ddr_pd_congestion_disable);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DDR_BYTE_CONGESTION_DISABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->ddr_byte_congestion_disable);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DDR_OCCUPANCY_DISABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->ddr_occupancy_disable);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DDR_FPM_CONGESTION_DISABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->ddr_fpm_congestion_disable);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, FPM_UG_DISABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->fpm_ug_disable);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->queue_occupancy_ddr_copy_decision_disable);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->psram_occupancy_ddr_copy_decision_disable);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DONT_SEND_MC_BIT_TO_BBH, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->dont_send_mc_bit_to_bbh);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->close_aggregation_on_timeout_disable);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->fpm_congestion_buf_release_mechanism_disable);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, FPM_BUFFER_GLOBAL_RES_ENABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->fpm_buffer_global_res_enable);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, QM_PRESERVE_PD_WITH_FPM, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->qm_preserve_pd_with_fpm);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, QM_RESIDUE_PER_QUEUE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->qm_residue_per_queue);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->ghost_rpt_update_after_close_agg_en);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, FPM_UG_FLOW_CTRL_DISABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->fpm_ug_flow_ctrl_disable);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DDR_WRITE_MULTI_SLAVE_EN, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->ddr_write_multi_slave_en);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DDR_PD_CONGESTION_AGG_PRIORITY, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->ddr_pd_congestion_agg_priority);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, PSRAM_OCCUPANCY_DROP_DISABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->psram_occupancy_drop_disable);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, QM_DDR_WRITE_ALIGNMENT, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->qm_ddr_write_alignment);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, EXCLUSIVE_DONT_DROP, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->exclusive_dont_drop);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DQMOL_JIRA_973_FIX_ENABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->dqmol_jira_973_fix_enable);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, GPON_DBR_CEIL, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->gpon_dbr_ceil);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DROP_CNT_WRED_DROPS, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->drop_cnt_wred_drops);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, SAME_SEC_LVL_BIT_AGG_EN, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->same_sec_lvl_bit_agg_en);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, GLBL_EGR_DROP_CNT_READ_CLEAR_ENABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->glbl_egr_drop_cnt_read_clear_enable);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, GLBL_EGR_AQM_DROP_CNT_READ_CLEAR_ENABLE, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->glbl_egr_aqm_drop_cnt_read_clear_enable);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, reg_global_cfg_qm_general_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_drop_counters_ctrl_get(qm_drop_counters_ctrl *drop_counters_ctrl)
{
    uint32_t reg_global_cfg_qm_general_ctrl;

#ifdef VALIDATE_PARMS
    if (!drop_counters_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, reg_global_cfg_qm_general_ctrl);

    drop_counters_ctrl->read_clear_pkts = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DROP_CNT_PKTS_READ_CLEAR_ENABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->read_clear_bytes = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DROP_CNT_BYTES_READ_CLEAR_ENABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->disable_wrap_around_pkts = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->disable_wrap_around_bytes = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->free_with_context_last_search = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, FREE_WITH_CONTEXT_LAST_SEARCH, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->wred_disable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, WRED_DISABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->ddr_pd_congestion_disable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DDR_PD_CONGESTION_DISABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->ddr_byte_congestion_disable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DDR_BYTE_CONGESTION_DISABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->ddr_occupancy_disable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DDR_OCCUPANCY_DISABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->ddr_fpm_congestion_disable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DDR_FPM_CONGESTION_DISABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->fpm_ug_disable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, FPM_UG_DISABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->queue_occupancy_ddr_copy_decision_disable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->psram_occupancy_ddr_copy_decision_disable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->dont_send_mc_bit_to_bbh = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DONT_SEND_MC_BIT_TO_BBH, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->close_aggregation_on_timeout_disable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->fpm_congestion_buf_release_mechanism_disable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->fpm_buffer_global_res_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, FPM_BUFFER_GLOBAL_RES_ENABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->qm_preserve_pd_with_fpm = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, QM_PRESERVE_PD_WITH_FPM, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->qm_residue_per_queue = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, QM_RESIDUE_PER_QUEUE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->ghost_rpt_update_after_close_agg_en = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->fpm_ug_flow_ctrl_disable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, FPM_UG_FLOW_CTRL_DISABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->ddr_write_multi_slave_en = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DDR_WRITE_MULTI_SLAVE_EN, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->ddr_pd_congestion_agg_priority = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DDR_PD_CONGESTION_AGG_PRIORITY, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->psram_occupancy_drop_disable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, PSRAM_OCCUPANCY_DROP_DISABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->qm_ddr_write_alignment = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, QM_DDR_WRITE_ALIGNMENT, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->exclusive_dont_drop = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, EXCLUSIVE_DONT_DROP, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->dqmol_jira_973_fix_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DQMOL_JIRA_973_FIX_ENABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->gpon_dbr_ceil = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, GPON_DBR_CEIL, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->drop_cnt_wred_drops = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DROP_CNT_WRED_DROPS, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->same_sec_lvl_bit_agg_en = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, SAME_SEC_LVL_BIT_AGG_EN, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->glbl_egr_drop_cnt_read_clear_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, GLBL_EGR_DROP_CNT_READ_CLEAR_ENABLE, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->glbl_egr_aqm_drop_cnt_read_clear_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, GLBL_EGR_AQM_DROP_CNT_READ_CLEAR_ENABLE, reg_global_cfg_qm_general_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_ctrl_set(const qm_fpm_ctrl *fpm_ctrl)
{
    uint32_t reg_global_cfg_fpm_control = 0;

#ifdef VALIDATE_PARMS
    if(!fpm_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((fpm_ctrl->fpm_pool_bp_enable >= _1BITS_MAX_VAL_) ||
       (fpm_ctrl->fpm_congestion_bp_enable >= _1BITS_MAX_VAL_) ||
       (fpm_ctrl->fpm_force_bp_lvl >= _5BITS_MAX_VAL_) ||
       (fpm_ctrl->fpm_prefetch_granularity >= _1BITS_MAX_VAL_) ||
       (fpm_ctrl->fpm_prefetch_min_pool_size >= _2BITS_MAX_VAL_) ||
       (fpm_ctrl->fpm_prefetch_pending_req_limit >= _7BITS_MAX_VAL_) ||
       (fpm_ctrl->fpm_override_bb_id_en >= _1BITS_MAX_VAL_) ||
       (fpm_ctrl->fpm_override_bb_id_value >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_fpm_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_POOL_BP_ENABLE, reg_global_cfg_fpm_control, fpm_ctrl->fpm_pool_bp_enable);
    reg_global_cfg_fpm_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_CONGESTION_BP_ENABLE, reg_global_cfg_fpm_control, fpm_ctrl->fpm_congestion_bp_enable);
    reg_global_cfg_fpm_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_FORCE_BP_LVL, reg_global_cfg_fpm_control, fpm_ctrl->fpm_force_bp_lvl);
    reg_global_cfg_fpm_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_PREFETCH_GRANULARITY, reg_global_cfg_fpm_control, fpm_ctrl->fpm_prefetch_granularity);
    reg_global_cfg_fpm_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_PREFETCH_MIN_POOL_SIZE, reg_global_cfg_fpm_control, fpm_ctrl->fpm_prefetch_min_pool_size);
    reg_global_cfg_fpm_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_PREFETCH_PENDING_REQ_LIMIT, reg_global_cfg_fpm_control, fpm_ctrl->fpm_prefetch_pending_req_limit);
    reg_global_cfg_fpm_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_OVERRIDE_BB_ID_EN, reg_global_cfg_fpm_control, fpm_ctrl->fpm_override_bb_id_en);
    reg_global_cfg_fpm_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_OVERRIDE_BB_ID_VALUE, reg_global_cfg_fpm_control, fpm_ctrl->fpm_override_bb_id_value);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_FPM_CONTROL, reg_global_cfg_fpm_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_ctrl_get(qm_fpm_ctrl *fpm_ctrl)
{
    uint32_t reg_global_cfg_fpm_control;

#ifdef VALIDATE_PARMS
    if (!fpm_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_FPM_CONTROL, reg_global_cfg_fpm_control);

    fpm_ctrl->fpm_pool_bp_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_POOL_BP_ENABLE, reg_global_cfg_fpm_control);
    fpm_ctrl->fpm_congestion_bp_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_CONGESTION_BP_ENABLE, reg_global_cfg_fpm_control);
    fpm_ctrl->fpm_force_bp_lvl = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_FORCE_BP_LVL, reg_global_cfg_fpm_control);
    fpm_ctrl->fpm_prefetch_granularity = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_PREFETCH_GRANULARITY, reg_global_cfg_fpm_control);
    fpm_ctrl->fpm_prefetch_min_pool_size = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_PREFETCH_MIN_POOL_SIZE, reg_global_cfg_fpm_control);
    fpm_ctrl->fpm_prefetch_pending_req_limit = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_PREFETCH_PENDING_REQ_LIMIT, reg_global_cfg_fpm_control);
    fpm_ctrl->fpm_override_bb_id_en = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_OVERRIDE_BB_ID_EN, reg_global_cfg_fpm_control);
    fpm_ctrl->fpm_override_bb_id_value = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_OVERRIDE_BB_ID_VALUE, reg_global_cfg_fpm_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_pd_cong_ctrl_set(uint32_t total_pd_thr)
{
    uint32_t reg_global_cfg_qm_pd_congestion_control = 0;

#ifdef VALIDATE_PARMS
    if ((total_pd_thr >= _28BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_qm_pd_congestion_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_PD_CONGESTION_CONTROL, TOTAL_PD_THR, reg_global_cfg_qm_pd_congestion_control, total_pd_thr);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_QM_PD_CONGESTION_CONTROL, reg_global_cfg_qm_pd_congestion_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_pd_cong_ctrl_get(uint32_t *total_pd_thr)
{
    uint32_t reg_global_cfg_qm_pd_congestion_control;

#ifdef VALIDATE_PARMS
    if (!total_pd_thr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_QM_PD_CONGESTION_CONTROL, reg_global_cfg_qm_pd_congestion_control);

    *total_pd_thr = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_PD_CONGESTION_CONTROL, TOTAL_PD_THR, reg_global_cfg_qm_pd_congestion_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_abs_drop_queue_set(uint16_t abs_drop_queue, bdmf_boolean abs_drop_queue_en)
{
    uint32_t reg_global_cfg_abs_drop_queue = 0;

#ifdef VALIDATE_PARMS
    if ((abs_drop_queue >= _9BITS_MAX_VAL_) ||
       (abs_drop_queue_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_abs_drop_queue = RU_FIELD_SET(0, QM, GLOBAL_CFG_ABS_DROP_QUEUE, ABS_DROP_QUEUE, reg_global_cfg_abs_drop_queue, abs_drop_queue);
    reg_global_cfg_abs_drop_queue = RU_FIELD_SET(0, QM, GLOBAL_CFG_ABS_DROP_QUEUE, ABS_DROP_QUEUE_EN, reg_global_cfg_abs_drop_queue, abs_drop_queue_en);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_ABS_DROP_QUEUE, reg_global_cfg_abs_drop_queue);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_abs_drop_queue_get(uint16_t *abs_drop_queue, bdmf_boolean *abs_drop_queue_en)
{
    uint32_t reg_global_cfg_abs_drop_queue;

#ifdef VALIDATE_PARMS
    if (!abs_drop_queue || !abs_drop_queue_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_ABS_DROP_QUEUE, reg_global_cfg_abs_drop_queue);

    *abs_drop_queue = RU_FIELD_GET(0, QM, GLOBAL_CFG_ABS_DROP_QUEUE, ABS_DROP_QUEUE, reg_global_cfg_abs_drop_queue);
    *abs_drop_queue_en = RU_FIELD_GET(0, QM, GLOBAL_CFG_ABS_DROP_QUEUE, ABS_DROP_QUEUE_EN, reg_global_cfg_abs_drop_queue);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl_set(const qm_global_cfg_aggregation_ctrl *global_cfg_aggregation_ctrl)
{
    uint32_t reg_global_cfg_aggregation_ctrl = 0;

#ifdef VALIDATE_PARMS
    if(!global_cfg_aggregation_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((global_cfg_aggregation_ctrl->max_agg_bytes >= _10BITS_MAX_VAL_) ||
       (global_cfg_aggregation_ctrl->max_agg_pkts >= _2BITS_MAX_VAL_) ||
       (global_cfg_aggregation_ctrl->agg_ovr_512b_en >= _1BITS_MAX_VAL_) ||
       (global_cfg_aggregation_ctrl->max_agg_pkt_size >= _10BITS_MAX_VAL_) ||
       (global_cfg_aggregation_ctrl->min_agg_pkt_size >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_aggregation_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, MAX_AGG_BYTES, reg_global_cfg_aggregation_ctrl, global_cfg_aggregation_ctrl->max_agg_bytes);
    reg_global_cfg_aggregation_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, MAX_AGG_PKTS, reg_global_cfg_aggregation_ctrl, global_cfg_aggregation_ctrl->max_agg_pkts);
    reg_global_cfg_aggregation_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, AGG_OVR_512B_EN, reg_global_cfg_aggregation_ctrl, global_cfg_aggregation_ctrl->agg_ovr_512b_en);
    reg_global_cfg_aggregation_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, MAX_AGG_PKT_SIZE, reg_global_cfg_aggregation_ctrl, global_cfg_aggregation_ctrl->max_agg_pkt_size);
    reg_global_cfg_aggregation_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, MIN_AGG_PKT_SIZE, reg_global_cfg_aggregation_ctrl, global_cfg_aggregation_ctrl->min_agg_pkt_size);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, reg_global_cfg_aggregation_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl_get(qm_global_cfg_aggregation_ctrl *global_cfg_aggregation_ctrl)
{
    uint32_t reg_global_cfg_aggregation_ctrl;

#ifdef VALIDATE_PARMS
    if (!global_cfg_aggregation_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, reg_global_cfg_aggregation_ctrl);

    global_cfg_aggregation_ctrl->max_agg_bytes = RU_FIELD_GET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, MAX_AGG_BYTES, reg_global_cfg_aggregation_ctrl);
    global_cfg_aggregation_ctrl->max_agg_pkts = RU_FIELD_GET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, MAX_AGG_PKTS, reg_global_cfg_aggregation_ctrl);
    global_cfg_aggregation_ctrl->agg_ovr_512b_en = RU_FIELD_GET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, AGG_OVR_512B_EN, reg_global_cfg_aggregation_ctrl);
    global_cfg_aggregation_ctrl->max_agg_pkt_size = RU_FIELD_GET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, MAX_AGG_PKT_SIZE, reg_global_cfg_aggregation_ctrl);
    global_cfg_aggregation_ctrl->min_agg_pkt_size = RU_FIELD_GET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, MIN_AGG_PKT_SIZE, reg_global_cfg_aggregation_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl2_set(bdmf_boolean agg_pool_sel_en, uint8_t agg_pool_sel)
{
    uint32_t reg_global_cfg_aggregation_ctrl2 = 0;

#ifdef VALIDATE_PARMS
    if ((agg_pool_sel_en >= _1BITS_MAX_VAL_) ||
       (agg_pool_sel >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_aggregation_ctrl2 = RU_FIELD_SET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL2, AGG_POOL_SEL_EN, reg_global_cfg_aggregation_ctrl2, agg_pool_sel_en);
    reg_global_cfg_aggregation_ctrl2 = RU_FIELD_SET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL2, AGG_POOL_SEL, reg_global_cfg_aggregation_ctrl2, agg_pool_sel);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_AGGREGATION_CTRL2, reg_global_cfg_aggregation_ctrl2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl2_get(bdmf_boolean *agg_pool_sel_en, uint8_t *agg_pool_sel)
{
    uint32_t reg_global_cfg_aggregation_ctrl2;

#ifdef VALIDATE_PARMS
    if (!agg_pool_sel_en || !agg_pool_sel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_AGGREGATION_CTRL2, reg_global_cfg_aggregation_ctrl2);

    *agg_pool_sel_en = RU_FIELD_GET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL2, AGG_POOL_SEL_EN, reg_global_cfg_aggregation_ctrl2);
    *agg_pool_sel = RU_FIELD_GET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL2, AGG_POOL_SEL, reg_global_cfg_aggregation_ctrl2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_base_addr_set(uint32_t fpm_base_addr)
{
    uint32_t reg_global_cfg_fpm_base_addr = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_global_cfg_fpm_base_addr = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_BASE_ADDR, FPM_BASE_ADDR, reg_global_cfg_fpm_base_addr, fpm_base_addr);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_FPM_BASE_ADDR, reg_global_cfg_fpm_base_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_base_addr_get(uint32_t *fpm_base_addr)
{
    uint32_t reg_global_cfg_fpm_base_addr;

#ifdef VALIDATE_PARMS
    if (!fpm_base_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_FPM_BASE_ADDR, reg_global_cfg_fpm_base_addr);

    *fpm_base_addr = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_BASE_ADDR, FPM_BASE_ADDR, reg_global_cfg_fpm_base_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_fpm_coherent_base_addr_set(uint32_t fpm_base_addr)
{
    uint32_t reg_global_cfg_fpm_coherent_base_addr = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_global_cfg_fpm_coherent_base_addr = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_COHERENT_BASE_ADDR, FPM_BASE_ADDR, reg_global_cfg_fpm_coherent_base_addr, fpm_base_addr);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_FPM_COHERENT_BASE_ADDR, reg_global_cfg_fpm_coherent_base_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_fpm_coherent_base_addr_get(uint32_t *fpm_base_addr)
{
    uint32_t reg_global_cfg_fpm_coherent_base_addr;

#ifdef VALIDATE_PARMS
    if (!fpm_base_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_FPM_COHERENT_BASE_ADDR, reg_global_cfg_fpm_coherent_base_addr);

    *fpm_base_addr = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_COHERENT_BASE_ADDR, FPM_BASE_ADDR, reg_global_cfg_fpm_coherent_base_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_ddr_sop_offset_set(uint16_t ddr_sop_offset0, uint16_t ddr_sop_offset1)
{
    uint32_t reg_global_cfg_ddr_sop_offset = 0;

#ifdef VALIDATE_PARMS
    if ((ddr_sop_offset0 >= _11BITS_MAX_VAL_) ||
       (ddr_sop_offset1 >= _11BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_ddr_sop_offset = RU_FIELD_SET(0, QM, GLOBAL_CFG_DDR_SOP_OFFSET, DDR_SOP_OFFSET0, reg_global_cfg_ddr_sop_offset, ddr_sop_offset0);
    reg_global_cfg_ddr_sop_offset = RU_FIELD_SET(0, QM, GLOBAL_CFG_DDR_SOP_OFFSET, DDR_SOP_OFFSET1, reg_global_cfg_ddr_sop_offset, ddr_sop_offset1);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_DDR_SOP_OFFSET, reg_global_cfg_ddr_sop_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_ddr_sop_offset_get(uint16_t *ddr_sop_offset0, uint16_t *ddr_sop_offset1)
{
    uint32_t reg_global_cfg_ddr_sop_offset;

#ifdef VALIDATE_PARMS
    if (!ddr_sop_offset0 || !ddr_sop_offset1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_DDR_SOP_OFFSET, reg_global_cfg_ddr_sop_offset);

    *ddr_sop_offset0 = RU_FIELD_GET(0, QM, GLOBAL_CFG_DDR_SOP_OFFSET, DDR_SOP_OFFSET0, reg_global_cfg_ddr_sop_offset);
    *ddr_sop_offset1 = RU_FIELD_GET(0, QM, GLOBAL_CFG_DDR_SOP_OFFSET, DDR_SOP_OFFSET1, reg_global_cfg_ddr_sop_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_epon_overhead_ctrl_set(const qm_epon_overhead_ctrl *epon_overhead_ctrl)
{
    uint32_t reg_global_cfg_epon_overhead_ctrl = 0;

#ifdef VALIDATE_PARMS
    if(!epon_overhead_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((epon_overhead_ctrl->epon_line_rate >= _1BITS_MAX_VAL_) ||
       (epon_overhead_ctrl->epon_crc_add_disable >= _1BITS_MAX_VAL_) ||
       (epon_overhead_ctrl->mac_flow_overwrite_crc_en >= _1BITS_MAX_VAL_) ||
       (epon_overhead_ctrl->fec_ipg_length >= _11BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_epon_overhead_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_EPON_OVERHEAD_CTRL, EPON_LINE_RATE, reg_global_cfg_epon_overhead_ctrl, epon_overhead_ctrl->epon_line_rate);
    reg_global_cfg_epon_overhead_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_EPON_OVERHEAD_CTRL, EPON_CRC_ADD_DISABLE, reg_global_cfg_epon_overhead_ctrl, epon_overhead_ctrl->epon_crc_add_disable);
    reg_global_cfg_epon_overhead_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_EPON_OVERHEAD_CTRL, MAC_FLOW_OVERWRITE_CRC_EN, reg_global_cfg_epon_overhead_ctrl, epon_overhead_ctrl->mac_flow_overwrite_crc_en);
    reg_global_cfg_epon_overhead_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_EPON_OVERHEAD_CTRL, MAC_FLOW_OVERWRITE_CRC, reg_global_cfg_epon_overhead_ctrl, epon_overhead_ctrl->mac_flow_overwrite_crc);
    reg_global_cfg_epon_overhead_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_EPON_OVERHEAD_CTRL, FEC_IPG_LENGTH, reg_global_cfg_epon_overhead_ctrl, epon_overhead_ctrl->fec_ipg_length);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_EPON_OVERHEAD_CTRL, reg_global_cfg_epon_overhead_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_epon_overhead_ctrl_get(qm_epon_overhead_ctrl *epon_overhead_ctrl)
{
    uint32_t reg_global_cfg_epon_overhead_ctrl;

#ifdef VALIDATE_PARMS
    if (!epon_overhead_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_EPON_OVERHEAD_CTRL, reg_global_cfg_epon_overhead_ctrl);

    epon_overhead_ctrl->epon_line_rate = RU_FIELD_GET(0, QM, GLOBAL_CFG_EPON_OVERHEAD_CTRL, EPON_LINE_RATE, reg_global_cfg_epon_overhead_ctrl);
    epon_overhead_ctrl->epon_crc_add_disable = RU_FIELD_GET(0, QM, GLOBAL_CFG_EPON_OVERHEAD_CTRL, EPON_CRC_ADD_DISABLE, reg_global_cfg_epon_overhead_ctrl);
    epon_overhead_ctrl->mac_flow_overwrite_crc_en = RU_FIELD_GET(0, QM, GLOBAL_CFG_EPON_OVERHEAD_CTRL, MAC_FLOW_OVERWRITE_CRC_EN, reg_global_cfg_epon_overhead_ctrl);
    epon_overhead_ctrl->mac_flow_overwrite_crc = RU_FIELD_GET(0, QM, GLOBAL_CFG_EPON_OVERHEAD_CTRL, MAC_FLOW_OVERWRITE_CRC, reg_global_cfg_epon_overhead_ctrl);
    epon_overhead_ctrl->fec_ipg_length = RU_FIELD_GET(0, QM, GLOBAL_CFG_EPON_OVERHEAD_CTRL, FEC_IPG_LENGTH, reg_global_cfg_epon_overhead_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_bbhtx_fifo_addr_set(uint8_t addr, uint8_t bbhtx_req_otf)
{
    uint32_t reg_global_cfg_bbhtx_fifo_addr = 0;

#ifdef VALIDATE_PARMS
    if ((addr >= _6BITS_MAX_VAL_) ||
       (bbhtx_req_otf >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_bbhtx_fifo_addr = RU_FIELD_SET(0, QM, GLOBAL_CFG_BBHTX_FIFO_ADDR, ADDR, reg_global_cfg_bbhtx_fifo_addr, addr);
    reg_global_cfg_bbhtx_fifo_addr = RU_FIELD_SET(0, QM, GLOBAL_CFG_BBHTX_FIFO_ADDR, BBHTX_REQ_OTF, reg_global_cfg_bbhtx_fifo_addr, bbhtx_req_otf);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_BBHTX_FIFO_ADDR, reg_global_cfg_bbhtx_fifo_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_bbhtx_fifo_addr_get(uint8_t *addr, uint8_t *bbhtx_req_otf)
{
    uint32_t reg_global_cfg_bbhtx_fifo_addr;

#ifdef VALIDATE_PARMS
    if (!addr || !bbhtx_req_otf)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_BBHTX_FIFO_ADDR, reg_global_cfg_bbhtx_fifo_addr);

    *addr = RU_FIELD_GET(0, QM, GLOBAL_CFG_BBHTX_FIFO_ADDR, ADDR, reg_global_cfg_bbhtx_fifo_addr);
    *bbhtx_req_otf = RU_FIELD_GET(0, QM, GLOBAL_CFG_BBHTX_FIFO_ADDR, BBHTX_REQ_OTF, reg_global_cfg_bbhtx_fifo_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_egress_flush_queue_set(uint16_t queue_num, bdmf_boolean flush_en)
{
    uint32_t reg_global_cfg_qm_egress_flush_queue = 0;

#ifdef VALIDATE_PARMS
    if ((queue_num >= _9BITS_MAX_VAL_) ||
       (flush_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_qm_egress_flush_queue = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE, QUEUE_NUM, reg_global_cfg_qm_egress_flush_queue, queue_num);
    reg_global_cfg_qm_egress_flush_queue = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE, FLUSH_EN, reg_global_cfg_qm_egress_flush_queue, flush_en);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE, reg_global_cfg_qm_egress_flush_queue);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_egress_flush_queue_get(uint16_t *queue_num, bdmf_boolean *flush_en)
{
    uint32_t reg_global_cfg_qm_egress_flush_queue;

#ifdef VALIDATE_PARMS
    if (!queue_num || !flush_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE, reg_global_cfg_qm_egress_flush_queue);

    *queue_num = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE, QUEUE_NUM, reg_global_cfg_qm_egress_flush_queue);
    *flush_en = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE, FLUSH_EN, reg_global_cfg_qm_egress_flush_queue);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set(uint8_t prescaler_granularity, uint8_t aggregation_timeout_value, bdmf_boolean pd_occupancy_en, uint8_t pd_occupancy_value)
{
    uint32_t reg_global_cfg_qm_aggregation_timer_ctrl = 0;

#ifdef VALIDATE_PARMS
    if ((prescaler_granularity >= _3BITS_MAX_VAL_) ||
       (aggregation_timeout_value >= _3BITS_MAX_VAL_) ||
       (pd_occupancy_en >= _1BITS_MAX_VAL_) ||
       (pd_occupancy_value >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_qm_aggregation_timer_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, PRESCALER_GRANULARITY, reg_global_cfg_qm_aggregation_timer_ctrl, prescaler_granularity);
    reg_global_cfg_qm_aggregation_timer_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, AGGREGATION_TIMEOUT_VALUE, reg_global_cfg_qm_aggregation_timer_ctrl, aggregation_timeout_value);
    reg_global_cfg_qm_aggregation_timer_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, PD_OCCUPANCY_EN, reg_global_cfg_qm_aggregation_timer_ctrl, pd_occupancy_en);
    reg_global_cfg_qm_aggregation_timer_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, PD_OCCUPANCY_VALUE, reg_global_cfg_qm_aggregation_timer_ctrl, pd_occupancy_value);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, reg_global_cfg_qm_aggregation_timer_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_get(uint8_t *prescaler_granularity, uint8_t *aggregation_timeout_value, bdmf_boolean *pd_occupancy_en, uint8_t *pd_occupancy_value)
{
    uint32_t reg_global_cfg_qm_aggregation_timer_ctrl;

#ifdef VALIDATE_PARMS
    if (!prescaler_granularity || !aggregation_timeout_value || !pd_occupancy_en || !pd_occupancy_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, reg_global_cfg_qm_aggregation_timer_ctrl);

    *prescaler_granularity = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, PRESCALER_GRANULARITY, reg_global_cfg_qm_aggregation_timer_ctrl);
    *aggregation_timeout_value = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, AGGREGATION_TIMEOUT_VALUE, reg_global_cfg_qm_aggregation_timer_ctrl);
    *pd_occupancy_en = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, PD_OCCUPANCY_EN, reg_global_cfg_qm_aggregation_timer_ctrl);
    *pd_occupancy_value = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, PD_OCCUPANCY_VALUE, reg_global_cfg_qm_aggregation_timer_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_set(uint32_t fpm_gbl_cnt)
{
    uint32_t reg_global_cfg_qm_fpm_ug_gbl_cnt = 0;

#ifdef VALIDATE_PARMS
    if ((fpm_gbl_cnt >= _18BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_qm_fpm_ug_gbl_cnt = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_FPM_UG_GBL_CNT, FPM_GBL_CNT, reg_global_cfg_qm_fpm_ug_gbl_cnt, fpm_gbl_cnt);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_QM_FPM_UG_GBL_CNT, reg_global_cfg_qm_fpm_ug_gbl_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_get(uint32_t *fpm_gbl_cnt)
{
    uint32_t reg_global_cfg_qm_fpm_ug_gbl_cnt;

#ifdef VALIDATE_PARMS
    if (!fpm_gbl_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_QM_FPM_UG_GBL_CNT, reg_global_cfg_qm_fpm_ug_gbl_cnt);

    *fpm_gbl_cnt = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_FPM_UG_GBL_CNT, FPM_GBL_CNT, reg_global_cfg_qm_fpm_ug_gbl_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_ddr_spare_room_set(uint16_t pair_idx, uint16_t ddr_headroom, uint16_t ddr_tailroom)
{
    uint32_t reg_global_cfg_ddr_spare_room = 0;

#ifdef VALIDATE_PARMS
    if ((pair_idx >= 4) ||
       (ddr_headroom >= _10BITS_MAX_VAL_) ||
       (ddr_tailroom >= _11BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_ddr_spare_room = RU_FIELD_SET(0, QM, GLOBAL_CFG_DDR_SPARE_ROOM, DDR_HEADROOM, reg_global_cfg_ddr_spare_room, ddr_headroom);
    reg_global_cfg_ddr_spare_room = RU_FIELD_SET(0, QM, GLOBAL_CFG_DDR_SPARE_ROOM, DDR_TAILROOM, reg_global_cfg_ddr_spare_room, ddr_tailroom);

    RU_REG_RAM_WRITE(0, pair_idx, QM, GLOBAL_CFG_DDR_SPARE_ROOM, reg_global_cfg_ddr_spare_room);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_ddr_spare_room_get(uint16_t pair_idx, uint16_t *ddr_headroom, uint16_t *ddr_tailroom)
{
    uint32_t reg_global_cfg_ddr_spare_room;

#ifdef VALIDATE_PARMS
    if (!ddr_headroom || !ddr_tailroom)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pair_idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, pair_idx, QM, GLOBAL_CFG_DDR_SPARE_ROOM, reg_global_cfg_ddr_spare_room);

    *ddr_headroom = RU_FIELD_GET(0, QM, GLOBAL_CFG_DDR_SPARE_ROOM, DDR_HEADROOM, reg_global_cfg_ddr_spare_room);
    *ddr_tailroom = RU_FIELD_GET(0, QM, GLOBAL_CFG_DDR_SPARE_ROOM, DDR_TAILROOM, reg_global_cfg_ddr_spare_room);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_dummy_spare_room_profile_id_set(uint8_t dummy_profile_0, uint8_t dummy_profile_1)
{
    uint32_t reg_global_cfg_dummy_spare_room_profile_id = 0;

#ifdef VALIDATE_PARMS
    if ((dummy_profile_0 >= _2BITS_MAX_VAL_) ||
       (dummy_profile_1 >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_dummy_spare_room_profile_id = RU_FIELD_SET(0, QM, GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID, DUMMY_PROFILE_0, reg_global_cfg_dummy_spare_room_profile_id, dummy_profile_0);
    reg_global_cfg_dummy_spare_room_profile_id = RU_FIELD_SET(0, QM, GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID, DUMMY_PROFILE_1, reg_global_cfg_dummy_spare_room_profile_id, dummy_profile_1);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID, reg_global_cfg_dummy_spare_room_profile_id);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_dummy_spare_room_profile_id_get(uint8_t *dummy_profile_0, uint8_t *dummy_profile_1)
{
    uint32_t reg_global_cfg_dummy_spare_room_profile_id;

#ifdef VALIDATE_PARMS
    if (!dummy_profile_0 || !dummy_profile_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID, reg_global_cfg_dummy_spare_room_profile_id);

    *dummy_profile_0 = RU_FIELD_GET(0, QM, GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID, DUMMY_PROFILE_0, reg_global_cfg_dummy_spare_room_profile_id);
    *dummy_profile_1 = RU_FIELD_GET(0, QM, GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID, DUMMY_PROFILE_1, reg_global_cfg_dummy_spare_room_profile_id);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_dqm_ubus_ctrl_set(uint8_t tkn_reqout_h, uint8_t tkn_reqout_d, uint8_t offload_reqout_h, uint8_t offload_reqout_d)
{
    uint32_t reg_global_cfg_dqm_ubus_ctrl = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_global_cfg_dqm_ubus_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_DQM_UBUS_CTRL, TKN_REQOUT_H, reg_global_cfg_dqm_ubus_ctrl, tkn_reqout_h);
    reg_global_cfg_dqm_ubus_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_DQM_UBUS_CTRL, TKN_REQOUT_D, reg_global_cfg_dqm_ubus_ctrl, tkn_reqout_d);
    reg_global_cfg_dqm_ubus_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_DQM_UBUS_CTRL, OFFLOAD_REQOUT_H, reg_global_cfg_dqm_ubus_ctrl, offload_reqout_h);
    reg_global_cfg_dqm_ubus_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_DQM_UBUS_CTRL, OFFLOAD_REQOUT_D, reg_global_cfg_dqm_ubus_ctrl, offload_reqout_d);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_DQM_UBUS_CTRL, reg_global_cfg_dqm_ubus_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_dqm_ubus_ctrl_get(uint8_t *tkn_reqout_h, uint8_t *tkn_reqout_d, uint8_t *offload_reqout_h, uint8_t *offload_reqout_d)
{
    uint32_t reg_global_cfg_dqm_ubus_ctrl;

#ifdef VALIDATE_PARMS
    if (!tkn_reqout_h || !tkn_reqout_d || !offload_reqout_h || !offload_reqout_d)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_DQM_UBUS_CTRL, reg_global_cfg_dqm_ubus_ctrl);

    *tkn_reqout_h = RU_FIELD_GET(0, QM, GLOBAL_CFG_DQM_UBUS_CTRL, TKN_REQOUT_H, reg_global_cfg_dqm_ubus_ctrl);
    *tkn_reqout_d = RU_FIELD_GET(0, QM, GLOBAL_CFG_DQM_UBUS_CTRL, TKN_REQOUT_D, reg_global_cfg_dqm_ubus_ctrl);
    *offload_reqout_h = RU_FIELD_GET(0, QM, GLOBAL_CFG_DQM_UBUS_CTRL, OFFLOAD_REQOUT_H, reg_global_cfg_dqm_ubus_ctrl);
    *offload_reqout_d = RU_FIELD_GET(0, QM, GLOBAL_CFG_DQM_UBUS_CTRL, OFFLOAD_REQOUT_D, reg_global_cfg_dqm_ubus_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_mem_auto_init_set(bdmf_boolean mem_init_en, uint8_t mem_sel_init, uint8_t mem_size_init)
{
    uint32_t reg_global_cfg_mem_auto_init = 0;

#ifdef VALIDATE_PARMS
    if ((mem_init_en >= _1BITS_MAX_VAL_) ||
       (mem_sel_init >= _3BITS_MAX_VAL_) ||
       (mem_size_init >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_mem_auto_init = RU_FIELD_SET(0, QM, GLOBAL_CFG_MEM_AUTO_INIT, MEM_INIT_EN, reg_global_cfg_mem_auto_init, mem_init_en);
    reg_global_cfg_mem_auto_init = RU_FIELD_SET(0, QM, GLOBAL_CFG_MEM_AUTO_INIT, MEM_SEL_INIT, reg_global_cfg_mem_auto_init, mem_sel_init);
    reg_global_cfg_mem_auto_init = RU_FIELD_SET(0, QM, GLOBAL_CFG_MEM_AUTO_INIT, MEM_SIZE_INIT, reg_global_cfg_mem_auto_init, mem_size_init);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_MEM_AUTO_INIT, reg_global_cfg_mem_auto_init);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_mem_auto_init_get(bdmf_boolean *mem_init_en, uint8_t *mem_sel_init, uint8_t *mem_size_init)
{
    uint32_t reg_global_cfg_mem_auto_init;

#ifdef VALIDATE_PARMS
    if (!mem_init_en || !mem_sel_init || !mem_size_init)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_MEM_AUTO_INIT, reg_global_cfg_mem_auto_init);

    *mem_init_en = RU_FIELD_GET(0, QM, GLOBAL_CFG_MEM_AUTO_INIT, MEM_INIT_EN, reg_global_cfg_mem_auto_init);
    *mem_sel_init = RU_FIELD_GET(0, QM, GLOBAL_CFG_MEM_AUTO_INIT, MEM_SEL_INIT, reg_global_cfg_mem_auto_init);
    *mem_size_init = RU_FIELD_GET(0, QM, GLOBAL_CFG_MEM_AUTO_INIT, MEM_SIZE_INIT, reg_global_cfg_mem_auto_init);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_mem_auto_init_sts_get(bdmf_boolean *mem_init_done)
{
    uint32_t reg_global_cfg_mem_auto_init_sts;

#ifdef VALIDATE_PARMS
    if (!mem_init_done)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_MEM_AUTO_INIT_STS, reg_global_cfg_mem_auto_init_sts);

    *mem_init_done = RU_FIELD_GET(0, QM, GLOBAL_CFG_MEM_AUTO_INIT_STS, MEM_INIT_DONE, reg_global_cfg_mem_auto_init_sts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_size_tokens_set(uint8_t pool_0_num_of_tkns, uint8_t pool_1_num_of_tkns, uint8_t pool_2_num_of_tkns, uint8_t pool_3_num_of_tkns)
{
    uint32_t reg_global_cfg_fpm_mpm_enhancement_pool_size_tokens = 0;

#ifdef VALIDATE_PARMS
    if ((pool_0_num_of_tkns >= _6BITS_MAX_VAL_) ||
       (pool_1_num_of_tkns >= _6BITS_MAX_VAL_) ||
       (pool_2_num_of_tkns >= _6BITS_MAX_VAL_) ||
       (pool_3_num_of_tkns >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_fpm_mpm_enhancement_pool_size_tokens = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS, POOL_0_NUM_OF_TKNS, reg_global_cfg_fpm_mpm_enhancement_pool_size_tokens, pool_0_num_of_tkns);
    reg_global_cfg_fpm_mpm_enhancement_pool_size_tokens = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS, POOL_1_NUM_OF_TKNS, reg_global_cfg_fpm_mpm_enhancement_pool_size_tokens, pool_1_num_of_tkns);
    reg_global_cfg_fpm_mpm_enhancement_pool_size_tokens = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS, POOL_2_NUM_OF_TKNS, reg_global_cfg_fpm_mpm_enhancement_pool_size_tokens, pool_2_num_of_tkns);
    reg_global_cfg_fpm_mpm_enhancement_pool_size_tokens = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS, POOL_3_NUM_OF_TKNS, reg_global_cfg_fpm_mpm_enhancement_pool_size_tokens, pool_3_num_of_tkns);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS, reg_global_cfg_fpm_mpm_enhancement_pool_size_tokens);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_size_tokens_get(uint8_t *pool_0_num_of_tkns, uint8_t *pool_1_num_of_tkns, uint8_t *pool_2_num_of_tkns, uint8_t *pool_3_num_of_tkns)
{
    uint32_t reg_global_cfg_fpm_mpm_enhancement_pool_size_tokens;

#ifdef VALIDATE_PARMS
    if (!pool_0_num_of_tkns || !pool_1_num_of_tkns || !pool_2_num_of_tkns || !pool_3_num_of_tkns)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS, reg_global_cfg_fpm_mpm_enhancement_pool_size_tokens);

    *pool_0_num_of_tkns = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS, POOL_0_NUM_OF_TKNS, reg_global_cfg_fpm_mpm_enhancement_pool_size_tokens);
    *pool_1_num_of_tkns = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS, POOL_1_NUM_OF_TKNS, reg_global_cfg_fpm_mpm_enhancement_pool_size_tokens);
    *pool_2_num_of_tkns = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS, POOL_2_NUM_OF_TKNS, reg_global_cfg_fpm_mpm_enhancement_pool_size_tokens);
    *pool_3_num_of_tkns = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS, POOL_3_NUM_OF_TKNS, reg_global_cfg_fpm_mpm_enhancement_pool_size_tokens);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte_set(uint16_t pool_0_num_of_bytes, uint16_t pool_1_num_of_bytes)
{
    uint32_t reg_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte = 0;

#ifdef VALIDATE_PARMS
    if ((pool_0_num_of_bytes >= _14BITS_MAX_VAL_) ||
       (pool_1_num_of_bytes >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE, POOL_0_NUM_OF_BYTES, reg_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte, pool_0_num_of_bytes);
    reg_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE, POOL_1_NUM_OF_BYTES, reg_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte, pool_1_num_of_bytes);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE, reg_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte_get(uint16_t *pool_0_num_of_bytes, uint16_t *pool_1_num_of_bytes)
{
    uint32_t reg_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte;

#ifdef VALIDATE_PARMS
    if (!pool_0_num_of_bytes || !pool_1_num_of_bytes)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE, reg_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte);

    *pool_0_num_of_bytes = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE, POOL_0_NUM_OF_BYTES, reg_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte);
    *pool_1_num_of_bytes = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE, POOL_1_NUM_OF_BYTES, reg_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte_set(uint16_t pool_2_num_of_bytes, uint16_t pool_3_num_of_bytes)
{
    uint32_t reg_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte = 0;

#ifdef VALIDATE_PARMS
    if ((pool_2_num_of_bytes >= _14BITS_MAX_VAL_) ||
       (pool_3_num_of_bytes >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE, POOL_2_NUM_OF_BYTES, reg_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte, pool_2_num_of_bytes);
    reg_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE, POOL_3_NUM_OF_BYTES, reg_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte, pool_3_num_of_bytes);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE, reg_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte_get(uint16_t *pool_2_num_of_bytes, uint16_t *pool_3_num_of_bytes)
{
    uint32_t reg_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte;

#ifdef VALIDATE_PARMS
    if (!pool_2_num_of_bytes || !pool_3_num_of_bytes)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE, reg_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte);

    *pool_2_num_of_bytes = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE, POOL_2_NUM_OF_BYTES, reg_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte);
    *pool_3_num_of_bytes = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE, POOL_3_NUM_OF_BYTES, reg_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_mc_ctrl_set(uint8_t mc_headers_pool_sel)
{
    uint32_t reg_global_cfg_mc_ctrl = 0;

#ifdef VALIDATE_PARMS
    if ((mc_headers_pool_sel >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_mc_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_MC_CTRL, MC_HEADERS_POOL_SEL, reg_global_cfg_mc_ctrl, mc_headers_pool_sel);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_MC_CTRL, reg_global_cfg_mc_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_mc_ctrl_get(uint8_t *mc_headers_pool_sel)
{
    uint32_t reg_global_cfg_mc_ctrl;

#ifdef VALIDATE_PARMS
    if (!mc_headers_pool_sel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_MC_CTRL, reg_global_cfg_mc_ctrl);

    *mc_headers_pool_sel = RU_FIELD_GET(0, QM, GLOBAL_CFG_MC_CTRL, MC_HEADERS_POOL_SEL, reg_global_cfg_mc_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_aqm_clk_counter_cycle_set(uint16_t value)
{
    uint32_t reg_global_cfg_aqm_clk_counter_cycle = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_global_cfg_aqm_clk_counter_cycle = RU_FIELD_SET(0, QM, GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE, VALUE, reg_global_cfg_aqm_clk_counter_cycle, value);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE, reg_global_cfg_aqm_clk_counter_cycle);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_aqm_clk_counter_cycle_get(uint16_t *value)
{
    uint32_t reg_global_cfg_aqm_clk_counter_cycle;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE, reg_global_cfg_aqm_clk_counter_cycle);

    *value = RU_FIELD_GET(0, QM, GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE, VALUE, reg_global_cfg_aqm_clk_counter_cycle);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_aqm_push_to_empty_thr_set(uint8_t value)
{
    uint32_t reg_global_cfg_aqm_push_to_empty_thr = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_global_cfg_aqm_push_to_empty_thr = RU_FIELD_SET(0, QM, GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR, VALUE, reg_global_cfg_aqm_push_to_empty_thr, value);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR, reg_global_cfg_aqm_push_to_empty_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_aqm_push_to_empty_thr_get(uint8_t *value)
{
    uint32_t reg_global_cfg_aqm_push_to_empty_thr;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR, reg_global_cfg_aqm_push_to_empty_thr);

    *value = RU_FIELD_GET(0, QM, GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR, VALUE, reg_global_cfg_aqm_push_to_empty_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_general_ctrl2_set(const qm_global_cfg_qm_general_ctrl2 *global_cfg_qm_general_ctrl2)
{
    uint32_t reg_global_cfg_qm_general_ctrl2 = 0;

#ifdef VALIDATE_PARMS
    if(!global_cfg_qm_general_ctrl2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((global_cfg_qm_general_ctrl2->egress_accumulated_cnt_pkts_read_clear_enable >= _1BITS_MAX_VAL_) ||
       (global_cfg_qm_general_ctrl2->egress_accumulated_cnt_bytes_read_clear_enable >= _1BITS_MAX_VAL_) ||
       (global_cfg_qm_general_ctrl2->agg_closure_suspend_on_bp >= _1BITS_MAX_VAL_) ||
       (global_cfg_qm_general_ctrl2->bufmng_en_or_ug_cntr >= _1BITS_MAX_VAL_) ||
       (global_cfg_qm_general_ctrl2->dqm_to_fpm_ubus_or_fpmini >= _1BITS_MAX_VAL_) ||
       (global_cfg_qm_general_ctrl2->agg_closure_suspend_on_fpm_congestion_disable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_qm_general_ctrl2 = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL2, EGRESS_ACCUMULATED_CNT_PKTS_READ_CLEAR_ENABLE, reg_global_cfg_qm_general_ctrl2, global_cfg_qm_general_ctrl2->egress_accumulated_cnt_pkts_read_clear_enable);
    reg_global_cfg_qm_general_ctrl2 = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL2, EGRESS_ACCUMULATED_CNT_BYTES_READ_CLEAR_ENABLE, reg_global_cfg_qm_general_ctrl2, global_cfg_qm_general_ctrl2->egress_accumulated_cnt_bytes_read_clear_enable);
    reg_global_cfg_qm_general_ctrl2 = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL2, AGG_CLOSURE_SUSPEND_ON_BP, reg_global_cfg_qm_general_ctrl2, global_cfg_qm_general_ctrl2->agg_closure_suspend_on_bp);
    reg_global_cfg_qm_general_ctrl2 = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL2, BUFMNG_EN_OR_UG_CNTR, reg_global_cfg_qm_general_ctrl2, global_cfg_qm_general_ctrl2->bufmng_en_or_ug_cntr);
    reg_global_cfg_qm_general_ctrl2 = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL2, DQM_TO_FPM_UBUS_OR_FPMINI, reg_global_cfg_qm_general_ctrl2, global_cfg_qm_general_ctrl2->dqm_to_fpm_ubus_or_fpmini);
    reg_global_cfg_qm_general_ctrl2 = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL2, AGG_CLOSURE_SUSPEND_ON_FPM_CONGESTION_DISABLE, reg_global_cfg_qm_general_ctrl2, global_cfg_qm_general_ctrl2->agg_closure_suspend_on_fpm_congestion_disable);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL2, reg_global_cfg_qm_general_ctrl2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_general_ctrl2_get(qm_global_cfg_qm_general_ctrl2 *global_cfg_qm_general_ctrl2)
{
    uint32_t reg_global_cfg_qm_general_ctrl2;

#ifdef VALIDATE_PARMS
    if (!global_cfg_qm_general_ctrl2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL2, reg_global_cfg_qm_general_ctrl2);

    global_cfg_qm_general_ctrl2->egress_accumulated_cnt_pkts_read_clear_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL2, EGRESS_ACCUMULATED_CNT_PKTS_READ_CLEAR_ENABLE, reg_global_cfg_qm_general_ctrl2);
    global_cfg_qm_general_ctrl2->egress_accumulated_cnt_bytes_read_clear_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL2, EGRESS_ACCUMULATED_CNT_BYTES_READ_CLEAR_ENABLE, reg_global_cfg_qm_general_ctrl2);
    global_cfg_qm_general_ctrl2->agg_closure_suspend_on_bp = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL2, AGG_CLOSURE_SUSPEND_ON_BP, reg_global_cfg_qm_general_ctrl2);
    global_cfg_qm_general_ctrl2->bufmng_en_or_ug_cntr = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL2, BUFMNG_EN_OR_UG_CNTR, reg_global_cfg_qm_general_ctrl2);
    global_cfg_qm_general_ctrl2->dqm_to_fpm_ubus_or_fpmini = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL2, DQM_TO_FPM_UBUS_OR_FPMINI, reg_global_cfg_qm_general_ctrl2);
    global_cfg_qm_general_ctrl2->agg_closure_suspend_on_fpm_congestion_disable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL2, AGG_CLOSURE_SUSPEND_ON_FPM_CONGESTION_DISABLE, reg_global_cfg_qm_general_ctrl2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_pool_thr_set(uint8_t pool_idx, const qm_fpm_pool_thr *fpm_pool_thr)
{
    uint32_t reg_fpm_pools_thr = 0;

#ifdef VALIDATE_PARMS
    if(!fpm_pool_thr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pool_idx >= 4) ||
       (fpm_pool_thr->lower_thr >= _7BITS_MAX_VAL_) ||
       (fpm_pool_thr->higher_thr >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpm_pools_thr = RU_FIELD_SET(0, QM, FPM_POOLS_THR, FPM_LOWER_THR, reg_fpm_pools_thr, fpm_pool_thr->lower_thr);
    reg_fpm_pools_thr = RU_FIELD_SET(0, QM, FPM_POOLS_THR, FPM_HIGHER_THR, reg_fpm_pools_thr, fpm_pool_thr->higher_thr);

    RU_REG_RAM_WRITE(0, pool_idx, QM, FPM_POOLS_THR, reg_fpm_pools_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_pool_thr_get(uint8_t pool_idx, qm_fpm_pool_thr *fpm_pool_thr)
{
    uint32_t reg_fpm_pools_thr;

#ifdef VALIDATE_PARMS
    if (!fpm_pool_thr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pool_idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, pool_idx, QM, FPM_POOLS_THR, reg_fpm_pools_thr);

    fpm_pool_thr->lower_thr = RU_FIELD_GET(0, QM, FPM_POOLS_THR, FPM_LOWER_THR, reg_fpm_pools_thr);
    fpm_pool_thr->higher_thr = RU_FIELD_GET(0, QM, FPM_POOLS_THR, FPM_HIGHER_THR, reg_fpm_pools_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_ug_cnt_set(uint8_t grp_idx, uint32_t fpm_ug_cnt)
{
    uint32_t reg_fpm_usr_grp_cnt = 0;

#ifdef VALIDATE_PARMS
    if ((grp_idx >= 4) ||
       (fpm_ug_cnt >= _18BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpm_usr_grp_cnt = RU_FIELD_SET(0, QM, FPM_USR_GRP_CNT, FPM_UG_CNT, reg_fpm_usr_grp_cnt, fpm_ug_cnt);

    RU_REG_RAM_WRITE(0, grp_idx, QM, FPM_USR_GRP_CNT, reg_fpm_usr_grp_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_ug_cnt_get(uint8_t grp_idx, uint32_t *fpm_ug_cnt)
{
    uint32_t reg_fpm_usr_grp_cnt;

#ifdef VALIDATE_PARMS
    if (!fpm_ug_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((grp_idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, grp_idx, QM, FPM_USR_GRP_CNT, reg_fpm_usr_grp_cnt);

    *fpm_ug_cnt = RU_FIELD_GET(0, QM, FPM_USR_GRP_CNT, FPM_UG_CNT, reg_fpm_usr_grp_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_intr_ctrl_isr_set(const qm_intr_ctrl_isr *intr_ctrl_isr)
{
    uint32_t reg_intr_ctrl_isr = 0;

#ifdef VALIDATE_PARMS
    if(!intr_ctrl_isr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((intr_ctrl_isr->qm_dqm_pop_on_empty >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_dqm_push_on_full >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_cpu_pop_on_empty >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_cpu_push_on_full >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_normal_queue_pd_no_credit >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_non_delayed_queue_pd_no_credit >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_non_valid_queue >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_agg_coherent_inconsistency >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_force_copy_on_non_delayed >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_fpm_pool_size_nonexistent >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_target_mem_abs_contradiction >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_1588_drop >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_1588_multicast_contradiction >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_byte_drop_cnt_overrun >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_pkt_drop_cnt_overrun >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_total_byte_cnt_underrun >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_total_pkt_cnt_underrun >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_fpm_ug0_underrun >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_fpm_ug1_underrun >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_fpm_ug2_underrun >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_fpm_ug3_underrun >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_timer_wraparound >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_copy_plen_zero >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_ingress_bb_unexpected_msg >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_egress_bb_unexpected_msg >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->dqm_reached_full >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_fpmini_intr >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_DQM_POP_ON_EMPTY, reg_intr_ctrl_isr, intr_ctrl_isr->qm_dqm_pop_on_empty);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_DQM_PUSH_ON_FULL, reg_intr_ctrl_isr, intr_ctrl_isr->qm_dqm_push_on_full);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_CPU_POP_ON_EMPTY, reg_intr_ctrl_isr, intr_ctrl_isr->qm_cpu_pop_on_empty);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_CPU_PUSH_ON_FULL, reg_intr_ctrl_isr, intr_ctrl_isr->qm_cpu_push_on_full);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_NORMAL_QUEUE_PD_NO_CREDIT, reg_intr_ctrl_isr, intr_ctrl_isr->qm_normal_queue_pd_no_credit);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_NON_DELAYED_QUEUE_PD_NO_CREDIT, reg_intr_ctrl_isr, intr_ctrl_isr->qm_non_delayed_queue_pd_no_credit);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_NON_VALID_QUEUE, reg_intr_ctrl_isr, intr_ctrl_isr->qm_non_valid_queue);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_AGG_COHERENT_INCONSISTENCY, reg_intr_ctrl_isr, intr_ctrl_isr->qm_agg_coherent_inconsistency);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_FORCE_COPY_ON_NON_DELAYED, reg_intr_ctrl_isr, intr_ctrl_isr->qm_force_copy_on_non_delayed);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_FPM_POOL_SIZE_NONEXISTENT, reg_intr_ctrl_isr, intr_ctrl_isr->qm_fpm_pool_size_nonexistent);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_TARGET_MEM_ABS_CONTRADICTION, reg_intr_ctrl_isr, intr_ctrl_isr->qm_target_mem_abs_contradiction);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_1588_DROP, reg_intr_ctrl_isr, intr_ctrl_isr->qm_1588_drop);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_1588_MULTICAST_CONTRADICTION, reg_intr_ctrl_isr, intr_ctrl_isr->qm_1588_multicast_contradiction);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_BYTE_DROP_CNT_OVERRUN, reg_intr_ctrl_isr, intr_ctrl_isr->qm_byte_drop_cnt_overrun);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_PKT_DROP_CNT_OVERRUN, reg_intr_ctrl_isr, intr_ctrl_isr->qm_pkt_drop_cnt_overrun);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_TOTAL_BYTE_CNT_UNDERRUN, reg_intr_ctrl_isr, intr_ctrl_isr->qm_total_byte_cnt_underrun);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_TOTAL_PKT_CNT_UNDERRUN, reg_intr_ctrl_isr, intr_ctrl_isr->qm_total_pkt_cnt_underrun);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_FPM_UG0_UNDERRUN, reg_intr_ctrl_isr, intr_ctrl_isr->qm_fpm_ug0_underrun);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_FPM_UG1_UNDERRUN, reg_intr_ctrl_isr, intr_ctrl_isr->qm_fpm_ug1_underrun);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_FPM_UG2_UNDERRUN, reg_intr_ctrl_isr, intr_ctrl_isr->qm_fpm_ug2_underrun);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_FPM_UG3_UNDERRUN, reg_intr_ctrl_isr, intr_ctrl_isr->qm_fpm_ug3_underrun);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_TIMER_WRAPAROUND, reg_intr_ctrl_isr, intr_ctrl_isr->qm_timer_wraparound);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_COPY_PLEN_ZERO, reg_intr_ctrl_isr, intr_ctrl_isr->qm_copy_plen_zero);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_INGRESS_BB_UNEXPECTED_MSG, reg_intr_ctrl_isr, intr_ctrl_isr->qm_ingress_bb_unexpected_msg);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_EGRESS_BB_UNEXPECTED_MSG, reg_intr_ctrl_isr, intr_ctrl_isr->qm_egress_bb_unexpected_msg);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, DQM_REACHED_FULL, reg_intr_ctrl_isr, intr_ctrl_isr->dqm_reached_full);
    reg_intr_ctrl_isr = RU_FIELD_SET(0, QM, INTR_CTRL_ISR, QM_FPMINI_INTR, reg_intr_ctrl_isr, intr_ctrl_isr->qm_fpmini_intr);

    RU_REG_WRITE(0, QM, INTR_CTRL_ISR, reg_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_intr_ctrl_isr_get(qm_intr_ctrl_isr *intr_ctrl_isr)
{
    uint32_t reg_intr_ctrl_isr;

#ifdef VALIDATE_PARMS
    if (!intr_ctrl_isr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, INTR_CTRL_ISR, reg_intr_ctrl_isr);

    intr_ctrl_isr->qm_dqm_pop_on_empty = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_DQM_POP_ON_EMPTY, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_dqm_push_on_full = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_DQM_PUSH_ON_FULL, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_cpu_pop_on_empty = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_CPU_POP_ON_EMPTY, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_cpu_push_on_full = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_CPU_PUSH_ON_FULL, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_normal_queue_pd_no_credit = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_NORMAL_QUEUE_PD_NO_CREDIT, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_non_delayed_queue_pd_no_credit = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_NON_DELAYED_QUEUE_PD_NO_CREDIT, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_non_valid_queue = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_NON_VALID_QUEUE, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_agg_coherent_inconsistency = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_AGG_COHERENT_INCONSISTENCY, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_force_copy_on_non_delayed = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_FORCE_COPY_ON_NON_DELAYED, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_fpm_pool_size_nonexistent = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_FPM_POOL_SIZE_NONEXISTENT, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_target_mem_abs_contradiction = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_TARGET_MEM_ABS_CONTRADICTION, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_1588_drop = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_1588_DROP, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_1588_multicast_contradiction = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_1588_MULTICAST_CONTRADICTION, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_byte_drop_cnt_overrun = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_BYTE_DROP_CNT_OVERRUN, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_pkt_drop_cnt_overrun = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_PKT_DROP_CNT_OVERRUN, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_total_byte_cnt_underrun = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_TOTAL_BYTE_CNT_UNDERRUN, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_total_pkt_cnt_underrun = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_TOTAL_PKT_CNT_UNDERRUN, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_fpm_ug0_underrun = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_FPM_UG0_UNDERRUN, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_fpm_ug1_underrun = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_FPM_UG1_UNDERRUN, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_fpm_ug2_underrun = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_FPM_UG2_UNDERRUN, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_fpm_ug3_underrun = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_FPM_UG3_UNDERRUN, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_timer_wraparound = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_TIMER_WRAPAROUND, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_copy_plen_zero = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_COPY_PLEN_ZERO, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_ingress_bb_unexpected_msg = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_INGRESS_BB_UNEXPECTED_MSG, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_egress_bb_unexpected_msg = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_EGRESS_BB_UNEXPECTED_MSG, reg_intr_ctrl_isr);
    intr_ctrl_isr->dqm_reached_full = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, DQM_REACHED_FULL, reg_intr_ctrl_isr);
    intr_ctrl_isr->qm_fpmini_intr = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_FPMINI_INTR, reg_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_intr_ctrl_ism_get(uint32_t *ism)
{
    uint32_t reg_intr_ctrl_ism;

#ifdef VALIDATE_PARMS
    if (!ism)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, INTR_CTRL_ISM, reg_intr_ctrl_ism);

    *ism = RU_FIELD_GET(0, QM, INTR_CTRL_ISM, ISM, reg_intr_ctrl_ism);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_intr_ctrl_ier_set(uint32_t iem)
{
    uint32_t reg_intr_ctrl_ier = 0;

#ifdef VALIDATE_PARMS
    if ((iem >= _27BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_ctrl_ier = RU_FIELD_SET(0, QM, INTR_CTRL_IER, IEM, reg_intr_ctrl_ier, iem);

    RU_REG_WRITE(0, QM, INTR_CTRL_IER, reg_intr_ctrl_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_intr_ctrl_ier_get(uint32_t *iem)
{
    uint32_t reg_intr_ctrl_ier;

#ifdef VALIDATE_PARMS
    if (!iem)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, INTR_CTRL_IER, reg_intr_ctrl_ier);

    *iem = RU_FIELD_GET(0, QM, INTR_CTRL_IER, IEM, reg_intr_ctrl_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_intr_ctrl_itr_set(uint32_t ist)
{
    uint32_t reg_intr_ctrl_itr = 0;

#ifdef VALIDATE_PARMS
    if ((ist >= _27BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_ctrl_itr = RU_FIELD_SET(0, QM, INTR_CTRL_ITR, IST, reg_intr_ctrl_itr, ist);

    RU_REG_WRITE(0, QM, INTR_CTRL_ITR, reg_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_intr_ctrl_itr_get(uint32_t *ist)
{
    uint32_t reg_intr_ctrl_itr;

#ifdef VALIDATE_PARMS
    if (!ist)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, INTR_CTRL_ITR, reg_intr_ctrl_itr);

    *ist = RU_FIELD_GET(0, QM, INTR_CTRL_ITR, IST, reg_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_clk_gate_clk_gate_cntrl_set(const qm_clk_gate_clk_gate_cntrl *clk_gate_clk_gate_cntrl)
{
    uint32_t reg_clk_gate_clk_gate_cntrl = 0;

#ifdef VALIDATE_PARMS
    if(!clk_gate_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((clk_gate_clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
       (clk_gate_clk_gate_cntrl->keep_alive_en >= _1BITS_MAX_VAL_) ||
       (clk_gate_clk_gate_cntrl->keep_alive_intrvl >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_clk_gate_clk_gate_cntrl = RU_FIELD_SET(0, QM, CLK_GATE_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_clk_gate_clk_gate_cntrl, clk_gate_clk_gate_cntrl->bypass_clk_gate);
    reg_clk_gate_clk_gate_cntrl = RU_FIELD_SET(0, QM, CLK_GATE_CLK_GATE_CNTRL, TIMER_VAL, reg_clk_gate_clk_gate_cntrl, clk_gate_clk_gate_cntrl->timer_val);
    reg_clk_gate_clk_gate_cntrl = RU_FIELD_SET(0, QM, CLK_GATE_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_clk_gate_clk_gate_cntrl, clk_gate_clk_gate_cntrl->keep_alive_en);
    reg_clk_gate_clk_gate_cntrl = RU_FIELD_SET(0, QM, CLK_GATE_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_clk_gate_clk_gate_cntrl, clk_gate_clk_gate_cntrl->keep_alive_intrvl);
    reg_clk_gate_clk_gate_cntrl = RU_FIELD_SET(0, QM, CLK_GATE_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_clk_gate_clk_gate_cntrl, clk_gate_clk_gate_cntrl->keep_alive_cyc);

    RU_REG_WRITE(0, QM, CLK_GATE_CLK_GATE_CNTRL, reg_clk_gate_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_clk_gate_clk_gate_cntrl_get(qm_clk_gate_clk_gate_cntrl *clk_gate_clk_gate_cntrl)
{
    uint32_t reg_clk_gate_clk_gate_cntrl;

#ifdef VALIDATE_PARMS
    if (!clk_gate_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, CLK_GATE_CLK_GATE_CNTRL, reg_clk_gate_clk_gate_cntrl);

    clk_gate_clk_gate_cntrl->bypass_clk_gate = RU_FIELD_GET(0, QM, CLK_GATE_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_clk_gate_clk_gate_cntrl);
    clk_gate_clk_gate_cntrl->timer_val = RU_FIELD_GET(0, QM, CLK_GATE_CLK_GATE_CNTRL, TIMER_VAL, reg_clk_gate_clk_gate_cntrl);
    clk_gate_clk_gate_cntrl->keep_alive_en = RU_FIELD_GET(0, QM, CLK_GATE_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_clk_gate_clk_gate_cntrl);
    clk_gate_clk_gate_cntrl->keep_alive_intrvl = RU_FIELD_GET(0, QM, CLK_GATE_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_clk_gate_clk_gate_cntrl);
    clk_gate_clk_gate_cntrl->keep_alive_cyc = RU_FIELD_GET(0, QM, CLK_GATE_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_clk_gate_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set(uint8_t indirect_grp_idx, uint16_t queue_num, uint8_t cmd, bdmf_boolean done, bdmf_boolean error)
{
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_ctrl = 0;

#ifdef VALIDATE_PARMS
    if ((indirect_grp_idx >= 4) ||
       (queue_num >= _9BITS_MAX_VAL_) ||
       (cmd >= _2BITS_MAX_VAL_) ||
       (done >= _1BITS_MAX_VAL_) ||
       (error >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cpu_indr_port_cpu_pd_indirect_ctrl = RU_FIELD_SET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL, QUEUE_NUM, reg_cpu_indr_port_cpu_pd_indirect_ctrl, queue_num);
    reg_cpu_indr_port_cpu_pd_indirect_ctrl = RU_FIELD_SET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL, CMD, reg_cpu_indr_port_cpu_pd_indirect_ctrl, cmd);
    reg_cpu_indr_port_cpu_pd_indirect_ctrl = RU_FIELD_SET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL, DONE, reg_cpu_indr_port_cpu_pd_indirect_ctrl, done);
    reg_cpu_indr_port_cpu_pd_indirect_ctrl = RU_FIELD_SET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL, ERROR, reg_cpu_indr_port_cpu_pd_indirect_ctrl, error);

    RU_REG_RAM_WRITE(0, indirect_grp_idx, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL, reg_cpu_indr_port_cpu_pd_indirect_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_get(uint8_t indirect_grp_idx, uint16_t *queue_num, uint8_t *cmd, bdmf_boolean *done, bdmf_boolean *error)
{
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_ctrl;

#ifdef VALIDATE_PARMS
    if (!queue_num || !cmd || !done || !error)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((indirect_grp_idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, indirect_grp_idx, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL, reg_cpu_indr_port_cpu_pd_indirect_ctrl);

    *queue_num = RU_FIELD_GET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL, QUEUE_NUM, reg_cpu_indr_port_cpu_pd_indirect_ctrl);
    *cmd = RU_FIELD_GET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL, CMD, reg_cpu_indr_port_cpu_pd_indirect_ctrl);
    *done = RU_FIELD_GET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL, DONE, reg_cpu_indr_port_cpu_pd_indirect_ctrl);
    *error = RU_FIELD_GET(0, QM, CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL, ERROR, reg_cpu_indr_port_cpu_pd_indirect_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_q_context_set(uint16_t q_idx, const qm_q_context *q_context)
{
    uint32_t reg_queue_context_context = 0;

#ifdef VALIDATE_PARMS
    if(!q_context)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((q_idx >= 160) ||
       (q_context->wred_profile >= _4BITS_MAX_VAL_) ||
       (q_context->copy_dec_profile >= _3BITS_MAX_VAL_) ||
       (q_context->ddr_copy_disable >= _1BITS_MAX_VAL_) ||
       (q_context->aggregation_disable >= _1BITS_MAX_VAL_) ||
       (q_context->fpm_ug_or_bufmng >= _5BITS_MAX_VAL_) ||
       (q_context->exclusive_priority >= _1BITS_MAX_VAL_) ||
       (q_context->q_802_1ae >= _1BITS_MAX_VAL_) ||
       (q_context->sci >= _1BITS_MAX_VAL_) ||
       (q_context->fec_enable >= _1BITS_MAX_VAL_) ||
       (q_context->res_profile >= _3BITS_MAX_VAL_) ||
       (q_context->spare_room_0 >= _2BITS_MAX_VAL_) ||
       (q_context->spare_room_1 >= _2BITS_MAX_VAL_) ||
       (q_context->service_queue_profile >= _5BITS_MAX_VAL_) ||
       (q_context->timestamp_res_profile >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, WRED_PROFILE, reg_queue_context_context, q_context->wred_profile);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, COPY_DEC_PROFILE, reg_queue_context_context, q_context->copy_dec_profile);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, DDR_COPY_DISABLE, reg_queue_context_context, q_context->ddr_copy_disable);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, AGGREGATION_DISABLE, reg_queue_context_context, q_context->aggregation_disable);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, FPM_UG_OR_BUFMNG, reg_queue_context_context, q_context->fpm_ug_or_bufmng);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, EXCLUSIVE_PRIORITY, reg_queue_context_context, q_context->exclusive_priority);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, Q_802_1AE, reg_queue_context_context, q_context->q_802_1ae);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, SCI, reg_queue_context_context, q_context->sci);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, FEC_ENABLE, reg_queue_context_context, q_context->fec_enable);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, RES_PROFILE, reg_queue_context_context, q_context->res_profile);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, SPARE_ROOM_0, reg_queue_context_context, q_context->spare_room_0);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, SPARE_ROOM_1, reg_queue_context_context, q_context->spare_room_1);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, SERVICE_QUEUE_PROFILE, reg_queue_context_context, q_context->service_queue_profile);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, TIMESTAMP_RES_PROFILE, reg_queue_context_context, q_context->timestamp_res_profile);

    RU_REG_RAM_WRITE(0, q_idx, QM, QUEUE_CONTEXT_CONTEXT, reg_queue_context_context);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_q_context_get(uint16_t q_idx, qm_q_context *q_context)
{
    uint32_t reg_queue_context_context;

#ifdef VALIDATE_PARMS
    if (!q_context)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((q_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, QM, QUEUE_CONTEXT_CONTEXT, reg_queue_context_context);

    q_context->wred_profile = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, WRED_PROFILE, reg_queue_context_context);
    q_context->copy_dec_profile = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, COPY_DEC_PROFILE, reg_queue_context_context);
    q_context->ddr_copy_disable = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, DDR_COPY_DISABLE, reg_queue_context_context);
    q_context->aggregation_disable = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, AGGREGATION_DISABLE, reg_queue_context_context);
    q_context->fpm_ug_or_bufmng = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, FPM_UG_OR_BUFMNG, reg_queue_context_context);
    q_context->exclusive_priority = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, EXCLUSIVE_PRIORITY, reg_queue_context_context);
    q_context->q_802_1ae = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, Q_802_1AE, reg_queue_context_context);
    q_context->sci = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, SCI, reg_queue_context_context);
    q_context->fec_enable = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, FEC_ENABLE, reg_queue_context_context);
    q_context->res_profile = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, RES_PROFILE, reg_queue_context_context);
    q_context->spare_room_0 = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, SPARE_ROOM_0, reg_queue_context_context);
    q_context->spare_room_1 = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, SPARE_ROOM_1, reg_queue_context_context);
    q_context->service_queue_profile = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, SERVICE_QUEUE_PROFILE, reg_queue_context_context);
    q_context->timestamp_res_profile = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, TIMESTAMP_RES_PROFILE, reg_queue_context_context);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_copy_decision_profile_set(uint8_t profile_idx, uint32_t queue_occupancy_thr, bdmf_boolean psram_thr)
{
    uint32_t reg_copy_decision_profile_thr = 0;

#ifdef VALIDATE_PARMS
    if ((profile_idx >= 8) ||
       (queue_occupancy_thr >= _30BITS_MAX_VAL_) ||
       (psram_thr >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_copy_decision_profile_thr = RU_FIELD_SET(0, QM, COPY_DECISION_PROFILE_THR, QUEUE_OCCUPANCY_THR, reg_copy_decision_profile_thr, queue_occupancy_thr);
    reg_copy_decision_profile_thr = RU_FIELD_SET(0, QM, COPY_DECISION_PROFILE_THR, PSRAM_THR, reg_copy_decision_profile_thr, psram_thr);

    RU_REG_RAM_WRITE(0, profile_idx, QM, COPY_DECISION_PROFILE_THR, reg_copy_decision_profile_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_copy_decision_profile_get(uint8_t profile_idx, uint32_t *queue_occupancy_thr, bdmf_boolean *psram_thr)
{
    uint32_t reg_copy_decision_profile_thr;

#ifdef VALIDATE_PARMS
    if (!queue_occupancy_thr || !psram_thr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((profile_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, profile_idx, QM, COPY_DECISION_PROFILE_THR, reg_copy_decision_profile_thr);

    *queue_occupancy_thr = RU_FIELD_GET(0, QM, COPY_DECISION_PROFILE_THR, QUEUE_OCCUPANCY_THR, reg_copy_decision_profile_thr);
    *psram_thr = RU_FIELD_GET(0, QM, COPY_DECISION_PROFILE_THR, PSRAM_THR, reg_copy_decision_profile_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_timestamp_res_profile_set(uint8_t profile_idx, uint8_t start)
{
    uint32_t reg_timestamp_res_profile_value = 0;

#ifdef VALIDATE_PARMS
    if ((profile_idx >= 4) ||
       (start >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_timestamp_res_profile_value = RU_FIELD_SET(0, QM, TIMESTAMP_RES_PROFILE_VALUE, START, reg_timestamp_res_profile_value, start);

    RU_REG_RAM_WRITE(0, profile_idx, QM, TIMESTAMP_RES_PROFILE_VALUE, reg_timestamp_res_profile_value);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_timestamp_res_profile_get(uint8_t profile_idx, uint8_t *start)
{
    uint32_t reg_timestamp_res_profile_value;

#ifdef VALIDATE_PARMS
    if (!start)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((profile_idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, profile_idx, QM, TIMESTAMP_RES_PROFILE_VALUE, reg_timestamp_res_profile_value);

    *start = RU_FIELD_GET(0, QM, TIMESTAMP_RES_PROFILE_VALUE, START, reg_timestamp_res_profile_value);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_egress_drop_counter_get(uint32_t *data)
{
    uint32_t reg_global_egress_drop_counter_counter;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_EGRESS_DROP_COUNTER_COUNTER, reg_global_egress_drop_counter_counter);

    *data = RU_FIELD_GET(0, QM, GLOBAL_EGRESS_DROP_COUNTER_COUNTER, DATA, reg_global_egress_drop_counter_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_egress_aqm_drop_counter_get(uint32_t *data)
{
    uint32_t reg_global_egress_aqm_drop_counter_counter;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER, reg_global_egress_aqm_drop_counter_counter);

    *data = RU_FIELD_GET(0, QM, GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER, DATA, reg_global_egress_aqm_drop_counter_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_total_valid_cnt_set(uint16_t q_idx, uint32_t data)
{
    uint32_t reg_total_valid_counter_counter = 0;

#ifdef VALIDATE_PARMS
    if ((q_idx >= 640))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_total_valid_counter_counter = RU_FIELD_SET(0, QM, TOTAL_VALID_COUNTER_COUNTER, DATA, reg_total_valid_counter_counter, data);

    RU_REG_RAM_WRITE(0, q_idx, QM, TOTAL_VALID_COUNTER_COUNTER, reg_total_valid_counter_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_total_valid_cnt_get(uint16_t q_idx, uint32_t *data)
{
    uint32_t reg_total_valid_counter_counter;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((q_idx >= 640))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, QM, TOTAL_VALID_COUNTER_COUNTER, reg_total_valid_counter_counter);

    *data = RU_FIELD_GET(0, QM, TOTAL_VALID_COUNTER_COUNTER, DATA, reg_total_valid_counter_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dqm_valid_cnt_set(uint16_t q_idx, uint32_t data)
{
    uint32_t reg_dqm_valid_counter_counter = 0;

#ifdef VALIDATE_PARMS
    if ((q_idx >= 320))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dqm_valid_counter_counter = RU_FIELD_SET(0, QM, DQM_VALID_COUNTER_COUNTER, DATA, reg_dqm_valid_counter_counter, data);

    RU_REG_RAM_WRITE(0, q_idx, QM, DQM_VALID_COUNTER_COUNTER, reg_dqm_valid_counter_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dqm_valid_cnt_get(uint16_t q_idx, uint32_t *data)
{
    uint32_t reg_dqm_valid_counter_counter;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((q_idx >= 320))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, QM, DQM_VALID_COUNTER_COUNTER, reg_dqm_valid_counter_counter);

    *data = RU_FIELD_GET(0, QM, DQM_VALID_COUNTER_COUNTER, DATA, reg_dqm_valid_counter_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_drop_counter_get(uint16_t q_idx, uint32_t *data)
{
    uint32_t reg_drop_counter_counter;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((q_idx >= 320))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, QM, DROP_COUNTER_COUNTER, reg_drop_counter_counter);

    *data = RU_FIELD_GET(0, QM, DROP_COUNTER_COUNTER, DATA, reg_drop_counter_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_accumulated_counter_get(uint32_t q_idx, uint32_t *data)
{
    uint32_t reg_total_egress_accumulated_counter_counter;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((q_idx >= 320))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, QM, TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER, reg_total_egress_accumulated_counter_counter);

    *data = RU_FIELD_GET(0, QM, TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER, DATA, reg_total_egress_accumulated_counter_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_epon_q_byte_cnt_set(uint16_t q_idx, uint32_t data)
{
    uint32_t reg_epon_rpt_cnt_counter = 0;

#ifdef VALIDATE_PARMS
    if ((q_idx >= 320))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_epon_rpt_cnt_counter = RU_FIELD_SET(0, QM, EPON_RPT_CNT_COUNTER, DATA, reg_epon_rpt_cnt_counter, data);

    RU_REG_RAM_WRITE(0, q_idx, QM, EPON_RPT_CNT_COUNTER, reg_epon_rpt_cnt_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_epon_q_byte_cnt_get(uint16_t q_idx, uint32_t *data)
{
    uint32_t reg_epon_rpt_cnt_counter;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((q_idx >= 320))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, QM, EPON_RPT_CNT_COUNTER, reg_epon_rpt_cnt_counter);

    *data = RU_FIELD_GET(0, QM, EPON_RPT_CNT_COUNTER, DATA, reg_epon_rpt_cnt_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_epon_q_status_get(uint16_t q_idx, uint32_t *status_bit_vector)
{
    uint32_t reg_epon_rpt_cnt_queue_status;

#ifdef VALIDATE_PARMS
    if (!status_bit_vector)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((q_idx >= 5))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, QM, EPON_RPT_CNT_QUEUE_STATUS, reg_epon_rpt_cnt_queue_status);

    *status_bit_vector = RU_FIELD_GET(0, QM, EPON_RPT_CNT_QUEUE_STATUS, STATUS_BIT_VECTOR, reg_epon_rpt_cnt_queue_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_rd_data_pool0_get(uint32_t *data)
{
    uint32_t reg_rd_data_pool0;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, RD_DATA_POOL0, reg_rd_data_pool0);

    *data = RU_FIELD_GET(0, QM, RD_DATA_POOL0, DATA, reg_rd_data_pool0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_rd_data_pool1_get(uint32_t *data)
{
    uint32_t reg_rd_data_pool1;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, RD_DATA_POOL1, reg_rd_data_pool1);

    *data = RU_FIELD_GET(0, QM, RD_DATA_POOL1, DATA, reg_rd_data_pool1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_rd_data_pool2_get(uint32_t *data)
{
    uint32_t reg_rd_data_pool2;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, RD_DATA_POOL2, reg_rd_data_pool2);

    *data = RU_FIELD_GET(0, QM, RD_DATA_POOL2, DATA, reg_rd_data_pool2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_rd_data_pool3_get(uint32_t *data)
{
    uint32_t reg_rd_data_pool3;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, RD_DATA_POOL3, reg_rd_data_pool3);

    *data = RU_FIELD_GET(0, QM, RD_DATA_POOL3, DATA, reg_rd_data_pool3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_pop_3_set(bdmf_boolean pop_pool0, bdmf_boolean pop_pool1, bdmf_boolean pop_pool2, bdmf_boolean pop_pool3)
{
    uint32_t reg_pop_3 = 0;

#ifdef VALIDATE_PARMS
    if ((pop_pool0 >= _1BITS_MAX_VAL_) ||
       (pop_pool1 >= _1BITS_MAX_VAL_) ||
       (pop_pool2 >= _1BITS_MAX_VAL_) ||
       (pop_pool3 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pop_3 = RU_FIELD_SET(0, QM, POP_3, POP_POOL0, reg_pop_3, pop_pool0);
    reg_pop_3 = RU_FIELD_SET(0, QM, POP_3, POP_POOL1, reg_pop_3, pop_pool1);
    reg_pop_3 = RU_FIELD_SET(0, QM, POP_3, POP_POOL2, reg_pop_3, pop_pool2);
    reg_pop_3 = RU_FIELD_SET(0, QM, POP_3, POP_POOL3, reg_pop_3, pop_pool3);

    RU_REG_WRITE(0, QM, POP_3, reg_pop_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_pop_3_get(bdmf_boolean *pop_pool0, bdmf_boolean *pop_pool1, bdmf_boolean *pop_pool2, bdmf_boolean *pop_pool3)
{
    uint32_t reg_pop_3;

#ifdef VALIDATE_PARMS
    if (!pop_pool0 || !pop_pool1 || !pop_pool2 || !pop_pool3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, POP_3, reg_pop_3);

    *pop_pool0 = RU_FIELD_GET(0, QM, POP_3, POP_POOL0, reg_pop_3);
    *pop_pool1 = RU_FIELD_GET(0, QM, POP_3, POP_POOL1, reg_pop_3);
    *pop_pool2 = RU_FIELD_GET(0, QM, POP_3, POP_POOL2, reg_pop_3);
    *pop_pool3 = RU_FIELD_GET(0, QM, POP_3, POP_POOL3, reg_pop_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_pdfifo_ptr_get(uint16_t q_idx, uint8_t *wr_ptr, uint8_t *rd_ptr)
{
    uint32_t reg_pdfifo_ptr;

#ifdef VALIDATE_PARMS
    if (!wr_ptr || !rd_ptr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((q_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, QM, PDFIFO_PTR, reg_pdfifo_ptr);

    *wr_ptr = RU_FIELD_GET(0, QM, PDFIFO_PTR, WR_PTR, reg_pdfifo_ptr);
    *rd_ptr = RU_FIELD_GET(0, QM, PDFIFO_PTR, RD_PTR, reg_pdfifo_ptr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_update_fifo_ptr_get(uint16_t fifo_idx, uint16_t *wr_ptr, uint8_t *rd_ptr)
{
    uint32_t reg_update_fifo_ptr;

#ifdef VALIDATE_PARMS
    if (!wr_ptr || !rd_ptr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((fifo_idx >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, fifo_idx, QM, UPDATE_FIFO_PTR, reg_update_fifo_ptr);

    *wr_ptr = RU_FIELD_GET(0, QM, UPDATE_FIFO_PTR, WR_PTR, reg_update_fifo_ptr);
    *rd_ptr = RU_FIELD_GET(0, QM, UPDATE_FIFO_PTR, RD_PTR, reg_update_fifo_ptr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_pop_2_set(bdmf_boolean pop)
{
    uint32_t reg_pop_2 = 0;

#ifdef VALIDATE_PARMS
    if ((pop >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pop_2 = RU_FIELD_SET(0, QM, POP_2, POP, reg_pop_2, pop);

    RU_REG_WRITE(0, QM, POP_2, reg_pop_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_pop_2_get(bdmf_boolean *pop)
{
    uint32_t reg_pop_2;

#ifdef VALIDATE_PARMS
    if (!pop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, POP_2, reg_pop_2);

    *pop = RU_FIELD_GET(0, QM, POP_2, POP, reg_pop_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_pop_1_set(bdmf_boolean pop)
{
    uint32_t reg_pop_1 = 0;

#ifdef VALIDATE_PARMS
    if ((pop >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pop_1 = RU_FIELD_SET(0, QM, POP_1, POP, reg_pop_1, pop);

    RU_REG_WRITE(0, QM, POP_1, reg_pop_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_pop_1_get(bdmf_boolean *pop)
{
    uint32_t reg_pop_1;

#ifdef VALIDATE_PARMS
    if (!pop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, POP_1, reg_pop_1);

    *pop = RU_FIELD_GET(0, QM, POP_1, POP, reg_pop_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bb0_egr_msg_out_fifo_data_get(uint32_t idx, uint32_t *data)
{
    uint32_t reg_bb0_egr_msg_out_fifo_data;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, BB0_EGR_MSG_OUT_FIFO_DATA, reg_bb0_egr_msg_out_fifo_data);

    *data = RU_FIELD_GET(0, QM, BB0_EGR_MSG_OUT_FIFO_DATA, DATA, reg_bb0_egr_msg_out_fifo_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_buffer_reservation_data_set(uint32_t idx, uint32_t data)
{
    uint32_t reg_fpm_buffer_reservation_data = 0;

#ifdef VALIDATE_PARMS
    if ((idx >= 8) ||
       (data >= _18BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpm_buffer_reservation_data = RU_FIELD_SET(0, QM, FPM_BUFFER_RESERVATION_DATA, DATA, reg_fpm_buffer_reservation_data, data);

    RU_REG_RAM_WRITE(0, idx, QM, FPM_BUFFER_RESERVATION_DATA, reg_fpm_buffer_reservation_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_buffer_reservation_data_get(uint32_t idx, uint32_t *data)
{
    uint32_t reg_fpm_buffer_reservation_data;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, FPM_BUFFER_RESERVATION_DATA, reg_fpm_buffer_reservation_data);

    *data = RU_FIELD_GET(0, QM, FPM_BUFFER_RESERVATION_DATA, DATA, reg_fpm_buffer_reservation_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_port_cfg_set(uint8_t idx, bdmf_boolean en_byte, bdmf_boolean en_ug, uint8_t bbh_rx_bb_id, uint8_t fw_port_id)
{
    uint32_t reg_port_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((idx >= 11) ||
       (en_byte >= _1BITS_MAX_VAL_) ||
       (en_ug >= _1BITS_MAX_VAL_) ||
       (bbh_rx_bb_id >= _6BITS_MAX_VAL_) ||
       (fw_port_id >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_port_cfg = RU_FIELD_SET(0, QM, PORT_CFG, EN_BYTE, reg_port_cfg, en_byte);
    reg_port_cfg = RU_FIELD_SET(0, QM, PORT_CFG, EN_UG, reg_port_cfg, en_ug);
    reg_port_cfg = RU_FIELD_SET(0, QM, PORT_CFG, BBH_RX_BB_ID, reg_port_cfg, bbh_rx_bb_id);
    reg_port_cfg = RU_FIELD_SET(0, QM, PORT_CFG, FW_PORT_ID, reg_port_cfg, fw_port_id);

    RU_REG_RAM_WRITE(0, idx, QM, PORT_CFG, reg_port_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_port_cfg_get(uint8_t idx, bdmf_boolean *en_byte, bdmf_boolean *en_ug, uint8_t *bbh_rx_bb_id, uint8_t *fw_port_id)
{
    uint32_t reg_port_cfg;

#ifdef VALIDATE_PARMS
    if (!en_byte || !en_ug || !bbh_rx_bb_id || !fw_port_id)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 11))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, PORT_CFG, reg_port_cfg);

    *en_byte = RU_FIELD_GET(0, QM, PORT_CFG, EN_BYTE, reg_port_cfg);
    *en_ug = RU_FIELD_GET(0, QM, PORT_CFG, EN_UG, reg_port_cfg);
    *bbh_rx_bb_id = RU_FIELD_GET(0, QM, PORT_CFG, BBH_RX_BB_ID, reg_port_cfg);
    *fw_port_id = RU_FIELD_GET(0, QM, PORT_CFG, FW_PORT_ID, reg_port_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fc_ug_mask_ug_en_set(uint32_t ug_en)
{
    uint32_t reg_fc_ug_mask_ug_en = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_fc_ug_mask_ug_en = RU_FIELD_SET(0, QM, FC_UG_MASK_UG_EN, UG_EN, reg_fc_ug_mask_ug_en, ug_en);

    RU_REG_WRITE(0, QM, FC_UG_MASK_UG_EN, reg_fc_ug_mask_ug_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fc_ug_mask_ug_en_get(uint32_t *ug_en)
{
    uint32_t reg_fc_ug_mask_ug_en;

#ifdef VALIDATE_PARMS
    if (!ug_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, FC_UG_MASK_UG_EN, reg_fc_ug_mask_ug_en);

    *ug_en = RU_FIELD_GET(0, QM, FC_UG_MASK_UG_EN, UG_EN, reg_fc_ug_mask_ug_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fc_queue_mask_set(uint8_t idx, uint32_t queue_vec)
{
    uint32_t reg_fc_queue_mask = 0;

#ifdef VALIDATE_PARMS
    if ((idx >= 5))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fc_queue_mask = RU_FIELD_SET(0, QM, FC_QUEUE_MASK, QUEUE_VEC, reg_fc_queue_mask, queue_vec);

    RU_REG_RAM_WRITE(0, idx, QM, FC_QUEUE_MASK, reg_fc_queue_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fc_queue_mask_get(uint8_t idx, uint32_t *queue_vec)
{
    uint32_t reg_fc_queue_mask;

#ifdef VALIDATE_PARMS
    if (!queue_vec)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 5))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, FC_QUEUE_MASK, reg_fc_queue_mask);

    *queue_vec = RU_FIELD_GET(0, QM, FC_QUEUE_MASK, QUEUE_VEC, reg_fc_queue_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fc_queue_range1_start_set(uint16_t start_queue)
{
    uint32_t reg_fc_queue_range1_start = 0;

#ifdef VALIDATE_PARMS
    if ((start_queue >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fc_queue_range1_start = RU_FIELD_SET(0, QM, FC_QUEUE_RANGE1_START, START_QUEUE, reg_fc_queue_range1_start, start_queue);

    RU_REG_WRITE(0, QM, FC_QUEUE_RANGE1_START, reg_fc_queue_range1_start);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fc_queue_range1_start_get(uint16_t *start_queue)
{
    uint32_t reg_fc_queue_range1_start;

#ifdef VALIDATE_PARMS
    if (!start_queue)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, FC_QUEUE_RANGE1_START, reg_fc_queue_range1_start);

    *start_queue = RU_FIELD_GET(0, QM, FC_QUEUE_RANGE1_START, START_QUEUE, reg_fc_queue_range1_start);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fc_queue_range2_start_set(uint16_t start_queue)
{
    uint32_t reg_fc_queue_range2_start = 0;

#ifdef VALIDATE_PARMS
    if ((start_queue >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fc_queue_range2_start = RU_FIELD_SET(0, QM, FC_QUEUE_RANGE2_START, START_QUEUE, reg_fc_queue_range2_start, start_queue);

    RU_REG_WRITE(0, QM, FC_QUEUE_RANGE2_START, reg_fc_queue_range2_start);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fc_queue_range2_start_get(uint16_t *start_queue)
{
    uint32_t reg_fc_queue_range2_start;

#ifdef VALIDATE_PARMS
    if (!start_queue)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, FC_QUEUE_RANGE2_START, reg_fc_queue_range2_start);

    *start_queue = RU_FIELD_GET(0, QM, FC_QUEUE_RANGE2_START, START_QUEUE, reg_fc_queue_range2_start);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dbg_get(uint32_t *status)
{
    uint32_t reg_dbg;

#ifdef VALIDATE_PARMS
    if (!status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, DBG, reg_dbg);

    *status = RU_FIELD_GET(0, QM, DBG, STATUS, reg_dbg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_ug_occupancy_status_get(uint32_t q_idx, uint32_t *status)
{
    uint32_t reg_ug_occupancy_status;

#ifdef VALIDATE_PARMS
    if (!status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((q_idx >= 11))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, QM, UG_OCCUPANCY_STATUS, reg_ug_occupancy_status);

    *status = RU_FIELD_GET(0, QM, UG_OCCUPANCY_STATUS, STATUS, reg_ug_occupancy_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_queue_range1_occupancy_status_get(uint8_t idx, uint32_t *status)
{
    uint32_t reg_queue_range1_occupancy_status;

#ifdef VALIDATE_PARMS
    if (!status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 44))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, QUEUE_RANGE1_OCCUPANCY_STATUS, reg_queue_range1_occupancy_status);

    *status = RU_FIELD_GET(0, QM, QUEUE_RANGE1_OCCUPANCY_STATUS, STATUS, reg_queue_range1_occupancy_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_queue_range2_occupancy_status_get(uint8_t idx, uint32_t *status)
{
    uint32_t reg_queue_range2_occupancy_status;

#ifdef VALIDATE_PARMS
    if (!status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 44))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, QUEUE_RANGE2_OCCUPANCY_STATUS, reg_queue_range2_occupancy_status);

    *status = RU_FIELD_GET(0, QM, QUEUE_RANGE2_OCCUPANCY_STATUS, STATUS, reg_queue_range2_occupancy_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_debug_sel_set(uint8_t select, bdmf_boolean enable)
{
    uint32_t reg_debug_sel = 0;

#ifdef VALIDATE_PARMS
    if ((select >= _5BITS_MAX_VAL_) ||
       (enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_sel = RU_FIELD_SET(0, QM, DEBUG_SEL, SELECT, reg_debug_sel, select);
    reg_debug_sel = RU_FIELD_SET(0, QM, DEBUG_SEL, ENABLE, reg_debug_sel, enable);

    RU_REG_WRITE(0, QM, DEBUG_SEL, reg_debug_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_debug_sel_get(uint8_t *select, bdmf_boolean *enable)
{
    uint32_t reg_debug_sel;

#ifdef VALIDATE_PARMS
    if (!select || !enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, DEBUG_SEL, reg_debug_sel);

    *select = RU_FIELD_GET(0, QM, DEBUG_SEL, SELECT, reg_debug_sel);
    *enable = RU_FIELD_GET(0, QM, DEBUG_SEL, ENABLE, reg_debug_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_debug_bus_lsb_get(uint32_t *data)
{
    uint32_t reg_debug_bus_lsb;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, DEBUG_BUS_LSB, reg_debug_bus_lsb);

    *data = RU_FIELD_GET(0, QM, DEBUG_BUS_LSB, DATA, reg_debug_bus_lsb);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_debug_bus_msb_get(uint32_t *data)
{
    uint32_t reg_debug_bus_msb;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, DEBUG_BUS_MSB, reg_debug_bus_msb);

    *data = RU_FIELD_GET(0, QM, DEBUG_BUS_MSB, DATA, reg_debug_bus_msb);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_spare_config_get(uint32_t *data)
{
    uint32_t reg_qm_spare_config;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, QM_SPARE_CONFIG, reg_qm_spare_config);

    *data = RU_FIELD_GET(0, QM, QM_SPARE_CONFIG, DATA, reg_qm_spare_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_good_lvl1_pkts_cnt_get(uint32_t *good_lvl1_pkts)
{
    uint32_t reg_good_lvl1_pkts_cnt;

#ifdef VALIDATE_PARMS
    if (!good_lvl1_pkts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GOOD_LVL1_PKTS_CNT, reg_good_lvl1_pkts_cnt);

    *good_lvl1_pkts = RU_FIELD_GET(0, QM, GOOD_LVL1_PKTS_CNT, COUNTER, reg_good_lvl1_pkts_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_good_lvl1_bytes_cnt_get(uint32_t *good_lvl1_bytes)
{
    uint32_t reg_good_lvl1_bytes_cnt;

#ifdef VALIDATE_PARMS
    if (!good_lvl1_bytes)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GOOD_LVL1_BYTES_CNT, reg_good_lvl1_bytes_cnt);

    *good_lvl1_bytes = RU_FIELD_GET(0, QM, GOOD_LVL1_BYTES_CNT, COUNTER, reg_good_lvl1_bytes_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_good_lvl2_pkts_cnt_get(uint32_t *good_lvl2_pkts)
{
    uint32_t reg_good_lvl2_pkts_cnt;

#ifdef VALIDATE_PARMS
    if (!good_lvl2_pkts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GOOD_LVL2_PKTS_CNT, reg_good_lvl2_pkts_cnt);

    *good_lvl2_pkts = RU_FIELD_GET(0, QM, GOOD_LVL2_PKTS_CNT, COUNTER, reg_good_lvl2_pkts_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_good_lvl2_bytes_cnt_get(uint32_t *good_lvl2_bytes)
{
    uint32_t reg_good_lvl2_bytes_cnt;

#ifdef VALIDATE_PARMS
    if (!good_lvl2_bytes)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GOOD_LVL2_BYTES_CNT, reg_good_lvl2_bytes_cnt);

    *good_lvl2_bytes = RU_FIELD_GET(0, QM, GOOD_LVL2_BYTES_CNT, COUNTER, reg_good_lvl2_bytes_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_copied_pkts_cnt_get(uint32_t *copied_pkts)
{
    uint32_t reg_copied_pkts_cnt;

#ifdef VALIDATE_PARMS
    if (!copied_pkts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, COPIED_PKTS_CNT, reg_copied_pkts_cnt);

    *copied_pkts = RU_FIELD_GET(0, QM, COPIED_PKTS_CNT, COUNTER, reg_copied_pkts_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_copied_bytes_cnt_get(uint32_t *copied_bytes)
{
    uint32_t reg_copied_bytes_cnt;

#ifdef VALIDATE_PARMS
    if (!copied_bytes)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, COPIED_BYTES_CNT, reg_copied_bytes_cnt);

    *copied_bytes = RU_FIELD_GET(0, QM, COPIED_BYTES_CNT, COUNTER, reg_copied_bytes_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_agg_pkts_cnt_get(uint32_t *agg_pkts)
{
    uint32_t reg_agg_pkts_cnt;

#ifdef VALIDATE_PARMS
    if (!agg_pkts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, AGG_PKTS_CNT, reg_agg_pkts_cnt);

    *agg_pkts = RU_FIELD_GET(0, QM, AGG_PKTS_CNT, COUNTER, reg_agg_pkts_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_agg_bytes_cnt_get(uint32_t *agg_bytes)
{
    uint32_t reg_agg_bytes_cnt;

#ifdef VALIDATE_PARMS
    if (!agg_bytes)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, AGG_BYTES_CNT, reg_agg_bytes_cnt);

    *agg_bytes = RU_FIELD_GET(0, QM, AGG_BYTES_CNT, COUNTER, reg_agg_bytes_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_agg_1_pkts_cnt_get(uint32_t *agg1_pkts)
{
    uint32_t reg_agg_1_pkts_cnt;

#ifdef VALIDATE_PARMS
    if (!agg1_pkts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, AGG_1_PKTS_CNT, reg_agg_1_pkts_cnt);

    *agg1_pkts = RU_FIELD_GET(0, QM, AGG_1_PKTS_CNT, COUNTER, reg_agg_1_pkts_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_agg_2_pkts_cnt_get(uint32_t *agg2_pkts)
{
    uint32_t reg_agg_2_pkts_cnt;

#ifdef VALIDATE_PARMS
    if (!agg2_pkts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, AGG_2_PKTS_CNT, reg_agg_2_pkts_cnt);

    *agg2_pkts = RU_FIELD_GET(0, QM, AGG_2_PKTS_CNT, COUNTER, reg_agg_2_pkts_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_agg_3_pkts_cnt_get(uint32_t *agg3_pkts)
{
    uint32_t reg_agg_3_pkts_cnt;

#ifdef VALIDATE_PARMS
    if (!agg3_pkts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, AGG_3_PKTS_CNT, reg_agg_3_pkts_cnt);

    *agg3_pkts = RU_FIELD_GET(0, QM, AGG_3_PKTS_CNT, COUNTER, reg_agg_3_pkts_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_agg_4_pkts_cnt_get(uint32_t *agg4_pkts)
{
    uint32_t reg_agg_4_pkts_cnt;

#ifdef VALIDATE_PARMS
    if (!agg4_pkts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, AGG_4_PKTS_CNT, reg_agg_4_pkts_cnt);

    *agg4_pkts = RU_FIELD_GET(0, QM, AGG_4_PKTS_CNT, COUNTER, reg_agg_4_pkts_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_wred_drop_cnt_get(uint32_t *wred_drop)
{
    uint32_t reg_wred_drop_cnt;

#ifdef VALIDATE_PARMS
    if (!wred_drop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, WRED_DROP_CNT, reg_wred_drop_cnt);

    *wred_drop = RU_FIELD_GET(0, QM, WRED_DROP_CNT, COUNTER, reg_wred_drop_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_congestion_drop_cnt_get(uint32_t *fpm_cong)
{
    uint32_t reg_fpm_congestion_drop_cnt;

#ifdef VALIDATE_PARMS
    if (!fpm_cong)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, FPM_CONGESTION_DROP_CNT, reg_fpm_congestion_drop_cnt);

    *fpm_cong = RU_FIELD_GET(0, QM, FPM_CONGESTION_DROP_CNT, COUNTER, reg_fpm_congestion_drop_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_ddr_pd_congestion_drop_cnt_get(uint32_t *ddr_pd_cong_drop)
{
    uint32_t reg_ddr_pd_congestion_drop_cnt;

#ifdef VALIDATE_PARMS
    if (!ddr_pd_cong_drop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, DDR_PD_CONGESTION_DROP_CNT, reg_ddr_pd_congestion_drop_cnt);

    *ddr_pd_cong_drop = RU_FIELD_GET(0, QM, DDR_PD_CONGESTION_DROP_CNT, COUNTER, reg_ddr_pd_congestion_drop_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_ddr_byte_congestion_drop_cnt_get(uint32_t *ddr_cong_byte_drop)
{
    uint32_t reg_ddr_byte_congestion_drop_cnt;

#ifdef VALIDATE_PARMS
    if (!ddr_cong_byte_drop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, DDR_BYTE_CONGESTION_DROP_CNT, reg_ddr_byte_congestion_drop_cnt);

    *ddr_cong_byte_drop = RU_FIELD_GET(0, QM, DDR_BYTE_CONGESTION_DROP_CNT, COUNTER, reg_ddr_byte_congestion_drop_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_pd_congestion_drop_cnt_get(uint32_t *counter)
{
    uint32_t reg_qm_pd_congestion_drop_cnt;

#ifdef VALIDATE_PARMS
    if (!counter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, QM_PD_CONGESTION_DROP_CNT, reg_qm_pd_congestion_drop_cnt);

    *counter = RU_FIELD_GET(0, QM, QM_PD_CONGESTION_DROP_CNT, COUNTER, reg_qm_pd_congestion_drop_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_abs_requeue_cnt_get(uint32_t *abs_requeue)
{
    uint32_t reg_qm_abs_requeue_cnt;

#ifdef VALIDATE_PARMS
    if (!abs_requeue)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, QM_ABS_REQUEUE_CNT, reg_qm_abs_requeue_cnt);

    *abs_requeue = RU_FIELD_GET(0, QM, QM_ABS_REQUEUE_CNT, COUNTER, reg_qm_abs_requeue_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_prefetch_fifo0_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_fpm_prefetch_fifo0_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, FPM_PREFETCH_FIFO0_STATUS, reg_fpm_prefetch_fifo0_status);

    *used_words = RU_FIELD_GET(0, QM, FPM_PREFETCH_FIFO0_STATUS, USED_WORDS, reg_fpm_prefetch_fifo0_status);
    *empty = RU_FIELD_GET(0, QM, FPM_PREFETCH_FIFO0_STATUS, EMPTY, reg_fpm_prefetch_fifo0_status);
    *full = RU_FIELD_GET(0, QM, FPM_PREFETCH_FIFO0_STATUS, FULL, reg_fpm_prefetch_fifo0_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_prefetch_fifo1_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_fpm_prefetch_fifo1_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, FPM_PREFETCH_FIFO1_STATUS, reg_fpm_prefetch_fifo1_status);

    *used_words = RU_FIELD_GET(0, QM, FPM_PREFETCH_FIFO1_STATUS, USED_WORDS, reg_fpm_prefetch_fifo1_status);
    *empty = RU_FIELD_GET(0, QM, FPM_PREFETCH_FIFO1_STATUS, EMPTY, reg_fpm_prefetch_fifo1_status);
    *full = RU_FIELD_GET(0, QM, FPM_PREFETCH_FIFO1_STATUS, FULL, reg_fpm_prefetch_fifo1_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_prefetch_fifo2_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_fpm_prefetch_fifo2_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, FPM_PREFETCH_FIFO2_STATUS, reg_fpm_prefetch_fifo2_status);

    *used_words = RU_FIELD_GET(0, QM, FPM_PREFETCH_FIFO2_STATUS, USED_WORDS, reg_fpm_prefetch_fifo2_status);
    *empty = RU_FIELD_GET(0, QM, FPM_PREFETCH_FIFO2_STATUS, EMPTY, reg_fpm_prefetch_fifo2_status);
    *full = RU_FIELD_GET(0, QM, FPM_PREFETCH_FIFO2_STATUS, FULL, reg_fpm_prefetch_fifo2_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_prefetch_fifo3_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_fpm_prefetch_fifo3_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, FPM_PREFETCH_FIFO3_STATUS, reg_fpm_prefetch_fifo3_status);

    *used_words = RU_FIELD_GET(0, QM, FPM_PREFETCH_FIFO3_STATUS, USED_WORDS, reg_fpm_prefetch_fifo3_status);
    *empty = RU_FIELD_GET(0, QM, FPM_PREFETCH_FIFO3_STATUS, EMPTY, reg_fpm_prefetch_fifo3_status);
    *full = RU_FIELD_GET(0, QM, FPM_PREFETCH_FIFO3_STATUS, FULL, reg_fpm_prefetch_fifo3_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_normal_rmt_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_normal_rmt_fifo_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, NORMAL_RMT_FIFO_STATUS, reg_normal_rmt_fifo_status);

    *used_words = RU_FIELD_GET(0, QM, NORMAL_RMT_FIFO_STATUS, USED_WORDS, reg_normal_rmt_fifo_status);
    *empty = RU_FIELD_GET(0, QM, NORMAL_RMT_FIFO_STATUS, EMPTY, reg_normal_rmt_fifo_status);
    *full = RU_FIELD_GET(0, QM, NORMAL_RMT_FIFO_STATUS, FULL, reg_normal_rmt_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_non_delayed_rmt_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_non_delayed_rmt_fifo_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, NON_DELAYED_RMT_FIFO_STATUS, reg_non_delayed_rmt_fifo_status);

    *used_words = RU_FIELD_GET(0, QM, NON_DELAYED_RMT_FIFO_STATUS, USED_WORDS, reg_non_delayed_rmt_fifo_status);
    *empty = RU_FIELD_GET(0, QM, NON_DELAYED_RMT_FIFO_STATUS, EMPTY, reg_non_delayed_rmt_fifo_status);
    *full = RU_FIELD_GET(0, QM, NON_DELAYED_RMT_FIFO_STATUS, FULL, reg_non_delayed_rmt_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_non_delayed_out_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_non_delayed_out_fifo_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, NON_DELAYED_OUT_FIFO_STATUS, reg_non_delayed_out_fifo_status);

    *used_words = RU_FIELD_GET(0, QM, NON_DELAYED_OUT_FIFO_STATUS, USED_WORDS, reg_non_delayed_out_fifo_status);
    *empty = RU_FIELD_GET(0, QM, NON_DELAYED_OUT_FIFO_STATUS, EMPTY, reg_non_delayed_out_fifo_status);
    *full = RU_FIELD_GET(0, QM, NON_DELAYED_OUT_FIFO_STATUS, FULL, reg_non_delayed_out_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_pre_cm_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_pre_cm_fifo_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, PRE_CM_FIFO_STATUS, reg_pre_cm_fifo_status);

    *used_words = RU_FIELD_GET(0, QM, PRE_CM_FIFO_STATUS, USED_WORDS, reg_pre_cm_fifo_status);
    *empty = RU_FIELD_GET(0, QM, PRE_CM_FIFO_STATUS, EMPTY, reg_pre_cm_fifo_status);
    *full = RU_FIELD_GET(0, QM, PRE_CM_FIFO_STATUS, FULL, reg_pre_cm_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_cm_rd_pd_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_cm_rd_pd_fifo_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, CM_RD_PD_FIFO_STATUS, reg_cm_rd_pd_fifo_status);

    *used_words = RU_FIELD_GET(0, QM, CM_RD_PD_FIFO_STATUS, USED_WORDS, reg_cm_rd_pd_fifo_status);
    *empty = RU_FIELD_GET(0, QM, CM_RD_PD_FIFO_STATUS, EMPTY, reg_cm_rd_pd_fifo_status);
    *full = RU_FIELD_GET(0, QM, CM_RD_PD_FIFO_STATUS, FULL, reg_cm_rd_pd_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_cm_wr_pd_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_cm_wr_pd_fifo_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, CM_WR_PD_FIFO_STATUS, reg_cm_wr_pd_fifo_status);

    *used_words = RU_FIELD_GET(0, QM, CM_WR_PD_FIFO_STATUS, USED_WORDS, reg_cm_wr_pd_fifo_status);
    *empty = RU_FIELD_GET(0, QM, CM_WR_PD_FIFO_STATUS, EMPTY, reg_cm_wr_pd_fifo_status);
    *full = RU_FIELD_GET(0, QM, CM_WR_PD_FIFO_STATUS, FULL, reg_cm_wr_pd_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_cm_common_input_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_cm_common_input_fifo_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, CM_COMMON_INPUT_FIFO_STATUS, reg_cm_common_input_fifo_status);

    *used_words = RU_FIELD_GET(0, QM, CM_COMMON_INPUT_FIFO_STATUS, USED_WORDS, reg_cm_common_input_fifo_status);
    *empty = RU_FIELD_GET(0, QM, CM_COMMON_INPUT_FIFO_STATUS, EMPTY, reg_cm_common_input_fifo_status);
    *full = RU_FIELD_GET(0, QM, CM_COMMON_INPUT_FIFO_STATUS, FULL, reg_cm_common_input_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bb0_output_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_bb0_output_fifo_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BB0_OUTPUT_FIFO_STATUS, reg_bb0_output_fifo_status);

    *used_words = RU_FIELD_GET(0, QM, BB0_OUTPUT_FIFO_STATUS, USED_WORDS, reg_bb0_output_fifo_status);
    *empty = RU_FIELD_GET(0, QM, BB0_OUTPUT_FIFO_STATUS, EMPTY, reg_bb0_output_fifo_status);
    *full = RU_FIELD_GET(0, QM, BB0_OUTPUT_FIFO_STATUS, FULL, reg_bb0_output_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bb1_output_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_bb1_output_fifo_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BB1_OUTPUT_FIFO_STATUS, reg_bb1_output_fifo_status);

    *used_words = RU_FIELD_GET(0, QM, BB1_OUTPUT_FIFO_STATUS, USED_WORDS, reg_bb1_output_fifo_status);
    *empty = RU_FIELD_GET(0, QM, BB1_OUTPUT_FIFO_STATUS, EMPTY, reg_bb1_output_fifo_status);
    *full = RU_FIELD_GET(0, QM, BB1_OUTPUT_FIFO_STATUS, FULL, reg_bb1_output_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bb1_input_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_bb1_input_fifo_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BB1_INPUT_FIFO_STATUS, reg_bb1_input_fifo_status);

    *used_words = RU_FIELD_GET(0, QM, BB1_INPUT_FIFO_STATUS, USED_WORDS, reg_bb1_input_fifo_status);
    *empty = RU_FIELD_GET(0, QM, BB1_INPUT_FIFO_STATUS, EMPTY, reg_bb1_input_fifo_status);
    *full = RU_FIELD_GET(0, QM, BB1_INPUT_FIFO_STATUS, FULL, reg_bb1_input_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_egress_data_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_egress_data_fifo_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, EGRESS_DATA_FIFO_STATUS, reg_egress_data_fifo_status);

    *used_words = RU_FIELD_GET(0, QM, EGRESS_DATA_FIFO_STATUS, USED_WORDS, reg_egress_data_fifo_status);
    *empty = RU_FIELD_GET(0, QM, EGRESS_DATA_FIFO_STATUS, EMPTY, reg_egress_data_fifo_status);
    *full = RU_FIELD_GET(0, QM, EGRESS_DATA_FIFO_STATUS, FULL, reg_egress_data_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_egress_rr_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_egress_rr_fifo_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, EGRESS_RR_FIFO_STATUS, reg_egress_rr_fifo_status);

    *used_words = RU_FIELD_GET(0, QM, EGRESS_RR_FIFO_STATUS, USED_WORDS, reg_egress_rr_fifo_status);
    *empty = RU_FIELD_GET(0, QM, EGRESS_RR_FIFO_STATUS, EMPTY, reg_egress_rr_fifo_status);
    *full = RU_FIELD_GET(0, QM, EGRESS_RR_FIFO_STATUS, FULL, reg_egress_rr_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bb_route_ovr_set(uint8_t idx, bdmf_boolean ovr_en, uint8_t dest_id, uint16_t route_addr)
{
    uint32_t reg_bb_route_ovr = 0;

#ifdef VALIDATE_PARMS
    if ((idx >= 3) ||
       (ovr_en >= _1BITS_MAX_VAL_) ||
       (dest_id >= _6BITS_MAX_VAL_) ||
       (route_addr >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bb_route_ovr = RU_FIELD_SET(0, QM, BB_ROUTE_OVR, OVR_EN, reg_bb_route_ovr, ovr_en);
    reg_bb_route_ovr = RU_FIELD_SET(0, QM, BB_ROUTE_OVR, DEST_ID, reg_bb_route_ovr, dest_id);
    reg_bb_route_ovr = RU_FIELD_SET(0, QM, BB_ROUTE_OVR, ROUTE_ADDR, reg_bb_route_ovr, route_addr);

    RU_REG_RAM_WRITE(0, idx, QM, BB_ROUTE_OVR, reg_bb_route_ovr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bb_route_ovr_get(uint8_t idx, bdmf_boolean *ovr_en, uint8_t *dest_id, uint16_t *route_addr)
{
    uint32_t reg_bb_route_ovr;

#ifdef VALIDATE_PARMS
    if (!ovr_en || !dest_id || !route_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 3))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, BB_ROUTE_OVR, reg_bb_route_ovr);

    *ovr_en = RU_FIELD_GET(0, QM, BB_ROUTE_OVR, OVR_EN, reg_bb_route_ovr);
    *dest_id = RU_FIELD_GET(0, QM, BB_ROUTE_OVR, DEST_ID, reg_bb_route_ovr);
    *route_addr = RU_FIELD_GET(0, QM, BB_ROUTE_OVR, ROUTE_ADDR, reg_bb_route_ovr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_ingress_stat_get(uint32_t *ingress_stat)
{
    uint32_t reg_qm_ingress_stat;

#ifdef VALIDATE_PARMS
    if (!ingress_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, QM_INGRESS_STAT, reg_qm_ingress_stat);

    *ingress_stat = RU_FIELD_GET(0, QM, QM_INGRESS_STAT, STAT, reg_qm_ingress_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_egress_stat_get(uint32_t *egress_stat)
{
    uint32_t reg_qm_egress_stat;

#ifdef VALIDATE_PARMS
    if (!egress_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, QM_EGRESS_STAT, reg_qm_egress_stat);

    *egress_stat = RU_FIELD_GET(0, QM, QM_EGRESS_STAT, STAT, reg_qm_egress_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_cm_stat_get(uint32_t *cm_stat)
{
    uint32_t reg_qm_cm_stat;

#ifdef VALIDATE_PARMS
    if (!cm_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, QM_CM_STAT, reg_qm_cm_stat);

    *cm_stat = RU_FIELD_GET(0, QM, QM_CM_STAT, STAT, reg_qm_cm_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_prefetch_stat_get(uint32_t *fpm_prefetch_stat)
{
    uint32_t reg_qm_fpm_prefetch_stat;

#ifdef VALIDATE_PARMS
    if (!fpm_prefetch_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, QM_FPM_PREFETCH_STAT, reg_qm_fpm_prefetch_stat);

    *fpm_prefetch_stat = RU_FIELD_GET(0, QM, QM_FPM_PREFETCH_STAT, STAT, reg_qm_fpm_prefetch_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_connect_ack_counter_get(uint8_t *connect_ack_counter)
{
    uint32_t reg_qm_connect_ack_counter;

#ifdef VALIDATE_PARMS
    if (!connect_ack_counter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, QM_CONNECT_ACK_COUNTER, reg_qm_connect_ack_counter);

    *connect_ack_counter = RU_FIELD_GET(0, QM, QM_CONNECT_ACK_COUNTER, CONNECT_ACK_COUNTER, reg_qm_connect_ack_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_ddr_wr_reply_counter_get(uint8_t *ddr_wr_reply_counter)
{
    uint32_t reg_qm_ddr_wr_reply_counter;

#ifdef VALIDATE_PARMS
    if (!ddr_wr_reply_counter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, QM_DDR_WR_REPLY_COUNTER, reg_qm_ddr_wr_reply_counter);

    *ddr_wr_reply_counter = RU_FIELD_GET(0, QM, QM_DDR_WR_REPLY_COUNTER, DDR_WR_REPLY_COUNTER, reg_qm_ddr_wr_reply_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_ddr_pipe_byte_counter_get(uint32_t *ddr_pipe)
{
    uint32_t reg_qm_ddr_pipe_byte_counter;

#ifdef VALIDATE_PARMS
    if (!ddr_pipe)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, QM_DDR_PIPE_BYTE_COUNTER, reg_qm_ddr_pipe_byte_counter);

    *ddr_pipe = RU_FIELD_GET(0, QM, QM_DDR_PIPE_BYTE_COUNTER, COUNTER, reg_qm_ddr_pipe_byte_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_abs_requeue_valid_counter_get(uint16_t *requeue_valid)
{
    uint32_t reg_qm_abs_requeue_valid_counter;

#ifdef VALIDATE_PARMS
    if (!requeue_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, QM_ABS_REQUEUE_VALID_COUNTER, reg_qm_abs_requeue_valid_counter);

    *requeue_valid = RU_FIELD_GET(0, QM, QM_ABS_REQUEUE_VALID_COUNTER, COUNTER, reg_qm_abs_requeue_valid_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_illegal_pd_capture_get(uint32_t idx, uint32_t *pd)
{
    uint32_t reg_qm_illegal_pd_capture;

#ifdef VALIDATE_PARMS
    if (!pd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, QM_ILLEGAL_PD_CAPTURE, reg_qm_illegal_pd_capture);

    *pd = RU_FIELD_GET(0, QM, QM_ILLEGAL_PD_CAPTURE, PD, reg_qm_illegal_pd_capture);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_ingress_processed_pd_capture_get(uint32_t idx, uint32_t *pd)
{
    uint32_t reg_qm_ingress_processed_pd_capture;

#ifdef VALIDATE_PARMS
    if (!pd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, QM_INGRESS_PROCESSED_PD_CAPTURE, reg_qm_ingress_processed_pd_capture);

    *pd = RU_FIELD_GET(0, QM, QM_INGRESS_PROCESSED_PD_CAPTURE, PD, reg_qm_ingress_processed_pd_capture);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_cm_processed_pd_capture_get(uint32_t idx, uint32_t *pd)
{
    uint32_t reg_qm_cm_processed_pd_capture;

#ifdef VALIDATE_PARMS
    if (!pd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, QM_CM_PROCESSED_PD_CAPTURE, reg_qm_cm_processed_pd_capture);

    *pd = RU_FIELD_GET(0, QM, QM_CM_PROCESSED_PD_CAPTURE, PD, reg_qm_cm_processed_pd_capture);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_grp_drop_cnt_get(uint32_t idx, uint32_t *fpm_grp_drop)
{
    uint32_t reg_fpm_grp_drop_cnt;

#ifdef VALIDATE_PARMS
    if (!fpm_grp_drop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, FPM_GRP_DROP_CNT, reg_fpm_grp_drop_cnt);

    *fpm_grp_drop = RU_FIELD_GET(0, QM, FPM_GRP_DROP_CNT, COUNTER, reg_fpm_grp_drop_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_pool_drop_cnt_get(uint32_t idx, uint32_t *fpm_drop)
{
    uint32_t reg_fpm_pool_drop_cnt;

#ifdef VALIDATE_PARMS
    if (!fpm_drop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, FPM_POOL_DROP_CNT, reg_fpm_pool_drop_cnt);

    *fpm_drop = RU_FIELD_GET(0, QM, FPM_POOL_DROP_CNT, COUNTER, reg_fpm_pool_drop_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_buffer_res_drop_cnt_get(uint32_t *counter)
{
    uint32_t reg_fpm_buffer_res_drop_cnt;

#ifdef VALIDATE_PARMS
    if (!counter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, FPM_BUFFER_RES_DROP_CNT, reg_fpm_buffer_res_drop_cnt);

    *counter = RU_FIELD_GET(0, QM, FPM_BUFFER_RES_DROP_CNT, COUNTER, reg_fpm_buffer_res_drop_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_psram_egress_cong_drp_cnt_get(uint32_t *counter)
{
    uint32_t reg_psram_egress_cong_drp_cnt;

#ifdef VALIDATE_PARMS
    if (!counter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, PSRAM_EGRESS_CONG_DRP_CNT, reg_psram_egress_cong_drp_cnt);

    *counter = RU_FIELD_GET(0, QM, PSRAM_EGRESS_CONG_DRP_CNT, COUNTER, reg_psram_egress_cong_drp_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_backpressure_set(uint8_t status)
{
    uint32_t reg_backpressure = 0;

#ifdef VALIDATE_PARMS
    if ((status >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_backpressure = RU_FIELD_SET(0, QM, BACKPRESSURE, STATUS, reg_backpressure, status);

    RU_REG_WRITE(0, QM, BACKPRESSURE, reg_backpressure);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_backpressure_get(uint8_t *status)
{
    uint32_t reg_backpressure;

#ifdef VALIDATE_PARMS
    if (!status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BACKPRESSURE, reg_backpressure);

    *status = RU_FIELD_GET(0, QM, BACKPRESSURE, STATUS, reg_backpressure);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_aqm_timestamp_curr_counter_get(uint32_t *value)
{
    uint32_t reg_aqm_timestamp_curr_counter;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, AQM_TIMESTAMP_CURR_COUNTER, reg_aqm_timestamp_curr_counter);

    *value = RU_FIELD_GET(0, QM, AQM_TIMESTAMP_CURR_COUNTER, VALUE, reg_aqm_timestamp_curr_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bb0_egr_msg_out_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_bb0_egr_msg_out_fifo_status;

#ifdef VALIDATE_PARMS
    if (!used_words || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BB0_EGR_MSG_OUT_FIFO_STATUS, reg_bb0_egr_msg_out_fifo_status);

    *used_words = RU_FIELD_GET(0, QM, BB0_EGR_MSG_OUT_FIFO_STATUS, USED_WORDS, reg_bb0_egr_msg_out_fifo_status);
    *empty = RU_FIELD_GET(0, QM, BB0_EGR_MSG_OUT_FIFO_STATUS, EMPTY, reg_bb0_egr_msg_out_fifo_status);
    *full = RU_FIELD_GET(0, QM, BB0_EGR_MSG_OUT_FIFO_STATUS, FULL, reg_bb0_egr_msg_out_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_count_pkt_not_pd_mode_bits_set(const qm_count_pkt_not_pd_mode_bits *count_pkt_not_pd_mode_bits)
{
    uint32_t reg_count_pkt_not_pd_mode_bits = 0;

#ifdef VALIDATE_PARMS
    if(!count_pkt_not_pd_mode_bits)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((count_pkt_not_pd_mode_bits->total_egress_accum_cnt_pkt >= _1BITS_MAX_VAL_) ||
       (count_pkt_not_pd_mode_bits->global_egress_drop_cnt_pkt >= _1BITS_MAX_VAL_) ||
       (count_pkt_not_pd_mode_bits->drop_ing_egr_cnt_pkt >= _1BITS_MAX_VAL_) ||
       (count_pkt_not_pd_mode_bits->fpm_grp_drop_cnt_pkt >= _1BITS_MAX_VAL_) ||
       (count_pkt_not_pd_mode_bits->qm_pd_congestion_drop_cnt_pkt >= _1BITS_MAX_VAL_) ||
       (count_pkt_not_pd_mode_bits->ddr_pd_congestion_drop_cnt_pkt >= _1BITS_MAX_VAL_) ||
       (count_pkt_not_pd_mode_bits->wred_drop_cnt_pkt >= _1BITS_MAX_VAL_) ||
       (count_pkt_not_pd_mode_bits->good_lvl2_pkts_cnt_pkt >= _1BITS_MAX_VAL_) ||
       (count_pkt_not_pd_mode_bits->good_lvl1_pkts_cnt_pkt >= _1BITS_MAX_VAL_) ||
       (count_pkt_not_pd_mode_bits->qm_total_valid_cnt_pkt >= _1BITS_MAX_VAL_) ||
       (count_pkt_not_pd_mode_bits->dqm_valid_cnt_pkt >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_count_pkt_not_pd_mode_bits = RU_FIELD_SET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, TOTAL_EGRESS_ACCUM_CNT_PKT, reg_count_pkt_not_pd_mode_bits, count_pkt_not_pd_mode_bits->total_egress_accum_cnt_pkt);
    reg_count_pkt_not_pd_mode_bits = RU_FIELD_SET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, GLOBAL_EGRESS_DROP_CNT_PKT, reg_count_pkt_not_pd_mode_bits, count_pkt_not_pd_mode_bits->global_egress_drop_cnt_pkt);
    reg_count_pkt_not_pd_mode_bits = RU_FIELD_SET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, DROP_ING_EGR_CNT_PKT, reg_count_pkt_not_pd_mode_bits, count_pkt_not_pd_mode_bits->drop_ing_egr_cnt_pkt);
    reg_count_pkt_not_pd_mode_bits = RU_FIELD_SET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, FPM_GRP_DROP_CNT_PKT, reg_count_pkt_not_pd_mode_bits, count_pkt_not_pd_mode_bits->fpm_grp_drop_cnt_pkt);
    reg_count_pkt_not_pd_mode_bits = RU_FIELD_SET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, QM_PD_CONGESTION_DROP_CNT_PKT, reg_count_pkt_not_pd_mode_bits, count_pkt_not_pd_mode_bits->qm_pd_congestion_drop_cnt_pkt);
    reg_count_pkt_not_pd_mode_bits = RU_FIELD_SET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, DDR_PD_CONGESTION_DROP_CNT_PKT, reg_count_pkt_not_pd_mode_bits, count_pkt_not_pd_mode_bits->ddr_pd_congestion_drop_cnt_pkt);
    reg_count_pkt_not_pd_mode_bits = RU_FIELD_SET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, WRED_DROP_CNT_PKT, reg_count_pkt_not_pd_mode_bits, count_pkt_not_pd_mode_bits->wred_drop_cnt_pkt);
    reg_count_pkt_not_pd_mode_bits = RU_FIELD_SET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, GOOD_LVL2_PKTS_CNT_PKT, reg_count_pkt_not_pd_mode_bits, count_pkt_not_pd_mode_bits->good_lvl2_pkts_cnt_pkt);
    reg_count_pkt_not_pd_mode_bits = RU_FIELD_SET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, GOOD_LVL1_PKTS_CNT_PKT, reg_count_pkt_not_pd_mode_bits, count_pkt_not_pd_mode_bits->good_lvl1_pkts_cnt_pkt);
    reg_count_pkt_not_pd_mode_bits = RU_FIELD_SET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, QM_TOTAL_VALID_CNT_PKT, reg_count_pkt_not_pd_mode_bits, count_pkt_not_pd_mode_bits->qm_total_valid_cnt_pkt);
    reg_count_pkt_not_pd_mode_bits = RU_FIELD_SET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, DQM_VALID_CNT_PKT, reg_count_pkt_not_pd_mode_bits, count_pkt_not_pd_mode_bits->dqm_valid_cnt_pkt);

    RU_REG_WRITE(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, reg_count_pkt_not_pd_mode_bits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_count_pkt_not_pd_mode_bits_get(qm_count_pkt_not_pd_mode_bits *count_pkt_not_pd_mode_bits)
{
    uint32_t reg_count_pkt_not_pd_mode_bits;

#ifdef VALIDATE_PARMS
    if (!count_pkt_not_pd_mode_bits)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, reg_count_pkt_not_pd_mode_bits);

    count_pkt_not_pd_mode_bits->total_egress_accum_cnt_pkt = RU_FIELD_GET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, TOTAL_EGRESS_ACCUM_CNT_PKT, reg_count_pkt_not_pd_mode_bits);
    count_pkt_not_pd_mode_bits->global_egress_drop_cnt_pkt = RU_FIELD_GET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, GLOBAL_EGRESS_DROP_CNT_PKT, reg_count_pkt_not_pd_mode_bits);
    count_pkt_not_pd_mode_bits->drop_ing_egr_cnt_pkt = RU_FIELD_GET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, DROP_ING_EGR_CNT_PKT, reg_count_pkt_not_pd_mode_bits);
    count_pkt_not_pd_mode_bits->fpm_grp_drop_cnt_pkt = RU_FIELD_GET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, FPM_GRP_DROP_CNT_PKT, reg_count_pkt_not_pd_mode_bits);
    count_pkt_not_pd_mode_bits->qm_pd_congestion_drop_cnt_pkt = RU_FIELD_GET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, QM_PD_CONGESTION_DROP_CNT_PKT, reg_count_pkt_not_pd_mode_bits);
    count_pkt_not_pd_mode_bits->ddr_pd_congestion_drop_cnt_pkt = RU_FIELD_GET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, DDR_PD_CONGESTION_DROP_CNT_PKT, reg_count_pkt_not_pd_mode_bits);
    count_pkt_not_pd_mode_bits->wred_drop_cnt_pkt = RU_FIELD_GET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, WRED_DROP_CNT_PKT, reg_count_pkt_not_pd_mode_bits);
    count_pkt_not_pd_mode_bits->good_lvl2_pkts_cnt_pkt = RU_FIELD_GET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, GOOD_LVL2_PKTS_CNT_PKT, reg_count_pkt_not_pd_mode_bits);
    count_pkt_not_pd_mode_bits->good_lvl1_pkts_cnt_pkt = RU_FIELD_GET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, GOOD_LVL1_PKTS_CNT_PKT, reg_count_pkt_not_pd_mode_bits);
    count_pkt_not_pd_mode_bits->qm_total_valid_cnt_pkt = RU_FIELD_GET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, QM_TOTAL_VALID_CNT_PKT, reg_count_pkt_not_pd_mode_bits);
    count_pkt_not_pd_mode_bits->dqm_valid_cnt_pkt = RU_FIELD_GET(0, QM, COUNT_PKT_NOT_PD_MODE_BITS, DQM_VALID_CNT_PKT, reg_count_pkt_not_pd_mode_bits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_cm_residue_data_get(uint16_t idx, uint32_t *data)
{
    uint32_t reg_data;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((idx >= 1024))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, DATA, reg_data);

    *data = RU_FIELD_GET(0, QM, DATA, DATA, reg_data);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_global_cfg_qm_enable_ctrl,
    bdmf_address_global_cfg_qm_sw_rst_ctrl,
    bdmf_address_global_cfg_qm_general_ctrl,
    bdmf_address_global_cfg_fpm_control,
    bdmf_address_global_cfg_ddr_byte_congestion_control,
    bdmf_address_global_cfg_ddr_byte_congestion_lower_thr,
    bdmf_address_global_cfg_ddr_byte_congestion_mid_thr,
    bdmf_address_global_cfg_ddr_byte_congestion_higher_thr,
    bdmf_address_global_cfg_ddr_pd_congestion_control,
    bdmf_address_global_cfg_qm_pd_congestion_control,
    bdmf_address_global_cfg_abs_drop_queue,
    bdmf_address_global_cfg_aggregation_ctrl,
    bdmf_address_global_cfg_aggregation_ctrl2,
    bdmf_address_global_cfg_fpm_base_addr,
    bdmf_address_global_cfg_fpm_coherent_base_addr,
    bdmf_address_global_cfg_ddr_sop_offset,
    bdmf_address_global_cfg_epon_overhead_ctrl,
    bdmf_address_global_cfg_bbhtx_fifo_addr,
    bdmf_address_global_cfg_dqm_full,
    bdmf_address_global_cfg_dqm_not_empty,
    bdmf_address_global_cfg_dqm_pop_ready,
    bdmf_address_global_cfg_aggregation_context_valid,
    bdmf_address_global_cfg_qm_egress_flush_queue,
    bdmf_address_global_cfg_qm_aggregation_timer_ctrl,
    bdmf_address_global_cfg_qm_fpm_ug_gbl_cnt,
    bdmf_address_global_cfg_ddr_spare_room,
    bdmf_address_global_cfg_dummy_spare_room_profile_id,
    bdmf_address_global_cfg_dqm_ubus_ctrl,
    bdmf_address_global_cfg_mem_auto_init,
    bdmf_address_global_cfg_mem_auto_init_sts,
    bdmf_address_global_cfg_fpm_mpm_enhancement_pool_size_tokens,
    bdmf_address_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte,
    bdmf_address_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte,
    bdmf_address_global_cfg_mc_ctrl,
    bdmf_address_global_cfg_aqm_clk_counter_cycle,
    bdmf_address_global_cfg_aqm_push_to_empty_thr,
    bdmf_address_global_cfg_qm_general_ctrl2,
    bdmf_address_fpm_pools_thr,
    bdmf_address_fpm_usr_grp_lower_thr,
    bdmf_address_fpm_usr_grp_mid_thr,
    bdmf_address_fpm_usr_grp_higher_thr,
    bdmf_address_fpm_usr_grp_cnt,
    bdmf_address_runner_grp_rnr_config,
    bdmf_address_runner_grp_queue_config,
    bdmf_address_runner_grp_pdfifo_config,
    bdmf_address_runner_grp_update_fifo_config,
    bdmf_address_intr_ctrl_isr,
    bdmf_address_intr_ctrl_ism,
    bdmf_address_intr_ctrl_ier,
    bdmf_address_intr_ctrl_itr,
    bdmf_address_clk_gate_clk_gate_cntrl,
    bdmf_address_cpu_indr_port_cpu_pd_indirect_ctrl,
    bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_0,
    bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_1,
    bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_2,
    bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_3,
    bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_0,
    bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_1,
    bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_2,
    bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_3,
    bdmf_address_queue_context_context,
    bdmf_address_wred_profile_color_min_thr_0,
    bdmf_address_wred_profile_color_min_thr_1,
    bdmf_address_wred_profile_color_max_thr_0,
    bdmf_address_wred_profile_color_max_thr_1,
    bdmf_address_wred_profile_color_slope_0,
    bdmf_address_wred_profile_color_slope_1,
    bdmf_address_copy_decision_profile_thr,
    bdmf_address_timestamp_res_profile_value,
    bdmf_address_global_egress_drop_counter_counter,
    bdmf_address_global_egress_aqm_drop_counter_counter,
    bdmf_address_total_valid_counter_counter,
    bdmf_address_dqm_valid_counter_counter,
    bdmf_address_drop_counter_counter,
    bdmf_address_total_egress_accumulated_counter_counter,
    bdmf_address_epon_rpt_cnt_counter,
    bdmf_address_epon_rpt_cnt_queue_status,
    bdmf_address_rd_data_pool0,
    bdmf_address_rd_data_pool1,
    bdmf_address_rd_data_pool2,
    bdmf_address_rd_data_pool3,
    bdmf_address_pop_3,
    bdmf_address_pdfifo_ptr,
    bdmf_address_update_fifo_ptr,
    bdmf_address_rd_data_2,
    bdmf_address_pop_2,
    bdmf_address_rd_data_1,
    bdmf_address_pop_1,
    bdmf_address_rd_data,
    bdmf_address_pop,
    bdmf_address_cm_common_input_fifo_data,
    bdmf_address_normal_rmt_fifo_data,
    bdmf_address_non_delayed_rmt_fifo_data,
    bdmf_address_egress_data_fifo_data,
    bdmf_address_egress_rr_fifo_data,
    bdmf_address_egress_bb_input_fifo_data,
    bdmf_address_egress_bb_output_fifo_data,
    bdmf_address_bb_output_fifo_data,
    bdmf_address_non_delayed_out_fifo_data,
    bdmf_address_bb0_egr_msg_out_fifo_data,
    bdmf_address_fpm_buffer_reservation_data,
    bdmf_address_port_cfg,
    bdmf_address_fc_ug_mask_ug_en,
    bdmf_address_fc_queue_mask,
    bdmf_address_fc_queue_range1_start,
    bdmf_address_fc_queue_range2_start,
    bdmf_address_dbg,
    bdmf_address_ug_occupancy_status,
    bdmf_address_queue_range1_occupancy_status,
    bdmf_address_queue_range2_occupancy_status,
    bdmf_address_context_data,
    bdmf_address_debug_sel,
    bdmf_address_debug_bus_lsb,
    bdmf_address_debug_bus_msb,
    bdmf_address_qm_spare_config,
    bdmf_address_good_lvl1_pkts_cnt,
    bdmf_address_good_lvl1_bytes_cnt,
    bdmf_address_good_lvl2_pkts_cnt,
    bdmf_address_good_lvl2_bytes_cnt,
    bdmf_address_copied_pkts_cnt,
    bdmf_address_copied_bytes_cnt,
    bdmf_address_agg_pkts_cnt,
    bdmf_address_agg_bytes_cnt,
    bdmf_address_agg_1_pkts_cnt,
    bdmf_address_agg_2_pkts_cnt,
    bdmf_address_agg_3_pkts_cnt,
    bdmf_address_agg_4_pkts_cnt,
    bdmf_address_wred_drop_cnt,
    bdmf_address_fpm_congestion_drop_cnt,
    bdmf_address_ddr_pd_congestion_drop_cnt,
    bdmf_address_ddr_byte_congestion_drop_cnt,
    bdmf_address_qm_pd_congestion_drop_cnt,
    bdmf_address_qm_abs_requeue_cnt,
    bdmf_address_fpm_prefetch_fifo0_status,
    bdmf_address_fpm_prefetch_fifo1_status,
    bdmf_address_fpm_prefetch_fifo2_status,
    bdmf_address_fpm_prefetch_fifo3_status,
    bdmf_address_normal_rmt_fifo_status,
    bdmf_address_non_delayed_rmt_fifo_status,
    bdmf_address_non_delayed_out_fifo_status,
    bdmf_address_pre_cm_fifo_status,
    bdmf_address_cm_rd_pd_fifo_status,
    bdmf_address_cm_wr_pd_fifo_status,
    bdmf_address_cm_common_input_fifo_status,
    bdmf_address_bb0_output_fifo_status,
    bdmf_address_bb1_output_fifo_status,
    bdmf_address_bb1_input_fifo_status,
    bdmf_address_egress_data_fifo_status,
    bdmf_address_egress_rr_fifo_status,
    bdmf_address_bb_route_ovr,
    bdmf_address_qm_ingress_stat,
    bdmf_address_qm_egress_stat,
    bdmf_address_qm_cm_stat,
    bdmf_address_qm_fpm_prefetch_stat,
    bdmf_address_qm_connect_ack_counter,
    bdmf_address_qm_ddr_wr_reply_counter,
    bdmf_address_qm_ddr_pipe_byte_counter,
    bdmf_address_qm_abs_requeue_valid_counter,
    bdmf_address_qm_illegal_pd_capture,
    bdmf_address_qm_ingress_processed_pd_capture,
    bdmf_address_qm_cm_processed_pd_capture,
    bdmf_address_fpm_grp_drop_cnt,
    bdmf_address_fpm_pool_drop_cnt,
    bdmf_address_fpm_buffer_res_drop_cnt,
    bdmf_address_psram_egress_cong_drp_cnt,
    bdmf_address_backpressure,
    bdmf_address_aqm_timestamp_curr_counter,
    bdmf_address_bb0_egr_msg_out_fifo_status,
    bdmf_address_count_pkt_not_pd_mode_bits,
    bdmf_address_data,
}
bdmf_address;

static int ag_drv_qm_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_qm_ddr_cong_ctrl:
    {
        qm_ddr_cong_ctrl ddr_cong_ctrl = { .ddr_byte_congestion_drop_enable = parm[1].value.unumber, .ddr_bytes_lower_thr = parm[2].value.unumber, .ddr_bytes_mid_thr = parm[3].value.unumber, .ddr_bytes_higher_thr = parm[4].value.unumber, .ddr_pd_congestion_drop_enable = parm[5].value.unumber, .ddr_pipe_lower_thr = parm[6].value.unumber, .ddr_pipe_higher_thr = parm[7].value.unumber};
        ag_err = ag_drv_qm_ddr_cong_ctrl_set(&ddr_cong_ctrl);
        break;
    }
    case cli_qm_fpm_ug_thr:
    {
        qm_fpm_ug_thr fpm_ug_thr = { .lower_thr = parm[2].value.unumber, .mid_thr = parm[3].value.unumber, .higher_thr = parm[4].value.unumber};
        ag_err = ag_drv_qm_fpm_ug_thr_set(parm[1].value.unumber, &fpm_ug_thr);
        break;
    }
    case cli_qm_rnr_group_cfg:
    {
        qm_rnr_group_cfg rnr_group_cfg = { .start_queue = parm[2].value.unumber, .end_queue = parm[3].value.unumber, .pd_fifo_base = parm[4].value.unumber, .pd_fifo_size = parm[5].value.unumber, .upd_fifo_base = parm[6].value.unumber, .upd_fifo_size = parm[7].value.unumber, .rnr_bb_id = parm[8].value.unumber, .rnr_task = parm[9].value.unumber, .rnr_enable = parm[10].value.unumber};
        ag_err = ag_drv_qm_rnr_group_cfg_set(parm[1].value.unumber, &rnr_group_cfg);
        break;
    }
    case cli_qm_cpu_pd_indirect_wr_data:
        ag_err = ag_drv_qm_cpu_pd_indirect_wr_data_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_qm_wred_profile_cfg:
    {
        qm_wred_profile_cfg wred_profile_cfg = { .min_thr0 = parm[2].value.unumber, .flw_ctrl_en0 = parm[3].value.unumber, .min_thr1 = parm[4].value.unumber, .flw_ctrl_en1 = parm[5].value.unumber, .max_thr0 = parm[6].value.unumber, .max_thr1 = parm[7].value.unumber, .slope_mantissa0 = parm[8].value.unumber, .slope_exp0 = parm[9].value.unumber, .slope_mantissa1 = parm[10].value.unumber, .slope_exp1 = parm[11].value.unumber};
        ag_err = ag_drv_qm_wred_profile_cfg_set(parm[1].value.unumber, &wred_profile_cfg);
        break;
    }
    case cli_qm_enable_ctrl:
    {
        qm_enable_ctrl enable_ctrl = { .fpm_prefetch_enable = parm[1].value.unumber, .reorder_credit_enable = parm[2].value.unumber, .dqm_pop_enable = parm[3].value.unumber, .rmt_fixed_arb_enable = parm[4].value.unumber, .dqm_push_fixed_arb_enable = parm[5].value.unumber, .aqm_clk_counter_enable = parm[6].value.unumber, .aqm_timestamp_counter_enable = parm[7].value.unumber, .aqm_timestamp_write_to_pd_enable = parm[8].value.unumber};
        ag_err = ag_drv_qm_enable_ctrl_set(&enable_ctrl);
        break;
    }
    case cli_qm_reset_ctrl:
    {
        qm_reset_ctrl reset_ctrl = { .fpm_prefetch0_sw_rst = parm[1].value.unumber, .fpm_prefetch1_sw_rst = parm[2].value.unumber, .fpm_prefetch2_sw_rst = parm[3].value.unumber, .fpm_prefetch3_sw_rst = parm[4].value.unumber, .normal_rmt_sw_rst = parm[5].value.unumber, .non_delayed_rmt_sw_rst = parm[6].value.unumber, .pre_cm_fifo_sw_rst = parm[7].value.unumber, .cm_rd_pd_fifo_sw_rst = parm[8].value.unumber, .cm_wr_pd_fifo_sw_rst = parm[9].value.unumber, .bb0_output_fifo_sw_rst = parm[10].value.unumber, .bb1_output_fifo_sw_rst = parm[11].value.unumber, .bb1_input_fifo_sw_rst = parm[12].value.unumber, .tm_fifo_ptr_sw_rst = parm[13].value.unumber, .non_delayed_out_fifo_sw_rst = parm[14].value.unumber, .bb0_egr_msg_out_fifo_sw_rst = parm[15].value.unumber};
        ag_err = ag_drv_qm_reset_ctrl_set(&reset_ctrl);
        break;
    }
    case cli_qm_drop_counters_ctrl:
    {
        qm_drop_counters_ctrl drop_counters_ctrl = { .read_clear_pkts = parm[1].value.unumber, .read_clear_bytes = parm[2].value.unumber, .disable_wrap_around_pkts = parm[3].value.unumber, .disable_wrap_around_bytes = parm[4].value.unumber, .free_with_context_last_search = parm[5].value.unumber, .wred_disable = parm[6].value.unumber, .ddr_pd_congestion_disable = parm[7].value.unumber, .ddr_byte_congestion_disable = parm[8].value.unumber, .ddr_occupancy_disable = parm[9].value.unumber, .ddr_fpm_congestion_disable = parm[10].value.unumber, .fpm_ug_disable = parm[11].value.unumber, .queue_occupancy_ddr_copy_decision_disable = parm[12].value.unumber, .psram_occupancy_ddr_copy_decision_disable = parm[13].value.unumber, .dont_send_mc_bit_to_bbh = parm[14].value.unumber, .close_aggregation_on_timeout_disable = parm[15].value.unumber, .fpm_congestion_buf_release_mechanism_disable = parm[16].value.unumber, .fpm_buffer_global_res_enable = parm[17].value.unumber, .qm_preserve_pd_with_fpm = parm[18].value.unumber, .qm_residue_per_queue = parm[19].value.unumber, .ghost_rpt_update_after_close_agg_en = parm[20].value.unumber, .fpm_ug_flow_ctrl_disable = parm[21].value.unumber, .ddr_write_multi_slave_en = parm[22].value.unumber, .ddr_pd_congestion_agg_priority = parm[23].value.unumber, .psram_occupancy_drop_disable = parm[24].value.unumber, .qm_ddr_write_alignment = parm[25].value.unumber, .exclusive_dont_drop = parm[26].value.unumber, .dqmol_jira_973_fix_enable = parm[27].value.unumber, .gpon_dbr_ceil = parm[28].value.unumber, .drop_cnt_wred_drops = parm[29].value.unumber, .same_sec_lvl_bit_agg_en = parm[30].value.unumber, .glbl_egr_drop_cnt_read_clear_enable = parm[31].value.unumber, .glbl_egr_aqm_drop_cnt_read_clear_enable = parm[32].value.unumber};
        ag_err = ag_drv_qm_drop_counters_ctrl_set(&drop_counters_ctrl);
        break;
    }
    case cli_qm_fpm_ctrl:
    {
        qm_fpm_ctrl fpm_ctrl = { .fpm_pool_bp_enable = parm[1].value.unumber, .fpm_congestion_bp_enable = parm[2].value.unumber, .fpm_force_bp_lvl = parm[3].value.unumber, .fpm_prefetch_granularity = parm[4].value.unumber, .fpm_prefetch_min_pool_size = parm[5].value.unumber, .fpm_prefetch_pending_req_limit = parm[6].value.unumber, .fpm_override_bb_id_en = parm[7].value.unumber, .fpm_override_bb_id_value = parm[8].value.unumber};
        ag_err = ag_drv_qm_fpm_ctrl_set(&fpm_ctrl);
        break;
    }
    case cli_qm_qm_pd_cong_ctrl:
        ag_err = ag_drv_qm_qm_pd_cong_ctrl_set(parm[1].value.unumber);
        break;
    case cli_qm_global_cfg_abs_drop_queue:
        ag_err = ag_drv_qm_global_cfg_abs_drop_queue_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_global_cfg_aggregation_ctrl:
    {
        qm_global_cfg_aggregation_ctrl global_cfg_aggregation_ctrl = { .max_agg_bytes = parm[1].value.unumber, .max_agg_pkts = parm[2].value.unumber, .agg_ovr_512b_en = parm[3].value.unumber, .max_agg_pkt_size = parm[4].value.unumber, .min_agg_pkt_size = parm[5].value.unumber};
        ag_err = ag_drv_qm_global_cfg_aggregation_ctrl_set(&global_cfg_aggregation_ctrl);
        break;
    }
    case cli_qm_global_cfg_aggregation_ctrl2:
        ag_err = ag_drv_qm_global_cfg_aggregation_ctrl2_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_fpm_base_addr:
        ag_err = ag_drv_qm_fpm_base_addr_set(parm[1].value.unumber);
        break;
    case cli_qm_global_cfg_fpm_coherent_base_addr:
        ag_err = ag_drv_qm_global_cfg_fpm_coherent_base_addr_set(parm[1].value.unumber);
        break;
    case cli_qm_ddr_sop_offset:
        ag_err = ag_drv_qm_ddr_sop_offset_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_epon_overhead_ctrl:
    {
        qm_epon_overhead_ctrl epon_overhead_ctrl = { .epon_line_rate = parm[1].value.unumber, .epon_crc_add_disable = parm[2].value.unumber, .mac_flow_overwrite_crc_en = parm[3].value.unumber, .mac_flow_overwrite_crc = parm[4].value.unumber, .fec_ipg_length = parm[5].value.unumber};
        ag_err = ag_drv_qm_epon_overhead_ctrl_set(&epon_overhead_ctrl);
        break;
    }
    case cli_qm_global_cfg_bbhtx_fifo_addr:
        ag_err = ag_drv_qm_global_cfg_bbhtx_fifo_addr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_global_cfg_qm_egress_flush_queue:
        ag_err = ag_drv_qm_global_cfg_qm_egress_flush_queue_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_global_cfg_qm_aggregation_timer_ctrl:
        ag_err = ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_qm_global_cfg_qm_fpm_ug_gbl_cnt:
        ag_err = ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_set(parm[1].value.unumber);
        break;
    case cli_qm_qm_ddr_spare_room:
        ag_err = ag_drv_qm_qm_ddr_spare_room_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_qm_global_cfg_dummy_spare_room_profile_id:
        ag_err = ag_drv_qm_global_cfg_dummy_spare_room_profile_id_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_global_cfg_dqm_ubus_ctrl:
        ag_err = ag_drv_qm_global_cfg_dqm_ubus_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_qm_global_cfg_mem_auto_init:
        ag_err = ag_drv_qm_global_cfg_mem_auto_init_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_qm_global_cfg_fpm_mpm_enhancement_pool_size_tokens:
        ag_err = ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_size_tokens_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte:
        ag_err = ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte:
        ag_err = ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_global_cfg_mc_ctrl:
        ag_err = ag_drv_qm_global_cfg_mc_ctrl_set(parm[1].value.unumber);
        break;
    case cli_qm_global_cfg_aqm_clk_counter_cycle:
        ag_err = ag_drv_qm_global_cfg_aqm_clk_counter_cycle_set(parm[1].value.unumber);
        break;
    case cli_qm_global_cfg_aqm_push_to_empty_thr:
        ag_err = ag_drv_qm_global_cfg_aqm_push_to_empty_thr_set(parm[1].value.unumber);
        break;
    case cli_qm_global_cfg_qm_general_ctrl2:
    {
        qm_global_cfg_qm_general_ctrl2 global_cfg_qm_general_ctrl2 = { .egress_accumulated_cnt_pkts_read_clear_enable = parm[1].value.unumber, .egress_accumulated_cnt_bytes_read_clear_enable = parm[2].value.unumber, .agg_closure_suspend_on_bp = parm[3].value.unumber, .bufmng_en_or_ug_cntr = parm[4].value.unumber, .dqm_to_fpm_ubus_or_fpmini = parm[5].value.unumber, .agg_closure_suspend_on_fpm_congestion_disable = parm[6].value.unumber};
        ag_err = ag_drv_qm_global_cfg_qm_general_ctrl2_set(&global_cfg_qm_general_ctrl2);
        break;
    }
    case cli_qm_fpm_pool_thr:
    {
        qm_fpm_pool_thr fpm_pool_thr = { .lower_thr = parm[2].value.unumber, .higher_thr = parm[3].value.unumber};
        ag_err = ag_drv_qm_fpm_pool_thr_set(parm[1].value.unumber, &fpm_pool_thr);
        break;
    }
    case cli_qm_fpm_ug_cnt:
        ag_err = ag_drv_qm_fpm_ug_cnt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_intr_ctrl_isr:
    {
        qm_intr_ctrl_isr intr_ctrl_isr = { .qm_dqm_pop_on_empty = parm[1].value.unumber, .qm_dqm_push_on_full = parm[2].value.unumber, .qm_cpu_pop_on_empty = parm[3].value.unumber, .qm_cpu_push_on_full = parm[4].value.unumber, .qm_normal_queue_pd_no_credit = parm[5].value.unumber, .qm_non_delayed_queue_pd_no_credit = parm[6].value.unumber, .qm_non_valid_queue = parm[7].value.unumber, .qm_agg_coherent_inconsistency = parm[8].value.unumber, .qm_force_copy_on_non_delayed = parm[9].value.unumber, .qm_fpm_pool_size_nonexistent = parm[10].value.unumber, .qm_target_mem_abs_contradiction = parm[11].value.unumber, .qm_1588_drop = parm[12].value.unumber, .qm_1588_multicast_contradiction = parm[13].value.unumber, .qm_byte_drop_cnt_overrun = parm[14].value.unumber, .qm_pkt_drop_cnt_overrun = parm[15].value.unumber, .qm_total_byte_cnt_underrun = parm[16].value.unumber, .qm_total_pkt_cnt_underrun = parm[17].value.unumber, .qm_fpm_ug0_underrun = parm[18].value.unumber, .qm_fpm_ug1_underrun = parm[19].value.unumber, .qm_fpm_ug2_underrun = parm[20].value.unumber, .qm_fpm_ug3_underrun = parm[21].value.unumber, .qm_timer_wraparound = parm[22].value.unumber, .qm_copy_plen_zero = parm[23].value.unumber, .qm_ingress_bb_unexpected_msg = parm[24].value.unumber, .qm_egress_bb_unexpected_msg = parm[25].value.unumber, .dqm_reached_full = parm[26].value.unumber, .qm_fpmini_intr = parm[27].value.unumber};
        ag_err = ag_drv_qm_intr_ctrl_isr_set(&intr_ctrl_isr);
        break;
    }
    case cli_qm_intr_ctrl_ier:
        ag_err = ag_drv_qm_intr_ctrl_ier_set(parm[1].value.unumber);
        break;
    case cli_qm_intr_ctrl_itr:
        ag_err = ag_drv_qm_intr_ctrl_itr_set(parm[1].value.unumber);
        break;
    case cli_qm_clk_gate_clk_gate_cntrl:
    {
        qm_clk_gate_clk_gate_cntrl clk_gate_clk_gate_cntrl = { .bypass_clk_gate = parm[1].value.unumber, .timer_val = parm[2].value.unumber, .keep_alive_en = parm[3].value.unumber, .keep_alive_intrvl = parm[4].value.unumber, .keep_alive_cyc = parm[5].value.unumber};
        ag_err = ag_drv_qm_clk_gate_clk_gate_cntrl_set(&clk_gate_clk_gate_cntrl);
        break;
    }
    case cli_qm_cpu_indr_port_cpu_pd_indirect_ctrl:
        ag_err = ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_qm_q_context:
    {
        qm_q_context q_context = { .wred_profile = parm[2].value.unumber, .copy_dec_profile = parm[3].value.unumber, .ddr_copy_disable = parm[4].value.unumber, .aggregation_disable = parm[5].value.unumber, .fpm_ug_or_bufmng = parm[6].value.unumber, .exclusive_priority = parm[7].value.unumber, .q_802_1ae = parm[8].value.unumber, .sci = parm[9].value.unumber, .fec_enable = parm[10].value.unumber, .res_profile = parm[11].value.unumber, .spare_room_0 = parm[12].value.unumber, .spare_room_1 = parm[13].value.unumber, .service_queue_profile = parm[14].value.unumber, .timestamp_res_profile = parm[15].value.unumber};
        ag_err = ag_drv_qm_q_context_set(parm[1].value.unumber, &q_context);
        break;
    }
    case cli_qm_copy_decision_profile:
        ag_err = ag_drv_qm_copy_decision_profile_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_qm_timestamp_res_profile:
        ag_err = ag_drv_qm_timestamp_res_profile_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_total_valid_cnt:
        ag_err = ag_drv_qm_total_valid_cnt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_dqm_valid_cnt:
        ag_err = ag_drv_qm_dqm_valid_cnt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_epon_q_byte_cnt:
        ag_err = ag_drv_qm_epon_q_byte_cnt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_pop_3:
        ag_err = ag_drv_qm_pop_3_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_qm_pop_2:
        ag_err = ag_drv_qm_pop_2_set(parm[1].value.unumber);
        break;
    case cli_qm_pop_1:
        ag_err = ag_drv_qm_pop_1_set(parm[1].value.unumber);
        break;
    case cli_qm_fpm_buffer_reservation_data:
        ag_err = ag_drv_qm_fpm_buffer_reservation_data_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_port_cfg:
        ag_err = ag_drv_qm_port_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_qm_fc_ug_mask_ug_en:
        ag_err = ag_drv_qm_fc_ug_mask_ug_en_set(parm[1].value.unumber);
        break;
    case cli_qm_fc_queue_mask:
        ag_err = ag_drv_qm_fc_queue_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_fc_queue_range1_start:
        ag_err = ag_drv_qm_fc_queue_range1_start_set(parm[1].value.unumber);
        break;
    case cli_qm_fc_queue_range2_start:
        ag_err = ag_drv_qm_fc_queue_range2_start_set(parm[1].value.unumber);
        break;
    case cli_qm_debug_sel:
        ag_err = ag_drv_qm_debug_sel_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_bb_route_ovr:
        ag_err = ag_drv_qm_bb_route_ovr_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_qm_backpressure:
        ag_err = ag_drv_qm_backpressure_set(parm[1].value.unumber);
        break;
    case cli_qm_count_pkt_not_pd_mode_bits:
    {
        qm_count_pkt_not_pd_mode_bits count_pkt_not_pd_mode_bits = { .total_egress_accum_cnt_pkt = parm[1].value.unumber, .global_egress_drop_cnt_pkt = parm[2].value.unumber, .drop_ing_egr_cnt_pkt = parm[3].value.unumber, .fpm_grp_drop_cnt_pkt = parm[4].value.unumber, .qm_pd_congestion_drop_cnt_pkt = parm[5].value.unumber, .ddr_pd_congestion_drop_cnt_pkt = parm[6].value.unumber, .wred_drop_cnt_pkt = parm[7].value.unumber, .good_lvl2_pkts_cnt_pkt = parm[8].value.unumber, .good_lvl1_pkts_cnt_pkt = parm[9].value.unumber, .qm_total_valid_cnt_pkt = parm[10].value.unumber, .dqm_valid_cnt_pkt = parm[11].value.unumber};
        ag_err = ag_drv_qm_count_pkt_not_pd_mode_bits_set(&count_pkt_not_pd_mode_bits);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_qm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_qm_ddr_cong_ctrl:
    {
        qm_ddr_cong_ctrl ddr_cong_ctrl;
        ag_err = ag_drv_qm_ddr_cong_ctrl_get(&ddr_cong_ctrl);
        bdmf_session_print(session, "ddr_byte_congestion_drop_enable = %u = 0x%x\n", ddr_cong_ctrl.ddr_byte_congestion_drop_enable, ddr_cong_ctrl.ddr_byte_congestion_drop_enable);
        bdmf_session_print(session, "ddr_bytes_lower_thr = %u = 0x%x\n", ddr_cong_ctrl.ddr_bytes_lower_thr, ddr_cong_ctrl.ddr_bytes_lower_thr);
        bdmf_session_print(session, "ddr_bytes_mid_thr = %u = 0x%x\n", ddr_cong_ctrl.ddr_bytes_mid_thr, ddr_cong_ctrl.ddr_bytes_mid_thr);
        bdmf_session_print(session, "ddr_bytes_higher_thr = %u = 0x%x\n", ddr_cong_ctrl.ddr_bytes_higher_thr, ddr_cong_ctrl.ddr_bytes_higher_thr);
        bdmf_session_print(session, "ddr_pd_congestion_drop_enable = %u = 0x%x\n", ddr_cong_ctrl.ddr_pd_congestion_drop_enable, ddr_cong_ctrl.ddr_pd_congestion_drop_enable);
        bdmf_session_print(session, "ddr_pipe_lower_thr = %u = 0x%x\n", ddr_cong_ctrl.ddr_pipe_lower_thr, ddr_cong_ctrl.ddr_pipe_lower_thr);
        bdmf_session_print(session, "ddr_pipe_higher_thr = %u = 0x%x\n", ddr_cong_ctrl.ddr_pipe_higher_thr, ddr_cong_ctrl.ddr_pipe_higher_thr);
        break;
    }
    case cli_qm_is_queue_not_empty:
    {
        bdmf_boolean data;
        ag_err = ag_drv_qm_is_queue_not_empty_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_is_queue_pop_ready:
    {
        bdmf_boolean data;
        ag_err = ag_drv_qm_is_queue_pop_ready_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_is_queue_full:
    {
        bdmf_boolean data;
        ag_err = ag_drv_qm_is_queue_full_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_fpm_ug_thr:
    {
        qm_fpm_ug_thr fpm_ug_thr;
        ag_err = ag_drv_qm_fpm_ug_thr_get(parm[1].value.unumber, &fpm_ug_thr);
        bdmf_session_print(session, "lower_thr = %u = 0x%x\n", fpm_ug_thr.lower_thr, fpm_ug_thr.lower_thr);
        bdmf_session_print(session, "mid_thr = %u = 0x%x\n", fpm_ug_thr.mid_thr, fpm_ug_thr.mid_thr);
        bdmf_session_print(session, "higher_thr = %u = 0x%x\n", fpm_ug_thr.higher_thr, fpm_ug_thr.higher_thr);
        break;
    }
    case cli_qm_rnr_group_cfg:
    {
        qm_rnr_group_cfg rnr_group_cfg;
        ag_err = ag_drv_qm_rnr_group_cfg_get(parm[1].value.unumber, &rnr_group_cfg);
        bdmf_session_print(session, "start_queue = %u = 0x%x\n", rnr_group_cfg.start_queue, rnr_group_cfg.start_queue);
        bdmf_session_print(session, "end_queue = %u = 0x%x\n", rnr_group_cfg.end_queue, rnr_group_cfg.end_queue);
        bdmf_session_print(session, "pd_fifo_base = %u = 0x%x\n", rnr_group_cfg.pd_fifo_base, rnr_group_cfg.pd_fifo_base);
        bdmf_session_print(session, "pd_fifo_size = %u = 0x%x\n", rnr_group_cfg.pd_fifo_size, rnr_group_cfg.pd_fifo_size);
        bdmf_session_print(session, "upd_fifo_base = %u = 0x%x\n", rnr_group_cfg.upd_fifo_base, rnr_group_cfg.upd_fifo_base);
        bdmf_session_print(session, "upd_fifo_size = %u = 0x%x\n", rnr_group_cfg.upd_fifo_size, rnr_group_cfg.upd_fifo_size);
        bdmf_session_print(session, "rnr_bb_id = %u = 0x%x\n", rnr_group_cfg.rnr_bb_id, rnr_group_cfg.rnr_bb_id);
        bdmf_session_print(session, "rnr_task = %u = 0x%x\n", rnr_group_cfg.rnr_task, rnr_group_cfg.rnr_task);
        bdmf_session_print(session, "rnr_enable = %u = 0x%x\n", rnr_group_cfg.rnr_enable, rnr_group_cfg.rnr_enable);
        break;
    }
    case cli_qm_cpu_pd_indirect_wr_data:
    {
        uint32_t data0;
        uint32_t data1;
        uint32_t data2;
        uint32_t data3;
        ag_err = ag_drv_qm_cpu_pd_indirect_wr_data_get(parm[1].value.unumber, &data0, &data1, &data2, &data3);
        bdmf_session_print(session, "data0 = %u = 0x%x\n", data0, data0);
        bdmf_session_print(session, "data1 = %u = 0x%x\n", data1, data1);
        bdmf_session_print(session, "data2 = %u = 0x%x\n", data2, data2);
        bdmf_session_print(session, "data3 = %u = 0x%x\n", data3, data3);
        break;
    }
    case cli_qm_cpu_pd_indirect_rd_data:
    {
        uint32_t data0;
        uint32_t data1;
        uint32_t data2;
        uint32_t data3;
        ag_err = ag_drv_qm_cpu_pd_indirect_rd_data_get(parm[1].value.unumber, &data0, &data1, &data2, &data3);
        bdmf_session_print(session, "data0 = %u = 0x%x\n", data0, data0);
        bdmf_session_print(session, "data1 = %u = 0x%x\n", data1, data1);
        bdmf_session_print(session, "data2 = %u = 0x%x\n", data2, data2);
        bdmf_session_print(session, "data3 = %u = 0x%x\n", data3, data3);
        break;
    }
    case cli_qm_aggr_context:
    {
        uint32_t context_valid;
        ag_err = ag_drv_qm_aggr_context_get(parm[1].value.unumber, &context_valid);
        bdmf_session_print(session, "context_valid = %u = 0x%x\n", context_valid, context_valid);
        break;
    }
    case cli_qm_wred_profile_cfg:
    {
        qm_wred_profile_cfg wred_profile_cfg;
        ag_err = ag_drv_qm_wred_profile_cfg_get(parm[1].value.unumber, &wred_profile_cfg);
        bdmf_session_print(session, "min_thr0 = %u = 0x%x\n", wred_profile_cfg.min_thr0, wred_profile_cfg.min_thr0);
        bdmf_session_print(session, "flw_ctrl_en0 = %u = 0x%x\n", wred_profile_cfg.flw_ctrl_en0, wred_profile_cfg.flw_ctrl_en0);
        bdmf_session_print(session, "min_thr1 = %u = 0x%x\n", wred_profile_cfg.min_thr1, wred_profile_cfg.min_thr1);
        bdmf_session_print(session, "flw_ctrl_en1 = %u = 0x%x\n", wred_profile_cfg.flw_ctrl_en1, wred_profile_cfg.flw_ctrl_en1);
        bdmf_session_print(session, "max_thr0 = %u = 0x%x\n", wred_profile_cfg.max_thr0, wred_profile_cfg.max_thr0);
        bdmf_session_print(session, "max_thr1 = %u = 0x%x\n", wred_profile_cfg.max_thr1, wred_profile_cfg.max_thr1);
        bdmf_session_print(session, "slope_mantissa0 = %u = 0x%x\n", wred_profile_cfg.slope_mantissa0, wred_profile_cfg.slope_mantissa0);
        bdmf_session_print(session, "slope_exp0 = %u = 0x%x\n", wred_profile_cfg.slope_exp0, wred_profile_cfg.slope_exp0);
        bdmf_session_print(session, "slope_mantissa1 = %u = 0x%x\n", wred_profile_cfg.slope_mantissa1, wred_profile_cfg.slope_mantissa1);
        bdmf_session_print(session, "slope_exp1 = %u = 0x%x\n", wred_profile_cfg.slope_exp1, wred_profile_cfg.slope_exp1);
        break;
    }
    case cli_qm_enable_ctrl:
    {
        qm_enable_ctrl enable_ctrl;
        ag_err = ag_drv_qm_enable_ctrl_get(&enable_ctrl);
        bdmf_session_print(session, "fpm_prefetch_enable = %u = 0x%x\n", enable_ctrl.fpm_prefetch_enable, enable_ctrl.fpm_prefetch_enable);
        bdmf_session_print(session, "reorder_credit_enable = %u = 0x%x\n", enable_ctrl.reorder_credit_enable, enable_ctrl.reorder_credit_enable);
        bdmf_session_print(session, "dqm_pop_enable = %u = 0x%x\n", enable_ctrl.dqm_pop_enable, enable_ctrl.dqm_pop_enable);
        bdmf_session_print(session, "rmt_fixed_arb_enable = %u = 0x%x\n", enable_ctrl.rmt_fixed_arb_enable, enable_ctrl.rmt_fixed_arb_enable);
        bdmf_session_print(session, "dqm_push_fixed_arb_enable = %u = 0x%x\n", enable_ctrl.dqm_push_fixed_arb_enable, enable_ctrl.dqm_push_fixed_arb_enable);
        bdmf_session_print(session, "aqm_clk_counter_enable = %u = 0x%x\n", enable_ctrl.aqm_clk_counter_enable, enable_ctrl.aqm_clk_counter_enable);
        bdmf_session_print(session, "aqm_timestamp_counter_enable = %u = 0x%x\n", enable_ctrl.aqm_timestamp_counter_enable, enable_ctrl.aqm_timestamp_counter_enable);
        bdmf_session_print(session, "aqm_timestamp_write_to_pd_enable = %u = 0x%x\n", enable_ctrl.aqm_timestamp_write_to_pd_enable, enable_ctrl.aqm_timestamp_write_to_pd_enable);
        break;
    }
    case cli_qm_reset_ctrl:
    {
        qm_reset_ctrl reset_ctrl;
        ag_err = ag_drv_qm_reset_ctrl_get(&reset_ctrl);
        bdmf_session_print(session, "fpm_prefetch0_sw_rst = %u = 0x%x\n", reset_ctrl.fpm_prefetch0_sw_rst, reset_ctrl.fpm_prefetch0_sw_rst);
        bdmf_session_print(session, "fpm_prefetch1_sw_rst = %u = 0x%x\n", reset_ctrl.fpm_prefetch1_sw_rst, reset_ctrl.fpm_prefetch1_sw_rst);
        bdmf_session_print(session, "fpm_prefetch2_sw_rst = %u = 0x%x\n", reset_ctrl.fpm_prefetch2_sw_rst, reset_ctrl.fpm_prefetch2_sw_rst);
        bdmf_session_print(session, "fpm_prefetch3_sw_rst = %u = 0x%x\n", reset_ctrl.fpm_prefetch3_sw_rst, reset_ctrl.fpm_prefetch3_sw_rst);
        bdmf_session_print(session, "normal_rmt_sw_rst = %u = 0x%x\n", reset_ctrl.normal_rmt_sw_rst, reset_ctrl.normal_rmt_sw_rst);
        bdmf_session_print(session, "non_delayed_rmt_sw_rst = %u = 0x%x\n", reset_ctrl.non_delayed_rmt_sw_rst, reset_ctrl.non_delayed_rmt_sw_rst);
        bdmf_session_print(session, "pre_cm_fifo_sw_rst = %u = 0x%x\n", reset_ctrl.pre_cm_fifo_sw_rst, reset_ctrl.pre_cm_fifo_sw_rst);
        bdmf_session_print(session, "cm_rd_pd_fifo_sw_rst = %u = 0x%x\n", reset_ctrl.cm_rd_pd_fifo_sw_rst, reset_ctrl.cm_rd_pd_fifo_sw_rst);
        bdmf_session_print(session, "cm_wr_pd_fifo_sw_rst = %u = 0x%x\n", reset_ctrl.cm_wr_pd_fifo_sw_rst, reset_ctrl.cm_wr_pd_fifo_sw_rst);
        bdmf_session_print(session, "bb0_output_fifo_sw_rst = %u = 0x%x\n", reset_ctrl.bb0_output_fifo_sw_rst, reset_ctrl.bb0_output_fifo_sw_rst);
        bdmf_session_print(session, "bb1_output_fifo_sw_rst = %u = 0x%x\n", reset_ctrl.bb1_output_fifo_sw_rst, reset_ctrl.bb1_output_fifo_sw_rst);
        bdmf_session_print(session, "bb1_input_fifo_sw_rst = %u = 0x%x\n", reset_ctrl.bb1_input_fifo_sw_rst, reset_ctrl.bb1_input_fifo_sw_rst);
        bdmf_session_print(session, "tm_fifo_ptr_sw_rst = %u = 0x%x\n", reset_ctrl.tm_fifo_ptr_sw_rst, reset_ctrl.tm_fifo_ptr_sw_rst);
        bdmf_session_print(session, "non_delayed_out_fifo_sw_rst = %u = 0x%x\n", reset_ctrl.non_delayed_out_fifo_sw_rst, reset_ctrl.non_delayed_out_fifo_sw_rst);
        bdmf_session_print(session, "bb0_egr_msg_out_fifo_sw_rst = %u = 0x%x\n", reset_ctrl.bb0_egr_msg_out_fifo_sw_rst, reset_ctrl.bb0_egr_msg_out_fifo_sw_rst);
        break;
    }
    case cli_qm_drop_counters_ctrl:
    {
        qm_drop_counters_ctrl drop_counters_ctrl;
        ag_err = ag_drv_qm_drop_counters_ctrl_get(&drop_counters_ctrl);
        bdmf_session_print(session, "read_clear_pkts = %u = 0x%x\n", drop_counters_ctrl.read_clear_pkts, drop_counters_ctrl.read_clear_pkts);
        bdmf_session_print(session, "read_clear_bytes = %u = 0x%x\n", drop_counters_ctrl.read_clear_bytes, drop_counters_ctrl.read_clear_bytes);
        bdmf_session_print(session, "disable_wrap_around_pkts = %u = 0x%x\n", drop_counters_ctrl.disable_wrap_around_pkts, drop_counters_ctrl.disable_wrap_around_pkts);
        bdmf_session_print(session, "disable_wrap_around_bytes = %u = 0x%x\n", drop_counters_ctrl.disable_wrap_around_bytes, drop_counters_ctrl.disable_wrap_around_bytes);
        bdmf_session_print(session, "free_with_context_last_search = %u = 0x%x\n", drop_counters_ctrl.free_with_context_last_search, drop_counters_ctrl.free_with_context_last_search);
        bdmf_session_print(session, "wred_disable = %u = 0x%x\n", drop_counters_ctrl.wred_disable, drop_counters_ctrl.wred_disable);
        bdmf_session_print(session, "ddr_pd_congestion_disable = %u = 0x%x\n", drop_counters_ctrl.ddr_pd_congestion_disable, drop_counters_ctrl.ddr_pd_congestion_disable);
        bdmf_session_print(session, "ddr_byte_congestion_disable = %u = 0x%x\n", drop_counters_ctrl.ddr_byte_congestion_disable, drop_counters_ctrl.ddr_byte_congestion_disable);
        bdmf_session_print(session, "ddr_occupancy_disable = %u = 0x%x\n", drop_counters_ctrl.ddr_occupancy_disable, drop_counters_ctrl.ddr_occupancy_disable);
        bdmf_session_print(session, "ddr_fpm_congestion_disable = %u = 0x%x\n", drop_counters_ctrl.ddr_fpm_congestion_disable, drop_counters_ctrl.ddr_fpm_congestion_disable);
        bdmf_session_print(session, "fpm_ug_disable = %u = 0x%x\n", drop_counters_ctrl.fpm_ug_disable, drop_counters_ctrl.fpm_ug_disable);
        bdmf_session_print(session, "queue_occupancy_ddr_copy_decision_disable = %u = 0x%x\n", drop_counters_ctrl.queue_occupancy_ddr_copy_decision_disable, drop_counters_ctrl.queue_occupancy_ddr_copy_decision_disable);
        bdmf_session_print(session, "psram_occupancy_ddr_copy_decision_disable = %u = 0x%x\n", drop_counters_ctrl.psram_occupancy_ddr_copy_decision_disable, drop_counters_ctrl.psram_occupancy_ddr_copy_decision_disable);
        bdmf_session_print(session, "dont_send_mc_bit_to_bbh = %u = 0x%x\n", drop_counters_ctrl.dont_send_mc_bit_to_bbh, drop_counters_ctrl.dont_send_mc_bit_to_bbh);
        bdmf_session_print(session, "close_aggregation_on_timeout_disable = %u = 0x%x\n", drop_counters_ctrl.close_aggregation_on_timeout_disable, drop_counters_ctrl.close_aggregation_on_timeout_disable);
        bdmf_session_print(session, "fpm_congestion_buf_release_mechanism_disable = %u = 0x%x\n", drop_counters_ctrl.fpm_congestion_buf_release_mechanism_disable, drop_counters_ctrl.fpm_congestion_buf_release_mechanism_disable);
        bdmf_session_print(session, "fpm_buffer_global_res_enable = %u = 0x%x\n", drop_counters_ctrl.fpm_buffer_global_res_enable, drop_counters_ctrl.fpm_buffer_global_res_enable);
        bdmf_session_print(session, "qm_preserve_pd_with_fpm = %u = 0x%x\n", drop_counters_ctrl.qm_preserve_pd_with_fpm, drop_counters_ctrl.qm_preserve_pd_with_fpm);
        bdmf_session_print(session, "qm_residue_per_queue = %u = 0x%x\n", drop_counters_ctrl.qm_residue_per_queue, drop_counters_ctrl.qm_residue_per_queue);
        bdmf_session_print(session, "ghost_rpt_update_after_close_agg_en = %u = 0x%x\n", drop_counters_ctrl.ghost_rpt_update_after_close_agg_en, drop_counters_ctrl.ghost_rpt_update_after_close_agg_en);
        bdmf_session_print(session, "fpm_ug_flow_ctrl_disable = %u = 0x%x\n", drop_counters_ctrl.fpm_ug_flow_ctrl_disable, drop_counters_ctrl.fpm_ug_flow_ctrl_disable);
        bdmf_session_print(session, "ddr_write_multi_slave_en = %u = 0x%x\n", drop_counters_ctrl.ddr_write_multi_slave_en, drop_counters_ctrl.ddr_write_multi_slave_en);
        bdmf_session_print(session, "ddr_pd_congestion_agg_priority = %u = 0x%x\n", drop_counters_ctrl.ddr_pd_congestion_agg_priority, drop_counters_ctrl.ddr_pd_congestion_agg_priority);
        bdmf_session_print(session, "psram_occupancy_drop_disable = %u = 0x%x\n", drop_counters_ctrl.psram_occupancy_drop_disable, drop_counters_ctrl.psram_occupancy_drop_disable);
        bdmf_session_print(session, "qm_ddr_write_alignment = %u = 0x%x\n", drop_counters_ctrl.qm_ddr_write_alignment, drop_counters_ctrl.qm_ddr_write_alignment);
        bdmf_session_print(session, "exclusive_dont_drop = %u = 0x%x\n", drop_counters_ctrl.exclusive_dont_drop, drop_counters_ctrl.exclusive_dont_drop);
        bdmf_session_print(session, "dqmol_jira_973_fix_enable = %u = 0x%x\n", drop_counters_ctrl.dqmol_jira_973_fix_enable, drop_counters_ctrl.dqmol_jira_973_fix_enable);
        bdmf_session_print(session, "gpon_dbr_ceil = %u = 0x%x\n", drop_counters_ctrl.gpon_dbr_ceil, drop_counters_ctrl.gpon_dbr_ceil);
        bdmf_session_print(session, "drop_cnt_wred_drops = %u = 0x%x\n", drop_counters_ctrl.drop_cnt_wred_drops, drop_counters_ctrl.drop_cnt_wred_drops);
        bdmf_session_print(session, "same_sec_lvl_bit_agg_en = %u = 0x%x\n", drop_counters_ctrl.same_sec_lvl_bit_agg_en, drop_counters_ctrl.same_sec_lvl_bit_agg_en);
        bdmf_session_print(session, "glbl_egr_drop_cnt_read_clear_enable = %u = 0x%x\n", drop_counters_ctrl.glbl_egr_drop_cnt_read_clear_enable, drop_counters_ctrl.glbl_egr_drop_cnt_read_clear_enable);
        bdmf_session_print(session, "glbl_egr_aqm_drop_cnt_read_clear_enable = %u = 0x%x\n", drop_counters_ctrl.glbl_egr_aqm_drop_cnt_read_clear_enable, drop_counters_ctrl.glbl_egr_aqm_drop_cnt_read_clear_enable);
        break;
    }
    case cli_qm_fpm_ctrl:
    {
        qm_fpm_ctrl fpm_ctrl;
        ag_err = ag_drv_qm_fpm_ctrl_get(&fpm_ctrl);
        bdmf_session_print(session, "fpm_pool_bp_enable = %u = 0x%x\n", fpm_ctrl.fpm_pool_bp_enable, fpm_ctrl.fpm_pool_bp_enable);
        bdmf_session_print(session, "fpm_congestion_bp_enable = %u = 0x%x\n", fpm_ctrl.fpm_congestion_bp_enable, fpm_ctrl.fpm_congestion_bp_enable);
        bdmf_session_print(session, "fpm_force_bp_lvl = %u = 0x%x\n", fpm_ctrl.fpm_force_bp_lvl, fpm_ctrl.fpm_force_bp_lvl);
        bdmf_session_print(session, "fpm_prefetch_granularity = %u = 0x%x\n", fpm_ctrl.fpm_prefetch_granularity, fpm_ctrl.fpm_prefetch_granularity);
        bdmf_session_print(session, "fpm_prefetch_min_pool_size = %u = 0x%x\n", fpm_ctrl.fpm_prefetch_min_pool_size, fpm_ctrl.fpm_prefetch_min_pool_size);
        bdmf_session_print(session, "fpm_prefetch_pending_req_limit = %u = 0x%x\n", fpm_ctrl.fpm_prefetch_pending_req_limit, fpm_ctrl.fpm_prefetch_pending_req_limit);
        bdmf_session_print(session, "fpm_override_bb_id_en = %u = 0x%x\n", fpm_ctrl.fpm_override_bb_id_en, fpm_ctrl.fpm_override_bb_id_en);
        bdmf_session_print(session, "fpm_override_bb_id_value = %u = 0x%x\n", fpm_ctrl.fpm_override_bb_id_value, fpm_ctrl.fpm_override_bb_id_value);
        break;
    }
    case cli_qm_qm_pd_cong_ctrl:
    {
        uint32_t total_pd_thr;
        ag_err = ag_drv_qm_qm_pd_cong_ctrl_get(&total_pd_thr);
        bdmf_session_print(session, "total_pd_thr = %u = 0x%x\n", total_pd_thr, total_pd_thr);
        break;
    }
    case cli_qm_global_cfg_abs_drop_queue:
    {
        uint16_t abs_drop_queue;
        bdmf_boolean abs_drop_queue_en;
        ag_err = ag_drv_qm_global_cfg_abs_drop_queue_get(&abs_drop_queue, &abs_drop_queue_en);
        bdmf_session_print(session, "abs_drop_queue = %u = 0x%x\n", abs_drop_queue, abs_drop_queue);
        bdmf_session_print(session, "abs_drop_queue_en = %u = 0x%x\n", abs_drop_queue_en, abs_drop_queue_en);
        break;
    }
    case cli_qm_global_cfg_aggregation_ctrl:
    {
        qm_global_cfg_aggregation_ctrl global_cfg_aggregation_ctrl;
        ag_err = ag_drv_qm_global_cfg_aggregation_ctrl_get(&global_cfg_aggregation_ctrl);
        bdmf_session_print(session, "max_agg_bytes = %u = 0x%x\n", global_cfg_aggregation_ctrl.max_agg_bytes, global_cfg_aggregation_ctrl.max_agg_bytes);
        bdmf_session_print(session, "max_agg_pkts = %u = 0x%x\n", global_cfg_aggregation_ctrl.max_agg_pkts, global_cfg_aggregation_ctrl.max_agg_pkts);
        bdmf_session_print(session, "agg_ovr_512b_en = %u = 0x%x\n", global_cfg_aggregation_ctrl.agg_ovr_512b_en, global_cfg_aggregation_ctrl.agg_ovr_512b_en);
        bdmf_session_print(session, "max_agg_pkt_size = %u = 0x%x\n", global_cfg_aggregation_ctrl.max_agg_pkt_size, global_cfg_aggregation_ctrl.max_agg_pkt_size);
        bdmf_session_print(session, "min_agg_pkt_size = %u = 0x%x\n", global_cfg_aggregation_ctrl.min_agg_pkt_size, global_cfg_aggregation_ctrl.min_agg_pkt_size);
        break;
    }
    case cli_qm_global_cfg_aggregation_ctrl2:
    {
        bdmf_boolean agg_pool_sel_en;
        uint8_t agg_pool_sel;
        ag_err = ag_drv_qm_global_cfg_aggregation_ctrl2_get(&agg_pool_sel_en, &agg_pool_sel);
        bdmf_session_print(session, "agg_pool_sel_en = %u = 0x%x\n", agg_pool_sel_en, agg_pool_sel_en);
        bdmf_session_print(session, "agg_pool_sel = %u = 0x%x\n", agg_pool_sel, agg_pool_sel);
        break;
    }
    case cli_qm_fpm_base_addr:
    {
        uint32_t fpm_base_addr;
        ag_err = ag_drv_qm_fpm_base_addr_get(&fpm_base_addr);
        bdmf_session_print(session, "fpm_base_addr = %u = 0x%x\n", fpm_base_addr, fpm_base_addr);
        break;
    }
    case cli_qm_global_cfg_fpm_coherent_base_addr:
    {
        uint32_t fpm_base_addr;
        ag_err = ag_drv_qm_global_cfg_fpm_coherent_base_addr_get(&fpm_base_addr);
        bdmf_session_print(session, "fpm_base_addr = %u = 0x%x\n", fpm_base_addr, fpm_base_addr);
        break;
    }
    case cli_qm_ddr_sop_offset:
    {
        uint16_t ddr_sop_offset0;
        uint16_t ddr_sop_offset1;
        ag_err = ag_drv_qm_ddr_sop_offset_get(&ddr_sop_offset0, &ddr_sop_offset1);
        bdmf_session_print(session, "ddr_sop_offset0 = %u = 0x%x\n", ddr_sop_offset0, ddr_sop_offset0);
        bdmf_session_print(session, "ddr_sop_offset1 = %u = 0x%x\n", ddr_sop_offset1, ddr_sop_offset1);
        break;
    }
    case cli_qm_epon_overhead_ctrl:
    {
        qm_epon_overhead_ctrl epon_overhead_ctrl;
        ag_err = ag_drv_qm_epon_overhead_ctrl_get(&epon_overhead_ctrl);
        bdmf_session_print(session, "epon_line_rate = %u = 0x%x\n", epon_overhead_ctrl.epon_line_rate, epon_overhead_ctrl.epon_line_rate);
        bdmf_session_print(session, "epon_crc_add_disable = %u = 0x%x\n", epon_overhead_ctrl.epon_crc_add_disable, epon_overhead_ctrl.epon_crc_add_disable);
        bdmf_session_print(session, "mac_flow_overwrite_crc_en = %u = 0x%x\n", epon_overhead_ctrl.mac_flow_overwrite_crc_en, epon_overhead_ctrl.mac_flow_overwrite_crc_en);
        bdmf_session_print(session, "mac_flow_overwrite_crc = %u = 0x%x\n", epon_overhead_ctrl.mac_flow_overwrite_crc, epon_overhead_ctrl.mac_flow_overwrite_crc);
        bdmf_session_print(session, "fec_ipg_length = %u = 0x%x\n", epon_overhead_ctrl.fec_ipg_length, epon_overhead_ctrl.fec_ipg_length);
        break;
    }
    case cli_qm_global_cfg_bbhtx_fifo_addr:
    {
        uint8_t addr;
        uint8_t bbhtx_req_otf;
        ag_err = ag_drv_qm_global_cfg_bbhtx_fifo_addr_get(&addr, &bbhtx_req_otf);
        bdmf_session_print(session, "addr = %u = 0x%x\n", addr, addr);
        bdmf_session_print(session, "bbhtx_req_otf = %u = 0x%x\n", bbhtx_req_otf, bbhtx_req_otf);
        break;
    }
    case cli_qm_global_cfg_qm_egress_flush_queue:
    {
        uint16_t queue_num;
        bdmf_boolean flush_en;
        ag_err = ag_drv_qm_global_cfg_qm_egress_flush_queue_get(&queue_num, &flush_en);
        bdmf_session_print(session, "queue_num = %u = 0x%x\n", queue_num, queue_num);
        bdmf_session_print(session, "flush_en = %u = 0x%x\n", flush_en, flush_en);
        break;
    }
    case cli_qm_global_cfg_qm_aggregation_timer_ctrl:
    {
        uint8_t prescaler_granularity;
        uint8_t aggregation_timeout_value;
        bdmf_boolean pd_occupancy_en;
        uint8_t pd_occupancy_value;
        ag_err = ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_get(&prescaler_granularity, &aggregation_timeout_value, &pd_occupancy_en, &pd_occupancy_value);
        bdmf_session_print(session, "prescaler_granularity = %u = 0x%x\n", prescaler_granularity, prescaler_granularity);
        bdmf_session_print(session, "aggregation_timeout_value = %u = 0x%x\n", aggregation_timeout_value, aggregation_timeout_value);
        bdmf_session_print(session, "pd_occupancy_en = %u = 0x%x\n", pd_occupancy_en, pd_occupancy_en);
        bdmf_session_print(session, "pd_occupancy_value = %u = 0x%x\n", pd_occupancy_value, pd_occupancy_value);
        break;
    }
    case cli_qm_global_cfg_qm_fpm_ug_gbl_cnt:
    {
        uint32_t fpm_gbl_cnt;
        ag_err = ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_get(&fpm_gbl_cnt);
        bdmf_session_print(session, "fpm_gbl_cnt = %u = 0x%x\n", fpm_gbl_cnt, fpm_gbl_cnt);
        break;
    }
    case cli_qm_qm_ddr_spare_room:
    {
        uint16_t ddr_headroom;
        uint16_t ddr_tailroom;
        ag_err = ag_drv_qm_qm_ddr_spare_room_get(parm[1].value.unumber, &ddr_headroom, &ddr_tailroom);
        bdmf_session_print(session, "ddr_headroom = %u = 0x%x\n", ddr_headroom, ddr_headroom);
        bdmf_session_print(session, "ddr_tailroom = %u = 0x%x\n", ddr_tailroom, ddr_tailroom);
        break;
    }
    case cli_qm_global_cfg_dummy_spare_room_profile_id:
    {
        uint8_t dummy_profile_0;
        uint8_t dummy_profile_1;
        ag_err = ag_drv_qm_global_cfg_dummy_spare_room_profile_id_get(&dummy_profile_0, &dummy_profile_1);
        bdmf_session_print(session, "dummy_profile_0 = %u = 0x%x\n", dummy_profile_0, dummy_profile_0);
        bdmf_session_print(session, "dummy_profile_1 = %u = 0x%x\n", dummy_profile_1, dummy_profile_1);
        break;
    }
    case cli_qm_global_cfg_dqm_ubus_ctrl:
    {
        uint8_t tkn_reqout_h;
        uint8_t tkn_reqout_d;
        uint8_t offload_reqout_h;
        uint8_t offload_reqout_d;
        ag_err = ag_drv_qm_global_cfg_dqm_ubus_ctrl_get(&tkn_reqout_h, &tkn_reqout_d, &offload_reqout_h, &offload_reqout_d);
        bdmf_session_print(session, "tkn_reqout_h = %u = 0x%x\n", tkn_reqout_h, tkn_reqout_h);
        bdmf_session_print(session, "tkn_reqout_d = %u = 0x%x\n", tkn_reqout_d, tkn_reqout_d);
        bdmf_session_print(session, "offload_reqout_h = %u = 0x%x\n", offload_reqout_h, offload_reqout_h);
        bdmf_session_print(session, "offload_reqout_d = %u = 0x%x\n", offload_reqout_d, offload_reqout_d);
        break;
    }
    case cli_qm_global_cfg_mem_auto_init:
    {
        bdmf_boolean mem_init_en;
        uint8_t mem_sel_init;
        uint8_t mem_size_init;
        ag_err = ag_drv_qm_global_cfg_mem_auto_init_get(&mem_init_en, &mem_sel_init, &mem_size_init);
        bdmf_session_print(session, "mem_init_en = %u = 0x%x\n", mem_init_en, mem_init_en);
        bdmf_session_print(session, "mem_sel_init = %u = 0x%x\n", mem_sel_init, mem_sel_init);
        bdmf_session_print(session, "mem_size_init = %u = 0x%x\n", mem_size_init, mem_size_init);
        break;
    }
    case cli_qm_global_cfg_mem_auto_init_sts:
    {
        bdmf_boolean mem_init_done;
        ag_err = ag_drv_qm_global_cfg_mem_auto_init_sts_get(&mem_init_done);
        bdmf_session_print(session, "mem_init_done = %u = 0x%x\n", mem_init_done, mem_init_done);
        break;
    }
    case cli_qm_global_cfg_fpm_mpm_enhancement_pool_size_tokens:
    {
        uint8_t pool_0_num_of_tkns;
        uint8_t pool_1_num_of_tkns;
        uint8_t pool_2_num_of_tkns;
        uint8_t pool_3_num_of_tkns;
        ag_err = ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_size_tokens_get(&pool_0_num_of_tkns, &pool_1_num_of_tkns, &pool_2_num_of_tkns, &pool_3_num_of_tkns);
        bdmf_session_print(session, "pool_0_num_of_tkns = %u = 0x%x\n", pool_0_num_of_tkns, pool_0_num_of_tkns);
        bdmf_session_print(session, "pool_1_num_of_tkns = %u = 0x%x\n", pool_1_num_of_tkns, pool_1_num_of_tkns);
        bdmf_session_print(session, "pool_2_num_of_tkns = %u = 0x%x\n", pool_2_num_of_tkns, pool_2_num_of_tkns);
        bdmf_session_print(session, "pool_3_num_of_tkns = %u = 0x%x\n", pool_3_num_of_tkns, pool_3_num_of_tkns);
        break;
    }
    case cli_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte:
    {
        uint16_t pool_0_num_of_bytes;
        uint16_t pool_1_num_of_bytes;
        ag_err = ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte_get(&pool_0_num_of_bytes, &pool_1_num_of_bytes);
        bdmf_session_print(session, "pool_0_num_of_bytes = %u = 0x%x\n", pool_0_num_of_bytes, pool_0_num_of_bytes);
        bdmf_session_print(session, "pool_1_num_of_bytes = %u = 0x%x\n", pool_1_num_of_bytes, pool_1_num_of_bytes);
        break;
    }
    case cli_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte:
    {
        uint16_t pool_2_num_of_bytes;
        uint16_t pool_3_num_of_bytes;
        ag_err = ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte_get(&pool_2_num_of_bytes, &pool_3_num_of_bytes);
        bdmf_session_print(session, "pool_2_num_of_bytes = %u = 0x%x\n", pool_2_num_of_bytes, pool_2_num_of_bytes);
        bdmf_session_print(session, "pool_3_num_of_bytes = %u = 0x%x\n", pool_3_num_of_bytes, pool_3_num_of_bytes);
        break;
    }
    case cli_qm_global_cfg_mc_ctrl:
    {
        uint8_t mc_headers_pool_sel;
        ag_err = ag_drv_qm_global_cfg_mc_ctrl_get(&mc_headers_pool_sel);
        bdmf_session_print(session, "mc_headers_pool_sel = %u = 0x%x\n", mc_headers_pool_sel, mc_headers_pool_sel);
        break;
    }
    case cli_qm_global_cfg_aqm_clk_counter_cycle:
    {
        uint16_t value;
        ag_err = ag_drv_qm_global_cfg_aqm_clk_counter_cycle_get(&value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_qm_global_cfg_aqm_push_to_empty_thr:
    {
        uint8_t value;
        ag_err = ag_drv_qm_global_cfg_aqm_push_to_empty_thr_get(&value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_qm_global_cfg_qm_general_ctrl2:
    {
        qm_global_cfg_qm_general_ctrl2 global_cfg_qm_general_ctrl2;
        ag_err = ag_drv_qm_global_cfg_qm_general_ctrl2_get(&global_cfg_qm_general_ctrl2);
        bdmf_session_print(session, "egress_accumulated_cnt_pkts_read_clear_enable = %u = 0x%x\n", global_cfg_qm_general_ctrl2.egress_accumulated_cnt_pkts_read_clear_enable, global_cfg_qm_general_ctrl2.egress_accumulated_cnt_pkts_read_clear_enable);
        bdmf_session_print(session, "egress_accumulated_cnt_bytes_read_clear_enable = %u = 0x%x\n", global_cfg_qm_general_ctrl2.egress_accumulated_cnt_bytes_read_clear_enable, global_cfg_qm_general_ctrl2.egress_accumulated_cnt_bytes_read_clear_enable);
        bdmf_session_print(session, "agg_closure_suspend_on_bp = %u = 0x%x\n", global_cfg_qm_general_ctrl2.agg_closure_suspend_on_bp, global_cfg_qm_general_ctrl2.agg_closure_suspend_on_bp);
        bdmf_session_print(session, "bufmng_en_or_ug_cntr = %u = 0x%x\n", global_cfg_qm_general_ctrl2.bufmng_en_or_ug_cntr, global_cfg_qm_general_ctrl2.bufmng_en_or_ug_cntr);
        bdmf_session_print(session, "dqm_to_fpm_ubus_or_fpmini = %u = 0x%x\n", global_cfg_qm_general_ctrl2.dqm_to_fpm_ubus_or_fpmini, global_cfg_qm_general_ctrl2.dqm_to_fpm_ubus_or_fpmini);
        bdmf_session_print(session, "agg_closure_suspend_on_fpm_congestion_disable = %u = 0x%x\n", global_cfg_qm_general_ctrl2.agg_closure_suspend_on_fpm_congestion_disable, global_cfg_qm_general_ctrl2.agg_closure_suspend_on_fpm_congestion_disable);
        break;
    }
    case cli_qm_fpm_pool_thr:
    {
        qm_fpm_pool_thr fpm_pool_thr;
        ag_err = ag_drv_qm_fpm_pool_thr_get(parm[1].value.unumber, &fpm_pool_thr);
        bdmf_session_print(session, "lower_thr = %u = 0x%x\n", fpm_pool_thr.lower_thr, fpm_pool_thr.lower_thr);
        bdmf_session_print(session, "higher_thr = %u = 0x%x\n", fpm_pool_thr.higher_thr, fpm_pool_thr.higher_thr);
        break;
    }
    case cli_qm_fpm_ug_cnt:
    {
        uint32_t fpm_ug_cnt;
        ag_err = ag_drv_qm_fpm_ug_cnt_get(parm[1].value.unumber, &fpm_ug_cnt);
        bdmf_session_print(session, "fpm_ug_cnt = %u = 0x%x\n", fpm_ug_cnt, fpm_ug_cnt);
        break;
    }
    case cli_qm_intr_ctrl_isr:
    {
        qm_intr_ctrl_isr intr_ctrl_isr;
        ag_err = ag_drv_qm_intr_ctrl_isr_get(&intr_ctrl_isr);
        bdmf_session_print(session, "qm_dqm_pop_on_empty = %u = 0x%x\n", intr_ctrl_isr.qm_dqm_pop_on_empty, intr_ctrl_isr.qm_dqm_pop_on_empty);
        bdmf_session_print(session, "qm_dqm_push_on_full = %u = 0x%x\n", intr_ctrl_isr.qm_dqm_push_on_full, intr_ctrl_isr.qm_dqm_push_on_full);
        bdmf_session_print(session, "qm_cpu_pop_on_empty = %u = 0x%x\n", intr_ctrl_isr.qm_cpu_pop_on_empty, intr_ctrl_isr.qm_cpu_pop_on_empty);
        bdmf_session_print(session, "qm_cpu_push_on_full = %u = 0x%x\n", intr_ctrl_isr.qm_cpu_push_on_full, intr_ctrl_isr.qm_cpu_push_on_full);
        bdmf_session_print(session, "qm_normal_queue_pd_no_credit = %u = 0x%x\n", intr_ctrl_isr.qm_normal_queue_pd_no_credit, intr_ctrl_isr.qm_normal_queue_pd_no_credit);
        bdmf_session_print(session, "qm_non_delayed_queue_pd_no_credit = %u = 0x%x\n", intr_ctrl_isr.qm_non_delayed_queue_pd_no_credit, intr_ctrl_isr.qm_non_delayed_queue_pd_no_credit);
        bdmf_session_print(session, "qm_non_valid_queue = %u = 0x%x\n", intr_ctrl_isr.qm_non_valid_queue, intr_ctrl_isr.qm_non_valid_queue);
        bdmf_session_print(session, "qm_agg_coherent_inconsistency = %u = 0x%x\n", intr_ctrl_isr.qm_agg_coherent_inconsistency, intr_ctrl_isr.qm_agg_coherent_inconsistency);
        bdmf_session_print(session, "qm_force_copy_on_non_delayed = %u = 0x%x\n", intr_ctrl_isr.qm_force_copy_on_non_delayed, intr_ctrl_isr.qm_force_copy_on_non_delayed);
        bdmf_session_print(session, "qm_fpm_pool_size_nonexistent = %u = 0x%x\n", intr_ctrl_isr.qm_fpm_pool_size_nonexistent, intr_ctrl_isr.qm_fpm_pool_size_nonexistent);
        bdmf_session_print(session, "qm_target_mem_abs_contradiction = %u = 0x%x\n", intr_ctrl_isr.qm_target_mem_abs_contradiction, intr_ctrl_isr.qm_target_mem_abs_contradiction);
        bdmf_session_print(session, "qm_1588_drop = %u = 0x%x\n", intr_ctrl_isr.qm_1588_drop, intr_ctrl_isr.qm_1588_drop);
        bdmf_session_print(session, "qm_1588_multicast_contradiction = %u = 0x%x\n", intr_ctrl_isr.qm_1588_multicast_contradiction, intr_ctrl_isr.qm_1588_multicast_contradiction);
        bdmf_session_print(session, "qm_byte_drop_cnt_overrun = %u = 0x%x\n", intr_ctrl_isr.qm_byte_drop_cnt_overrun, intr_ctrl_isr.qm_byte_drop_cnt_overrun);
        bdmf_session_print(session, "qm_pkt_drop_cnt_overrun = %u = 0x%x\n", intr_ctrl_isr.qm_pkt_drop_cnt_overrun, intr_ctrl_isr.qm_pkt_drop_cnt_overrun);
        bdmf_session_print(session, "qm_total_byte_cnt_underrun = %u = 0x%x\n", intr_ctrl_isr.qm_total_byte_cnt_underrun, intr_ctrl_isr.qm_total_byte_cnt_underrun);
        bdmf_session_print(session, "qm_total_pkt_cnt_underrun = %u = 0x%x\n", intr_ctrl_isr.qm_total_pkt_cnt_underrun, intr_ctrl_isr.qm_total_pkt_cnt_underrun);
        bdmf_session_print(session, "qm_fpm_ug0_underrun = %u = 0x%x\n", intr_ctrl_isr.qm_fpm_ug0_underrun, intr_ctrl_isr.qm_fpm_ug0_underrun);
        bdmf_session_print(session, "qm_fpm_ug1_underrun = %u = 0x%x\n", intr_ctrl_isr.qm_fpm_ug1_underrun, intr_ctrl_isr.qm_fpm_ug1_underrun);
        bdmf_session_print(session, "qm_fpm_ug2_underrun = %u = 0x%x\n", intr_ctrl_isr.qm_fpm_ug2_underrun, intr_ctrl_isr.qm_fpm_ug2_underrun);
        bdmf_session_print(session, "qm_fpm_ug3_underrun = %u = 0x%x\n", intr_ctrl_isr.qm_fpm_ug3_underrun, intr_ctrl_isr.qm_fpm_ug3_underrun);
        bdmf_session_print(session, "qm_timer_wraparound = %u = 0x%x\n", intr_ctrl_isr.qm_timer_wraparound, intr_ctrl_isr.qm_timer_wraparound);
        bdmf_session_print(session, "qm_copy_plen_zero = %u = 0x%x\n", intr_ctrl_isr.qm_copy_plen_zero, intr_ctrl_isr.qm_copy_plen_zero);
        bdmf_session_print(session, "qm_ingress_bb_unexpected_msg = %u = 0x%x\n", intr_ctrl_isr.qm_ingress_bb_unexpected_msg, intr_ctrl_isr.qm_ingress_bb_unexpected_msg);
        bdmf_session_print(session, "qm_egress_bb_unexpected_msg = %u = 0x%x\n", intr_ctrl_isr.qm_egress_bb_unexpected_msg, intr_ctrl_isr.qm_egress_bb_unexpected_msg);
        bdmf_session_print(session, "dqm_reached_full = %u = 0x%x\n", intr_ctrl_isr.dqm_reached_full, intr_ctrl_isr.dqm_reached_full);
        bdmf_session_print(session, "qm_fpmini_intr = %u = 0x%x\n", intr_ctrl_isr.qm_fpmini_intr, intr_ctrl_isr.qm_fpmini_intr);
        break;
    }
    case cli_qm_intr_ctrl_ism:
    {
        uint32_t ism;
        ag_err = ag_drv_qm_intr_ctrl_ism_get(&ism);
        bdmf_session_print(session, "ism = %u = 0x%x\n", ism, ism);
        break;
    }
    case cli_qm_intr_ctrl_ier:
    {
        uint32_t iem;
        ag_err = ag_drv_qm_intr_ctrl_ier_get(&iem);
        bdmf_session_print(session, "iem = %u = 0x%x\n", iem, iem);
        break;
    }
    case cli_qm_intr_ctrl_itr:
    {
        uint32_t ist;
        ag_err = ag_drv_qm_intr_ctrl_itr_get(&ist);
        bdmf_session_print(session, "ist = %u = 0x%x\n", ist, ist);
        break;
    }
    case cli_qm_clk_gate_clk_gate_cntrl:
    {
        qm_clk_gate_clk_gate_cntrl clk_gate_clk_gate_cntrl;
        ag_err = ag_drv_qm_clk_gate_clk_gate_cntrl_get(&clk_gate_clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u = 0x%x\n", clk_gate_clk_gate_cntrl.bypass_clk_gate, clk_gate_clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u = 0x%x\n", clk_gate_clk_gate_cntrl.timer_val, clk_gate_clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u = 0x%x\n", clk_gate_clk_gate_cntrl.keep_alive_en, clk_gate_clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intrvl = %u = 0x%x\n", clk_gate_clk_gate_cntrl.keep_alive_intrvl, clk_gate_clk_gate_cntrl.keep_alive_intrvl);
        bdmf_session_print(session, "keep_alive_cyc = %u = 0x%x\n", clk_gate_clk_gate_cntrl.keep_alive_cyc, clk_gate_clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    case cli_qm_cpu_indr_port_cpu_pd_indirect_ctrl:
    {
        uint16_t queue_num;
        uint8_t cmd;
        bdmf_boolean done;
        bdmf_boolean error;
        ag_err = ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_get(parm[1].value.unumber, &queue_num, &cmd, &done, &error);
        bdmf_session_print(session, "queue_num = %u = 0x%x\n", queue_num, queue_num);
        bdmf_session_print(session, "cmd = %u = 0x%x\n", cmd, cmd);
        bdmf_session_print(session, "done = %u = 0x%x\n", done, done);
        bdmf_session_print(session, "error = %u = 0x%x\n", error, error);
        break;
    }
    case cli_qm_q_context:
    {
        qm_q_context q_context;
        ag_err = ag_drv_qm_q_context_get(parm[1].value.unumber, &q_context);
        bdmf_session_print(session, "wred_profile = %u = 0x%x\n", q_context.wred_profile, q_context.wred_profile);
        bdmf_session_print(session, "copy_dec_profile = %u = 0x%x\n", q_context.copy_dec_profile, q_context.copy_dec_profile);
        bdmf_session_print(session, "ddr_copy_disable = %u = 0x%x\n", q_context.ddr_copy_disable, q_context.ddr_copy_disable);
        bdmf_session_print(session, "aggregation_disable = %u = 0x%x\n", q_context.aggregation_disable, q_context.aggregation_disable);
        bdmf_session_print(session, "fpm_ug_or_bufmng = %u = 0x%x\n", q_context.fpm_ug_or_bufmng, q_context.fpm_ug_or_bufmng);
        bdmf_session_print(session, "exclusive_priority = %u = 0x%x\n", q_context.exclusive_priority, q_context.exclusive_priority);
        bdmf_session_print(session, "q_802_1ae = %u = 0x%x\n", q_context.q_802_1ae, q_context.q_802_1ae);
        bdmf_session_print(session, "sci = %u = 0x%x\n", q_context.sci, q_context.sci);
        bdmf_session_print(session, "fec_enable = %u = 0x%x\n", q_context.fec_enable, q_context.fec_enable);
        bdmf_session_print(session, "res_profile = %u = 0x%x\n", q_context.res_profile, q_context.res_profile);
        bdmf_session_print(session, "spare_room_0 = %u = 0x%x\n", q_context.spare_room_0, q_context.spare_room_0);
        bdmf_session_print(session, "spare_room_1 = %u = 0x%x\n", q_context.spare_room_1, q_context.spare_room_1);
        bdmf_session_print(session, "service_queue_profile = %u = 0x%x\n", q_context.service_queue_profile, q_context.service_queue_profile);
        bdmf_session_print(session, "timestamp_res_profile = %u = 0x%x\n", q_context.timestamp_res_profile, q_context.timestamp_res_profile);
        break;
    }
    case cli_qm_copy_decision_profile:
    {
        uint32_t queue_occupancy_thr;
        bdmf_boolean psram_thr;
        ag_err = ag_drv_qm_copy_decision_profile_get(parm[1].value.unumber, &queue_occupancy_thr, &psram_thr);
        bdmf_session_print(session, "queue_occupancy_thr = %u = 0x%x\n", queue_occupancy_thr, queue_occupancy_thr);
        bdmf_session_print(session, "psram_thr = %u = 0x%x\n", psram_thr, psram_thr);
        break;
    }
    case cli_qm_timestamp_res_profile:
    {
        uint8_t start;
        ag_err = ag_drv_qm_timestamp_res_profile_get(parm[1].value.unumber, &start);
        bdmf_session_print(session, "start = %u = 0x%x\n", start, start);
        break;
    }
    case cli_qm_global_egress_drop_counter:
    {
        uint32_t data;
        ag_err = ag_drv_qm_global_egress_drop_counter_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_global_egress_aqm_drop_counter:
    {
        uint32_t data;
        ag_err = ag_drv_qm_global_egress_aqm_drop_counter_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_total_valid_cnt:
    {
        uint32_t data;
        ag_err = ag_drv_qm_total_valid_cnt_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_dqm_valid_cnt:
    {
        uint32_t data;
        ag_err = ag_drv_qm_dqm_valid_cnt_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_drop_counter:
    {
        uint32_t data;
        ag_err = ag_drv_qm_drop_counter_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_accumulated_counter:
    {
        uint32_t data;
        ag_err = ag_drv_qm_accumulated_counter_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_epon_q_byte_cnt:
    {
        uint32_t data;
        ag_err = ag_drv_qm_epon_q_byte_cnt_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_epon_q_status:
    {
        uint32_t status_bit_vector;
        ag_err = ag_drv_qm_epon_q_status_get(parm[1].value.unumber, &status_bit_vector);
        bdmf_session_print(session, "status_bit_vector = %u = 0x%x\n", status_bit_vector, status_bit_vector);
        break;
    }
    case cli_qm_rd_data_pool0:
    {
        uint32_t data;
        ag_err = ag_drv_qm_rd_data_pool0_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_rd_data_pool1:
    {
        uint32_t data;
        ag_err = ag_drv_qm_rd_data_pool1_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_rd_data_pool2:
    {
        uint32_t data;
        ag_err = ag_drv_qm_rd_data_pool2_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_rd_data_pool3:
    {
        uint32_t data;
        ag_err = ag_drv_qm_rd_data_pool3_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_pop_3:
    {
        bdmf_boolean pop_pool0;
        bdmf_boolean pop_pool1;
        bdmf_boolean pop_pool2;
        bdmf_boolean pop_pool3;
        ag_err = ag_drv_qm_pop_3_get(&pop_pool0, &pop_pool1, &pop_pool2, &pop_pool3);
        bdmf_session_print(session, "pop_pool0 = %u = 0x%x\n", pop_pool0, pop_pool0);
        bdmf_session_print(session, "pop_pool1 = %u = 0x%x\n", pop_pool1, pop_pool1);
        bdmf_session_print(session, "pop_pool2 = %u = 0x%x\n", pop_pool2, pop_pool2);
        bdmf_session_print(session, "pop_pool3 = %u = 0x%x\n", pop_pool3, pop_pool3);
        break;
    }
    case cli_qm_pdfifo_ptr:
    {
        uint8_t wr_ptr;
        uint8_t rd_ptr;
        ag_err = ag_drv_qm_pdfifo_ptr_get(parm[1].value.unumber, &wr_ptr, &rd_ptr);
        bdmf_session_print(session, "wr_ptr = %u = 0x%x\n", wr_ptr, wr_ptr);
        bdmf_session_print(session, "rd_ptr = %u = 0x%x\n", rd_ptr, rd_ptr);
        break;
    }
    case cli_qm_update_fifo_ptr:
    {
        uint16_t wr_ptr;
        uint8_t rd_ptr;
        ag_err = ag_drv_qm_update_fifo_ptr_get(parm[1].value.unumber, &wr_ptr, &rd_ptr);
        bdmf_session_print(session, "wr_ptr = %u = 0x%x\n", wr_ptr, wr_ptr);
        bdmf_session_print(session, "rd_ptr = %u = 0x%x\n", rd_ptr, rd_ptr);
        break;
    }
    case cli_qm_pop_2:
    {
        bdmf_boolean pop;
        ag_err = ag_drv_qm_pop_2_get(&pop);
        bdmf_session_print(session, "pop = %u = 0x%x\n", pop, pop);
        break;
    }
    case cli_qm_pop_1:
    {
        bdmf_boolean pop;
        ag_err = ag_drv_qm_pop_1_get(&pop);
        bdmf_session_print(session, "pop = %u = 0x%x\n", pop, pop);
        break;
    }
    case cli_qm_bb0_egr_msg_out_fifo_data:
    {
        uint32_t data;
        ag_err = ag_drv_qm_bb0_egr_msg_out_fifo_data_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_fpm_buffer_reservation_data:
    {
        uint32_t data;
        ag_err = ag_drv_qm_fpm_buffer_reservation_data_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_port_cfg:
    {
        bdmf_boolean en_byte;
        bdmf_boolean en_ug;
        uint8_t bbh_rx_bb_id;
        uint8_t fw_port_id;
        ag_err = ag_drv_qm_port_cfg_get(parm[1].value.unumber, &en_byte, &en_ug, &bbh_rx_bb_id, &fw_port_id);
        bdmf_session_print(session, "en_byte = %u = 0x%x\n", en_byte, en_byte);
        bdmf_session_print(session, "en_ug = %u = 0x%x\n", en_ug, en_ug);
        bdmf_session_print(session, "bbh_rx_bb_id = %u = 0x%x\n", bbh_rx_bb_id, bbh_rx_bb_id);
        bdmf_session_print(session, "fw_port_id = %u = 0x%x\n", fw_port_id, fw_port_id);
        break;
    }
    case cli_qm_fc_ug_mask_ug_en:
    {
        uint32_t ug_en;
        ag_err = ag_drv_qm_fc_ug_mask_ug_en_get(&ug_en);
        bdmf_session_print(session, "ug_en = %u = 0x%x\n", ug_en, ug_en);
        break;
    }
    case cli_qm_fc_queue_mask:
    {
        uint32_t queue_vec;
        ag_err = ag_drv_qm_fc_queue_mask_get(parm[1].value.unumber, &queue_vec);
        bdmf_session_print(session, "queue_vec = %u = 0x%x\n", queue_vec, queue_vec);
        break;
    }
    case cli_qm_fc_queue_range1_start:
    {
        uint16_t start_queue;
        ag_err = ag_drv_qm_fc_queue_range1_start_get(&start_queue);
        bdmf_session_print(session, "start_queue = %u = 0x%x\n", start_queue, start_queue);
        break;
    }
    case cli_qm_fc_queue_range2_start:
    {
        uint16_t start_queue;
        ag_err = ag_drv_qm_fc_queue_range2_start_get(&start_queue);
        bdmf_session_print(session, "start_queue = %u = 0x%x\n", start_queue, start_queue);
        break;
    }
    case cli_qm_dbg:
    {
        uint32_t status;
        ag_err = ag_drv_qm_dbg_get(&status);
        bdmf_session_print(session, "status = %u = 0x%x\n", status, status);
        break;
    }
    case cli_qm_ug_occupancy_status:
    {
        uint32_t status;
        ag_err = ag_drv_qm_ug_occupancy_status_get(parm[1].value.unumber, &status);
        bdmf_session_print(session, "status = %u = 0x%x\n", status, status);
        break;
    }
    case cli_qm_queue_range1_occupancy_status:
    {
        uint32_t status;
        ag_err = ag_drv_qm_queue_range1_occupancy_status_get(parm[1].value.unumber, &status);
        bdmf_session_print(session, "status = %u = 0x%x\n", status, status);
        break;
    }
    case cli_qm_queue_range2_occupancy_status:
    {
        uint32_t status;
        ag_err = ag_drv_qm_queue_range2_occupancy_status_get(parm[1].value.unumber, &status);
        bdmf_session_print(session, "status = %u = 0x%x\n", status, status);
        break;
    }
    case cli_qm_debug_sel:
    {
        uint8_t select;
        bdmf_boolean enable;
        ag_err = ag_drv_qm_debug_sel_get(&select, &enable);
        bdmf_session_print(session, "select = %u = 0x%x\n", select, select);
        bdmf_session_print(session, "enable = %u = 0x%x\n", enable, enable);
        break;
    }
    case cli_qm_debug_bus_lsb:
    {
        uint32_t data;
        ag_err = ag_drv_qm_debug_bus_lsb_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_debug_bus_msb:
    {
        uint32_t data;
        ag_err = ag_drv_qm_debug_bus_msb_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_qm_spare_config:
    {
        uint32_t data;
        ag_err = ag_drv_qm_qm_spare_config_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_qm_good_lvl1_pkts_cnt:
    {
        uint32_t good_lvl1_pkts;
        ag_err = ag_drv_qm_good_lvl1_pkts_cnt_get(&good_lvl1_pkts);
        bdmf_session_print(session, "good_lvl1_pkts = %u = 0x%x\n", good_lvl1_pkts, good_lvl1_pkts);
        break;
    }
    case cli_qm_good_lvl1_bytes_cnt:
    {
        uint32_t good_lvl1_bytes;
        ag_err = ag_drv_qm_good_lvl1_bytes_cnt_get(&good_lvl1_bytes);
        bdmf_session_print(session, "good_lvl1_bytes = %u = 0x%x\n", good_lvl1_bytes, good_lvl1_bytes);
        break;
    }
    case cli_qm_good_lvl2_pkts_cnt:
    {
        uint32_t good_lvl2_pkts;
        ag_err = ag_drv_qm_good_lvl2_pkts_cnt_get(&good_lvl2_pkts);
        bdmf_session_print(session, "good_lvl2_pkts = %u = 0x%x\n", good_lvl2_pkts, good_lvl2_pkts);
        break;
    }
    case cli_qm_good_lvl2_bytes_cnt:
    {
        uint32_t good_lvl2_bytes;
        ag_err = ag_drv_qm_good_lvl2_bytes_cnt_get(&good_lvl2_bytes);
        bdmf_session_print(session, "good_lvl2_bytes = %u = 0x%x\n", good_lvl2_bytes, good_lvl2_bytes);
        break;
    }
    case cli_qm_copied_pkts_cnt:
    {
        uint32_t copied_pkts;
        ag_err = ag_drv_qm_copied_pkts_cnt_get(&copied_pkts);
        bdmf_session_print(session, "copied_pkts = %u = 0x%x\n", copied_pkts, copied_pkts);
        break;
    }
    case cli_qm_copied_bytes_cnt:
    {
        uint32_t copied_bytes;
        ag_err = ag_drv_qm_copied_bytes_cnt_get(&copied_bytes);
        bdmf_session_print(session, "copied_bytes = %u = 0x%x\n", copied_bytes, copied_bytes);
        break;
    }
    case cli_qm_agg_pkts_cnt:
    {
        uint32_t agg_pkts;
        ag_err = ag_drv_qm_agg_pkts_cnt_get(&agg_pkts);
        bdmf_session_print(session, "agg_pkts = %u = 0x%x\n", agg_pkts, agg_pkts);
        break;
    }
    case cli_qm_agg_bytes_cnt:
    {
        uint32_t agg_bytes;
        ag_err = ag_drv_qm_agg_bytes_cnt_get(&agg_bytes);
        bdmf_session_print(session, "agg_bytes = %u = 0x%x\n", agg_bytes, agg_bytes);
        break;
    }
    case cli_qm_agg_1_pkts_cnt:
    {
        uint32_t agg1_pkts;
        ag_err = ag_drv_qm_agg_1_pkts_cnt_get(&agg1_pkts);
        bdmf_session_print(session, "agg1_pkts = %u = 0x%x\n", agg1_pkts, agg1_pkts);
        break;
    }
    case cli_qm_agg_2_pkts_cnt:
    {
        uint32_t agg2_pkts;
        ag_err = ag_drv_qm_agg_2_pkts_cnt_get(&agg2_pkts);
        bdmf_session_print(session, "agg2_pkts = %u = 0x%x\n", agg2_pkts, agg2_pkts);
        break;
    }
    case cli_qm_agg_3_pkts_cnt:
    {
        uint32_t agg3_pkts;
        ag_err = ag_drv_qm_agg_3_pkts_cnt_get(&agg3_pkts);
        bdmf_session_print(session, "agg3_pkts = %u = 0x%x\n", agg3_pkts, agg3_pkts);
        break;
    }
    case cli_qm_agg_4_pkts_cnt:
    {
        uint32_t agg4_pkts;
        ag_err = ag_drv_qm_agg_4_pkts_cnt_get(&agg4_pkts);
        bdmf_session_print(session, "agg4_pkts = %u = 0x%x\n", agg4_pkts, agg4_pkts);
        break;
    }
    case cli_qm_wred_drop_cnt:
    {
        uint32_t wred_drop;
        ag_err = ag_drv_qm_wred_drop_cnt_get(&wred_drop);
        bdmf_session_print(session, "wred_drop = %u = 0x%x\n", wred_drop, wred_drop);
        break;
    }
    case cli_qm_fpm_congestion_drop_cnt:
    {
        uint32_t fpm_cong;
        ag_err = ag_drv_qm_fpm_congestion_drop_cnt_get(&fpm_cong);
        bdmf_session_print(session, "fpm_cong = %u = 0x%x\n", fpm_cong, fpm_cong);
        break;
    }
    case cli_qm_ddr_pd_congestion_drop_cnt:
    {
        uint32_t ddr_pd_cong_drop;
        ag_err = ag_drv_qm_ddr_pd_congestion_drop_cnt_get(&ddr_pd_cong_drop);
        bdmf_session_print(session, "ddr_pd_cong_drop = %u = 0x%x\n", ddr_pd_cong_drop, ddr_pd_cong_drop);
        break;
    }
    case cli_qm_ddr_byte_congestion_drop_cnt:
    {
        uint32_t ddr_cong_byte_drop;
        ag_err = ag_drv_qm_ddr_byte_congestion_drop_cnt_get(&ddr_cong_byte_drop);
        bdmf_session_print(session, "ddr_cong_byte_drop = %u = 0x%x\n", ddr_cong_byte_drop, ddr_cong_byte_drop);
        break;
    }
    case cli_qm_qm_pd_congestion_drop_cnt:
    {
        uint32_t counter;
        ag_err = ag_drv_qm_qm_pd_congestion_drop_cnt_get(&counter);
        bdmf_session_print(session, "counter = %u = 0x%x\n", counter, counter);
        break;
    }
    case cli_qm_qm_abs_requeue_cnt:
    {
        uint32_t abs_requeue;
        ag_err = ag_drv_qm_qm_abs_requeue_cnt_get(&abs_requeue);
        bdmf_session_print(session, "abs_requeue = %u = 0x%x\n", abs_requeue, abs_requeue);
        break;
    }
    case cli_qm_fpm_prefetch_fifo0_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_fpm_prefetch_fifo0_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_fpm_prefetch_fifo1_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_fpm_prefetch_fifo1_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_fpm_prefetch_fifo2_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_fpm_prefetch_fifo2_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_fpm_prefetch_fifo3_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_fpm_prefetch_fifo3_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_normal_rmt_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_normal_rmt_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_non_delayed_rmt_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_non_delayed_rmt_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_non_delayed_out_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_non_delayed_out_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_pre_cm_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_pre_cm_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_cm_rd_pd_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_cm_rd_pd_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_cm_wr_pd_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_cm_wr_pd_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_cm_common_input_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_cm_common_input_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_bb0_output_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_bb0_output_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_bb1_output_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_bb1_output_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_bb1_input_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_bb1_input_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_egress_data_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_egress_data_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_egress_rr_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_egress_rr_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_bb_route_ovr:
    {
        bdmf_boolean ovr_en;
        uint8_t dest_id;
        uint16_t route_addr;
        ag_err = ag_drv_qm_bb_route_ovr_get(parm[1].value.unumber, &ovr_en, &dest_id, &route_addr);
        bdmf_session_print(session, "ovr_en = %u = 0x%x\n", ovr_en, ovr_en);
        bdmf_session_print(session, "dest_id = %u = 0x%x\n", dest_id, dest_id);
        bdmf_session_print(session, "route_addr = %u = 0x%x\n", route_addr, route_addr);
        break;
    }
    case cli_qm_ingress_stat:
    {
        uint32_t ingress_stat;
        ag_err = ag_drv_qm_ingress_stat_get(&ingress_stat);
        bdmf_session_print(session, "ingress_stat = %u = 0x%x\n", ingress_stat, ingress_stat);
        break;
    }
    case cli_qm_egress_stat:
    {
        uint32_t egress_stat;
        ag_err = ag_drv_qm_egress_stat_get(&egress_stat);
        bdmf_session_print(session, "egress_stat = %u = 0x%x\n", egress_stat, egress_stat);
        break;
    }
    case cli_qm_cm_stat:
    {
        uint32_t cm_stat;
        ag_err = ag_drv_qm_cm_stat_get(&cm_stat);
        bdmf_session_print(session, "cm_stat = %u = 0x%x\n", cm_stat, cm_stat);
        break;
    }
    case cli_qm_fpm_prefetch_stat:
    {
        uint32_t fpm_prefetch_stat;
        ag_err = ag_drv_qm_fpm_prefetch_stat_get(&fpm_prefetch_stat);
        bdmf_session_print(session, "fpm_prefetch_stat = %u = 0x%x\n", fpm_prefetch_stat, fpm_prefetch_stat);
        break;
    }
    case cli_qm_qm_connect_ack_counter:
    {
        uint8_t connect_ack_counter;
        ag_err = ag_drv_qm_qm_connect_ack_counter_get(&connect_ack_counter);
        bdmf_session_print(session, "connect_ack_counter = %u = 0x%x\n", connect_ack_counter, connect_ack_counter);
        break;
    }
    case cli_qm_qm_ddr_wr_reply_counter:
    {
        uint8_t ddr_wr_reply_counter;
        ag_err = ag_drv_qm_qm_ddr_wr_reply_counter_get(&ddr_wr_reply_counter);
        bdmf_session_print(session, "ddr_wr_reply_counter = %u = 0x%x\n", ddr_wr_reply_counter, ddr_wr_reply_counter);
        break;
    }
    case cli_qm_qm_ddr_pipe_byte_counter:
    {
        uint32_t ddr_pipe;
        ag_err = ag_drv_qm_qm_ddr_pipe_byte_counter_get(&ddr_pipe);
        bdmf_session_print(session, "ddr_pipe = %u = 0x%x\n", ddr_pipe, ddr_pipe);
        break;
    }
    case cli_qm_qm_abs_requeue_valid_counter:
    {
        uint16_t requeue_valid;
        ag_err = ag_drv_qm_qm_abs_requeue_valid_counter_get(&requeue_valid);
        bdmf_session_print(session, "requeue_valid = %u = 0x%x\n", requeue_valid, requeue_valid);
        break;
    }
    case cli_qm_qm_illegal_pd_capture:
    {
        uint32_t pd;
        ag_err = ag_drv_qm_qm_illegal_pd_capture_get(parm[1].value.unumber, &pd);
        bdmf_session_print(session, "pd = %u = 0x%x\n", pd, pd);
        break;
    }
    case cli_qm_qm_ingress_processed_pd_capture:
    {
        uint32_t pd;
        ag_err = ag_drv_qm_qm_ingress_processed_pd_capture_get(parm[1].value.unumber, &pd);
        bdmf_session_print(session, "pd = %u = 0x%x\n", pd, pd);
        break;
    }
    case cli_qm_qm_cm_processed_pd_capture:
    {
        uint32_t pd;
        ag_err = ag_drv_qm_qm_cm_processed_pd_capture_get(parm[1].value.unumber, &pd);
        bdmf_session_print(session, "pd = %u = 0x%x\n", pd, pd);
        break;
    }
    case cli_qm_fpm_grp_drop_cnt:
    {
        uint32_t fpm_grp_drop;
        ag_err = ag_drv_qm_fpm_grp_drop_cnt_get(parm[1].value.unumber, &fpm_grp_drop);
        bdmf_session_print(session, "fpm_grp_drop = %u = 0x%x\n", fpm_grp_drop, fpm_grp_drop);
        break;
    }
    case cli_qm_fpm_pool_drop_cnt:
    {
        uint32_t fpm_drop;
        ag_err = ag_drv_qm_fpm_pool_drop_cnt_get(parm[1].value.unumber, &fpm_drop);
        bdmf_session_print(session, "fpm_drop = %u = 0x%x\n", fpm_drop, fpm_drop);
        break;
    }
    case cli_qm_fpm_buffer_res_drop_cnt:
    {
        uint32_t counter;
        ag_err = ag_drv_qm_fpm_buffer_res_drop_cnt_get(&counter);
        bdmf_session_print(session, "counter = %u = 0x%x\n", counter, counter);
        break;
    }
    case cli_qm_psram_egress_cong_drp_cnt:
    {
        uint32_t counter;
        ag_err = ag_drv_qm_psram_egress_cong_drp_cnt_get(&counter);
        bdmf_session_print(session, "counter = %u = 0x%x\n", counter, counter);
        break;
    }
    case cli_qm_backpressure:
    {
        uint8_t status;
        ag_err = ag_drv_qm_backpressure_get(&status);
        bdmf_session_print(session, "status = %u = 0x%x\n", status, status);
        break;
    }
    case cli_qm_aqm_timestamp_curr_counter:
    {
        uint32_t value;
        ag_err = ag_drv_qm_aqm_timestamp_curr_counter_get(&value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_qm_bb0_egr_msg_out_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_qm_bb0_egr_msg_out_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_qm_count_pkt_not_pd_mode_bits:
    {
        qm_count_pkt_not_pd_mode_bits count_pkt_not_pd_mode_bits;
        ag_err = ag_drv_qm_count_pkt_not_pd_mode_bits_get(&count_pkt_not_pd_mode_bits);
        bdmf_session_print(session, "total_egress_accum_cnt_pkt = %u = 0x%x\n", count_pkt_not_pd_mode_bits.total_egress_accum_cnt_pkt, count_pkt_not_pd_mode_bits.total_egress_accum_cnt_pkt);
        bdmf_session_print(session, "global_egress_drop_cnt_pkt = %u = 0x%x\n", count_pkt_not_pd_mode_bits.global_egress_drop_cnt_pkt, count_pkt_not_pd_mode_bits.global_egress_drop_cnt_pkt);
        bdmf_session_print(session, "drop_ing_egr_cnt_pkt = %u = 0x%x\n", count_pkt_not_pd_mode_bits.drop_ing_egr_cnt_pkt, count_pkt_not_pd_mode_bits.drop_ing_egr_cnt_pkt);
        bdmf_session_print(session, "fpm_grp_drop_cnt_pkt = %u = 0x%x\n", count_pkt_not_pd_mode_bits.fpm_grp_drop_cnt_pkt, count_pkt_not_pd_mode_bits.fpm_grp_drop_cnt_pkt);
        bdmf_session_print(session, "qm_pd_congestion_drop_cnt_pkt = %u = 0x%x\n", count_pkt_not_pd_mode_bits.qm_pd_congestion_drop_cnt_pkt, count_pkt_not_pd_mode_bits.qm_pd_congestion_drop_cnt_pkt);
        bdmf_session_print(session, "ddr_pd_congestion_drop_cnt_pkt = %u = 0x%x\n", count_pkt_not_pd_mode_bits.ddr_pd_congestion_drop_cnt_pkt, count_pkt_not_pd_mode_bits.ddr_pd_congestion_drop_cnt_pkt);
        bdmf_session_print(session, "wred_drop_cnt_pkt = %u = 0x%x\n", count_pkt_not_pd_mode_bits.wred_drop_cnt_pkt, count_pkt_not_pd_mode_bits.wred_drop_cnt_pkt);
        bdmf_session_print(session, "good_lvl2_pkts_cnt_pkt = %u = 0x%x\n", count_pkt_not_pd_mode_bits.good_lvl2_pkts_cnt_pkt, count_pkt_not_pd_mode_bits.good_lvl2_pkts_cnt_pkt);
        bdmf_session_print(session, "good_lvl1_pkts_cnt_pkt = %u = 0x%x\n", count_pkt_not_pd_mode_bits.good_lvl1_pkts_cnt_pkt, count_pkt_not_pd_mode_bits.good_lvl1_pkts_cnt_pkt);
        bdmf_session_print(session, "qm_total_valid_cnt_pkt = %u = 0x%x\n", count_pkt_not_pd_mode_bits.qm_total_valid_cnt_pkt, count_pkt_not_pd_mode_bits.qm_total_valid_cnt_pkt);
        bdmf_session_print(session, "dqm_valid_cnt_pkt = %u = 0x%x\n", count_pkt_not_pd_mode_bits.dqm_valid_cnt_pkt, count_pkt_not_pd_mode_bits.dqm_valid_cnt_pkt);
        break;
    }
    case cli_qm_cm_residue_data:
    {
        uint32_t data;
        ag_err = ag_drv_qm_cm_residue_data_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_qm_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        qm_ddr_cong_ctrl ddr_cong_ctrl = {.ddr_byte_congestion_drop_enable = gtmv(m, 1), .ddr_bytes_lower_thr = gtmv(m, 30), .ddr_bytes_mid_thr = gtmv(m, 30), .ddr_bytes_higher_thr = gtmv(m, 30), .ddr_pd_congestion_drop_enable = gtmv(m, 1), .ddr_pipe_lower_thr = gtmv(m, 8), .ddr_pipe_higher_thr = gtmv(m, 8)};
        bdmf_session_print(session, "ag_drv_qm_ddr_cong_ctrl_set( %u %u %u %u %u %u %u)\n",
            ddr_cong_ctrl.ddr_byte_congestion_drop_enable, ddr_cong_ctrl.ddr_bytes_lower_thr, ddr_cong_ctrl.ddr_bytes_mid_thr, ddr_cong_ctrl.ddr_bytes_higher_thr, 
            ddr_cong_ctrl.ddr_pd_congestion_drop_enable, ddr_cong_ctrl.ddr_pipe_lower_thr, ddr_cong_ctrl.ddr_pipe_higher_thr);
        ag_err = ag_drv_qm_ddr_cong_ctrl_set(&ddr_cong_ctrl);
        if (!ag_err)
            ag_err = ag_drv_qm_ddr_cong_ctrl_get(&ddr_cong_ctrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_ddr_cong_ctrl_get( %u %u %u %u %u %u %u)\n",
                ddr_cong_ctrl.ddr_byte_congestion_drop_enable, ddr_cong_ctrl.ddr_bytes_lower_thr, ddr_cong_ctrl.ddr_bytes_mid_thr, ddr_cong_ctrl.ddr_bytes_higher_thr, 
                ddr_cong_ctrl.ddr_pd_congestion_drop_enable, ddr_cong_ctrl.ddr_pipe_lower_thr, ddr_cong_ctrl.ddr_pipe_higher_thr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddr_cong_ctrl.ddr_byte_congestion_drop_enable != gtmv(m, 1) || ddr_cong_ctrl.ddr_bytes_lower_thr != gtmv(m, 30) || ddr_cong_ctrl.ddr_bytes_mid_thr != gtmv(m, 30) || ddr_cong_ctrl.ddr_bytes_higher_thr != gtmv(m, 30) || ddr_cong_ctrl.ddr_pd_congestion_drop_enable != gtmv(m, 1) || ddr_cong_ctrl.ddr_pipe_lower_thr != gtmv(m, 8) || ddr_cong_ctrl.ddr_pipe_higher_thr != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t q_idx = gtmv(m, 7);
        bdmf_boolean data = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_is_queue_not_empty_get(q_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_is_queue_not_empty_get( %u %u)\n", q_idx,
                data);
        }
    }

    {
        uint16_t q_idx = gtmv(m, 7);
        bdmf_boolean data = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_is_queue_pop_ready_get(q_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_is_queue_pop_ready_get( %u %u)\n", q_idx,
                data);
        }
    }

    {
        uint16_t q_idx = gtmv(m, 7);
        bdmf_boolean data = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_is_queue_full_get(q_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_is_queue_full_get( %u %u)\n", q_idx,
                data);
        }
    }

    {
        uint8_t ug_grp_idx = gtmv(m, 2);
        qm_fpm_ug_thr fpm_ug_thr = {.lower_thr = gtmv(m, 18), .mid_thr = gtmv(m, 18), .higher_thr = gtmv(m, 18)};
        bdmf_session_print(session, "ag_drv_qm_fpm_ug_thr_set( %u %u %u %u)\n", ug_grp_idx,
            fpm_ug_thr.lower_thr, fpm_ug_thr.mid_thr, fpm_ug_thr.higher_thr);
        ag_err = ag_drv_qm_fpm_ug_thr_set(ug_grp_idx, &fpm_ug_thr);
        if (!ag_err)
            ag_err = ag_drv_qm_fpm_ug_thr_get(ug_grp_idx, &fpm_ug_thr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_ug_thr_get( %u %u %u %u)\n", ug_grp_idx,
                fpm_ug_thr.lower_thr, fpm_ug_thr.mid_thr, fpm_ug_thr.higher_thr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fpm_ug_thr.lower_thr != gtmv(m, 18) || fpm_ug_thr.mid_thr != gtmv(m, 18) || fpm_ug_thr.higher_thr != gtmv(m, 18))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t rnr_idx = gtmv(m, 4);
        qm_rnr_group_cfg rnr_group_cfg = {.start_queue = gtmv(m, 9), .end_queue = gtmv(m, 9), .pd_fifo_base = gtmv(m, 11), .pd_fifo_size = gtmv(m, 2), .upd_fifo_base = gtmv(m, 11), .upd_fifo_size = gtmv(m, 3), .rnr_bb_id = gtmv(m, 6), .rnr_task = gtmv(m, 4), .rnr_enable = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_qm_rnr_group_cfg_set( %u %u %u %u %u %u %u %u %u %u)\n", rnr_idx,
            rnr_group_cfg.start_queue, rnr_group_cfg.end_queue, rnr_group_cfg.pd_fifo_base, rnr_group_cfg.pd_fifo_size, 
            rnr_group_cfg.upd_fifo_base, rnr_group_cfg.upd_fifo_size, rnr_group_cfg.rnr_bb_id, rnr_group_cfg.rnr_task, 
            rnr_group_cfg.rnr_enable);
        ag_err = ag_drv_qm_rnr_group_cfg_set(rnr_idx, &rnr_group_cfg);
        if (!ag_err)
            ag_err = ag_drv_qm_rnr_group_cfg_get(rnr_idx, &rnr_group_cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_rnr_group_cfg_get( %u %u %u %u %u %u %u %u %u %u)\n", rnr_idx,
                rnr_group_cfg.start_queue, rnr_group_cfg.end_queue, rnr_group_cfg.pd_fifo_base, rnr_group_cfg.pd_fifo_size, 
                rnr_group_cfg.upd_fifo_base, rnr_group_cfg.upd_fifo_size, rnr_group_cfg.rnr_bb_id, rnr_group_cfg.rnr_task, 
                rnr_group_cfg.rnr_enable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (rnr_group_cfg.start_queue != gtmv(m, 9) || rnr_group_cfg.end_queue != gtmv(m, 9) || rnr_group_cfg.pd_fifo_base != gtmv(m, 11) || rnr_group_cfg.pd_fifo_size != gtmv(m, 2) || rnr_group_cfg.upd_fifo_base != gtmv(m, 11) || rnr_group_cfg.upd_fifo_size != gtmv(m, 3) || rnr_group_cfg.rnr_bb_id != gtmv(m, 6) || rnr_group_cfg.rnr_task != gtmv(m, 4) || rnr_group_cfg.rnr_enable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t indirect_grp_idx = gtmv(m, 2);
        uint32_t data0 = gtmv(m, 32);
        uint32_t data1 = gtmv(m, 32);
        uint32_t data2 = gtmv(m, 32);
        uint32_t data3 = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_qm_cpu_pd_indirect_wr_data_set( %u %u %u %u %u)\n", indirect_grp_idx,
            data0, data1, data2, data3);
        ag_err = ag_drv_qm_cpu_pd_indirect_wr_data_set(indirect_grp_idx, data0, data1, data2, data3);
        if (!ag_err)
            ag_err = ag_drv_qm_cpu_pd_indirect_wr_data_get(indirect_grp_idx, &data0, &data1, &data2, &data3);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_cpu_pd_indirect_wr_data_get( %u %u %u %u %u)\n", indirect_grp_idx,
                data0, data1, data2, data3);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (data0 != gtmv(m, 32) || data1 != gtmv(m, 32) || data2 != gtmv(m, 32) || data3 != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t indirect_grp_idx = gtmv(m, 2);
        uint32_t data0 = gtmv(m, 32);
        uint32_t data1 = gtmv(m, 32);
        uint32_t data2 = gtmv(m, 32);
        uint32_t data3 = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_cpu_pd_indirect_rd_data_get(indirect_grp_idx, &data0, &data1, &data2, &data3);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_cpu_pd_indirect_rd_data_get( %u %u %u %u %u)\n", indirect_grp_idx,
                data0, data1, data2, data3);
        }
    }

    {
        uint16_t idx = gtmv(m, 2);
        uint32_t context_valid = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_aggr_context_get(idx, &context_valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_aggr_context_get( %u %u)\n", idx,
                context_valid);
        }
    }

    {
        uint8_t profile_idx = gtmv(m, 4);
        qm_wred_profile_cfg wred_profile_cfg = {.min_thr0 = gtmv(m, 24), .flw_ctrl_en0 = gtmv(m, 1), .min_thr1 = gtmv(m, 24), .flw_ctrl_en1 = gtmv(m, 1), .max_thr0 = gtmv(m, 24), .max_thr1 = gtmv(m, 24), .slope_mantissa0 = gtmv(m, 8), .slope_exp0 = gtmv(m, 5), .slope_mantissa1 = gtmv(m, 8), .slope_exp1 = gtmv(m, 5)};
        bdmf_session_print(session, "ag_drv_qm_wred_profile_cfg_set( %u %u %u %u %u %u %u %u %u %u %u)\n", profile_idx,
            wred_profile_cfg.min_thr0, wred_profile_cfg.flw_ctrl_en0, wred_profile_cfg.min_thr1, wred_profile_cfg.flw_ctrl_en1, 
            wred_profile_cfg.max_thr0, wred_profile_cfg.max_thr1, wred_profile_cfg.slope_mantissa0, wred_profile_cfg.slope_exp0, 
            wred_profile_cfg.slope_mantissa1, wred_profile_cfg.slope_exp1);
        ag_err = ag_drv_qm_wred_profile_cfg_set(profile_idx, &wred_profile_cfg);
        if (!ag_err)
            ag_err = ag_drv_qm_wred_profile_cfg_get(profile_idx, &wred_profile_cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_wred_profile_cfg_get( %u %u %u %u %u %u %u %u %u %u %u)\n", profile_idx,
                wred_profile_cfg.min_thr0, wred_profile_cfg.flw_ctrl_en0, wred_profile_cfg.min_thr1, wred_profile_cfg.flw_ctrl_en1, 
                wred_profile_cfg.max_thr0, wred_profile_cfg.max_thr1, wred_profile_cfg.slope_mantissa0, wred_profile_cfg.slope_exp0, 
                wred_profile_cfg.slope_mantissa1, wred_profile_cfg.slope_exp1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (wred_profile_cfg.min_thr0 != gtmv(m, 24) || wred_profile_cfg.flw_ctrl_en0 != gtmv(m, 1) || wred_profile_cfg.min_thr1 != gtmv(m, 24) || wred_profile_cfg.flw_ctrl_en1 != gtmv(m, 1) || wred_profile_cfg.max_thr0 != gtmv(m, 24) || wred_profile_cfg.max_thr1 != gtmv(m, 24) || wred_profile_cfg.slope_mantissa0 != gtmv(m, 8) || wred_profile_cfg.slope_exp0 != gtmv(m, 5) || wred_profile_cfg.slope_mantissa1 != gtmv(m, 8) || wred_profile_cfg.slope_exp1 != gtmv(m, 5))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        qm_enable_ctrl enable_ctrl = {.fpm_prefetch_enable = gtmv(m, 1), .reorder_credit_enable = gtmv(m, 1), .dqm_pop_enable = gtmv(m, 1), .rmt_fixed_arb_enable = gtmv(m, 1), .dqm_push_fixed_arb_enable = gtmv(m, 1), .aqm_clk_counter_enable = gtmv(m, 1), .aqm_timestamp_counter_enable = gtmv(m, 1), .aqm_timestamp_write_to_pd_enable = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_qm_enable_ctrl_set( %u %u %u %u %u %u %u %u)\n",
            enable_ctrl.fpm_prefetch_enable, enable_ctrl.reorder_credit_enable, enable_ctrl.dqm_pop_enable, enable_ctrl.rmt_fixed_arb_enable, 
            enable_ctrl.dqm_push_fixed_arb_enable, enable_ctrl.aqm_clk_counter_enable, enable_ctrl.aqm_timestamp_counter_enable, enable_ctrl.aqm_timestamp_write_to_pd_enable);
        ag_err = ag_drv_qm_enable_ctrl_set(&enable_ctrl);
        if (!ag_err)
            ag_err = ag_drv_qm_enable_ctrl_get(&enable_ctrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_enable_ctrl_get( %u %u %u %u %u %u %u %u)\n",
                enable_ctrl.fpm_prefetch_enable, enable_ctrl.reorder_credit_enable, enable_ctrl.dqm_pop_enable, enable_ctrl.rmt_fixed_arb_enable, 
                enable_ctrl.dqm_push_fixed_arb_enable, enable_ctrl.aqm_clk_counter_enable, enable_ctrl.aqm_timestamp_counter_enable, enable_ctrl.aqm_timestamp_write_to_pd_enable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (enable_ctrl.fpm_prefetch_enable != gtmv(m, 1) || enable_ctrl.reorder_credit_enable != gtmv(m, 1) || enable_ctrl.dqm_pop_enable != gtmv(m, 1) || enable_ctrl.rmt_fixed_arb_enable != gtmv(m, 1) || enable_ctrl.dqm_push_fixed_arb_enable != gtmv(m, 1) || enable_ctrl.aqm_clk_counter_enable != gtmv(m, 1) || enable_ctrl.aqm_timestamp_counter_enable != gtmv(m, 1) || enable_ctrl.aqm_timestamp_write_to_pd_enable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        qm_reset_ctrl reset_ctrl = {.fpm_prefetch0_sw_rst = gtmv(m, 1), .fpm_prefetch1_sw_rst = gtmv(m, 1), .fpm_prefetch2_sw_rst = gtmv(m, 1), .fpm_prefetch3_sw_rst = gtmv(m, 1), .normal_rmt_sw_rst = gtmv(m, 1), .non_delayed_rmt_sw_rst = gtmv(m, 1), .pre_cm_fifo_sw_rst = gtmv(m, 1), .cm_rd_pd_fifo_sw_rst = gtmv(m, 1), .cm_wr_pd_fifo_sw_rst = gtmv(m, 1), .bb0_output_fifo_sw_rst = gtmv(m, 1), .bb1_output_fifo_sw_rst = gtmv(m, 1), .bb1_input_fifo_sw_rst = gtmv(m, 1), .tm_fifo_ptr_sw_rst = gtmv(m, 1), .non_delayed_out_fifo_sw_rst = gtmv(m, 1), .bb0_egr_msg_out_fifo_sw_rst = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_qm_reset_ctrl_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            reset_ctrl.fpm_prefetch0_sw_rst, reset_ctrl.fpm_prefetch1_sw_rst, reset_ctrl.fpm_prefetch2_sw_rst, reset_ctrl.fpm_prefetch3_sw_rst, 
            reset_ctrl.normal_rmt_sw_rst, reset_ctrl.non_delayed_rmt_sw_rst, reset_ctrl.pre_cm_fifo_sw_rst, reset_ctrl.cm_rd_pd_fifo_sw_rst, 
            reset_ctrl.cm_wr_pd_fifo_sw_rst, reset_ctrl.bb0_output_fifo_sw_rst, reset_ctrl.bb1_output_fifo_sw_rst, reset_ctrl.bb1_input_fifo_sw_rst, 
            reset_ctrl.tm_fifo_ptr_sw_rst, reset_ctrl.non_delayed_out_fifo_sw_rst, reset_ctrl.bb0_egr_msg_out_fifo_sw_rst);
        ag_err = ag_drv_qm_reset_ctrl_set(&reset_ctrl);
        if (!ag_err)
            ag_err = ag_drv_qm_reset_ctrl_get(&reset_ctrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_reset_ctrl_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                reset_ctrl.fpm_prefetch0_sw_rst, reset_ctrl.fpm_prefetch1_sw_rst, reset_ctrl.fpm_prefetch2_sw_rst, reset_ctrl.fpm_prefetch3_sw_rst, 
                reset_ctrl.normal_rmt_sw_rst, reset_ctrl.non_delayed_rmt_sw_rst, reset_ctrl.pre_cm_fifo_sw_rst, reset_ctrl.cm_rd_pd_fifo_sw_rst, 
                reset_ctrl.cm_wr_pd_fifo_sw_rst, reset_ctrl.bb0_output_fifo_sw_rst, reset_ctrl.bb1_output_fifo_sw_rst, reset_ctrl.bb1_input_fifo_sw_rst, 
                reset_ctrl.tm_fifo_ptr_sw_rst, reset_ctrl.non_delayed_out_fifo_sw_rst, reset_ctrl.bb0_egr_msg_out_fifo_sw_rst);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (reset_ctrl.fpm_prefetch0_sw_rst != gtmv(m, 1) || reset_ctrl.fpm_prefetch1_sw_rst != gtmv(m, 1) || reset_ctrl.fpm_prefetch2_sw_rst != gtmv(m, 1) || reset_ctrl.fpm_prefetch3_sw_rst != gtmv(m, 1) || reset_ctrl.normal_rmt_sw_rst != gtmv(m, 1) || reset_ctrl.non_delayed_rmt_sw_rst != gtmv(m, 1) || reset_ctrl.pre_cm_fifo_sw_rst != gtmv(m, 1) || reset_ctrl.cm_rd_pd_fifo_sw_rst != gtmv(m, 1) || reset_ctrl.cm_wr_pd_fifo_sw_rst != gtmv(m, 1) || reset_ctrl.bb0_output_fifo_sw_rst != gtmv(m, 1) || reset_ctrl.bb1_output_fifo_sw_rst != gtmv(m, 1) || reset_ctrl.bb1_input_fifo_sw_rst != gtmv(m, 1) || reset_ctrl.tm_fifo_ptr_sw_rst != gtmv(m, 1) || reset_ctrl.non_delayed_out_fifo_sw_rst != gtmv(m, 1) || reset_ctrl.bb0_egr_msg_out_fifo_sw_rst != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        qm_drop_counters_ctrl drop_counters_ctrl = {.read_clear_pkts = gtmv(m, 1), .read_clear_bytes = gtmv(m, 1), .disable_wrap_around_pkts = gtmv(m, 1), .disable_wrap_around_bytes = gtmv(m, 1), .free_with_context_last_search = gtmv(m, 1), .wred_disable = gtmv(m, 1), .ddr_pd_congestion_disable = gtmv(m, 1), .ddr_byte_congestion_disable = gtmv(m, 1), .ddr_occupancy_disable = gtmv(m, 1), .ddr_fpm_congestion_disable = gtmv(m, 1), .fpm_ug_disable = gtmv(m, 1), .queue_occupancy_ddr_copy_decision_disable = gtmv(m, 1), .psram_occupancy_ddr_copy_decision_disable = gtmv(m, 1), .dont_send_mc_bit_to_bbh = gtmv(m, 1), .close_aggregation_on_timeout_disable = gtmv(m, 1), .fpm_congestion_buf_release_mechanism_disable = gtmv(m, 1), .fpm_buffer_global_res_enable = gtmv(m, 1), .qm_preserve_pd_with_fpm = gtmv(m, 1), .qm_residue_per_queue = gtmv(m, 1), .ghost_rpt_update_after_close_agg_en = gtmv(m, 1), .fpm_ug_flow_ctrl_disable = gtmv(m, 1), .ddr_write_multi_slave_en = gtmv(m, 1), .ddr_pd_congestion_agg_priority = gtmv(m, 1), .psram_occupancy_drop_disable = gtmv(m, 1), .qm_ddr_write_alignment = gtmv(m, 1), .exclusive_dont_drop = gtmv(m, 1), .dqmol_jira_973_fix_enable = gtmv(m, 1), .gpon_dbr_ceil = gtmv(m, 1), .drop_cnt_wred_drops = gtmv(m, 1), .same_sec_lvl_bit_agg_en = gtmv(m, 1), .glbl_egr_drop_cnt_read_clear_enable = gtmv(m, 1), .glbl_egr_aqm_drop_cnt_read_clear_enable = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_qm_drop_counters_ctrl_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            drop_counters_ctrl.read_clear_pkts, drop_counters_ctrl.read_clear_bytes, drop_counters_ctrl.disable_wrap_around_pkts, drop_counters_ctrl.disable_wrap_around_bytes, 
            drop_counters_ctrl.free_with_context_last_search, drop_counters_ctrl.wred_disable, drop_counters_ctrl.ddr_pd_congestion_disable, drop_counters_ctrl.ddr_byte_congestion_disable, 
            drop_counters_ctrl.ddr_occupancy_disable, drop_counters_ctrl.ddr_fpm_congestion_disable, drop_counters_ctrl.fpm_ug_disable, drop_counters_ctrl.queue_occupancy_ddr_copy_decision_disable, 
            drop_counters_ctrl.psram_occupancy_ddr_copy_decision_disable, drop_counters_ctrl.dont_send_mc_bit_to_bbh, drop_counters_ctrl.close_aggregation_on_timeout_disable, drop_counters_ctrl.fpm_congestion_buf_release_mechanism_disable, 
            drop_counters_ctrl.fpm_buffer_global_res_enable, drop_counters_ctrl.qm_preserve_pd_with_fpm, drop_counters_ctrl.qm_residue_per_queue, drop_counters_ctrl.ghost_rpt_update_after_close_agg_en, 
            drop_counters_ctrl.fpm_ug_flow_ctrl_disable, drop_counters_ctrl.ddr_write_multi_slave_en, drop_counters_ctrl.ddr_pd_congestion_agg_priority, drop_counters_ctrl.psram_occupancy_drop_disable, 
            drop_counters_ctrl.qm_ddr_write_alignment, drop_counters_ctrl.exclusive_dont_drop, drop_counters_ctrl.dqmol_jira_973_fix_enable, drop_counters_ctrl.gpon_dbr_ceil, 
            drop_counters_ctrl.drop_cnt_wred_drops, drop_counters_ctrl.same_sec_lvl_bit_agg_en, drop_counters_ctrl.glbl_egr_drop_cnt_read_clear_enable, drop_counters_ctrl.glbl_egr_aqm_drop_cnt_read_clear_enable);
        ag_err = ag_drv_qm_drop_counters_ctrl_set(&drop_counters_ctrl);
        if (!ag_err)
            ag_err = ag_drv_qm_drop_counters_ctrl_get(&drop_counters_ctrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_drop_counters_ctrl_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                drop_counters_ctrl.read_clear_pkts, drop_counters_ctrl.read_clear_bytes, drop_counters_ctrl.disable_wrap_around_pkts, drop_counters_ctrl.disable_wrap_around_bytes, 
                drop_counters_ctrl.free_with_context_last_search, drop_counters_ctrl.wred_disable, drop_counters_ctrl.ddr_pd_congestion_disable, drop_counters_ctrl.ddr_byte_congestion_disable, 
                drop_counters_ctrl.ddr_occupancy_disable, drop_counters_ctrl.ddr_fpm_congestion_disable, drop_counters_ctrl.fpm_ug_disable, drop_counters_ctrl.queue_occupancy_ddr_copy_decision_disable, 
                drop_counters_ctrl.psram_occupancy_ddr_copy_decision_disable, drop_counters_ctrl.dont_send_mc_bit_to_bbh, drop_counters_ctrl.close_aggregation_on_timeout_disable, drop_counters_ctrl.fpm_congestion_buf_release_mechanism_disable, 
                drop_counters_ctrl.fpm_buffer_global_res_enable, drop_counters_ctrl.qm_preserve_pd_with_fpm, drop_counters_ctrl.qm_residue_per_queue, drop_counters_ctrl.ghost_rpt_update_after_close_agg_en, 
                drop_counters_ctrl.fpm_ug_flow_ctrl_disable, drop_counters_ctrl.ddr_write_multi_slave_en, drop_counters_ctrl.ddr_pd_congestion_agg_priority, drop_counters_ctrl.psram_occupancy_drop_disable, 
                drop_counters_ctrl.qm_ddr_write_alignment, drop_counters_ctrl.exclusive_dont_drop, drop_counters_ctrl.dqmol_jira_973_fix_enable, drop_counters_ctrl.gpon_dbr_ceil, 
                drop_counters_ctrl.drop_cnt_wred_drops, drop_counters_ctrl.same_sec_lvl_bit_agg_en, drop_counters_ctrl.glbl_egr_drop_cnt_read_clear_enable, drop_counters_ctrl.glbl_egr_aqm_drop_cnt_read_clear_enable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (drop_counters_ctrl.read_clear_pkts != gtmv(m, 1) || drop_counters_ctrl.read_clear_bytes != gtmv(m, 1) || drop_counters_ctrl.disable_wrap_around_pkts != gtmv(m, 1) || drop_counters_ctrl.disable_wrap_around_bytes != gtmv(m, 1) || drop_counters_ctrl.free_with_context_last_search != gtmv(m, 1) || drop_counters_ctrl.wred_disable != gtmv(m, 1) || drop_counters_ctrl.ddr_pd_congestion_disable != gtmv(m, 1) || drop_counters_ctrl.ddr_byte_congestion_disable != gtmv(m, 1) || drop_counters_ctrl.ddr_occupancy_disable != gtmv(m, 1) || drop_counters_ctrl.ddr_fpm_congestion_disable != gtmv(m, 1) || drop_counters_ctrl.fpm_ug_disable != gtmv(m, 1) || drop_counters_ctrl.queue_occupancy_ddr_copy_decision_disable != gtmv(m, 1) || drop_counters_ctrl.psram_occupancy_ddr_copy_decision_disable != gtmv(m, 1) || drop_counters_ctrl.dont_send_mc_bit_to_bbh != gtmv(m, 1) || drop_counters_ctrl.close_aggregation_on_timeout_disable != gtmv(m, 1) || drop_counters_ctrl.fpm_congestion_buf_release_mechanism_disable != gtmv(m, 1) || drop_counters_ctrl.fpm_buffer_global_res_enable != gtmv(m, 1) || drop_counters_ctrl.qm_preserve_pd_with_fpm != gtmv(m, 1) || drop_counters_ctrl.qm_residue_per_queue != gtmv(m, 1) || drop_counters_ctrl.ghost_rpt_update_after_close_agg_en != gtmv(m, 1) || drop_counters_ctrl.fpm_ug_flow_ctrl_disable != gtmv(m, 1) || drop_counters_ctrl.ddr_write_multi_slave_en != gtmv(m, 1) || drop_counters_ctrl.ddr_pd_congestion_agg_priority != gtmv(m, 1) || drop_counters_ctrl.psram_occupancy_drop_disable != gtmv(m, 1) || drop_counters_ctrl.qm_ddr_write_alignment != gtmv(m, 1) || drop_counters_ctrl.exclusive_dont_drop != gtmv(m, 1) || drop_counters_ctrl.dqmol_jira_973_fix_enable != gtmv(m, 1) || drop_counters_ctrl.gpon_dbr_ceil != gtmv(m, 1) || drop_counters_ctrl.drop_cnt_wred_drops != gtmv(m, 1) || drop_counters_ctrl.same_sec_lvl_bit_agg_en != gtmv(m, 1) || drop_counters_ctrl.glbl_egr_drop_cnt_read_clear_enable != gtmv(m, 1) || drop_counters_ctrl.glbl_egr_aqm_drop_cnt_read_clear_enable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        qm_fpm_ctrl fpm_ctrl = {.fpm_pool_bp_enable = gtmv(m, 1), .fpm_congestion_bp_enable = gtmv(m, 1), .fpm_force_bp_lvl = gtmv(m, 5), .fpm_prefetch_granularity = gtmv(m, 1), .fpm_prefetch_min_pool_size = gtmv(m, 2), .fpm_prefetch_pending_req_limit = gtmv(m, 7), .fpm_override_bb_id_en = gtmv(m, 1), .fpm_override_bb_id_value = gtmv(m, 6)};
        bdmf_session_print(session, "ag_drv_qm_fpm_ctrl_set( %u %u %u %u %u %u %u %u)\n",
            fpm_ctrl.fpm_pool_bp_enable, fpm_ctrl.fpm_congestion_bp_enable, fpm_ctrl.fpm_force_bp_lvl, fpm_ctrl.fpm_prefetch_granularity, 
            fpm_ctrl.fpm_prefetch_min_pool_size, fpm_ctrl.fpm_prefetch_pending_req_limit, fpm_ctrl.fpm_override_bb_id_en, fpm_ctrl.fpm_override_bb_id_value);
        ag_err = ag_drv_qm_fpm_ctrl_set(&fpm_ctrl);
        if (!ag_err)
            ag_err = ag_drv_qm_fpm_ctrl_get(&fpm_ctrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_ctrl_get( %u %u %u %u %u %u %u %u)\n",
                fpm_ctrl.fpm_pool_bp_enable, fpm_ctrl.fpm_congestion_bp_enable, fpm_ctrl.fpm_force_bp_lvl, fpm_ctrl.fpm_prefetch_granularity, 
                fpm_ctrl.fpm_prefetch_min_pool_size, fpm_ctrl.fpm_prefetch_pending_req_limit, fpm_ctrl.fpm_override_bb_id_en, fpm_ctrl.fpm_override_bb_id_value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fpm_ctrl.fpm_pool_bp_enable != gtmv(m, 1) || fpm_ctrl.fpm_congestion_bp_enable != gtmv(m, 1) || fpm_ctrl.fpm_force_bp_lvl != gtmv(m, 5) || fpm_ctrl.fpm_prefetch_granularity != gtmv(m, 1) || fpm_ctrl.fpm_prefetch_min_pool_size != gtmv(m, 2) || fpm_ctrl.fpm_prefetch_pending_req_limit != gtmv(m, 7) || fpm_ctrl.fpm_override_bb_id_en != gtmv(m, 1) || fpm_ctrl.fpm_override_bb_id_value != gtmv(m, 6))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t total_pd_thr = gtmv(m, 28);
        bdmf_session_print(session, "ag_drv_qm_qm_pd_cong_ctrl_set( %u)\n",
            total_pd_thr);
        ag_err = ag_drv_qm_qm_pd_cong_ctrl_set(total_pd_thr);
        if (!ag_err)
            ag_err = ag_drv_qm_qm_pd_cong_ctrl_get(&total_pd_thr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_qm_pd_cong_ctrl_get( %u)\n",
                total_pd_thr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (total_pd_thr != gtmv(m, 28))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t abs_drop_queue = gtmv(m, 9);
        bdmf_boolean abs_drop_queue_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_qm_global_cfg_abs_drop_queue_set( %u %u)\n",
            abs_drop_queue, abs_drop_queue_en);
        ag_err = ag_drv_qm_global_cfg_abs_drop_queue_set(abs_drop_queue, abs_drop_queue_en);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_abs_drop_queue_get(&abs_drop_queue, &abs_drop_queue_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_abs_drop_queue_get( %u %u)\n",
                abs_drop_queue, abs_drop_queue_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (abs_drop_queue != gtmv(m, 9) || abs_drop_queue_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        qm_global_cfg_aggregation_ctrl global_cfg_aggregation_ctrl = {.max_agg_bytes = gtmv(m, 10), .max_agg_pkts = gtmv(m, 2), .agg_ovr_512b_en = gtmv(m, 1), .max_agg_pkt_size = gtmv(m, 10), .min_agg_pkt_size = gtmv(m, 9)};
        bdmf_session_print(session, "ag_drv_qm_global_cfg_aggregation_ctrl_set( %u %u %u %u %u)\n",
            global_cfg_aggregation_ctrl.max_agg_bytes, global_cfg_aggregation_ctrl.max_agg_pkts, global_cfg_aggregation_ctrl.agg_ovr_512b_en, global_cfg_aggregation_ctrl.max_agg_pkt_size, 
            global_cfg_aggregation_ctrl.min_agg_pkt_size);
        ag_err = ag_drv_qm_global_cfg_aggregation_ctrl_set(&global_cfg_aggregation_ctrl);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_aggregation_ctrl_get(&global_cfg_aggregation_ctrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_aggregation_ctrl_get( %u %u %u %u %u)\n",
                global_cfg_aggregation_ctrl.max_agg_bytes, global_cfg_aggregation_ctrl.max_agg_pkts, global_cfg_aggregation_ctrl.agg_ovr_512b_en, global_cfg_aggregation_ctrl.max_agg_pkt_size, 
                global_cfg_aggregation_ctrl.min_agg_pkt_size);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (global_cfg_aggregation_ctrl.max_agg_bytes != gtmv(m, 10) || global_cfg_aggregation_ctrl.max_agg_pkts != gtmv(m, 2) || global_cfg_aggregation_ctrl.agg_ovr_512b_en != gtmv(m, 1) || global_cfg_aggregation_ctrl.max_agg_pkt_size != gtmv(m, 10) || global_cfg_aggregation_ctrl.min_agg_pkt_size != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean agg_pool_sel_en = gtmv(m, 1);
        uint8_t agg_pool_sel = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_qm_global_cfg_aggregation_ctrl2_set( %u %u)\n",
            agg_pool_sel_en, agg_pool_sel);
        ag_err = ag_drv_qm_global_cfg_aggregation_ctrl2_set(agg_pool_sel_en, agg_pool_sel);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_aggregation_ctrl2_get(&agg_pool_sel_en, &agg_pool_sel);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_aggregation_ctrl2_get( %u %u)\n",
                agg_pool_sel_en, agg_pool_sel);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (agg_pool_sel_en != gtmv(m, 1) || agg_pool_sel != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t fpm_base_addr = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_qm_fpm_base_addr_set( %u)\n",
            fpm_base_addr);
        ag_err = ag_drv_qm_fpm_base_addr_set(fpm_base_addr);
        if (!ag_err)
            ag_err = ag_drv_qm_fpm_base_addr_get(&fpm_base_addr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_base_addr_get( %u)\n",
                fpm_base_addr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fpm_base_addr != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t fpm_base_addr = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_qm_global_cfg_fpm_coherent_base_addr_set( %u)\n",
            fpm_base_addr);
        ag_err = ag_drv_qm_global_cfg_fpm_coherent_base_addr_set(fpm_base_addr);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_fpm_coherent_base_addr_get(&fpm_base_addr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_fpm_coherent_base_addr_get( %u)\n",
                fpm_base_addr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fpm_base_addr != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t ddr_sop_offset0 = gtmv(m, 11);
        uint16_t ddr_sop_offset1 = gtmv(m, 11);
        bdmf_session_print(session, "ag_drv_qm_ddr_sop_offset_set( %u %u)\n",
            ddr_sop_offset0, ddr_sop_offset1);
        ag_err = ag_drv_qm_ddr_sop_offset_set(ddr_sop_offset0, ddr_sop_offset1);
        if (!ag_err)
            ag_err = ag_drv_qm_ddr_sop_offset_get(&ddr_sop_offset0, &ddr_sop_offset1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_ddr_sop_offset_get( %u %u)\n",
                ddr_sop_offset0, ddr_sop_offset1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddr_sop_offset0 != gtmv(m, 11) || ddr_sop_offset1 != gtmv(m, 11))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        qm_epon_overhead_ctrl epon_overhead_ctrl = {.epon_line_rate = gtmv(m, 1), .epon_crc_add_disable = gtmv(m, 1), .mac_flow_overwrite_crc_en = gtmv(m, 1), .mac_flow_overwrite_crc = gtmv(m, 8), .fec_ipg_length = gtmv(m, 11)};
        bdmf_session_print(session, "ag_drv_qm_epon_overhead_ctrl_set( %u %u %u %u %u)\n",
            epon_overhead_ctrl.epon_line_rate, epon_overhead_ctrl.epon_crc_add_disable, epon_overhead_ctrl.mac_flow_overwrite_crc_en, epon_overhead_ctrl.mac_flow_overwrite_crc, 
            epon_overhead_ctrl.fec_ipg_length);
        ag_err = ag_drv_qm_epon_overhead_ctrl_set(&epon_overhead_ctrl);
        if (!ag_err)
            ag_err = ag_drv_qm_epon_overhead_ctrl_get(&epon_overhead_ctrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_epon_overhead_ctrl_get( %u %u %u %u %u)\n",
                epon_overhead_ctrl.epon_line_rate, epon_overhead_ctrl.epon_crc_add_disable, epon_overhead_ctrl.mac_flow_overwrite_crc_en, epon_overhead_ctrl.mac_flow_overwrite_crc, 
                epon_overhead_ctrl.fec_ipg_length);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (epon_overhead_ctrl.epon_line_rate != gtmv(m, 1) || epon_overhead_ctrl.epon_crc_add_disable != gtmv(m, 1) || epon_overhead_ctrl.mac_flow_overwrite_crc_en != gtmv(m, 1) || epon_overhead_ctrl.mac_flow_overwrite_crc != gtmv(m, 8) || epon_overhead_ctrl.fec_ipg_length != gtmv(m, 11))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t addr = gtmv(m, 6);
        uint8_t bbhtx_req_otf = gtmv(m, 6);
        bdmf_session_print(session, "ag_drv_qm_global_cfg_bbhtx_fifo_addr_set( %u %u)\n",
            addr, bbhtx_req_otf);
        ag_err = ag_drv_qm_global_cfg_bbhtx_fifo_addr_set(addr, bbhtx_req_otf);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_bbhtx_fifo_addr_get(&addr, &bbhtx_req_otf);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_bbhtx_fifo_addr_get( %u %u)\n",
                addr, bbhtx_req_otf);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (addr != gtmv(m, 6) || bbhtx_req_otf != gtmv(m, 6))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t queue_num = gtmv(m, 9);
        bdmf_boolean flush_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_egress_flush_queue_set( %u %u)\n",
            queue_num, flush_en);
        ag_err = ag_drv_qm_global_cfg_qm_egress_flush_queue_set(queue_num, flush_en);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_qm_egress_flush_queue_get(&queue_num, &flush_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_egress_flush_queue_get( %u %u)\n",
                queue_num, flush_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (queue_num != gtmv(m, 9) || flush_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t prescaler_granularity = gtmv(m, 3);
        uint8_t aggregation_timeout_value = gtmv(m, 3);
        bdmf_boolean pd_occupancy_en = gtmv(m, 1);
        uint8_t pd_occupancy_value = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set( %u %u %u %u)\n",
            prescaler_granularity, aggregation_timeout_value, pd_occupancy_en, pd_occupancy_value);
        ag_err = ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set(prescaler_granularity, aggregation_timeout_value, pd_occupancy_en, pd_occupancy_value);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_get(&prescaler_granularity, &aggregation_timeout_value, &pd_occupancy_en, &pd_occupancy_value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_get( %u %u %u %u)\n",
                prescaler_granularity, aggregation_timeout_value, pd_occupancy_en, pd_occupancy_value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (prescaler_granularity != gtmv(m, 3) || aggregation_timeout_value != gtmv(m, 3) || pd_occupancy_en != gtmv(m, 1) || pd_occupancy_value != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t fpm_gbl_cnt = gtmv(m, 18);
        bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_set( %u)\n",
            fpm_gbl_cnt);
        ag_err = ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_set(fpm_gbl_cnt);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_get(&fpm_gbl_cnt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_get( %u)\n",
                fpm_gbl_cnt);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fpm_gbl_cnt != gtmv(m, 18))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t pair_idx = gtmv(m, 2);
        uint16_t ddr_headroom = gtmv(m, 10);
        uint16_t ddr_tailroom = gtmv(m, 11);
        bdmf_session_print(session, "ag_drv_qm_qm_ddr_spare_room_set( %u %u %u)\n", pair_idx,
            ddr_headroom, ddr_tailroom);
        ag_err = ag_drv_qm_qm_ddr_spare_room_set(pair_idx, ddr_headroom, ddr_tailroom);
        if (!ag_err)
            ag_err = ag_drv_qm_qm_ddr_spare_room_get(pair_idx, &ddr_headroom, &ddr_tailroom);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_qm_ddr_spare_room_get( %u %u %u)\n", pair_idx,
                ddr_headroom, ddr_tailroom);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddr_headroom != gtmv(m, 10) || ddr_tailroom != gtmv(m, 11))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t dummy_profile_0 = gtmv(m, 2);
        uint8_t dummy_profile_1 = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_qm_global_cfg_dummy_spare_room_profile_id_set( %u %u)\n",
            dummy_profile_0, dummy_profile_1);
        ag_err = ag_drv_qm_global_cfg_dummy_spare_room_profile_id_set(dummy_profile_0, dummy_profile_1);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_dummy_spare_room_profile_id_get(&dummy_profile_0, &dummy_profile_1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_dummy_spare_room_profile_id_get( %u %u)\n",
                dummy_profile_0, dummy_profile_1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (dummy_profile_0 != gtmv(m, 2) || dummy_profile_1 != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t tkn_reqout_h = gtmv(m, 8);
        uint8_t tkn_reqout_d = gtmv(m, 8);
        uint8_t offload_reqout_h = gtmv(m, 8);
        uint8_t offload_reqout_d = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_qm_global_cfg_dqm_ubus_ctrl_set( %u %u %u %u)\n",
            tkn_reqout_h, tkn_reqout_d, offload_reqout_h, offload_reqout_d);
        ag_err = ag_drv_qm_global_cfg_dqm_ubus_ctrl_set(tkn_reqout_h, tkn_reqout_d, offload_reqout_h, offload_reqout_d);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_dqm_ubus_ctrl_get(&tkn_reqout_h, &tkn_reqout_d, &offload_reqout_h, &offload_reqout_d);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_dqm_ubus_ctrl_get( %u %u %u %u)\n",
                tkn_reqout_h, tkn_reqout_d, offload_reqout_h, offload_reqout_d);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (tkn_reqout_h != gtmv(m, 8) || tkn_reqout_d != gtmv(m, 8) || offload_reqout_h != gtmv(m, 8) || offload_reqout_d != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean mem_init_en = gtmv(m, 1);
        uint8_t mem_sel_init = gtmv(m, 3);
        uint8_t mem_size_init = gtmv(m, 3);
        bdmf_session_print(session, "ag_drv_qm_global_cfg_mem_auto_init_set( %u %u %u)\n",
            mem_init_en, mem_sel_init, mem_size_init);
        ag_err = ag_drv_qm_global_cfg_mem_auto_init_set(mem_init_en, mem_sel_init, mem_size_init);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_mem_auto_init_get(&mem_init_en, &mem_sel_init, &mem_size_init);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_mem_auto_init_get( %u %u %u)\n",
                mem_init_en, mem_sel_init, mem_size_init);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mem_init_en != gtmv(m, 1) || mem_sel_init != gtmv(m, 3) || mem_size_init != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean mem_init_done = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_mem_auto_init_sts_get(&mem_init_done);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_mem_auto_init_sts_get( %u)\n",
                mem_init_done);
        }
    }

    {
        uint8_t pool_0_num_of_tkns = gtmv(m, 6);
        uint8_t pool_1_num_of_tkns = gtmv(m, 6);
        uint8_t pool_2_num_of_tkns = gtmv(m, 6);
        uint8_t pool_3_num_of_tkns = gtmv(m, 6);
        bdmf_session_print(session, "ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_size_tokens_set( %u %u %u %u)\n",
            pool_0_num_of_tkns, pool_1_num_of_tkns, pool_2_num_of_tkns, pool_3_num_of_tkns);
        ag_err = ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_size_tokens_set(pool_0_num_of_tkns, pool_1_num_of_tkns, pool_2_num_of_tkns, pool_3_num_of_tkns);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_size_tokens_get(&pool_0_num_of_tkns, &pool_1_num_of_tkns, &pool_2_num_of_tkns, &pool_3_num_of_tkns);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_size_tokens_get( %u %u %u %u)\n",
                pool_0_num_of_tkns, pool_1_num_of_tkns, pool_2_num_of_tkns, pool_3_num_of_tkns);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool_0_num_of_tkns != gtmv(m, 6) || pool_1_num_of_tkns != gtmv(m, 6) || pool_2_num_of_tkns != gtmv(m, 6) || pool_3_num_of_tkns != gtmv(m, 6))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t pool_0_num_of_bytes = gtmv(m, 14);
        uint16_t pool_1_num_of_bytes = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte_set( %u %u)\n",
            pool_0_num_of_bytes, pool_1_num_of_bytes);
        ag_err = ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte_set(pool_0_num_of_bytes, pool_1_num_of_bytes);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte_get(&pool_0_num_of_bytes, &pool_1_num_of_bytes);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte_get( %u %u)\n",
                pool_0_num_of_bytes, pool_1_num_of_bytes);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool_0_num_of_bytes != gtmv(m, 14) || pool_1_num_of_bytes != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t pool_2_num_of_bytes = gtmv(m, 14);
        uint16_t pool_3_num_of_bytes = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte_set( %u %u)\n",
            pool_2_num_of_bytes, pool_3_num_of_bytes);
        ag_err = ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte_set(pool_2_num_of_bytes, pool_3_num_of_bytes);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte_get(&pool_2_num_of_bytes, &pool_3_num_of_bytes);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte_get( %u %u)\n",
                pool_2_num_of_bytes, pool_3_num_of_bytes);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool_2_num_of_bytes != gtmv(m, 14) || pool_3_num_of_bytes != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t mc_headers_pool_sel = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_qm_global_cfg_mc_ctrl_set( %u)\n",
            mc_headers_pool_sel);
        ag_err = ag_drv_qm_global_cfg_mc_ctrl_set(mc_headers_pool_sel);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_mc_ctrl_get(&mc_headers_pool_sel);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_mc_ctrl_get( %u)\n",
                mc_headers_pool_sel);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mc_headers_pool_sel != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t value = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_qm_global_cfg_aqm_clk_counter_cycle_set( %u)\n",
            value);
        ag_err = ag_drv_qm_global_cfg_aqm_clk_counter_cycle_set(value);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_aqm_clk_counter_cycle_get(&value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_aqm_clk_counter_cycle_get( %u)\n",
                value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (value != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t value = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_qm_global_cfg_aqm_push_to_empty_thr_set( %u)\n",
            value);
        ag_err = ag_drv_qm_global_cfg_aqm_push_to_empty_thr_set(value);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_aqm_push_to_empty_thr_get(&value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_aqm_push_to_empty_thr_get( %u)\n",
                value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (value != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        qm_global_cfg_qm_general_ctrl2 global_cfg_qm_general_ctrl2 = {.egress_accumulated_cnt_pkts_read_clear_enable = gtmv(m, 1), .egress_accumulated_cnt_bytes_read_clear_enable = gtmv(m, 1), .agg_closure_suspend_on_bp = gtmv(m, 1), .bufmng_en_or_ug_cntr = gtmv(m, 1), .dqm_to_fpm_ubus_or_fpmini = gtmv(m, 1), .agg_closure_suspend_on_fpm_congestion_disable = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_general_ctrl2_set( %u %u %u %u %u %u)\n",
            global_cfg_qm_general_ctrl2.egress_accumulated_cnt_pkts_read_clear_enable, global_cfg_qm_general_ctrl2.egress_accumulated_cnt_bytes_read_clear_enable, global_cfg_qm_general_ctrl2.agg_closure_suspend_on_bp, global_cfg_qm_general_ctrl2.bufmng_en_or_ug_cntr, 
            global_cfg_qm_general_ctrl2.dqm_to_fpm_ubus_or_fpmini, global_cfg_qm_general_ctrl2.agg_closure_suspend_on_fpm_congestion_disable);
        ag_err = ag_drv_qm_global_cfg_qm_general_ctrl2_set(&global_cfg_qm_general_ctrl2);
        if (!ag_err)
            ag_err = ag_drv_qm_global_cfg_qm_general_ctrl2_get(&global_cfg_qm_general_ctrl2);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_general_ctrl2_get( %u %u %u %u %u %u)\n",
                global_cfg_qm_general_ctrl2.egress_accumulated_cnt_pkts_read_clear_enable, global_cfg_qm_general_ctrl2.egress_accumulated_cnt_bytes_read_clear_enable, global_cfg_qm_general_ctrl2.agg_closure_suspend_on_bp, global_cfg_qm_general_ctrl2.bufmng_en_or_ug_cntr, 
                global_cfg_qm_general_ctrl2.dqm_to_fpm_ubus_or_fpmini, global_cfg_qm_general_ctrl2.agg_closure_suspend_on_fpm_congestion_disable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (global_cfg_qm_general_ctrl2.egress_accumulated_cnt_pkts_read_clear_enable != gtmv(m, 1) || global_cfg_qm_general_ctrl2.egress_accumulated_cnt_bytes_read_clear_enable != gtmv(m, 1) || global_cfg_qm_general_ctrl2.agg_closure_suspend_on_bp != gtmv(m, 1) || global_cfg_qm_general_ctrl2.bufmng_en_or_ug_cntr != gtmv(m, 1) || global_cfg_qm_general_ctrl2.dqm_to_fpm_ubus_or_fpmini != gtmv(m, 1) || global_cfg_qm_general_ctrl2.agg_closure_suspend_on_fpm_congestion_disable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t pool_idx = gtmv(m, 2);
        qm_fpm_pool_thr fpm_pool_thr = {.lower_thr = gtmv(m, 7), .higher_thr = gtmv(m, 7)};
        bdmf_session_print(session, "ag_drv_qm_fpm_pool_thr_set( %u %u %u)\n", pool_idx,
            fpm_pool_thr.lower_thr, fpm_pool_thr.higher_thr);
        ag_err = ag_drv_qm_fpm_pool_thr_set(pool_idx, &fpm_pool_thr);
        if (!ag_err)
            ag_err = ag_drv_qm_fpm_pool_thr_get(pool_idx, &fpm_pool_thr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_pool_thr_get( %u %u %u)\n", pool_idx,
                fpm_pool_thr.lower_thr, fpm_pool_thr.higher_thr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fpm_pool_thr.lower_thr != gtmv(m, 7) || fpm_pool_thr.higher_thr != gtmv(m, 7))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t grp_idx = gtmv(m, 2);
        uint32_t fpm_ug_cnt = gtmv(m, 18);
        bdmf_session_print(session, "ag_drv_qm_fpm_ug_cnt_set( %u %u)\n", grp_idx,
            fpm_ug_cnt);
        ag_err = ag_drv_qm_fpm_ug_cnt_set(grp_idx, fpm_ug_cnt);
        if (!ag_err)
            ag_err = ag_drv_qm_fpm_ug_cnt_get(grp_idx, &fpm_ug_cnt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_ug_cnt_get( %u %u)\n", grp_idx,
                fpm_ug_cnt);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fpm_ug_cnt != gtmv(m, 18))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        qm_intr_ctrl_isr intr_ctrl_isr = {.qm_dqm_pop_on_empty = gtmv(m, 1), .qm_dqm_push_on_full = gtmv(m, 1), .qm_cpu_pop_on_empty = gtmv(m, 1), .qm_cpu_push_on_full = gtmv(m, 1), .qm_normal_queue_pd_no_credit = gtmv(m, 1), .qm_non_delayed_queue_pd_no_credit = gtmv(m, 1), .qm_non_valid_queue = gtmv(m, 1), .qm_agg_coherent_inconsistency = gtmv(m, 1), .qm_force_copy_on_non_delayed = gtmv(m, 1), .qm_fpm_pool_size_nonexistent = gtmv(m, 1), .qm_target_mem_abs_contradiction = gtmv(m, 1), .qm_1588_drop = gtmv(m, 1), .qm_1588_multicast_contradiction = gtmv(m, 1), .qm_byte_drop_cnt_overrun = gtmv(m, 1), .qm_pkt_drop_cnt_overrun = gtmv(m, 1), .qm_total_byte_cnt_underrun = gtmv(m, 1), .qm_total_pkt_cnt_underrun = gtmv(m, 1), .qm_fpm_ug0_underrun = gtmv(m, 1), .qm_fpm_ug1_underrun = gtmv(m, 1), .qm_fpm_ug2_underrun = gtmv(m, 1), .qm_fpm_ug3_underrun = gtmv(m, 1), .qm_timer_wraparound = gtmv(m, 1), .qm_copy_plen_zero = gtmv(m, 1), .qm_ingress_bb_unexpected_msg = gtmv(m, 1), .qm_egress_bb_unexpected_msg = gtmv(m, 1), .dqm_reached_full = gtmv(m, 1), .qm_fpmini_intr = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_qm_intr_ctrl_isr_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
            intr_ctrl_isr.qm_dqm_pop_on_empty, intr_ctrl_isr.qm_dqm_push_on_full, intr_ctrl_isr.qm_cpu_pop_on_empty, intr_ctrl_isr.qm_cpu_push_on_full, 
            intr_ctrl_isr.qm_normal_queue_pd_no_credit, intr_ctrl_isr.qm_non_delayed_queue_pd_no_credit, intr_ctrl_isr.qm_non_valid_queue, intr_ctrl_isr.qm_agg_coherent_inconsistency, 
            intr_ctrl_isr.qm_force_copy_on_non_delayed, intr_ctrl_isr.qm_fpm_pool_size_nonexistent, intr_ctrl_isr.qm_target_mem_abs_contradiction, intr_ctrl_isr.qm_1588_drop, 
            intr_ctrl_isr.qm_1588_multicast_contradiction, intr_ctrl_isr.qm_byte_drop_cnt_overrun, intr_ctrl_isr.qm_pkt_drop_cnt_overrun, intr_ctrl_isr.qm_total_byte_cnt_underrun, 
            intr_ctrl_isr.qm_total_pkt_cnt_underrun, intr_ctrl_isr.qm_fpm_ug0_underrun, intr_ctrl_isr.qm_fpm_ug1_underrun, intr_ctrl_isr.qm_fpm_ug2_underrun, 
            intr_ctrl_isr.qm_fpm_ug3_underrun, intr_ctrl_isr.qm_timer_wraparound, intr_ctrl_isr.qm_copy_plen_zero, intr_ctrl_isr.qm_ingress_bb_unexpected_msg, 
            intr_ctrl_isr.qm_egress_bb_unexpected_msg, intr_ctrl_isr.dqm_reached_full, intr_ctrl_isr.qm_fpmini_intr);
        ag_err = ag_drv_qm_intr_ctrl_isr_set(&intr_ctrl_isr);
        if (!ag_err)
            ag_err = ag_drv_qm_intr_ctrl_isr_get(&intr_ctrl_isr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_intr_ctrl_isr_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n",
                intr_ctrl_isr.qm_dqm_pop_on_empty, intr_ctrl_isr.qm_dqm_push_on_full, intr_ctrl_isr.qm_cpu_pop_on_empty, intr_ctrl_isr.qm_cpu_push_on_full, 
                intr_ctrl_isr.qm_normal_queue_pd_no_credit, intr_ctrl_isr.qm_non_delayed_queue_pd_no_credit, intr_ctrl_isr.qm_non_valid_queue, intr_ctrl_isr.qm_agg_coherent_inconsistency, 
                intr_ctrl_isr.qm_force_copy_on_non_delayed, intr_ctrl_isr.qm_fpm_pool_size_nonexistent, intr_ctrl_isr.qm_target_mem_abs_contradiction, intr_ctrl_isr.qm_1588_drop, 
                intr_ctrl_isr.qm_1588_multicast_contradiction, intr_ctrl_isr.qm_byte_drop_cnt_overrun, intr_ctrl_isr.qm_pkt_drop_cnt_overrun, intr_ctrl_isr.qm_total_byte_cnt_underrun, 
                intr_ctrl_isr.qm_total_pkt_cnt_underrun, intr_ctrl_isr.qm_fpm_ug0_underrun, intr_ctrl_isr.qm_fpm_ug1_underrun, intr_ctrl_isr.qm_fpm_ug2_underrun, 
                intr_ctrl_isr.qm_fpm_ug3_underrun, intr_ctrl_isr.qm_timer_wraparound, intr_ctrl_isr.qm_copy_plen_zero, intr_ctrl_isr.qm_ingress_bb_unexpected_msg, 
                intr_ctrl_isr.qm_egress_bb_unexpected_msg, intr_ctrl_isr.dqm_reached_full, intr_ctrl_isr.qm_fpmini_intr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (intr_ctrl_isr.qm_dqm_pop_on_empty != gtmv(m, 1) || intr_ctrl_isr.qm_dqm_push_on_full != gtmv(m, 1) || intr_ctrl_isr.qm_cpu_pop_on_empty != gtmv(m, 1) || intr_ctrl_isr.qm_cpu_push_on_full != gtmv(m, 1) || intr_ctrl_isr.qm_normal_queue_pd_no_credit != gtmv(m, 1) || intr_ctrl_isr.qm_non_delayed_queue_pd_no_credit != gtmv(m, 1) || intr_ctrl_isr.qm_non_valid_queue != gtmv(m, 1) || intr_ctrl_isr.qm_agg_coherent_inconsistency != gtmv(m, 1) || intr_ctrl_isr.qm_force_copy_on_non_delayed != gtmv(m, 1) || intr_ctrl_isr.qm_fpm_pool_size_nonexistent != gtmv(m, 1) || intr_ctrl_isr.qm_target_mem_abs_contradiction != gtmv(m, 1) || intr_ctrl_isr.qm_1588_drop != gtmv(m, 1) || intr_ctrl_isr.qm_1588_multicast_contradiction != gtmv(m, 1) || intr_ctrl_isr.qm_byte_drop_cnt_overrun != gtmv(m, 1) || intr_ctrl_isr.qm_pkt_drop_cnt_overrun != gtmv(m, 1) || intr_ctrl_isr.qm_total_byte_cnt_underrun != gtmv(m, 1) || intr_ctrl_isr.qm_total_pkt_cnt_underrun != gtmv(m, 1) || intr_ctrl_isr.qm_fpm_ug0_underrun != gtmv(m, 1) || intr_ctrl_isr.qm_fpm_ug1_underrun != gtmv(m, 1) || intr_ctrl_isr.qm_fpm_ug2_underrun != gtmv(m, 1) || intr_ctrl_isr.qm_fpm_ug3_underrun != gtmv(m, 1) || intr_ctrl_isr.qm_timer_wraparound != gtmv(m, 1) || intr_ctrl_isr.qm_copy_plen_zero != gtmv(m, 1) || intr_ctrl_isr.qm_ingress_bb_unexpected_msg != gtmv(m, 1) || intr_ctrl_isr.qm_egress_bb_unexpected_msg != gtmv(m, 1) || intr_ctrl_isr.dqm_reached_full != gtmv(m, 1) || intr_ctrl_isr.qm_fpmini_intr != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t ism = gtmv(m, 27);
        if (!ag_err)
            ag_err = ag_drv_qm_intr_ctrl_ism_get(&ism);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_intr_ctrl_ism_get( %u)\n",
                ism);
        }
    }

    {
        uint32_t iem = gtmv(m, 27);
        bdmf_session_print(session, "ag_drv_qm_intr_ctrl_ier_set( %u)\n",
            iem);
        ag_err = ag_drv_qm_intr_ctrl_ier_set(iem);
        if (!ag_err)
            ag_err = ag_drv_qm_intr_ctrl_ier_get(&iem);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_intr_ctrl_ier_get( %u)\n",
                iem);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (iem != gtmv(m, 27))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t ist = gtmv(m, 27);
        bdmf_session_print(session, "ag_drv_qm_intr_ctrl_itr_set( %u)\n",
            ist);
        ag_err = ag_drv_qm_intr_ctrl_itr_set(ist);
        if (!ag_err)
            ag_err = ag_drv_qm_intr_ctrl_itr_get(&ist);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_intr_ctrl_itr_get( %u)\n",
                ist);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ist != gtmv(m, 27))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        qm_clk_gate_clk_gate_cntrl clk_gate_clk_gate_cntrl = {.bypass_clk_gate = gtmv(m, 1), .timer_val = gtmv(m, 8), .keep_alive_en = gtmv(m, 1), .keep_alive_intrvl = gtmv(m, 3), .keep_alive_cyc = gtmv(m, 8)};
        bdmf_session_print(session, "ag_drv_qm_clk_gate_clk_gate_cntrl_set( %u %u %u %u %u)\n",
            clk_gate_clk_gate_cntrl.bypass_clk_gate, clk_gate_clk_gate_cntrl.timer_val, clk_gate_clk_gate_cntrl.keep_alive_en, clk_gate_clk_gate_cntrl.keep_alive_intrvl, 
            clk_gate_clk_gate_cntrl.keep_alive_cyc);
        ag_err = ag_drv_qm_clk_gate_clk_gate_cntrl_set(&clk_gate_clk_gate_cntrl);
        if (!ag_err)
            ag_err = ag_drv_qm_clk_gate_clk_gate_cntrl_get(&clk_gate_clk_gate_cntrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_clk_gate_clk_gate_cntrl_get( %u %u %u %u %u)\n",
                clk_gate_clk_gate_cntrl.bypass_clk_gate, clk_gate_clk_gate_cntrl.timer_val, clk_gate_clk_gate_cntrl.keep_alive_en, clk_gate_clk_gate_cntrl.keep_alive_intrvl, 
                clk_gate_clk_gate_cntrl.keep_alive_cyc);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (clk_gate_clk_gate_cntrl.bypass_clk_gate != gtmv(m, 1) || clk_gate_clk_gate_cntrl.timer_val != gtmv(m, 8) || clk_gate_clk_gate_cntrl.keep_alive_en != gtmv(m, 1) || clk_gate_clk_gate_cntrl.keep_alive_intrvl != gtmv(m, 3) || clk_gate_clk_gate_cntrl.keep_alive_cyc != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t indirect_grp_idx = gtmv(m, 2);
        uint16_t queue_num = gtmv(m, 9);
        uint8_t cmd = gtmv(m, 2);
        bdmf_boolean done = gtmv(m, 1);
        bdmf_boolean error = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set( %u %u %u %u %u)\n", indirect_grp_idx,
            queue_num, cmd, done, error);
        ag_err = ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set(indirect_grp_idx, queue_num, cmd, done, error);
        if (!ag_err)
            ag_err = ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_get(indirect_grp_idx, &queue_num, &cmd, &done, &error);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_get( %u %u %u %u %u)\n", indirect_grp_idx,
                queue_num, cmd, done, error);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (queue_num != gtmv(m, 9) || cmd != gtmv(m, 2) || done != gtmv(m, 1) || error != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t q_idx = gtmv(m, 7);
        qm_q_context q_context = {.wred_profile = gtmv(m, 4), .copy_dec_profile = gtmv(m, 3), .ddr_copy_disable = gtmv(m, 1), .aggregation_disable = gtmv(m, 1), .fpm_ug_or_bufmng = gtmv(m, 5), .exclusive_priority = gtmv(m, 1), .q_802_1ae = gtmv(m, 1), .sci = gtmv(m, 1), .fec_enable = gtmv(m, 1), .res_profile = gtmv(m, 3), .spare_room_0 = gtmv(m, 2), .spare_room_1 = gtmv(m, 2), .service_queue_profile = gtmv(m, 5), .timestamp_res_profile = gtmv(m, 2)};
        bdmf_session_print(session, "ag_drv_qm_q_context_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", q_idx,
            q_context.wred_profile, q_context.copy_dec_profile, q_context.ddr_copy_disable, q_context.aggregation_disable, 
            q_context.fpm_ug_or_bufmng, q_context.exclusive_priority, q_context.q_802_1ae, q_context.sci, 
            q_context.fec_enable, q_context.res_profile, q_context.spare_room_0, q_context.spare_room_1, 
            q_context.service_queue_profile, q_context.timestamp_res_profile);
        ag_err = ag_drv_qm_q_context_set(q_idx, &q_context);
        if (!ag_err)
            ag_err = ag_drv_qm_q_context_get(q_idx, &q_context);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_q_context_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", q_idx,
                q_context.wred_profile, q_context.copy_dec_profile, q_context.ddr_copy_disable, q_context.aggregation_disable, 
                q_context.fpm_ug_or_bufmng, q_context.exclusive_priority, q_context.q_802_1ae, q_context.sci, 
                q_context.fec_enable, q_context.res_profile, q_context.spare_room_0, q_context.spare_room_1, 
                q_context.service_queue_profile, q_context.timestamp_res_profile);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (q_context.wred_profile != gtmv(m, 4) || q_context.copy_dec_profile != gtmv(m, 3) || q_context.ddr_copy_disable != gtmv(m, 1) || q_context.aggregation_disable != gtmv(m, 1) || q_context.fpm_ug_or_bufmng != gtmv(m, 5) || q_context.exclusive_priority != gtmv(m, 1) || q_context.q_802_1ae != gtmv(m, 1) || q_context.sci != gtmv(m, 1) || q_context.fec_enable != gtmv(m, 1) || q_context.res_profile != gtmv(m, 3) || q_context.spare_room_0 != gtmv(m, 2) || q_context.spare_room_1 != gtmv(m, 2) || q_context.service_queue_profile != gtmv(m, 5) || q_context.timestamp_res_profile != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_idx = gtmv(m, 3);
        uint32_t queue_occupancy_thr = gtmv(m, 30);
        bdmf_boolean psram_thr = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_qm_copy_decision_profile_set( %u %u %u)\n", profile_idx,
            queue_occupancy_thr, psram_thr);
        ag_err = ag_drv_qm_copy_decision_profile_set(profile_idx, queue_occupancy_thr, psram_thr);
        if (!ag_err)
            ag_err = ag_drv_qm_copy_decision_profile_get(profile_idx, &queue_occupancy_thr, &psram_thr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_copy_decision_profile_get( %u %u %u)\n", profile_idx,
                queue_occupancy_thr, psram_thr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (queue_occupancy_thr != gtmv(m, 30) || psram_thr != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_idx = gtmv(m, 2);
        uint8_t start = gtmv(m, 5);
        bdmf_session_print(session, "ag_drv_qm_timestamp_res_profile_set( %u %u)\n", profile_idx,
            start);
        ag_err = ag_drv_qm_timestamp_res_profile_set(profile_idx, start);
        if (!ag_err)
            ag_err = ag_drv_qm_timestamp_res_profile_get(profile_idx, &start);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_timestamp_res_profile_get( %u %u)\n", profile_idx,
                start);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (start != gtmv(m, 5))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_global_egress_drop_counter_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_egress_drop_counter_get( %u)\n",
                data);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_global_egress_aqm_drop_counter_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_global_egress_aqm_drop_counter_get( %u)\n",
                data);
        }
    }

    {
        uint16_t q_idx = gtmv(m, 9);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_qm_total_valid_cnt_set( %u %u)\n", q_idx,
            data);
        ag_err = ag_drv_qm_total_valid_cnt_set(q_idx, data);
        if (!ag_err)
            ag_err = ag_drv_qm_total_valid_cnt_get(q_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_total_valid_cnt_get( %u %u)\n", q_idx,
                data);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (data != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t q_idx = gtmv(m, 8);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_qm_dqm_valid_cnt_set( %u %u)\n", q_idx,
            data);
        ag_err = ag_drv_qm_dqm_valid_cnt_set(q_idx, data);
        if (!ag_err)
            ag_err = ag_drv_qm_dqm_valid_cnt_get(q_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_dqm_valid_cnt_get( %u %u)\n", q_idx,
                data);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (data != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t q_idx = gtmv(m, 8);
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_drop_counter_get(q_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_drop_counter_get( %u %u)\n", q_idx,
                data);
        }
    }

    {
        uint32_t q_idx = gtmv(m, 8);
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_accumulated_counter_get(q_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_accumulated_counter_get( %u %u)\n", q_idx,
                data);
        }
    }

    {
        uint16_t q_idx = gtmv(m, 8);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_qm_epon_q_byte_cnt_set( %u %u)\n", q_idx,
            data);
        ag_err = ag_drv_qm_epon_q_byte_cnt_set(q_idx, data);
        if (!ag_err)
            ag_err = ag_drv_qm_epon_q_byte_cnt_get(q_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_epon_q_byte_cnt_get( %u %u)\n", q_idx,
                data);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (data != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t q_idx = gtmv(m, 2);
        uint32_t status_bit_vector = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_epon_q_status_get(q_idx, &status_bit_vector);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_epon_q_status_get( %u %u)\n", q_idx,
                status_bit_vector);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_rd_data_pool0_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_rd_data_pool0_get( %u)\n",
                data);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_rd_data_pool1_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_rd_data_pool1_get( %u)\n",
                data);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_rd_data_pool2_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_rd_data_pool2_get( %u)\n",
                data);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_rd_data_pool3_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_rd_data_pool3_get( %u)\n",
                data);
        }
    }

    {
        bdmf_boolean pop_pool0 = gtmv(m, 1);
        bdmf_boolean pop_pool1 = gtmv(m, 1);
        bdmf_boolean pop_pool2 = gtmv(m, 1);
        bdmf_boolean pop_pool3 = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_qm_pop_3_set( %u %u %u %u)\n",
            pop_pool0, pop_pool1, pop_pool2, pop_pool3);
        ag_err = ag_drv_qm_pop_3_set(pop_pool0, pop_pool1, pop_pool2, pop_pool3);
        if (!ag_err)
            ag_err = ag_drv_qm_pop_3_get(&pop_pool0, &pop_pool1, &pop_pool2, &pop_pool3);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_pop_3_get( %u %u %u %u)\n",
                pop_pool0, pop_pool1, pop_pool2, pop_pool3);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pop_pool0 != gtmv(m, 1) || pop_pool1 != gtmv(m, 1) || pop_pool2 != gtmv(m, 1) || pop_pool3 != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t q_idx = gtmv(m, 7);
        uint8_t wr_ptr = gtmv(m, 4);
        uint8_t rd_ptr = gtmv(m, 4);
        if (!ag_err)
            ag_err = ag_drv_qm_pdfifo_ptr_get(q_idx, &wr_ptr, &rd_ptr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_pdfifo_ptr_get( %u %u %u)\n", q_idx,
                wr_ptr, rd_ptr);
        }
    }

    {
        uint16_t fifo_idx = gtmv(m, 4);
        uint16_t wr_ptr = gtmv(m, 9);
        uint8_t rd_ptr = gtmv(m, 7);
        if (!ag_err)
            ag_err = ag_drv_qm_update_fifo_ptr_get(fifo_idx, &wr_ptr, &rd_ptr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_update_fifo_ptr_get( %u %u %u)\n", fifo_idx,
                wr_ptr, rd_ptr);
        }
    }

    {
        bdmf_boolean pop = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_qm_pop_2_set( %u)\n",
            pop);
        ag_err = ag_drv_qm_pop_2_set(pop);
        if (!ag_err)
            ag_err = ag_drv_qm_pop_2_get(&pop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_pop_2_get( %u)\n",
                pop);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pop != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean pop = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_qm_pop_1_set( %u)\n",
            pop);
        ag_err = ag_drv_qm_pop_1_set(pop);
        if (!ag_err)
            ag_err = ag_drv_qm_pop_1_get(&pop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_pop_1_get( %u)\n",
                pop);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pop != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t idx = gtmv(m, 2);
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_bb0_egr_msg_out_fifo_data_get(idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_bb0_egr_msg_out_fifo_data_get( %u %u)\n", idx,
                data);
        }
    }

    {
        uint32_t idx = gtmv(m, 3);
        uint32_t data = gtmv(m, 18);
        bdmf_session_print(session, "ag_drv_qm_fpm_buffer_reservation_data_set( %u %u)\n", idx,
            data);
        ag_err = ag_drv_qm_fpm_buffer_reservation_data_set(idx, data);
        if (!ag_err)
            ag_err = ag_drv_qm_fpm_buffer_reservation_data_get(idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_buffer_reservation_data_get( %u %u)\n", idx,
                data);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (data != gtmv(m, 18))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t idx = gtmv(m, 3);
        bdmf_boolean en_byte = gtmv(m, 1);
        bdmf_boolean en_ug = gtmv(m, 1);
        uint8_t bbh_rx_bb_id = gtmv(m, 6);
        uint8_t fw_port_id = gtmv(m, 5);
        bdmf_session_print(session, "ag_drv_qm_port_cfg_set( %u %u %u %u %u)\n", idx,
            en_byte, en_ug, bbh_rx_bb_id, fw_port_id);
        ag_err = ag_drv_qm_port_cfg_set(idx, en_byte, en_ug, bbh_rx_bb_id, fw_port_id);
        if (!ag_err)
            ag_err = ag_drv_qm_port_cfg_get(idx, &en_byte, &en_ug, &bbh_rx_bb_id, &fw_port_id);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_port_cfg_get( %u %u %u %u %u)\n", idx,
                en_byte, en_ug, bbh_rx_bb_id, fw_port_id);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (en_byte != gtmv(m, 1) || en_ug != gtmv(m, 1) || bbh_rx_bb_id != gtmv(m, 6) || fw_port_id != gtmv(m, 5))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t ug_en = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_qm_fc_ug_mask_ug_en_set( %u)\n",
            ug_en);
        ag_err = ag_drv_qm_fc_ug_mask_ug_en_set(ug_en);
        if (!ag_err)
            ag_err = ag_drv_qm_fc_ug_mask_ug_en_get(&ug_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fc_ug_mask_ug_en_get( %u)\n",
                ug_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ug_en != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t idx = gtmv(m, 2);
        uint32_t queue_vec = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_qm_fc_queue_mask_set( %u %u)\n", idx,
            queue_vec);
        ag_err = ag_drv_qm_fc_queue_mask_set(idx, queue_vec);
        if (!ag_err)
            ag_err = ag_drv_qm_fc_queue_mask_get(idx, &queue_vec);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fc_queue_mask_get( %u %u)\n", idx,
                queue_vec);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (queue_vec != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t start_queue = gtmv(m, 9);
        bdmf_session_print(session, "ag_drv_qm_fc_queue_range1_start_set( %u)\n",
            start_queue);
        ag_err = ag_drv_qm_fc_queue_range1_start_set(start_queue);
        if (!ag_err)
            ag_err = ag_drv_qm_fc_queue_range1_start_get(&start_queue);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fc_queue_range1_start_get( %u)\n",
                start_queue);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (start_queue != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t start_queue = gtmv(m, 9);
        bdmf_session_print(session, "ag_drv_qm_fc_queue_range2_start_set( %u)\n",
            start_queue);
        ag_err = ag_drv_qm_fc_queue_range2_start_set(start_queue);
        if (!ag_err)
            ag_err = ag_drv_qm_fc_queue_range2_start_get(&start_queue);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fc_queue_range2_start_get( %u)\n",
                start_queue);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (start_queue != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t status = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_dbg_get(&status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_dbg_get( %u)\n",
                status);
        }
    }

    {
        uint32_t q_idx = gtmv(m, 3);
        uint32_t status = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_ug_occupancy_status_get(q_idx, &status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_ug_occupancy_status_get( %u %u)\n", q_idx,
                status);
        }
    }

    {
        uint8_t idx = gtmv(m, 5);
        uint32_t status = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_queue_range1_occupancy_status_get(idx, &status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_queue_range1_occupancy_status_get( %u %u)\n", idx,
                status);
        }
    }

    {
        uint8_t idx = gtmv(m, 5);
        uint32_t status = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_queue_range2_occupancy_status_get(idx, &status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_queue_range2_occupancy_status_get( %u %u)\n", idx,
                status);
        }
    }

    {
        uint8_t select = gtmv(m, 5);
        bdmf_boolean enable = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_qm_debug_sel_set( %u %u)\n",
            select, enable);
        ag_err = ag_drv_qm_debug_sel_set(select, enable);
        if (!ag_err)
            ag_err = ag_drv_qm_debug_sel_get(&select, &enable);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_debug_sel_get( %u %u)\n",
                select, enable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (select != gtmv(m, 5) || enable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_debug_bus_lsb_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_debug_bus_lsb_get( %u)\n",
                data);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_debug_bus_msb_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_debug_bus_msb_get( %u)\n",
                data);
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_qm_spare_config_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_qm_spare_config_get( %u)\n",
                data);
        }
    }

    {
        uint32_t good_lvl1_pkts = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_good_lvl1_pkts_cnt_get(&good_lvl1_pkts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_good_lvl1_pkts_cnt_get( %u)\n",
                good_lvl1_pkts);
        }
    }

    {
        uint32_t good_lvl1_bytes = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_good_lvl1_bytes_cnt_get(&good_lvl1_bytes);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_good_lvl1_bytes_cnt_get( %u)\n",
                good_lvl1_bytes);
        }
    }

    {
        uint32_t good_lvl2_pkts = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_good_lvl2_pkts_cnt_get(&good_lvl2_pkts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_good_lvl2_pkts_cnt_get( %u)\n",
                good_lvl2_pkts);
        }
    }

    {
        uint32_t good_lvl2_bytes = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_good_lvl2_bytes_cnt_get(&good_lvl2_bytes);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_good_lvl2_bytes_cnt_get( %u)\n",
                good_lvl2_bytes);
        }
    }

    {
        uint32_t copied_pkts = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_copied_pkts_cnt_get(&copied_pkts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_copied_pkts_cnt_get( %u)\n",
                copied_pkts);
        }
    }

    {
        uint32_t copied_bytes = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_copied_bytes_cnt_get(&copied_bytes);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_copied_bytes_cnt_get( %u)\n",
                copied_bytes);
        }
    }

    {
        uint32_t agg_pkts = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_agg_pkts_cnt_get(&agg_pkts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_agg_pkts_cnt_get( %u)\n",
                agg_pkts);
        }
    }

    {
        uint32_t agg_bytes = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_agg_bytes_cnt_get(&agg_bytes);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_agg_bytes_cnt_get( %u)\n",
                agg_bytes);
        }
    }

    {
        uint32_t agg1_pkts = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_agg_1_pkts_cnt_get(&agg1_pkts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_agg_1_pkts_cnt_get( %u)\n",
                agg1_pkts);
        }
    }

    {
        uint32_t agg2_pkts = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_agg_2_pkts_cnt_get(&agg2_pkts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_agg_2_pkts_cnt_get( %u)\n",
                agg2_pkts);
        }
    }

    {
        uint32_t agg3_pkts = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_agg_3_pkts_cnt_get(&agg3_pkts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_agg_3_pkts_cnt_get( %u)\n",
                agg3_pkts);
        }
    }

    {
        uint32_t agg4_pkts = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_agg_4_pkts_cnt_get(&agg4_pkts);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_agg_4_pkts_cnt_get( %u)\n",
                agg4_pkts);
        }
    }

    {
        uint32_t wred_drop = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_wred_drop_cnt_get(&wred_drop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_wred_drop_cnt_get( %u)\n",
                wred_drop);
        }
    }

    {
        uint32_t fpm_cong = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_fpm_congestion_drop_cnt_get(&fpm_cong);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_congestion_drop_cnt_get( %u)\n",
                fpm_cong);
        }
    }

    {
        uint32_t ddr_pd_cong_drop = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_ddr_pd_congestion_drop_cnt_get(&ddr_pd_cong_drop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_ddr_pd_congestion_drop_cnt_get( %u)\n",
                ddr_pd_cong_drop);
        }
    }

    {
        uint32_t ddr_cong_byte_drop = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_ddr_byte_congestion_drop_cnt_get(&ddr_cong_byte_drop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_ddr_byte_congestion_drop_cnt_get( %u)\n",
                ddr_cong_byte_drop);
        }
    }

    {
        uint32_t counter = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_qm_pd_congestion_drop_cnt_get(&counter);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_qm_pd_congestion_drop_cnt_get( %u)\n",
                counter);
        }
    }

    {
        uint32_t abs_requeue = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_qm_abs_requeue_cnt_get(&abs_requeue);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_qm_abs_requeue_cnt_get( %u)\n",
                abs_requeue);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_fpm_prefetch_fifo0_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_prefetch_fifo0_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_fpm_prefetch_fifo1_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_prefetch_fifo1_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_fpm_prefetch_fifo2_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_prefetch_fifo2_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_fpm_prefetch_fifo3_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_prefetch_fifo3_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_normal_rmt_fifo_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_normal_rmt_fifo_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_non_delayed_rmt_fifo_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_non_delayed_rmt_fifo_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_non_delayed_out_fifo_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_non_delayed_out_fifo_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_pre_cm_fifo_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_pre_cm_fifo_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_cm_rd_pd_fifo_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_cm_rd_pd_fifo_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_cm_wr_pd_fifo_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_cm_wr_pd_fifo_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_cm_common_input_fifo_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_cm_common_input_fifo_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_bb0_output_fifo_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_bb0_output_fifo_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_bb1_output_fifo_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_bb1_output_fifo_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_bb1_input_fifo_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_bb1_input_fifo_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_egress_data_fifo_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_egress_data_fifo_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_egress_rr_fifo_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_egress_rr_fifo_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        uint8_t idx = gtmv(m, 1);
        bdmf_boolean ovr_en = gtmv(m, 1);
        uint8_t dest_id = gtmv(m, 6);
        uint16_t route_addr = gtmv(m, 10);
        bdmf_session_print(session, "ag_drv_qm_bb_route_ovr_set( %u %u %u %u)\n", idx,
            ovr_en, dest_id, route_addr);
        ag_err = ag_drv_qm_bb_route_ovr_set(idx, ovr_en, dest_id, route_addr);
        if (!ag_err)
            ag_err = ag_drv_qm_bb_route_ovr_get(idx, &ovr_en, &dest_id, &route_addr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_bb_route_ovr_get( %u %u %u %u)\n", idx,
                ovr_en, dest_id, route_addr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ovr_en != gtmv(m, 1) || dest_id != gtmv(m, 6) || route_addr != gtmv(m, 10))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t ingress_stat = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_ingress_stat_get(&ingress_stat);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_ingress_stat_get( %u)\n",
                ingress_stat);
        }
    }

    {
        uint32_t egress_stat = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_egress_stat_get(&egress_stat);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_egress_stat_get( %u)\n",
                egress_stat);
        }
    }

    {
        uint32_t cm_stat = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_cm_stat_get(&cm_stat);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_cm_stat_get( %u)\n",
                cm_stat);
        }
    }

    {
        uint32_t fpm_prefetch_stat = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_fpm_prefetch_stat_get(&fpm_prefetch_stat);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_prefetch_stat_get( %u)\n",
                fpm_prefetch_stat);
        }
    }

    {
        uint8_t connect_ack_counter = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_qm_qm_connect_ack_counter_get(&connect_ack_counter);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_qm_connect_ack_counter_get( %u)\n",
                connect_ack_counter);
        }
    }

    {
        uint8_t ddr_wr_reply_counter = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_qm_qm_ddr_wr_reply_counter_get(&ddr_wr_reply_counter);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_qm_ddr_wr_reply_counter_get( %u)\n",
                ddr_wr_reply_counter);
        }
    }

    {
        uint32_t ddr_pipe = gtmv(m, 28);
        if (!ag_err)
            ag_err = ag_drv_qm_qm_ddr_pipe_byte_counter_get(&ddr_pipe);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_qm_ddr_pipe_byte_counter_get( %u)\n",
                ddr_pipe);
        }
    }

    {
        uint16_t requeue_valid = gtmv(m, 15);
        if (!ag_err)
            ag_err = ag_drv_qm_qm_abs_requeue_valid_counter_get(&requeue_valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_qm_abs_requeue_valid_counter_get( %u)\n",
                requeue_valid);
        }
    }

    {
        uint32_t idx = gtmv(m, 2);
        uint32_t pd = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_qm_illegal_pd_capture_get(idx, &pd);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_qm_illegal_pd_capture_get( %u %u)\n", idx,
                pd);
        }
    }

    {
        uint32_t idx = gtmv(m, 2);
        uint32_t pd = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_qm_ingress_processed_pd_capture_get(idx, &pd);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_qm_ingress_processed_pd_capture_get( %u %u)\n", idx,
                pd);
        }
    }

    {
        uint32_t idx = gtmv(m, 2);
        uint32_t pd = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_qm_cm_processed_pd_capture_get(idx, &pd);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_qm_cm_processed_pd_capture_get( %u %u)\n", idx,
                pd);
        }
    }

    {
        uint32_t idx = gtmv(m, 5);
        uint32_t fpm_grp_drop = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_fpm_grp_drop_cnt_get(idx, &fpm_grp_drop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_grp_drop_cnt_get( %u %u)\n", idx,
                fpm_grp_drop);
        }
    }

    {
        uint32_t idx = gtmv(m, 2);
        uint32_t fpm_drop = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_fpm_pool_drop_cnt_get(idx, &fpm_drop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_pool_drop_cnt_get( %u %u)\n", idx,
                fpm_drop);
        }
    }

    {
        uint32_t counter = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_fpm_buffer_res_drop_cnt_get(&counter);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_buffer_res_drop_cnt_get( %u)\n",
                counter);
        }
    }

    {
        uint32_t counter = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_psram_egress_cong_drp_cnt_get(&counter);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_psram_egress_cong_drp_cnt_get( %u)\n",
                counter);
        }
    }

    {
        uint8_t status = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_qm_backpressure_set( %u)\n",
            status);
        ag_err = ag_drv_qm_backpressure_set(status);
        if (!ag_err)
            ag_err = ag_drv_qm_backpressure_get(&status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_backpressure_get( %u)\n",
                status);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (status != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t value = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_aqm_timestamp_curr_counter_get(&value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_aqm_timestamp_curr_counter_get( %u)\n",
                value);
        }
    }

    {
        uint16_t used_words = gtmv(m, 16);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_qm_bb0_egr_msg_out_fifo_status_get(&used_words, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_bb0_egr_msg_out_fifo_status_get( %u %u %u)\n",
                used_words, empty, full);
        }
    }

    {
        qm_count_pkt_not_pd_mode_bits count_pkt_not_pd_mode_bits = {.total_egress_accum_cnt_pkt = gtmv(m, 1), .global_egress_drop_cnt_pkt = gtmv(m, 1), .drop_ing_egr_cnt_pkt = gtmv(m, 1), .fpm_grp_drop_cnt_pkt = gtmv(m, 1), .qm_pd_congestion_drop_cnt_pkt = gtmv(m, 1), .ddr_pd_congestion_drop_cnt_pkt = gtmv(m, 1), .wred_drop_cnt_pkt = gtmv(m, 1), .good_lvl2_pkts_cnt_pkt = gtmv(m, 1), .good_lvl1_pkts_cnt_pkt = gtmv(m, 1), .qm_total_valid_cnt_pkt = gtmv(m, 1), .dqm_valid_cnt_pkt = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_qm_count_pkt_not_pd_mode_bits_set( %u %u %u %u %u %u %u %u %u %u %u)\n",
            count_pkt_not_pd_mode_bits.total_egress_accum_cnt_pkt, count_pkt_not_pd_mode_bits.global_egress_drop_cnt_pkt, count_pkt_not_pd_mode_bits.drop_ing_egr_cnt_pkt, count_pkt_not_pd_mode_bits.fpm_grp_drop_cnt_pkt, 
            count_pkt_not_pd_mode_bits.qm_pd_congestion_drop_cnt_pkt, count_pkt_not_pd_mode_bits.ddr_pd_congestion_drop_cnt_pkt, count_pkt_not_pd_mode_bits.wred_drop_cnt_pkt, count_pkt_not_pd_mode_bits.good_lvl2_pkts_cnt_pkt, 
            count_pkt_not_pd_mode_bits.good_lvl1_pkts_cnt_pkt, count_pkt_not_pd_mode_bits.qm_total_valid_cnt_pkt, count_pkt_not_pd_mode_bits.dqm_valid_cnt_pkt);
        ag_err = ag_drv_qm_count_pkt_not_pd_mode_bits_set(&count_pkt_not_pd_mode_bits);
        if (!ag_err)
            ag_err = ag_drv_qm_count_pkt_not_pd_mode_bits_get(&count_pkt_not_pd_mode_bits);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_count_pkt_not_pd_mode_bits_get( %u %u %u %u %u %u %u %u %u %u %u)\n",
                count_pkt_not_pd_mode_bits.total_egress_accum_cnt_pkt, count_pkt_not_pd_mode_bits.global_egress_drop_cnt_pkt, count_pkt_not_pd_mode_bits.drop_ing_egr_cnt_pkt, count_pkt_not_pd_mode_bits.fpm_grp_drop_cnt_pkt, 
                count_pkt_not_pd_mode_bits.qm_pd_congestion_drop_cnt_pkt, count_pkt_not_pd_mode_bits.ddr_pd_congestion_drop_cnt_pkt, count_pkt_not_pd_mode_bits.wred_drop_cnt_pkt, count_pkt_not_pd_mode_bits.good_lvl2_pkts_cnt_pkt, 
                count_pkt_not_pd_mode_bits.good_lvl1_pkts_cnt_pkt, count_pkt_not_pd_mode_bits.qm_total_valid_cnt_pkt, count_pkt_not_pd_mode_bits.dqm_valid_cnt_pkt);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_pkt_not_pd_mode_bits.total_egress_accum_cnt_pkt != gtmv(m, 1) || count_pkt_not_pd_mode_bits.global_egress_drop_cnt_pkt != gtmv(m, 1) || count_pkt_not_pd_mode_bits.drop_ing_egr_cnt_pkt != gtmv(m, 1) || count_pkt_not_pd_mode_bits.fpm_grp_drop_cnt_pkt != gtmv(m, 1) || count_pkt_not_pd_mode_bits.qm_pd_congestion_drop_cnt_pkt != gtmv(m, 1) || count_pkt_not_pd_mode_bits.ddr_pd_congestion_drop_cnt_pkt != gtmv(m, 1) || count_pkt_not_pd_mode_bits.wred_drop_cnt_pkt != gtmv(m, 1) || count_pkt_not_pd_mode_bits.good_lvl2_pkts_cnt_pkt != gtmv(m, 1) || count_pkt_not_pd_mode_bits.good_lvl1_pkts_cnt_pkt != gtmv(m, 1) || count_pkt_not_pd_mode_bits.qm_total_valid_cnt_pkt != gtmv(m, 1) || count_pkt_not_pd_mode_bits.dqm_valid_cnt_pkt != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t idx = gtmv(m, 10);
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_qm_cm_residue_data_get(idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_qm_cm_residue_data_get( %u %u)\n", idx,
                data);
        }
    }

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_qm_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m, input_method = parm[0].value.unumber;
    bdmfmon_cmd_parm_t *p_start, *p_stop;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t ext_test_success_cnt = 0;
    uint32_t ext_test_failure_cnt = 0;
    uint32_t start_idx = 0;
    uint32_t stop_idx = 0;

    p_start = bdmfmon_cmd_find(session, "start_idx");
    p_stop = bdmfmon_cmd_find(session, "stop_idx");

    if (p_start)
        start_idx = p_start->value.unumber;
    if (p_stop)
        stop_idx = p_stop->value.unumber;

    if ((start_idx > stop_idx) && (stop_idx != 0))
    {
        bdmf_session_print(session, "ERROR: start_idx must be less than stop_idx\n");
        return BDMF_ERR_PARM;
    }

    m = bdmf_test_method_high; /* "Initialization" method */
    switch (parm[1].value.unumber)
    {
    case cli_qm_fpm_ug_thr:
    {
        uint8_t max_ug_grp_idx = 4;
        uint8_t ug_grp_idx = gtmv(m, 2);
        qm_fpm_ug_thr fpm_ug_thr = {
            .lower_thr = gtmv(m, 18), 
            .mid_thr = gtmv(m, 18), 
            .higher_thr = gtmv(m, 18) };

        if ((start_idx >= max_ug_grp_idx) || (stop_idx >= max_ug_grp_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_ug_grp_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (ug_grp_idx = 0; ug_grp_idx < max_ug_grp_idx; ug_grp_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_ug_thr_set( %u %u %u %u)\n", ug_grp_idx,
                fpm_ug_thr.lower_thr, fpm_ug_thr.mid_thr, fpm_ug_thr.higher_thr);
            ag_err = ag_drv_qm_fpm_ug_thr_set(ug_grp_idx, &fpm_ug_thr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", ug_grp_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        fpm_ug_thr.lower_thr = gtmv(m, 18);
        fpm_ug_thr.mid_thr = gtmv(m, 18);
        fpm_ug_thr.higher_thr = gtmv(m, 18);

        for (ug_grp_idx = start_idx; ug_grp_idx <= stop_idx; ug_grp_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_ug_thr_set( %u %u %u %u)\n", ug_grp_idx,
                fpm_ug_thr.lower_thr, fpm_ug_thr.mid_thr, fpm_ug_thr.higher_thr);
            ag_err = ag_drv_qm_fpm_ug_thr_set(ug_grp_idx, &fpm_ug_thr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", ug_grp_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (ug_grp_idx = 0; ug_grp_idx < max_ug_grp_idx; ug_grp_idx++)
        {
            if (ug_grp_idx < start_idx || ug_grp_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_fpm_ug_thr_get(ug_grp_idx, &fpm_ug_thr);

            bdmf_session_print(session, "ag_drv_qm_fpm_ug_thr_get( %u %u %u %u)\n", ug_grp_idx,
                fpm_ug_thr.lower_thr, fpm_ug_thr.mid_thr, fpm_ug_thr.higher_thr);

            if (fpm_ug_thr.lower_thr != gtmv(m, 18) || 
                fpm_ug_thr.mid_thr != gtmv(m, 18) || 
                fpm_ug_thr.higher_thr != gtmv(m, 18) || 
                0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", ug_grp_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of fpm_ug_thr completed. Number of tested entries %u.\n", max_ug_grp_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_rnr_group_cfg:
    {
        uint8_t max_rnr_idx = 16;
        uint8_t rnr_idx = gtmv(m, 4);
        qm_rnr_group_cfg rnr_group_cfg = {
            .start_queue = gtmv(m, 9), 
            .end_queue = gtmv(m, 9), 
            .pd_fifo_base = gtmv(m, 11), 
            .pd_fifo_size = gtmv(m, 2), 
            .upd_fifo_base = gtmv(m, 11), 
            .upd_fifo_size = gtmv(m, 3), 
            .rnr_bb_id = gtmv(m, 6), 
            .rnr_task = gtmv(m, 4), 
            .rnr_enable = gtmv(m, 1) };

        if ((start_idx >= max_rnr_idx) || (stop_idx >= max_rnr_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_rnr_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (rnr_idx = 0; rnr_idx < max_rnr_idx; rnr_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_rnr_group_cfg_set( %u %u %u %u %u %u %u %u %u %u)\n", rnr_idx,
                rnr_group_cfg.start_queue, rnr_group_cfg.end_queue, rnr_group_cfg.pd_fifo_base, rnr_group_cfg.pd_fifo_size, 
                rnr_group_cfg.upd_fifo_base, rnr_group_cfg.upd_fifo_size, rnr_group_cfg.rnr_bb_id, rnr_group_cfg.rnr_task, 
                rnr_group_cfg.rnr_enable);
            ag_err = ag_drv_qm_rnr_group_cfg_set(rnr_idx, &rnr_group_cfg);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", rnr_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        rnr_group_cfg.start_queue = gtmv(m, 9);
        rnr_group_cfg.end_queue = gtmv(m, 9);
        rnr_group_cfg.pd_fifo_base = gtmv(m, 11);
        rnr_group_cfg.pd_fifo_size = gtmv(m, 2);
        rnr_group_cfg.upd_fifo_base = gtmv(m, 11);
        rnr_group_cfg.upd_fifo_size = gtmv(m, 3);
        rnr_group_cfg.rnr_bb_id = gtmv(m, 6);
        rnr_group_cfg.rnr_task = gtmv(m, 4);
        rnr_group_cfg.rnr_enable = gtmv(m, 1);

        for (rnr_idx = start_idx; rnr_idx <= stop_idx; rnr_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_rnr_group_cfg_set( %u %u %u %u %u %u %u %u %u %u)\n", rnr_idx,
                rnr_group_cfg.start_queue, rnr_group_cfg.end_queue, rnr_group_cfg.pd_fifo_base, rnr_group_cfg.pd_fifo_size, 
                rnr_group_cfg.upd_fifo_base, rnr_group_cfg.upd_fifo_size, rnr_group_cfg.rnr_bb_id, rnr_group_cfg.rnr_task, 
                rnr_group_cfg.rnr_enable);
            ag_err = ag_drv_qm_rnr_group_cfg_set(rnr_idx, &rnr_group_cfg);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", rnr_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (rnr_idx = 0; rnr_idx < max_rnr_idx; rnr_idx++)
        {
            if (rnr_idx < start_idx || rnr_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_rnr_group_cfg_get(rnr_idx, &rnr_group_cfg);

            bdmf_session_print(session, "ag_drv_qm_rnr_group_cfg_get( %u %u %u %u %u %u %u %u %u %u)\n", rnr_idx,
                rnr_group_cfg.start_queue, rnr_group_cfg.end_queue, rnr_group_cfg.pd_fifo_base, rnr_group_cfg.pd_fifo_size, 
                rnr_group_cfg.upd_fifo_base, rnr_group_cfg.upd_fifo_size, rnr_group_cfg.rnr_bb_id, rnr_group_cfg.rnr_task, 
                rnr_group_cfg.rnr_enable);

            if (rnr_group_cfg.start_queue != gtmv(m, 9) || 
                rnr_group_cfg.end_queue != gtmv(m, 9) || 
                rnr_group_cfg.pd_fifo_base != gtmv(m, 11) || 
                rnr_group_cfg.pd_fifo_size != gtmv(m, 2) || 
                rnr_group_cfg.upd_fifo_base != gtmv(m, 11) || 
                rnr_group_cfg.upd_fifo_size != gtmv(m, 3) || 
                rnr_group_cfg.rnr_bb_id != gtmv(m, 6) || 
                rnr_group_cfg.rnr_task != gtmv(m, 4) || 
                rnr_group_cfg.rnr_enable != gtmv(m, 1) || 
                0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", rnr_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of rnr_group_cfg completed. Number of tested entries %u.\n", max_rnr_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_cpu_pd_indirect_wr_data:
    {
        uint8_t max_indirect_grp_idx = 4;
        uint8_t indirect_grp_idx = gtmv(m, 2);
        uint32_t data0 = gtmv(m, 32);
        uint32_t data1 = gtmv(m, 32);
        uint32_t data2 = gtmv(m, 32);
        uint32_t data3 = gtmv(m, 32);

        if ((start_idx >= max_indirect_grp_idx) || (stop_idx >= max_indirect_grp_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_indirect_grp_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (indirect_grp_idx = 0; indirect_grp_idx < max_indirect_grp_idx; indirect_grp_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_cpu_pd_indirect_wr_data_set( %u %u %u %u %u)\n", indirect_grp_idx,
                data0, data1, data2, data3);
            ag_err = ag_drv_qm_cpu_pd_indirect_wr_data_set(indirect_grp_idx, data0, data1, data2, data3);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", indirect_grp_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data0 = gtmv(m, 32);
        data1 = gtmv(m, 32);
        data2 = gtmv(m, 32);
        data3 = gtmv(m, 32);

        for (indirect_grp_idx = start_idx; indirect_grp_idx <= stop_idx; indirect_grp_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_cpu_pd_indirect_wr_data_set( %u %u %u %u %u)\n", indirect_grp_idx,
                data0, data1, data2, data3);
            ag_err = ag_drv_qm_cpu_pd_indirect_wr_data_set(indirect_grp_idx, data0, data1, data2, data3);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", indirect_grp_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (indirect_grp_idx = 0; indirect_grp_idx < max_indirect_grp_idx; indirect_grp_idx++)
        {
            if (indirect_grp_idx < start_idx || indirect_grp_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_cpu_pd_indirect_wr_data_get(indirect_grp_idx, &data0, &data1, &data2, &data3);

            bdmf_session_print(session, "ag_drv_qm_cpu_pd_indirect_wr_data_get( %u %u %u %u %u)\n", indirect_grp_idx,
                data0, data1, data2, data3);

            if (data0 != gtmv(m, 32) || data1 != gtmv(m, 32) || data2 != gtmv(m, 32) || data3 != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", indirect_grp_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of cpu_pd_indirect_wr_data completed. Number of tested entries %u.\n", max_indirect_grp_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_wred_profile_cfg:
    {
        uint8_t max_profile_idx = 16;
        uint8_t profile_idx = gtmv(m, 4);
        qm_wred_profile_cfg wred_profile_cfg = {
            .min_thr0 = gtmv(m, 24), 
            .flw_ctrl_en0 = gtmv(m, 1), 
            .min_thr1 = gtmv(m, 24), 
            .flw_ctrl_en1 = gtmv(m, 1), 
            .max_thr0 = gtmv(m, 24), 
            .max_thr1 = gtmv(m, 24), 
            .slope_mantissa0 = gtmv(m, 8), 
            .slope_exp0 = gtmv(m, 5), 
            .slope_mantissa1 = gtmv(m, 8), 
            .slope_exp1 = gtmv(m, 5) };

        if ((start_idx >= max_profile_idx) || (stop_idx >= max_profile_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_idx = 0; profile_idx < max_profile_idx; profile_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_wred_profile_cfg_set( %u %u %u %u %u %u %u %u %u %u %u)\n", profile_idx,
                wred_profile_cfg.min_thr0, wred_profile_cfg.flw_ctrl_en0, wred_profile_cfg.min_thr1, wred_profile_cfg.flw_ctrl_en1, 
                wred_profile_cfg.max_thr0, wred_profile_cfg.max_thr1, wred_profile_cfg.slope_mantissa0, wred_profile_cfg.slope_exp0, 
                wred_profile_cfg.slope_mantissa1, wred_profile_cfg.slope_exp1);
            ag_err = ag_drv_qm_wred_profile_cfg_set(profile_idx, &wred_profile_cfg);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        wred_profile_cfg.min_thr0 = gtmv(m, 24);
        wred_profile_cfg.flw_ctrl_en0 = gtmv(m, 1);
        wred_profile_cfg.min_thr1 = gtmv(m, 24);
        wred_profile_cfg.flw_ctrl_en1 = gtmv(m, 1);
        wred_profile_cfg.max_thr0 = gtmv(m, 24);
        wred_profile_cfg.max_thr1 = gtmv(m, 24);
        wred_profile_cfg.slope_mantissa0 = gtmv(m, 8);
        wred_profile_cfg.slope_exp0 = gtmv(m, 5);
        wred_profile_cfg.slope_mantissa1 = gtmv(m, 8);
        wred_profile_cfg.slope_exp1 = gtmv(m, 5);

        for (profile_idx = start_idx; profile_idx <= stop_idx; profile_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_wred_profile_cfg_set( %u %u %u %u %u %u %u %u %u %u %u)\n", profile_idx,
                wred_profile_cfg.min_thr0, wred_profile_cfg.flw_ctrl_en0, wred_profile_cfg.min_thr1, wred_profile_cfg.flw_ctrl_en1, 
                wred_profile_cfg.max_thr0, wred_profile_cfg.max_thr1, wred_profile_cfg.slope_mantissa0, wred_profile_cfg.slope_exp0, 
                wred_profile_cfg.slope_mantissa1, wred_profile_cfg.slope_exp1);
            ag_err = ag_drv_qm_wred_profile_cfg_set(profile_idx, &wred_profile_cfg);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_idx = 0; profile_idx < max_profile_idx; profile_idx++)
        {
            if (profile_idx < start_idx || profile_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_wred_profile_cfg_get(profile_idx, &wred_profile_cfg);

            bdmf_session_print(session, "ag_drv_qm_wred_profile_cfg_get( %u %u %u %u %u %u %u %u %u %u %u)\n", profile_idx,
                wred_profile_cfg.min_thr0, wred_profile_cfg.flw_ctrl_en0, wred_profile_cfg.min_thr1, wred_profile_cfg.flw_ctrl_en1, 
                wred_profile_cfg.max_thr0, wred_profile_cfg.max_thr1, wred_profile_cfg.slope_mantissa0, wred_profile_cfg.slope_exp0, 
                wred_profile_cfg.slope_mantissa1, wred_profile_cfg.slope_exp1);

            if (wred_profile_cfg.min_thr0 != gtmv(m, 24) || 
                wred_profile_cfg.flw_ctrl_en0 != gtmv(m, 1) || 
                wred_profile_cfg.min_thr1 != gtmv(m, 24) || 
                wred_profile_cfg.flw_ctrl_en1 != gtmv(m, 1) || 
                wred_profile_cfg.max_thr0 != gtmv(m, 24) || 
                wred_profile_cfg.max_thr1 != gtmv(m, 24) || 
                wred_profile_cfg.slope_mantissa0 != gtmv(m, 8) || 
                wred_profile_cfg.slope_exp0 != gtmv(m, 5) || 
                wred_profile_cfg.slope_mantissa1 != gtmv(m, 8) || 
                wred_profile_cfg.slope_exp1 != gtmv(m, 5) || 
                0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of wred_profile_cfg completed. Number of tested entries %u.\n", max_profile_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_qm_ddr_spare_room:
    {
        uint16_t max_pair_idx = 4;
        uint16_t pair_idx = gtmv(m, 2);
        uint16_t ddr_headroom = gtmv(m, 10);
        uint16_t ddr_tailroom = gtmv(m, 11);

        if ((start_idx >= max_pair_idx) || (stop_idx >= max_pair_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_pair_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (pair_idx = 0; pair_idx < max_pair_idx; pair_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_qm_ddr_spare_room_set( %u %u %u)\n", pair_idx,
                ddr_headroom, ddr_tailroom);
            ag_err = ag_drv_qm_qm_ddr_spare_room_set(pair_idx, ddr_headroom, ddr_tailroom);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", pair_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ddr_headroom = gtmv(m, 10);
        ddr_tailroom = gtmv(m, 11);

        for (pair_idx = start_idx; pair_idx <= stop_idx; pair_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_qm_ddr_spare_room_set( %u %u %u)\n", pair_idx,
                ddr_headroom, ddr_tailroom);
            ag_err = ag_drv_qm_qm_ddr_spare_room_set(pair_idx, ddr_headroom, ddr_tailroom);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", pair_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (pair_idx = 0; pair_idx < max_pair_idx; pair_idx++)
        {
            if (pair_idx < start_idx || pair_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_qm_ddr_spare_room_get(pair_idx, &ddr_headroom, &ddr_tailroom);

            bdmf_session_print(session, "ag_drv_qm_qm_ddr_spare_room_get( %u %u %u)\n", pair_idx,
                ddr_headroom, ddr_tailroom);

            if (ddr_headroom != gtmv(m, 10) || ddr_tailroom != gtmv(m, 11) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", pair_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of qm_ddr_spare_room completed. Number of tested entries %u.\n", max_pair_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_fpm_pool_thr:
    {
        uint8_t max_pool_idx = 4;
        uint8_t pool_idx = gtmv(m, 2);
        qm_fpm_pool_thr fpm_pool_thr = {
            .lower_thr = gtmv(m, 7), 
            .higher_thr = gtmv(m, 7) };

        if ((start_idx >= max_pool_idx) || (stop_idx >= max_pool_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_pool_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (pool_idx = 0; pool_idx < max_pool_idx; pool_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_pool_thr_set( %u %u %u)\n", pool_idx,
                fpm_pool_thr.lower_thr, fpm_pool_thr.higher_thr);
            ag_err = ag_drv_qm_fpm_pool_thr_set(pool_idx, &fpm_pool_thr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", pool_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        fpm_pool_thr.lower_thr = gtmv(m, 7);
        fpm_pool_thr.higher_thr = gtmv(m, 7);

        for (pool_idx = start_idx; pool_idx <= stop_idx; pool_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_pool_thr_set( %u %u %u)\n", pool_idx,
                fpm_pool_thr.lower_thr, fpm_pool_thr.higher_thr);
            ag_err = ag_drv_qm_fpm_pool_thr_set(pool_idx, &fpm_pool_thr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", pool_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (pool_idx = 0; pool_idx < max_pool_idx; pool_idx++)
        {
            if (pool_idx < start_idx || pool_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_fpm_pool_thr_get(pool_idx, &fpm_pool_thr);

            bdmf_session_print(session, "ag_drv_qm_fpm_pool_thr_get( %u %u %u)\n", pool_idx,
                fpm_pool_thr.lower_thr, fpm_pool_thr.higher_thr);

            if (fpm_pool_thr.lower_thr != gtmv(m, 7) || 
                fpm_pool_thr.higher_thr != gtmv(m, 7) || 
                0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", pool_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of fpm_pool_thr completed. Number of tested entries %u.\n", max_pool_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_fpm_ug_cnt:
    {
        uint8_t max_grp_idx = 4;
        uint8_t grp_idx = gtmv(m, 2);
        uint32_t fpm_ug_cnt = gtmv(m, 18);

        if ((start_idx >= max_grp_idx) || (stop_idx >= max_grp_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_grp_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (grp_idx = 0; grp_idx < max_grp_idx; grp_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_ug_cnt_set( %u %u)\n", grp_idx,
                fpm_ug_cnt);
            ag_err = ag_drv_qm_fpm_ug_cnt_set(grp_idx, fpm_ug_cnt);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", grp_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        fpm_ug_cnt = gtmv(m, 18);

        for (grp_idx = start_idx; grp_idx <= stop_idx; grp_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_ug_cnt_set( %u %u)\n", grp_idx,
                fpm_ug_cnt);
            ag_err = ag_drv_qm_fpm_ug_cnt_set(grp_idx, fpm_ug_cnt);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", grp_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (grp_idx = 0; grp_idx < max_grp_idx; grp_idx++)
        {
            if (grp_idx < start_idx || grp_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_fpm_ug_cnt_get(grp_idx, &fpm_ug_cnt);

            bdmf_session_print(session, "ag_drv_qm_fpm_ug_cnt_get( %u %u)\n", grp_idx,
                fpm_ug_cnt);

            if (fpm_ug_cnt != gtmv(m, 18) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", grp_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of fpm_ug_cnt completed. Number of tested entries %u.\n", max_grp_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_cpu_indr_port_cpu_pd_indirect_ctrl:
    {
        uint8_t max_indirect_grp_idx = 4;
        uint8_t indirect_grp_idx = gtmv(m, 2);
        uint16_t queue_num = gtmv(m, 9);
        uint8_t cmd = gtmv(m, 2);
        bdmf_boolean done = gtmv(m, 1);
        bdmf_boolean error = gtmv(m, 1);

        if ((start_idx >= max_indirect_grp_idx) || (stop_idx >= max_indirect_grp_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_indirect_grp_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (indirect_grp_idx = 0; indirect_grp_idx < max_indirect_grp_idx; indirect_grp_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set( %u %u %u %u %u)\n", indirect_grp_idx,
                queue_num, cmd, done, error);
            ag_err = ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set(indirect_grp_idx, queue_num, cmd, done, error);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", indirect_grp_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        queue_num = gtmv(m, 9);
        cmd = gtmv(m, 2);
        done = gtmv(m, 1);
        error = gtmv(m, 1);

        for (indirect_grp_idx = start_idx; indirect_grp_idx <= stop_idx; indirect_grp_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set( %u %u %u %u %u)\n", indirect_grp_idx,
                queue_num, cmd, done, error);
            ag_err = ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set(indirect_grp_idx, queue_num, cmd, done, error);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", indirect_grp_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (indirect_grp_idx = 0; indirect_grp_idx < max_indirect_grp_idx; indirect_grp_idx++)
        {
            if (indirect_grp_idx < start_idx || indirect_grp_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_get(indirect_grp_idx, &queue_num, &cmd, &done, &error);

            bdmf_session_print(session, "ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_get( %u %u %u %u %u)\n", indirect_grp_idx,
                queue_num, cmd, done, error);

            if (queue_num != gtmv(m, 9) || cmd != gtmv(m, 2) || done != gtmv(m, 1) || error != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", indirect_grp_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of cpu_indr_port_cpu_pd_indirect_ctrl completed. Number of tested entries %u.\n", max_indirect_grp_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_q_context:
    {
        uint16_t max_q_idx = 160;
        uint16_t q_idx = gtmv(m, 7);
        qm_q_context q_context = {
            .wred_profile = gtmv(m, 4), 
            .copy_dec_profile = gtmv(m, 3), 
            .ddr_copy_disable = gtmv(m, 1), 
            .aggregation_disable = gtmv(m, 1), 
            .fpm_ug_or_bufmng = gtmv(m, 5), 
            .exclusive_priority = gtmv(m, 1), 
            .q_802_1ae = gtmv(m, 1), 
            .sci = gtmv(m, 1), 
            .fec_enable = gtmv(m, 1), 
            .res_profile = gtmv(m, 3), 
            .spare_room_0 = gtmv(m, 2), 
            .spare_room_1 = gtmv(m, 2), 
            .service_queue_profile = gtmv(m, 5), 
            .timestamp_res_profile = gtmv(m, 2) };

        if ((start_idx >= max_q_idx) || (stop_idx >= max_q_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_q_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (q_idx = 0; q_idx < max_q_idx; q_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_q_context_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", q_idx,
                q_context.wred_profile, q_context.copy_dec_profile, q_context.ddr_copy_disable, q_context.aggregation_disable, 
                q_context.fpm_ug_or_bufmng, q_context.exclusive_priority, q_context.q_802_1ae, q_context.sci, 
                q_context.fec_enable, q_context.res_profile, q_context.spare_room_0, q_context.spare_room_1, 
                q_context.service_queue_profile, q_context.timestamp_res_profile);
            ag_err = ag_drv_qm_q_context_set(q_idx, &q_context);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", q_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        q_context.wred_profile = gtmv(m, 4);
        q_context.copy_dec_profile = gtmv(m, 3);
        q_context.ddr_copy_disable = gtmv(m, 1);
        q_context.aggregation_disable = gtmv(m, 1);
        q_context.fpm_ug_or_bufmng = gtmv(m, 5);
        q_context.exclusive_priority = gtmv(m, 1);
        q_context.q_802_1ae = gtmv(m, 1);
        q_context.sci = gtmv(m, 1);
        q_context.fec_enable = gtmv(m, 1);
        q_context.res_profile = gtmv(m, 3);
        q_context.spare_room_0 = gtmv(m, 2);
        q_context.spare_room_1 = gtmv(m, 2);
        q_context.service_queue_profile = gtmv(m, 5);
        q_context.timestamp_res_profile = gtmv(m, 2);

        for (q_idx = start_idx; q_idx <= stop_idx; q_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_q_context_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", q_idx,
                q_context.wred_profile, q_context.copy_dec_profile, q_context.ddr_copy_disable, q_context.aggregation_disable, 
                q_context.fpm_ug_or_bufmng, q_context.exclusive_priority, q_context.q_802_1ae, q_context.sci, 
                q_context.fec_enable, q_context.res_profile, q_context.spare_room_0, q_context.spare_room_1, 
                q_context.service_queue_profile, q_context.timestamp_res_profile);
            ag_err = ag_drv_qm_q_context_set(q_idx, &q_context);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", q_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (q_idx = 0; q_idx < max_q_idx; q_idx++)
        {
            if (q_idx < start_idx || q_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_q_context_get(q_idx, &q_context);

            bdmf_session_print(session, "ag_drv_qm_q_context_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", q_idx,
                q_context.wred_profile, q_context.copy_dec_profile, q_context.ddr_copy_disable, q_context.aggregation_disable, 
                q_context.fpm_ug_or_bufmng, q_context.exclusive_priority, q_context.q_802_1ae, q_context.sci, 
                q_context.fec_enable, q_context.res_profile, q_context.spare_room_0, q_context.spare_room_1, 
                q_context.service_queue_profile, q_context.timestamp_res_profile);

            if (q_context.wred_profile != gtmv(m, 4) || 
                q_context.copy_dec_profile != gtmv(m, 3) || 
                q_context.ddr_copy_disable != gtmv(m, 1) || 
                q_context.aggregation_disable != gtmv(m, 1) || 
                q_context.fpm_ug_or_bufmng != gtmv(m, 5) || 
                q_context.exclusive_priority != gtmv(m, 1) || 
                q_context.q_802_1ae != gtmv(m, 1) || 
                q_context.sci != gtmv(m, 1) || 
                q_context.fec_enable != gtmv(m, 1) || 
                q_context.res_profile != gtmv(m, 3) || 
                q_context.spare_room_0 != gtmv(m, 2) || 
                q_context.spare_room_1 != gtmv(m, 2) || 
                q_context.service_queue_profile != gtmv(m, 5) || 
                q_context.timestamp_res_profile != gtmv(m, 2) || 
                0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", q_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of q_context completed. Number of tested entries %u.\n", max_q_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_copy_decision_profile:
    {
        uint8_t max_profile_idx = 8;
        uint8_t profile_idx = gtmv(m, 3);
        uint32_t queue_occupancy_thr = gtmv(m, 30);
        bdmf_boolean psram_thr = gtmv(m, 1);

        if ((start_idx >= max_profile_idx) || (stop_idx >= max_profile_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_idx = 0; profile_idx < max_profile_idx; profile_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_copy_decision_profile_set( %u %u %u)\n", profile_idx,
                queue_occupancy_thr, psram_thr);
            ag_err = ag_drv_qm_copy_decision_profile_set(profile_idx, queue_occupancy_thr, psram_thr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        queue_occupancy_thr = gtmv(m, 30);
        psram_thr = gtmv(m, 1);

        for (profile_idx = start_idx; profile_idx <= stop_idx; profile_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_copy_decision_profile_set( %u %u %u)\n", profile_idx,
                queue_occupancy_thr, psram_thr);
            ag_err = ag_drv_qm_copy_decision_profile_set(profile_idx, queue_occupancy_thr, psram_thr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_idx = 0; profile_idx < max_profile_idx; profile_idx++)
        {
            if (profile_idx < start_idx || profile_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_copy_decision_profile_get(profile_idx, &queue_occupancy_thr, &psram_thr);

            bdmf_session_print(session, "ag_drv_qm_copy_decision_profile_get( %u %u %u)\n", profile_idx,
                queue_occupancy_thr, psram_thr);

            if (queue_occupancy_thr != gtmv(m, 30) || psram_thr != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of copy_decision_profile completed. Number of tested entries %u.\n", max_profile_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_timestamp_res_profile:
    {
        uint8_t max_profile_idx = 4;
        uint8_t profile_idx = gtmv(m, 2);
        uint8_t start = gtmv(m, 5);

        if ((start_idx >= max_profile_idx) || (stop_idx >= max_profile_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_idx = 0; profile_idx < max_profile_idx; profile_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_timestamp_res_profile_set( %u %u)\n", profile_idx,
                start);
            ag_err = ag_drv_qm_timestamp_res_profile_set(profile_idx, start);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        start = gtmv(m, 5);

        for (profile_idx = start_idx; profile_idx <= stop_idx; profile_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_timestamp_res_profile_set( %u %u)\n", profile_idx,
                start);
            ag_err = ag_drv_qm_timestamp_res_profile_set(profile_idx, start);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_idx = 0; profile_idx < max_profile_idx; profile_idx++)
        {
            if (profile_idx < start_idx || profile_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_timestamp_res_profile_get(profile_idx, &start);

            bdmf_session_print(session, "ag_drv_qm_timestamp_res_profile_get( %u %u)\n", profile_idx,
                start);

            if (start != gtmv(m, 5) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of timestamp_res_profile completed. Number of tested entries %u.\n", max_profile_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_total_valid_cnt:
    {
        uint16_t max_q_idx = 640;
        uint16_t q_idx = gtmv(m, 9);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_q_idx) || (stop_idx >= max_q_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_q_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (q_idx = 0; q_idx < max_q_idx; q_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_total_valid_cnt_set( %u %u)\n", q_idx,
                data);
            ag_err = ag_drv_qm_total_valid_cnt_set(q_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", q_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 32);

        for (q_idx = start_idx; q_idx <= stop_idx; q_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_total_valid_cnt_set( %u %u)\n", q_idx,
                data);
            ag_err = ag_drv_qm_total_valid_cnt_set(q_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", q_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (q_idx = 0; q_idx < max_q_idx; q_idx++)
        {
            if (q_idx < start_idx || q_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_total_valid_cnt_get(q_idx, &data);

            bdmf_session_print(session, "ag_drv_qm_total_valid_cnt_get( %u %u)\n", q_idx,
                data);

            if (data != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", q_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of total_valid_cnt completed. Number of tested entries %u.\n", max_q_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_dqm_valid_cnt:
    {
        uint16_t max_q_idx = 320;
        uint16_t q_idx = gtmv(m, 8);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_q_idx) || (stop_idx >= max_q_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_q_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (q_idx = 0; q_idx < max_q_idx; q_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_dqm_valid_cnt_set( %u %u)\n", q_idx,
                data);
            ag_err = ag_drv_qm_dqm_valid_cnt_set(q_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", q_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 32);

        for (q_idx = start_idx; q_idx <= stop_idx; q_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_dqm_valid_cnt_set( %u %u)\n", q_idx,
                data);
            ag_err = ag_drv_qm_dqm_valid_cnt_set(q_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", q_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (q_idx = 0; q_idx < max_q_idx; q_idx++)
        {
            if (q_idx < start_idx || q_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_dqm_valid_cnt_get(q_idx, &data);

            bdmf_session_print(session, "ag_drv_qm_dqm_valid_cnt_get( %u %u)\n", q_idx,
                data);

            if (data != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", q_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of dqm_valid_cnt completed. Number of tested entries %u.\n", max_q_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_epon_q_byte_cnt:
    {
        uint16_t max_q_idx = 320;
        uint16_t q_idx = gtmv(m, 8);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_q_idx) || (stop_idx >= max_q_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_q_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (q_idx = 0; q_idx < max_q_idx; q_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_epon_q_byte_cnt_set( %u %u)\n", q_idx,
                data);
            ag_err = ag_drv_qm_epon_q_byte_cnt_set(q_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", q_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 32);

        for (q_idx = start_idx; q_idx <= stop_idx; q_idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_epon_q_byte_cnt_set( %u %u)\n", q_idx,
                data);
            ag_err = ag_drv_qm_epon_q_byte_cnt_set(q_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", q_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (q_idx = 0; q_idx < max_q_idx; q_idx++)
        {
            if (q_idx < start_idx || q_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_epon_q_byte_cnt_get(q_idx, &data);

            bdmf_session_print(session, "ag_drv_qm_epon_q_byte_cnt_get( %u %u)\n", q_idx,
                data);

            if (data != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", q_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of epon_q_byte_cnt completed. Number of tested entries %u.\n", max_q_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_fpm_buffer_reservation_data:
    {
        uint32_t max_idx = 8;
        uint32_t idx = gtmv(m, 3);
        uint32_t data = gtmv(m, 18);

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_buffer_reservation_data_set( %u %u)\n", idx,
                data);
            ag_err = ag_drv_qm_fpm_buffer_reservation_data_set(idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 18);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_fpm_buffer_reservation_data_set( %u %u)\n", idx,
                data);
            ag_err = ag_drv_qm_fpm_buffer_reservation_data_set(idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (idx = 0; idx < max_idx; idx++)
        {
            if (idx < start_idx || idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_fpm_buffer_reservation_data_get(idx, &data);

            bdmf_session_print(session, "ag_drv_qm_fpm_buffer_reservation_data_get( %u %u)\n", idx,
                data);

            if (data != gtmv(m, 18) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of fpm_buffer_reservation_data completed. Number of tested entries %u.\n", max_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_port_cfg:
    {
        uint8_t max_idx = 11;
        uint8_t idx = gtmv(m, 3);
        bdmf_boolean en_byte = gtmv(m, 1);
        bdmf_boolean en_ug = gtmv(m, 1);
        uint8_t bbh_rx_bb_id = gtmv(m, 6);
        uint8_t fw_port_id = gtmv(m, 5);

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_port_cfg_set( %u %u %u %u %u)\n", idx,
                en_byte, en_ug, bbh_rx_bb_id, fw_port_id);
            ag_err = ag_drv_qm_port_cfg_set(idx, en_byte, en_ug, bbh_rx_bb_id, fw_port_id);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        en_byte = gtmv(m, 1);
        en_ug = gtmv(m, 1);
        bbh_rx_bb_id = gtmv(m, 6);
        fw_port_id = gtmv(m, 5);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_port_cfg_set( %u %u %u %u %u)\n", idx,
                en_byte, en_ug, bbh_rx_bb_id, fw_port_id);
            ag_err = ag_drv_qm_port_cfg_set(idx, en_byte, en_ug, bbh_rx_bb_id, fw_port_id);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (idx = 0; idx < max_idx; idx++)
        {
            if (idx < start_idx || idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_port_cfg_get(idx, &en_byte, &en_ug, &bbh_rx_bb_id, &fw_port_id);

            bdmf_session_print(session, "ag_drv_qm_port_cfg_get( %u %u %u %u %u)\n", idx,
                en_byte, en_ug, bbh_rx_bb_id, fw_port_id);

            if (en_byte != gtmv(m, 1) || en_ug != gtmv(m, 1) || bbh_rx_bb_id != gtmv(m, 6) || fw_port_id != gtmv(m, 5) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of port_cfg completed. Number of tested entries %u.\n", max_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_fc_queue_mask:
    {
        uint8_t max_idx = 5;
        uint8_t idx = gtmv(m, 2);
        uint32_t queue_vec = gtmv(m, 32);

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_fc_queue_mask_set( %u %u)\n", idx,
                queue_vec);
            ag_err = ag_drv_qm_fc_queue_mask_set(idx, queue_vec);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        queue_vec = gtmv(m, 32);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_fc_queue_mask_set( %u %u)\n", idx,
                queue_vec);
            ag_err = ag_drv_qm_fc_queue_mask_set(idx, queue_vec);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (idx = 0; idx < max_idx; idx++)
        {
            if (idx < start_idx || idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_fc_queue_mask_get(idx, &queue_vec);

            bdmf_session_print(session, "ag_drv_qm_fc_queue_mask_get( %u %u)\n", idx,
                queue_vec);

            if (queue_vec != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of fc_queue_mask completed. Number of tested entries %u.\n", max_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_qm_bb_route_ovr:
    {
        uint8_t max_idx = 3;
        uint8_t idx = gtmv(m, 1);
        bdmf_boolean ovr_en = gtmv(m, 1);
        uint8_t dest_id = gtmv(m, 6);
        uint16_t route_addr = gtmv(m, 10);

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_bb_route_ovr_set( %u %u %u %u)\n", idx,
                ovr_en, dest_id, route_addr);
            ag_err = ag_drv_qm_bb_route_ovr_set(idx, ovr_en, dest_id, route_addr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ovr_en = gtmv(m, 1);
        dest_id = gtmv(m, 6);
        route_addr = gtmv(m, 10);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_qm_bb_route_ovr_set( %u %u %u %u)\n", idx,
                ovr_en, dest_id, route_addr);
            ag_err = ag_drv_qm_bb_route_ovr_set(idx, ovr_en, dest_id, route_addr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (idx = 0; idx < max_idx; idx++)
        {
            if (idx < start_idx || idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_qm_bb_route_ovr_get(idx, &ovr_en, &dest_id, &route_addr);

            bdmf_session_print(session, "ag_drv_qm_bb_route_ovr_get( %u %u %u %u)\n", idx,
                ovr_en, dest_id, route_addr);

            if (ovr_en != gtmv(m, 1) || dest_id != gtmv(m, 6) || route_addr != gtmv(m, 10) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of bb_route_ovr completed. Number of tested entries %u.\n", max_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_qm_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int chip_rev_idx = RU_CHIP_REV_IDX_GET();
    uint32_t i;
    uint32_t j;
    uint32_t index1_start = 0;
    uint32_t index1_stop;
    uint32_t index2_start = 0;
    uint32_t index2_stop;
    bdmfmon_cmd_parm_t *cliparm;
    const ru_reg_rec *reg;
    const ru_block_rec *blk;
    const char *enum_string = bdmfmon_enum_parm_stringval(session, &parm[0]);

    if(!enum_string)
        return BDMF_ERR_INTERNAL;

    switch (parm[0].value.unumber)
    {
    case bdmf_address_global_cfg_qm_enable_ctrl: reg = &RU_REG(QM, GLOBAL_CFG_QM_ENABLE_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_sw_rst_ctrl: reg = &RU_REG(QM, GLOBAL_CFG_QM_SW_RST_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_general_ctrl: reg = &RU_REG(QM, GLOBAL_CFG_QM_GENERAL_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_fpm_control: reg = &RU_REG(QM, GLOBAL_CFG_FPM_CONTROL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_ddr_byte_congestion_control: reg = &RU_REG(QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_ddr_byte_congestion_lower_thr: reg = &RU_REG(QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_ddr_byte_congestion_mid_thr: reg = &RU_REG(QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_ddr_byte_congestion_higher_thr: reg = &RU_REG(QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_ddr_pd_congestion_control: reg = &RU_REG(QM, GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_pd_congestion_control: reg = &RU_REG(QM, GLOBAL_CFG_QM_PD_CONGESTION_CONTROL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_abs_drop_queue: reg = &RU_REG(QM, GLOBAL_CFG_ABS_DROP_QUEUE); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_aggregation_ctrl: reg = &RU_REG(QM, GLOBAL_CFG_AGGREGATION_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_aggregation_ctrl2: reg = &RU_REG(QM, GLOBAL_CFG_AGGREGATION_CTRL2); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_fpm_base_addr: reg = &RU_REG(QM, GLOBAL_CFG_FPM_BASE_ADDR); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_fpm_coherent_base_addr: reg = &RU_REG(QM, GLOBAL_CFG_FPM_COHERENT_BASE_ADDR); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_ddr_sop_offset: reg = &RU_REG(QM, GLOBAL_CFG_DDR_SOP_OFFSET); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_epon_overhead_ctrl: reg = &RU_REG(QM, GLOBAL_CFG_EPON_OVERHEAD_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_bbhtx_fifo_addr: reg = &RU_REG(QM, GLOBAL_CFG_BBHTX_FIFO_ADDR); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_dqm_full: reg = &RU_REG(QM, GLOBAL_CFG_DQM_FULL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_dqm_not_empty: reg = &RU_REG(QM, GLOBAL_CFG_DQM_NOT_EMPTY); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_dqm_pop_ready: reg = &RU_REG(QM, GLOBAL_CFG_DQM_POP_READY); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_aggregation_context_valid: reg = &RU_REG(QM, GLOBAL_CFG_AGGREGATION_CONTEXT_VALID); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_egress_flush_queue: reg = &RU_REG(QM, GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_aggregation_timer_ctrl: reg = &RU_REG(QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_fpm_ug_gbl_cnt: reg = &RU_REG(QM, GLOBAL_CFG_QM_FPM_UG_GBL_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_ddr_spare_room: reg = &RU_REG(QM, GLOBAL_CFG_DDR_SPARE_ROOM); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_dummy_spare_room_profile_id: reg = &RU_REG(QM, GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_dqm_ubus_ctrl: reg = &RU_REG(QM, GLOBAL_CFG_DQM_UBUS_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_mem_auto_init: reg = &RU_REG(QM, GLOBAL_CFG_MEM_AUTO_INIT); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_mem_auto_init_sts: reg = &RU_REG(QM, GLOBAL_CFG_MEM_AUTO_INIT_STS); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_fpm_mpm_enhancement_pool_size_tokens: reg = &RU_REG(QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte: reg = &RU_REG(QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte: reg = &RU_REG(QM, GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_mc_ctrl: reg = &RU_REG(QM, GLOBAL_CFG_MC_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_aqm_clk_counter_cycle: reg = &RU_REG(QM, GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_aqm_push_to_empty_thr: reg = &RU_REG(QM, GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_general_ctrl2: reg = &RU_REG(QM, GLOBAL_CFG_QM_GENERAL_CTRL2); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_pools_thr: reg = &RU_REG(QM, FPM_POOLS_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_usr_grp_lower_thr: reg = &RU_REG(QM, FPM_USR_GRP_LOWER_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_usr_grp_mid_thr: reg = &RU_REG(QM, FPM_USR_GRP_MID_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_usr_grp_higher_thr: reg = &RU_REG(QM, FPM_USR_GRP_HIGHER_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_usr_grp_cnt: reg = &RU_REG(QM, FPM_USR_GRP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_runner_grp_rnr_config: reg = &RU_REG(QM, RUNNER_GRP_RNR_CONFIG); blk = &RU_BLK(QM); break;
    case bdmf_address_runner_grp_queue_config: reg = &RU_REG(QM, RUNNER_GRP_QUEUE_CONFIG); blk = &RU_BLK(QM); break;
    case bdmf_address_runner_grp_pdfifo_config: reg = &RU_REG(QM, RUNNER_GRP_PDFIFO_CONFIG); blk = &RU_BLK(QM); break;
    case bdmf_address_runner_grp_update_fifo_config: reg = &RU_REG(QM, RUNNER_GRP_UPDATE_FIFO_CONFIG); blk = &RU_BLK(QM); break;
    case bdmf_address_intr_ctrl_isr: reg = &RU_REG(QM, INTR_CTRL_ISR); blk = &RU_BLK(QM); break;
    case bdmf_address_intr_ctrl_ism: reg = &RU_REG(QM, INTR_CTRL_ISM); blk = &RU_BLK(QM); break;
    case bdmf_address_intr_ctrl_ier: reg = &RU_REG(QM, INTR_CTRL_IER); blk = &RU_BLK(QM); break;
    case bdmf_address_intr_ctrl_itr: reg = &RU_REG(QM, INTR_CTRL_ITR); blk = &RU_BLK(QM); break;
    case bdmf_address_clk_gate_clk_gate_cntrl: reg = &RU_REG(QM, CLK_GATE_CLK_GATE_CNTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_ctrl: reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_0: reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_1: reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_2: reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_3: reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_0: reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_1: reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_2: reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_3: reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3); blk = &RU_BLK(QM); break;
    case bdmf_address_queue_context_context: reg = &RU_REG(QM, QUEUE_CONTEXT_CONTEXT); blk = &RU_BLK(QM); break;
    case bdmf_address_wred_profile_color_min_thr_0: reg = &RU_REG(QM, WRED_PROFILE_COLOR_MIN_THR_0); blk = &RU_BLK(QM); break;
    case bdmf_address_wred_profile_color_min_thr_1: reg = &RU_REG(QM, WRED_PROFILE_COLOR_MIN_THR_1); blk = &RU_BLK(QM); break;
    case bdmf_address_wred_profile_color_max_thr_0: reg = &RU_REG(QM, WRED_PROFILE_COLOR_MAX_THR_0); blk = &RU_BLK(QM); break;
    case bdmf_address_wred_profile_color_max_thr_1: reg = &RU_REG(QM, WRED_PROFILE_COLOR_MAX_THR_1); blk = &RU_BLK(QM); break;
    case bdmf_address_wred_profile_color_slope_0: reg = &RU_REG(QM, WRED_PROFILE_COLOR_SLOPE_0); blk = &RU_BLK(QM); break;
    case bdmf_address_wred_profile_color_slope_1: reg = &RU_REG(QM, WRED_PROFILE_COLOR_SLOPE_1); blk = &RU_BLK(QM); break;
    case bdmf_address_copy_decision_profile_thr: reg = &RU_REG(QM, COPY_DECISION_PROFILE_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_timestamp_res_profile_value: reg = &RU_REG(QM, TIMESTAMP_RES_PROFILE_VALUE); blk = &RU_BLK(QM); break;
    case bdmf_address_global_egress_drop_counter_counter: reg = &RU_REG(QM, GLOBAL_EGRESS_DROP_COUNTER_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_global_egress_aqm_drop_counter_counter: reg = &RU_REG(QM, GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_total_valid_counter_counter: reg = &RU_REG(QM, TOTAL_VALID_COUNTER_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_dqm_valid_counter_counter: reg = &RU_REG(QM, DQM_VALID_COUNTER_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_drop_counter_counter: reg = &RU_REG(QM, DROP_COUNTER_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_total_egress_accumulated_counter_counter: reg = &RU_REG(QM, TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_epon_rpt_cnt_counter: reg = &RU_REG(QM, EPON_RPT_CNT_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_epon_rpt_cnt_queue_status: reg = &RU_REG(QM, EPON_RPT_CNT_QUEUE_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_rd_data_pool0: reg = &RU_REG(QM, RD_DATA_POOL0); blk = &RU_BLK(QM); break;
    case bdmf_address_rd_data_pool1: reg = &RU_REG(QM, RD_DATA_POOL1); blk = &RU_BLK(QM); break;
    case bdmf_address_rd_data_pool2: reg = &RU_REG(QM, RD_DATA_POOL2); blk = &RU_BLK(QM); break;
    case bdmf_address_rd_data_pool3: reg = &RU_REG(QM, RD_DATA_POOL3); blk = &RU_BLK(QM); break;
    case bdmf_address_pop_3: reg = &RU_REG(QM, POP_3); blk = &RU_BLK(QM); break;
    case bdmf_address_pdfifo_ptr: reg = &RU_REG(QM, PDFIFO_PTR); blk = &RU_BLK(QM); break;
    case bdmf_address_update_fifo_ptr: reg = &RU_REG(QM, UPDATE_FIFO_PTR); blk = &RU_BLK(QM); break;
    case bdmf_address_rd_data_2: reg = &RU_REG(QM, RD_DATA_2); blk = &RU_BLK(QM); break;
    case bdmf_address_pop_2: reg = &RU_REG(QM, POP_2); blk = &RU_BLK(QM); break;
    case bdmf_address_rd_data_1: reg = &RU_REG(QM, RD_DATA_1); blk = &RU_BLK(QM); break;
    case bdmf_address_pop_1: reg = &RU_REG(QM, POP_1); blk = &RU_BLK(QM); break;
    case bdmf_address_rd_data: reg = &RU_REG(QM, RD_DATA); blk = &RU_BLK(QM); break;
    case bdmf_address_pop: reg = &RU_REG(QM, POP); blk = &RU_BLK(QM); break;
    case bdmf_address_cm_common_input_fifo_data: reg = &RU_REG(QM, CM_COMMON_INPUT_FIFO_DATA); blk = &RU_BLK(QM); break;
    case bdmf_address_normal_rmt_fifo_data: reg = &RU_REG(QM, NORMAL_RMT_FIFO_DATA); blk = &RU_BLK(QM); break;
    case bdmf_address_non_delayed_rmt_fifo_data: reg = &RU_REG(QM, NON_DELAYED_RMT_FIFO_DATA); blk = &RU_BLK(QM); break;
    case bdmf_address_egress_data_fifo_data: reg = &RU_REG(QM, EGRESS_DATA_FIFO_DATA); blk = &RU_BLK(QM); break;
    case bdmf_address_egress_rr_fifo_data: reg = &RU_REG(QM, EGRESS_RR_FIFO_DATA); blk = &RU_BLK(QM); break;
    case bdmf_address_egress_bb_input_fifo_data: reg = &RU_REG(QM, EGRESS_BB_INPUT_FIFO_DATA); blk = &RU_BLK(QM); break;
    case bdmf_address_egress_bb_output_fifo_data: reg = &RU_REG(QM, EGRESS_BB_OUTPUT_FIFO_DATA); blk = &RU_BLK(QM); break;
    case bdmf_address_bb_output_fifo_data: reg = &RU_REG(QM, BB_OUTPUT_FIFO_DATA); blk = &RU_BLK(QM); break;
    case bdmf_address_non_delayed_out_fifo_data: reg = &RU_REG(QM, NON_DELAYED_OUT_FIFO_DATA); blk = &RU_BLK(QM); break;
    case bdmf_address_bb0_egr_msg_out_fifo_data: reg = &RU_REG(QM, BB0_EGR_MSG_OUT_FIFO_DATA); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_buffer_reservation_data: reg = &RU_REG(QM, FPM_BUFFER_RESERVATION_DATA); blk = &RU_BLK(QM); break;
    case bdmf_address_port_cfg: reg = &RU_REG(QM, PORT_CFG); blk = &RU_BLK(QM); break;
    case bdmf_address_fc_ug_mask_ug_en: reg = &RU_REG(QM, FC_UG_MASK_UG_EN); blk = &RU_BLK(QM); break;
    case bdmf_address_fc_queue_mask: reg = &RU_REG(QM, FC_QUEUE_MASK); blk = &RU_BLK(QM); break;
    case bdmf_address_fc_queue_range1_start: reg = &RU_REG(QM, FC_QUEUE_RANGE1_START); blk = &RU_BLK(QM); break;
    case bdmf_address_fc_queue_range2_start: reg = &RU_REG(QM, FC_QUEUE_RANGE2_START); blk = &RU_BLK(QM); break;
    case bdmf_address_dbg: reg = &RU_REG(QM, DBG); blk = &RU_BLK(QM); break;
    case bdmf_address_ug_occupancy_status: reg = &RU_REG(QM, UG_OCCUPANCY_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_queue_range1_occupancy_status: reg = &RU_REG(QM, QUEUE_RANGE1_OCCUPANCY_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_queue_range2_occupancy_status: reg = &RU_REG(QM, QUEUE_RANGE2_OCCUPANCY_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_context_data: reg = &RU_REG(QM, CONTEXT_DATA); blk = &RU_BLK(QM); break;
    case bdmf_address_debug_sel: reg = &RU_REG(QM, DEBUG_SEL); blk = &RU_BLK(QM); break;
    case bdmf_address_debug_bus_lsb: reg = &RU_REG(QM, DEBUG_BUS_LSB); blk = &RU_BLK(QM); break;
    case bdmf_address_debug_bus_msb: reg = &RU_REG(QM, DEBUG_BUS_MSB); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_spare_config: reg = &RU_REG(QM, QM_SPARE_CONFIG); blk = &RU_BLK(QM); break;
    case bdmf_address_good_lvl1_pkts_cnt: reg = &RU_REG(QM, GOOD_LVL1_PKTS_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_good_lvl1_bytes_cnt: reg = &RU_REG(QM, GOOD_LVL1_BYTES_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_good_lvl2_pkts_cnt: reg = &RU_REG(QM, GOOD_LVL2_PKTS_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_good_lvl2_bytes_cnt: reg = &RU_REG(QM, GOOD_LVL2_BYTES_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_copied_pkts_cnt: reg = &RU_REG(QM, COPIED_PKTS_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_copied_bytes_cnt: reg = &RU_REG(QM, COPIED_BYTES_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_agg_pkts_cnt: reg = &RU_REG(QM, AGG_PKTS_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_agg_bytes_cnt: reg = &RU_REG(QM, AGG_BYTES_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_agg_1_pkts_cnt: reg = &RU_REG(QM, AGG_1_PKTS_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_agg_2_pkts_cnt: reg = &RU_REG(QM, AGG_2_PKTS_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_agg_3_pkts_cnt: reg = &RU_REG(QM, AGG_3_PKTS_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_agg_4_pkts_cnt: reg = &RU_REG(QM, AGG_4_PKTS_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_wred_drop_cnt: reg = &RU_REG(QM, WRED_DROP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_congestion_drop_cnt: reg = &RU_REG(QM, FPM_CONGESTION_DROP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_ddr_pd_congestion_drop_cnt: reg = &RU_REG(QM, DDR_PD_CONGESTION_DROP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_ddr_byte_congestion_drop_cnt: reg = &RU_REG(QM, DDR_BYTE_CONGESTION_DROP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_pd_congestion_drop_cnt: reg = &RU_REG(QM, QM_PD_CONGESTION_DROP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_abs_requeue_cnt: reg = &RU_REG(QM, QM_ABS_REQUEUE_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_prefetch_fifo0_status: reg = &RU_REG(QM, FPM_PREFETCH_FIFO0_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_prefetch_fifo1_status: reg = &RU_REG(QM, FPM_PREFETCH_FIFO1_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_prefetch_fifo2_status: reg = &RU_REG(QM, FPM_PREFETCH_FIFO2_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_prefetch_fifo3_status: reg = &RU_REG(QM, FPM_PREFETCH_FIFO3_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_normal_rmt_fifo_status: reg = &RU_REG(QM, NORMAL_RMT_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_non_delayed_rmt_fifo_status: reg = &RU_REG(QM, NON_DELAYED_RMT_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_non_delayed_out_fifo_status: reg = &RU_REG(QM, NON_DELAYED_OUT_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_pre_cm_fifo_status: reg = &RU_REG(QM, PRE_CM_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_cm_rd_pd_fifo_status: reg = &RU_REG(QM, CM_RD_PD_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_cm_wr_pd_fifo_status: reg = &RU_REG(QM, CM_WR_PD_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_cm_common_input_fifo_status: reg = &RU_REG(QM, CM_COMMON_INPUT_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_bb0_output_fifo_status: reg = &RU_REG(QM, BB0_OUTPUT_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_bb1_output_fifo_status: reg = &RU_REG(QM, BB1_OUTPUT_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_bb1_input_fifo_status: reg = &RU_REG(QM, BB1_INPUT_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_egress_data_fifo_status: reg = &RU_REG(QM, EGRESS_DATA_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_egress_rr_fifo_status: reg = &RU_REG(QM, EGRESS_RR_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_bb_route_ovr: reg = &RU_REG(QM, BB_ROUTE_OVR); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_ingress_stat: reg = &RU_REG(QM, QM_INGRESS_STAT); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_egress_stat: reg = &RU_REG(QM, QM_EGRESS_STAT); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_cm_stat: reg = &RU_REG(QM, QM_CM_STAT); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_fpm_prefetch_stat: reg = &RU_REG(QM, QM_FPM_PREFETCH_STAT); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_connect_ack_counter: reg = &RU_REG(QM, QM_CONNECT_ACK_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_ddr_wr_reply_counter: reg = &RU_REG(QM, QM_DDR_WR_REPLY_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_ddr_pipe_byte_counter: reg = &RU_REG(QM, QM_DDR_PIPE_BYTE_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_abs_requeue_valid_counter: reg = &RU_REG(QM, QM_ABS_REQUEUE_VALID_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_illegal_pd_capture: reg = &RU_REG(QM, QM_ILLEGAL_PD_CAPTURE); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_ingress_processed_pd_capture: reg = &RU_REG(QM, QM_INGRESS_PROCESSED_PD_CAPTURE); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_cm_processed_pd_capture: reg = &RU_REG(QM, QM_CM_PROCESSED_PD_CAPTURE); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_grp_drop_cnt: reg = &RU_REG(QM, FPM_GRP_DROP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_pool_drop_cnt: reg = &RU_REG(QM, FPM_POOL_DROP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_buffer_res_drop_cnt: reg = &RU_REG(QM, FPM_BUFFER_RES_DROP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_psram_egress_cong_drp_cnt: reg = &RU_REG(QM, PSRAM_EGRESS_CONG_DRP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_backpressure: reg = &RU_REG(QM, BACKPRESSURE); blk = &RU_BLK(QM); break;
    case bdmf_address_aqm_timestamp_curr_counter: reg = &RU_REG(QM, AQM_TIMESTAMP_CURR_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_bb0_egr_msg_out_fifo_status: reg = &RU_REG(QM, BB0_EGR_MSG_OUT_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_count_pkt_not_pd_mode_bits: reg = &RU_REG(QM, COUNT_PKT_NOT_PD_MODE_BITS); blk = &RU_BLK(QM); break;
    case bdmf_address_data: reg = &RU_REG(QM, DATA); blk = &RU_BLK(QM); break;
    default :
        return BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    if ((cliparm = bdmfmon_cmd_find(session, "index1")))
    {
        index1_start = cliparm->value.unumber;
        index1_stop = index1_start + 1;
    }
    else
        index1_stop = blk->addr_count;
    if ((cliparm = bdmfmon_cmd_find(session, "index2")))
    {
        index2_start = cliparm->value.unumber;
        index2_stop = index2_start + 1;
    }
    else
        index2_stop = reg->ram_count;
    if (index1_stop > blk->addr_count)
    {
        bdmf_session_print(session, "index1 (%u) is out of range (%u).\n", index1_stop, blk->addr_count);
        return BDMF_ERR_RANGE;
    }
    if (index2_stop > (reg->ram_count))
    {
        bdmf_session_print(session, "index2 (%u) is out of range (%u).\n", index2_stop, reg->ram_count);
        return BDMF_ERR_RANGE;
    }
    if (reg->ram_count)
        for (i = index1_start; i < index1_stop; i++)
        {
            bdmf_session_print(session, "index1 = %u\n", i);
            for (j = index2_start; j < index2_stop; j++)
                bdmf_session_print(session, TAB "(%5u) 0x%08X\n", j, ((blk->addr[i] + reg->addr[chip_rev_idx]) + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%08X\n", i, blk->addr[i]+reg->addr[chip_rev_idx]);
    return 0;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

bdmfmon_handle_t ag_drv_qm_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "qm", "qm", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_ddr_cong_ctrl[] = {
            BDMFMON_MAKE_PARM("ddr_byte_congestion_drop_enable", "ddr_byte_congestion_drop_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bytes_lower_thr", "ddr_bytes_lower_thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bytes_mid_thr", "ddr_bytes_mid_thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bytes_higher_thr", "ddr_bytes_higher_thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_pd_congestion_drop_enable", "ddr_pd_congestion_drop_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_pipe_lower_thr", "ddr_pipe_lower_thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_pipe_higher_thr", "ddr_pipe_higher_thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_ug_thr[] = {
            BDMFMON_MAKE_PARM("ug_grp_idx", "ug_grp_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("lower_thr", "lower_thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mid_thr", "mid_thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("higher_thr", "higher_thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rnr_group_cfg[] = {
            BDMFMON_MAKE_PARM("rnr_idx", "rnr_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("start_queue", "start_queue", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("end_queue", "end_queue", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pd_fifo_base", "pd_fifo_base", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pd_fifo_size", "pd_fifo_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("upd_fifo_base", "upd_fifo_base", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("upd_fifo_size", "upd_fifo_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_bb_id", "rnr_bb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_task", "rnr_task", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_enable", "rnr_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cpu_pd_indirect_wr_data[] = {
            BDMFMON_MAKE_PARM("indirect_grp_idx", "indirect_grp_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data0", "data0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data1", "data1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data2", "data2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data3", "data3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wred_profile_cfg[] = {
            BDMFMON_MAKE_PARM("profile_idx", "profile_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("min_thr0", "min_thr0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flw_ctrl_en0", "flw_ctrl_en0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("min_thr1", "min_thr1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flw_ctrl_en1", "flw_ctrl_en1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("max_thr0", "max_thr0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("max_thr1", "max_thr1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("slope_mantissa0", "slope_mantissa0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("slope_exp0", "slope_exp0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("slope_mantissa1", "slope_mantissa1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("slope_exp1", "slope_exp1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_enable_ctrl[] = {
            BDMFMON_MAKE_PARM("fpm_prefetch_enable", "fpm_prefetch_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("reorder_credit_enable", "reorder_credit_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dqm_pop_enable", "dqm_pop_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rmt_fixed_arb_enable", "rmt_fixed_arb_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dqm_push_fixed_arb_enable", "dqm_push_fixed_arb_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("aqm_clk_counter_enable", "aqm_clk_counter_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("aqm_timestamp_counter_enable", "aqm_timestamp_counter_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("aqm_timestamp_write_to_pd_enable", "aqm_timestamp_write_to_pd_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_reset_ctrl[] = {
            BDMFMON_MAKE_PARM("fpm_prefetch0_sw_rst", "fpm_prefetch0_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_prefetch1_sw_rst", "fpm_prefetch1_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_prefetch2_sw_rst", "fpm_prefetch2_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_prefetch3_sw_rst", "fpm_prefetch3_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("normal_rmt_sw_rst", "normal_rmt_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_delayed_rmt_sw_rst", "non_delayed_rmt_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pre_cm_fifo_sw_rst", "pre_cm_fifo_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cm_rd_pd_fifo_sw_rst", "cm_rd_pd_fifo_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cm_wr_pd_fifo_sw_rst", "cm_wr_pd_fifo_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bb0_output_fifo_sw_rst", "bb0_output_fifo_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bb1_output_fifo_sw_rst", "bb1_output_fifo_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bb1_input_fifo_sw_rst", "bb1_input_fifo_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tm_fifo_ptr_sw_rst", "tm_fifo_ptr_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("non_delayed_out_fifo_sw_rst", "non_delayed_out_fifo_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bb0_egr_msg_out_fifo_sw_rst", "bb0_egr_msg_out_fifo_sw_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_drop_counters_ctrl[] = {
            BDMFMON_MAKE_PARM("read_clear_pkts", "read_clear_pkts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("read_clear_bytes", "read_clear_bytes", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("disable_wrap_around_pkts", "disable_wrap_around_pkts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("disable_wrap_around_bytes", "disable_wrap_around_bytes", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("free_with_context_last_search", "free_with_context_last_search", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("wred_disable", "wred_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_pd_congestion_disable", "ddr_pd_congestion_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_byte_congestion_disable", "ddr_byte_congestion_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_occupancy_disable", "ddr_occupancy_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_fpm_congestion_disable", "ddr_fpm_congestion_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_ug_disable", "fpm_ug_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("queue_occupancy_ddr_copy_decision_disable", "queue_occupancy_ddr_copy_decision_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram_occupancy_ddr_copy_decision_disable", "psram_occupancy_ddr_copy_decision_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dont_send_mc_bit_to_bbh", "dont_send_mc_bit_to_bbh", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("close_aggregation_on_timeout_disable", "close_aggregation_on_timeout_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_congestion_buf_release_mechanism_disable", "fpm_congestion_buf_release_mechanism_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_buffer_global_res_enable", "fpm_buffer_global_res_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_preserve_pd_with_fpm", "qm_preserve_pd_with_fpm", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_residue_per_queue", "qm_residue_per_queue", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ghost_rpt_update_after_close_agg_en", "ghost_rpt_update_after_close_agg_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_ug_flow_ctrl_disable", "fpm_ug_flow_ctrl_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_write_multi_slave_en", "ddr_write_multi_slave_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_pd_congestion_agg_priority", "ddr_pd_congestion_agg_priority", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram_occupancy_drop_disable", "psram_occupancy_drop_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_ddr_write_alignment", "qm_ddr_write_alignment", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("exclusive_dont_drop", "exclusive_dont_drop", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dqmol_jira_973_fix_enable", "dqmol_jira_973_fix_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gpon_dbr_ceil", "gpon_dbr_ceil", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("drop_cnt_wred_drops", "drop_cnt_wred_drops", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("same_sec_lvl_bit_agg_en", "same_sec_lvl_bit_agg_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("glbl_egr_drop_cnt_read_clear_enable", "glbl_egr_drop_cnt_read_clear_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("glbl_egr_aqm_drop_cnt_read_clear_enable", "glbl_egr_aqm_drop_cnt_read_clear_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_ctrl[] = {
            BDMFMON_MAKE_PARM("fpm_pool_bp_enable", "fpm_pool_bp_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_congestion_bp_enable", "fpm_congestion_bp_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_force_bp_lvl", "fpm_force_bp_lvl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_prefetch_granularity", "fpm_prefetch_granularity", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_prefetch_min_pool_size", "fpm_prefetch_min_pool_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_prefetch_pending_req_limit", "fpm_prefetch_pending_req_limit", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_override_bb_id_en", "fpm_override_bb_id_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_override_bb_id_value", "fpm_override_bb_id_value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qm_pd_cong_ctrl[] = {
            BDMFMON_MAKE_PARM("total_pd_thr", "total_pd_thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_abs_drop_queue[] = {
            BDMFMON_MAKE_PARM("abs_drop_queue", "abs_drop_queue", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("abs_drop_queue_en", "abs_drop_queue_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_aggregation_ctrl[] = {
            BDMFMON_MAKE_PARM("max_agg_bytes", "max_agg_bytes", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("max_agg_pkts", "max_agg_pkts", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("agg_ovr_512b_en", "agg_ovr_512b_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("max_agg_pkt_size", "max_agg_pkt_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("min_agg_pkt_size", "min_agg_pkt_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_aggregation_ctrl2[] = {
            BDMFMON_MAKE_PARM("agg_pool_sel_en", "agg_pool_sel_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("agg_pool_sel", "agg_pool_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_base_addr[] = {
            BDMFMON_MAKE_PARM("fpm_base_addr", "fpm_base_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_fpm_coherent_base_addr[] = {
            BDMFMON_MAKE_PARM("fpm_base_addr", "fpm_base_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ddr_sop_offset[] = {
            BDMFMON_MAKE_PARM("ddr_sop_offset0", "ddr_sop_offset0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_sop_offset1", "ddr_sop_offset1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_epon_overhead_ctrl[] = {
            BDMFMON_MAKE_PARM("epon_line_rate", "epon_line_rate", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("epon_crc_add_disable", "epon_crc_add_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mac_flow_overwrite_crc_en", "mac_flow_overwrite_crc_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mac_flow_overwrite_crc", "mac_flow_overwrite_crc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fec_ipg_length", "fec_ipg_length", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_bbhtx_fifo_addr[] = {
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bbhtx_req_otf", "bbhtx_req_otf", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_qm_egress_flush_queue[] = {
            BDMFMON_MAKE_PARM("queue_num", "queue_num", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flush_en", "flush_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_qm_aggregation_timer_ctrl[] = {
            BDMFMON_MAKE_PARM("prescaler_granularity", "prescaler_granularity", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("aggregation_timeout_value", "aggregation_timeout_value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pd_occupancy_en", "pd_occupancy_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pd_occupancy_value", "pd_occupancy_value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_qm_fpm_ug_gbl_cnt[] = {
            BDMFMON_MAKE_PARM("fpm_gbl_cnt", "fpm_gbl_cnt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qm_ddr_spare_room[] = {
            BDMFMON_MAKE_PARM("pair_idx", "pair_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_headroom", "ddr_headroom", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_tailroom", "ddr_tailroom", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_dummy_spare_room_profile_id[] = {
            BDMFMON_MAKE_PARM("dummy_profile_0", "dummy_profile_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dummy_profile_1", "dummy_profile_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_dqm_ubus_ctrl[] = {
            BDMFMON_MAKE_PARM("tkn_reqout_h", "tkn_reqout_h", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tkn_reqout_d", "tkn_reqout_d", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("offload_reqout_h", "offload_reqout_h", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("offload_reqout_d", "offload_reqout_d", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_mem_auto_init[] = {
            BDMFMON_MAKE_PARM("mem_init_en", "mem_init_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mem_sel_init", "mem_sel_init", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mem_size_init", "mem_size_init", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_fpm_mpm_enhancement_pool_size_tokens[] = {
            BDMFMON_MAKE_PARM("pool_0_num_of_tkns", "pool_0_num_of_tkns", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_1_num_of_tkns", "pool_1_num_of_tkns", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_2_num_of_tkns", "pool_2_num_of_tkns", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_3_num_of_tkns", "pool_3_num_of_tkns", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte[] = {
            BDMFMON_MAKE_PARM("pool_0_num_of_bytes", "pool_0_num_of_bytes", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_1_num_of_bytes", "pool_1_num_of_bytes", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte[] = {
            BDMFMON_MAKE_PARM("pool_2_num_of_bytes", "pool_2_num_of_bytes", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pool_3_num_of_bytes", "pool_3_num_of_bytes", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_mc_ctrl[] = {
            BDMFMON_MAKE_PARM("mc_headers_pool_sel", "mc_headers_pool_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_aqm_clk_counter_cycle[] = {
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_aqm_push_to_empty_thr[] = {
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_qm_general_ctrl2[] = {
            BDMFMON_MAKE_PARM("egress_accumulated_cnt_pkts_read_clear_enable", "egress_accumulated_cnt_pkts_read_clear_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("egress_accumulated_cnt_bytes_read_clear_enable", "egress_accumulated_cnt_bytes_read_clear_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("agg_closure_suspend_on_bp", "agg_closure_suspend_on_bp", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bufmng_en_or_ug_cntr", "bufmng_en_or_ug_cntr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dqm_to_fpm_ubus_or_fpmini", "dqm_to_fpm_ubus_or_fpmini", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("agg_closure_suspend_on_fpm_congestion_disable", "agg_closure_suspend_on_fpm_congestion_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_pool_thr[] = {
            BDMFMON_MAKE_PARM("pool_idx", "pool_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("lower_thr", "lower_thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("higher_thr", "higher_thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_ug_cnt[] = {
            BDMFMON_MAKE_PARM("grp_idx", "grp_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_ug_cnt", "fpm_ug_cnt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_isr[] = {
            BDMFMON_MAKE_PARM("qm_dqm_pop_on_empty", "qm_dqm_pop_on_empty", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_dqm_push_on_full", "qm_dqm_push_on_full", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_cpu_pop_on_empty", "qm_cpu_pop_on_empty", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_cpu_push_on_full", "qm_cpu_push_on_full", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_normal_queue_pd_no_credit", "qm_normal_queue_pd_no_credit", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_non_delayed_queue_pd_no_credit", "qm_non_delayed_queue_pd_no_credit", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_non_valid_queue", "qm_non_valid_queue", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_agg_coherent_inconsistency", "qm_agg_coherent_inconsistency", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_force_copy_on_non_delayed", "qm_force_copy_on_non_delayed", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_fpm_pool_size_nonexistent", "qm_fpm_pool_size_nonexistent", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_target_mem_abs_contradiction", "qm_target_mem_abs_contradiction", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_1588_drop", "qm_1588_drop", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_1588_multicast_contradiction", "qm_1588_multicast_contradiction", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_byte_drop_cnt_overrun", "qm_byte_drop_cnt_overrun", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_pkt_drop_cnt_overrun", "qm_pkt_drop_cnt_overrun", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_total_byte_cnt_underrun", "qm_total_byte_cnt_underrun", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_total_pkt_cnt_underrun", "qm_total_pkt_cnt_underrun", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_fpm_ug0_underrun", "qm_fpm_ug0_underrun", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_fpm_ug1_underrun", "qm_fpm_ug1_underrun", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_fpm_ug2_underrun", "qm_fpm_ug2_underrun", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_fpm_ug3_underrun", "qm_fpm_ug3_underrun", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_timer_wraparound", "qm_timer_wraparound", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_copy_plen_zero", "qm_copy_plen_zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_ingress_bb_unexpected_msg", "qm_ingress_bb_unexpected_msg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_egress_bb_unexpected_msg", "qm_egress_bb_unexpected_msg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dqm_reached_full", "dqm_reached_full", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_fpmini_intr", "qm_fpmini_intr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_ier[] = {
            BDMFMON_MAKE_PARM("iem", "iem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_itr[] = {
            BDMFMON_MAKE_PARM("ist", "ist", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_clk_gate_clk_gate_cntrl[] = {
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intrvl", "keep_alive_intrvl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cpu_indr_port_cpu_pd_indirect_ctrl[] = {
            BDMFMON_MAKE_PARM("indirect_grp_idx", "indirect_grp_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("queue_num", "queue_num", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cmd", "cmd", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("done", "done", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("error", "error", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_q_context[] = {
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("wred_profile", "wred_profile", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("copy_dec_profile", "copy_dec_profile", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_copy_disable", "ddr_copy_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("aggregation_disable", "aggregation_disable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_ug_or_bufmng", "fpm_ug_or_bufmng", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("exclusive_priority", "exclusive_priority", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("q_802_1ae", "q_802_1ae", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sci", "sci", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fec_enable", "fec_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("res_profile", "res_profile", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("spare_room_0", "spare_room_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("spare_room_1", "spare_room_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("service_queue_profile", "service_queue_profile", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("timestamp_res_profile", "timestamp_res_profile", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_copy_decision_profile[] = {
            BDMFMON_MAKE_PARM("profile_idx", "profile_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("queue_occupancy_thr", "queue_occupancy_thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram_thr", "psram_thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_timestamp_res_profile[] = {
            BDMFMON_MAKE_PARM("profile_idx", "profile_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("start", "start", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_total_valid_cnt[] = {
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqm_valid_cnt[] = {
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_epon_q_byte_cnt[] = {
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pop_3[] = {
            BDMFMON_MAKE_PARM("pop_pool0", "pop_pool0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pop_pool1", "pop_pool1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pop_pool2", "pop_pool2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pop_pool3", "pop_pool3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pop_2[] = {
            BDMFMON_MAKE_PARM("pop", "pop", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pop_1[] = {
            BDMFMON_MAKE_PARM("pop", "pop", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_buffer_reservation_data[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_port_cfg[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en_byte", "en_byte", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en_ug", "en_ug", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bbh_rx_bb_id", "bbh_rx_bb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fw_port_id", "fw_port_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fc_ug_mask_ug_en[] = {
            BDMFMON_MAKE_PARM("ug_en", "ug_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fc_queue_mask[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("queue_vec", "queue_vec", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fc_queue_range1_start[] = {
            BDMFMON_MAKE_PARM("start_queue", "start_queue", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fc_queue_range2_start[] = {
            BDMFMON_MAKE_PARM("start_queue", "start_queue", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_sel[] = {
            BDMFMON_MAKE_PARM("select", "select", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bb_route_ovr[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ovr_en", "ovr_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dest_id", "dest_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("route_addr", "route_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_backpressure[] = {
            BDMFMON_MAKE_PARM("status", "status", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_count_pkt_not_pd_mode_bits[] = {
            BDMFMON_MAKE_PARM("total_egress_accum_cnt_pkt", "total_egress_accum_cnt_pkt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("global_egress_drop_cnt_pkt", "global_egress_drop_cnt_pkt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("drop_ing_egr_cnt_pkt", "drop_ing_egr_cnt_pkt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_grp_drop_cnt_pkt", "fpm_grp_drop_cnt_pkt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_pd_congestion_drop_cnt_pkt", "qm_pd_congestion_drop_cnt_pkt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_pd_congestion_drop_cnt_pkt", "ddr_pd_congestion_drop_cnt_pkt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("wred_drop_cnt_pkt", "wred_drop_cnt_pkt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("good_lvl2_pkts_cnt_pkt", "good_lvl2_pkts_cnt_pkt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("good_lvl1_pkts_cnt_pkt", "good_lvl1_pkts_cnt_pkt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_total_valid_cnt_pkt", "qm_total_valid_cnt_pkt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dqm_valid_cnt_pkt", "dqm_valid_cnt_pkt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "ddr_cong_ctrl", .val = cli_qm_ddr_cong_ctrl, .parms = set_ddr_cong_ctrl },
            { .name = "fpm_ug_thr", .val = cli_qm_fpm_ug_thr, .parms = set_fpm_ug_thr },
            { .name = "rnr_group_cfg", .val = cli_qm_rnr_group_cfg, .parms = set_rnr_group_cfg },
            { .name = "cpu_pd_indirect_wr_data", .val = cli_qm_cpu_pd_indirect_wr_data, .parms = set_cpu_pd_indirect_wr_data },
            { .name = "wred_profile_cfg", .val = cli_qm_wred_profile_cfg, .parms = set_wred_profile_cfg },
            { .name = "enable_ctrl", .val = cli_qm_enable_ctrl, .parms = set_enable_ctrl },
            { .name = "reset_ctrl", .val = cli_qm_reset_ctrl, .parms = set_reset_ctrl },
            { .name = "drop_counters_ctrl", .val = cli_qm_drop_counters_ctrl, .parms = set_drop_counters_ctrl },
            { .name = "fpm_ctrl", .val = cli_qm_fpm_ctrl, .parms = set_fpm_ctrl },
            { .name = "qm_pd_cong_ctrl", .val = cli_qm_qm_pd_cong_ctrl, .parms = set_qm_pd_cong_ctrl },
            { .name = "global_cfg_abs_drop_queue", .val = cli_qm_global_cfg_abs_drop_queue, .parms = set_global_cfg_abs_drop_queue },
            { .name = "global_cfg_aggregation_ctrl", .val = cli_qm_global_cfg_aggregation_ctrl, .parms = set_global_cfg_aggregation_ctrl },
            { .name = "global_cfg_aggregation_ctrl2", .val = cli_qm_global_cfg_aggregation_ctrl2, .parms = set_global_cfg_aggregation_ctrl2 },
            { .name = "fpm_base_addr", .val = cli_qm_fpm_base_addr, .parms = set_fpm_base_addr },
            { .name = "global_cfg_fpm_coherent_base_addr", .val = cli_qm_global_cfg_fpm_coherent_base_addr, .parms = set_global_cfg_fpm_coherent_base_addr },
            { .name = "ddr_sop_offset", .val = cli_qm_ddr_sop_offset, .parms = set_ddr_sop_offset },
            { .name = "epon_overhead_ctrl", .val = cli_qm_epon_overhead_ctrl, .parms = set_epon_overhead_ctrl },
            { .name = "global_cfg_bbhtx_fifo_addr", .val = cli_qm_global_cfg_bbhtx_fifo_addr, .parms = set_global_cfg_bbhtx_fifo_addr },
            { .name = "global_cfg_qm_egress_flush_queue", .val = cli_qm_global_cfg_qm_egress_flush_queue, .parms = set_global_cfg_qm_egress_flush_queue },
            { .name = "global_cfg_qm_aggregation_timer_ctrl", .val = cli_qm_global_cfg_qm_aggregation_timer_ctrl, .parms = set_global_cfg_qm_aggregation_timer_ctrl },
            { .name = "global_cfg_qm_fpm_ug_gbl_cnt", .val = cli_qm_global_cfg_qm_fpm_ug_gbl_cnt, .parms = set_global_cfg_qm_fpm_ug_gbl_cnt },
            { .name = "qm_ddr_spare_room", .val = cli_qm_qm_ddr_spare_room, .parms = set_qm_ddr_spare_room },
            { .name = "global_cfg_dummy_spare_room_profile_id", .val = cli_qm_global_cfg_dummy_spare_room_profile_id, .parms = set_global_cfg_dummy_spare_room_profile_id },
            { .name = "global_cfg_dqm_ubus_ctrl", .val = cli_qm_global_cfg_dqm_ubus_ctrl, .parms = set_global_cfg_dqm_ubus_ctrl },
            { .name = "global_cfg_mem_auto_init", .val = cli_qm_global_cfg_mem_auto_init, .parms = set_global_cfg_mem_auto_init },
            { .name = "global_cfg_fpm_mpm_enhancement_pool_size_tokens", .val = cli_qm_global_cfg_fpm_mpm_enhancement_pool_size_tokens, .parms = set_global_cfg_fpm_mpm_enhancement_pool_size_tokens },
            { .name = "global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte", .val = cli_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte, .parms = set_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte },
            { .name = "global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte", .val = cli_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte, .parms = set_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte },
            { .name = "global_cfg_mc_ctrl", .val = cli_qm_global_cfg_mc_ctrl, .parms = set_global_cfg_mc_ctrl },
            { .name = "global_cfg_aqm_clk_counter_cycle", .val = cli_qm_global_cfg_aqm_clk_counter_cycle, .parms = set_global_cfg_aqm_clk_counter_cycle },
            { .name = "global_cfg_aqm_push_to_empty_thr", .val = cli_qm_global_cfg_aqm_push_to_empty_thr, .parms = set_global_cfg_aqm_push_to_empty_thr },
            { .name = "global_cfg_qm_general_ctrl2", .val = cli_qm_global_cfg_qm_general_ctrl2, .parms = set_global_cfg_qm_general_ctrl2 },
            { .name = "fpm_pool_thr", .val = cli_qm_fpm_pool_thr, .parms = set_fpm_pool_thr },
            { .name = "fpm_ug_cnt", .val = cli_qm_fpm_ug_cnt, .parms = set_fpm_ug_cnt },
            { .name = "intr_ctrl_isr", .val = cli_qm_intr_ctrl_isr, .parms = set_intr_ctrl_isr },
            { .name = "intr_ctrl_ier", .val = cli_qm_intr_ctrl_ier, .parms = set_intr_ctrl_ier },
            { .name = "intr_ctrl_itr", .val = cli_qm_intr_ctrl_itr, .parms = set_intr_ctrl_itr },
            { .name = "clk_gate_clk_gate_cntrl", .val = cli_qm_clk_gate_clk_gate_cntrl, .parms = set_clk_gate_clk_gate_cntrl },
            { .name = "cpu_indr_port_cpu_pd_indirect_ctrl", .val = cli_qm_cpu_indr_port_cpu_pd_indirect_ctrl, .parms = set_cpu_indr_port_cpu_pd_indirect_ctrl },
            { .name = "q_context", .val = cli_qm_q_context, .parms = set_q_context },
            { .name = "copy_decision_profile", .val = cli_qm_copy_decision_profile, .parms = set_copy_decision_profile },
            { .name = "timestamp_res_profile", .val = cli_qm_timestamp_res_profile, .parms = set_timestamp_res_profile },
            { .name = "total_valid_cnt", .val = cli_qm_total_valid_cnt, .parms = set_total_valid_cnt },
            { .name = "dqm_valid_cnt", .val = cli_qm_dqm_valid_cnt, .parms = set_dqm_valid_cnt },
            { .name = "epon_q_byte_cnt", .val = cli_qm_epon_q_byte_cnt, .parms = set_epon_q_byte_cnt },
            { .name = "pop_3", .val = cli_qm_pop_3, .parms = set_pop_3 },
            { .name = "pop_2", .val = cli_qm_pop_2, .parms = set_pop_2 },
            { .name = "pop_1", .val = cli_qm_pop_1, .parms = set_pop_1 },
            { .name = "fpm_buffer_reservation_data", .val = cli_qm_fpm_buffer_reservation_data, .parms = set_fpm_buffer_reservation_data },
            { .name = "port_cfg", .val = cli_qm_port_cfg, .parms = set_port_cfg },
            { .name = "fc_ug_mask_ug_en", .val = cli_qm_fc_ug_mask_ug_en, .parms = set_fc_ug_mask_ug_en },
            { .name = "fc_queue_mask", .val = cli_qm_fc_queue_mask, .parms = set_fc_queue_mask },
            { .name = "fc_queue_range1_start", .val = cli_qm_fc_queue_range1_start, .parms = set_fc_queue_range1_start },
            { .name = "fc_queue_range2_start", .val = cli_qm_fc_queue_range2_start, .parms = set_fc_queue_range2_start },
            { .name = "debug_sel", .val = cli_qm_debug_sel, .parms = set_debug_sel },
            { .name = "bb_route_ovr", .val = cli_qm_bb_route_ovr, .parms = set_bb_route_ovr },
            { .name = "backpressure", .val = cli_qm_backpressure, .parms = set_backpressure },
            { .name = "count_pkt_not_pd_mode_bits", .val = cli_qm_count_pkt_not_pd_mode_bits, .parms = set_count_pkt_not_pd_mode_bits },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_qm_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_is_queue_not_empty[] = {
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_is_queue_pop_ready[] = {
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_is_queue_full[] = {
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_fpm_ug_thr[] = {
            BDMFMON_MAKE_PARM("ug_grp_idx", "ug_grp_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_rnr_group_cfg[] = {
            BDMFMON_MAKE_PARM("rnr_idx", "rnr_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_cpu_pd_indirect_wr_data[] = {
            BDMFMON_MAKE_PARM("indirect_grp_idx", "indirect_grp_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_cpu_pd_indirect_rd_data[] = {
            BDMFMON_MAKE_PARM("indirect_grp_idx", "indirect_grp_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_aggr_context[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_wred_profile_cfg[] = {
            BDMFMON_MAKE_PARM("profile_idx", "profile_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_qm_ddr_spare_room[] = {
            BDMFMON_MAKE_PARM("pair_idx", "pair_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_fpm_pool_thr[] = {
            BDMFMON_MAKE_PARM("pool_idx", "pool_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_fpm_ug_cnt[] = {
            BDMFMON_MAKE_PARM("grp_idx", "grp_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_cpu_indr_port_cpu_pd_indirect_ctrl[] = {
            BDMFMON_MAKE_PARM("indirect_grp_idx", "indirect_grp_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_q_context[] = {
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_copy_decision_profile[] = {
            BDMFMON_MAKE_PARM("profile_idx", "profile_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_timestamp_res_profile[] = {
            BDMFMON_MAKE_PARM("profile_idx", "profile_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_total_valid_cnt[] = {
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_dqm_valid_cnt[] = {
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_drop_counter[] = {
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_accumulated_counter[] = {
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_epon_q_byte_cnt[] = {
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_epon_q_status[] = {
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_pdfifo_ptr[] = {
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_update_fifo_ptr[] = {
            BDMFMON_MAKE_PARM("fifo_idx", "fifo_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_bb0_egr_msg_out_fifo_data[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_fpm_buffer_reservation_data[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_port_cfg[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_fc_queue_mask[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_ug_occupancy_status[] = {
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_queue_range1_occupancy_status[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_queue_range2_occupancy_status[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_bb_route_ovr[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_qm_illegal_pd_capture[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_qm_ingress_processed_pd_capture[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_qm_cm_processed_pd_capture[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_fpm_grp_drop_cnt[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_fpm_pool_drop_cnt[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_cm_residue_data[] = {
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "ddr_cong_ctrl", .val = cli_qm_ddr_cong_ctrl, .parms = get_default },
            { .name = "is_queue_not_empty", .val = cli_qm_is_queue_not_empty, .parms = get_is_queue_not_empty },
            { .name = "is_queue_pop_ready", .val = cli_qm_is_queue_pop_ready, .parms = get_is_queue_pop_ready },
            { .name = "is_queue_full", .val = cli_qm_is_queue_full, .parms = get_is_queue_full },
            { .name = "fpm_ug_thr", .val = cli_qm_fpm_ug_thr, .parms = get_fpm_ug_thr },
            { .name = "rnr_group_cfg", .val = cli_qm_rnr_group_cfg, .parms = get_rnr_group_cfg },
            { .name = "cpu_pd_indirect_wr_data", .val = cli_qm_cpu_pd_indirect_wr_data, .parms = get_cpu_pd_indirect_wr_data },
            { .name = "cpu_pd_indirect_rd_data", .val = cli_qm_cpu_pd_indirect_rd_data, .parms = get_cpu_pd_indirect_rd_data },
            { .name = "aggr_context", .val = cli_qm_aggr_context, .parms = get_aggr_context },
            { .name = "wred_profile_cfg", .val = cli_qm_wred_profile_cfg, .parms = get_wred_profile_cfg },
            { .name = "enable_ctrl", .val = cli_qm_enable_ctrl, .parms = get_default },
            { .name = "reset_ctrl", .val = cli_qm_reset_ctrl, .parms = get_default },
            { .name = "drop_counters_ctrl", .val = cli_qm_drop_counters_ctrl, .parms = get_default },
            { .name = "fpm_ctrl", .val = cli_qm_fpm_ctrl, .parms = get_default },
            { .name = "qm_pd_cong_ctrl", .val = cli_qm_qm_pd_cong_ctrl, .parms = get_default },
            { .name = "global_cfg_abs_drop_queue", .val = cli_qm_global_cfg_abs_drop_queue, .parms = get_default },
            { .name = "global_cfg_aggregation_ctrl", .val = cli_qm_global_cfg_aggregation_ctrl, .parms = get_default },
            { .name = "global_cfg_aggregation_ctrl2", .val = cli_qm_global_cfg_aggregation_ctrl2, .parms = get_default },
            { .name = "fpm_base_addr", .val = cli_qm_fpm_base_addr, .parms = get_default },
            { .name = "global_cfg_fpm_coherent_base_addr", .val = cli_qm_global_cfg_fpm_coherent_base_addr, .parms = get_default },
            { .name = "ddr_sop_offset", .val = cli_qm_ddr_sop_offset, .parms = get_default },
            { .name = "epon_overhead_ctrl", .val = cli_qm_epon_overhead_ctrl, .parms = get_default },
            { .name = "global_cfg_bbhtx_fifo_addr", .val = cli_qm_global_cfg_bbhtx_fifo_addr, .parms = get_default },
            { .name = "global_cfg_qm_egress_flush_queue", .val = cli_qm_global_cfg_qm_egress_flush_queue, .parms = get_default },
            { .name = "global_cfg_qm_aggregation_timer_ctrl", .val = cli_qm_global_cfg_qm_aggregation_timer_ctrl, .parms = get_default },
            { .name = "global_cfg_qm_fpm_ug_gbl_cnt", .val = cli_qm_global_cfg_qm_fpm_ug_gbl_cnt, .parms = get_default },
            { .name = "qm_ddr_spare_room", .val = cli_qm_qm_ddr_spare_room, .parms = get_qm_ddr_spare_room },
            { .name = "global_cfg_dummy_spare_room_profile_id", .val = cli_qm_global_cfg_dummy_spare_room_profile_id, .parms = get_default },
            { .name = "global_cfg_dqm_ubus_ctrl", .val = cli_qm_global_cfg_dqm_ubus_ctrl, .parms = get_default },
            { .name = "global_cfg_mem_auto_init", .val = cli_qm_global_cfg_mem_auto_init, .parms = get_default },
            { .name = "global_cfg_mem_auto_init_sts", .val = cli_qm_global_cfg_mem_auto_init_sts, .parms = get_default },
            { .name = "global_cfg_fpm_mpm_enhancement_pool_size_tokens", .val = cli_qm_global_cfg_fpm_mpm_enhancement_pool_size_tokens, .parms = get_default },
            { .name = "global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte", .val = cli_qm_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte, .parms = get_default },
            { .name = "global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte", .val = cli_qm_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte, .parms = get_default },
            { .name = "global_cfg_mc_ctrl", .val = cli_qm_global_cfg_mc_ctrl, .parms = get_default },
            { .name = "global_cfg_aqm_clk_counter_cycle", .val = cli_qm_global_cfg_aqm_clk_counter_cycle, .parms = get_default },
            { .name = "global_cfg_aqm_push_to_empty_thr", .val = cli_qm_global_cfg_aqm_push_to_empty_thr, .parms = get_default },
            { .name = "global_cfg_qm_general_ctrl2", .val = cli_qm_global_cfg_qm_general_ctrl2, .parms = get_default },
            { .name = "fpm_pool_thr", .val = cli_qm_fpm_pool_thr, .parms = get_fpm_pool_thr },
            { .name = "fpm_ug_cnt", .val = cli_qm_fpm_ug_cnt, .parms = get_fpm_ug_cnt },
            { .name = "intr_ctrl_isr", .val = cli_qm_intr_ctrl_isr, .parms = get_default },
            { .name = "intr_ctrl_ism", .val = cli_qm_intr_ctrl_ism, .parms = get_default },
            { .name = "intr_ctrl_ier", .val = cli_qm_intr_ctrl_ier, .parms = get_default },
            { .name = "intr_ctrl_itr", .val = cli_qm_intr_ctrl_itr, .parms = get_default },
            { .name = "clk_gate_clk_gate_cntrl", .val = cli_qm_clk_gate_clk_gate_cntrl, .parms = get_default },
            { .name = "cpu_indr_port_cpu_pd_indirect_ctrl", .val = cli_qm_cpu_indr_port_cpu_pd_indirect_ctrl, .parms = get_cpu_indr_port_cpu_pd_indirect_ctrl },
            { .name = "q_context", .val = cli_qm_q_context, .parms = get_q_context },
            { .name = "copy_decision_profile", .val = cli_qm_copy_decision_profile, .parms = get_copy_decision_profile },
            { .name = "timestamp_res_profile", .val = cli_qm_timestamp_res_profile, .parms = get_timestamp_res_profile },
            { .name = "global_egress_drop_counter", .val = cli_qm_global_egress_drop_counter, .parms = get_default },
            { .name = "global_egress_aqm_drop_counter", .val = cli_qm_global_egress_aqm_drop_counter, .parms = get_default },
            { .name = "total_valid_cnt", .val = cli_qm_total_valid_cnt, .parms = get_total_valid_cnt },
            { .name = "dqm_valid_cnt", .val = cli_qm_dqm_valid_cnt, .parms = get_dqm_valid_cnt },
            { .name = "drop_counter", .val = cli_qm_drop_counter, .parms = get_drop_counter },
            { .name = "accumulated_counter", .val = cli_qm_accumulated_counter, .parms = get_accumulated_counter },
            { .name = "epon_q_byte_cnt", .val = cli_qm_epon_q_byte_cnt, .parms = get_epon_q_byte_cnt },
            { .name = "epon_q_status", .val = cli_qm_epon_q_status, .parms = get_epon_q_status },
            { .name = "rd_data_pool0", .val = cli_qm_rd_data_pool0, .parms = get_default },
            { .name = "rd_data_pool1", .val = cli_qm_rd_data_pool1, .parms = get_default },
            { .name = "rd_data_pool2", .val = cli_qm_rd_data_pool2, .parms = get_default },
            { .name = "rd_data_pool3", .val = cli_qm_rd_data_pool3, .parms = get_default },
            { .name = "pop_3", .val = cli_qm_pop_3, .parms = get_default },
            { .name = "pdfifo_ptr", .val = cli_qm_pdfifo_ptr, .parms = get_pdfifo_ptr },
            { .name = "update_fifo_ptr", .val = cli_qm_update_fifo_ptr, .parms = get_update_fifo_ptr },
            { .name = "pop_2", .val = cli_qm_pop_2, .parms = get_default },
            { .name = "pop_1", .val = cli_qm_pop_1, .parms = get_default },
            { .name = "bb0_egr_msg_out_fifo_data", .val = cli_qm_bb0_egr_msg_out_fifo_data, .parms = get_bb0_egr_msg_out_fifo_data },
            { .name = "fpm_buffer_reservation_data", .val = cli_qm_fpm_buffer_reservation_data, .parms = get_fpm_buffer_reservation_data },
            { .name = "port_cfg", .val = cli_qm_port_cfg, .parms = get_port_cfg },
            { .name = "fc_ug_mask_ug_en", .val = cli_qm_fc_ug_mask_ug_en, .parms = get_default },
            { .name = "fc_queue_mask", .val = cli_qm_fc_queue_mask, .parms = get_fc_queue_mask },
            { .name = "fc_queue_range1_start", .val = cli_qm_fc_queue_range1_start, .parms = get_default },
            { .name = "fc_queue_range2_start", .val = cli_qm_fc_queue_range2_start, .parms = get_default },
            { .name = "dbg", .val = cli_qm_dbg, .parms = get_default },
            { .name = "ug_occupancy_status", .val = cli_qm_ug_occupancy_status, .parms = get_ug_occupancy_status },
            { .name = "queue_range1_occupancy_status", .val = cli_qm_queue_range1_occupancy_status, .parms = get_queue_range1_occupancy_status },
            { .name = "queue_range2_occupancy_status", .val = cli_qm_queue_range2_occupancy_status, .parms = get_queue_range2_occupancy_status },
            { .name = "debug_sel", .val = cli_qm_debug_sel, .parms = get_default },
            { .name = "debug_bus_lsb", .val = cli_qm_debug_bus_lsb, .parms = get_default },
            { .name = "debug_bus_msb", .val = cli_qm_debug_bus_msb, .parms = get_default },
            { .name = "qm_spare_config", .val = cli_qm_qm_spare_config, .parms = get_default },
            { .name = "good_lvl1_pkts_cnt", .val = cli_qm_good_lvl1_pkts_cnt, .parms = get_default },
            { .name = "good_lvl1_bytes_cnt", .val = cli_qm_good_lvl1_bytes_cnt, .parms = get_default },
            { .name = "good_lvl2_pkts_cnt", .val = cli_qm_good_lvl2_pkts_cnt, .parms = get_default },
            { .name = "good_lvl2_bytes_cnt", .val = cli_qm_good_lvl2_bytes_cnt, .parms = get_default },
            { .name = "copied_pkts_cnt", .val = cli_qm_copied_pkts_cnt, .parms = get_default },
            { .name = "copied_bytes_cnt", .val = cli_qm_copied_bytes_cnt, .parms = get_default },
            { .name = "agg_pkts_cnt", .val = cli_qm_agg_pkts_cnt, .parms = get_default },
            { .name = "agg_bytes_cnt", .val = cli_qm_agg_bytes_cnt, .parms = get_default },
            { .name = "agg_1_pkts_cnt", .val = cli_qm_agg_1_pkts_cnt, .parms = get_default },
            { .name = "agg_2_pkts_cnt", .val = cli_qm_agg_2_pkts_cnt, .parms = get_default },
            { .name = "agg_3_pkts_cnt", .val = cli_qm_agg_3_pkts_cnt, .parms = get_default },
            { .name = "agg_4_pkts_cnt", .val = cli_qm_agg_4_pkts_cnt, .parms = get_default },
            { .name = "wred_drop_cnt", .val = cli_qm_wred_drop_cnt, .parms = get_default },
            { .name = "fpm_congestion_drop_cnt", .val = cli_qm_fpm_congestion_drop_cnt, .parms = get_default },
            { .name = "ddr_pd_congestion_drop_cnt", .val = cli_qm_ddr_pd_congestion_drop_cnt, .parms = get_default },
            { .name = "ddr_byte_congestion_drop_cnt", .val = cli_qm_ddr_byte_congestion_drop_cnt, .parms = get_default },
            { .name = "qm_pd_congestion_drop_cnt", .val = cli_qm_qm_pd_congestion_drop_cnt, .parms = get_default },
            { .name = "qm_abs_requeue_cnt", .val = cli_qm_qm_abs_requeue_cnt, .parms = get_default },
            { .name = "fpm_prefetch_fifo0_status", .val = cli_qm_fpm_prefetch_fifo0_status, .parms = get_default },
            { .name = "fpm_prefetch_fifo1_status", .val = cli_qm_fpm_prefetch_fifo1_status, .parms = get_default },
            { .name = "fpm_prefetch_fifo2_status", .val = cli_qm_fpm_prefetch_fifo2_status, .parms = get_default },
            { .name = "fpm_prefetch_fifo3_status", .val = cli_qm_fpm_prefetch_fifo3_status, .parms = get_default },
            { .name = "normal_rmt_fifo_status", .val = cli_qm_normal_rmt_fifo_status, .parms = get_default },
            { .name = "non_delayed_rmt_fifo_status", .val = cli_qm_non_delayed_rmt_fifo_status, .parms = get_default },
            { .name = "non_delayed_out_fifo_status", .val = cli_qm_non_delayed_out_fifo_status, .parms = get_default },
            { .name = "pre_cm_fifo_status", .val = cli_qm_pre_cm_fifo_status, .parms = get_default },
            { .name = "cm_rd_pd_fifo_status", .val = cli_qm_cm_rd_pd_fifo_status, .parms = get_default },
            { .name = "cm_wr_pd_fifo_status", .val = cli_qm_cm_wr_pd_fifo_status, .parms = get_default },
            { .name = "cm_common_input_fifo_status", .val = cli_qm_cm_common_input_fifo_status, .parms = get_default },
            { .name = "bb0_output_fifo_status", .val = cli_qm_bb0_output_fifo_status, .parms = get_default },
            { .name = "bb1_output_fifo_status", .val = cli_qm_bb1_output_fifo_status, .parms = get_default },
            { .name = "bb1_input_fifo_status", .val = cli_qm_bb1_input_fifo_status, .parms = get_default },
            { .name = "egress_data_fifo_status", .val = cli_qm_egress_data_fifo_status, .parms = get_default },
            { .name = "egress_rr_fifo_status", .val = cli_qm_egress_rr_fifo_status, .parms = get_default },
            { .name = "bb_route_ovr", .val = cli_qm_bb_route_ovr, .parms = get_bb_route_ovr },
            { .name = "ingress_stat", .val = cli_qm_ingress_stat, .parms = get_default },
            { .name = "egress_stat", .val = cli_qm_egress_stat, .parms = get_default },
            { .name = "cm_stat", .val = cli_qm_cm_stat, .parms = get_default },
            { .name = "fpm_prefetch_stat", .val = cli_qm_fpm_prefetch_stat, .parms = get_default },
            { .name = "qm_connect_ack_counter", .val = cli_qm_qm_connect_ack_counter, .parms = get_default },
            { .name = "qm_ddr_wr_reply_counter", .val = cli_qm_qm_ddr_wr_reply_counter, .parms = get_default },
            { .name = "qm_ddr_pipe_byte_counter", .val = cli_qm_qm_ddr_pipe_byte_counter, .parms = get_default },
            { .name = "qm_abs_requeue_valid_counter", .val = cli_qm_qm_abs_requeue_valid_counter, .parms = get_default },
            { .name = "qm_illegal_pd_capture", .val = cli_qm_qm_illegal_pd_capture, .parms = get_qm_illegal_pd_capture },
            { .name = "qm_ingress_processed_pd_capture", .val = cli_qm_qm_ingress_processed_pd_capture, .parms = get_qm_ingress_processed_pd_capture },
            { .name = "qm_cm_processed_pd_capture", .val = cli_qm_qm_cm_processed_pd_capture, .parms = get_qm_cm_processed_pd_capture },
            { .name = "fpm_grp_drop_cnt", .val = cli_qm_fpm_grp_drop_cnt, .parms = get_fpm_grp_drop_cnt },
            { .name = "fpm_pool_drop_cnt", .val = cli_qm_fpm_pool_drop_cnt, .parms = get_fpm_pool_drop_cnt },
            { .name = "fpm_buffer_res_drop_cnt", .val = cli_qm_fpm_buffer_res_drop_cnt, .parms = get_default },
            { .name = "psram_egress_cong_drp_cnt", .val = cli_qm_psram_egress_cong_drp_cnt, .parms = get_default },
            { .name = "backpressure", .val = cli_qm_backpressure, .parms = get_default },
            { .name = "aqm_timestamp_curr_counter", .val = cli_qm_aqm_timestamp_curr_counter, .parms = get_default },
            { .name = "bb0_egr_msg_out_fifo_status", .val = cli_qm_bb0_egr_msg_out_fifo_status, .parms = get_default },
            { .name = "count_pkt_not_pd_mode_bits", .val = cli_qm_count_pkt_not_pd_mode_bits, .parms = get_default },
            { .name = "cm_residue_data", .val = cli_qm_cm_residue_data, .parms = get_cm_residue_data },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_qm_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            { .name = "high", .val = ag_drv_cli_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_qm_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "fpm_ug_thr", .val = cli_qm_fpm_ug_thr, .parms = ext_test_default},
            { .name = "rnr_group_cfg", .val = cli_qm_rnr_group_cfg, .parms = ext_test_default},
            { .name = "cpu_pd_indirect_wr_data", .val = cli_qm_cpu_pd_indirect_wr_data, .parms = ext_test_default},
            { .name = "wred_profile_cfg", .val = cli_qm_wred_profile_cfg, .parms = ext_test_default},
            { .name = "qm_ddr_spare_room", .val = cli_qm_qm_ddr_spare_room, .parms = ext_test_default},
            { .name = "fpm_pool_thr", .val = cli_qm_fpm_pool_thr, .parms = ext_test_default},
            { .name = "fpm_ug_cnt", .val = cli_qm_fpm_ug_cnt, .parms = ext_test_default},
            { .name = "cpu_indr_port_cpu_pd_indirect_ctrl", .val = cli_qm_cpu_indr_port_cpu_pd_indirect_ctrl, .parms = ext_test_default},
            { .name = "q_context", .val = cli_qm_q_context, .parms = ext_test_default},
            { .name = "copy_decision_profile", .val = cli_qm_copy_decision_profile, .parms = ext_test_default},
            { .name = "timestamp_res_profile", .val = cli_qm_timestamp_res_profile, .parms = ext_test_default},
            { .name = "total_valid_cnt", .val = cli_qm_total_valid_cnt, .parms = ext_test_default},
            { .name = "dqm_valid_cnt", .val = cli_qm_dqm_valid_cnt, .parms = ext_test_default},
            { .name = "epon_q_byte_cnt", .val = cli_qm_epon_q_byte_cnt, .parms = ext_test_default},
            { .name = "fpm_buffer_reservation_data", .val = cli_qm_fpm_buffer_reservation_data, .parms = ext_test_default},
            { .name = "port_cfg", .val = cli_qm_port_cfg, .parms = ext_test_default},
            { .name = "fc_queue_mask", .val = cli_qm_fc_queue_mask, .parms = ext_test_default},
            { .name = "bb_route_ovr", .val = cli_qm_bb_route_ovr, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_qm_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "GLOBAL_CFG_QM_ENABLE_CTRL", .val = bdmf_address_global_cfg_qm_enable_ctrl },
            { .name = "GLOBAL_CFG_QM_SW_RST_CTRL", .val = bdmf_address_global_cfg_qm_sw_rst_ctrl },
            { .name = "GLOBAL_CFG_QM_GENERAL_CTRL", .val = bdmf_address_global_cfg_qm_general_ctrl },
            { .name = "GLOBAL_CFG_FPM_CONTROL", .val = bdmf_address_global_cfg_fpm_control },
            { .name = "GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL", .val = bdmf_address_global_cfg_ddr_byte_congestion_control },
            { .name = "GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR", .val = bdmf_address_global_cfg_ddr_byte_congestion_lower_thr },
            { .name = "GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR", .val = bdmf_address_global_cfg_ddr_byte_congestion_mid_thr },
            { .name = "GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR", .val = bdmf_address_global_cfg_ddr_byte_congestion_higher_thr },
            { .name = "GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL", .val = bdmf_address_global_cfg_ddr_pd_congestion_control },
            { .name = "GLOBAL_CFG_QM_PD_CONGESTION_CONTROL", .val = bdmf_address_global_cfg_qm_pd_congestion_control },
            { .name = "GLOBAL_CFG_ABS_DROP_QUEUE", .val = bdmf_address_global_cfg_abs_drop_queue },
            { .name = "GLOBAL_CFG_AGGREGATION_CTRL", .val = bdmf_address_global_cfg_aggregation_ctrl },
            { .name = "GLOBAL_CFG_AGGREGATION_CTRL2", .val = bdmf_address_global_cfg_aggregation_ctrl2 },
            { .name = "GLOBAL_CFG_FPM_BASE_ADDR", .val = bdmf_address_global_cfg_fpm_base_addr },
            { .name = "GLOBAL_CFG_FPM_COHERENT_BASE_ADDR", .val = bdmf_address_global_cfg_fpm_coherent_base_addr },
            { .name = "GLOBAL_CFG_DDR_SOP_OFFSET", .val = bdmf_address_global_cfg_ddr_sop_offset },
            { .name = "GLOBAL_CFG_EPON_OVERHEAD_CTRL", .val = bdmf_address_global_cfg_epon_overhead_ctrl },
            { .name = "GLOBAL_CFG_BBHTX_FIFO_ADDR", .val = bdmf_address_global_cfg_bbhtx_fifo_addr },
            { .name = "GLOBAL_CFG_DQM_FULL", .val = bdmf_address_global_cfg_dqm_full },
            { .name = "GLOBAL_CFG_DQM_NOT_EMPTY", .val = bdmf_address_global_cfg_dqm_not_empty },
            { .name = "GLOBAL_CFG_DQM_POP_READY", .val = bdmf_address_global_cfg_dqm_pop_ready },
            { .name = "GLOBAL_CFG_AGGREGATION_CONTEXT_VALID", .val = bdmf_address_global_cfg_aggregation_context_valid },
            { .name = "GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE", .val = bdmf_address_global_cfg_qm_egress_flush_queue },
            { .name = "GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL", .val = bdmf_address_global_cfg_qm_aggregation_timer_ctrl },
            { .name = "GLOBAL_CFG_QM_FPM_UG_GBL_CNT", .val = bdmf_address_global_cfg_qm_fpm_ug_gbl_cnt },
            { .name = "GLOBAL_CFG_DDR_SPARE_ROOM", .val = bdmf_address_global_cfg_ddr_spare_room },
            { .name = "GLOBAL_CFG_DUMMY_SPARE_ROOM_PROFILE_ID", .val = bdmf_address_global_cfg_dummy_spare_room_profile_id },
            { .name = "GLOBAL_CFG_DQM_UBUS_CTRL", .val = bdmf_address_global_cfg_dqm_ubus_ctrl },
            { .name = "GLOBAL_CFG_MEM_AUTO_INIT", .val = bdmf_address_global_cfg_mem_auto_init },
            { .name = "GLOBAL_CFG_MEM_AUTO_INIT_STS", .val = bdmf_address_global_cfg_mem_auto_init_sts },
            { .name = "GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_SIZE_TOKENS", .val = bdmf_address_global_cfg_fpm_mpm_enhancement_pool_size_tokens },
            { .name = "GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_0_1_SIZE_BYTE", .val = bdmf_address_global_cfg_fpm_mpm_enhancement_pool_0_1_size_byte },
            { .name = "GLOBAL_CFG_FPM_MPM_ENHANCEMENT_POOL_2_3_SIZE_BYTE", .val = bdmf_address_global_cfg_fpm_mpm_enhancement_pool_2_3_size_byte },
            { .name = "GLOBAL_CFG_MC_CTRL", .val = bdmf_address_global_cfg_mc_ctrl },
            { .name = "GLOBAL_CFG_AQM_CLK_COUNTER_CYCLE", .val = bdmf_address_global_cfg_aqm_clk_counter_cycle },
            { .name = "GLOBAL_CFG_AQM_PUSH_TO_EMPTY_THR", .val = bdmf_address_global_cfg_aqm_push_to_empty_thr },
            { .name = "GLOBAL_CFG_QM_GENERAL_CTRL2", .val = bdmf_address_global_cfg_qm_general_ctrl2 },
            { .name = "FPM_POOLS_THR", .val = bdmf_address_fpm_pools_thr },
            { .name = "FPM_USR_GRP_LOWER_THR", .val = bdmf_address_fpm_usr_grp_lower_thr },
            { .name = "FPM_USR_GRP_MID_THR", .val = bdmf_address_fpm_usr_grp_mid_thr },
            { .name = "FPM_USR_GRP_HIGHER_THR", .val = bdmf_address_fpm_usr_grp_higher_thr },
            { .name = "FPM_USR_GRP_CNT", .val = bdmf_address_fpm_usr_grp_cnt },
            { .name = "RUNNER_GRP_RNR_CONFIG", .val = bdmf_address_runner_grp_rnr_config },
            { .name = "RUNNER_GRP_QUEUE_CONFIG", .val = bdmf_address_runner_grp_queue_config },
            { .name = "RUNNER_GRP_PDFIFO_CONFIG", .val = bdmf_address_runner_grp_pdfifo_config },
            { .name = "RUNNER_GRP_UPDATE_FIFO_CONFIG", .val = bdmf_address_runner_grp_update_fifo_config },
            { .name = "INTR_CTRL_ISR", .val = bdmf_address_intr_ctrl_isr },
            { .name = "INTR_CTRL_ISM", .val = bdmf_address_intr_ctrl_ism },
            { .name = "INTR_CTRL_IER", .val = bdmf_address_intr_ctrl_ier },
            { .name = "INTR_CTRL_ITR", .val = bdmf_address_intr_ctrl_itr },
            { .name = "CLK_GATE_CLK_GATE_CNTRL", .val = bdmf_address_clk_gate_clk_gate_cntrl },
            { .name = "CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL", .val = bdmf_address_cpu_indr_port_cpu_pd_indirect_ctrl },
            { .name = "CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0", .val = bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_0 },
            { .name = "CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1", .val = bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_1 },
            { .name = "CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2", .val = bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_2 },
            { .name = "CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3", .val = bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_3 },
            { .name = "CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0", .val = bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_0 },
            { .name = "CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1", .val = bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_1 },
            { .name = "CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2", .val = bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_2 },
            { .name = "CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3", .val = bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_3 },
            { .name = "QUEUE_CONTEXT_CONTEXT", .val = bdmf_address_queue_context_context },
            { .name = "WRED_PROFILE_COLOR_MIN_THR_0", .val = bdmf_address_wred_profile_color_min_thr_0 },
            { .name = "WRED_PROFILE_COLOR_MIN_THR_1", .val = bdmf_address_wred_profile_color_min_thr_1 },
            { .name = "WRED_PROFILE_COLOR_MAX_THR_0", .val = bdmf_address_wred_profile_color_max_thr_0 },
            { .name = "WRED_PROFILE_COLOR_MAX_THR_1", .val = bdmf_address_wred_profile_color_max_thr_1 },
            { .name = "WRED_PROFILE_COLOR_SLOPE_0", .val = bdmf_address_wred_profile_color_slope_0 },
            { .name = "WRED_PROFILE_COLOR_SLOPE_1", .val = bdmf_address_wred_profile_color_slope_1 },
            { .name = "COPY_DECISION_PROFILE_THR", .val = bdmf_address_copy_decision_profile_thr },
            { .name = "TIMESTAMP_RES_PROFILE_VALUE", .val = bdmf_address_timestamp_res_profile_value },
            { .name = "GLOBAL_EGRESS_DROP_COUNTER_COUNTER", .val = bdmf_address_global_egress_drop_counter_counter },
            { .name = "GLOBAL_EGRESS_AQM_DROP_COUNTER_COUNTER", .val = bdmf_address_global_egress_aqm_drop_counter_counter },
            { .name = "TOTAL_VALID_COUNTER_COUNTER", .val = bdmf_address_total_valid_counter_counter },
            { .name = "DQM_VALID_COUNTER_COUNTER", .val = bdmf_address_dqm_valid_counter_counter },
            { .name = "DROP_COUNTER_COUNTER", .val = bdmf_address_drop_counter_counter },
            { .name = "TOTAL_EGRESS_ACCUMULATED_COUNTER_COUNTER", .val = bdmf_address_total_egress_accumulated_counter_counter },
            { .name = "EPON_RPT_CNT_COUNTER", .val = bdmf_address_epon_rpt_cnt_counter },
            { .name = "EPON_RPT_CNT_QUEUE_STATUS", .val = bdmf_address_epon_rpt_cnt_queue_status },
            { .name = "RD_DATA_POOL0", .val = bdmf_address_rd_data_pool0 },
            { .name = "RD_DATA_POOL1", .val = bdmf_address_rd_data_pool1 },
            { .name = "RD_DATA_POOL2", .val = bdmf_address_rd_data_pool2 },
            { .name = "RD_DATA_POOL3", .val = bdmf_address_rd_data_pool3 },
            { .name = "POP_3", .val = bdmf_address_pop_3 },
            { .name = "PDFIFO_PTR", .val = bdmf_address_pdfifo_ptr },
            { .name = "UPDATE_FIFO_PTR", .val = bdmf_address_update_fifo_ptr },
            { .name = "RD_DATA_2", .val = bdmf_address_rd_data_2 },
            { .name = "POP_2", .val = bdmf_address_pop_2 },
            { .name = "RD_DATA_1", .val = bdmf_address_rd_data_1 },
            { .name = "POP_1", .val = bdmf_address_pop_1 },
            { .name = "RD_DATA", .val = bdmf_address_rd_data },
            { .name = "POP", .val = bdmf_address_pop },
            { .name = "CM_COMMON_INPUT_FIFO_DATA", .val = bdmf_address_cm_common_input_fifo_data },
            { .name = "NORMAL_RMT_FIFO_DATA", .val = bdmf_address_normal_rmt_fifo_data },
            { .name = "NON_DELAYED_RMT_FIFO_DATA", .val = bdmf_address_non_delayed_rmt_fifo_data },
            { .name = "EGRESS_DATA_FIFO_DATA", .val = bdmf_address_egress_data_fifo_data },
            { .name = "EGRESS_RR_FIFO_DATA", .val = bdmf_address_egress_rr_fifo_data },
            { .name = "EGRESS_BB_INPUT_FIFO_DATA", .val = bdmf_address_egress_bb_input_fifo_data },
            { .name = "EGRESS_BB_OUTPUT_FIFO_DATA", .val = bdmf_address_egress_bb_output_fifo_data },
            { .name = "BB_OUTPUT_FIFO_DATA", .val = bdmf_address_bb_output_fifo_data },
            { .name = "NON_DELAYED_OUT_FIFO_DATA", .val = bdmf_address_non_delayed_out_fifo_data },
            { .name = "BB0_EGR_MSG_OUT_FIFO_DATA", .val = bdmf_address_bb0_egr_msg_out_fifo_data },
            { .name = "FPM_BUFFER_RESERVATION_DATA", .val = bdmf_address_fpm_buffer_reservation_data },
            { .name = "PORT_CFG", .val = bdmf_address_port_cfg },
            { .name = "FC_UG_MASK_UG_EN", .val = bdmf_address_fc_ug_mask_ug_en },
            { .name = "FC_QUEUE_MASK", .val = bdmf_address_fc_queue_mask },
            { .name = "FC_QUEUE_RANGE1_START", .val = bdmf_address_fc_queue_range1_start },
            { .name = "FC_QUEUE_RANGE2_START", .val = bdmf_address_fc_queue_range2_start },
            { .name = "DBG", .val = bdmf_address_dbg },
            { .name = "UG_OCCUPANCY_STATUS", .val = bdmf_address_ug_occupancy_status },
            { .name = "QUEUE_RANGE1_OCCUPANCY_STATUS", .val = bdmf_address_queue_range1_occupancy_status },
            { .name = "QUEUE_RANGE2_OCCUPANCY_STATUS", .val = bdmf_address_queue_range2_occupancy_status },
            { .name = "CONTEXT_DATA", .val = bdmf_address_context_data },
            { .name = "DEBUG_SEL", .val = bdmf_address_debug_sel },
            { .name = "DEBUG_BUS_LSB", .val = bdmf_address_debug_bus_lsb },
            { .name = "DEBUG_BUS_MSB", .val = bdmf_address_debug_bus_msb },
            { .name = "QM_SPARE_CONFIG", .val = bdmf_address_qm_spare_config },
            { .name = "GOOD_LVL1_PKTS_CNT", .val = bdmf_address_good_lvl1_pkts_cnt },
            { .name = "GOOD_LVL1_BYTES_CNT", .val = bdmf_address_good_lvl1_bytes_cnt },
            { .name = "GOOD_LVL2_PKTS_CNT", .val = bdmf_address_good_lvl2_pkts_cnt },
            { .name = "GOOD_LVL2_BYTES_CNT", .val = bdmf_address_good_lvl2_bytes_cnt },
            { .name = "COPIED_PKTS_CNT", .val = bdmf_address_copied_pkts_cnt },
            { .name = "COPIED_BYTES_CNT", .val = bdmf_address_copied_bytes_cnt },
            { .name = "AGG_PKTS_CNT", .val = bdmf_address_agg_pkts_cnt },
            { .name = "AGG_BYTES_CNT", .val = bdmf_address_agg_bytes_cnt },
            { .name = "AGG_1_PKTS_CNT", .val = bdmf_address_agg_1_pkts_cnt },
            { .name = "AGG_2_PKTS_CNT", .val = bdmf_address_agg_2_pkts_cnt },
            { .name = "AGG_3_PKTS_CNT", .val = bdmf_address_agg_3_pkts_cnt },
            { .name = "AGG_4_PKTS_CNT", .val = bdmf_address_agg_4_pkts_cnt },
            { .name = "WRED_DROP_CNT", .val = bdmf_address_wred_drop_cnt },
            { .name = "FPM_CONGESTION_DROP_CNT", .val = bdmf_address_fpm_congestion_drop_cnt },
            { .name = "DDR_PD_CONGESTION_DROP_CNT", .val = bdmf_address_ddr_pd_congestion_drop_cnt },
            { .name = "DDR_BYTE_CONGESTION_DROP_CNT", .val = bdmf_address_ddr_byte_congestion_drop_cnt },
            { .name = "QM_PD_CONGESTION_DROP_CNT", .val = bdmf_address_qm_pd_congestion_drop_cnt },
            { .name = "QM_ABS_REQUEUE_CNT", .val = bdmf_address_qm_abs_requeue_cnt },
            { .name = "FPM_PREFETCH_FIFO0_STATUS", .val = bdmf_address_fpm_prefetch_fifo0_status },
            { .name = "FPM_PREFETCH_FIFO1_STATUS", .val = bdmf_address_fpm_prefetch_fifo1_status },
            { .name = "FPM_PREFETCH_FIFO2_STATUS", .val = bdmf_address_fpm_prefetch_fifo2_status },
            { .name = "FPM_PREFETCH_FIFO3_STATUS", .val = bdmf_address_fpm_prefetch_fifo3_status },
            { .name = "NORMAL_RMT_FIFO_STATUS", .val = bdmf_address_normal_rmt_fifo_status },
            { .name = "NON_DELAYED_RMT_FIFO_STATUS", .val = bdmf_address_non_delayed_rmt_fifo_status },
            { .name = "NON_DELAYED_OUT_FIFO_STATUS", .val = bdmf_address_non_delayed_out_fifo_status },
            { .name = "PRE_CM_FIFO_STATUS", .val = bdmf_address_pre_cm_fifo_status },
            { .name = "CM_RD_PD_FIFO_STATUS", .val = bdmf_address_cm_rd_pd_fifo_status },
            { .name = "CM_WR_PD_FIFO_STATUS", .val = bdmf_address_cm_wr_pd_fifo_status },
            { .name = "CM_COMMON_INPUT_FIFO_STATUS", .val = bdmf_address_cm_common_input_fifo_status },
            { .name = "BB0_OUTPUT_FIFO_STATUS", .val = bdmf_address_bb0_output_fifo_status },
            { .name = "BB1_OUTPUT_FIFO_STATUS", .val = bdmf_address_bb1_output_fifo_status },
            { .name = "BB1_INPUT_FIFO_STATUS", .val = bdmf_address_bb1_input_fifo_status },
            { .name = "EGRESS_DATA_FIFO_STATUS", .val = bdmf_address_egress_data_fifo_status },
            { .name = "EGRESS_RR_FIFO_STATUS", .val = bdmf_address_egress_rr_fifo_status },
            { .name = "BB_ROUTE_OVR", .val = bdmf_address_bb_route_ovr },
            { .name = "QM_INGRESS_STAT", .val = bdmf_address_qm_ingress_stat },
            { .name = "QM_EGRESS_STAT", .val = bdmf_address_qm_egress_stat },
            { .name = "QM_CM_STAT", .val = bdmf_address_qm_cm_stat },
            { .name = "QM_FPM_PREFETCH_STAT", .val = bdmf_address_qm_fpm_prefetch_stat },
            { .name = "QM_CONNECT_ACK_COUNTER", .val = bdmf_address_qm_connect_ack_counter },
            { .name = "QM_DDR_WR_REPLY_COUNTER", .val = bdmf_address_qm_ddr_wr_reply_counter },
            { .name = "QM_DDR_PIPE_BYTE_COUNTER", .val = bdmf_address_qm_ddr_pipe_byte_counter },
            { .name = "QM_ABS_REQUEUE_VALID_COUNTER", .val = bdmf_address_qm_abs_requeue_valid_counter },
            { .name = "QM_ILLEGAL_PD_CAPTURE", .val = bdmf_address_qm_illegal_pd_capture },
            { .name = "QM_INGRESS_PROCESSED_PD_CAPTURE", .val = bdmf_address_qm_ingress_processed_pd_capture },
            { .name = "QM_CM_PROCESSED_PD_CAPTURE", .val = bdmf_address_qm_cm_processed_pd_capture },
            { .name = "FPM_GRP_DROP_CNT", .val = bdmf_address_fpm_grp_drop_cnt },
            { .name = "FPM_POOL_DROP_CNT", .val = bdmf_address_fpm_pool_drop_cnt },
            { .name = "FPM_BUFFER_RES_DROP_CNT", .val = bdmf_address_fpm_buffer_res_drop_cnt },
            { .name = "PSRAM_EGRESS_CONG_DRP_CNT", .val = bdmf_address_psram_egress_cong_drp_cnt },
            { .name = "BACKPRESSURE", .val = bdmf_address_backpressure },
            { .name = "AQM_TIMESTAMP_CURR_COUNTER", .val = bdmf_address_aqm_timestamp_curr_counter },
            { .name = "BB0_EGR_MSG_OUT_FIFO_STATUS", .val = bdmf_address_bb0_egr_msg_out_fifo_status },
            { .name = "COUNT_PKT_NOT_PD_MODE_BITS", .val = bdmf_address_count_pkt_not_pd_mode_bits },
            { .name = "DATA", .val = bdmf_address_data },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_qm_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
