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

#define SBPM_MAX_NUM_OF_ITERS 1000

bdmf_error_t drv_sbpm_free_list(uint16_t head_bn)
{
    int rc, num_of_iters;
    sbpm_regs_bn_free_without_contxt_rply reply;

    rc = ag_drv_sbpm_regs_bn_free_without_contxt_set(head_bn, SBPM_CPU_SA, 1);
    if (rc)
        return rc;
    for (num_of_iters = 0; num_of_iters < SBPM_MAX_NUM_OF_ITERS; num_of_iters++)
    {
        rc = ag_drv_sbpm_regs_bn_free_without_contxt_rply_get(&reply);
        if (rc)
            return rc;
        if (reply.rdy)
            break;
    }
    if (num_of_iters == SBPM_MAX_NUM_OF_ITERS)
        return BDMF_ERR_INTERNAL;

    return 0;
}

static bdmf_error_t drv_sbpm_copy_buf_to_bn(uint16_t bn, uint32_t headroom, uint8_t *data, uint32_t size)
{
#ifdef BCM_PON_XRDP
    psram_mem_memory_data _data = {};

    memcpy((uint8_t *)(&_data) + headroom, data, MIN(size, SBPM_BUF_SIZE - headroom));
    return ag_drv_psram_mem_memory_data_set(bn, &_data);
#else
    psram_memory_data _data = {};

    memcpy((uint8_t *)(&_data) + headroom, data, MIN(size, SBPM_BUF_SIZE - headroom));
    return ag_drv_psram_memory_data_set(bn, &_data);
#endif
}

static bdmf_error_t drv_sbpm_alloc_single(uint32_t size, uint32_t headroom, uint8_t *data, uint16_t *bn)
{
    int rc, num_of_iters;
    sbpm_regs_bn_alloc_rply reply;

    rc = ag_drv_sbpm_regs_bn_alloc_set(SBPM_CPU_SA);
    if (rc)
        goto error;

    for (num_of_iters = 0; num_of_iters < SBPM_MAX_NUM_OF_ITERS; num_of_iters++)
    {
        rc = ag_drv_sbpm_regs_bn_alloc_rply_get(&reply);
        if (rc)
            goto error;
        if (reply.rdy && reply.alloc_bn_valid)
            break;
    }
    if (num_of_iters == SBPM_MAX_NUM_OF_ITERS)
    {
        rc = BDMF_ERR_INTERNAL;
        goto error;
    }

    drv_sbpm_copy_buf_to_bn(reply.alloc_bn, headroom, data, size);
    *bn = reply.alloc_bn;
    return 0;

error:
    *bn = SBPM_INVALID_BUFFER_NUMBER;
    return rc;
}

static int drv_sbpm_connect_single(uint16_t bn, uint16_t next_bn)
{
    int rc, num_of_iters;
    bdmf_boolean connect_ack, busy, rdy;

    rc = ag_drv_sbpm_regs_bn_connect_set(bn, 1, 0, next_bn);
    if (rc)
        return rc;
    for (num_of_iters = 0; num_of_iters < SBPM_MAX_NUM_OF_ITERS; num_of_iters++)
    {
        rc = ag_drv_sbpm_regs_bn_connect_rply_get(&connect_ack, &busy, &rdy);
        if (rc)
            return rc;
        if (rdy && connect_ack)
            break;
    }
    if (num_of_iters == SBPM_MAX_NUM_OF_ITERS)
        return BDMF_ERR_INTERNAL;
    return 0;
}

