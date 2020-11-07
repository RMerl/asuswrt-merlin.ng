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

#include "rdp_common.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_qm_ag.h"

bdmf_error_t ag_drv_qm_ddr_cong_ctrl_set(const qm_ddr_cong_ctrl *ddr_cong_ctrl)
{
    uint32_t reg_global_cfg_ddr_byte_congestion_control=0;
    uint32_t reg_global_cfg_ddr_byte_congestion_lower_thr=0;
    uint32_t reg_global_cfg_ddr_byte_congestion_mid_thr=0;
    uint32_t reg_global_cfg_ddr_byte_congestion_higher_thr=0;
    uint32_t reg_global_cfg_ddr_pd_congestion_control=0;

#ifdef VALIDATE_PARMS
    if(!ddr_cong_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ddr_cong_ctrl->ddr_byte_congestion_drop_enable >= _1BITS_MAX_VAL_) ||
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
    if(!ddr_cong_ctrl)
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
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 288))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx / 32, QM, GLOBAL_CFG_DQM_NOT_EMPTY, reg_global_cfg_dqm_not_empty);

    *data = FIELD_GET(reg_global_cfg_dqm_not_empty, (q_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_is_queue_pop_ready_get(uint16_t q_idx, bdmf_boolean *data)
{
    uint32_t reg_global_cfg_dqm_pop_ready;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 288))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx / 32, QM, GLOBAL_CFG_DQM_POP_READY, reg_global_cfg_dqm_pop_ready);

    *data = FIELD_GET(reg_global_cfg_dqm_pop_ready, (q_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_is_queue_full_get(uint16_t q_idx, bdmf_boolean *data)
{
    uint32_t reg_global_cfg_dqm_full;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 288))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx / 32, QM, GLOBAL_CFG_DQM_FULL, reg_global_cfg_dqm_full);

    *data = FIELD_GET(reg_global_cfg_dqm_full, (q_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_ug_thr_set(uint8_t ug_grp_idx, const qm_fpm_ug_thr *fpm_ug_thr)
{
    uint32_t reg_fpm_usr_grp_lower_thr=0;
    uint32_t reg_fpm_usr_grp_mid_thr=0;
    uint32_t reg_fpm_usr_grp_higher_thr=0;

#ifdef VALIDATE_PARMS
    if(!fpm_ug_thr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ug_grp_idx >= 4))
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
    if(!fpm_ug_thr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ug_grp_idx >= 4))
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
    uint32_t reg_runner_grp_queue_config=0;
    uint32_t reg_runner_grp_pdfifo_config=0;
    uint32_t reg_runner_grp_update_fifo_config=0;
    uint32_t reg_runner_grp_rnr_config=0;

#ifdef VALIDATE_PARMS
    if(!rnr_group_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_idx >= 16) ||
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
    if(!rnr_group_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_idx >= 16))
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
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_wr_data_0=0;
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_wr_data_1=0;
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_wr_data_2=0;
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_wr_data_3=0;

#ifdef VALIDATE_PARMS
    if((indirect_grp_idx >= 4))
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
    if(!data0 || !data1 || !data2 || !data3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((indirect_grp_idx >= 4))
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
    if(!data0 || !data1 || !data2 || !data3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((indirect_grp_idx >= 4))
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
    if(!context_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 9))
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
    uint32_t reg_wred_profile_color_min_thr_0=0;
    uint32_t reg_wred_profile_color_min_thr_1=0;
    uint32_t reg_wred_profile_color_max_thr_0=0;
    uint32_t reg_wred_profile_color_max_thr_1=0;
    uint32_t reg_wred_profile_color_slope_0=0;
    uint32_t reg_wred_profile_color_slope_1=0;

#ifdef VALIDATE_PARMS
    if(!wred_profile_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((profile_idx >= 16) ||
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
    if(!wred_profile_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((profile_idx >= 16))
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

bdmf_error_t ag_drv_qm_ubus_slave_set(const qm_ubus_slave *ubus_slave)
{
    uint32_t reg_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_base=0;
    uint32_t reg_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_mask=0;
    uint32_t reg_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_base=0;
    uint32_t reg_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_mask=0;
    uint32_t reg_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_base=0;
    uint32_t reg_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_mask=0;

#ifdef VALIDATE_PARMS
    if(!ubus_slave)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    reg_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_base = RU_FIELD_SET(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE, BASE, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_base, ubus_slave->vpb_base);
    reg_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_mask = RU_FIELD_SET(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK, MASK, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_mask, ubus_slave->vpb_mask);
    reg_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_base = RU_FIELD_SET(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE, BASE, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_base, ubus_slave->apb_base);
    reg_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_mask = RU_FIELD_SET(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK, MASK, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_mask, ubus_slave->apb_mask);
    reg_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_base = RU_FIELD_SET(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE, BASE, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_base, ubus_slave->dqm_base);
    reg_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_mask = RU_FIELD_SET(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK, MASK, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_mask, ubus_slave->dqm_mask);

    RU_REG_WRITE(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_base);
    RU_REG_WRITE(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_mask);
    RU_REG_WRITE(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_base);
    RU_REG_WRITE(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_mask);
    RU_REG_WRITE(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_base);
    RU_REG_WRITE(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_ubus_slave_get(qm_ubus_slave *ubus_slave)
{
    uint32_t reg_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_base;
    uint32_t reg_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_mask;
    uint32_t reg_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_base;
    uint32_t reg_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_mask;
    uint32_t reg_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_base;
    uint32_t reg_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_mask;

#ifdef VALIDATE_PARMS
    if(!ubus_slave)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_base);
    RU_REG_READ(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_mask);
    RU_REG_READ(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_base);
    RU_REG_READ(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_mask);
    RU_REG_READ(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_base);
    RU_REG_READ(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_mask);

    ubus_slave->vpb_base = RU_FIELD_GET(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE, BASE, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_base);
    ubus_slave->vpb_mask = RU_FIELD_GET(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK, MASK, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_mask);
    ubus_slave->apb_base = RU_FIELD_GET(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE, BASE, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_base);
    ubus_slave->apb_mask = RU_FIELD_GET(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK, MASK, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_mask);
    ubus_slave->dqm_base = RU_FIELD_GET(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE, BASE, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_base);
    ubus_slave->dqm_mask = RU_FIELD_GET(0, QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK, MASK, reg_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_cfg_src_id_set(const qm_cfg_src_id *cfg_src_id)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx=0;
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx=0;

#ifdef VALIDATE_PARMS
    if(!cfg_src_id)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((cfg_src_id->fpmsrc >= _6BITS_MAX_VAL_) ||
       (cfg_src_id->sbpmsrc >= _6BITS_MAX_VAL_) ||
       (cfg_src_id->stsrnrsrc >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx);

    reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, FPMSRC, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx, cfg_src_id->fpmsrc);
    reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, SBPMSRC, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx, cfg_src_id->sbpmsrc);
    reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX, STSRNRSRC, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx, cfg_src_id->stsrnrsrc);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx);
    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_cfg_src_id_get(qm_cfg_src_id *cfg_src_id)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx;
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx;

#ifdef VALIDATE_PARMS
    if(!cfg_src_id)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx);

    cfg_src_id->fpmsrc = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, FPMSRC, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx);
    cfg_src_id->sbpmsrc = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, SBPMSRC, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx);
    cfg_src_id->stsrnrsrc = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX, STSRNRSRC, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_rnr_src_id_set(uint8_t pdrnr0src, uint8_t pdrnr1src)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx=0;

#ifdef VALIDATE_PARMS
    if((pdrnr0src >= _6BITS_MAX_VAL_) ||
       (pdrnr1src >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx);

    reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX, PDRNR0SRC, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx, pdrnr0src);
    reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX, PDRNR1SRC, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx, pdrnr1src);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_rnr_src_id_get(uint8_t *pdrnr0src, uint8_t *pdrnr1src)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx;

#ifdef VALIDATE_PARMS
    if(!pdrnr0src || !pdrnr1src)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx);

    *pdrnr0src = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX, PDRNR0SRC, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx);
    *pdrnr1src = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX, PDRNR1SRC, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_dma_cfg_set(const qm_bbh_dma_cfg *bbh_dma_cfg)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx=0;
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx=0;

#ifdef VALIDATE_PARMS
    if(!bbh_dma_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_dma_cfg->dmasrc >= _6BITS_MAX_VAL_) ||
       (bbh_dma_cfg->descbase >= _6BITS_MAX_VAL_) ||
       (bbh_dma_cfg->descsize >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx);

    reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, DMASRC, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx, bbh_dma_cfg->dmasrc);
    reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, DESCBASE, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx, bbh_dma_cfg->descbase);
    reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, DESCSIZE, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx, bbh_dma_cfg->descsize);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx);
    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_dma_cfg_get(qm_bbh_dma_cfg *bbh_dma_cfg)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx;
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx;

#ifdef VALIDATE_PARMS
    if(!bbh_dma_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx);

    bbh_dma_cfg->dmasrc = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, DMASRC, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx);
    bbh_dma_cfg->descbase = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, DESCBASE, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx);
    bbh_dma_cfg->descsize = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, DESCSIZE, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_max_otf_read_request_set(uint8_t maxreq)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx=0;

#ifdef VALIDATE_PARMS
    if((maxreq >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx);

    reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, MAXREQ, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx, maxreq);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_max_otf_read_request_get(uint8_t *maxreq)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx;

#ifdef VALIDATE_PARMS
    if(!maxreq)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx);

    *maxreq = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, MAXREQ, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_epon_urgent_set(bdmf_boolean epnurgnt)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx=0;

#ifdef VALIDATE_PARMS
    if((epnurgnt >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx);

    reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, EPNURGNT, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx, epnurgnt);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_epon_urgent_get(bdmf_boolean *epnurgnt)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx;

#ifdef VALIDATE_PARMS
    if(!epnurgnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx);

    *epnurgnt = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX, EPNURGNT, reg_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_sdma_cfg_set(const qm_bbh_sdma_cfg *bbh_sdma_cfg)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx=0;
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx=0;

#ifdef VALIDATE_PARMS
    if(!bbh_sdma_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_sdma_cfg->sdmasrc >= _6BITS_MAX_VAL_) ||
       (bbh_sdma_cfg->descbase >= _6BITS_MAX_VAL_) ||
       (bbh_sdma_cfg->descsize >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx);

    reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, SDMASRC, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx, bbh_sdma_cfg->sdmasrc);
    reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, DESCBASE, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx, bbh_sdma_cfg->descbase);
    reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, DESCSIZE, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx, bbh_sdma_cfg->descsize);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx);
    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_sdma_cfg_get(qm_bbh_sdma_cfg *bbh_sdma_cfg)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx;
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx;

#ifdef VALIDATE_PARMS
    if(!bbh_sdma_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx);

    bbh_sdma_cfg->sdmasrc = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX, SDMASRC, reg_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx);
    bbh_sdma_cfg->descbase = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, DESCBASE, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx);
    bbh_sdma_cfg->descsize = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, DESCSIZE, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_sdma_max_otf_read_request_set(uint8_t maxreq)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx=0;

#ifdef VALIDATE_PARMS
    if((maxreq >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx);

    reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, MAXREQ, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx, maxreq);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_sdma_max_otf_read_request_get(uint8_t *maxreq)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx;

#ifdef VALIDATE_PARMS
    if(!maxreq)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx);

    *maxreq = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, MAXREQ, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_sdma_epon_urgent_set(bdmf_boolean epnurgnt)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx=0;

#ifdef VALIDATE_PARMS
    if((epnurgnt >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx);

    reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, EPNURGNT, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx, epnurgnt);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_sdma_epon_urgent_get(bdmf_boolean *epnurgnt)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx;

#ifdef VALIDATE_PARMS
    if(!epnurgnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx);

    *epnurgnt = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX, EPNURGNT, reg_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_ddr_cfg_set(const qm_bbh_ddr_cfg *bbh_ddr_cfg)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx=0;

#ifdef VALIDATE_PARMS
    if(!bbh_ddr_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_ddr_cfg->bufsize >= _3BITS_MAX_VAL_) ||
       (bbh_ddr_cfg->byteresul >= _1BITS_MAX_VAL_) ||
       (bbh_ddr_cfg->ddrtxoffset >= _9BITS_MAX_VAL_) ||
       (bbh_ddr_cfg->hnsize0 >= _7BITS_MAX_VAL_) ||
       (bbh_ddr_cfg->hnsize1 >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX, BUFSIZE, reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx, bbh_ddr_cfg->bufsize);
    reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX, BYTERESUL, reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx, bbh_ddr_cfg->byteresul);
    reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX, DDRTXOFFSET, reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx, bbh_ddr_cfg->ddrtxoffset);
    reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX, HNSIZE0, reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx, bbh_ddr_cfg->hnsize0);
    reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX, HNSIZE1, reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx, bbh_ddr_cfg->hnsize1);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_ddr_cfg_get(qm_bbh_ddr_cfg *bbh_ddr_cfg)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx;

#ifdef VALIDATE_PARMS
    if(!bbh_ddr_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX, reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx);

    bbh_ddr_cfg->bufsize = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX, BUFSIZE, reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx);
    bbh_ddr_cfg->byteresul = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX, BYTERESUL, reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx);
    bbh_ddr_cfg->ddrtxoffset = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX, DDRTXOFFSET, reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx);
    bbh_ddr_cfg->hnsize0 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX, HNSIZE0, reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx);
    bbh_ddr_cfg->hnsize1 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX, HNSIZE1, reg_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_debug_counters_get(qm_debug_counters *debug_counters)
{
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_srampd;
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_ddrpd;
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_pddrop;
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_stscnt;
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_stsdrop;
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_msgcnt;
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_msgdrop;
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_getnextnull;
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_lenerr;
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_aggrlenerr;
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_srampkt;
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_ddrpkt;
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_flushpkts;

#ifdef VALIDATE_PARMS
    if(!debug_counters)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD, reg_bbh_tx_qm_bbhtx_debug_counters_srampd);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD, reg_bbh_tx_qm_bbhtx_debug_counters_ddrpd);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP, reg_bbh_tx_qm_bbhtx_debug_counters_pddrop);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT, reg_bbh_tx_qm_bbhtx_debug_counters_stscnt);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP, reg_bbh_tx_qm_bbhtx_debug_counters_stsdrop);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT, reg_bbh_tx_qm_bbhtx_debug_counters_msgcnt);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP, reg_bbh_tx_qm_bbhtx_debug_counters_msgdrop);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL, reg_bbh_tx_qm_bbhtx_debug_counters_getnextnull);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR, reg_bbh_tx_qm_bbhtx_debug_counters_lenerr);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR, reg_bbh_tx_qm_bbhtx_debug_counters_aggrlenerr);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT, reg_bbh_tx_qm_bbhtx_debug_counters_srampkt);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT, reg_bbh_tx_qm_bbhtx_debug_counters_ddrpkt);
    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS, reg_bbh_tx_qm_bbhtx_debug_counters_flushpkts);

    debug_counters->srampd = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD, SRAMPD, reg_bbh_tx_qm_bbhtx_debug_counters_srampd);
    debug_counters->ddrpd = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD, DDRPD, reg_bbh_tx_qm_bbhtx_debug_counters_ddrpd);
    debug_counters->pddrop = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP, PDDROP, reg_bbh_tx_qm_bbhtx_debug_counters_pddrop);
    debug_counters->stscnt = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT, STSCNT, reg_bbh_tx_qm_bbhtx_debug_counters_stscnt);
    debug_counters->stsdrop = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP, STSDROP, reg_bbh_tx_qm_bbhtx_debug_counters_stsdrop);
    debug_counters->msgcnt = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT, MSGCNT, reg_bbh_tx_qm_bbhtx_debug_counters_msgcnt);
    debug_counters->msgdrop = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP, MSGDROP, reg_bbh_tx_qm_bbhtx_debug_counters_msgdrop);
    debug_counters->getnextnull = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL, GETNEXTNULL, reg_bbh_tx_qm_bbhtx_debug_counters_getnextnull);
    debug_counters->lenerr = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR, LENERR, reg_bbh_tx_qm_bbhtx_debug_counters_lenerr);
    debug_counters->aggrlenerr = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR, AGGRLENERR, reg_bbh_tx_qm_bbhtx_debug_counters_aggrlenerr);
    debug_counters->srampkt = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT, SRAMPKT, reg_bbh_tx_qm_bbhtx_debug_counters_srampkt);
    debug_counters->ddrpkt = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT, DDRPKT, reg_bbh_tx_qm_bbhtx_debug_counters_ddrpkt);
    debug_counters->flshpkts = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS, FLSHPKTS, reg_bbh_tx_qm_bbhtx_debug_counters_flushpkts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_debug_info_set(const qm_debug_info *debug_info)
{
    uint32_t reg_dma_qm_dma_debug_nempty=0;
    uint32_t reg_dma_qm_dma_debug_urgnt=0;
    uint32_t reg_dma_qm_dma_debug_selsrc=0;
    uint32_t reg_dma_qm_dma_debug_rdadd=0;
    uint32_t reg_dma_qm_dma_debug_rdvalid=0;
    uint32_t reg_dma_qm_dma_debug_rddatardy=0;

#ifdef VALIDATE_PARMS
    if(!debug_info)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((debug_info->sel_src >= _6BITS_MAX_VAL_) ||
       (debug_info->address >= _10BITS_MAX_VAL_) ||
       (debug_info->datacs >= _1BITS_MAX_VAL_) ||
       (debug_info->cdcs >= _1BITS_MAX_VAL_) ||
       (debug_info->rrcs >= _1BITS_MAX_VAL_) ||
       (debug_info->valid >= _1BITS_MAX_VAL_) ||
       (debug_info->ready >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dma_qm_dma_debug_nempty = RU_FIELD_SET(0, QM, DMA_QM_DMA_DEBUG_NEMPTY, NEMPTY, reg_dma_qm_dma_debug_nempty, debug_info->nempty);
    reg_dma_qm_dma_debug_urgnt = RU_FIELD_SET(0, QM, DMA_QM_DMA_DEBUG_URGNT, URGNT, reg_dma_qm_dma_debug_urgnt, debug_info->urgnt);
    reg_dma_qm_dma_debug_selsrc = RU_FIELD_SET(0, QM, DMA_QM_DMA_DEBUG_SELSRC, SEL_SRC, reg_dma_qm_dma_debug_selsrc, debug_info->sel_src);
    reg_dma_qm_dma_debug_rdadd = RU_FIELD_SET(0, QM, DMA_QM_DMA_DEBUG_RDADD, ADDRESS, reg_dma_qm_dma_debug_rdadd, debug_info->address);
    reg_dma_qm_dma_debug_rdadd = RU_FIELD_SET(0, QM, DMA_QM_DMA_DEBUG_RDADD, DATACS, reg_dma_qm_dma_debug_rdadd, debug_info->datacs);
    reg_dma_qm_dma_debug_rdadd = RU_FIELD_SET(0, QM, DMA_QM_DMA_DEBUG_RDADD, CDCS, reg_dma_qm_dma_debug_rdadd, debug_info->cdcs);
    reg_dma_qm_dma_debug_rdadd = RU_FIELD_SET(0, QM, DMA_QM_DMA_DEBUG_RDADD, RRCS, reg_dma_qm_dma_debug_rdadd, debug_info->rrcs);
    reg_dma_qm_dma_debug_rdvalid = RU_FIELD_SET(0, QM, DMA_QM_DMA_DEBUG_RDVALID, VALID, reg_dma_qm_dma_debug_rdvalid, debug_info->valid);
    reg_dma_qm_dma_debug_rddatardy = RU_FIELD_SET(0, QM, DMA_QM_DMA_DEBUG_RDDATARDY, READY, reg_dma_qm_dma_debug_rddatardy, debug_info->ready);

    RU_REG_WRITE(0, QM, DMA_QM_DMA_DEBUG_NEMPTY, reg_dma_qm_dma_debug_nempty);
    RU_REG_WRITE(0, QM, DMA_QM_DMA_DEBUG_URGNT, reg_dma_qm_dma_debug_urgnt);
    RU_REG_WRITE(0, QM, DMA_QM_DMA_DEBUG_SELSRC, reg_dma_qm_dma_debug_selsrc);
    RU_REG_WRITE(0, QM, DMA_QM_DMA_DEBUG_RDADD, reg_dma_qm_dma_debug_rdadd);
    RU_REG_WRITE(0, QM, DMA_QM_DMA_DEBUG_RDVALID, reg_dma_qm_dma_debug_rdvalid);
    RU_REG_WRITE(0, QM, DMA_QM_DMA_DEBUG_RDDATARDY, reg_dma_qm_dma_debug_rddatardy);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_debug_info_get(qm_debug_info *debug_info)
{
    uint32_t reg_dma_qm_dma_debug_nempty;
    uint32_t reg_dma_qm_dma_debug_urgnt;
    uint32_t reg_dma_qm_dma_debug_selsrc;
    uint32_t reg_dma_qm_dma_debug_rdadd;
    uint32_t reg_dma_qm_dma_debug_rdvalid;
    uint32_t reg_dma_qm_dma_debug_rddatardy;

#ifdef VALIDATE_PARMS
    if(!debug_info)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, DMA_QM_DMA_DEBUG_NEMPTY, reg_dma_qm_dma_debug_nempty);
    RU_REG_READ(0, QM, DMA_QM_DMA_DEBUG_URGNT, reg_dma_qm_dma_debug_urgnt);
    RU_REG_READ(0, QM, DMA_QM_DMA_DEBUG_SELSRC, reg_dma_qm_dma_debug_selsrc);
    RU_REG_READ(0, QM, DMA_QM_DMA_DEBUG_RDADD, reg_dma_qm_dma_debug_rdadd);
    RU_REG_READ(0, QM, DMA_QM_DMA_DEBUG_RDVALID, reg_dma_qm_dma_debug_rdvalid);
    RU_REG_READ(0, QM, DMA_QM_DMA_DEBUG_RDDATARDY, reg_dma_qm_dma_debug_rddatardy);

    debug_info->nempty = RU_FIELD_GET(0, QM, DMA_QM_DMA_DEBUG_NEMPTY, NEMPTY, reg_dma_qm_dma_debug_nempty);
    debug_info->urgnt = RU_FIELD_GET(0, QM, DMA_QM_DMA_DEBUG_URGNT, URGNT, reg_dma_qm_dma_debug_urgnt);
    debug_info->sel_src = RU_FIELD_GET(0, QM, DMA_QM_DMA_DEBUG_SELSRC, SEL_SRC, reg_dma_qm_dma_debug_selsrc);
    debug_info->address = RU_FIELD_GET(0, QM, DMA_QM_DMA_DEBUG_RDADD, ADDRESS, reg_dma_qm_dma_debug_rdadd);
    debug_info->datacs = RU_FIELD_GET(0, QM, DMA_QM_DMA_DEBUG_RDADD, DATACS, reg_dma_qm_dma_debug_rdadd);
    debug_info->cdcs = RU_FIELD_GET(0, QM, DMA_QM_DMA_DEBUG_RDADD, CDCS, reg_dma_qm_dma_debug_rdadd);
    debug_info->rrcs = RU_FIELD_GET(0, QM, DMA_QM_DMA_DEBUG_RDADD, RRCS, reg_dma_qm_dma_debug_rdadd);
    debug_info->valid = RU_FIELD_GET(0, QM, DMA_QM_DMA_DEBUG_RDVALID, VALID, reg_dma_qm_dma_debug_rdvalid);
    debug_info->ready = RU_FIELD_GET(0, QM, DMA_QM_DMA_DEBUG_RDDATARDY, READY, reg_dma_qm_dma_debug_rddatardy);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_enable_ctrl_set(const qm_enable_ctrl *enable_ctrl)
{
    uint32_t reg_global_cfg_qm_enable_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!enable_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((enable_ctrl->fpm_prefetch_enable >= _1BITS_MAX_VAL_) ||
       (enable_ctrl->reorder_credit_enable >= _1BITS_MAX_VAL_) ||
       (enable_ctrl->dqm_pop_enable >= _1BITS_MAX_VAL_) ||
       (enable_ctrl->rmt_fixed_arb_enable >= _1BITS_MAX_VAL_) ||
       (enable_ctrl->dqm_push_fixed_arb_enable >= _1BITS_MAX_VAL_))
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

    RU_REG_WRITE(0, QM, GLOBAL_CFG_QM_ENABLE_CTRL, reg_global_cfg_qm_enable_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_enable_ctrl_get(qm_enable_ctrl *enable_ctrl)
{
    uint32_t reg_global_cfg_qm_enable_ctrl;

#ifdef VALIDATE_PARMS
    if(!enable_ctrl)
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

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_reset_ctrl_set(const qm_reset_ctrl *reset_ctrl)
{
    uint32_t reg_global_cfg_qm_sw_rst_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!reset_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((reset_ctrl->fpm_prefetch0_sw_rst >= _1BITS_MAX_VAL_) ||
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
       (reset_ctrl->non_delayed_out_fifo_sw_rst >= _1BITS_MAX_VAL_))
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

    RU_REG_WRITE(0, QM, GLOBAL_CFG_QM_SW_RST_CTRL, reg_global_cfg_qm_sw_rst_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_reset_ctrl_get(qm_reset_ctrl *reset_ctrl)
{
    uint32_t reg_global_cfg_qm_sw_rst_ctrl;

#ifdef VALIDATE_PARMS
    if(!reset_ctrl)
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

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_drop_counters_ctrl_set(const qm_drop_counters_ctrl *drop_counters_ctrl)
{
    uint32_t reg_global_cfg_qm_general_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!drop_counters_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((drop_counters_ctrl->read_clear_pkts >= _1BITS_MAX_VAL_) ||
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
       (drop_counters_ctrl->exclusive_dont_drop_bp_en >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->gpon_dbr_ceil >= _1BITS_MAX_VAL_))
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
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, EXCLUSIVE_DONT_DROP_BP_EN, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->exclusive_dont_drop_bp_en);
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, GPON_DBR_CEIL, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->gpon_dbr_ceil);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, reg_global_cfg_qm_general_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_drop_counters_ctrl_get(qm_drop_counters_ctrl *drop_counters_ctrl)
{
    uint32_t reg_global_cfg_qm_general_ctrl;

#ifdef VALIDATE_PARMS
    if(!drop_counters_ctrl)
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
    drop_counters_ctrl->exclusive_dont_drop_bp_en = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, EXCLUSIVE_DONT_DROP_BP_EN, reg_global_cfg_qm_general_ctrl);
    drop_counters_ctrl->gpon_dbr_ceil = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, GPON_DBR_CEIL, reg_global_cfg_qm_general_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_ctrl_set(bdmf_boolean fpm_pool_bp_enable, bdmf_boolean fpm_congestion_bp_enable, uint8_t fpm_prefetch_min_pool_size, uint8_t fpm_prefetch_pending_req_limit)
{
    uint32_t reg_global_cfg_fpm_control=0;

#ifdef VALIDATE_PARMS
    if((fpm_pool_bp_enable >= _1BITS_MAX_VAL_) ||
       (fpm_congestion_bp_enable >= _1BITS_MAX_VAL_) ||
       (fpm_prefetch_min_pool_size >= _2BITS_MAX_VAL_) ||
       (fpm_prefetch_pending_req_limit >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_fpm_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_POOL_BP_ENABLE, reg_global_cfg_fpm_control, fpm_pool_bp_enable);
    reg_global_cfg_fpm_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_CONGESTION_BP_ENABLE, reg_global_cfg_fpm_control, fpm_congestion_bp_enable);
    reg_global_cfg_fpm_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_PREFETCH_MIN_POOL_SIZE, reg_global_cfg_fpm_control, fpm_prefetch_min_pool_size);
    reg_global_cfg_fpm_control = RU_FIELD_SET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_PREFETCH_PENDING_REQ_LIMIT, reg_global_cfg_fpm_control, fpm_prefetch_pending_req_limit);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_FPM_CONTROL, reg_global_cfg_fpm_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_ctrl_get(bdmf_boolean *fpm_pool_bp_enable, bdmf_boolean *fpm_congestion_bp_enable, uint8_t *fpm_prefetch_min_pool_size, uint8_t *fpm_prefetch_pending_req_limit)
{
    uint32_t reg_global_cfg_fpm_control;

#ifdef VALIDATE_PARMS
    if(!fpm_pool_bp_enable || !fpm_congestion_bp_enable || !fpm_prefetch_min_pool_size || !fpm_prefetch_pending_req_limit)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_FPM_CONTROL, reg_global_cfg_fpm_control);

    *fpm_pool_bp_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_POOL_BP_ENABLE, reg_global_cfg_fpm_control);
    *fpm_congestion_bp_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_CONGESTION_BP_ENABLE, reg_global_cfg_fpm_control);
    *fpm_prefetch_min_pool_size = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_PREFETCH_MIN_POOL_SIZE, reg_global_cfg_fpm_control);
    *fpm_prefetch_pending_req_limit = RU_FIELD_GET(0, QM, GLOBAL_CFG_FPM_CONTROL, FPM_PREFETCH_PENDING_REQ_LIMIT, reg_global_cfg_fpm_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_pd_cong_ctrl_set(uint32_t total_pd_thr)
{
    uint32_t reg_global_cfg_qm_pd_congestion_control=0;

#ifdef VALIDATE_PARMS
    if((total_pd_thr >= _28BITS_MAX_VAL_))
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
    if(!total_pd_thr)
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
    uint32_t reg_global_cfg_abs_drop_queue=0;

#ifdef VALIDATE_PARMS
    if((abs_drop_queue >= _9BITS_MAX_VAL_) ||
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
    if(!abs_drop_queue || !abs_drop_queue_en)
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

bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl_set(uint16_t max_agg_bytes, uint8_t max_agg_pkts)
{
    uint32_t reg_global_cfg_aggregation_ctrl=0;

#ifdef VALIDATE_PARMS
    if((max_agg_bytes >= _10BITS_MAX_VAL_) ||
       (max_agg_pkts >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_aggregation_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, MAX_AGG_BYTES, reg_global_cfg_aggregation_ctrl, max_agg_bytes);
    reg_global_cfg_aggregation_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, MAX_AGG_PKTS, reg_global_cfg_aggregation_ctrl, max_agg_pkts);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, reg_global_cfg_aggregation_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl_get(uint16_t *max_agg_bytes, uint8_t *max_agg_pkts)
{
    uint32_t reg_global_cfg_aggregation_ctrl;

#ifdef VALIDATE_PARMS
    if(!max_agg_bytes || !max_agg_pkts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, reg_global_cfg_aggregation_ctrl);

    *max_agg_bytes = RU_FIELD_GET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, MAX_AGG_BYTES, reg_global_cfg_aggregation_ctrl);
    *max_agg_pkts = RU_FIELD_GET(0, QM, GLOBAL_CFG_AGGREGATION_CTRL, MAX_AGG_PKTS, reg_global_cfg_aggregation_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_base_addr_set(uint32_t fpm_base_addr)
{
    uint32_t reg_global_cfg_fpm_base_addr=0;

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
    if(!fpm_base_addr)
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
    uint32_t reg_global_cfg_fpm_coherent_base_addr=0;

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
    if(!fpm_base_addr)
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
    uint32_t reg_global_cfg_ddr_sop_offset=0;

#ifdef VALIDATE_PARMS
    if((ddr_sop_offset0 >= _11BITS_MAX_VAL_) ||
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
    if(!ddr_sop_offset0 || !ddr_sop_offset1)
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
    uint32_t reg_global_cfg_epon_overhead_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!epon_overhead_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((epon_overhead_ctrl->epon_line_rate >= _1BITS_MAX_VAL_) ||
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
    if(!epon_overhead_ctrl)
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

bdmf_error_t ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set(uint8_t prescaler_granularity, uint8_t aggregation_timeout_value)
{
    uint32_t reg_global_cfg_qm_aggregation_timer_ctrl=0;

#ifdef VALIDATE_PARMS
    if((prescaler_granularity >= _3BITS_MAX_VAL_) ||
       (aggregation_timeout_value >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_qm_aggregation_timer_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, PRESCALER_GRANULARITY, reg_global_cfg_qm_aggregation_timer_ctrl, prescaler_granularity);
    reg_global_cfg_qm_aggregation_timer_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, AGGREGATION_TIMEOUT_VALUE, reg_global_cfg_qm_aggregation_timer_ctrl, aggregation_timeout_value);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, reg_global_cfg_qm_aggregation_timer_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_get(uint8_t *prescaler_granularity, uint8_t *aggregation_timeout_value)
{
    uint32_t reg_global_cfg_qm_aggregation_timer_ctrl;

#ifdef VALIDATE_PARMS
    if(!prescaler_granularity || !aggregation_timeout_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, reg_global_cfg_qm_aggregation_timer_ctrl);

    *prescaler_granularity = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, PRESCALER_GRANULARITY, reg_global_cfg_qm_aggregation_timer_ctrl);
    *aggregation_timeout_value = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL, AGGREGATION_TIMEOUT_VALUE, reg_global_cfg_qm_aggregation_timer_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_buffer_grp_res_set(uint32_t idx, uint8_t res_thr_0, uint8_t res_thr_1, uint8_t res_thr_2, uint8_t res_thr_3)
{
    uint32_t reg_global_cfg_qm_fpm_buffer_grp_res=0;

#ifdef VALIDATE_PARMS
    if((idx >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_qm_fpm_buffer_grp_res = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES, RES_THR_0, reg_global_cfg_qm_fpm_buffer_grp_res, res_thr_0);
    reg_global_cfg_qm_fpm_buffer_grp_res = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES, RES_THR_1, reg_global_cfg_qm_fpm_buffer_grp_res, res_thr_1);
    reg_global_cfg_qm_fpm_buffer_grp_res = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES, RES_THR_2, reg_global_cfg_qm_fpm_buffer_grp_res, res_thr_2);
    reg_global_cfg_qm_fpm_buffer_grp_res = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES, RES_THR_3, reg_global_cfg_qm_fpm_buffer_grp_res, res_thr_3);

    RU_REG_RAM_WRITE(0, idx, QM, GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES, reg_global_cfg_qm_fpm_buffer_grp_res);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_buffer_grp_res_get(uint32_t idx, uint8_t *res_thr_0, uint8_t *res_thr_1, uint8_t *res_thr_2, uint8_t *res_thr_3)
{
    uint32_t reg_global_cfg_qm_fpm_buffer_grp_res;

#ifdef VALIDATE_PARMS
    if(!res_thr_0 || !res_thr_1 || !res_thr_2 || !res_thr_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES, reg_global_cfg_qm_fpm_buffer_grp_res);

    *res_thr_0 = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES, RES_THR_0, reg_global_cfg_qm_fpm_buffer_grp_res);
    *res_thr_1 = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES, RES_THR_1, reg_global_cfg_qm_fpm_buffer_grp_res);
    *res_thr_2 = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES, RES_THR_2, reg_global_cfg_qm_fpm_buffer_grp_res);
    *res_thr_3 = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES, RES_THR_3, reg_global_cfg_qm_fpm_buffer_grp_res);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_buffer_gbl_thr_set(uint16_t res_thr_global)
{
    uint32_t reg_global_cfg_qm_fpm_buffer_gbl_thr=0;

#ifdef VALIDATE_PARMS
#endif

    reg_global_cfg_qm_fpm_buffer_gbl_thr = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR, RES_THR_GLOBAL, reg_global_cfg_qm_fpm_buffer_gbl_thr, res_thr_global);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR, reg_global_cfg_qm_fpm_buffer_gbl_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_buffer_gbl_thr_get(uint16_t *res_thr_global)
{
    uint32_t reg_global_cfg_qm_fpm_buffer_gbl_thr;

#ifdef VALIDATE_PARMS
    if(!res_thr_global)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR, reg_global_cfg_qm_fpm_buffer_gbl_thr);

    *res_thr_global = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR, RES_THR_GLOBAL, reg_global_cfg_qm_fpm_buffer_gbl_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_flow_ctrl_rnr_cfg_set(uint8_t rnr_bb_id, uint8_t rnr_task, bdmf_boolean rnr_enable)
{
    uint32_t reg_global_cfg_qm_flow_ctrl_rnr_cfg=0;

#ifdef VALIDATE_PARMS
    if((rnr_bb_id >= _6BITS_MAX_VAL_) ||
       (rnr_task >= _4BITS_MAX_VAL_) ||
       (rnr_enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_qm_flow_ctrl_rnr_cfg = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG, RNR_BB_ID, reg_global_cfg_qm_flow_ctrl_rnr_cfg, rnr_bb_id);
    reg_global_cfg_qm_flow_ctrl_rnr_cfg = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG, RNR_TASK, reg_global_cfg_qm_flow_ctrl_rnr_cfg, rnr_task);
    reg_global_cfg_qm_flow_ctrl_rnr_cfg = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG, RNR_ENABLE, reg_global_cfg_qm_flow_ctrl_rnr_cfg, rnr_enable);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG, reg_global_cfg_qm_flow_ctrl_rnr_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_flow_ctrl_rnr_cfg_get(uint8_t *rnr_bb_id, uint8_t *rnr_task, bdmf_boolean *rnr_enable)
{
    uint32_t reg_global_cfg_qm_flow_ctrl_rnr_cfg;

#ifdef VALIDATE_PARMS
    if(!rnr_bb_id || !rnr_task || !rnr_enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG, reg_global_cfg_qm_flow_ctrl_rnr_cfg);

    *rnr_bb_id = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG, RNR_BB_ID, reg_global_cfg_qm_flow_ctrl_rnr_cfg);
    *rnr_task = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG, RNR_TASK, reg_global_cfg_qm_flow_ctrl_rnr_cfg);
    *rnr_enable = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG, RNR_ENABLE, reg_global_cfg_qm_flow_ctrl_rnr_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_flow_ctrl_intr_set(uint8_t qm_flow_ctrl_intr)
{
    uint32_t reg_global_cfg_qm_flow_ctrl_intr=0;

#ifdef VALIDATE_PARMS
    if((qm_flow_ctrl_intr >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg_qm_flow_ctrl_intr = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_FLOW_CTRL_INTR, QM_FLOW_CTRL_INTR, reg_global_cfg_qm_flow_ctrl_intr, qm_flow_ctrl_intr);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_QM_FLOW_CTRL_INTR, reg_global_cfg_qm_flow_ctrl_intr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_flow_ctrl_intr_get(uint8_t *qm_flow_ctrl_intr)
{
    uint32_t reg_global_cfg_qm_flow_ctrl_intr;

#ifdef VALIDATE_PARMS
    if(!qm_flow_ctrl_intr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_QM_FLOW_CTRL_INTR, reg_global_cfg_qm_flow_ctrl_intr);

    *qm_flow_ctrl_intr = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_FLOW_CTRL_INTR, QM_FLOW_CTRL_INTR, reg_global_cfg_qm_flow_ctrl_intr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_set(uint32_t fpm_gbl_cnt)
{
    uint32_t reg_global_cfg_qm_fpm_ug_gbl_cnt=0;

#ifdef VALIDATE_PARMS
    if((fpm_gbl_cnt >= _17BITS_MAX_VAL_))
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
    if(!fpm_gbl_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG_QM_FPM_UG_GBL_CNT, reg_global_cfg_qm_fpm_ug_gbl_cnt);

    *fpm_gbl_cnt = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_FPM_UG_GBL_CNT, FPM_GBL_CNT, reg_global_cfg_qm_fpm_ug_gbl_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_egress_flush_queue_set(uint16_t queue_num, bdmf_boolean flush_en)
{
    uint32_t reg_global_cfg_qm_egress_flush_queue=0;

#ifdef VALIDATE_PARMS
    if((queue_num >= _9BITS_MAX_VAL_) ||
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
    if(!queue_num || !flush_en)
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

bdmf_error_t ag_drv_qm_fpm_pool_thr_set(uint8_t pool_idx, const qm_fpm_pool_thr *fpm_pool_thr)
{
    uint32_t reg_fpm_pools_thr=0;

#ifdef VALIDATE_PARMS
    if(!fpm_pool_thr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pool_idx >= 4) ||
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
    if(!fpm_pool_thr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pool_idx >= 4))
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

bdmf_error_t ag_drv_qm_fpm_ug_cnt_set(uint8_t grp_idx, uint16_t fpm_ug_cnt)
{
    uint32_t reg_fpm_usr_grp_cnt=0;

#ifdef VALIDATE_PARMS
    if((grp_idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpm_usr_grp_cnt = RU_FIELD_SET(0, QM, FPM_USR_GRP_CNT, FPM_UG_CNT, reg_fpm_usr_grp_cnt, fpm_ug_cnt);

    RU_REG_RAM_WRITE(0, grp_idx, QM, FPM_USR_GRP_CNT, reg_fpm_usr_grp_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_ug_cnt_get(uint8_t grp_idx, uint16_t *fpm_ug_cnt)
{
    uint32_t reg_fpm_usr_grp_cnt;

#ifdef VALIDATE_PARMS
    if(!fpm_ug_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((grp_idx >= 4))
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
    uint32_t reg_intr_ctrl_isr=0;

#ifdef VALIDATE_PARMS
    if(!intr_ctrl_isr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((intr_ctrl_isr->qm_dqm_pop_on_empty >= _1BITS_MAX_VAL_) ||
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
       (intr_ctrl_isr->qm_timer_wraparound >= _1BITS_MAX_VAL_))
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

    RU_REG_WRITE(0, QM, INTR_CTRL_ISR, reg_intr_ctrl_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_intr_ctrl_isr_get(qm_intr_ctrl_isr *intr_ctrl_isr)
{
    uint32_t reg_intr_ctrl_isr;

#ifdef VALIDATE_PARMS
    if(!intr_ctrl_isr)
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

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_intr_ctrl_ism_get(uint32_t *ism)
{
    uint32_t reg_intr_ctrl_ism;

#ifdef VALIDATE_PARMS
    if(!ism)
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
    uint32_t reg_intr_ctrl_ier=0;

#ifdef VALIDATE_PARMS
    if((iem >= _21BITS_MAX_VAL_))
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
    if(!iem)
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
    uint32_t reg_intr_ctrl_itr=0;

#ifdef VALIDATE_PARMS
    if((ist >= _21BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_intr_ctrl_itr = RU_FIELD_SET(0, QM, INTR_CTRL_ITR, IST, reg_intr_ctrl_itr, ist);

    RU_REG_WRITE(0, QM, INTR_CTRL_ITR, reg_intr_ctrl_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_clk_gate_clk_gate_cntrl_set(const qm_clk_gate_clk_gate_cntrl *clk_gate_clk_gate_cntrl)
{
    uint32_t reg_clk_gate_clk_gate_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!clk_gate_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((clk_gate_clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
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
    if(!clk_gate_clk_gate_cntrl)
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
    uint32_t reg_cpu_indr_port_cpu_pd_indirect_ctrl=0;

#ifdef VALIDATE_PARMS
    if((indirect_grp_idx >= 4) ||
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
    if(!queue_num || !cmd || !done || !error)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((indirect_grp_idx >= 4))
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
    uint32_t reg_queue_context_context=0;

#ifdef VALIDATE_PARMS
    if(!q_context)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 96) ||
       (q_context->wred_profile >= _4BITS_MAX_VAL_) ||
       (q_context->copy_dec_profile >= _3BITS_MAX_VAL_) ||
       (q_context->copy_to_ddr >= _1BITS_MAX_VAL_) ||
       (q_context->ddr_copy_disable >= _1BITS_MAX_VAL_) ||
       (q_context->aggregation_disable >= _1BITS_MAX_VAL_) ||
       (q_context->fpm_ug >= _3BITS_MAX_VAL_) ||
       (q_context->exclusive_priority >= _1BITS_MAX_VAL_) ||
       (q_context->q_802_1ae >= _1BITS_MAX_VAL_) ||
       (q_context->sci >= _1BITS_MAX_VAL_) ||
       (q_context->fec_enable >= _1BITS_MAX_VAL_) ||
       (q_context->res_profile >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, WRED_PROFILE, reg_queue_context_context, q_context->wred_profile);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, COPY_DEC_PROFILE, reg_queue_context_context, q_context->copy_dec_profile);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, COPY_TO_DDR, reg_queue_context_context, q_context->copy_to_ddr);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, DDR_COPY_DISABLE, reg_queue_context_context, q_context->ddr_copy_disable);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, AGGREGATION_DISABLE, reg_queue_context_context, q_context->aggregation_disable);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, FPM_UG, reg_queue_context_context, q_context->fpm_ug);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, EXCLUSIVE_PRIORITY, reg_queue_context_context, q_context->exclusive_priority);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, Q_802_1AE, reg_queue_context_context, q_context->q_802_1ae);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, SCI, reg_queue_context_context, q_context->sci);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, FEC_ENABLE, reg_queue_context_context, q_context->fec_enable);
    reg_queue_context_context = RU_FIELD_SET(0, QM, QUEUE_CONTEXT_CONTEXT, RES_PROFILE, reg_queue_context_context, q_context->res_profile);

    RU_REG_RAM_WRITE(0, q_idx, QM, QUEUE_CONTEXT_CONTEXT, reg_queue_context_context);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_q_context_get(uint16_t q_idx, qm_q_context *q_context)
{
    uint32_t reg_queue_context_context;

#ifdef VALIDATE_PARMS
    if(!q_context)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 96))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, QM, QUEUE_CONTEXT_CONTEXT, reg_queue_context_context);

    q_context->wred_profile = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, WRED_PROFILE, reg_queue_context_context);
    q_context->copy_dec_profile = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, COPY_DEC_PROFILE, reg_queue_context_context);
    q_context->copy_to_ddr = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, COPY_TO_DDR, reg_queue_context_context);
    q_context->ddr_copy_disable = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, DDR_COPY_DISABLE, reg_queue_context_context);
    q_context->aggregation_disable = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, AGGREGATION_DISABLE, reg_queue_context_context);
    q_context->fpm_ug = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, FPM_UG, reg_queue_context_context);
    q_context->exclusive_priority = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, EXCLUSIVE_PRIORITY, reg_queue_context_context);
    q_context->q_802_1ae = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, Q_802_1AE, reg_queue_context_context);
    q_context->sci = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, SCI, reg_queue_context_context);
    q_context->fec_enable = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, FEC_ENABLE, reg_queue_context_context);
    q_context->res_profile = RU_FIELD_GET(0, QM, QUEUE_CONTEXT_CONTEXT, RES_PROFILE, reg_queue_context_context);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_copy_decision_profile_set(uint8_t profile_idx, uint32_t queue_occupancy_thr, bdmf_boolean psram_thr)
{
    uint32_t reg_copy_decision_profile_thr=0;

#ifdef VALIDATE_PARMS
    if((profile_idx >= 8) ||
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
    if(!queue_occupancy_thr || !psram_thr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((profile_idx >= 8))
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

bdmf_error_t ag_drv_qm_total_valid_cnt_set(uint16_t q_idx, uint32_t data)
{
    uint32_t reg_total_valid_counter_counter=0;

#ifdef VALIDATE_PARMS
    if((q_idx >= 192))
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
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 192))
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
    uint32_t reg_dqm_valid_counter_counter=0;

#ifdef VALIDATE_PARMS
    if((q_idx >= 192))
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
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 192))
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
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 192))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_idx, QM, DROP_COUNTER_COUNTER, reg_drop_counter_counter);

    *data = RU_FIELD_GET(0, QM, DROP_COUNTER_COUNTER, DATA, reg_drop_counter_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_epon_q_byte_cnt_set(uint16_t q_idx, uint32_t data)
{
    uint32_t reg_epon_rpt_cnt_counter=0;

#ifdef VALIDATE_PARMS
    if((q_idx >= 192))
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
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 192))
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
    if(!status_bit_vector)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 3))
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
    if(!data)
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
    if(!data)
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
    if(!data)
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
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, RD_DATA_POOL3, reg_rd_data_pool3);

    *data = RU_FIELD_GET(0, QM, RD_DATA_POOL3, DATA, reg_rd_data_pool3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_pdfifo_ptr_get(uint16_t q_idx, uint8_t *wr_ptr, uint8_t *rd_ptr)
{
    uint32_t reg_pdfifo_ptr;

#ifdef VALIDATE_PARMS
    if(!wr_ptr || !rd_ptr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_idx >= 96))
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
    if(!wr_ptr || !rd_ptr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((fifo_idx >= 16))
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

bdmf_error_t ag_drv_qm_debug_sel_set(uint8_t select, bdmf_boolean enable)
{
    uint32_t reg_debug_sel=0;

#ifdef VALIDATE_PARMS
    if((select >= _5BITS_MAX_VAL_) ||
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
    if(!select || !enable)
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
    if(!data)
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
    if(!data)
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
    if(!data)
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
    if(!good_lvl1_pkts)
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
    if(!good_lvl1_bytes)
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
    if(!good_lvl2_pkts)
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
    if(!good_lvl2_bytes)
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
    if(!copied_pkts)
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
    if(!copied_bytes)
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
    if(!agg_pkts)
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
    if(!agg_bytes)
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
    if(!agg1_pkts)
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
    if(!agg2_pkts)
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
    if(!agg3_pkts)
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
    if(!agg4_pkts)
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
    if(!wred_drop)
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
    if(!fpm_cong)
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
    if(!ddr_pd_cong_drop)
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
    if(!ddr_cong_byte_drop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, DDR_BYTE_CONGESTION_DROP_CNT, reg_ddr_byte_congestion_drop_cnt);

    *ddr_cong_byte_drop = RU_FIELD_GET(0, QM, DDR_BYTE_CONGESTION_DROP_CNT, COUNTER, reg_ddr_byte_congestion_drop_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_pd_congestion_drop_cnt_get(uint32_t *pd_cong_drop)
{
    uint32_t reg_qm_pd_congestion_drop_cnt;

#ifdef VALIDATE_PARMS
    if(!pd_cong_drop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, QM_PD_CONGESTION_DROP_CNT, reg_qm_pd_congestion_drop_cnt);

    *pd_cong_drop = RU_FIELD_GET(0, QM, QM_PD_CONGESTION_DROP_CNT, COUNTER, reg_qm_pd_congestion_drop_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_qm_abs_requeue_cnt_get(uint32_t *abs_requeue)
{
    uint32_t reg_qm_abs_requeue_cnt;

#ifdef VALIDATE_PARMS
    if(!abs_requeue)
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
    if(!used_words || !empty || !full)
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
    if(!used_words || !empty || !full)
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
    if(!used_words || !empty || !full)
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
    if(!used_words || !empty || !full)
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
    if(!used_words || !empty || !full)
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
    if(!used_words || !empty || !full)
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
    if(!used_words || !empty || !full)
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
    if(!used_words || !empty || !full)
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
    if(!used_words || !empty || !full)
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
    if(!used_words || !empty || !full)
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
    if(!used_words || !empty || !full)
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
    if(!used_words || !empty || !full)
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
    if(!used_words || !empty || !full)
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
    if(!used_words || !empty || !full)
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
    if(!used_words || !empty || !full)
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
    if(!used_words || !empty || !full)
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
    uint32_t reg_bb_route_ovr=0;

#ifdef VALIDATE_PARMS
    if((idx >= 2) ||
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
    if(!ovr_en || !dest_id || !route_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 2))
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
    if(!ingress_stat)
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
    if(!egress_stat)
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
    if(!cm_stat)
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
    if(!fpm_prefetch_stat)
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
    if(!connect_ack_counter)
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
    if(!ddr_wr_reply_counter)
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
    if(!ddr_pipe)
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
    if(!requeue_valid)
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
    if(!pd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 4))
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
    if(!pd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 4))
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
    if(!pd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, QM_CM_PROCESSED_PD_CAPTURE, reg_qm_cm_processed_pd_capture);

    *pd = RU_FIELD_GET(0, QM, QM_CM_PROCESSED_PD_CAPTURE, PD, reg_qm_cm_processed_pd_capture);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_pool_drop_cnt_get(uint32_t idx, uint32_t *fpm_drop)
{
    uint32_t reg_fpm_pool_drop_cnt;

#ifdef VALIDATE_PARMS
    if(!fpm_drop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, FPM_POOL_DROP_CNT, reg_fpm_pool_drop_cnt);

    *fpm_drop = RU_FIELD_GET(0, QM, FPM_POOL_DROP_CNT, COUNTER, reg_fpm_pool_drop_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_grp_drop_cnt_get(uint32_t idx, uint32_t *fpm_grp_drop)
{
    uint32_t reg_fpm_grp_drop_cnt;

#ifdef VALIDATE_PARMS
    if(!fpm_grp_drop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, FPM_GRP_DROP_CNT, reg_fpm_grp_drop_cnt);

    *fpm_grp_drop = RU_FIELD_GET(0, QM, FPM_GRP_DROP_CNT, COUNTER, reg_fpm_grp_drop_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_buffer_res_drop_cnt_get(uint32_t *counter)
{
    uint32_t reg_fpm_buffer_res_drop_cnt;

#ifdef VALIDATE_PARMS
    if(!counter)
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
    if(!counter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, PSRAM_EGRESS_CONG_DRP_CNT, reg_psram_egress_cong_drp_cnt);

    *counter = RU_FIELD_GET(0, QM, PSRAM_EGRESS_CONG_DRP_CNT, COUNTER, reg_psram_egress_cong_drp_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_cm_residue_data_get(uint16_t idx, uint32_t *data)
{
    uint32_t reg_data;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 1536))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, DATA, reg_data);

    *data = RU_FIELD_GET(0, QM, DATA, DATA, reg_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_mactype_set(uint8_t type)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_mactype=0;

#ifdef VALIDATE_PARMS
    if((type >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_common_configurations_mactype = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE, TYPE, reg_bbh_tx_qm_bbhtx_common_configurations_mactype, type);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE, reg_bbh_tx_qm_bbhtx_common_configurations_mactype);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_mactype_get(uint8_t *type)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_mactype;

#ifdef VALIDATE_PARMS
    if(!type)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE, reg_bbh_tx_qm_bbhtx_common_configurations_mactype);

    *type = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE, TYPE, reg_bbh_tx_qm_bbhtx_common_configurations_mactype);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1_set(uint8_t rnr_cfg_index_1, uint16_t tcontaddr, uint16_t skbaddr)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1=0;

#ifdef VALIDATE_PARMS
    if((rnr_cfg_index_1 >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1 = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1, TCONTADDR, reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1, tcontaddr);
    reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1 = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1, SKBADDR, reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1, skbaddr);

    RU_REG_RAM_WRITE(0, rnr_cfg_index_1, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1, reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1_get(uint8_t rnr_cfg_index_1, uint16_t *tcontaddr, uint16_t *skbaddr)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1;

#ifdef VALIDATE_PARMS
    if(!tcontaddr || !skbaddr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_cfg_index_1 >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, rnr_cfg_index_1, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1, reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1);

    *tcontaddr = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1, TCONTADDR, reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1);
    *skbaddr = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1, SKBADDR, reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2_set(uint16_t rnr_cfg_index_2, uint16_t ptraddr, uint8_t task)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2=0;

#ifdef VALIDATE_PARMS
    if((rnr_cfg_index_2 >= 2) ||
       (task >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2 = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2, PTRADDR, reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2, ptraddr);
    reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2 = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2, TASK, reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2, task);

    RU_REG_RAM_WRITE(0, rnr_cfg_index_2, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2, reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2_get(uint16_t rnr_cfg_index_2, uint16_t *ptraddr, uint8_t *task)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2;

#ifdef VALIDATE_PARMS
    if(!ptraddr || !task)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rnr_cfg_index_2 >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, rnr_cfg_index_2, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2, reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2);

    *ptraddr = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2, PTRADDR, reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2);
    *task = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2, TASK, reg_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg_set(bdmf_boolean freenocntxt, bdmf_boolean specialfree, uint8_t maxgn)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg=0;

#ifdef VALIDATE_PARMS
    if((freenocntxt >= _1BITS_MAX_VAL_) ||
       (specialfree >= _1BITS_MAX_VAL_) ||
       (maxgn >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG, FREENOCNTXT, reg_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg, freenocntxt);
    reg_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG, SPECIALFREE, reg_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg, specialfree);
    reg_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG, MAXGN, reg_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg, maxgn);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG, reg_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg_get(bdmf_boolean *freenocntxt, bdmf_boolean *specialfree, uint8_t *maxgn)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg;

#ifdef VALIDATE_PARMS
    if(!freenocntxt || !specialfree || !maxgn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG, reg_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg);

    *freenocntxt = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG, FREENOCNTXT, reg_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg);
    *specialfree = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG, SPECIALFREE, reg_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg);
    *maxgn = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG, MAXGN, reg_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel_set(uint8_t zero, const qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel *bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel)
{
#ifdef VALIDATE_PARMS
    if(!bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_WRITE(0, zero *2 + 0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL, bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel->addr[0]);
    RU_REG_RAM_WRITE(0, zero *2 + 1, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL, bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel->addr[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel_get(uint8_t zero, qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel *bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel)
{
#ifdef VALIDATE_PARMS
    if(!bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, zero *2 + 0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL, bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel->addr[0]);
    RU_REG_RAM_READ(0, zero *2 + 1, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL, bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel->addr[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh_set(uint8_t zero, const qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh *bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh)
{
#ifdef VALIDATE_PARMS
    if(!bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_WRITE(0, zero *2 + 0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH, bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh->addr[0]);
    RU_REG_RAM_WRITE(0, zero *2 + 1, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH, bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh->addr[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh_get(uint8_t zero, qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh *bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh)
{
#ifdef VALIDATE_PARMS
    if(!bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, zero *2 + 0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH, bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh->addr[0]);
    RU_REG_RAM_READ(0, zero *2 + 1, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH, bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh->addr[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl_set(uint16_t psramsize, uint16_t ddrsize, uint16_t psrambase)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl=0;

#ifdef VALIDATE_PARMS
    if((psramsize >= _10BITS_MAX_VAL_) ||
       (ddrsize >= _10BITS_MAX_VAL_) ||
       (psrambase >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL, PSRAMSIZE, reg_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl, psramsize);
    reg_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL, DDRSIZE, reg_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl, ddrsize);
    reg_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL, PSRAMBASE, reg_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl, psrambase);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL, reg_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl_get(uint16_t *psramsize, uint16_t *ddrsize, uint16_t *psrambase)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl;

#ifdef VALIDATE_PARMS
    if(!psramsize || !ddrsize || !psrambase)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL, reg_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl);

    *psramsize = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL, PSRAMSIZE, reg_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl);
    *ddrsize = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL, DDRSIZE, reg_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl);
    *psrambase = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL, PSRAMBASE, reg_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_arb_cfg_set(bdmf_boolean hightrxq)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_arb_cfg=0;

#ifdef VALIDATE_PARMS
    if((hightrxq >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_common_configurations_arb_cfg = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG, HIGHTRXQ, reg_bbh_tx_qm_bbhtx_common_configurations_arb_cfg, hightrxq);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG, reg_bbh_tx_qm_bbhtx_common_configurations_arb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_arb_cfg_get(bdmf_boolean *hightrxq)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_arb_cfg;

#ifdef VALIDATE_PARMS
    if(!hightrxq)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG, reg_bbh_tx_qm_bbhtx_common_configurations_arb_cfg);

    *hightrxq = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG, HIGHTRXQ, reg_bbh_tx_qm_bbhtx_common_configurations_arb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_bbroute_set(uint16_t route, uint8_t dest, bdmf_boolean en)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_bbroute=0;

#ifdef VALIDATE_PARMS
    if((route >= _10BITS_MAX_VAL_) ||
       (dest >= _6BITS_MAX_VAL_) ||
       (en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_common_configurations_bbroute = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE, ROUTE, reg_bbh_tx_qm_bbhtx_common_configurations_bbroute, route);
    reg_bbh_tx_qm_bbhtx_common_configurations_bbroute = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE, DEST, reg_bbh_tx_qm_bbhtx_common_configurations_bbroute, dest);
    reg_bbh_tx_qm_bbhtx_common_configurations_bbroute = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE, EN, reg_bbh_tx_qm_bbhtx_common_configurations_bbroute, en);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE, reg_bbh_tx_qm_bbhtx_common_configurations_bbroute);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_bbroute_get(uint16_t *route, uint8_t *dest, bdmf_boolean *en)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_bbroute;

#ifdef VALIDATE_PARMS
    if(!route || !dest || !en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE, reg_bbh_tx_qm_bbhtx_common_configurations_bbroute);

    *route = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE, ROUTE, reg_bbh_tx_qm_bbhtx_common_configurations_bbroute);
    *dest = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE, DEST, reg_bbh_tx_qm_bbhtx_common_configurations_bbroute);
    *en = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE, EN, reg_bbh_tx_qm_bbhtx_common_configurations_bbroute);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_q2rnr_set(uint8_t q_2_rnr_index, bdmf_boolean q0, bdmf_boolean q1)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_q2rnr=0;

#ifdef VALIDATE_PARMS
    if((q_2_rnr_index >= 20) ||
       (q0 >= _1BITS_MAX_VAL_) ||
       (q1 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_common_configurations_q2rnr = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR, Q0, reg_bbh_tx_qm_bbhtx_common_configurations_q2rnr, q0);
    reg_bbh_tx_qm_bbhtx_common_configurations_q2rnr = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR, Q1, reg_bbh_tx_qm_bbhtx_common_configurations_q2rnr, q1);

    RU_REG_RAM_WRITE(0, q_2_rnr_index, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR, reg_bbh_tx_qm_bbhtx_common_configurations_q2rnr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_q2rnr_get(uint8_t q_2_rnr_index, bdmf_boolean *q0, bdmf_boolean *q1)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_q2rnr;

#ifdef VALIDATE_PARMS
    if(!q0 || !q1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((q_2_rnr_index >= 20))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, q_2_rnr_index, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR, reg_bbh_tx_qm_bbhtx_common_configurations_q2rnr);

    *q0 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR, Q0, reg_bbh_tx_qm_bbhtx_common_configurations_q2rnr);
    *q1 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR, Q1, reg_bbh_tx_qm_bbhtx_common_configurations_q2rnr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_perqtask_set(const qm_bbh_tx_qm_bbhtx_common_configurations_perqtask *bbh_tx_qm_bbhtx_common_configurations_perqtask)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_perqtask=0;

#ifdef VALIDATE_PARMS
    if(!bbh_tx_qm_bbhtx_common_configurations_perqtask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_tx_qm_bbhtx_common_configurations_perqtask->task0 >= _4BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_perqtask->task1 >= _4BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_perqtask->task2 >= _4BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_perqtask->task3 >= _4BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_perqtask->task4 >= _4BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_perqtask->task5 >= _4BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_perqtask->task6 >= _4BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_perqtask->task7 >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_common_configurations_perqtask = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, TASK0, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask, bbh_tx_qm_bbhtx_common_configurations_perqtask->task0);
    reg_bbh_tx_qm_bbhtx_common_configurations_perqtask = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, TASK1, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask, bbh_tx_qm_bbhtx_common_configurations_perqtask->task1);
    reg_bbh_tx_qm_bbhtx_common_configurations_perqtask = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, TASK2, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask, bbh_tx_qm_bbhtx_common_configurations_perqtask->task2);
    reg_bbh_tx_qm_bbhtx_common_configurations_perqtask = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, TASK3, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask, bbh_tx_qm_bbhtx_common_configurations_perqtask->task3);
    reg_bbh_tx_qm_bbhtx_common_configurations_perqtask = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, TASK4, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask, bbh_tx_qm_bbhtx_common_configurations_perqtask->task4);
    reg_bbh_tx_qm_bbhtx_common_configurations_perqtask = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, TASK5, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask, bbh_tx_qm_bbhtx_common_configurations_perqtask->task5);
    reg_bbh_tx_qm_bbhtx_common_configurations_perqtask = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, TASK6, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask, bbh_tx_qm_bbhtx_common_configurations_perqtask->task6);
    reg_bbh_tx_qm_bbhtx_common_configurations_perqtask = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, TASK7, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask, bbh_tx_qm_bbhtx_common_configurations_perqtask->task7);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_perqtask_get(qm_bbh_tx_qm_bbhtx_common_configurations_perqtask *bbh_tx_qm_bbhtx_common_configurations_perqtask)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_perqtask;

#ifdef VALIDATE_PARMS
    if(!bbh_tx_qm_bbhtx_common_configurations_perqtask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask);

    bbh_tx_qm_bbhtx_common_configurations_perqtask->task0 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, TASK0, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask);
    bbh_tx_qm_bbhtx_common_configurations_perqtask->task1 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, TASK1, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask);
    bbh_tx_qm_bbhtx_common_configurations_perqtask->task2 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, TASK2, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask);
    bbh_tx_qm_bbhtx_common_configurations_perqtask->task3 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, TASK3, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask);
    bbh_tx_qm_bbhtx_common_configurations_perqtask->task4 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, TASK4, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask);
    bbh_tx_qm_bbhtx_common_configurations_perqtask->task5 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, TASK5, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask);
    bbh_tx_qm_bbhtx_common_configurations_perqtask->task6 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, TASK6, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask);
    bbh_tx_qm_bbhtx_common_configurations_perqtask->task7 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK, TASK7, reg_bbh_tx_qm_bbhtx_common_configurations_perqtask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd_set(const qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd *bbh_tx_qm_bbhtx_common_configurations_txrstcmd)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd=0;

#ifdef VALIDATE_PARMS
    if(!bbh_tx_qm_bbhtx_common_configurations_txrstcmd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_tx_qm_bbhtx_common_configurations_txrstcmd->cntxtrst >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_txrstcmd->pdfiforst >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_txrstcmd->dmaptrrst >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_txrstcmd->sdmaptrrst >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_txrstcmd->bpmfiforst >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_txrstcmd->sbpmfiforst >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_txrstcmd->okfiforst >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_txrstcmd->ddrfiforst >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_txrstcmd->sramfiforst >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_txrstcmd->skbptrrst >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_txrstcmd->stsfiforst >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_txrstcmd->reqfiforst >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_txrstcmd->msgfiforst >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_txrstcmd->gnxtfiforst >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_txrstcmd->fbnfiforst >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, CNTXTRST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, bbh_tx_qm_bbhtx_common_configurations_txrstcmd->cntxtrst);
    reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, PDFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, bbh_tx_qm_bbhtx_common_configurations_txrstcmd->pdfiforst);
    reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, DMAPTRRST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, bbh_tx_qm_bbhtx_common_configurations_txrstcmd->dmaptrrst);
    reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, SDMAPTRRST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, bbh_tx_qm_bbhtx_common_configurations_txrstcmd->sdmaptrrst);
    reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, BPMFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, bbh_tx_qm_bbhtx_common_configurations_txrstcmd->bpmfiforst);
    reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, SBPMFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, bbh_tx_qm_bbhtx_common_configurations_txrstcmd->sbpmfiforst);
    reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, OKFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, bbh_tx_qm_bbhtx_common_configurations_txrstcmd->okfiforst);
    reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, DDRFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, bbh_tx_qm_bbhtx_common_configurations_txrstcmd->ddrfiforst);
    reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, SRAMFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, bbh_tx_qm_bbhtx_common_configurations_txrstcmd->sramfiforst);
    reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, SKBPTRRST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, bbh_tx_qm_bbhtx_common_configurations_txrstcmd->skbptrrst);
    reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, STSFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, bbh_tx_qm_bbhtx_common_configurations_txrstcmd->stsfiforst);
    reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, REQFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, bbh_tx_qm_bbhtx_common_configurations_txrstcmd->reqfiforst);
    reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, MSGFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, bbh_tx_qm_bbhtx_common_configurations_txrstcmd->msgfiforst);
    reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, GNXTFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, bbh_tx_qm_bbhtx_common_configurations_txrstcmd->gnxtfiforst);
    reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, FBNFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, bbh_tx_qm_bbhtx_common_configurations_txrstcmd->fbnfiforst);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd_get(qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd *bbh_tx_qm_bbhtx_common_configurations_txrstcmd)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd;

#ifdef VALIDATE_PARMS
    if(!bbh_tx_qm_bbhtx_common_configurations_txrstcmd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);

    bbh_tx_qm_bbhtx_common_configurations_txrstcmd->cntxtrst = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, CNTXTRST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
    bbh_tx_qm_bbhtx_common_configurations_txrstcmd->pdfiforst = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, PDFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
    bbh_tx_qm_bbhtx_common_configurations_txrstcmd->dmaptrrst = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, DMAPTRRST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
    bbh_tx_qm_bbhtx_common_configurations_txrstcmd->sdmaptrrst = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, SDMAPTRRST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
    bbh_tx_qm_bbhtx_common_configurations_txrstcmd->bpmfiforst = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, BPMFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
    bbh_tx_qm_bbhtx_common_configurations_txrstcmd->sbpmfiforst = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, SBPMFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
    bbh_tx_qm_bbhtx_common_configurations_txrstcmd->okfiforst = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, OKFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
    bbh_tx_qm_bbhtx_common_configurations_txrstcmd->ddrfiforst = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, DDRFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
    bbh_tx_qm_bbhtx_common_configurations_txrstcmd->sramfiforst = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, SRAMFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
    bbh_tx_qm_bbhtx_common_configurations_txrstcmd->skbptrrst = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, SKBPTRRST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
    bbh_tx_qm_bbhtx_common_configurations_txrstcmd->stsfiforst = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, STSFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
    bbh_tx_qm_bbhtx_common_configurations_txrstcmd->reqfiforst = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, REQFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
    bbh_tx_qm_bbhtx_common_configurations_txrstcmd->msgfiforst = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, MSGFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
    bbh_tx_qm_bbhtx_common_configurations_txrstcmd->gnxtfiforst = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, GNXTFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
    bbh_tx_qm_bbhtx_common_configurations_txrstcmd->fbnfiforst = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD, FBNFIFORST, reg_bbh_tx_qm_bbhtx_common_configurations_txrstcmd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dbgsel_set(uint8_t dbgsel)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_dbgsel=0;

#ifdef VALIDATE_PARMS
    if((dbgsel >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_common_configurations_dbgsel = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL, DBGSEL, reg_bbh_tx_qm_bbhtx_common_configurations_dbgsel, dbgsel);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL, reg_bbh_tx_qm_bbhtx_common_configurations_dbgsel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dbgsel_get(uint8_t *dbgsel)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_dbgsel;

#ifdef VALIDATE_PARMS
    if(!dbgsel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL, reg_bbh_tx_qm_bbhtx_common_configurations_dbgsel);

    *dbgsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL, DBGSEL, reg_bbh_tx_qm_bbhtx_common_configurations_dbgsel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl_set(const qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl *bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl->keep_alive_en >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl->keep_alive_intrvl >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl->bypass_clk_gate);
    reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, TIMER_VAL, reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl->timer_val);
    reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl->keep_alive_en);
    reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl->keep_alive_intrvl);
    reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl->keep_alive_cyc);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl_get(qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl *bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl;

#ifdef VALIDATE_PARMS
    if(!bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl);

    bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl->bypass_clk_gate = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl);
    bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl->timer_val = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, TIMER_VAL, reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl);
    bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl->keep_alive_en = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl);
    bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl->keep_alive_intrvl = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl);
    bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl->keep_alive_cyc = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_gpr_set(uint32_t gpr)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_gpr=0;

#ifdef VALIDATE_PARMS
#endif

    reg_bbh_tx_qm_bbhtx_common_configurations_gpr = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR, GPR, reg_bbh_tx_qm_bbhtx_common_configurations_gpr, gpr);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR, reg_bbh_tx_qm_bbhtx_common_configurations_gpr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_gpr_get(uint32_t *gpr)
{
    uint32_t reg_bbh_tx_qm_bbhtx_common_configurations_gpr;

#ifdef VALIDATE_PARMS
    if(!gpr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR, reg_bbh_tx_qm_bbhtx_common_configurations_gpr);

    *gpr = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR, GPR, reg_bbh_tx_qm_bbhtx_common_configurations_gpr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdbase_set(uint16_t fifobase0, uint16_t fifobase1)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_pdbase=0;

#ifdef VALIDATE_PARMS
    if((fifobase0 >= _9BITS_MAX_VAL_) ||
       (fifobase1 >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_lan_configurations_pdbase = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE, FIFOBASE0, reg_bbh_tx_qm_bbhtx_lan_configurations_pdbase, fifobase0);
    reg_bbh_tx_qm_bbhtx_lan_configurations_pdbase = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE, FIFOBASE1, reg_bbh_tx_qm_bbhtx_lan_configurations_pdbase, fifobase1);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE, reg_bbh_tx_qm_bbhtx_lan_configurations_pdbase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdbase_get(uint16_t *fifobase0, uint16_t *fifobase1)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_pdbase;

#ifdef VALIDATE_PARMS
    if(!fifobase0 || !fifobase1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE, reg_bbh_tx_qm_bbhtx_lan_configurations_pdbase);

    *fifobase0 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE, FIFOBASE0, reg_bbh_tx_qm_bbhtx_lan_configurations_pdbase);
    *fifobase1 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE, FIFOBASE1, reg_bbh_tx_qm_bbhtx_lan_configurations_pdbase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdsize_set(uint16_t fifosize0, uint16_t fifosize1)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_pdsize=0;

#ifdef VALIDATE_PARMS
    if((fifosize0 >= _9BITS_MAX_VAL_) ||
       (fifosize1 >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_lan_configurations_pdsize = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE, FIFOSIZE0, reg_bbh_tx_qm_bbhtx_lan_configurations_pdsize, fifosize0);
    reg_bbh_tx_qm_bbhtx_lan_configurations_pdsize = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE, FIFOSIZE1, reg_bbh_tx_qm_bbhtx_lan_configurations_pdsize, fifosize1);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE, reg_bbh_tx_qm_bbhtx_lan_configurations_pdsize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdsize_get(uint16_t *fifosize0, uint16_t *fifosize1)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_pdsize;

#ifdef VALIDATE_PARMS
    if(!fifosize0 || !fifosize1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE, reg_bbh_tx_qm_bbhtx_lan_configurations_pdsize);

    *fifosize0 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE, FIFOSIZE0, reg_bbh_tx_qm_bbhtx_lan_configurations_pdsize);
    *fifosize1 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE, FIFOSIZE1, reg_bbh_tx_qm_bbhtx_lan_configurations_pdsize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph_set(uint8_t wkupthresh0, uint8_t wkupthresh1)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph=0;

#ifdef VALIDATE_PARMS
#endif

    reg_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH, WKUPTHRESH0, reg_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph, wkupthresh0);
    reg_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH, WKUPTHRESH1, reg_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph, wkupthresh1);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH, reg_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph_get(uint8_t *wkupthresh0, uint8_t *wkupthresh1)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph;

#ifdef VALIDATE_PARMS
    if(!wkupthresh0 || !wkupthresh1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH, reg_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph);

    *wkupthresh0 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH, WKUPTHRESH0, reg_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph);
    *wkupthresh1 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH, WKUPTHRESH1, reg_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_set(uint16_t pdlimit0, uint16_t pdlimit1)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th=0;

#ifdef VALIDATE_PARMS
#endif

    reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH, PDLIMIT0, reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th, pdlimit0);
    reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH, PDLIMIT1, reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th, pdlimit1);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH, reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_get(uint16_t *pdlimit0, uint16_t *pdlimit1)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th;

#ifdef VALIDATE_PARMS
    if(!pdlimit0 || !pdlimit1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH, reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th);

    *pdlimit0 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH, PDLIMIT0, reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th);
    *pdlimit1 = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH, PDLIMIT1, reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en_set(bdmf_boolean pdlimiten)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en=0;

#ifdef VALIDATE_PARMS
    if((pdlimiten >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN, PDLIMITEN, reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en, pdlimiten);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN, reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en_get(bdmf_boolean *pdlimiten)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en;

#ifdef VALIDATE_PARMS
    if(!pdlimiten)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN, reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en);

    *pdlimiten = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN, PDLIMITEN, reg_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdempty_set(uint8_t empty)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_pdempty=0;

#ifdef VALIDATE_PARMS
#endif

    reg_bbh_tx_qm_bbhtx_lan_configurations_pdempty = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY, EMPTY, reg_bbh_tx_qm_bbhtx_lan_configurations_pdempty, empty);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY, reg_bbh_tx_qm_bbhtx_lan_configurations_pdempty);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdempty_get(uint8_t *empty)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_pdempty;

#ifdef VALIDATE_PARMS
    if(!empty)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY, reg_bbh_tx_qm_bbhtx_lan_configurations_pdempty);

    *empty = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY, EMPTY, reg_bbh_tx_qm_bbhtx_lan_configurations_pdempty);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_txthresh_set(uint16_t ddrthresh, uint16_t sramthresh)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_txthresh=0;

#ifdef VALIDATE_PARMS
    if((ddrthresh >= _9BITS_MAX_VAL_) ||
       (sramthresh >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_lan_configurations_txthresh = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH, DDRTHRESH, reg_bbh_tx_qm_bbhtx_lan_configurations_txthresh, ddrthresh);
    reg_bbh_tx_qm_bbhtx_lan_configurations_txthresh = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH, SRAMTHRESH, reg_bbh_tx_qm_bbhtx_lan_configurations_txthresh, sramthresh);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH, reg_bbh_tx_qm_bbhtx_lan_configurations_txthresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_txthresh_get(uint16_t *ddrthresh, uint16_t *sramthresh)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_txthresh;

#ifdef VALIDATE_PARMS
    if(!ddrthresh || !sramthresh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH, reg_bbh_tx_qm_bbhtx_lan_configurations_txthresh);

    *ddrthresh = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH, DDRTHRESH, reg_bbh_tx_qm_bbhtx_lan_configurations_txthresh);
    *sramthresh = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH, SRAMTHRESH, reg_bbh_tx_qm_bbhtx_lan_configurations_txthresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_eee_set(bdmf_boolean en)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_eee=0;

#ifdef VALIDATE_PARMS
    if((en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_lan_configurations_eee = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE, EN, reg_bbh_tx_qm_bbhtx_lan_configurations_eee, en);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE, reg_bbh_tx_qm_bbhtx_lan_configurations_eee);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_eee_get(bdmf_boolean *en)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_eee;

#ifdef VALIDATE_PARMS
    if(!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE, reg_bbh_tx_qm_bbhtx_lan_configurations_eee);

    *en = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE, EN, reg_bbh_tx_qm_bbhtx_lan_configurations_eee);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_ts_set(bdmf_boolean en)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_ts=0;

#ifdef VALIDATE_PARMS
    if((en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_lan_configurations_ts = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS, EN, reg_bbh_tx_qm_bbhtx_lan_configurations_ts, en);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS, reg_bbh_tx_qm_bbhtx_lan_configurations_ts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_ts_get(bdmf_boolean *en)
{
    uint32_t reg_bbh_tx_qm_bbhtx_lan_configurations_ts;

#ifdef VALIDATE_PARMS
    if(!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS, reg_bbh_tx_qm_bbhtx_lan_configurations_ts);

    *en = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS, EN, reg_bbh_tx_qm_bbhtx_lan_configurations_ts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_srambyte_get(uint32_t *srambyte)
{
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_srambyte;

#ifdef VALIDATE_PARMS
    if(!srambyte)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE, reg_bbh_tx_qm_bbhtx_debug_counters_srambyte);

    *srambyte = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE, SRAMBYTE, reg_bbh_tx_qm_bbhtx_debug_counters_srambyte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_ddrbyte_get(uint32_t *ddrbyte)
{
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_ddrbyte;

#ifdef VALIDATE_PARMS
    if(!ddrbyte)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE, reg_bbh_tx_qm_bbhtx_debug_counters_ddrbyte);

    *ddrbyte = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE, DDRBYTE, reg_bbh_tx_qm_bbhtx_debug_counters_ddrbyte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrden_set(const qm_bbh_tx_qm_bbhtx_debug_counters_swrden *bbh_tx_qm_bbhtx_debug_counters_swrden)
{
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_swrden=0;

#ifdef VALIDATE_PARMS
    if(!bbh_tx_qm_bbhtx_debug_counters_swrden)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_tx_qm_bbhtx_debug_counters_swrden->pdsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->pdvsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->pdemptysel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->pdfullsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->pdbemptysel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->pdffwkpsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->fbnsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->fbnvsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->fbnemptysel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->fbnfullsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->getnextsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->getnextvsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->getnextemptysel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->getnextfullsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->gpncntxtsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->bpmsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->bpmfsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->sbpmsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->sbpmfsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->stssel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->stsvsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->stsemptysel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->stsfullsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->stsbemptysel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->stsffwkpsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->msgsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->msgvsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->epnreqsel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->datasel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->reordersel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->tsinfosel >= _1BITS_MAX_VAL_) ||
       (bbh_tx_qm_bbhtx_debug_counters_swrden->mactxsel >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, PDSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->pdsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, PDVSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->pdvsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, PDEMPTYSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->pdemptysel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, PDFULLSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->pdfullsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, PDBEMPTYSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->pdbemptysel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, PDFFWKPSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->pdffwkpsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, FBNSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->fbnsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, FBNVSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->fbnvsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, FBNEMPTYSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->fbnemptysel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, FBNFULLSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->fbnfullsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, GETNEXTSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->getnextsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, GETNEXTVSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->getnextvsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, GETNEXTEMPTYSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->getnextemptysel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, GETNEXTFULLSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->getnextfullsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, GPNCNTXTSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->gpncntxtsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, BPMSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->bpmsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, BPMFSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->bpmfsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, SBPMSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->sbpmsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, SBPMFSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->sbpmfsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, STSSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->stssel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, STSVSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->stsvsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, STSEMPTYSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->stsemptysel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, STSFULLSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->stsfullsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, STSBEMPTYSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->stsbemptysel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, STSFFWKPSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->stsffwkpsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, MSGSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->msgsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, MSGVSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->msgvsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, EPNREQSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->epnreqsel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, DATASEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->datasel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, REORDERSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->reordersel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, TSINFOSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->tsinfosel);
    reg_bbh_tx_qm_bbhtx_debug_counters_swrden = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, MACTXSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden, bbh_tx_qm_bbhtx_debug_counters_swrden->mactxsel);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrden_get(qm_bbh_tx_qm_bbhtx_debug_counters_swrden *bbh_tx_qm_bbhtx_debug_counters_swrden)
{
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_swrden;

#ifdef VALIDATE_PARMS
    if(!bbh_tx_qm_bbhtx_debug_counters_swrden)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);

    bbh_tx_qm_bbhtx_debug_counters_swrden->pdsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, PDSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->pdvsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, PDVSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->pdemptysel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, PDEMPTYSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->pdfullsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, PDFULLSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->pdbemptysel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, PDBEMPTYSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->pdffwkpsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, PDFFWKPSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->fbnsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, FBNSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->fbnvsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, FBNVSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->fbnemptysel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, FBNEMPTYSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->fbnfullsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, FBNFULLSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->getnextsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, GETNEXTSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->getnextvsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, GETNEXTVSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->getnextemptysel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, GETNEXTEMPTYSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->getnextfullsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, GETNEXTFULLSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->gpncntxtsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, GPNCNTXTSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->bpmsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, BPMSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->bpmfsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, BPMFSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->sbpmsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, SBPMSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->sbpmfsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, SBPMFSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->stssel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, STSSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->stsvsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, STSVSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->stsemptysel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, STSEMPTYSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->stsfullsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, STSFULLSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->stsbemptysel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, STSBEMPTYSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->stsffwkpsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, STSFFWKPSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->msgsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, MSGSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->msgvsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, MSGVSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->epnreqsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, EPNREQSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->datasel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, DATASEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->reordersel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, REORDERSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->tsinfosel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, TSINFOSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);
    bbh_tx_qm_bbhtx_debug_counters_swrden->mactxsel = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN, MACTXSEL, reg_bbh_tx_qm_bbhtx_debug_counters_swrden);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrdaddr_set(uint16_t rdaddr)
{
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_swrdaddr=0;

#ifdef VALIDATE_PARMS
    if((rdaddr >= _11BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_tx_qm_bbhtx_debug_counters_swrdaddr = RU_FIELD_SET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR, RDADDR, reg_bbh_tx_qm_bbhtx_debug_counters_swrdaddr, rdaddr);

    RU_REG_WRITE(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR, reg_bbh_tx_qm_bbhtx_debug_counters_swrdaddr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrdaddr_get(uint16_t *rdaddr)
{
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_swrdaddr;

#ifdef VALIDATE_PARMS
    if(!rdaddr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR, reg_bbh_tx_qm_bbhtx_debug_counters_swrdaddr);

    *rdaddr = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR, RDADDR, reg_bbh_tx_qm_bbhtx_debug_counters_swrdaddr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrddata_get(uint32_t *data)
{
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_swrddata;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA, reg_bbh_tx_qm_bbhtx_debug_counters_swrddata);

    *data = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA, DATA, reg_bbh_tx_qm_bbhtx_debug_counters_swrddata);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_unifiedpkt_get(uint8_t debug_unified_pkt_ctr_idx, uint32_t *ddrbyte)
{
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_unifiedpkt;

#ifdef VALIDATE_PARMS
    if(!ddrbyte)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((debug_unified_pkt_ctr_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, debug_unified_pkt_ctr_idx, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT, reg_bbh_tx_qm_bbhtx_debug_counters_unifiedpkt);

    *ddrbyte = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT, DDRBYTE, reg_bbh_tx_qm_bbhtx_debug_counters_unifiedpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_unifiedbyte_get(uint8_t debug_unified_byte_ctr_idx, uint32_t *ddrbyte)
{
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_unifiedbyte;

#ifdef VALIDATE_PARMS
    if(!ddrbyte)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((debug_unified_byte_ctr_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, debug_unified_byte_ctr_idx, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE, reg_bbh_tx_qm_bbhtx_debug_counters_unifiedbyte);

    *ddrbyte = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE, DDRBYTE, reg_bbh_tx_qm_bbhtx_debug_counters_unifiedbyte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg_get(uint8_t zero, qm_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg *bbh_tx_qm_bbhtx_debug_counters_dbgoutreg)
{
#ifdef VALIDATE_PARMS
    if(!bbh_tx_qm_bbhtx_debug_counters_dbgoutreg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((zero >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, zero *8 + 0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG, bbh_tx_qm_bbhtx_debug_counters_dbgoutreg->debug_out_reg[0]);
    RU_REG_RAM_READ(0, zero *8 + 1, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG, bbh_tx_qm_bbhtx_debug_counters_dbgoutreg->debug_out_reg[1]);
    RU_REG_RAM_READ(0, zero *8 + 2, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG, bbh_tx_qm_bbhtx_debug_counters_dbgoutreg->debug_out_reg[2]);
    RU_REG_RAM_READ(0, zero *8 + 3, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG, bbh_tx_qm_bbhtx_debug_counters_dbgoutreg->debug_out_reg[3]);
    RU_REG_RAM_READ(0, zero *8 + 4, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG, bbh_tx_qm_bbhtx_debug_counters_dbgoutreg->debug_out_reg[4]);
    RU_REG_RAM_READ(0, zero *8 + 5, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG, bbh_tx_qm_bbhtx_debug_counters_dbgoutreg->debug_out_reg[5]);
    RU_REG_RAM_READ(0, zero *8 + 6, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG, bbh_tx_qm_bbhtx_debug_counters_dbgoutreg->debug_out_reg[6]);
    RU_REG_RAM_READ(0, zero *8 + 7, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG, bbh_tx_qm_bbhtx_debug_counters_dbgoutreg->debug_out_reg[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_in_segmentation_get(uint8_t debug_counters_in_segmentation_byte_ctr_idx, uint32_t *in_segmentation)
{
    uint32_t reg_bbh_tx_qm_bbhtx_debug_counters_in_segmentation;

#ifdef VALIDATE_PARMS
    if(!in_segmentation)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((debug_counters_in_segmentation_byte_ctr_idx >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, debug_counters_in_segmentation_byte_ctr_idx, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION, reg_bbh_tx_qm_bbhtx_debug_counters_in_segmentation);

    *in_segmentation = RU_FIELD_GET(0, QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION, IN_SEGMENTATION, reg_bbh_tx_qm_bbhtx_debug_counters_in_segmentation);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_bbrouteovrd_set(uint8_t dest, uint16_t route, bdmf_boolean ovrd)
{
    uint32_t reg_dma_qm_dma_config_bbrouteovrd=0;

#ifdef VALIDATE_PARMS
    if((dest >= _6BITS_MAX_VAL_) ||
       (route >= _10BITS_MAX_VAL_) ||
       (ovrd >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dma_qm_dma_config_bbrouteovrd = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_BBROUTEOVRD, DEST, reg_dma_qm_dma_config_bbrouteovrd, dest);
    reg_dma_qm_dma_config_bbrouteovrd = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_BBROUTEOVRD, ROUTE, reg_dma_qm_dma_config_bbrouteovrd, route);
    reg_dma_qm_dma_config_bbrouteovrd = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_BBROUTEOVRD, OVRD, reg_dma_qm_dma_config_bbrouteovrd, ovrd);

    RU_REG_WRITE(0, QM, DMA_QM_DMA_CONFIG_BBROUTEOVRD, reg_dma_qm_dma_config_bbrouteovrd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_bbrouteovrd_get(uint8_t *dest, uint16_t *route, bdmf_boolean *ovrd)
{
    uint32_t reg_dma_qm_dma_config_bbrouteovrd;

#ifdef VALIDATE_PARMS
    if(!dest || !route || !ovrd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, DMA_QM_DMA_CONFIG_BBROUTEOVRD, reg_dma_qm_dma_config_bbrouteovrd);

    *dest = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_BBROUTEOVRD, DEST, reg_dma_qm_dma_config_bbrouteovrd);
    *route = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_BBROUTEOVRD, ROUTE, reg_dma_qm_dma_config_bbrouteovrd);
    *ovrd = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_BBROUTEOVRD, OVRD, reg_dma_qm_dma_config_bbrouteovrd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_num_of_writes_set(uint8_t emac_index, uint8_t numofbuff)
{
    uint32_t reg_dma_qm_dma_config_num_of_writes=0;

#ifdef VALIDATE_PARMS
    if((emac_index >= 8) ||
       (numofbuff >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dma_qm_dma_config_num_of_writes = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_NUM_OF_WRITES, NUMOFBUFF, reg_dma_qm_dma_config_num_of_writes, numofbuff);

    RU_REG_RAM_WRITE(0, emac_index, QM, DMA_QM_DMA_CONFIG_NUM_OF_WRITES, reg_dma_qm_dma_config_num_of_writes);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_num_of_writes_get(uint8_t emac_index, uint8_t *numofbuff)
{
    uint32_t reg_dma_qm_dma_config_num_of_writes;

#ifdef VALIDATE_PARMS
    if(!numofbuff)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, emac_index, QM, DMA_QM_DMA_CONFIG_NUM_OF_WRITES, reg_dma_qm_dma_config_num_of_writes);

    *numofbuff = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_NUM_OF_WRITES, NUMOFBUFF, reg_dma_qm_dma_config_num_of_writes);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_num_of_reads_set(uint8_t emac_index, uint8_t rr_num)
{
    uint32_t reg_dma_qm_dma_config_num_of_reads=0;

#ifdef VALIDATE_PARMS
    if((emac_index >= 8) ||
       (rr_num >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dma_qm_dma_config_num_of_reads = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_NUM_OF_READS, RR_NUM, reg_dma_qm_dma_config_num_of_reads, rr_num);

    RU_REG_RAM_WRITE(0, emac_index, QM, DMA_QM_DMA_CONFIG_NUM_OF_READS, reg_dma_qm_dma_config_num_of_reads);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_num_of_reads_get(uint8_t emac_index, uint8_t *rr_num)
{
    uint32_t reg_dma_qm_dma_config_num_of_reads;

#ifdef VALIDATE_PARMS
    if(!rr_num)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, emac_index, QM, DMA_QM_DMA_CONFIG_NUM_OF_READS, reg_dma_qm_dma_config_num_of_reads);

    *rr_num = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_NUM_OF_READS, RR_NUM, reg_dma_qm_dma_config_num_of_reads);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_u_thresh_set(uint8_t emac_index, uint8_t into_u, uint8_t out_of_u)
{
    uint32_t reg_dma_qm_dma_config_u_thresh=0;

#ifdef VALIDATE_PARMS
    if((emac_index >= 8) ||
       (into_u >= _6BITS_MAX_VAL_) ||
       (out_of_u >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dma_qm_dma_config_u_thresh = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_U_THRESH, INTO_U, reg_dma_qm_dma_config_u_thresh, into_u);
    reg_dma_qm_dma_config_u_thresh = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_U_THRESH, OUT_OF_U, reg_dma_qm_dma_config_u_thresh, out_of_u);

    RU_REG_RAM_WRITE(0, emac_index, QM, DMA_QM_DMA_CONFIG_U_THRESH, reg_dma_qm_dma_config_u_thresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_u_thresh_get(uint8_t emac_index, uint8_t *into_u, uint8_t *out_of_u)
{
    uint32_t reg_dma_qm_dma_config_u_thresh;

#ifdef VALIDATE_PARMS
    if(!into_u || !out_of_u)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, emac_index, QM, DMA_QM_DMA_CONFIG_U_THRESH, reg_dma_qm_dma_config_u_thresh);

    *into_u = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_U_THRESH, INTO_U, reg_dma_qm_dma_config_u_thresh);
    *out_of_u = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_U_THRESH, OUT_OF_U, reg_dma_qm_dma_config_u_thresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_pri_set(uint8_t emac_index, uint8_t rxpri, uint8_t txpri)
{
    uint32_t reg_dma_qm_dma_config_pri=0;

#ifdef VALIDATE_PARMS
    if((emac_index >= 8) ||
       (rxpri >= _4BITS_MAX_VAL_) ||
       (txpri >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dma_qm_dma_config_pri = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_PRI, RXPRI, reg_dma_qm_dma_config_pri, rxpri);
    reg_dma_qm_dma_config_pri = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_PRI, TXPRI, reg_dma_qm_dma_config_pri, txpri);

    RU_REG_RAM_WRITE(0, emac_index, QM, DMA_QM_DMA_CONFIG_PRI, reg_dma_qm_dma_config_pri);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_pri_get(uint8_t emac_index, uint8_t *rxpri, uint8_t *txpri)
{
    uint32_t reg_dma_qm_dma_config_pri;

#ifdef VALIDATE_PARMS
    if(!rxpri || !txpri)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, emac_index, QM, DMA_QM_DMA_CONFIG_PRI, reg_dma_qm_dma_config_pri);

    *rxpri = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_PRI, RXPRI, reg_dma_qm_dma_config_pri);
    *txpri = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_PRI, TXPRI, reg_dma_qm_dma_config_pri);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_periph_source_set(uint8_t emac_index, uint8_t rxsource, uint8_t txsource)
{
    uint32_t reg_dma_qm_dma_config_periph_source=0;

#ifdef VALIDATE_PARMS
    if((emac_index >= 8) ||
       (rxsource >= _6BITS_MAX_VAL_) ||
       (txsource >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dma_qm_dma_config_periph_source = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_PERIPH_SOURCE, RXSOURCE, reg_dma_qm_dma_config_periph_source, rxsource);
    reg_dma_qm_dma_config_periph_source = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_PERIPH_SOURCE, TXSOURCE, reg_dma_qm_dma_config_periph_source, txsource);

    RU_REG_RAM_WRITE(0, emac_index, QM, DMA_QM_DMA_CONFIG_PERIPH_SOURCE, reg_dma_qm_dma_config_periph_source);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_periph_source_get(uint8_t emac_index, uint8_t *rxsource, uint8_t *txsource)
{
    uint32_t reg_dma_qm_dma_config_periph_source;

#ifdef VALIDATE_PARMS
    if(!rxsource || !txsource)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, emac_index, QM, DMA_QM_DMA_CONFIG_PERIPH_SOURCE, reg_dma_qm_dma_config_periph_source);

    *rxsource = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_PERIPH_SOURCE, RXSOURCE, reg_dma_qm_dma_config_periph_source);
    *txsource = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_PERIPH_SOURCE, TXSOURCE, reg_dma_qm_dma_config_periph_source);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_weight_set(uint8_t emac_index, uint8_t rxweight, uint8_t txweight)
{
    uint32_t reg_dma_qm_dma_config_weight=0;

#ifdef VALIDATE_PARMS
    if((emac_index >= 8) ||
       (rxweight >= _3BITS_MAX_VAL_) ||
       (txweight >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dma_qm_dma_config_weight = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_WEIGHT, RXWEIGHT, reg_dma_qm_dma_config_weight, rxweight);
    reg_dma_qm_dma_config_weight = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_WEIGHT, TXWEIGHT, reg_dma_qm_dma_config_weight, txweight);

    RU_REG_RAM_WRITE(0, emac_index, QM, DMA_QM_DMA_CONFIG_WEIGHT, reg_dma_qm_dma_config_weight);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_weight_get(uint8_t emac_index, uint8_t *rxweight, uint8_t *txweight)
{
    uint32_t reg_dma_qm_dma_config_weight;

#ifdef VALIDATE_PARMS
    if(!rxweight || !txweight)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, emac_index, QM, DMA_QM_DMA_CONFIG_WEIGHT, reg_dma_qm_dma_config_weight);

    *rxweight = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_WEIGHT, RXWEIGHT, reg_dma_qm_dma_config_weight);
    *txweight = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_WEIGHT, TXWEIGHT, reg_dma_qm_dma_config_weight);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_ptrrst_set(uint16_t rstvec)
{
    uint32_t reg_dma_qm_dma_config_ptrrst=0;

#ifdef VALIDATE_PARMS
#endif

    reg_dma_qm_dma_config_ptrrst = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_PTRRST, RSTVEC, reg_dma_qm_dma_config_ptrrst, rstvec);

    RU_REG_WRITE(0, QM, DMA_QM_DMA_CONFIG_PTRRST, reg_dma_qm_dma_config_ptrrst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_ptrrst_get(uint16_t *rstvec)
{
    uint32_t reg_dma_qm_dma_config_ptrrst;

#ifdef VALIDATE_PARMS
    if(!rstvec)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, DMA_QM_DMA_CONFIG_PTRRST, reg_dma_qm_dma_config_ptrrst);

    *rstvec = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_PTRRST, RSTVEC, reg_dma_qm_dma_config_ptrrst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_max_otf_set(uint8_t max)
{
    uint32_t reg_dma_qm_dma_config_max_otf=0;

#ifdef VALIDATE_PARMS
    if((max >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dma_qm_dma_config_max_otf = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_MAX_OTF, MAX, reg_dma_qm_dma_config_max_otf, max);

    RU_REG_WRITE(0, QM, DMA_QM_DMA_CONFIG_MAX_OTF, reg_dma_qm_dma_config_max_otf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_max_otf_get(uint8_t *max)
{
    uint32_t reg_dma_qm_dma_config_max_otf;

#ifdef VALIDATE_PARMS
    if(!max)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, DMA_QM_DMA_CONFIG_MAX_OTF, reg_dma_qm_dma_config_max_otf);

    *max = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_MAX_OTF, MAX, reg_dma_qm_dma_config_max_otf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_clk_gate_cntrl_set(const qm_dma_qm_dma_config_clk_gate_cntrl *dma_qm_dma_config_clk_gate_cntrl)
{
    uint32_t reg_dma_qm_dma_config_clk_gate_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!dma_qm_dma_config_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_qm_dma_config_clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
       (dma_qm_dma_config_clk_gate_cntrl->keep_alive_en >= _1BITS_MAX_VAL_) ||
       (dma_qm_dma_config_clk_gate_cntrl->keep_alive_intrvl >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dma_qm_dma_config_clk_gate_cntrl = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_dma_qm_dma_config_clk_gate_cntrl, dma_qm_dma_config_clk_gate_cntrl->bypass_clk_gate);
    reg_dma_qm_dma_config_clk_gate_cntrl = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL, TIMER_VAL, reg_dma_qm_dma_config_clk_gate_cntrl, dma_qm_dma_config_clk_gate_cntrl->timer_val);
    reg_dma_qm_dma_config_clk_gate_cntrl = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_dma_qm_dma_config_clk_gate_cntrl, dma_qm_dma_config_clk_gate_cntrl->keep_alive_en);
    reg_dma_qm_dma_config_clk_gate_cntrl = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_dma_qm_dma_config_clk_gate_cntrl, dma_qm_dma_config_clk_gate_cntrl->keep_alive_intrvl);
    reg_dma_qm_dma_config_clk_gate_cntrl = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_dma_qm_dma_config_clk_gate_cntrl, dma_qm_dma_config_clk_gate_cntrl->keep_alive_cyc);

    RU_REG_WRITE(0, QM, DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL, reg_dma_qm_dma_config_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_clk_gate_cntrl_get(qm_dma_qm_dma_config_clk_gate_cntrl *dma_qm_dma_config_clk_gate_cntrl)
{
    uint32_t reg_dma_qm_dma_config_clk_gate_cntrl;

#ifdef VALIDATE_PARMS
    if(!dma_qm_dma_config_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL, reg_dma_qm_dma_config_clk_gate_cntrl);

    dma_qm_dma_config_clk_gate_cntrl->bypass_clk_gate = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_dma_qm_dma_config_clk_gate_cntrl);
    dma_qm_dma_config_clk_gate_cntrl->timer_val = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL, TIMER_VAL, reg_dma_qm_dma_config_clk_gate_cntrl);
    dma_qm_dma_config_clk_gate_cntrl->keep_alive_en = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_dma_qm_dma_config_clk_gate_cntrl);
    dma_qm_dma_config_clk_gate_cntrl->keep_alive_intrvl = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_dma_qm_dma_config_clk_gate_cntrl);
    dma_qm_dma_config_clk_gate_cntrl->keep_alive_cyc = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_dma_qm_dma_config_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_dbg_sel_set(uint8_t dbgsel)
{
    uint32_t reg_dma_qm_dma_config_dbg_sel=0;

#ifdef VALIDATE_PARMS
    if((dbgsel >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dma_qm_dma_config_dbg_sel = RU_FIELD_SET(0, QM, DMA_QM_DMA_CONFIG_DBG_SEL, DBGSEL, reg_dma_qm_dma_config_dbg_sel, dbgsel);

    RU_REG_WRITE(0, QM, DMA_QM_DMA_CONFIG_DBG_SEL, reg_dma_qm_dma_config_dbg_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_config_dbg_sel_get(uint8_t *dbgsel)
{
    uint32_t reg_dma_qm_dma_config_dbg_sel;

#ifdef VALIDATE_PARMS
    if(!dbgsel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, DMA_QM_DMA_CONFIG_DBG_SEL, reg_dma_qm_dma_config_dbg_sel);

    *dbgsel = RU_FIELD_GET(0, QM, DMA_QM_DMA_CONFIG_DBG_SEL, DBGSEL, reg_dma_qm_dma_config_dbg_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_debug_req_cnt_rx_get(uint8_t emac_index, uint8_t *req_cnt)
{
    uint32_t reg_dma_qm_dma_debug_req_cnt_rx;

#ifdef VALIDATE_PARMS
    if(!req_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, emac_index, QM, DMA_QM_DMA_DEBUG_REQ_CNT_RX, reg_dma_qm_dma_debug_req_cnt_rx);

    *req_cnt = RU_FIELD_GET(0, QM, DMA_QM_DMA_DEBUG_REQ_CNT_RX, REQ_CNT, reg_dma_qm_dma_debug_req_cnt_rx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_debug_req_cnt_tx_get(uint8_t emac_index, uint8_t *req_cnt)
{
    uint32_t reg_dma_qm_dma_debug_req_cnt_tx;

#ifdef VALIDATE_PARMS
    if(!req_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, emac_index, QM, DMA_QM_DMA_DEBUG_REQ_CNT_TX, reg_dma_qm_dma_debug_req_cnt_tx);

    *req_cnt = RU_FIELD_GET(0, QM, DMA_QM_DMA_DEBUG_REQ_CNT_TX, REQ_CNT, reg_dma_qm_dma_debug_req_cnt_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_debug_req_cnt_rx_acc_get(uint8_t emac_index, uint32_t *req_cnt)
{
    uint32_t reg_dma_qm_dma_debug_req_cnt_rx_acc;

#ifdef VALIDATE_PARMS
    if(!req_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, emac_index, QM, DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC, reg_dma_qm_dma_debug_req_cnt_rx_acc);

    *req_cnt = RU_FIELD_GET(0, QM, DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC, REQ_CNT, reg_dma_qm_dma_debug_req_cnt_rx_acc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_debug_req_cnt_tx_acc_get(uint8_t emac_index, uint32_t *req_cnt)
{
    uint32_t reg_dma_qm_dma_debug_req_cnt_tx_acc;

#ifdef VALIDATE_PARMS
    if(!req_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, emac_index, QM, DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC, reg_dma_qm_dma_debug_req_cnt_tx_acc);

    *req_cnt = RU_FIELD_GET(0, QM, DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC, REQ_CNT, reg_dma_qm_dma_debug_req_cnt_tx_acc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_dma_qm_dma_debug_rddata_get(uint8_t word_index, uint32_t *data)
{
    uint32_t reg_dma_qm_dma_debug_rddata;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((word_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, word_index, QM, DMA_QM_DMA_DEBUG_RDDATA, reg_dma_qm_dma_debug_rddata);

    *data = RU_FIELD_GET(0, QM, DMA_QM_DMA_DEBUG_RDDATA, DATA, reg_dma_qm_dma_debug_rddata);

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
    bdmf_address_global_cfg_fpm_base_addr,
    bdmf_address_global_cfg_fpm_coherent_base_addr,
    bdmf_address_global_cfg_ddr_sop_offset,
    bdmf_address_global_cfg_epon_overhead_ctrl,
    bdmf_address_global_cfg_dqm_full,
    bdmf_address_global_cfg_dqm_not_empty,
    bdmf_address_global_cfg_dqm_pop_ready,
    bdmf_address_global_cfg_aggregation_context_valid,
    bdmf_address_global_cfg_qm_aggregation_timer_ctrl,
    bdmf_address_global_cfg_qm_fpm_buffer_grp_res,
    bdmf_address_global_cfg_qm_fpm_buffer_gbl_thr,
    bdmf_address_global_cfg_qm_flow_ctrl_rnr_cfg,
    bdmf_address_global_cfg_qm_flow_ctrl_intr,
    bdmf_address_global_cfg_qm_fpm_ug_gbl_cnt,
    bdmf_address_global_cfg_qm_egress_flush_queue,
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
    bdmf_address_total_valid_counter_counter,
    bdmf_address_dqm_valid_counter_counter,
    bdmf_address_drop_counter_counter,
    bdmf_address_epon_rpt_cnt_counter,
    bdmf_address_epon_rpt_cnt_queue_status,
    bdmf_address_rd_data_pool0,
    bdmf_address_rd_data_pool1,
    bdmf_address_rd_data_pool2,
    bdmf_address_rd_data_pool3,
    bdmf_address_pdfifo_ptr,
    bdmf_address_update_fifo_ptr,
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
    bdmf_address_fpm_pool_drop_cnt,
    bdmf_address_fpm_grp_drop_cnt,
    bdmf_address_fpm_buffer_res_drop_cnt,
    bdmf_address_psram_egress_cong_drp_cnt,
    bdmf_address_data,
    bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_base,
    bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_mask,
    bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_base,
    bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_mask,
    bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_base,
    bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_mask,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_mactype,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_arb_cfg,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_bbroute,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_q2rnr,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_perqtask,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_txrstcmd,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_dbgsel,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl,
    bdmf_address_bbh_tx_qm_bbhtx_common_configurations_gpr,
    bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pdbase,
    bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pdsize,
    bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph,
    bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th,
    bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en,
    bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pdempty,
    bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_txthresh,
    bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_eee,
    bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_ts,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_srampd,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_ddrpd,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_pddrop,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_stscnt,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_stsdrop,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_msgcnt,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_msgdrop,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_getnextnull,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_flushpkts,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_lenerr,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_aggrlenerr,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_srampkt,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_ddrpkt,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_srambyte,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_ddrbyte,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_swrden,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_swrdaddr,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_swrddata,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_unifiedpkt,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_unifiedbyte,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg,
    bdmf_address_bbh_tx_qm_bbhtx_debug_counters_in_segmentation,
    bdmf_address_dma_qm_dma_config_bbrouteovrd,
    bdmf_address_dma_qm_dma_config_num_of_writes,
    bdmf_address_dma_qm_dma_config_num_of_reads,
    bdmf_address_dma_qm_dma_config_u_thresh,
    bdmf_address_dma_qm_dma_config_pri,
    bdmf_address_dma_qm_dma_config_periph_source,
    bdmf_address_dma_qm_dma_config_weight,
    bdmf_address_dma_qm_dma_config_ptrrst,
    bdmf_address_dma_qm_dma_config_max_otf,
    bdmf_address_dma_qm_dma_config_clk_gate_cntrl,
    bdmf_address_dma_qm_dma_config_dbg_sel,
    bdmf_address_dma_qm_dma_debug_nempty,
    bdmf_address_dma_qm_dma_debug_urgnt,
    bdmf_address_dma_qm_dma_debug_selsrc,
    bdmf_address_dma_qm_dma_debug_req_cnt_rx,
    bdmf_address_dma_qm_dma_debug_req_cnt_tx,
    bdmf_address_dma_qm_dma_debug_req_cnt_rx_acc,
    bdmf_address_dma_qm_dma_debug_req_cnt_tx_acc,
    bdmf_address_dma_qm_dma_debug_rdadd,
    bdmf_address_dma_qm_dma_debug_rdvalid,
    bdmf_address_dma_qm_dma_debug_rddata,
    bdmf_address_dma_qm_dma_debug_rddatardy,
}
bdmf_address;

static int bcm_qm_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_qm_ddr_cong_ctrl:
    {
        qm_ddr_cong_ctrl ddr_cong_ctrl = { .ddr_byte_congestion_drop_enable=parm[1].value.unumber, .ddr_bytes_lower_thr=parm[2].value.unumber, .ddr_bytes_mid_thr=parm[3].value.unumber, .ddr_bytes_higher_thr=parm[4].value.unumber, .ddr_pd_congestion_drop_enable=parm[5].value.unumber, .ddr_pipe_lower_thr=parm[6].value.unumber, .ddr_pipe_higher_thr=parm[7].value.unumber};
        err = ag_drv_qm_ddr_cong_ctrl_set(&ddr_cong_ctrl);
        break;
    }
    case cli_qm_fpm_ug_thr:
    {
        qm_fpm_ug_thr fpm_ug_thr = { .lower_thr=parm[2].value.unumber, .mid_thr=parm[3].value.unumber, .higher_thr=parm[4].value.unumber};
        err = ag_drv_qm_fpm_ug_thr_set(parm[1].value.unumber, &fpm_ug_thr);
        break;
    }
    case cli_qm_rnr_group_cfg:
    {
        qm_rnr_group_cfg rnr_group_cfg = { .start_queue=parm[2].value.unumber, .end_queue=parm[3].value.unumber, .pd_fifo_base=parm[4].value.unumber, .pd_fifo_size=parm[5].value.unumber, .upd_fifo_base=parm[6].value.unumber, .upd_fifo_size=parm[7].value.unumber, .rnr_bb_id=parm[8].value.unumber, .rnr_task=parm[9].value.unumber, .rnr_enable=parm[10].value.unumber};
        err = ag_drv_qm_rnr_group_cfg_set(parm[1].value.unumber, &rnr_group_cfg);
        break;
    }
    case cli_qm_cpu_pd_indirect_wr_data:
        err = ag_drv_qm_cpu_pd_indirect_wr_data_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_qm_wred_profile_cfg:
    {
        qm_wred_profile_cfg wred_profile_cfg = { .min_thr0=parm[2].value.unumber, .flw_ctrl_en0=parm[3].value.unumber, .min_thr1=parm[4].value.unumber, .flw_ctrl_en1=parm[5].value.unumber, .max_thr0=parm[6].value.unumber, .max_thr1=parm[7].value.unumber, .slope_mantissa0=parm[8].value.unumber, .slope_exp0=parm[9].value.unumber, .slope_mantissa1=parm[10].value.unumber, .slope_exp1=parm[11].value.unumber};
        err = ag_drv_qm_wred_profile_cfg_set(parm[1].value.unumber, &wred_profile_cfg);
        break;
    }
    case cli_qm_ubus_slave:
    {
        qm_ubus_slave ubus_slave = { .vpb_base=parm[1].value.unumber, .vpb_mask=parm[2].value.unumber, .apb_base=parm[3].value.unumber, .apb_mask=parm[4].value.unumber, .dqm_base=parm[5].value.unumber, .dqm_mask=parm[6].value.unumber};
        err = ag_drv_qm_ubus_slave_set(&ubus_slave);
        break;
    }
    case cli_qm_cfg_src_id:
    {
        qm_cfg_src_id cfg_src_id = { .fpmsrc=parm[1].value.unumber, .sbpmsrc=parm[2].value.unumber, .stsrnrsrc=parm[3].value.unumber};
        err = ag_drv_qm_cfg_src_id_set(&cfg_src_id);
        break;
    }
    case cli_qm_rnr_src_id:
        err = ag_drv_qm_rnr_src_id_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_bbh_dma_cfg:
    {
        qm_bbh_dma_cfg bbh_dma_cfg = { .dmasrc=parm[1].value.unumber, .descbase=parm[2].value.unumber, .descsize=parm[3].value.unumber};
        err = ag_drv_qm_bbh_dma_cfg_set(&bbh_dma_cfg);
        break;
    }
    case cli_qm_dma_max_otf_read_request:
        err = ag_drv_qm_dma_max_otf_read_request_set(parm[1].value.unumber);
        break;
    case cli_qm_dma_epon_urgent:
        err = ag_drv_qm_dma_epon_urgent_set(parm[1].value.unumber);
        break;
    case cli_qm_bbh_sdma_cfg:
    {
        qm_bbh_sdma_cfg bbh_sdma_cfg = { .sdmasrc=parm[1].value.unumber, .descbase=parm[2].value.unumber, .descsize=parm[3].value.unumber};
        err = ag_drv_qm_bbh_sdma_cfg_set(&bbh_sdma_cfg);
        break;
    }
    case cli_qm_sdma_max_otf_read_request:
        err = ag_drv_qm_sdma_max_otf_read_request_set(parm[1].value.unumber);
        break;
    case cli_qm_sdma_epon_urgent:
        err = ag_drv_qm_sdma_epon_urgent_set(parm[1].value.unumber);
        break;
    case cli_qm_bbh_ddr_cfg:
    {
        qm_bbh_ddr_cfg bbh_ddr_cfg = { .bufsize=parm[1].value.unumber, .byteresul=parm[2].value.unumber, .ddrtxoffset=parm[3].value.unumber, .hnsize0=parm[4].value.unumber, .hnsize1=parm[5].value.unumber};
        err = ag_drv_qm_bbh_ddr_cfg_set(&bbh_ddr_cfg);
        break;
    }
    case cli_qm_debug_info:
    {
        qm_debug_info debug_info = { .nempty=parm[1].value.unumber, .urgnt=parm[2].value.unumber, .sel_src=parm[3].value.unumber, .address=parm[4].value.unumber, .datacs=parm[5].value.unumber, .cdcs=parm[6].value.unumber, .rrcs=parm[7].value.unumber, .valid=parm[8].value.unumber, .ready=parm[9].value.unumber};
        err = ag_drv_qm_debug_info_set(&debug_info);
        break;
    }
    case cli_qm_enable_ctrl:
    {
        qm_enable_ctrl enable_ctrl = { .fpm_prefetch_enable=parm[1].value.unumber, .reorder_credit_enable=parm[2].value.unumber, .dqm_pop_enable=parm[3].value.unumber, .rmt_fixed_arb_enable=parm[4].value.unumber, .dqm_push_fixed_arb_enable=parm[5].value.unumber};
        err = ag_drv_qm_enable_ctrl_set(&enable_ctrl);
        break;
    }
    case cli_qm_reset_ctrl:
    {
        qm_reset_ctrl reset_ctrl = { .fpm_prefetch0_sw_rst=parm[1].value.unumber, .fpm_prefetch1_sw_rst=parm[2].value.unumber, .fpm_prefetch2_sw_rst=parm[3].value.unumber, .fpm_prefetch3_sw_rst=parm[4].value.unumber, .normal_rmt_sw_rst=parm[5].value.unumber, .non_delayed_rmt_sw_rst=parm[6].value.unumber, .pre_cm_fifo_sw_rst=parm[7].value.unumber, .cm_rd_pd_fifo_sw_rst=parm[8].value.unumber, .cm_wr_pd_fifo_sw_rst=parm[9].value.unumber, .bb0_output_fifo_sw_rst=parm[10].value.unumber, .bb1_output_fifo_sw_rst=parm[11].value.unumber, .bb1_input_fifo_sw_rst=parm[12].value.unumber, .tm_fifo_ptr_sw_rst=parm[13].value.unumber, .non_delayed_out_fifo_sw_rst=parm[14].value.unumber};
        err = ag_drv_qm_reset_ctrl_set(&reset_ctrl);
        break;
    }
    case cli_qm_drop_counters_ctrl:
    {
        qm_drop_counters_ctrl drop_counters_ctrl = { .read_clear_pkts=parm[1].value.unumber, .read_clear_bytes=parm[2].value.unumber, .disable_wrap_around_pkts=parm[3].value.unumber, .disable_wrap_around_bytes=parm[4].value.unumber, .free_with_context_last_search=parm[5].value.unumber, .wred_disable=parm[6].value.unumber, .ddr_pd_congestion_disable=parm[7].value.unumber, .ddr_byte_congestion_disable=parm[8].value.unumber, .ddr_occupancy_disable=parm[9].value.unumber, .ddr_fpm_congestion_disable=parm[10].value.unumber, .fpm_ug_disable=parm[11].value.unumber, .queue_occupancy_ddr_copy_decision_disable=parm[12].value.unumber, .psram_occupancy_ddr_copy_decision_disable=parm[13].value.unumber, .dont_send_mc_bit_to_bbh=parm[14].value.unumber, .close_aggregation_on_timeout_disable=parm[15].value.unumber, .fpm_congestion_buf_release_mechanism_disable=parm[16].value.unumber, .fpm_buffer_global_res_enable=parm[17].value.unumber, .qm_preserve_pd_with_fpm=parm[18].value.unumber, .qm_residue_per_queue=parm[19].value.unumber, .ghost_rpt_update_after_close_agg_en=parm[20].value.unumber, .fpm_ug_flow_ctrl_disable=parm[21].value.unumber, .ddr_write_multi_slave_en=parm[22].value.unumber, .ddr_pd_congestion_agg_priority=parm[23].value.unumber, .psram_occupancy_drop_disable=parm[24].value.unumber, .qm_ddr_write_alignment=parm[25].value.unumber, .exclusive_dont_drop=parm[26].value.unumber, .exclusive_dont_drop_bp_en=parm[27].value.unumber, .gpon_dbr_ceil=parm[28].value.unumber};
        err = ag_drv_qm_drop_counters_ctrl_set(&drop_counters_ctrl);
        break;
    }
    case cli_qm_fpm_ctrl:
        err = ag_drv_qm_fpm_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_qm_qm_pd_cong_ctrl:
        err = ag_drv_qm_qm_pd_cong_ctrl_set(parm[1].value.unumber);
        break;
    case cli_qm_global_cfg_abs_drop_queue:
        err = ag_drv_qm_global_cfg_abs_drop_queue_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_global_cfg_aggregation_ctrl:
        err = ag_drv_qm_global_cfg_aggregation_ctrl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_fpm_base_addr:
        err = ag_drv_qm_fpm_base_addr_set(parm[1].value.unumber);
        break;
    case cli_qm_global_cfg_fpm_coherent_base_addr:
        err = ag_drv_qm_global_cfg_fpm_coherent_base_addr_set(parm[1].value.unumber);
        break;
    case cli_qm_ddr_sop_offset:
        err = ag_drv_qm_ddr_sop_offset_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_epon_overhead_ctrl:
    {
        qm_epon_overhead_ctrl epon_overhead_ctrl = { .epon_line_rate=parm[1].value.unumber, .epon_crc_add_disable=parm[2].value.unumber, .mac_flow_overwrite_crc_en=parm[3].value.unumber, .mac_flow_overwrite_crc=parm[4].value.unumber, .fec_ipg_length=parm[5].value.unumber};
        err = ag_drv_qm_epon_overhead_ctrl_set(&epon_overhead_ctrl);
        break;
    }
    case cli_qm_global_cfg_qm_aggregation_timer_ctrl:
        err = ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_global_cfg_qm_fpm_buffer_grp_res:
        err = ag_drv_qm_global_cfg_qm_fpm_buffer_grp_res_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_qm_global_cfg_qm_fpm_buffer_gbl_thr:
        err = ag_drv_qm_global_cfg_qm_fpm_buffer_gbl_thr_set(parm[1].value.unumber);
        break;
    case cli_qm_global_cfg_qm_flow_ctrl_rnr_cfg:
        err = ag_drv_qm_global_cfg_qm_flow_ctrl_rnr_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_qm_global_cfg_qm_flow_ctrl_intr:
        err = ag_drv_qm_global_cfg_qm_flow_ctrl_intr_set(parm[1].value.unumber);
        break;
    case cli_qm_global_cfg_qm_fpm_ug_gbl_cnt:
        err = ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_set(parm[1].value.unumber);
        break;
    case cli_qm_global_cfg_qm_egress_flush_queue:
        err = ag_drv_qm_global_cfg_qm_egress_flush_queue_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_fpm_pool_thr:
    {
        qm_fpm_pool_thr fpm_pool_thr = { .lower_thr=parm[2].value.unumber, .higher_thr=parm[3].value.unumber};
        err = ag_drv_qm_fpm_pool_thr_set(parm[1].value.unumber, &fpm_pool_thr);
        break;
    }
    case cli_qm_fpm_ug_cnt:
        err = ag_drv_qm_fpm_ug_cnt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_intr_ctrl_isr:
    {
        qm_intr_ctrl_isr intr_ctrl_isr = { .qm_dqm_pop_on_empty=parm[1].value.unumber, .qm_dqm_push_on_full=parm[2].value.unumber, .qm_cpu_pop_on_empty=parm[3].value.unumber, .qm_cpu_push_on_full=parm[4].value.unumber, .qm_normal_queue_pd_no_credit=parm[5].value.unumber, .qm_non_delayed_queue_pd_no_credit=parm[6].value.unumber, .qm_non_valid_queue=parm[7].value.unumber, .qm_agg_coherent_inconsistency=parm[8].value.unumber, .qm_force_copy_on_non_delayed=parm[9].value.unumber, .qm_fpm_pool_size_nonexistent=parm[10].value.unumber, .qm_target_mem_abs_contradiction=parm[11].value.unumber, .qm_1588_drop=parm[12].value.unumber, .qm_1588_multicast_contradiction=parm[13].value.unumber, .qm_byte_drop_cnt_overrun=parm[14].value.unumber, .qm_pkt_drop_cnt_overrun=parm[15].value.unumber, .qm_total_byte_cnt_underrun=parm[16].value.unumber, .qm_total_pkt_cnt_underrun=parm[17].value.unumber, .qm_fpm_ug0_underrun=parm[18].value.unumber, .qm_fpm_ug1_underrun=parm[19].value.unumber, .qm_fpm_ug2_underrun=parm[20].value.unumber, .qm_fpm_ug3_underrun=parm[21].value.unumber, .qm_timer_wraparound=parm[22].value.unumber};
        err = ag_drv_qm_intr_ctrl_isr_set(&intr_ctrl_isr);
        break;
    }
    case cli_qm_intr_ctrl_ier:
        err = ag_drv_qm_intr_ctrl_ier_set(parm[1].value.unumber);
        break;
    case cli_qm_intr_ctrl_itr:
        err = ag_drv_qm_intr_ctrl_itr_set(parm[1].value.unumber);
        break;
    case cli_qm_clk_gate_clk_gate_cntrl:
    {
        qm_clk_gate_clk_gate_cntrl clk_gate_clk_gate_cntrl = { .bypass_clk_gate=parm[1].value.unumber, .timer_val=parm[2].value.unumber, .keep_alive_en=parm[3].value.unumber, .keep_alive_intrvl=parm[4].value.unumber, .keep_alive_cyc=parm[5].value.unumber};
        err = ag_drv_qm_clk_gate_clk_gate_cntrl_set(&clk_gate_clk_gate_cntrl);
        break;
    }
    case cli_qm_cpu_indr_port_cpu_pd_indirect_ctrl:
        err = ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_qm_q_context:
    {
        qm_q_context q_context = { .wred_profile=parm[2].value.unumber, .copy_dec_profile=parm[3].value.unumber, .copy_to_ddr=parm[4].value.unumber, .ddr_copy_disable=parm[5].value.unumber, .aggregation_disable=parm[6].value.unumber, .fpm_ug=parm[7].value.unumber, .exclusive_priority=parm[8].value.unumber, .q_802_1ae=parm[9].value.unumber, .sci=parm[10].value.unumber, .fec_enable=parm[11].value.unumber, .res_profile=parm[12].value.unumber};
        err = ag_drv_qm_q_context_set(parm[1].value.unumber, &q_context);
        break;
    }
    case cli_qm_copy_decision_profile:
        err = ag_drv_qm_copy_decision_profile_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_qm_total_valid_cnt:
        err = ag_drv_qm_total_valid_cnt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_dqm_valid_cnt:
        err = ag_drv_qm_dqm_valid_cnt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_epon_q_byte_cnt:
        err = ag_drv_qm_epon_q_byte_cnt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_debug_sel:
        err = ag_drv_qm_debug_sel_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_bb_route_ovr:
        err = ag_drv_qm_bb_route_ovr_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_mactype:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_mactype_set(parm[1].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel:
    {
        qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel = { .addr = { parm[2].value.unumber, parm[3].value.unumber}};
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel_set(parm[1].value.unumber, &bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh:
    {
        qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh = { .addr = { parm[2].value.unumber, parm[3].value.unumber}};
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh_set(parm[1].value.unumber, &bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_arb_cfg:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_arb_cfg_set(parm[1].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_bbroute:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_bbroute_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_q2rnr:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_q2rnr_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_perqtask:
    {
        qm_bbh_tx_qm_bbhtx_common_configurations_perqtask bbh_tx_qm_bbhtx_common_configurations_perqtask = { .task0=parm[1].value.unumber, .task1=parm[2].value.unumber, .task2=parm[3].value.unumber, .task3=parm[4].value.unumber, .task4=parm[5].value.unumber, .task5=parm[6].value.unumber, .task6=parm[7].value.unumber, .task7=parm[8].value.unumber};
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_perqtask_set(&bbh_tx_qm_bbhtx_common_configurations_perqtask);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd:
    {
        qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd bbh_tx_qm_bbhtx_common_configurations_txrstcmd = { .cntxtrst=parm[1].value.unumber, .pdfiforst=parm[2].value.unumber, .dmaptrrst=parm[3].value.unumber, .sdmaptrrst=parm[4].value.unumber, .bpmfiforst=parm[5].value.unumber, .sbpmfiforst=parm[6].value.unumber, .okfiforst=parm[7].value.unumber, .ddrfiforst=parm[8].value.unumber, .sramfiforst=parm[9].value.unumber, .skbptrrst=parm[10].value.unumber, .stsfiforst=parm[11].value.unumber, .reqfiforst=parm[12].value.unumber, .msgfiforst=parm[13].value.unumber, .gnxtfiforst=parm[14].value.unumber, .fbnfiforst=parm[15].value.unumber};
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd_set(&bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_dbgsel:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dbgsel_set(parm[1].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl:
    {
        qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl = { .bypass_clk_gate=parm[1].value.unumber, .timer_val=parm[2].value.unumber, .keep_alive_en=parm[3].value.unumber, .keep_alive_intrvl=parm[4].value.unumber, .keep_alive_cyc=parm[5].value.unumber};
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl_set(&bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_gpr:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_gpr_set(parm[1].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdbase:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdbase_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdsize:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdsize_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en_set(parm[1].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdempty:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdempty_set(parm[1].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_txthresh:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_txthresh_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_eee:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_eee_set(parm[1].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_ts:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_ts_set(parm[1].value.unumber);
        break;
    case cli_qm_bbh_tx_qm_bbhtx_debug_counters_swrden:
    {
        qm_bbh_tx_qm_bbhtx_debug_counters_swrden bbh_tx_qm_bbhtx_debug_counters_swrden = { .pdsel=parm[1].value.unumber, .pdvsel=parm[2].value.unumber, .pdemptysel=parm[3].value.unumber, .pdfullsel=parm[4].value.unumber, .pdbemptysel=parm[5].value.unumber, .pdffwkpsel=parm[6].value.unumber, .fbnsel=parm[7].value.unumber, .fbnvsel=parm[8].value.unumber, .fbnemptysel=parm[9].value.unumber, .fbnfullsel=parm[10].value.unumber, .getnextsel=parm[11].value.unumber, .getnextvsel=parm[12].value.unumber, .getnextemptysel=parm[13].value.unumber, .getnextfullsel=parm[14].value.unumber, .gpncntxtsel=parm[15].value.unumber, .bpmsel=parm[16].value.unumber, .bpmfsel=parm[17].value.unumber, .sbpmsel=parm[18].value.unumber, .sbpmfsel=parm[19].value.unumber, .stssel=parm[20].value.unumber, .stsvsel=parm[21].value.unumber, .stsemptysel=parm[22].value.unumber, .stsfullsel=parm[23].value.unumber, .stsbemptysel=parm[24].value.unumber, .stsffwkpsel=parm[25].value.unumber, .msgsel=parm[26].value.unumber, .msgvsel=parm[27].value.unumber, .epnreqsel=parm[28].value.unumber, .datasel=parm[29].value.unumber, .reordersel=parm[30].value.unumber, .tsinfosel=parm[31].value.unumber, .mactxsel=parm[32].value.unumber};
        err = ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrden_set(&bbh_tx_qm_bbhtx_debug_counters_swrden);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_debug_counters_swrdaddr:
        err = ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrdaddr_set(parm[1].value.unumber);
        break;
    case cli_qm_dma_qm_dma_config_bbrouteovrd:
        err = ag_drv_qm_dma_qm_dma_config_bbrouteovrd_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_qm_dma_qm_dma_config_num_of_writes:
        err = ag_drv_qm_dma_qm_dma_config_num_of_writes_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_dma_qm_dma_config_num_of_reads:
        err = ag_drv_qm_dma_qm_dma_config_num_of_reads_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_dma_qm_dma_config_u_thresh:
        err = ag_drv_qm_dma_qm_dma_config_u_thresh_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_qm_dma_qm_dma_config_pri:
        err = ag_drv_qm_dma_qm_dma_config_pri_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_qm_dma_qm_dma_config_periph_source:
        err = ag_drv_qm_dma_qm_dma_config_periph_source_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_qm_dma_qm_dma_config_weight:
        err = ag_drv_qm_dma_qm_dma_config_weight_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_qm_dma_qm_dma_config_ptrrst:
        err = ag_drv_qm_dma_qm_dma_config_ptrrst_set(parm[1].value.unumber);
        break;
    case cli_qm_dma_qm_dma_config_max_otf:
        err = ag_drv_qm_dma_qm_dma_config_max_otf_set(parm[1].value.unumber);
        break;
    case cli_qm_dma_qm_dma_config_clk_gate_cntrl:
    {
        qm_dma_qm_dma_config_clk_gate_cntrl dma_qm_dma_config_clk_gate_cntrl = { .bypass_clk_gate=parm[1].value.unumber, .timer_val=parm[2].value.unumber, .keep_alive_en=parm[3].value.unumber, .keep_alive_intrvl=parm[4].value.unumber, .keep_alive_cyc=parm[5].value.unumber};
        err = ag_drv_qm_dma_qm_dma_config_clk_gate_cntrl_set(&dma_qm_dma_config_clk_gate_cntrl);
        break;
    }
    case cli_qm_dma_qm_dma_config_dbg_sel:
        err = ag_drv_qm_dma_qm_dma_config_dbg_sel_set(parm[1].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_qm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_qm_ddr_cong_ctrl:
    {
        qm_ddr_cong_ctrl ddr_cong_ctrl;
        err = ag_drv_qm_ddr_cong_ctrl_get(&ddr_cong_ctrl);
        bdmf_session_print(session, "ddr_byte_congestion_drop_enable = %u (0x%x)\n", ddr_cong_ctrl.ddr_byte_congestion_drop_enable, ddr_cong_ctrl.ddr_byte_congestion_drop_enable);
        bdmf_session_print(session, "ddr_bytes_lower_thr = %u (0x%x)\n", ddr_cong_ctrl.ddr_bytes_lower_thr, ddr_cong_ctrl.ddr_bytes_lower_thr);
        bdmf_session_print(session, "ddr_bytes_mid_thr = %u (0x%x)\n", ddr_cong_ctrl.ddr_bytes_mid_thr, ddr_cong_ctrl.ddr_bytes_mid_thr);
        bdmf_session_print(session, "ddr_bytes_higher_thr = %u (0x%x)\n", ddr_cong_ctrl.ddr_bytes_higher_thr, ddr_cong_ctrl.ddr_bytes_higher_thr);
        bdmf_session_print(session, "ddr_pd_congestion_drop_enable = %u (0x%x)\n", ddr_cong_ctrl.ddr_pd_congestion_drop_enable, ddr_cong_ctrl.ddr_pd_congestion_drop_enable);
        bdmf_session_print(session, "ddr_pipe_lower_thr = %u (0x%x)\n", ddr_cong_ctrl.ddr_pipe_lower_thr, ddr_cong_ctrl.ddr_pipe_lower_thr);
        bdmf_session_print(session, "ddr_pipe_higher_thr = %u (0x%x)\n", ddr_cong_ctrl.ddr_pipe_higher_thr, ddr_cong_ctrl.ddr_pipe_higher_thr);
        break;
    }
    case cli_qm_is_queue_not_empty:
    {
        bdmf_boolean data;
        err = ag_drv_qm_is_queue_not_empty_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_is_queue_pop_ready:
    {
        bdmf_boolean data;
        err = ag_drv_qm_is_queue_pop_ready_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_is_queue_full:
    {
        bdmf_boolean data;
        err = ag_drv_qm_is_queue_full_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_fpm_ug_thr:
    {
        qm_fpm_ug_thr fpm_ug_thr;
        err = ag_drv_qm_fpm_ug_thr_get(parm[1].value.unumber, &fpm_ug_thr);
        bdmf_session_print(session, "lower_thr = %u (0x%x)\n", fpm_ug_thr.lower_thr, fpm_ug_thr.lower_thr);
        bdmf_session_print(session, "mid_thr = %u (0x%x)\n", fpm_ug_thr.mid_thr, fpm_ug_thr.mid_thr);
        bdmf_session_print(session, "higher_thr = %u (0x%x)\n", fpm_ug_thr.higher_thr, fpm_ug_thr.higher_thr);
        break;
    }
    case cli_qm_rnr_group_cfg:
    {
        qm_rnr_group_cfg rnr_group_cfg;
        err = ag_drv_qm_rnr_group_cfg_get(parm[1].value.unumber, &rnr_group_cfg);
        bdmf_session_print(session, "start_queue = %u (0x%x)\n", rnr_group_cfg.start_queue, rnr_group_cfg.start_queue);
        bdmf_session_print(session, "end_queue = %u (0x%x)\n", rnr_group_cfg.end_queue, rnr_group_cfg.end_queue);
        bdmf_session_print(session, "pd_fifo_base = %u (0x%x)\n", rnr_group_cfg.pd_fifo_base, rnr_group_cfg.pd_fifo_base);
        bdmf_session_print(session, "pd_fifo_size = %u (0x%x)\n", rnr_group_cfg.pd_fifo_size, rnr_group_cfg.pd_fifo_size);
        bdmf_session_print(session, "upd_fifo_base = %u (0x%x)\n", rnr_group_cfg.upd_fifo_base, rnr_group_cfg.upd_fifo_base);
        bdmf_session_print(session, "upd_fifo_size = %u (0x%x)\n", rnr_group_cfg.upd_fifo_size, rnr_group_cfg.upd_fifo_size);
        bdmf_session_print(session, "rnr_bb_id = %u (0x%x)\n", rnr_group_cfg.rnr_bb_id, rnr_group_cfg.rnr_bb_id);
        bdmf_session_print(session, "rnr_task = %u (0x%x)\n", rnr_group_cfg.rnr_task, rnr_group_cfg.rnr_task);
        bdmf_session_print(session, "rnr_enable = %u (0x%x)\n", rnr_group_cfg.rnr_enable, rnr_group_cfg.rnr_enable);
        break;
    }
    case cli_qm_cpu_pd_indirect_wr_data:
    {
        uint32_t data0;
        uint32_t data1;
        uint32_t data2;
        uint32_t data3;
        err = ag_drv_qm_cpu_pd_indirect_wr_data_get(parm[1].value.unumber, &data0, &data1, &data2, &data3);
        bdmf_session_print(session, "data0 = %u (0x%x)\n", data0, data0);
        bdmf_session_print(session, "data1 = %u (0x%x)\n", data1, data1);
        bdmf_session_print(session, "data2 = %u (0x%x)\n", data2, data2);
        bdmf_session_print(session, "data3 = %u (0x%x)\n", data3, data3);
        break;
    }
    case cli_qm_cpu_pd_indirect_rd_data:
    {
        uint32_t data0;
        uint32_t data1;
        uint32_t data2;
        uint32_t data3;
        err = ag_drv_qm_cpu_pd_indirect_rd_data_get(parm[1].value.unumber, &data0, &data1, &data2, &data3);
        bdmf_session_print(session, "data0 = %u (0x%x)\n", data0, data0);
        bdmf_session_print(session, "data1 = %u (0x%x)\n", data1, data1);
        bdmf_session_print(session, "data2 = %u (0x%x)\n", data2, data2);
        bdmf_session_print(session, "data3 = %u (0x%x)\n", data3, data3);
        break;
    }
    case cli_qm_aggr_context:
    {
        uint32_t context_valid;
        err = ag_drv_qm_aggr_context_get(parm[1].value.unumber, &context_valid);
        bdmf_session_print(session, "context_valid = %u (0x%x)\n", context_valid, context_valid);
        break;
    }
    case cli_qm_wred_profile_cfg:
    {
        qm_wred_profile_cfg wred_profile_cfg;
        err = ag_drv_qm_wred_profile_cfg_get(parm[1].value.unumber, &wred_profile_cfg);
        bdmf_session_print(session, "min_thr0 = %u (0x%x)\n", wred_profile_cfg.min_thr0, wred_profile_cfg.min_thr0);
        bdmf_session_print(session, "flw_ctrl_en0 = %u (0x%x)\n", wred_profile_cfg.flw_ctrl_en0, wred_profile_cfg.flw_ctrl_en0);
        bdmf_session_print(session, "min_thr1 = %u (0x%x)\n", wred_profile_cfg.min_thr1, wred_profile_cfg.min_thr1);
        bdmf_session_print(session, "flw_ctrl_en1 = %u (0x%x)\n", wred_profile_cfg.flw_ctrl_en1, wred_profile_cfg.flw_ctrl_en1);
        bdmf_session_print(session, "max_thr0 = %u (0x%x)\n", wred_profile_cfg.max_thr0, wred_profile_cfg.max_thr0);
        bdmf_session_print(session, "max_thr1 = %u (0x%x)\n", wred_profile_cfg.max_thr1, wred_profile_cfg.max_thr1);
        bdmf_session_print(session, "slope_mantissa0 = %u (0x%x)\n", wred_profile_cfg.slope_mantissa0, wred_profile_cfg.slope_mantissa0);
        bdmf_session_print(session, "slope_exp0 = %u (0x%x)\n", wred_profile_cfg.slope_exp0, wred_profile_cfg.slope_exp0);
        bdmf_session_print(session, "slope_mantissa1 = %u (0x%x)\n", wred_profile_cfg.slope_mantissa1, wred_profile_cfg.slope_mantissa1);
        bdmf_session_print(session, "slope_exp1 = %u (0x%x)\n", wred_profile_cfg.slope_exp1, wred_profile_cfg.slope_exp1);
        break;
    }
    case cli_qm_ubus_slave:
    {
        qm_ubus_slave ubus_slave;
        err = ag_drv_qm_ubus_slave_get(&ubus_slave);
        bdmf_session_print(session, "vpb_base = %u (0x%x)\n", ubus_slave.vpb_base, ubus_slave.vpb_base);
        bdmf_session_print(session, "vpb_mask = %u (0x%x)\n", ubus_slave.vpb_mask, ubus_slave.vpb_mask);
        bdmf_session_print(session, "apb_base = %u (0x%x)\n", ubus_slave.apb_base, ubus_slave.apb_base);
        bdmf_session_print(session, "apb_mask = %u (0x%x)\n", ubus_slave.apb_mask, ubus_slave.apb_mask);
        bdmf_session_print(session, "dqm_base = %u (0x%x)\n", ubus_slave.dqm_base, ubus_slave.dqm_base);
        bdmf_session_print(session, "dqm_mask = %u (0x%x)\n", ubus_slave.dqm_mask, ubus_slave.dqm_mask);
        break;
    }
    case cli_qm_cfg_src_id:
    {
        qm_cfg_src_id cfg_src_id;
        err = ag_drv_qm_cfg_src_id_get(&cfg_src_id);
        bdmf_session_print(session, "fpmsrc = %u (0x%x)\n", cfg_src_id.fpmsrc, cfg_src_id.fpmsrc);
        bdmf_session_print(session, "sbpmsrc = %u (0x%x)\n", cfg_src_id.sbpmsrc, cfg_src_id.sbpmsrc);
        bdmf_session_print(session, "stsrnrsrc = %u (0x%x)\n", cfg_src_id.stsrnrsrc, cfg_src_id.stsrnrsrc);
        break;
    }
    case cli_qm_rnr_src_id:
    {
        uint8_t pdrnr0src;
        uint8_t pdrnr1src;
        err = ag_drv_qm_rnr_src_id_get(&pdrnr0src, &pdrnr1src);
        bdmf_session_print(session, "pdrnr0src = %u (0x%x)\n", pdrnr0src, pdrnr0src);
        bdmf_session_print(session, "pdrnr1src = %u (0x%x)\n", pdrnr1src, pdrnr1src);
        break;
    }
    case cli_qm_bbh_dma_cfg:
    {
        qm_bbh_dma_cfg bbh_dma_cfg;
        err = ag_drv_qm_bbh_dma_cfg_get(&bbh_dma_cfg);
        bdmf_session_print(session, "dmasrc = %u (0x%x)\n", bbh_dma_cfg.dmasrc, bbh_dma_cfg.dmasrc);
        bdmf_session_print(session, "descbase = %u (0x%x)\n", bbh_dma_cfg.descbase, bbh_dma_cfg.descbase);
        bdmf_session_print(session, "descsize = %u (0x%x)\n", bbh_dma_cfg.descsize, bbh_dma_cfg.descsize);
        break;
    }
    case cli_qm_dma_max_otf_read_request:
    {
        uint8_t maxreq;
        err = ag_drv_qm_dma_max_otf_read_request_get(&maxreq);
        bdmf_session_print(session, "maxreq = %u (0x%x)\n", maxreq, maxreq);
        break;
    }
    case cli_qm_dma_epon_urgent:
    {
        bdmf_boolean epnurgnt;
        err = ag_drv_qm_dma_epon_urgent_get(&epnurgnt);
        bdmf_session_print(session, "epnurgnt = %u (0x%x)\n", epnurgnt, epnurgnt);
        break;
    }
    case cli_qm_bbh_sdma_cfg:
    {
        qm_bbh_sdma_cfg bbh_sdma_cfg;
        err = ag_drv_qm_bbh_sdma_cfg_get(&bbh_sdma_cfg);
        bdmf_session_print(session, "sdmasrc = %u (0x%x)\n", bbh_sdma_cfg.sdmasrc, bbh_sdma_cfg.sdmasrc);
        bdmf_session_print(session, "descbase = %u (0x%x)\n", bbh_sdma_cfg.descbase, bbh_sdma_cfg.descbase);
        bdmf_session_print(session, "descsize = %u (0x%x)\n", bbh_sdma_cfg.descsize, bbh_sdma_cfg.descsize);
        break;
    }
    case cli_qm_sdma_max_otf_read_request:
    {
        uint8_t maxreq;
        err = ag_drv_qm_sdma_max_otf_read_request_get(&maxreq);
        bdmf_session_print(session, "maxreq = %u (0x%x)\n", maxreq, maxreq);
        break;
    }
    case cli_qm_sdma_epon_urgent:
    {
        bdmf_boolean epnurgnt;
        err = ag_drv_qm_sdma_epon_urgent_get(&epnurgnt);
        bdmf_session_print(session, "epnurgnt = %u (0x%x)\n", epnurgnt, epnurgnt);
        break;
    }
    case cli_qm_bbh_ddr_cfg:
    {
        qm_bbh_ddr_cfg bbh_ddr_cfg;
        err = ag_drv_qm_bbh_ddr_cfg_get(&bbh_ddr_cfg);
        bdmf_session_print(session, "bufsize = %u (0x%x)\n", bbh_ddr_cfg.bufsize, bbh_ddr_cfg.bufsize);
        bdmf_session_print(session, "byteresul = %u (0x%x)\n", bbh_ddr_cfg.byteresul, bbh_ddr_cfg.byteresul);
        bdmf_session_print(session, "ddrtxoffset = %u (0x%x)\n", bbh_ddr_cfg.ddrtxoffset, bbh_ddr_cfg.ddrtxoffset);
        bdmf_session_print(session, "hnsize0 = %u (0x%x)\n", bbh_ddr_cfg.hnsize0, bbh_ddr_cfg.hnsize0);
        bdmf_session_print(session, "hnsize1 = %u (0x%x)\n", bbh_ddr_cfg.hnsize1, bbh_ddr_cfg.hnsize1);
        break;
    }
    case cli_qm_debug_counters:
    {
        qm_debug_counters debug_counters;
        err = ag_drv_qm_debug_counters_get(&debug_counters);
        bdmf_session_print(session, "srampd = %u (0x%x)\n", debug_counters.srampd, debug_counters.srampd);
        bdmf_session_print(session, "ddrpd = %u (0x%x)\n", debug_counters.ddrpd, debug_counters.ddrpd);
        bdmf_session_print(session, "pddrop = %u (0x%x)\n", debug_counters.pddrop, debug_counters.pddrop);
        bdmf_session_print(session, "stscnt = %u (0x%x)\n", debug_counters.stscnt, debug_counters.stscnt);
        bdmf_session_print(session, "stsdrop = %u (0x%x)\n", debug_counters.stsdrop, debug_counters.stsdrop);
        bdmf_session_print(session, "msgcnt = %u (0x%x)\n", debug_counters.msgcnt, debug_counters.msgcnt);
        bdmf_session_print(session, "msgdrop = %u (0x%x)\n", debug_counters.msgdrop, debug_counters.msgdrop);
        bdmf_session_print(session, "getnextnull = %u (0x%x)\n", debug_counters.getnextnull, debug_counters.getnextnull);
        bdmf_session_print(session, "lenerr = %u (0x%x)\n", debug_counters.lenerr, debug_counters.lenerr);
        bdmf_session_print(session, "aggrlenerr = %u (0x%x)\n", debug_counters.aggrlenerr, debug_counters.aggrlenerr);
        bdmf_session_print(session, "srampkt = %u (0x%x)\n", debug_counters.srampkt, debug_counters.srampkt);
        bdmf_session_print(session, "ddrpkt = %u (0x%x)\n", debug_counters.ddrpkt, debug_counters.ddrpkt);
        bdmf_session_print(session, "flshpkts = %u (0x%x)\n", debug_counters.flshpkts, debug_counters.flshpkts);
        break;
    }
    case cli_qm_debug_info:
    {
        qm_debug_info debug_info;
        err = ag_drv_qm_debug_info_get(&debug_info);
        bdmf_session_print(session, "nempty = %u (0x%x)\n", debug_info.nempty, debug_info.nempty);
        bdmf_session_print(session, "urgnt = %u (0x%x)\n", debug_info.urgnt, debug_info.urgnt);
        bdmf_session_print(session, "sel_src = %u (0x%x)\n", debug_info.sel_src, debug_info.sel_src);
        bdmf_session_print(session, "address = %u (0x%x)\n", debug_info.address, debug_info.address);
        bdmf_session_print(session, "datacs = %u (0x%x)\n", debug_info.datacs, debug_info.datacs);
        bdmf_session_print(session, "cdcs = %u (0x%x)\n", debug_info.cdcs, debug_info.cdcs);
        bdmf_session_print(session, "rrcs = %u (0x%x)\n", debug_info.rrcs, debug_info.rrcs);
        bdmf_session_print(session, "valid = %u (0x%x)\n", debug_info.valid, debug_info.valid);
        bdmf_session_print(session, "ready = %u (0x%x)\n", debug_info.ready, debug_info.ready);
        break;
    }
    case cli_qm_enable_ctrl:
    {
        qm_enable_ctrl enable_ctrl;
        err = ag_drv_qm_enable_ctrl_get(&enable_ctrl);
        bdmf_session_print(session, "fpm_prefetch_enable = %u (0x%x)\n", enable_ctrl.fpm_prefetch_enable, enable_ctrl.fpm_prefetch_enable);
        bdmf_session_print(session, "reorder_credit_enable = %u (0x%x)\n", enable_ctrl.reorder_credit_enable, enable_ctrl.reorder_credit_enable);
        bdmf_session_print(session, "dqm_pop_enable = %u (0x%x)\n", enable_ctrl.dqm_pop_enable, enable_ctrl.dqm_pop_enable);
        bdmf_session_print(session, "rmt_fixed_arb_enable = %u (0x%x)\n", enable_ctrl.rmt_fixed_arb_enable, enable_ctrl.rmt_fixed_arb_enable);
        bdmf_session_print(session, "dqm_push_fixed_arb_enable = %u (0x%x)\n", enable_ctrl.dqm_push_fixed_arb_enable, enable_ctrl.dqm_push_fixed_arb_enable);
        break;
    }
    case cli_qm_reset_ctrl:
    {
        qm_reset_ctrl reset_ctrl;
        err = ag_drv_qm_reset_ctrl_get(&reset_ctrl);
        bdmf_session_print(session, "fpm_prefetch0_sw_rst = %u (0x%x)\n", reset_ctrl.fpm_prefetch0_sw_rst, reset_ctrl.fpm_prefetch0_sw_rst);
        bdmf_session_print(session, "fpm_prefetch1_sw_rst = %u (0x%x)\n", reset_ctrl.fpm_prefetch1_sw_rst, reset_ctrl.fpm_prefetch1_sw_rst);
        bdmf_session_print(session, "fpm_prefetch2_sw_rst = %u (0x%x)\n", reset_ctrl.fpm_prefetch2_sw_rst, reset_ctrl.fpm_prefetch2_sw_rst);
        bdmf_session_print(session, "fpm_prefetch3_sw_rst = %u (0x%x)\n", reset_ctrl.fpm_prefetch3_sw_rst, reset_ctrl.fpm_prefetch3_sw_rst);
        bdmf_session_print(session, "normal_rmt_sw_rst = %u (0x%x)\n", reset_ctrl.normal_rmt_sw_rst, reset_ctrl.normal_rmt_sw_rst);
        bdmf_session_print(session, "non_delayed_rmt_sw_rst = %u (0x%x)\n", reset_ctrl.non_delayed_rmt_sw_rst, reset_ctrl.non_delayed_rmt_sw_rst);
        bdmf_session_print(session, "pre_cm_fifo_sw_rst = %u (0x%x)\n", reset_ctrl.pre_cm_fifo_sw_rst, reset_ctrl.pre_cm_fifo_sw_rst);
        bdmf_session_print(session, "cm_rd_pd_fifo_sw_rst = %u (0x%x)\n", reset_ctrl.cm_rd_pd_fifo_sw_rst, reset_ctrl.cm_rd_pd_fifo_sw_rst);
        bdmf_session_print(session, "cm_wr_pd_fifo_sw_rst = %u (0x%x)\n", reset_ctrl.cm_wr_pd_fifo_sw_rst, reset_ctrl.cm_wr_pd_fifo_sw_rst);
        bdmf_session_print(session, "bb0_output_fifo_sw_rst = %u (0x%x)\n", reset_ctrl.bb0_output_fifo_sw_rst, reset_ctrl.bb0_output_fifo_sw_rst);
        bdmf_session_print(session, "bb1_output_fifo_sw_rst = %u (0x%x)\n", reset_ctrl.bb1_output_fifo_sw_rst, reset_ctrl.bb1_output_fifo_sw_rst);
        bdmf_session_print(session, "bb1_input_fifo_sw_rst = %u (0x%x)\n", reset_ctrl.bb1_input_fifo_sw_rst, reset_ctrl.bb1_input_fifo_sw_rst);
        bdmf_session_print(session, "tm_fifo_ptr_sw_rst = %u (0x%x)\n", reset_ctrl.tm_fifo_ptr_sw_rst, reset_ctrl.tm_fifo_ptr_sw_rst);
        bdmf_session_print(session, "non_delayed_out_fifo_sw_rst = %u (0x%x)\n", reset_ctrl.non_delayed_out_fifo_sw_rst, reset_ctrl.non_delayed_out_fifo_sw_rst);
        break;
    }
    case cli_qm_drop_counters_ctrl:
    {
        qm_drop_counters_ctrl drop_counters_ctrl;
        err = ag_drv_qm_drop_counters_ctrl_get(&drop_counters_ctrl);
        bdmf_session_print(session, "read_clear_pkts = %u (0x%x)\n", drop_counters_ctrl.read_clear_pkts, drop_counters_ctrl.read_clear_pkts);
        bdmf_session_print(session, "read_clear_bytes = %u (0x%x)\n", drop_counters_ctrl.read_clear_bytes, drop_counters_ctrl.read_clear_bytes);
        bdmf_session_print(session, "disable_wrap_around_pkts = %u (0x%x)\n", drop_counters_ctrl.disable_wrap_around_pkts, drop_counters_ctrl.disable_wrap_around_pkts);
        bdmf_session_print(session, "disable_wrap_around_bytes = %u (0x%x)\n", drop_counters_ctrl.disable_wrap_around_bytes, drop_counters_ctrl.disable_wrap_around_bytes);
        bdmf_session_print(session, "free_with_context_last_search = %u (0x%x)\n", drop_counters_ctrl.free_with_context_last_search, drop_counters_ctrl.free_with_context_last_search);
        bdmf_session_print(session, "wred_disable = %u (0x%x)\n", drop_counters_ctrl.wred_disable, drop_counters_ctrl.wred_disable);
        bdmf_session_print(session, "ddr_pd_congestion_disable = %u (0x%x)\n", drop_counters_ctrl.ddr_pd_congestion_disable, drop_counters_ctrl.ddr_pd_congestion_disable);
        bdmf_session_print(session, "ddr_byte_congestion_disable = %u (0x%x)\n", drop_counters_ctrl.ddr_byte_congestion_disable, drop_counters_ctrl.ddr_byte_congestion_disable);
        bdmf_session_print(session, "ddr_occupancy_disable = %u (0x%x)\n", drop_counters_ctrl.ddr_occupancy_disable, drop_counters_ctrl.ddr_occupancy_disable);
        bdmf_session_print(session, "ddr_fpm_congestion_disable = %u (0x%x)\n", drop_counters_ctrl.ddr_fpm_congestion_disable, drop_counters_ctrl.ddr_fpm_congestion_disable);
        bdmf_session_print(session, "fpm_ug_disable = %u (0x%x)\n", drop_counters_ctrl.fpm_ug_disable, drop_counters_ctrl.fpm_ug_disable);
        bdmf_session_print(session, "queue_occupancy_ddr_copy_decision_disable = %u (0x%x)\n", drop_counters_ctrl.queue_occupancy_ddr_copy_decision_disable, drop_counters_ctrl.queue_occupancy_ddr_copy_decision_disable);
        bdmf_session_print(session, "psram_occupancy_ddr_copy_decision_disable = %u (0x%x)\n", drop_counters_ctrl.psram_occupancy_ddr_copy_decision_disable, drop_counters_ctrl.psram_occupancy_ddr_copy_decision_disable);
        bdmf_session_print(session, "dont_send_mc_bit_to_bbh = %u (0x%x)\n", drop_counters_ctrl.dont_send_mc_bit_to_bbh, drop_counters_ctrl.dont_send_mc_bit_to_bbh);
        bdmf_session_print(session, "close_aggregation_on_timeout_disable = %u (0x%x)\n", drop_counters_ctrl.close_aggregation_on_timeout_disable, drop_counters_ctrl.close_aggregation_on_timeout_disable);
        bdmf_session_print(session, "fpm_congestion_buf_release_mechanism_disable = %u (0x%x)\n", drop_counters_ctrl.fpm_congestion_buf_release_mechanism_disable, drop_counters_ctrl.fpm_congestion_buf_release_mechanism_disable);
        bdmf_session_print(session, "fpm_buffer_global_res_enable = %u (0x%x)\n", drop_counters_ctrl.fpm_buffer_global_res_enable, drop_counters_ctrl.fpm_buffer_global_res_enable);
        bdmf_session_print(session, "qm_preserve_pd_with_fpm = %u (0x%x)\n", drop_counters_ctrl.qm_preserve_pd_with_fpm, drop_counters_ctrl.qm_preserve_pd_with_fpm);
        bdmf_session_print(session, "qm_residue_per_queue = %u (0x%x)\n", drop_counters_ctrl.qm_residue_per_queue, drop_counters_ctrl.qm_residue_per_queue);
        bdmf_session_print(session, "ghost_rpt_update_after_close_agg_en = %u (0x%x)\n", drop_counters_ctrl.ghost_rpt_update_after_close_agg_en, drop_counters_ctrl.ghost_rpt_update_after_close_agg_en);
        bdmf_session_print(session, "fpm_ug_flow_ctrl_disable = %u (0x%x)\n", drop_counters_ctrl.fpm_ug_flow_ctrl_disable, drop_counters_ctrl.fpm_ug_flow_ctrl_disable);
        bdmf_session_print(session, "ddr_write_multi_slave_en = %u (0x%x)\n", drop_counters_ctrl.ddr_write_multi_slave_en, drop_counters_ctrl.ddr_write_multi_slave_en);
        bdmf_session_print(session, "ddr_pd_congestion_agg_priority = %u (0x%x)\n", drop_counters_ctrl.ddr_pd_congestion_agg_priority, drop_counters_ctrl.ddr_pd_congestion_agg_priority);
        bdmf_session_print(session, "psram_occupancy_drop_disable = %u (0x%x)\n", drop_counters_ctrl.psram_occupancy_drop_disable, drop_counters_ctrl.psram_occupancy_drop_disable);
        bdmf_session_print(session, "qm_ddr_write_alignment = %u (0x%x)\n", drop_counters_ctrl.qm_ddr_write_alignment, drop_counters_ctrl.qm_ddr_write_alignment);
        bdmf_session_print(session, "exclusive_dont_drop = %u (0x%x)\n", drop_counters_ctrl.exclusive_dont_drop, drop_counters_ctrl.exclusive_dont_drop);
        bdmf_session_print(session, "exclusive_dont_drop_bp_en = %u (0x%x)\n", drop_counters_ctrl.exclusive_dont_drop_bp_en, drop_counters_ctrl.exclusive_dont_drop_bp_en);
        bdmf_session_print(session, "gpon_dbr_ceil = %u (0x%x)\n", drop_counters_ctrl.gpon_dbr_ceil, drop_counters_ctrl.gpon_dbr_ceil);
        break;
    }
    case cli_qm_fpm_ctrl:
    {
        bdmf_boolean fpm_pool_bp_enable;
        bdmf_boolean fpm_congestion_bp_enable;
        uint8_t fpm_prefetch_min_pool_size;
        uint8_t fpm_prefetch_pending_req_limit;
        err = ag_drv_qm_fpm_ctrl_get(&fpm_pool_bp_enable, &fpm_congestion_bp_enable, &fpm_prefetch_min_pool_size, &fpm_prefetch_pending_req_limit);
        bdmf_session_print(session, "fpm_pool_bp_enable = %u (0x%x)\n", fpm_pool_bp_enable, fpm_pool_bp_enable);
        bdmf_session_print(session, "fpm_congestion_bp_enable = %u (0x%x)\n", fpm_congestion_bp_enable, fpm_congestion_bp_enable);
        bdmf_session_print(session, "fpm_prefetch_min_pool_size = %u (0x%x)\n", fpm_prefetch_min_pool_size, fpm_prefetch_min_pool_size);
        bdmf_session_print(session, "fpm_prefetch_pending_req_limit = %u (0x%x)\n", fpm_prefetch_pending_req_limit, fpm_prefetch_pending_req_limit);
        break;
    }
    case cli_qm_qm_pd_cong_ctrl:
    {
        uint32_t total_pd_thr;
        err = ag_drv_qm_qm_pd_cong_ctrl_get(&total_pd_thr);
        bdmf_session_print(session, "total_pd_thr = %u (0x%x)\n", total_pd_thr, total_pd_thr);
        break;
    }
    case cli_qm_global_cfg_abs_drop_queue:
    {
        uint16_t abs_drop_queue;
        bdmf_boolean abs_drop_queue_en;
        err = ag_drv_qm_global_cfg_abs_drop_queue_get(&abs_drop_queue, &abs_drop_queue_en);
        bdmf_session_print(session, "abs_drop_queue = %u (0x%x)\n", abs_drop_queue, abs_drop_queue);
        bdmf_session_print(session, "abs_drop_queue_en = %u (0x%x)\n", abs_drop_queue_en, abs_drop_queue_en);
        break;
    }
    case cli_qm_global_cfg_aggregation_ctrl:
    {
        uint16_t max_agg_bytes;
        uint8_t max_agg_pkts;
        err = ag_drv_qm_global_cfg_aggregation_ctrl_get(&max_agg_bytes, &max_agg_pkts);
        bdmf_session_print(session, "max_agg_bytes = %u (0x%x)\n", max_agg_bytes, max_agg_bytes);
        bdmf_session_print(session, "max_agg_pkts = %u (0x%x)\n", max_agg_pkts, max_agg_pkts);
        break;
    }
    case cli_qm_fpm_base_addr:
    {
        uint32_t fpm_base_addr;
        err = ag_drv_qm_fpm_base_addr_get(&fpm_base_addr);
        bdmf_session_print(session, "fpm_base_addr = %u (0x%x)\n", fpm_base_addr, fpm_base_addr);
        break;
    }
    case cli_qm_global_cfg_fpm_coherent_base_addr:
    {
        uint32_t fpm_base_addr;
        err = ag_drv_qm_global_cfg_fpm_coherent_base_addr_get(&fpm_base_addr);
        bdmf_session_print(session, "fpm_base_addr = %u (0x%x)\n", fpm_base_addr, fpm_base_addr);
        break;
    }
    case cli_qm_ddr_sop_offset:
    {
        uint16_t ddr_sop_offset0;
        uint16_t ddr_sop_offset1;
        err = ag_drv_qm_ddr_sop_offset_get(&ddr_sop_offset0, &ddr_sop_offset1);
        bdmf_session_print(session, "ddr_sop_offset0 = %u (0x%x)\n", ddr_sop_offset0, ddr_sop_offset0);
        bdmf_session_print(session, "ddr_sop_offset1 = %u (0x%x)\n", ddr_sop_offset1, ddr_sop_offset1);
        break;
    }
    case cli_qm_epon_overhead_ctrl:
    {
        qm_epon_overhead_ctrl epon_overhead_ctrl;
        err = ag_drv_qm_epon_overhead_ctrl_get(&epon_overhead_ctrl);
        bdmf_session_print(session, "epon_line_rate = %u (0x%x)\n", epon_overhead_ctrl.epon_line_rate, epon_overhead_ctrl.epon_line_rate);
        bdmf_session_print(session, "epon_crc_add_disable = %u (0x%x)\n", epon_overhead_ctrl.epon_crc_add_disable, epon_overhead_ctrl.epon_crc_add_disable);
        bdmf_session_print(session, "mac_flow_overwrite_crc_en = %u (0x%x)\n", epon_overhead_ctrl.mac_flow_overwrite_crc_en, epon_overhead_ctrl.mac_flow_overwrite_crc_en);
        bdmf_session_print(session, "mac_flow_overwrite_crc = %u (0x%x)\n", epon_overhead_ctrl.mac_flow_overwrite_crc, epon_overhead_ctrl.mac_flow_overwrite_crc);
        bdmf_session_print(session, "fec_ipg_length = %u (0x%x)\n", epon_overhead_ctrl.fec_ipg_length, epon_overhead_ctrl.fec_ipg_length);
        break;
    }
    case cli_qm_global_cfg_qm_aggregation_timer_ctrl:
    {
        uint8_t prescaler_granularity;
        uint8_t aggregation_timeout_value;
        err = ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_get(&prescaler_granularity, &aggregation_timeout_value);
        bdmf_session_print(session, "prescaler_granularity = %u (0x%x)\n", prescaler_granularity, prescaler_granularity);
        bdmf_session_print(session, "aggregation_timeout_value = %u (0x%x)\n", aggregation_timeout_value, aggregation_timeout_value);
        break;
    }
    case cli_qm_global_cfg_qm_fpm_buffer_grp_res:
    {
        uint8_t res_thr_0;
        uint8_t res_thr_1;
        uint8_t res_thr_2;
        uint8_t res_thr_3;
        err = ag_drv_qm_global_cfg_qm_fpm_buffer_grp_res_get(parm[1].value.unumber, &res_thr_0, &res_thr_1, &res_thr_2, &res_thr_3);
        bdmf_session_print(session, "res_thr_0 = %u (0x%x)\n", res_thr_0, res_thr_0);
        bdmf_session_print(session, "res_thr_1 = %u (0x%x)\n", res_thr_1, res_thr_1);
        bdmf_session_print(session, "res_thr_2 = %u (0x%x)\n", res_thr_2, res_thr_2);
        bdmf_session_print(session, "res_thr_3 = %u (0x%x)\n", res_thr_3, res_thr_3);
        break;
    }
    case cli_qm_global_cfg_qm_fpm_buffer_gbl_thr:
    {
        uint16_t res_thr_global;
        err = ag_drv_qm_global_cfg_qm_fpm_buffer_gbl_thr_get(&res_thr_global);
        bdmf_session_print(session, "res_thr_global = %u (0x%x)\n", res_thr_global, res_thr_global);
        break;
    }
    case cli_qm_global_cfg_qm_flow_ctrl_rnr_cfg:
    {
        uint8_t rnr_bb_id;
        uint8_t rnr_task;
        bdmf_boolean rnr_enable;
        err = ag_drv_qm_global_cfg_qm_flow_ctrl_rnr_cfg_get(&rnr_bb_id, &rnr_task, &rnr_enable);
        bdmf_session_print(session, "rnr_bb_id = %u (0x%x)\n", rnr_bb_id, rnr_bb_id);
        bdmf_session_print(session, "rnr_task = %u (0x%x)\n", rnr_task, rnr_task);
        bdmf_session_print(session, "rnr_enable = %u (0x%x)\n", rnr_enable, rnr_enable);
        break;
    }
    case cli_qm_global_cfg_qm_flow_ctrl_intr:
    {
        uint8_t qm_flow_ctrl_intr;
        err = ag_drv_qm_global_cfg_qm_flow_ctrl_intr_get(&qm_flow_ctrl_intr);
        bdmf_session_print(session, "qm_flow_ctrl_intr = %u (0x%x)\n", qm_flow_ctrl_intr, qm_flow_ctrl_intr);
        break;
    }
    case cli_qm_global_cfg_qm_fpm_ug_gbl_cnt:
    {
        uint32_t fpm_gbl_cnt;
        err = ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_get(&fpm_gbl_cnt);
        bdmf_session_print(session, "fpm_gbl_cnt = %u (0x%x)\n", fpm_gbl_cnt, fpm_gbl_cnt);
        break;
    }
    case cli_qm_global_cfg_qm_egress_flush_queue:
    {
        uint16_t queue_num;
        bdmf_boolean flush_en;
        err = ag_drv_qm_global_cfg_qm_egress_flush_queue_get(&queue_num, &flush_en);
        bdmf_session_print(session, "queue_num = %u (0x%x)\n", queue_num, queue_num);
        bdmf_session_print(session, "flush_en = %u (0x%x)\n", flush_en, flush_en);
        break;
    }
    case cli_qm_fpm_pool_thr:
    {
        qm_fpm_pool_thr fpm_pool_thr;
        err = ag_drv_qm_fpm_pool_thr_get(parm[1].value.unumber, &fpm_pool_thr);
        bdmf_session_print(session, "lower_thr = %u (0x%x)\n", fpm_pool_thr.lower_thr, fpm_pool_thr.lower_thr);
        bdmf_session_print(session, "higher_thr = %u (0x%x)\n", fpm_pool_thr.higher_thr, fpm_pool_thr.higher_thr);
        break;
    }
    case cli_qm_fpm_ug_cnt:
    {
        uint16_t fpm_ug_cnt;
        err = ag_drv_qm_fpm_ug_cnt_get(parm[1].value.unumber, &fpm_ug_cnt);
        bdmf_session_print(session, "fpm_ug_cnt = %u (0x%x)\n", fpm_ug_cnt, fpm_ug_cnt);
        break;
    }
    case cli_qm_intr_ctrl_isr:
    {
        qm_intr_ctrl_isr intr_ctrl_isr;
        err = ag_drv_qm_intr_ctrl_isr_get(&intr_ctrl_isr);
        bdmf_session_print(session, "qm_dqm_pop_on_empty = %u (0x%x)\n", intr_ctrl_isr.qm_dqm_pop_on_empty, intr_ctrl_isr.qm_dqm_pop_on_empty);
        bdmf_session_print(session, "qm_dqm_push_on_full = %u (0x%x)\n", intr_ctrl_isr.qm_dqm_push_on_full, intr_ctrl_isr.qm_dqm_push_on_full);
        bdmf_session_print(session, "qm_cpu_pop_on_empty = %u (0x%x)\n", intr_ctrl_isr.qm_cpu_pop_on_empty, intr_ctrl_isr.qm_cpu_pop_on_empty);
        bdmf_session_print(session, "qm_cpu_push_on_full = %u (0x%x)\n", intr_ctrl_isr.qm_cpu_push_on_full, intr_ctrl_isr.qm_cpu_push_on_full);
        bdmf_session_print(session, "qm_normal_queue_pd_no_credit = %u (0x%x)\n", intr_ctrl_isr.qm_normal_queue_pd_no_credit, intr_ctrl_isr.qm_normal_queue_pd_no_credit);
        bdmf_session_print(session, "qm_non_delayed_queue_pd_no_credit = %u (0x%x)\n", intr_ctrl_isr.qm_non_delayed_queue_pd_no_credit, intr_ctrl_isr.qm_non_delayed_queue_pd_no_credit);
        bdmf_session_print(session, "qm_non_valid_queue = %u (0x%x)\n", intr_ctrl_isr.qm_non_valid_queue, intr_ctrl_isr.qm_non_valid_queue);
        bdmf_session_print(session, "qm_agg_coherent_inconsistency = %u (0x%x)\n", intr_ctrl_isr.qm_agg_coherent_inconsistency, intr_ctrl_isr.qm_agg_coherent_inconsistency);
        bdmf_session_print(session, "qm_force_copy_on_non_delayed = %u (0x%x)\n", intr_ctrl_isr.qm_force_copy_on_non_delayed, intr_ctrl_isr.qm_force_copy_on_non_delayed);
        bdmf_session_print(session, "qm_fpm_pool_size_nonexistent = %u (0x%x)\n", intr_ctrl_isr.qm_fpm_pool_size_nonexistent, intr_ctrl_isr.qm_fpm_pool_size_nonexistent);
        bdmf_session_print(session, "qm_target_mem_abs_contradiction = %u (0x%x)\n", intr_ctrl_isr.qm_target_mem_abs_contradiction, intr_ctrl_isr.qm_target_mem_abs_contradiction);
        bdmf_session_print(session, "qm_1588_drop = %u (0x%x)\n", intr_ctrl_isr.qm_1588_drop, intr_ctrl_isr.qm_1588_drop);
        bdmf_session_print(session, "qm_1588_multicast_contradiction = %u (0x%x)\n", intr_ctrl_isr.qm_1588_multicast_contradiction, intr_ctrl_isr.qm_1588_multicast_contradiction);
        bdmf_session_print(session, "qm_byte_drop_cnt_overrun = %u (0x%x)\n", intr_ctrl_isr.qm_byte_drop_cnt_overrun, intr_ctrl_isr.qm_byte_drop_cnt_overrun);
        bdmf_session_print(session, "qm_pkt_drop_cnt_overrun = %u (0x%x)\n", intr_ctrl_isr.qm_pkt_drop_cnt_overrun, intr_ctrl_isr.qm_pkt_drop_cnt_overrun);
        bdmf_session_print(session, "qm_total_byte_cnt_underrun = %u (0x%x)\n", intr_ctrl_isr.qm_total_byte_cnt_underrun, intr_ctrl_isr.qm_total_byte_cnt_underrun);
        bdmf_session_print(session, "qm_total_pkt_cnt_underrun = %u (0x%x)\n", intr_ctrl_isr.qm_total_pkt_cnt_underrun, intr_ctrl_isr.qm_total_pkt_cnt_underrun);
        bdmf_session_print(session, "qm_fpm_ug0_underrun = %u (0x%x)\n", intr_ctrl_isr.qm_fpm_ug0_underrun, intr_ctrl_isr.qm_fpm_ug0_underrun);
        bdmf_session_print(session, "qm_fpm_ug1_underrun = %u (0x%x)\n", intr_ctrl_isr.qm_fpm_ug1_underrun, intr_ctrl_isr.qm_fpm_ug1_underrun);
        bdmf_session_print(session, "qm_fpm_ug2_underrun = %u (0x%x)\n", intr_ctrl_isr.qm_fpm_ug2_underrun, intr_ctrl_isr.qm_fpm_ug2_underrun);
        bdmf_session_print(session, "qm_fpm_ug3_underrun = %u (0x%x)\n", intr_ctrl_isr.qm_fpm_ug3_underrun, intr_ctrl_isr.qm_fpm_ug3_underrun);
        bdmf_session_print(session, "qm_timer_wraparound = %u (0x%x)\n", intr_ctrl_isr.qm_timer_wraparound, intr_ctrl_isr.qm_timer_wraparound);
        break;
    }
    case cli_qm_intr_ctrl_ism:
    {
        uint32_t ism;
        err = ag_drv_qm_intr_ctrl_ism_get(&ism);
        bdmf_session_print(session, "ism = %u (0x%x)\n", ism, ism);
        break;
    }
    case cli_qm_intr_ctrl_ier:
    {
        uint32_t iem;
        err = ag_drv_qm_intr_ctrl_ier_get(&iem);
        bdmf_session_print(session, "iem = %u (0x%x)\n", iem, iem);
        break;
    }
    case cli_qm_clk_gate_clk_gate_cntrl:
    {
        qm_clk_gate_clk_gate_cntrl clk_gate_clk_gate_cntrl;
        err = ag_drv_qm_clk_gate_clk_gate_cntrl_get(&clk_gate_clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u (0x%x)\n", clk_gate_clk_gate_cntrl.bypass_clk_gate, clk_gate_clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u (0x%x)\n", clk_gate_clk_gate_cntrl.timer_val, clk_gate_clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u (0x%x)\n", clk_gate_clk_gate_cntrl.keep_alive_en, clk_gate_clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intrvl = %u (0x%x)\n", clk_gate_clk_gate_cntrl.keep_alive_intrvl, clk_gate_clk_gate_cntrl.keep_alive_intrvl);
        bdmf_session_print(session, "keep_alive_cyc = %u (0x%x)\n", clk_gate_clk_gate_cntrl.keep_alive_cyc, clk_gate_clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    case cli_qm_cpu_indr_port_cpu_pd_indirect_ctrl:
    {
        uint16_t queue_num;
        uint8_t cmd;
        bdmf_boolean done;
        bdmf_boolean error;
        err = ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_get(parm[1].value.unumber, &queue_num, &cmd, &done, &error);
        bdmf_session_print(session, "queue_num = %u (0x%x)\n", queue_num, queue_num);
        bdmf_session_print(session, "cmd = %u (0x%x)\n", cmd, cmd);
        bdmf_session_print(session, "done = %u (0x%x)\n", done, done);
        bdmf_session_print(session, "error = %u (0x%x)\n", error, error);
        break;
    }
    case cli_qm_q_context:
    {
        qm_q_context q_context;
        err = ag_drv_qm_q_context_get(parm[1].value.unumber, &q_context);
        bdmf_session_print(session, "wred_profile = %u (0x%x)\n", q_context.wred_profile, q_context.wred_profile);
        bdmf_session_print(session, "copy_dec_profile = %u (0x%x)\n", q_context.copy_dec_profile, q_context.copy_dec_profile);
        bdmf_session_print(session, "copy_to_ddr = %u (0x%x)\n", q_context.copy_to_ddr, q_context.copy_to_ddr);
        bdmf_session_print(session, "ddr_copy_disable = %u (0x%x)\n", q_context.ddr_copy_disable, q_context.ddr_copy_disable);
        bdmf_session_print(session, "aggregation_disable = %u (0x%x)\n", q_context.aggregation_disable, q_context.aggregation_disable);
        bdmf_session_print(session, "fpm_ug = %u (0x%x)\n", q_context.fpm_ug, q_context.fpm_ug);
        bdmf_session_print(session, "exclusive_priority = %u (0x%x)\n", q_context.exclusive_priority, q_context.exclusive_priority);
        bdmf_session_print(session, "q_802_1ae = %u (0x%x)\n", q_context.q_802_1ae, q_context.q_802_1ae);
        bdmf_session_print(session, "sci = %u (0x%x)\n", q_context.sci, q_context.sci);
        bdmf_session_print(session, "fec_enable = %u (0x%x)\n", q_context.fec_enable, q_context.fec_enable);
        bdmf_session_print(session, "res_profile = %u (0x%x)\n", q_context.res_profile, q_context.res_profile);
        break;
    }
    case cli_qm_copy_decision_profile:
    {
        uint32_t queue_occupancy_thr;
        bdmf_boolean psram_thr;
        err = ag_drv_qm_copy_decision_profile_get(parm[1].value.unumber, &queue_occupancy_thr, &psram_thr);
        bdmf_session_print(session, "queue_occupancy_thr = %u (0x%x)\n", queue_occupancy_thr, queue_occupancy_thr);
        bdmf_session_print(session, "psram_thr = %u (0x%x)\n", psram_thr, psram_thr);
        break;
    }
    case cli_qm_total_valid_cnt:
    {
        uint32_t data;
        err = ag_drv_qm_total_valid_cnt_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_dqm_valid_cnt:
    {
        uint32_t data;
        err = ag_drv_qm_dqm_valid_cnt_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_drop_counter:
    {
        uint32_t data;
        err = ag_drv_qm_drop_counter_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_epon_q_byte_cnt:
    {
        uint32_t data;
        err = ag_drv_qm_epon_q_byte_cnt_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_epon_q_status:
    {
        uint32_t status_bit_vector;
        err = ag_drv_qm_epon_q_status_get(parm[1].value.unumber, &status_bit_vector);
        bdmf_session_print(session, "status_bit_vector = %u (0x%x)\n", status_bit_vector, status_bit_vector);
        break;
    }
    case cli_qm_rd_data_pool0:
    {
        uint32_t data;
        err = ag_drv_qm_rd_data_pool0_get(&data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_rd_data_pool1:
    {
        uint32_t data;
        err = ag_drv_qm_rd_data_pool1_get(&data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_rd_data_pool2:
    {
        uint32_t data;
        err = ag_drv_qm_rd_data_pool2_get(&data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_rd_data_pool3:
    {
        uint32_t data;
        err = ag_drv_qm_rd_data_pool3_get(&data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_pdfifo_ptr:
    {
        uint8_t wr_ptr;
        uint8_t rd_ptr;
        err = ag_drv_qm_pdfifo_ptr_get(parm[1].value.unumber, &wr_ptr, &rd_ptr);
        bdmf_session_print(session, "wr_ptr = %u (0x%x)\n", wr_ptr, wr_ptr);
        bdmf_session_print(session, "rd_ptr = %u (0x%x)\n", rd_ptr, rd_ptr);
        break;
    }
    case cli_qm_update_fifo_ptr:
    {
        uint16_t wr_ptr;
        uint8_t rd_ptr;
        err = ag_drv_qm_update_fifo_ptr_get(parm[1].value.unumber, &wr_ptr, &rd_ptr);
        bdmf_session_print(session, "wr_ptr = %u (0x%x)\n", wr_ptr, wr_ptr);
        bdmf_session_print(session, "rd_ptr = %u (0x%x)\n", rd_ptr, rd_ptr);
        break;
    }
    case cli_qm_debug_sel:
    {
        uint8_t select;
        bdmf_boolean enable;
        err = ag_drv_qm_debug_sel_get(&select, &enable);
        bdmf_session_print(session, "select = %u (0x%x)\n", select, select);
        bdmf_session_print(session, "enable = %u (0x%x)\n", enable, enable);
        break;
    }
    case cli_qm_debug_bus_lsb:
    {
        uint32_t data;
        err = ag_drv_qm_debug_bus_lsb_get(&data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_debug_bus_msb:
    {
        uint32_t data;
        err = ag_drv_qm_debug_bus_msb_get(&data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_qm_spare_config:
    {
        uint32_t data;
        err = ag_drv_qm_qm_spare_config_get(&data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_good_lvl1_pkts_cnt:
    {
        uint32_t good_lvl1_pkts;
        err = ag_drv_qm_good_lvl1_pkts_cnt_get(&good_lvl1_pkts);
        bdmf_session_print(session, "good_lvl1_pkts = %u (0x%x)\n", good_lvl1_pkts, good_lvl1_pkts);
        break;
    }
    case cli_qm_good_lvl1_bytes_cnt:
    {
        uint32_t good_lvl1_bytes;
        err = ag_drv_qm_good_lvl1_bytes_cnt_get(&good_lvl1_bytes);
        bdmf_session_print(session, "good_lvl1_bytes = %u (0x%x)\n", good_lvl1_bytes, good_lvl1_bytes);
        break;
    }
    case cli_qm_good_lvl2_pkts_cnt:
    {
        uint32_t good_lvl2_pkts;
        err = ag_drv_qm_good_lvl2_pkts_cnt_get(&good_lvl2_pkts);
        bdmf_session_print(session, "good_lvl2_pkts = %u (0x%x)\n", good_lvl2_pkts, good_lvl2_pkts);
        break;
    }
    case cli_qm_good_lvl2_bytes_cnt:
    {
        uint32_t good_lvl2_bytes;
        err = ag_drv_qm_good_lvl2_bytes_cnt_get(&good_lvl2_bytes);
        bdmf_session_print(session, "good_lvl2_bytes = %u (0x%x)\n", good_lvl2_bytes, good_lvl2_bytes);
        break;
    }
    case cli_qm_copied_pkts_cnt:
    {
        uint32_t copied_pkts;
        err = ag_drv_qm_copied_pkts_cnt_get(&copied_pkts);
        bdmf_session_print(session, "copied_pkts = %u (0x%x)\n", copied_pkts, copied_pkts);
        break;
    }
    case cli_qm_copied_bytes_cnt:
    {
        uint32_t copied_bytes;
        err = ag_drv_qm_copied_bytes_cnt_get(&copied_bytes);
        bdmf_session_print(session, "copied_bytes = %u (0x%x)\n", copied_bytes, copied_bytes);
        break;
    }
    case cli_qm_agg_pkts_cnt:
    {
        uint32_t agg_pkts;
        err = ag_drv_qm_agg_pkts_cnt_get(&agg_pkts);
        bdmf_session_print(session, "agg_pkts = %u (0x%x)\n", agg_pkts, agg_pkts);
        break;
    }
    case cli_qm_agg_bytes_cnt:
    {
        uint32_t agg_bytes;
        err = ag_drv_qm_agg_bytes_cnt_get(&agg_bytes);
        bdmf_session_print(session, "agg_bytes = %u (0x%x)\n", agg_bytes, agg_bytes);
        break;
    }
    case cli_qm_agg_1_pkts_cnt:
    {
        uint32_t agg1_pkts;
        err = ag_drv_qm_agg_1_pkts_cnt_get(&agg1_pkts);
        bdmf_session_print(session, "agg1_pkts = %u (0x%x)\n", agg1_pkts, agg1_pkts);
        break;
    }
    case cli_qm_agg_2_pkts_cnt:
    {
        uint32_t agg2_pkts;
        err = ag_drv_qm_agg_2_pkts_cnt_get(&agg2_pkts);
        bdmf_session_print(session, "agg2_pkts = %u (0x%x)\n", agg2_pkts, agg2_pkts);
        break;
    }
    case cli_qm_agg_3_pkts_cnt:
    {
        uint32_t agg3_pkts;
        err = ag_drv_qm_agg_3_pkts_cnt_get(&agg3_pkts);
        bdmf_session_print(session, "agg3_pkts = %u (0x%x)\n", agg3_pkts, agg3_pkts);
        break;
    }
    case cli_qm_agg_4_pkts_cnt:
    {
        uint32_t agg4_pkts;
        err = ag_drv_qm_agg_4_pkts_cnt_get(&agg4_pkts);
        bdmf_session_print(session, "agg4_pkts = %u (0x%x)\n", agg4_pkts, agg4_pkts);
        break;
    }
    case cli_qm_wred_drop_cnt:
    {
        uint32_t wred_drop;
        err = ag_drv_qm_wred_drop_cnt_get(&wred_drop);
        bdmf_session_print(session, "wred_drop = %u (0x%x)\n", wred_drop, wred_drop);
        break;
    }
    case cli_qm_fpm_congestion_drop_cnt:
    {
        uint32_t fpm_cong;
        err = ag_drv_qm_fpm_congestion_drop_cnt_get(&fpm_cong);
        bdmf_session_print(session, "fpm_cong = %u (0x%x)\n", fpm_cong, fpm_cong);
        break;
    }
    case cli_qm_ddr_pd_congestion_drop_cnt:
    {
        uint32_t ddr_pd_cong_drop;
        err = ag_drv_qm_ddr_pd_congestion_drop_cnt_get(&ddr_pd_cong_drop);
        bdmf_session_print(session, "ddr_pd_cong_drop = %u (0x%x)\n", ddr_pd_cong_drop, ddr_pd_cong_drop);
        break;
    }
    case cli_qm_ddr_byte_congestion_drop_cnt:
    {
        uint32_t ddr_cong_byte_drop;
        err = ag_drv_qm_ddr_byte_congestion_drop_cnt_get(&ddr_cong_byte_drop);
        bdmf_session_print(session, "ddr_cong_byte_drop = %u (0x%x)\n", ddr_cong_byte_drop, ddr_cong_byte_drop);
        break;
    }
    case cli_qm_qm_pd_congestion_drop_cnt:
    {
        uint32_t pd_cong_drop;
        err = ag_drv_qm_qm_pd_congestion_drop_cnt_get(&pd_cong_drop);
        bdmf_session_print(session, "pd_cong_drop = %u (0x%x)\n", pd_cong_drop, pd_cong_drop);
        break;
    }
    case cli_qm_qm_abs_requeue_cnt:
    {
        uint32_t abs_requeue;
        err = ag_drv_qm_qm_abs_requeue_cnt_get(&abs_requeue);
        bdmf_session_print(session, "abs_requeue = %u (0x%x)\n", abs_requeue, abs_requeue);
        break;
    }
    case cli_qm_fpm_prefetch_fifo0_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        err = ag_drv_qm_fpm_prefetch_fifo0_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        bdmf_session_print(session, "full = %u (0x%x)\n", full, full);
        break;
    }
    case cli_qm_fpm_prefetch_fifo1_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        err = ag_drv_qm_fpm_prefetch_fifo1_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        bdmf_session_print(session, "full = %u (0x%x)\n", full, full);
        break;
    }
    case cli_qm_fpm_prefetch_fifo2_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        err = ag_drv_qm_fpm_prefetch_fifo2_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        bdmf_session_print(session, "full = %u (0x%x)\n", full, full);
        break;
    }
    case cli_qm_fpm_prefetch_fifo3_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        err = ag_drv_qm_fpm_prefetch_fifo3_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        bdmf_session_print(session, "full = %u (0x%x)\n", full, full);
        break;
    }
    case cli_qm_normal_rmt_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        err = ag_drv_qm_normal_rmt_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        bdmf_session_print(session, "full = %u (0x%x)\n", full, full);
        break;
    }
    case cli_qm_non_delayed_rmt_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        err = ag_drv_qm_non_delayed_rmt_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        bdmf_session_print(session, "full = %u (0x%x)\n", full, full);
        break;
    }
    case cli_qm_non_delayed_out_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        err = ag_drv_qm_non_delayed_out_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        bdmf_session_print(session, "full = %u (0x%x)\n", full, full);
        break;
    }
    case cli_qm_pre_cm_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        err = ag_drv_qm_pre_cm_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        bdmf_session_print(session, "full = %u (0x%x)\n", full, full);
        break;
    }
    case cli_qm_cm_rd_pd_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        err = ag_drv_qm_cm_rd_pd_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        bdmf_session_print(session, "full = %u (0x%x)\n", full, full);
        break;
    }
    case cli_qm_cm_wr_pd_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        err = ag_drv_qm_cm_wr_pd_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        bdmf_session_print(session, "full = %u (0x%x)\n", full, full);
        break;
    }
    case cli_qm_cm_common_input_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        err = ag_drv_qm_cm_common_input_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        bdmf_session_print(session, "full = %u (0x%x)\n", full, full);
        break;
    }
    case cli_qm_bb0_output_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        err = ag_drv_qm_bb0_output_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        bdmf_session_print(session, "full = %u (0x%x)\n", full, full);
        break;
    }
    case cli_qm_bb1_output_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        err = ag_drv_qm_bb1_output_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        bdmf_session_print(session, "full = %u (0x%x)\n", full, full);
        break;
    }
    case cli_qm_bb1_input_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        err = ag_drv_qm_bb1_input_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        bdmf_session_print(session, "full = %u (0x%x)\n", full, full);
        break;
    }
    case cli_qm_egress_data_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        err = ag_drv_qm_egress_data_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        bdmf_session_print(session, "full = %u (0x%x)\n", full, full);
        break;
    }
    case cli_qm_egress_rr_fifo_status:
    {
        uint16_t used_words;
        bdmf_boolean empty;
        bdmf_boolean full;
        err = ag_drv_qm_egress_rr_fifo_status_get(&used_words, &empty, &full);
        bdmf_session_print(session, "used_words = %u (0x%x)\n", used_words, used_words);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        bdmf_session_print(session, "full = %u (0x%x)\n", full, full);
        break;
    }
    case cli_qm_bb_route_ovr:
    {
        bdmf_boolean ovr_en;
        uint8_t dest_id;
        uint16_t route_addr;
        err = ag_drv_qm_bb_route_ovr_get(parm[1].value.unumber, &ovr_en, &dest_id, &route_addr);
        bdmf_session_print(session, "ovr_en = %u (0x%x)\n", ovr_en, ovr_en);
        bdmf_session_print(session, "dest_id = %u (0x%x)\n", dest_id, dest_id);
        bdmf_session_print(session, "route_addr = %u (0x%x)\n", route_addr, route_addr);
        break;
    }
    case cli_qm_ingress_stat:
    {
        uint32_t ingress_stat;
        err = ag_drv_qm_ingress_stat_get(&ingress_stat);
        bdmf_session_print(session, "ingress_stat = %u (0x%x)\n", ingress_stat, ingress_stat);
        break;
    }
    case cli_qm_egress_stat:
    {
        uint32_t egress_stat;
        err = ag_drv_qm_egress_stat_get(&egress_stat);
        bdmf_session_print(session, "egress_stat = %u (0x%x)\n", egress_stat, egress_stat);
        break;
    }
    case cli_qm_cm_stat:
    {
        uint32_t cm_stat;
        err = ag_drv_qm_cm_stat_get(&cm_stat);
        bdmf_session_print(session, "cm_stat = %u (0x%x)\n", cm_stat, cm_stat);
        break;
    }
    case cli_qm_fpm_prefetch_stat:
    {
        uint32_t fpm_prefetch_stat;
        err = ag_drv_qm_fpm_prefetch_stat_get(&fpm_prefetch_stat);
        bdmf_session_print(session, "fpm_prefetch_stat = %u (0x%x)\n", fpm_prefetch_stat, fpm_prefetch_stat);
        break;
    }
    case cli_qm_qm_connect_ack_counter:
    {
        uint8_t connect_ack_counter;
        err = ag_drv_qm_qm_connect_ack_counter_get(&connect_ack_counter);
        bdmf_session_print(session, "connect_ack_counter = %u (0x%x)\n", connect_ack_counter, connect_ack_counter);
        break;
    }
    case cli_qm_qm_ddr_wr_reply_counter:
    {
        uint8_t ddr_wr_reply_counter;
        err = ag_drv_qm_qm_ddr_wr_reply_counter_get(&ddr_wr_reply_counter);
        bdmf_session_print(session, "ddr_wr_reply_counter = %u (0x%x)\n", ddr_wr_reply_counter, ddr_wr_reply_counter);
        break;
    }
    case cli_qm_qm_ddr_pipe_byte_counter:
    {
        uint32_t ddr_pipe;
        err = ag_drv_qm_qm_ddr_pipe_byte_counter_get(&ddr_pipe);
        bdmf_session_print(session, "ddr_pipe = %u (0x%x)\n", ddr_pipe, ddr_pipe);
        break;
    }
    case cli_qm_qm_abs_requeue_valid_counter:
    {
        uint16_t requeue_valid;
        err = ag_drv_qm_qm_abs_requeue_valid_counter_get(&requeue_valid);
        bdmf_session_print(session, "requeue_valid = %u (0x%x)\n", requeue_valid, requeue_valid);
        break;
    }
    case cli_qm_qm_illegal_pd_capture:
    {
        uint32_t pd;
        err = ag_drv_qm_qm_illegal_pd_capture_get(parm[1].value.unumber, &pd);
        bdmf_session_print(session, "pd = %u (0x%x)\n", pd, pd);
        break;
    }
    case cli_qm_qm_ingress_processed_pd_capture:
    {
        uint32_t pd;
        err = ag_drv_qm_qm_ingress_processed_pd_capture_get(parm[1].value.unumber, &pd);
        bdmf_session_print(session, "pd = %u (0x%x)\n", pd, pd);
        break;
    }
    case cli_qm_qm_cm_processed_pd_capture:
    {
        uint32_t pd;
        err = ag_drv_qm_qm_cm_processed_pd_capture_get(parm[1].value.unumber, &pd);
        bdmf_session_print(session, "pd = %u (0x%x)\n", pd, pd);
        break;
    }
    case cli_qm_fpm_pool_drop_cnt:
    {
        uint32_t fpm_drop;
        err = ag_drv_qm_fpm_pool_drop_cnt_get(parm[1].value.unumber, &fpm_drop);
        bdmf_session_print(session, "fpm_drop = %u (0x%x)\n", fpm_drop, fpm_drop);
        break;
    }
    case cli_qm_fpm_grp_drop_cnt:
    {
        uint32_t fpm_grp_drop;
        err = ag_drv_qm_fpm_grp_drop_cnt_get(parm[1].value.unumber, &fpm_grp_drop);
        bdmf_session_print(session, "fpm_grp_drop = %u (0x%x)\n", fpm_grp_drop, fpm_grp_drop);
        break;
    }
    case cli_qm_fpm_buffer_res_drop_cnt:
    {
        uint32_t counter;
        err = ag_drv_qm_fpm_buffer_res_drop_cnt_get(&counter);
        bdmf_session_print(session, "counter = %u (0x%x)\n", counter, counter);
        break;
    }
    case cli_qm_psram_egress_cong_drp_cnt:
    {
        uint32_t counter;
        err = ag_drv_qm_psram_egress_cong_drp_cnt_get(&counter);
        bdmf_session_print(session, "counter = %u (0x%x)\n", counter, counter);
        break;
    }
    case cli_qm_cm_residue_data:
    {
        uint32_t data;
        err = ag_drv_qm_cm_residue_data_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_mactype:
    {
        uint8_t type;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_mactype_get(&type);
        bdmf_session_print(session, "type = %u (0x%x)\n", type, type);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1:
    {
        uint16_t tcontaddr;
        uint16_t skbaddr;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1_get(parm[1].value.unumber, &tcontaddr, &skbaddr);
        bdmf_session_print(session, "tcontaddr = %u (0x%x)\n", tcontaddr, tcontaddr);
        bdmf_session_print(session, "skbaddr = %u (0x%x)\n", skbaddr, skbaddr);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2:
    {
        uint16_t ptraddr;
        uint8_t task;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2_get(parm[1].value.unumber, &ptraddr, &task);
        bdmf_session_print(session, "ptraddr = %u (0x%x)\n", ptraddr, ptraddr);
        bdmf_session_print(session, "task = %u (0x%x)\n", task, task);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg:
    {
        bdmf_boolean freenocntxt;
        bdmf_boolean specialfree;
        uint8_t maxgn;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg_get(&freenocntxt, &specialfree, &maxgn);
        bdmf_session_print(session, "freenocntxt = %u (0x%x)\n", freenocntxt, freenocntxt);
        bdmf_session_print(session, "specialfree = %u (0x%x)\n", specialfree, specialfree);
        bdmf_session_print(session, "maxgn = %u (0x%x)\n", maxgn, maxgn);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel:
    {
        qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel_get(parm[1].value.unumber, &bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel);
        bdmf_session_print(session, "addr[0] = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel.addr[0], bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel.addr[0]);
        bdmf_session_print(session, "addr[1] = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel.addr[1], bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel.addr[1]);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh:
    {
        qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh_get(parm[1].value.unumber, &bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh);
        bdmf_session_print(session, "addr[0] = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh.addr[0], bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh.addr[0]);
        bdmf_session_print(session, "addr[1] = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh.addr[1], bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh.addr[1]);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl:
    {
        uint16_t psramsize;
        uint16_t ddrsize;
        uint16_t psrambase;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl_get(&psramsize, &ddrsize, &psrambase);
        bdmf_session_print(session, "psramsize = %u (0x%x)\n", psramsize, psramsize);
        bdmf_session_print(session, "ddrsize = %u (0x%x)\n", ddrsize, ddrsize);
        bdmf_session_print(session, "psrambase = %u (0x%x)\n", psrambase, psrambase);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_arb_cfg:
    {
        bdmf_boolean hightrxq;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_arb_cfg_get(&hightrxq);
        bdmf_session_print(session, "hightrxq = %u (0x%x)\n", hightrxq, hightrxq);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_bbroute:
    {
        uint16_t route;
        uint8_t dest;
        bdmf_boolean en;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_bbroute_get(&route, &dest, &en);
        bdmf_session_print(session, "route = %u (0x%x)\n", route, route);
        bdmf_session_print(session, "dest = %u (0x%x)\n", dest, dest);
        bdmf_session_print(session, "en = %u (0x%x)\n", en, en);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_q2rnr:
    {
        bdmf_boolean q0;
        bdmf_boolean q1;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_q2rnr_get(parm[1].value.unumber, &q0, &q1);
        bdmf_session_print(session, "q0 = %u (0x%x)\n", q0, q0);
        bdmf_session_print(session, "q1 = %u (0x%x)\n", q1, q1);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_perqtask:
    {
        qm_bbh_tx_qm_bbhtx_common_configurations_perqtask bbh_tx_qm_bbhtx_common_configurations_perqtask;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_perqtask_get(&bbh_tx_qm_bbhtx_common_configurations_perqtask);
        bdmf_session_print(session, "task0 = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_perqtask.task0, bbh_tx_qm_bbhtx_common_configurations_perqtask.task0);
        bdmf_session_print(session, "task1 = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_perqtask.task1, bbh_tx_qm_bbhtx_common_configurations_perqtask.task1);
        bdmf_session_print(session, "task2 = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_perqtask.task2, bbh_tx_qm_bbhtx_common_configurations_perqtask.task2);
        bdmf_session_print(session, "task3 = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_perqtask.task3, bbh_tx_qm_bbhtx_common_configurations_perqtask.task3);
        bdmf_session_print(session, "task4 = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_perqtask.task4, bbh_tx_qm_bbhtx_common_configurations_perqtask.task4);
        bdmf_session_print(session, "task5 = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_perqtask.task5, bbh_tx_qm_bbhtx_common_configurations_perqtask.task5);
        bdmf_session_print(session, "task6 = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_perqtask.task6, bbh_tx_qm_bbhtx_common_configurations_perqtask.task6);
        bdmf_session_print(session, "task7 = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_perqtask.task7, bbh_tx_qm_bbhtx_common_configurations_perqtask.task7);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd:
    {
        qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd bbh_tx_qm_bbhtx_common_configurations_txrstcmd;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd_get(&bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
        bdmf_session_print(session, "cntxtrst = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.cntxtrst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.cntxtrst);
        bdmf_session_print(session, "pdfiforst = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.pdfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.pdfiforst);
        bdmf_session_print(session, "dmaptrrst = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.dmaptrrst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.dmaptrrst);
        bdmf_session_print(session, "sdmaptrrst = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.sdmaptrrst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.sdmaptrrst);
        bdmf_session_print(session, "bpmfiforst = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.bpmfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.bpmfiforst);
        bdmf_session_print(session, "sbpmfiforst = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.sbpmfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.sbpmfiforst);
        bdmf_session_print(session, "okfiforst = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.okfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.okfiforst);
        bdmf_session_print(session, "ddrfiforst = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.ddrfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.ddrfiforst);
        bdmf_session_print(session, "sramfiforst = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.sramfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.sramfiforst);
        bdmf_session_print(session, "skbptrrst = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.skbptrrst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.skbptrrst);
        bdmf_session_print(session, "stsfiforst = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.stsfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.stsfiforst);
        bdmf_session_print(session, "reqfiforst = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.reqfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.reqfiforst);
        bdmf_session_print(session, "msgfiforst = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.msgfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.msgfiforst);
        bdmf_session_print(session, "gnxtfiforst = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.gnxtfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.gnxtfiforst);
        bdmf_session_print(session, "fbnfiforst = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.fbnfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.fbnfiforst);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_dbgsel:
    {
        uint8_t dbgsel;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dbgsel_get(&dbgsel);
        bdmf_session_print(session, "dbgsel = %u (0x%x)\n", dbgsel, dbgsel);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl:
    {
        qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl_get(&bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.bypass_clk_gate, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.timer_val, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.keep_alive_en, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intrvl = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.keep_alive_intrvl, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.keep_alive_intrvl);
        bdmf_session_print(session, "keep_alive_cyc = %u (0x%x)\n", bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.keep_alive_cyc, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_common_configurations_gpr:
    {
        uint32_t gpr;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_gpr_get(&gpr);
        bdmf_session_print(session, "gpr = %u (0x%x)\n", gpr, gpr);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdbase:
    {
        uint16_t fifobase0;
        uint16_t fifobase1;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdbase_get(&fifobase0, &fifobase1);
        bdmf_session_print(session, "fifobase0 = %u (0x%x)\n", fifobase0, fifobase0);
        bdmf_session_print(session, "fifobase1 = %u (0x%x)\n", fifobase1, fifobase1);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdsize:
    {
        uint16_t fifosize0;
        uint16_t fifosize1;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdsize_get(&fifosize0, &fifosize1);
        bdmf_session_print(session, "fifosize0 = %u (0x%x)\n", fifosize0, fifosize0);
        bdmf_session_print(session, "fifosize1 = %u (0x%x)\n", fifosize1, fifosize1);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph:
    {
        uint8_t wkupthresh0;
        uint8_t wkupthresh1;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph_get(&wkupthresh0, &wkupthresh1);
        bdmf_session_print(session, "wkupthresh0 = %u (0x%x)\n", wkupthresh0, wkupthresh0);
        bdmf_session_print(session, "wkupthresh1 = %u (0x%x)\n", wkupthresh1, wkupthresh1);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th:
    {
        uint16_t pdlimit0;
        uint16_t pdlimit1;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_get(&pdlimit0, &pdlimit1);
        bdmf_session_print(session, "pdlimit0 = %u (0x%x)\n", pdlimit0, pdlimit0);
        bdmf_session_print(session, "pdlimit1 = %u (0x%x)\n", pdlimit1, pdlimit1);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en:
    {
        bdmf_boolean pdlimiten;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en_get(&pdlimiten);
        bdmf_session_print(session, "pdlimiten = %u (0x%x)\n", pdlimiten, pdlimiten);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdempty:
    {
        uint8_t empty;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdempty_get(&empty);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_txthresh:
    {
        uint16_t ddrthresh;
        uint16_t sramthresh;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_txthresh_get(&ddrthresh, &sramthresh);
        bdmf_session_print(session, "ddrthresh = %u (0x%x)\n", ddrthresh, ddrthresh);
        bdmf_session_print(session, "sramthresh = %u (0x%x)\n", sramthresh, sramthresh);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_eee:
    {
        bdmf_boolean en;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_eee_get(&en);
        bdmf_session_print(session, "en = %u (0x%x)\n", en, en);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_lan_configurations_ts:
    {
        bdmf_boolean en;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_ts_get(&en);
        bdmf_session_print(session, "en = %u (0x%x)\n", en, en);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_debug_counters_srambyte:
    {
        uint32_t srambyte;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_srambyte_get(&srambyte);
        bdmf_session_print(session, "srambyte = %u (0x%x)\n", srambyte, srambyte);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_debug_counters_ddrbyte:
    {
        uint32_t ddrbyte;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_ddrbyte_get(&ddrbyte);
        bdmf_session_print(session, "ddrbyte = %u (0x%x)\n", ddrbyte, ddrbyte);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_debug_counters_swrden:
    {
        qm_bbh_tx_qm_bbhtx_debug_counters_swrden bbh_tx_qm_bbhtx_debug_counters_swrden;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrden_get(&bbh_tx_qm_bbhtx_debug_counters_swrden);
        bdmf_session_print(session, "pdsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.pdsel, bbh_tx_qm_bbhtx_debug_counters_swrden.pdsel);
        bdmf_session_print(session, "pdvsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.pdvsel, bbh_tx_qm_bbhtx_debug_counters_swrden.pdvsel);
        bdmf_session_print(session, "pdemptysel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.pdemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.pdemptysel);
        bdmf_session_print(session, "pdfullsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.pdfullsel, bbh_tx_qm_bbhtx_debug_counters_swrden.pdfullsel);
        bdmf_session_print(session, "pdbemptysel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.pdbemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.pdbemptysel);
        bdmf_session_print(session, "pdffwkpsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.pdffwkpsel, bbh_tx_qm_bbhtx_debug_counters_swrden.pdffwkpsel);
        bdmf_session_print(session, "fbnsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.fbnsel, bbh_tx_qm_bbhtx_debug_counters_swrden.fbnsel);
        bdmf_session_print(session, "fbnvsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.fbnvsel, bbh_tx_qm_bbhtx_debug_counters_swrden.fbnvsel);
        bdmf_session_print(session, "fbnemptysel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.fbnemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.fbnemptysel);
        bdmf_session_print(session, "fbnfullsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.fbnfullsel, bbh_tx_qm_bbhtx_debug_counters_swrden.fbnfullsel);
        bdmf_session_print(session, "getnextsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.getnextsel, bbh_tx_qm_bbhtx_debug_counters_swrden.getnextsel);
        bdmf_session_print(session, "getnextvsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.getnextvsel, bbh_tx_qm_bbhtx_debug_counters_swrden.getnextvsel);
        bdmf_session_print(session, "getnextemptysel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.getnextemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.getnextemptysel);
        bdmf_session_print(session, "getnextfullsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.getnextfullsel, bbh_tx_qm_bbhtx_debug_counters_swrden.getnextfullsel);
        bdmf_session_print(session, "gpncntxtsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.gpncntxtsel, bbh_tx_qm_bbhtx_debug_counters_swrden.gpncntxtsel);
        bdmf_session_print(session, "bpmsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.bpmsel, bbh_tx_qm_bbhtx_debug_counters_swrden.bpmsel);
        bdmf_session_print(session, "bpmfsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.bpmfsel, bbh_tx_qm_bbhtx_debug_counters_swrden.bpmfsel);
        bdmf_session_print(session, "sbpmsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.sbpmsel, bbh_tx_qm_bbhtx_debug_counters_swrden.sbpmsel);
        bdmf_session_print(session, "sbpmfsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.sbpmfsel, bbh_tx_qm_bbhtx_debug_counters_swrden.sbpmfsel);
        bdmf_session_print(session, "stssel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.stssel, bbh_tx_qm_bbhtx_debug_counters_swrden.stssel);
        bdmf_session_print(session, "stsvsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.stsvsel, bbh_tx_qm_bbhtx_debug_counters_swrden.stsvsel);
        bdmf_session_print(session, "stsemptysel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.stsemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.stsemptysel);
        bdmf_session_print(session, "stsfullsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.stsfullsel, bbh_tx_qm_bbhtx_debug_counters_swrden.stsfullsel);
        bdmf_session_print(session, "stsbemptysel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.stsbemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.stsbemptysel);
        bdmf_session_print(session, "stsffwkpsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.stsffwkpsel, bbh_tx_qm_bbhtx_debug_counters_swrden.stsffwkpsel);
        bdmf_session_print(session, "msgsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.msgsel, bbh_tx_qm_bbhtx_debug_counters_swrden.msgsel);
        bdmf_session_print(session, "msgvsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.msgvsel, bbh_tx_qm_bbhtx_debug_counters_swrden.msgvsel);
        bdmf_session_print(session, "epnreqsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.epnreqsel, bbh_tx_qm_bbhtx_debug_counters_swrden.epnreqsel);
        bdmf_session_print(session, "datasel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.datasel, bbh_tx_qm_bbhtx_debug_counters_swrden.datasel);
        bdmf_session_print(session, "reordersel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.reordersel, bbh_tx_qm_bbhtx_debug_counters_swrden.reordersel);
        bdmf_session_print(session, "tsinfosel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.tsinfosel, bbh_tx_qm_bbhtx_debug_counters_swrden.tsinfosel);
        bdmf_session_print(session, "mactxsel = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.mactxsel, bbh_tx_qm_bbhtx_debug_counters_swrden.mactxsel);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_debug_counters_swrdaddr:
    {
        uint16_t rdaddr;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrdaddr_get(&rdaddr);
        bdmf_session_print(session, "rdaddr = %u (0x%x)\n", rdaddr, rdaddr);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_debug_counters_swrddata:
    {
        uint32_t data;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrddata_get(&data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_debug_counters_unifiedpkt:
    {
        uint32_t ddrbyte;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_unifiedpkt_get(parm[1].value.unumber, &ddrbyte);
        bdmf_session_print(session, "ddrbyte = %u (0x%x)\n", ddrbyte, ddrbyte);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_debug_counters_unifiedbyte:
    {
        uint32_t ddrbyte;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_unifiedbyte_get(parm[1].value.unumber, &ddrbyte);
        bdmf_session_print(session, "ddrbyte = %u (0x%x)\n", ddrbyte, ddrbyte);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg:
    {
        qm_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg bbh_tx_qm_bbhtx_debug_counters_dbgoutreg;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg_get(parm[1].value.unumber, &bbh_tx_qm_bbhtx_debug_counters_dbgoutreg);
        bdmf_session_print(session, "debug_out_reg[0] = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[0], bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[0]);
        bdmf_session_print(session, "debug_out_reg[1] = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[1], bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[1]);
        bdmf_session_print(session, "debug_out_reg[2] = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[2], bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[2]);
        bdmf_session_print(session, "debug_out_reg[3] = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[3], bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[3]);
        bdmf_session_print(session, "debug_out_reg[4] = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[4], bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[4]);
        bdmf_session_print(session, "debug_out_reg[5] = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[5], bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[5]);
        bdmf_session_print(session, "debug_out_reg[6] = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[6], bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[6]);
        bdmf_session_print(session, "debug_out_reg[7] = %u (0x%x)\n", bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[7], bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[7]);
        break;
    }
    case cli_qm_bbh_tx_qm_bbhtx_debug_counters_in_segmentation:
    {
        uint32_t in_segmentation;
        err = ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_in_segmentation_get(parm[1].value.unumber, &in_segmentation);
        bdmf_session_print(session, "in_segmentation = %u (0x%x)\n", in_segmentation, in_segmentation);
        break;
    }
    case cli_qm_dma_qm_dma_config_bbrouteovrd:
    {
        uint8_t dest;
        uint16_t route;
        bdmf_boolean ovrd;
        err = ag_drv_qm_dma_qm_dma_config_bbrouteovrd_get(&dest, &route, &ovrd);
        bdmf_session_print(session, "dest = %u (0x%x)\n", dest, dest);
        bdmf_session_print(session, "route = %u (0x%x)\n", route, route);
        bdmf_session_print(session, "ovrd = %u (0x%x)\n", ovrd, ovrd);
        break;
    }
    case cli_qm_dma_qm_dma_config_num_of_writes:
    {
        uint8_t numofbuff;
        err = ag_drv_qm_dma_qm_dma_config_num_of_writes_get(parm[1].value.unumber, &numofbuff);
        bdmf_session_print(session, "numofbuff = %u (0x%x)\n", numofbuff, numofbuff);
        break;
    }
    case cli_qm_dma_qm_dma_config_num_of_reads:
    {
        uint8_t rr_num;
        err = ag_drv_qm_dma_qm_dma_config_num_of_reads_get(parm[1].value.unumber, &rr_num);
        bdmf_session_print(session, "rr_num = %u (0x%x)\n", rr_num, rr_num);
        break;
    }
    case cli_qm_dma_qm_dma_config_u_thresh:
    {
        uint8_t into_u;
        uint8_t out_of_u;
        err = ag_drv_qm_dma_qm_dma_config_u_thresh_get(parm[1].value.unumber, &into_u, &out_of_u);
        bdmf_session_print(session, "into_u = %u (0x%x)\n", into_u, into_u);
        bdmf_session_print(session, "out_of_u = %u (0x%x)\n", out_of_u, out_of_u);
        break;
    }
    case cli_qm_dma_qm_dma_config_pri:
    {
        uint8_t rxpri;
        uint8_t txpri;
        err = ag_drv_qm_dma_qm_dma_config_pri_get(parm[1].value.unumber, &rxpri, &txpri);
        bdmf_session_print(session, "rxpri = %u (0x%x)\n", rxpri, rxpri);
        bdmf_session_print(session, "txpri = %u (0x%x)\n", txpri, txpri);
        break;
    }
    case cli_qm_dma_qm_dma_config_periph_source:
    {
        uint8_t rxsource;
        uint8_t txsource;
        err = ag_drv_qm_dma_qm_dma_config_periph_source_get(parm[1].value.unumber, &rxsource, &txsource);
        bdmf_session_print(session, "rxsource = %u (0x%x)\n", rxsource, rxsource);
        bdmf_session_print(session, "txsource = %u (0x%x)\n", txsource, txsource);
        break;
    }
    case cli_qm_dma_qm_dma_config_weight:
    {
        uint8_t rxweight;
        uint8_t txweight;
        err = ag_drv_qm_dma_qm_dma_config_weight_get(parm[1].value.unumber, &rxweight, &txweight);
        bdmf_session_print(session, "rxweight = %u (0x%x)\n", rxweight, rxweight);
        bdmf_session_print(session, "txweight = %u (0x%x)\n", txweight, txweight);
        break;
    }
    case cli_qm_dma_qm_dma_config_ptrrst:
    {
        uint16_t rstvec;
        err = ag_drv_qm_dma_qm_dma_config_ptrrst_get(&rstvec);
        bdmf_session_print(session, "rstvec = %u (0x%x)\n", rstvec, rstvec);
        break;
    }
    case cli_qm_dma_qm_dma_config_max_otf:
    {
        uint8_t max;
        err = ag_drv_qm_dma_qm_dma_config_max_otf_get(&max);
        bdmf_session_print(session, "max = %u (0x%x)\n", max, max);
        break;
    }
    case cli_qm_dma_qm_dma_config_clk_gate_cntrl:
    {
        qm_dma_qm_dma_config_clk_gate_cntrl dma_qm_dma_config_clk_gate_cntrl;
        err = ag_drv_qm_dma_qm_dma_config_clk_gate_cntrl_get(&dma_qm_dma_config_clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u (0x%x)\n", dma_qm_dma_config_clk_gate_cntrl.bypass_clk_gate, dma_qm_dma_config_clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u (0x%x)\n", dma_qm_dma_config_clk_gate_cntrl.timer_val, dma_qm_dma_config_clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u (0x%x)\n", dma_qm_dma_config_clk_gate_cntrl.keep_alive_en, dma_qm_dma_config_clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intrvl = %u (0x%x)\n", dma_qm_dma_config_clk_gate_cntrl.keep_alive_intrvl, dma_qm_dma_config_clk_gate_cntrl.keep_alive_intrvl);
        bdmf_session_print(session, "keep_alive_cyc = %u (0x%x)\n", dma_qm_dma_config_clk_gate_cntrl.keep_alive_cyc, dma_qm_dma_config_clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    case cli_qm_dma_qm_dma_config_dbg_sel:
    {
        uint8_t dbgsel;
        err = ag_drv_qm_dma_qm_dma_config_dbg_sel_get(&dbgsel);
        bdmf_session_print(session, "dbgsel = %u (0x%x)\n", dbgsel, dbgsel);
        break;
    }
    case cli_qm_dma_qm_dma_debug_req_cnt_rx:
    {
        uint8_t req_cnt;
        err = ag_drv_qm_dma_qm_dma_debug_req_cnt_rx_get(parm[1].value.unumber, &req_cnt);
        bdmf_session_print(session, "req_cnt = %u (0x%x)\n", req_cnt, req_cnt);
        break;
    }
    case cli_qm_dma_qm_dma_debug_req_cnt_tx:
    {
        uint8_t req_cnt;
        err = ag_drv_qm_dma_qm_dma_debug_req_cnt_tx_get(parm[1].value.unumber, &req_cnt);
        bdmf_session_print(session, "req_cnt = %u (0x%x)\n", req_cnt, req_cnt);
        break;
    }
    case cli_qm_dma_qm_dma_debug_req_cnt_rx_acc:
    {
        uint32_t req_cnt;
        err = ag_drv_qm_dma_qm_dma_debug_req_cnt_rx_acc_get(parm[1].value.unumber, &req_cnt);
        bdmf_session_print(session, "req_cnt = %u (0x%x)\n", req_cnt, req_cnt);
        break;
    }
    case cli_qm_dma_qm_dma_debug_req_cnt_tx_acc:
    {
        uint32_t req_cnt;
        err = ag_drv_qm_dma_qm_dma_debug_req_cnt_tx_acc_get(parm[1].value.unumber, &req_cnt);
        bdmf_session_print(session, "req_cnt = %u (0x%x)\n", req_cnt, req_cnt);
        break;
    }
    case cli_qm_dma_qm_dma_debug_rddata:
    {
        uint32_t data;
        err = ag_drv_qm_dma_qm_dma_debug_rddata_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_qm_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        qm_ddr_cong_ctrl ddr_cong_ctrl = {.ddr_byte_congestion_drop_enable=gtmv(m, 1), .ddr_bytes_lower_thr=gtmv(m, 30), .ddr_bytes_mid_thr=gtmv(m, 30), .ddr_bytes_higher_thr=gtmv(m, 30), .ddr_pd_congestion_drop_enable=gtmv(m, 1), .ddr_pipe_lower_thr=gtmv(m, 8), .ddr_pipe_higher_thr=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_ddr_cong_ctrl_set( %u %u %u %u %u %u %u)\n", ddr_cong_ctrl.ddr_byte_congestion_drop_enable, ddr_cong_ctrl.ddr_bytes_lower_thr, ddr_cong_ctrl.ddr_bytes_mid_thr, ddr_cong_ctrl.ddr_bytes_higher_thr, ddr_cong_ctrl.ddr_pd_congestion_drop_enable, ddr_cong_ctrl.ddr_pipe_lower_thr, ddr_cong_ctrl.ddr_pipe_higher_thr);
        if(!err) ag_drv_qm_ddr_cong_ctrl_set(&ddr_cong_ctrl);
        if(!err) ag_drv_qm_ddr_cong_ctrl_get( &ddr_cong_ctrl);
        if(!err) bdmf_session_print(session, "ag_drv_qm_ddr_cong_ctrl_get( %u %u %u %u %u %u %u)\n", ddr_cong_ctrl.ddr_byte_congestion_drop_enable, ddr_cong_ctrl.ddr_bytes_lower_thr, ddr_cong_ctrl.ddr_bytes_mid_thr, ddr_cong_ctrl.ddr_bytes_higher_thr, ddr_cong_ctrl.ddr_pd_congestion_drop_enable, ddr_cong_ctrl.ddr_pipe_lower_thr, ddr_cong_ctrl.ddr_pipe_higher_thr);
        if(err || ddr_cong_ctrl.ddr_byte_congestion_drop_enable!=gtmv(m, 1) || ddr_cong_ctrl.ddr_bytes_lower_thr!=gtmv(m, 30) || ddr_cong_ctrl.ddr_bytes_mid_thr!=gtmv(m, 30) || ddr_cong_ctrl.ddr_bytes_higher_thr!=gtmv(m, 30) || ddr_cong_ctrl.ddr_pd_congestion_drop_enable!=gtmv(m, 1) || ddr_cong_ctrl.ddr_pipe_lower_thr!=gtmv(m, 8) || ddr_cong_ctrl.ddr_pipe_higher_thr!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t q_idx=gtmv(m, 5);
        bdmf_boolean data=gtmv(m, 1);
        if(!err) ag_drv_qm_is_queue_not_empty_get( q_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_is_queue_not_empty_get( %u %u)\n", q_idx, data);
    }
    {
        uint16_t q_idx=gtmv(m, 5);
        bdmf_boolean data=gtmv(m, 1);
        if(!err) ag_drv_qm_is_queue_pop_ready_get( q_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_is_queue_pop_ready_get( %u %u)\n", q_idx, data);
    }
    {
        uint16_t q_idx=gtmv(m, 5);
        bdmf_boolean data=gtmv(m, 1);
        if(!err) ag_drv_qm_is_queue_full_get( q_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_is_queue_full_get( %u %u)\n", q_idx, data);
    }
    {
        uint8_t ug_grp_idx=gtmv(m, 2);
        qm_fpm_ug_thr fpm_ug_thr = {.lower_thr=gtmv(m, 16), .mid_thr=gtmv(m, 16), .higher_thr=gtmv(m, 16)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_ug_thr_set( %u %u %u %u)\n", ug_grp_idx, fpm_ug_thr.lower_thr, fpm_ug_thr.mid_thr, fpm_ug_thr.higher_thr);
        if(!err) ag_drv_qm_fpm_ug_thr_set(ug_grp_idx, &fpm_ug_thr);
        if(!err) ag_drv_qm_fpm_ug_thr_get( ug_grp_idx, &fpm_ug_thr);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_ug_thr_get( %u %u %u %u)\n", ug_grp_idx, fpm_ug_thr.lower_thr, fpm_ug_thr.mid_thr, fpm_ug_thr.higher_thr);
        if(err || fpm_ug_thr.lower_thr!=gtmv(m, 16) || fpm_ug_thr.mid_thr!=gtmv(m, 16) || fpm_ug_thr.higher_thr!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t rnr_idx=gtmv(m, 4);
        qm_rnr_group_cfg rnr_group_cfg = {.start_queue=gtmv(m, 9), .end_queue=gtmv(m, 9), .pd_fifo_base=gtmv(m, 11), .pd_fifo_size=gtmv(m, 2), .upd_fifo_base=gtmv(m, 11), .upd_fifo_size=gtmv(m, 3), .rnr_bb_id=gtmv(m, 6), .rnr_task=gtmv(m, 4), .rnr_enable=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_rnr_group_cfg_set( %u %u %u %u %u %u %u %u %u %u)\n", rnr_idx, rnr_group_cfg.start_queue, rnr_group_cfg.end_queue, rnr_group_cfg.pd_fifo_base, rnr_group_cfg.pd_fifo_size, rnr_group_cfg.upd_fifo_base, rnr_group_cfg.upd_fifo_size, rnr_group_cfg.rnr_bb_id, rnr_group_cfg.rnr_task, rnr_group_cfg.rnr_enable);
        if(!err) ag_drv_qm_rnr_group_cfg_set(rnr_idx, &rnr_group_cfg);
        if(!err) ag_drv_qm_rnr_group_cfg_get( rnr_idx, &rnr_group_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_qm_rnr_group_cfg_get( %u %u %u %u %u %u %u %u %u %u)\n", rnr_idx, rnr_group_cfg.start_queue, rnr_group_cfg.end_queue, rnr_group_cfg.pd_fifo_base, rnr_group_cfg.pd_fifo_size, rnr_group_cfg.upd_fifo_base, rnr_group_cfg.upd_fifo_size, rnr_group_cfg.rnr_bb_id, rnr_group_cfg.rnr_task, rnr_group_cfg.rnr_enable);
        if(err || rnr_group_cfg.start_queue!=gtmv(m, 9) || rnr_group_cfg.end_queue!=gtmv(m, 9) || rnr_group_cfg.pd_fifo_base!=gtmv(m, 11) || rnr_group_cfg.pd_fifo_size!=gtmv(m, 2) || rnr_group_cfg.upd_fifo_base!=gtmv(m, 11) || rnr_group_cfg.upd_fifo_size!=gtmv(m, 3) || rnr_group_cfg.rnr_bb_id!=gtmv(m, 6) || rnr_group_cfg.rnr_task!=gtmv(m, 4) || rnr_group_cfg.rnr_enable!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t indirect_grp_idx=gtmv(m, 2);
        uint32_t data0=gtmv(m, 32);
        uint32_t data1=gtmv(m, 32);
        uint32_t data2=gtmv(m, 32);
        uint32_t data3=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_qm_cpu_pd_indirect_wr_data_set( %u %u %u %u %u)\n", indirect_grp_idx, data0, data1, data2, data3);
        if(!err) ag_drv_qm_cpu_pd_indirect_wr_data_set(indirect_grp_idx, data0, data1, data2, data3);
        if(!err) ag_drv_qm_cpu_pd_indirect_wr_data_get( indirect_grp_idx, &data0, &data1, &data2, &data3);
        if(!err) bdmf_session_print(session, "ag_drv_qm_cpu_pd_indirect_wr_data_get( %u %u %u %u %u)\n", indirect_grp_idx, data0, data1, data2, data3);
        if(err || data0!=gtmv(m, 32) || data1!=gtmv(m, 32) || data2!=gtmv(m, 32) || data3!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t indirect_grp_idx=gtmv(m, 2);
        uint32_t data0=gtmv(m, 32);
        uint32_t data1=gtmv(m, 32);
        uint32_t data2=gtmv(m, 32);
        uint32_t data3=gtmv(m, 32);
        if(!err) ag_drv_qm_cpu_pd_indirect_rd_data_get( indirect_grp_idx, &data0, &data1, &data2, &data3);
        if(!err) bdmf_session_print(session, "ag_drv_qm_cpu_pd_indirect_rd_data_get( %u %u %u %u %u)\n", indirect_grp_idx, data0, data1, data2, data3);
    }
    {
        uint16_t idx=gtmv(m, 0);
        uint32_t context_valid=gtmv(m, 32);
        if(!err) ag_drv_qm_aggr_context_get( idx, &context_valid);
        if(!err) bdmf_session_print(session, "ag_drv_qm_aggr_context_get( %u %u)\n", idx, context_valid);
    }
    {
        uint8_t profile_idx=gtmv(m, 4);
        qm_wred_profile_cfg wred_profile_cfg = {.min_thr0=gtmv(m, 24), .flw_ctrl_en0=gtmv(m, 1), .min_thr1=gtmv(m, 24), .flw_ctrl_en1=gtmv(m, 1), .max_thr0=gtmv(m, 24), .max_thr1=gtmv(m, 24), .slope_mantissa0=gtmv(m, 8), .slope_exp0=gtmv(m, 5), .slope_mantissa1=gtmv(m, 8), .slope_exp1=gtmv(m, 5)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_wred_profile_cfg_set( %u %u %u %u %u %u %u %u %u %u %u)\n", profile_idx, wred_profile_cfg.min_thr0, wred_profile_cfg.flw_ctrl_en0, wred_profile_cfg.min_thr1, wred_profile_cfg.flw_ctrl_en1, wred_profile_cfg.max_thr0, wred_profile_cfg.max_thr1, wred_profile_cfg.slope_mantissa0, wred_profile_cfg.slope_exp0, wred_profile_cfg.slope_mantissa1, wred_profile_cfg.slope_exp1);
        if(!err) ag_drv_qm_wred_profile_cfg_set(profile_idx, &wred_profile_cfg);
        if(!err) ag_drv_qm_wred_profile_cfg_get( profile_idx, &wred_profile_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_qm_wred_profile_cfg_get( %u %u %u %u %u %u %u %u %u %u %u)\n", profile_idx, wred_profile_cfg.min_thr0, wred_profile_cfg.flw_ctrl_en0, wred_profile_cfg.min_thr1, wred_profile_cfg.flw_ctrl_en1, wred_profile_cfg.max_thr0, wred_profile_cfg.max_thr1, wred_profile_cfg.slope_mantissa0, wred_profile_cfg.slope_exp0, wred_profile_cfg.slope_mantissa1, wred_profile_cfg.slope_exp1);
        if(err || wred_profile_cfg.min_thr0!=gtmv(m, 24) || wred_profile_cfg.flw_ctrl_en0!=gtmv(m, 1) || wred_profile_cfg.min_thr1!=gtmv(m, 24) || wred_profile_cfg.flw_ctrl_en1!=gtmv(m, 1) || wred_profile_cfg.max_thr0!=gtmv(m, 24) || wred_profile_cfg.max_thr1!=gtmv(m, 24) || wred_profile_cfg.slope_mantissa0!=gtmv(m, 8) || wred_profile_cfg.slope_exp0!=gtmv(m, 5) || wred_profile_cfg.slope_mantissa1!=gtmv(m, 8) || wred_profile_cfg.slope_exp1!=gtmv(m, 5))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_ubus_slave ubus_slave = {.vpb_base=gtmv(m, 32), .vpb_mask=gtmv(m, 32), .apb_base=gtmv(m, 32), .apb_mask=gtmv(m, 32), .dqm_base=gtmv(m, 32), .dqm_mask=gtmv(m, 32)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_ubus_slave_set( %u %u %u %u %u %u)\n", ubus_slave.vpb_base, ubus_slave.vpb_mask, ubus_slave.apb_base, ubus_slave.apb_mask, ubus_slave.dqm_base, ubus_slave.dqm_mask);
        if(!err) ag_drv_qm_ubus_slave_set(&ubus_slave);
        if(!err) ag_drv_qm_ubus_slave_get( &ubus_slave);
        if(!err) bdmf_session_print(session, "ag_drv_qm_ubus_slave_get( %u %u %u %u %u %u)\n", ubus_slave.vpb_base, ubus_slave.vpb_mask, ubus_slave.apb_base, ubus_slave.apb_mask, ubus_slave.dqm_base, ubus_slave.dqm_mask);
        if(err || ubus_slave.vpb_base!=gtmv(m, 32) || ubus_slave.vpb_mask!=gtmv(m, 32) || ubus_slave.apb_base!=gtmv(m, 32) || ubus_slave.apb_mask!=gtmv(m, 32) || ubus_slave.dqm_base!=gtmv(m, 32) || ubus_slave.dqm_mask!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_cfg_src_id cfg_src_id = {.fpmsrc=gtmv(m, 6), .sbpmsrc=gtmv(m, 6), .stsrnrsrc=gtmv(m, 6)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_cfg_src_id_set( %u %u %u)\n", cfg_src_id.fpmsrc, cfg_src_id.sbpmsrc, cfg_src_id.stsrnrsrc);
        if(!err) ag_drv_qm_cfg_src_id_set(&cfg_src_id);
        if(!err) ag_drv_qm_cfg_src_id_get( &cfg_src_id);
        if(!err) bdmf_session_print(session, "ag_drv_qm_cfg_src_id_get( %u %u %u)\n", cfg_src_id.fpmsrc, cfg_src_id.sbpmsrc, cfg_src_id.stsrnrsrc);
        if(err || cfg_src_id.fpmsrc!=gtmv(m, 6) || cfg_src_id.sbpmsrc!=gtmv(m, 6) || cfg_src_id.stsrnrsrc!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t pdrnr0src=gtmv(m, 6);
        uint8_t pdrnr1src=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_qm_rnr_src_id_set( %u %u)\n", pdrnr0src, pdrnr1src);
        if(!err) ag_drv_qm_rnr_src_id_set(pdrnr0src, pdrnr1src);
        if(!err) ag_drv_qm_rnr_src_id_get( &pdrnr0src, &pdrnr1src);
        if(!err) bdmf_session_print(session, "ag_drv_qm_rnr_src_id_get( %u %u)\n", pdrnr0src, pdrnr1src);
        if(err || pdrnr0src!=gtmv(m, 6) || pdrnr1src!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_bbh_dma_cfg bbh_dma_cfg = {.dmasrc=gtmv(m, 6), .descbase=gtmv(m, 6), .descsize=gtmv(m, 6)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_dma_cfg_set( %u %u %u)\n", bbh_dma_cfg.dmasrc, bbh_dma_cfg.descbase, bbh_dma_cfg.descsize);
        if(!err) ag_drv_qm_bbh_dma_cfg_set(&bbh_dma_cfg);
        if(!err) ag_drv_qm_bbh_dma_cfg_get( &bbh_dma_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_dma_cfg_get( %u %u %u)\n", bbh_dma_cfg.dmasrc, bbh_dma_cfg.descbase, bbh_dma_cfg.descsize);
        if(err || bbh_dma_cfg.dmasrc!=gtmv(m, 6) || bbh_dma_cfg.descbase!=gtmv(m, 6) || bbh_dma_cfg.descsize!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t maxreq=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_max_otf_read_request_set( %u)\n", maxreq);
        if(!err) ag_drv_qm_dma_max_otf_read_request_set(maxreq);
        if(!err) ag_drv_qm_dma_max_otf_read_request_get( &maxreq);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_max_otf_read_request_get( %u)\n", maxreq);
        if(err || maxreq!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean epnurgnt=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_epon_urgent_set( %u)\n", epnurgnt);
        if(!err) ag_drv_qm_dma_epon_urgent_set(epnurgnt);
        if(!err) ag_drv_qm_dma_epon_urgent_get( &epnurgnt);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_epon_urgent_get( %u)\n", epnurgnt);
        if(err || epnurgnt!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_bbh_sdma_cfg bbh_sdma_cfg = {.sdmasrc=gtmv(m, 6), .descbase=gtmv(m, 6), .descsize=gtmv(m, 6)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_sdma_cfg_set( %u %u %u)\n", bbh_sdma_cfg.sdmasrc, bbh_sdma_cfg.descbase, bbh_sdma_cfg.descsize);
        if(!err) ag_drv_qm_bbh_sdma_cfg_set(&bbh_sdma_cfg);
        if(!err) ag_drv_qm_bbh_sdma_cfg_get( &bbh_sdma_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_sdma_cfg_get( %u %u %u)\n", bbh_sdma_cfg.sdmasrc, bbh_sdma_cfg.descbase, bbh_sdma_cfg.descsize);
        if(err || bbh_sdma_cfg.sdmasrc!=gtmv(m, 6) || bbh_sdma_cfg.descbase!=gtmv(m, 6) || bbh_sdma_cfg.descsize!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t maxreq=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_qm_sdma_max_otf_read_request_set( %u)\n", maxreq);
        if(!err) ag_drv_qm_sdma_max_otf_read_request_set(maxreq);
        if(!err) ag_drv_qm_sdma_max_otf_read_request_get( &maxreq);
        if(!err) bdmf_session_print(session, "ag_drv_qm_sdma_max_otf_read_request_get( %u)\n", maxreq);
        if(err || maxreq!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean epnurgnt=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_sdma_epon_urgent_set( %u)\n", epnurgnt);
        if(!err) ag_drv_qm_sdma_epon_urgent_set(epnurgnt);
        if(!err) ag_drv_qm_sdma_epon_urgent_get( &epnurgnt);
        if(!err) bdmf_session_print(session, "ag_drv_qm_sdma_epon_urgent_get( %u)\n", epnurgnt);
        if(err || epnurgnt!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_bbh_ddr_cfg bbh_ddr_cfg = {.bufsize=gtmv(m, 3), .byteresul=gtmv(m, 1), .ddrtxoffset=gtmv(m, 9), .hnsize0=gtmv(m, 7), .hnsize1=gtmv(m, 7)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_ddr_cfg_set( %u %u %u %u %u)\n", bbh_ddr_cfg.bufsize, bbh_ddr_cfg.byteresul, bbh_ddr_cfg.ddrtxoffset, bbh_ddr_cfg.hnsize0, bbh_ddr_cfg.hnsize1);
        if(!err) ag_drv_qm_bbh_ddr_cfg_set(&bbh_ddr_cfg);
        if(!err) ag_drv_qm_bbh_ddr_cfg_get( &bbh_ddr_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_ddr_cfg_get( %u %u %u %u %u)\n", bbh_ddr_cfg.bufsize, bbh_ddr_cfg.byteresul, bbh_ddr_cfg.ddrtxoffset, bbh_ddr_cfg.hnsize0, bbh_ddr_cfg.hnsize1);
        if(err || bbh_ddr_cfg.bufsize!=gtmv(m, 3) || bbh_ddr_cfg.byteresul!=gtmv(m, 1) || bbh_ddr_cfg.ddrtxoffset!=gtmv(m, 9) || bbh_ddr_cfg.hnsize0!=gtmv(m, 7) || bbh_ddr_cfg.hnsize1!=gtmv(m, 7))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_debug_counters debug_counters = {.srampd=gtmv(m, 32), .ddrpd=gtmv(m, 32), .pddrop=gtmv(m, 16), .stscnt=gtmv(m, 32), .stsdrop=gtmv(m, 16), .msgcnt=gtmv(m, 32), .msgdrop=gtmv(m, 16), .getnextnull=gtmv(m, 16), .lenerr=gtmv(m, 16), .aggrlenerr=gtmv(m, 16), .srampkt=gtmv(m, 32), .ddrpkt=gtmv(m, 32), .flshpkts=gtmv(m, 16)};
        if(!err) ag_drv_qm_debug_counters_get( &debug_counters);
        if(!err) bdmf_session_print(session, "ag_drv_qm_debug_counters_get( %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", debug_counters.srampd, debug_counters.ddrpd, debug_counters.pddrop, debug_counters.stscnt, debug_counters.stsdrop, debug_counters.msgcnt, debug_counters.msgdrop, debug_counters.getnextnull, debug_counters.lenerr, debug_counters.aggrlenerr, debug_counters.srampkt, debug_counters.ddrpkt, debug_counters.flshpkts);
    }
    {
        qm_debug_info debug_info = {.nempty=gtmv(m, 16), .urgnt=gtmv(m, 16), .sel_src=gtmv(m, 6), .address=gtmv(m, 10), .datacs=gtmv(m, 1), .cdcs=gtmv(m, 1), .rrcs=gtmv(m, 1), .valid=gtmv(m, 1), .ready=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_debug_info_set( %u %u %u %u %u %u %u %u %u)\n", debug_info.nempty, debug_info.urgnt, debug_info.sel_src, debug_info.address, debug_info.datacs, debug_info.cdcs, debug_info.rrcs, debug_info.valid, debug_info.ready);
        if(!err) ag_drv_qm_debug_info_set(&debug_info);
        if(!err) ag_drv_qm_debug_info_get( &debug_info);
        if(!err) bdmf_session_print(session, "ag_drv_qm_debug_info_get( %u %u %u %u %u %u %u %u %u)\n", debug_info.nempty, debug_info.urgnt, debug_info.sel_src, debug_info.address, debug_info.datacs, debug_info.cdcs, debug_info.rrcs, debug_info.valid, debug_info.ready);
        if(err || debug_info.nempty!=gtmv(m, 16) || debug_info.urgnt!=gtmv(m, 16) || debug_info.sel_src!=gtmv(m, 6) || debug_info.address!=gtmv(m, 10) || debug_info.datacs!=gtmv(m, 1) || debug_info.cdcs!=gtmv(m, 1) || debug_info.rrcs!=gtmv(m, 1) || debug_info.valid!=gtmv(m, 1) || debug_info.ready!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_enable_ctrl enable_ctrl = {.fpm_prefetch_enable=gtmv(m, 1), .reorder_credit_enable=gtmv(m, 1), .dqm_pop_enable=gtmv(m, 1), .rmt_fixed_arb_enable=gtmv(m, 1), .dqm_push_fixed_arb_enable=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_enable_ctrl_set( %u %u %u %u %u)\n", enable_ctrl.fpm_prefetch_enable, enable_ctrl.reorder_credit_enable, enable_ctrl.dqm_pop_enable, enable_ctrl.rmt_fixed_arb_enable, enable_ctrl.dqm_push_fixed_arb_enable);
        if(!err) ag_drv_qm_enable_ctrl_set(&enable_ctrl);
        if(!err) ag_drv_qm_enable_ctrl_get( &enable_ctrl);
        if(!err) bdmf_session_print(session, "ag_drv_qm_enable_ctrl_get( %u %u %u %u %u)\n", enable_ctrl.fpm_prefetch_enable, enable_ctrl.reorder_credit_enable, enable_ctrl.dqm_pop_enable, enable_ctrl.rmt_fixed_arb_enable, enable_ctrl.dqm_push_fixed_arb_enable);
        if(err || enable_ctrl.fpm_prefetch_enable!=gtmv(m, 1) || enable_ctrl.reorder_credit_enable!=gtmv(m, 1) || enable_ctrl.dqm_pop_enable!=gtmv(m, 1) || enable_ctrl.rmt_fixed_arb_enable!=gtmv(m, 1) || enable_ctrl.dqm_push_fixed_arb_enable!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_reset_ctrl reset_ctrl = {.fpm_prefetch0_sw_rst=gtmv(m, 1), .fpm_prefetch1_sw_rst=gtmv(m, 1), .fpm_prefetch2_sw_rst=gtmv(m, 1), .fpm_prefetch3_sw_rst=gtmv(m, 1), .normal_rmt_sw_rst=gtmv(m, 1), .non_delayed_rmt_sw_rst=gtmv(m, 1), .pre_cm_fifo_sw_rst=gtmv(m, 1), .cm_rd_pd_fifo_sw_rst=gtmv(m, 1), .cm_wr_pd_fifo_sw_rst=gtmv(m, 1), .bb0_output_fifo_sw_rst=gtmv(m, 1), .bb1_output_fifo_sw_rst=gtmv(m, 1), .bb1_input_fifo_sw_rst=gtmv(m, 1), .tm_fifo_ptr_sw_rst=gtmv(m, 1), .non_delayed_out_fifo_sw_rst=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_reset_ctrl_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", reset_ctrl.fpm_prefetch0_sw_rst, reset_ctrl.fpm_prefetch1_sw_rst, reset_ctrl.fpm_prefetch2_sw_rst, reset_ctrl.fpm_prefetch3_sw_rst, reset_ctrl.normal_rmt_sw_rst, reset_ctrl.non_delayed_rmt_sw_rst, reset_ctrl.pre_cm_fifo_sw_rst, reset_ctrl.cm_rd_pd_fifo_sw_rst, reset_ctrl.cm_wr_pd_fifo_sw_rst, reset_ctrl.bb0_output_fifo_sw_rst, reset_ctrl.bb1_output_fifo_sw_rst, reset_ctrl.bb1_input_fifo_sw_rst, reset_ctrl.tm_fifo_ptr_sw_rst, reset_ctrl.non_delayed_out_fifo_sw_rst);
        if(!err) ag_drv_qm_reset_ctrl_set(&reset_ctrl);
        if(!err) ag_drv_qm_reset_ctrl_get( &reset_ctrl);
        if(!err) bdmf_session_print(session, "ag_drv_qm_reset_ctrl_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", reset_ctrl.fpm_prefetch0_sw_rst, reset_ctrl.fpm_prefetch1_sw_rst, reset_ctrl.fpm_prefetch2_sw_rst, reset_ctrl.fpm_prefetch3_sw_rst, reset_ctrl.normal_rmt_sw_rst, reset_ctrl.non_delayed_rmt_sw_rst, reset_ctrl.pre_cm_fifo_sw_rst, reset_ctrl.cm_rd_pd_fifo_sw_rst, reset_ctrl.cm_wr_pd_fifo_sw_rst, reset_ctrl.bb0_output_fifo_sw_rst, reset_ctrl.bb1_output_fifo_sw_rst, reset_ctrl.bb1_input_fifo_sw_rst, reset_ctrl.tm_fifo_ptr_sw_rst, reset_ctrl.non_delayed_out_fifo_sw_rst);
        if(err || reset_ctrl.fpm_prefetch0_sw_rst!=gtmv(m, 1) || reset_ctrl.fpm_prefetch1_sw_rst!=gtmv(m, 1) || reset_ctrl.fpm_prefetch2_sw_rst!=gtmv(m, 1) || reset_ctrl.fpm_prefetch3_sw_rst!=gtmv(m, 1) || reset_ctrl.normal_rmt_sw_rst!=gtmv(m, 1) || reset_ctrl.non_delayed_rmt_sw_rst!=gtmv(m, 1) || reset_ctrl.pre_cm_fifo_sw_rst!=gtmv(m, 1) || reset_ctrl.cm_rd_pd_fifo_sw_rst!=gtmv(m, 1) || reset_ctrl.cm_wr_pd_fifo_sw_rst!=gtmv(m, 1) || reset_ctrl.bb0_output_fifo_sw_rst!=gtmv(m, 1) || reset_ctrl.bb1_output_fifo_sw_rst!=gtmv(m, 1) || reset_ctrl.bb1_input_fifo_sw_rst!=gtmv(m, 1) || reset_ctrl.tm_fifo_ptr_sw_rst!=gtmv(m, 1) || reset_ctrl.non_delayed_out_fifo_sw_rst!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_drop_counters_ctrl drop_counters_ctrl = {.read_clear_pkts=gtmv(m, 1), .read_clear_bytes=gtmv(m, 1), .disable_wrap_around_pkts=gtmv(m, 1), .disable_wrap_around_bytes=gtmv(m, 1), .free_with_context_last_search=gtmv(m, 1), .wred_disable=gtmv(m, 1), .ddr_pd_congestion_disable=gtmv(m, 1), .ddr_byte_congestion_disable=gtmv(m, 1), .ddr_occupancy_disable=gtmv(m, 1), .ddr_fpm_congestion_disable=gtmv(m, 1), .fpm_ug_disable=gtmv(m, 1), .queue_occupancy_ddr_copy_decision_disable=gtmv(m, 1), .psram_occupancy_ddr_copy_decision_disable=gtmv(m, 1), .dont_send_mc_bit_to_bbh=gtmv(m, 1), .close_aggregation_on_timeout_disable=gtmv(m, 1), .fpm_congestion_buf_release_mechanism_disable=gtmv(m, 1), .fpm_buffer_global_res_enable=gtmv(m, 1), .qm_preserve_pd_with_fpm=gtmv(m, 1), .qm_residue_per_queue=gtmv(m, 1), .ghost_rpt_update_after_close_agg_en=gtmv(m, 1), .fpm_ug_flow_ctrl_disable=gtmv(m, 1), .ddr_write_multi_slave_en=gtmv(m, 1), .ddr_pd_congestion_agg_priority=gtmv(m, 1), .psram_occupancy_drop_disable=gtmv(m, 1), .qm_ddr_write_alignment=gtmv(m, 1), .exclusive_dont_drop=gtmv(m, 1), .exclusive_dont_drop_bp_en=gtmv(m, 1), .gpon_dbr_ceil=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_drop_counters_ctrl_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", drop_counters_ctrl.read_clear_pkts, drop_counters_ctrl.read_clear_bytes, drop_counters_ctrl.disable_wrap_around_pkts, drop_counters_ctrl.disable_wrap_around_bytes, drop_counters_ctrl.free_with_context_last_search, drop_counters_ctrl.wred_disable, drop_counters_ctrl.ddr_pd_congestion_disable, drop_counters_ctrl.ddr_byte_congestion_disable, drop_counters_ctrl.ddr_occupancy_disable, drop_counters_ctrl.ddr_fpm_congestion_disable, drop_counters_ctrl.fpm_ug_disable, drop_counters_ctrl.queue_occupancy_ddr_copy_decision_disable, drop_counters_ctrl.psram_occupancy_ddr_copy_decision_disable, drop_counters_ctrl.dont_send_mc_bit_to_bbh, drop_counters_ctrl.close_aggregation_on_timeout_disable, drop_counters_ctrl.fpm_congestion_buf_release_mechanism_disable, drop_counters_ctrl.fpm_buffer_global_res_enable, drop_counters_ctrl.qm_preserve_pd_with_fpm, drop_counters_ctrl.qm_residue_per_queue, drop_counters_ctrl.ghost_rpt_update_after_close_agg_en, drop_counters_ctrl.fpm_ug_flow_ctrl_disable, drop_counters_ctrl.ddr_write_multi_slave_en, drop_counters_ctrl.ddr_pd_congestion_agg_priority, drop_counters_ctrl.psram_occupancy_drop_disable, drop_counters_ctrl.qm_ddr_write_alignment, drop_counters_ctrl.exclusive_dont_drop, drop_counters_ctrl.exclusive_dont_drop_bp_en, drop_counters_ctrl.gpon_dbr_ceil);
        if(!err) ag_drv_qm_drop_counters_ctrl_set(&drop_counters_ctrl);
        if(!err) ag_drv_qm_drop_counters_ctrl_get( &drop_counters_ctrl);
        if(!err) bdmf_session_print(session, "ag_drv_qm_drop_counters_ctrl_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", drop_counters_ctrl.read_clear_pkts, drop_counters_ctrl.read_clear_bytes, drop_counters_ctrl.disable_wrap_around_pkts, drop_counters_ctrl.disable_wrap_around_bytes, drop_counters_ctrl.free_with_context_last_search, drop_counters_ctrl.wred_disable, drop_counters_ctrl.ddr_pd_congestion_disable, drop_counters_ctrl.ddr_byte_congestion_disable, drop_counters_ctrl.ddr_occupancy_disable, drop_counters_ctrl.ddr_fpm_congestion_disable, drop_counters_ctrl.fpm_ug_disable, drop_counters_ctrl.queue_occupancy_ddr_copy_decision_disable, drop_counters_ctrl.psram_occupancy_ddr_copy_decision_disable, drop_counters_ctrl.dont_send_mc_bit_to_bbh, drop_counters_ctrl.close_aggregation_on_timeout_disable, drop_counters_ctrl.fpm_congestion_buf_release_mechanism_disable, drop_counters_ctrl.fpm_buffer_global_res_enable, drop_counters_ctrl.qm_preserve_pd_with_fpm, drop_counters_ctrl.qm_residue_per_queue, drop_counters_ctrl.ghost_rpt_update_after_close_agg_en, drop_counters_ctrl.fpm_ug_flow_ctrl_disable, drop_counters_ctrl.ddr_write_multi_slave_en, drop_counters_ctrl.ddr_pd_congestion_agg_priority, drop_counters_ctrl.psram_occupancy_drop_disable, drop_counters_ctrl.qm_ddr_write_alignment, drop_counters_ctrl.exclusive_dont_drop, drop_counters_ctrl.exclusive_dont_drop_bp_en, drop_counters_ctrl.gpon_dbr_ceil);
        if(err || drop_counters_ctrl.read_clear_pkts!=gtmv(m, 1) || drop_counters_ctrl.read_clear_bytes!=gtmv(m, 1) || drop_counters_ctrl.disable_wrap_around_pkts!=gtmv(m, 1) || drop_counters_ctrl.disable_wrap_around_bytes!=gtmv(m, 1) || drop_counters_ctrl.free_with_context_last_search!=gtmv(m, 1) || drop_counters_ctrl.wred_disable!=gtmv(m, 1) || drop_counters_ctrl.ddr_pd_congestion_disable!=gtmv(m, 1) || drop_counters_ctrl.ddr_byte_congestion_disable!=gtmv(m, 1) || drop_counters_ctrl.ddr_occupancy_disable!=gtmv(m, 1) || drop_counters_ctrl.ddr_fpm_congestion_disable!=gtmv(m, 1) || drop_counters_ctrl.fpm_ug_disable!=gtmv(m, 1) || drop_counters_ctrl.queue_occupancy_ddr_copy_decision_disable!=gtmv(m, 1) || drop_counters_ctrl.psram_occupancy_ddr_copy_decision_disable!=gtmv(m, 1) || drop_counters_ctrl.dont_send_mc_bit_to_bbh!=gtmv(m, 1) || drop_counters_ctrl.close_aggregation_on_timeout_disable!=gtmv(m, 1) || drop_counters_ctrl.fpm_congestion_buf_release_mechanism_disable!=gtmv(m, 1) || drop_counters_ctrl.fpm_buffer_global_res_enable!=gtmv(m, 1) || drop_counters_ctrl.qm_preserve_pd_with_fpm!=gtmv(m, 1) || drop_counters_ctrl.qm_residue_per_queue!=gtmv(m, 1) || drop_counters_ctrl.ghost_rpt_update_after_close_agg_en!=gtmv(m, 1) || drop_counters_ctrl.fpm_ug_flow_ctrl_disable!=gtmv(m, 1) || drop_counters_ctrl.ddr_write_multi_slave_en!=gtmv(m, 1) || drop_counters_ctrl.ddr_pd_congestion_agg_priority!=gtmv(m, 1) || drop_counters_ctrl.psram_occupancy_drop_disable!=gtmv(m, 1) || drop_counters_ctrl.qm_ddr_write_alignment!=gtmv(m, 1) || drop_counters_ctrl.exclusive_dont_drop!=gtmv(m, 1) || drop_counters_ctrl.exclusive_dont_drop_bp_en!=gtmv(m, 1) || drop_counters_ctrl.gpon_dbr_ceil!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean fpm_pool_bp_enable=gtmv(m, 1);
        bdmf_boolean fpm_congestion_bp_enable=gtmv(m, 1);
        uint8_t fpm_prefetch_min_pool_size=gtmv(m, 2);
        uint8_t fpm_prefetch_pending_req_limit=gtmv(m, 7);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_ctrl_set( %u %u %u %u)\n", fpm_pool_bp_enable, fpm_congestion_bp_enable, fpm_prefetch_min_pool_size, fpm_prefetch_pending_req_limit);
        if(!err) ag_drv_qm_fpm_ctrl_set(fpm_pool_bp_enable, fpm_congestion_bp_enable, fpm_prefetch_min_pool_size, fpm_prefetch_pending_req_limit);
        if(!err) ag_drv_qm_fpm_ctrl_get( &fpm_pool_bp_enable, &fpm_congestion_bp_enable, &fpm_prefetch_min_pool_size, &fpm_prefetch_pending_req_limit);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_ctrl_get( %u %u %u %u)\n", fpm_pool_bp_enable, fpm_congestion_bp_enable, fpm_prefetch_min_pool_size, fpm_prefetch_pending_req_limit);
        if(err || fpm_pool_bp_enable!=gtmv(m, 1) || fpm_congestion_bp_enable!=gtmv(m, 1) || fpm_prefetch_min_pool_size!=gtmv(m, 2) || fpm_prefetch_pending_req_limit!=gtmv(m, 7))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t total_pd_thr=gtmv(m, 28);
        if(!err) bdmf_session_print(session, "ag_drv_qm_qm_pd_cong_ctrl_set( %u)\n", total_pd_thr);
        if(!err) ag_drv_qm_qm_pd_cong_ctrl_set(total_pd_thr);
        if(!err) ag_drv_qm_qm_pd_cong_ctrl_get( &total_pd_thr);
        if(!err) bdmf_session_print(session, "ag_drv_qm_qm_pd_cong_ctrl_get( %u)\n", total_pd_thr);
        if(err || total_pd_thr!=gtmv(m, 28))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t abs_drop_queue=gtmv(m, 9);
        bdmf_boolean abs_drop_queue_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_abs_drop_queue_set( %u %u)\n", abs_drop_queue, abs_drop_queue_en);
        if(!err) ag_drv_qm_global_cfg_abs_drop_queue_set(abs_drop_queue, abs_drop_queue_en);
        if(!err) ag_drv_qm_global_cfg_abs_drop_queue_get( &abs_drop_queue, &abs_drop_queue_en);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_abs_drop_queue_get( %u %u)\n", abs_drop_queue, abs_drop_queue_en);
        if(err || abs_drop_queue!=gtmv(m, 9) || abs_drop_queue_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t max_agg_bytes=gtmv(m, 10);
        uint8_t max_agg_pkts=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_aggregation_ctrl_set( %u %u)\n", max_agg_bytes, max_agg_pkts);
        if(!err) ag_drv_qm_global_cfg_aggregation_ctrl_set(max_agg_bytes, max_agg_pkts);
        if(!err) ag_drv_qm_global_cfg_aggregation_ctrl_get( &max_agg_bytes, &max_agg_pkts);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_aggregation_ctrl_get( %u %u)\n", max_agg_bytes, max_agg_pkts);
        if(err || max_agg_bytes!=gtmv(m, 10) || max_agg_pkts!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t fpm_base_addr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_base_addr_set( %u)\n", fpm_base_addr);
        if(!err) ag_drv_qm_fpm_base_addr_set(fpm_base_addr);
        if(!err) ag_drv_qm_fpm_base_addr_get( &fpm_base_addr);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_base_addr_get( %u)\n", fpm_base_addr);
        if(err || fpm_base_addr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t fpm_base_addr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_fpm_coherent_base_addr_set( %u)\n", fpm_base_addr);
        if(!err) ag_drv_qm_global_cfg_fpm_coherent_base_addr_set(fpm_base_addr);
        if(!err) ag_drv_qm_global_cfg_fpm_coherent_base_addr_get( &fpm_base_addr);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_fpm_coherent_base_addr_get( %u)\n", fpm_base_addr);
        if(err || fpm_base_addr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t ddr_sop_offset0=gtmv(m, 11);
        uint16_t ddr_sop_offset1=gtmv(m, 11);
        if(!err) bdmf_session_print(session, "ag_drv_qm_ddr_sop_offset_set( %u %u)\n", ddr_sop_offset0, ddr_sop_offset1);
        if(!err) ag_drv_qm_ddr_sop_offset_set(ddr_sop_offset0, ddr_sop_offset1);
        if(!err) ag_drv_qm_ddr_sop_offset_get( &ddr_sop_offset0, &ddr_sop_offset1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_ddr_sop_offset_get( %u %u)\n", ddr_sop_offset0, ddr_sop_offset1);
        if(err || ddr_sop_offset0!=gtmv(m, 11) || ddr_sop_offset1!=gtmv(m, 11))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_epon_overhead_ctrl epon_overhead_ctrl = {.epon_line_rate=gtmv(m, 1), .epon_crc_add_disable=gtmv(m, 1), .mac_flow_overwrite_crc_en=gtmv(m, 1), .mac_flow_overwrite_crc=gtmv(m, 8), .fec_ipg_length=gtmv(m, 11)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_epon_overhead_ctrl_set( %u %u %u %u %u)\n", epon_overhead_ctrl.epon_line_rate, epon_overhead_ctrl.epon_crc_add_disable, epon_overhead_ctrl.mac_flow_overwrite_crc_en, epon_overhead_ctrl.mac_flow_overwrite_crc, epon_overhead_ctrl.fec_ipg_length);
        if(!err) ag_drv_qm_epon_overhead_ctrl_set(&epon_overhead_ctrl);
        if(!err) ag_drv_qm_epon_overhead_ctrl_get( &epon_overhead_ctrl);
        if(!err) bdmf_session_print(session, "ag_drv_qm_epon_overhead_ctrl_get( %u %u %u %u %u)\n", epon_overhead_ctrl.epon_line_rate, epon_overhead_ctrl.epon_crc_add_disable, epon_overhead_ctrl.mac_flow_overwrite_crc_en, epon_overhead_ctrl.mac_flow_overwrite_crc, epon_overhead_ctrl.fec_ipg_length);
        if(err || epon_overhead_ctrl.epon_line_rate!=gtmv(m, 1) || epon_overhead_ctrl.epon_crc_add_disable!=gtmv(m, 1) || epon_overhead_ctrl.mac_flow_overwrite_crc_en!=gtmv(m, 1) || epon_overhead_ctrl.mac_flow_overwrite_crc!=gtmv(m, 8) || epon_overhead_ctrl.fec_ipg_length!=gtmv(m, 11))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t prescaler_granularity=gtmv(m, 3);
        uint8_t aggregation_timeout_value=gtmv(m, 3);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set( %u %u)\n", prescaler_granularity, aggregation_timeout_value);
        if(!err) ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set(prescaler_granularity, aggregation_timeout_value);
        if(!err) ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_get( &prescaler_granularity, &aggregation_timeout_value);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_get( %u %u)\n", prescaler_granularity, aggregation_timeout_value);
        if(err || prescaler_granularity!=gtmv(m, 3) || aggregation_timeout_value!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t idx=gtmv(m, 1);
        uint8_t res_thr_0=gtmv(m, 8);
        uint8_t res_thr_1=gtmv(m, 8);
        uint8_t res_thr_2=gtmv(m, 8);
        uint8_t res_thr_3=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_fpm_buffer_grp_res_set( %u %u %u %u %u)\n", idx, res_thr_0, res_thr_1, res_thr_2, res_thr_3);
        if(!err) ag_drv_qm_global_cfg_qm_fpm_buffer_grp_res_set(idx, res_thr_0, res_thr_1, res_thr_2, res_thr_3);
        if(!err) ag_drv_qm_global_cfg_qm_fpm_buffer_grp_res_get( idx, &res_thr_0, &res_thr_1, &res_thr_2, &res_thr_3);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_fpm_buffer_grp_res_get( %u %u %u %u %u)\n", idx, res_thr_0, res_thr_1, res_thr_2, res_thr_3);
        if(err || res_thr_0!=gtmv(m, 8) || res_thr_1!=gtmv(m, 8) || res_thr_2!=gtmv(m, 8) || res_thr_3!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t res_thr_global=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_fpm_buffer_gbl_thr_set( %u)\n", res_thr_global);
        if(!err) ag_drv_qm_global_cfg_qm_fpm_buffer_gbl_thr_set(res_thr_global);
        if(!err) ag_drv_qm_global_cfg_qm_fpm_buffer_gbl_thr_get( &res_thr_global);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_fpm_buffer_gbl_thr_get( %u)\n", res_thr_global);
        if(err || res_thr_global!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t rnr_bb_id=gtmv(m, 6);
        uint8_t rnr_task=gtmv(m, 4);
        bdmf_boolean rnr_enable=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_flow_ctrl_rnr_cfg_set( %u %u %u)\n", rnr_bb_id, rnr_task, rnr_enable);
        if(!err) ag_drv_qm_global_cfg_qm_flow_ctrl_rnr_cfg_set(rnr_bb_id, rnr_task, rnr_enable);
        if(!err) ag_drv_qm_global_cfg_qm_flow_ctrl_rnr_cfg_get( &rnr_bb_id, &rnr_task, &rnr_enable);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_flow_ctrl_rnr_cfg_get( %u %u %u)\n", rnr_bb_id, rnr_task, rnr_enable);
        if(err || rnr_bb_id!=gtmv(m, 6) || rnr_task!=gtmv(m, 4) || rnr_enable!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t qm_flow_ctrl_intr=gtmv(m, 5);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_flow_ctrl_intr_set( %u)\n", qm_flow_ctrl_intr);
        if(!err) ag_drv_qm_global_cfg_qm_flow_ctrl_intr_set(qm_flow_ctrl_intr);
        if(!err) ag_drv_qm_global_cfg_qm_flow_ctrl_intr_get( &qm_flow_ctrl_intr);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_flow_ctrl_intr_get( %u)\n", qm_flow_ctrl_intr);
        if(err || qm_flow_ctrl_intr!=gtmv(m, 5))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t fpm_gbl_cnt=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_set( %u)\n", fpm_gbl_cnt);
        if(!err) ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_set(fpm_gbl_cnt);
        if(!err) ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_get( &fpm_gbl_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_get( %u)\n", fpm_gbl_cnt);
        if(err || fpm_gbl_cnt!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t queue_num=gtmv(m, 9);
        bdmf_boolean flush_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_egress_flush_queue_set( %u %u)\n", queue_num, flush_en);
        if(!err) ag_drv_qm_global_cfg_qm_egress_flush_queue_set(queue_num, flush_en);
        if(!err) ag_drv_qm_global_cfg_qm_egress_flush_queue_get( &queue_num, &flush_en);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_egress_flush_queue_get( %u %u)\n", queue_num, flush_en);
        if(err || queue_num!=gtmv(m, 9) || flush_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t pool_idx=gtmv(m, 2);
        qm_fpm_pool_thr fpm_pool_thr = {.lower_thr=gtmv(m, 7), .higher_thr=gtmv(m, 7)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_pool_thr_set( %u %u %u)\n", pool_idx, fpm_pool_thr.lower_thr, fpm_pool_thr.higher_thr);
        if(!err) ag_drv_qm_fpm_pool_thr_set(pool_idx, &fpm_pool_thr);
        if(!err) ag_drv_qm_fpm_pool_thr_get( pool_idx, &fpm_pool_thr);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_pool_thr_get( %u %u %u)\n", pool_idx, fpm_pool_thr.lower_thr, fpm_pool_thr.higher_thr);
        if(err || fpm_pool_thr.lower_thr!=gtmv(m, 7) || fpm_pool_thr.higher_thr!=gtmv(m, 7))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t grp_idx=gtmv(m, 2);
        uint16_t fpm_ug_cnt=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_ug_cnt_set( %u %u)\n", grp_idx, fpm_ug_cnt);
        if(!err) ag_drv_qm_fpm_ug_cnt_set(grp_idx, fpm_ug_cnt);
        if(!err) ag_drv_qm_fpm_ug_cnt_get( grp_idx, &fpm_ug_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_ug_cnt_get( %u %u)\n", grp_idx, fpm_ug_cnt);
        if(err || fpm_ug_cnt!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_intr_ctrl_isr intr_ctrl_isr = {.qm_dqm_pop_on_empty=gtmv(m, 1), .qm_dqm_push_on_full=gtmv(m, 1), .qm_cpu_pop_on_empty=gtmv(m, 1), .qm_cpu_push_on_full=gtmv(m, 1), .qm_normal_queue_pd_no_credit=gtmv(m, 1), .qm_non_delayed_queue_pd_no_credit=gtmv(m, 1), .qm_non_valid_queue=gtmv(m, 1), .qm_agg_coherent_inconsistency=gtmv(m, 1), .qm_force_copy_on_non_delayed=gtmv(m, 1), .qm_fpm_pool_size_nonexistent=gtmv(m, 1), .qm_target_mem_abs_contradiction=gtmv(m, 1), .qm_1588_drop=gtmv(m, 1), .qm_1588_multicast_contradiction=gtmv(m, 1), .qm_byte_drop_cnt_overrun=gtmv(m, 1), .qm_pkt_drop_cnt_overrun=gtmv(m, 1), .qm_total_byte_cnt_underrun=gtmv(m, 1), .qm_total_pkt_cnt_underrun=gtmv(m, 1), .qm_fpm_ug0_underrun=gtmv(m, 1), .qm_fpm_ug1_underrun=gtmv(m, 1), .qm_fpm_ug2_underrun=gtmv(m, 1), .qm_fpm_ug3_underrun=gtmv(m, 1), .qm_timer_wraparound=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_intr_ctrl_isr_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", intr_ctrl_isr.qm_dqm_pop_on_empty, intr_ctrl_isr.qm_dqm_push_on_full, intr_ctrl_isr.qm_cpu_pop_on_empty, intr_ctrl_isr.qm_cpu_push_on_full, intr_ctrl_isr.qm_normal_queue_pd_no_credit, intr_ctrl_isr.qm_non_delayed_queue_pd_no_credit, intr_ctrl_isr.qm_non_valid_queue, intr_ctrl_isr.qm_agg_coherent_inconsistency, intr_ctrl_isr.qm_force_copy_on_non_delayed, intr_ctrl_isr.qm_fpm_pool_size_nonexistent, intr_ctrl_isr.qm_target_mem_abs_contradiction, intr_ctrl_isr.qm_1588_drop, intr_ctrl_isr.qm_1588_multicast_contradiction, intr_ctrl_isr.qm_byte_drop_cnt_overrun, intr_ctrl_isr.qm_pkt_drop_cnt_overrun, intr_ctrl_isr.qm_total_byte_cnt_underrun, intr_ctrl_isr.qm_total_pkt_cnt_underrun, intr_ctrl_isr.qm_fpm_ug0_underrun, intr_ctrl_isr.qm_fpm_ug1_underrun, intr_ctrl_isr.qm_fpm_ug2_underrun, intr_ctrl_isr.qm_fpm_ug3_underrun, intr_ctrl_isr.qm_timer_wraparound);
        if(!err) ag_drv_qm_intr_ctrl_isr_set(&intr_ctrl_isr);
        if(!err) ag_drv_qm_intr_ctrl_isr_get( &intr_ctrl_isr);
        if(!err) bdmf_session_print(session, "ag_drv_qm_intr_ctrl_isr_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", intr_ctrl_isr.qm_dqm_pop_on_empty, intr_ctrl_isr.qm_dqm_push_on_full, intr_ctrl_isr.qm_cpu_pop_on_empty, intr_ctrl_isr.qm_cpu_push_on_full, intr_ctrl_isr.qm_normal_queue_pd_no_credit, intr_ctrl_isr.qm_non_delayed_queue_pd_no_credit, intr_ctrl_isr.qm_non_valid_queue, intr_ctrl_isr.qm_agg_coherent_inconsistency, intr_ctrl_isr.qm_force_copy_on_non_delayed, intr_ctrl_isr.qm_fpm_pool_size_nonexistent, intr_ctrl_isr.qm_target_mem_abs_contradiction, intr_ctrl_isr.qm_1588_drop, intr_ctrl_isr.qm_1588_multicast_contradiction, intr_ctrl_isr.qm_byte_drop_cnt_overrun, intr_ctrl_isr.qm_pkt_drop_cnt_overrun, intr_ctrl_isr.qm_total_byte_cnt_underrun, intr_ctrl_isr.qm_total_pkt_cnt_underrun, intr_ctrl_isr.qm_fpm_ug0_underrun, intr_ctrl_isr.qm_fpm_ug1_underrun, intr_ctrl_isr.qm_fpm_ug2_underrun, intr_ctrl_isr.qm_fpm_ug3_underrun, intr_ctrl_isr.qm_timer_wraparound);
        if(err || intr_ctrl_isr.qm_dqm_pop_on_empty!=gtmv(m, 1) || intr_ctrl_isr.qm_dqm_push_on_full!=gtmv(m, 1) || intr_ctrl_isr.qm_cpu_pop_on_empty!=gtmv(m, 1) || intr_ctrl_isr.qm_cpu_push_on_full!=gtmv(m, 1) || intr_ctrl_isr.qm_normal_queue_pd_no_credit!=gtmv(m, 1) || intr_ctrl_isr.qm_non_delayed_queue_pd_no_credit!=gtmv(m, 1) || intr_ctrl_isr.qm_non_valid_queue!=gtmv(m, 1) || intr_ctrl_isr.qm_agg_coherent_inconsistency!=gtmv(m, 1) || intr_ctrl_isr.qm_force_copy_on_non_delayed!=gtmv(m, 1) || intr_ctrl_isr.qm_fpm_pool_size_nonexistent!=gtmv(m, 1) || intr_ctrl_isr.qm_target_mem_abs_contradiction!=gtmv(m, 1) || intr_ctrl_isr.qm_1588_drop!=gtmv(m, 1) || intr_ctrl_isr.qm_1588_multicast_contradiction!=gtmv(m, 1) || intr_ctrl_isr.qm_byte_drop_cnt_overrun!=gtmv(m, 1) || intr_ctrl_isr.qm_pkt_drop_cnt_overrun!=gtmv(m, 1) || intr_ctrl_isr.qm_total_byte_cnt_underrun!=gtmv(m, 1) || intr_ctrl_isr.qm_total_pkt_cnt_underrun!=gtmv(m, 1) || intr_ctrl_isr.qm_fpm_ug0_underrun!=gtmv(m, 1) || intr_ctrl_isr.qm_fpm_ug1_underrun!=gtmv(m, 1) || intr_ctrl_isr.qm_fpm_ug2_underrun!=gtmv(m, 1) || intr_ctrl_isr.qm_fpm_ug3_underrun!=gtmv(m, 1) || intr_ctrl_isr.qm_timer_wraparound!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t ism=gtmv(m, 21);
        if(!err) ag_drv_qm_intr_ctrl_ism_get( &ism);
        if(!err) bdmf_session_print(session, "ag_drv_qm_intr_ctrl_ism_get( %u)\n", ism);
    }
    {
        uint32_t iem=gtmv(m, 21);
        if(!err) bdmf_session_print(session, "ag_drv_qm_intr_ctrl_ier_set( %u)\n", iem);
        if(!err) ag_drv_qm_intr_ctrl_ier_set(iem);
        if(!err) ag_drv_qm_intr_ctrl_ier_get( &iem);
        if(!err) bdmf_session_print(session, "ag_drv_qm_intr_ctrl_ier_get( %u)\n", iem);
        if(err || iem!=gtmv(m, 21))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t ist=gtmv(m, 21);
        if(!err) bdmf_session_print(session, "ag_drv_qm_intr_ctrl_itr_set( %u)\n", ist);
        if(!err) ag_drv_qm_intr_ctrl_itr_set(ist);
    }
    {
        qm_clk_gate_clk_gate_cntrl clk_gate_clk_gate_cntrl = {.bypass_clk_gate=gtmv(m, 1), .timer_val=gtmv(m, 8), .keep_alive_en=gtmv(m, 1), .keep_alive_intrvl=gtmv(m, 3), .keep_alive_cyc=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_clk_gate_clk_gate_cntrl_set( %u %u %u %u %u)\n", clk_gate_clk_gate_cntrl.bypass_clk_gate, clk_gate_clk_gate_cntrl.timer_val, clk_gate_clk_gate_cntrl.keep_alive_en, clk_gate_clk_gate_cntrl.keep_alive_intrvl, clk_gate_clk_gate_cntrl.keep_alive_cyc);
        if(!err) ag_drv_qm_clk_gate_clk_gate_cntrl_set(&clk_gate_clk_gate_cntrl);
        if(!err) ag_drv_qm_clk_gate_clk_gate_cntrl_get( &clk_gate_clk_gate_cntrl);
        if(!err) bdmf_session_print(session, "ag_drv_qm_clk_gate_clk_gate_cntrl_get( %u %u %u %u %u)\n", clk_gate_clk_gate_cntrl.bypass_clk_gate, clk_gate_clk_gate_cntrl.timer_val, clk_gate_clk_gate_cntrl.keep_alive_en, clk_gate_clk_gate_cntrl.keep_alive_intrvl, clk_gate_clk_gate_cntrl.keep_alive_cyc);
        if(err || clk_gate_clk_gate_cntrl.bypass_clk_gate!=gtmv(m, 1) || clk_gate_clk_gate_cntrl.timer_val!=gtmv(m, 8) || clk_gate_clk_gate_cntrl.keep_alive_en!=gtmv(m, 1) || clk_gate_clk_gate_cntrl.keep_alive_intrvl!=gtmv(m, 3) || clk_gate_clk_gate_cntrl.keep_alive_cyc!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t indirect_grp_idx=gtmv(m, 2);
        uint16_t queue_num=gtmv(m, 9);
        uint8_t cmd=gtmv(m, 2);
        bdmf_boolean done=gtmv(m, 1);
        bdmf_boolean error=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set( %u %u %u %u %u)\n", indirect_grp_idx, queue_num, cmd, done, error);
        if(!err) ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set(indirect_grp_idx, queue_num, cmd, done, error);
        if(!err) ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_get( indirect_grp_idx, &queue_num, &cmd, &done, &error);
        if(!err) bdmf_session_print(session, "ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_get( %u %u %u %u %u)\n", indirect_grp_idx, queue_num, cmd, done, error);
        if(err || queue_num!=gtmv(m, 9) || cmd!=gtmv(m, 2) || done!=gtmv(m, 1) || error!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t q_idx=gtmv(m, 5);
        qm_q_context q_context = {.wred_profile=gtmv(m, 4), .copy_dec_profile=gtmv(m, 3), .copy_to_ddr=gtmv(m, 1), .ddr_copy_disable=gtmv(m, 1), .aggregation_disable=gtmv(m, 1), .fpm_ug=gtmv(m, 3), .exclusive_priority=gtmv(m, 1), .q_802_1ae=gtmv(m, 1), .sci=gtmv(m, 1), .fec_enable=gtmv(m, 1), .res_profile=gtmv(m, 3)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_q_context_set( %u %u %u %u %u %u %u %u %u %u %u %u)\n", q_idx, q_context.wred_profile, q_context.copy_dec_profile, q_context.copy_to_ddr, q_context.ddr_copy_disable, q_context.aggregation_disable, q_context.fpm_ug, q_context.exclusive_priority, q_context.q_802_1ae, q_context.sci, q_context.fec_enable, q_context.res_profile);
        if(!err) ag_drv_qm_q_context_set(q_idx, &q_context);
        if(!err) ag_drv_qm_q_context_get( q_idx, &q_context);
        if(!err) bdmf_session_print(session, "ag_drv_qm_q_context_get( %u %u %u %u %u %u %u %u %u %u %u %u)\n", q_idx, q_context.wred_profile, q_context.copy_dec_profile, q_context.copy_to_ddr, q_context.ddr_copy_disable, q_context.aggregation_disable, q_context.fpm_ug, q_context.exclusive_priority, q_context.q_802_1ae, q_context.sci, q_context.fec_enable, q_context.res_profile);
        if(err || q_context.wred_profile!=gtmv(m, 4) || q_context.copy_dec_profile!=gtmv(m, 3) || q_context.copy_to_ddr!=gtmv(m, 1) || q_context.ddr_copy_disable!=gtmv(m, 1) || q_context.aggregation_disable!=gtmv(m, 1) || q_context.fpm_ug!=gtmv(m, 3) || q_context.exclusive_priority!=gtmv(m, 1) || q_context.q_802_1ae!=gtmv(m, 1) || q_context.sci!=gtmv(m, 1) || q_context.fec_enable!=gtmv(m, 1) || q_context.res_profile!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t profile_idx=gtmv(m, 3);
        uint32_t queue_occupancy_thr=gtmv(m, 30);
        bdmf_boolean psram_thr=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_copy_decision_profile_set( %u %u %u)\n", profile_idx, queue_occupancy_thr, psram_thr);
        if(!err) ag_drv_qm_copy_decision_profile_set(profile_idx, queue_occupancy_thr, psram_thr);
        if(!err) ag_drv_qm_copy_decision_profile_get( profile_idx, &queue_occupancy_thr, &psram_thr);
        if(!err) bdmf_session_print(session, "ag_drv_qm_copy_decision_profile_get( %u %u %u)\n", profile_idx, queue_occupancy_thr, psram_thr);
        if(err || queue_occupancy_thr!=gtmv(m, 30) || psram_thr!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t q_idx=gtmv(m, 6);
        uint32_t data=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_qm_total_valid_cnt_set( %u %u)\n", q_idx, data);
        if(!err) ag_drv_qm_total_valid_cnt_set(q_idx, data);
        if(!err) ag_drv_qm_total_valid_cnt_get( q_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_total_valid_cnt_get( %u %u)\n", q_idx, data);
        if(err || data!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t q_idx=gtmv(m, 6);
        uint32_t data=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dqm_valid_cnt_set( %u %u)\n", q_idx, data);
        if(!err) ag_drv_qm_dqm_valid_cnt_set(q_idx, data);
        if(!err) ag_drv_qm_dqm_valid_cnt_get( q_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dqm_valid_cnt_get( %u %u)\n", q_idx, data);
        if(err || data!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t q_idx=gtmv(m, 6);
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_qm_drop_counter_get( q_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_drop_counter_get( %u %u)\n", q_idx, data);
    }
    {
        uint16_t q_idx=gtmv(m, 6);
        uint32_t data=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_qm_epon_q_byte_cnt_set( %u %u)\n", q_idx, data);
        if(!err) ag_drv_qm_epon_q_byte_cnt_set(q_idx, data);
        if(!err) ag_drv_qm_epon_q_byte_cnt_get( q_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_epon_q_byte_cnt_get( %u %u)\n", q_idx, data);
        if(err || data!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t q_idx=gtmv(m, 0);
        uint32_t status_bit_vector=gtmv(m, 32);
        if(!err) ag_drv_qm_epon_q_status_get( q_idx, &status_bit_vector);
        if(!err) bdmf_session_print(session, "ag_drv_qm_epon_q_status_get( %u %u)\n", q_idx, status_bit_vector);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_qm_rd_data_pool0_get( &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_rd_data_pool0_get( %u)\n", data);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_qm_rd_data_pool1_get( &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_rd_data_pool1_get( %u)\n", data);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_qm_rd_data_pool2_get( &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_rd_data_pool2_get( %u)\n", data);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_qm_rd_data_pool3_get( &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_rd_data_pool3_get( %u)\n", data);
    }
    {
        uint16_t q_idx=gtmv(m, 5);
        uint8_t wr_ptr=gtmv(m, 4);
        uint8_t rd_ptr=gtmv(m, 4);
        if(!err) ag_drv_qm_pdfifo_ptr_get( q_idx, &wr_ptr, &rd_ptr);
        if(!err) bdmf_session_print(session, "ag_drv_qm_pdfifo_ptr_get( %u %u %u)\n", q_idx, wr_ptr, rd_ptr);
    }
    {
        uint16_t fifo_idx=gtmv(m, 4);
        uint16_t wr_ptr=gtmv(m, 9);
        uint8_t rd_ptr=gtmv(m, 7);
        if(!err) ag_drv_qm_update_fifo_ptr_get( fifo_idx, &wr_ptr, &rd_ptr);
        if(!err) bdmf_session_print(session, "ag_drv_qm_update_fifo_ptr_get( %u %u %u)\n", fifo_idx, wr_ptr, rd_ptr);
    }
    {
        uint8_t select=gtmv(m, 5);
        bdmf_boolean enable=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_debug_sel_set( %u %u)\n", select, enable);
        if(!err) ag_drv_qm_debug_sel_set(select, enable);
        if(!err) ag_drv_qm_debug_sel_get( &select, &enable);
        if(!err) bdmf_session_print(session, "ag_drv_qm_debug_sel_get( %u %u)\n", select, enable);
        if(err || select!=gtmv(m, 5) || enable!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_qm_debug_bus_lsb_get( &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_debug_bus_lsb_get( %u)\n", data);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_qm_debug_bus_msb_get( &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_debug_bus_msb_get( %u)\n", data);
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_qm_qm_spare_config_get( &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_qm_spare_config_get( %u)\n", data);
    }
    {
        uint32_t good_lvl1_pkts=gtmv(m, 32);
        if(!err) ag_drv_qm_good_lvl1_pkts_cnt_get( &good_lvl1_pkts);
        if(!err) bdmf_session_print(session, "ag_drv_qm_good_lvl1_pkts_cnt_get( %u)\n", good_lvl1_pkts);
    }
    {
        uint32_t good_lvl1_bytes=gtmv(m, 32);
        if(!err) ag_drv_qm_good_lvl1_bytes_cnt_get( &good_lvl1_bytes);
        if(!err) bdmf_session_print(session, "ag_drv_qm_good_lvl1_bytes_cnt_get( %u)\n", good_lvl1_bytes);
    }
    {
        uint32_t good_lvl2_pkts=gtmv(m, 32);
        if(!err) ag_drv_qm_good_lvl2_pkts_cnt_get( &good_lvl2_pkts);
        if(!err) bdmf_session_print(session, "ag_drv_qm_good_lvl2_pkts_cnt_get( %u)\n", good_lvl2_pkts);
    }
    {
        uint32_t good_lvl2_bytes=gtmv(m, 32);
        if(!err) ag_drv_qm_good_lvl2_bytes_cnt_get( &good_lvl2_bytes);
        if(!err) bdmf_session_print(session, "ag_drv_qm_good_lvl2_bytes_cnt_get( %u)\n", good_lvl2_bytes);
    }
    {
        uint32_t copied_pkts=gtmv(m, 32);
        if(!err) ag_drv_qm_copied_pkts_cnt_get( &copied_pkts);
        if(!err) bdmf_session_print(session, "ag_drv_qm_copied_pkts_cnt_get( %u)\n", copied_pkts);
    }
    {
        uint32_t copied_bytes=gtmv(m, 32);
        if(!err) ag_drv_qm_copied_bytes_cnt_get( &copied_bytes);
        if(!err) bdmf_session_print(session, "ag_drv_qm_copied_bytes_cnt_get( %u)\n", copied_bytes);
    }
    {
        uint32_t agg_pkts=gtmv(m, 32);
        if(!err) ag_drv_qm_agg_pkts_cnt_get( &agg_pkts);
        if(!err) bdmf_session_print(session, "ag_drv_qm_agg_pkts_cnt_get( %u)\n", agg_pkts);
    }
    {
        uint32_t agg_bytes=gtmv(m, 32);
        if(!err) ag_drv_qm_agg_bytes_cnt_get( &agg_bytes);
        if(!err) bdmf_session_print(session, "ag_drv_qm_agg_bytes_cnt_get( %u)\n", agg_bytes);
    }
    {
        uint32_t agg1_pkts=gtmv(m, 32);
        if(!err) ag_drv_qm_agg_1_pkts_cnt_get( &agg1_pkts);
        if(!err) bdmf_session_print(session, "ag_drv_qm_agg_1_pkts_cnt_get( %u)\n", agg1_pkts);
    }
    {
        uint32_t agg2_pkts=gtmv(m, 32);
        if(!err) ag_drv_qm_agg_2_pkts_cnt_get( &agg2_pkts);
        if(!err) bdmf_session_print(session, "ag_drv_qm_agg_2_pkts_cnt_get( %u)\n", agg2_pkts);
    }
    {
        uint32_t agg3_pkts=gtmv(m, 32);
        if(!err) ag_drv_qm_agg_3_pkts_cnt_get( &agg3_pkts);
        if(!err) bdmf_session_print(session, "ag_drv_qm_agg_3_pkts_cnt_get( %u)\n", agg3_pkts);
    }
    {
        uint32_t agg4_pkts=gtmv(m, 32);
        if(!err) ag_drv_qm_agg_4_pkts_cnt_get( &agg4_pkts);
        if(!err) bdmf_session_print(session, "ag_drv_qm_agg_4_pkts_cnt_get( %u)\n", agg4_pkts);
    }
    {
        uint32_t wred_drop=gtmv(m, 32);
        if(!err) ag_drv_qm_wred_drop_cnt_get( &wred_drop);
        if(!err) bdmf_session_print(session, "ag_drv_qm_wred_drop_cnt_get( %u)\n", wred_drop);
    }
    {
        uint32_t fpm_cong=gtmv(m, 32);
        if(!err) ag_drv_qm_fpm_congestion_drop_cnt_get( &fpm_cong);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_congestion_drop_cnt_get( %u)\n", fpm_cong);
    }
    {
        uint32_t ddr_pd_cong_drop=gtmv(m, 32);
        if(!err) ag_drv_qm_ddr_pd_congestion_drop_cnt_get( &ddr_pd_cong_drop);
        if(!err) bdmf_session_print(session, "ag_drv_qm_ddr_pd_congestion_drop_cnt_get( %u)\n", ddr_pd_cong_drop);
    }
    {
        uint32_t ddr_cong_byte_drop=gtmv(m, 32);
        if(!err) ag_drv_qm_ddr_byte_congestion_drop_cnt_get( &ddr_cong_byte_drop);
        if(!err) bdmf_session_print(session, "ag_drv_qm_ddr_byte_congestion_drop_cnt_get( %u)\n", ddr_cong_byte_drop);
    }
    {
        uint32_t pd_cong_drop=gtmv(m, 32);
        if(!err) ag_drv_qm_qm_pd_congestion_drop_cnt_get( &pd_cong_drop);
        if(!err) bdmf_session_print(session, "ag_drv_qm_qm_pd_congestion_drop_cnt_get( %u)\n", pd_cong_drop);
    }
    {
        uint32_t abs_requeue=gtmv(m, 32);
        if(!err) ag_drv_qm_qm_abs_requeue_cnt_get( &abs_requeue);
        if(!err) bdmf_session_print(session, "ag_drv_qm_qm_abs_requeue_cnt_get( %u)\n", abs_requeue);
    }
    {
        uint16_t used_words=gtmv(m, 16);
        bdmf_boolean empty=gtmv(m, 1);
        bdmf_boolean full=gtmv(m, 1);
        if(!err) ag_drv_qm_fpm_prefetch_fifo0_status_get( &used_words, &empty, &full);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_prefetch_fifo0_status_get( %u %u %u)\n", used_words, empty, full);
    }
    {
        uint16_t used_words=gtmv(m, 16);
        bdmf_boolean empty=gtmv(m, 1);
        bdmf_boolean full=gtmv(m, 1);
        if(!err) ag_drv_qm_fpm_prefetch_fifo1_status_get( &used_words, &empty, &full);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_prefetch_fifo1_status_get( %u %u %u)\n", used_words, empty, full);
    }
    {
        uint16_t used_words=gtmv(m, 16);
        bdmf_boolean empty=gtmv(m, 1);
        bdmf_boolean full=gtmv(m, 1);
        if(!err) ag_drv_qm_fpm_prefetch_fifo2_status_get( &used_words, &empty, &full);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_prefetch_fifo2_status_get( %u %u %u)\n", used_words, empty, full);
    }
    {
        uint16_t used_words=gtmv(m, 16);
        bdmf_boolean empty=gtmv(m, 1);
        bdmf_boolean full=gtmv(m, 1);
        if(!err) ag_drv_qm_fpm_prefetch_fifo3_status_get( &used_words, &empty, &full);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_prefetch_fifo3_status_get( %u %u %u)\n", used_words, empty, full);
    }
    {
        uint16_t used_words=gtmv(m, 16);
        bdmf_boolean empty=gtmv(m, 1);
        bdmf_boolean full=gtmv(m, 1);
        if(!err) ag_drv_qm_normal_rmt_fifo_status_get( &used_words, &empty, &full);
        if(!err) bdmf_session_print(session, "ag_drv_qm_normal_rmt_fifo_status_get( %u %u %u)\n", used_words, empty, full);
    }
    {
        uint16_t used_words=gtmv(m, 16);
        bdmf_boolean empty=gtmv(m, 1);
        bdmf_boolean full=gtmv(m, 1);
        if(!err) ag_drv_qm_non_delayed_rmt_fifo_status_get( &used_words, &empty, &full);
        if(!err) bdmf_session_print(session, "ag_drv_qm_non_delayed_rmt_fifo_status_get( %u %u %u)\n", used_words, empty, full);
    }
    {
        uint16_t used_words=gtmv(m, 16);
        bdmf_boolean empty=gtmv(m, 1);
        bdmf_boolean full=gtmv(m, 1);
        if(!err) ag_drv_qm_non_delayed_out_fifo_status_get( &used_words, &empty, &full);
        if(!err) bdmf_session_print(session, "ag_drv_qm_non_delayed_out_fifo_status_get( %u %u %u)\n", used_words, empty, full);
    }
    {
        uint16_t used_words=gtmv(m, 16);
        bdmf_boolean empty=gtmv(m, 1);
        bdmf_boolean full=gtmv(m, 1);
        if(!err) ag_drv_qm_pre_cm_fifo_status_get( &used_words, &empty, &full);
        if(!err) bdmf_session_print(session, "ag_drv_qm_pre_cm_fifo_status_get( %u %u %u)\n", used_words, empty, full);
    }
    {
        uint16_t used_words=gtmv(m, 16);
        bdmf_boolean empty=gtmv(m, 1);
        bdmf_boolean full=gtmv(m, 1);
        if(!err) ag_drv_qm_cm_rd_pd_fifo_status_get( &used_words, &empty, &full);
        if(!err) bdmf_session_print(session, "ag_drv_qm_cm_rd_pd_fifo_status_get( %u %u %u)\n", used_words, empty, full);
    }
    {
        uint16_t used_words=gtmv(m, 16);
        bdmf_boolean empty=gtmv(m, 1);
        bdmf_boolean full=gtmv(m, 1);
        if(!err) ag_drv_qm_cm_wr_pd_fifo_status_get( &used_words, &empty, &full);
        if(!err) bdmf_session_print(session, "ag_drv_qm_cm_wr_pd_fifo_status_get( %u %u %u)\n", used_words, empty, full);
    }
    {
        uint16_t used_words=gtmv(m, 16);
        bdmf_boolean empty=gtmv(m, 1);
        bdmf_boolean full=gtmv(m, 1);
        if(!err) ag_drv_qm_cm_common_input_fifo_status_get( &used_words, &empty, &full);
        if(!err) bdmf_session_print(session, "ag_drv_qm_cm_common_input_fifo_status_get( %u %u %u)\n", used_words, empty, full);
    }
    {
        uint16_t used_words=gtmv(m, 16);
        bdmf_boolean empty=gtmv(m, 1);
        bdmf_boolean full=gtmv(m, 1);
        if(!err) ag_drv_qm_bb0_output_fifo_status_get( &used_words, &empty, &full);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bb0_output_fifo_status_get( %u %u %u)\n", used_words, empty, full);
    }
    {
        uint16_t used_words=gtmv(m, 16);
        bdmf_boolean empty=gtmv(m, 1);
        bdmf_boolean full=gtmv(m, 1);
        if(!err) ag_drv_qm_bb1_output_fifo_status_get( &used_words, &empty, &full);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bb1_output_fifo_status_get( %u %u %u)\n", used_words, empty, full);
    }
    {
        uint16_t used_words=gtmv(m, 16);
        bdmf_boolean empty=gtmv(m, 1);
        bdmf_boolean full=gtmv(m, 1);
        if(!err) ag_drv_qm_bb1_input_fifo_status_get( &used_words, &empty, &full);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bb1_input_fifo_status_get( %u %u %u)\n", used_words, empty, full);
    }
    {
        uint16_t used_words=gtmv(m, 16);
        bdmf_boolean empty=gtmv(m, 1);
        bdmf_boolean full=gtmv(m, 1);
        if(!err) ag_drv_qm_egress_data_fifo_status_get( &used_words, &empty, &full);
        if(!err) bdmf_session_print(session, "ag_drv_qm_egress_data_fifo_status_get( %u %u %u)\n", used_words, empty, full);
    }
    {
        uint16_t used_words=gtmv(m, 16);
        bdmf_boolean empty=gtmv(m, 1);
        bdmf_boolean full=gtmv(m, 1);
        if(!err) ag_drv_qm_egress_rr_fifo_status_get( &used_words, &empty, &full);
        if(!err) bdmf_session_print(session, "ag_drv_qm_egress_rr_fifo_status_get( %u %u %u)\n", used_words, empty, full);
    }
    {
        uint8_t idx=gtmv(m, 1);
        bdmf_boolean ovr_en=gtmv(m, 1);
        uint8_t dest_id=gtmv(m, 6);
        uint16_t route_addr=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bb_route_ovr_set( %u %u %u %u)\n", idx, ovr_en, dest_id, route_addr);
        if(!err) ag_drv_qm_bb_route_ovr_set(idx, ovr_en, dest_id, route_addr);
        if(!err) ag_drv_qm_bb_route_ovr_get( idx, &ovr_en, &dest_id, &route_addr);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bb_route_ovr_get( %u %u %u %u)\n", idx, ovr_en, dest_id, route_addr);
        if(err || ovr_en!=gtmv(m, 1) || dest_id!=gtmv(m, 6) || route_addr!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t ingress_stat=gtmv(m, 32);
        if(!err) ag_drv_qm_ingress_stat_get( &ingress_stat);
        if(!err) bdmf_session_print(session, "ag_drv_qm_ingress_stat_get( %u)\n", ingress_stat);
    }
    {
        uint32_t egress_stat=gtmv(m, 32);
        if(!err) ag_drv_qm_egress_stat_get( &egress_stat);
        if(!err) bdmf_session_print(session, "ag_drv_qm_egress_stat_get( %u)\n", egress_stat);
    }
    {
        uint32_t cm_stat=gtmv(m, 32);
        if(!err) ag_drv_qm_cm_stat_get( &cm_stat);
        if(!err) bdmf_session_print(session, "ag_drv_qm_cm_stat_get( %u)\n", cm_stat);
    }
    {
        uint32_t fpm_prefetch_stat=gtmv(m, 32);
        if(!err) ag_drv_qm_fpm_prefetch_stat_get( &fpm_prefetch_stat);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_prefetch_stat_get( %u)\n", fpm_prefetch_stat);
    }
    {
        uint8_t connect_ack_counter=gtmv(m, 8);
        if(!err) ag_drv_qm_qm_connect_ack_counter_get( &connect_ack_counter);
        if(!err) bdmf_session_print(session, "ag_drv_qm_qm_connect_ack_counter_get( %u)\n", connect_ack_counter);
    }
    {
        uint8_t ddr_wr_reply_counter=gtmv(m, 8);
        if(!err) ag_drv_qm_qm_ddr_wr_reply_counter_get( &ddr_wr_reply_counter);
        if(!err) bdmf_session_print(session, "ag_drv_qm_qm_ddr_wr_reply_counter_get( %u)\n", ddr_wr_reply_counter);
    }
    {
        uint32_t ddr_pipe=gtmv(m, 28);
        if(!err) ag_drv_qm_qm_ddr_pipe_byte_counter_get( &ddr_pipe);
        if(!err) bdmf_session_print(session, "ag_drv_qm_qm_ddr_pipe_byte_counter_get( %u)\n", ddr_pipe);
    }
    {
        uint16_t requeue_valid=gtmv(m, 15);
        if(!err) ag_drv_qm_qm_abs_requeue_valid_counter_get( &requeue_valid);
        if(!err) bdmf_session_print(session, "ag_drv_qm_qm_abs_requeue_valid_counter_get( %u)\n", requeue_valid);
    }
    {
        uint32_t idx=gtmv(m, 2);
        uint32_t pd=gtmv(m, 32);
        if(!err) ag_drv_qm_qm_illegal_pd_capture_get( idx, &pd);
        if(!err) bdmf_session_print(session, "ag_drv_qm_qm_illegal_pd_capture_get( %u %u)\n", idx, pd);
    }
    {
        uint32_t idx=gtmv(m, 2);
        uint32_t pd=gtmv(m, 32);
        if(!err) ag_drv_qm_qm_ingress_processed_pd_capture_get( idx, &pd);
        if(!err) bdmf_session_print(session, "ag_drv_qm_qm_ingress_processed_pd_capture_get( %u %u)\n", idx, pd);
    }
    {
        uint32_t idx=gtmv(m, 2);
        uint32_t pd=gtmv(m, 32);
        if(!err) ag_drv_qm_qm_cm_processed_pd_capture_get( idx, &pd);
        if(!err) bdmf_session_print(session, "ag_drv_qm_qm_cm_processed_pd_capture_get( %u %u)\n", idx, pd);
    }
    {
        uint32_t idx=gtmv(m, 2);
        uint32_t fpm_drop=gtmv(m, 32);
        if(!err) ag_drv_qm_fpm_pool_drop_cnt_get( idx, &fpm_drop);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_pool_drop_cnt_get( %u %u)\n", idx, fpm_drop);
    }
    {
        uint32_t idx=gtmv(m, 2);
        uint32_t fpm_grp_drop=gtmv(m, 32);
        if(!err) ag_drv_qm_fpm_grp_drop_cnt_get( idx, &fpm_grp_drop);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_grp_drop_cnt_get( %u %u)\n", idx, fpm_grp_drop);
    }
    {
        uint32_t counter=gtmv(m, 32);
        if(!err) ag_drv_qm_fpm_buffer_res_drop_cnt_get( &counter);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_buffer_res_drop_cnt_get( %u)\n", counter);
    }
    {
        uint32_t counter=gtmv(m, 32);
        if(!err) ag_drv_qm_psram_egress_cong_drp_cnt_get( &counter);
        if(!err) bdmf_session_print(session, "ag_drv_qm_psram_egress_cong_drp_cnt_get( %u)\n", counter);
    }
    {
        uint16_t idx=gtmv(m, 9);
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_qm_cm_residue_data_get( idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_cm_residue_data_get( %u %u)\n", idx, data);
    }
    {
        uint8_t type=gtmv(m, 3);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_mactype_set( %u)\n", type);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_mactype_set(type);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_mactype_get( &type);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_mactype_get( %u)\n", type);
        if(err || type!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t rnr_cfg_index_1=gtmv(m, 1);
        uint16_t tcontaddr=gtmv(m, 16);
        uint16_t skbaddr=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1_set( %u %u %u)\n", rnr_cfg_index_1, tcontaddr, skbaddr);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1_set(rnr_cfg_index_1, tcontaddr, skbaddr);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1_get( rnr_cfg_index_1, &tcontaddr, &skbaddr);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1_get( %u %u %u)\n", rnr_cfg_index_1, tcontaddr, skbaddr);
        if(err || tcontaddr!=gtmv(m, 16) || skbaddr!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t rnr_cfg_index_2=gtmv(m, 1);
        uint16_t ptraddr=gtmv(m, 16);
        uint8_t task=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2_set( %u %u %u)\n", rnr_cfg_index_2, ptraddr, task);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2_set(rnr_cfg_index_2, ptraddr, task);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2_get( rnr_cfg_index_2, &ptraddr, &task);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2_get( %u %u %u)\n", rnr_cfg_index_2, ptraddr, task);
        if(err || ptraddr!=gtmv(m, 16) || task!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean freenocntxt=gtmv(m, 1);
        bdmf_boolean specialfree=gtmv(m, 1);
        uint8_t maxgn=gtmv(m, 5);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg_set( %u %u %u)\n", freenocntxt, specialfree, maxgn);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg_set(freenocntxt, specialfree, maxgn);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg_get( &freenocntxt, &specialfree, &maxgn);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg_get( %u %u %u)\n", freenocntxt, specialfree, maxgn);
        if(err || freenocntxt!=gtmv(m, 1) || specialfree!=gtmv(m, 1) || maxgn!=gtmv(m, 5))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t zero=gtmv(m, 0);
        qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel = {.addr={gtmv(m, 32), gtmv(m, 32)}};
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel_set( %u %u %u)\n", zero, bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel.addr[0], bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel.addr[1]);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel_set(zero, &bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel_get( zero, &bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel_get( %u %u %u)\n", zero, bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel.addr[0], bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel.addr[1]);
        if(err || bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel.addr[0]!=gtmv(m, 32) || bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel.addr[1]!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t zero=gtmv(m, 0);
        qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh = {.addr={gtmv(m, 32), gtmv(m, 32)}};
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh_set( %u %u %u)\n", zero, bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh.addr[0], bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh.addr[1]);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh_set(zero, &bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh_get( zero, &bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh_get( %u %u %u)\n", zero, bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh.addr[0], bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh.addr[1]);
        if(err || bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh.addr[0]!=gtmv(m, 32) || bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh.addr[1]!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t psramsize=gtmv(m, 10);
        uint16_t ddrsize=gtmv(m, 10);
        uint16_t psrambase=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl_set( %u %u %u)\n", psramsize, ddrsize, psrambase);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl_set(psramsize, ddrsize, psrambase);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl_get( &psramsize, &ddrsize, &psrambase);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl_get( %u %u %u)\n", psramsize, ddrsize, psrambase);
        if(err || psramsize!=gtmv(m, 10) || ddrsize!=gtmv(m, 10) || psrambase!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean hightrxq=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_arb_cfg_set( %u)\n", hightrxq);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_arb_cfg_set(hightrxq);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_arb_cfg_get( &hightrxq);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_arb_cfg_get( %u)\n", hightrxq);
        if(err || hightrxq!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t route=gtmv(m, 10);
        uint8_t dest=gtmv(m, 6);
        bdmf_boolean en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_bbroute_set( %u %u %u)\n", route, dest, en);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_bbroute_set(route, dest, en);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_bbroute_get( &route, &dest, &en);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_bbroute_get( %u %u %u)\n", route, dest, en);
        if(err || route!=gtmv(m, 10) || dest!=gtmv(m, 6) || en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t q_2_rnr_index=gtmv(m, 2);
        bdmf_boolean q0=gtmv(m, 1);
        bdmf_boolean q1=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_q2rnr_set( %u %u %u)\n", q_2_rnr_index, q0, q1);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_q2rnr_set(q_2_rnr_index, q0, q1);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_q2rnr_get( q_2_rnr_index, &q0, &q1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_q2rnr_get( %u %u %u)\n", q_2_rnr_index, q0, q1);
        if(err || q0!=gtmv(m, 1) || q1!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_bbh_tx_qm_bbhtx_common_configurations_perqtask bbh_tx_qm_bbhtx_common_configurations_perqtask = {.task0=gtmv(m, 4), .task1=gtmv(m, 4), .task2=gtmv(m, 4), .task3=gtmv(m, 4), .task4=gtmv(m, 4), .task5=gtmv(m, 4), .task6=gtmv(m, 4), .task7=gtmv(m, 4)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_perqtask_set( %u %u %u %u %u %u %u %u)\n", bbh_tx_qm_bbhtx_common_configurations_perqtask.task0, bbh_tx_qm_bbhtx_common_configurations_perqtask.task1, bbh_tx_qm_bbhtx_common_configurations_perqtask.task2, bbh_tx_qm_bbhtx_common_configurations_perqtask.task3, bbh_tx_qm_bbhtx_common_configurations_perqtask.task4, bbh_tx_qm_bbhtx_common_configurations_perqtask.task5, bbh_tx_qm_bbhtx_common_configurations_perqtask.task6, bbh_tx_qm_bbhtx_common_configurations_perqtask.task7);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_perqtask_set(&bbh_tx_qm_bbhtx_common_configurations_perqtask);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_perqtask_get( &bbh_tx_qm_bbhtx_common_configurations_perqtask);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_perqtask_get( %u %u %u %u %u %u %u %u)\n", bbh_tx_qm_bbhtx_common_configurations_perqtask.task0, bbh_tx_qm_bbhtx_common_configurations_perqtask.task1, bbh_tx_qm_bbhtx_common_configurations_perqtask.task2, bbh_tx_qm_bbhtx_common_configurations_perqtask.task3, bbh_tx_qm_bbhtx_common_configurations_perqtask.task4, bbh_tx_qm_bbhtx_common_configurations_perqtask.task5, bbh_tx_qm_bbhtx_common_configurations_perqtask.task6, bbh_tx_qm_bbhtx_common_configurations_perqtask.task7);
        if(err || bbh_tx_qm_bbhtx_common_configurations_perqtask.task0!=gtmv(m, 4) || bbh_tx_qm_bbhtx_common_configurations_perqtask.task1!=gtmv(m, 4) || bbh_tx_qm_bbhtx_common_configurations_perqtask.task2!=gtmv(m, 4) || bbh_tx_qm_bbhtx_common_configurations_perqtask.task3!=gtmv(m, 4) || bbh_tx_qm_bbhtx_common_configurations_perqtask.task4!=gtmv(m, 4) || bbh_tx_qm_bbhtx_common_configurations_perqtask.task5!=gtmv(m, 4) || bbh_tx_qm_bbhtx_common_configurations_perqtask.task6!=gtmv(m, 4) || bbh_tx_qm_bbhtx_common_configurations_perqtask.task7!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd bbh_tx_qm_bbhtx_common_configurations_txrstcmd = {.cntxtrst=gtmv(m, 1), .pdfiforst=gtmv(m, 1), .dmaptrrst=gtmv(m, 1), .sdmaptrrst=gtmv(m, 1), .bpmfiforst=gtmv(m, 1), .sbpmfiforst=gtmv(m, 1), .okfiforst=gtmv(m, 1), .ddrfiforst=gtmv(m, 1), .sramfiforst=gtmv(m, 1), .skbptrrst=gtmv(m, 1), .stsfiforst=gtmv(m, 1), .reqfiforst=gtmv(m, 1), .msgfiforst=gtmv(m, 1), .gnxtfiforst=gtmv(m, 1), .fbnfiforst=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.cntxtrst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.pdfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.dmaptrrst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.sdmaptrrst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.bpmfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.sbpmfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.okfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.ddrfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.sramfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.skbptrrst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.stsfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.reqfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.msgfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.gnxtfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.fbnfiforst);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd_set(&bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd_get( &bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", bbh_tx_qm_bbhtx_common_configurations_txrstcmd.cntxtrst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.pdfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.dmaptrrst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.sdmaptrrst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.bpmfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.sbpmfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.okfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.ddrfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.sramfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.skbptrrst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.stsfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.reqfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.msgfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.gnxtfiforst, bbh_tx_qm_bbhtx_common_configurations_txrstcmd.fbnfiforst);
        if(err || bbh_tx_qm_bbhtx_common_configurations_txrstcmd.cntxtrst!=gtmv(m, 1) || bbh_tx_qm_bbhtx_common_configurations_txrstcmd.pdfiforst!=gtmv(m, 1) || bbh_tx_qm_bbhtx_common_configurations_txrstcmd.dmaptrrst!=gtmv(m, 1) || bbh_tx_qm_bbhtx_common_configurations_txrstcmd.sdmaptrrst!=gtmv(m, 1) || bbh_tx_qm_bbhtx_common_configurations_txrstcmd.bpmfiforst!=gtmv(m, 1) || bbh_tx_qm_bbhtx_common_configurations_txrstcmd.sbpmfiforst!=gtmv(m, 1) || bbh_tx_qm_bbhtx_common_configurations_txrstcmd.okfiforst!=gtmv(m, 1) || bbh_tx_qm_bbhtx_common_configurations_txrstcmd.ddrfiforst!=gtmv(m, 1) || bbh_tx_qm_bbhtx_common_configurations_txrstcmd.sramfiforst!=gtmv(m, 1) || bbh_tx_qm_bbhtx_common_configurations_txrstcmd.skbptrrst!=gtmv(m, 1) || bbh_tx_qm_bbhtx_common_configurations_txrstcmd.stsfiforst!=gtmv(m, 1) || bbh_tx_qm_bbhtx_common_configurations_txrstcmd.reqfiforst!=gtmv(m, 1) || bbh_tx_qm_bbhtx_common_configurations_txrstcmd.msgfiforst!=gtmv(m, 1) || bbh_tx_qm_bbhtx_common_configurations_txrstcmd.gnxtfiforst!=gtmv(m, 1) || bbh_tx_qm_bbhtx_common_configurations_txrstcmd.fbnfiforst!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t dbgsel=gtmv(m, 5);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dbgsel_set( %u)\n", dbgsel);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dbgsel_set(dbgsel);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dbgsel_get( &dbgsel);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dbgsel_get( %u)\n", dbgsel);
        if(err || dbgsel!=gtmv(m, 5))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl = {.bypass_clk_gate=gtmv(m, 1), .timer_val=gtmv(m, 8), .keep_alive_en=gtmv(m, 1), .keep_alive_intrvl=gtmv(m, 3), .keep_alive_cyc=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl_set( %u %u %u %u %u)\n", bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.bypass_clk_gate, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.timer_val, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.keep_alive_en, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.keep_alive_intrvl, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.keep_alive_cyc);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl_set(&bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl_get( &bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl_get( %u %u %u %u %u)\n", bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.bypass_clk_gate, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.timer_val, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.keep_alive_en, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.keep_alive_intrvl, bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.keep_alive_cyc);
        if(err || bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.bypass_clk_gate!=gtmv(m, 1) || bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.timer_val!=gtmv(m, 8) || bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.keep_alive_en!=gtmv(m, 1) || bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.keep_alive_intrvl!=gtmv(m, 3) || bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl.keep_alive_cyc!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gpr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_gpr_set( %u)\n", gpr);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_gpr_set(gpr);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_gpr_get( &gpr);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_gpr_get( %u)\n", gpr);
        if(err || gpr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t fifobase0=gtmv(m, 9);
        uint16_t fifobase1=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdbase_set( %u %u)\n", fifobase0, fifobase1);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdbase_set(fifobase0, fifobase1);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdbase_get( &fifobase0, &fifobase1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdbase_get( %u %u)\n", fifobase0, fifobase1);
        if(err || fifobase0!=gtmv(m, 9) || fifobase1!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t fifosize0=gtmv(m, 9);
        uint16_t fifosize1=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdsize_set( %u %u)\n", fifosize0, fifosize1);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdsize_set(fifosize0, fifosize1);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdsize_get( &fifosize0, &fifosize1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdsize_get( %u %u)\n", fifosize0, fifosize1);
        if(err || fifosize0!=gtmv(m, 9) || fifosize1!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t wkupthresh0=gtmv(m, 8);
        uint8_t wkupthresh1=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph_set( %u %u)\n", wkupthresh0, wkupthresh1);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph_set(wkupthresh0, wkupthresh1);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph_get( &wkupthresh0, &wkupthresh1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph_get( %u %u)\n", wkupthresh0, wkupthresh1);
        if(err || wkupthresh0!=gtmv(m, 8) || wkupthresh1!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t pdlimit0=gtmv(m, 16);
        uint16_t pdlimit1=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_set( %u %u)\n", pdlimit0, pdlimit1);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_set(pdlimit0, pdlimit1);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_get( &pdlimit0, &pdlimit1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_get( %u %u)\n", pdlimit0, pdlimit1);
        if(err || pdlimit0!=gtmv(m, 16) || pdlimit1!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean pdlimiten=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en_set( %u)\n", pdlimiten);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en_set(pdlimiten);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en_get( &pdlimiten);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en_get( %u)\n", pdlimiten);
        if(err || pdlimiten!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t empty=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdempty_set( %u)\n", empty);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdempty_set(empty);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdempty_get( &empty);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdempty_get( %u)\n", empty);
        if(err || empty!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t ddrthresh=gtmv(m, 9);
        uint16_t sramthresh=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_txthresh_set( %u %u)\n", ddrthresh, sramthresh);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_txthresh_set(ddrthresh, sramthresh);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_txthresh_get( &ddrthresh, &sramthresh);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_txthresh_get( %u %u)\n", ddrthresh, sramthresh);
        if(err || ddrthresh!=gtmv(m, 9) || sramthresh!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_eee_set( %u)\n", en);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_eee_set(en);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_eee_get( &en);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_eee_get( %u)\n", en);
        if(err || en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_ts_set( %u)\n", en);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_ts_set(en);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_ts_get( &en);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_ts_get( %u)\n", en);
        if(err || en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t srambyte=gtmv(m, 32);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_srambyte_get( &srambyte);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_srambyte_get( %u)\n", srambyte);
    }
    {
        uint32_t ddrbyte=gtmv(m, 32);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_ddrbyte_get( &ddrbyte);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_ddrbyte_get( %u)\n", ddrbyte);
    }
    {
        qm_bbh_tx_qm_bbhtx_debug_counters_swrden bbh_tx_qm_bbhtx_debug_counters_swrden = {.pdsel=gtmv(m, 1), .pdvsel=gtmv(m, 1), .pdemptysel=gtmv(m, 1), .pdfullsel=gtmv(m, 1), .pdbemptysel=gtmv(m, 1), .pdffwkpsel=gtmv(m, 1), .fbnsel=gtmv(m, 1), .fbnvsel=gtmv(m, 1), .fbnemptysel=gtmv(m, 1), .fbnfullsel=gtmv(m, 1), .getnextsel=gtmv(m, 1), .getnextvsel=gtmv(m, 1), .getnextemptysel=gtmv(m, 1), .getnextfullsel=gtmv(m, 1), .gpncntxtsel=gtmv(m, 1), .bpmsel=gtmv(m, 1), .bpmfsel=gtmv(m, 1), .sbpmsel=gtmv(m, 1), .sbpmfsel=gtmv(m, 1), .stssel=gtmv(m, 1), .stsvsel=gtmv(m, 1), .stsemptysel=gtmv(m, 1), .stsfullsel=gtmv(m, 1), .stsbemptysel=gtmv(m, 1), .stsffwkpsel=gtmv(m, 1), .msgsel=gtmv(m, 1), .msgvsel=gtmv(m, 1), .epnreqsel=gtmv(m, 1), .datasel=gtmv(m, 1), .reordersel=gtmv(m, 1), .tsinfosel=gtmv(m, 1), .mactxsel=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrden_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.pdsel, bbh_tx_qm_bbhtx_debug_counters_swrden.pdvsel, bbh_tx_qm_bbhtx_debug_counters_swrden.pdemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.pdfullsel, bbh_tx_qm_bbhtx_debug_counters_swrden.pdbemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.pdffwkpsel, bbh_tx_qm_bbhtx_debug_counters_swrden.fbnsel, bbh_tx_qm_bbhtx_debug_counters_swrden.fbnvsel, bbh_tx_qm_bbhtx_debug_counters_swrden.fbnemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.fbnfullsel, bbh_tx_qm_bbhtx_debug_counters_swrden.getnextsel, bbh_tx_qm_bbhtx_debug_counters_swrden.getnextvsel, bbh_tx_qm_bbhtx_debug_counters_swrden.getnextemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.getnextfullsel, bbh_tx_qm_bbhtx_debug_counters_swrden.gpncntxtsel, bbh_tx_qm_bbhtx_debug_counters_swrden.bpmsel, bbh_tx_qm_bbhtx_debug_counters_swrden.bpmfsel, bbh_tx_qm_bbhtx_debug_counters_swrden.sbpmsel, bbh_tx_qm_bbhtx_debug_counters_swrden.sbpmfsel, bbh_tx_qm_bbhtx_debug_counters_swrden.stssel, bbh_tx_qm_bbhtx_debug_counters_swrden.stsvsel, bbh_tx_qm_bbhtx_debug_counters_swrden.stsemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.stsfullsel, bbh_tx_qm_bbhtx_debug_counters_swrden.stsbemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.stsffwkpsel, bbh_tx_qm_bbhtx_debug_counters_swrden.msgsel, bbh_tx_qm_bbhtx_debug_counters_swrden.msgvsel, bbh_tx_qm_bbhtx_debug_counters_swrden.epnreqsel, bbh_tx_qm_bbhtx_debug_counters_swrden.datasel, bbh_tx_qm_bbhtx_debug_counters_swrden.reordersel, bbh_tx_qm_bbhtx_debug_counters_swrden.tsinfosel, bbh_tx_qm_bbhtx_debug_counters_swrden.mactxsel);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrden_set(&bbh_tx_qm_bbhtx_debug_counters_swrden);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrden_get( &bbh_tx_qm_bbhtx_debug_counters_swrden);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrden_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", bbh_tx_qm_bbhtx_debug_counters_swrden.pdsel, bbh_tx_qm_bbhtx_debug_counters_swrden.pdvsel, bbh_tx_qm_bbhtx_debug_counters_swrden.pdemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.pdfullsel, bbh_tx_qm_bbhtx_debug_counters_swrden.pdbemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.pdffwkpsel, bbh_tx_qm_bbhtx_debug_counters_swrden.fbnsel, bbh_tx_qm_bbhtx_debug_counters_swrden.fbnvsel, bbh_tx_qm_bbhtx_debug_counters_swrden.fbnemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.fbnfullsel, bbh_tx_qm_bbhtx_debug_counters_swrden.getnextsel, bbh_tx_qm_bbhtx_debug_counters_swrden.getnextvsel, bbh_tx_qm_bbhtx_debug_counters_swrden.getnextemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.getnextfullsel, bbh_tx_qm_bbhtx_debug_counters_swrden.gpncntxtsel, bbh_tx_qm_bbhtx_debug_counters_swrden.bpmsel, bbh_tx_qm_bbhtx_debug_counters_swrden.bpmfsel, bbh_tx_qm_bbhtx_debug_counters_swrden.sbpmsel, bbh_tx_qm_bbhtx_debug_counters_swrden.sbpmfsel, bbh_tx_qm_bbhtx_debug_counters_swrden.stssel, bbh_tx_qm_bbhtx_debug_counters_swrden.stsvsel, bbh_tx_qm_bbhtx_debug_counters_swrden.stsemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.stsfullsel, bbh_tx_qm_bbhtx_debug_counters_swrden.stsbemptysel, bbh_tx_qm_bbhtx_debug_counters_swrden.stsffwkpsel, bbh_tx_qm_bbhtx_debug_counters_swrden.msgsel, bbh_tx_qm_bbhtx_debug_counters_swrden.msgvsel, bbh_tx_qm_bbhtx_debug_counters_swrden.epnreqsel, bbh_tx_qm_bbhtx_debug_counters_swrden.datasel, bbh_tx_qm_bbhtx_debug_counters_swrden.reordersel, bbh_tx_qm_bbhtx_debug_counters_swrden.tsinfosel, bbh_tx_qm_bbhtx_debug_counters_swrden.mactxsel);
        if(err || bbh_tx_qm_bbhtx_debug_counters_swrden.pdsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.pdvsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.pdemptysel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.pdfullsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.pdbemptysel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.pdffwkpsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.fbnsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.fbnvsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.fbnemptysel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.fbnfullsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.getnextsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.getnextvsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.getnextemptysel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.getnextfullsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.gpncntxtsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.bpmsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.bpmfsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.sbpmsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.sbpmfsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.stssel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.stsvsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.stsemptysel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.stsfullsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.stsbemptysel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.stsffwkpsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.msgsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.msgvsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.epnreqsel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.datasel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.reordersel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.tsinfosel!=gtmv(m, 1) || bbh_tx_qm_bbhtx_debug_counters_swrden.mactxsel!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t rdaddr=gtmv(m, 11);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrdaddr_set( %u)\n", rdaddr);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrdaddr_set(rdaddr);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrdaddr_get( &rdaddr);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrdaddr_get( %u)\n", rdaddr);
        if(err || rdaddr!=gtmv(m, 11))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrddata_get( &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrddata_get( %u)\n", data);
    }
    {
        uint8_t debug_unified_pkt_ctr_idx=gtmv(m, 3);
        uint32_t ddrbyte=gtmv(m, 32);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_unifiedpkt_get( debug_unified_pkt_ctr_idx, &ddrbyte);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_unifiedpkt_get( %u %u)\n", debug_unified_pkt_ctr_idx, ddrbyte);
    }
    {
        uint8_t debug_unified_byte_ctr_idx=gtmv(m, 3);
        uint32_t ddrbyte=gtmv(m, 32);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_unifiedbyte_get( debug_unified_byte_ctr_idx, &ddrbyte);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_unifiedbyte_get( %u %u)\n", debug_unified_byte_ctr_idx, ddrbyte);
    }
    {
        uint8_t zero=gtmv(m, 2);
        qm_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg bbh_tx_qm_bbhtx_debug_counters_dbgoutreg = {.debug_out_reg={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg_get( zero, &bbh_tx_qm_bbhtx_debug_counters_dbgoutreg);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg_get( %u %u %u %u %u %u %u %u %u)\n", zero, bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[0], bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[1], bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[2], bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[3], bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[4], bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[5], bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[6], bbh_tx_qm_bbhtx_debug_counters_dbgoutreg.debug_out_reg[7]);
    }
    {
        uint8_t debug_counters_in_segmentation_byte_ctr_idx=gtmv(m, 1);
        uint32_t in_segmentation=gtmv(m, 32);
        if(!err) ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_in_segmentation_get( debug_counters_in_segmentation_byte_ctr_idx, &in_segmentation);
        if(!err) bdmf_session_print(session, "ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_in_segmentation_get( %u %u)\n", debug_counters_in_segmentation_byte_ctr_idx, in_segmentation);
    }
    {
        uint8_t dest=gtmv(m, 6);
        uint16_t route=gtmv(m, 10);
        bdmf_boolean ovrd=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_bbrouteovrd_set( %u %u %u)\n", dest, route, ovrd);
        if(!err) ag_drv_qm_dma_qm_dma_config_bbrouteovrd_set(dest, route, ovrd);
        if(!err) ag_drv_qm_dma_qm_dma_config_bbrouteovrd_get( &dest, &route, &ovrd);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_bbrouteovrd_get( %u %u %u)\n", dest, route, ovrd);
        if(err || dest!=gtmv(m, 6) || route!=gtmv(m, 10) || ovrd!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint8_t numofbuff=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_num_of_writes_set( %u %u)\n", emac_index, numofbuff);
        if(!err) ag_drv_qm_dma_qm_dma_config_num_of_writes_set(emac_index, numofbuff);
        if(!err) ag_drv_qm_dma_qm_dma_config_num_of_writes_get( emac_index, &numofbuff);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_num_of_writes_get( %u %u)\n", emac_index, numofbuff);
        if(err || numofbuff!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint8_t rr_num=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_num_of_reads_set( %u %u)\n", emac_index, rr_num);
        if(!err) ag_drv_qm_dma_qm_dma_config_num_of_reads_set(emac_index, rr_num);
        if(!err) ag_drv_qm_dma_qm_dma_config_num_of_reads_get( emac_index, &rr_num);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_num_of_reads_get( %u %u)\n", emac_index, rr_num);
        if(err || rr_num!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint8_t into_u=gtmv(m, 6);
        uint8_t out_of_u=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_u_thresh_set( %u %u %u)\n", emac_index, into_u, out_of_u);
        if(!err) ag_drv_qm_dma_qm_dma_config_u_thresh_set(emac_index, into_u, out_of_u);
        if(!err) ag_drv_qm_dma_qm_dma_config_u_thresh_get( emac_index, &into_u, &out_of_u);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_u_thresh_get( %u %u %u)\n", emac_index, into_u, out_of_u);
        if(err || into_u!=gtmv(m, 6) || out_of_u!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint8_t rxpri=gtmv(m, 4);
        uint8_t txpri=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_pri_set( %u %u %u)\n", emac_index, rxpri, txpri);
        if(!err) ag_drv_qm_dma_qm_dma_config_pri_set(emac_index, rxpri, txpri);
        if(!err) ag_drv_qm_dma_qm_dma_config_pri_get( emac_index, &rxpri, &txpri);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_pri_get( %u %u %u)\n", emac_index, rxpri, txpri);
        if(err || rxpri!=gtmv(m, 4) || txpri!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint8_t rxsource=gtmv(m, 6);
        uint8_t txsource=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_periph_source_set( %u %u %u)\n", emac_index, rxsource, txsource);
        if(!err) ag_drv_qm_dma_qm_dma_config_periph_source_set(emac_index, rxsource, txsource);
        if(!err) ag_drv_qm_dma_qm_dma_config_periph_source_get( emac_index, &rxsource, &txsource);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_periph_source_get( %u %u %u)\n", emac_index, rxsource, txsource);
        if(err || rxsource!=gtmv(m, 6) || txsource!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint8_t rxweight=gtmv(m, 3);
        uint8_t txweight=gtmv(m, 3);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_weight_set( %u %u %u)\n", emac_index, rxweight, txweight);
        if(!err) ag_drv_qm_dma_qm_dma_config_weight_set(emac_index, rxweight, txweight);
        if(!err) ag_drv_qm_dma_qm_dma_config_weight_get( emac_index, &rxweight, &txweight);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_weight_get( %u %u %u)\n", emac_index, rxweight, txweight);
        if(err || rxweight!=gtmv(m, 3) || txweight!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t rstvec=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_ptrrst_set( %u)\n", rstvec);
        if(!err) ag_drv_qm_dma_qm_dma_config_ptrrst_set(rstvec);
        if(!err) ag_drv_qm_dma_qm_dma_config_ptrrst_get( &rstvec);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_ptrrst_get( %u)\n", rstvec);
        if(err || rstvec!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t max=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_max_otf_set( %u)\n", max);
        if(!err) ag_drv_qm_dma_qm_dma_config_max_otf_set(max);
        if(!err) ag_drv_qm_dma_qm_dma_config_max_otf_get( &max);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_max_otf_get( %u)\n", max);
        if(err || max!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_dma_qm_dma_config_clk_gate_cntrl dma_qm_dma_config_clk_gate_cntrl = {.bypass_clk_gate=gtmv(m, 1), .timer_val=gtmv(m, 8), .keep_alive_en=gtmv(m, 1), .keep_alive_intrvl=gtmv(m, 3), .keep_alive_cyc=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_clk_gate_cntrl_set( %u %u %u %u %u)\n", dma_qm_dma_config_clk_gate_cntrl.bypass_clk_gate, dma_qm_dma_config_clk_gate_cntrl.timer_val, dma_qm_dma_config_clk_gate_cntrl.keep_alive_en, dma_qm_dma_config_clk_gate_cntrl.keep_alive_intrvl, dma_qm_dma_config_clk_gate_cntrl.keep_alive_cyc);
        if(!err) ag_drv_qm_dma_qm_dma_config_clk_gate_cntrl_set(&dma_qm_dma_config_clk_gate_cntrl);
        if(!err) ag_drv_qm_dma_qm_dma_config_clk_gate_cntrl_get( &dma_qm_dma_config_clk_gate_cntrl);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_clk_gate_cntrl_get( %u %u %u %u %u)\n", dma_qm_dma_config_clk_gate_cntrl.bypass_clk_gate, dma_qm_dma_config_clk_gate_cntrl.timer_val, dma_qm_dma_config_clk_gate_cntrl.keep_alive_en, dma_qm_dma_config_clk_gate_cntrl.keep_alive_intrvl, dma_qm_dma_config_clk_gate_cntrl.keep_alive_cyc);
        if(err || dma_qm_dma_config_clk_gate_cntrl.bypass_clk_gate!=gtmv(m, 1) || dma_qm_dma_config_clk_gate_cntrl.timer_val!=gtmv(m, 8) || dma_qm_dma_config_clk_gate_cntrl.keep_alive_en!=gtmv(m, 1) || dma_qm_dma_config_clk_gate_cntrl.keep_alive_intrvl!=gtmv(m, 3) || dma_qm_dma_config_clk_gate_cntrl.keep_alive_cyc!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t dbgsel=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_dbg_sel_set( %u)\n", dbgsel);
        if(!err) ag_drv_qm_dma_qm_dma_config_dbg_sel_set(dbgsel);
        if(!err) ag_drv_qm_dma_qm_dma_config_dbg_sel_get( &dbgsel);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_config_dbg_sel_get( %u)\n", dbgsel);
        if(err || dbgsel!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint8_t req_cnt=gtmv(m, 6);
        if(!err) ag_drv_qm_dma_qm_dma_debug_req_cnt_rx_get( emac_index, &req_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_debug_req_cnt_rx_get( %u %u)\n", emac_index, req_cnt);
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint8_t req_cnt=gtmv(m, 6);
        if(!err) ag_drv_qm_dma_qm_dma_debug_req_cnt_tx_get( emac_index, &req_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_debug_req_cnt_tx_get( %u %u)\n", emac_index, req_cnt);
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint32_t req_cnt=gtmv(m, 32);
        if(!err) ag_drv_qm_dma_qm_dma_debug_req_cnt_rx_acc_get( emac_index, &req_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_debug_req_cnt_rx_acc_get( %u %u)\n", emac_index, req_cnt);
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint32_t req_cnt=gtmv(m, 32);
        if(!err) ag_drv_qm_dma_qm_dma_debug_req_cnt_tx_acc_get( emac_index, &req_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_debug_req_cnt_tx_acc_get( %u %u)\n", emac_index, req_cnt);
    }
    {
        uint8_t word_index=gtmv(m, 2);
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_qm_dma_qm_dma_debug_rddata_get( word_index, &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dma_qm_dma_debug_rddata_get( %u %u)\n", word_index, data);
    }
    return err;
}

static int bcm_qm_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t i;
    uint32_t j;
    uint32_t index1_start=0;
    uint32_t index1_stop;
    uint32_t index2_start=0;
    uint32_t index2_stop;
    bdmfmon_cmd_parm_t * bdmf_parm;
    const ru_reg_rec * reg;
    const ru_block_rec * blk;
    const char * enum_string = bdmfmon_enum_parm_stringval(session, 0, parm[0].value.unumber);

    if(!enum_string)
        return BDMF_ERR_INTERNAL;

    switch (parm[0].value.unumber)
    {
    case bdmf_address_global_cfg_qm_enable_ctrl : reg = &RU_REG(QM, GLOBAL_CFG_QM_ENABLE_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_sw_rst_ctrl : reg = &RU_REG(QM, GLOBAL_CFG_QM_SW_RST_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_general_ctrl : reg = &RU_REG(QM, GLOBAL_CFG_QM_GENERAL_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_fpm_control : reg = &RU_REG(QM, GLOBAL_CFG_FPM_CONTROL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_ddr_byte_congestion_control : reg = &RU_REG(QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_ddr_byte_congestion_lower_thr : reg = &RU_REG(QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_ddr_byte_congestion_mid_thr : reg = &RU_REG(QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_ddr_byte_congestion_higher_thr : reg = &RU_REG(QM, GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_ddr_pd_congestion_control : reg = &RU_REG(QM, GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_pd_congestion_control : reg = &RU_REG(QM, GLOBAL_CFG_QM_PD_CONGESTION_CONTROL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_abs_drop_queue : reg = &RU_REG(QM, GLOBAL_CFG_ABS_DROP_QUEUE); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_aggregation_ctrl : reg = &RU_REG(QM, GLOBAL_CFG_AGGREGATION_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_fpm_base_addr : reg = &RU_REG(QM, GLOBAL_CFG_FPM_BASE_ADDR); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_fpm_coherent_base_addr : reg = &RU_REG(QM, GLOBAL_CFG_FPM_COHERENT_BASE_ADDR); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_ddr_sop_offset : reg = &RU_REG(QM, GLOBAL_CFG_DDR_SOP_OFFSET); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_epon_overhead_ctrl : reg = &RU_REG(QM, GLOBAL_CFG_EPON_OVERHEAD_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_dqm_full : reg = &RU_REG(QM, GLOBAL_CFG_DQM_FULL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_dqm_not_empty : reg = &RU_REG(QM, GLOBAL_CFG_DQM_NOT_EMPTY); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_dqm_pop_ready : reg = &RU_REG(QM, GLOBAL_CFG_DQM_POP_READY); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_aggregation_context_valid : reg = &RU_REG(QM, GLOBAL_CFG_AGGREGATION_CONTEXT_VALID); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_aggregation_timer_ctrl : reg = &RU_REG(QM, GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_fpm_buffer_grp_res : reg = &RU_REG(QM, GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_fpm_buffer_gbl_thr : reg = &RU_REG(QM, GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_flow_ctrl_rnr_cfg : reg = &RU_REG(QM, GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_flow_ctrl_intr : reg = &RU_REG(QM, GLOBAL_CFG_QM_FLOW_CTRL_INTR); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_fpm_ug_gbl_cnt : reg = &RU_REG(QM, GLOBAL_CFG_QM_FPM_UG_GBL_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg_qm_egress_flush_queue : reg = &RU_REG(QM, GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_pools_thr : reg = &RU_REG(QM, FPM_POOLS_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_usr_grp_lower_thr : reg = &RU_REG(QM, FPM_USR_GRP_LOWER_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_usr_grp_mid_thr : reg = &RU_REG(QM, FPM_USR_GRP_MID_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_usr_grp_higher_thr : reg = &RU_REG(QM, FPM_USR_GRP_HIGHER_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_usr_grp_cnt : reg = &RU_REG(QM, FPM_USR_GRP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_runner_grp_rnr_config : reg = &RU_REG(QM, RUNNER_GRP_RNR_CONFIG); blk = &RU_BLK(QM); break;
    case bdmf_address_runner_grp_queue_config : reg = &RU_REG(QM, RUNNER_GRP_QUEUE_CONFIG); blk = &RU_BLK(QM); break;
    case bdmf_address_runner_grp_pdfifo_config : reg = &RU_REG(QM, RUNNER_GRP_PDFIFO_CONFIG); blk = &RU_BLK(QM); break;
    case bdmf_address_runner_grp_update_fifo_config : reg = &RU_REG(QM, RUNNER_GRP_UPDATE_FIFO_CONFIG); blk = &RU_BLK(QM); break;
    case bdmf_address_intr_ctrl_isr : reg = &RU_REG(QM, INTR_CTRL_ISR); blk = &RU_BLK(QM); break;
    case bdmf_address_intr_ctrl_ism : reg = &RU_REG(QM, INTR_CTRL_ISM); blk = &RU_BLK(QM); break;
    case bdmf_address_intr_ctrl_ier : reg = &RU_REG(QM, INTR_CTRL_IER); blk = &RU_BLK(QM); break;
    case bdmf_address_intr_ctrl_itr : reg = &RU_REG(QM, INTR_CTRL_ITR); blk = &RU_BLK(QM); break;
    case bdmf_address_clk_gate_clk_gate_cntrl : reg = &RU_REG(QM, CLK_GATE_CLK_GATE_CNTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_ctrl : reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_0 : reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_1 : reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_2 : reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_3 : reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_0 : reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_1 : reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_2 : reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2); blk = &RU_BLK(QM); break;
    case bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_3 : reg = &RU_REG(QM, CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3); blk = &RU_BLK(QM); break;
    case bdmf_address_queue_context_context : reg = &RU_REG(QM, QUEUE_CONTEXT_CONTEXT); blk = &RU_BLK(QM); break;
    case bdmf_address_wred_profile_color_min_thr_0 : reg = &RU_REG(QM, WRED_PROFILE_COLOR_MIN_THR_0); blk = &RU_BLK(QM); break;
    case bdmf_address_wred_profile_color_min_thr_1 : reg = &RU_REG(QM, WRED_PROFILE_COLOR_MIN_THR_1); blk = &RU_BLK(QM); break;
    case bdmf_address_wred_profile_color_max_thr_0 : reg = &RU_REG(QM, WRED_PROFILE_COLOR_MAX_THR_0); blk = &RU_BLK(QM); break;
    case bdmf_address_wred_profile_color_max_thr_1 : reg = &RU_REG(QM, WRED_PROFILE_COLOR_MAX_THR_1); blk = &RU_BLK(QM); break;
    case bdmf_address_wred_profile_color_slope_0 : reg = &RU_REG(QM, WRED_PROFILE_COLOR_SLOPE_0); blk = &RU_BLK(QM); break;
    case bdmf_address_wred_profile_color_slope_1 : reg = &RU_REG(QM, WRED_PROFILE_COLOR_SLOPE_1); blk = &RU_BLK(QM); break;
    case bdmf_address_copy_decision_profile_thr : reg = &RU_REG(QM, COPY_DECISION_PROFILE_THR); blk = &RU_BLK(QM); break;
    case bdmf_address_total_valid_counter_counter : reg = &RU_REG(QM, TOTAL_VALID_COUNTER_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_dqm_valid_counter_counter : reg = &RU_REG(QM, DQM_VALID_COUNTER_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_drop_counter_counter : reg = &RU_REG(QM, DROP_COUNTER_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_epon_rpt_cnt_counter : reg = &RU_REG(QM, EPON_RPT_CNT_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_epon_rpt_cnt_queue_status : reg = &RU_REG(QM, EPON_RPT_CNT_QUEUE_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_rd_data_pool0 : reg = &RU_REG(QM, RD_DATA_POOL0); blk = &RU_BLK(QM); break;
    case bdmf_address_rd_data_pool1 : reg = &RU_REG(QM, RD_DATA_POOL1); blk = &RU_BLK(QM); break;
    case bdmf_address_rd_data_pool2 : reg = &RU_REG(QM, RD_DATA_POOL2); blk = &RU_BLK(QM); break;
    case bdmf_address_rd_data_pool3 : reg = &RU_REG(QM, RD_DATA_POOL3); blk = &RU_BLK(QM); break;
    case bdmf_address_pdfifo_ptr : reg = &RU_REG(QM, PDFIFO_PTR); blk = &RU_BLK(QM); break;
    case bdmf_address_update_fifo_ptr : reg = &RU_REG(QM, UPDATE_FIFO_PTR); blk = &RU_BLK(QM); break;
    case bdmf_address_debug_sel : reg = &RU_REG(QM, DEBUG_SEL); blk = &RU_BLK(QM); break;
    case bdmf_address_debug_bus_lsb : reg = &RU_REG(QM, DEBUG_BUS_LSB); blk = &RU_BLK(QM); break;
    case bdmf_address_debug_bus_msb : reg = &RU_REG(QM, DEBUG_BUS_MSB); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_spare_config : reg = &RU_REG(QM, QM_SPARE_CONFIG); blk = &RU_BLK(QM); break;
    case bdmf_address_good_lvl1_pkts_cnt : reg = &RU_REG(QM, GOOD_LVL1_PKTS_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_good_lvl1_bytes_cnt : reg = &RU_REG(QM, GOOD_LVL1_BYTES_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_good_lvl2_pkts_cnt : reg = &RU_REG(QM, GOOD_LVL2_PKTS_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_good_lvl2_bytes_cnt : reg = &RU_REG(QM, GOOD_LVL2_BYTES_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_copied_pkts_cnt : reg = &RU_REG(QM, COPIED_PKTS_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_copied_bytes_cnt : reg = &RU_REG(QM, COPIED_BYTES_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_agg_pkts_cnt : reg = &RU_REG(QM, AGG_PKTS_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_agg_bytes_cnt : reg = &RU_REG(QM, AGG_BYTES_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_agg_1_pkts_cnt : reg = &RU_REG(QM, AGG_1_PKTS_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_agg_2_pkts_cnt : reg = &RU_REG(QM, AGG_2_PKTS_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_agg_3_pkts_cnt : reg = &RU_REG(QM, AGG_3_PKTS_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_agg_4_pkts_cnt : reg = &RU_REG(QM, AGG_4_PKTS_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_wred_drop_cnt : reg = &RU_REG(QM, WRED_DROP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_congestion_drop_cnt : reg = &RU_REG(QM, FPM_CONGESTION_DROP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_ddr_pd_congestion_drop_cnt : reg = &RU_REG(QM, DDR_PD_CONGESTION_DROP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_ddr_byte_congestion_drop_cnt : reg = &RU_REG(QM, DDR_BYTE_CONGESTION_DROP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_pd_congestion_drop_cnt : reg = &RU_REG(QM, QM_PD_CONGESTION_DROP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_abs_requeue_cnt : reg = &RU_REG(QM, QM_ABS_REQUEUE_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_prefetch_fifo0_status : reg = &RU_REG(QM, FPM_PREFETCH_FIFO0_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_prefetch_fifo1_status : reg = &RU_REG(QM, FPM_PREFETCH_FIFO1_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_prefetch_fifo2_status : reg = &RU_REG(QM, FPM_PREFETCH_FIFO2_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_prefetch_fifo3_status : reg = &RU_REG(QM, FPM_PREFETCH_FIFO3_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_normal_rmt_fifo_status : reg = &RU_REG(QM, NORMAL_RMT_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_non_delayed_rmt_fifo_status : reg = &RU_REG(QM, NON_DELAYED_RMT_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_non_delayed_out_fifo_status : reg = &RU_REG(QM, NON_DELAYED_OUT_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_pre_cm_fifo_status : reg = &RU_REG(QM, PRE_CM_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_cm_rd_pd_fifo_status : reg = &RU_REG(QM, CM_RD_PD_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_cm_wr_pd_fifo_status : reg = &RU_REG(QM, CM_WR_PD_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_cm_common_input_fifo_status : reg = &RU_REG(QM, CM_COMMON_INPUT_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_bb0_output_fifo_status : reg = &RU_REG(QM, BB0_OUTPUT_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_bb1_output_fifo_status : reg = &RU_REG(QM, BB1_OUTPUT_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_bb1_input_fifo_status : reg = &RU_REG(QM, BB1_INPUT_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_egress_data_fifo_status : reg = &RU_REG(QM, EGRESS_DATA_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_egress_rr_fifo_status : reg = &RU_REG(QM, EGRESS_RR_FIFO_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_bb_route_ovr : reg = &RU_REG(QM, BB_ROUTE_OVR); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_ingress_stat : reg = &RU_REG(QM, QM_INGRESS_STAT); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_egress_stat : reg = &RU_REG(QM, QM_EGRESS_STAT); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_cm_stat : reg = &RU_REG(QM, QM_CM_STAT); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_fpm_prefetch_stat : reg = &RU_REG(QM, QM_FPM_PREFETCH_STAT); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_connect_ack_counter : reg = &RU_REG(QM, QM_CONNECT_ACK_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_ddr_wr_reply_counter : reg = &RU_REG(QM, QM_DDR_WR_REPLY_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_ddr_pipe_byte_counter : reg = &RU_REG(QM, QM_DDR_PIPE_BYTE_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_abs_requeue_valid_counter : reg = &RU_REG(QM, QM_ABS_REQUEUE_VALID_COUNTER); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_illegal_pd_capture : reg = &RU_REG(QM, QM_ILLEGAL_PD_CAPTURE); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_ingress_processed_pd_capture : reg = &RU_REG(QM, QM_INGRESS_PROCESSED_PD_CAPTURE); blk = &RU_BLK(QM); break;
    case bdmf_address_qm_cm_processed_pd_capture : reg = &RU_REG(QM, QM_CM_PROCESSED_PD_CAPTURE); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_pool_drop_cnt : reg = &RU_REG(QM, FPM_POOL_DROP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_grp_drop_cnt : reg = &RU_REG(QM, FPM_GRP_DROP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_fpm_buffer_res_drop_cnt : reg = &RU_REG(QM, FPM_BUFFER_RES_DROP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_psram_egress_cong_drp_cnt : reg = &RU_REG(QM, PSRAM_EGRESS_CONG_DRP_CNT); blk = &RU_BLK(QM); break;
    case bdmf_address_data : reg = &RU_REG(QM, DATA); blk = &RU_BLK(QM); break;
    case bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_base : reg = &RU_REG(QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE); blk = &RU_BLK(QM); break;
    case bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_mask : reg = &RU_REG(QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK); blk = &RU_BLK(QM); break;
    case bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_base : reg = &RU_REG(QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE); blk = &RU_BLK(QM); break;
    case bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_mask : reg = &RU_REG(QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK); blk = &RU_BLK(QM); break;
    case bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_base : reg = &RU_REG(QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE); blk = &RU_BLK(QM); break;
    case bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_mask : reg = &RU_REG(QM, XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_mactype : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1 : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2 : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_arb_cfg : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_bbroute : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_q2rnr : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_perqtask : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_txrstcmd : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_dbgsel : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_common_configurations_gpr : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pdbase : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pdsize : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pdempty : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_txthresh : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_eee : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_ts : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_srampd : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_ddrpd : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_pddrop : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_stscnt : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_stsdrop : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_msgcnt : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_msgdrop : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_getnextnull : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_flushpkts : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_lenerr : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_aggrlenerr : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_srampkt : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_ddrpkt : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_srambyte : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_ddrbyte : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_swrden : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_swrdaddr : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_swrddata : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_unifiedpkt : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_unifiedbyte : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG); blk = &RU_BLK(QM); break;
    case bdmf_address_bbh_tx_qm_bbhtx_debug_counters_in_segmentation : reg = &RU_REG(QM, BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_config_bbrouteovrd : reg = &RU_REG(QM, DMA_QM_DMA_CONFIG_BBROUTEOVRD); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_config_num_of_writes : reg = &RU_REG(QM, DMA_QM_DMA_CONFIG_NUM_OF_WRITES); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_config_num_of_reads : reg = &RU_REG(QM, DMA_QM_DMA_CONFIG_NUM_OF_READS); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_config_u_thresh : reg = &RU_REG(QM, DMA_QM_DMA_CONFIG_U_THRESH); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_config_pri : reg = &RU_REG(QM, DMA_QM_DMA_CONFIG_PRI); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_config_periph_source : reg = &RU_REG(QM, DMA_QM_DMA_CONFIG_PERIPH_SOURCE); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_config_weight : reg = &RU_REG(QM, DMA_QM_DMA_CONFIG_WEIGHT); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_config_ptrrst : reg = &RU_REG(QM, DMA_QM_DMA_CONFIG_PTRRST); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_config_max_otf : reg = &RU_REG(QM, DMA_QM_DMA_CONFIG_MAX_OTF); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_config_clk_gate_cntrl : reg = &RU_REG(QM, DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_config_dbg_sel : reg = &RU_REG(QM, DMA_QM_DMA_CONFIG_DBG_SEL); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_debug_nempty : reg = &RU_REG(QM, DMA_QM_DMA_DEBUG_NEMPTY); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_debug_urgnt : reg = &RU_REG(QM, DMA_QM_DMA_DEBUG_URGNT); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_debug_selsrc : reg = &RU_REG(QM, DMA_QM_DMA_DEBUG_SELSRC); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_debug_req_cnt_rx : reg = &RU_REG(QM, DMA_QM_DMA_DEBUG_REQ_CNT_RX); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_debug_req_cnt_tx : reg = &RU_REG(QM, DMA_QM_DMA_DEBUG_REQ_CNT_TX); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_debug_req_cnt_rx_acc : reg = &RU_REG(QM, DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_debug_req_cnt_tx_acc : reg = &RU_REG(QM, DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_debug_rdadd : reg = &RU_REG(QM, DMA_QM_DMA_DEBUG_RDADD); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_debug_rdvalid : reg = &RU_REG(QM, DMA_QM_DMA_DEBUG_RDVALID); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_debug_rddata : reg = &RU_REG(QM, DMA_QM_DMA_DEBUG_RDDATA); blk = &RU_BLK(QM); break;
    case bdmf_address_dma_qm_dma_debug_rddatardy : reg = &RU_REG(QM, DMA_QM_DMA_DEBUG_RDDATARDY); blk = &RU_BLK(QM); break;
    default :
        return BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    if((bdmf_parm = bdmfmon_find_named_parm(session,"index1")))
    {
        index1_start = bdmf_parm->value.unumber;
        index1_stop = index1_start + 1;
    }
    else
        index1_stop = blk->addr_count;
    if((bdmf_parm = bdmfmon_find_named_parm(session,"index2")))
    {
        index2_start = bdmf_parm->value.unumber;
        index2_stop = index2_start + 1;
    }
    else
        index2_stop = reg->ram_count + 1;
    if(index1_stop > blk->addr_count)
    {
        bdmf_session_print(session, "index1 (%u) is out of range (%u).\n", index1_stop, blk->addr_count);
        return BDMF_ERR_RANGE;
    }
    if(index2_stop > (reg->ram_count + 1))
    {
        bdmf_session_print(session, "index2 (%u) is out of range (%u).\n", index2_stop, reg->ram_count + 1);
        return BDMF_ERR_RANGE;
    }
    if(reg->ram_count)
        for (i = index1_start; i < index1_stop; i++)
        {
            bdmf_session_print(session, "index1 = %u\n", i);
            for (j = index2_start; j < index2_stop; j++)
                bdmf_session_print(session, 	 "(%5u) 0x%lX\n", j, (blk->addr[i] + reg->addr + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%lX\n", i, blk->addr[i]+reg->addr);
    return 0;
}

bdmfmon_handle_t ag_drv_qm_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "qm"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "qm", "qm", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_ddr_cong_ctrl[]={
            BDMFMON_MAKE_PARM("ddr_byte_congestion_drop_enable", "ddr_byte_congestion_drop_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bytes_lower_thr", "ddr_bytes_lower_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bytes_mid_thr", "ddr_bytes_mid_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_bytes_higher_thr", "ddr_bytes_higher_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_pd_congestion_drop_enable", "ddr_pd_congestion_drop_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_pipe_lower_thr", "ddr_pipe_lower_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_pipe_higher_thr", "ddr_pipe_higher_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_ug_thr[]={
            BDMFMON_MAKE_PARM("ug_grp_idx", "ug_grp_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("lower_thr", "lower_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mid_thr", "mid_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("higher_thr", "higher_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rnr_group_cfg[]={
            BDMFMON_MAKE_PARM("rnr_idx", "rnr_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("start_queue", "start_queue", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("end_queue", "end_queue", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pd_fifo_base", "pd_fifo_base", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pd_fifo_size", "pd_fifo_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("upd_fifo_base", "upd_fifo_base", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("upd_fifo_size", "upd_fifo_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_bb_id", "rnr_bb_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_task", "rnr_task", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_enable", "rnr_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cpu_pd_indirect_wr_data[]={
            BDMFMON_MAKE_PARM("indirect_grp_idx", "indirect_grp_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data0", "data0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data1", "data1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data2", "data2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data3", "data3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wred_profile_cfg[]={
            BDMFMON_MAKE_PARM("profile_idx", "profile_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("min_thr0", "min_thr0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("flw_ctrl_en0", "flw_ctrl_en0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("min_thr1", "min_thr1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("flw_ctrl_en1", "flw_ctrl_en1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("max_thr0", "max_thr0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("max_thr1", "max_thr1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("slope_mantissa0", "slope_mantissa0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("slope_exp0", "slope_exp0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("slope_mantissa1", "slope_mantissa1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("slope_exp1", "slope_exp1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ubus_slave[]={
            BDMFMON_MAKE_PARM("vpb_base", "vpb_base", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("vpb_mask", "vpb_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("apb_base", "apb_base", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("apb_mask", "apb_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dqm_base", "dqm_base", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dqm_mask", "dqm_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_src_id[]={
            BDMFMON_MAKE_PARM("fpmsrc", "fpmsrc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sbpmsrc", "sbpmsrc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("stsrnrsrc", "stsrnrsrc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rnr_src_id[]={
            BDMFMON_MAKE_PARM("pdrnr0src", "pdrnr0src", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pdrnr1src", "pdrnr1src", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_dma_cfg[]={
            BDMFMON_MAKE_PARM("dmasrc", "dmasrc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("descbase", "descbase", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("descsize", "descsize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_max_otf_read_request[]={
            BDMFMON_MAKE_PARM("maxreq", "maxreq", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_epon_urgent[]={
            BDMFMON_MAKE_PARM("epnurgnt", "epnurgnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_sdma_cfg[]={
            BDMFMON_MAKE_PARM("sdmasrc", "sdmasrc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("descbase", "descbase", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("descsize", "descsize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sdma_max_otf_read_request[]={
            BDMFMON_MAKE_PARM("maxreq", "maxreq", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sdma_epon_urgent[]={
            BDMFMON_MAKE_PARM("epnurgnt", "epnurgnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_ddr_cfg[]={
            BDMFMON_MAKE_PARM("bufsize", "bufsize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("byteresul", "byteresul", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddrtxoffset", "ddrtxoffset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hnsize0", "hnsize0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hnsize1", "hnsize1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_info[]={
            BDMFMON_MAKE_PARM("nempty", "nempty", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("urgnt", "urgnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sel_src", "sel_src", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("address", "address", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("datacs", "datacs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cdcs", "cdcs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rrcs", "rrcs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ready", "ready", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_enable_ctrl[]={
            BDMFMON_MAKE_PARM("fpm_prefetch_enable", "fpm_prefetch_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("reorder_credit_enable", "reorder_credit_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dqm_pop_enable", "dqm_pop_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rmt_fixed_arb_enable", "rmt_fixed_arb_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dqm_push_fixed_arb_enable", "dqm_push_fixed_arb_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_reset_ctrl[]={
            BDMFMON_MAKE_PARM("fpm_prefetch0_sw_rst", "fpm_prefetch0_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_prefetch1_sw_rst", "fpm_prefetch1_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_prefetch2_sw_rst", "fpm_prefetch2_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_prefetch3_sw_rst", "fpm_prefetch3_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("normal_rmt_sw_rst", "normal_rmt_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("non_delayed_rmt_sw_rst", "non_delayed_rmt_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pre_cm_fifo_sw_rst", "pre_cm_fifo_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cm_rd_pd_fifo_sw_rst", "cm_rd_pd_fifo_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cm_wr_pd_fifo_sw_rst", "cm_wr_pd_fifo_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bb0_output_fifo_sw_rst", "bb0_output_fifo_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bb1_output_fifo_sw_rst", "bb1_output_fifo_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bb1_input_fifo_sw_rst", "bb1_input_fifo_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tm_fifo_ptr_sw_rst", "tm_fifo_ptr_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("non_delayed_out_fifo_sw_rst", "non_delayed_out_fifo_sw_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_drop_counters_ctrl[]={
            BDMFMON_MAKE_PARM("read_clear_pkts", "read_clear_pkts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("read_clear_bytes", "read_clear_bytes", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("disable_wrap_around_pkts", "disable_wrap_around_pkts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("disable_wrap_around_bytes", "disable_wrap_around_bytes", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("free_with_context_last_search", "free_with_context_last_search", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wred_disable", "wred_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_pd_congestion_disable", "ddr_pd_congestion_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_byte_congestion_disable", "ddr_byte_congestion_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_occupancy_disable", "ddr_occupancy_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_fpm_congestion_disable", "ddr_fpm_congestion_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_ug_disable", "fpm_ug_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("queue_occupancy_ddr_copy_decision_disable", "queue_occupancy_ddr_copy_decision_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("psram_occupancy_ddr_copy_decision_disable", "psram_occupancy_ddr_copy_decision_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dont_send_mc_bit_to_bbh", "dont_send_mc_bit_to_bbh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("close_aggregation_on_timeout_disable", "close_aggregation_on_timeout_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_congestion_buf_release_mechanism_disable", "fpm_congestion_buf_release_mechanism_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_buffer_global_res_enable", "fpm_buffer_global_res_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_preserve_pd_with_fpm", "qm_preserve_pd_with_fpm", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_residue_per_queue", "qm_residue_per_queue", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ghost_rpt_update_after_close_agg_en", "ghost_rpt_update_after_close_agg_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_ug_flow_ctrl_disable", "fpm_ug_flow_ctrl_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_write_multi_slave_en", "ddr_write_multi_slave_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_pd_congestion_agg_priority", "ddr_pd_congestion_agg_priority", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("psram_occupancy_drop_disable", "psram_occupancy_drop_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_ddr_write_alignment", "qm_ddr_write_alignment", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("exclusive_dont_drop", "exclusive_dont_drop", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("exclusive_dont_drop_bp_en", "exclusive_dont_drop_bp_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gpon_dbr_ceil", "gpon_dbr_ceil", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_ctrl[]={
            BDMFMON_MAKE_PARM("fpm_pool_bp_enable", "fpm_pool_bp_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_congestion_bp_enable", "fpm_congestion_bp_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_prefetch_min_pool_size", "fpm_prefetch_min_pool_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_prefetch_pending_req_limit", "fpm_prefetch_pending_req_limit", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qm_pd_cong_ctrl[]={
            BDMFMON_MAKE_PARM("total_pd_thr", "total_pd_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_abs_drop_queue[]={
            BDMFMON_MAKE_PARM("abs_drop_queue", "abs_drop_queue", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("abs_drop_queue_en", "abs_drop_queue_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_aggregation_ctrl[]={
            BDMFMON_MAKE_PARM("max_agg_bytes", "max_agg_bytes", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("max_agg_pkts", "max_agg_pkts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_base_addr[]={
            BDMFMON_MAKE_PARM("fpm_base_addr", "fpm_base_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_fpm_coherent_base_addr[]={
            BDMFMON_MAKE_PARM("fpm_base_addr", "fpm_base_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ddr_sop_offset[]={
            BDMFMON_MAKE_PARM("ddr_sop_offset0", "ddr_sop_offset0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_sop_offset1", "ddr_sop_offset1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_epon_overhead_ctrl[]={
            BDMFMON_MAKE_PARM("epon_line_rate", "epon_line_rate", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("epon_crc_add_disable", "epon_crc_add_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mac_flow_overwrite_crc_en", "mac_flow_overwrite_crc_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mac_flow_overwrite_crc", "mac_flow_overwrite_crc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fec_ipg_length", "fec_ipg_length", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_qm_aggregation_timer_ctrl[]={
            BDMFMON_MAKE_PARM("prescaler_granularity", "prescaler_granularity", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("aggregation_timeout_value", "aggregation_timeout_value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_qm_fpm_buffer_grp_res[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("res_thr_0", "res_thr_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("res_thr_1", "res_thr_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("res_thr_2", "res_thr_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("res_thr_3", "res_thr_3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_qm_fpm_buffer_gbl_thr[]={
            BDMFMON_MAKE_PARM("res_thr_global", "res_thr_global", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_qm_flow_ctrl_rnr_cfg[]={
            BDMFMON_MAKE_PARM("rnr_bb_id", "rnr_bb_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_task", "rnr_task", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_enable", "rnr_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_qm_flow_ctrl_intr[]={
            BDMFMON_MAKE_PARM("qm_flow_ctrl_intr", "qm_flow_ctrl_intr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_qm_fpm_ug_gbl_cnt[]={
            BDMFMON_MAKE_PARM("fpm_gbl_cnt", "fpm_gbl_cnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_qm_egress_flush_queue[]={
            BDMFMON_MAKE_PARM("queue_num", "queue_num", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("flush_en", "flush_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_pool_thr[]={
            BDMFMON_MAKE_PARM("pool_idx", "pool_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("lower_thr", "lower_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("higher_thr", "higher_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_ug_cnt[]={
            BDMFMON_MAKE_PARM("grp_idx", "grp_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_ug_cnt", "fpm_ug_cnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_isr[]={
            BDMFMON_MAKE_PARM("qm_dqm_pop_on_empty", "qm_dqm_pop_on_empty", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_dqm_push_on_full", "qm_dqm_push_on_full", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_cpu_pop_on_empty", "qm_cpu_pop_on_empty", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_cpu_push_on_full", "qm_cpu_push_on_full", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_normal_queue_pd_no_credit", "qm_normal_queue_pd_no_credit", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_non_delayed_queue_pd_no_credit", "qm_non_delayed_queue_pd_no_credit", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_non_valid_queue", "qm_non_valid_queue", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_agg_coherent_inconsistency", "qm_agg_coherent_inconsistency", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_force_copy_on_non_delayed", "qm_force_copy_on_non_delayed", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_fpm_pool_size_nonexistent", "qm_fpm_pool_size_nonexistent", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_target_mem_abs_contradiction", "qm_target_mem_abs_contradiction", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_1588_drop", "qm_1588_drop", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_1588_multicast_contradiction", "qm_1588_multicast_contradiction", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_byte_drop_cnt_overrun", "qm_byte_drop_cnt_overrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_pkt_drop_cnt_overrun", "qm_pkt_drop_cnt_overrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_total_byte_cnt_underrun", "qm_total_byte_cnt_underrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_total_pkt_cnt_underrun", "qm_total_pkt_cnt_underrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_fpm_ug0_underrun", "qm_fpm_ug0_underrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_fpm_ug1_underrun", "qm_fpm_ug1_underrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_fpm_ug2_underrun", "qm_fpm_ug2_underrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_fpm_ug3_underrun", "qm_fpm_ug3_underrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("qm_timer_wraparound", "qm_timer_wraparound", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_ier[]={
            BDMFMON_MAKE_PARM("iem", "iem", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_intr_ctrl_itr[]={
            BDMFMON_MAKE_PARM("ist", "ist", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_clk_gate_clk_gate_cntrl[]={
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intrvl", "keep_alive_intrvl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cpu_indr_port_cpu_pd_indirect_ctrl[]={
            BDMFMON_MAKE_PARM("indirect_grp_idx", "indirect_grp_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("queue_num", "queue_num", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cmd", "cmd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("done", "done", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("error", "error", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_q_context[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wred_profile", "wred_profile", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("copy_dec_profile", "copy_dec_profile", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("copy_to_ddr", "copy_to_ddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_copy_disable", "ddr_copy_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("aggregation_disable", "aggregation_disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_ug", "fpm_ug", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("exclusive_priority", "exclusive_priority", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q_802_1ae", "q_802_1ae", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sci", "sci", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fec_enable", "fec_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("res_profile", "res_profile", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_copy_decision_profile[]={
            BDMFMON_MAKE_PARM("profile_idx", "profile_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("queue_occupancy_thr", "queue_occupancy_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("psram_thr", "psram_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_total_valid_cnt[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqm_valid_cnt[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_epon_q_byte_cnt[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_sel[]={
            BDMFMON_MAKE_PARM("select", "select", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bb_route_ovr[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ovr_en", "ovr_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dest_id", "dest_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("route_addr", "route_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_mactype[]={
            BDMFMON_MAKE_PARM("type", "type", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1[]={
            BDMFMON_MAKE_PARM("rnr_cfg_index_1", "rnr_cfg_index_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tcontaddr", "tcontaddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("skbaddr", "skbaddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2[]={
            BDMFMON_MAKE_PARM("rnr_cfg_index_2", "rnr_cfg_index_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ptraddr", "ptraddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("task", "task", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg[]={
            BDMFMON_MAKE_PARM("freenocntxt", "freenocntxt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("specialfree", "specialfree", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maxgn", "maxgn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("addr0", "addr0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("addr1", "addr1", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("addr0", "addr0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("addr1", "addr1", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl[]={
            BDMFMON_MAKE_PARM("psramsize", "psramsize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddrsize", "ddrsize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("psrambase", "psrambase", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_arb_cfg[]={
            BDMFMON_MAKE_PARM("hightrxq", "hightrxq", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_bbroute[]={
            BDMFMON_MAKE_PARM("route", "route", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dest", "dest", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_q2rnr[]={
            BDMFMON_MAKE_PARM("q_2_rnr_index", "q_2_rnr_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q0", "q0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q1", "q1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_perqtask[]={
            BDMFMON_MAKE_PARM("task0", "task0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("task1", "task1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("task2", "task2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("task3", "task3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("task4", "task4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("task5", "task5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("task6", "task6", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("task7", "task7", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_txrstcmd[]={
            BDMFMON_MAKE_PARM("cntxtrst", "cntxtrst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pdfiforst", "pdfiforst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dmaptrrst", "dmaptrrst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sdmaptrrst", "sdmaptrrst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bpmfiforst", "bpmfiforst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sbpmfiforst", "sbpmfiforst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("okfiforst", "okfiforst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddrfiforst", "ddrfiforst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sramfiforst", "sramfiforst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("skbptrrst", "skbptrrst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("stsfiforst", "stsfiforst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("reqfiforst", "reqfiforst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("msgfiforst", "msgfiforst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gnxtfiforst", "gnxtfiforst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fbnfiforst", "fbnfiforst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_dbgsel[]={
            BDMFMON_MAKE_PARM("dbgsel", "dbgsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl[]={
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intrvl", "keep_alive_intrvl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_gpr[]={
            BDMFMON_MAKE_PARM("gpr", "gpr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_lan_configurations_pdbase[]={
            BDMFMON_MAKE_PARM("fifobase0", "fifobase0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifobase1", "fifobase1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_lan_configurations_pdsize[]={
            BDMFMON_MAKE_PARM("fifosize0", "fifosize0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifosize1", "fifosize1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph[]={
            BDMFMON_MAKE_PARM("wkupthresh0", "wkupthresh0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wkupthresh1", "wkupthresh1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th[]={
            BDMFMON_MAKE_PARM("pdlimit0", "pdlimit0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pdlimit1", "pdlimit1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en[]={
            BDMFMON_MAKE_PARM("pdlimiten", "pdlimiten", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_lan_configurations_pdempty[]={
            BDMFMON_MAKE_PARM("empty", "empty", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_lan_configurations_txthresh[]={
            BDMFMON_MAKE_PARM("ddrthresh", "ddrthresh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sramthresh", "sramthresh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_lan_configurations_eee[]={
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_lan_configurations_ts[]={
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_debug_counters_swrden[]={
            BDMFMON_MAKE_PARM("pdsel", "pdsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pdvsel", "pdvsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pdemptysel", "pdemptysel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pdfullsel", "pdfullsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pdbemptysel", "pdbemptysel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pdffwkpsel", "pdffwkpsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fbnsel", "fbnsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fbnvsel", "fbnvsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fbnemptysel", "fbnemptysel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fbnfullsel", "fbnfullsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("getnextsel", "getnextsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("getnextvsel", "getnextvsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("getnextemptysel", "getnextemptysel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("getnextfullsel", "getnextfullsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gpncntxtsel", "gpncntxtsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bpmsel", "bpmsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bpmfsel", "bpmfsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sbpmsel", "sbpmsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sbpmfsel", "sbpmfsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("stssel", "stssel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("stsvsel", "stsvsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("stsemptysel", "stsemptysel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("stsfullsel", "stsfullsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("stsbemptysel", "stsbemptysel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("stsffwkpsel", "stsffwkpsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("msgsel", "msgsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("msgvsel", "msgvsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("epnreqsel", "epnreqsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("datasel", "datasel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("reordersel", "reordersel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsinfosel", "tsinfosel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mactxsel", "mactxsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_debug_counters_swrdaddr[]={
            BDMFMON_MAKE_PARM("rdaddr", "rdaddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_bbrouteovrd[]={
            BDMFMON_MAKE_PARM("dest", "dest", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("route", "route", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ovrd", "ovrd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_num_of_writes[]={
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("numofbuff", "numofbuff", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_num_of_reads[]={
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rr_num", "rr_num", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_u_thresh[]={
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("into_u", "into_u", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("out_of_u", "out_of_u", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_pri[]={
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxpri", "rxpri", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txpri", "txpri", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_periph_source[]={
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxsource", "rxsource", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txsource", "txsource", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_weight[]={
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxweight", "rxweight", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txweight", "txweight", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_ptrrst[]={
            BDMFMON_MAKE_PARM("rstvec", "rstvec", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_max_otf[]={
            BDMFMON_MAKE_PARM("max", "max", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_clk_gate_cntrl[]={
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intrvl", "keep_alive_intrvl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_dbg_sel[]={
            BDMFMON_MAKE_PARM("dbgsel", "dbgsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="ddr_cong_ctrl", .val=cli_qm_ddr_cong_ctrl, .parms=set_ddr_cong_ctrl },
            { .name="fpm_ug_thr", .val=cli_qm_fpm_ug_thr, .parms=set_fpm_ug_thr },
            { .name="rnr_group_cfg", .val=cli_qm_rnr_group_cfg, .parms=set_rnr_group_cfg },
            { .name="cpu_pd_indirect_wr_data", .val=cli_qm_cpu_pd_indirect_wr_data, .parms=set_cpu_pd_indirect_wr_data },
            { .name="wred_profile_cfg", .val=cli_qm_wred_profile_cfg, .parms=set_wred_profile_cfg },
            { .name="ubus_slave", .val=cli_qm_ubus_slave, .parms=set_ubus_slave },
            { .name="cfg_src_id", .val=cli_qm_cfg_src_id, .parms=set_cfg_src_id },
            { .name="rnr_src_id", .val=cli_qm_rnr_src_id, .parms=set_rnr_src_id },
            { .name="bbh_dma_cfg", .val=cli_qm_bbh_dma_cfg, .parms=set_bbh_dma_cfg },
            { .name="dma_max_otf_read_request", .val=cli_qm_dma_max_otf_read_request, .parms=set_dma_max_otf_read_request },
            { .name="dma_epon_urgent", .val=cli_qm_dma_epon_urgent, .parms=set_dma_epon_urgent },
            { .name="bbh_sdma_cfg", .val=cli_qm_bbh_sdma_cfg, .parms=set_bbh_sdma_cfg },
            { .name="sdma_max_otf_read_request", .val=cli_qm_sdma_max_otf_read_request, .parms=set_sdma_max_otf_read_request },
            { .name="sdma_epon_urgent", .val=cli_qm_sdma_epon_urgent, .parms=set_sdma_epon_urgent },
            { .name="bbh_ddr_cfg", .val=cli_qm_bbh_ddr_cfg, .parms=set_bbh_ddr_cfg },
            { .name="debug_info", .val=cli_qm_debug_info, .parms=set_debug_info },
            { .name="enable_ctrl", .val=cli_qm_enable_ctrl, .parms=set_enable_ctrl },
            { .name="reset_ctrl", .val=cli_qm_reset_ctrl, .parms=set_reset_ctrl },
            { .name="drop_counters_ctrl", .val=cli_qm_drop_counters_ctrl, .parms=set_drop_counters_ctrl },
            { .name="fpm_ctrl", .val=cli_qm_fpm_ctrl, .parms=set_fpm_ctrl },
            { .name="qm_pd_cong_ctrl", .val=cli_qm_qm_pd_cong_ctrl, .parms=set_qm_pd_cong_ctrl },
            { .name="global_cfg_abs_drop_queue", .val=cli_qm_global_cfg_abs_drop_queue, .parms=set_global_cfg_abs_drop_queue },
            { .name="global_cfg_aggregation_ctrl", .val=cli_qm_global_cfg_aggregation_ctrl, .parms=set_global_cfg_aggregation_ctrl },
            { .name="fpm_base_addr", .val=cli_qm_fpm_base_addr, .parms=set_fpm_base_addr },
            { .name="global_cfg_fpm_coherent_base_addr", .val=cli_qm_global_cfg_fpm_coherent_base_addr, .parms=set_global_cfg_fpm_coherent_base_addr },
            { .name="ddr_sop_offset", .val=cli_qm_ddr_sop_offset, .parms=set_ddr_sop_offset },
            { .name="epon_overhead_ctrl", .val=cli_qm_epon_overhead_ctrl, .parms=set_epon_overhead_ctrl },
            { .name="global_cfg_qm_aggregation_timer_ctrl", .val=cli_qm_global_cfg_qm_aggregation_timer_ctrl, .parms=set_global_cfg_qm_aggregation_timer_ctrl },
            { .name="global_cfg_qm_fpm_buffer_grp_res", .val=cli_qm_global_cfg_qm_fpm_buffer_grp_res, .parms=set_global_cfg_qm_fpm_buffer_grp_res },
            { .name="global_cfg_qm_fpm_buffer_gbl_thr", .val=cli_qm_global_cfg_qm_fpm_buffer_gbl_thr, .parms=set_global_cfg_qm_fpm_buffer_gbl_thr },
            { .name="global_cfg_qm_flow_ctrl_rnr_cfg", .val=cli_qm_global_cfg_qm_flow_ctrl_rnr_cfg, .parms=set_global_cfg_qm_flow_ctrl_rnr_cfg },
            { .name="global_cfg_qm_flow_ctrl_intr", .val=cli_qm_global_cfg_qm_flow_ctrl_intr, .parms=set_global_cfg_qm_flow_ctrl_intr },
            { .name="global_cfg_qm_fpm_ug_gbl_cnt", .val=cli_qm_global_cfg_qm_fpm_ug_gbl_cnt, .parms=set_global_cfg_qm_fpm_ug_gbl_cnt },
            { .name="global_cfg_qm_egress_flush_queue", .val=cli_qm_global_cfg_qm_egress_flush_queue, .parms=set_global_cfg_qm_egress_flush_queue },
            { .name="fpm_pool_thr", .val=cli_qm_fpm_pool_thr, .parms=set_fpm_pool_thr },
            { .name="fpm_ug_cnt", .val=cli_qm_fpm_ug_cnt, .parms=set_fpm_ug_cnt },
            { .name="intr_ctrl_isr", .val=cli_qm_intr_ctrl_isr, .parms=set_intr_ctrl_isr },
            { .name="intr_ctrl_ier", .val=cli_qm_intr_ctrl_ier, .parms=set_intr_ctrl_ier },
            { .name="intr_ctrl_itr", .val=cli_qm_intr_ctrl_itr, .parms=set_intr_ctrl_itr },
            { .name="clk_gate_clk_gate_cntrl", .val=cli_qm_clk_gate_clk_gate_cntrl, .parms=set_clk_gate_clk_gate_cntrl },
            { .name="cpu_indr_port_cpu_pd_indirect_ctrl", .val=cli_qm_cpu_indr_port_cpu_pd_indirect_ctrl, .parms=set_cpu_indr_port_cpu_pd_indirect_ctrl },
            { .name="q_context", .val=cli_qm_q_context, .parms=set_q_context },
            { .name="copy_decision_profile", .val=cli_qm_copy_decision_profile, .parms=set_copy_decision_profile },
            { .name="total_valid_cnt", .val=cli_qm_total_valid_cnt, .parms=set_total_valid_cnt },
            { .name="dqm_valid_cnt", .val=cli_qm_dqm_valid_cnt, .parms=set_dqm_valid_cnt },
            { .name="epon_q_byte_cnt", .val=cli_qm_epon_q_byte_cnt, .parms=set_epon_q_byte_cnt },
            { .name="debug_sel", .val=cli_qm_debug_sel, .parms=set_debug_sel },
            { .name="bb_route_ovr", .val=cli_qm_bb_route_ovr, .parms=set_bb_route_ovr },
            { .name="bbh_tx_qm_bbhtx_common_configurations_mactype", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_mactype, .parms=set_bbh_tx_qm_bbhtx_common_configurations_mactype },
            { .name="bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1, .parms=set_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1 },
            { .name="bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2, .parms=set_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2 },
            { .name="bbh_tx_qm_bbhtx_common_configurations_sbpmcfg", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg, .parms=set_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg },
            { .name="bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel, .parms=set_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel },
            { .name="bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh, .parms=set_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh },
            { .name="bbh_tx_qm_bbhtx_common_configurations_dfifoctrl", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl, .parms=set_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl },
            { .name="bbh_tx_qm_bbhtx_common_configurations_arb_cfg", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_arb_cfg, .parms=set_bbh_tx_qm_bbhtx_common_configurations_arb_cfg },
            { .name="bbh_tx_qm_bbhtx_common_configurations_bbroute", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_bbroute, .parms=set_bbh_tx_qm_bbhtx_common_configurations_bbroute },
            { .name="bbh_tx_qm_bbhtx_common_configurations_q2rnr", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_q2rnr, .parms=set_bbh_tx_qm_bbhtx_common_configurations_q2rnr },
            { .name="bbh_tx_qm_bbhtx_common_configurations_perqtask", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_perqtask, .parms=set_bbh_tx_qm_bbhtx_common_configurations_perqtask },
            { .name="bbh_tx_qm_bbhtx_common_configurations_txrstcmd", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, .parms=set_bbh_tx_qm_bbhtx_common_configurations_txrstcmd },
            { .name="bbh_tx_qm_bbhtx_common_configurations_dbgsel", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_dbgsel, .parms=set_bbh_tx_qm_bbhtx_common_configurations_dbgsel },
            { .name="bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl, .parms=set_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl },
            { .name="bbh_tx_qm_bbhtx_common_configurations_gpr", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_gpr, .parms=set_bbh_tx_qm_bbhtx_common_configurations_gpr },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_pdbase", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdbase, .parms=set_bbh_tx_qm_bbhtx_lan_configurations_pdbase },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_pdsize", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdsize, .parms=set_bbh_tx_qm_bbhtx_lan_configurations_pdsize },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_pdwkuph", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph, .parms=set_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th, .parms=set_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en, .parms=set_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_pdempty", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdempty, .parms=set_bbh_tx_qm_bbhtx_lan_configurations_pdempty },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_txthresh", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_txthresh, .parms=set_bbh_tx_qm_bbhtx_lan_configurations_txthresh },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_eee", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_eee, .parms=set_bbh_tx_qm_bbhtx_lan_configurations_eee },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_ts", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_ts, .parms=set_bbh_tx_qm_bbhtx_lan_configurations_ts },
            { .name="bbh_tx_qm_bbhtx_debug_counters_swrden", .val=cli_qm_bbh_tx_qm_bbhtx_debug_counters_swrden, .parms=set_bbh_tx_qm_bbhtx_debug_counters_swrden },
            { .name="bbh_tx_qm_bbhtx_debug_counters_swrdaddr", .val=cli_qm_bbh_tx_qm_bbhtx_debug_counters_swrdaddr, .parms=set_bbh_tx_qm_bbhtx_debug_counters_swrdaddr },
            { .name="dma_qm_dma_config_bbrouteovrd", .val=cli_qm_dma_qm_dma_config_bbrouteovrd, .parms=set_dma_qm_dma_config_bbrouteovrd },
            { .name="dma_qm_dma_config_num_of_writes", .val=cli_qm_dma_qm_dma_config_num_of_writes, .parms=set_dma_qm_dma_config_num_of_writes },
            { .name="dma_qm_dma_config_num_of_reads", .val=cli_qm_dma_qm_dma_config_num_of_reads, .parms=set_dma_qm_dma_config_num_of_reads },
            { .name="dma_qm_dma_config_u_thresh", .val=cli_qm_dma_qm_dma_config_u_thresh, .parms=set_dma_qm_dma_config_u_thresh },
            { .name="dma_qm_dma_config_pri", .val=cli_qm_dma_qm_dma_config_pri, .parms=set_dma_qm_dma_config_pri },
            { .name="dma_qm_dma_config_periph_source", .val=cli_qm_dma_qm_dma_config_periph_source, .parms=set_dma_qm_dma_config_periph_source },
            { .name="dma_qm_dma_config_weight", .val=cli_qm_dma_qm_dma_config_weight, .parms=set_dma_qm_dma_config_weight },
            { .name="dma_qm_dma_config_ptrrst", .val=cli_qm_dma_qm_dma_config_ptrrst, .parms=set_dma_qm_dma_config_ptrrst },
            { .name="dma_qm_dma_config_max_otf", .val=cli_qm_dma_qm_dma_config_max_otf, .parms=set_dma_qm_dma_config_max_otf },
            { .name="dma_qm_dma_config_clk_gate_cntrl", .val=cli_qm_dma_qm_dma_config_clk_gate_cntrl, .parms=set_dma_qm_dma_config_clk_gate_cntrl },
            { .name="dma_qm_dma_config_dbg_sel", .val=cli_qm_dma_qm_dma_config_dbg_sel, .parms=set_dma_qm_dma_config_dbg_sel },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_qm_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_is_queue_not_empty[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_is_queue_pop_ready[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_is_queue_full[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_ug_thr[]={
            BDMFMON_MAKE_PARM("ug_grp_idx", "ug_grp_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rnr_group_cfg[]={
            BDMFMON_MAKE_PARM("rnr_idx", "rnr_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cpu_pd_indirect_wr_data[]={
            BDMFMON_MAKE_PARM("indirect_grp_idx", "indirect_grp_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cpu_pd_indirect_rd_data[]={
            BDMFMON_MAKE_PARM("indirect_grp_idx", "indirect_grp_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_aggr_context[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wred_profile_cfg[]={
            BDMFMON_MAKE_PARM("profile_idx", "profile_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_cfg_qm_fpm_buffer_grp_res[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_pool_thr[]={
            BDMFMON_MAKE_PARM("pool_idx", "pool_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_ug_cnt[]={
            BDMFMON_MAKE_PARM("grp_idx", "grp_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cpu_indr_port_cpu_pd_indirect_ctrl[]={
            BDMFMON_MAKE_PARM("indirect_grp_idx", "indirect_grp_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_q_context[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_copy_decision_profile[]={
            BDMFMON_MAKE_PARM("profile_idx", "profile_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_total_valid_cnt[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqm_valid_cnt[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_drop_counter[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_epon_q_byte_cnt[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_epon_q_status[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pdfifo_ptr[]={
            BDMFMON_MAKE_PARM("q_idx", "q_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_update_fifo_ptr[]={
            BDMFMON_MAKE_PARM("fifo_idx", "fifo_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bb_route_ovr[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qm_illegal_pd_capture[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qm_ingress_processed_pd_capture[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qm_cm_processed_pd_capture[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_pool_drop_cnt[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_grp_drop_cnt[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cm_residue_data[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1[]={
            BDMFMON_MAKE_PARM("rnr_cfg_index_1", "rnr_cfg_index_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2[]={
            BDMFMON_MAKE_PARM("rnr_cfg_index_2", "rnr_cfg_index_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_common_configurations_q2rnr[]={
            BDMFMON_MAKE_PARM("q_2_rnr_index", "q_2_rnr_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_debug_counters_unifiedpkt[]={
            BDMFMON_MAKE_PARM("debug_unified_pkt_ctr_idx", "debug_unified_pkt_ctr_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_debug_counters_unifiedbyte[]={
            BDMFMON_MAKE_PARM("debug_unified_byte_ctr_idx", "debug_unified_byte_ctr_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg[]={
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_tx_qm_bbhtx_debug_counters_in_segmentation[]={
            BDMFMON_MAKE_PARM("debug_counters_in_segmentation_byte_ctr_idx", "debug_counters_in_segmentation_byte_ctr_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_num_of_writes[]={
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_num_of_reads[]={
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_u_thresh[]={
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_pri[]={
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_periph_source[]={
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_config_weight[]={
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_debug_req_cnt_rx[]={
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_debug_req_cnt_tx[]={
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_debug_req_cnt_rx_acc[]={
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_debug_req_cnt_tx_acc[]={
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_qm_dma_debug_rddata[]={
            BDMFMON_MAKE_PARM("word_index", "word_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="ddr_cong_ctrl", .val=cli_qm_ddr_cong_ctrl, .parms=set_default },
            { .name="is_queue_not_empty", .val=cli_qm_is_queue_not_empty, .parms=set_is_queue_not_empty },
            { .name="is_queue_pop_ready", .val=cli_qm_is_queue_pop_ready, .parms=set_is_queue_pop_ready },
            { .name="is_queue_full", .val=cli_qm_is_queue_full, .parms=set_is_queue_full },
            { .name="fpm_ug_thr", .val=cli_qm_fpm_ug_thr, .parms=set_fpm_ug_thr },
            { .name="rnr_group_cfg", .val=cli_qm_rnr_group_cfg, .parms=set_rnr_group_cfg },
            { .name="cpu_pd_indirect_wr_data", .val=cli_qm_cpu_pd_indirect_wr_data, .parms=set_cpu_pd_indirect_wr_data },
            { .name="cpu_pd_indirect_rd_data", .val=cli_qm_cpu_pd_indirect_rd_data, .parms=set_cpu_pd_indirect_rd_data },
            { .name="aggr_context", .val=cli_qm_aggr_context, .parms=set_aggr_context },
            { .name="wred_profile_cfg", .val=cli_qm_wred_profile_cfg, .parms=set_wred_profile_cfg },
            { .name="ubus_slave", .val=cli_qm_ubus_slave, .parms=set_default },
            { .name="cfg_src_id", .val=cli_qm_cfg_src_id, .parms=set_default },
            { .name="rnr_src_id", .val=cli_qm_rnr_src_id, .parms=set_default },
            { .name="bbh_dma_cfg", .val=cli_qm_bbh_dma_cfg, .parms=set_default },
            { .name="dma_max_otf_read_request", .val=cli_qm_dma_max_otf_read_request, .parms=set_default },
            { .name="dma_epon_urgent", .val=cli_qm_dma_epon_urgent, .parms=set_default },
            { .name="bbh_sdma_cfg", .val=cli_qm_bbh_sdma_cfg, .parms=set_default },
            { .name="sdma_max_otf_read_request", .val=cli_qm_sdma_max_otf_read_request, .parms=set_default },
            { .name="sdma_epon_urgent", .val=cli_qm_sdma_epon_urgent, .parms=set_default },
            { .name="bbh_ddr_cfg", .val=cli_qm_bbh_ddr_cfg, .parms=set_default },
            { .name="debug_counters", .val=cli_qm_debug_counters, .parms=set_default },
            { .name="debug_info", .val=cli_qm_debug_info, .parms=set_default },
            { .name="enable_ctrl", .val=cli_qm_enable_ctrl, .parms=set_default },
            { .name="reset_ctrl", .val=cli_qm_reset_ctrl, .parms=set_default },
            { .name="drop_counters_ctrl", .val=cli_qm_drop_counters_ctrl, .parms=set_default },
            { .name="fpm_ctrl", .val=cli_qm_fpm_ctrl, .parms=set_default },
            { .name="qm_pd_cong_ctrl", .val=cli_qm_qm_pd_cong_ctrl, .parms=set_default },
            { .name="global_cfg_abs_drop_queue", .val=cli_qm_global_cfg_abs_drop_queue, .parms=set_default },
            { .name="global_cfg_aggregation_ctrl", .val=cli_qm_global_cfg_aggregation_ctrl, .parms=set_default },
            { .name="fpm_base_addr", .val=cli_qm_fpm_base_addr, .parms=set_default },
            { .name="global_cfg_fpm_coherent_base_addr", .val=cli_qm_global_cfg_fpm_coherent_base_addr, .parms=set_default },
            { .name="ddr_sop_offset", .val=cli_qm_ddr_sop_offset, .parms=set_default },
            { .name="epon_overhead_ctrl", .val=cli_qm_epon_overhead_ctrl, .parms=set_default },
            { .name="global_cfg_qm_aggregation_timer_ctrl", .val=cli_qm_global_cfg_qm_aggregation_timer_ctrl, .parms=set_default },
            { .name="global_cfg_qm_fpm_buffer_grp_res", .val=cli_qm_global_cfg_qm_fpm_buffer_grp_res, .parms=set_global_cfg_qm_fpm_buffer_grp_res },
            { .name="global_cfg_qm_fpm_buffer_gbl_thr", .val=cli_qm_global_cfg_qm_fpm_buffer_gbl_thr, .parms=set_default },
            { .name="global_cfg_qm_flow_ctrl_rnr_cfg", .val=cli_qm_global_cfg_qm_flow_ctrl_rnr_cfg, .parms=set_default },
            { .name="global_cfg_qm_flow_ctrl_intr", .val=cli_qm_global_cfg_qm_flow_ctrl_intr, .parms=set_default },
            { .name="global_cfg_qm_fpm_ug_gbl_cnt", .val=cli_qm_global_cfg_qm_fpm_ug_gbl_cnt, .parms=set_default },
            { .name="global_cfg_qm_egress_flush_queue", .val=cli_qm_global_cfg_qm_egress_flush_queue, .parms=set_default },
            { .name="fpm_pool_thr", .val=cli_qm_fpm_pool_thr, .parms=set_fpm_pool_thr },
            { .name="fpm_ug_cnt", .val=cli_qm_fpm_ug_cnt, .parms=set_fpm_ug_cnt },
            { .name="intr_ctrl_isr", .val=cli_qm_intr_ctrl_isr, .parms=set_default },
            { .name="intr_ctrl_ism", .val=cli_qm_intr_ctrl_ism, .parms=set_default },
            { .name="intr_ctrl_ier", .val=cli_qm_intr_ctrl_ier, .parms=set_default },
            { .name="clk_gate_clk_gate_cntrl", .val=cli_qm_clk_gate_clk_gate_cntrl, .parms=set_default },
            { .name="cpu_indr_port_cpu_pd_indirect_ctrl", .val=cli_qm_cpu_indr_port_cpu_pd_indirect_ctrl, .parms=set_cpu_indr_port_cpu_pd_indirect_ctrl },
            { .name="q_context", .val=cli_qm_q_context, .parms=set_q_context },
            { .name="copy_decision_profile", .val=cli_qm_copy_decision_profile, .parms=set_copy_decision_profile },
            { .name="total_valid_cnt", .val=cli_qm_total_valid_cnt, .parms=set_total_valid_cnt },
            { .name="dqm_valid_cnt", .val=cli_qm_dqm_valid_cnt, .parms=set_dqm_valid_cnt },
            { .name="drop_counter", .val=cli_qm_drop_counter, .parms=set_drop_counter },
            { .name="epon_q_byte_cnt", .val=cli_qm_epon_q_byte_cnt, .parms=set_epon_q_byte_cnt },
            { .name="epon_q_status", .val=cli_qm_epon_q_status, .parms=set_epon_q_status },
            { .name="rd_data_pool0", .val=cli_qm_rd_data_pool0, .parms=set_default },
            { .name="rd_data_pool1", .val=cli_qm_rd_data_pool1, .parms=set_default },
            { .name="rd_data_pool2", .val=cli_qm_rd_data_pool2, .parms=set_default },
            { .name="rd_data_pool3", .val=cli_qm_rd_data_pool3, .parms=set_default },
            { .name="pdfifo_ptr", .val=cli_qm_pdfifo_ptr, .parms=set_pdfifo_ptr },
            { .name="update_fifo_ptr", .val=cli_qm_update_fifo_ptr, .parms=set_update_fifo_ptr },
            { .name="debug_sel", .val=cli_qm_debug_sel, .parms=set_default },
            { .name="debug_bus_lsb", .val=cli_qm_debug_bus_lsb, .parms=set_default },
            { .name="debug_bus_msb", .val=cli_qm_debug_bus_msb, .parms=set_default },
            { .name="qm_spare_config", .val=cli_qm_qm_spare_config, .parms=set_default },
            { .name="good_lvl1_pkts_cnt", .val=cli_qm_good_lvl1_pkts_cnt, .parms=set_default },
            { .name="good_lvl1_bytes_cnt", .val=cli_qm_good_lvl1_bytes_cnt, .parms=set_default },
            { .name="good_lvl2_pkts_cnt", .val=cli_qm_good_lvl2_pkts_cnt, .parms=set_default },
            { .name="good_lvl2_bytes_cnt", .val=cli_qm_good_lvl2_bytes_cnt, .parms=set_default },
            { .name="copied_pkts_cnt", .val=cli_qm_copied_pkts_cnt, .parms=set_default },
            { .name="copied_bytes_cnt", .val=cli_qm_copied_bytes_cnt, .parms=set_default },
            { .name="agg_pkts_cnt", .val=cli_qm_agg_pkts_cnt, .parms=set_default },
            { .name="agg_bytes_cnt", .val=cli_qm_agg_bytes_cnt, .parms=set_default },
            { .name="agg_1_pkts_cnt", .val=cli_qm_agg_1_pkts_cnt, .parms=set_default },
            { .name="agg_2_pkts_cnt", .val=cli_qm_agg_2_pkts_cnt, .parms=set_default },
            { .name="agg_3_pkts_cnt", .val=cli_qm_agg_3_pkts_cnt, .parms=set_default },
            { .name="agg_4_pkts_cnt", .val=cli_qm_agg_4_pkts_cnt, .parms=set_default },
            { .name="wred_drop_cnt", .val=cli_qm_wred_drop_cnt, .parms=set_default },
            { .name="fpm_congestion_drop_cnt", .val=cli_qm_fpm_congestion_drop_cnt, .parms=set_default },
            { .name="ddr_pd_congestion_drop_cnt", .val=cli_qm_ddr_pd_congestion_drop_cnt, .parms=set_default },
            { .name="ddr_byte_congestion_drop_cnt", .val=cli_qm_ddr_byte_congestion_drop_cnt, .parms=set_default },
            { .name="qm_pd_congestion_drop_cnt", .val=cli_qm_qm_pd_congestion_drop_cnt, .parms=set_default },
            { .name="qm_abs_requeue_cnt", .val=cli_qm_qm_abs_requeue_cnt, .parms=set_default },
            { .name="fpm_prefetch_fifo0_status", .val=cli_qm_fpm_prefetch_fifo0_status, .parms=set_default },
            { .name="fpm_prefetch_fifo1_status", .val=cli_qm_fpm_prefetch_fifo1_status, .parms=set_default },
            { .name="fpm_prefetch_fifo2_status", .val=cli_qm_fpm_prefetch_fifo2_status, .parms=set_default },
            { .name="fpm_prefetch_fifo3_status", .val=cli_qm_fpm_prefetch_fifo3_status, .parms=set_default },
            { .name="normal_rmt_fifo_status", .val=cli_qm_normal_rmt_fifo_status, .parms=set_default },
            { .name="non_delayed_rmt_fifo_status", .val=cli_qm_non_delayed_rmt_fifo_status, .parms=set_default },
            { .name="non_delayed_out_fifo_status", .val=cli_qm_non_delayed_out_fifo_status, .parms=set_default },
            { .name="pre_cm_fifo_status", .val=cli_qm_pre_cm_fifo_status, .parms=set_default },
            { .name="cm_rd_pd_fifo_status", .val=cli_qm_cm_rd_pd_fifo_status, .parms=set_default },
            { .name="cm_wr_pd_fifo_status", .val=cli_qm_cm_wr_pd_fifo_status, .parms=set_default },
            { .name="cm_common_input_fifo_status", .val=cli_qm_cm_common_input_fifo_status, .parms=set_default },
            { .name="bb0_output_fifo_status", .val=cli_qm_bb0_output_fifo_status, .parms=set_default },
            { .name="bb1_output_fifo_status", .val=cli_qm_bb1_output_fifo_status, .parms=set_default },
            { .name="bb1_input_fifo_status", .val=cli_qm_bb1_input_fifo_status, .parms=set_default },
            { .name="egress_data_fifo_status", .val=cli_qm_egress_data_fifo_status, .parms=set_default },
            { .name="egress_rr_fifo_status", .val=cli_qm_egress_rr_fifo_status, .parms=set_default },
            { .name="bb_route_ovr", .val=cli_qm_bb_route_ovr, .parms=set_bb_route_ovr },
            { .name="ingress_stat", .val=cli_qm_ingress_stat, .parms=set_default },
            { .name="egress_stat", .val=cli_qm_egress_stat, .parms=set_default },
            { .name="cm_stat", .val=cli_qm_cm_stat, .parms=set_default },
            { .name="fpm_prefetch_stat", .val=cli_qm_fpm_prefetch_stat, .parms=set_default },
            { .name="qm_connect_ack_counter", .val=cli_qm_qm_connect_ack_counter, .parms=set_default },
            { .name="qm_ddr_wr_reply_counter", .val=cli_qm_qm_ddr_wr_reply_counter, .parms=set_default },
            { .name="qm_ddr_pipe_byte_counter", .val=cli_qm_qm_ddr_pipe_byte_counter, .parms=set_default },
            { .name="qm_abs_requeue_valid_counter", .val=cli_qm_qm_abs_requeue_valid_counter, .parms=set_default },
            { .name="qm_illegal_pd_capture", .val=cli_qm_qm_illegal_pd_capture, .parms=set_qm_illegal_pd_capture },
            { .name="qm_ingress_processed_pd_capture", .val=cli_qm_qm_ingress_processed_pd_capture, .parms=set_qm_ingress_processed_pd_capture },
            { .name="qm_cm_processed_pd_capture", .val=cli_qm_qm_cm_processed_pd_capture, .parms=set_qm_cm_processed_pd_capture },
            { .name="fpm_pool_drop_cnt", .val=cli_qm_fpm_pool_drop_cnt, .parms=set_fpm_pool_drop_cnt },
            { .name="fpm_grp_drop_cnt", .val=cli_qm_fpm_grp_drop_cnt, .parms=set_fpm_grp_drop_cnt },
            { .name="fpm_buffer_res_drop_cnt", .val=cli_qm_fpm_buffer_res_drop_cnt, .parms=set_default },
            { .name="psram_egress_cong_drp_cnt", .val=cli_qm_psram_egress_cong_drp_cnt, .parms=set_default },
            { .name="cm_residue_data", .val=cli_qm_cm_residue_data, .parms=set_cm_residue_data },
            { .name="bbh_tx_qm_bbhtx_common_configurations_mactype", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_mactype, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1, .parms=set_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1 },
            { .name="bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2, .parms=set_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2 },
            { .name="bbh_tx_qm_bbhtx_common_configurations_sbpmcfg", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel, .parms=set_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel },
            { .name="bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh, .parms=set_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh },
            { .name="bbh_tx_qm_bbhtx_common_configurations_dfifoctrl", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_common_configurations_arb_cfg", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_arb_cfg, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_common_configurations_bbroute", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_bbroute, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_common_configurations_q2rnr", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_q2rnr, .parms=set_bbh_tx_qm_bbhtx_common_configurations_q2rnr },
            { .name="bbh_tx_qm_bbhtx_common_configurations_perqtask", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_perqtask, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_common_configurations_txrstcmd", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_common_configurations_dbgsel", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_dbgsel, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_common_configurations_gpr", .val=cli_qm_bbh_tx_qm_bbhtx_common_configurations_gpr, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_pdbase", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdbase, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_pdsize", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdsize, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_pdwkuph", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_pdempty", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdempty, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_txthresh", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_txthresh, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_eee", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_eee, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_lan_configurations_ts", .val=cli_qm_bbh_tx_qm_bbhtx_lan_configurations_ts, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_debug_counters_srambyte", .val=cli_qm_bbh_tx_qm_bbhtx_debug_counters_srambyte, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_debug_counters_ddrbyte", .val=cli_qm_bbh_tx_qm_bbhtx_debug_counters_ddrbyte, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_debug_counters_swrden", .val=cli_qm_bbh_tx_qm_bbhtx_debug_counters_swrden, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_debug_counters_swrdaddr", .val=cli_qm_bbh_tx_qm_bbhtx_debug_counters_swrdaddr, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_debug_counters_swrddata", .val=cli_qm_bbh_tx_qm_bbhtx_debug_counters_swrddata, .parms=set_default },
            { .name="bbh_tx_qm_bbhtx_debug_counters_unifiedpkt", .val=cli_qm_bbh_tx_qm_bbhtx_debug_counters_unifiedpkt, .parms=set_bbh_tx_qm_bbhtx_debug_counters_unifiedpkt },
            { .name="bbh_tx_qm_bbhtx_debug_counters_unifiedbyte", .val=cli_qm_bbh_tx_qm_bbhtx_debug_counters_unifiedbyte, .parms=set_bbh_tx_qm_bbhtx_debug_counters_unifiedbyte },
            { .name="bbh_tx_qm_bbhtx_debug_counters_dbgoutreg", .val=cli_qm_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg, .parms=set_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg },
            { .name="bbh_tx_qm_bbhtx_debug_counters_in_segmentation", .val=cli_qm_bbh_tx_qm_bbhtx_debug_counters_in_segmentation, .parms=set_bbh_tx_qm_bbhtx_debug_counters_in_segmentation },
            { .name="dma_qm_dma_config_bbrouteovrd", .val=cli_qm_dma_qm_dma_config_bbrouteovrd, .parms=set_default },
            { .name="dma_qm_dma_config_num_of_writes", .val=cli_qm_dma_qm_dma_config_num_of_writes, .parms=set_dma_qm_dma_config_num_of_writes },
            { .name="dma_qm_dma_config_num_of_reads", .val=cli_qm_dma_qm_dma_config_num_of_reads, .parms=set_dma_qm_dma_config_num_of_reads },
            { .name="dma_qm_dma_config_u_thresh", .val=cli_qm_dma_qm_dma_config_u_thresh, .parms=set_dma_qm_dma_config_u_thresh },
            { .name="dma_qm_dma_config_pri", .val=cli_qm_dma_qm_dma_config_pri, .parms=set_dma_qm_dma_config_pri },
            { .name="dma_qm_dma_config_periph_source", .val=cli_qm_dma_qm_dma_config_periph_source, .parms=set_dma_qm_dma_config_periph_source },
            { .name="dma_qm_dma_config_weight", .val=cli_qm_dma_qm_dma_config_weight, .parms=set_dma_qm_dma_config_weight },
            { .name="dma_qm_dma_config_ptrrst", .val=cli_qm_dma_qm_dma_config_ptrrst, .parms=set_default },
            { .name="dma_qm_dma_config_max_otf", .val=cli_qm_dma_qm_dma_config_max_otf, .parms=set_default },
            { .name="dma_qm_dma_config_clk_gate_cntrl", .val=cli_qm_dma_qm_dma_config_clk_gate_cntrl, .parms=set_default },
            { .name="dma_qm_dma_config_dbg_sel", .val=cli_qm_dma_qm_dma_config_dbg_sel, .parms=set_default },
            { .name="dma_qm_dma_debug_req_cnt_rx", .val=cli_qm_dma_qm_dma_debug_req_cnt_rx, .parms=set_dma_qm_dma_debug_req_cnt_rx },
            { .name="dma_qm_dma_debug_req_cnt_tx", .val=cli_qm_dma_qm_dma_debug_req_cnt_tx, .parms=set_dma_qm_dma_debug_req_cnt_tx },
            { .name="dma_qm_dma_debug_req_cnt_rx_acc", .val=cli_qm_dma_qm_dma_debug_req_cnt_rx_acc, .parms=set_dma_qm_dma_debug_req_cnt_rx_acc },
            { .name="dma_qm_dma_debug_req_cnt_tx_acc", .val=cli_qm_dma_qm_dma_debug_req_cnt_tx_acc, .parms=set_dma_qm_dma_debug_req_cnt_tx_acc },
            { .name="dma_qm_dma_debug_rddata", .val=cli_qm_dma_qm_dma_debug_rddata, .parms=set_dma_qm_dma_debug_rddata },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_qm_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_qm_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="GLOBAL_CFG_QM_ENABLE_CTRL" , .val=bdmf_address_global_cfg_qm_enable_ctrl },
            { .name="GLOBAL_CFG_QM_SW_RST_CTRL" , .val=bdmf_address_global_cfg_qm_sw_rst_ctrl },
            { .name="GLOBAL_CFG_QM_GENERAL_CTRL" , .val=bdmf_address_global_cfg_qm_general_ctrl },
            { .name="GLOBAL_CFG_FPM_CONTROL" , .val=bdmf_address_global_cfg_fpm_control },
            { .name="GLOBAL_CFG_DDR_BYTE_CONGESTION_CONTROL" , .val=bdmf_address_global_cfg_ddr_byte_congestion_control },
            { .name="GLOBAL_CFG_DDR_BYTE_CONGESTION_LOWER_THR" , .val=bdmf_address_global_cfg_ddr_byte_congestion_lower_thr },
            { .name="GLOBAL_CFG_DDR_BYTE_CONGESTION_MID_THR" , .val=bdmf_address_global_cfg_ddr_byte_congestion_mid_thr },
            { .name="GLOBAL_CFG_DDR_BYTE_CONGESTION_HIGHER_THR" , .val=bdmf_address_global_cfg_ddr_byte_congestion_higher_thr },
            { .name="GLOBAL_CFG_DDR_PD_CONGESTION_CONTROL" , .val=bdmf_address_global_cfg_ddr_pd_congestion_control },
            { .name="GLOBAL_CFG_QM_PD_CONGESTION_CONTROL" , .val=bdmf_address_global_cfg_qm_pd_congestion_control },
            { .name="GLOBAL_CFG_ABS_DROP_QUEUE" , .val=bdmf_address_global_cfg_abs_drop_queue },
            { .name="GLOBAL_CFG_AGGREGATION_CTRL" , .val=bdmf_address_global_cfg_aggregation_ctrl },
            { .name="GLOBAL_CFG_FPM_BASE_ADDR" , .val=bdmf_address_global_cfg_fpm_base_addr },
            { .name="GLOBAL_CFG_FPM_COHERENT_BASE_ADDR" , .val=bdmf_address_global_cfg_fpm_coherent_base_addr },
            { .name="GLOBAL_CFG_DDR_SOP_OFFSET" , .val=bdmf_address_global_cfg_ddr_sop_offset },
            { .name="GLOBAL_CFG_EPON_OVERHEAD_CTRL" , .val=bdmf_address_global_cfg_epon_overhead_ctrl },
            { .name="GLOBAL_CFG_DQM_FULL" , .val=bdmf_address_global_cfg_dqm_full },
            { .name="GLOBAL_CFG_DQM_NOT_EMPTY" , .val=bdmf_address_global_cfg_dqm_not_empty },
            { .name="GLOBAL_CFG_DQM_POP_READY" , .val=bdmf_address_global_cfg_dqm_pop_ready },
            { .name="GLOBAL_CFG_AGGREGATION_CONTEXT_VALID" , .val=bdmf_address_global_cfg_aggregation_context_valid },
            { .name="GLOBAL_CFG_QM_AGGREGATION_TIMER_CTRL" , .val=bdmf_address_global_cfg_qm_aggregation_timer_ctrl },
            { .name="GLOBAL_CFG_QM_FPM_BUFFER_GRP_RES" , .val=bdmf_address_global_cfg_qm_fpm_buffer_grp_res },
            { .name="GLOBAL_CFG_QM_FPM_BUFFER_GBL_THR" , .val=bdmf_address_global_cfg_qm_fpm_buffer_gbl_thr },
            { .name="GLOBAL_CFG_QM_FLOW_CTRL_RNR_CFG" , .val=bdmf_address_global_cfg_qm_flow_ctrl_rnr_cfg },
            { .name="GLOBAL_CFG_QM_FLOW_CTRL_INTR" , .val=bdmf_address_global_cfg_qm_flow_ctrl_intr },
            { .name="GLOBAL_CFG_QM_FPM_UG_GBL_CNT" , .val=bdmf_address_global_cfg_qm_fpm_ug_gbl_cnt },
            { .name="GLOBAL_CFG_QM_EGRESS_FLUSH_QUEUE" , .val=bdmf_address_global_cfg_qm_egress_flush_queue },
            { .name="FPM_POOLS_THR" , .val=bdmf_address_fpm_pools_thr },
            { .name="FPM_USR_GRP_LOWER_THR" , .val=bdmf_address_fpm_usr_grp_lower_thr },
            { .name="FPM_USR_GRP_MID_THR" , .val=bdmf_address_fpm_usr_grp_mid_thr },
            { .name="FPM_USR_GRP_HIGHER_THR" , .val=bdmf_address_fpm_usr_grp_higher_thr },
            { .name="FPM_USR_GRP_CNT" , .val=bdmf_address_fpm_usr_grp_cnt },
            { .name="RUNNER_GRP_RNR_CONFIG" , .val=bdmf_address_runner_grp_rnr_config },
            { .name="RUNNER_GRP_QUEUE_CONFIG" , .val=bdmf_address_runner_grp_queue_config },
            { .name="RUNNER_GRP_PDFIFO_CONFIG" , .val=bdmf_address_runner_grp_pdfifo_config },
            { .name="RUNNER_GRP_UPDATE_FIFO_CONFIG" , .val=bdmf_address_runner_grp_update_fifo_config },
            { .name="INTR_CTRL_ISR" , .val=bdmf_address_intr_ctrl_isr },
            { .name="INTR_CTRL_ISM" , .val=bdmf_address_intr_ctrl_ism },
            { .name="INTR_CTRL_IER" , .val=bdmf_address_intr_ctrl_ier },
            { .name="INTR_CTRL_ITR" , .val=bdmf_address_intr_ctrl_itr },
            { .name="CLK_GATE_CLK_GATE_CNTRL" , .val=bdmf_address_clk_gate_clk_gate_cntrl },
            { .name="CPU_INDR_PORT_CPU_PD_INDIRECT_CTRL" , .val=bdmf_address_cpu_indr_port_cpu_pd_indirect_ctrl },
            { .name="CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_0" , .val=bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_0 },
            { .name="CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_1" , .val=bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_1 },
            { .name="CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_2" , .val=bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_2 },
            { .name="CPU_INDR_PORT_CPU_PD_INDIRECT_WR_DATA_3" , .val=bdmf_address_cpu_indr_port_cpu_pd_indirect_wr_data_3 },
            { .name="CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_0" , .val=bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_0 },
            { .name="CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_1" , .val=bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_1 },
            { .name="CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_2" , .val=bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_2 },
            { .name="CPU_INDR_PORT_CPU_PD_INDIRECT_RD_DATA_3" , .val=bdmf_address_cpu_indr_port_cpu_pd_indirect_rd_data_3 },
            { .name="QUEUE_CONTEXT_CONTEXT" , .val=bdmf_address_queue_context_context },
            { .name="WRED_PROFILE_COLOR_MIN_THR_0" , .val=bdmf_address_wred_profile_color_min_thr_0 },
            { .name="WRED_PROFILE_COLOR_MIN_THR_1" , .val=bdmf_address_wred_profile_color_min_thr_1 },
            { .name="WRED_PROFILE_COLOR_MAX_THR_0" , .val=bdmf_address_wred_profile_color_max_thr_0 },
            { .name="WRED_PROFILE_COLOR_MAX_THR_1" , .val=bdmf_address_wred_profile_color_max_thr_1 },
            { .name="WRED_PROFILE_COLOR_SLOPE_0" , .val=bdmf_address_wred_profile_color_slope_0 },
            { .name="WRED_PROFILE_COLOR_SLOPE_1" , .val=bdmf_address_wred_profile_color_slope_1 },
            { .name="COPY_DECISION_PROFILE_THR" , .val=bdmf_address_copy_decision_profile_thr },
            { .name="TOTAL_VALID_COUNTER_COUNTER" , .val=bdmf_address_total_valid_counter_counter },
            { .name="DQM_VALID_COUNTER_COUNTER" , .val=bdmf_address_dqm_valid_counter_counter },
            { .name="DROP_COUNTER_COUNTER" , .val=bdmf_address_drop_counter_counter },
            { .name="EPON_RPT_CNT_COUNTER" , .val=bdmf_address_epon_rpt_cnt_counter },
            { .name="EPON_RPT_CNT_QUEUE_STATUS" , .val=bdmf_address_epon_rpt_cnt_queue_status },
            { .name="RD_DATA_POOL0" , .val=bdmf_address_rd_data_pool0 },
            { .name="RD_DATA_POOL1" , .val=bdmf_address_rd_data_pool1 },
            { .name="RD_DATA_POOL2" , .val=bdmf_address_rd_data_pool2 },
            { .name="RD_DATA_POOL3" , .val=bdmf_address_rd_data_pool3 },
            { .name="PDFIFO_PTR" , .val=bdmf_address_pdfifo_ptr },
            { .name="UPDATE_FIFO_PTR" , .val=bdmf_address_update_fifo_ptr },
            { .name="DEBUG_SEL" , .val=bdmf_address_debug_sel },
            { .name="DEBUG_BUS_LSB" , .val=bdmf_address_debug_bus_lsb },
            { .name="DEBUG_BUS_MSB" , .val=bdmf_address_debug_bus_msb },
            { .name="QM_SPARE_CONFIG" , .val=bdmf_address_qm_spare_config },
            { .name="GOOD_LVL1_PKTS_CNT" , .val=bdmf_address_good_lvl1_pkts_cnt },
            { .name="GOOD_LVL1_BYTES_CNT" , .val=bdmf_address_good_lvl1_bytes_cnt },
            { .name="GOOD_LVL2_PKTS_CNT" , .val=bdmf_address_good_lvl2_pkts_cnt },
            { .name="GOOD_LVL2_BYTES_CNT" , .val=bdmf_address_good_lvl2_bytes_cnt },
            { .name="COPIED_PKTS_CNT" , .val=bdmf_address_copied_pkts_cnt },
            { .name="COPIED_BYTES_CNT" , .val=bdmf_address_copied_bytes_cnt },
            { .name="AGG_PKTS_CNT" , .val=bdmf_address_agg_pkts_cnt },
            { .name="AGG_BYTES_CNT" , .val=bdmf_address_agg_bytes_cnt },
            { .name="AGG_1_PKTS_CNT" , .val=bdmf_address_agg_1_pkts_cnt },
            { .name="AGG_2_PKTS_CNT" , .val=bdmf_address_agg_2_pkts_cnt },
            { .name="AGG_3_PKTS_CNT" , .val=bdmf_address_agg_3_pkts_cnt },
            { .name="AGG_4_PKTS_CNT" , .val=bdmf_address_agg_4_pkts_cnt },
            { .name="WRED_DROP_CNT" , .val=bdmf_address_wred_drop_cnt },
            { .name="FPM_CONGESTION_DROP_CNT" , .val=bdmf_address_fpm_congestion_drop_cnt },
            { .name="DDR_PD_CONGESTION_DROP_CNT" , .val=bdmf_address_ddr_pd_congestion_drop_cnt },
            { .name="DDR_BYTE_CONGESTION_DROP_CNT" , .val=bdmf_address_ddr_byte_congestion_drop_cnt },
            { .name="QM_PD_CONGESTION_DROP_CNT" , .val=bdmf_address_qm_pd_congestion_drop_cnt },
            { .name="QM_ABS_REQUEUE_CNT" , .val=bdmf_address_qm_abs_requeue_cnt },
            { .name="FPM_PREFETCH_FIFO0_STATUS" , .val=bdmf_address_fpm_prefetch_fifo0_status },
            { .name="FPM_PREFETCH_FIFO1_STATUS" , .val=bdmf_address_fpm_prefetch_fifo1_status },
            { .name="FPM_PREFETCH_FIFO2_STATUS" , .val=bdmf_address_fpm_prefetch_fifo2_status },
            { .name="FPM_PREFETCH_FIFO3_STATUS" , .val=bdmf_address_fpm_prefetch_fifo3_status },
            { .name="NORMAL_RMT_FIFO_STATUS" , .val=bdmf_address_normal_rmt_fifo_status },
            { .name="NON_DELAYED_RMT_FIFO_STATUS" , .val=bdmf_address_non_delayed_rmt_fifo_status },
            { .name="NON_DELAYED_OUT_FIFO_STATUS" , .val=bdmf_address_non_delayed_out_fifo_status },
            { .name="PRE_CM_FIFO_STATUS" , .val=bdmf_address_pre_cm_fifo_status },
            { .name="CM_RD_PD_FIFO_STATUS" , .val=bdmf_address_cm_rd_pd_fifo_status },
            { .name="CM_WR_PD_FIFO_STATUS" , .val=bdmf_address_cm_wr_pd_fifo_status },
            { .name="CM_COMMON_INPUT_FIFO_STATUS" , .val=bdmf_address_cm_common_input_fifo_status },
            { .name="BB0_OUTPUT_FIFO_STATUS" , .val=bdmf_address_bb0_output_fifo_status },
            { .name="BB1_OUTPUT_FIFO_STATUS" , .val=bdmf_address_bb1_output_fifo_status },
            { .name="BB1_INPUT_FIFO_STATUS" , .val=bdmf_address_bb1_input_fifo_status },
            { .name="EGRESS_DATA_FIFO_STATUS" , .val=bdmf_address_egress_data_fifo_status },
            { .name="EGRESS_RR_FIFO_STATUS" , .val=bdmf_address_egress_rr_fifo_status },
            { .name="BB_ROUTE_OVR" , .val=bdmf_address_bb_route_ovr },
            { .name="QM_INGRESS_STAT" , .val=bdmf_address_qm_ingress_stat },
            { .name="QM_EGRESS_STAT" , .val=bdmf_address_qm_egress_stat },
            { .name="QM_CM_STAT" , .val=bdmf_address_qm_cm_stat },
            { .name="QM_FPM_PREFETCH_STAT" , .val=bdmf_address_qm_fpm_prefetch_stat },
            { .name="QM_CONNECT_ACK_COUNTER" , .val=bdmf_address_qm_connect_ack_counter },
            { .name="QM_DDR_WR_REPLY_COUNTER" , .val=bdmf_address_qm_ddr_wr_reply_counter },
            { .name="QM_DDR_PIPE_BYTE_COUNTER" , .val=bdmf_address_qm_ddr_pipe_byte_counter },
            { .name="QM_ABS_REQUEUE_VALID_COUNTER" , .val=bdmf_address_qm_abs_requeue_valid_counter },
            { .name="QM_ILLEGAL_PD_CAPTURE" , .val=bdmf_address_qm_illegal_pd_capture },
            { .name="QM_INGRESS_PROCESSED_PD_CAPTURE" , .val=bdmf_address_qm_ingress_processed_pd_capture },
            { .name="QM_CM_PROCESSED_PD_CAPTURE" , .val=bdmf_address_qm_cm_processed_pd_capture },
            { .name="FPM_POOL_DROP_CNT" , .val=bdmf_address_fpm_pool_drop_cnt },
            { .name="FPM_GRP_DROP_CNT" , .val=bdmf_address_fpm_grp_drop_cnt },
            { .name="FPM_BUFFER_RES_DROP_CNT" , .val=bdmf_address_fpm_buffer_res_drop_cnt },
            { .name="PSRAM_EGRESS_CONG_DRP_CNT" , .val=bdmf_address_psram_egress_cong_drp_cnt },
            { .name="DATA" , .val=bdmf_address_data },
            { .name="XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_BASE" , .val=bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_base },
            { .name="XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_VPB_MASK" , .val=bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_vpb_mask },
            { .name="XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_BASE" , .val=bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_base },
            { .name="XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_APB_MASK" , .val=bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_apb_mask },
            { .name="XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_BASE" , .val=bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_base },
            { .name="XRDP_UBUS_TOP_QM_XRDP_UBUS_SLV_DQM_MASK" , .val=bdmf_address_xrdp_ubus_top_qm_xrdp_ubus_slv_dqm_mask },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_MACTYPE" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_mactype },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_1_TX" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_bbcfg_1_tx },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBCFG_2_TX" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_bbcfg_2_tx },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRCFG_TX" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_ddrcfg_tx },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_1" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1 },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_RNRCFG_2" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2 },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DMACFG_TX" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_dmacfg_tx },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SDMACFG_TX" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_sdmacfg_tx },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_SBPMCFG" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEL" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DDRTMBASEH" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DFIFOCTRL" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_ARB_CFG" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_arb_cfg },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_BBROUTE" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_bbroute },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_Q2RNR" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_q2rnr },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_PERQTASK" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_perqtask },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_TXRSTCMD" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_txrstcmd },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_DBGSEL" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_dbgsel },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_CLK_GATE_CNTRL" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl },
            { .name="BBH_TX_QM_BBHTX_COMMON_CONFIGURATIONS_GPR" , .val=bdmf_address_bbh_tx_qm_bbhtx_common_configurations_gpr },
            { .name="BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDBASE" , .val=bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pdbase },
            { .name="BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDSIZE" , .val=bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pdsize },
            { .name="BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDWKUPH" , .val=bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph },
            { .name="BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH" , .val=bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th },
            { .name="BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PD_BYTE_TH_EN" , .val=bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en },
            { .name="BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_PDEMPTY" , .val=bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_pdempty },
            { .name="BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TXTHRESH" , .val=bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_txthresh },
            { .name="BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_EEE" , .val=bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_eee },
            { .name="BBH_TX_QM_BBHTX_LAN_CONFIGURATIONS_TS" , .val=bdmf_address_bbh_tx_qm_bbhtx_lan_configurations_ts },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPD" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_srampd },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPD" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_ddrpd },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_PDDROP" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_pddrop },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSCNT" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_stscnt },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_STSDROP" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_stsdrop },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGCNT" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_msgcnt },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_MSGDROP" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_msgdrop },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_GETNEXTNULL" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_getnextnull },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_FLUSHPKTS" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_flushpkts },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_LENERR" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_lenerr },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_AGGRLENERR" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_aggrlenerr },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMPKT" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_srampkt },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRPKT" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_ddrpkt },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SRAMBYTE" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_srambyte },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DDRBYTE" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_ddrbyte },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDEN" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_swrden },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDADDR" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_swrdaddr },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_SWRDDATA" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_swrddata },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDPKT" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_unifiedpkt },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_UNIFIEDBYTE" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_unifiedbyte },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_DBGOUTREG" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg },
            { .name="BBH_TX_QM_BBHTX_DEBUG_COUNTERS_IN_SEGMENTATION" , .val=bdmf_address_bbh_tx_qm_bbhtx_debug_counters_in_segmentation },
            { .name="DMA_QM_DMA_CONFIG_BBROUTEOVRD" , .val=bdmf_address_dma_qm_dma_config_bbrouteovrd },
            { .name="DMA_QM_DMA_CONFIG_NUM_OF_WRITES" , .val=bdmf_address_dma_qm_dma_config_num_of_writes },
            { .name="DMA_QM_DMA_CONFIG_NUM_OF_READS" , .val=bdmf_address_dma_qm_dma_config_num_of_reads },
            { .name="DMA_QM_DMA_CONFIG_U_THRESH" , .val=bdmf_address_dma_qm_dma_config_u_thresh },
            { .name="DMA_QM_DMA_CONFIG_PRI" , .val=bdmf_address_dma_qm_dma_config_pri },
            { .name="DMA_QM_DMA_CONFIG_PERIPH_SOURCE" , .val=bdmf_address_dma_qm_dma_config_periph_source },
            { .name="DMA_QM_DMA_CONFIG_WEIGHT" , .val=bdmf_address_dma_qm_dma_config_weight },
            { .name="DMA_QM_DMA_CONFIG_PTRRST" , .val=bdmf_address_dma_qm_dma_config_ptrrst },
            { .name="DMA_QM_DMA_CONFIG_MAX_OTF" , .val=bdmf_address_dma_qm_dma_config_max_otf },
            { .name="DMA_QM_DMA_CONFIG_CLK_GATE_CNTRL" , .val=bdmf_address_dma_qm_dma_config_clk_gate_cntrl },
            { .name="DMA_QM_DMA_CONFIG_DBG_SEL" , .val=bdmf_address_dma_qm_dma_config_dbg_sel },
            { .name="DMA_QM_DMA_DEBUG_NEMPTY" , .val=bdmf_address_dma_qm_dma_debug_nempty },
            { .name="DMA_QM_DMA_DEBUG_URGNT" , .val=bdmf_address_dma_qm_dma_debug_urgnt },
            { .name="DMA_QM_DMA_DEBUG_SELSRC" , .val=bdmf_address_dma_qm_dma_debug_selsrc },
            { .name="DMA_QM_DMA_DEBUG_REQ_CNT_RX" , .val=bdmf_address_dma_qm_dma_debug_req_cnt_rx },
            { .name="DMA_QM_DMA_DEBUG_REQ_CNT_TX" , .val=bdmf_address_dma_qm_dma_debug_req_cnt_tx },
            { .name="DMA_QM_DMA_DEBUG_REQ_CNT_RX_ACC" , .val=bdmf_address_dma_qm_dma_debug_req_cnt_rx_acc },
            { .name="DMA_QM_DMA_DEBUG_REQ_CNT_TX_ACC" , .val=bdmf_address_dma_qm_dma_debug_req_cnt_tx_acc },
            { .name="DMA_QM_DMA_DEBUG_RDADD" , .val=bdmf_address_dma_qm_dma_debug_rdadd },
            { .name="DMA_QM_DMA_DEBUG_RDVALID" , .val=bdmf_address_dma_qm_dma_debug_rdvalid },
            { .name="DMA_QM_DMA_DEBUG_RDDATA" , .val=bdmf_address_dma_qm_dma_debug_rddata },
            { .name="DMA_QM_DMA_DEBUG_RDDATARDY" , .val=bdmf_address_dma_qm_dma_debug_rddatardy },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_qm_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

