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

#include "rdp_subsystem_common.h"
#include "rdp_drv_sbpm.h"
#include "rdp_common.h"

static inline uint16_t drv_sbpm_get_abs_val(uint16_t val1, uint16_t val2)
{
    return (val1 < val2) ? (val2-val1) : (val1-val2);
}

bdmf_error_t drv_sbpm_thr_ug0_set(const sbpm_thr_ug *thr_ug)
{
    uint32_t reg_regs_sbpm_ug0_trsh = 0;
    uint32_t reg_regs_sbpm_ug0_excl_high_trsh = 0;
    uint32_t reg_regs_sbpm_ug0_excl_low_trsh = 0;

#ifdef VALIDATE_PARMS
    if (!thr_ug)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((thr_ug->bn_hyst >= _14BITS_MAX_VAL_) ||
       (thr_ug->bn_thr >= _14BITS_MAX_VAL_) ||
       (thr_ug->excl_high_hyst >= _14BITS_MAX_VAL_) ||
       (thr_ug->excl_high_thr >= _14BITS_MAX_VAL_) ||
       (thr_ug->excl_low_hyst >= _14BITS_MAX_VAL_) ||
       (thr_ug->excl_low_thr >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
    if (drv_sbpm_get_abs_val(thr_ug->excl_high_thr, thr_ug->excl_low_thr) < thr_ug->excl_high_hyst + thr_ug->excl_low_hyst)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;   
    }
#endif

    reg_regs_sbpm_ug0_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_TRSH, UG_BAH, reg_regs_sbpm_ug0_trsh, thr_ug->bn_hyst);
    reg_regs_sbpm_ug0_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_TRSH, UG_BAT, reg_regs_sbpm_ug0_trsh, thr_ug->bn_thr);
    reg_regs_sbpm_ug0_excl_high_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH, EXCLH, reg_regs_sbpm_ug0_excl_high_trsh, thr_ug->excl_high_hyst);
    reg_regs_sbpm_ug0_excl_high_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH, EXCLT, reg_regs_sbpm_ug0_excl_high_trsh, thr_ug->excl_high_thr);
    reg_regs_sbpm_ug0_excl_low_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH, EXCLH, reg_regs_sbpm_ug0_excl_low_trsh, thr_ug->excl_low_hyst);
    reg_regs_sbpm_ug0_excl_low_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH, EXCLT, reg_regs_sbpm_ug0_excl_low_trsh, thr_ug->excl_low_thr);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG0_TRSH, reg_regs_sbpm_ug0_trsh);
    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH, reg_regs_sbpm_ug0_excl_high_trsh);
    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH, reg_regs_sbpm_ug0_excl_low_trsh);

    return BDMF_ERR_OK;
}

