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
#include "xrdp_drv_gsu_reg_ag.h"

#define BLOCK_ADDR_COUNT_BITS 3
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_gsu_reg_profiling_downcnt_set(uint8_t gsu_reg_id, uint32_t downcnt_value)
{
    uint32_t reg_profiling_downcnt = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiling_downcnt = RU_FIELD_SET(gsu_reg_id, GSU_REG, PROFILING_DOWNCNT, DOWNCNT_VALUE, reg_profiling_downcnt, downcnt_value);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, PROFILING_DOWNCNT, reg_profiling_downcnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_profiling_downcnt_get(uint8_t gsu_reg_id, uint32_t *downcnt_value)
{
    uint32_t reg_profiling_downcnt;

#ifdef VALIDATE_PARMS
    if (!downcnt_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, PROFILING_DOWNCNT, reg_profiling_downcnt);

    *downcnt_value = RU_FIELD_GET(gsu_reg_id, GSU_REG, PROFILING_DOWNCNT, DOWNCNT_VALUE, reg_profiling_downcnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_profiling_counter_a_get(uint8_t gsu_reg_id, uint32_t *value)
{
    uint32_t reg_profiling_counter_a;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, PROFILING_COUNTER_A, reg_profiling_counter_a);

    *value = RU_FIELD_GET(gsu_reg_id, GSU_REG, PROFILING_COUNTER_A, VALUE, reg_profiling_counter_a);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_profiling_counter_b_get(uint8_t gsu_reg_id, uint32_t *value)
{
    uint32_t reg_profiling_counter_b;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, PROFILING_COUNTER_B, reg_profiling_counter_b);

    *value = RU_FIELD_GET(gsu_reg_id, GSU_REG, PROFILING_COUNTER_B, VALUE, reg_profiling_counter_b);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_profiling_mode_set(uint8_t gsu_reg_id, bdmf_boolean counter_a_mode, uint8_t counter_b_mode, uint8_t command_id)
{
    uint32_t reg_profiling_mode = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT) ||
       (counter_a_mode >= _1BITS_MAX_VAL_) ||
       (counter_b_mode >= _2BITS_MAX_VAL_) ||
       (command_id >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiling_mode = RU_FIELD_SET(gsu_reg_id, GSU_REG, PROFILING_MODE, COUNTER_A_MODE, reg_profiling_mode, counter_a_mode);
    reg_profiling_mode = RU_FIELD_SET(gsu_reg_id, GSU_REG, PROFILING_MODE, COUNTER_B_MODE, reg_profiling_mode, counter_b_mode);
    reg_profiling_mode = RU_FIELD_SET(gsu_reg_id, GSU_REG, PROFILING_MODE, COMMAND_ID, reg_profiling_mode, command_id);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, PROFILING_MODE, reg_profiling_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_profiling_mode_get(uint8_t gsu_reg_id, bdmf_boolean *counter_a_mode, uint8_t *counter_b_mode, uint8_t *command_id)
{
    uint32_t reg_profiling_mode;

#ifdef VALIDATE_PARMS
    if (!counter_a_mode || !counter_b_mode || !command_id)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, PROFILING_MODE, reg_profiling_mode);

    *counter_a_mode = RU_FIELD_GET(gsu_reg_id, GSU_REG, PROFILING_MODE, COUNTER_A_MODE, reg_profiling_mode);
    *counter_b_mode = RU_FIELD_GET(gsu_reg_id, GSU_REG, PROFILING_MODE, COUNTER_B_MODE, reg_profiling_mode);
    *command_id = RU_FIELD_GET(gsu_reg_id, GSU_REG, PROFILING_MODE, COMMAND_ID, reg_profiling_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_qm_cfg_1_set(uint8_t gsu_reg_id, uint16_t qm_0_base_queue, uint16_t qm_1_base_queue)
{
    uint32_t reg_qm_cfg_1 = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT) ||
       (qm_0_base_queue >= _15BITS_MAX_VAL_) ||
       (qm_1_base_queue >= _15BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_qm_cfg_1 = RU_FIELD_SET(gsu_reg_id, GSU_REG, QM_CFG_1, QM_0_BASE_QUEUE, reg_qm_cfg_1, qm_0_base_queue);
    reg_qm_cfg_1 = RU_FIELD_SET(gsu_reg_id, GSU_REG, QM_CFG_1, QM_1_BASE_QUEUE, reg_qm_cfg_1, qm_1_base_queue);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, QM_CFG_1, reg_qm_cfg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_qm_cfg_1_get(uint8_t gsu_reg_id, uint16_t *qm_0_base_queue, uint16_t *qm_1_base_queue)
{
    uint32_t reg_qm_cfg_1;

#ifdef VALIDATE_PARMS
    if (!qm_0_base_queue || !qm_1_base_queue)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, QM_CFG_1, reg_qm_cfg_1);

    *qm_0_base_queue = RU_FIELD_GET(gsu_reg_id, GSU_REG, QM_CFG_1, QM_0_BASE_QUEUE, reg_qm_cfg_1);
    *qm_1_base_queue = RU_FIELD_GET(gsu_reg_id, GSU_REG, QM_CFG_1, QM_1_BASE_QUEUE, reg_qm_cfg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_qm_cfg_2_set(uint8_t gsu_reg_id, uint16_t pd_fifo_qm0, uint16_t pd_fifo_qm1)
{
    uint32_t reg_qm_cfg_2 = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT) ||
       (pd_fifo_qm0 >= _15BITS_MAX_VAL_) ||
       (pd_fifo_qm1 >= _15BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_qm_cfg_2 = RU_FIELD_SET(gsu_reg_id, GSU_REG, QM_CFG_2, PD_FIFO_QM0, reg_qm_cfg_2, pd_fifo_qm0);
    reg_qm_cfg_2 = RU_FIELD_SET(gsu_reg_id, GSU_REG, QM_CFG_2, PD_FIFO_QM1, reg_qm_cfg_2, pd_fifo_qm1);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, QM_CFG_2, reg_qm_cfg_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_qm_cfg_2_get(uint8_t gsu_reg_id, uint16_t *pd_fifo_qm0, uint16_t *pd_fifo_qm1)
{
    uint32_t reg_qm_cfg_2;

#ifdef VALIDATE_PARMS
    if (!pd_fifo_qm0 || !pd_fifo_qm1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, QM_CFG_2, reg_qm_cfg_2);

    *pd_fifo_qm0 = RU_FIELD_GET(gsu_reg_id, GSU_REG, QM_CFG_2, PD_FIFO_QM0, reg_qm_cfg_2);
    *pd_fifo_qm1 = RU_FIELD_GET(gsu_reg_id, GSU_REG, QM_CFG_2, PD_FIFO_QM1, reg_qm_cfg_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_qm_cfg_3_set(uint8_t gsu_reg_id, uint16_t num_of_qm_0_queues)
{
    uint32_t reg_qm_cfg_3 = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT) ||
       (num_of_qm_0_queues >= _11BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_qm_cfg_3 = RU_FIELD_SET(gsu_reg_id, GSU_REG, QM_CFG_3, NUM_OF_QM_0_QUEUES, reg_qm_cfg_3, num_of_qm_0_queues);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, QM_CFG_3, reg_qm_cfg_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_qm_cfg_3_get(uint8_t gsu_reg_id, uint16_t *num_of_qm_0_queues)
{
    uint32_t reg_qm_cfg_3;

#ifdef VALIDATE_PARMS
    if (!num_of_qm_0_queues)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, QM_CFG_3, reg_qm_cfg_3);

    *num_of_qm_0_queues = RU_FIELD_GET(gsu_reg_id, GSU_REG, QM_CFG_3, NUM_OF_QM_0_QUEUES, reg_qm_cfg_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_transmit_cmd_cfg_set(uint8_t gsu_reg_id, uint16_t pd_return_addr, uint16_t status_return_addr)
{
    uint32_t reg_transmit_cmd_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_transmit_cmd_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, TRANSMIT_CMD_CFG, PD_RETURN_ADDR, reg_transmit_cmd_cfg, pd_return_addr);
    reg_transmit_cmd_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, TRANSMIT_CMD_CFG, STATUS_RETURN_ADDR, reg_transmit_cmd_cfg, status_return_addr);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, TRANSMIT_CMD_CFG, reg_transmit_cmd_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_transmit_cmd_cfg_get(uint8_t gsu_reg_id, uint16_t *pd_return_addr, uint16_t *status_return_addr)
{
    uint32_t reg_transmit_cmd_cfg;

#ifdef VALIDATE_PARMS
    if (!pd_return_addr || !status_return_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, TRANSMIT_CMD_CFG, reg_transmit_cmd_cfg);

    *pd_return_addr = RU_FIELD_GET(gsu_reg_id, GSU_REG, TRANSMIT_CMD_CFG, PD_RETURN_ADDR, reg_transmit_cmd_cfg);
    *status_return_addr = RU_FIELD_GET(gsu_reg_id, GSU_REG, TRANSMIT_CMD_CFG, STATUS_RETURN_ADDR, reg_transmit_cmd_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_rl_status_cfg_set(uint8_t gsu_reg_id, uint16_t forced_incr_rl_status_addr, uint16_t periodic_rl_status_addr)
{
    uint32_t reg_rl_status_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rl_status_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, RL_STATUS_CFG, FORCED_INCR_RL_STATUS_ADDR, reg_rl_status_cfg, forced_incr_rl_status_addr);
    reg_rl_status_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, RL_STATUS_CFG, PERIODIC_RL_STATUS_ADDR, reg_rl_status_cfg, periodic_rl_status_addr);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, RL_STATUS_CFG, reg_rl_status_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_rl_status_cfg_get(uint8_t gsu_reg_id, uint16_t *forced_incr_rl_status_addr, uint16_t *periodic_rl_status_addr)
{
    uint32_t reg_rl_status_cfg;

#ifdef VALIDATE_PARMS
    if (!forced_incr_rl_status_addr || !periodic_rl_status_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, RL_STATUS_CFG, reg_rl_status_cfg);

    *forced_incr_rl_status_addr = RU_FIELD_GET(gsu_reg_id, GSU_REG, RL_STATUS_CFG, FORCED_INCR_RL_STATUS_ADDR, reg_rl_status_cfg);
    *periodic_rl_status_addr = RU_FIELD_GET(gsu_reg_id, GSU_REG, RL_STATUS_CFG, PERIODIC_RL_STATUS_ADDR, reg_rl_status_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_rl_fetch_cfg_set(uint8_t gsu_reg_id, uint16_t fecth_rl_base_addr)
{
    uint32_t reg_rl_fetch_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rl_fetch_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, RL_FETCH_CFG, FECTH_RL_BASE_ADDR, reg_rl_fetch_cfg, fecth_rl_base_addr);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, RL_FETCH_CFG, reg_rl_fetch_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_rl_fetch_cfg_get(uint8_t gsu_reg_id, uint16_t *fecth_rl_base_addr)
{
    uint32_t reg_rl_fetch_cfg;

#ifdef VALIDATE_PARMS
    if (!fecth_rl_base_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, RL_FETCH_CFG, reg_rl_fetch_cfg);

    *fecth_rl_base_addr = RU_FIELD_GET(gsu_reg_id, GSU_REG, RL_FETCH_CFG, FECTH_RL_BASE_ADDR, reg_rl_fetch_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_misc_cfg_set(uint8_t gsu_reg_id, const gsu_reg_misc_cfg *misc_cfg)
{
    uint32_t reg_misc_cfg = 0;

#ifdef VALIDATE_PARMS
    if(!misc_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT) ||
       (misc_cfg->dont_transmit_last_pd >= _1BITS_MAX_VAL_) ||
       (misc_cfg->rl_count_dithering_en >= _1BITS_MAX_VAL_) ||
       (misc_cfg->cfg_pck_overhead_bytes_a >= _6BITS_MAX_VAL_) ||
       (misc_cfg->cfg_pck_overhead_bytes_b >= _6BITS_MAX_VAL_) ||
       (misc_cfg->use_extended_memory >= _1BITS_MAX_VAL_) ||
       (misc_cfg->cfg_send_ack_to_rnr >= _1BITS_MAX_VAL_) ||
       (misc_cfg->task_num_for_rnr_ack >= _4BITS_MAX_VAL_) ||
       (misc_cfg->cfg_send_nack_to_bbhtx >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_misc_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, MISC_CFG, DONT_TRANSMIT_LAST_PD, reg_misc_cfg, misc_cfg->dont_transmit_last_pd);
    reg_misc_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, MISC_CFG, RL_COUNT_DITHERING_EN, reg_misc_cfg, misc_cfg->rl_count_dithering_en);
    reg_misc_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, MISC_CFG, CFG_PCK_OVERHEAD_BYTES_A, reg_misc_cfg, misc_cfg->cfg_pck_overhead_bytes_a);
    reg_misc_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, MISC_CFG, CFG_PCK_OVERHEAD_BYTES_B, reg_misc_cfg, misc_cfg->cfg_pck_overhead_bytes_b);
    reg_misc_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, MISC_CFG, USE_EXTENDED_MEMORY, reg_misc_cfg, misc_cfg->use_extended_memory);
    reg_misc_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, MISC_CFG, CFG_SEND_ACK_TO_RNR, reg_misc_cfg, misc_cfg->cfg_send_ack_to_rnr);
    reg_misc_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, MISC_CFG, TASK_NUM_FOR_RNR_ACK, reg_misc_cfg, misc_cfg->task_num_for_rnr_ack);
    reg_misc_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, MISC_CFG, CFG_SEND_NACK_TO_BBHTX, reg_misc_cfg, misc_cfg->cfg_send_nack_to_bbhtx);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, MISC_CFG, reg_misc_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_misc_cfg_get(uint8_t gsu_reg_id, gsu_reg_misc_cfg *misc_cfg)
{
    uint32_t reg_misc_cfg;

#ifdef VALIDATE_PARMS
    if (!misc_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, MISC_CFG, reg_misc_cfg);

    misc_cfg->dont_transmit_last_pd = RU_FIELD_GET(gsu_reg_id, GSU_REG, MISC_CFG, DONT_TRANSMIT_LAST_PD, reg_misc_cfg);
    misc_cfg->rl_count_dithering_en = RU_FIELD_GET(gsu_reg_id, GSU_REG, MISC_CFG, RL_COUNT_DITHERING_EN, reg_misc_cfg);
    misc_cfg->cfg_pck_overhead_bytes_a = RU_FIELD_GET(gsu_reg_id, GSU_REG, MISC_CFG, CFG_PCK_OVERHEAD_BYTES_A, reg_misc_cfg);
    misc_cfg->cfg_pck_overhead_bytes_b = RU_FIELD_GET(gsu_reg_id, GSU_REG, MISC_CFG, CFG_PCK_OVERHEAD_BYTES_B, reg_misc_cfg);
    misc_cfg->use_extended_memory = RU_FIELD_GET(gsu_reg_id, GSU_REG, MISC_CFG, USE_EXTENDED_MEMORY, reg_misc_cfg);
    misc_cfg->cfg_send_ack_to_rnr = RU_FIELD_GET(gsu_reg_id, GSU_REG, MISC_CFG, CFG_SEND_ACK_TO_RNR, reg_misc_cfg);
    misc_cfg->task_num_for_rnr_ack = RU_FIELD_GET(gsu_reg_id, GSU_REG, MISC_CFG, TASK_NUM_FOR_RNR_ACK, reg_misc_cfg);
    misc_cfg->cfg_send_nack_to_bbhtx = RU_FIELD_GET(gsu_reg_id, GSU_REG, MISC_CFG, CFG_SEND_NACK_TO_BBHTX, reg_misc_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_pause_cfg_set(uint8_t gsu_reg_id, bdmf_boolean pause, bdmf_boolean step_mode, bdmf_boolean do_single_command)
{
    uint32_t reg_pause_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT) ||
       (pause >= _1BITS_MAX_VAL_) ||
       (step_mode >= _1BITS_MAX_VAL_) ||
       (do_single_command >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pause_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, PAUSE_CFG, PAUSE, reg_pause_cfg, pause);
    reg_pause_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, PAUSE_CFG, STEP_MODE, reg_pause_cfg, step_mode);
    reg_pause_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, PAUSE_CFG, DO_SINGLE_COMMAND, reg_pause_cfg, do_single_command);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, PAUSE_CFG, reg_pause_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_pause_cfg_get(uint8_t gsu_reg_id, bdmf_boolean *pause, bdmf_boolean *step_mode, bdmf_boolean *do_single_command)
{
    uint32_t reg_pause_cfg;

#ifdef VALIDATE_PARMS
    if (!pause || !step_mode || !do_single_command)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, PAUSE_CFG, reg_pause_cfg);

    *pause = RU_FIELD_GET(gsu_reg_id, GSU_REG, PAUSE_CFG, PAUSE, reg_pause_cfg);
    *step_mode = RU_FIELD_GET(gsu_reg_id, GSU_REG, PAUSE_CFG, STEP_MODE, reg_pause_cfg);
    *do_single_command = RU_FIELD_GET(gsu_reg_id, GSU_REG, PAUSE_CFG, DO_SINGLE_COMMAND, reg_pause_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_aqm_cmd_cfg_set(uint8_t gsu_reg_id, uint16_t aqm_return_addr)
{
    uint32_t reg_aqm_cmd_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_aqm_cmd_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, AQM_CMD_CFG, AQM_RETURN_ADDR, reg_aqm_cmd_cfg, aqm_return_addr);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, AQM_CMD_CFG, reg_aqm_cmd_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_aqm_cmd_cfg_get(uint8_t gsu_reg_id, uint16_t *aqm_return_addr)
{
    uint32_t reg_aqm_cmd_cfg;

#ifdef VALIDATE_PARMS
    if (!aqm_return_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, AQM_CMD_CFG, reg_aqm_cmd_cfg);

    *aqm_return_addr = RU_FIELD_GET(gsu_reg_id, GSU_REG, AQM_CMD_CFG, AQM_RETURN_ADDR, reg_aqm_cmd_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_flush_cmd_cfg_set(uint8_t gsu_reg_id, uint16_t status_return_addr)
{
    uint32_t reg_flush_cmd_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_flush_cmd_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, FLUSH_CMD_CFG, STATUS_RETURN_ADDR, reg_flush_cmd_cfg, status_return_addr);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, FLUSH_CMD_CFG, reg_flush_cmd_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_flush_cmd_cfg_get(uint8_t gsu_reg_id, uint16_t *status_return_addr)
{
    uint32_t reg_flush_cmd_cfg;

#ifdef VALIDATE_PARMS
    if (!status_return_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, FLUSH_CMD_CFG, reg_flush_cmd_cfg);

    *status_return_addr = RU_FIELD_GET(gsu_reg_id, GSU_REG, FLUSH_CMD_CFG, STATUS_RETURN_ADDR, reg_flush_cmd_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_rl_update_cmd_cfg_set(uint8_t gsu_reg_id, uint16_t last_valid_node)
{
    uint32_t reg_rl_update_cmd_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT) ||
       (last_valid_node >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rl_update_cmd_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, RL_UPDATE_CMD_CFG, LAST_VALID_NODE, reg_rl_update_cmd_cfg, last_valid_node);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, RL_UPDATE_CMD_CFG, reg_rl_update_cmd_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_rl_update_cmd_cfg_get(uint8_t gsu_reg_id, uint16_t *last_valid_node)
{
    uint32_t reg_rl_update_cmd_cfg;

#ifdef VALIDATE_PARMS
    if (!last_valid_node)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, RL_UPDATE_CMD_CFG, reg_rl_update_cmd_cfg);

    *last_valid_node = RU_FIELD_GET(gsu_reg_id, GSU_REG, RL_UPDATE_CMD_CFG, LAST_VALID_NODE, reg_rl_update_cmd_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_pfc_modify_cmd_cfg_set(uint8_t gsu_reg_id, uint16_t last_valid_node)
{
    uint32_t reg_pfc_modify_cmd_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT) ||
       (last_valid_node >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pfc_modify_cmd_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, PFC_MODIFY_CMD_CFG, LAST_VALID_NODE, reg_pfc_modify_cmd_cfg, last_valid_node);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, PFC_MODIFY_CMD_CFG, reg_pfc_modify_cmd_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_pfc_modify_cmd_cfg_get(uint8_t gsu_reg_id, uint16_t *last_valid_node)
{
    uint32_t reg_pfc_modify_cmd_cfg;

#ifdef VALIDATE_PARMS
    if (!last_valid_node)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, PFC_MODIFY_CMD_CFG, reg_pfc_modify_cmd_cfg);

    *last_valid_node = RU_FIELD_GET(gsu_reg_id, GSU_REG, PFC_MODIFY_CMD_CFG, LAST_VALID_NODE, reg_pfc_modify_cmd_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_pd_fifo_cfg_set(uint8_t gsu_reg_id, uint8_t pds_per_queue)
{
    uint32_t reg_pd_fifo_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT) ||
       (pds_per_queue >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pd_fifo_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, PD_FIFO_CFG, PDS_PER_QUEUE, reg_pd_fifo_cfg, pds_per_queue);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, PD_FIFO_CFG, reg_pd_fifo_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_pd_fifo_cfg_get(uint8_t gsu_reg_id, uint8_t *pds_per_queue)
{
    uint32_t reg_pd_fifo_cfg;

#ifdef VALIDATE_PARMS
    if (!pds_per_queue)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, PD_FIFO_CFG, reg_pd_fifo_cfg);

    *pds_per_queue = RU_FIELD_GET(gsu_reg_id, GSU_REG, PD_FIFO_CFG, PDS_PER_QUEUE, reg_pd_fifo_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_pfc_status_cfg_set(uint8_t gsu_reg_id, uint16_t status_return_addr)
{
    uint32_t reg_pfc_status_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pfc_status_cfg = RU_FIELD_SET(gsu_reg_id, GSU_REG, PFC_STATUS_CFG, STATUS_RETURN_ADDR, reg_pfc_status_cfg, status_return_addr);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, PFC_STATUS_CFG, reg_pfc_status_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_pfc_status_cfg_get(uint8_t gsu_reg_id, uint16_t *status_return_addr)
{
    uint32_t reg_pfc_status_cfg;

#ifdef VALIDATE_PARMS
    if (!status_return_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, PFC_STATUS_CFG, reg_pfc_status_cfg);

    *status_return_addr = RU_FIELD_GET(gsu_reg_id, GSU_REG, PFC_STATUS_CFG, STATUS_RETURN_ADDR, reg_pfc_status_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_gsu_status_get(uint8_t gsu_reg_id, bdmf_boolean *active)
{
    uint32_t reg_gsu_status;

#ifdef VALIDATE_PARMS
    if (!active)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, GSU_STATUS, reg_gsu_status);

    *active = RU_FIELD_GET(gsu_reg_id, GSU_REG, GSU_STATUS, ACTIVE, reg_gsu_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_clk_gate_cntrl_set(uint8_t gsu_reg_id, const gsu_reg_clk_gate_cntrl *clk_gate_cntrl)
{
    uint32_t reg_clk_gate_cntrl = 0;

#ifdef VALIDATE_PARMS
    if(!clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT) ||
       (clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
       (clk_gate_cntrl->keep_alive_en >= _1BITS_MAX_VAL_) ||
       (clk_gate_cntrl->keep_alive_intrvl >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_clk_gate_cntrl = RU_FIELD_SET(gsu_reg_id, GSU_REG, CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_clk_gate_cntrl, clk_gate_cntrl->bypass_clk_gate);
    reg_clk_gate_cntrl = RU_FIELD_SET(gsu_reg_id, GSU_REG, CLK_GATE_CNTRL, TIMER_VAL, reg_clk_gate_cntrl, clk_gate_cntrl->timer_val);
    reg_clk_gate_cntrl = RU_FIELD_SET(gsu_reg_id, GSU_REG, CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_clk_gate_cntrl, clk_gate_cntrl->keep_alive_en);
    reg_clk_gate_cntrl = RU_FIELD_SET(gsu_reg_id, GSU_REG, CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_clk_gate_cntrl, clk_gate_cntrl->keep_alive_intrvl);
    reg_clk_gate_cntrl = RU_FIELD_SET(gsu_reg_id, GSU_REG, CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_clk_gate_cntrl, clk_gate_cntrl->keep_alive_cyc);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, CLK_GATE_CNTRL, reg_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_clk_gate_cntrl_get(uint8_t gsu_reg_id, gsu_reg_clk_gate_cntrl *clk_gate_cntrl)
{
    uint32_t reg_clk_gate_cntrl;

#ifdef VALIDATE_PARMS
    if (!clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, CLK_GATE_CNTRL, reg_clk_gate_cntrl);

    clk_gate_cntrl->bypass_clk_gate = RU_FIELD_GET(gsu_reg_id, GSU_REG, CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_clk_gate_cntrl);
    clk_gate_cntrl->timer_val = RU_FIELD_GET(gsu_reg_id, GSU_REG, CLK_GATE_CNTRL, TIMER_VAL, reg_clk_gate_cntrl);
    clk_gate_cntrl->keep_alive_en = RU_FIELD_GET(gsu_reg_id, GSU_REG, CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_clk_gate_cntrl);
    clk_gate_cntrl->keep_alive_intrvl = RU_FIELD_GET(gsu_reg_id, GSU_REG, CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_clk_gate_cntrl);
    clk_gate_cntrl->keep_alive_cyc = RU_FIELD_GET(gsu_reg_id, GSU_REG, CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_pd_queue_id_en_set(uint8_t gsu_reg_id, bdmf_boolean en_send_queue_id, uint16_t queue_id_return_addr)
{
    uint32_t reg_pd_queue_id_en = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT) ||
       (en_send_queue_id >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pd_queue_id_en = RU_FIELD_SET(gsu_reg_id, GSU_REG, PD_QUEUE_ID_EN, EN_SEND_QUEUE_ID, reg_pd_queue_id_en, en_send_queue_id);
    reg_pd_queue_id_en = RU_FIELD_SET(gsu_reg_id, GSU_REG, PD_QUEUE_ID_EN, QUEUE_ID_RETURN_ADDR, reg_pd_queue_id_en, queue_id_return_addr);

    RU_REG_WRITE(gsu_reg_id, GSU_REG, PD_QUEUE_ID_EN, reg_pd_queue_id_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_reg_pd_queue_id_en_get(uint8_t gsu_reg_id, bdmf_boolean *en_send_queue_id, uint16_t *queue_id_return_addr)
{
    uint32_t reg_pd_queue_id_en;

#ifdef VALIDATE_PARMS
    if (!en_send_queue_id || !queue_id_return_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_reg_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(gsu_reg_id, GSU_REG, PD_QUEUE_ID_EN, reg_pd_queue_id_en);

    *en_send_queue_id = RU_FIELD_GET(gsu_reg_id, GSU_REG, PD_QUEUE_ID_EN, EN_SEND_QUEUE_ID, reg_pd_queue_id_en);
    *queue_id_return_addr = RU_FIELD_GET(gsu_reg_id, GSU_REG, PD_QUEUE_ID_EN, QUEUE_ID_RETURN_ADDR, reg_pd_queue_id_en);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_profiling_downcnt,
    bdmf_address_profiling_counter_a,
    bdmf_address_profiling_counter_b,
    bdmf_address_profiling_mode,
    bdmf_address_qm_cfg_1,
    bdmf_address_qm_cfg_2,
    bdmf_address_qm_cfg_3,
    bdmf_address_transmit_cmd_cfg,
    bdmf_address_rl_status_cfg,
    bdmf_address_rl_fetch_cfg,
    bdmf_address_misc_cfg,
    bdmf_address_pause_cfg,
    bdmf_address_aqm_cmd_cfg,
    bdmf_address_flush_cmd_cfg,
    bdmf_address_rl_update_cmd_cfg,
    bdmf_address_pfc_modify_cmd_cfg,
    bdmf_address_pd_fifo_cfg,
    bdmf_address_pfc_status_cfg,
    bdmf_address_gsu_status,
    bdmf_address_clk_gate_cntrl,
    bdmf_address_pd_queue_id_en,
}
bdmf_address;

static int ag_drv_gsu_reg_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_gsu_reg_profiling_downcnt:
        ag_err = ag_drv_gsu_reg_profiling_downcnt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_gsu_reg_profiling_mode:
        ag_err = ag_drv_gsu_reg_profiling_mode_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_gsu_reg_qm_cfg_1:
        ag_err = ag_drv_gsu_reg_qm_cfg_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_gsu_reg_qm_cfg_2:
        ag_err = ag_drv_gsu_reg_qm_cfg_2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_gsu_reg_qm_cfg_3:
        ag_err = ag_drv_gsu_reg_qm_cfg_3_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_gsu_reg_transmit_cmd_cfg:
        ag_err = ag_drv_gsu_reg_transmit_cmd_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_gsu_reg_rl_status_cfg:
        ag_err = ag_drv_gsu_reg_rl_status_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_gsu_reg_rl_fetch_cfg:
        ag_err = ag_drv_gsu_reg_rl_fetch_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_gsu_reg_misc_cfg:
    {
        gsu_reg_misc_cfg misc_cfg = { .dont_transmit_last_pd = parm[2].value.unumber, .rl_count_dithering_en = parm[3].value.unumber, .cfg_pck_overhead_bytes_a = parm[4].value.unumber, .cfg_pck_overhead_bytes_b = parm[5].value.unumber, .use_extended_memory = parm[6].value.unumber, .cfg_send_ack_to_rnr = parm[7].value.unumber, .task_num_for_rnr_ack = parm[8].value.unumber, .cfg_send_nack_to_bbhtx = parm[9].value.unumber};
        ag_err = ag_drv_gsu_reg_misc_cfg_set(parm[1].value.unumber, &misc_cfg);
        break;
    }
    case cli_gsu_reg_pause_cfg:
        ag_err = ag_drv_gsu_reg_pause_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_gsu_reg_aqm_cmd_cfg:
        ag_err = ag_drv_gsu_reg_aqm_cmd_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_gsu_reg_flush_cmd_cfg:
        ag_err = ag_drv_gsu_reg_flush_cmd_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_gsu_reg_rl_update_cmd_cfg:
        ag_err = ag_drv_gsu_reg_rl_update_cmd_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_gsu_reg_pfc_modify_cmd_cfg:
        ag_err = ag_drv_gsu_reg_pfc_modify_cmd_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_gsu_reg_pd_fifo_cfg:
        ag_err = ag_drv_gsu_reg_pd_fifo_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_gsu_reg_pfc_status_cfg:
        ag_err = ag_drv_gsu_reg_pfc_status_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_gsu_reg_clk_gate_cntrl:
    {
        gsu_reg_clk_gate_cntrl clk_gate_cntrl = { .bypass_clk_gate = parm[2].value.unumber, .timer_val = parm[3].value.unumber, .keep_alive_en = parm[4].value.unumber, .keep_alive_intrvl = parm[5].value.unumber, .keep_alive_cyc = parm[6].value.unumber};
        ag_err = ag_drv_gsu_reg_clk_gate_cntrl_set(parm[1].value.unumber, &clk_gate_cntrl);
        break;
    }
    case cli_gsu_reg_pd_queue_id_en:
        ag_err = ag_drv_gsu_reg_pd_queue_id_en_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_gsu_reg_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_gsu_reg_profiling_downcnt:
    {
        uint32_t downcnt_value;
        ag_err = ag_drv_gsu_reg_profiling_downcnt_get(parm[1].value.unumber, &downcnt_value);
        bdmf_session_print(session, "downcnt_value = %u = 0x%x\n", downcnt_value, downcnt_value);
        break;
    }
    case cli_gsu_reg_profiling_counter_a:
    {
        uint32_t value;
        ag_err = ag_drv_gsu_reg_profiling_counter_a_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_gsu_reg_profiling_counter_b:
    {
        uint32_t value;
        ag_err = ag_drv_gsu_reg_profiling_counter_b_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_gsu_reg_profiling_mode:
    {
        bdmf_boolean counter_a_mode;
        uint8_t counter_b_mode;
        uint8_t command_id;
        ag_err = ag_drv_gsu_reg_profiling_mode_get(parm[1].value.unumber, &counter_a_mode, &counter_b_mode, &command_id);
        bdmf_session_print(session, "counter_a_mode = %u = 0x%x\n", counter_a_mode, counter_a_mode);
        bdmf_session_print(session, "counter_b_mode = %u = 0x%x\n", counter_b_mode, counter_b_mode);
        bdmf_session_print(session, "command_id = %u = 0x%x\n", command_id, command_id);
        break;
    }
    case cli_gsu_reg_qm_cfg_1:
    {
        uint16_t qm_0_base_queue;
        uint16_t qm_1_base_queue;
        ag_err = ag_drv_gsu_reg_qm_cfg_1_get(parm[1].value.unumber, &qm_0_base_queue, &qm_1_base_queue);
        bdmf_session_print(session, "qm_0_base_queue = %u = 0x%x\n", qm_0_base_queue, qm_0_base_queue);
        bdmf_session_print(session, "qm_1_base_queue = %u = 0x%x\n", qm_1_base_queue, qm_1_base_queue);
        break;
    }
    case cli_gsu_reg_qm_cfg_2:
    {
        uint16_t pd_fifo_qm0;
        uint16_t pd_fifo_qm1;
        ag_err = ag_drv_gsu_reg_qm_cfg_2_get(parm[1].value.unumber, &pd_fifo_qm0, &pd_fifo_qm1);
        bdmf_session_print(session, "pd_fifo_qm0 = %u = 0x%x\n", pd_fifo_qm0, pd_fifo_qm0);
        bdmf_session_print(session, "pd_fifo_qm1 = %u = 0x%x\n", pd_fifo_qm1, pd_fifo_qm1);
        break;
    }
    case cli_gsu_reg_qm_cfg_3:
    {
        uint16_t num_of_qm_0_queues;
        ag_err = ag_drv_gsu_reg_qm_cfg_3_get(parm[1].value.unumber, &num_of_qm_0_queues);
        bdmf_session_print(session, "num_of_qm_0_queues = %u = 0x%x\n", num_of_qm_0_queues, num_of_qm_0_queues);
        break;
    }
    case cli_gsu_reg_transmit_cmd_cfg:
    {
        uint16_t pd_return_addr;
        uint16_t status_return_addr;
        ag_err = ag_drv_gsu_reg_transmit_cmd_cfg_get(parm[1].value.unumber, &pd_return_addr, &status_return_addr);
        bdmf_session_print(session, "pd_return_addr = %u = 0x%x\n", pd_return_addr, pd_return_addr);
        bdmf_session_print(session, "status_return_addr = %u = 0x%x\n", status_return_addr, status_return_addr);
        break;
    }
    case cli_gsu_reg_rl_status_cfg:
    {
        uint16_t forced_incr_rl_status_addr;
        uint16_t periodic_rl_status_addr;
        ag_err = ag_drv_gsu_reg_rl_status_cfg_get(parm[1].value.unumber, &forced_incr_rl_status_addr, &periodic_rl_status_addr);
        bdmf_session_print(session, "forced_incr_rl_status_addr = %u = 0x%x\n", forced_incr_rl_status_addr, forced_incr_rl_status_addr);
        bdmf_session_print(session, "periodic_rl_status_addr = %u = 0x%x\n", periodic_rl_status_addr, periodic_rl_status_addr);
        break;
    }
    case cli_gsu_reg_rl_fetch_cfg:
    {
        uint16_t fecth_rl_base_addr;
        ag_err = ag_drv_gsu_reg_rl_fetch_cfg_get(parm[1].value.unumber, &fecth_rl_base_addr);
        bdmf_session_print(session, "fecth_rl_base_addr = %u = 0x%x\n", fecth_rl_base_addr, fecth_rl_base_addr);
        break;
    }
    case cli_gsu_reg_misc_cfg:
    {
        gsu_reg_misc_cfg misc_cfg;
        ag_err = ag_drv_gsu_reg_misc_cfg_get(parm[1].value.unumber, &misc_cfg);
        bdmf_session_print(session, "dont_transmit_last_pd = %u = 0x%x\n", misc_cfg.dont_transmit_last_pd, misc_cfg.dont_transmit_last_pd);
        bdmf_session_print(session, "rl_count_dithering_en = %u = 0x%x\n", misc_cfg.rl_count_dithering_en, misc_cfg.rl_count_dithering_en);
        bdmf_session_print(session, "cfg_pck_overhead_bytes_a = %u = 0x%x\n", misc_cfg.cfg_pck_overhead_bytes_a, misc_cfg.cfg_pck_overhead_bytes_a);
        bdmf_session_print(session, "cfg_pck_overhead_bytes_b = %u = 0x%x\n", misc_cfg.cfg_pck_overhead_bytes_b, misc_cfg.cfg_pck_overhead_bytes_b);
        bdmf_session_print(session, "use_extended_memory = %u = 0x%x\n", misc_cfg.use_extended_memory, misc_cfg.use_extended_memory);
        bdmf_session_print(session, "cfg_send_ack_to_rnr = %u = 0x%x\n", misc_cfg.cfg_send_ack_to_rnr, misc_cfg.cfg_send_ack_to_rnr);
        bdmf_session_print(session, "task_num_for_rnr_ack = %u = 0x%x\n", misc_cfg.task_num_for_rnr_ack, misc_cfg.task_num_for_rnr_ack);
        bdmf_session_print(session, "cfg_send_nack_to_bbhtx = %u = 0x%x\n", misc_cfg.cfg_send_nack_to_bbhtx, misc_cfg.cfg_send_nack_to_bbhtx);
        break;
    }
    case cli_gsu_reg_pause_cfg:
    {
        bdmf_boolean pause;
        bdmf_boolean step_mode;
        bdmf_boolean do_single_command;
        ag_err = ag_drv_gsu_reg_pause_cfg_get(parm[1].value.unumber, &pause, &step_mode, &do_single_command);
        bdmf_session_print(session, "pause = %u = 0x%x\n", pause, pause);
        bdmf_session_print(session, "step_mode = %u = 0x%x\n", step_mode, step_mode);
        bdmf_session_print(session, "do_single_command = %u = 0x%x\n", do_single_command, do_single_command);
        break;
    }
    case cli_gsu_reg_aqm_cmd_cfg:
    {
        uint16_t aqm_return_addr;
        ag_err = ag_drv_gsu_reg_aqm_cmd_cfg_get(parm[1].value.unumber, &aqm_return_addr);
        bdmf_session_print(session, "aqm_return_addr = %u = 0x%x\n", aqm_return_addr, aqm_return_addr);
        break;
    }
    case cli_gsu_reg_flush_cmd_cfg:
    {
        uint16_t status_return_addr;
        ag_err = ag_drv_gsu_reg_flush_cmd_cfg_get(parm[1].value.unumber, &status_return_addr);
        bdmf_session_print(session, "status_return_addr = %u = 0x%x\n", status_return_addr, status_return_addr);
        break;
    }
    case cli_gsu_reg_rl_update_cmd_cfg:
    {
        uint16_t last_valid_node;
        ag_err = ag_drv_gsu_reg_rl_update_cmd_cfg_get(parm[1].value.unumber, &last_valid_node);
        bdmf_session_print(session, "last_valid_node = %u = 0x%x\n", last_valid_node, last_valid_node);
        break;
    }
    case cli_gsu_reg_pfc_modify_cmd_cfg:
    {
        uint16_t last_valid_node;
        ag_err = ag_drv_gsu_reg_pfc_modify_cmd_cfg_get(parm[1].value.unumber, &last_valid_node);
        bdmf_session_print(session, "last_valid_node = %u = 0x%x\n", last_valid_node, last_valid_node);
        break;
    }
    case cli_gsu_reg_pd_fifo_cfg:
    {
        uint8_t pds_per_queue;
        ag_err = ag_drv_gsu_reg_pd_fifo_cfg_get(parm[1].value.unumber, &pds_per_queue);
        bdmf_session_print(session, "pds_per_queue = %u = 0x%x\n", pds_per_queue, pds_per_queue);
        break;
    }
    case cli_gsu_reg_pfc_status_cfg:
    {
        uint16_t status_return_addr;
        ag_err = ag_drv_gsu_reg_pfc_status_cfg_get(parm[1].value.unumber, &status_return_addr);
        bdmf_session_print(session, "status_return_addr = %u = 0x%x\n", status_return_addr, status_return_addr);
        break;
    }
    case cli_gsu_reg_gsu_status:
    {
        bdmf_boolean active;
        ag_err = ag_drv_gsu_reg_gsu_status_get(parm[1].value.unumber, &active);
        bdmf_session_print(session, "active = %u = 0x%x\n", active, active);
        break;
    }
    case cli_gsu_reg_clk_gate_cntrl:
    {
        gsu_reg_clk_gate_cntrl clk_gate_cntrl;
        ag_err = ag_drv_gsu_reg_clk_gate_cntrl_get(parm[1].value.unumber, &clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u = 0x%x\n", clk_gate_cntrl.bypass_clk_gate, clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u = 0x%x\n", clk_gate_cntrl.timer_val, clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u = 0x%x\n", clk_gate_cntrl.keep_alive_en, clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intrvl = %u = 0x%x\n", clk_gate_cntrl.keep_alive_intrvl, clk_gate_cntrl.keep_alive_intrvl);
        bdmf_session_print(session, "keep_alive_cyc = %u = 0x%x\n", clk_gate_cntrl.keep_alive_cyc, clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    case cli_gsu_reg_pd_queue_id_en:
    {
        bdmf_boolean en_send_queue_id;
        uint16_t queue_id_return_addr;
        ag_err = ag_drv_gsu_reg_pd_queue_id_en_get(parm[1].value.unumber, &en_send_queue_id, &queue_id_return_addr);
        bdmf_session_print(session, "en_send_queue_id = %u = 0x%x\n", en_send_queue_id, en_send_queue_id);
        bdmf_session_print(session, "queue_id_return_addr = %u = 0x%x\n", queue_id_return_addr, queue_id_return_addr);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_gsu_reg_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t gsu_reg_id = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint32_t downcnt_value = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_gsu_reg_profiling_downcnt_set(%u %u)\n", gsu_reg_id,
            downcnt_value);
        ag_err = ag_drv_gsu_reg_profiling_downcnt_set(gsu_reg_id, downcnt_value);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_profiling_downcnt_get(gsu_reg_id, &downcnt_value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_profiling_downcnt_get(%u %u)\n", gsu_reg_id,
                downcnt_value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (downcnt_value != gtmv(m, 32))
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
            ag_err = ag_drv_gsu_reg_profiling_counter_a_get(gsu_reg_id, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_profiling_counter_a_get(%u %u)\n", gsu_reg_id,
                value);
        }
    }

    {
        uint32_t value = gtmv(m, 24);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_profiling_counter_b_get(gsu_reg_id, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_profiling_counter_b_get(%u %u)\n", gsu_reg_id,
                value);
        }
    }

    {
        bdmf_boolean counter_a_mode = gtmv(m, 1);
        uint8_t counter_b_mode = gtmv(m, 2);
        uint8_t command_id = gtmv(m, 3);
        bdmf_session_print(session, "ag_drv_gsu_reg_profiling_mode_set(%u %u %u %u)\n", gsu_reg_id,
            counter_a_mode, counter_b_mode, command_id);
        ag_err = ag_drv_gsu_reg_profiling_mode_set(gsu_reg_id, counter_a_mode, counter_b_mode, command_id);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_profiling_mode_get(gsu_reg_id, &counter_a_mode, &counter_b_mode, &command_id);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_profiling_mode_get(%u %u %u %u)\n", gsu_reg_id,
                counter_a_mode, counter_b_mode, command_id);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (counter_a_mode != gtmv(m, 1) || counter_b_mode != gtmv(m, 2) || command_id != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t qm_0_base_queue = gtmv(m, 15);
        uint16_t qm_1_base_queue = gtmv(m, 15);
        bdmf_session_print(session, "ag_drv_gsu_reg_qm_cfg_1_set(%u %u %u)\n", gsu_reg_id,
            qm_0_base_queue, qm_1_base_queue);
        ag_err = ag_drv_gsu_reg_qm_cfg_1_set(gsu_reg_id, qm_0_base_queue, qm_1_base_queue);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_qm_cfg_1_get(gsu_reg_id, &qm_0_base_queue, &qm_1_base_queue);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_qm_cfg_1_get(%u %u %u)\n", gsu_reg_id,
                qm_0_base_queue, qm_1_base_queue);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (qm_0_base_queue != gtmv(m, 15) || qm_1_base_queue != gtmv(m, 15))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t pd_fifo_qm0 = gtmv(m, 15);
        uint16_t pd_fifo_qm1 = gtmv(m, 15);
        bdmf_session_print(session, "ag_drv_gsu_reg_qm_cfg_2_set(%u %u %u)\n", gsu_reg_id,
            pd_fifo_qm0, pd_fifo_qm1);
        ag_err = ag_drv_gsu_reg_qm_cfg_2_set(gsu_reg_id, pd_fifo_qm0, pd_fifo_qm1);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_qm_cfg_2_get(gsu_reg_id, &pd_fifo_qm0, &pd_fifo_qm1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_qm_cfg_2_get(%u %u %u)\n", gsu_reg_id,
                pd_fifo_qm0, pd_fifo_qm1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pd_fifo_qm0 != gtmv(m, 15) || pd_fifo_qm1 != gtmv(m, 15))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t num_of_qm_0_queues = gtmv(m, 11);
        bdmf_session_print(session, "ag_drv_gsu_reg_qm_cfg_3_set(%u %u)\n", gsu_reg_id,
            num_of_qm_0_queues);
        ag_err = ag_drv_gsu_reg_qm_cfg_3_set(gsu_reg_id, num_of_qm_0_queues);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_qm_cfg_3_get(gsu_reg_id, &num_of_qm_0_queues);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_qm_cfg_3_get(%u %u)\n", gsu_reg_id,
                num_of_qm_0_queues);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (num_of_qm_0_queues != gtmv(m, 11))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t pd_return_addr = gtmv(m, 16);
        uint16_t status_return_addr = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_gsu_reg_transmit_cmd_cfg_set(%u %u %u)\n", gsu_reg_id,
            pd_return_addr, status_return_addr);
        ag_err = ag_drv_gsu_reg_transmit_cmd_cfg_set(gsu_reg_id, pd_return_addr, status_return_addr);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_transmit_cmd_cfg_get(gsu_reg_id, &pd_return_addr, &status_return_addr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_transmit_cmd_cfg_get(%u %u %u)\n", gsu_reg_id,
                pd_return_addr, status_return_addr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pd_return_addr != gtmv(m, 16) || status_return_addr != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t forced_incr_rl_status_addr = gtmv(m, 16);
        uint16_t periodic_rl_status_addr = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_gsu_reg_rl_status_cfg_set(%u %u %u)\n", gsu_reg_id,
            forced_incr_rl_status_addr, periodic_rl_status_addr);
        ag_err = ag_drv_gsu_reg_rl_status_cfg_set(gsu_reg_id, forced_incr_rl_status_addr, periodic_rl_status_addr);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_rl_status_cfg_get(gsu_reg_id, &forced_incr_rl_status_addr, &periodic_rl_status_addr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_rl_status_cfg_get(%u %u %u)\n", gsu_reg_id,
                forced_incr_rl_status_addr, periodic_rl_status_addr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (forced_incr_rl_status_addr != gtmv(m, 16) || periodic_rl_status_addr != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t fecth_rl_base_addr = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_gsu_reg_rl_fetch_cfg_set(%u %u)\n", gsu_reg_id,
            fecth_rl_base_addr);
        ag_err = ag_drv_gsu_reg_rl_fetch_cfg_set(gsu_reg_id, fecth_rl_base_addr);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_rl_fetch_cfg_get(gsu_reg_id, &fecth_rl_base_addr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_rl_fetch_cfg_get(%u %u)\n", gsu_reg_id,
                fecth_rl_base_addr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fecth_rl_base_addr != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        gsu_reg_misc_cfg misc_cfg = {.dont_transmit_last_pd = gtmv(m, 1), .rl_count_dithering_en = gtmv(m, 1), .cfg_pck_overhead_bytes_a = gtmv(m, 6), .cfg_pck_overhead_bytes_b = gtmv(m, 6), .use_extended_memory = gtmv(m, 1), .cfg_send_ack_to_rnr = gtmv(m, 1), .task_num_for_rnr_ack = gtmv(m, 4), .cfg_send_nack_to_bbhtx = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_gsu_reg_misc_cfg_set(%u %u %u %u %u %u %u %u %u)\n", gsu_reg_id,
            misc_cfg.dont_transmit_last_pd, misc_cfg.rl_count_dithering_en, misc_cfg.cfg_pck_overhead_bytes_a, misc_cfg.cfg_pck_overhead_bytes_b, 
            misc_cfg.use_extended_memory, misc_cfg.cfg_send_ack_to_rnr, misc_cfg.task_num_for_rnr_ack, misc_cfg.cfg_send_nack_to_bbhtx);
        ag_err = ag_drv_gsu_reg_misc_cfg_set(gsu_reg_id, &misc_cfg);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_misc_cfg_get(gsu_reg_id, &misc_cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_misc_cfg_get(%u %u %u %u %u %u %u %u %u)\n", gsu_reg_id,
                misc_cfg.dont_transmit_last_pd, misc_cfg.rl_count_dithering_en, misc_cfg.cfg_pck_overhead_bytes_a, misc_cfg.cfg_pck_overhead_bytes_b, 
                misc_cfg.use_extended_memory, misc_cfg.cfg_send_ack_to_rnr, misc_cfg.task_num_for_rnr_ack, misc_cfg.cfg_send_nack_to_bbhtx);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (misc_cfg.dont_transmit_last_pd != gtmv(m, 1) || misc_cfg.rl_count_dithering_en != gtmv(m, 1) || misc_cfg.cfg_pck_overhead_bytes_a != gtmv(m, 6) || misc_cfg.cfg_pck_overhead_bytes_b != gtmv(m, 6) || misc_cfg.use_extended_memory != gtmv(m, 1) || misc_cfg.cfg_send_ack_to_rnr != gtmv(m, 1) || misc_cfg.task_num_for_rnr_ack != gtmv(m, 4) || misc_cfg.cfg_send_nack_to_bbhtx != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean pause = gtmv(m, 1);
        bdmf_boolean step_mode = gtmv(m, 1);
        bdmf_boolean do_single_command = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_gsu_reg_pause_cfg_set(%u %u %u %u)\n", gsu_reg_id,
            pause, step_mode, do_single_command);
        ag_err = ag_drv_gsu_reg_pause_cfg_set(gsu_reg_id, pause, step_mode, do_single_command);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_pause_cfg_get(gsu_reg_id, &pause, &step_mode, &do_single_command);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_pause_cfg_get(%u %u %u %u)\n", gsu_reg_id,
                pause, step_mode, do_single_command);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pause != gtmv(m, 1) || step_mode != gtmv(m, 1) || do_single_command != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t aqm_return_addr = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_gsu_reg_aqm_cmd_cfg_set(%u %u)\n", gsu_reg_id,
            aqm_return_addr);
        ag_err = ag_drv_gsu_reg_aqm_cmd_cfg_set(gsu_reg_id, aqm_return_addr);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_aqm_cmd_cfg_get(gsu_reg_id, &aqm_return_addr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_aqm_cmd_cfg_get(%u %u)\n", gsu_reg_id,
                aqm_return_addr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (aqm_return_addr != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t status_return_addr = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_gsu_reg_flush_cmd_cfg_set(%u %u)\n", gsu_reg_id,
            status_return_addr);
        ag_err = ag_drv_gsu_reg_flush_cmd_cfg_set(gsu_reg_id, status_return_addr);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_flush_cmd_cfg_get(gsu_reg_id, &status_return_addr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_flush_cmd_cfg_get(%u %u)\n", gsu_reg_id,
                status_return_addr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (status_return_addr != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t last_valid_node = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_gsu_reg_rl_update_cmd_cfg_set(%u %u)\n", gsu_reg_id,
            last_valid_node);
        ag_err = ag_drv_gsu_reg_rl_update_cmd_cfg_set(gsu_reg_id, last_valid_node);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_rl_update_cmd_cfg_get(gsu_reg_id, &last_valid_node);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_rl_update_cmd_cfg_get(%u %u)\n", gsu_reg_id,
                last_valid_node);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (last_valid_node != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t last_valid_node = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_gsu_reg_pfc_modify_cmd_cfg_set(%u %u)\n", gsu_reg_id,
            last_valid_node);
        ag_err = ag_drv_gsu_reg_pfc_modify_cmd_cfg_set(gsu_reg_id, last_valid_node);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_pfc_modify_cmd_cfg_get(gsu_reg_id, &last_valid_node);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_pfc_modify_cmd_cfg_get(%u %u)\n", gsu_reg_id,
                last_valid_node);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (last_valid_node != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t pds_per_queue = gtmv(m, 3);
        bdmf_session_print(session, "ag_drv_gsu_reg_pd_fifo_cfg_set(%u %u)\n", gsu_reg_id,
            pds_per_queue);
        ag_err = ag_drv_gsu_reg_pd_fifo_cfg_set(gsu_reg_id, pds_per_queue);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_pd_fifo_cfg_get(gsu_reg_id, &pds_per_queue);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_pd_fifo_cfg_get(%u %u)\n", gsu_reg_id,
                pds_per_queue);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pds_per_queue != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t status_return_addr = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_gsu_reg_pfc_status_cfg_set(%u %u)\n", gsu_reg_id,
            status_return_addr);
        ag_err = ag_drv_gsu_reg_pfc_status_cfg_set(gsu_reg_id, status_return_addr);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_pfc_status_cfg_get(gsu_reg_id, &status_return_addr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_pfc_status_cfg_get(%u %u)\n", gsu_reg_id,
                status_return_addr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (status_return_addr != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean active = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_gsu_status_get(gsu_reg_id, &active);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_gsu_status_get(%u %u)\n", gsu_reg_id,
                active);
        }
    }

    {
        gsu_reg_clk_gate_cntrl clk_gate_cntrl = {.bypass_clk_gate = gtmv(m, 1), .timer_val = gtmv(m, 8), .keep_alive_en = gtmv(m, 1), .keep_alive_intrvl = gtmv(m, 3), .keep_alive_cyc = gtmv(m, 8)};
        bdmf_session_print(session, "ag_drv_gsu_reg_clk_gate_cntrl_set(%u %u %u %u %u %u)\n", gsu_reg_id,
            clk_gate_cntrl.bypass_clk_gate, clk_gate_cntrl.timer_val, clk_gate_cntrl.keep_alive_en, clk_gate_cntrl.keep_alive_intrvl, 
            clk_gate_cntrl.keep_alive_cyc);
        ag_err = ag_drv_gsu_reg_clk_gate_cntrl_set(gsu_reg_id, &clk_gate_cntrl);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_clk_gate_cntrl_get(gsu_reg_id, &clk_gate_cntrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_clk_gate_cntrl_get(%u %u %u %u %u %u)\n", gsu_reg_id,
                clk_gate_cntrl.bypass_clk_gate, clk_gate_cntrl.timer_val, clk_gate_cntrl.keep_alive_en, clk_gate_cntrl.keep_alive_intrvl, 
                clk_gate_cntrl.keep_alive_cyc);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (clk_gate_cntrl.bypass_clk_gate != gtmv(m, 1) || clk_gate_cntrl.timer_val != gtmv(m, 8) || clk_gate_cntrl.keep_alive_en != gtmv(m, 1) || clk_gate_cntrl.keep_alive_intrvl != gtmv(m, 3) || clk_gate_cntrl.keep_alive_cyc != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean en_send_queue_id = gtmv(m, 1);
        uint16_t queue_id_return_addr = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_gsu_reg_pd_queue_id_en_set(%u %u %u)\n", gsu_reg_id,
            en_send_queue_id, queue_id_return_addr);
        ag_err = ag_drv_gsu_reg_pd_queue_id_en_set(gsu_reg_id, en_send_queue_id, queue_id_return_addr);
        if (!ag_err)
            ag_err = ag_drv_gsu_reg_pd_queue_id_en_get(gsu_reg_id, &en_send_queue_id, &queue_id_return_addr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_reg_pd_queue_id_en_get(%u %u %u)\n", gsu_reg_id,
                en_send_queue_id, queue_id_return_addr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (en_send_queue_id != gtmv(m, 1) || queue_id_return_addr != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_gsu_reg_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_profiling_downcnt: reg = &RU_REG(GSU_REG, PROFILING_DOWNCNT); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_profiling_counter_a: reg = &RU_REG(GSU_REG, PROFILING_COUNTER_A); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_profiling_counter_b: reg = &RU_REG(GSU_REG, PROFILING_COUNTER_B); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_profiling_mode: reg = &RU_REG(GSU_REG, PROFILING_MODE); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_qm_cfg_1: reg = &RU_REG(GSU_REG, QM_CFG_1); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_qm_cfg_2: reg = &RU_REG(GSU_REG, QM_CFG_2); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_qm_cfg_3: reg = &RU_REG(GSU_REG, QM_CFG_3); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_transmit_cmd_cfg: reg = &RU_REG(GSU_REG, TRANSMIT_CMD_CFG); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_rl_status_cfg: reg = &RU_REG(GSU_REG, RL_STATUS_CFG); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_rl_fetch_cfg: reg = &RU_REG(GSU_REG, RL_FETCH_CFG); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_misc_cfg: reg = &RU_REG(GSU_REG, MISC_CFG); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_pause_cfg: reg = &RU_REG(GSU_REG, PAUSE_CFG); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_aqm_cmd_cfg: reg = &RU_REG(GSU_REG, AQM_CMD_CFG); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_flush_cmd_cfg: reg = &RU_REG(GSU_REG, FLUSH_CMD_CFG); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_rl_update_cmd_cfg: reg = &RU_REG(GSU_REG, RL_UPDATE_CMD_CFG); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_pfc_modify_cmd_cfg: reg = &RU_REG(GSU_REG, PFC_MODIFY_CMD_CFG); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_pd_fifo_cfg: reg = &RU_REG(GSU_REG, PD_FIFO_CFG); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_pfc_status_cfg: reg = &RU_REG(GSU_REG, PFC_STATUS_CFG); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_gsu_status: reg = &RU_REG(GSU_REG, GSU_STATUS); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_clk_gate_cntrl: reg = &RU_REG(GSU_REG, CLK_GATE_CNTRL); blk = &RU_BLK(GSU_REG); break;
    case bdmf_address_pd_queue_id_en: reg = &RU_REG(GSU_REG, PD_QUEUE_ID_EN); blk = &RU_BLK(GSU_REG); break;
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

bdmfmon_handle_t ag_drv_gsu_reg_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "gsu_reg", "gsu_reg", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_profiling_downcnt[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("downcnt_value", "downcnt_value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiling_mode[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("counter_a_mode", "counter_a_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("counter_b_mode", "counter_b_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("command_id", "command_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qm_cfg_1[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_0_base_queue", "qm_0_base_queue", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("qm_1_base_queue", "qm_1_base_queue", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qm_cfg_2[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pd_fifo_qm0", "pd_fifo_qm0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pd_fifo_qm1", "pd_fifo_qm1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qm_cfg_3[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("num_of_qm_0_queues", "num_of_qm_0_queues", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_transmit_cmd_cfg[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pd_return_addr", "pd_return_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("status_return_addr", "status_return_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rl_status_cfg[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("forced_incr_rl_status_addr", "forced_incr_rl_status_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("periodic_rl_status_addr", "periodic_rl_status_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rl_fetch_cfg[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fecth_rl_base_addr", "fecth_rl_base_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_misc_cfg[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dont_transmit_last_pd", "dont_transmit_last_pd", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rl_count_dithering_en", "rl_count_dithering_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_pck_overhead_bytes_a", "cfg_pck_overhead_bytes_a", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_pck_overhead_bytes_b", "cfg_pck_overhead_bytes_b", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("use_extended_memory", "use_extended_memory", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_send_ack_to_rnr", "cfg_send_ack_to_rnr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("task_num_for_rnr_ack", "task_num_for_rnr_ack", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_send_nack_to_bbhtx", "cfg_send_nack_to_bbhtx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pause_cfg[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pause", "pause", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("step_mode", "step_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("do_single_command", "do_single_command", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_aqm_cmd_cfg[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("aqm_return_addr", "aqm_return_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flush_cmd_cfg[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("status_return_addr", "status_return_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rl_update_cmd_cfg[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("last_valid_node", "last_valid_node", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pfc_modify_cmd_cfg[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("last_valid_node", "last_valid_node", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pd_fifo_cfg[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pds_per_queue", "pds_per_queue", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pfc_status_cfg[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("status_return_addr", "status_return_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_clk_gate_cntrl[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intrvl", "keep_alive_intrvl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pd_queue_id_en[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en_send_queue_id", "en_send_queue_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("queue_id_return_addr", "queue_id_return_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "profiling_downcnt", .val = cli_gsu_reg_profiling_downcnt, .parms = set_profiling_downcnt },
            { .name = "profiling_mode", .val = cli_gsu_reg_profiling_mode, .parms = set_profiling_mode },
            { .name = "qm_cfg_1", .val = cli_gsu_reg_qm_cfg_1, .parms = set_qm_cfg_1 },
            { .name = "qm_cfg_2", .val = cli_gsu_reg_qm_cfg_2, .parms = set_qm_cfg_2 },
            { .name = "qm_cfg_3", .val = cli_gsu_reg_qm_cfg_3, .parms = set_qm_cfg_3 },
            { .name = "transmit_cmd_cfg", .val = cli_gsu_reg_transmit_cmd_cfg, .parms = set_transmit_cmd_cfg },
            { .name = "rl_status_cfg", .val = cli_gsu_reg_rl_status_cfg, .parms = set_rl_status_cfg },
            { .name = "rl_fetch_cfg", .val = cli_gsu_reg_rl_fetch_cfg, .parms = set_rl_fetch_cfg },
            { .name = "misc_cfg", .val = cli_gsu_reg_misc_cfg, .parms = set_misc_cfg },
            { .name = "pause_cfg", .val = cli_gsu_reg_pause_cfg, .parms = set_pause_cfg },
            { .name = "aqm_cmd_cfg", .val = cli_gsu_reg_aqm_cmd_cfg, .parms = set_aqm_cmd_cfg },
            { .name = "flush_cmd_cfg", .val = cli_gsu_reg_flush_cmd_cfg, .parms = set_flush_cmd_cfg },
            { .name = "rl_update_cmd_cfg", .val = cli_gsu_reg_rl_update_cmd_cfg, .parms = set_rl_update_cmd_cfg },
            { .name = "pfc_modify_cmd_cfg", .val = cli_gsu_reg_pfc_modify_cmd_cfg, .parms = set_pfc_modify_cmd_cfg },
            { .name = "pd_fifo_cfg", .val = cli_gsu_reg_pd_fifo_cfg, .parms = set_pd_fifo_cfg },
            { .name = "pfc_status_cfg", .val = cli_gsu_reg_pfc_status_cfg, .parms = set_pfc_status_cfg },
            { .name = "clk_gate_cntrl", .val = cli_gsu_reg_clk_gate_cntrl, .parms = set_clk_gate_cntrl },
            { .name = "pd_queue_id_en", .val = cli_gsu_reg_pd_queue_id_en, .parms = set_pd_queue_id_en },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_gsu_reg_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "profiling_downcnt", .val = cli_gsu_reg_profiling_downcnt, .parms = get_default },
            { .name = "profiling_counter_a", .val = cli_gsu_reg_profiling_counter_a, .parms = get_default },
            { .name = "profiling_counter_b", .val = cli_gsu_reg_profiling_counter_b, .parms = get_default },
            { .name = "profiling_mode", .val = cli_gsu_reg_profiling_mode, .parms = get_default },
            { .name = "qm_cfg_1", .val = cli_gsu_reg_qm_cfg_1, .parms = get_default },
            { .name = "qm_cfg_2", .val = cli_gsu_reg_qm_cfg_2, .parms = get_default },
            { .name = "qm_cfg_3", .val = cli_gsu_reg_qm_cfg_3, .parms = get_default },
            { .name = "transmit_cmd_cfg", .val = cli_gsu_reg_transmit_cmd_cfg, .parms = get_default },
            { .name = "rl_status_cfg", .val = cli_gsu_reg_rl_status_cfg, .parms = get_default },
            { .name = "rl_fetch_cfg", .val = cli_gsu_reg_rl_fetch_cfg, .parms = get_default },
            { .name = "misc_cfg", .val = cli_gsu_reg_misc_cfg, .parms = get_default },
            { .name = "pause_cfg", .val = cli_gsu_reg_pause_cfg, .parms = get_default },
            { .name = "aqm_cmd_cfg", .val = cli_gsu_reg_aqm_cmd_cfg, .parms = get_default },
            { .name = "flush_cmd_cfg", .val = cli_gsu_reg_flush_cmd_cfg, .parms = get_default },
            { .name = "rl_update_cmd_cfg", .val = cli_gsu_reg_rl_update_cmd_cfg, .parms = get_default },
            { .name = "pfc_modify_cmd_cfg", .val = cli_gsu_reg_pfc_modify_cmd_cfg, .parms = get_default },
            { .name = "pd_fifo_cfg", .val = cli_gsu_reg_pd_fifo_cfg, .parms = get_default },
            { .name = "pfc_status_cfg", .val = cli_gsu_reg_pfc_status_cfg, .parms = get_default },
            { .name = "gsu_status", .val = cli_gsu_reg_gsu_status, .parms = get_default },
            { .name = "clk_gate_cntrl", .val = cli_gsu_reg_clk_gate_cntrl, .parms = get_default },
            { .name = "pd_queue_id_en", .val = cli_gsu_reg_pd_queue_id_en, .parms = get_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_gsu_reg_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_gsu_reg_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("gsu_reg_id", "gsu_reg_id", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "PROFILING_DOWNCNT", .val = bdmf_address_profiling_downcnt },
            { .name = "PROFILING_COUNTER_A", .val = bdmf_address_profiling_counter_a },
            { .name = "PROFILING_COUNTER_B", .val = bdmf_address_profiling_counter_b },
            { .name = "PROFILING_MODE", .val = bdmf_address_profiling_mode },
            { .name = "QM_CFG_1", .val = bdmf_address_qm_cfg_1 },
            { .name = "QM_CFG_2", .val = bdmf_address_qm_cfg_2 },
            { .name = "QM_CFG_3", .val = bdmf_address_qm_cfg_3 },
            { .name = "TRANSMIT_CMD_CFG", .val = bdmf_address_transmit_cmd_cfg },
            { .name = "RL_STATUS_CFG", .val = bdmf_address_rl_status_cfg },
            { .name = "RL_FETCH_CFG", .val = bdmf_address_rl_fetch_cfg },
            { .name = "MISC_CFG", .val = bdmf_address_misc_cfg },
            { .name = "PAUSE_CFG", .val = bdmf_address_pause_cfg },
            { .name = "AQM_CMD_CFG", .val = bdmf_address_aqm_cmd_cfg },
            { .name = "FLUSH_CMD_CFG", .val = bdmf_address_flush_cmd_cfg },
            { .name = "RL_UPDATE_CMD_CFG", .val = bdmf_address_rl_update_cmd_cfg },
            { .name = "PFC_MODIFY_CMD_CFG", .val = bdmf_address_pfc_modify_cmd_cfg },
            { .name = "PD_FIFO_CFG", .val = bdmf_address_pd_fifo_cfg },
            { .name = "PFC_STATUS_CFG", .val = bdmf_address_pfc_status_cfg },
            { .name = "GSU_STATUS", .val = bdmf_address_gsu_status },
            { .name = "CLK_GATE_CNTRL", .val = bdmf_address_clk_gate_cntrl },
            { .name = "PD_QUEUE_ID_EN", .val = bdmf_address_pd_queue_id_en },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_gsu_reg_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "gsu_reg_id", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
