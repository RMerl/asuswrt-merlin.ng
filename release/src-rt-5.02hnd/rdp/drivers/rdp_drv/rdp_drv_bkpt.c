/*
    <:copyright-BRCM:2015-2016:DUAL/GPL:standard
    
       Copyright (c) 2015-2016 Broadcom 
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

#ifdef USE_BDMF_SHELL
#include "rdp_drv_shell.h"
#endif
#include "rdp_drv_bkpt.h"
#include "rdp_drv_dis_reor.h"
#include "rdp_drv_rnr.h"
#include "rdp_drv_bkpt.h"
#include "access_macros.h"
#include "rdd_runner_0_labels.h"

int rdp_drv_bkpt_init(void)
{
    uint16_t handlr_addr, pc_val;
    uint32_t quad_idx;
    int rc = BDMF_ERR_OK;

    for (quad_idx = 0; !rc && quad_idx <= RNR_QUAD_ID_LAST; quad_idx++)
    {
        rc = ag_drv_rnr_quad_general_config_bkpt_gen_cfg_get(quad_idx, &handlr_addr, &pc_val);
        handlr_addr = image_0_debug_routine_handler >> 2;
        rc = rc ? rc : ag_drv_rnr_quad_general_config_bkpt_gen_cfg_set(quad_idx, handlr_addr, pc_val);
    }
    return rc;
}

#ifdef USE_BDMF_SHELL

bdmfmon_handle_t drv_bkpt_dir;

static int _rdp_drv_bkpt_rnr_en(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    return ag_drv_rnr_regs_rnr_enable_set((uint32_t)parm[0].value.unumber, (bdmf_boolean)parm[1].value.unumber);
}

static int _rdp_drv_bkpt_step_mode_cfg(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    bdmf_error_t rc;
    rnr_regs_cfg_bkpt_cfg bps_cfg = {};

    rc = ag_drv_rnr_regs_cfg_bkpt_cfg_get((uint32_t)parm[0].value.unumber, &bps_cfg);
    if (rc)
        return rc;
    bps_cfg.step_mode = (bdmf_boolean)parm[1].value.unumber;
    return ag_drv_rnr_regs_cfg_bkpt_cfg_set((uint32_t)parm[0].value.unumber, &bps_cfg);
}

#define RUNNER_BP_CFG_SET(rnr_idx, bp_idx, bp_en, bp_addr, use_thread, thread_idx) \
    rnr_regs_cfg_bkpt_cfg bps_cfg = {}; \
    rc = ag_drv_rnr_regs_cfg_bkpt_cfg_get(rnr_idx, &bps_cfg); \
    if (rc) \
        return rc; \
    bps_cfg.bkpt_##bp_idx##_en = bp_en; \
    bps_cfg.bkpt_##bp_idx##_use_thread = use_thread; \
    rc = ag_drv_rnr_quad_general_config_bkpt_##bp_idx##_cfg_set(rnr_idx / 4, bp_addr, thread_idx); \
    rc = rc ? rc : ag_drv_rnr_regs_cfg_bkpt_cfg_set(rnr_idx, &bps_cfg);

static int _rdp_drv_bkpt_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t rnr_idx, bp_idx, bp_en, bp_addr, use_thread, thread_idx;
	int rc;

    rnr_idx = (uint32_t)parm[0].value.unumber;
    bp_idx = (uint32_t)parm[1].value.unumber;
    bp_en = (uint32_t)parm[2].value.unumber;
    bp_addr = (uint32_t)parm[3].value.unumber >> 2;
    use_thread = (uint32_t)parm[4].value.unumber;
    thread_idx = (uint32_t)parm[5].value.unumber;

    switch (bp_idx)
    {
        case 0:
        {
            RUNNER_BP_CFG_SET(rnr_idx, 0, bp_en, bp_addr, use_thread, thread_idx);
            break;
        }
        case 1:
        {
            RUNNER_BP_CFG_SET(rnr_idx, 1, bp_en, bp_addr, use_thread, thread_idx);
            break;
        }
        case 2:
        {
            RUNNER_BP_CFG_SET(rnr_idx, 2, bp_en, bp_addr, use_thread, thread_idx);
            break;
        }
        case 3:
        {
            RUNNER_BP_CFG_SET(rnr_idx, 3, bp_en, bp_addr, use_thread, thread_idx);
            break;
        }
        case 4:
        {
            RUNNER_BP_CFG_SET(rnr_idx, 4, bp_en, bp_addr, use_thread, thread_idx);
            break;
        }
        case 5:
        {
            RUNNER_BP_CFG_SET(rnr_idx, 5, bp_en, bp_addr, use_thread, thread_idx);
            break;
        }
        case 6:
        {
            RUNNER_BP_CFG_SET(rnr_idx, 6, bp_en, bp_addr, use_thread, thread_idx);
            break;
        }
        case 7:
        {
            RUNNER_BP_CFG_SET(rnr_idx, 7, bp_en, bp_addr, use_thread, thread_idx);
            break;
        }
        default:
            return BDMF_ERR_PARM;
    }
    return rc;
}

static int _rdp_drv_bkpt_print_regs(bdmf_session_handle session, uint32_t rnr_idx)
{
    int rc;
    bdmf_boolean bp_active;
    uint16_t bp_addr;
    uint32_t reg_idx, reg_val, thrd_idx;
    RDD_REGISTERS_BUFFER_DTS *regs_buf;
    RDD_TASK_IDX_DTS *thrd_idx_buf;

    rc = ag_drv_rnr_regs_cfg_bkpt_sts_get(rnr_idx, &bp_addr, &bp_active);
    if (rc)
        return rc;

    if (!bp_active)
    {
        bdmf_session_print(session, "Runner %d Breakpoint is not active\n", rnr_idx);
        /* Decided to return in case bp is not active */
        return rc;
    }
    bdmf_session_print(session, "Runner %d Breakpoint is active\n", rnr_idx);
    bdmf_session_print(session, "----------------\n");
    bdmf_session_print(session, "Breakpoint address:  0x0%-4x (0x%-4x)\n\n", bp_addr << 2, bp_addr);

    // read current thread idx
    thrd_idx_buf = RDD_TASK_IDX_PTR(rnr_idx);
    MREAD_32(&(thrd_idx_buf->entry),thrd_idx);
    thrd_idx = thrd_idx & (RUNNER_MAX_TASKS_NUM - 1);
    bdmf_session_print(session, "Current thread number: 0x%-8x\n\n", thrd_idx);

    // read thread context registers
    regs_buf = RDD_REGISTERS_BUFFER_PTR(rnr_idx);
    for (reg_idx = 0; reg_idx < RDD_REGISTERS_BUFFER_SIZE; reg_idx++)
    {
        MREAD_32((uint8_t *)&(regs_buf->entry[reg_idx]), reg_val);
        bdmf_session_print(session, "register %2u = 0x%-8x\n", reg_idx, reg_val);
    }
    bdmf_session_print(session, "\n");

    return rc;
}