bdmf_error_t drv_sbpm_thr_ug0_get(sbpm_thr_ug *thr_ug)
{
    uint32_t reg_regs_sbpm_ug0_trsh;
    uint32_t reg_regs_sbpm_ug0_excl_high_trsh;
    uint32_t reg_regs_sbpm_ug0_excl_low_trsh;

#ifdef VALIDATE_PARMS
    if (!thr_ug)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_UG0_TRSH, reg_regs_sbpm_ug0_trsh);
    RU_REG_READ(0, SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH, reg_regs_sbpm_ug0_excl_high_trsh);
    RU_REG_READ(0, SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH, reg_regs_sbpm_ug0_excl_low_trsh);

    thr_ug->bn_hyst = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_TRSH, UG_BAH, reg_regs_sbpm_ug0_trsh);
    thr_ug->bn_thr = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_TRSH, UG_BAT, reg_regs_sbpm_ug0_trsh);
    thr_ug->excl_high_hyst = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH, EXCLH, reg_regs_sbpm_ug0_excl_high_trsh);
    thr_ug->excl_high_thr = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH, EXCLT, reg_regs_sbpm_ug0_excl_high_trsh);
    thr_ug->excl_low_hyst = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH, EXCLH, reg_regs_sbpm_ug0_excl_low_trsh);
    thr_ug->excl_low_thr = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH, EXCLT, reg_regs_sbpm_ug0_excl_low_trsh);

    return BDMF_ERR_OK;
}
bdmf_error_t drv_sbpm_thr_ug1_set(const sbpm_thr_ug *thr_ug)
{
    uint32_t reg_regs_sbpm_ug0_trsh = 0;
    uint32_t reg_regs_sbpm_ug0_excl_high_trsh = 0;
    uint32_t reg_regs_sbpm_ug0_excl_low_trsh = 0;

#ifdef VALIDATE_PARMS
    if (!thr_ug)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((thr_ug->bn_hyst >= _14BITS_MAX_VAL_) ||
       (thr_ug->bn_thr >= _14BITS_MAX_VAL_) ||
       (thr_ug->excl_high_hyst >= _14BITS_MAX_VAL_) ||
       (thr_ug->excl_high_thr >= _14BITS_MAX_VAL_) ||
       (thr_ug->excl_low_hyst >= _14BITS_MAX_VAL_) ||
       (thr_ug->excl_low_thr >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
    if (drv_sbpm_get_abs_val(thr_ug->excl_high_thr, thr_ug->excl_low_thr) < thr_ug->excl_high_hyst + thr_ug->excl_low_hyst)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;   
    }
#endif

    reg_regs_sbpm_ug0_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_TRSH, UG_BAH, reg_regs_sbpm_ug0_trsh, thr_ug->bn_hyst);
    reg_regs_sbpm_ug0_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_TRSH, UG_BAT, reg_regs_sbpm_ug0_trsh, thr_ug->bn_thr);
    reg_regs_sbpm_ug0_excl_high_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH, EXCLH, reg_regs_sbpm_ug0_excl_high_trsh, thr_ug->excl_high_hyst);
    reg_regs_sbpm_ug0_excl_high_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH, EXCLT, reg_regs_sbpm_ug0_excl_high_trsh, thr_ug->excl_high_thr);
    reg_regs_sbpm_ug0_excl_low_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH, EXCLH, reg_regs_sbpm_ug0_excl_low_trsh, thr_ug->excl_low_hyst);
    reg_regs_sbpm_ug0_excl_low_trsh = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH, EXCLT, reg_regs_sbpm_ug0_excl_low_trsh, thr_ug->excl_low_thr);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG1_TRSH, reg_regs_sbpm_ug0_trsh);
    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG1_EXCL_HIGH_TRSH, reg_regs_sbpm_ug0_excl_high_trsh);
    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG1_EXCL_LOW_TRSH, reg_regs_sbpm_ug0_excl_low_trsh);

    return BDMF_ERR_OK;
}

void drv_sbpm_default_val_init(void)
{
    uint32_t sp_high = 0, sp_low = 0, ug_high = 0, ug_low = 0;

    sp_low = RU_FIELD_SET(0, SBPM, REGS_SBPM_SP_RNR_LOW, SBPM_SP_RNR_LOW, sp_low, SBPM_SP_RNR_LOW_INIT_VAL);
    sp_high = RU_FIELD_SET(0, SBPM, REGS_SBPM_SP_RNR_HIGH, SBPM_SP_RNR_HIGH, sp_high, SBPM_SP_RNR_HIGH_INIT_VAL);
    ug_low = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG_MAP_LOW, SBPM_UG_MAP_LOW, ug_low, SBPM_UG_MAP_LOW_INIT_VAL);
    ug_high = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG_MAP_HIGH, SBPM_UG_MAP_HIGH, ug_high, SBPM_UG_MAP_HIGH_INIT_VAL);

    RU_REG_WRITE(0, SBPM, REGS_SBPM_SP_RNR_LOW, sp_low);
    RU_REG_WRITE(0, SBPM, REGS_SBPM_SP_RNR_HIGH, sp_high);
    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG_MAP_LOW, ug_low);
    RU_REG_WRITE(0, SBPM, REGS_SBPM_UG_MAP_HIGH, ug_high);
}