bdmf_error_t drv_sbpm_alloc_and_connect_next_bn(uint32_t size, uint32_t headroom, uint8_t *data, uint16_t next_bn,
    bdmf_boolean inc_next_mc_counter, uint16_t *bn)
{
    uint16_t new_bn = SBPM_INVALID_BUFFER_NUMBER;
    int rc;

    /* Allocate new header and connect to next if available. */
    rc = drv_sbpm_alloc_single(size, headroom, data, &new_bn);
    if (rc)
        return BDMF_ERR_INTERNAL;

    if (next_bn != SBPM_INVALID_BUFFER_NUMBER)
    {
        rc = drv_sbpm_connect_single(new_bn, next_bn);
        if (rc)
            goto error;
        if (inc_next_mc_counter)
        {
            rc = ag_drv_sbpm_regs_mcst_inc_set(next_bn, 1, 1);
            if (rc)
                goto error;
        }
    }

    *bn = new_bn;
    return 0;

error:
    if (new_bn != SBPM_INVALID_BUFFER_NUMBER)
        drv_sbpm_free_list(new_bn);

    return BDMF_ERR_INTERNAL;
}

bdmf_error_t drv_sbpm_alloc_list(uint32_t size, uint32_t headroom, uint8_t *data,
    uint16_t *bn0, uint16_t *bn1, uint8_t *bns_num)
{
    uint16_t head_bn = SBPM_INVALID_BUFFER_NUMBER, new_bn, curr_bn;
    int rc = 0, _size, sbpm_buf_size_wo_headroom;
    uint8_t _bns_num = 1;

    _size = (int)size;
    sbpm_buf_size_wo_headroom = SBPM_BUF_SIZE - headroom;

    /* Allocate first; then in loop, copy and allocate next as long as needed. */
    rc = drv_sbpm_alloc_single(_size, headroom, data, &head_bn);
    if (rc)
        goto error;
    if (_size <= sbpm_buf_size_wo_headroom)
    {
        /* We are done, short packet */
        new_bn = head_bn; /* Last BN = head BN */
        goto exit;
    }

    for (_size -= sbpm_buf_size_wo_headroom, data += sbpm_buf_size_wo_headroom, curr_bn = head_bn;
        _size > 0;
        _size -= SBPM_BUF_SIZE, data += SBPM_BUF_SIZE, curr_bn = new_bn)
    {
        rc = drv_sbpm_alloc_single(_size, 0, data, &new_bn);
        if (rc)
            goto error;
        rc = drv_sbpm_connect_single(curr_bn, new_bn);
        if (rc)
        {
            drv_sbpm_free_list(new_bn);
            goto error;
        }
        _bns_num++;
    }

exit:

    if (bn0)
        *bn0 = head_bn;
    if (bn1)
        *bn1 = new_bn;
    if (bns_num)
        *bns_num = _bns_num;
    return 0;

error:
    if (head_bn != SBPM_INVALID_BUFFER_NUMBER)
        drv_sbpm_free_list(head_bn);

    if (bn0)
        *bn0 = SBPM_INVALID_BUFFER_NUMBER;
    if (bn1)
        *bn1 = SBPM_INVALID_BUFFER_NUMBER;
    if (bns_num)
        *bns_num = 0;
    return rc;
}

#ifdef USE_BDMF_SHELL

int drv_sbpm_cli_debug_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    static uint32_t sbpm_debug[] = {cli_sbpm_bac, cli_sbpm_regs_sbpm_ug_status, cli_sbpm_regs_sbpm_ug_bac_max};

    /* get sbpm debug information */
    bdmf_session_print(session, "\nSBPM debug:\n");
    HAL_CLI_PRINT_LIST(session, sbpm, sbpm_debug);

    return drv_sbpm_cli_sanity_check(session, parm, n_parms);
}