static int _rdp_drv_bkpt_sts(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    return _rdp_drv_bkpt_print_regs(session, (uint32_t)parm[0].value.unumber);
}

static int _rdp_drv_bkpt_cores_bp_sts(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t rnr_idx;
    int rc = BDMF_ERR_OK;

    for (rnr_idx = 0; rnr_idx <= RNR_LAST; rnr_idx++)
        rc = _rdp_drv_bkpt_print_regs(session, rnr_idx);

    return rc;
}

#define QUAD_BP_CONFIG_TRACE(quad_idx, bp_idx, addr, thread) \
    rc = ag_drv_rnr_quad_general_config_bkpt_##bp_idx##_cfg_get(quad_idx, &addr, &thread); \
    if (rc) \
        return rc; \
    bdmf_trace("bkpt[%d]: addr = 0x%x, thread = %d\n", bp_idx, addr << 2, thread); \


int rdp_drv_bkpt_cfg(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t thread;
    uint16_t addr;
    uint32_t rnr_idx, quad_idx = (uint32_t)parm[0].value.unumber;
    static uint32_t quad_bkpt_cfg[] = {cli_rnr_quad_general_config_bkpt_gen_cfg};
    static uint32_t rnr_bkpt_cfg[] = {cli_rnr_regs_cfg_bkpt_cfg};
    int rc = BDMF_ERR_OK;

    bdmf_session_print(session, "Runner Quad [%d] bkpt configrations:\n", quad_idx);
    QUAD_BP_CONFIG_TRACE(quad_idx, 0, addr, thread);
    QUAD_BP_CONFIG_TRACE(quad_idx, 1, addr, thread);
    QUAD_BP_CONFIG_TRACE(quad_idx, 2, addr, thread);
    QUAD_BP_CONFIG_TRACE(quad_idx, 3, addr, thread);
    QUAD_BP_CONFIG_TRACE(quad_idx, 4, addr, thread);
    QUAD_BP_CONFIG_TRACE(quad_idx, 5, addr, thread);
    QUAD_BP_CONFIG_TRACE(quad_idx, 6, addr, thread);
    QUAD_BP_CONFIG_TRACE(quad_idx, 7, addr, thread);

    HAL_CLI_IDX_PRINT_LIST(session, rnr_quad, quad_bkpt_cfg, quad_idx);
    for (rnr_idx = 0; !rc && rnr_idx < 4; rnr_idx++)
    {
        bdmf_session_print(session, "core %d bkpt configurations:\n", rnr_idx + (quad_idx * 4));
        HAL_CLI_IDX_PRINT_LIST(session, rnr_regs, rnr_bkpt_cfg, rnr_idx);
    }

    return rc;
}