bdmf_error_t drv_sbpm_runner_sp_set(uint16_t sp_id, uint8_t ug_id)
{
    uint32_t sp_mask, sp_new_mask, ug_mask, ug_new_mask;

    if (sp_id >= 64)
        return BDMF_ERR_RANGE;
    if (ug_id > 1)
        return BDMF_ERR_RANGE;

    if (sp_id < 32)
    {
        /* making sure the same SP is not used in BBH */
        RU_REG_READ(0, SBPM, REGS_SBPM_SP_BBH_LOW, sp_mask);
        sp_mask = RU_FIELD_GET(0, SBPM, REGS_SBPM_SP_BBH_LOW, SBPM_SP_BBH_LOW, sp_mask);
        if (sp_mask & (0x1 << sp_id))
            return BDMF_ERR_PARM;

        RU_REG_READ(0, SBPM, REGS_SBPM_SP_RNR_LOW, sp_mask);
        sp_mask = RU_FIELD_GET(0, SBPM, REGS_SBPM_SP_RNR_LOW, SBPM_SP_RNR_LOW, sp_mask);
        sp_new_mask = sp_mask | (0x1 << sp_id);
        sp_mask = RU_FIELD_SET(0, SBPM, REGS_SBPM_SP_RNR_LOW, SBPM_SP_RNR_LOW, sp_mask, sp_new_mask);
        RU_REG_WRITE(0, SBPM, REGS_SBPM_SP_RNR_LOW, sp_mask);

        RU_REG_READ(0, SBPM, REGS_SBPM_UG_MAP_LOW, ug_mask);
        ug_mask = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG_MAP_LOW, SBPM_UG_MAP_LOW, ug_mask);
        if (ug_id == 1)
            ug_new_mask = ug_mask | (0x1 << sp_id);
        else
            ug_new_mask = ug_mask & ~(0x1 << sp_id);
        ug_mask = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG_MAP_LOW, SBPM_UG_MAP_LOW, ug_mask, ug_new_mask);
        RU_REG_WRITE(0, SBPM, REGS_SBPM_UG_MAP_LOW, ug_mask);
    }
    else
    {
        sp_id -= 32;

        /* making sure the same SP is not used in BBH */
        RU_REG_READ(0, SBPM, REGS_SBPM_SP_BBH_HIGH, sp_mask);
        sp_mask = RU_FIELD_GET(0, SBPM, REGS_SBPM_SP_BBH_HIGH, SBPM_SP_BBH_HIGH, sp_mask);
        if (sp_mask & (0x1 << sp_id))
            return BDMF_ERR_PARM;

        RU_REG_READ(0, SBPM, REGS_SBPM_SP_RNR_HIGH, sp_mask);
        sp_mask = RU_FIELD_GET(0, SBPM, REGS_SBPM_SP_RNR_HIGH, SBPM_SP_RNR_HIGH, sp_mask);
        sp_new_mask = sp_mask | (0x1 << sp_id);
        sp_mask = RU_FIELD_SET(0, SBPM, REGS_SBPM_SP_RNR_HIGH, SBPM_SP_RNR_HIGH, sp_mask, sp_new_mask);
        RU_REG_WRITE(0, SBPM, REGS_SBPM_SP_RNR_HIGH, sp_mask);

        RU_REG_READ(0, SBPM, REGS_SBPM_UG_MAP_HIGH, ug_mask);
        ug_mask = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG_MAP_HIGH, SBPM_UG_MAP_HIGH, ug_mask);
        if (ug_id == 1)
            ug_new_mask = ug_mask | (0x1 << sp_id);
        else
            ug_new_mask = ug_mask & ~(0x1 << sp_id);
        ug_mask = RU_FIELD_SET(0, SBPM, REGS_SBPM_UG_MAP_HIGH, SBPM_UG_MAP_HIGH, ug_mask, ug_new_mask);
        RU_REG_WRITE(0, SBPM, REGS_SBPM_UG_MAP_HIGH, ug_mask);
    }

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

int drv_sbpm_cli_debug_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    static uint32_t sbpm_debug[] = {cli_sbpm_bac, cli_sbpm_regs_sbpm_ug_status};

    /* get sbpm debug information */
    bdmf_session_print(session, "\nSBPM debug:\n");
    HAL_CLI_PRINT_LIST(session, sbpm, sbpm_debug);

    return drv_sbpm_cli_sanity_check(session, parm, n_parms);
}

