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
       (drop_counters_ctrl->gpon_dbr_ceil >= _1BITS_MAX_VAL_) ||
       (drop_counters_ctrl->drop_cnt_wred_drops >= _1BITS_MAX_VAL_))
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
    reg_global_cfg_qm_general_ctrl = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DROP_CNT_WRED_DROPS, reg_global_cfg_qm_general_ctrl, drop_counters_ctrl->drop_cnt_wred_drops);

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
    drop_counters_ctrl->drop_cnt_wred_drops = RU_FIELD_GET(0, QM, GLOBAL_CFG_QM_GENERAL_CTRL, DROP_CNT_WRED_DROPS, reg_global_cfg_qm_general_ctrl);

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

bdmf_error_t ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set(uint8_t prescaler_granularity, uint8_t aggregation_timeout_value, bdmf_boolean pd_occupancy_en, uint8_t pd_occupancy_value)
{
    uint32_t reg_global_cfg_qm_aggregation_timer_ctrl=0;

#ifdef VALIDATE_PARMS
    if((prescaler_granularity >= _3BITS_MAX_VAL_) ||
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
    if(!prescaler_granularity || !aggregation_timeout_value || !pd_occupancy_en || !pd_occupancy_value)
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

bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_set(uint16_t fpm_gbl_cnt)
{
    uint32_t reg_global_cfg_qm_fpm_ug_gbl_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_global_cfg_qm_fpm_ug_gbl_cnt = RU_FIELD_SET(0, QM, GLOBAL_CFG_QM_FPM_UG_GBL_CNT, FPM_GBL_CNT, reg_global_cfg_qm_fpm_ug_gbl_cnt, fpm_gbl_cnt);

    RU_REG_WRITE(0, QM, GLOBAL_CFG_QM_FPM_UG_GBL_CNT, reg_global_cfg_qm_fpm_ug_gbl_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_get(uint16_t *fpm_gbl_cnt)
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
       (intr_ctrl_isr->qm_timer_wraparound >= _1BITS_MAX_VAL_) ||
       (intr_ctrl_isr->qm_copy_plen_zero >= _1BITS_MAX_VAL_))
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
    intr_ctrl_isr->qm_copy_plen_zero = RU_FIELD_GET(0, QM, INTR_CTRL_ISR, QM_COPY_PLEN_ZERO, reg_intr_ctrl_isr);

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
    if((iem >= _23BITS_MAX_VAL_))
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
    if((ist >= _23BITS_MAX_VAL_))
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
    if(!ist)
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
    if((q_idx >= 128) ||
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
    if((q_idx >= 128))
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
    if((q_idx >= 512))
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
    if((q_idx >= 512))
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
    if((q_idx >= 256))
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
    if((q_idx >= 256))
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
    if((q_idx >= 256))
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
    if((q_idx >= 256))
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
    if((q_idx >= 256))
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
    if((q_idx >= 4))
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
    if((q_idx >= 128))
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

bdmf_error_t ag_drv_qm_fpm_buffer_reservation_data_set(uint32_t idx, uint16_t data)
{
    uint32_t reg_fpm_buffer_reservation_data=0;

#ifdef VALIDATE_PARMS
    if((idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fpm_buffer_reservation_data = RU_FIELD_SET(0, QM, FPM_BUFFER_RESERVATION_DATA, DATA, reg_fpm_buffer_reservation_data, data);

    RU_REG_RAM_WRITE(0, idx, QM, FPM_BUFFER_RESERVATION_DATA, reg_fpm_buffer_reservation_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_fpm_buffer_reservation_data_get(uint32_t idx, uint16_t *data)
{
    uint32_t reg_fpm_buffer_reservation_data;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, QM, FPM_BUFFER_RESERVATION_DATA, reg_fpm_buffer_reservation_data);

    *data = RU_FIELD_GET(0, QM, FPM_BUFFER_RESERVATION_DATA, DATA, reg_fpm_buffer_reservation_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_flow_ctrl_ug_ctrl_set(bdmf_boolean flow_ctrl_ug0_en, bdmf_boolean flow_ctrl_ug1_en, bdmf_boolean flow_ctrl_ug2_en, bdmf_boolean flow_ctrl_ug3_en)
{
    uint32_t reg_flow_ctrl_ug_ctrl=0;

#ifdef VALIDATE_PARMS
    if((flow_ctrl_ug0_en >= _1BITS_MAX_VAL_) ||
       (flow_ctrl_ug1_en >= _1BITS_MAX_VAL_) ||
       (flow_ctrl_ug2_en >= _1BITS_MAX_VAL_) ||
       (flow_ctrl_ug3_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_flow_ctrl_ug_ctrl = RU_FIELD_SET(0, QM, FLOW_CTRL_UG_CTRL, FLOW_CTRL_UG0_EN, reg_flow_ctrl_ug_ctrl, flow_ctrl_ug0_en);
    reg_flow_ctrl_ug_ctrl = RU_FIELD_SET(0, QM, FLOW_CTRL_UG_CTRL, FLOW_CTRL_UG1_EN, reg_flow_ctrl_ug_ctrl, flow_ctrl_ug1_en);
    reg_flow_ctrl_ug_ctrl = RU_FIELD_SET(0, QM, FLOW_CTRL_UG_CTRL, FLOW_CTRL_UG2_EN, reg_flow_ctrl_ug_ctrl, flow_ctrl_ug2_en);
    reg_flow_ctrl_ug_ctrl = RU_FIELD_SET(0, QM, FLOW_CTRL_UG_CTRL, FLOW_CTRL_UG3_EN, reg_flow_ctrl_ug_ctrl, flow_ctrl_ug3_en);

    RU_REG_WRITE(0, QM, FLOW_CTRL_UG_CTRL, reg_flow_ctrl_ug_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_flow_ctrl_ug_ctrl_get(bdmf_boolean *flow_ctrl_ug0_en, bdmf_boolean *flow_ctrl_ug1_en, bdmf_boolean *flow_ctrl_ug2_en, bdmf_boolean *flow_ctrl_ug3_en)
{
    uint32_t reg_flow_ctrl_ug_ctrl;

#ifdef VALIDATE_PARMS
    if(!flow_ctrl_ug0_en || !flow_ctrl_ug1_en || !flow_ctrl_ug2_en || !flow_ctrl_ug3_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, FLOW_CTRL_UG_CTRL, reg_flow_ctrl_ug_ctrl);

    *flow_ctrl_ug0_en = RU_FIELD_GET(0, QM, FLOW_CTRL_UG_CTRL, FLOW_CTRL_UG0_EN, reg_flow_ctrl_ug_ctrl);
    *flow_ctrl_ug1_en = RU_FIELD_GET(0, QM, FLOW_CTRL_UG_CTRL, FLOW_CTRL_UG1_EN, reg_flow_ctrl_ug_ctrl);
    *flow_ctrl_ug2_en = RU_FIELD_GET(0, QM, FLOW_CTRL_UG_CTRL, FLOW_CTRL_UG2_EN, reg_flow_ctrl_ug_ctrl);
    *flow_ctrl_ug3_en = RU_FIELD_GET(0, QM, FLOW_CTRL_UG_CTRL, FLOW_CTRL_UG3_EN, reg_flow_ctrl_ug_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_flow_ctrl_status_set(const qm_flow_ctrl_status *flow_ctrl_status)
{
    uint32_t reg_flow_ctrl_status=0;

#ifdef VALIDATE_PARMS
    if(!flow_ctrl_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((flow_ctrl_status->ug0 >= _1BITS_MAX_VAL_) ||
       (flow_ctrl_status->ug1 >= _1BITS_MAX_VAL_) ||
       (flow_ctrl_status->ug2 >= _1BITS_MAX_VAL_) ||
       (flow_ctrl_status->ug3 >= _1BITS_MAX_VAL_) ||
       (flow_ctrl_status->wred >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_flow_ctrl_status = RU_FIELD_SET(0, QM, FLOW_CTRL_STATUS, UG0, reg_flow_ctrl_status, flow_ctrl_status->ug0);
    reg_flow_ctrl_status = RU_FIELD_SET(0, QM, FLOW_CTRL_STATUS, UG1, reg_flow_ctrl_status, flow_ctrl_status->ug1);
    reg_flow_ctrl_status = RU_FIELD_SET(0, QM, FLOW_CTRL_STATUS, UG2, reg_flow_ctrl_status, flow_ctrl_status->ug2);
    reg_flow_ctrl_status = RU_FIELD_SET(0, QM, FLOW_CTRL_STATUS, UG3, reg_flow_ctrl_status, flow_ctrl_status->ug3);
    reg_flow_ctrl_status = RU_FIELD_SET(0, QM, FLOW_CTRL_STATUS, WRED, reg_flow_ctrl_status, flow_ctrl_status->wred);

    RU_REG_WRITE(0, QM, FLOW_CTRL_STATUS, reg_flow_ctrl_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_flow_ctrl_status_get(qm_flow_ctrl_status *flow_ctrl_status)
{
    uint32_t reg_flow_ctrl_status;

#ifdef VALIDATE_PARMS
    if(!flow_ctrl_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, FLOW_CTRL_STATUS, reg_flow_ctrl_status);

    flow_ctrl_status->ug0 = RU_FIELD_GET(0, QM, FLOW_CTRL_STATUS, UG0, reg_flow_ctrl_status);
    flow_ctrl_status->ug1 = RU_FIELD_GET(0, QM, FLOW_CTRL_STATUS, UG1, reg_flow_ctrl_status);
    flow_ctrl_status->ug2 = RU_FIELD_GET(0, QM, FLOW_CTRL_STATUS, UG2, reg_flow_ctrl_status);
    flow_ctrl_status->ug3 = RU_FIELD_GET(0, QM, FLOW_CTRL_STATUS, UG3, reg_flow_ctrl_status);
    flow_ctrl_status->wred = RU_FIELD_GET(0, QM, FLOW_CTRL_STATUS, WRED, reg_flow_ctrl_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_flow_ctrl_qm_flow_ctrl_rnr_cfg_set(const qm_flow_ctrl_qm_flow_ctrl_rnr_cfg *flow_ctrl_qm_flow_ctrl_rnr_cfg)
{
    uint32_t reg_flow_ctrl_qm_flow_ctrl_rnr_cfg=0;

#ifdef VALIDATE_PARMS
    if(!flow_ctrl_qm_flow_ctrl_rnr_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((flow_ctrl_qm_flow_ctrl_rnr_cfg->rnr_bb_id >= _6BITS_MAX_VAL_) ||
       (flow_ctrl_qm_flow_ctrl_rnr_cfg->rnr_task >= _4BITS_MAX_VAL_) ||
       (flow_ctrl_qm_flow_ctrl_rnr_cfg->rnr_enable >= _1BITS_MAX_VAL_) ||
       (flow_ctrl_qm_flow_ctrl_rnr_cfg->sram_wr_en >= _1BITS_MAX_VAL_) ||
       (flow_ctrl_qm_flow_ctrl_rnr_cfg->sram_wr_addr >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_flow_ctrl_qm_flow_ctrl_rnr_cfg = RU_FIELD_SET(0, QM, FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG, RNR_BB_ID, reg_flow_ctrl_qm_flow_ctrl_rnr_cfg, flow_ctrl_qm_flow_ctrl_rnr_cfg->rnr_bb_id);
    reg_flow_ctrl_qm_flow_ctrl_rnr_cfg = RU_FIELD_SET(0, QM, FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG, RNR_TASK, reg_flow_ctrl_qm_flow_ctrl_rnr_cfg, flow_ctrl_qm_flow_ctrl_rnr_cfg->rnr_task);
    reg_flow_ctrl_qm_flow_ctrl_rnr_cfg = RU_FIELD_SET(0, QM, FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG, RNR_ENABLE, reg_flow_ctrl_qm_flow_ctrl_rnr_cfg, flow_ctrl_qm_flow_ctrl_rnr_cfg->rnr_enable);
    reg_flow_ctrl_qm_flow_ctrl_rnr_cfg = RU_FIELD_SET(0, QM, FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG, SRAM_WR_EN, reg_flow_ctrl_qm_flow_ctrl_rnr_cfg, flow_ctrl_qm_flow_ctrl_rnr_cfg->sram_wr_en);
    reg_flow_ctrl_qm_flow_ctrl_rnr_cfg = RU_FIELD_SET(0, QM, FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG, SRAM_WR_ADDR, reg_flow_ctrl_qm_flow_ctrl_rnr_cfg, flow_ctrl_qm_flow_ctrl_rnr_cfg->sram_wr_addr);

    RU_REG_WRITE(0, QM, FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG, reg_flow_ctrl_qm_flow_ctrl_rnr_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_flow_ctrl_qm_flow_ctrl_rnr_cfg_get(qm_flow_ctrl_qm_flow_ctrl_rnr_cfg *flow_ctrl_qm_flow_ctrl_rnr_cfg)
{
    uint32_t reg_flow_ctrl_qm_flow_ctrl_rnr_cfg;

#ifdef VALIDATE_PARMS
    if(!flow_ctrl_qm_flow_ctrl_rnr_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG, reg_flow_ctrl_qm_flow_ctrl_rnr_cfg);

    flow_ctrl_qm_flow_ctrl_rnr_cfg->rnr_bb_id = RU_FIELD_GET(0, QM, FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG, RNR_BB_ID, reg_flow_ctrl_qm_flow_ctrl_rnr_cfg);
    flow_ctrl_qm_flow_ctrl_rnr_cfg->rnr_task = RU_FIELD_GET(0, QM, FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG, RNR_TASK, reg_flow_ctrl_qm_flow_ctrl_rnr_cfg);
    flow_ctrl_qm_flow_ctrl_rnr_cfg->rnr_enable = RU_FIELD_GET(0, QM, FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG, RNR_ENABLE, reg_flow_ctrl_qm_flow_ctrl_rnr_cfg);
    flow_ctrl_qm_flow_ctrl_rnr_cfg->sram_wr_en = RU_FIELD_GET(0, QM, FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG, SRAM_WR_EN, reg_flow_ctrl_qm_flow_ctrl_rnr_cfg);
    flow_ctrl_qm_flow_ctrl_rnr_cfg->sram_wr_addr = RU_FIELD_GET(0, QM, FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG, SRAM_WR_ADDR, reg_flow_ctrl_qm_flow_ctrl_rnr_cfg);

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

bdmf_error_t ag_drv_qm_qm_pd_congestion_drop_cnt_get(uint32_t *counter)
{
    uint32_t reg_qm_pd_congestion_drop_cnt;

#ifdef VALIDATE_PARMS
    if(!counter)
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

bdmf_error_t ag_drv_qm_backpressure_get(uint32_t *counter)
{
    uint32_t reg_backpressure;

#ifdef VALIDATE_PARMS
    if(!counter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, BACKPRESSURE, reg_backpressure);

    *counter = RU_FIELD_GET(0, QM, BACKPRESSURE, COUNTER, reg_backpressure);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg2_bbhtx_fifo_addr_set(uint8_t addr, uint8_t bbhtx_req_otf)
{
    uint32_t reg_global_cfg2_bbhtx_fifo_addr=0;

#ifdef VALIDATE_PARMS
    if((addr >= _6BITS_MAX_VAL_) ||
       (bbhtx_req_otf >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_global_cfg2_bbhtx_fifo_addr = RU_FIELD_SET(0, QM, GLOBAL_CFG2_BBHTX_FIFO_ADDR, ADDR, reg_global_cfg2_bbhtx_fifo_addr, addr);
    reg_global_cfg2_bbhtx_fifo_addr = RU_FIELD_SET(0, QM, GLOBAL_CFG2_BBHTX_FIFO_ADDR, BBHTX_REQ_OTF, reg_global_cfg2_bbhtx_fifo_addr, bbhtx_req_otf);

    RU_REG_WRITE(0, QM, GLOBAL_CFG2_BBHTX_FIFO_ADDR, reg_global_cfg2_bbhtx_fifo_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_qm_global_cfg2_bbhtx_fifo_addr_get(uint8_t *addr, uint8_t *bbhtx_req_otf)
{
    uint32_t reg_global_cfg2_bbhtx_fifo_addr;

#ifdef VALIDATE_PARMS
    if(!addr || !bbhtx_req_otf)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, QM, GLOBAL_CFG2_BBHTX_FIFO_ADDR, reg_global_cfg2_bbhtx_fifo_addr);

    *addr = RU_FIELD_GET(0, QM, GLOBAL_CFG2_BBHTX_FIFO_ADDR, ADDR, reg_global_cfg2_bbhtx_fifo_addr);
    *bbhtx_req_otf = RU_FIELD_GET(0, QM, GLOBAL_CFG2_BBHTX_FIFO_ADDR, BBHTX_REQ_OTF, reg_global_cfg2_bbhtx_fifo_addr);

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
    if((idx >= 1024))
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
    bdmf_address_global_cfg_fpm_base_addr,
    bdmf_address_global_cfg_fpm_coherent_base_addr,
    bdmf_address_global_cfg_ddr_sop_offset,
    bdmf_address_global_cfg_epon_overhead_ctrl,
    bdmf_address_global_cfg_dqm_full,
    bdmf_address_global_cfg_dqm_not_empty,
    bdmf_address_global_cfg_dqm_pop_ready,
    bdmf_address_global_cfg_aggregation_context_valid,
    bdmf_address_global_cfg_qm_aggregation_timer_ctrl,
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
    bdmf_address_fpm_buffer_reservation_data,
    bdmf_address_flow_ctrl_ug_ctrl,
    bdmf_address_flow_ctrl_status,
    bdmf_address_flow_ctrl_qm_flow_ctrl_rnr_cfg,
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
    bdmf_address_backpressure,
    bdmf_address_global_cfg2_bbhtx_fifo_addr,
    bdmf_address_data,
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
        qm_drop_counters_ctrl drop_counters_ctrl = { .read_clear_pkts=parm[1].value.unumber, .read_clear_bytes=parm[2].value.unumber, .disable_wrap_around_pkts=parm[3].value.unumber, .disable_wrap_around_bytes=parm[4].value.unumber, .free_with_context_last_search=parm[5].value.unumber, .wred_disable=parm[6].value.unumber, .ddr_pd_congestion_disable=parm[7].value.unumber, .ddr_byte_congestion_disable=parm[8].value.unumber, .ddr_occupancy_disable=parm[9].value.unumber, .ddr_fpm_congestion_disable=parm[10].value.unumber, .fpm_ug_disable=parm[11].value.unumber, .queue_occupancy_ddr_copy_decision_disable=parm[12].value.unumber, .psram_occupancy_ddr_copy_decision_disable=parm[13].value.unumber, .dont_send_mc_bit_to_bbh=parm[14].value.unumber, .close_aggregation_on_timeout_disable=parm[15].value.unumber, .fpm_congestion_buf_release_mechanism_disable=parm[16].value.unumber, .fpm_buffer_global_res_enable=parm[17].value.unumber, .qm_preserve_pd_with_fpm=parm[18].value.unumber, .qm_residue_per_queue=parm[19].value.unumber, .ghost_rpt_update_after_close_agg_en=parm[20].value.unumber, .fpm_ug_flow_ctrl_disable=parm[21].value.unumber, .ddr_write_multi_slave_en=parm[22].value.unumber, .ddr_pd_congestion_agg_priority=parm[23].value.unumber, .psram_occupancy_drop_disable=parm[24].value.unumber, .qm_ddr_write_alignment=parm[25].value.unumber, .exclusive_dont_drop=parm[26].value.unumber, .exclusive_dont_drop_bp_en=parm[27].value.unumber, .gpon_dbr_ceil=parm[28].value.unumber, .drop_cnt_wred_drops=parm[29].value.unumber};
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
        err = ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
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
        qm_intr_ctrl_isr intr_ctrl_isr = { .qm_dqm_pop_on_empty=parm[1].value.unumber, .qm_dqm_push_on_full=parm[2].value.unumber, .qm_cpu_pop_on_empty=parm[3].value.unumber, .qm_cpu_push_on_full=parm[4].value.unumber, .qm_normal_queue_pd_no_credit=parm[5].value.unumber, .qm_non_delayed_queue_pd_no_credit=parm[6].value.unumber, .qm_non_valid_queue=parm[7].value.unumber, .qm_agg_coherent_inconsistency=parm[8].value.unumber, .qm_force_copy_on_non_delayed=parm[9].value.unumber, .qm_fpm_pool_size_nonexistent=parm[10].value.unumber, .qm_target_mem_abs_contradiction=parm[11].value.unumber, .qm_1588_drop=parm[12].value.unumber, .qm_1588_multicast_contradiction=parm[13].value.unumber, .qm_byte_drop_cnt_overrun=parm[14].value.unumber, .qm_pkt_drop_cnt_overrun=parm[15].value.unumber, .qm_total_byte_cnt_underrun=parm[16].value.unumber, .qm_total_pkt_cnt_underrun=parm[17].value.unumber, .qm_fpm_ug0_underrun=parm[18].value.unumber, .qm_fpm_ug1_underrun=parm[19].value.unumber, .qm_fpm_ug2_underrun=parm[20].value.unumber, .qm_fpm_ug3_underrun=parm[21].value.unumber, .qm_timer_wraparound=parm[22].value.unumber, .qm_copy_plen_zero=parm[23].value.unumber};
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
    case cli_qm_fpm_buffer_reservation_data:
        err = ag_drv_qm_fpm_buffer_reservation_data_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_flow_ctrl_ug_ctrl:
        err = ag_drv_qm_flow_ctrl_ug_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_qm_flow_ctrl_status:
    {
        qm_flow_ctrl_status flow_ctrl_status = { .ug0=parm[1].value.unumber, .ug1=parm[2].value.unumber, .ug2=parm[3].value.unumber, .ug3=parm[4].value.unumber, .wred=parm[5].value.unumber};
        err = ag_drv_qm_flow_ctrl_status_set(&flow_ctrl_status);
        break;
    }
    case cli_qm_flow_ctrl_qm_flow_ctrl_rnr_cfg:
    {
        qm_flow_ctrl_qm_flow_ctrl_rnr_cfg flow_ctrl_qm_flow_ctrl_rnr_cfg = { .rnr_bb_id=parm[1].value.unumber, .rnr_task=parm[2].value.unumber, .rnr_enable=parm[3].value.unumber, .sram_wr_en=parm[4].value.unumber, .sram_wr_addr=parm[5].value.unumber};
        err = ag_drv_qm_flow_ctrl_qm_flow_ctrl_rnr_cfg_set(&flow_ctrl_qm_flow_ctrl_rnr_cfg);
        break;
    }
    case cli_qm_debug_sel:
        err = ag_drv_qm_debug_sel_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_qm_bb_route_ovr:
        err = ag_drv_qm_bb_route_ovr_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_qm_global_cfg2_bbhtx_fifo_addr:
        err = ag_drv_qm_global_cfg2_bbhtx_fifo_addr_set(parm[1].value.unumber, parm[2].value.unumber);
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
        bdmf_session_print(session, "drop_cnt_wred_drops = %u (0x%x)\n", drop_counters_ctrl.drop_cnt_wred_drops, drop_counters_ctrl.drop_cnt_wred_drops);
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
        bdmf_boolean pd_occupancy_en;
        uint8_t pd_occupancy_value;
        err = ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_get(&prescaler_granularity, &aggregation_timeout_value, &pd_occupancy_en, &pd_occupancy_value);
        bdmf_session_print(session, "prescaler_granularity = %u (0x%x)\n", prescaler_granularity, prescaler_granularity);
        bdmf_session_print(session, "aggregation_timeout_value = %u (0x%x)\n", aggregation_timeout_value, aggregation_timeout_value);
        bdmf_session_print(session, "pd_occupancy_en = %u (0x%x)\n", pd_occupancy_en, pd_occupancy_en);
        bdmf_session_print(session, "pd_occupancy_value = %u (0x%x)\n", pd_occupancy_value, pd_occupancy_value);
        break;
    }
    case cli_qm_global_cfg_qm_fpm_ug_gbl_cnt:
    {
        uint16_t fpm_gbl_cnt;
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
        bdmf_session_print(session, "qm_copy_plen_zero = %u (0x%x)\n", intr_ctrl_isr.qm_copy_plen_zero, intr_ctrl_isr.qm_copy_plen_zero);
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
    case cli_qm_intr_ctrl_itr:
    {
        uint32_t ist;
        err = ag_drv_qm_intr_ctrl_itr_get(&ist);
        bdmf_session_print(session, "ist = %u (0x%x)\n", ist, ist);
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
    case cli_qm_fpm_buffer_reservation_data:
    {
        uint16_t data;
        err = ag_drv_qm_fpm_buffer_reservation_data_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_qm_flow_ctrl_ug_ctrl:
    {
        bdmf_boolean flow_ctrl_ug0_en;
        bdmf_boolean flow_ctrl_ug1_en;
        bdmf_boolean flow_ctrl_ug2_en;
        bdmf_boolean flow_ctrl_ug3_en;
        err = ag_drv_qm_flow_ctrl_ug_ctrl_get(&flow_ctrl_ug0_en, &flow_ctrl_ug1_en, &flow_ctrl_ug2_en, &flow_ctrl_ug3_en);
        bdmf_session_print(session, "flow_ctrl_ug0_en = %u (0x%x)\n", flow_ctrl_ug0_en, flow_ctrl_ug0_en);
        bdmf_session_print(session, "flow_ctrl_ug1_en = %u (0x%x)\n", flow_ctrl_ug1_en, flow_ctrl_ug1_en);
        bdmf_session_print(session, "flow_ctrl_ug2_en = %u (0x%x)\n", flow_ctrl_ug2_en, flow_ctrl_ug2_en);
        bdmf_session_print(session, "flow_ctrl_ug3_en = %u (0x%x)\n", flow_ctrl_ug3_en, flow_ctrl_ug3_en);
        break;
    }
    case cli_qm_flow_ctrl_status:
    {
        qm_flow_ctrl_status flow_ctrl_status;
        err = ag_drv_qm_flow_ctrl_status_get(&flow_ctrl_status);
        bdmf_session_print(session, "ug0 = %u (0x%x)\n", flow_ctrl_status.ug0, flow_ctrl_status.ug0);
        bdmf_session_print(session, "ug1 = %u (0x%x)\n", flow_ctrl_status.ug1, flow_ctrl_status.ug1);
        bdmf_session_print(session, "ug2 = %u (0x%x)\n", flow_ctrl_status.ug2, flow_ctrl_status.ug2);
        bdmf_session_print(session, "ug3 = %u (0x%x)\n", flow_ctrl_status.ug3, flow_ctrl_status.ug3);
        bdmf_session_print(session, "wred = %u (0x%x)\n", flow_ctrl_status.wred, flow_ctrl_status.wred);
        break;
    }
    case cli_qm_flow_ctrl_qm_flow_ctrl_rnr_cfg:
    {
        qm_flow_ctrl_qm_flow_ctrl_rnr_cfg flow_ctrl_qm_flow_ctrl_rnr_cfg;
        err = ag_drv_qm_flow_ctrl_qm_flow_ctrl_rnr_cfg_get(&flow_ctrl_qm_flow_ctrl_rnr_cfg);
        bdmf_session_print(session, "rnr_bb_id = %u (0x%x)\n", flow_ctrl_qm_flow_ctrl_rnr_cfg.rnr_bb_id, flow_ctrl_qm_flow_ctrl_rnr_cfg.rnr_bb_id);
        bdmf_session_print(session, "rnr_task = %u (0x%x)\n", flow_ctrl_qm_flow_ctrl_rnr_cfg.rnr_task, flow_ctrl_qm_flow_ctrl_rnr_cfg.rnr_task);
        bdmf_session_print(session, "rnr_enable = %u (0x%x)\n", flow_ctrl_qm_flow_ctrl_rnr_cfg.rnr_enable, flow_ctrl_qm_flow_ctrl_rnr_cfg.rnr_enable);
        bdmf_session_print(session, "sram_wr_en = %u (0x%x)\n", flow_ctrl_qm_flow_ctrl_rnr_cfg.sram_wr_en, flow_ctrl_qm_flow_ctrl_rnr_cfg.sram_wr_en);
        bdmf_session_print(session, "sram_wr_addr = %u (0x%x)\n", flow_ctrl_qm_flow_ctrl_rnr_cfg.sram_wr_addr, flow_ctrl_qm_flow_ctrl_rnr_cfg.sram_wr_addr);
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
        uint32_t counter;
        err = ag_drv_qm_qm_pd_congestion_drop_cnt_get(&counter);
        bdmf_session_print(session, "counter = %u (0x%x)\n", counter, counter);
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
    case cli_qm_backpressure:
    {
        uint32_t counter;
        err = ag_drv_qm_backpressure_get(&counter);
        bdmf_session_print(session, "counter = %u (0x%x)\n", counter, counter);
        break;
    }
    case cli_qm_global_cfg2_bbhtx_fifo_addr:
    {
        uint8_t addr;
        uint8_t bbhtx_req_otf;
        err = ag_drv_qm_global_cfg2_bbhtx_fifo_addr_get(&addr, &bbhtx_req_otf);
        bdmf_session_print(session, "addr = %u (0x%x)\n", addr, addr);
        bdmf_session_print(session, "bbhtx_req_otf = %u (0x%x)\n", bbhtx_req_otf, bbhtx_req_otf);
        break;
    }
    case cli_qm_cm_residue_data:
    {
        uint32_t data;
        err = ag_drv_qm_cm_residue_data_get(parm[1].value.unumber, &data);
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
        qm_drop_counters_ctrl drop_counters_ctrl = {.read_clear_pkts=gtmv(m, 1), .read_clear_bytes=gtmv(m, 1), .disable_wrap_around_pkts=gtmv(m, 1), .disable_wrap_around_bytes=gtmv(m, 1), .free_with_context_last_search=gtmv(m, 1), .wred_disable=gtmv(m, 1), .ddr_pd_congestion_disable=gtmv(m, 1), .ddr_byte_congestion_disable=gtmv(m, 1), .ddr_occupancy_disable=gtmv(m, 1), .ddr_fpm_congestion_disable=gtmv(m, 1), .fpm_ug_disable=gtmv(m, 1), .queue_occupancy_ddr_copy_decision_disable=gtmv(m, 1), .psram_occupancy_ddr_copy_decision_disable=gtmv(m, 1), .dont_send_mc_bit_to_bbh=gtmv(m, 1), .close_aggregation_on_timeout_disable=gtmv(m, 1), .fpm_congestion_buf_release_mechanism_disable=gtmv(m, 1), .fpm_buffer_global_res_enable=gtmv(m, 1), .qm_preserve_pd_with_fpm=gtmv(m, 1), .qm_residue_per_queue=gtmv(m, 1), .ghost_rpt_update_after_close_agg_en=gtmv(m, 1), .fpm_ug_flow_ctrl_disable=gtmv(m, 1), .ddr_write_multi_slave_en=gtmv(m, 1), .ddr_pd_congestion_agg_priority=gtmv(m, 1), .psram_occupancy_drop_disable=gtmv(m, 1), .qm_ddr_write_alignment=gtmv(m, 1), .exclusive_dont_drop=gtmv(m, 1), .exclusive_dont_drop_bp_en=gtmv(m, 1), .gpon_dbr_ceil=gtmv(m, 1), .drop_cnt_wred_drops=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_drop_counters_ctrl_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", drop_counters_ctrl.read_clear_pkts, drop_counters_ctrl.read_clear_bytes, drop_counters_ctrl.disable_wrap_around_pkts, drop_counters_ctrl.disable_wrap_around_bytes, drop_counters_ctrl.free_with_context_last_search, drop_counters_ctrl.wred_disable, drop_counters_ctrl.ddr_pd_congestion_disable, drop_counters_ctrl.ddr_byte_congestion_disable, drop_counters_ctrl.ddr_occupancy_disable, drop_counters_ctrl.ddr_fpm_congestion_disable, drop_counters_ctrl.fpm_ug_disable, drop_counters_ctrl.queue_occupancy_ddr_copy_decision_disable, drop_counters_ctrl.psram_occupancy_ddr_copy_decision_disable, drop_counters_ctrl.dont_send_mc_bit_to_bbh, drop_counters_ctrl.close_aggregation_on_timeout_disable, drop_counters_ctrl.fpm_congestion_buf_release_mechanism_disable, drop_counters_ctrl.fpm_buffer_global_res_enable, drop_counters_ctrl.qm_preserve_pd_with_fpm, drop_counters_ctrl.qm_residue_per_queue, drop_counters_ctrl.ghost_rpt_update_after_close_agg_en, drop_counters_ctrl.fpm_ug_flow_ctrl_disable, drop_counters_ctrl.ddr_write_multi_slave_en, drop_counters_ctrl.ddr_pd_congestion_agg_priority, drop_counters_ctrl.psram_occupancy_drop_disable, drop_counters_ctrl.qm_ddr_write_alignment, drop_counters_ctrl.exclusive_dont_drop, drop_counters_ctrl.exclusive_dont_drop_bp_en, drop_counters_ctrl.gpon_dbr_ceil, drop_counters_ctrl.drop_cnt_wred_drops);
        if(!err) ag_drv_qm_drop_counters_ctrl_set(&drop_counters_ctrl);
        if(!err) ag_drv_qm_drop_counters_ctrl_get( &drop_counters_ctrl);
        if(!err) bdmf_session_print(session, "ag_drv_qm_drop_counters_ctrl_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", drop_counters_ctrl.read_clear_pkts, drop_counters_ctrl.read_clear_bytes, drop_counters_ctrl.disable_wrap_around_pkts, drop_counters_ctrl.disable_wrap_around_bytes, drop_counters_ctrl.free_with_context_last_search, drop_counters_ctrl.wred_disable, drop_counters_ctrl.ddr_pd_congestion_disable, drop_counters_ctrl.ddr_byte_congestion_disable, drop_counters_ctrl.ddr_occupancy_disable, drop_counters_ctrl.ddr_fpm_congestion_disable, drop_counters_ctrl.fpm_ug_disable, drop_counters_ctrl.queue_occupancy_ddr_copy_decision_disable, drop_counters_ctrl.psram_occupancy_ddr_copy_decision_disable, drop_counters_ctrl.dont_send_mc_bit_to_bbh, drop_counters_ctrl.close_aggregation_on_timeout_disable, drop_counters_ctrl.fpm_congestion_buf_release_mechanism_disable, drop_counters_ctrl.fpm_buffer_global_res_enable, drop_counters_ctrl.qm_preserve_pd_with_fpm, drop_counters_ctrl.qm_residue_per_queue, drop_counters_ctrl.ghost_rpt_update_after_close_agg_en, drop_counters_ctrl.fpm_ug_flow_ctrl_disable, drop_counters_ctrl.ddr_write_multi_slave_en, drop_counters_ctrl.ddr_pd_congestion_agg_priority, drop_counters_ctrl.psram_occupancy_drop_disable, drop_counters_ctrl.qm_ddr_write_alignment, drop_counters_ctrl.exclusive_dont_drop, drop_counters_ctrl.exclusive_dont_drop_bp_en, drop_counters_ctrl.gpon_dbr_ceil, drop_counters_ctrl.drop_cnt_wred_drops);
        if(err || drop_counters_ctrl.read_clear_pkts!=gtmv(m, 1) || drop_counters_ctrl.read_clear_bytes!=gtmv(m, 1) || drop_counters_ctrl.disable_wrap_around_pkts!=gtmv(m, 1) || drop_counters_ctrl.disable_wrap_around_bytes!=gtmv(m, 1) || drop_counters_ctrl.free_with_context_last_search!=gtmv(m, 1) || drop_counters_ctrl.wred_disable!=gtmv(m, 1) || drop_counters_ctrl.ddr_pd_congestion_disable!=gtmv(m, 1) || drop_counters_ctrl.ddr_byte_congestion_disable!=gtmv(m, 1) || drop_counters_ctrl.ddr_occupancy_disable!=gtmv(m, 1) || drop_counters_ctrl.ddr_fpm_congestion_disable!=gtmv(m, 1) || drop_counters_ctrl.fpm_ug_disable!=gtmv(m, 1) || drop_counters_ctrl.queue_occupancy_ddr_copy_decision_disable!=gtmv(m, 1) || drop_counters_ctrl.psram_occupancy_ddr_copy_decision_disable!=gtmv(m, 1) || drop_counters_ctrl.dont_send_mc_bit_to_bbh!=gtmv(m, 1) || drop_counters_ctrl.close_aggregation_on_timeout_disable!=gtmv(m, 1) || drop_counters_ctrl.fpm_congestion_buf_release_mechanism_disable!=gtmv(m, 1) || drop_counters_ctrl.fpm_buffer_global_res_enable!=gtmv(m, 1) || drop_counters_ctrl.qm_preserve_pd_with_fpm!=gtmv(m, 1) || drop_counters_ctrl.qm_residue_per_queue!=gtmv(m, 1) || drop_counters_ctrl.ghost_rpt_update_after_close_agg_en!=gtmv(m, 1) || drop_counters_ctrl.fpm_ug_flow_ctrl_disable!=gtmv(m, 1) || drop_counters_ctrl.ddr_write_multi_slave_en!=gtmv(m, 1) || drop_counters_ctrl.ddr_pd_congestion_agg_priority!=gtmv(m, 1) || drop_counters_ctrl.psram_occupancy_drop_disable!=gtmv(m, 1) || drop_counters_ctrl.qm_ddr_write_alignment!=gtmv(m, 1) || drop_counters_ctrl.exclusive_dont_drop!=gtmv(m, 1) || drop_counters_ctrl.exclusive_dont_drop_bp_en!=gtmv(m, 1) || drop_counters_ctrl.gpon_dbr_ceil!=gtmv(m, 1) || drop_counters_ctrl.drop_cnt_wred_drops!=gtmv(m, 1))
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
        bdmf_boolean pd_occupancy_en=gtmv(m, 1);
        uint8_t pd_occupancy_value=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set( %u %u %u %u)\n", prescaler_granularity, aggregation_timeout_value, pd_occupancy_en, pd_occupancy_value);
        if(!err) ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set(prescaler_granularity, aggregation_timeout_value, pd_occupancy_en, pd_occupancy_value);
        if(!err) ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_get( &prescaler_granularity, &aggregation_timeout_value, &pd_occupancy_en, &pd_occupancy_value);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_get( %u %u %u %u)\n", prescaler_granularity, aggregation_timeout_value, pd_occupancy_en, pd_occupancy_value);
        if(err || prescaler_granularity!=gtmv(m, 3) || aggregation_timeout_value!=gtmv(m, 3) || pd_occupancy_en!=gtmv(m, 1) || pd_occupancy_value!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t fpm_gbl_cnt=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_set( %u)\n", fpm_gbl_cnt);
        if(!err) ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_set(fpm_gbl_cnt);
        if(!err) ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_get( &fpm_gbl_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_get( %u)\n", fpm_gbl_cnt);
        if(err || fpm_gbl_cnt!=gtmv(m, 16))
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
        qm_intr_ctrl_isr intr_ctrl_isr = {.qm_dqm_pop_on_empty=gtmv(m, 1), .qm_dqm_push_on_full=gtmv(m, 1), .qm_cpu_pop_on_empty=gtmv(m, 1), .qm_cpu_push_on_full=gtmv(m, 1), .qm_normal_queue_pd_no_credit=gtmv(m, 1), .qm_non_delayed_queue_pd_no_credit=gtmv(m, 1), .qm_non_valid_queue=gtmv(m, 1), .qm_agg_coherent_inconsistency=gtmv(m, 1), .qm_force_copy_on_non_delayed=gtmv(m, 1), .qm_fpm_pool_size_nonexistent=gtmv(m, 1), .qm_target_mem_abs_contradiction=gtmv(m, 1), .qm_1588_drop=gtmv(m, 1), .qm_1588_multicast_contradiction=gtmv(m, 1), .qm_byte_drop_cnt_overrun=gtmv(m, 1), .qm_pkt_drop_cnt_overrun=gtmv(m, 1), .qm_total_byte_cnt_underrun=gtmv(m, 1), .qm_total_pkt_cnt_underrun=gtmv(m, 1), .qm_fpm_ug0_underrun=gtmv(m, 1), .qm_fpm_ug1_underrun=gtmv(m, 1), .qm_fpm_ug2_underrun=gtmv(m, 1), .qm_fpm_ug3_underrun=gtmv(m, 1), .qm_timer_wraparound=gtmv(m, 1), .qm_copy_plen_zero=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_intr_ctrl_isr_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", intr_ctrl_isr.qm_dqm_pop_on_empty, intr_ctrl_isr.qm_dqm_push_on_full, intr_ctrl_isr.qm_cpu_pop_on_empty, intr_ctrl_isr.qm_cpu_push_on_full, intr_ctrl_isr.qm_normal_queue_pd_no_credit, intr_ctrl_isr.qm_non_delayed_queue_pd_no_credit, intr_ctrl_isr.qm_non_valid_queue, intr_ctrl_isr.qm_agg_coherent_inconsistency, intr_ctrl_isr.qm_force_copy_on_non_delayed, intr_ctrl_isr.qm_fpm_pool_size_nonexistent, intr_ctrl_isr.qm_target_mem_abs_contradiction, intr_ctrl_isr.qm_1588_drop, intr_ctrl_isr.qm_1588_multicast_contradiction, intr_ctrl_isr.qm_byte_drop_cnt_overrun, intr_ctrl_isr.qm_pkt_drop_cnt_overrun, intr_ctrl_isr.qm_total_byte_cnt_underrun, intr_ctrl_isr.qm_total_pkt_cnt_underrun, intr_ctrl_isr.qm_fpm_ug0_underrun, intr_ctrl_isr.qm_fpm_ug1_underrun, intr_ctrl_isr.qm_fpm_ug2_underrun, intr_ctrl_isr.qm_fpm_ug3_underrun, intr_ctrl_isr.qm_timer_wraparound, intr_ctrl_isr.qm_copy_plen_zero);
        if(!err) ag_drv_qm_intr_ctrl_isr_set(&intr_ctrl_isr);
        if(!err) ag_drv_qm_intr_ctrl_isr_get( &intr_ctrl_isr);
        if(!err) bdmf_session_print(session, "ag_drv_qm_intr_ctrl_isr_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", intr_ctrl_isr.qm_dqm_pop_on_empty, intr_ctrl_isr.qm_dqm_push_on_full, intr_ctrl_isr.qm_cpu_pop_on_empty, intr_ctrl_isr.qm_cpu_push_on_full, intr_ctrl_isr.qm_normal_queue_pd_no_credit, intr_ctrl_isr.qm_non_delayed_queue_pd_no_credit, intr_ctrl_isr.qm_non_valid_queue, intr_ctrl_isr.qm_agg_coherent_inconsistency, intr_ctrl_isr.qm_force_copy_on_non_delayed, intr_ctrl_isr.qm_fpm_pool_size_nonexistent, intr_ctrl_isr.qm_target_mem_abs_contradiction, intr_ctrl_isr.qm_1588_drop, intr_ctrl_isr.qm_1588_multicast_contradiction, intr_ctrl_isr.qm_byte_drop_cnt_overrun, intr_ctrl_isr.qm_pkt_drop_cnt_overrun, intr_ctrl_isr.qm_total_byte_cnt_underrun, intr_ctrl_isr.qm_total_pkt_cnt_underrun, intr_ctrl_isr.qm_fpm_ug0_underrun, intr_ctrl_isr.qm_fpm_ug1_underrun, intr_ctrl_isr.qm_fpm_ug2_underrun, intr_ctrl_isr.qm_fpm_ug3_underrun, intr_ctrl_isr.qm_timer_wraparound, intr_ctrl_isr.qm_copy_plen_zero);
        if(err || intr_ctrl_isr.qm_dqm_pop_on_empty!=gtmv(m, 1) || intr_ctrl_isr.qm_dqm_push_on_full!=gtmv(m, 1) || intr_ctrl_isr.qm_cpu_pop_on_empty!=gtmv(m, 1) || intr_ctrl_isr.qm_cpu_push_on_full!=gtmv(m, 1) || intr_ctrl_isr.qm_normal_queue_pd_no_credit!=gtmv(m, 1) || intr_ctrl_isr.qm_non_delayed_queue_pd_no_credit!=gtmv(m, 1) || intr_ctrl_isr.qm_non_valid_queue!=gtmv(m, 1) || intr_ctrl_isr.qm_agg_coherent_inconsistency!=gtmv(m, 1) || intr_ctrl_isr.qm_force_copy_on_non_delayed!=gtmv(m, 1) || intr_ctrl_isr.qm_fpm_pool_size_nonexistent!=gtmv(m, 1) || intr_ctrl_isr.qm_target_mem_abs_contradiction!=gtmv(m, 1) || intr_ctrl_isr.qm_1588_drop!=gtmv(m, 1) || intr_ctrl_isr.qm_1588_multicast_contradiction!=gtmv(m, 1) || intr_ctrl_isr.qm_byte_drop_cnt_overrun!=gtmv(m, 1) || intr_ctrl_isr.qm_pkt_drop_cnt_overrun!=gtmv(m, 1) || intr_ctrl_isr.qm_total_byte_cnt_underrun!=gtmv(m, 1) || intr_ctrl_isr.qm_total_pkt_cnt_underrun!=gtmv(m, 1) || intr_ctrl_isr.qm_fpm_ug0_underrun!=gtmv(m, 1) || intr_ctrl_isr.qm_fpm_ug1_underrun!=gtmv(m, 1) || intr_ctrl_isr.qm_fpm_ug2_underrun!=gtmv(m, 1) || intr_ctrl_isr.qm_fpm_ug3_underrun!=gtmv(m, 1) || intr_ctrl_isr.qm_timer_wraparound!=gtmv(m, 1) || intr_ctrl_isr.qm_copy_plen_zero!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t ism=gtmv(m, 23);
        if(!err) ag_drv_qm_intr_ctrl_ism_get( &ism);
        if(!err) bdmf_session_print(session, "ag_drv_qm_intr_ctrl_ism_get( %u)\n", ism);
    }
    {
        uint32_t iem=gtmv(m, 23);
        if(!err) bdmf_session_print(session, "ag_drv_qm_intr_ctrl_ier_set( %u)\n", iem);
        if(!err) ag_drv_qm_intr_ctrl_ier_set(iem);
        if(!err) ag_drv_qm_intr_ctrl_ier_get( &iem);
        if(!err) bdmf_session_print(session, "ag_drv_qm_intr_ctrl_ier_get( %u)\n", iem);
        if(err || iem!=gtmv(m, 23))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t ist=gtmv(m, 23);
        if(!err) bdmf_session_print(session, "ag_drv_qm_intr_ctrl_itr_set( %u)\n", ist);
        if(!err) ag_drv_qm_intr_ctrl_itr_set(ist);
        if(!err) ag_drv_qm_intr_ctrl_itr_get( &ist);
        if(!err) bdmf_session_print(session, "ag_drv_qm_intr_ctrl_itr_get( %u)\n", ist);
        if(err || ist!=gtmv(m, 23))
            return err ? err : BDMF_ERR_IO;
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
        uint16_t q_idx=gtmv(m, 7);
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
        uint16_t q_idx=gtmv(m, 9);
        uint32_t data=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_qm_total_valid_cnt_set( %u %u)\n", q_idx, data);
        if(!err) ag_drv_qm_total_valid_cnt_set(q_idx, data);
        if(!err) ag_drv_qm_total_valid_cnt_get( q_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_total_valid_cnt_get( %u %u)\n", q_idx, data);
        if(err || data!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t q_idx=gtmv(m, 8);
        uint32_t data=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dqm_valid_cnt_set( %u %u)\n", q_idx, data);
        if(!err) ag_drv_qm_dqm_valid_cnt_set(q_idx, data);
        if(!err) ag_drv_qm_dqm_valid_cnt_get( q_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_dqm_valid_cnt_get( %u %u)\n", q_idx, data);
        if(err || data!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t q_idx=gtmv(m, 8);
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_qm_drop_counter_get( q_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_drop_counter_get( %u %u)\n", q_idx, data);
    }
    {
        uint16_t q_idx=gtmv(m, 8);
        uint32_t data=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_qm_epon_q_byte_cnt_set( %u %u)\n", q_idx, data);
        if(!err) ag_drv_qm_epon_q_byte_cnt_set(q_idx, data);
        if(!err) ag_drv_qm_epon_q_byte_cnt_get( q_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_epon_q_byte_cnt_get( %u %u)\n", q_idx, data);
        if(err || data!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t q_idx=gtmv(m, 2);
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
        uint16_t q_idx=gtmv(m, 7);
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
        uint32_t idx=gtmv(m, 3);
        uint16_t data=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_buffer_reservation_data_set( %u %u)\n", idx, data);
        if(!err) ag_drv_qm_fpm_buffer_reservation_data_set(idx, data);
        if(!err) ag_drv_qm_fpm_buffer_reservation_data_get( idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_fpm_buffer_reservation_data_get( %u %u)\n", idx, data);
        if(err || data!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean flow_ctrl_ug0_en=gtmv(m, 1);
        bdmf_boolean flow_ctrl_ug1_en=gtmv(m, 1);
        bdmf_boolean flow_ctrl_ug2_en=gtmv(m, 1);
        bdmf_boolean flow_ctrl_ug3_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_qm_flow_ctrl_ug_ctrl_set( %u %u %u %u)\n", flow_ctrl_ug0_en, flow_ctrl_ug1_en, flow_ctrl_ug2_en, flow_ctrl_ug3_en);
        if(!err) ag_drv_qm_flow_ctrl_ug_ctrl_set(flow_ctrl_ug0_en, flow_ctrl_ug1_en, flow_ctrl_ug2_en, flow_ctrl_ug3_en);
        if(!err) ag_drv_qm_flow_ctrl_ug_ctrl_get( &flow_ctrl_ug0_en, &flow_ctrl_ug1_en, &flow_ctrl_ug2_en, &flow_ctrl_ug3_en);
        if(!err) bdmf_session_print(session, "ag_drv_qm_flow_ctrl_ug_ctrl_get( %u %u %u %u)\n", flow_ctrl_ug0_en, flow_ctrl_ug1_en, flow_ctrl_ug2_en, flow_ctrl_ug3_en);
        if(err || flow_ctrl_ug0_en!=gtmv(m, 1) || flow_ctrl_ug1_en!=gtmv(m, 1) || flow_ctrl_ug2_en!=gtmv(m, 1) || flow_ctrl_ug3_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_flow_ctrl_status flow_ctrl_status = {.ug0=gtmv(m, 1), .ug1=gtmv(m, 1), .ug2=gtmv(m, 1), .ug3=gtmv(m, 1), .wred=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_flow_ctrl_status_set( %u %u %u %u %u)\n", flow_ctrl_status.ug0, flow_ctrl_status.ug1, flow_ctrl_status.ug2, flow_ctrl_status.ug3, flow_ctrl_status.wred);
        if(!err) ag_drv_qm_flow_ctrl_status_set(&flow_ctrl_status);
        if(!err) ag_drv_qm_flow_ctrl_status_get( &flow_ctrl_status);
        if(!err) bdmf_session_print(session, "ag_drv_qm_flow_ctrl_status_get( %u %u %u %u %u)\n", flow_ctrl_status.ug0, flow_ctrl_status.ug1, flow_ctrl_status.ug2, flow_ctrl_status.ug3, flow_ctrl_status.wred);
        if(err || flow_ctrl_status.ug0!=gtmv(m, 1) || flow_ctrl_status.ug1!=gtmv(m, 1) || flow_ctrl_status.ug2!=gtmv(m, 1) || flow_ctrl_status.ug3!=gtmv(m, 1) || flow_ctrl_status.wred!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        qm_flow_ctrl_qm_flow_ctrl_rnr_cfg flow_ctrl_qm_flow_ctrl_rnr_cfg = {.rnr_bb_id=gtmv(m, 6), .rnr_task=gtmv(m, 4), .rnr_enable=gtmv(m, 1), .sram_wr_en=gtmv(m, 1), .sram_wr_addr=gtmv(m, 12)};
        if(!err) bdmf_session_print(session, "ag_drv_qm_flow_ctrl_qm_flow_ctrl_rnr_cfg_set( %u %u %u %u %u)\n", flow_ctrl_qm_flow_ctrl_rnr_cfg.rnr_bb_id, flow_ctrl_qm_flow_ctrl_rnr_cfg.rnr_task, flow_ctrl_qm_flow_ctrl_rnr_cfg.rnr_enable, flow_ctrl_qm_flow_ctrl_rnr_cfg.sram_wr_en, flow_ctrl_qm_flow_ctrl_rnr_cfg.sram_wr_addr);
        if(!err) ag_drv_qm_flow_ctrl_qm_flow_ctrl_rnr_cfg_set(&flow_ctrl_qm_flow_ctrl_rnr_cfg);
        if(!err) ag_drv_qm_flow_ctrl_qm_flow_ctrl_rnr_cfg_get( &flow_ctrl_qm_flow_ctrl_rnr_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_qm_flow_ctrl_qm_flow_ctrl_rnr_cfg_get( %u %u %u %u %u)\n", flow_ctrl_qm_flow_ctrl_rnr_cfg.rnr_bb_id, flow_ctrl_qm_flow_ctrl_rnr_cfg.rnr_task, flow_ctrl_qm_flow_ctrl_rnr_cfg.rnr_enable, flow_ctrl_qm_flow_ctrl_rnr_cfg.sram_wr_en, flow_ctrl_qm_flow_ctrl_rnr_cfg.sram_wr_addr);
        if(err || flow_ctrl_qm_flow_ctrl_rnr_cfg.rnr_bb_id!=gtmv(m, 6) || flow_ctrl_qm_flow_ctrl_rnr_cfg.rnr_task!=gtmv(m, 4) || flow_ctrl_qm_flow_ctrl_rnr_cfg.rnr_enable!=gtmv(m, 1) || flow_ctrl_qm_flow_ctrl_rnr_cfg.sram_wr_en!=gtmv(m, 1) || flow_ctrl_qm_flow_ctrl_rnr_cfg.sram_wr_addr!=gtmv(m, 12))
            return err ? err : BDMF_ERR_IO;
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
        uint32_t counter=gtmv(m, 32);
        if(!err) ag_drv_qm_qm_pd_congestion_drop_cnt_get( &counter);
        if(!err) bdmf_session_print(session, "ag_drv_qm_qm_pd_congestion_drop_cnt_get( %u)\n", counter);
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
        uint32_t counter=gtmv(m, 32);
        if(!err) ag_drv_qm_backpressure_get( &counter);
        if(!err) bdmf_session_print(session, "ag_drv_qm_backpressure_get( %u)\n", counter);
    }
    {
        uint8_t addr=gtmv(m, 6);
        uint8_t bbhtx_req_otf=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg2_bbhtx_fifo_addr_set( %u %u)\n", addr, bbhtx_req_otf);
        if(!err) ag_drv_qm_global_cfg2_bbhtx_fifo_addr_set(addr, bbhtx_req_otf);
        if(!err) ag_drv_qm_global_cfg2_bbhtx_fifo_addr_get( &addr, &bbhtx_req_otf);
        if(!err) bdmf_session_print(session, "ag_drv_qm_global_cfg2_bbhtx_fifo_addr_get( %u %u)\n", addr, bbhtx_req_otf);
        if(err || addr!=gtmv(m, 6) || bbhtx_req_otf!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t idx=gtmv(m, 10);
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_qm_cm_residue_data_get( idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_qm_cm_residue_data_get( %u %u)\n", idx, data);
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
    case bdmf_address_fpm_buffer_reservation_data : reg = &RU_REG(QM, FPM_BUFFER_RESERVATION_DATA); blk = &RU_BLK(QM); break;
    case bdmf_address_flow_ctrl_ug_ctrl : reg = &RU_REG(QM, FLOW_CTRL_UG_CTRL); blk = &RU_BLK(QM); break;
    case bdmf_address_flow_ctrl_status : reg = &RU_REG(QM, FLOW_CTRL_STATUS); blk = &RU_BLK(QM); break;
    case bdmf_address_flow_ctrl_qm_flow_ctrl_rnr_cfg : reg = &RU_REG(QM, FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG); blk = &RU_BLK(QM); break;
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
    case bdmf_address_backpressure : reg = &RU_REG(QM, BACKPRESSURE); blk = &RU_BLK(QM); break;
    case bdmf_address_global_cfg2_bbhtx_fifo_addr : reg = &RU_REG(QM, GLOBAL_CFG2_BBHTX_FIFO_ADDR); blk = &RU_BLK(QM); break;
    case bdmf_address_data : reg = &RU_REG(QM, DATA); blk = &RU_BLK(QM); break;
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
            BDMFMON_MAKE_PARM("drop_cnt_wred_drops", "drop_cnt_wred_drops", BDMFMON_PARM_NUMBER, 0),
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
            BDMFMON_MAKE_PARM("pd_occupancy_en", "pd_occupancy_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pd_occupancy_value", "pd_occupancy_value", BDMFMON_PARM_NUMBER, 0),
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
            BDMFMON_MAKE_PARM("qm_copy_plen_zero", "qm_copy_plen_zero", BDMFMON_PARM_NUMBER, 0),
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
        static bdmfmon_cmd_parm_t set_fpm_buffer_reservation_data[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flow_ctrl_ug_ctrl[]={
            BDMFMON_MAKE_PARM("flow_ctrl_ug0_en", "flow_ctrl_ug0_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("flow_ctrl_ug1_en", "flow_ctrl_ug1_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("flow_ctrl_ug2_en", "flow_ctrl_ug2_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("flow_ctrl_ug3_en", "flow_ctrl_ug3_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flow_ctrl_status[]={
            BDMFMON_MAKE_PARM("ug0", "ug0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ug1", "ug1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ug2", "ug2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ug3", "ug3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wred", "wred", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flow_ctrl_qm_flow_ctrl_rnr_cfg[]={
            BDMFMON_MAKE_PARM("rnr_bb_id", "rnr_bb_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_task", "rnr_task", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_enable", "rnr_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sram_wr_en", "sram_wr_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sram_wr_addr", "sram_wr_addr", BDMFMON_PARM_NUMBER, 0),
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
        static bdmfmon_cmd_parm_t set_global_cfg2_bbhtx_fifo_addr[]={
            BDMFMON_MAKE_PARM("addr", "addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bbhtx_req_otf", "bbhtx_req_otf", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="ddr_cong_ctrl", .val=cli_qm_ddr_cong_ctrl, .parms=set_ddr_cong_ctrl },
            { .name="fpm_ug_thr", .val=cli_qm_fpm_ug_thr, .parms=set_fpm_ug_thr },
            { .name="rnr_group_cfg", .val=cli_qm_rnr_group_cfg, .parms=set_rnr_group_cfg },
            { .name="cpu_pd_indirect_wr_data", .val=cli_qm_cpu_pd_indirect_wr_data, .parms=set_cpu_pd_indirect_wr_data },
            { .name="wred_profile_cfg", .val=cli_qm_wred_profile_cfg, .parms=set_wred_profile_cfg },
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
            { .name="fpm_buffer_reservation_data", .val=cli_qm_fpm_buffer_reservation_data, .parms=set_fpm_buffer_reservation_data },
            { .name="flow_ctrl_ug_ctrl", .val=cli_qm_flow_ctrl_ug_ctrl, .parms=set_flow_ctrl_ug_ctrl },
            { .name="flow_ctrl_status", .val=cli_qm_flow_ctrl_status, .parms=set_flow_ctrl_status },
            { .name="flow_ctrl_qm_flow_ctrl_rnr_cfg", .val=cli_qm_flow_ctrl_qm_flow_ctrl_rnr_cfg, .parms=set_flow_ctrl_qm_flow_ctrl_rnr_cfg },
            { .name="debug_sel", .val=cli_qm_debug_sel, .parms=set_debug_sel },
            { .name="bb_route_ovr", .val=cli_qm_bb_route_ovr, .parms=set_bb_route_ovr },
            { .name="global_cfg2_bbhtx_fifo_addr", .val=cli_qm_global_cfg2_bbhtx_fifo_addr, .parms=set_global_cfg2_bbhtx_fifo_addr },
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
        static bdmfmon_cmd_parm_t set_fpm_buffer_reservation_data[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
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
            { .name="global_cfg_qm_fpm_ug_gbl_cnt", .val=cli_qm_global_cfg_qm_fpm_ug_gbl_cnt, .parms=set_default },
            { .name="global_cfg_qm_egress_flush_queue", .val=cli_qm_global_cfg_qm_egress_flush_queue, .parms=set_default },
            { .name="fpm_pool_thr", .val=cli_qm_fpm_pool_thr, .parms=set_fpm_pool_thr },
            { .name="fpm_ug_cnt", .val=cli_qm_fpm_ug_cnt, .parms=set_fpm_ug_cnt },
            { .name="intr_ctrl_isr", .val=cli_qm_intr_ctrl_isr, .parms=set_default },
            { .name="intr_ctrl_ism", .val=cli_qm_intr_ctrl_ism, .parms=set_default },
            { .name="intr_ctrl_ier", .val=cli_qm_intr_ctrl_ier, .parms=set_default },
            { .name="intr_ctrl_itr", .val=cli_qm_intr_ctrl_itr, .parms=set_default },
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
            { .name="fpm_buffer_reservation_data", .val=cli_qm_fpm_buffer_reservation_data, .parms=set_fpm_buffer_reservation_data },
            { .name="flow_ctrl_ug_ctrl", .val=cli_qm_flow_ctrl_ug_ctrl, .parms=set_default },
            { .name="flow_ctrl_status", .val=cli_qm_flow_ctrl_status, .parms=set_default },
            { .name="flow_ctrl_qm_flow_ctrl_rnr_cfg", .val=cli_qm_flow_ctrl_qm_flow_ctrl_rnr_cfg, .parms=set_default },
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
            { .name="backpressure", .val=cli_qm_backpressure, .parms=set_default },
            { .name="global_cfg2_bbhtx_fifo_addr", .val=cli_qm_global_cfg2_bbhtx_fifo_addr, .parms=set_default },
            { .name="cm_residue_data", .val=cli_qm_cm_residue_data, .parms=set_cm_residue_data },
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
            { .name="FPM_BUFFER_RESERVATION_DATA" , .val=bdmf_address_fpm_buffer_reservation_data },
            { .name="FLOW_CTRL_UG_CTRL" , .val=bdmf_address_flow_ctrl_ug_ctrl },
            { .name="FLOW_CTRL_STATUS" , .val=bdmf_address_flow_ctrl_status },
            { .name="FLOW_CTRL_QM_FLOW_CTRL_RNR_CFG" , .val=bdmf_address_flow_ctrl_qm_flow_ctrl_rnr_cfg },
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
            { .name="BACKPRESSURE" , .val=bdmf_address_backpressure },
            { .name="GLOBAL_CFG2_BBHTX_FIFO_ADDR" , .val=bdmf_address_global_cfg2_bbhtx_fifo_addr },
            { .name="DATA" , .val=bdmf_address_data },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_qm_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