int rdp_drv_bkpt_cli_init(bdmfmon_handle_t driver_dir)
{
    drv_bkpt_dir = bdmfmon_dir_add(driver_dir, "bkpt", "breakpoints", BDMF_ACCESS_ADMIN, NULL);

    BDMFMON_MAKE_CMD(drv_bkpt_dir, "re", "enable or disable runner", _rdp_drv_bkpt_rnr_en,
        BDMFMON_MAKE_PARM_RANGE("core", "core index", BDMFMON_PARM_NUMBER, 0, 0, RNR_LAST),
        BDMFMON_MAKE_PARM_RANGE("enable", "enable=1, disable=0", BDMFMON_PARM_NUMBER, 0, 0, 1));

    BDMFMON_MAKE_CMD(drv_bkpt_dir, "bc",      "enable disable breakpoint step mode", _rdp_drv_bkpt_step_mode_cfg,
        BDMFMON_MAKE_PARM_RANGE("core", "core index", BDMFMON_PARM_NUMBER, 0, 0, RNR_LAST),
        BDMFMON_MAKE_PARM_RANGE("step_mode", "enable=1, disable=0", BDMFMON_PARM_NUMBER, 0, 0, 1));

    BDMFMON_MAKE_CMD(drv_bkpt_dir, "bs",      "set breakpoint", _rdp_drv_bkpt_set,
        BDMFMON_MAKE_PARM_RANGE("core", "core index", BDMFMON_PARM_NUMBER, 0, 0, RNR_LAST),
        BDMFMON_MAKE_PARM_RANGE("index", "Breakpoint index", BDMFMON_PARM_NUMBER, 0, 0, (CORE_BREAK_POINT_NUM - 1)),
        BDMFMON_MAKE_PARM_RANGE("enable", "enable=1, disable=0 breakpoint", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("breakpoint_address", "Address (16 bit)", BDMFMON_PARM_HEX, 0, 0, 0x1FFF),
        BDMFMON_MAKE_PARM_RANGE("use_thread", "enable=1, disable=0", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("thread", "Thread number", BDMFMON_PARM_NUMBER, 0, 0, (RUNNER_MAX_TASKS_NUM - 1)));

    BDMFMON_MAKE_CMD(drv_bkpt_dir, "pbs",     "print breakpoint status", _rdp_drv_bkpt_sts,
        BDMFMON_MAKE_PARM_RANGE("core", "core index", BDMFMON_PARM_NUMBER, 0, 0, RNR_LAST));

    BDMFMON_MAKE_CMD(drv_bkpt_dir, "pbm", "print breakpoints mapping & configurations", rdp_drv_bkpt_cfg,
        BDMFMON_MAKE_PARM_RANGE("quad", "quad index", BDMFMON_PARM_NUMBER, 0, 0, RNR_QUAD_ID_LAST));

    BDMFMON_MAKE_CMD_NOPARM(drv_bkpt_dir, "pcs", "print breakpoints status in all cores", _rdp_drv_bkpt_cores_bp_sts);

    return 0;
}

#endif