int drv_sbpm_cli_sanity_check(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    sbpm_intr_ctrl_isr intr_ctrl_isr = {};
    int rc;

    rc = ag_drv_sbpm_intr_ctrl_isr_get(&intr_ctrl_isr);

    if (!rc)
    {
        if (intr_ctrl_isr.bac_underrun)
            bdmf_session_print(session, "\nSBPM: buffer underrun\n");
        if (intr_ctrl_isr.mcst_overflow)
            bdmf_session_print(session, "SBPM: multicast overflow\n");
        if (intr_ctrl_isr.check_last_err)
            bdmf_session_print(session, "SBPM: free with context error\n");
        if (intr_ctrl_isr.max_search_err)
            bdmf_session_print(session, "SBPM: free without context error\n");
        if (intr_ctrl_isr.invalid_in2e)
            bdmf_session_print(session, "SBPM: invalid ingress to egress command\n");
        if (intr_ctrl_isr.multi_get_next_null)
            bdmf_session_print(session, "SBPM: got null on multi get next\n");
        if (intr_ctrl_isr.cnct_null)
            bdmf_session_print(session, "SBPM: connect null buffer\n");
    }

    return rc;
}

static bdmf_error_t drv_sbpm_excl_mask_get(uint32_t *sbpm_excl_mask_high, uint32_t *sbpm_excl_mask_low)
{
    uint32_t _sbpm_excl_mask_high;
    uint32_t _sbpm_excl_mask_low;

#ifdef VALIDATE_PARMS
    if (!sbpm_excl_mask_high || !sbpm_excl_mask_low)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_EXCL_MASK_HIGH, _sbpm_excl_mask_high);
    RU_REG_READ(0, SBPM, REGS_SBPM_EXCL_MASK_LOW, _sbpm_excl_mask_low);

    *sbpm_excl_mask_high = RU_FIELD_GET(0, SBPM, REGS_SBPM_EXCL_MASK_HIGH, SBPM_EXCL_MASK_HIGH, _sbpm_excl_mask_high);
    *sbpm_excl_mask_low = RU_FIELD_GET(0, SBPM, REGS_SBPM_EXCL_MASK_LOW, SBPM_EXCL_MASK_LOW, _sbpm_excl_mask_low);

    return BDMF_ERR_OK;
}

bdmf_error_t drv_sbpm_thr_ug1_get(sbpm_thr_ug *thr_ug)
{
    uint32_t reg_regs_sbpm_ug0_trsh;
    uint32_t reg_regs_sbpm_ug0_excl_high_trsh;
    uint32_t reg_regs_sbpm_ug0_excl_low_trsh;

#ifdef VALIDATE_PARMS
    if (!thr_ug)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_UG1_TRSH, reg_regs_sbpm_ug0_trsh);
    RU_REG_READ(0, SBPM, REGS_SBPM_UG1_EXCL_HIGH_TRSH, reg_regs_sbpm_ug0_excl_high_trsh);
    RU_REG_READ(0, SBPM, REGS_SBPM_UG1_EXCL_LOW_TRSH, reg_regs_sbpm_ug0_excl_low_trsh);

    thr_ug->bn_hyst = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_TRSH, UG_BAH, reg_regs_sbpm_ug0_trsh);
    thr_ug->bn_thr = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_TRSH, UG_BAT, reg_regs_sbpm_ug0_trsh);
    thr_ug->excl_high_hyst = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH, EXCLH, reg_regs_sbpm_ug0_excl_high_trsh);
    thr_ug->excl_high_thr = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_EXCL_HIGH_TRSH, EXCLT, reg_regs_sbpm_ug0_excl_high_trsh);
    thr_ug->excl_low_hyst = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH, EXCLH, reg_regs_sbpm_ug0_excl_low_trsh);
    thr_ug->excl_low_thr = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG0_EXCL_LOW_TRSH, EXCLT, reg_regs_sbpm_ug0_excl_low_trsh);

    return BDMF_ERR_OK;
}