#if defined BCM6858 || defined BCM6856
static void drv_sbpm_parse_irr(bdmf_session_handle session, uint8_t cmd_sa, uint8_t cmd_ta, uint32_t cmd_data_22to0, uint32_t cmd_data_23to63)
{
    if (cmd_sa || cmd_ta || cmd_data_22to0 || cmd_data_23to63) {
        bdmf_session_print(session, "\nError:SBPM: Interrupt information register\n");

        switch(cmd_sa) {
        case BB_ID_RNR0 ... BB_ID_RNR15:
            bdmf_session_print(session, "Runner core %d ", cmd_sa);
            break;

#ifdef BCM6858
        case BB_ID_RX_BBH_0 ... BB_ID_TX_BBH_6:
            if (cmd_sa % 2)
                bdmf_session_print(session, "BBH RX LAN %d ", (cmd_sa - BB_ID_RX_BBH_0) / 2);
            else
                bdmf_session_print(session, "BBH TX LAN %d ", (cmd_sa - BB_ID_TX_BBH_0) / 2);
#endif
#ifdef BCM6856
        case BB_ID_RX_BBH_0 ... BB_ID_RX_BBH_5:
           if (cmd_sa == BB_ID_TX_LAN)
               bdmf_session_print(session, "BBH TX LAN");
           else
               bdmf_session_print(session, "BBH RX LAN %d ", (cmd_sa - BB_ID_RX_BBH_0) / 2);
#endif
            break;

#ifdef BCM6858
        case BB_ID_RX_BBH_7:
            bdmf_session_print(session, "BBH RX LAN7 ");
            break;

        case BB_ID_TX_BBH_7:
            bdmf_session_print(session, "BBH TX LAN7 ");
            break;
#endif
#ifdef BCM6856
        case BB_ID_RX_PON:
#endif
#ifdef BCM6858
        case BB_ID_RX_PON_ETH:
#endif
            bdmf_session_print(session, "BBH RX WAN ");
            break;

        case BB_ID_TX_PON_ETH_PD ... BB_ID_TX_PON_ETH_STAT:
            bdmf_session_print(session, "BBH TX WAN ");
            break;

        case BB_ID_QM_CP_SDMA ... BB_ID_QM_CP_MACHINE :
            bdmf_session_print(session, "QM ");
            break;

        default:
            bdmf_session_print(session, "SA not supported ");
            break;
        }

        bdmf_session_print(session, "to SBPM ");

        switch(cmd_ta) {
        case 0:
            bdmf_session_print(session, "multi get next BN = 0x%x\n", cmd_data_22to0);
            break;

        case 1:
            bdmf_session_print(session, "alloc\n");
            break;

        case 2:
            bdmf_session_print(session, "multicast increment BN = 0x%x value= %d\n",
                               cmd_data_22to0 & 0x3FFF, (cmd_data_22to0 & 0x3FC000) >> 14);
            break;

        case 3:
            bdmf_session_print(session, "free with context BN = 0x%x last BN = 0x%x\n",
                               cmd_data_22to0 & 0x3FFF, (cmd_data_23to63 & 0x7FFE00) >> 9);
            break;

        case 4:
            bdmf_session_print(session, "connect BN = 0x%x next BN = 0x%x\n",
                               cmd_data_22to0 & 0x3FFF,
                               (cmd_data_22to0 & 0x7F0000) >> 16 | (cmd_data_23to63 & 0x7F) << 7);
            break;

        case 5:
            bdmf_session_print(session, "get next BN = 0x%x\n", cmd_data_22to0);
            break;

        case 6:
            bdmf_session_print(session, "free without context BN = 0x%x  bb_src_id = %d\n",
                               cmd_data_22to0 & 0x3FFF, (cmd_data_22to0 & 0xFC000) >> 14);
            break;

        case 7:
            bdmf_session_print(session, "ingress to egress number of buffers = %d\n",
                               cmd_data_22to0 & 0x7F);
            break;
        }
    }
}
#endif

