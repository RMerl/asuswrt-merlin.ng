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
#include "xrdp_drv_unimac_mib_ag.h"

#define BLOCK_ADDR_COUNT_BITS 2
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_unimac_mib_mib_cntrl_set(uint8_t umac_mib_id, bdmf_boolean tx_cnt_rst, bdmf_boolean runt_cnt_rst, bdmf_boolean rx_cnt_st)
{
    uint32_t reg_mib_cntrl=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT) ||
       (tx_cnt_rst >= _1BITS_MAX_VAL_) ||
       (runt_cnt_rst >= _1BITS_MAX_VAL_) ||
       (rx_cnt_st >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mib_cntrl = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, MIB_CNTRL, TX_CNT_RST, reg_mib_cntrl, tx_cnt_rst);
    reg_mib_cntrl = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, MIB_CNTRL, RUNT_CNT_RST, reg_mib_cntrl, runt_cnt_rst);
    reg_mib_cntrl = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, MIB_CNTRL, RX_CNT_ST, reg_mib_cntrl, rx_cnt_st);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, MIB_CNTRL, reg_mib_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_mib_cntrl_get(uint8_t umac_mib_id, bdmf_boolean *tx_cnt_rst, bdmf_boolean *runt_cnt_rst, bdmf_boolean *rx_cnt_st)
{
    uint32_t reg_mib_cntrl;

#ifdef VALIDATE_PARMS
    if(!tx_cnt_rst || !runt_cnt_rst || !rx_cnt_st)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, MIB_CNTRL, reg_mib_cntrl);

    *tx_cnt_rst = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, MIB_CNTRL, TX_CNT_RST, reg_mib_cntrl);
    *runt_cnt_rst = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, MIB_CNTRL, RUNT_CNT_RST, reg_mib_cntrl);
    *rx_cnt_st = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, MIB_CNTRL, RX_CNT_ST, reg_mib_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr64_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_gr64=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr64 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GR64, GR, reg_gr64, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GR64, reg_gr64);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr64_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_gr64;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GR64, reg_gr64);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GR64, GR, reg_gr64);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr127_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_gr127=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr127 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GR127, GR, reg_gr127, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GR127, reg_gr127);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr127_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_gr127;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GR127, reg_gr127);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GR127, GR, reg_gr127);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr255_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_gr255=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr255 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GR255, GR, reg_gr255, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GR255, reg_gr255);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr255_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_gr255;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GR255, reg_gr255);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GR255, GR, reg_gr255);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr511_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_gr511=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr511 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GR511, GR, reg_gr511, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GR511, reg_gr511);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr511_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_gr511;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GR511, reg_gr511);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GR511, GR, reg_gr511);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr1023_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_gr1023=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr1023 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GR1023, GR, reg_gr1023, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GR1023, reg_gr1023);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr1023_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_gr1023;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GR1023, reg_gr1023);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GR1023, GR, reg_gr1023);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr1518_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_gr1518=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr1518 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GR1518, GR, reg_gr1518, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GR1518, reg_gr1518);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr1518_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_gr1518;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GR1518, reg_gr1518);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GR1518, GR, reg_gr1518);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grmgv_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grmgv=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grmgv = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRMGV, GR, reg_grmgv, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRMGV, reg_grmgv);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grmgv_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grmgv;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRMGV, reg_grmgv);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRMGV, GR, reg_grmgv);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr2047_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_gr2047=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr2047 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GR2047, GR, reg_gr2047, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GR2047, reg_gr2047);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr2047_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_gr2047;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GR2047, reg_gr2047);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GR2047, GR, reg_gr2047);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr4095_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_gr4095=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr4095 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GR4095, GR, reg_gr4095, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GR4095, reg_gr4095);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr4095_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_gr4095;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GR4095, reg_gr4095);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GR4095, GR, reg_gr4095);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr9216_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_gr9216=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr9216 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GR9216, GR, reg_gr9216, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GR9216, reg_gr9216);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gr9216_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_gr9216;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GR9216, reg_gr9216);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GR9216, GR, reg_gr9216);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grpkt_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grpkt=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grpkt = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRPKT, GR, reg_grpkt, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRPKT, reg_grpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grpkt_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grpkt;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRPKT, reg_grpkt);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRPKT, GR, reg_grpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grbyt_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grbyt=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grbyt = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRBYT, GR, reg_grbyt, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRBYT, reg_grbyt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grbyt_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grbyt;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRBYT, reg_grbyt);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRBYT, GR, reg_grbyt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grmca_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grmca=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grmca = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRMCA, GR, reg_grmca, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRMCA, reg_grmca);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grmca_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grmca;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRMCA, reg_grmca);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRMCA, GR, reg_grmca);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grbca_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grbca=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grbca = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRBCA, GR, reg_grbca, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRBCA, reg_grbca);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grbca_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grbca;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRBCA, reg_grbca);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRBCA, GR, reg_grbca);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grfcs_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grfcs=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grfcs = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRFCS, GR, reg_grfcs, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRFCS, reg_grfcs);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grfcs_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grfcs;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRFCS, reg_grfcs);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRFCS, GR, reg_grfcs);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grxcf_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grxcf=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grxcf = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRXCF, GR, reg_grxcf, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRXCF, reg_grxcf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grxcf_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grxcf;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRXCF, reg_grxcf);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRXCF, GR, reg_grxcf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grxpf_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grxpf=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grxpf = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRXPF, GR, reg_grxpf, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRXPF, reg_grxpf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grxpf_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grxpf;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRXPF, reg_grxpf);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRXPF, GR, reg_grxpf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grxuo_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grxuo=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grxuo = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRXUO, GR, reg_grxuo, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRXUO, reg_grxuo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grxuo_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grxuo;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRXUO, reg_grxuo);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRXUO, GR, reg_grxuo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_graln_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_graln=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_graln = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRALN, GR, reg_graln, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRALN, reg_graln);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_graln_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_graln;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRALN, reg_graln);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRALN, GR, reg_graln);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grflr_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grflr=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grflr = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRFLR, GR, reg_grflr, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRFLR, reg_grflr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grflr_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grflr;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRFLR, reg_grflr);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRFLR, GR, reg_grflr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grcde_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grcde=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grcde = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRCDE, GR, reg_grcde, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRCDE, reg_grcde);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grcde_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grcde;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRCDE, reg_grcde);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRCDE, GR, reg_grcde);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grfcr_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grfcr=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grfcr = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRFCR, GR, reg_grfcr, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRFCR, reg_grfcr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grfcr_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grfcr;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRFCR, reg_grfcr);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRFCR, GR, reg_grfcr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grovr_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grovr=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grovr = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GROVR, GR, reg_grovr, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GROVR, reg_grovr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grovr_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grovr;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GROVR, reg_grovr);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GROVR, GR, reg_grovr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grjbr_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grjbr=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grjbr = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRJBR, GR, reg_grjbr, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRJBR, reg_grjbr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grjbr_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grjbr;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRJBR, reg_grjbr);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRJBR, GR, reg_grjbr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grmtue_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grmtue=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grmtue = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRMTUE, GR, reg_grmtue, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRMTUE, reg_grmtue);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grmtue_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grmtue;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRMTUE, reg_grmtue);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRMTUE, GR, reg_grmtue);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grpok_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grpok=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grpok = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRPOK, GR, reg_grpok, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRPOK, reg_grpok);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grpok_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grpok;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRPOK, reg_grpok);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRPOK, GR, reg_grpok);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gruc_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_gruc=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gruc = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRUC, GR, reg_gruc, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRUC, reg_gruc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gruc_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_gruc;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRUC, reg_gruc);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRUC, GR, reg_gruc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grppp_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grppp=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grppp = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRPPP, GR, reg_grppp, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRPPP, reg_grppp);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grppp_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grppp;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRPPP, reg_grppp);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRPPP, GR, reg_grppp);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grcrc_set(uint8_t umac_mib_id, uint32_t gr)
{
    uint32_t reg_grcrc=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grcrc = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GRCRC, GR, reg_grcrc, gr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GRCRC, reg_grcrc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_grcrc_get(uint8_t umac_mib_id, uint32_t *gr)
{
    uint32_t reg_grcrc;

#ifdef VALIDATE_PARMS
    if(!gr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GRCRC, reg_grcrc);

    *gr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GRCRC, GR, reg_grcrc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_rrpkt_set(uint8_t umac_mib_id, uint32_t rr)
{
    uint32_t reg_rrpkt=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rrpkt = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, RRPKT, RR, reg_rrpkt, rr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, RRPKT, reg_rrpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_rrpkt_get(uint8_t umac_mib_id, uint32_t *rr)
{
    uint32_t reg_rrpkt;

#ifdef VALIDATE_PARMS
    if(!rr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, RRPKT, reg_rrpkt);

    *rr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, RRPKT, RR, reg_rrpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_rrund_set(uint8_t umac_mib_id, uint32_t rr)
{
    uint32_t reg_rrund=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rrund = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, RRUND, RR, reg_rrund, rr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, RRUND, reg_rrund);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_rrund_get(uint8_t umac_mib_id, uint32_t *rr)
{
    uint32_t reg_rrund;

#ifdef VALIDATE_PARMS
    if(!rr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, RRUND, reg_rrund);

    *rr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, RRUND, RR, reg_rrund);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_rrfrg_set(uint8_t umac_mib_id, uint32_t rr)
{
    uint32_t reg_rrfrg=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rrfrg = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, RRFRG, RR, reg_rrfrg, rr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, RRFRG, reg_rrfrg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_rrfrg_get(uint8_t umac_mib_id, uint32_t *rr)
{
    uint32_t reg_rrfrg;

#ifdef VALIDATE_PARMS
    if(!rr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, RRFRG, reg_rrfrg);

    *rr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, RRFRG, RR, reg_rrfrg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_rrbyt_set(uint8_t umac_mib_id, uint32_t rr)
{
    uint32_t reg_rrbyt=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rrbyt = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, RRBYT, RR, reg_rrbyt, rr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, RRBYT, reg_rrbyt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_rrbyt_get(uint8_t umac_mib_id, uint32_t *rr)
{
    uint32_t reg_rrbyt;

#ifdef VALIDATE_PARMS
    if(!rr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, RRBYT, reg_rrbyt);

    *rr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, RRBYT, RR, reg_rrbyt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr64_set(uint8_t umac_mib_id, uint32_t tr)
{
    uint32_t reg_tr64=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr64 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, TR64, TR, reg_tr64, tr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, TR64, reg_tr64);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr64_get(uint8_t umac_mib_id, uint32_t *tr)
{
    uint32_t reg_tr64;

#ifdef VALIDATE_PARMS
    if(!tr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, TR64, reg_tr64);

    *tr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, TR64, TR, reg_tr64);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr127_set(uint8_t umac_mib_id, uint32_t tr)
{
    uint32_t reg_tr127=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr127 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, TR127, TR, reg_tr127, tr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, TR127, reg_tr127);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr127_get(uint8_t umac_mib_id, uint32_t *tr)
{
    uint32_t reg_tr127;

#ifdef VALIDATE_PARMS
    if(!tr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, TR127, reg_tr127);

    *tr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, TR127, TR, reg_tr127);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr255_set(uint8_t umac_mib_id, uint32_t tr)
{
    uint32_t reg_tr255=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr255 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, TR255, TR, reg_tr255, tr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, TR255, reg_tr255);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr255_get(uint8_t umac_mib_id, uint32_t *tr)
{
    uint32_t reg_tr255;

#ifdef VALIDATE_PARMS
    if(!tr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, TR255, reg_tr255);

    *tr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, TR255, TR, reg_tr255);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr511_set(uint8_t umac_mib_id, uint32_t tr)
{
    uint32_t reg_tr511=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr511 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, TR511, TR, reg_tr511, tr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, TR511, reg_tr511);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr511_get(uint8_t umac_mib_id, uint32_t *tr)
{
    uint32_t reg_tr511;

#ifdef VALIDATE_PARMS
    if(!tr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, TR511, reg_tr511);

    *tr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, TR511, TR, reg_tr511);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr1023_set(uint8_t umac_mib_id, uint32_t tr)
{
    uint32_t reg_tr1023=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr1023 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, TR1023, TR, reg_tr1023, tr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, TR1023, reg_tr1023);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr1023_get(uint8_t umac_mib_id, uint32_t *tr)
{
    uint32_t reg_tr1023;

#ifdef VALIDATE_PARMS
    if(!tr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, TR1023, reg_tr1023);

    *tr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, TR1023, TR, reg_tr1023);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr1518_set(uint8_t umac_mib_id, uint32_t tr)
{
    uint32_t reg_tr1518=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr1518 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, TR1518, TR, reg_tr1518, tr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, TR1518, reg_tr1518);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr1518_get(uint8_t umac_mib_id, uint32_t *tr)
{
    uint32_t reg_tr1518;

#ifdef VALIDATE_PARMS
    if(!tr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, TR1518, reg_tr1518);

    *tr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, TR1518, TR, reg_tr1518);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_trmgv_set(uint8_t umac_mib_id, uint32_t tr)
{
    uint32_t reg_trmgv=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_trmgv = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, TRMGV, TR, reg_trmgv, tr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, TRMGV, reg_trmgv);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_trmgv_get(uint8_t umac_mib_id, uint32_t *tr)
{
    uint32_t reg_trmgv;

#ifdef VALIDATE_PARMS
    if(!tr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, TRMGV, reg_trmgv);

    *tr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, TRMGV, TR, reg_trmgv);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr2047_set(uint8_t umac_mib_id, uint32_t tr)
{
    uint32_t reg_tr2047=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr2047 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, TR2047, TR, reg_tr2047, tr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, TR2047, reg_tr2047);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr2047_get(uint8_t umac_mib_id, uint32_t *tr)
{
    uint32_t reg_tr2047;

#ifdef VALIDATE_PARMS
    if(!tr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, TR2047, reg_tr2047);

    *tr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, TR2047, TR, reg_tr2047);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr4095_set(uint8_t umac_mib_id, uint32_t tr)
{
    uint32_t reg_tr4095=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr4095 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, TR4095, TR, reg_tr4095, tr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, TR4095, reg_tr4095);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr4095_get(uint8_t umac_mib_id, uint32_t *tr)
{
    uint32_t reg_tr4095;

#ifdef VALIDATE_PARMS
    if(!tr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, TR4095, reg_tr4095);

    *tr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, TR4095, TR, reg_tr4095);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr9216_set(uint8_t umac_mib_id, uint32_t tr)
{
    uint32_t reg_tr9216=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr9216 = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, TR9216, TR, reg_tr9216, tr);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, TR9216, reg_tr9216);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_tr9216_get(uint8_t umac_mib_id, uint32_t *tr)
{
    uint32_t reg_tr9216;

#ifdef VALIDATE_PARMS
    if(!tr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, TR9216, reg_tr9216);

    *tr = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, TR9216, TR, reg_tr9216);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtpkt_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtpkt=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtpkt = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTPKT, GT, reg_gtpkt, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTPKT, reg_gtpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtpkt_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtpkt;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTPKT, reg_gtpkt);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTPKT, GT, reg_gtpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtmca_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtmca=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtmca = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTMCA, GT, reg_gtmca, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTMCA, reg_gtmca);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtmca_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtmca;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTMCA, reg_gtmca);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTMCA, GT, reg_gtmca);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtbca_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtbca=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtbca = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTBCA, GT, reg_gtbca, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTBCA, reg_gtbca);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtbca_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtbca;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTBCA, reg_gtbca);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTBCA, GT, reg_gtbca);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtxpf_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtxpf=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtxpf = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTXPF, GT, reg_gtxpf, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTXPF, reg_gtxpf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtxpf_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtxpf;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTXPF, reg_gtxpf);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTXPF, GT, reg_gtxpf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtxcf_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtxcf=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtxcf = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTXCF, GT, reg_gtxcf, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTXCF, reg_gtxcf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtxcf_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtxcf;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTXCF, reg_gtxcf);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTXCF, GT, reg_gtxcf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtfcs_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtfcs=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtfcs = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTFCS, GT, reg_gtfcs, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTFCS, reg_gtfcs);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtfcs_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtfcs;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTFCS, reg_gtfcs);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTFCS, GT, reg_gtfcs);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtovr_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtovr=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtovr = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTOVR, GT, reg_gtovr, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTOVR, reg_gtovr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtovr_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtovr;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTOVR, reg_gtovr);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTOVR, GT, reg_gtovr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtdrf_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtdrf=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtdrf = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTDRF, GT, reg_gtdrf, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTDRF, reg_gtdrf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtdrf_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtdrf;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTDRF, reg_gtdrf);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTDRF, GT, reg_gtdrf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtedf_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtedf=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtedf = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTEDF, GT, reg_gtedf, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTEDF, reg_gtedf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtedf_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtedf;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTEDF, reg_gtedf);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTEDF, GT, reg_gtedf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtscl_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtscl=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtscl = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTSCL, GT, reg_gtscl, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTSCL, reg_gtscl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtscl_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtscl;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTSCL, reg_gtscl);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTSCL, GT, reg_gtscl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtmcl_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtmcl=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtmcl = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTMCL, GT, reg_gtmcl, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTMCL, reg_gtmcl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtmcl_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtmcl;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTMCL, reg_gtmcl);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTMCL, GT, reg_gtmcl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtlcl_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtlcl=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtlcl = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTLCL, GT, reg_gtlcl, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTLCL, reg_gtlcl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtlcl_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtlcl;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTLCL, reg_gtlcl);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTLCL, GT, reg_gtlcl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtxcl_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtxcl=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtxcl = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTXCL, GT, reg_gtxcl, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTXCL, reg_gtxcl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtxcl_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtxcl;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTXCL, reg_gtxcl);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTXCL, GT, reg_gtxcl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtfrg_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtfrg=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtfrg = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTFRG, GT, reg_gtfrg, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTFRG, reg_gtfrg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtfrg_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtfrg;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTFRG, reg_gtfrg);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTFRG, GT, reg_gtfrg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtncl_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtncl=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtncl = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTNCL, GT, reg_gtncl, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTNCL, reg_gtncl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtncl_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtncl;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTNCL, reg_gtncl);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTNCL, GT, reg_gtncl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtjbr_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtjbr=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtjbr = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTJBR, GT, reg_gtjbr, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTJBR, reg_gtjbr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtjbr_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtjbr;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTJBR, reg_gtjbr);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTJBR, GT, reg_gtjbr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtbyt_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtbyt=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtbyt = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTBYT, GT, reg_gtbyt, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTBYT, reg_gtbyt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtbyt_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtbyt;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTBYT, reg_gtbyt);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTBYT, GT, reg_gtbyt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtpok_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtpok=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtpok = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTPOK, GT, reg_gtpok, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTPOK, reg_gtpok);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtpok_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtpok;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTPOK, reg_gtpok);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTPOK, GT, reg_gtpok);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtuc_set(uint8_t umac_mib_id, uint32_t gt)
{
    uint32_t reg_gtuc=0;

#ifdef VALIDATE_PARMS
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtuc = RU_FIELD_SET(umac_mib_id, UNIMAC_MIB, GTUC, GT, reg_gtuc, gt);

    RU_REG_WRITE(umac_mib_id, UNIMAC_MIB, GTUC, reg_gtuc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_mib_gtuc_get(uint8_t umac_mib_id, uint32_t *gt)
{
    uint32_t reg_gtuc;

#ifdef VALIDATE_PARMS
    if(!gt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_mib_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_mib_id, UNIMAC_MIB, GTUC, reg_gtuc);

    *gt = RU_FIELD_GET(umac_mib_id, UNIMAC_MIB, GTUC, GT, reg_gtuc);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_mib_cntrl,
    bdmf_address_gr64,
    bdmf_address_gr127,
    bdmf_address_gr255,
    bdmf_address_gr511,
    bdmf_address_gr1023,
    bdmf_address_gr1518,
    bdmf_address_grmgv,
    bdmf_address_gr2047,
    bdmf_address_gr4095,
    bdmf_address_gr9216,
    bdmf_address_grpkt,
    bdmf_address_grbyt,
    bdmf_address_grmca,
    bdmf_address_grbca,
    bdmf_address_grfcs,
    bdmf_address_grxcf,
    bdmf_address_grxpf,
    bdmf_address_grxuo,
    bdmf_address_graln,
    bdmf_address_grflr,
    bdmf_address_grcde,
    bdmf_address_grfcr,
    bdmf_address_grovr,
    bdmf_address_grjbr,
    bdmf_address_grmtue,
    bdmf_address_grpok,
    bdmf_address_gruc,
    bdmf_address_grppp,
    bdmf_address_grcrc,
    bdmf_address_rrpkt,
    bdmf_address_rrund,
    bdmf_address_rrfrg,
    bdmf_address_rrbyt,
    bdmf_address_tr64,
    bdmf_address_tr127,
    bdmf_address_tr255,
    bdmf_address_tr511,
    bdmf_address_tr1023,
    bdmf_address_tr1518,
    bdmf_address_trmgv,
    bdmf_address_tr2047,
    bdmf_address_tr4095,
    bdmf_address_tr9216,
    bdmf_address_gtpkt,
    bdmf_address_gtmca,
    bdmf_address_gtbca,
    bdmf_address_gtxpf,
    bdmf_address_gtxcf,
    bdmf_address_gtfcs,
    bdmf_address_gtovr,
    bdmf_address_gtdrf,
    bdmf_address_gtedf,
    bdmf_address_gtscl,
    bdmf_address_gtmcl,
    bdmf_address_gtlcl,
    bdmf_address_gtxcl,
    bdmf_address_gtfrg,
    bdmf_address_gtncl,
    bdmf_address_gtjbr,
    bdmf_address_gtbyt,
    bdmf_address_gtpok,
    bdmf_address_gtuc,
}
bdmf_address;

static int bcm_unimac_mib_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_unimac_mib_mib_cntrl:
        err = ag_drv_unimac_mib_mib_cntrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_unimac_mib_gr64:
        err = ag_drv_unimac_mib_gr64_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gr127:
        err = ag_drv_unimac_mib_gr127_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gr255:
        err = ag_drv_unimac_mib_gr255_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gr511:
        err = ag_drv_unimac_mib_gr511_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gr1023:
        err = ag_drv_unimac_mib_gr1023_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gr1518:
        err = ag_drv_unimac_mib_gr1518_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grmgv:
        err = ag_drv_unimac_mib_grmgv_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gr2047:
        err = ag_drv_unimac_mib_gr2047_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gr4095:
        err = ag_drv_unimac_mib_gr4095_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gr9216:
        err = ag_drv_unimac_mib_gr9216_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grpkt:
        err = ag_drv_unimac_mib_grpkt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grbyt:
        err = ag_drv_unimac_mib_grbyt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grmca:
        err = ag_drv_unimac_mib_grmca_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grbca:
        err = ag_drv_unimac_mib_grbca_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grfcs:
        err = ag_drv_unimac_mib_grfcs_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grxcf:
        err = ag_drv_unimac_mib_grxcf_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grxpf:
        err = ag_drv_unimac_mib_grxpf_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grxuo:
        err = ag_drv_unimac_mib_grxuo_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_graln:
        err = ag_drv_unimac_mib_graln_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grflr:
        err = ag_drv_unimac_mib_grflr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grcde:
        err = ag_drv_unimac_mib_grcde_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grfcr:
        err = ag_drv_unimac_mib_grfcr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grovr:
        err = ag_drv_unimac_mib_grovr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grjbr:
        err = ag_drv_unimac_mib_grjbr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grmtue:
        err = ag_drv_unimac_mib_grmtue_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grpok:
        err = ag_drv_unimac_mib_grpok_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gruc:
        err = ag_drv_unimac_mib_gruc_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grppp:
        err = ag_drv_unimac_mib_grppp_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_grcrc:
        err = ag_drv_unimac_mib_grcrc_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_rrpkt:
        err = ag_drv_unimac_mib_rrpkt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_rrund:
        err = ag_drv_unimac_mib_rrund_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_rrfrg:
        err = ag_drv_unimac_mib_rrfrg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_rrbyt:
        err = ag_drv_unimac_mib_rrbyt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_tr64:
        err = ag_drv_unimac_mib_tr64_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_tr127:
        err = ag_drv_unimac_mib_tr127_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_tr255:
        err = ag_drv_unimac_mib_tr255_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_tr511:
        err = ag_drv_unimac_mib_tr511_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_tr1023:
        err = ag_drv_unimac_mib_tr1023_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_tr1518:
        err = ag_drv_unimac_mib_tr1518_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_trmgv:
        err = ag_drv_unimac_mib_trmgv_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_tr2047:
        err = ag_drv_unimac_mib_tr2047_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_tr4095:
        err = ag_drv_unimac_mib_tr4095_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_tr9216:
        err = ag_drv_unimac_mib_tr9216_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtpkt:
        err = ag_drv_unimac_mib_gtpkt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtmca:
        err = ag_drv_unimac_mib_gtmca_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtbca:
        err = ag_drv_unimac_mib_gtbca_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtxpf:
        err = ag_drv_unimac_mib_gtxpf_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtxcf:
        err = ag_drv_unimac_mib_gtxcf_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtfcs:
        err = ag_drv_unimac_mib_gtfcs_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtovr:
        err = ag_drv_unimac_mib_gtovr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtdrf:
        err = ag_drv_unimac_mib_gtdrf_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtedf:
        err = ag_drv_unimac_mib_gtedf_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtscl:
        err = ag_drv_unimac_mib_gtscl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtmcl:
        err = ag_drv_unimac_mib_gtmcl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtlcl:
        err = ag_drv_unimac_mib_gtlcl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtxcl:
        err = ag_drv_unimac_mib_gtxcl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtfrg:
        err = ag_drv_unimac_mib_gtfrg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtncl:
        err = ag_drv_unimac_mib_gtncl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtjbr:
        err = ag_drv_unimac_mib_gtjbr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtbyt:
        err = ag_drv_unimac_mib_gtbyt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtpok:
        err = ag_drv_unimac_mib_gtpok_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_mib_gtuc:
        err = ag_drv_unimac_mib_gtuc_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_unimac_mib_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_unimac_mib_mib_cntrl:
    {
        bdmf_boolean tx_cnt_rst;
        bdmf_boolean runt_cnt_rst;
        bdmf_boolean rx_cnt_st;
        err = ag_drv_unimac_mib_mib_cntrl_get(parm[1].value.unumber, &tx_cnt_rst, &runt_cnt_rst, &rx_cnt_st);
        bdmf_session_print(session, "tx_cnt_rst = %u (0x%x)\n", tx_cnt_rst, tx_cnt_rst);
        bdmf_session_print(session, "runt_cnt_rst = %u (0x%x)\n", runt_cnt_rst, runt_cnt_rst);
        bdmf_session_print(session, "rx_cnt_st = %u (0x%x)\n", rx_cnt_st, rx_cnt_st);
        break;
    }
    case cli_unimac_mib_gr64:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_gr64_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_gr127:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_gr127_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_gr255:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_gr255_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_gr511:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_gr511_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_gr1023:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_gr1023_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_gr1518:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_gr1518_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grmgv:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grmgv_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_gr2047:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_gr2047_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_gr4095:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_gr4095_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_gr9216:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_gr9216_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grpkt:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grpkt_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grbyt:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grbyt_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grmca:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grmca_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grbca:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grbca_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grfcs:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grfcs_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grxcf:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grxcf_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grxpf:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grxpf_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grxuo:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grxuo_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_graln:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_graln_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grflr:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grflr_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grcde:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grcde_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grfcr:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grfcr_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grovr:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grovr_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grjbr:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grjbr_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grmtue:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grmtue_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grpok:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grpok_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_gruc:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_gruc_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grppp:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grppp_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_grcrc:
    {
        uint32_t gr;
        err = ag_drv_unimac_mib_grcrc_get(parm[1].value.unumber, &gr);
        bdmf_session_print(session, "gr = %u (0x%x)\n", gr, gr);
        break;
    }
    case cli_unimac_mib_rrpkt:
    {
        uint32_t rr;
        err = ag_drv_unimac_mib_rrpkt_get(parm[1].value.unumber, &rr);
        bdmf_session_print(session, "rr = %u (0x%x)\n", rr, rr);
        break;
    }
    case cli_unimac_mib_rrund:
    {
        uint32_t rr;
        err = ag_drv_unimac_mib_rrund_get(parm[1].value.unumber, &rr);
        bdmf_session_print(session, "rr = %u (0x%x)\n", rr, rr);
        break;
    }
    case cli_unimac_mib_rrfrg:
    {
        uint32_t rr;
        err = ag_drv_unimac_mib_rrfrg_get(parm[1].value.unumber, &rr);
        bdmf_session_print(session, "rr = %u (0x%x)\n", rr, rr);
        break;
    }
    case cli_unimac_mib_rrbyt:
    {
        uint32_t rr;
        err = ag_drv_unimac_mib_rrbyt_get(parm[1].value.unumber, &rr);
        bdmf_session_print(session, "rr = %u (0x%x)\n", rr, rr);
        break;
    }
    case cli_unimac_mib_tr64:
    {
        uint32_t tr;
        err = ag_drv_unimac_mib_tr64_get(parm[1].value.unumber, &tr);
        bdmf_session_print(session, "tr = %u (0x%x)\n", tr, tr);
        break;
    }
    case cli_unimac_mib_tr127:
    {
        uint32_t tr;
        err = ag_drv_unimac_mib_tr127_get(parm[1].value.unumber, &tr);
        bdmf_session_print(session, "tr = %u (0x%x)\n", tr, tr);
        break;
    }
    case cli_unimac_mib_tr255:
    {
        uint32_t tr;
        err = ag_drv_unimac_mib_tr255_get(parm[1].value.unumber, &tr);
        bdmf_session_print(session, "tr = %u (0x%x)\n", tr, tr);
        break;
    }
    case cli_unimac_mib_tr511:
    {
        uint32_t tr;
        err = ag_drv_unimac_mib_tr511_get(parm[1].value.unumber, &tr);
        bdmf_session_print(session, "tr = %u (0x%x)\n", tr, tr);
        break;
    }
    case cli_unimac_mib_tr1023:
    {
        uint32_t tr;
        err = ag_drv_unimac_mib_tr1023_get(parm[1].value.unumber, &tr);
        bdmf_session_print(session, "tr = %u (0x%x)\n", tr, tr);
        break;
    }
    case cli_unimac_mib_tr1518:
    {
        uint32_t tr;
        err = ag_drv_unimac_mib_tr1518_get(parm[1].value.unumber, &tr);
        bdmf_session_print(session, "tr = %u (0x%x)\n", tr, tr);
        break;
    }
    case cli_unimac_mib_trmgv:
    {
        uint32_t tr;
        err = ag_drv_unimac_mib_trmgv_get(parm[1].value.unumber, &tr);
        bdmf_session_print(session, "tr = %u (0x%x)\n", tr, tr);
        break;
    }
    case cli_unimac_mib_tr2047:
    {
        uint32_t tr;
        err = ag_drv_unimac_mib_tr2047_get(parm[1].value.unumber, &tr);
        bdmf_session_print(session, "tr = %u (0x%x)\n", tr, tr);
        break;
    }
    case cli_unimac_mib_tr4095:
    {
        uint32_t tr;
        err = ag_drv_unimac_mib_tr4095_get(parm[1].value.unumber, &tr);
        bdmf_session_print(session, "tr = %u (0x%x)\n", tr, tr);
        break;
    }
    case cli_unimac_mib_tr9216:
    {
        uint32_t tr;
        err = ag_drv_unimac_mib_tr9216_get(parm[1].value.unumber, &tr);
        bdmf_session_print(session, "tr = %u (0x%x)\n", tr, tr);
        break;
    }
    case cli_unimac_mib_gtpkt:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtpkt_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtmca:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtmca_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtbca:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtbca_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtxpf:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtxpf_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtxcf:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtxcf_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtfcs:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtfcs_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtovr:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtovr_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtdrf:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtdrf_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtedf:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtedf_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtscl:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtscl_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtmcl:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtmcl_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtlcl:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtlcl_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtxcl:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtxcl_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtfrg:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtfrg_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtncl:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtncl_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtjbr:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtjbr_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtbyt:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtbyt_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtpok:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtpok_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    case cli_unimac_mib_gtuc:
    {
        uint32_t gt;
        err = ag_drv_unimac_mib_gtuc_get(parm[1].value.unumber, &gt);
        bdmf_session_print(session, "gt = %u (0x%x)\n", gt, gt);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_unimac_mib_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t umac_mib_id = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        bdmf_boolean tx_cnt_rst=gtmv(m, 1);
        bdmf_boolean runt_cnt_rst=gtmv(m, 1);
        bdmf_boolean rx_cnt_st=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_mib_cntrl_set(%u %u %u %u)\n", umac_mib_id, tx_cnt_rst, runt_cnt_rst, rx_cnt_st);
        if(!err) ag_drv_unimac_mib_mib_cntrl_set(umac_mib_id, tx_cnt_rst, runt_cnt_rst, rx_cnt_st);
        if(!err) ag_drv_unimac_mib_mib_cntrl_get( umac_mib_id, &tx_cnt_rst, &runt_cnt_rst, &rx_cnt_st);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_mib_cntrl_get(%u %u %u %u)\n", umac_mib_id, tx_cnt_rst, runt_cnt_rst, rx_cnt_st);
        if(err || tx_cnt_rst!=gtmv(m, 1) || runt_cnt_rst!=gtmv(m, 1) || rx_cnt_st!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr64_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr64_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr64_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr64_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr127_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr127_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr127_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr127_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr255_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr255_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr255_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr255_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr511_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr511_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr511_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr511_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr1023_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr1023_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr1023_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr1023_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr1518_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr1518_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr1518_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr1518_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grmgv_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grmgv_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grmgv_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grmgv_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr2047_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr2047_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr2047_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr2047_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr4095_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr4095_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr4095_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr4095_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr9216_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr9216_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gr9216_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gr9216_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grpkt_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grpkt_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grpkt_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grpkt_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grbyt_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grbyt_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grbyt_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grbyt_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grmca_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grmca_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grmca_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grmca_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grbca_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grbca_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grbca_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grbca_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grfcs_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grfcs_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grfcs_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grfcs_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grxcf_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grxcf_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grxcf_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grxcf_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grxpf_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grxpf_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grxpf_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grxpf_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grxuo_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grxuo_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grxuo_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grxuo_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_graln_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_graln_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_graln_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_graln_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grflr_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grflr_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grflr_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grflr_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grcde_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grcde_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grcde_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grcde_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grfcr_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grfcr_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grfcr_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grfcr_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grovr_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grovr_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grovr_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grovr_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grjbr_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grjbr_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grjbr_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grjbr_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grmtue_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grmtue_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grmtue_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grmtue_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grpok_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grpok_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grpok_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grpok_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gruc_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gruc_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_gruc_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gruc_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grppp_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grppp_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grppp_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grppp_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grcrc_set(%u %u)\n", umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grcrc_set(umac_mib_id, gr);
        if(!err) ag_drv_unimac_mib_grcrc_get( umac_mib_id, &gr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_grcrc_get(%u %u)\n", umac_mib_id, gr);
        if(err || gr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_rrpkt_set(%u %u)\n", umac_mib_id, rr);
        if(!err) ag_drv_unimac_mib_rrpkt_set(umac_mib_id, rr);
        if(!err) ag_drv_unimac_mib_rrpkt_get( umac_mib_id, &rr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_rrpkt_get(%u %u)\n", umac_mib_id, rr);
        if(err || rr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_rrund_set(%u %u)\n", umac_mib_id, rr);
        if(!err) ag_drv_unimac_mib_rrund_set(umac_mib_id, rr);
        if(!err) ag_drv_unimac_mib_rrund_get( umac_mib_id, &rr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_rrund_get(%u %u)\n", umac_mib_id, rr);
        if(err || rr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_rrfrg_set(%u %u)\n", umac_mib_id, rr);
        if(!err) ag_drv_unimac_mib_rrfrg_set(umac_mib_id, rr);
        if(!err) ag_drv_unimac_mib_rrfrg_get( umac_mib_id, &rr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_rrfrg_get(%u %u)\n", umac_mib_id, rr);
        if(err || rr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_rrbyt_set(%u %u)\n", umac_mib_id, rr);
        if(!err) ag_drv_unimac_mib_rrbyt_set(umac_mib_id, rr);
        if(!err) ag_drv_unimac_mib_rrbyt_get( umac_mib_id, &rr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_rrbyt_get(%u %u)\n", umac_mib_id, rr);
        if(err || rr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t tr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr64_set(%u %u)\n", umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr64_set(umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr64_get( umac_mib_id, &tr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr64_get(%u %u)\n", umac_mib_id, tr);
        if(err || tr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t tr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr127_set(%u %u)\n", umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr127_set(umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr127_get( umac_mib_id, &tr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr127_get(%u %u)\n", umac_mib_id, tr);
        if(err || tr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t tr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr255_set(%u %u)\n", umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr255_set(umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr255_get( umac_mib_id, &tr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr255_get(%u %u)\n", umac_mib_id, tr);
        if(err || tr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t tr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr511_set(%u %u)\n", umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr511_set(umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr511_get( umac_mib_id, &tr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr511_get(%u %u)\n", umac_mib_id, tr);
        if(err || tr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t tr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr1023_set(%u %u)\n", umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr1023_set(umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr1023_get( umac_mib_id, &tr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr1023_get(%u %u)\n", umac_mib_id, tr);
        if(err || tr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t tr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr1518_set(%u %u)\n", umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr1518_set(umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr1518_get( umac_mib_id, &tr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr1518_get(%u %u)\n", umac_mib_id, tr);
        if(err || tr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t tr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_trmgv_set(%u %u)\n", umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_trmgv_set(umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_trmgv_get( umac_mib_id, &tr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_trmgv_get(%u %u)\n", umac_mib_id, tr);
        if(err || tr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t tr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr2047_set(%u %u)\n", umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr2047_set(umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr2047_get( umac_mib_id, &tr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr2047_get(%u %u)\n", umac_mib_id, tr);
        if(err || tr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t tr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr4095_set(%u %u)\n", umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr4095_set(umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr4095_get( umac_mib_id, &tr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr4095_get(%u %u)\n", umac_mib_id, tr);
        if(err || tr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t tr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr9216_set(%u %u)\n", umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr9216_set(umac_mib_id, tr);
        if(!err) ag_drv_unimac_mib_tr9216_get( umac_mib_id, &tr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_tr9216_get(%u %u)\n", umac_mib_id, tr);
        if(err || tr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtpkt_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtpkt_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtpkt_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtpkt_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtmca_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtmca_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtmca_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtmca_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtbca_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtbca_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtbca_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtbca_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtxpf_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtxpf_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtxpf_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtxpf_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtxcf_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtxcf_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtxcf_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtxcf_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtfcs_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtfcs_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtfcs_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtfcs_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtovr_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtovr_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtovr_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtovr_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtdrf_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtdrf_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtdrf_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtdrf_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtedf_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtedf_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtedf_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtedf_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtscl_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtscl_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtscl_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtscl_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtmcl_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtmcl_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtmcl_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtmcl_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtlcl_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtlcl_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtlcl_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtlcl_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtxcl_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtxcl_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtxcl_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtxcl_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtfrg_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtfrg_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtfrg_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtfrg_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtncl_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtncl_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtncl_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtncl_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtjbr_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtjbr_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtjbr_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtjbr_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtbyt_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtbyt_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtbyt_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtbyt_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtpok_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtpok_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtpok_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtpok_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtuc_set(%u %u)\n", umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtuc_set(umac_mib_id, gt);
        if(!err) ag_drv_unimac_mib_gtuc_get( umac_mib_id, &gt);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_mib_gtuc_get(%u %u)\n", umac_mib_id, gt);
        if(err || gt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_unimac_mib_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_mib_cntrl : reg = &RU_REG(UNIMAC_MIB, MIB_CNTRL); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gr64 : reg = &RU_REG(UNIMAC_MIB, GR64); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gr127 : reg = &RU_REG(UNIMAC_MIB, GR127); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gr255 : reg = &RU_REG(UNIMAC_MIB, GR255); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gr511 : reg = &RU_REG(UNIMAC_MIB, GR511); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gr1023 : reg = &RU_REG(UNIMAC_MIB, GR1023); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gr1518 : reg = &RU_REG(UNIMAC_MIB, GR1518); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grmgv : reg = &RU_REG(UNIMAC_MIB, GRMGV); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gr2047 : reg = &RU_REG(UNIMAC_MIB, GR2047); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gr4095 : reg = &RU_REG(UNIMAC_MIB, GR4095); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gr9216 : reg = &RU_REG(UNIMAC_MIB, GR9216); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grpkt : reg = &RU_REG(UNIMAC_MIB, GRPKT); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grbyt : reg = &RU_REG(UNIMAC_MIB, GRBYT); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grmca : reg = &RU_REG(UNIMAC_MIB, GRMCA); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grbca : reg = &RU_REG(UNIMAC_MIB, GRBCA); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grfcs : reg = &RU_REG(UNIMAC_MIB, GRFCS); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grxcf : reg = &RU_REG(UNIMAC_MIB, GRXCF); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grxpf : reg = &RU_REG(UNIMAC_MIB, GRXPF); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grxuo : reg = &RU_REG(UNIMAC_MIB, GRXUO); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_graln : reg = &RU_REG(UNIMAC_MIB, GRALN); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grflr : reg = &RU_REG(UNIMAC_MIB, GRFLR); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grcde : reg = &RU_REG(UNIMAC_MIB, GRCDE); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grfcr : reg = &RU_REG(UNIMAC_MIB, GRFCR); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grovr : reg = &RU_REG(UNIMAC_MIB, GROVR); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grjbr : reg = &RU_REG(UNIMAC_MIB, GRJBR); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grmtue : reg = &RU_REG(UNIMAC_MIB, GRMTUE); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grpok : reg = &RU_REG(UNIMAC_MIB, GRPOK); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gruc : reg = &RU_REG(UNIMAC_MIB, GRUC); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grppp : reg = &RU_REG(UNIMAC_MIB, GRPPP); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_grcrc : reg = &RU_REG(UNIMAC_MIB, GRCRC); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_rrpkt : reg = &RU_REG(UNIMAC_MIB, RRPKT); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_rrund : reg = &RU_REG(UNIMAC_MIB, RRUND); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_rrfrg : reg = &RU_REG(UNIMAC_MIB, RRFRG); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_rrbyt : reg = &RU_REG(UNIMAC_MIB, RRBYT); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_tr64 : reg = &RU_REG(UNIMAC_MIB, TR64); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_tr127 : reg = &RU_REG(UNIMAC_MIB, TR127); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_tr255 : reg = &RU_REG(UNIMAC_MIB, TR255); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_tr511 : reg = &RU_REG(UNIMAC_MIB, TR511); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_tr1023 : reg = &RU_REG(UNIMAC_MIB, TR1023); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_tr1518 : reg = &RU_REG(UNIMAC_MIB, TR1518); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_trmgv : reg = &RU_REG(UNIMAC_MIB, TRMGV); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_tr2047 : reg = &RU_REG(UNIMAC_MIB, TR2047); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_tr4095 : reg = &RU_REG(UNIMAC_MIB, TR4095); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_tr9216 : reg = &RU_REG(UNIMAC_MIB, TR9216); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtpkt : reg = &RU_REG(UNIMAC_MIB, GTPKT); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtmca : reg = &RU_REG(UNIMAC_MIB, GTMCA); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtbca : reg = &RU_REG(UNIMAC_MIB, GTBCA); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtxpf : reg = &RU_REG(UNIMAC_MIB, GTXPF); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtxcf : reg = &RU_REG(UNIMAC_MIB, GTXCF); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtfcs : reg = &RU_REG(UNIMAC_MIB, GTFCS); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtovr : reg = &RU_REG(UNIMAC_MIB, GTOVR); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtdrf : reg = &RU_REG(UNIMAC_MIB, GTDRF); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtedf : reg = &RU_REG(UNIMAC_MIB, GTEDF); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtscl : reg = &RU_REG(UNIMAC_MIB, GTSCL); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtmcl : reg = &RU_REG(UNIMAC_MIB, GTMCL); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtlcl : reg = &RU_REG(UNIMAC_MIB, GTLCL); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtxcl : reg = &RU_REG(UNIMAC_MIB, GTXCL); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtfrg : reg = &RU_REG(UNIMAC_MIB, GTFRG); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtncl : reg = &RU_REG(UNIMAC_MIB, GTNCL); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtjbr : reg = &RU_REG(UNIMAC_MIB, GTJBR); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtbyt : reg = &RU_REG(UNIMAC_MIB, GTBYT); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtpok : reg = &RU_REG(UNIMAC_MIB, GTPOK); blk = &RU_BLK(UNIMAC_MIB); break;
    case bdmf_address_gtuc : reg = &RU_REG(UNIMAC_MIB, GTUC); blk = &RU_BLK(UNIMAC_MIB); break;
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

bdmfmon_handle_t ag_drv_unimac_mib_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "unimac_mib"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "unimac_mib", "unimac_mib", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_mib_cntrl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tx_cnt_rst", "tx_cnt_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("runt_cnt_rst", "runt_cnt_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rx_cnt_st", "rx_cnt_st", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr64[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr127[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr255[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr511[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr1023[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr1518[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grmgv[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr2047[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr4095[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr9216[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grpkt[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grbyt[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grmca[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grbca[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grfcs[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grxcf[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grxpf[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grxuo[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_graln[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grflr[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grcde[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grfcr[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grovr[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grjbr[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grmtue[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grpok[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gruc[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grppp[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grcrc[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gr", "gr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rrpkt[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("rr", "rr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rrund[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("rr", "rr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rrfrg[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("rr", "rr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rrbyt[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("rr", "rr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr64[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tr", "tr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr127[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tr", "tr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr255[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tr", "tr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr511[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tr", "tr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr1023[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tr", "tr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr1518[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tr", "tr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_trmgv[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tr", "tr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr2047[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tr", "tr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr4095[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tr", "tr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr9216[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tr", "tr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtpkt[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtmca[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtbca[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtxpf[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtxcf[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtfcs[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtovr[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtdrf[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtedf[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtscl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtmcl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtlcl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtxcl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtfrg[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtncl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtjbr[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtbyt[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtpok[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtuc[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gt", "gt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="mib_cntrl", .val=cli_unimac_mib_mib_cntrl, .parms=set_mib_cntrl },
            { .name="gr64", .val=cli_unimac_mib_gr64, .parms=set_gr64 },
            { .name="gr127", .val=cli_unimac_mib_gr127, .parms=set_gr127 },
            { .name="gr255", .val=cli_unimac_mib_gr255, .parms=set_gr255 },
            { .name="gr511", .val=cli_unimac_mib_gr511, .parms=set_gr511 },
            { .name="gr1023", .val=cli_unimac_mib_gr1023, .parms=set_gr1023 },
            { .name="gr1518", .val=cli_unimac_mib_gr1518, .parms=set_gr1518 },
            { .name="grmgv", .val=cli_unimac_mib_grmgv, .parms=set_grmgv },
            { .name="gr2047", .val=cli_unimac_mib_gr2047, .parms=set_gr2047 },
            { .name="gr4095", .val=cli_unimac_mib_gr4095, .parms=set_gr4095 },
            { .name="gr9216", .val=cli_unimac_mib_gr9216, .parms=set_gr9216 },
            { .name="grpkt", .val=cli_unimac_mib_grpkt, .parms=set_grpkt },
            { .name="grbyt", .val=cli_unimac_mib_grbyt, .parms=set_grbyt },
            { .name="grmca", .val=cli_unimac_mib_grmca, .parms=set_grmca },
            { .name="grbca", .val=cli_unimac_mib_grbca, .parms=set_grbca },
            { .name="grfcs", .val=cli_unimac_mib_grfcs, .parms=set_grfcs },
            { .name="grxcf", .val=cli_unimac_mib_grxcf, .parms=set_grxcf },
            { .name="grxpf", .val=cli_unimac_mib_grxpf, .parms=set_grxpf },
            { .name="grxuo", .val=cli_unimac_mib_grxuo, .parms=set_grxuo },
            { .name="graln", .val=cli_unimac_mib_graln, .parms=set_graln },
            { .name="grflr", .val=cli_unimac_mib_grflr, .parms=set_grflr },
            { .name="grcde", .val=cli_unimac_mib_grcde, .parms=set_grcde },
            { .name="grfcr", .val=cli_unimac_mib_grfcr, .parms=set_grfcr },
            { .name="grovr", .val=cli_unimac_mib_grovr, .parms=set_grovr },
            { .name="grjbr", .val=cli_unimac_mib_grjbr, .parms=set_grjbr },
            { .name="grmtue", .val=cli_unimac_mib_grmtue, .parms=set_grmtue },
            { .name="grpok", .val=cli_unimac_mib_grpok, .parms=set_grpok },
            { .name="gruc", .val=cli_unimac_mib_gruc, .parms=set_gruc },
            { .name="grppp", .val=cli_unimac_mib_grppp, .parms=set_grppp },
            { .name="grcrc", .val=cli_unimac_mib_grcrc, .parms=set_grcrc },
            { .name="rrpkt", .val=cli_unimac_mib_rrpkt, .parms=set_rrpkt },
            { .name="rrund", .val=cli_unimac_mib_rrund, .parms=set_rrund },
            { .name="rrfrg", .val=cli_unimac_mib_rrfrg, .parms=set_rrfrg },
            { .name="rrbyt", .val=cli_unimac_mib_rrbyt, .parms=set_rrbyt },
            { .name="tr64", .val=cli_unimac_mib_tr64, .parms=set_tr64 },
            { .name="tr127", .val=cli_unimac_mib_tr127, .parms=set_tr127 },
            { .name="tr255", .val=cli_unimac_mib_tr255, .parms=set_tr255 },
            { .name="tr511", .val=cli_unimac_mib_tr511, .parms=set_tr511 },
            { .name="tr1023", .val=cli_unimac_mib_tr1023, .parms=set_tr1023 },
            { .name="tr1518", .val=cli_unimac_mib_tr1518, .parms=set_tr1518 },
            { .name="trmgv", .val=cli_unimac_mib_trmgv, .parms=set_trmgv },
            { .name="tr2047", .val=cli_unimac_mib_tr2047, .parms=set_tr2047 },
            { .name="tr4095", .val=cli_unimac_mib_tr4095, .parms=set_tr4095 },
            { .name="tr9216", .val=cli_unimac_mib_tr9216, .parms=set_tr9216 },
            { .name="gtpkt", .val=cli_unimac_mib_gtpkt, .parms=set_gtpkt },
            { .name="gtmca", .val=cli_unimac_mib_gtmca, .parms=set_gtmca },
            { .name="gtbca", .val=cli_unimac_mib_gtbca, .parms=set_gtbca },
            { .name="gtxpf", .val=cli_unimac_mib_gtxpf, .parms=set_gtxpf },
            { .name="gtxcf", .val=cli_unimac_mib_gtxcf, .parms=set_gtxcf },
            { .name="gtfcs", .val=cli_unimac_mib_gtfcs, .parms=set_gtfcs },
            { .name="gtovr", .val=cli_unimac_mib_gtovr, .parms=set_gtovr },
            { .name="gtdrf", .val=cli_unimac_mib_gtdrf, .parms=set_gtdrf },
            { .name="gtedf", .val=cli_unimac_mib_gtedf, .parms=set_gtedf },
            { .name="gtscl", .val=cli_unimac_mib_gtscl, .parms=set_gtscl },
            { .name="gtmcl", .val=cli_unimac_mib_gtmcl, .parms=set_gtmcl },
            { .name="gtlcl", .val=cli_unimac_mib_gtlcl, .parms=set_gtlcl },
            { .name="gtxcl", .val=cli_unimac_mib_gtxcl, .parms=set_gtxcl },
            { .name="gtfrg", .val=cli_unimac_mib_gtfrg, .parms=set_gtfrg },
            { .name="gtncl", .val=cli_unimac_mib_gtncl, .parms=set_gtncl },
            { .name="gtjbr", .val=cli_unimac_mib_gtjbr, .parms=set_gtjbr },
            { .name="gtbyt", .val=cli_unimac_mib_gtbyt, .parms=set_gtbyt },
            { .name="gtpok", .val=cli_unimac_mib_gtpok, .parms=set_gtpok },
            { .name="gtuc", .val=cli_unimac_mib_gtuc, .parms=set_gtuc },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_unimac_mib_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="mib_cntrl", .val=cli_unimac_mib_mib_cntrl, .parms=set_default },
            { .name="gr64", .val=cli_unimac_mib_gr64, .parms=set_default },
            { .name="gr127", .val=cli_unimac_mib_gr127, .parms=set_default },
            { .name="gr255", .val=cli_unimac_mib_gr255, .parms=set_default },
            { .name="gr511", .val=cli_unimac_mib_gr511, .parms=set_default },
            { .name="gr1023", .val=cli_unimac_mib_gr1023, .parms=set_default },
            { .name="gr1518", .val=cli_unimac_mib_gr1518, .parms=set_default },
            { .name="grmgv", .val=cli_unimac_mib_grmgv, .parms=set_default },
            { .name="gr2047", .val=cli_unimac_mib_gr2047, .parms=set_default },
            { .name="gr4095", .val=cli_unimac_mib_gr4095, .parms=set_default },
            { .name="gr9216", .val=cli_unimac_mib_gr9216, .parms=set_default },
            { .name="grpkt", .val=cli_unimac_mib_grpkt, .parms=set_default },
            { .name="grbyt", .val=cli_unimac_mib_grbyt, .parms=set_default },
            { .name="grmca", .val=cli_unimac_mib_grmca, .parms=set_default },
            { .name="grbca", .val=cli_unimac_mib_grbca, .parms=set_default },
            { .name="grfcs", .val=cli_unimac_mib_grfcs, .parms=set_default },
            { .name="grxcf", .val=cli_unimac_mib_grxcf, .parms=set_default },
            { .name="grxpf", .val=cli_unimac_mib_grxpf, .parms=set_default },
            { .name="grxuo", .val=cli_unimac_mib_grxuo, .parms=set_default },
            { .name="graln", .val=cli_unimac_mib_graln, .parms=set_default },
            { .name="grflr", .val=cli_unimac_mib_grflr, .parms=set_default },
            { .name="grcde", .val=cli_unimac_mib_grcde, .parms=set_default },
            { .name="grfcr", .val=cli_unimac_mib_grfcr, .parms=set_default },
            { .name="grovr", .val=cli_unimac_mib_grovr, .parms=set_default },
            { .name="grjbr", .val=cli_unimac_mib_grjbr, .parms=set_default },
            { .name="grmtue", .val=cli_unimac_mib_grmtue, .parms=set_default },
            { .name="grpok", .val=cli_unimac_mib_grpok, .parms=set_default },
            { .name="gruc", .val=cli_unimac_mib_gruc, .parms=set_default },
            { .name="grppp", .val=cli_unimac_mib_grppp, .parms=set_default },
            { .name="grcrc", .val=cli_unimac_mib_grcrc, .parms=set_default },
            { .name="rrpkt", .val=cli_unimac_mib_rrpkt, .parms=set_default },
            { .name="rrund", .val=cli_unimac_mib_rrund, .parms=set_default },
            { .name="rrfrg", .val=cli_unimac_mib_rrfrg, .parms=set_default },
            { .name="rrbyt", .val=cli_unimac_mib_rrbyt, .parms=set_default },
            { .name="tr64", .val=cli_unimac_mib_tr64, .parms=set_default },
            { .name="tr127", .val=cli_unimac_mib_tr127, .parms=set_default },
            { .name="tr255", .val=cli_unimac_mib_tr255, .parms=set_default },
            { .name="tr511", .val=cli_unimac_mib_tr511, .parms=set_default },
            { .name="tr1023", .val=cli_unimac_mib_tr1023, .parms=set_default },
            { .name="tr1518", .val=cli_unimac_mib_tr1518, .parms=set_default },
            { .name="trmgv", .val=cli_unimac_mib_trmgv, .parms=set_default },
            { .name="tr2047", .val=cli_unimac_mib_tr2047, .parms=set_default },
            { .name="tr4095", .val=cli_unimac_mib_tr4095, .parms=set_default },
            { .name="tr9216", .val=cli_unimac_mib_tr9216, .parms=set_default },
            { .name="gtpkt", .val=cli_unimac_mib_gtpkt, .parms=set_default },
            { .name="gtmca", .val=cli_unimac_mib_gtmca, .parms=set_default },
            { .name="gtbca", .val=cli_unimac_mib_gtbca, .parms=set_default },
            { .name="gtxpf", .val=cli_unimac_mib_gtxpf, .parms=set_default },
            { .name="gtxcf", .val=cli_unimac_mib_gtxcf, .parms=set_default },
            { .name="gtfcs", .val=cli_unimac_mib_gtfcs, .parms=set_default },
            { .name="gtovr", .val=cli_unimac_mib_gtovr, .parms=set_default },
            { .name="gtdrf", .val=cli_unimac_mib_gtdrf, .parms=set_default },
            { .name="gtedf", .val=cli_unimac_mib_gtedf, .parms=set_default },
            { .name="gtscl", .val=cli_unimac_mib_gtscl, .parms=set_default },
            { .name="gtmcl", .val=cli_unimac_mib_gtmcl, .parms=set_default },
            { .name="gtlcl", .val=cli_unimac_mib_gtlcl, .parms=set_default },
            { .name="gtxcl", .val=cli_unimac_mib_gtxcl, .parms=set_default },
            { .name="gtfrg", .val=cli_unimac_mib_gtfrg, .parms=set_default },
            { .name="gtncl", .val=cli_unimac_mib_gtncl, .parms=set_default },
            { .name="gtjbr", .val=cli_unimac_mib_gtjbr, .parms=set_default },
            { .name="gtbyt", .val=cli_unimac_mib_gtbyt, .parms=set_default },
            { .name="gtpok", .val=cli_unimac_mib_gtpok, .parms=set_default },
            { .name="gtuc", .val=cli_unimac_mib_gtuc, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_unimac_mib_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_unimac_mib_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("umac_mib_id", "umac_mib_id", umac_mib_id_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="MIB_CNTRL" , .val=bdmf_address_mib_cntrl },
            { .name="GR64" , .val=bdmf_address_gr64 },
            { .name="GR127" , .val=bdmf_address_gr127 },
            { .name="GR255" , .val=bdmf_address_gr255 },
            { .name="GR511" , .val=bdmf_address_gr511 },
            { .name="GR1023" , .val=bdmf_address_gr1023 },
            { .name="GR1518" , .val=bdmf_address_gr1518 },
            { .name="GRMGV" , .val=bdmf_address_grmgv },
            { .name="GR2047" , .val=bdmf_address_gr2047 },
            { .name="GR4095" , .val=bdmf_address_gr4095 },
            { .name="GR9216" , .val=bdmf_address_gr9216 },
            { .name="GRPKT" , .val=bdmf_address_grpkt },
            { .name="GRBYT" , .val=bdmf_address_grbyt },
            { .name="GRMCA" , .val=bdmf_address_grmca },
            { .name="GRBCA" , .val=bdmf_address_grbca },
            { .name="GRFCS" , .val=bdmf_address_grfcs },
            { .name="GRXCF" , .val=bdmf_address_grxcf },
            { .name="GRXPF" , .val=bdmf_address_grxpf },
            { .name="GRXUO" , .val=bdmf_address_grxuo },
            { .name="GRALN" , .val=bdmf_address_graln },
            { .name="GRFLR" , .val=bdmf_address_grflr },
            { .name="GRCDE" , .val=bdmf_address_grcde },
            { .name="GRFCR" , .val=bdmf_address_grfcr },
            { .name="GROVR" , .val=bdmf_address_grovr },
            { .name="GRJBR" , .val=bdmf_address_grjbr },
            { .name="GRMTUE" , .val=bdmf_address_grmtue },
            { .name="GRPOK" , .val=bdmf_address_grpok },
            { .name="GRUC" , .val=bdmf_address_gruc },
            { .name="GRPPP" , .val=bdmf_address_grppp },
            { .name="GRCRC" , .val=bdmf_address_grcrc },
            { .name="RRPKT" , .val=bdmf_address_rrpkt },
            { .name="RRUND" , .val=bdmf_address_rrund },
            { .name="RRFRG" , .val=bdmf_address_rrfrg },
            { .name="RRBYT" , .val=bdmf_address_rrbyt },
            { .name="TR64" , .val=bdmf_address_tr64 },
            { .name="TR127" , .val=bdmf_address_tr127 },
            { .name="TR255" , .val=bdmf_address_tr255 },
            { .name="TR511" , .val=bdmf_address_tr511 },
            { .name="TR1023" , .val=bdmf_address_tr1023 },
            { .name="TR1518" , .val=bdmf_address_tr1518 },
            { .name="TRMGV" , .val=bdmf_address_trmgv },
            { .name="TR2047" , .val=bdmf_address_tr2047 },
            { .name="TR4095" , .val=bdmf_address_tr4095 },
            { .name="TR9216" , .val=bdmf_address_tr9216 },
            { .name="GTPKT" , .val=bdmf_address_gtpkt },
            { .name="GTMCA" , .val=bdmf_address_gtmca },
            { .name="GTBCA" , .val=bdmf_address_gtbca },
            { .name="GTXPF" , .val=bdmf_address_gtxpf },
            { .name="GTXCF" , .val=bdmf_address_gtxcf },
            { .name="GTFCS" , .val=bdmf_address_gtfcs },
            { .name="GTOVR" , .val=bdmf_address_gtovr },
            { .name="GTDRF" , .val=bdmf_address_gtdrf },
            { .name="GTEDF" , .val=bdmf_address_gtedf },
            { .name="GTSCL" , .val=bdmf_address_gtscl },
            { .name="GTMCL" , .val=bdmf_address_gtmcl },
            { .name="GTLCL" , .val=bdmf_address_gtlcl },
            { .name="GTXCL" , .val=bdmf_address_gtxcl },
            { .name="GTFRG" , .val=bdmf_address_gtfrg },
            { .name="GTNCL" , .val=bdmf_address_gtncl },
            { .name="GTJBR" , .val=bdmf_address_gtjbr },
            { .name="GTBYT" , .val=bdmf_address_gtbyt },
            { .name="GTPOK" , .val=bdmf_address_gtpok },
            { .name="GTUC" , .val=bdmf_address_gtuc },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_unimac_mib_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "umac_mib_id", umac_mib_id_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