static bdmf_error_t drv_sbpm_sp_rnr_get(uint32_t *sbpm_sp_rnr_high, uint32_t *sbpm_sp_rnr_low)
{
    uint32_t _sbpm_sp_rnr_high;
    uint32_t _sbpm_sp_rnr_low;

#ifdef VALIDATE_PARMS
    if (!sbpm_sp_rnr_high || !sbpm_sp_rnr_low)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_SP_RNR_HIGH, _sbpm_sp_rnr_high);
    RU_REG_READ(0, SBPM, REGS_SBPM_SP_RNR_LOW, _sbpm_sp_rnr_low);

    *sbpm_sp_rnr_high = RU_FIELD_GET(0, SBPM, REGS_SBPM_SP_RNR_HIGH, SBPM_SP_RNR_HIGH, _sbpm_sp_rnr_high);
    *sbpm_sp_rnr_low = RU_FIELD_GET(0, SBPM, REGS_SBPM_SP_RNR_LOW, SBPM_SP_RNR_LOW, _sbpm_sp_rnr_low);

    return BDMF_ERR_OK;
}

static bdmf_error_t drv_sbpm_sp_bbh_get(uint32_t *sbpm_sp_bbh_high, uint32_t *sbpm_sp_bbh_low)
{
    uint32_t _sbpm_sp_bbh_high;
    uint32_t _sbpm_sp_bbh_low;

#ifdef VALIDATE_PARMS
    if (!sbpm_sp_bbh_high || !sbpm_sp_bbh_low)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_SP_BBH_HIGH, _sbpm_sp_bbh_high);
    RU_REG_READ(0, SBPM, REGS_SBPM_SP_BBH_LOW, _sbpm_sp_bbh_low);

    *sbpm_sp_bbh_high = RU_FIELD_GET(0, SBPM, REGS_SBPM_SP_BBH_HIGH, SBPM_SP_BBH_HIGH, _sbpm_sp_bbh_high);
    *sbpm_sp_bbh_low = RU_FIELD_GET(0, SBPM, REGS_SBPM_SP_BBH_LOW, SBPM_SP_BBH_LOW, _sbpm_sp_bbh_low);

    return BDMF_ERR_OK;
}

static bdmf_error_t drv_sbpm_ug_map_get(uint32_t *sbpm_ug_map_high, uint32_t *sbpm_ug_map_low)
{
    uint32_t reg_regs_sbpm_ug_map_high;
    uint32_t reg_regs_sbpm_ug_map_low;

#ifdef VALIDATE_PARMS
    if (!sbpm_ug_map_high || !sbpm_ug_map_low)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SBPM, REGS_SBPM_UG_MAP_HIGH, reg_regs_sbpm_ug_map_high);
    RU_REG_READ(0, SBPM, REGS_SBPM_UG_MAP_LOW, reg_regs_sbpm_ug_map_low);

    *sbpm_ug_map_high = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG_MAP_HIGH, SBPM_UG_MAP_HIGH, reg_regs_sbpm_ug_map_high);
    *sbpm_ug_map_low = RU_FIELD_GET(0, SBPM, REGS_SBPM_UG_MAP_LOW, SBPM_UG_MAP_LOW, reg_regs_sbpm_ug_map_low);

    return BDMF_ERR_OK;
}

static bdmf_error_t drv_sbpm_cli_thr_ug0_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    sbpm_thr_ug thr_ug0 = {};
    bdmf_error_t rc;

    rc = drv_sbpm_thr_ug0_get(&thr_ug0);
    if (!rc)
    {
        bdmf_session_print(session, "SBPM user group 0 configurations:\n\r");
        bdmf_session_print(session, "=================================\n\r");
        bdmf_session_print(session, "thr:                       %d\n\r", thr_ug0.bn_thr);
        bdmf_session_print(session, "hyst:                      %d\n\r", thr_ug0.bn_hyst);
        bdmf_session_print(session, "excl high thr:             %d\n\r", thr_ug0.excl_high_thr);
        bdmf_session_print(session, "excl high hyst:            %d\n\r", thr_ug0.excl_high_hyst);
        bdmf_session_print(session, "excl low thr:              %d\n\r", thr_ug0.excl_low_thr);
        bdmf_session_print(session, "excl low hyst:             %d\n\r", thr_ug0.excl_low_hyst);
    }
    else
    {
        bdmf_session_print(session, "ERROR %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_INTERNAL), BDMF_ERR_INTERNAL);
    }
    return rc;
}