int drv_sbpm_cli_sanity_check(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    sbpm_intr_ctrl_isr intr_ctrl_isr = {};
#if defined BCM6858 || defined BCM6856
    uint8_t cmd_sa;
    uint8_t cmd_ta;
    uint32_t cmd_data_22to0;
    uint32_t cmd_data_23to63;
#endif
    int rc;

    rc = ag_drv_sbpm_intr_ctrl_isr_get(&intr_ctrl_isr);

    if (!rc)
    {
        if (intr_ctrl_isr.bac_underrun)
            bdmf_session_print(session, "\nError:SBPM: buffer underrun\n");
        if (intr_ctrl_isr.mcst_overflow)
            bdmf_session_print(session, "\nError:SBPM: multicast overflow\n");
        if (intr_ctrl_isr.check_last_err)
            bdmf_session_print(session, "\nError:SBPM: free with context error\n");
        if (intr_ctrl_isr.max_search_err)
            bdmf_session_print(session, "\nError:SBPM: free without context error\n");
        if (intr_ctrl_isr.invalid_in2e)
            bdmf_session_print(session, "\nError:SBPM: invalid ingress to egress command\n");
        if (intr_ctrl_isr.multi_get_next_null)
            bdmf_session_print(session, "\nError:SBPM: got null on multi get next\n");
        if (intr_ctrl_isr.cnct_null)
            bdmf_session_print(session, "\nError:SBPM: connect null buffer\n");
    }

#if defined BCM6858 || defined BCM6856
    rc = rc ? rc : ag_drv_sbpm_regs_sbpm_iir_low_get(&cmd_sa, &cmd_ta, &cmd_data_22to0);
    rc = rc ? rc : ag_drv_sbpm_regs_sbpm_iir_high_get(&cmd_data_23to63);

    drv_sbpm_parse_irr(session, cmd_sa, cmd_ta, cmd_data_22to0, cmd_data_23to63);
#endif

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

uint16_t drv_sbpm_get_next_bn(int16_t bn)
{
    int rc;
    sbpm_regs_get_next_rply next_rply = {};
    uint32_t next_bn = SBPM_INVALID_BUFFER_NUMBER;

    rc = ag_drv_sbpm_regs_get_next_set(bn);
    while (!rc)
    {
        rc = ag_drv_sbpm_regs_get_next_rply_get(&next_rply);
        if (!next_rply.busy)
            break;
    }
    if (rc)
        return SBPM_INVALID_BUFFER_NUMBER;

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

int drv_sbpm_cli_scan_bn_lists(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint16_t bn, count, curr_bn, next_bn;
    uint32_t print_bns = parm[0].value.unumber;
    uint16_t *bn_head_count, *bn_next = NULL;
    int i, rc = BDMF_ERR_NOMEM;

    bn_head_count = (uint16_t *)bdmf_alloc(SBPM_MAX_NUM_OF_BNS * sizeof(uint16_t));
    if (!bn_head_count)
        goto exit;
    bn_next = (uint16_t *)bdmf_alloc(SBPM_MAX_NUM_OF_BNS * sizeof(uint16_t));
    if (!bn_next)
        goto exit;

    for (i = 0; i < SBPM_MAX_NUM_OF_BNS; i++)
    {
        bn_head_count[i] = 1;
        bn_next[i] = SBPM_INVALID_BUFFER_NUMBER;
    }
    for (bn = 0; bn < SBPM_MAX_NUM_OF_BNS; bn++)
    {
        if (bn_next[bn] != SBPM_INVALID_BUFFER_NUMBER) /* Already checked this BN */
            continue;

        for (count = 1, curr_bn = bn; ; count++)
        {
            if (count >= SBPM_MAX_NUM_OF_BNS)
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

            next_bn = drv_sbpm_get_next_bn(curr_bn);
            if (next_bn == SBPM_INVALID_BUFFER_NUMBER)
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
    bdmf_trace("======");
    for (i = 0; i < SBPM_MAX_NUM_OF_BNS; i++)
    {
        if (bn_head_count[i] == 1)
            continue;
        bdmf_trace("\nHead BN %d, num of BNs in list %d\n", i, bn_head_count[i]);
        if (print_bns)
        {
            bdmf_trace("\t%d => ", i);
            for (next_bn = bn_next[i]; next_bn != SBPM_INVALID_BUFFER_NUMBER;)
            {
                bdmf_trace("%d ", next_bn);
                next_bn = bn_next[next_bn];
                if (next_bn != SBPM_INVALID_BUFFER_NUMBER)
                    bdmf_trace("=> ");
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

static bdmf_error_t drv_sbpm_dump_list(uint16_t bn)
{
    uint16_t next_bn;
    int i;

    bdmf_trace("\n List: [ %d ", bn);
    for (i = 0; i < SBPM_MAX_NUM_OF_BNS; i++)
    {
        next_bn = drv_sbpm_get_next_bn(bn);
        if (next_bn == SBPM_INVALID_BUFFER_NUMBER)
        {
            bdmf_trace(" ]\n");
            break;
        }
        bdmf_trace("%d ", next_bn);
        bn = next_bn;
    }
    if (i == SBPM_MAX_NUM_OF_BNS)
    {
        bdmf_trace("===== BAD LIST ALLOCATED, STOP SCANNING....\n");
        return BDMF_ERR_INTERNAL;
    }
    return 0;
}

static void prepare_data(uint8_t data[], int size)
{
    int i;

    for (i = 0; i < size;)
    {
        data[i++] = 0xa;
        data[i++] = 0xb;
        data[i++] = 0xc;
        data[i++] = 0xd;
        data[i++] = 0xe;
        data[i++] = 0xf;
        data[i++] = 0x1;
        data[i++] = 0x2;
    }
}

static bdmf_error_t drv_sbpm_cli_list_alloc(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{   
    uint32_t size = parm[0].value.unumber;
    uint32_t headroom = parm[1].value.unumber;
    uint8_t *data, bn_num;
    uint16_t bn0, bn1;
    int rc;

    data = bdmf_alloc(size);
    if (!data)
        return BDMF_ERR_NOMEM;
    prepare_data(data, size);
    rc = drv_sbpm_alloc_list(size, headroom, data, &bn0, &bn1, &bn_num);
    bdmf_free(data);
    if (rc)
        return rc;
    bdmf_trace("\n Allocated, BN0 = %d, BN1 = %d, BN_NUM = %d\n", bn0, bn1, bn_num);
    return drv_sbpm_dump_list(bn0);
}

static bdmf_error_t drv_sbpm_cli_list_free(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{   
    uint16_t head_bn = parm[0].value.unumber;

    return drv_sbpm_free_list(head_bn);
}

static bdmf_error_t drv_sbpm_cli_alloc_and_connect(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{   
    uint32_t size = parm[0].value.unumber;
    uint32_t headroom = parm[1].value.unumber;
    uint16_t next_bn = parm[2].value.unumber;
    bdmf_boolean inc_mc_next = parm[3].value.unumber;
    uint16_t bn;
    uint8_t *data;
    int rc;

    data = bdmf_alloc(size);
    if (!data)
        return BDMF_ERR_NOMEM;
    prepare_data(data, size);
    rc = drv_sbpm_alloc_and_connect_next_bn(size, headroom, data, next_bn, inc_mc_next, &bn);
    if (rc)
        return rc;
    bdmf_trace("\n Allocated BN = %d\n", bn);
    return drv_sbpm_dump_list(bn);
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
    BDMFMON_MAKE_CMD(sbpm_dir, "list_alloc", "Allocate SBPM list, filled with pattern (replicated)",
        drv_sbpm_cli_list_alloc,
            BDMFMON_MAKE_PARM("size", "size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("headroom", "headroom", BDMFMON_PARM_NUMBER, 0));
    BDMFMON_MAKE_CMD(sbpm_dir, "list_free", "FREE SBPM list, starting from BN num", drv_sbpm_cli_list_free,
            BDMFMON_MAKE_PARM("bn", "bn", BDMFMON_PARM_NUMBER, 0));
    BDMFMON_MAKE_CMD(sbpm_dir, "alloc_and_connect_bn",
        "Allocate new BN, filled with pattern (replicated), and connect to next bn", drv_sbpm_cli_alloc_and_connect,
            BDMFMON_MAKE_PARM("size", "size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("headroom", "headroom", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("next_bn", "next_bn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("inc_mc_next", "increment MC counter of next_bn", BDMFMON_PARM_NUMBER, 0));
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

