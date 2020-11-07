/*
   Copyright (c) 2015 Broadcom Corporation
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

#include "drivers_epon_ag.h"
#include "drv_epon_epon_top_ag.h"
bdmf_error_t ag_drv_epon_top_scratch_set(uint32_t scratch)
{
    uint32_t reg_scratch=0;

#ifdef VALIDATE_PARMS
#endif

    reg_scratch = RU_FIELD_SET(0, EPON_TOP, SCRATCH, SCRATCH, reg_scratch, scratch);

    RU_REG_WRITE(0, EPON_TOP, SCRATCH, reg_scratch);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epon_top_scratch_get(uint32_t *scratch)
{
    uint32_t reg_scratch=0;

#ifdef VALIDATE_PARMS
    if(!scratch)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPON_TOP, SCRATCH, reg_scratch);

    *scratch = RU_FIELD_GET(0, EPON_TOP, SCRATCH, SCRATCH, reg_scratch);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epon_top_reset_set(const epon_top_reset *reset)
{
    uint32_t reg_reset=0;

#ifdef VALIDATE_PARMS
    if(!reset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((reset->xpcsrxrst_n >= _1BITS_MAX_VAL_) ||
       (reset->xpcstxrst_n >= _1BITS_MAX_VAL_) ||
       (reset->xifrst_n >= _1BITS_MAX_VAL_) ||
       (reset->clkprgrst_n >= _1BITS_MAX_VAL_) ||
       (reset->ncorst_n >= _1BITS_MAX_VAL_) ||
       (reset->lifrst_n >= _1BITS_MAX_VAL_) ||
       (reset->epnrst_n >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_reset = RU_FIELD_SET(0, EPON_TOP, RESET, XPCSRXRST_N, reg_reset, reset->xpcsrxrst_n);
    reg_reset = RU_FIELD_SET(0, EPON_TOP, RESET, XPCSTXRST_N, reg_reset, reset->xpcstxrst_n);
    reg_reset = RU_FIELD_SET(0, EPON_TOP, RESET, XIFRST_N, reg_reset, reset->xifrst_n);
    reg_reset = RU_FIELD_SET(0, EPON_TOP, RESET, CLKPRGRST_N, reg_reset, reset->clkprgrst_n);
    reg_reset = RU_FIELD_SET(0, EPON_TOP, RESET, NCORST_N, reg_reset, reset->ncorst_n);
    reg_reset = RU_FIELD_SET(0, EPON_TOP, RESET, LIFRST_N, reg_reset, reset->lifrst_n);
    reg_reset = RU_FIELD_SET(0, EPON_TOP, RESET, EPNRST_N, reg_reset, reset->epnrst_n);

    RU_REG_WRITE(0, EPON_TOP, RESET, reg_reset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epon_top_reset_get(epon_top_reset *reset)
{
    uint32_t reg_reset=0;

#ifdef VALIDATE_PARMS
    if(!reset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPON_TOP, RESET, reg_reset);

    reset->xpcsrxrst_n = RU_FIELD_GET(0, EPON_TOP, RESET, XPCSRXRST_N, reg_reset);
    reset->xpcstxrst_n = RU_FIELD_GET(0, EPON_TOP, RESET, XPCSTXRST_N, reg_reset);
    reset->xifrst_n = RU_FIELD_GET(0, EPON_TOP, RESET, XIFRST_N, reg_reset);
    reset->clkprgrst_n = RU_FIELD_GET(0, EPON_TOP, RESET, CLKPRGRST_N, reg_reset);
    reset->ncorst_n = RU_FIELD_GET(0, EPON_TOP, RESET, NCORST_N, reg_reset);
    reset->lifrst_n = RU_FIELD_GET(0, EPON_TOP, RESET, LIFRST_N, reg_reset);
    reset->epnrst_n = RU_FIELD_GET(0, EPON_TOP, RESET, EPNRST_N, reg_reset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epon_top_interrupt_set(const epon_top_interrupt *interrupt)
{
    uint32_t reg_interrupt=0;

#ifdef VALIDATE_PARMS
    if(!interrupt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((interrupt->int_1pps >= _1BITS_MAX_VAL_) ||
       (interrupt->int_xpcs_tx >= _1BITS_MAX_VAL_) ||
       (interrupt->int_xpcs_rx >= _1BITS_MAX_VAL_) ||
       (interrupt->int_xif >= _1BITS_MAX_VAL_) ||
       (interrupt->int_nco >= _1BITS_MAX_VAL_) ||
       (interrupt->int_lif >= _1BITS_MAX_VAL_) ||
       (interrupt->int_epn >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_interrupt = RU_FIELD_SET(0, EPON_TOP, INTERRUPT, INT_1PPS, reg_interrupt, interrupt->int_1pps);
    reg_interrupt = RU_FIELD_SET(0, EPON_TOP, INTERRUPT, INT_XPCS_TX, reg_interrupt, interrupt->int_xpcs_tx);
    reg_interrupt = RU_FIELD_SET(0, EPON_TOP, INTERRUPT, INT_XPCS_RX, reg_interrupt, interrupt->int_xpcs_rx);
    reg_interrupt = RU_FIELD_SET(0, EPON_TOP, INTERRUPT, INT_XIF, reg_interrupt, interrupt->int_xif);
    reg_interrupt = RU_FIELD_SET(0, EPON_TOP, INTERRUPT, INT_NCO, reg_interrupt, interrupt->int_nco);
    reg_interrupt = RU_FIELD_SET(0, EPON_TOP, INTERRUPT, INT_LIF, reg_interrupt, interrupt->int_lif);
    reg_interrupt = RU_FIELD_SET(0, EPON_TOP, INTERRUPT, INT_EPN, reg_interrupt, interrupt->int_epn);

    RU_REG_WRITE(0, EPON_TOP, INTERRUPT, reg_interrupt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epon_top_interrupt_get(epon_top_interrupt *interrupt)
{
    uint32_t reg_interrupt=0;

#ifdef VALIDATE_PARMS
    if(!interrupt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPON_TOP, INTERRUPT, reg_interrupt);

    interrupt->int_1pps = RU_FIELD_GET(0, EPON_TOP, INTERRUPT, INT_1PPS, reg_interrupt);
    interrupt->int_xpcs_tx = RU_FIELD_GET(0, EPON_TOP, INTERRUPT, INT_XPCS_TX, reg_interrupt);
    interrupt->int_xpcs_rx = RU_FIELD_GET(0, EPON_TOP, INTERRUPT, INT_XPCS_RX, reg_interrupt);
    interrupt->int_xif = RU_FIELD_GET(0, EPON_TOP, INTERRUPT, INT_XIF, reg_interrupt);
    interrupt->int_nco = RU_FIELD_GET(0, EPON_TOP, INTERRUPT, INT_NCO, reg_interrupt);
    interrupt->int_lif = RU_FIELD_GET(0, EPON_TOP, INTERRUPT, INT_LIF, reg_interrupt);
    interrupt->int_epn = RU_FIELD_GET(0, EPON_TOP, INTERRUPT, INT_EPN, reg_interrupt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epon_top_interrupt_mask_set(const epon_top_interrupt_mask *interrupt_mask)
{
    uint32_t reg_interrupt_mask=0;

#ifdef VALIDATE_PARMS
    if(!interrupt_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((interrupt_mask->int_1pps_mask >= _1BITS_MAX_VAL_) ||
       (interrupt_mask->int_xpcs_tx_mask >= _1BITS_MAX_VAL_) ||
       (interrupt_mask->int_xpcs_rx_mask >= _1BITS_MAX_VAL_) ||
       (interrupt_mask->int_xif_mask >= _1BITS_MAX_VAL_) ||
       (interrupt_mask->int_nco_mask >= _1BITS_MAX_VAL_) ||
       (interrupt_mask->int_lif_mask >= _1BITS_MAX_VAL_) ||
       (interrupt_mask->int_epn_mask >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_interrupt_mask = RU_FIELD_SET(0, EPON_TOP, INTERRUPT_MASK, INT_1PPS_MASK, reg_interrupt_mask, interrupt_mask->int_1pps_mask);
    reg_interrupt_mask = RU_FIELD_SET(0, EPON_TOP, INTERRUPT_MASK, INT_XPCS_TX_MASK, reg_interrupt_mask, interrupt_mask->int_xpcs_tx_mask);
    reg_interrupt_mask = RU_FIELD_SET(0, EPON_TOP, INTERRUPT_MASK, INT_XPCS_RX_MASK, reg_interrupt_mask, interrupt_mask->int_xpcs_rx_mask);
    reg_interrupt_mask = RU_FIELD_SET(0, EPON_TOP, INTERRUPT_MASK, INT_XIF_MASK, reg_interrupt_mask, interrupt_mask->int_xif_mask);
    reg_interrupt_mask = RU_FIELD_SET(0, EPON_TOP, INTERRUPT_MASK, INT_NCO_MASK, reg_interrupt_mask, interrupt_mask->int_nco_mask);
    reg_interrupt_mask = RU_FIELD_SET(0, EPON_TOP, INTERRUPT_MASK, INT_LIF_MASK, reg_interrupt_mask, interrupt_mask->int_lif_mask);
    reg_interrupt_mask = RU_FIELD_SET(0, EPON_TOP, INTERRUPT_MASK, INT_EPN_MASK, reg_interrupt_mask, interrupt_mask->int_epn_mask);

    RU_REG_WRITE(0, EPON_TOP, INTERRUPT_MASK, reg_interrupt_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epon_top_interrupt_mask_get(epon_top_interrupt_mask *interrupt_mask)
{
    uint32_t reg_interrupt_mask=0;

#ifdef VALIDATE_PARMS
    if(!interrupt_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPON_TOP, INTERRUPT_MASK, reg_interrupt_mask);

    interrupt_mask->int_1pps_mask = RU_FIELD_GET(0, EPON_TOP, INTERRUPT_MASK, INT_1PPS_MASK, reg_interrupt_mask);
    interrupt_mask->int_xpcs_tx_mask = RU_FIELD_GET(0, EPON_TOP, INTERRUPT_MASK, INT_XPCS_TX_MASK, reg_interrupt_mask);
    interrupt_mask->int_xpcs_rx_mask = RU_FIELD_GET(0, EPON_TOP, INTERRUPT_MASK, INT_XPCS_RX_MASK, reg_interrupt_mask);
    interrupt_mask->int_xif_mask = RU_FIELD_GET(0, EPON_TOP, INTERRUPT_MASK, INT_XIF_MASK, reg_interrupt_mask);
    interrupt_mask->int_nco_mask = RU_FIELD_GET(0, EPON_TOP, INTERRUPT_MASK, INT_NCO_MASK, reg_interrupt_mask);
    interrupt_mask->int_lif_mask = RU_FIELD_GET(0, EPON_TOP, INTERRUPT_MASK, INT_LIF_MASK, reg_interrupt_mask);
    interrupt_mask->int_epn_mask = RU_FIELD_GET(0, EPON_TOP, INTERRUPT_MASK, INT_EPN_MASK, reg_interrupt_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epon_top_control_set(bdmf_boolean cfgtwogigpondns, bdmf_boolean cfgtengigponup, bdmf_boolean cfgtengigdns)
{
    uint32_t reg_control=0;

#ifdef VALIDATE_PARMS
    if((cfgtwogigpondns >= _1BITS_MAX_VAL_) ||
       (cfgtengigponup >= _1BITS_MAX_VAL_) ||
       (cfgtengigdns >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_control = RU_FIELD_SET(0, EPON_TOP, CONTROL, CFGTWOGIGPONDNS, reg_control, cfgtwogigpondns);
    reg_control = RU_FIELD_SET(0, EPON_TOP, CONTROL, CFGTENGIGPONUP, reg_control, cfgtengigponup);
    reg_control = RU_FIELD_SET(0, EPON_TOP, CONTROL, CFGTENGIGDNS, reg_control, cfgtengigdns);

    RU_REG_WRITE(0, EPON_TOP, CONTROL, reg_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epon_top_control_get(bdmf_boolean *cfgtwogigpondns, bdmf_boolean *cfgtengigponup, bdmf_boolean *cfgtengigdns)
{
    uint32_t reg_control=0;

#ifdef VALIDATE_PARMS
    if(!cfgtwogigpondns || !cfgtengigponup || !cfgtengigdns)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPON_TOP, CONTROL, reg_control);

    *cfgtwogigpondns = RU_FIELD_GET(0, EPON_TOP, CONTROL, CFGTWOGIGPONDNS, reg_control);
    *cfgtengigponup = RU_FIELD_GET(0, EPON_TOP, CONTROL, CFGTENGIGPONUP, reg_control);
    *cfgtengigdns = RU_FIELD_GET(0, EPON_TOP, CONTROL, CFGTENGIGDNS, reg_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epon_top_one_pps_mpcp_offset_set(uint32_t cfg_1pps_mpcp_offset)
{
    uint32_t reg_one_pps_mpcp_offset=0;

#ifdef VALIDATE_PARMS
#endif

    reg_one_pps_mpcp_offset = RU_FIELD_SET(0, EPON_TOP, ONE_PPS_MPCP_OFFSET, CFG_1PPS_MPCP_OFFSET, reg_one_pps_mpcp_offset, cfg_1pps_mpcp_offset);

    RU_REG_WRITE(0, EPON_TOP, ONE_PPS_MPCP_OFFSET, reg_one_pps_mpcp_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epon_top_one_pps_mpcp_offset_get(uint32_t *cfg_1pps_mpcp_offset)
{
    uint32_t reg_one_pps_mpcp_offset=0;

#ifdef VALIDATE_PARMS
    if(!cfg_1pps_mpcp_offset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPON_TOP, ONE_PPS_MPCP_OFFSET, reg_one_pps_mpcp_offset);

    *cfg_1pps_mpcp_offset = RU_FIELD_GET(0, EPON_TOP, ONE_PPS_MPCP_OFFSET, CFG_1PPS_MPCP_OFFSET, reg_one_pps_mpcp_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epon_top_one_pps_captured_mpcp_time_get(uint32_t *capture_1pps_mpcp_time)
{
    uint32_t reg_one_pps_captured_mpcp_time=0;

#ifdef VALIDATE_PARMS
    if(!capture_1pps_mpcp_time)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPON_TOP, ONE_PPS_CAPTURED_MPCP_TIME, reg_one_pps_captured_mpcp_time);

    *capture_1pps_mpcp_time = RU_FIELD_GET(0, EPON_TOP, ONE_PPS_CAPTURED_MPCP_TIME, CAPTURE_1PPS_MPCP_TIME, reg_one_pps_captured_mpcp_time);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_scratch,
    BDMF_reset,
    BDMF_interrupt,
    BDMF_interrupt_mask,
    BDMF_control,
    BDMF_one_pps_mpcp_offset,
    BDMF_one_pps_captured_mpcp_time,
};

typedef enum
{
    bdmf_address_scratch,
    bdmf_address_reset,
    bdmf_address_interrupt,
    bdmf_address_interrupt_mask,
    bdmf_address_control,
    bdmf_address_one_pps_mpcp_offset,
    bdmf_address_one_pps_captured_mpcp_time,
}
bdmf_address;

static int bcm_epon_top_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_scratch:
        err = ag_drv_epon_top_scratch_set(parm[1].value.unumber);
        break;
    case BDMF_reset:
    {
        epon_top_reset reset = { .xpcsrxrst_n=parm[1].value.unumber, .xpcstxrst_n=parm[2].value.unumber, .xifrst_n=parm[3].value.unumber, .clkprgrst_n=parm[4].value.unumber, .ncorst_n=parm[5].value.unumber, .lifrst_n=parm[6].value.unumber, .epnrst_n=parm[7].value.unumber};
        err = ag_drv_epon_top_reset_set(&reset);
        break;
    }
    case BDMF_interrupt:
    {
        epon_top_interrupt interrupt = { .int_1pps=parm[1].value.unumber, .int_xpcs_tx=parm[2].value.unumber, .int_xpcs_rx=parm[3].value.unumber, .int_xif=parm[4].value.unumber, .int_nco=parm[5].value.unumber, .int_lif=parm[6].value.unumber, .int_epn=parm[7].value.unumber};
        err = ag_drv_epon_top_interrupt_set(&interrupt);
        break;
    }
    case BDMF_interrupt_mask:
    {
        epon_top_interrupt_mask interrupt_mask = { .int_1pps_mask=parm[1].value.unumber, .int_xpcs_tx_mask=parm[2].value.unumber, .int_xpcs_rx_mask=parm[3].value.unumber, .int_xif_mask=parm[4].value.unumber, .int_nco_mask=parm[5].value.unumber, .int_lif_mask=parm[6].value.unumber, .int_epn_mask=parm[7].value.unumber};
        err = ag_drv_epon_top_interrupt_mask_set(&interrupt_mask);
        break;
    }
    case BDMF_control:
        err = ag_drv_epon_top_control_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case BDMF_one_pps_mpcp_offset:
        err = ag_drv_epon_top_one_pps_mpcp_offset_set(parm[1].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_epon_top_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_scratch:
    {
        uint32_t scratch;
        err = ag_drv_epon_top_scratch_get(&scratch);
        bdmf_session_print(session, "scratch = %u = 0x%x\n", scratch, scratch);
        break;
    }
    case BDMF_reset:
    {
        epon_top_reset reset;
        err = ag_drv_epon_top_reset_get(&reset);
        bdmf_session_print(session, "xpcsrxrst_n = %u = 0x%x\n", reset.xpcsrxrst_n, reset.xpcsrxrst_n);
        bdmf_session_print(session, "xpcstxrst_n = %u = 0x%x\n", reset.xpcstxrst_n, reset.xpcstxrst_n);
        bdmf_session_print(session, "xifrst_n = %u = 0x%x\n", reset.xifrst_n, reset.xifrst_n);
        bdmf_session_print(session, "clkprgrst_n = %u = 0x%x\n", reset.clkprgrst_n, reset.clkprgrst_n);
        bdmf_session_print(session, "ncorst_n = %u = 0x%x\n", reset.ncorst_n, reset.ncorst_n);
        bdmf_session_print(session, "lifrst_n = %u = 0x%x\n", reset.lifrst_n, reset.lifrst_n);
        bdmf_session_print(session, "epnrst_n = %u = 0x%x\n", reset.epnrst_n, reset.epnrst_n);
        break;
    }
    case BDMF_interrupt:
    {
        epon_top_interrupt interrupt;
        err = ag_drv_epon_top_interrupt_get(&interrupt);
        bdmf_session_print(session, "int_1pps = %u = 0x%x\n", interrupt.int_1pps, interrupt.int_1pps);
        bdmf_session_print(session, "int_xpcs_tx = %u = 0x%x\n", interrupt.int_xpcs_tx, interrupt.int_xpcs_tx);
        bdmf_session_print(session, "int_xpcs_rx = %u = 0x%x\n", interrupt.int_xpcs_rx, interrupt.int_xpcs_rx);
        bdmf_session_print(session, "int_xif = %u = 0x%x\n", interrupt.int_xif, interrupt.int_xif);
        bdmf_session_print(session, "int_nco = %u = 0x%x\n", interrupt.int_nco, interrupt.int_nco);
        bdmf_session_print(session, "int_lif = %u = 0x%x\n", interrupt.int_lif, interrupt.int_lif);
        bdmf_session_print(session, "int_epn = %u = 0x%x\n", interrupt.int_epn, interrupt.int_epn);
        break;
    }
    case BDMF_interrupt_mask:
    {
        epon_top_interrupt_mask interrupt_mask;
        err = ag_drv_epon_top_interrupt_mask_get(&interrupt_mask);
        bdmf_session_print(session, "int_1pps_mask = %u = 0x%x\n", interrupt_mask.int_1pps_mask, interrupt_mask.int_1pps_mask);
        bdmf_session_print(session, "int_xpcs_tx_mask = %u = 0x%x\n", interrupt_mask.int_xpcs_tx_mask, interrupt_mask.int_xpcs_tx_mask);
        bdmf_session_print(session, "int_xpcs_rx_mask = %u = 0x%x\n", interrupt_mask.int_xpcs_rx_mask, interrupt_mask.int_xpcs_rx_mask);
        bdmf_session_print(session, "int_xif_mask = %u = 0x%x\n", interrupt_mask.int_xif_mask, interrupt_mask.int_xif_mask);
        bdmf_session_print(session, "int_nco_mask = %u = 0x%x\n", interrupt_mask.int_nco_mask, interrupt_mask.int_nco_mask);
        bdmf_session_print(session, "int_lif_mask = %u = 0x%x\n", interrupt_mask.int_lif_mask, interrupt_mask.int_lif_mask);
        bdmf_session_print(session, "int_epn_mask = %u = 0x%x\n", interrupt_mask.int_epn_mask, interrupt_mask.int_epn_mask);
        break;
    }
    case BDMF_control:
    {
        bdmf_boolean cfgtwogigpondns;
        bdmf_boolean cfgtengigponup;
        bdmf_boolean cfgtengigdns;
        err = ag_drv_epon_top_control_get(&cfgtwogigpondns, &cfgtengigponup, &cfgtengigdns);
        bdmf_session_print(session, "cfgtwogigpondns = %u = 0x%x\n", cfgtwogigpondns, cfgtwogigpondns);
        bdmf_session_print(session, "cfgtengigponup = %u = 0x%x\n", cfgtengigponup, cfgtengigponup);
        bdmf_session_print(session, "cfgtengigdns = %u = 0x%x\n", cfgtengigdns, cfgtengigdns);
        break;
    }
    case BDMF_one_pps_mpcp_offset:
    {
        uint32_t cfg_1pps_mpcp_offset;
        err = ag_drv_epon_top_one_pps_mpcp_offset_get(&cfg_1pps_mpcp_offset);
        bdmf_session_print(session, "cfg_1pps_mpcp_offset = %u = 0x%x\n", cfg_1pps_mpcp_offset, cfg_1pps_mpcp_offset);
        break;
    }
    case BDMF_one_pps_captured_mpcp_time:
    {
        uint32_t capture_1pps_mpcp_time;
        err = ag_drv_epon_top_one_pps_captured_mpcp_time_get(&capture_1pps_mpcp_time);
        bdmf_session_print(session, "capture_1pps_mpcp_time = %u = 0x%x\n", capture_1pps_mpcp_time, capture_1pps_mpcp_time);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_epon_top_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        uint32_t scratch=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epon_top_scratch_set( %u)\n", scratch);
        if(!err) ag_drv_epon_top_scratch_set(scratch);
        if(!err) ag_drv_epon_top_scratch_get( &scratch);
        if(!err) bdmf_session_print(session, "ag_drv_epon_top_scratch_get( %u)\n", scratch);
        if(err || scratch!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        epon_top_reset reset = {.xpcsrxrst_n=gtmv(m, 1), .xpcstxrst_n=gtmv(m, 1), .xifrst_n=gtmv(m, 1), .clkprgrst_n=gtmv(m, 1), .ncorst_n=gtmv(m, 1), .lifrst_n=gtmv(m, 1), .epnrst_n=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_epon_top_reset_set( %u %u %u %u %u %u %u)\n", reset.xpcsrxrst_n, reset.xpcstxrst_n, reset.xifrst_n, reset.clkprgrst_n, reset.ncorst_n, reset.lifrst_n, reset.epnrst_n);
        if(!err) ag_drv_epon_top_reset_set(&reset);
        if(!err) ag_drv_epon_top_reset_get( &reset);
        if(!err) bdmf_session_print(session, "ag_drv_epon_top_reset_get( %u %u %u %u %u %u %u)\n", reset.xpcsrxrst_n, reset.xpcstxrst_n, reset.xifrst_n, reset.clkprgrst_n, reset.ncorst_n, reset.lifrst_n, reset.epnrst_n);
        if(err || reset.xpcsrxrst_n!=gtmv(m, 1) || reset.xpcstxrst_n!=gtmv(m, 1) || reset.xifrst_n!=gtmv(m, 1) || reset.clkprgrst_n!=gtmv(m, 1) || reset.ncorst_n!=gtmv(m, 1) || reset.lifrst_n!=gtmv(m, 1) || reset.epnrst_n!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        epon_top_interrupt interrupt = {.int_1pps=gtmv(m, 1), .int_xpcs_tx=gtmv(m, 1), .int_xpcs_rx=gtmv(m, 1), .int_xif=gtmv(m, 1), .int_nco=gtmv(m, 1), .int_lif=gtmv(m, 1), .int_epn=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_epon_top_interrupt_set( %u %u %u %u %u %u %u)\n", interrupt.int_1pps, interrupt.int_xpcs_tx, interrupt.int_xpcs_rx, interrupt.int_xif, interrupt.int_nco, interrupt.int_lif, interrupt.int_epn);
        if(!err) ag_drv_epon_top_interrupt_set(&interrupt);
        if(!err) ag_drv_epon_top_interrupt_get( &interrupt);
        if(!err) bdmf_session_print(session, "ag_drv_epon_top_interrupt_get( %u %u %u %u %u %u %u)\n", interrupt.int_1pps, interrupt.int_xpcs_tx, interrupt.int_xpcs_rx, interrupt.int_xif, interrupt.int_nco, interrupt.int_lif, interrupt.int_epn);
        if(err || interrupt.int_1pps!=gtmv(m, 1) || interrupt.int_xpcs_tx!=gtmv(m, 1) || interrupt.int_xpcs_rx!=gtmv(m, 1) || interrupt.int_xif!=gtmv(m, 1) || interrupt.int_nco!=gtmv(m, 1) || interrupt.int_lif!=gtmv(m, 1) || interrupt.int_epn!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        epon_top_interrupt_mask interrupt_mask = {.int_1pps_mask=gtmv(m, 1), .int_xpcs_tx_mask=gtmv(m, 1), .int_xpcs_rx_mask=gtmv(m, 1), .int_xif_mask=gtmv(m, 1), .int_nco_mask=gtmv(m, 1), .int_lif_mask=gtmv(m, 1), .int_epn_mask=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_epon_top_interrupt_mask_set( %u %u %u %u %u %u %u)\n", interrupt_mask.int_1pps_mask, interrupt_mask.int_xpcs_tx_mask, interrupt_mask.int_xpcs_rx_mask, interrupt_mask.int_xif_mask, interrupt_mask.int_nco_mask, interrupt_mask.int_lif_mask, interrupt_mask.int_epn_mask);
        if(!err) ag_drv_epon_top_interrupt_mask_set(&interrupt_mask);
        if(!err) ag_drv_epon_top_interrupt_mask_get( &interrupt_mask);
        if(!err) bdmf_session_print(session, "ag_drv_epon_top_interrupt_mask_get( %u %u %u %u %u %u %u)\n", interrupt_mask.int_1pps_mask, interrupt_mask.int_xpcs_tx_mask, interrupt_mask.int_xpcs_rx_mask, interrupt_mask.int_xif_mask, interrupt_mask.int_nco_mask, interrupt_mask.int_lif_mask, interrupt_mask.int_epn_mask);
        if(err || interrupt_mask.int_1pps_mask!=gtmv(m, 1) || interrupt_mask.int_xpcs_tx_mask!=gtmv(m, 1) || interrupt_mask.int_xpcs_rx_mask!=gtmv(m, 1) || interrupt_mask.int_xif_mask!=gtmv(m, 1) || interrupt_mask.int_nco_mask!=gtmv(m, 1) || interrupt_mask.int_lif_mask!=gtmv(m, 1) || interrupt_mask.int_epn_mask!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean cfgtwogigpondns=gtmv(m, 1);
        bdmf_boolean cfgtengigponup=gtmv(m, 1);
        bdmf_boolean cfgtengigdns=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epon_top_control_set( %u %u %u)\n", cfgtwogigpondns, cfgtengigponup, cfgtengigdns);
        if(!err) ag_drv_epon_top_control_set(cfgtwogigpondns, cfgtengigponup, cfgtengigdns);
        if(!err) ag_drv_epon_top_control_get( &cfgtwogigpondns, &cfgtengigponup, &cfgtengigdns);
        if(!err) bdmf_session_print(session, "ag_drv_epon_top_control_get( %u %u %u)\n", cfgtwogigpondns, cfgtengigponup, cfgtengigdns);
        if(err || cfgtwogigpondns!=gtmv(m, 1) || cfgtengigponup!=gtmv(m, 1) || cfgtengigdns!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfg_1pps_mpcp_offset=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epon_top_one_pps_mpcp_offset_set( %u)\n", cfg_1pps_mpcp_offset);
        if(!err) ag_drv_epon_top_one_pps_mpcp_offset_set(cfg_1pps_mpcp_offset);
        if(!err) ag_drv_epon_top_one_pps_mpcp_offset_get( &cfg_1pps_mpcp_offset);
        if(!err) bdmf_session_print(session, "ag_drv_epon_top_one_pps_mpcp_offset_get( %u)\n", cfg_1pps_mpcp_offset);
        if(err || cfg_1pps_mpcp_offset!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t capture_1pps_mpcp_time=gtmv(m, 32);
        if(!err) ag_drv_epon_top_one_pps_captured_mpcp_time_get( &capture_1pps_mpcp_time);
        if(!err) bdmf_session_print(session, "ag_drv_epon_top_one_pps_captured_mpcp_time_get( %u)\n", capture_1pps_mpcp_time);
    }
    return err;
}

static int bcm_epon_top_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_scratch : reg = &RU_REG(EPON_TOP, SCRATCH); blk = &RU_BLK(EPON_TOP); break;
    case bdmf_address_reset : reg = &RU_REG(EPON_TOP, RESET); blk = &RU_BLK(EPON_TOP); break;
    case bdmf_address_interrupt : reg = &RU_REG(EPON_TOP, INTERRUPT); blk = &RU_BLK(EPON_TOP); break;
    case bdmf_address_interrupt_mask : reg = &RU_REG(EPON_TOP, INTERRUPT_MASK); blk = &RU_BLK(EPON_TOP); break;
    case bdmf_address_control : reg = &RU_REG(EPON_TOP, CONTROL); blk = &RU_BLK(EPON_TOP); break;
    case bdmf_address_one_pps_mpcp_offset : reg = &RU_REG(EPON_TOP, ONE_PPS_MPCP_OFFSET); blk = &RU_BLK(EPON_TOP); break;
    case bdmf_address_one_pps_captured_mpcp_time : reg = &RU_REG(EPON_TOP, ONE_PPS_CAPTURED_MPCP_TIME); blk = &RU_BLK(EPON_TOP); break;
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
                bdmf_session_print(session, 	 "(%5u) 0x%08X\n", j, (uint32_t)((uint32_t*)(blk->addr[i] + reg->addr) + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%08X\n", i, (uint32_t)((uint32_t*)(blk->addr[i] + reg->addr)));
    return 0;
}

bdmfmon_handle_t ag_drv_epon_top_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "epon_top"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "epon_top", "epon_top", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_scratch[]={
            BDMFMON_MAKE_PARM("scratch", "scratch", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_reset[]={
            BDMFMON_MAKE_PARM("xpcsrxrst_n", "xpcsrxrst_n", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("xpcstxrst_n", "xpcstxrst_n", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("xifrst_n", "xifrst_n", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("clkprgrst_n", "clkprgrst_n", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ncorst_n", "ncorst_n", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("lifrst_n", "lifrst_n", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("epnrst_n", "epnrst_n", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_interrupt[]={
            BDMFMON_MAKE_PARM("int_1pps", "int_1pps", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int_xpcs_tx", "int_xpcs_tx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int_xpcs_rx", "int_xpcs_rx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int_xif", "int_xif", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int_nco", "int_nco", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int_lif", "int_lif", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int_epn", "int_epn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_interrupt_mask[]={
            BDMFMON_MAKE_PARM("int_1pps_mask", "int_1pps_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int_xpcs_tx_mask", "int_xpcs_tx_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int_xpcs_rx_mask", "int_xpcs_rx_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int_xif_mask", "int_xif_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int_nco_mask", "int_nco_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int_lif_mask", "int_lif_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int_epn_mask", "int_epn_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_control[]={
            BDMFMON_MAKE_PARM("cfgtwogigpondns", "cfgtwogigpondns", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgtengigponup", "cfgtengigponup", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgtengigdns", "cfgtengigdns", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_one_pps_mpcp_offset[]={
            BDMFMON_MAKE_PARM("cfg_1pps_mpcp_offset", "cfg_1pps_mpcp_offset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="scratch", .val=BDMF_scratch, .parms=set_scratch },
            { .name="reset", .val=BDMF_reset, .parms=set_reset },
            { .name="interrupt", .val=BDMF_interrupt, .parms=set_interrupt },
            { .name="interrupt_mask", .val=BDMF_interrupt_mask, .parms=set_interrupt_mask },
            { .name="control", .val=BDMF_control, .parms=set_control },
            { .name="one_pps_mpcp_offset", .val=BDMF_one_pps_mpcp_offset, .parms=set_one_pps_mpcp_offset },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_epon_top_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="scratch", .val=BDMF_scratch, .parms=set_default },
            { .name="reset", .val=BDMF_reset, .parms=set_default },
            { .name="interrupt", .val=BDMF_interrupt, .parms=set_default },
            { .name="interrupt_mask", .val=BDMF_interrupt_mask, .parms=set_default },
            { .name="control", .val=BDMF_control, .parms=set_default },
            { .name="one_pps_mpcp_offset", .val=BDMF_one_pps_mpcp_offset, .parms=set_default },
            { .name="one_pps_captured_mpcp_time", .val=BDMF_one_pps_captured_mpcp_time, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_epon_top_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_epon_top_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="SCRATCH" , .val=bdmf_address_scratch },
            { .name="RESET" , .val=bdmf_address_reset },
            { .name="INTERRUPT" , .val=bdmf_address_interrupt },
            { .name="INTERRUPT_MASK" , .val=bdmf_address_interrupt_mask },
            { .name="CONTROL" , .val=bdmf_address_control },
            { .name="ONE_PPS_MPCP_OFFSET" , .val=bdmf_address_one_pps_mpcp_offset },
            { .name="ONE_PPS_CAPTURED_MPCP_TIME" , .val=bdmf_address_one_pps_captured_mpcp_time },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_epon_top_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