static bdmf_error_t drv_sbpm_cli_thr_ug1_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    sbpm_thr_ug thr_ug1 = {};
    bdmf_error_t rc;

    rc = drv_sbpm_thr_ug1_get(&thr_ug1);
    if (!rc)
    {
        bdmf_session_print(session, "SBPM user group 1 configurations:\n\r");
        bdmf_session_print(session, "=================================\n\r");
        bdmf_session_print(session, "thr:                       %d\n\r", thr_ug1.bn_thr);
        bdmf_session_print(session, "hyst:                      %d\n\r", thr_ug1.bn_hyst);
        bdmf_session_print(session, "excl high thr:             %d\n\r", thr_ug1.excl_high_thr);
        bdmf_session_print(session, "excl high hyst:            %d\n\r", thr_ug1.excl_high_hyst);
        bdmf_session_print(session, "excl low thr:              %d\n\r", thr_ug1.excl_low_thr);
        bdmf_session_print(session, "excl low hyst:             %d\n\r", thr_ug1.excl_low_hyst);
    }
    else
    {
        bdmf_session_print(session, "ERROR %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_INTERNAL), BDMF_ERR_INTERNAL);
    }
    return rc;
}

static bdmf_error_t drv_sbpm_cli_sp_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t sbpm_sp_rnr_high;
    uint32_t sbpm_sp_rnr_low;
    uint32_t sbpm_sp_bbh_high;
    uint32_t sbpm_sp_bbh_low;
    bdmf_error_t rc;

    rc = drv_sbpm_sp_bbh_get(&sbpm_sp_bbh_high, &sbpm_sp_bbh_low);
    rc = rc ? rc : drv_sbpm_sp_rnr_get(&sbpm_sp_rnr_high, &sbpm_sp_rnr_low);
    if (!rc)
    {
        bdmf_session_print(session, "SBPM SP configurations:\n\r");
        bdmf_session_print(session, "=================================\n\r");
        bdmf_session_print(session, "sp runner:                 %x%x\n\r", sbpm_sp_rnr_high, sbpm_sp_rnr_low);
        bdmf_session_print(session, "sp bbh:                    %x%x\n\r", sbpm_sp_bbh_high, sbpm_sp_bbh_low);
    }
    else
    {
        bdmf_session_print(session, "ERROR %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_INTERNAL), BDMF_ERR_INTERNAL);
    }
    return rc;
}

int drv_sbpm_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t excl_mask_high, excl_mask_low;
    uint32_t nack_mask_high, nack_mask_low;    
    uint32_t ug_map_high, ug_map_low;    
    uint16_t gl_trash, gl_hyst;
    bdmf_error_t rc;
    
    rc = drv_sbpm_excl_mask_get(&excl_mask_high, &excl_mask_low);
    rc = rc ? rc : ag_drv_sbpm_nack_mask_get(&nack_mask_high, &nack_mask_low);
    rc = rc ? rc : drv_sbpm_ug_map_get(&ug_map_high, &ug_map_low);
    rc = rc ? rc : ag_drv_sbpm_regs_sbpm_gl_trsh_get(&gl_trash, &gl_hyst);    
    
    if (!rc)
    {
        bdmf_session_print(session, "SBPM configurations:\n\r");
        bdmf_session_print(session, "=================================\n\r\n\r");
    }
    
    rc = rc ? rc : drv_sbpm_cli_thr_ug1_get(session, parm, n_parms);
    if (!rc)
        bdmf_session_print(session, "\n\r");
    rc = rc ? rc : drv_sbpm_cli_thr_ug0_get(session, parm, n_parms);
    if (!rc)
        bdmf_session_print(session, "\n\r");
    rc = rc ? rc : drv_sbpm_cli_sp_get(session, parm, n_parms);

    if (!rc)
    {
        bdmf_session_print(session, "\n\rSBPM global configurations:\n\r");
        bdmf_session_print(session, "=================================\n\r\n\r");    
        bdmf_session_print(session, "exclusive mask:            %x%x\n\r", excl_mask_high, excl_mask_low);
        bdmf_session_print(session, "nack mask:                 %x%x\n\r", nack_mask_high, nack_mask_low);
        bdmf_session_print(session, "user group mapping:        %x%x\n\r", ug_map_high, ug_map_low);
        bdmf_session_print(session, "global treshold:           %x\n\r", gl_trash);
        bdmf_session_print(session, "global hysteresis:         %x\n\r", gl_hyst);
    }
    else
    {
        bdmf_session_print(session, "ERROR %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_INTERNAL),
            BDMF_ERR_INTERNAL);
    }

    return rc;
}

static bdmf_error_t drv_sbpm_cli_thr_ug0_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{   
    sbpm_thr_ug thr_ug = { .bn_hyst=parm[0].value.unumber, .bn_thr=parm[1].value.unumber, .excl_high_hyst=parm[2].value.unumber, .excl_high_thr=parm[3].value.unumber, .excl_low_hyst=parm[4].value.unumber, .excl_low_thr=parm[5].value.unumber};
    return drv_sbpm_thr_ug0_set(&thr_ug);
}

static bdmf_error_t drv_sbpm_cli_thr_ug1_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{   
    sbpm_thr_ug thr_ug = { .bn_hyst=parm[0].value.unumber, .bn_thr=parm[1].value.unumber, .excl_high_hyst=parm[2].value.unumber, .excl_high_thr=parm[3].value.unumber, .excl_low_hyst=parm[4].value.unumber, .excl_low_thr=parm[5].value.unumber};
    return drv_sbpm_thr_ug1_set(&thr_ug);
}

static uint32_t get_next_bn(uint32_t bn)
{
    uint32_t rc ;
    sbpm_regs_get_next_rply next_rply = {};
    uint32_t next_bn = (uint32_t)BDMF_INDEX_UNASSIGNED;

    rc = ag_drv_sbpm_regs_get_next_set(bn);
    while (!rc)
    {
        rc = ag_drv_sbpm_regs_get_next_rply_get(&next_rply);
        if (!next_rply.busy)
            break;
    }
    if (rc)
        return (uint32_t)BDMF_INDEX_UNASSIGNED;

    if (next_rply.bn_valid)
    {
        if (!next_rply.bn_null)
            next_bn = next_rply.next_bn;
    }
    else
        BDMF_TRACE_DBG(" ### BN_NULL (0x%x)\n", next_rply.next_bn);

    if (next_rply.mcnt_val != 0)
    {
      RDD_BTRACE("bn: %d, mcast value: %d\n", bn, next_rply.mcnt_val);
    }
    return next_bn;
}

#define MAX_NUM_OF_SBPMS 4096
#define INVALID_SBPM_BN 0x3fff

int drv_sbpm_cli_scan_bn_lists(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t bn, count, curr_bn, next_bn;
    uint32_t print_bns = parm[0].value.unumber;
    int *bn_head_count, *bn_next = NULL;
    int i, rc = BDMF_ERR_NOMEM;

    bn_head_count = (int *)bdmf_alloc(MAX_NUM_OF_SBPMS * sizeof(int));
    if (!bn_head_count)
        goto exit;
    bn_next = (int *)bdmf_alloc(MAX_NUM_OF_SBPMS * sizeof(int));
    if (!bn_next)
        goto exit;

    for (i = 0; i < MAX_NUM_OF_SBPMS; i++)
    {
        bn_head_count[i] = 1;
        bn_next[i] = BDMF_INDEX_UNASSIGNED;
    }
    for (bn = 0; bn < MAX_NUM_OF_SBPMS; bn++)
    {
        if (bn_next[bn] != BDMF_INDEX_UNASSIGNED) /* Already checked this BN */
            continue;

        for (count = 1, curr_bn = bn; ; count++)
        {
            if (count >= MAX_NUM_OF_SBPMS)
            {
               bdmf_trace("\n\nOverrun when testing BN %d (part of %d). BUG?\n", curr_bn, bn);
               for (next_bn = bn, i = 0; i<15;i++)
               {
                bdmf_trace("%d ", next_bn);
                next_bn = bn_next[next_bn];
                if (next_bn != SBPM_INVALID_BUFFER_NUMBER)
                    bdmf_trace("=> ");
               }
               bdmf_trace("\n\n");
               rc = BDMF_ERR_INTERNAL;
               goto exit;
            }

            next_bn = get_next_bn(curr_bn);
            if (next_bn == INVALID_SBPM_BN || next_bn == (uint32_t)BDMF_INDEX_UNASSIGNED)
                break;

            bn_next[curr_bn] = next_bn;
            if (bn_head_count[next_bn] > 1) /* Already counted it, reuse */
            {
                count += bn_head_count[next_bn];
                bn_head_count[next_bn] = 1; /* Don't count it twice */
                break;
            }
            curr_bn = next_bn;
        }
        bn_head_count[bn] = count;
    }
    bdmf_trace("Lists:\n");
    bdmf_trace("======\n");
    for (i = 0; i < MAX_NUM_OF_SBPMS; i++)
    {
        if (bn_head_count[i] == 1)
            continue;
        bdmf_trace("Head BN %d, num of BNs in list %d\n", i, bn_head_count[i]);
        if (print_bns)
        {
            bdmf_trace("\t%d => ", i);
            for (next_bn = (uint32_t)bn_next[i]; next_bn != (uint32_t)BDMF_INDEX_UNASSIGNED;)
            {
                bdmf_trace("%d => ", next_bn);
                next_bn = bn_next[next_bn];
            }
        }
    } 
    bdmf_trace("\n");
    rc = BDMF_ERR_OK;

exit:
    if (bn_next)
        bdmf_free(bn_next);
    if (bn_head_count)
        bdmf_free(bn_head_count);

    return rc;
}

static bdmfmon_handle_t sbpm_dir;

void drv_sbpm_cli_init(bdmfmon_handle_t driver_dir)
{
    sbpm_dir = ag_drv_sbpm_cli_init(driver_dir);

    BDMFMON_MAKE_CMD_NOPARM(sbpm_dir, "debug_get", "get debug information", (bdmfmon_cmd_cb_t)drv_sbpm_cli_debug_get);
    BDMFMON_MAKE_CMD_NOPARM(sbpm_dir, "sanity", "sanity check", (bdmfmon_cmd_cb_t)drv_sbpm_cli_sanity_check);
    BDMFMON_MAKE_CMD_NOPARM(sbpm_dir, "cfg_get", "sbpm configurations", (bdmfmon_cmd_cb_t)drv_sbpm_cli_config_get);
    BDMFMON_MAKE_CMD(sbpm_dir, "scan_bn_lists", "Scan BN lists", (bdmfmon_cmd_cb_t)drv_sbpm_cli_scan_bn_lists,
            BDMFMON_MAKE_PARM("print_bns", "Print buffers (0 - no, 1 - yes)", BDMFMON_PARM_NUMBER, 0));
    BDMFMON_MAKE_CMD(sbpm_dir, "ugo_thr_set", "user group 0 threshold", drv_sbpm_cli_thr_ug0_set,
            BDMFMON_MAKE_PARM("bn_hyst", "bn_hyst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bn_thr", "bn_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("excl_high_hyst", "excl_high_hyst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("excl_high_thr", "excl_high_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("excl_low_hyst", "excl_low_hyst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("excl_low_thr", "excl_low_thr", BDMFMON_PARM_NUMBER, 0));
    BDMFMON_MAKE_CMD(sbpm_dir, "ug1_thr_set", "user group 1 threshold", drv_sbpm_cli_thr_ug1_set,
            BDMFMON_MAKE_PARM("bn_hyst", "bn_hyst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bn_thr", "bn_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("excl_high_hyst", "excl_high_hyst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("excl_high_thr", "excl_high_thr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("excl_low_hyst", "excl_low_hyst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("excl_low_thr", "excl_low_thr", BDMFMON_PARM_NUMBER, 0));     
}

void drv_sbpm_cli_exit(bdmfmon_handle_t driver_dir)
{
    if (sbpm_dir)
    {
        bdmfmon_token_destroy(sbpm_dir);
        sbpm_dir = NULL;
    }
}

/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/
/*
    drv_sbpm_cli_config_get
    ag:
        bac (ug0, ug1, gl)
*/

#endif /* USE_BDMF_SHELL */

