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
#include "xrdp_drv_crdp_ds_cmn_bbh_tx_ag.h"

#define BLOCK_ADDR_COUNT_BITS 2
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_mac_type_set(uint8_t bbh_id, uint8_t zero, uint8_t type)
{
    uint32_t reg_common_configurations_mactype = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 8) ||
       (type >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MACTYPE, reg_common_configurations_mactype);

    reg_common_configurations_mactype = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MACTYPE, TYPE, reg_common_configurations_mactype, type);

    RU_REG_RAM_WRITE(bbh_id, zero, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MACTYPE, reg_common_configurations_mactype);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_mac_type_get(uint8_t bbh_id, uint8_t zero, uint8_t *type)
{
    uint32_t reg_common_configurations_mactype;

#ifdef VALIDATE_PARMS
    if (!type)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MACTYPE, reg_common_configurations_mactype);

    *type = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MACTYPE, TYPE, reg_common_configurations_mactype);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ddrbyte_get(uint8_t bbh_id, uint32_t *ddrbyte)
{
    uint32_t reg_debug_counters_txddrbyte;

#ifdef VALIDATE_PARMS
    if (!ddrbyte)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXDDRBYTE, reg_debug_counters_txddrbyte);

    *ddrbyte = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXDDRBYTE, DDRBYTE, reg_debug_counters_txddrbyte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram0byte_get(uint8_t bbh_id, uint32_t *srambyte)
{
    uint32_t reg_debug_counters_txsram0byte;

#ifdef VALIDATE_PARMS
    if (!srambyte)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXSRAM0BYTE, reg_debug_counters_txsram0byte);

    *srambyte = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXSRAM0BYTE, SRAMBYTE, reg_debug_counters_txsram0byte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram1byte_get(uint8_t bbh_id, uint32_t *srambyte)
{
    uint32_t reg_debug_counters_txsram1byte;

#ifdef VALIDATE_PARMS
    if (!srambyte)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXSRAM1BYTE, reg_debug_counters_txsram1byte);

    *srambyte = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXSRAM1BYTE, SRAMBYTE, reg_debug_counters_txsram1byte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbcfg_tx_set(uint8_t bbh_id, uint16_t sbpmsrc, uint16_t fpmsrc)
{
    uint32_t reg_common_configurations_bbcfg_tx = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (sbpmsrc >= _9BITS_MAX_VAL_) ||
       (fpmsrc >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_bbcfg_tx = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBCFG_TX, SBPMSRC, reg_common_configurations_bbcfg_tx, sbpmsrc);
    reg_common_configurations_bbcfg_tx = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBCFG_TX, FPMSRC, reg_common_configurations_bbcfg_tx, fpmsrc);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBCFG_TX, reg_common_configurations_bbcfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbcfg_tx_get(uint8_t bbh_id, uint16_t *sbpmsrc, uint16_t *fpmsrc)
{
    uint32_t reg_common_configurations_bbcfg_tx;

#ifdef VALIDATE_PARMS
    if (!sbpmsrc || !fpmsrc)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBCFG_TX, reg_common_configurations_bbcfg_tx);

    *sbpmsrc = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBCFG_TX, SBPMSRC, reg_common_configurations_bbcfg_tx);
    *fpmsrc = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBCFG_TX, FPMSRC, reg_common_configurations_bbcfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg_set(uint8_t bbh_id, const crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg *common_configurations_sbpmcfg)
{
    uint32_t reg_common_configurations_sbpmcfg = 0;

#ifdef VALIDATE_PARMS
    if(!common_configurations_sbpmcfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (common_configurations_sbpmcfg->freenocntxt >= _1BITS_MAX_VAL_) ||
       (common_configurations_sbpmcfg->mc_type_3 >= _1BITS_MAX_VAL_) ||
       (common_configurations_sbpmcfg->mc_type_2 >= _1BITS_MAX_VAL_) ||
       (common_configurations_sbpmcfg->second_bn_dis >= _1BITS_MAX_VAL_) ||
       (common_configurations_sbpmcfg->bufsize >= _1BITS_MAX_VAL_) ||
       (common_configurations_sbpmcfg->enrrspread >= _1BITS_MAX_VAL_) ||
       (common_configurations_sbpmcfg->maxgn >= _5BITS_MAX_VAL_) ||
       (common_configurations_sbpmcfg->listfreetimer >= _10BITS_MAX_VAL_) ||
       (common_configurations_sbpmcfg->listfreethr >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_sbpmcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, FREENOCNTXT, reg_common_configurations_sbpmcfg, common_configurations_sbpmcfg->freenocntxt);
    reg_common_configurations_sbpmcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, MC_TYPE_3, reg_common_configurations_sbpmcfg, common_configurations_sbpmcfg->mc_type_3);
    reg_common_configurations_sbpmcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, MC_TYPE_2, reg_common_configurations_sbpmcfg, common_configurations_sbpmcfg->mc_type_2);
    reg_common_configurations_sbpmcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, SECOND_BN_DIS, reg_common_configurations_sbpmcfg, common_configurations_sbpmcfg->second_bn_dis);
    reg_common_configurations_sbpmcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, BUFSIZE, reg_common_configurations_sbpmcfg, common_configurations_sbpmcfg->bufsize);
    reg_common_configurations_sbpmcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, ENRRSPREAD, reg_common_configurations_sbpmcfg, common_configurations_sbpmcfg->enrrspread);
    reg_common_configurations_sbpmcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, MAXGN, reg_common_configurations_sbpmcfg, common_configurations_sbpmcfg->maxgn);
    reg_common_configurations_sbpmcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, LISTFREETIMER, reg_common_configurations_sbpmcfg, common_configurations_sbpmcfg->listfreetimer);
    reg_common_configurations_sbpmcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, LISTFREETHR, reg_common_configurations_sbpmcfg, common_configurations_sbpmcfg->listfreethr);
    reg_common_configurations_sbpmcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, SMID, reg_common_configurations_sbpmcfg, common_configurations_sbpmcfg->smid);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, reg_common_configurations_sbpmcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg_get(uint8_t bbh_id, crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg *common_configurations_sbpmcfg)
{
    uint32_t reg_common_configurations_sbpmcfg;

#ifdef VALIDATE_PARMS
    if (!common_configurations_sbpmcfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, reg_common_configurations_sbpmcfg);

    common_configurations_sbpmcfg->freenocntxt = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, FREENOCNTXT, reg_common_configurations_sbpmcfg);
    common_configurations_sbpmcfg->mc_type_3 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, MC_TYPE_3, reg_common_configurations_sbpmcfg);
    common_configurations_sbpmcfg->mc_type_2 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, MC_TYPE_2, reg_common_configurations_sbpmcfg);
    common_configurations_sbpmcfg->second_bn_dis = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, SECOND_BN_DIS, reg_common_configurations_sbpmcfg);
    common_configurations_sbpmcfg->bufsize = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, BUFSIZE, reg_common_configurations_sbpmcfg);
    common_configurations_sbpmcfg->enrrspread = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, ENRRSPREAD, reg_common_configurations_sbpmcfg);
    common_configurations_sbpmcfg->maxgn = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, MAXGN, reg_common_configurations_sbpmcfg);
    common_configurations_sbpmcfg->listfreetimer = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, LISTFREETIMER, reg_common_configurations_sbpmcfg);
    common_configurations_sbpmcfg->listfreethr = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, LISTFREETHR, reg_common_configurations_sbpmcfg);
    common_configurations_sbpmcfg->smid = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, SMID, reg_common_configurations_sbpmcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbsrcid_set(uint8_t bbh_id, uint16_t bbsrcid, bdmf_boolean byp)
{
    uint32_t reg_common_configurations_bbsrcid = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (bbsrcid >= _9BITS_MAX_VAL_) ||
       (byp >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_bbsrcid = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBSRCID, BBSRCID, reg_common_configurations_bbsrcid, bbsrcid);
    reg_common_configurations_bbsrcid = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBSRCID, BYP, reg_common_configurations_bbsrcid, byp);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBSRCID, reg_common_configurations_bbsrcid);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbsrcid_get(uint8_t bbh_id, uint16_t *bbsrcid, bdmf_boolean *byp)
{
    uint32_t reg_common_configurations_bbsrcid;

#ifdef VALIDATE_PARMS
    if (!bbsrcid || !byp)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBSRCID, reg_common_configurations_bbsrcid);

    *bbsrcid = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBSRCID, BBSRCID, reg_common_configurations_bbsrcid);
    *byp = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBSRCID, BYP, reg_common_configurations_bbsrcid);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrcfg_tx_set(uint8_t bbh_id, uint8_t bufsize0, uint8_t bufsize1, uint8_t hnsize)
{
    uint32_t reg_common_configurations_ddrcfg_tx = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (bufsize0 >= _3BITS_MAX_VAL_) ||
       (bufsize1 >= _3BITS_MAX_VAL_) ||
       (hnsize >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_ddrcfg_tx = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, BUFSIZE0, reg_common_configurations_ddrcfg_tx, bufsize0);
    reg_common_configurations_ddrcfg_tx = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, BUFSIZE1, reg_common_configurations_ddrcfg_tx, bufsize1);
    reg_common_configurations_ddrcfg_tx = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, HNSIZE, reg_common_configurations_ddrcfg_tx, hnsize);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, reg_common_configurations_ddrcfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrcfg_tx_get(uint8_t bbh_id, uint8_t *bufsize0, uint8_t *bufsize1, uint8_t *hnsize)
{
    uint32_t reg_common_configurations_ddrcfg_tx;

#ifdef VALIDATE_PARMS
    if (!bufsize0 || !bufsize1 || !hnsize)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, reg_common_configurations_ddrcfg_tx);

    *bufsize0 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, BUFSIZE0, reg_common_configurations_ddrcfg_tx);
    *bufsize1 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, BUFSIZE1, reg_common_configurations_ddrcfg_tx);
    *hnsize = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, HNSIZE, reg_common_configurations_ddrcfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg_set(uint8_t bbh_id, uint8_t pd_rnr_cfg_index, uint16_t wkpaddr, uint8_t task, uint16_t bbsrcid)
{
    uint32_t reg_common_configurations_pdrnrcfg = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (pd_rnr_cfg_index >= 8) ||
       (task >= _4BITS_MAX_VAL_) ||
       (bbsrcid >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_pdrnrcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_PDRNRCFG, WKPADDR, reg_common_configurations_pdrnrcfg, wkpaddr);
    reg_common_configurations_pdrnrcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_PDRNRCFG, TASK, reg_common_configurations_pdrnrcfg, task);
    reg_common_configurations_pdrnrcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_PDRNRCFG, BBSRCID, reg_common_configurations_pdrnrcfg, bbsrcid);

    RU_REG_RAM_WRITE(bbh_id, pd_rnr_cfg_index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_PDRNRCFG, reg_common_configurations_pdrnrcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg_get(uint8_t bbh_id, uint8_t pd_rnr_cfg_index, uint16_t *wkpaddr, uint8_t *task, uint16_t *bbsrcid)
{
    uint32_t reg_common_configurations_pdrnrcfg;

#ifdef VALIDATE_PARMS
    if (!wkpaddr || !task || !bbsrcid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (pd_rnr_cfg_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, pd_rnr_cfg_index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_PDRNRCFG, reg_common_configurations_pdrnrcfg);

    *wkpaddr = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_PDRNRCFG, WKPADDR, reg_common_configurations_pdrnrcfg);
    *task = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_PDRNRCFG, TASK, reg_common_configurations_pdrnrcfg);
    *bbsrcid = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_PDRNRCFG, BBSRCID, reg_common_configurations_pdrnrcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd_set(uint8_t bbh_id, const crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd *common_configurations_txrstcmd)
{
    uint32_t reg_common_configurations_txrstcmd = 0;

#ifdef VALIDATE_PARMS
    if(!common_configurations_txrstcmd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (common_configurations_txrstcmd->cntxtrst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->pdfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->bpmfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->sbpmfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->ddrfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->sramfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->skbptrrst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->gnxtfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->fbnfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->port >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, CNTXTRST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->cntxtrst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, PDFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->pdfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, BPMFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->bpmfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, SBPMFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->sbpmfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, DDRFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->ddrfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, SRAMFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->sramfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, SKBPTRRST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->skbptrrst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, GNXTFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->gnxtfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, FBNFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->fbnfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, PORT, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->port);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, reg_common_configurations_txrstcmd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd_get(uint8_t bbh_id, crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd *common_configurations_txrstcmd)
{
    uint32_t reg_common_configurations_txrstcmd;

#ifdef VALIDATE_PARMS
    if (!common_configurations_txrstcmd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, reg_common_configurations_txrstcmd);

    common_configurations_txrstcmd->cntxtrst = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, CNTXTRST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->pdfiforst = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, PDFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->bpmfiforst = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, BPMFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->sbpmfiforst = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, SBPMFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->ddrfiforst = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, DDRFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->sramfiforst = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, SRAMFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->skbptrrst = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, SKBPTRRST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->gnxtfiforst = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, GNXTFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->fbnfiforst = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, FBNFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->port = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, PORT, reg_common_configurations_txrstcmd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_dbgsel_set(uint8_t bbh_id, uint8_t dbgsel)
{
    uint32_t reg_common_configurations_dbgsel = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (dbgsel >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_dbgsel = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DBGSEL, DBGSEL, reg_common_configurations_dbgsel, dbgsel);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DBGSEL, reg_common_configurations_dbgsel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_dbgsel_get(uint8_t bbh_id, uint8_t *dbgsel)
{
    uint32_t reg_common_configurations_dbgsel;

#ifdef VALIDATE_PARMS
    if (!dbgsel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DBGSEL, reg_common_configurations_dbgsel);

    *dbgsel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DBGSEL, DBGSEL, reg_common_configurations_dbgsel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl_set(uint8_t bbh_id, const crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl *common_configurations_clk_gate_cntrl)
{
    uint32_t reg_common_configurations_clk_gate_cntrl = 0;

#ifdef VALIDATE_PARMS
    if(!common_configurations_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (common_configurations_clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
       (common_configurations_clk_gate_cntrl->keep_alive_en >= _1BITS_MAX_VAL_) ||
       (common_configurations_clk_gate_cntrl->keep_alive_intrvl >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_clk_gate_cntrl = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_common_configurations_clk_gate_cntrl, common_configurations_clk_gate_cntrl->bypass_clk_gate);
    reg_common_configurations_clk_gate_cntrl = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, TIMER_VAL, reg_common_configurations_clk_gate_cntrl, common_configurations_clk_gate_cntrl->timer_val);
    reg_common_configurations_clk_gate_cntrl = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_common_configurations_clk_gate_cntrl, common_configurations_clk_gate_cntrl->keep_alive_en);
    reg_common_configurations_clk_gate_cntrl = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_common_configurations_clk_gate_cntrl, common_configurations_clk_gate_cntrl->keep_alive_intrvl);
    reg_common_configurations_clk_gate_cntrl = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_common_configurations_clk_gate_cntrl, common_configurations_clk_gate_cntrl->keep_alive_cyc);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, reg_common_configurations_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl_get(uint8_t bbh_id, crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl *common_configurations_clk_gate_cntrl)
{
    uint32_t reg_common_configurations_clk_gate_cntrl;

#ifdef VALIDATE_PARMS
    if (!common_configurations_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, reg_common_configurations_clk_gate_cntrl);

    common_configurations_clk_gate_cntrl->bypass_clk_gate = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_common_configurations_clk_gate_cntrl);
    common_configurations_clk_gate_cntrl->timer_val = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, TIMER_VAL, reg_common_configurations_clk_gate_cntrl);
    common_configurations_clk_gate_cntrl->keep_alive_en = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_common_configurations_clk_gate_cntrl);
    common_configurations_clk_gate_cntrl->keep_alive_intrvl = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_common_configurations_clk_gate_cntrl);
    common_configurations_clk_gate_cntrl->keep_alive_cyc = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_common_configurations_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_gpr_set(uint8_t bbh_id, uint32_t gpr)
{
    uint32_t reg_common_configurations_gpr = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_gpr = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GPR, GPR, reg_common_configurations_gpr, gpr);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GPR, reg_common_configurations_gpr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_gpr_get(uint8_t bbh_id, uint32_t *gpr)
{
    uint32_t reg_common_configurations_gpr;

#ifdef VALIDATE_PARMS
    if (!gpr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GPR, reg_common_configurations_gpr);

    *gpr = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GPR, GPR, reg_common_configurations_gpr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_general_cfg_set(uint8_t bbh_id, const crdp_ds_cmn_bbh_tx_common_configurations_general_cfg *common_configurations_general_cfg)
{
    uint32_t reg_common_configurations_general_cfg = 0;

#ifdef VALIDATE_PARMS
    if(!common_configurations_general_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (common_configurations_general_cfg->macdrop >= _1BITS_MAX_VAL_) ||
       (common_configurations_general_cfg->discopycrdt >= _1BITS_MAX_VAL_) ||
       (common_configurations_general_cfg->ecnrmrk >= _1BITS_MAX_VAL_) ||
       (common_configurations_general_cfg->cntraddcrc >= _1BITS_MAX_VAL_) ||
       (common_configurations_general_cfg->crcforward >= _1BITS_MAX_VAL_) ||
       (common_configurations_general_cfg->multi_tcont >= _1BITS_MAX_VAL_) ||
       (common_configurations_general_cfg->distargetperport >= _1BITS_MAX_VAL_) ||
       (common_configurations_general_cfg->discrdtchk >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_general_cfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, MACDROP, reg_common_configurations_general_cfg, common_configurations_general_cfg->macdrop);
    reg_common_configurations_general_cfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, DISCOPYCRDT, reg_common_configurations_general_cfg, common_configurations_general_cfg->discopycrdt);
    reg_common_configurations_general_cfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, ECNRMRK, reg_common_configurations_general_cfg, common_configurations_general_cfg->ecnrmrk);
    reg_common_configurations_general_cfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, CNTRADDCRC, reg_common_configurations_general_cfg, common_configurations_general_cfg->cntraddcrc);
    reg_common_configurations_general_cfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, CRCFORWARD, reg_common_configurations_general_cfg, common_configurations_general_cfg->crcforward);
    reg_common_configurations_general_cfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, MULTI_TCONT, reg_common_configurations_general_cfg, common_configurations_general_cfg->multi_tcont);
    reg_common_configurations_general_cfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, DISTARGETPERPORT, reg_common_configurations_general_cfg, common_configurations_general_cfg->distargetperport);
    reg_common_configurations_general_cfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, DISCRDTCHK, reg_common_configurations_general_cfg, common_configurations_general_cfg->discrdtchk);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, reg_common_configurations_general_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_general_cfg_get(uint8_t bbh_id, crdp_ds_cmn_bbh_tx_common_configurations_general_cfg *common_configurations_general_cfg)
{
    uint32_t reg_common_configurations_general_cfg;

#ifdef VALIDATE_PARMS
    if (!common_configurations_general_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, reg_common_configurations_general_cfg);

    common_configurations_general_cfg->macdrop = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, MACDROP, reg_common_configurations_general_cfg);
    common_configurations_general_cfg->discopycrdt = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, DISCOPYCRDT, reg_common_configurations_general_cfg);
    common_configurations_general_cfg->ecnrmrk = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, ECNRMRK, reg_common_configurations_general_cfg);
    common_configurations_general_cfg->cntraddcrc = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, CNTRADDCRC, reg_common_configurations_general_cfg);
    common_configurations_general_cfg->crcforward = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, CRCFORWARD, reg_common_configurations_general_cfg);
    common_configurations_general_cfg->multi_tcont = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, MULTI_TCONT, reg_common_configurations_general_cfg);
    common_configurations_general_cfg->distargetperport = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, DISTARGETPERPORT, reg_common_configurations_general_cfg);
    common_configurations_general_cfg->discrdtchk = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG, DISCRDTCHK, reg_common_configurations_general_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ecncfg_set(uint8_t bbh_id, const crdp_ds_cmn_bbh_tx_common_configurations_ecncfg *common_configurations_ecncfg)
{
    uint32_t reg_common_configurations_ecncfg = 0;

#ifdef VALIDATE_PARMS
    if(!common_configurations_ecncfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (common_configurations_ecncfg->ecnbyteipv4 >= _5BITS_MAX_VAL_) ||
       (common_configurations_ecncfg->ecnbitipv4 >= _3BITS_MAX_VAL_) ||
       (common_configurations_ecncfg->ecnbyteipv6 >= _5BITS_MAX_VAL_) ||
       (common_configurations_ecncfg->ecnbitipv6 >= _3BITS_MAX_VAL_) ||
       (common_configurations_ecncfg->chksumbyteipv4 >= _6BITS_MAX_VAL_) ||
       (common_configurations_ecncfg->assignedqcnt >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_ecncfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ECNCFG, ECNBYTEIPV4, reg_common_configurations_ecncfg, common_configurations_ecncfg->ecnbyteipv4);
    reg_common_configurations_ecncfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ECNCFG, ECNBITIPV4, reg_common_configurations_ecncfg, common_configurations_ecncfg->ecnbitipv4);
    reg_common_configurations_ecncfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ECNCFG, ECNBYTEIPV6, reg_common_configurations_ecncfg, common_configurations_ecncfg->ecnbyteipv6);
    reg_common_configurations_ecncfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ECNCFG, ECNBITIPV6, reg_common_configurations_ecncfg, common_configurations_ecncfg->ecnbitipv6);
    reg_common_configurations_ecncfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ECNCFG, CHKSUMBYTEIPV4, reg_common_configurations_ecncfg, common_configurations_ecncfg->chksumbyteipv4);
    reg_common_configurations_ecncfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ECNCFG, ASSIGNEDQCNT, reg_common_configurations_ecncfg, common_configurations_ecncfg->assignedqcnt);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ECNCFG, reg_common_configurations_ecncfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ecncfg_get(uint8_t bbh_id, crdp_ds_cmn_bbh_tx_common_configurations_ecncfg *common_configurations_ecncfg)
{
    uint32_t reg_common_configurations_ecncfg;

#ifdef VALIDATE_PARMS
    if (!common_configurations_ecncfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ECNCFG, reg_common_configurations_ecncfg);

    common_configurations_ecncfg->ecnbyteipv4 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ECNCFG, ECNBYTEIPV4, reg_common_configurations_ecncfg);
    common_configurations_ecncfg->ecnbitipv4 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ECNCFG, ECNBITIPV4, reg_common_configurations_ecncfg);
    common_configurations_ecncfg->ecnbyteipv6 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ECNCFG, ECNBYTEIPV6, reg_common_configurations_ecncfg);
    common_configurations_ecncfg->ecnbitipv6 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ECNCFG, ECNBITIPV6, reg_common_configurations_ecncfg);
    common_configurations_ecncfg->chksumbyteipv4 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ECNCFG, CHKSUMBYTEIPV4, reg_common_configurations_ecncfg);
    common_configurations_ecncfg->assignedqcnt = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ECNCFG, ASSIGNEDQCNT, reg_common_configurations_ecncfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel_set(uint8_t bbh_id, uint8_t ddr_fpmini_basel_index, uint32_t ddrfpminibase)
{
    uint32_t reg_common_configurations_ddrfpminibasel = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (ddr_fpmini_basel_index >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_ddrfpminibasel = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRFPMINIBASEL, DDRFPMINIBASE, reg_common_configurations_ddrfpminibasel, ddrfpminibase);

    RU_REG_RAM_WRITE(bbh_id, ddr_fpmini_basel_index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRFPMINIBASEL, reg_common_configurations_ddrfpminibasel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel_get(uint8_t bbh_id, uint8_t ddr_fpmini_basel_index, uint32_t *ddrfpminibase)
{
    uint32_t reg_common_configurations_ddrfpminibasel;

#ifdef VALIDATE_PARMS
    if (!ddrfpminibase)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (ddr_fpmini_basel_index >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, ddr_fpmini_basel_index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRFPMINIBASEL, reg_common_configurations_ddrfpminibasel);

    *ddrfpminibase = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRFPMINIBASEL, DDRFPMINIBASE, reg_common_configurations_ddrfpminibasel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh_set(uint8_t bbh_id, uint8_t ddr_fpmini_baseh_index, uint8_t ddrfpminibase)
{
    uint32_t reg_common_configurations_ddrfpminibaseh = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (ddr_fpmini_baseh_index >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_ddrfpminibaseh = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRFPMINIBASEH, DDRFPMINIBASE, reg_common_configurations_ddrfpminibaseh, ddrfpminibase);

    RU_REG_RAM_WRITE(bbh_id, ddr_fpmini_baseh_index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRFPMINIBASEH, reg_common_configurations_ddrfpminibaseh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh_get(uint8_t bbh_id, uint8_t ddr_fpmini_baseh_index, uint8_t *ddrfpminibase)
{
    uint32_t reg_common_configurations_ddrfpminibaseh;

#ifdef VALIDATE_PARMS
    if (!ddrfpminibase)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (ddr_fpmini_baseh_index >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, ddr_fpmini_baseh_index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRFPMINIBASEH, reg_common_configurations_ddrfpminibaseh);

    *ddrfpminibase = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRFPMINIBASEH, DDRFPMINIBASE, reg_common_configurations_ddrfpminibaseh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdroute_set(uint8_t bbh_id, uint32_t route)
{
    uint32_t reg_common_configurations_bbovrdroute = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (route >= _25BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_bbovrdroute = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBOVRDROUTE, ROUTE, reg_common_configurations_bbovrdroute, route);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBOVRDROUTE, reg_common_configurations_bbovrdroute);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdroute_get(uint8_t bbh_id, uint32_t *route)
{
    uint32_t reg_common_configurations_bbovrdroute;

#ifdef VALIDATE_PARMS
    if (!route)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBOVRDROUTE, reg_common_configurations_bbovrdroute);

    *route = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBOVRDROUTE, ROUTE, reg_common_configurations_bbovrdroute);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdctrl_set(uint8_t bbh_id, uint16_t dest, bdmf_boolean en)
{
    uint32_t reg_common_configurations_bbovrdctrl = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (dest >= _9BITS_MAX_VAL_) ||
       (en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_bbovrdctrl = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBOVRDCTRL, DEST, reg_common_configurations_bbovrdctrl, dest);
    reg_common_configurations_bbovrdctrl = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBOVRDCTRL, EN, reg_common_configurations_bbovrdctrl, en);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBOVRDCTRL, reg_common_configurations_bbovrdctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdctrl_get(uint8_t bbh_id, uint16_t *dest, bdmf_boolean *en)
{
    uint32_t reg_common_configurations_bbovrdctrl;

#ifdef VALIDATE_PARMS
    if (!dest || !en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBOVRDCTRL, reg_common_configurations_bbovrdctrl);

    *dest = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBOVRDCTRL, DEST, reg_common_configurations_bbovrdctrl);
    *en = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBOVRDCTRL, EN, reg_common_configurations_bbovrdctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel_set(uint8_t bbh_id, uint8_t tm_basel_index, uint32_t ddrtmbase)
{
    uint32_t reg_common_configurations_tmbasel = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (tm_basel_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_tmbasel = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TMBASEL, DDRTMBASE, reg_common_configurations_tmbasel, ddrtmbase);

    RU_REG_RAM_WRITE(bbh_id, tm_basel_index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TMBASEL, reg_common_configurations_tmbasel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel_get(uint8_t bbh_id, uint8_t tm_basel_index, uint32_t *ddrtmbase)
{
    uint32_t reg_common_configurations_tmbasel;

#ifdef VALIDATE_PARMS
    if (!ddrtmbase)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (tm_basel_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, tm_basel_index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TMBASEL, reg_common_configurations_tmbasel);

    *ddrtmbase = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TMBASEL, DDRTMBASE, reg_common_configurations_tmbasel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh_set(uint8_t bbh_id, uint8_t tm_baseh_index, uint8_t ddrtmbase)
{
    uint32_t reg_common_configurations_tmbaseh = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (tm_baseh_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_tmbaseh = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TMBASEH, DDRTMBASE, reg_common_configurations_tmbaseh, ddrtmbase);

    RU_REG_RAM_WRITE(bbh_id, tm_baseh_index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TMBASEH, reg_common_configurations_tmbaseh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh_get(uint8_t bbh_id, uint8_t tm_baseh_index, uint8_t *ddrtmbase)
{
    uint32_t reg_common_configurations_tmbaseh;

#ifdef VALIDATE_PARMS
    if (!ddrtmbase)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (tm_baseh_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, tm_baseh_index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TMBASEH, reg_common_configurations_tmbaseh);

    *ddrtmbase = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TMBASEH, DDRTMBASE, reg_common_configurations_tmbaseh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize_set(uint8_t bbh_id, uint8_t index, uint16_t size)
{
    uint32_t reg_common_configurations_reorderfifosize = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 24) ||
       (size >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_reorderfifosize = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_REORDERFIFOSIZE, SIZE, reg_common_configurations_reorderfifosize, size);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_REORDERFIFOSIZE, reg_common_configurations_reorderfifosize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize_get(uint8_t bbh_id, uint8_t index, uint16_t *size)
{
    uint32_t reg_common_configurations_reorderfifosize;

#ifdef VALIDATE_PARMS
    if (!size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 24))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_REORDERFIFOSIZE, reg_common_configurations_reorderfifosize);

    *size = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_REORDERFIFOSIZE, SIZE, reg_common_configurations_reorderfifosize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits_set(uint8_t bbh_id, uint8_t index, uint8_t credits)
{
    uint32_t reg_common_configurations_ubuscredits = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 24))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_ubuscredits = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_UBUSCREDITS, CREDITS, reg_common_configurations_ubuscredits, credits);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_UBUSCREDITS, reg_common_configurations_ubuscredits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits_get(uint8_t bbh_id, uint8_t index, uint8_t *credits)
{
    uint32_t reg_common_configurations_ubuscredits;

#ifdef VALIDATE_PARMS
    if (!credits)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 24))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_UBUSCREDITS, reg_common_configurations_ubuscredits);

    *credits = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_UBUSCREDITS, CREDITS, reg_common_configurations_ubuscredits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_creditinit_set(uint8_t bbh_id, uint32_t init)
{
    uint32_t reg_common_configurations_creditinit = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (init >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_creditinit = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CREDITINIT, INIT, reg_common_configurations_creditinit, init);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CREDITINIT, reg_common_configurations_creditinit);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_creditinit_get(uint8_t bbh_id, uint32_t *init)
{
    uint32_t reg_common_configurations_creditinit;

#ifdef VALIDATE_PARMS
    if (!init)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CREDITINIT, reg_common_configurations_creditinit);

    *init = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CREDITINIT, INIT, reg_common_configurations_creditinit);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg_set(uint8_t bbh_id, uint8_t index, uint16_t wkpaddr, uint8_t task, uint16_t bbsrcid)
{
    uint32_t reg_common_configurations_mirrorrnrcfg = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (task >= _4BITS_MAX_VAL_) ||
       (bbsrcid >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_mirrorrnrcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORRNRCFG, WKPADDR, reg_common_configurations_mirrorrnrcfg, wkpaddr);
    reg_common_configurations_mirrorrnrcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORRNRCFG, TASK, reg_common_configurations_mirrorrnrcfg, task);
    reg_common_configurations_mirrorrnrcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORRNRCFG, BBSRCID, reg_common_configurations_mirrorrnrcfg, bbsrcid);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORRNRCFG, reg_common_configurations_mirrorrnrcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg_get(uint8_t bbh_id, uint8_t index, uint16_t *wkpaddr, uint8_t *task, uint16_t *bbsrcid)
{
    uint32_t reg_common_configurations_mirrorrnrcfg;

#ifdef VALIDATE_PARMS
    if (!wkpaddr || !task || !bbsrcid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORRNRCFG, reg_common_configurations_mirrorrnrcfg);

    *wkpaddr = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORRNRCFG, WKPADDR, reg_common_configurations_mirrorrnrcfg);
    *task = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORRNRCFG, TASK, reg_common_configurations_mirrorrnrcfg);
    *bbsrcid = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORRNRCFG, BBSRCID, reg_common_configurations_mirrorrnrcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg_set(uint8_t bbh_id, uint8_t index, uint8_t wkpgran, bdmf_boolean ptrrst, bdmf_boolean enable, uint8_t timer)
{
    uint32_t reg_common_configurations_mirrorcfg = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (ptrrst >= _1BITS_MAX_VAL_) ||
       (enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_mirrorcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORCFG, WKPGRAN, reg_common_configurations_mirrorcfg, wkpgran);
    reg_common_configurations_mirrorcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORCFG, PTRRST, reg_common_configurations_mirrorcfg, ptrrst);
    reg_common_configurations_mirrorcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORCFG, ENABLE, reg_common_configurations_mirrorcfg, enable);
    reg_common_configurations_mirrorcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORCFG, TIMER, reg_common_configurations_mirrorcfg, timer);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORCFG, reg_common_configurations_mirrorcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg_get(uint8_t bbh_id, uint8_t index, uint8_t *wkpgran, bdmf_boolean *ptrrst, bdmf_boolean *enable, uint8_t *timer)
{
    uint32_t reg_common_configurations_mirrorcfg;

#ifdef VALIDATE_PARMS
    if (!wkpgran || !ptrrst || !enable || !timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORCFG, reg_common_configurations_mirrorcfg);

    *wkpgran = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORCFG, WKPGRAN, reg_common_configurations_mirrorcfg);
    *ptrrst = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORCFG, PTRRST, reg_common_configurations_mirrorcfg);
    *enable = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORCFG, ENABLE, reg_common_configurations_mirrorcfg);
    *timer = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORCFG, TIMER, reg_common_configurations_mirrorcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbhcfg_set(uint8_t bbh_id, uint16_t sninitval, bdmf_boolean sninit, uint8_t lbctrrst, uint8_t mode)
{
    uint32_t reg_common_configurations_multibbhcfg = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (sninit >= _1BITS_MAX_VAL_) ||
       (lbctrrst >= _4BITS_MAX_VAL_) ||
       (mode >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_multibbhcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MULTIBBHCFG, SNINITVAL, reg_common_configurations_multibbhcfg, sninitval);
    reg_common_configurations_multibbhcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MULTIBBHCFG, SNINIT, reg_common_configurations_multibbhcfg, sninit);
    reg_common_configurations_multibbhcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MULTIBBHCFG, LBCTRRST, reg_common_configurations_multibbhcfg, lbctrrst);
    reg_common_configurations_multibbhcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MULTIBBHCFG, MODE, reg_common_configurations_multibbhcfg, mode);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MULTIBBHCFG, reg_common_configurations_multibbhcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbhcfg_get(uint8_t bbh_id, uint16_t *sninitval, bdmf_boolean *sninit, uint8_t *lbctrrst, uint8_t *mode)
{
    uint32_t reg_common_configurations_multibbhcfg;

#ifdef VALIDATE_PARMS
    if (!sninitval || !sninit || !lbctrrst || !mode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MULTIBBHCFG, reg_common_configurations_multibbhcfg);

    *sninitval = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MULTIBBHCFG, SNINITVAL, reg_common_configurations_multibbhcfg);
    *sninit = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MULTIBBHCFG, SNINIT, reg_common_configurations_multibbhcfg);
    *lbctrrst = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MULTIBBHCFG, LBCTRRST, reg_common_configurations_multibbhcfg);
    *mode = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MULTIBBHCFG, MODE, reg_common_configurations_multibbhcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc_set(uint8_t bbh_id, uint8_t index, uint16_t bbsrcid)
{
    uint32_t reg_common_configurations_multibbsrc = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 3) ||
       (bbsrcid >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_multibbsrc = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MULTIBBSRC, BBSRCID, reg_common_configurations_multibbsrc, bbsrcid);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MULTIBBSRC, reg_common_configurations_multibbsrc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc_get(uint8_t bbh_id, uint8_t index, uint16_t *bbsrcid)
{
    uint32_t reg_common_configurations_multibbsrc;

#ifdef VALIDATE_PARMS
    if (!bbsrcid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 3))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MULTIBBSRC, reg_common_configurations_multibbsrc);

    *bbsrcid = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MULTIBBSRC, BBSRCID, reg_common_configurations_multibbsrc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader_set(uint8_t bbh_id, const crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader *common_configurations_addr_spreader)
{
    uint32_t reg_common_configurations_addr_spreader = 0;

#ifdef VALIDATE_PARMS
    if(!common_configurations_addr_spreader)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (common_configurations_addr_spreader->num_active_dram >= _2BITS_MAX_VAL_) ||
       (common_configurations_addr_spreader->en_ddr >= _1BITS_MAX_VAL_) ||
       (common_configurations_addr_spreader->en_epsr >= _1BITS_MAX_VAL_) ||
       (common_configurations_addr_spreader->simple_spread >= _1BITS_MAX_VAL_) ||
       (common_configurations_addr_spreader->active_dram_0 >= _3BITS_MAX_VAL_) ||
       (common_configurations_addr_spreader->active_dram_1 >= _3BITS_MAX_VAL_) ||
       (common_configurations_addr_spreader->block_spread_zone_en >= _1BITS_MAX_VAL_) ||
       (common_configurations_addr_spreader->block_spread_zone >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_addr_spreader = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, NUM_ACTIVE_DRAM, reg_common_configurations_addr_spreader, common_configurations_addr_spreader->num_active_dram);
    reg_common_configurations_addr_spreader = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, EN_DDR, reg_common_configurations_addr_spreader, common_configurations_addr_spreader->en_ddr);
    reg_common_configurations_addr_spreader = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, EN_EPSR, reg_common_configurations_addr_spreader, common_configurations_addr_spreader->en_epsr);
    reg_common_configurations_addr_spreader = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, SIMPLE_SPREAD, reg_common_configurations_addr_spreader, common_configurations_addr_spreader->simple_spread);
    reg_common_configurations_addr_spreader = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, ACTIVE_DRAM_0, reg_common_configurations_addr_spreader, common_configurations_addr_spreader->active_dram_0);
    reg_common_configurations_addr_spreader = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, ACTIVE_DRAM_1, reg_common_configurations_addr_spreader, common_configurations_addr_spreader->active_dram_1);
    reg_common_configurations_addr_spreader = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, BLOCK_SPREAD_ZONE_EN, reg_common_configurations_addr_spreader, common_configurations_addr_spreader->block_spread_zone_en);
    reg_common_configurations_addr_spreader = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, BLOCK_SPREAD_ZONE, reg_common_configurations_addr_spreader, common_configurations_addr_spreader->block_spread_zone);
    reg_common_configurations_addr_spreader = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, SPARE, reg_common_configurations_addr_spreader, common_configurations_addr_spreader->spare);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, reg_common_configurations_addr_spreader);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader_get(uint8_t bbh_id, crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader *common_configurations_addr_spreader)
{
    uint32_t reg_common_configurations_addr_spreader;

#ifdef VALIDATE_PARMS
    if (!common_configurations_addr_spreader)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, reg_common_configurations_addr_spreader);

    common_configurations_addr_spreader->num_active_dram = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, NUM_ACTIVE_DRAM, reg_common_configurations_addr_spreader);
    common_configurations_addr_spreader->en_ddr = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, EN_DDR, reg_common_configurations_addr_spreader);
    common_configurations_addr_spreader->en_epsr = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, EN_EPSR, reg_common_configurations_addr_spreader);
    common_configurations_addr_spreader->simple_spread = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, SIMPLE_SPREAD, reg_common_configurations_addr_spreader);
    common_configurations_addr_spreader->active_dram_0 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, ACTIVE_DRAM_0, reg_common_configurations_addr_spreader);
    common_configurations_addr_spreader->active_dram_1 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, ACTIVE_DRAM_1, reg_common_configurations_addr_spreader);
    common_configurations_addr_spreader->block_spread_zone_en = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, BLOCK_SPREAD_ZONE_EN, reg_common_configurations_addr_spreader);
    common_configurations_addr_spreader->block_spread_zone = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, BLOCK_SPREAD_ZONE, reg_common_configurations_addr_spreader);
    common_configurations_addr_spreader->spare = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER, SPARE, reg_common_configurations_addr_spreader);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_flowctrcfg_set(uint8_t bbh_id, bdmf_boolean numports, uint8_t numctrs, uint8_t macflowcnten, uint8_t ctridxcnten)
{
    uint32_t reg_common_configurations_flowctrcfg = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (numports >= _1BITS_MAX_VAL_) ||
       (numctrs >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_flowctrcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FLOWCTRCFG, NUMPORTS, reg_common_configurations_flowctrcfg, numports);
    reg_common_configurations_flowctrcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FLOWCTRCFG, NUMCTRS, reg_common_configurations_flowctrcfg, numctrs);
    reg_common_configurations_flowctrcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FLOWCTRCFG, MACFLOWCNTEN, reg_common_configurations_flowctrcfg, macflowcnten);
    reg_common_configurations_flowctrcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FLOWCTRCFG, CTRIDXCNTEN, reg_common_configurations_flowctrcfg, ctridxcnten);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FLOWCTRCFG, reg_common_configurations_flowctrcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_flowctrcfg_get(uint8_t bbh_id, bdmf_boolean *numports, uint8_t *numctrs, uint8_t *macflowcnten, uint8_t *ctridxcnten)
{
    uint32_t reg_common_configurations_flowctrcfg;

#ifdef VALIDATE_PARMS
    if (!numports || !numctrs || !macflowcnten || !ctridxcnten)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FLOWCTRCFG, reg_common_configurations_flowctrcfg);

    *numports = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FLOWCTRCFG, NUMPORTS, reg_common_configurations_flowctrcfg);
    *numctrs = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FLOWCTRCFG, NUMCTRS, reg_common_configurations_flowctrcfg);
    *macflowcnten = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FLOWCTRCFG, MACFLOWCNTEN, reg_common_configurations_flowctrcfg);
    *ctridxcnten = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FLOWCTRCFG, CTRIDXCNTEN, reg_common_configurations_flowctrcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg_set(uint8_t bbh_id, uint8_t index, uint16_t wkpaddr, uint8_t task, uint16_t bbsrcid)
{
    uint32_t reg_common_configurations_fcrnrcfg = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (task >= _4BITS_MAX_VAL_) ||
       (bbsrcid >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_fcrnrcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FCRNRCFG, WKPADDR, reg_common_configurations_fcrnrcfg, wkpaddr);
    reg_common_configurations_fcrnrcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FCRNRCFG, TASK, reg_common_configurations_fcrnrcfg, task);
    reg_common_configurations_fcrnrcfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FCRNRCFG, BBSRCID, reg_common_configurations_fcrnrcfg, bbsrcid);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FCRNRCFG, reg_common_configurations_fcrnrcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg_get(uint8_t bbh_id, uint8_t index, uint16_t *wkpaddr, uint8_t *task, uint16_t *bbsrcid)
{
    uint32_t reg_common_configurations_fcrnrcfg;

#ifdef VALIDATE_PARMS
    if (!wkpaddr || !task || !bbsrcid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FCRNRCFG, reg_common_configurations_fcrnrcfg);

    *wkpaddr = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FCRNRCFG, WKPADDR, reg_common_configurations_fcrnrcfg);
    *task = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FCRNRCFG, TASK, reg_common_configurations_fcrnrcfg);
    *bbsrcid = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FCRNRCFG, BBSRCID, reg_common_configurations_fcrnrcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fccfg_set(uint8_t bbh_id, uint16_t fc_stblfilter, bdmf_boolean fc_en)
{
    uint32_t reg_common_configurations_fccfg = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (fc_stblfilter >= _10BITS_MAX_VAL_) ||
       (fc_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_fccfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FCCFG, FC_STBLFILTER, reg_common_configurations_fccfg, fc_stblfilter);
    reg_common_configurations_fccfg = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FCCFG, FC_EN, reg_common_configurations_fccfg, fc_en);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FCCFG, reg_common_configurations_fccfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fccfg_get(uint8_t bbh_id, uint16_t *fc_stblfilter, bdmf_boolean *fc_en)
{
    uint32_t reg_common_configurations_fccfg;

#ifdef VALIDATE_PARMS
    if (!fc_stblfilter || !fc_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FCCFG, reg_common_configurations_fccfg);

    *fc_stblfilter = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FCCFG, FC_STBLFILTER, reg_common_configurations_fccfg);
    *fc_en = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FCCFG, FC_EN, reg_common_configurations_fccfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdack_set(uint8_t bbh_id, uint8_t q)
{
    uint32_t reg_per_port_configurations_pdack = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_pdack = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDACK, Q, reg_per_port_configurations_pdack, q);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDACK, reg_per_port_configurations_pdack);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdack_get(uint8_t bbh_id, uint8_t *q)
{
    uint32_t reg_per_port_configurations_pdack;

#ifdef VALIDATE_PARMS
    if (!q)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDACK, reg_per_port_configurations_pdack);

    *q = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDACK, Q, reg_per_port_configurations_pdack);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize_set(uint8_t bbh_id, uint8_t index, uint16_t fifosize)
{
    uint32_t reg_per_port_configurations_pdsize = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (fifosize >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_pdsize = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDSIZE, FIFOSIZE, reg_per_port_configurations_pdsize, fifosize);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDSIZE, reg_per_port_configurations_pdsize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize_get(uint8_t bbh_id, uint8_t index, uint16_t *fifosize)
{
    uint32_t reg_per_port_configurations_pdsize;

#ifdef VALIDATE_PARMS
    if (!fifosize)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDSIZE, reg_per_port_configurations_pdsize);

    *fifosize = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDSIZE, FIFOSIZE, reg_per_port_configurations_pdsize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph_set(uint8_t bbh_id, uint8_t index, uint16_t wkupthresh)
{
    uint32_t reg_per_port_configurations_pdwkuph = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (wkupthresh >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_pdwkuph = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDWKUPH, WKUPTHRESH, reg_per_port_configurations_pdwkuph, wkupthresh);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDWKUPH, reg_per_port_configurations_pdwkuph);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph_get(uint8_t bbh_id, uint8_t index, uint16_t *wkupthresh)
{
    uint32_t reg_per_port_configurations_pdwkuph;

#ifdef VALIDATE_PARMS
    if (!wkupthresh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDWKUPH, reg_per_port_configurations_pdwkuph);

    *wkupthresh = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDWKUPH, WKUPTHRESH, reg_per_port_configurations_pdwkuph);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty_set(uint8_t bbh_id, uint8_t index, uint16_t empty)
{
    uint32_t reg_per_port_configurations_pdempty = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (empty >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_pdempty = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDEMPTY, EMPTY, reg_per_port_configurations_pdempty, empty);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDEMPTY, reg_per_port_configurations_pdempty);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty_get(uint8_t bbh_id, uint8_t index, uint16_t *empty)
{
    uint32_t reg_per_port_configurations_pdempty;

#ifdef VALIDATE_PARMS
    if (!empty)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDEMPTY, reg_per_port_configurations_pdempty);

    *empty = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDEMPTY, EMPTY, reg_per_port_configurations_pdempty);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_set(uint8_t bbh_id, uint8_t index, uint32_t bytelimit)
{
    uint32_t reg_per_port_configurations_pd_byte_th = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (bytelimit >= _22BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_pd_byte_th = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PD_BYTE_TH, BYTELIMIT, reg_per_port_configurations_pd_byte_th, bytelimit);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PD_BYTE_TH, reg_per_port_configurations_pd_byte_th);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_get(uint8_t bbh_id, uint8_t index, uint32_t *bytelimit)
{
    uint32_t reg_per_port_configurations_pd_byte_th;

#ifdef VALIDATE_PARMS
    if (!bytelimit)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PD_BYTE_TH, reg_per_port_configurations_pd_byte_th);

    *bytelimit = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PD_BYTE_TH, BYTELIMIT, reg_per_port_configurations_pd_byte_th);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th_set(uint8_t bbh_id, uint8_t index, uint16_t pdlimit)
{
    uint32_t reg_per_port_configurations_max_pd_th = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (pdlimit >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_max_pd_th = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_MAX_PD_TH, PDLIMIT, reg_per_port_configurations_max_pd_th, pdlimit);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_MAX_PD_TH, reg_per_port_configurations_max_pd_th);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th_get(uint8_t bbh_id, uint8_t index, uint16_t *pdlimit)
{
    uint32_t reg_per_port_configurations_max_pd_th;

#ifdef VALIDATE_PARMS
    if (!pdlimit)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_MAX_PD_TH, reg_per_port_configurations_max_pd_th);

    *pdlimit = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_MAX_PD_TH, PDLIMIT, reg_per_port_configurations_max_pd_th);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_en_set(uint8_t bbh_id, uint8_t pdbytelimiten, uint8_t pdmaxlimiten, uint8_t disfesf)
{
    uint32_t reg_per_port_configurations_pd_byte_th_en = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_pd_byte_th_en = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN, PDBYTELIMITEN, reg_per_port_configurations_pd_byte_th_en, pdbytelimiten);
    reg_per_port_configurations_pd_byte_th_en = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN, PDMAXLIMITEN, reg_per_port_configurations_pd_byte_th_en, pdmaxlimiten);
    reg_per_port_configurations_pd_byte_th_en = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN, DISFESF, reg_per_port_configurations_pd_byte_th_en, disfesf);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN, reg_per_port_configurations_pd_byte_th_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_en_get(uint8_t bbh_id, uint8_t *pdbytelimiten, uint8_t *pdmaxlimiten, uint8_t *disfesf)
{
    uint32_t reg_per_port_configurations_pd_byte_th_en;

#ifdef VALIDATE_PARMS
    if (!pdbytelimiten || !pdmaxlimiten || !disfesf)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN, reg_per_port_configurations_pd_byte_th_en);

    *pdbytelimiten = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN, PDBYTELIMITEN, reg_per_port_configurations_pd_byte_th_en);
    *pdmaxlimiten = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN, PDMAXLIMITEN, reg_per_port_configurations_pd_byte_th_en);
    *disfesf = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN, DISFESF, reg_per_port_configurations_pd_byte_th_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset_set(uint8_t bbh_id, uint8_t index, uint16_t offset0, uint16_t offset1)
{
    uint32_t reg_per_port_configurations_flowtogemoffset = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_flowtogemoffset = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET, OFFSET0, reg_per_port_configurations_flowtogemoffset, offset0);
    reg_per_port_configurations_flowtogemoffset = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET, OFFSET1, reg_per_port_configurations_flowtogemoffset, offset1);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET, reg_per_port_configurations_flowtogemoffset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset_get(uint8_t bbh_id, uint8_t index, uint16_t *offset0, uint16_t *offset1)
{
    uint32_t reg_per_port_configurations_flowtogemoffset;

#ifdef VALIDATE_PARMS
    if (!offset0 || !offset1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET, reg_per_port_configurations_flowtogemoffset);

    *offset0 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET, OFFSET0, reg_per_port_configurations_flowtogemoffset);
    *offset1 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET, OFFSET1, reg_per_port_configurations_flowtogemoffset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr_set(uint8_t bbh_id, uint8_t index, uint16_t thr)
{
    uint32_t reg_per_port_configurations_flowtogemthr = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (thr >= _13BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_flowtogemthr = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR, THR, reg_per_port_configurations_flowtogemthr, thr);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR, reg_per_port_configurations_flowtogemthr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr_get(uint8_t bbh_id, uint8_t index, uint16_t *thr)
{
    uint32_t reg_per_port_configurations_flowtogemthr;

#ifdef VALIDATE_PARMS
    if (!thr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR, reg_per_port_configurations_flowtogemthr);

    *thr = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR, THR, reg_per_port_configurations_flowtogemthr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_febase_set(uint8_t bbh_id, uint8_t index, uint16_t fifobase0)
{
    uint32_t reg_per_port_configurations_febase = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (fifobase0 >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_febase = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FEBASE, FIFOBASE0, reg_per_port_configurations_febase, fifobase0);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FEBASE, reg_per_port_configurations_febase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_febase_get(uint8_t bbh_id, uint8_t index, uint16_t *fifobase0)
{
    uint32_t reg_per_port_configurations_febase;

#ifdef VALIDATE_PARMS
    if (!fifobase0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FEBASE, reg_per_port_configurations_febase);

    *fifobase0 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FEBASE, FIFOBASE0, reg_per_port_configurations_febase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize_set(uint8_t bbh_id, uint8_t index, uint16_t fifosize)
{
    uint32_t reg_per_port_configurations_fesize = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (fifosize >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_fesize = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FESIZE, FIFOSIZE, reg_per_port_configurations_fesize, fifosize);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FESIZE, reg_per_port_configurations_fesize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize_get(uint8_t bbh_id, uint8_t index, uint16_t *fifosize)
{
    uint32_t reg_per_port_configurations_fesize;

#ifdef VALIDATE_PARMS
    if (!fifosize)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FESIZE, reg_per_port_configurations_fesize);

    *fifosize = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FESIZE, FIFOSIZE, reg_per_port_configurations_fesize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase_set(uint8_t bbh_id, uint8_t index, uint16_t fifobase0)
{
    uint32_t reg_per_port_configurations_fepdbase = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (fifobase0 >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_fepdbase = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FEPDBASE, FIFOBASE0, reg_per_port_configurations_fepdbase, fifobase0);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FEPDBASE, reg_per_port_configurations_fepdbase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase_get(uint8_t bbh_id, uint8_t index, uint16_t *fifobase0)
{
    uint32_t reg_per_port_configurations_fepdbase;

#ifdef VALIDATE_PARMS
    if (!fifobase0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FEPDBASE, reg_per_port_configurations_fepdbase);

    *fifobase0 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FEPDBASE, FIFOBASE0, reg_per_port_configurations_fepdbase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize_set(uint8_t bbh_id, uint8_t index, uint16_t fifosize0)
{
    uint32_t reg_per_port_configurations_fepdsize = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (fifosize0 >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_fepdsize = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FEPDSIZE, FIFOSIZE0, reg_per_port_configurations_fepdsize, fifosize0);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FEPDSIZE, reg_per_port_configurations_fepdsize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize_get(uint8_t bbh_id, uint8_t index, uint16_t *fifosize0)
{
    uint32_t reg_per_port_configurations_fepdsize;

#ifdef VALIDATE_PARMS
    if (!fifosize0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FEPDSIZE, reg_per_port_configurations_fepdsize);

    *fifosize0 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FEPDSIZE, FIFOSIZE0, reg_per_port_configurations_fepdsize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr_set(uint8_t bbh_id, uint8_t index, uint8_t w0)
{
    uint32_t reg_per_port_configurations_sgmtwrr = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (w0 >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_sgmtwrr = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_SGMTWRR, W0, reg_per_port_configurations_sgmtwrr, w0);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_SGMTWRR, reg_per_port_configurations_sgmtwrr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr_get(uint8_t bbh_id, uint8_t index, uint8_t *w0)
{
    uint32_t reg_per_port_configurations_sgmtwrr;

#ifdef VALIDATE_PARMS
    if (!w0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_SGMTWRR, reg_per_port_configurations_sgmtwrr);

    *w0 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_SGMTWRR, W0, reg_per_port_configurations_sgmtwrr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fe_credits_set(uint8_t bbh_id, uint8_t init, uint16_t min_pd, uint16_t min_data)
{
    uint32_t reg_per_port_configurations_fe_credits = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (min_pd >= _9BITS_MAX_VAL_) ||
       (min_data >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_fe_credits = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FE_CREDITS, INIT, reg_per_port_configurations_fe_credits, init);
    reg_per_port_configurations_fe_credits = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FE_CREDITS, MIN_PD, reg_per_port_configurations_fe_credits, min_pd);
    reg_per_port_configurations_fe_credits = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FE_CREDITS, MIN_DATA, reg_per_port_configurations_fe_credits, min_data);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FE_CREDITS, reg_per_port_configurations_fe_credits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fe_credits_get(uint8_t bbh_id, uint8_t *init, uint16_t *min_pd, uint16_t *min_data)
{
    uint32_t reg_per_port_configurations_fe_credits;

#ifdef VALIDATE_PARMS
    if (!init || !min_pd || !min_data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FE_CREDITS, reg_per_port_configurations_fe_credits);

    *init = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FE_CREDITS, INIT, reg_per_port_configurations_fe_credits);
    *min_pd = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FE_CREDITS, MIN_PD, reg_per_port_configurations_fe_credits);
    *min_data = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FE_CREDITS, MIN_DATA, reg_per_port_configurations_fe_credits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_en_init_set(uint8_t bbh_id, uint16_t init)
{
    uint32_t reg_per_port_configurations_gem_en_init = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_gem_en_init = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_GEM_EN_INIT, INIT, reg_per_port_configurations_gem_en_init, init);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_GEM_EN_INIT, reg_per_port_configurations_gem_en_init);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_en_init_get(uint8_t bbh_id, uint16_t *init)
{
    uint32_t reg_per_port_configurations_gem_en_init;

#ifdef VALIDATE_PARMS
    if (!init)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_GEM_EN_INIT, reg_per_port_configurations_gem_en_init);

    *init = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_GEM_EN_INIT, INIT, reg_per_port_configurations_gem_en_init);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_trns_init_set(uint8_t bbh_id, uint8_t init)
{
    uint32_t reg_per_port_configurations_gem_trns_init = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (init >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_gem_trns_init = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT, INIT, reg_per_port_configurations_gem_trns_init, init);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT, reg_per_port_configurations_gem_trns_init);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_trns_init_get(uint8_t bbh_id, uint8_t *init)
{
    uint32_t reg_per_port_configurations_gem_trns_init;

#ifdef VALIDATE_PARMS
    if (!init)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT, reg_per_port_configurations_gem_trns_init);

    *init = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT, INIT, reg_per_port_configurations_gem_trns_init);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes_set(uint8_t bbh_id, uint8_t index, uint16_t ddr, uint16_t psram0, uint16_t psram1)
{
    uint32_t reg_per_port_configurations_otfbytes = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (ddr >= _10BITS_MAX_VAL_) ||
       (psram0 >= _10BITS_MAX_VAL_) ||
       (psram1 >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_otfbytes = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFBYTES, DDR, reg_per_port_configurations_otfbytes, ddr);
    reg_per_port_configurations_otfbytes = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFBYTES, PSRAM0, reg_per_port_configurations_otfbytes, psram0);
    reg_per_port_configurations_otfbytes = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFBYTES, PSRAM1, reg_per_port_configurations_otfbytes, psram1);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFBYTES, reg_per_port_configurations_otfbytes);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes_get(uint8_t bbh_id, uint8_t index, uint16_t *ddr, uint16_t *psram0, uint16_t *psram1)
{
    uint32_t reg_per_port_configurations_otfbytes;

#ifdef VALIDATE_PARMS
    if (!ddr || !psram0 || !psram1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFBYTES, reg_per_port_configurations_otfbytes);

    *ddr = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFBYTES, DDR, reg_per_port_configurations_otfbytes);
    *psram0 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFBYTES, PSRAM0, reg_per_port_configurations_otfbytes);
    *psram1 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFBYTES, PSRAM1, reg_per_port_configurations_otfbytes);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd_set(uint8_t bbh_id, uint8_t index, uint8_t ddr, uint8_t psram0, uint8_t psram1)
{
    uint32_t reg_per_port_configurations_otfcmd = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_otfcmd = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFCMD, DDR, reg_per_port_configurations_otfcmd, ddr);
    reg_per_port_configurations_otfcmd = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFCMD, PSRAM0, reg_per_port_configurations_otfcmd, psram0);
    reg_per_port_configurations_otfcmd = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFCMD, PSRAM1, reg_per_port_configurations_otfcmd, psram1);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFCMD, reg_per_port_configurations_otfcmd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd_get(uint8_t bbh_id, uint8_t index, uint8_t *ddr, uint8_t *psram0, uint8_t *psram1)
{
    uint32_t reg_per_port_configurations_otfcmd;

#ifdef VALIDATE_PARMS
    if (!ddr || !psram0 || !psram1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFCMD, reg_per_port_configurations_otfcmd);

    *ddr = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFCMD, DDR, reg_per_port_configurations_otfcmd);
    *psram0 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFCMD, PSRAM0, reg_per_port_configurations_otfcmd);
    *psram1 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFCMD, PSRAM1, reg_per_port_configurations_otfcmd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_autootfcredits_set(uint8_t bbh_id, uint8_t numports)
{
    uint32_t reg_per_port_configurations_autootfcredits = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (numports >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_per_port_configurations_autootfcredits = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS, NUMPORTS, reg_per_port_configurations_autootfcredits, numports);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS, reg_per_port_configurations_autootfcredits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_autootfcredits_get(uint8_t bbh_id, uint8_t *numports)
{
    uint32_t reg_per_port_configurations_autootfcredits;

#ifdef VALIDATE_PARMS
    if (!numports)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS, reg_per_port_configurations_autootfcredits);

    *numports = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS, NUMPORTS, reg_per_port_configurations_autootfcredits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_txsram0pd_get(uint8_t bbh_id, uint32_t *srampd)
{
    uint32_t reg_debug_counters_txsram0pd;

#ifdef VALIDATE_PARMS
    if (!srampd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXSRAM0PD, reg_debug_counters_txsram0pd);

    *srampd = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXSRAM0PD, SRAMPD, reg_debug_counters_txsram0pd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_txsram1pd_get(uint8_t bbh_id, uint32_t *srampd)
{
    uint32_t reg_debug_counters_txsram1pd;

#ifdef VALIDATE_PARMS
    if (!srampd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXSRAM1PD, reg_debug_counters_txsram1pd);

    *srampd = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXSRAM1PD, SRAMPD, reg_debug_counters_txsram1pd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_txddrpd_get(uint8_t bbh_id, uint32_t *ddrpd)
{
    uint32_t reg_debug_counters_txddrpd;

#ifdef VALIDATE_PARMS
    if (!ddrpd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXDDRPD, reg_debug_counters_txddrpd);

    *ddrpd = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXDDRPD, DDRPD, reg_debug_counters_txddrpd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pddrop_get(uint8_t bbh_id, uint16_t *pddrop)
{
    uint32_t reg_debug_counters_pddrop;

#ifdef VALIDATE_PARMS
    if (!pddrop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PDDROP, reg_debug_counters_pddrop);

    *pddrop = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PDDROP, PDDROP, reg_debug_counters_pddrop);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_getnextnull_get(uint8_t bbh_id, uint16_t *getnextnull)
{
    uint32_t reg_debug_counters_getnextnull;

#ifdef VALIDATE_PARMS
    if (!getnextnull)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_GETNEXTNULL, reg_debug_counters_getnextnull);

    *getnextnull = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_GETNEXTNULL, GETNEXTNULL, reg_debug_counters_getnextnull);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_getnextdrop_get(uint8_t bbh_id, uint16_t *getnextdrop)
{
    uint32_t reg_debug_counters_getnextdrop;

#ifdef VALIDATE_PARMS
    if (!getnextdrop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_GETNEXTDROP, reg_debug_counters_getnextdrop);

    *getnextdrop = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_GETNEXTDROP, GETNEXTDROP, reg_debug_counters_getnextdrop);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_inputfifodrop_get(uint8_t bbh_id, uint16_t *inputfifodrop)
{
    uint32_t reg_debug_counters_inputfifodrop;

#ifdef VALIDATE_PARMS
    if (!inputfifodrop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_INPUTFIFODROP, reg_debug_counters_inputfifodrop);

    *inputfifodrop = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_INPUTFIFODROP, INPUTFIFODROP, reg_debug_counters_inputfifodrop);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram0pkt_get(uint8_t bbh_id, uint32_t *srampkt)
{
    uint32_t reg_debug_counters_sram0pkt;

#ifdef VALIDATE_PARMS
    if (!srampkt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SRAM0PKT, reg_debug_counters_sram0pkt);

    *srampkt = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SRAM0PKT, SRAMPKT, reg_debug_counters_sram0pkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram1pkt_get(uint8_t bbh_id, uint32_t *srampkt)
{
    uint32_t reg_debug_counters_sram1pkt;

#ifdef VALIDATE_PARMS
    if (!srampkt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SRAM1PKT, reg_debug_counters_sram1pkt);

    *srampkt = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SRAM1PKT, SRAMPKT, reg_debug_counters_sram1pkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ddrpkt_get(uint8_t bbh_id, uint32_t *ddrpkt)
{
    uint32_t reg_debug_counters_ddrpkt;

#ifdef VALIDATE_PARMS
    if (!ddrpkt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DDRPKT, reg_debug_counters_ddrpkt);

    *ddrpkt = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DDRPKT, DDRPKT, reg_debug_counters_ddrpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrden_set(uint8_t bbh_id, const crdp_ds_cmn_bbh_tx_debug_counters_swrden *debug_counters_swrden)
{
    uint32_t reg_debug_counters_swrden = 0;

#ifdef VALIDATE_PARMS
    if(!debug_counters_swrden)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (debug_counters_swrden->pdsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->pdvsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->pdemptysel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->pdfullsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->pdbemptysel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->pdffwkpsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->fbn_0_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->fbnv_0_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->fbnempty_0_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->fbnfull_0_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->getnext_0_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->getnextv_0_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->getnextempty_0_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->getnextfull_0_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->fpm_0_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->sbpm_0_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->fbnv_1_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->fbnempty_1_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->fbnfull_1_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->getnext_1_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->getnextv_1_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->getnextempty_1_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->getnextfull_1_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->fpm_1_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->sbpm_1_sel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->fbn_1_sel >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, PDSEL, reg_debug_counters_swrden, debug_counters_swrden->pdsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, PDVSEL, reg_debug_counters_swrden, debug_counters_swrden->pdvsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, PDEMPTYSEL, reg_debug_counters_swrden, debug_counters_swrden->pdemptysel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, PDFULLSEL, reg_debug_counters_swrden, debug_counters_swrden->pdfullsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, PDBEMPTYSEL, reg_debug_counters_swrden, debug_counters_swrden->pdbemptysel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, PDFFWKPSEL, reg_debug_counters_swrden, debug_counters_swrden->pdffwkpsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FBN_0_SEL, reg_debug_counters_swrden, debug_counters_swrden->fbn_0_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNV_0_SEL, reg_debug_counters_swrden, debug_counters_swrden->fbnv_0_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNEMPTY_0_SEL, reg_debug_counters_swrden, debug_counters_swrden->fbnempty_0_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNFULL_0_SEL, reg_debug_counters_swrden, debug_counters_swrden->fbnfull_0_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXT_0_SEL, reg_debug_counters_swrden, debug_counters_swrden->getnext_0_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTV_0_SEL, reg_debug_counters_swrden, debug_counters_swrden->getnextv_0_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTEMPTY_0_SEL, reg_debug_counters_swrden, debug_counters_swrden->getnextempty_0_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTFULL_0_SEL, reg_debug_counters_swrden, debug_counters_swrden->getnextfull_0_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FPM_0_SEL, reg_debug_counters_swrden, debug_counters_swrden->fpm_0_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, SBPM_0_SEL, reg_debug_counters_swrden, debug_counters_swrden->sbpm_0_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNV_1_SEL, reg_debug_counters_swrden, debug_counters_swrden->fbnv_1_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNEMPTY_1_SEL, reg_debug_counters_swrden, debug_counters_swrden->fbnempty_1_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNFULL_1_SEL, reg_debug_counters_swrden, debug_counters_swrden->fbnfull_1_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXT_1_SEL, reg_debug_counters_swrden, debug_counters_swrden->getnext_1_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTV_1_SEL, reg_debug_counters_swrden, debug_counters_swrden->getnextv_1_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTEMPTY_1_SEL, reg_debug_counters_swrden, debug_counters_swrden->getnextempty_1_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTFULL_1_SEL, reg_debug_counters_swrden, debug_counters_swrden->getnextfull_1_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FPM_1_SEL, reg_debug_counters_swrden, debug_counters_swrden->fpm_1_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, SBPM_1_SEL, reg_debug_counters_swrden, debug_counters_swrden->sbpm_1_sel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FBN_1_SEL, reg_debug_counters_swrden, debug_counters_swrden->fbn_1_sel);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, reg_debug_counters_swrden);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrden_get(uint8_t bbh_id, crdp_ds_cmn_bbh_tx_debug_counters_swrden *debug_counters_swrden)
{
    uint32_t reg_debug_counters_swrden;

#ifdef VALIDATE_PARMS
    if (!debug_counters_swrden)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, reg_debug_counters_swrden);

    debug_counters_swrden->pdsel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, PDSEL, reg_debug_counters_swrden);
    debug_counters_swrden->pdvsel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, PDVSEL, reg_debug_counters_swrden);
    debug_counters_swrden->pdemptysel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, PDEMPTYSEL, reg_debug_counters_swrden);
    debug_counters_swrden->pdfullsel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, PDFULLSEL, reg_debug_counters_swrden);
    debug_counters_swrden->pdbemptysel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, PDBEMPTYSEL, reg_debug_counters_swrden);
    debug_counters_swrden->pdffwkpsel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, PDFFWKPSEL, reg_debug_counters_swrden);
    debug_counters_swrden->fbn_0_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FBN_0_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->fbnv_0_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNV_0_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->fbnempty_0_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNEMPTY_0_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->fbnfull_0_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNFULL_0_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->getnext_0_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXT_0_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->getnextv_0_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTV_0_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->getnextempty_0_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTEMPTY_0_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->getnextfull_0_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTFULL_0_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->fpm_0_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FPM_0_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->sbpm_0_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, SBPM_0_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->fbnv_1_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNV_1_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->fbnempty_1_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNEMPTY_1_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->fbnfull_1_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNFULL_1_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->getnext_1_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXT_1_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->getnextv_1_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTV_1_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->getnextempty_1_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTEMPTY_1_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->getnextfull_1_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTFULL_1_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->fpm_1_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FPM_1_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->sbpm_1_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, SBPM_1_SEL, reg_debug_counters_swrden);
    debug_counters_swrden->fbn_1_sel = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN, FBN_1_SEL, reg_debug_counters_swrden);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrdaddr_set(uint8_t bbh_id, uint16_t rdaddr)
{
    uint32_t reg_debug_counters_swrdaddr = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (rdaddr >= _11BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_counters_swrdaddr = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDADDR, RDADDR, reg_debug_counters_swrdaddr, rdaddr);

    RU_REG_WRITE(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDADDR, reg_debug_counters_swrdaddr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrdaddr_get(uint8_t bbh_id, uint16_t *rdaddr)
{
    uint32_t reg_debug_counters_swrdaddr;

#ifdef VALIDATE_PARMS
    if (!rdaddr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDADDR, reg_debug_counters_swrdaddr);

    *rdaddr = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDADDR, RDADDR, reg_debug_counters_swrdaddr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrddata_get(uint8_t bbh_id, uint32_t *data)
{
    uint32_t reg_debug_counters_swrddata;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDDATA, reg_debug_counters_swrddata);

    *data = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDDATA, DATA, reg_debug_counters_swrddata);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unifiedpkt_get(uint8_t bbh_id, uint8_t debug_unified_pkt_ctr_idx, uint32_t *unified_pkt)
{
    uint32_t reg_debug_counters_unifiedpkt;

#ifdef VALIDATE_PARMS
    if (!unified_pkt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (debug_unified_pkt_ctr_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, debug_unified_pkt_ctr_idx, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_UNIFIEDPKT, reg_debug_counters_unifiedpkt);

    *unified_pkt = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_UNIFIEDPKT, UNIFIED_PKT, reg_debug_counters_unifiedpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unifiedbyte_get(uint8_t bbh_id, uint8_t debug_unified_byte_ctr_idx, uint32_t *ddrbyte)
{
    uint32_t reg_debug_counters_unifiedbyte;

#ifdef VALIDATE_PARMS
    if (!ddrbyte)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (debug_unified_byte_ctr_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, debug_unified_byte_ctr_idx, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_UNIFIEDBYTE, reg_debug_counters_unifiedbyte);

    *ddrbyte = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_UNIFIEDBYTE, DDRBYTE, reg_debug_counters_unifiedbyte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_dbgoutreg_get(uint8_t bbh_id, uint8_t zero, crdp_ds_cmn_bbh_tx_debug_counters_dbgoutreg *debug_counters_dbgoutreg)
{
#ifdef VALIDATE_PARMS
    if (!debug_counters_dbgoutreg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero * 8 + 0, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DBGOUTREG, debug_counters_dbgoutreg->debug_out_reg[0]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 1, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DBGOUTREG, debug_counters_dbgoutreg->debug_out_reg[1]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 2, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DBGOUTREG, debug_counters_dbgoutreg->debug_out_reg[2]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 3, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DBGOUTREG, debug_counters_dbgoutreg->debug_out_reg[3]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 4, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DBGOUTREG, debug_counters_dbgoutreg->debug_out_reg[4]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 5, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DBGOUTREG, debug_counters_dbgoutreg->debug_out_reg[5]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 6, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DBGOUTREG, debug_counters_dbgoutreg->debug_out_reg[6]);
    RU_REG_RAM_READ(bbh_id, zero * 8 + 7, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DBGOUTREG, debug_counters_dbgoutreg->debug_out_reg[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_in_segmentation_get(uint8_t bbh_id, uint32_t *in_segmentation)
{
    uint32_t reg_debug_counters_in_segmentation;

#ifdef VALIDATE_PARMS
    if (!in_segmentation)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_IN_SEGMENTATION, reg_debug_counters_in_segmentation);

    *in_segmentation = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_IN_SEGMENTATION, IN_SEGMENTATION, reg_debug_counters_in_segmentation);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unified_data_credits_get(uint8_t bbh_id, uint8_t bbh_tx_debug_counters_unified_data_credits_byte_ctr_idx, uint16_t *credits)
{
    uint32_t reg_debug_counters_unified_data_credits;

#ifdef VALIDATE_PARMS
    if (!credits)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (bbh_tx_debug_counters_unified_data_credits_byte_ctr_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, bbh_tx_debug_counters_unified_data_credits_byte_ctr_idx, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_UNIFIED_DATA_CREDITS, reg_debug_counters_unified_data_credits);

    *credits = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_UNIFIED_DATA_CREDITS, CREDITS, reg_debug_counters_unified_data_credits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unified_pd_credits_get(uint8_t bbh_id, uint8_t bbh_tx_debug_counters_unified_pd_credits_byte_ctr_idx, uint16_t *credits)
{
    uint32_t reg_debug_counters_unified_pd_credits;

#ifdef VALIDATE_PARMS
    if (!credits)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (bbh_tx_debug_counters_unified_pd_credits_byte_ctr_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, bbh_tx_debug_counters_unified_pd_credits_byte_ctr_idx, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_UNIFIED_PD_CREDITS, reg_debug_counters_unified_pd_credits);

    *credits = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_UNIFIED_PD_CREDITS, CREDITS, reg_debug_counters_unified_pd_credits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_disflowdroppkt_get(uint8_t bbh_id, uint32_t *droppkt)
{
    uint32_t reg_debug_counters_disflowdroppkt;

#ifdef VALIDATE_PARMS
    if (!droppkt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DISFLOWDROPPKT, reg_debug_counters_disflowdroppkt);

    *droppkt = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DISFLOWDROPPKT, DROPPKT, reg_debug_counters_disflowdroppkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_disflowdropbyte_get(uint8_t bbh_id, uint32_t *dropbyte)
{
    uint32_t reg_debug_counters_disflowdropbyte;

#ifdef VALIDATE_PARMS
    if (!dropbyte)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DISFLOWDROPBYTE, reg_debug_counters_disflowdropbyte);

    *dropbyte = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DISFLOWDROPBYTE, DROPBYTE, reg_debug_counters_disflowdropbyte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnpkt_get(uint8_t bbh_id, uint32_t *ecnpkt)
{
    uint32_t reg_debug_counters_ecnpkt;

#ifdef VALIDATE_PARMS
    if (!ecnpkt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_ECNPKT, reg_debug_counters_ecnpkt);

    *ecnpkt = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_ECNPKT, ECNPKT, reg_debug_counters_ecnpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnbytes_get(uint8_t bbh_id, uint32_t *ecnbytes)
{
    uint32_t reg_debug_counters_ecnbytes;

#ifdef VALIDATE_PARMS
    if (!ecnbytes)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_ECNBYTES, reg_debug_counters_ecnbytes);

    *ecnbytes = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_ECNBYTES, ECNBYTES, reg_debug_counters_ecnbytes);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnmarkerr_get(uint8_t bbh_id, uint16_t *v1, uint16_t *v2)
{
    uint32_t reg_debug_counters_ecnmarkerr;

#ifdef VALIDATE_PARMS
    if (!v1 || !v2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_ECNMARKERR, reg_debug_counters_ecnmarkerr);

    *v1 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_ECNMARKERR, V1, reg_debug_counters_ecnmarkerr);
    *v2 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_ECNMARKERR, V2, reg_debug_counters_ecnmarkerr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnlenerr_get(uint8_t bbh_id, uint16_t *v1, uint16_t *v2)
{
    uint32_t reg_debug_counters_ecnlenerr;

#ifdef VALIDATE_PARMS
    if (!v1 || !v2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_ECNLENERR, reg_debug_counters_ecnlenerr);

    *v1 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_ECNLENERR, V1, reg_debug_counters_ecnlenerr);
    *v2 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_ECNLENERR, V2, reg_debug_counters_ecnlenerr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_flush_info_get(uint8_t bbh_id, uint16_t *sn, uint8_t *pdfifoempty, uint8_t *pdarrayempty)
{
    uint32_t reg_debug_counters_flush_info;

#ifdef VALIDATE_PARMS
    if (!sn || !pdfifoempty || !pdarrayempty)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_FLUSH_INFO, reg_debug_counters_flush_info);

    *sn = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_FLUSH_INFO, SN, reg_debug_counters_flush_info);
    *pdfifoempty = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_FLUSH_INFO, PDFIFOEMPTY, reg_debug_counters_flush_info);
    *pdarrayempty = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_FLUSH_INFO, PDARRAYEMPTY, reg_debug_counters_flush_info);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_flush_info2_get(uint8_t bbh_id, uint8_t *pfbingempty, uint8_t *fepdempty, uint8_t *fedataempty, uint8_t *portvld)
{
    uint32_t reg_debug_counters_flush_info2;

#ifdef VALIDATE_PARMS
    if (!pfbingempty || !fepdempty || !fedataempty || !portvld)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_FLUSH_INFO2, reg_debug_counters_flush_info2);

    *pfbingempty = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_FLUSH_INFO2, PFBINGEMPTY, reg_debug_counters_flush_info2);
    *fepdempty = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_FLUSH_INFO2, FEPDEMPTY, reg_debug_counters_flush_info2);
    *fedataempty = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_FLUSH_INFO2, FEDATAEMPTY, reg_debug_counters_flush_info2);
    *portvld = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_FLUSH_INFO2, PORTVLD, reg_debug_counters_flush_info2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pd_eh_mismatch_get(uint8_t bbh_id, uint16_t *cnt)
{
    uint32_t reg_debug_counters_pd_eh_mismatch;

#ifdef VALIDATE_PARMS
    if (!cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PD_EH_MISMATCH, reg_debug_counters_pd_eh_mismatch);

    *cnt = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PD_EH_MISMATCH, CNT, reg_debug_counters_pd_eh_mismatch);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pd_eh_mismatch_data_get(uint8_t bbh_id, uint8_t *port, uint16_t *plen, uint8_t *pktnum)
{
    uint32_t reg_debug_counters_pd_eh_mismatch_data;

#ifdef VALIDATE_PARMS
    if (!port || !plen || !pktnum)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PD_EH_MISMATCH_DATA, reg_debug_counters_pd_eh_mismatch_data);

    *port = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PD_EH_MISMATCH_DATA, PORT, reg_debug_counters_pd_eh_mismatch_data);
    *plen = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PD_EH_MISMATCH_DATA, PLEN, reg_debug_counters_pd_eh_mismatch_data);
    *pktnum = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PD_EH_MISMATCH_DATA, PKTNUM, reg_debug_counters_pd_eh_mismatch_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_outofrange_errors_get(uint8_t bbh_id, bdmf_boolean *macflow, bdmf_boolean *ctridx, uint8_t *port)
{
    uint32_t reg_debug_counters_outofrange_errors;

#ifdef VALIDATE_PARMS
    if (!macflow || !ctridx || !port)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_OUTOFRANGE_ERRORS, reg_debug_counters_outofrange_errors);

    *macflow = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_OUTOFRANGE_ERRORS, MACFLOW, reg_debug_counters_outofrange_errors);
    *ctridx = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_OUTOFRANGE_ERRORS, CTRIDX, reg_debug_counters_outofrange_errors);
    *port = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_OUTOFRANGE_ERRORS, PORT, reg_debug_counters_outofrange_errors);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_gemdroppkt_get(uint8_t bbh_id, uint8_t index, uint32_t *pkt)
{
    uint32_t reg_debug_counters_gemdroppkt;

#ifdef VALIDATE_PARMS
    if (!pkt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_GEMDROPPKT, reg_debug_counters_gemdroppkt);

    *pkt = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_GEMDROPPKT, PKT, reg_debug_counters_gemdroppkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_gemdropbytes_get(uint8_t bbh_id, uint8_t index, uint32_t *bytes)
{
    uint32_t reg_debug_counters_gemdropbytes;

#ifdef VALIDATE_PARMS
    if (!bytes)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_GEMDROPBYTES, reg_debug_counters_gemdropbytes);

    *bytes = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_GEMDROPBYTES, BYTES, reg_debug_counters_gemdropbytes);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_cmdotfddr_get(uint8_t bbh_id, uint8_t index, uint8_t *value)
{
    uint32_t reg_debug_counters_cmdotfddr;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_CMDOTFDDR, reg_debug_counters_cmdotfddr);

    *value = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_CMDOTFDDR, VALUE, reg_debug_counters_cmdotfddr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_bytesotfddr_get(uint8_t bbh_id, uint8_t index, uint16_t *value)
{
    uint32_t reg_debug_counters_bytesotfddr;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_BYTESOTFDDR, reg_debug_counters_bytesotfddr);

    *value = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_BYTESOTFDDR, VALUE, reg_debug_counters_bytesotfddr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_cmdotfepsram_get(uint8_t bbh_id, uint8_t index, uint8_t *value)
{
    uint32_t reg_debug_counters_cmdotfepsram;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_CMDOTFEPSRAM, reg_debug_counters_cmdotfepsram);

    *value = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_CMDOTFEPSRAM, VALUE, reg_debug_counters_cmdotfepsram);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_bytesotfepsram_get(uint8_t bbh_id, uint8_t index, uint16_t *value)
{
    uint32_t reg_debug_counters_bytesotfepsram;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_BYTESOTFEPSRAM, reg_debug_counters_bytesotfepsram);

    *value = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_BYTESOTFEPSRAM, VALUE, reg_debug_counters_bytesotfepsram);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_gnext_get(uint8_t bbh_id, uint8_t index, uint32_t *entry)
{
    uint32_t reg_debug_counters_gnext;

#ifdef VALIDATE_PARMS
    if (!entry)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 64))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_GNEXT, reg_debug_counters_gnext);

    *entry = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_GNEXT, ENTRY, reg_debug_counters_gnext);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pdarray_get(uint8_t bbh_id, uint8_t index, uint32_t *entry)
{
    uint32_t reg_debug_counters_pdarray;

#ifdef VALIDATE_PARMS
    if (!entry)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PDARRAY, reg_debug_counters_pdarray);

    *entry = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PDARRAY, ENTRY, reg_debug_counters_pdarray);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_curpd_get(uint8_t bbh_id, uint8_t index, uint32_t *entry)
{
    uint32_t reg_debug_counters_curpd;

#ifdef VALIDATE_PARMS
    if (!entry)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_CURPD, reg_debug_counters_curpd);

    *entry = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_CURPD, ENTRY, reg_debug_counters_curpd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_space4req_get(uint8_t bbh_id, uint8_t *resps, uint8_t *target_mem, uint8_t *fe, uint8_t *dp)
{
    uint32_t reg_debug_counters_space4req;

#ifdef VALIDATE_PARMS
    if (!resps || !target_mem || !fe || !dp)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SPACE4REQ, reg_debug_counters_space4req);

    *resps = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SPACE4REQ, RESPS, reg_debug_counters_space4req);
    *target_mem = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SPACE4REQ, TARGET_MEM, reg_debug_counters_space4req);
    *fe = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SPACE4REQ, FE, reg_debug_counters_space4req);
    *dp = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SPACE4REQ, DP, reg_debug_counters_space4req);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_seginfo_get(uint8_t bbh_id, uint8_t *pdempty, uint8_t *gnrdy0, uint8_t *gnrdy1, uint8_t *qrdy)
{
    uint32_t reg_debug_counters_seginfo;

#ifdef VALIDATE_PARMS
    if (!pdempty || !gnrdy0 || !gnrdy1 || !qrdy)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SEGINFO, reg_debug_counters_seginfo);

    *pdempty = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SEGINFO, PDEMPTY, reg_debug_counters_seginfo);
    *gnrdy0 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SEGINFO, GNRDY0, reg_debug_counters_seginfo);
    *gnrdy1 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SEGINFO, GNRDY1, reg_debug_counters_seginfo);
    *qrdy = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SEGINFO, QRDY, reg_debug_counters_seginfo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ehdatval_get(uint8_t bbh_id, uint8_t *val)
{
    uint32_t reg_debug_counters_ehdatval;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_EHDATVAL, reg_debug_counters_ehdatval);

    *val = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_EHDATVAL, VAL, reg_debug_counters_ehdatval);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ehdat_get(uint8_t bbh_id, uint8_t index, uint32_t *data)
{
    uint32_t reg_debug_counters_ehdat;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_EHDAT, reg_debug_counters_ehdat);

    *data = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_EHDAT, DATA, reg_debug_counters_ehdat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rmtpdocc_get(uint8_t bbh_id, uint16_t *occ1, uint16_t *occ2, uint16_t *occ3)
{
    uint32_t reg_debug_counters_rmtpdocc;

#ifdef VALIDATE_PARMS
    if (!occ1 || !occ2 || !occ3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RMTPDOCC, reg_debug_counters_rmtpdocc);

    *occ1 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RMTPDOCC, OCC1, reg_debug_counters_rmtpdocc);
    *occ2 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RMTPDOCC, OCC2, reg_debug_counters_rmtpdocc);
    *occ3 = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RMTPDOCC, OCC3, reg_debug_counters_rmtpdocc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rmtbytes_get(uint8_t bbh_id, uint32_t *val)
{
    uint32_t reg_debug_counters_rmtbytes;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RMTBYTES, reg_debug_counters_rmtbytes);

    *val = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RMTBYTES, VAL, reg_debug_counters_rmtbytes);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rmtlbcnt_get(uint8_t bbh_id, uint8_t index, uint32_t *val)
{
    uint32_t reg_debug_counters_rmtlbcnt;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RMTLBCNT, reg_debug_counters_rmtlbcnt);

    *val = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RMTLBCNT, VAL, reg_debug_counters_rmtlbcnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_respspd_get(uint8_t bbh_id, uint8_t index, uint8_t *credit)
{
    uint32_t reg_debug_counters_respspd;

#ifdef VALIDATE_PARMS
    if (!credit)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 24))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RESPSPD, reg_debug_counters_respspd);

    *credit = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RESPSPD, CREDIT, reg_debug_counters_respspd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_respsdata_get(uint8_t bbh_id, uint8_t index, uint8_t *credit)
{
    uint32_t reg_debug_counters_respsdata;

#ifdef VALIDATE_PARMS
    if (!credit)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 24))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RESPSDATA, reg_debug_counters_respsdata);

    *credit = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RESPSDATA, CREDIT, reg_debug_counters_respsdata);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rnrsmpdcnt_get(uint8_t bbh_id, uint8_t index, uint32_t *value)
{
    uint32_t reg_debug_counters_rnrsmpdcnt;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RNRSMPDCNT, reg_debug_counters_rnrsmpdcnt);

    *value = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RNRSMPDCNT, VALUE, reg_debug_counters_rnrsmpdcnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rnrsmbytecnt_get(uint8_t bbh_id, uint8_t index, uint32_t *value)
{
    uint32_t reg_debug_counters_rnrsmbytecnt;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RNRSMBYTECNT, reg_debug_counters_rnrsmbytecnt);

    *value = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RNRSMBYTECNT, VALUE, reg_debug_counters_rnrsmbytecnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_portdbg_get(uint8_t bbh_id, uint8_t index, uint32_t *data)
{
    uint32_t reg_debug_counters_portdbg;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 24))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PORTDBG, reg_debug_counters_portdbg);

    *data = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PORTDBG, DATA, reg_debug_counters_portdbg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_respdbg_get(uint8_t bbh_id, uint8_t index, uint32_t *data)
{
    uint32_t reg_debug_counters_respdbg;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 24))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RESPDBG, reg_debug_counters_respdbg);

    *data = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RESPDBG, DATA, reg_debug_counters_respdbg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0_set(uint8_t bbh_id, uint8_t index, uint16_t flow, uint16_t gem, bdmf_boolean valid)
{
    uint32_t reg_idx_to_gem_translate_table_entry_0 = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (flow >= _13BITS_MAX_VAL_) ||
       (valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_idx_to_gem_translate_table_entry_0 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0, FLOW, reg_idx_to_gem_translate_table_entry_0, flow);
    reg_idx_to_gem_translate_table_entry_0 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0, GEM, reg_idx_to_gem_translate_table_entry_0, gem);
    reg_idx_to_gem_translate_table_entry_0 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0, VALID, reg_idx_to_gem_translate_table_entry_0, valid);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0, reg_idx_to_gem_translate_table_entry_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0_get(uint8_t bbh_id, uint8_t index, uint16_t *flow, uint16_t *gem, bdmf_boolean *valid)
{
    uint32_t reg_idx_to_gem_translate_table_entry_0;

#ifdef VALIDATE_PARMS
    if (!flow || !gem || !valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0, reg_idx_to_gem_translate_table_entry_0);

    *flow = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0, FLOW, reg_idx_to_gem_translate_table_entry_0);
    *gem = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0, GEM, reg_idx_to_gem_translate_table_entry_0);
    *valid = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0, VALID, reg_idx_to_gem_translate_table_entry_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1_set(uint8_t bbh_id, uint8_t index, uint16_t flow, uint16_t gem, bdmf_boolean valid)
{
    uint32_t reg_idx_to_gem_translate_table_entry_1 = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (flow >= _13BITS_MAX_VAL_) ||
       (valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_idx_to_gem_translate_table_entry_1 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1, FLOW, reg_idx_to_gem_translate_table_entry_1, flow);
    reg_idx_to_gem_translate_table_entry_1 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1, GEM, reg_idx_to_gem_translate_table_entry_1, gem);
    reg_idx_to_gem_translate_table_entry_1 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1, VALID, reg_idx_to_gem_translate_table_entry_1, valid);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1, reg_idx_to_gem_translate_table_entry_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1_get(uint8_t bbh_id, uint8_t index, uint16_t *flow, uint16_t *gem, bdmf_boolean *valid)
{
    uint32_t reg_idx_to_gem_translate_table_entry_1;

#ifdef VALIDATE_PARMS
    if (!flow || !gem || !valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1, reg_idx_to_gem_translate_table_entry_1);

    *flow = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1, FLOW, reg_idx_to_gem_translate_table_entry_1);
    *gem = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1, GEM, reg_idx_to_gem_translate_table_entry_1);
    *valid = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1, VALID, reg_idx_to_gem_translate_table_entry_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2_set(uint8_t bbh_id, uint8_t index, uint16_t flow, uint16_t gem, bdmf_boolean valid)
{
    uint32_t reg_idx_to_gem_translate_table_entry_2 = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (flow >= _13BITS_MAX_VAL_) ||
       (valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_idx_to_gem_translate_table_entry_2 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2, FLOW, reg_idx_to_gem_translate_table_entry_2, flow);
    reg_idx_to_gem_translate_table_entry_2 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2, GEM, reg_idx_to_gem_translate_table_entry_2, gem);
    reg_idx_to_gem_translate_table_entry_2 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2, VALID, reg_idx_to_gem_translate_table_entry_2, valid);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2, reg_idx_to_gem_translate_table_entry_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2_get(uint8_t bbh_id, uint8_t index, uint16_t *flow, uint16_t *gem, bdmf_boolean *valid)
{
    uint32_t reg_idx_to_gem_translate_table_entry_2;

#ifdef VALIDATE_PARMS
    if (!flow || !gem || !valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2, reg_idx_to_gem_translate_table_entry_2);

    *flow = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2, FLOW, reg_idx_to_gem_translate_table_entry_2);
    *gem = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2, GEM, reg_idx_to_gem_translate_table_entry_2);
    *valid = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2, VALID, reg_idx_to_gem_translate_table_entry_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3_set(uint8_t bbh_id, uint8_t index, uint16_t flow, uint16_t gem, bdmf_boolean valid)
{
    uint32_t reg_idx_to_gem_translate_table_entry_3 = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (flow >= _13BITS_MAX_VAL_) ||
       (valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_idx_to_gem_translate_table_entry_3 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3, FLOW, reg_idx_to_gem_translate_table_entry_3, flow);
    reg_idx_to_gem_translate_table_entry_3 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3, GEM, reg_idx_to_gem_translate_table_entry_3, gem);
    reg_idx_to_gem_translate_table_entry_3 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3, VALID, reg_idx_to_gem_translate_table_entry_3, valid);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3, reg_idx_to_gem_translate_table_entry_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3_get(uint8_t bbh_id, uint8_t index, uint16_t *flow, uint16_t *gem, bdmf_boolean *valid)
{
    uint32_t reg_idx_to_gem_translate_table_entry_3;

#ifdef VALIDATE_PARMS
    if (!flow || !gem || !valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3, reg_idx_to_gem_translate_table_entry_3);

    *flow = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3, FLOW, reg_idx_to_gem_translate_table_entry_3);
    *gem = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3, GEM, reg_idx_to_gem_translate_table_entry_3);
    *valid = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3, VALID, reg_idx_to_gem_translate_table_entry_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4_set(uint8_t bbh_id, uint8_t index, uint16_t flow, uint16_t gem, bdmf_boolean valid)
{
    uint32_t reg_idx_to_gem_translate_table_entry_4 = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (flow >= _13BITS_MAX_VAL_) ||
       (valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_idx_to_gem_translate_table_entry_4 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4, FLOW, reg_idx_to_gem_translate_table_entry_4, flow);
    reg_idx_to_gem_translate_table_entry_4 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4, GEM, reg_idx_to_gem_translate_table_entry_4, gem);
    reg_idx_to_gem_translate_table_entry_4 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4, VALID, reg_idx_to_gem_translate_table_entry_4, valid);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4, reg_idx_to_gem_translate_table_entry_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4_get(uint8_t bbh_id, uint8_t index, uint16_t *flow, uint16_t *gem, bdmf_boolean *valid)
{
    uint32_t reg_idx_to_gem_translate_table_entry_4;

#ifdef VALIDATE_PARMS
    if (!flow || !gem || !valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4, reg_idx_to_gem_translate_table_entry_4);

    *flow = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4, FLOW, reg_idx_to_gem_translate_table_entry_4);
    *gem = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4, GEM, reg_idx_to_gem_translate_table_entry_4);
    *valid = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4, VALID, reg_idx_to_gem_translate_table_entry_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5_set(uint8_t bbh_id, uint8_t index, uint16_t flow, uint16_t gem, bdmf_boolean valid)
{
    uint32_t reg_idx_to_gem_translate_table_entry_5 = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (flow >= _13BITS_MAX_VAL_) ||
       (valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_idx_to_gem_translate_table_entry_5 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5, FLOW, reg_idx_to_gem_translate_table_entry_5, flow);
    reg_idx_to_gem_translate_table_entry_5 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5, GEM, reg_idx_to_gem_translate_table_entry_5, gem);
    reg_idx_to_gem_translate_table_entry_5 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5, VALID, reg_idx_to_gem_translate_table_entry_5, valid);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5, reg_idx_to_gem_translate_table_entry_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5_get(uint8_t bbh_id, uint8_t index, uint16_t *flow, uint16_t *gem, bdmf_boolean *valid)
{
    uint32_t reg_idx_to_gem_translate_table_entry_5;

#ifdef VALIDATE_PARMS
    if (!flow || !gem || !valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5, reg_idx_to_gem_translate_table_entry_5);

    *flow = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5, FLOW, reg_idx_to_gem_translate_table_entry_5);
    *gem = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5, GEM, reg_idx_to_gem_translate_table_entry_5);
    *valid = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5, VALID, reg_idx_to_gem_translate_table_entry_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6_set(uint8_t bbh_id, uint8_t index, uint16_t flow, uint16_t gem, bdmf_boolean valid)
{
    uint32_t reg_idx_to_gem_translate_table_entry_6 = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (flow >= _13BITS_MAX_VAL_) ||
       (valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_idx_to_gem_translate_table_entry_6 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6, FLOW, reg_idx_to_gem_translate_table_entry_6, flow);
    reg_idx_to_gem_translate_table_entry_6 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6, GEM, reg_idx_to_gem_translate_table_entry_6, gem);
    reg_idx_to_gem_translate_table_entry_6 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6, VALID, reg_idx_to_gem_translate_table_entry_6, valid);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6, reg_idx_to_gem_translate_table_entry_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6_get(uint8_t bbh_id, uint8_t index, uint16_t *flow, uint16_t *gem, bdmf_boolean *valid)
{
    uint32_t reg_idx_to_gem_translate_table_entry_6;

#ifdef VALIDATE_PARMS
    if (!flow || !gem || !valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6, reg_idx_to_gem_translate_table_entry_6);

    *flow = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6, FLOW, reg_idx_to_gem_translate_table_entry_6);
    *gem = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6, GEM, reg_idx_to_gem_translate_table_entry_6);
    *valid = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6, VALID, reg_idx_to_gem_translate_table_entry_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7_set(uint8_t bbh_id, uint8_t index, uint16_t flow, uint16_t gem, bdmf_boolean valid)
{
    uint32_t reg_idx_to_gem_translate_table_entry_7 = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8) ||
       (flow >= _13BITS_MAX_VAL_) ||
       (valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_idx_to_gem_translate_table_entry_7 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7, FLOW, reg_idx_to_gem_translate_table_entry_7, flow);
    reg_idx_to_gem_translate_table_entry_7 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7, GEM, reg_idx_to_gem_translate_table_entry_7, gem);
    reg_idx_to_gem_translate_table_entry_7 = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7, VALID, reg_idx_to_gem_translate_table_entry_7, valid);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7, reg_idx_to_gem_translate_table_entry_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7_get(uint8_t bbh_id, uint8_t index, uint16_t *flow, uint16_t *gem, bdmf_boolean *valid)
{
    uint32_t reg_idx_to_gem_translate_table_entry_7;

#ifdef VALIDATE_PARMS
    if (!flow || !gem || !valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7, reg_idx_to_gem_translate_table_entry_7);

    *flow = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7, FLOW, reg_idx_to_gem_translate_table_entry_7);
    *gem = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7, GEM, reg_idx_to_gem_translate_table_entry_7);
    *valid = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7, VALID, reg_idx_to_gem_translate_table_entry_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val_set(uint8_t bbh_id, uint8_t index, uint32_t val)
{
    uint32_t reg_gem_trans_tbl_gem_trns_val = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 128))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gem_trans_tbl_gem_trns_val = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, GEM_TRANS_TBL_GEM_TRNS_VAL, VAL, reg_gem_trans_tbl_gem_trns_val, val);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, GEM_TRANS_TBL_GEM_TRNS_VAL, reg_gem_trans_tbl_gem_trns_val);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val_get(uint8_t bbh_id, uint8_t index, uint32_t *val)
{
    uint32_t reg_gem_trans_tbl_gem_trns_val;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 128))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, GEM_TRANS_TBL_GEM_TRNS_VAL, reg_gem_trans_tbl_gem_trns_val);

    *val = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, GEM_TRANS_TBL_GEM_TRNS_VAL, VAL, reg_gem_trans_tbl_gem_trns_val);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en_set(uint8_t bbh_id, uint8_t index, uint32_t enable)
{
    uint32_t reg_mac_flow_en_table_gem_en = 0;

#ifdef VALIDATE_PARMS
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 2048))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_flow_en_table_gem_en = RU_FIELD_SET(bbh_id, CRDP_DS_CMN_BBH_TX, MAC_FLOW_EN_TABLE_GEM_EN, ENABLE, reg_mac_flow_en_table_gem_en, enable);

    RU_REG_RAM_WRITE(bbh_id, index, CRDP_DS_CMN_BBH_TX, MAC_FLOW_EN_TABLE_GEM_EN, reg_mac_flow_en_table_gem_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en_get(uint8_t bbh_id, uint8_t index, uint32_t *enable)
{
    uint32_t reg_mac_flow_en_table_gem_en;

#ifdef VALIDATE_PARMS
    if (!enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((bbh_id >= BLOCK_ADDR_COUNT) ||
       (index >= 2048))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, index, CRDP_DS_CMN_BBH_TX, MAC_FLOW_EN_TABLE_GEM_EN, reg_mac_flow_en_table_gem_en);

    *enable = RU_FIELD_GET(bbh_id, CRDP_DS_CMN_BBH_TX, MAC_FLOW_EN_TABLE_GEM_EN, ENABLE, reg_mac_flow_en_table_gem_en);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_common_configurations_mactype,
    bdmf_address_common_configurations_bbcfg_tx,
    bdmf_address_common_configurations_sbpmcfg,
    bdmf_address_common_configurations_bbsrcid,
    bdmf_address_common_configurations_ddrcfg_tx,
    bdmf_address_common_configurations_pdrnrcfg,
    bdmf_address_common_configurations_txrstcmd,
    bdmf_address_common_configurations_dbgsel,
    bdmf_address_common_configurations_clk_gate_cntrl,
    bdmf_address_common_configurations_gpr,
    bdmf_address_common_configurations_general_cfg,
    bdmf_address_common_configurations_ecncfg,
    bdmf_address_common_configurations_ddrfpminibasel,
    bdmf_address_common_configurations_ddrfpminibaseh,
    bdmf_address_common_configurations_bbovrdroute,
    bdmf_address_common_configurations_bbovrdctrl,
    bdmf_address_common_configurations_tmbasel,
    bdmf_address_common_configurations_tmbaseh,
    bdmf_address_common_configurations_reorderfifosize,
    bdmf_address_common_configurations_ubuscredits,
    bdmf_address_common_configurations_creditinit,
    bdmf_address_common_configurations_mirrorrnrcfg,
    bdmf_address_common_configurations_mirrorcfg,
    bdmf_address_common_configurations_multibbhcfg,
    bdmf_address_common_configurations_multibbsrc,
    bdmf_address_common_configurations_addr_spreader,
    bdmf_address_common_configurations_flowctrcfg,
    bdmf_address_common_configurations_fcrnrcfg,
    bdmf_address_common_configurations_fccfg,
    bdmf_address_per_port_configurations_pdack,
    bdmf_address_per_port_configurations_pdsize,
    bdmf_address_per_port_configurations_pdwkuph,
    bdmf_address_per_port_configurations_pdempty,
    bdmf_address_per_port_configurations_pd_byte_th,
    bdmf_address_per_port_configurations_max_pd_th,
    bdmf_address_per_port_configurations_pd_byte_th_en,
    bdmf_address_per_port_configurations_flowtogemoffset,
    bdmf_address_per_port_configurations_flowtogemthr,
    bdmf_address_per_port_configurations_febase,
    bdmf_address_per_port_configurations_fesize,
    bdmf_address_per_port_configurations_fepdbase,
    bdmf_address_per_port_configurations_fepdsize,
    bdmf_address_per_port_configurations_sgmtwrr,
    bdmf_address_per_port_configurations_fe_credits,
    bdmf_address_per_port_configurations_gem_en_init,
    bdmf_address_per_port_configurations_gem_trns_init,
    bdmf_address_per_port_configurations_otfbytes,
    bdmf_address_per_port_configurations_otfcmd,
    bdmf_address_per_port_configurations_autootfcredits,
    bdmf_address_debug_counters_txsram0pd,
    bdmf_address_debug_counters_txsram1pd,
    bdmf_address_debug_counters_txddrpd,
    bdmf_address_debug_counters_pddrop,
    bdmf_address_debug_counters_getnextnull,
    bdmf_address_debug_counters_getnextdrop,
    bdmf_address_debug_counters_inputfifodrop,
    bdmf_address_debug_counters_sram0pkt,
    bdmf_address_debug_counters_sram1pkt,
    bdmf_address_debug_counters_ddrpkt,
    bdmf_address_debug_counters_txsram0byte,
    bdmf_address_debug_counters_txsram1byte,
    bdmf_address_debug_counters_txddrbyte,
    bdmf_address_debug_counters_swrden,
    bdmf_address_debug_counters_swrdaddr,
    bdmf_address_debug_counters_swrddata,
    bdmf_address_debug_counters_unifiedpkt,
    bdmf_address_debug_counters_unifiedbyte,
    bdmf_address_debug_counters_dbgoutreg,
    bdmf_address_debug_counters_in_segmentation,
    bdmf_address_debug_counters_unified_data_credits,
    bdmf_address_debug_counters_unified_pd_credits,
    bdmf_address_debug_counters_disflowdroppkt,
    bdmf_address_debug_counters_disflowdropbyte,
    bdmf_address_debug_counters_ecnpkt,
    bdmf_address_debug_counters_ecnbytes,
    bdmf_address_debug_counters_ecnmarkerr,
    bdmf_address_debug_counters_ecnlenerr,
    bdmf_address_debug_counters_flush_info,
    bdmf_address_debug_counters_flush_info2,
    bdmf_address_debug_counters_pd_eh_mismatch,
    bdmf_address_debug_counters_pd_eh_mismatch_data,
    bdmf_address_debug_counters_outofrange_errors,
    bdmf_address_debug_counters_gemdroppkt,
    bdmf_address_debug_counters_gemdropbytes,
    bdmf_address_debug_counters_cmdotfddr,
    bdmf_address_debug_counters_bytesotfddr,
    bdmf_address_debug_counters_cmdotfepsram,
    bdmf_address_debug_counters_bytesotfepsram,
    bdmf_address_debug_counters_gnext,
    bdmf_address_debug_counters_pdarray,
    bdmf_address_debug_counters_curpd,
    bdmf_address_debug_counters_space4req,
    bdmf_address_debug_counters_seginfo,
    bdmf_address_debug_counters_ehdatval,
    bdmf_address_debug_counters_ehdat,
    bdmf_address_debug_counters_rmtpdocc,
    bdmf_address_debug_counters_rmtbytes,
    bdmf_address_debug_counters_rmtlbcnt,
    bdmf_address_debug_counters_respspd,
    bdmf_address_debug_counters_respsdata,
    bdmf_address_debug_counters_rnrsmpdcnt,
    bdmf_address_debug_counters_rnrsmbytecnt,
    bdmf_address_debug_counters_portdbg,
    bdmf_address_debug_counters_respdbg,
    bdmf_address_idx_to_gem_translate_table_entry_0,
    bdmf_address_idx_to_gem_translate_table_entry_1,
    bdmf_address_idx_to_gem_translate_table_entry_2,
    bdmf_address_idx_to_gem_translate_table_entry_3,
    bdmf_address_idx_to_gem_translate_table_entry_4,
    bdmf_address_idx_to_gem_translate_table_entry_5,
    bdmf_address_idx_to_gem_translate_table_entry_6,
    bdmf_address_idx_to_gem_translate_table_entry_7,
    bdmf_address_gem_trans_tbl_gem_trns_val,
    bdmf_address_mac_flow_en_table_gem_en,
}
bdmf_address;

static int ag_drv_crdp_ds_cmn_bbh_tx_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_crdp_ds_cmn_bbh_tx_mac_type:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_mac_type_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_bbcfg_tx:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbcfg_tx_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg:
    {
        crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg common_configurations_sbpmcfg = { .freenocntxt = parm[2].value.unumber, .mc_type_3 = parm[3].value.unumber, .mc_type_2 = parm[4].value.unumber, .second_bn_dis = parm[5].value.unumber, .bufsize = parm[6].value.unumber, .enrrspread = parm[7].value.unumber, .maxgn = parm[8].value.unumber, .listfreetimer = parm[9].value.unumber, .listfreethr = parm[10].value.unumber, .smid = parm[11].value.unumber};
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg_set(parm[1].value.unumber, &common_configurations_sbpmcfg);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_bbsrcid:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbsrcid_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrcfg_tx:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrcfg_tx_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd:
    {
        crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd common_configurations_txrstcmd = { .cntxtrst = parm[2].value.unumber, .pdfiforst = parm[3].value.unumber, .bpmfiforst = parm[4].value.unumber, .sbpmfiforst = parm[5].value.unumber, .ddrfiforst = parm[6].value.unumber, .sramfiforst = parm[7].value.unumber, .skbptrrst = parm[8].value.unumber, .gnxtfiforst = parm[9].value.unumber, .fbnfiforst = parm[10].value.unumber, .port = parm[11].value.unumber};
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd_set(parm[1].value.unumber, &common_configurations_txrstcmd);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_dbgsel:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_dbgsel_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl:
    {
        crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl common_configurations_clk_gate_cntrl = { .bypass_clk_gate = parm[2].value.unumber, .timer_val = parm[3].value.unumber, .keep_alive_en = parm[4].value.unumber, .keep_alive_intrvl = parm[5].value.unumber, .keep_alive_cyc = parm[6].value.unumber};
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl_set(parm[1].value.unumber, &common_configurations_clk_gate_cntrl);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_gpr:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_gpr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_general_cfg:
    {
        crdp_ds_cmn_bbh_tx_common_configurations_general_cfg common_configurations_general_cfg = { .macdrop = parm[2].value.unumber, .discopycrdt = parm[3].value.unumber, .ecnrmrk = parm[4].value.unumber, .cntraddcrc = parm[5].value.unumber, .crcforward = parm[6].value.unumber, .multi_tcont = parm[7].value.unumber, .distargetperport = parm[8].value.unumber, .discrdtchk = parm[9].value.unumber};
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_general_cfg_set(parm[1].value.unumber, &common_configurations_general_cfg);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_ecncfg:
    {
        crdp_ds_cmn_bbh_tx_common_configurations_ecncfg common_configurations_ecncfg = { .ecnbyteipv4 = parm[2].value.unumber, .ecnbitipv4 = parm[3].value.unumber, .ecnbyteipv6 = parm[4].value.unumber, .ecnbitipv6 = parm[5].value.unumber, .chksumbyteipv4 = parm[6].value.unumber, .assignedqcnt = parm[7].value.unumber};
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ecncfg_set(parm[1].value.unumber, &common_configurations_ecncfg);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdroute:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdroute_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdctrl:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_creditinit:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_creditinit_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_multibbhcfg:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbhcfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader:
    {
        crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader common_configurations_addr_spreader = { .num_active_dram = parm[2].value.unumber, .en_ddr = parm[3].value.unumber, .en_epsr = parm[4].value.unumber, .simple_spread = parm[5].value.unumber, .active_dram_0 = parm[6].value.unumber, .active_dram_1 = parm[7].value.unumber, .block_spread_zone_en = parm[8].value.unumber, .block_spread_zone = parm[9].value.unumber, .spare = parm[10].value.unumber};
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader_set(parm[1].value.unumber, &common_configurations_addr_spreader);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_flowctrcfg:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_flowctrcfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_fccfg:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fccfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdack:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdack_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_en:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_en_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_febase:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_febase_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fe_credits:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fe_credits_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_en_init:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_en_init_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_trns_init:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_trns_init_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_autootfcredits:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_autootfcredits_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_swrden:
    {
        crdp_ds_cmn_bbh_tx_debug_counters_swrden debug_counters_swrden = { .pdsel = parm[2].value.unumber, .pdvsel = parm[3].value.unumber, .pdemptysel = parm[4].value.unumber, .pdfullsel = parm[5].value.unumber, .pdbemptysel = parm[6].value.unumber, .pdffwkpsel = parm[7].value.unumber, .fbn_0_sel = parm[8].value.unumber, .fbnv_0_sel = parm[9].value.unumber, .fbnempty_0_sel = parm[10].value.unumber, .fbnfull_0_sel = parm[11].value.unumber, .getnext_0_sel = parm[12].value.unumber, .getnextv_0_sel = parm[13].value.unumber, .getnextempty_0_sel = parm[14].value.unumber, .getnextfull_0_sel = parm[15].value.unumber, .fpm_0_sel = parm[16].value.unumber, .sbpm_0_sel = parm[17].value.unumber, .fbnv_1_sel = parm[18].value.unumber, .fbnempty_1_sel = parm[19].value.unumber, .fbnfull_1_sel = parm[20].value.unumber, .getnext_1_sel = parm[21].value.unumber, .getnextv_1_sel = parm[22].value.unumber, .getnextempty_1_sel = parm[23].value.unumber, .getnextfull_1_sel = parm[24].value.unumber, .fpm_1_sel = parm[25].value.unumber, .sbpm_1_sel = parm[26].value.unumber, .fbn_1_sel = parm[27].value.unumber};
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrden_set(parm[1].value.unumber, &debug_counters_swrden);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_swrdaddr:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrdaddr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en:
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_crdp_ds_cmn_bbh_tx_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_crdp_ds_cmn_bbh_tx_mac_type:
    {
        uint8_t type;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_mac_type_get(parm[1].value.unumber, parm[2].value.unumber, &type);
        bdmf_session_print(session, "type = %u = 0x%x\n", type, type);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_ddrbyte:
    {
        uint32_t ddrbyte;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ddrbyte_get(parm[1].value.unumber, &ddrbyte);
        bdmf_session_print(session, "ddrbyte = %u = 0x%x\n", ddrbyte, ddrbyte);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_sram0byte:
    {
        uint32_t srambyte;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram0byte_get(parm[1].value.unumber, &srambyte);
        bdmf_session_print(session, "srambyte = %u = 0x%x\n", srambyte, srambyte);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_sram1byte:
    {
        uint32_t srambyte;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram1byte_get(parm[1].value.unumber, &srambyte);
        bdmf_session_print(session, "srambyte = %u = 0x%x\n", srambyte, srambyte);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_bbcfg_tx:
    {
        uint16_t sbpmsrc;
        uint16_t fpmsrc;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbcfg_tx_get(parm[1].value.unumber, &sbpmsrc, &fpmsrc);
        bdmf_session_print(session, "sbpmsrc = %u = 0x%x\n", sbpmsrc, sbpmsrc);
        bdmf_session_print(session, "fpmsrc = %u = 0x%x\n", fpmsrc, fpmsrc);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg:
    {
        crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg common_configurations_sbpmcfg;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg_get(parm[1].value.unumber, &common_configurations_sbpmcfg);
        bdmf_session_print(session, "freenocntxt = %u = 0x%x\n", common_configurations_sbpmcfg.freenocntxt, common_configurations_sbpmcfg.freenocntxt);
        bdmf_session_print(session, "mc_type_3 = %u = 0x%x\n", common_configurations_sbpmcfg.mc_type_3, common_configurations_sbpmcfg.mc_type_3);
        bdmf_session_print(session, "mc_type_2 = %u = 0x%x\n", common_configurations_sbpmcfg.mc_type_2, common_configurations_sbpmcfg.mc_type_2);
        bdmf_session_print(session, "second_bn_dis = %u = 0x%x\n", common_configurations_sbpmcfg.second_bn_dis, common_configurations_sbpmcfg.second_bn_dis);
        bdmf_session_print(session, "bufsize = %u = 0x%x\n", common_configurations_sbpmcfg.bufsize, common_configurations_sbpmcfg.bufsize);
        bdmf_session_print(session, "enrrspread = %u = 0x%x\n", common_configurations_sbpmcfg.enrrspread, common_configurations_sbpmcfg.enrrspread);
        bdmf_session_print(session, "maxgn = %u = 0x%x\n", common_configurations_sbpmcfg.maxgn, common_configurations_sbpmcfg.maxgn);
        bdmf_session_print(session, "listfreetimer = %u = 0x%x\n", common_configurations_sbpmcfg.listfreetimer, common_configurations_sbpmcfg.listfreetimer);
        bdmf_session_print(session, "listfreethr = %u = 0x%x\n", common_configurations_sbpmcfg.listfreethr, common_configurations_sbpmcfg.listfreethr);
        bdmf_session_print(session, "smid = %u = 0x%x\n", common_configurations_sbpmcfg.smid, common_configurations_sbpmcfg.smid);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_bbsrcid:
    {
        uint16_t bbsrcid;
        bdmf_boolean byp;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbsrcid_get(parm[1].value.unumber, &bbsrcid, &byp);
        bdmf_session_print(session, "bbsrcid = %u = 0x%x\n", bbsrcid, bbsrcid);
        bdmf_session_print(session, "byp = %u = 0x%x\n", byp, byp);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrcfg_tx:
    {
        uint8_t bufsize0;
        uint8_t bufsize1;
        uint8_t hnsize;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrcfg_tx_get(parm[1].value.unumber, &bufsize0, &bufsize1, &hnsize);
        bdmf_session_print(session, "bufsize0 = %u = 0x%x\n", bufsize0, bufsize0);
        bdmf_session_print(session, "bufsize1 = %u = 0x%x\n", bufsize1, bufsize1);
        bdmf_session_print(session, "hnsize = %u = 0x%x\n", hnsize, hnsize);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg:
    {
        uint16_t wkpaddr;
        uint8_t task;
        uint16_t bbsrcid;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg_get(parm[1].value.unumber, parm[2].value.unumber, &wkpaddr, &task, &bbsrcid);
        bdmf_session_print(session, "wkpaddr = %u = 0x%x\n", wkpaddr, wkpaddr);
        bdmf_session_print(session, "task = %u = 0x%x\n", task, task);
        bdmf_session_print(session, "bbsrcid = %u = 0x%x\n", bbsrcid, bbsrcid);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd:
    {
        crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd common_configurations_txrstcmd;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd_get(parm[1].value.unumber, &common_configurations_txrstcmd);
        bdmf_session_print(session, "cntxtrst = %u = 0x%x\n", common_configurations_txrstcmd.cntxtrst, common_configurations_txrstcmd.cntxtrst);
        bdmf_session_print(session, "pdfiforst = %u = 0x%x\n", common_configurations_txrstcmd.pdfiforst, common_configurations_txrstcmd.pdfiforst);
        bdmf_session_print(session, "bpmfiforst = %u = 0x%x\n", common_configurations_txrstcmd.bpmfiforst, common_configurations_txrstcmd.bpmfiforst);
        bdmf_session_print(session, "sbpmfiforst = %u = 0x%x\n", common_configurations_txrstcmd.sbpmfiforst, common_configurations_txrstcmd.sbpmfiforst);
        bdmf_session_print(session, "ddrfiforst = %u = 0x%x\n", common_configurations_txrstcmd.ddrfiforst, common_configurations_txrstcmd.ddrfiforst);
        bdmf_session_print(session, "sramfiforst = %u = 0x%x\n", common_configurations_txrstcmd.sramfiforst, common_configurations_txrstcmd.sramfiforst);
        bdmf_session_print(session, "skbptrrst = %u = 0x%x\n", common_configurations_txrstcmd.skbptrrst, common_configurations_txrstcmd.skbptrrst);
        bdmf_session_print(session, "gnxtfiforst = %u = 0x%x\n", common_configurations_txrstcmd.gnxtfiforst, common_configurations_txrstcmd.gnxtfiforst);
        bdmf_session_print(session, "fbnfiforst = %u = 0x%x\n", common_configurations_txrstcmd.fbnfiforst, common_configurations_txrstcmd.fbnfiforst);
        bdmf_session_print(session, "port = %u = 0x%x\n", common_configurations_txrstcmd.port, common_configurations_txrstcmd.port);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_dbgsel:
    {
        uint8_t dbgsel;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_dbgsel_get(parm[1].value.unumber, &dbgsel);
        bdmf_session_print(session, "dbgsel = %u = 0x%x\n", dbgsel, dbgsel);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl:
    {
        crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl common_configurations_clk_gate_cntrl;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl_get(parm[1].value.unumber, &common_configurations_clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u = 0x%x\n", common_configurations_clk_gate_cntrl.bypass_clk_gate, common_configurations_clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u = 0x%x\n", common_configurations_clk_gate_cntrl.timer_val, common_configurations_clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u = 0x%x\n", common_configurations_clk_gate_cntrl.keep_alive_en, common_configurations_clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intrvl = %u = 0x%x\n", common_configurations_clk_gate_cntrl.keep_alive_intrvl, common_configurations_clk_gate_cntrl.keep_alive_intrvl);
        bdmf_session_print(session, "keep_alive_cyc = %u = 0x%x\n", common_configurations_clk_gate_cntrl.keep_alive_cyc, common_configurations_clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_gpr:
    {
        uint32_t gpr;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_gpr_get(parm[1].value.unumber, &gpr);
        bdmf_session_print(session, "gpr = %u = 0x%x\n", gpr, gpr);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_general_cfg:
    {
        crdp_ds_cmn_bbh_tx_common_configurations_general_cfg common_configurations_general_cfg;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_general_cfg_get(parm[1].value.unumber, &common_configurations_general_cfg);
        bdmf_session_print(session, "macdrop = %u = 0x%x\n", common_configurations_general_cfg.macdrop, common_configurations_general_cfg.macdrop);
        bdmf_session_print(session, "discopycrdt = %u = 0x%x\n", common_configurations_general_cfg.discopycrdt, common_configurations_general_cfg.discopycrdt);
        bdmf_session_print(session, "ecnrmrk = %u = 0x%x\n", common_configurations_general_cfg.ecnrmrk, common_configurations_general_cfg.ecnrmrk);
        bdmf_session_print(session, "cntraddcrc = %u = 0x%x\n", common_configurations_general_cfg.cntraddcrc, common_configurations_general_cfg.cntraddcrc);
        bdmf_session_print(session, "crcforward = %u = 0x%x\n", common_configurations_general_cfg.crcforward, common_configurations_general_cfg.crcforward);
        bdmf_session_print(session, "multi_tcont = %u = 0x%x\n", common_configurations_general_cfg.multi_tcont, common_configurations_general_cfg.multi_tcont);
        bdmf_session_print(session, "distargetperport = %u = 0x%x\n", common_configurations_general_cfg.distargetperport, common_configurations_general_cfg.distargetperport);
        bdmf_session_print(session, "discrdtchk = %u = 0x%x\n", common_configurations_general_cfg.discrdtchk, common_configurations_general_cfg.discrdtchk);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_ecncfg:
    {
        crdp_ds_cmn_bbh_tx_common_configurations_ecncfg common_configurations_ecncfg;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ecncfg_get(parm[1].value.unumber, &common_configurations_ecncfg);
        bdmf_session_print(session, "ecnbyteipv4 = %u = 0x%x\n", common_configurations_ecncfg.ecnbyteipv4, common_configurations_ecncfg.ecnbyteipv4);
        bdmf_session_print(session, "ecnbitipv4 = %u = 0x%x\n", common_configurations_ecncfg.ecnbitipv4, common_configurations_ecncfg.ecnbitipv4);
        bdmf_session_print(session, "ecnbyteipv6 = %u = 0x%x\n", common_configurations_ecncfg.ecnbyteipv6, common_configurations_ecncfg.ecnbyteipv6);
        bdmf_session_print(session, "ecnbitipv6 = %u = 0x%x\n", common_configurations_ecncfg.ecnbitipv6, common_configurations_ecncfg.ecnbitipv6);
        bdmf_session_print(session, "chksumbyteipv4 = %u = 0x%x\n", common_configurations_ecncfg.chksumbyteipv4, common_configurations_ecncfg.chksumbyteipv4);
        bdmf_session_print(session, "assignedqcnt = %u = 0x%x\n", common_configurations_ecncfg.assignedqcnt, common_configurations_ecncfg.assignedqcnt);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel:
    {
        uint32_t ddrfpminibase;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel_get(parm[1].value.unumber, parm[2].value.unumber, &ddrfpminibase);
        bdmf_session_print(session, "ddrfpminibase = %u = 0x%x\n", ddrfpminibase, ddrfpminibase);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh:
    {
        uint8_t ddrfpminibase;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh_get(parm[1].value.unumber, parm[2].value.unumber, &ddrfpminibase);
        bdmf_session_print(session, "ddrfpminibase = %u = 0x%x\n", ddrfpminibase, ddrfpminibase);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdroute:
    {
        uint32_t route;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdroute_get(parm[1].value.unumber, &route);
        bdmf_session_print(session, "route = %u = 0x%x\n", route, route);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdctrl:
    {
        uint16_t dest;
        bdmf_boolean en;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdctrl_get(parm[1].value.unumber, &dest, &en);
        bdmf_session_print(session, "dest = %u = 0x%x\n", dest, dest);
        bdmf_session_print(session, "en = %u = 0x%x\n", en, en);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel:
    {
        uint32_t ddrtmbase;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel_get(parm[1].value.unumber, parm[2].value.unumber, &ddrtmbase);
        bdmf_session_print(session, "ddrtmbase = %u = 0x%x\n", ddrtmbase, ddrtmbase);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh:
    {
        uint8_t ddrtmbase;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh_get(parm[1].value.unumber, parm[2].value.unumber, &ddrtmbase);
        bdmf_session_print(session, "ddrtmbase = %u = 0x%x\n", ddrtmbase, ddrtmbase);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize:
    {
        uint16_t size;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize_get(parm[1].value.unumber, parm[2].value.unumber, &size);
        bdmf_session_print(session, "size = %u = 0x%x\n", size, size);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits:
    {
        uint8_t credits;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits_get(parm[1].value.unumber, parm[2].value.unumber, &credits);
        bdmf_session_print(session, "credits = %u = 0x%x\n", credits, credits);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_creditinit:
    {
        uint32_t init;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_creditinit_get(parm[1].value.unumber, &init);
        bdmf_session_print(session, "init = %u = 0x%x\n", init, init);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg:
    {
        uint16_t wkpaddr;
        uint8_t task;
        uint16_t bbsrcid;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg_get(parm[1].value.unumber, parm[2].value.unumber, &wkpaddr, &task, &bbsrcid);
        bdmf_session_print(session, "wkpaddr = %u = 0x%x\n", wkpaddr, wkpaddr);
        bdmf_session_print(session, "task = %u = 0x%x\n", task, task);
        bdmf_session_print(session, "bbsrcid = %u = 0x%x\n", bbsrcid, bbsrcid);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg:
    {
        uint8_t wkpgran;
        bdmf_boolean ptrrst;
        bdmf_boolean enable;
        uint8_t timer;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg_get(parm[1].value.unumber, parm[2].value.unumber, &wkpgran, &ptrrst, &enable, &timer);
        bdmf_session_print(session, "wkpgran = %u = 0x%x\n", wkpgran, wkpgran);
        bdmf_session_print(session, "ptrrst = %u = 0x%x\n", ptrrst, ptrrst);
        bdmf_session_print(session, "enable = %u = 0x%x\n", enable, enable);
        bdmf_session_print(session, "timer = %u = 0x%x\n", timer, timer);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_multibbhcfg:
    {
        uint16_t sninitval;
        bdmf_boolean sninit;
        uint8_t lbctrrst;
        uint8_t mode;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbhcfg_get(parm[1].value.unumber, &sninitval, &sninit, &lbctrrst, &mode);
        bdmf_session_print(session, "sninitval = %u = 0x%x\n", sninitval, sninitval);
        bdmf_session_print(session, "sninit = %u = 0x%x\n", sninit, sninit);
        bdmf_session_print(session, "lbctrrst = %u = 0x%x\n", lbctrrst, lbctrrst);
        bdmf_session_print(session, "mode = %u = 0x%x\n", mode, mode);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc:
    {
        uint16_t bbsrcid;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc_get(parm[1].value.unumber, parm[2].value.unumber, &bbsrcid);
        bdmf_session_print(session, "bbsrcid = %u = 0x%x\n", bbsrcid, bbsrcid);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader:
    {
        crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader common_configurations_addr_spreader;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader_get(parm[1].value.unumber, &common_configurations_addr_spreader);
        bdmf_session_print(session, "num_active_dram = %u = 0x%x\n", common_configurations_addr_spreader.num_active_dram, common_configurations_addr_spreader.num_active_dram);
        bdmf_session_print(session, "en_ddr = %u = 0x%x\n", common_configurations_addr_spreader.en_ddr, common_configurations_addr_spreader.en_ddr);
        bdmf_session_print(session, "en_epsr = %u = 0x%x\n", common_configurations_addr_spreader.en_epsr, common_configurations_addr_spreader.en_epsr);
        bdmf_session_print(session, "simple_spread = %u = 0x%x\n", common_configurations_addr_spreader.simple_spread, common_configurations_addr_spreader.simple_spread);
        bdmf_session_print(session, "active_dram_0 = %u = 0x%x\n", common_configurations_addr_spreader.active_dram_0, common_configurations_addr_spreader.active_dram_0);
        bdmf_session_print(session, "active_dram_1 = %u = 0x%x\n", common_configurations_addr_spreader.active_dram_1, common_configurations_addr_spreader.active_dram_1);
        bdmf_session_print(session, "block_spread_zone_en = %u = 0x%x\n", common_configurations_addr_spreader.block_spread_zone_en, common_configurations_addr_spreader.block_spread_zone_en);
        bdmf_session_print(session, "block_spread_zone = %u = 0x%x\n", common_configurations_addr_spreader.block_spread_zone, common_configurations_addr_spreader.block_spread_zone);
        bdmf_session_print(session, "spare = %u = 0x%x\n", common_configurations_addr_spreader.spare, common_configurations_addr_spreader.spare);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_flowctrcfg:
    {
        bdmf_boolean numports;
        uint8_t numctrs;
        uint8_t macflowcnten;
        uint8_t ctridxcnten;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_flowctrcfg_get(parm[1].value.unumber, &numports, &numctrs, &macflowcnten, &ctridxcnten);
        bdmf_session_print(session, "numports = %u = 0x%x\n", numports, numports);
        bdmf_session_print(session, "numctrs = %u = 0x%x\n", numctrs, numctrs);
        bdmf_session_print(session, "macflowcnten = %u = 0x%x\n", macflowcnten, macflowcnten);
        bdmf_session_print(session, "ctridxcnten = %u = 0x%x\n", ctridxcnten, ctridxcnten);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg:
    {
        uint16_t wkpaddr;
        uint8_t task;
        uint16_t bbsrcid;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg_get(parm[1].value.unumber, parm[2].value.unumber, &wkpaddr, &task, &bbsrcid);
        bdmf_session_print(session, "wkpaddr = %u = 0x%x\n", wkpaddr, wkpaddr);
        bdmf_session_print(session, "task = %u = 0x%x\n", task, task);
        bdmf_session_print(session, "bbsrcid = %u = 0x%x\n", bbsrcid, bbsrcid);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_fccfg:
    {
        uint16_t fc_stblfilter;
        bdmf_boolean fc_en;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fccfg_get(parm[1].value.unumber, &fc_stblfilter, &fc_en);
        bdmf_session_print(session, "fc_stblfilter = %u = 0x%x\n", fc_stblfilter, fc_stblfilter);
        bdmf_session_print(session, "fc_en = %u = 0x%x\n", fc_en, fc_en);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdack:
    {
        uint8_t q;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdack_get(parm[1].value.unumber, &q);
        bdmf_session_print(session, "q = %u = 0x%x\n", q, q);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize:
    {
        uint16_t fifosize;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize_get(parm[1].value.unumber, parm[2].value.unumber, &fifosize);
        bdmf_session_print(session, "fifosize = %u = 0x%x\n", fifosize, fifosize);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph:
    {
        uint16_t wkupthresh;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph_get(parm[1].value.unumber, parm[2].value.unumber, &wkupthresh);
        bdmf_session_print(session, "wkupthresh = %u = 0x%x\n", wkupthresh, wkupthresh);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty:
    {
        uint16_t empty;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty_get(parm[1].value.unumber, parm[2].value.unumber, &empty);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th:
    {
        uint32_t bytelimit;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_get(parm[1].value.unumber, parm[2].value.unumber, &bytelimit);
        bdmf_session_print(session, "bytelimit = %u = 0x%x\n", bytelimit, bytelimit);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th:
    {
        uint16_t pdlimit;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th_get(parm[1].value.unumber, parm[2].value.unumber, &pdlimit);
        bdmf_session_print(session, "pdlimit = %u = 0x%x\n", pdlimit, pdlimit);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_en:
    {
        uint8_t pdbytelimiten;
        uint8_t pdmaxlimiten;
        uint8_t disfesf;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_en_get(parm[1].value.unumber, &pdbytelimiten, &pdmaxlimiten, &disfesf);
        bdmf_session_print(session, "pdbytelimiten = %u = 0x%x\n", pdbytelimiten, pdbytelimiten);
        bdmf_session_print(session, "pdmaxlimiten = %u = 0x%x\n", pdmaxlimiten, pdmaxlimiten);
        bdmf_session_print(session, "disfesf = %u = 0x%x\n", disfesf, disfesf);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset:
    {
        uint16_t offset0;
        uint16_t offset1;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset_get(parm[1].value.unumber, parm[2].value.unumber, &offset0, &offset1);
        bdmf_session_print(session, "offset0 = %u = 0x%x\n", offset0, offset0);
        bdmf_session_print(session, "offset1 = %u = 0x%x\n", offset1, offset1);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr:
    {
        uint16_t thr;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr_get(parm[1].value.unumber, parm[2].value.unumber, &thr);
        bdmf_session_print(session, "thr = %u = 0x%x\n", thr, thr);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_febase:
    {
        uint16_t fifobase0;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_febase_get(parm[1].value.unumber, parm[2].value.unumber, &fifobase0);
        bdmf_session_print(session, "fifobase0 = %u = 0x%x\n", fifobase0, fifobase0);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize:
    {
        uint16_t fifosize;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize_get(parm[1].value.unumber, parm[2].value.unumber, &fifosize);
        bdmf_session_print(session, "fifosize = %u = 0x%x\n", fifosize, fifosize);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase:
    {
        uint16_t fifobase0;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase_get(parm[1].value.unumber, parm[2].value.unumber, &fifobase0);
        bdmf_session_print(session, "fifobase0 = %u = 0x%x\n", fifobase0, fifobase0);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize:
    {
        uint16_t fifosize0;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize_get(parm[1].value.unumber, parm[2].value.unumber, &fifosize0);
        bdmf_session_print(session, "fifosize0 = %u = 0x%x\n", fifosize0, fifosize0);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr:
    {
        uint8_t w0;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr_get(parm[1].value.unumber, parm[2].value.unumber, &w0);
        bdmf_session_print(session, "w0 = %u = 0x%x\n", w0, w0);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fe_credits:
    {
        uint8_t init;
        uint16_t min_pd;
        uint16_t min_data;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fe_credits_get(parm[1].value.unumber, &init, &min_pd, &min_data);
        bdmf_session_print(session, "init = %u = 0x%x\n", init, init);
        bdmf_session_print(session, "min_pd = %u = 0x%x\n", min_pd, min_pd);
        bdmf_session_print(session, "min_data = %u = 0x%x\n", min_data, min_data);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_en_init:
    {
        uint16_t init;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_en_init_get(parm[1].value.unumber, &init);
        bdmf_session_print(session, "init = %u = 0x%x\n", init, init);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_trns_init:
    {
        uint8_t init;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_trns_init_get(parm[1].value.unumber, &init);
        bdmf_session_print(session, "init = %u = 0x%x\n", init, init);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes:
    {
        uint16_t ddr;
        uint16_t psram0;
        uint16_t psram1;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes_get(parm[1].value.unumber, parm[2].value.unumber, &ddr, &psram0, &psram1);
        bdmf_session_print(session, "ddr = %u = 0x%x\n", ddr, ddr);
        bdmf_session_print(session, "psram0 = %u = 0x%x\n", psram0, psram0);
        bdmf_session_print(session, "psram1 = %u = 0x%x\n", psram1, psram1);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd:
    {
        uint8_t ddr;
        uint8_t psram0;
        uint8_t psram1;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd_get(parm[1].value.unumber, parm[2].value.unumber, &ddr, &psram0, &psram1);
        bdmf_session_print(session, "ddr = %u = 0x%x\n", ddr, ddr);
        bdmf_session_print(session, "psram0 = %u = 0x%x\n", psram0, psram0);
        bdmf_session_print(session, "psram1 = %u = 0x%x\n", psram1, psram1);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_autootfcredits:
    {
        uint8_t numports;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_autootfcredits_get(parm[1].value.unumber, &numports);
        bdmf_session_print(session, "numports = %u = 0x%x\n", numports, numports);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_txsram0pd:
    {
        uint32_t srampd;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_txsram0pd_get(parm[1].value.unumber, &srampd);
        bdmf_session_print(session, "srampd = %u = 0x%x\n", srampd, srampd);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_txsram1pd:
    {
        uint32_t srampd;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_txsram1pd_get(parm[1].value.unumber, &srampd);
        bdmf_session_print(session, "srampd = %u = 0x%x\n", srampd, srampd);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_txddrpd:
    {
        uint32_t ddrpd;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_txddrpd_get(parm[1].value.unumber, &ddrpd);
        bdmf_session_print(session, "ddrpd = %u = 0x%x\n", ddrpd, ddrpd);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_pddrop:
    {
        uint16_t pddrop;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pddrop_get(parm[1].value.unumber, &pddrop);
        bdmf_session_print(session, "pddrop = %u = 0x%x\n", pddrop, pddrop);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_getnextnull:
    {
        uint16_t getnextnull;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_getnextnull_get(parm[1].value.unumber, &getnextnull);
        bdmf_session_print(session, "getnextnull = %u = 0x%x\n", getnextnull, getnextnull);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_getnextdrop:
    {
        uint16_t getnextdrop;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_getnextdrop_get(parm[1].value.unumber, &getnextdrop);
        bdmf_session_print(session, "getnextdrop = %u = 0x%x\n", getnextdrop, getnextdrop);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_inputfifodrop:
    {
        uint16_t inputfifodrop;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_inputfifodrop_get(parm[1].value.unumber, &inputfifodrop);
        bdmf_session_print(session, "inputfifodrop = %u = 0x%x\n", inputfifodrop, inputfifodrop);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_sram0pkt:
    {
        uint32_t srampkt;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram0pkt_get(parm[1].value.unumber, &srampkt);
        bdmf_session_print(session, "srampkt = %u = 0x%x\n", srampkt, srampkt);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_sram1pkt:
    {
        uint32_t srampkt;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram1pkt_get(parm[1].value.unumber, &srampkt);
        bdmf_session_print(session, "srampkt = %u = 0x%x\n", srampkt, srampkt);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_ddrpkt:
    {
        uint32_t ddrpkt;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ddrpkt_get(parm[1].value.unumber, &ddrpkt);
        bdmf_session_print(session, "ddrpkt = %u = 0x%x\n", ddrpkt, ddrpkt);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_swrden:
    {
        crdp_ds_cmn_bbh_tx_debug_counters_swrden debug_counters_swrden;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrden_get(parm[1].value.unumber, &debug_counters_swrden);
        bdmf_session_print(session, "pdsel = %u = 0x%x\n", debug_counters_swrden.pdsel, debug_counters_swrden.pdsel);
        bdmf_session_print(session, "pdvsel = %u = 0x%x\n", debug_counters_swrden.pdvsel, debug_counters_swrden.pdvsel);
        bdmf_session_print(session, "pdemptysel = %u = 0x%x\n", debug_counters_swrden.pdemptysel, debug_counters_swrden.pdemptysel);
        bdmf_session_print(session, "pdfullsel = %u = 0x%x\n", debug_counters_swrden.pdfullsel, debug_counters_swrden.pdfullsel);
        bdmf_session_print(session, "pdbemptysel = %u = 0x%x\n", debug_counters_swrden.pdbemptysel, debug_counters_swrden.pdbemptysel);
        bdmf_session_print(session, "pdffwkpsel = %u = 0x%x\n", debug_counters_swrden.pdffwkpsel, debug_counters_swrden.pdffwkpsel);
        bdmf_session_print(session, "fbn_0_sel = %u = 0x%x\n", debug_counters_swrden.fbn_0_sel, debug_counters_swrden.fbn_0_sel);
        bdmf_session_print(session, "fbnv_0_sel = %u = 0x%x\n", debug_counters_swrden.fbnv_0_sel, debug_counters_swrden.fbnv_0_sel);
        bdmf_session_print(session, "fbnempty_0_sel = %u = 0x%x\n", debug_counters_swrden.fbnempty_0_sel, debug_counters_swrden.fbnempty_0_sel);
        bdmf_session_print(session, "fbnfull_0_sel = %u = 0x%x\n", debug_counters_swrden.fbnfull_0_sel, debug_counters_swrden.fbnfull_0_sel);
        bdmf_session_print(session, "getnext_0_sel = %u = 0x%x\n", debug_counters_swrden.getnext_0_sel, debug_counters_swrden.getnext_0_sel);
        bdmf_session_print(session, "getnextv_0_sel = %u = 0x%x\n", debug_counters_swrden.getnextv_0_sel, debug_counters_swrden.getnextv_0_sel);
        bdmf_session_print(session, "getnextempty_0_sel = %u = 0x%x\n", debug_counters_swrden.getnextempty_0_sel, debug_counters_swrden.getnextempty_0_sel);
        bdmf_session_print(session, "getnextfull_0_sel = %u = 0x%x\n", debug_counters_swrden.getnextfull_0_sel, debug_counters_swrden.getnextfull_0_sel);
        bdmf_session_print(session, "fpm_0_sel = %u = 0x%x\n", debug_counters_swrden.fpm_0_sel, debug_counters_swrden.fpm_0_sel);
        bdmf_session_print(session, "sbpm_0_sel = %u = 0x%x\n", debug_counters_swrden.sbpm_0_sel, debug_counters_swrden.sbpm_0_sel);
        bdmf_session_print(session, "fbnv_1_sel = %u = 0x%x\n", debug_counters_swrden.fbnv_1_sel, debug_counters_swrden.fbnv_1_sel);
        bdmf_session_print(session, "fbnempty_1_sel = %u = 0x%x\n", debug_counters_swrden.fbnempty_1_sel, debug_counters_swrden.fbnempty_1_sel);
        bdmf_session_print(session, "fbnfull_1_sel = %u = 0x%x\n", debug_counters_swrden.fbnfull_1_sel, debug_counters_swrden.fbnfull_1_sel);
        bdmf_session_print(session, "getnext_1_sel = %u = 0x%x\n", debug_counters_swrden.getnext_1_sel, debug_counters_swrden.getnext_1_sel);
        bdmf_session_print(session, "getnextv_1_sel = %u = 0x%x\n", debug_counters_swrden.getnextv_1_sel, debug_counters_swrden.getnextv_1_sel);
        bdmf_session_print(session, "getnextempty_1_sel = %u = 0x%x\n", debug_counters_swrden.getnextempty_1_sel, debug_counters_swrden.getnextempty_1_sel);
        bdmf_session_print(session, "getnextfull_1_sel = %u = 0x%x\n", debug_counters_swrden.getnextfull_1_sel, debug_counters_swrden.getnextfull_1_sel);
        bdmf_session_print(session, "fpm_1_sel = %u = 0x%x\n", debug_counters_swrden.fpm_1_sel, debug_counters_swrden.fpm_1_sel);
        bdmf_session_print(session, "sbpm_1_sel = %u = 0x%x\n", debug_counters_swrden.sbpm_1_sel, debug_counters_swrden.sbpm_1_sel);
        bdmf_session_print(session, "fbn_1_sel = %u = 0x%x\n", debug_counters_swrden.fbn_1_sel, debug_counters_swrden.fbn_1_sel);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_swrdaddr:
    {
        uint16_t rdaddr;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrdaddr_get(parm[1].value.unumber, &rdaddr);
        bdmf_session_print(session, "rdaddr = %u = 0x%x\n", rdaddr, rdaddr);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_swrddata:
    {
        uint32_t data;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrddata_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_unifiedpkt:
    {
        uint32_t unified_pkt;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unifiedpkt_get(parm[1].value.unumber, parm[2].value.unumber, &unified_pkt);
        bdmf_session_print(session, "unified_pkt = %u = 0x%x\n", unified_pkt, unified_pkt);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_unifiedbyte:
    {
        uint32_t ddrbyte;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unifiedbyte_get(parm[1].value.unumber, parm[2].value.unumber, &ddrbyte);
        bdmf_session_print(session, "ddrbyte = %u = 0x%x\n", ddrbyte, ddrbyte);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_dbgoutreg:
    {
        crdp_ds_cmn_bbh_tx_debug_counters_dbgoutreg debug_counters_dbgoutreg;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_dbgoutreg_get(parm[1].value.unumber, parm[2].value.unumber, &debug_counters_dbgoutreg);
        bdmf_session_print(session, "debug_out_reg[0] = %u = 0x%x\n", debug_counters_dbgoutreg.debug_out_reg[0], debug_counters_dbgoutreg.debug_out_reg[0]);
        bdmf_session_print(session, "debug_out_reg[1] = %u = 0x%x\n", debug_counters_dbgoutreg.debug_out_reg[1], debug_counters_dbgoutreg.debug_out_reg[1]);
        bdmf_session_print(session, "debug_out_reg[2] = %u = 0x%x\n", debug_counters_dbgoutreg.debug_out_reg[2], debug_counters_dbgoutreg.debug_out_reg[2]);
        bdmf_session_print(session, "debug_out_reg[3] = %u = 0x%x\n", debug_counters_dbgoutreg.debug_out_reg[3], debug_counters_dbgoutreg.debug_out_reg[3]);
        bdmf_session_print(session, "debug_out_reg[4] = %u = 0x%x\n", debug_counters_dbgoutreg.debug_out_reg[4], debug_counters_dbgoutreg.debug_out_reg[4]);
        bdmf_session_print(session, "debug_out_reg[5] = %u = 0x%x\n", debug_counters_dbgoutreg.debug_out_reg[5], debug_counters_dbgoutreg.debug_out_reg[5]);
        bdmf_session_print(session, "debug_out_reg[6] = %u = 0x%x\n", debug_counters_dbgoutreg.debug_out_reg[6], debug_counters_dbgoutreg.debug_out_reg[6]);
        bdmf_session_print(session, "debug_out_reg[7] = %u = 0x%x\n", debug_counters_dbgoutreg.debug_out_reg[7], debug_counters_dbgoutreg.debug_out_reg[7]);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_in_segmentation:
    {
        uint32_t in_segmentation;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_in_segmentation_get(parm[1].value.unumber, &in_segmentation);
        bdmf_session_print(session, "in_segmentation = %u = 0x%x\n", in_segmentation, in_segmentation);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_unified_data_credits:
    {
        uint16_t credits;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unified_data_credits_get(parm[1].value.unumber, parm[2].value.unumber, &credits);
        bdmf_session_print(session, "credits = %u = 0x%x\n", credits, credits);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_unified_pd_credits:
    {
        uint16_t credits;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unified_pd_credits_get(parm[1].value.unumber, parm[2].value.unumber, &credits);
        bdmf_session_print(session, "credits = %u = 0x%x\n", credits, credits);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_disflowdroppkt:
    {
        uint32_t droppkt;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_disflowdroppkt_get(parm[1].value.unumber, &droppkt);
        bdmf_session_print(session, "droppkt = %u = 0x%x\n", droppkt, droppkt);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_disflowdropbyte:
    {
        uint32_t dropbyte;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_disflowdropbyte_get(parm[1].value.unumber, &dropbyte);
        bdmf_session_print(session, "dropbyte = %u = 0x%x\n", dropbyte, dropbyte);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_ecnpkt:
    {
        uint32_t ecnpkt;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnpkt_get(parm[1].value.unumber, &ecnpkt);
        bdmf_session_print(session, "ecnpkt = %u = 0x%x\n", ecnpkt, ecnpkt);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_ecnbytes:
    {
        uint32_t ecnbytes;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnbytes_get(parm[1].value.unumber, &ecnbytes);
        bdmf_session_print(session, "ecnbytes = %u = 0x%x\n", ecnbytes, ecnbytes);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_ecnmarkerr:
    {
        uint16_t v1;
        uint16_t v2;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnmarkerr_get(parm[1].value.unumber, &v1, &v2);
        bdmf_session_print(session, "v1 = %u = 0x%x\n", v1, v1);
        bdmf_session_print(session, "v2 = %u = 0x%x\n", v2, v2);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_ecnlenerr:
    {
        uint16_t v1;
        uint16_t v2;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnlenerr_get(parm[1].value.unumber, &v1, &v2);
        bdmf_session_print(session, "v1 = %u = 0x%x\n", v1, v1);
        bdmf_session_print(session, "v2 = %u = 0x%x\n", v2, v2);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_flush_info:
    {
        uint16_t sn;
        uint8_t pdfifoempty;
        uint8_t pdarrayempty;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_flush_info_get(parm[1].value.unumber, &sn, &pdfifoempty, &pdarrayempty);
        bdmf_session_print(session, "sn = %u = 0x%x\n", sn, sn);
        bdmf_session_print(session, "pdfifoempty = %u = 0x%x\n", pdfifoempty, pdfifoempty);
        bdmf_session_print(session, "pdarrayempty = %u = 0x%x\n", pdarrayempty, pdarrayempty);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_flush_info2:
    {
        uint8_t pfbingempty;
        uint8_t fepdempty;
        uint8_t fedataempty;
        uint8_t portvld;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_flush_info2_get(parm[1].value.unumber, &pfbingempty, &fepdempty, &fedataempty, &portvld);
        bdmf_session_print(session, "pfbingempty = %u = 0x%x\n", pfbingempty, pfbingempty);
        bdmf_session_print(session, "fepdempty = %u = 0x%x\n", fepdempty, fepdempty);
        bdmf_session_print(session, "fedataempty = %u = 0x%x\n", fedataempty, fedataempty);
        bdmf_session_print(session, "portvld = %u = 0x%x\n", portvld, portvld);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_pd_eh_mismatch:
    {
        uint16_t cnt;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pd_eh_mismatch_get(parm[1].value.unumber, &cnt);
        bdmf_session_print(session, "cnt = %u = 0x%x\n", cnt, cnt);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_pd_eh_mismatch_data:
    {
        uint8_t port;
        uint16_t plen;
        uint8_t pktnum;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pd_eh_mismatch_data_get(parm[1].value.unumber, &port, &plen, &pktnum);
        bdmf_session_print(session, "port = %u = 0x%x\n", port, port);
        bdmf_session_print(session, "plen = %u = 0x%x\n", plen, plen);
        bdmf_session_print(session, "pktnum = %u = 0x%x\n", pktnum, pktnum);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_outofrange_errors:
    {
        bdmf_boolean macflow;
        bdmf_boolean ctridx;
        uint8_t port;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_outofrange_errors_get(parm[1].value.unumber, &macflow, &ctridx, &port);
        bdmf_session_print(session, "macflow = %u = 0x%x\n", macflow, macflow);
        bdmf_session_print(session, "ctridx = %u = 0x%x\n", ctridx, ctridx);
        bdmf_session_print(session, "port = %u = 0x%x\n", port, port);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_gemdroppkt:
    {
        uint32_t pkt;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_gemdroppkt_get(parm[1].value.unumber, parm[2].value.unumber, &pkt);
        bdmf_session_print(session, "pkt = %u = 0x%x\n", pkt, pkt);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_gemdropbytes:
    {
        uint32_t bytes;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_gemdropbytes_get(parm[1].value.unumber, parm[2].value.unumber, &bytes);
        bdmf_session_print(session, "bytes = %u = 0x%x\n", bytes, bytes);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_cmdotfddr:
    {
        uint8_t value;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_cmdotfddr_get(parm[1].value.unumber, parm[2].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_bytesotfddr:
    {
        uint16_t value;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_bytesotfddr_get(parm[1].value.unumber, parm[2].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_cmdotfepsram:
    {
        uint8_t value;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_cmdotfepsram_get(parm[1].value.unumber, parm[2].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_bytesotfepsram:
    {
        uint16_t value;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_bytesotfepsram_get(parm[1].value.unumber, parm[2].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_gnext:
    {
        uint32_t entry;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_gnext_get(parm[1].value.unumber, parm[2].value.unumber, &entry);
        bdmf_session_print(session, "entry = %u = 0x%x\n", entry, entry);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_pdarray:
    {
        uint32_t entry;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pdarray_get(parm[1].value.unumber, parm[2].value.unumber, &entry);
        bdmf_session_print(session, "entry = %u = 0x%x\n", entry, entry);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_curpd:
    {
        uint32_t entry;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_curpd_get(parm[1].value.unumber, parm[2].value.unumber, &entry);
        bdmf_session_print(session, "entry = %u = 0x%x\n", entry, entry);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_space4req:
    {
        uint8_t resps;
        uint8_t target_mem;
        uint8_t fe;
        uint8_t dp;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_space4req_get(parm[1].value.unumber, &resps, &target_mem, &fe, &dp);
        bdmf_session_print(session, "resps = %u = 0x%x\n", resps, resps);
        bdmf_session_print(session, "target_mem = %u = 0x%x\n", target_mem, target_mem);
        bdmf_session_print(session, "fe = %u = 0x%x\n", fe, fe);
        bdmf_session_print(session, "dp = %u = 0x%x\n", dp, dp);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_seginfo:
    {
        uint8_t pdempty;
        uint8_t gnrdy0;
        uint8_t gnrdy1;
        uint8_t qrdy;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_seginfo_get(parm[1].value.unumber, &pdempty, &gnrdy0, &gnrdy1, &qrdy);
        bdmf_session_print(session, "pdempty = %u = 0x%x\n", pdempty, pdempty);
        bdmf_session_print(session, "gnrdy0 = %u = 0x%x\n", gnrdy0, gnrdy0);
        bdmf_session_print(session, "gnrdy1 = %u = 0x%x\n", gnrdy1, gnrdy1);
        bdmf_session_print(session, "qrdy = %u = 0x%x\n", qrdy, qrdy);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_ehdatval:
    {
        uint8_t val;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ehdatval_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_ehdat:
    {
        uint32_t data;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ehdat_get(parm[1].value.unumber, parm[2].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_rmtpdocc:
    {
        uint16_t occ1;
        uint16_t occ2;
        uint16_t occ3;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rmtpdocc_get(parm[1].value.unumber, &occ1, &occ2, &occ3);
        bdmf_session_print(session, "occ1 = %u = 0x%x\n", occ1, occ1);
        bdmf_session_print(session, "occ2 = %u = 0x%x\n", occ2, occ2);
        bdmf_session_print(session, "occ3 = %u = 0x%x\n", occ3, occ3);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_rmtbytes:
    {
        uint32_t val;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rmtbytes_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_rmtlbcnt:
    {
        uint32_t val;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rmtlbcnt_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_respspd:
    {
        uint8_t credit;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_respspd_get(parm[1].value.unumber, parm[2].value.unumber, &credit);
        bdmf_session_print(session, "credit = %u = 0x%x\n", credit, credit);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_respsdata:
    {
        uint8_t credit;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_respsdata_get(parm[1].value.unumber, parm[2].value.unumber, &credit);
        bdmf_session_print(session, "credit = %u = 0x%x\n", credit, credit);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_rnrsmpdcnt:
    {
        uint32_t value;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rnrsmpdcnt_get(parm[1].value.unumber, parm[2].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_rnrsmbytecnt:
    {
        uint32_t value;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rnrsmbytecnt_get(parm[1].value.unumber, parm[2].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_portdbg:
    {
        uint32_t data;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_portdbg_get(parm[1].value.unumber, parm[2].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_debug_counters_respdbg:
    {
        uint32_t data;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_respdbg_get(parm[1].value.unumber, parm[2].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0:
    {
        uint16_t flow;
        uint16_t gem;
        bdmf_boolean valid;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0_get(parm[1].value.unumber, parm[2].value.unumber, &flow, &gem, &valid);
        bdmf_session_print(session, "flow = %u = 0x%x\n", flow, flow);
        bdmf_session_print(session, "gem = %u = 0x%x\n", gem, gem);
        bdmf_session_print(session, "valid = %u = 0x%x\n", valid, valid);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1:
    {
        uint16_t flow;
        uint16_t gem;
        bdmf_boolean valid;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1_get(parm[1].value.unumber, parm[2].value.unumber, &flow, &gem, &valid);
        bdmf_session_print(session, "flow = %u = 0x%x\n", flow, flow);
        bdmf_session_print(session, "gem = %u = 0x%x\n", gem, gem);
        bdmf_session_print(session, "valid = %u = 0x%x\n", valid, valid);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2:
    {
        uint16_t flow;
        uint16_t gem;
        bdmf_boolean valid;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2_get(parm[1].value.unumber, parm[2].value.unumber, &flow, &gem, &valid);
        bdmf_session_print(session, "flow = %u = 0x%x\n", flow, flow);
        bdmf_session_print(session, "gem = %u = 0x%x\n", gem, gem);
        bdmf_session_print(session, "valid = %u = 0x%x\n", valid, valid);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3:
    {
        uint16_t flow;
        uint16_t gem;
        bdmf_boolean valid;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3_get(parm[1].value.unumber, parm[2].value.unumber, &flow, &gem, &valid);
        bdmf_session_print(session, "flow = %u = 0x%x\n", flow, flow);
        bdmf_session_print(session, "gem = %u = 0x%x\n", gem, gem);
        bdmf_session_print(session, "valid = %u = 0x%x\n", valid, valid);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4:
    {
        uint16_t flow;
        uint16_t gem;
        bdmf_boolean valid;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4_get(parm[1].value.unumber, parm[2].value.unumber, &flow, &gem, &valid);
        bdmf_session_print(session, "flow = %u = 0x%x\n", flow, flow);
        bdmf_session_print(session, "gem = %u = 0x%x\n", gem, gem);
        bdmf_session_print(session, "valid = %u = 0x%x\n", valid, valid);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5:
    {
        uint16_t flow;
        uint16_t gem;
        bdmf_boolean valid;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5_get(parm[1].value.unumber, parm[2].value.unumber, &flow, &gem, &valid);
        bdmf_session_print(session, "flow = %u = 0x%x\n", flow, flow);
        bdmf_session_print(session, "gem = %u = 0x%x\n", gem, gem);
        bdmf_session_print(session, "valid = %u = 0x%x\n", valid, valid);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6:
    {
        uint16_t flow;
        uint16_t gem;
        bdmf_boolean valid;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6_get(parm[1].value.unumber, parm[2].value.unumber, &flow, &gem, &valid);
        bdmf_session_print(session, "flow = %u = 0x%x\n", flow, flow);
        bdmf_session_print(session, "gem = %u = 0x%x\n", gem, gem);
        bdmf_session_print(session, "valid = %u = 0x%x\n", valid, valid);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7:
    {
        uint16_t flow;
        uint16_t gem;
        bdmf_boolean valid;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7_get(parm[1].value.unumber, parm[2].value.unumber, &flow, &gem, &valid);
        bdmf_session_print(session, "flow = %u = 0x%x\n", flow, flow);
        bdmf_session_print(session, "gem = %u = 0x%x\n", gem, gem);
        bdmf_session_print(session, "valid = %u = 0x%x\n", valid, valid);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val:
    {
        uint32_t val;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en:
    {
        uint32_t enable;
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en_get(parm[1].value.unumber, parm[2].value.unumber, &enable);
        bdmf_session_print(session, "enable = %u = 0x%x\n", enable, enable);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_crdp_ds_cmn_bbh_tx_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t bbh_id = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint8_t zero = gtmv(m, 3);
        uint8_t type = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_mac_type_set(%u %u %u)\n", bbh_id, zero,
            type);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_mac_type_set(bbh_id, zero, type);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_mac_type_get(bbh_id, zero, &type);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_mac_type_get(%u %u %u)\n", bbh_id, zero,
                type);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (type != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t ddrbyte = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ddrbyte_get(bbh_id, &ddrbyte);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ddrbyte_get(%u %u)\n", bbh_id,
                ddrbyte);
        }
    }

    {
        uint32_t srambyte = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram0byte_get(bbh_id, &srambyte);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram0byte_get(%u %u)\n", bbh_id,
                srambyte);
        }
    }

    {
        uint32_t srambyte = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram1byte_get(bbh_id, &srambyte);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram1byte_get(%u %u)\n", bbh_id,
                srambyte);
        }
    }

    {
        uint16_t sbpmsrc = gtmv(m, 9);
        uint16_t fpmsrc = gtmv(m, 9);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbcfg_tx_set(%u %u %u)\n", bbh_id,
            sbpmsrc, fpmsrc);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbcfg_tx_set(bbh_id, sbpmsrc, fpmsrc);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbcfg_tx_get(bbh_id, &sbpmsrc, &fpmsrc);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbcfg_tx_get(%u %u %u)\n", bbh_id,
                sbpmsrc, fpmsrc);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sbpmsrc != gtmv(m, 9) || fpmsrc != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg common_configurations_sbpmcfg = {.freenocntxt = gtmv(m, 1), .mc_type_3 = gtmv(m, 1), .mc_type_2 = gtmv(m, 1), .second_bn_dis = gtmv(m, 1), .bufsize = gtmv(m, 1), .enrrspread = gtmv(m, 1), .maxgn = gtmv(m, 5), .listfreetimer = gtmv(m, 10), .listfreethr = gtmv(m, 3), .smid = gtmv(m, 8)};
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg_set(%u %u %u %u %u %u %u %u %u %u %u)\n", bbh_id,
            common_configurations_sbpmcfg.freenocntxt, common_configurations_sbpmcfg.mc_type_3, common_configurations_sbpmcfg.mc_type_2, common_configurations_sbpmcfg.second_bn_dis, 
            common_configurations_sbpmcfg.bufsize, common_configurations_sbpmcfg.enrrspread, common_configurations_sbpmcfg.maxgn, common_configurations_sbpmcfg.listfreetimer, 
            common_configurations_sbpmcfg.listfreethr, common_configurations_sbpmcfg.smid);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg_set(bbh_id, &common_configurations_sbpmcfg);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg_get(bbh_id, &common_configurations_sbpmcfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg_get(%u %u %u %u %u %u %u %u %u %u %u)\n", bbh_id,
                common_configurations_sbpmcfg.freenocntxt, common_configurations_sbpmcfg.mc_type_3, common_configurations_sbpmcfg.mc_type_2, common_configurations_sbpmcfg.second_bn_dis, 
                common_configurations_sbpmcfg.bufsize, common_configurations_sbpmcfg.enrrspread, common_configurations_sbpmcfg.maxgn, common_configurations_sbpmcfg.listfreetimer, 
                common_configurations_sbpmcfg.listfreethr, common_configurations_sbpmcfg.smid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (common_configurations_sbpmcfg.freenocntxt != gtmv(m, 1) || common_configurations_sbpmcfg.mc_type_3 != gtmv(m, 1) || common_configurations_sbpmcfg.mc_type_2 != gtmv(m, 1) || common_configurations_sbpmcfg.second_bn_dis != gtmv(m, 1) || common_configurations_sbpmcfg.bufsize != gtmv(m, 1) || common_configurations_sbpmcfg.enrrspread != gtmv(m, 1) || common_configurations_sbpmcfg.maxgn != gtmv(m, 5) || common_configurations_sbpmcfg.listfreetimer != gtmv(m, 10) || common_configurations_sbpmcfg.listfreethr != gtmv(m, 3) || common_configurations_sbpmcfg.smid != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t bbsrcid = gtmv(m, 9);
        bdmf_boolean byp = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbsrcid_set(%u %u %u)\n", bbh_id,
            bbsrcid, byp);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbsrcid_set(bbh_id, bbsrcid, byp);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbsrcid_get(bbh_id, &bbsrcid, &byp);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbsrcid_get(%u %u %u)\n", bbh_id,
                bbsrcid, byp);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bbsrcid != gtmv(m, 9) || byp != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t bufsize0 = gtmv(m, 3);
        uint8_t bufsize1 = gtmv(m, 3);
        uint8_t hnsize = gtmv(m, 7);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrcfg_tx_set(%u %u %u %u)\n", bbh_id,
            bufsize0, bufsize1, hnsize);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrcfg_tx_set(bbh_id, bufsize0, bufsize1, hnsize);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrcfg_tx_get(bbh_id, &bufsize0, &bufsize1, &hnsize);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrcfg_tx_get(%u %u %u %u)\n", bbh_id,
                bufsize0, bufsize1, hnsize);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bufsize0 != gtmv(m, 3) || bufsize1 != gtmv(m, 3) || hnsize != gtmv(m, 7))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t pd_rnr_cfg_index = gtmv(m, 3);
        uint16_t wkpaddr = gtmv(m, 16);
        uint8_t task = gtmv(m, 4);
        uint16_t bbsrcid = gtmv(m, 9);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg_set(%u %u %u %u %u)\n", bbh_id, pd_rnr_cfg_index,
            wkpaddr, task, bbsrcid);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg_set(bbh_id, pd_rnr_cfg_index, wkpaddr, task, bbsrcid);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg_get(bbh_id, pd_rnr_cfg_index, &wkpaddr, &task, &bbsrcid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg_get(%u %u %u %u %u)\n", bbh_id, pd_rnr_cfg_index,
                wkpaddr, task, bbsrcid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (wkpaddr != gtmv(m, 16) || task != gtmv(m, 4) || bbsrcid != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd common_configurations_txrstcmd = {.cntxtrst = gtmv(m, 1), .pdfiforst = gtmv(m, 1), .bpmfiforst = gtmv(m, 1), .sbpmfiforst = gtmv(m, 1), .ddrfiforst = gtmv(m, 1), .sramfiforst = gtmv(m, 1), .skbptrrst = gtmv(m, 1), .gnxtfiforst = gtmv(m, 1), .fbnfiforst = gtmv(m, 1), .port = gtmv(m, 3)};
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd_set(%u %u %u %u %u %u %u %u %u %u %u)\n", bbh_id,
            common_configurations_txrstcmd.cntxtrst, common_configurations_txrstcmd.pdfiforst, common_configurations_txrstcmd.bpmfiforst, common_configurations_txrstcmd.sbpmfiforst, 
            common_configurations_txrstcmd.ddrfiforst, common_configurations_txrstcmd.sramfiforst, common_configurations_txrstcmd.skbptrrst, common_configurations_txrstcmd.gnxtfiforst, 
            common_configurations_txrstcmd.fbnfiforst, common_configurations_txrstcmd.port);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd_set(bbh_id, &common_configurations_txrstcmd);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd_get(bbh_id, &common_configurations_txrstcmd);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd_get(%u %u %u %u %u %u %u %u %u %u %u)\n", bbh_id,
                common_configurations_txrstcmd.cntxtrst, common_configurations_txrstcmd.pdfiforst, common_configurations_txrstcmd.bpmfiforst, common_configurations_txrstcmd.sbpmfiforst, 
                common_configurations_txrstcmd.ddrfiforst, common_configurations_txrstcmd.sramfiforst, common_configurations_txrstcmd.skbptrrst, common_configurations_txrstcmd.gnxtfiforst, 
                common_configurations_txrstcmd.fbnfiforst, common_configurations_txrstcmd.port);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (common_configurations_txrstcmd.cntxtrst != gtmv(m, 1) || common_configurations_txrstcmd.pdfiforst != gtmv(m, 1) || common_configurations_txrstcmd.bpmfiforst != gtmv(m, 1) || common_configurations_txrstcmd.sbpmfiforst != gtmv(m, 1) || common_configurations_txrstcmd.ddrfiforst != gtmv(m, 1) || common_configurations_txrstcmd.sramfiforst != gtmv(m, 1) || common_configurations_txrstcmd.skbptrrst != gtmv(m, 1) || common_configurations_txrstcmd.gnxtfiforst != gtmv(m, 1) || common_configurations_txrstcmd.fbnfiforst != gtmv(m, 1) || common_configurations_txrstcmd.port != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t dbgsel = gtmv(m, 6);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_dbgsel_set(%u %u)\n", bbh_id,
            dbgsel);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_dbgsel_set(bbh_id, dbgsel);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_dbgsel_get(bbh_id, &dbgsel);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_dbgsel_get(%u %u)\n", bbh_id,
                dbgsel);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (dbgsel != gtmv(m, 6))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl common_configurations_clk_gate_cntrl = {.bypass_clk_gate = gtmv(m, 1), .timer_val = gtmv(m, 8), .keep_alive_en = gtmv(m, 1), .keep_alive_intrvl = gtmv(m, 3), .keep_alive_cyc = gtmv(m, 8)};
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl_set(%u %u %u %u %u %u)\n", bbh_id,
            common_configurations_clk_gate_cntrl.bypass_clk_gate, common_configurations_clk_gate_cntrl.timer_val, common_configurations_clk_gate_cntrl.keep_alive_en, common_configurations_clk_gate_cntrl.keep_alive_intrvl, 
            common_configurations_clk_gate_cntrl.keep_alive_cyc);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl_set(bbh_id, &common_configurations_clk_gate_cntrl);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl_get(bbh_id, &common_configurations_clk_gate_cntrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl_get(%u %u %u %u %u %u)\n", bbh_id,
                common_configurations_clk_gate_cntrl.bypass_clk_gate, common_configurations_clk_gate_cntrl.timer_val, common_configurations_clk_gate_cntrl.keep_alive_en, common_configurations_clk_gate_cntrl.keep_alive_intrvl, 
                common_configurations_clk_gate_cntrl.keep_alive_cyc);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (common_configurations_clk_gate_cntrl.bypass_clk_gate != gtmv(m, 1) || common_configurations_clk_gate_cntrl.timer_val != gtmv(m, 8) || common_configurations_clk_gate_cntrl.keep_alive_en != gtmv(m, 1) || common_configurations_clk_gate_cntrl.keep_alive_intrvl != gtmv(m, 3) || common_configurations_clk_gate_cntrl.keep_alive_cyc != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t gpr = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_gpr_set(%u %u)\n", bbh_id,
            gpr);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_gpr_set(bbh_id, gpr);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_gpr_get(bbh_id, &gpr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_gpr_get(%u %u)\n", bbh_id,
                gpr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (gpr != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        crdp_ds_cmn_bbh_tx_common_configurations_general_cfg common_configurations_general_cfg = {.macdrop = gtmv(m, 1), .discopycrdt = gtmv(m, 1), .ecnrmrk = gtmv(m, 1), .cntraddcrc = gtmv(m, 1), .crcforward = gtmv(m, 1), .multi_tcont = gtmv(m, 1), .distargetperport = gtmv(m, 1), .discrdtchk = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_general_cfg_set(%u %u %u %u %u %u %u %u %u)\n", bbh_id,
            common_configurations_general_cfg.macdrop, common_configurations_general_cfg.discopycrdt, common_configurations_general_cfg.ecnrmrk, common_configurations_general_cfg.cntraddcrc, 
            common_configurations_general_cfg.crcforward, common_configurations_general_cfg.multi_tcont, common_configurations_general_cfg.distargetperport, common_configurations_general_cfg.discrdtchk);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_general_cfg_set(bbh_id, &common_configurations_general_cfg);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_general_cfg_get(bbh_id, &common_configurations_general_cfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_general_cfg_get(%u %u %u %u %u %u %u %u %u)\n", bbh_id,
                common_configurations_general_cfg.macdrop, common_configurations_general_cfg.discopycrdt, common_configurations_general_cfg.ecnrmrk, common_configurations_general_cfg.cntraddcrc, 
                common_configurations_general_cfg.crcforward, common_configurations_general_cfg.multi_tcont, common_configurations_general_cfg.distargetperport, common_configurations_general_cfg.discrdtchk);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (common_configurations_general_cfg.macdrop != gtmv(m, 1) || common_configurations_general_cfg.discopycrdt != gtmv(m, 1) || common_configurations_general_cfg.ecnrmrk != gtmv(m, 1) || common_configurations_general_cfg.cntraddcrc != gtmv(m, 1) || common_configurations_general_cfg.crcforward != gtmv(m, 1) || common_configurations_general_cfg.multi_tcont != gtmv(m, 1) || common_configurations_general_cfg.distargetperport != gtmv(m, 1) || common_configurations_general_cfg.discrdtchk != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        crdp_ds_cmn_bbh_tx_common_configurations_ecncfg common_configurations_ecncfg = {.ecnbyteipv4 = gtmv(m, 5), .ecnbitipv4 = gtmv(m, 3), .ecnbyteipv6 = gtmv(m, 5), .ecnbitipv6 = gtmv(m, 3), .chksumbyteipv4 = gtmv(m, 6), .assignedqcnt = gtmv(m, 3)};
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ecncfg_set(%u %u %u %u %u %u %u)\n", bbh_id,
            common_configurations_ecncfg.ecnbyteipv4, common_configurations_ecncfg.ecnbitipv4, common_configurations_ecncfg.ecnbyteipv6, common_configurations_ecncfg.ecnbitipv6, 
            common_configurations_ecncfg.chksumbyteipv4, common_configurations_ecncfg.assignedqcnt);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ecncfg_set(bbh_id, &common_configurations_ecncfg);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ecncfg_get(bbh_id, &common_configurations_ecncfg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ecncfg_get(%u %u %u %u %u %u %u)\n", bbh_id,
                common_configurations_ecncfg.ecnbyteipv4, common_configurations_ecncfg.ecnbitipv4, common_configurations_ecncfg.ecnbyteipv6, common_configurations_ecncfg.ecnbitipv6, 
                common_configurations_ecncfg.chksumbyteipv4, common_configurations_ecncfg.assignedqcnt);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (common_configurations_ecncfg.ecnbyteipv4 != gtmv(m, 5) || common_configurations_ecncfg.ecnbitipv4 != gtmv(m, 3) || common_configurations_ecncfg.ecnbyteipv6 != gtmv(m, 5) || common_configurations_ecncfg.ecnbitipv6 != gtmv(m, 3) || common_configurations_ecncfg.chksumbyteipv4 != gtmv(m, 6) || common_configurations_ecncfg.assignedqcnt != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t ddr_fpmini_basel_index = gtmv(m, 1);
        uint32_t ddrfpminibase = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel_set(%u %u %u)\n", bbh_id, ddr_fpmini_basel_index,
            ddrfpminibase);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel_set(bbh_id, ddr_fpmini_basel_index, ddrfpminibase);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel_get(bbh_id, ddr_fpmini_basel_index, &ddrfpminibase);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel_get(%u %u %u)\n", bbh_id, ddr_fpmini_basel_index,
                ddrfpminibase);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddrfpminibase != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t ddr_fpmini_baseh_index = gtmv(m, 1);
        uint8_t ddrfpminibase = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh_set(%u %u %u)\n", bbh_id, ddr_fpmini_baseh_index,
            ddrfpminibase);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh_set(bbh_id, ddr_fpmini_baseh_index, ddrfpminibase);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh_get(bbh_id, ddr_fpmini_baseh_index, &ddrfpminibase);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh_get(%u %u %u)\n", bbh_id, ddr_fpmini_baseh_index,
                ddrfpminibase);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddrfpminibase != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t route = gtmv(m, 25);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdroute_set(%u %u)\n", bbh_id,
            route);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdroute_set(bbh_id, route);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdroute_get(bbh_id, &route);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdroute_get(%u %u)\n", bbh_id,
                route);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (route != gtmv(m, 25))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t dest = gtmv(m, 9);
        bdmf_boolean en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdctrl_set(%u %u %u)\n", bbh_id,
            dest, en);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdctrl_set(bbh_id, dest, en);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdctrl_get(bbh_id, &dest, &en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdctrl_get(%u %u %u)\n", bbh_id,
                dest, en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (dest != gtmv(m, 9) || en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t tm_basel_index = gtmv(m, 2);
        uint32_t ddrtmbase = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel_set(%u %u %u)\n", bbh_id, tm_basel_index,
            ddrtmbase);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel_set(bbh_id, tm_basel_index, ddrtmbase);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel_get(bbh_id, tm_basel_index, &ddrtmbase);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel_get(%u %u %u)\n", bbh_id, tm_basel_index,
                ddrtmbase);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddrtmbase != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t tm_baseh_index = gtmv(m, 2);
        uint8_t ddrtmbase = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh_set(%u %u %u)\n", bbh_id, tm_baseh_index,
            ddrtmbase);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh_set(bbh_id, tm_baseh_index, ddrtmbase);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh_get(bbh_id, tm_baseh_index, &ddrtmbase);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh_get(%u %u %u)\n", bbh_id, tm_baseh_index,
                ddrtmbase);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddrtmbase != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 4);
        uint16_t size = gtmv(m, 10);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize_set(%u %u %u)\n", bbh_id, index,
            size);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize_set(bbh_id, index, size);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize_get(bbh_id, index, &size);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize_get(%u %u %u)\n", bbh_id, index,
                size);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (size != gtmv(m, 10))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 4);
        uint8_t credits = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits_set(%u %u %u)\n", bbh_id, index,
            credits);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits_set(bbh_id, index, credits);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits_get(bbh_id, index, &credits);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits_get(%u %u %u)\n", bbh_id, index,
                credits);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (credits != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t init = gtmv(m, 24);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_creditinit_set(%u %u)\n", bbh_id,
            init);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_creditinit_set(bbh_id, init);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_creditinit_get(bbh_id, &init);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_creditinit_get(%u %u)\n", bbh_id,
                init);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (init != gtmv(m, 24))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t wkpaddr = gtmv(m, 16);
        uint8_t task = gtmv(m, 4);
        uint16_t bbsrcid = gtmv(m, 9);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg_set(%u %u %u %u %u)\n", bbh_id, index,
            wkpaddr, task, bbsrcid);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg_set(bbh_id, index, wkpaddr, task, bbsrcid);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg_get(bbh_id, index, &wkpaddr, &task, &bbsrcid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg_get(%u %u %u %u %u)\n", bbh_id, index,
                wkpaddr, task, bbsrcid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (wkpaddr != gtmv(m, 16) || task != gtmv(m, 4) || bbsrcid != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint8_t wkpgran = gtmv(m, 8);
        bdmf_boolean ptrrst = gtmv(m, 1);
        bdmf_boolean enable = gtmv(m, 1);
        uint8_t timer = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg_set(%u %u %u %u %u %u)\n", bbh_id, index,
            wkpgran, ptrrst, enable, timer);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg_set(bbh_id, index, wkpgran, ptrrst, enable, timer);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg_get(bbh_id, index, &wkpgran, &ptrrst, &enable, &timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg_get(%u %u %u %u %u %u)\n", bbh_id, index,
                wkpgran, ptrrst, enable, timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (wkpgran != gtmv(m, 8) || ptrrst != gtmv(m, 1) || enable != gtmv(m, 1) || timer != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t sninitval = gtmv(m, 16);
        bdmf_boolean sninit = gtmv(m, 1);
        uint8_t lbctrrst = gtmv(m, 4);
        uint8_t mode = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbhcfg_set(%u %u %u %u %u)\n", bbh_id,
            sninitval, sninit, lbctrrst, mode);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbhcfg_set(bbh_id, sninitval, sninit, lbctrrst, mode);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbhcfg_get(bbh_id, &sninitval, &sninit, &lbctrrst, &mode);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbhcfg_get(%u %u %u %u %u)\n", bbh_id,
                sninitval, sninit, lbctrrst, mode);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sninitval != gtmv(m, 16) || sninit != gtmv(m, 1) || lbctrrst != gtmv(m, 4) || mode != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 1);
        uint16_t bbsrcid = gtmv(m, 9);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc_set(%u %u %u)\n", bbh_id, index,
            bbsrcid);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc_set(bbh_id, index, bbsrcid);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc_get(bbh_id, index, &bbsrcid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc_get(%u %u %u)\n", bbh_id, index,
                bbsrcid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bbsrcid != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader common_configurations_addr_spreader = {.num_active_dram = gtmv(m, 2), .en_ddr = gtmv(m, 1), .en_epsr = gtmv(m, 1), .simple_spread = gtmv(m, 1), .active_dram_0 = gtmv(m, 3), .active_dram_1 = gtmv(m, 3), .block_spread_zone_en = gtmv(m, 1), .block_spread_zone = gtmv(m, 4), .spare = gtmv(m, 16)};
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader_set(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id,
            common_configurations_addr_spreader.num_active_dram, common_configurations_addr_spreader.en_ddr, common_configurations_addr_spreader.en_epsr, common_configurations_addr_spreader.simple_spread, 
            common_configurations_addr_spreader.active_dram_0, common_configurations_addr_spreader.active_dram_1, common_configurations_addr_spreader.block_spread_zone_en, common_configurations_addr_spreader.block_spread_zone, 
            common_configurations_addr_spreader.spare);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader_set(bbh_id, &common_configurations_addr_spreader);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader_get(bbh_id, &common_configurations_addr_spreader);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id,
                common_configurations_addr_spreader.num_active_dram, common_configurations_addr_spreader.en_ddr, common_configurations_addr_spreader.en_epsr, common_configurations_addr_spreader.simple_spread, 
                common_configurations_addr_spreader.active_dram_0, common_configurations_addr_spreader.active_dram_1, common_configurations_addr_spreader.block_spread_zone_en, common_configurations_addr_spreader.block_spread_zone, 
                common_configurations_addr_spreader.spare);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (common_configurations_addr_spreader.num_active_dram != gtmv(m, 2) || common_configurations_addr_spreader.en_ddr != gtmv(m, 1) || common_configurations_addr_spreader.en_epsr != gtmv(m, 1) || common_configurations_addr_spreader.simple_spread != gtmv(m, 1) || common_configurations_addr_spreader.active_dram_0 != gtmv(m, 3) || common_configurations_addr_spreader.active_dram_1 != gtmv(m, 3) || common_configurations_addr_spreader.block_spread_zone_en != gtmv(m, 1) || common_configurations_addr_spreader.block_spread_zone != gtmv(m, 4) || common_configurations_addr_spreader.spare != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean numports = gtmv(m, 1);
        uint8_t numctrs = gtmv(m, 2);
        uint8_t macflowcnten = gtmv(m, 8);
        uint8_t ctridxcnten = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_flowctrcfg_set(%u %u %u %u %u)\n", bbh_id,
            numports, numctrs, macflowcnten, ctridxcnten);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_flowctrcfg_set(bbh_id, numports, numctrs, macflowcnten, ctridxcnten);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_flowctrcfg_get(bbh_id, &numports, &numctrs, &macflowcnten, &ctridxcnten);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_flowctrcfg_get(%u %u %u %u %u)\n", bbh_id,
                numports, numctrs, macflowcnten, ctridxcnten);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (numports != gtmv(m, 1) || numctrs != gtmv(m, 2) || macflowcnten != gtmv(m, 8) || ctridxcnten != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t wkpaddr = gtmv(m, 16);
        uint8_t task = gtmv(m, 4);
        uint16_t bbsrcid = gtmv(m, 9);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg_set(%u %u %u %u %u)\n", bbh_id, index,
            wkpaddr, task, bbsrcid);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg_set(bbh_id, index, wkpaddr, task, bbsrcid);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg_get(bbh_id, index, &wkpaddr, &task, &bbsrcid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg_get(%u %u %u %u %u)\n", bbh_id, index,
                wkpaddr, task, bbsrcid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (wkpaddr != gtmv(m, 16) || task != gtmv(m, 4) || bbsrcid != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t fc_stblfilter = gtmv(m, 10);
        bdmf_boolean fc_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fccfg_set(%u %u %u)\n", bbh_id,
            fc_stblfilter, fc_en);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fccfg_set(bbh_id, fc_stblfilter, fc_en);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fccfg_get(bbh_id, &fc_stblfilter, &fc_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fccfg_get(%u %u %u)\n", bbh_id,
                fc_stblfilter, fc_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fc_stblfilter != gtmv(m, 10) || fc_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t q = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdack_set(%u %u)\n", bbh_id,
            q);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdack_set(bbh_id, q);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdack_get(bbh_id, &q);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdack_get(%u %u)\n", bbh_id,
                q);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (q != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t fifosize = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize_set(%u %u %u)\n", bbh_id, index,
            fifosize);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize_set(bbh_id, index, fifosize);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize_get(bbh_id, index, &fifosize);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize_get(%u %u %u)\n", bbh_id, index,
                fifosize);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fifosize != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t wkupthresh = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph_set(%u %u %u)\n", bbh_id, index,
            wkupthresh);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph_set(bbh_id, index, wkupthresh);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph_get(bbh_id, index, &wkupthresh);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph_get(%u %u %u)\n", bbh_id, index,
                wkupthresh);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (wkupthresh != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t empty = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty_set(%u %u %u)\n", bbh_id, index,
            empty);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty_set(bbh_id, index, empty);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty_get(bbh_id, index, &empty);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty_get(%u %u %u)\n", bbh_id, index,
                empty);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (empty != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint32_t bytelimit = gtmv(m, 22);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_set(%u %u %u)\n", bbh_id, index,
            bytelimit);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_set(bbh_id, index, bytelimit);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_get(bbh_id, index, &bytelimit);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_get(%u %u %u)\n", bbh_id, index,
                bytelimit);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (bytelimit != gtmv(m, 22))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t pdlimit = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th_set(%u %u %u)\n", bbh_id, index,
            pdlimit);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th_set(bbh_id, index, pdlimit);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th_get(bbh_id, index, &pdlimit);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th_get(%u %u %u)\n", bbh_id, index,
                pdlimit);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pdlimit != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t pdbytelimiten = gtmv(m, 8);
        uint8_t pdmaxlimiten = gtmv(m, 8);
        uint8_t disfesf = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_en_set(%u %u %u %u)\n", bbh_id,
            pdbytelimiten, pdmaxlimiten, disfesf);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_en_set(bbh_id, pdbytelimiten, pdmaxlimiten, disfesf);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_en_get(bbh_id, &pdbytelimiten, &pdmaxlimiten, &disfesf);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_en_get(%u %u %u %u)\n", bbh_id,
                pdbytelimiten, pdmaxlimiten, disfesf);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pdbytelimiten != gtmv(m, 8) || pdmaxlimiten != gtmv(m, 8) || disfesf != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t offset0 = gtmv(m, 16);
        uint16_t offset1 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset_set(%u %u %u %u)\n", bbh_id, index,
            offset0, offset1);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset_set(bbh_id, index, offset0, offset1);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset_get(bbh_id, index, &offset0, &offset1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset_get(%u %u %u %u)\n", bbh_id, index,
                offset0, offset1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (offset0 != gtmv(m, 16) || offset1 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t thr = gtmv(m, 13);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr_set(%u %u %u)\n", bbh_id, index,
            thr);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr_set(bbh_id, index, thr);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr_get(bbh_id, index, &thr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr_get(%u %u %u)\n", bbh_id, index,
                thr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (thr != gtmv(m, 13))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t fifobase0 = gtmv(m, 12);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_febase_set(%u %u %u)\n", bbh_id, index,
            fifobase0);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_febase_set(bbh_id, index, fifobase0);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_febase_get(bbh_id, index, &fifobase0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_febase_get(%u %u %u)\n", bbh_id, index,
                fifobase0);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fifobase0 != gtmv(m, 12))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t fifosize = gtmv(m, 12);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize_set(%u %u %u)\n", bbh_id, index,
            fifosize);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize_set(bbh_id, index, fifosize);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize_get(bbh_id, index, &fifosize);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize_get(%u %u %u)\n", bbh_id, index,
                fifosize);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fifosize != gtmv(m, 12))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t fifobase0 = gtmv(m, 9);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase_set(%u %u %u)\n", bbh_id, index,
            fifobase0);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase_set(bbh_id, index, fifobase0);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase_get(bbh_id, index, &fifobase0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase_get(%u %u %u)\n", bbh_id, index,
                fifobase0);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fifobase0 != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t fifosize0 = gtmv(m, 9);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize_set(%u %u %u)\n", bbh_id, index,
            fifosize0);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize_set(bbh_id, index, fifosize0);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize_get(bbh_id, index, &fifosize0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize_get(%u %u %u)\n", bbh_id, index,
                fifosize0);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fifosize0 != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint8_t w0 = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr_set(%u %u %u)\n", bbh_id, index,
            w0);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr_set(bbh_id, index, w0);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr_get(bbh_id, index, &w0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr_get(%u %u %u)\n", bbh_id, index,
                w0);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (w0 != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t init = gtmv(m, 8);
        uint16_t min_pd = gtmv(m, 9);
        uint16_t min_data = gtmv(m, 12);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fe_credits_set(%u %u %u %u)\n", bbh_id,
            init, min_pd, min_data);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fe_credits_set(bbh_id, init, min_pd, min_data);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fe_credits_get(bbh_id, &init, &min_pd, &min_data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fe_credits_get(%u %u %u %u)\n", bbh_id,
                init, min_pd, min_data);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (init != gtmv(m, 8) || min_pd != gtmv(m, 9) || min_data != gtmv(m, 12))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t init = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_en_init_set(%u %u)\n", bbh_id,
            init);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_en_init_set(bbh_id, init);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_en_init_get(bbh_id, &init);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_en_init_get(%u %u)\n", bbh_id,
                init);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (init != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t init = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_trns_init_set(%u %u)\n", bbh_id,
            init);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_trns_init_set(bbh_id, init);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_trns_init_get(bbh_id, &init);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_trns_init_get(%u %u)\n", bbh_id,
                init);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (init != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t ddr = gtmv(m, 10);
        uint16_t psram0 = gtmv(m, 10);
        uint16_t psram1 = gtmv(m, 10);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes_set(%u %u %u %u %u)\n", bbh_id, index,
            ddr, psram0, psram1);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes_set(bbh_id, index, ddr, psram0, psram1);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes_get(bbh_id, index, &ddr, &psram0, &psram1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes_get(%u %u %u %u %u)\n", bbh_id, index,
                ddr, psram0, psram1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddr != gtmv(m, 10) || psram0 != gtmv(m, 10) || psram1 != gtmv(m, 10))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint8_t ddr = gtmv(m, 8);
        uint8_t psram0 = gtmv(m, 8);
        uint8_t psram1 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd_set(%u %u %u %u %u)\n", bbh_id, index,
            ddr, psram0, psram1);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd_set(bbh_id, index, ddr, psram0, psram1);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd_get(bbh_id, index, &ddr, &psram0, &psram1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd_get(%u %u %u %u %u)\n", bbh_id, index,
                ddr, psram0, psram1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ddr != gtmv(m, 8) || psram0 != gtmv(m, 8) || psram1 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t numports = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_autootfcredits_set(%u %u)\n", bbh_id,
            numports);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_autootfcredits_set(bbh_id, numports);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_autootfcredits_get(bbh_id, &numports);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_autootfcredits_get(%u %u)\n", bbh_id,
                numports);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (numports != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t srampd = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_txsram0pd_get(bbh_id, &srampd);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_txsram0pd_get(%u %u)\n", bbh_id,
                srampd);
        }
    }

    {
        uint32_t srampd = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_txsram1pd_get(bbh_id, &srampd);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_txsram1pd_get(%u %u)\n", bbh_id,
                srampd);
        }
    }

    {
        uint32_t ddrpd = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_txddrpd_get(bbh_id, &ddrpd);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_txddrpd_get(%u %u)\n", bbh_id,
                ddrpd);
        }
    }

    {
        uint16_t pddrop = gtmv(m, 16);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pddrop_get(bbh_id, &pddrop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pddrop_get(%u %u)\n", bbh_id,
                pddrop);
        }
    }

    {
        uint16_t getnextnull = gtmv(m, 16);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_getnextnull_get(bbh_id, &getnextnull);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_getnextnull_get(%u %u)\n", bbh_id,
                getnextnull);
        }
    }

    {
        uint16_t getnextdrop = gtmv(m, 16);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_getnextdrop_get(bbh_id, &getnextdrop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_getnextdrop_get(%u %u)\n", bbh_id,
                getnextdrop);
        }
    }

    {
        uint16_t inputfifodrop = gtmv(m, 16);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_inputfifodrop_get(bbh_id, &inputfifodrop);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_inputfifodrop_get(%u %u)\n", bbh_id,
                inputfifodrop);
        }
    }

    {
        uint32_t srampkt = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram0pkt_get(bbh_id, &srampkt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram0pkt_get(%u %u)\n", bbh_id,
                srampkt);
        }
    }

    {
        uint32_t srampkt = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram1pkt_get(bbh_id, &srampkt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_sram1pkt_get(%u %u)\n", bbh_id,
                srampkt);
        }
    }

    {
        uint32_t ddrpkt = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ddrpkt_get(bbh_id, &ddrpkt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ddrpkt_get(%u %u)\n", bbh_id,
                ddrpkt);
        }
    }

    {
        crdp_ds_cmn_bbh_tx_debug_counters_swrden debug_counters_swrden = {.pdsel = gtmv(m, 1), .pdvsel = gtmv(m, 1), .pdemptysel = gtmv(m, 1), .pdfullsel = gtmv(m, 1), .pdbemptysel = gtmv(m, 1), .pdffwkpsel = gtmv(m, 1), .fbn_0_sel = gtmv(m, 1), .fbnv_0_sel = gtmv(m, 1), .fbnempty_0_sel = gtmv(m, 1), .fbnfull_0_sel = gtmv(m, 1), .getnext_0_sel = gtmv(m, 1), .getnextv_0_sel = gtmv(m, 1), .getnextempty_0_sel = gtmv(m, 1), .getnextfull_0_sel = gtmv(m, 1), .fpm_0_sel = gtmv(m, 1), .sbpm_0_sel = gtmv(m, 1), .fbnv_1_sel = gtmv(m, 1), .fbnempty_1_sel = gtmv(m, 1), .fbnfull_1_sel = gtmv(m, 1), .getnext_1_sel = gtmv(m, 1), .getnextv_1_sel = gtmv(m, 1), .getnextempty_1_sel = gtmv(m, 1), .getnextfull_1_sel = gtmv(m, 1), .fpm_1_sel = gtmv(m, 1), .sbpm_1_sel = gtmv(m, 1), .fbn_1_sel = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrden_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", bbh_id,
            debug_counters_swrden.pdsel, debug_counters_swrden.pdvsel, debug_counters_swrden.pdemptysel, debug_counters_swrden.pdfullsel, 
            debug_counters_swrden.pdbemptysel, debug_counters_swrden.pdffwkpsel, debug_counters_swrden.fbn_0_sel, debug_counters_swrden.fbnv_0_sel, 
            debug_counters_swrden.fbnempty_0_sel, debug_counters_swrden.fbnfull_0_sel, debug_counters_swrden.getnext_0_sel, debug_counters_swrden.getnextv_0_sel, 
            debug_counters_swrden.getnextempty_0_sel, debug_counters_swrden.getnextfull_0_sel, debug_counters_swrden.fpm_0_sel, debug_counters_swrden.sbpm_0_sel, 
            debug_counters_swrden.fbnv_1_sel, debug_counters_swrden.fbnempty_1_sel, debug_counters_swrden.fbnfull_1_sel, debug_counters_swrden.getnext_1_sel, 
            debug_counters_swrden.getnextv_1_sel, debug_counters_swrden.getnextempty_1_sel, debug_counters_swrden.getnextfull_1_sel, debug_counters_swrden.fpm_1_sel, 
            debug_counters_swrden.sbpm_1_sel, debug_counters_swrden.fbn_1_sel);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrden_set(bbh_id, &debug_counters_swrden);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrden_get(bbh_id, &debug_counters_swrden);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrden_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", bbh_id,
                debug_counters_swrden.pdsel, debug_counters_swrden.pdvsel, debug_counters_swrden.pdemptysel, debug_counters_swrden.pdfullsel, 
                debug_counters_swrden.pdbemptysel, debug_counters_swrden.pdffwkpsel, debug_counters_swrden.fbn_0_sel, debug_counters_swrden.fbnv_0_sel, 
                debug_counters_swrden.fbnempty_0_sel, debug_counters_swrden.fbnfull_0_sel, debug_counters_swrden.getnext_0_sel, debug_counters_swrden.getnextv_0_sel, 
                debug_counters_swrden.getnextempty_0_sel, debug_counters_swrden.getnextfull_0_sel, debug_counters_swrden.fpm_0_sel, debug_counters_swrden.sbpm_0_sel, 
                debug_counters_swrden.fbnv_1_sel, debug_counters_swrden.fbnempty_1_sel, debug_counters_swrden.fbnfull_1_sel, debug_counters_swrden.getnext_1_sel, 
                debug_counters_swrden.getnextv_1_sel, debug_counters_swrden.getnextempty_1_sel, debug_counters_swrden.getnextfull_1_sel, debug_counters_swrden.fpm_1_sel, 
                debug_counters_swrden.sbpm_1_sel, debug_counters_swrden.fbn_1_sel);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (debug_counters_swrden.pdsel != gtmv(m, 1) || debug_counters_swrden.pdvsel != gtmv(m, 1) || debug_counters_swrden.pdemptysel != gtmv(m, 1) || debug_counters_swrden.pdfullsel != gtmv(m, 1) || debug_counters_swrden.pdbemptysel != gtmv(m, 1) || debug_counters_swrden.pdffwkpsel != gtmv(m, 1) || debug_counters_swrden.fbn_0_sel != gtmv(m, 1) || debug_counters_swrden.fbnv_0_sel != gtmv(m, 1) || debug_counters_swrden.fbnempty_0_sel != gtmv(m, 1) || debug_counters_swrden.fbnfull_0_sel != gtmv(m, 1) || debug_counters_swrden.getnext_0_sel != gtmv(m, 1) || debug_counters_swrden.getnextv_0_sel != gtmv(m, 1) || debug_counters_swrden.getnextempty_0_sel != gtmv(m, 1) || debug_counters_swrden.getnextfull_0_sel != gtmv(m, 1) || debug_counters_swrden.fpm_0_sel != gtmv(m, 1) || debug_counters_swrden.sbpm_0_sel != gtmv(m, 1) || debug_counters_swrden.fbnv_1_sel != gtmv(m, 1) || debug_counters_swrden.fbnempty_1_sel != gtmv(m, 1) || debug_counters_swrden.fbnfull_1_sel != gtmv(m, 1) || debug_counters_swrden.getnext_1_sel != gtmv(m, 1) || debug_counters_swrden.getnextv_1_sel != gtmv(m, 1) || debug_counters_swrden.getnextempty_1_sel != gtmv(m, 1) || debug_counters_swrden.getnextfull_1_sel != gtmv(m, 1) || debug_counters_swrden.fpm_1_sel != gtmv(m, 1) || debug_counters_swrden.sbpm_1_sel != gtmv(m, 1) || debug_counters_swrden.fbn_1_sel != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t rdaddr = gtmv(m, 11);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrdaddr_set(%u %u)\n", bbh_id,
            rdaddr);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrdaddr_set(bbh_id, rdaddr);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrdaddr_get(bbh_id, &rdaddr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrdaddr_get(%u %u)\n", bbh_id,
                rdaddr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (rdaddr != gtmv(m, 11))
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
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrddata_get(bbh_id, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_swrddata_get(%u %u)\n", bbh_id,
                data);
        }
    }

    {
        uint8_t debug_unified_pkt_ctr_idx = gtmv(m, 3);
        uint32_t unified_pkt = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unifiedpkt_get(bbh_id, debug_unified_pkt_ctr_idx, &unified_pkt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unifiedpkt_get(%u %u %u)\n", bbh_id, debug_unified_pkt_ctr_idx,
                unified_pkt);
        }
    }

    {
        uint8_t debug_unified_byte_ctr_idx = gtmv(m, 3);
        uint32_t ddrbyte = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unifiedbyte_get(bbh_id, debug_unified_byte_ctr_idx, &ddrbyte);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unifiedbyte_get(%u %u %u)\n", bbh_id, debug_unified_byte_ctr_idx,
                ddrbyte);
        }
    }

    {
        uint8_t zero = gtmv(m, 2);
        crdp_ds_cmn_bbh_tx_debug_counters_dbgoutreg debug_counters_dbgoutreg = {.debug_out_reg = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_dbgoutreg_get(bbh_id, zero, &debug_counters_dbgoutreg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_dbgoutreg_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero,
                debug_counters_dbgoutreg.debug_out_reg[0], debug_counters_dbgoutreg.debug_out_reg[1], debug_counters_dbgoutreg.debug_out_reg[2], debug_counters_dbgoutreg.debug_out_reg[3], 
                debug_counters_dbgoutreg.debug_out_reg[4], debug_counters_dbgoutreg.debug_out_reg[5], debug_counters_dbgoutreg.debug_out_reg[6], debug_counters_dbgoutreg.debug_out_reg[7]);
        }
    }

    {
        uint32_t in_segmentation = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_in_segmentation_get(bbh_id, &in_segmentation);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_in_segmentation_get(%u %u)\n", bbh_id,
                in_segmentation);
        }
    }

    {
        uint8_t bbh_tx_debug_counters_unified_data_credits_byte_ctr_idx = gtmv(m, 3);
        uint16_t credits = gtmv(m, 12);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unified_data_credits_get(bbh_id, bbh_tx_debug_counters_unified_data_credits_byte_ctr_idx, &credits);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unified_data_credits_get(%u %u %u)\n", bbh_id, bbh_tx_debug_counters_unified_data_credits_byte_ctr_idx,
                credits);
        }
    }

    {
        uint8_t bbh_tx_debug_counters_unified_pd_credits_byte_ctr_idx = gtmv(m, 3);
        uint16_t credits = gtmv(m, 9);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unified_pd_credits_get(bbh_id, bbh_tx_debug_counters_unified_pd_credits_byte_ctr_idx, &credits);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_unified_pd_credits_get(%u %u %u)\n", bbh_id, bbh_tx_debug_counters_unified_pd_credits_byte_ctr_idx,
                credits);
        }
    }

    {
        uint32_t droppkt = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_disflowdroppkt_get(bbh_id, &droppkt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_disflowdroppkt_get(%u %u)\n", bbh_id,
                droppkt);
        }
    }

    {
        uint32_t dropbyte = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_disflowdropbyte_get(bbh_id, &dropbyte);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_disflowdropbyte_get(%u %u)\n", bbh_id,
                dropbyte);
        }
    }

    {
        uint32_t ecnpkt = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnpkt_get(bbh_id, &ecnpkt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnpkt_get(%u %u)\n", bbh_id,
                ecnpkt);
        }
    }

    {
        uint32_t ecnbytes = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnbytes_get(bbh_id, &ecnbytes);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnbytes_get(%u %u)\n", bbh_id,
                ecnbytes);
        }
    }

    {
        uint16_t v1 = gtmv(m, 16);
        uint16_t v2 = gtmv(m, 16);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnmarkerr_get(bbh_id, &v1, &v2);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnmarkerr_get(%u %u %u)\n", bbh_id,
                v1, v2);
        }
    }

    {
        uint16_t v1 = gtmv(m, 16);
        uint16_t v2 = gtmv(m, 16);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnlenerr_get(bbh_id, &v1, &v2);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ecnlenerr_get(%u %u %u)\n", bbh_id,
                v1, v2);
        }
    }

    {
        uint16_t sn = gtmv(m, 16);
        uint8_t pdfifoempty = gtmv(m, 8);
        uint8_t pdarrayempty = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_flush_info_get(bbh_id, &sn, &pdfifoempty, &pdarrayempty);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_flush_info_get(%u %u %u %u)\n", bbh_id,
                sn, pdfifoempty, pdarrayempty);
        }
    }

    {
        uint8_t pfbingempty = gtmv(m, 8);
        uint8_t fepdempty = gtmv(m, 8);
        uint8_t fedataempty = gtmv(m, 8);
        uint8_t portvld = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_flush_info2_get(bbh_id, &pfbingempty, &fepdempty, &fedataempty, &portvld);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_flush_info2_get(%u %u %u %u %u)\n", bbh_id,
                pfbingempty, fepdempty, fedataempty, portvld);
        }
    }

    {
        uint16_t cnt = gtmv(m, 16);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pd_eh_mismatch_get(bbh_id, &cnt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pd_eh_mismatch_get(%u %u)\n", bbh_id,
                cnt);
        }
    }

    {
        uint8_t port = gtmv(m, 3);
        uint16_t plen = gtmv(m, 14);
        uint8_t pktnum = gtmv(m, 7);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pd_eh_mismatch_data_get(bbh_id, &port, &plen, &pktnum);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pd_eh_mismatch_data_get(%u %u %u %u)\n", bbh_id,
                port, plen, pktnum);
        }
    }

    {
        bdmf_boolean macflow = gtmv(m, 1);
        bdmf_boolean ctridx = gtmv(m, 1);
        uint8_t port = gtmv(m, 3);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_outofrange_errors_get(bbh_id, &macflow, &ctridx, &port);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_outofrange_errors_get(%u %u %u %u)\n", bbh_id,
                macflow, ctridx, port);
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint32_t pkt = gtmv(m, 26);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_gemdroppkt_get(bbh_id, index, &pkt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_gemdroppkt_get(%u %u %u)\n", bbh_id, index,
                pkt);
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint32_t bytes = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_gemdropbytes_get(bbh_id, index, &bytes);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_gemdropbytes_get(%u %u %u)\n", bbh_id, index,
                bytes);
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint8_t value = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_cmdotfddr_get(bbh_id, index, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_cmdotfddr_get(%u %u %u)\n", bbh_id, index,
                value);
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t value = gtmv(m, 10);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_bytesotfddr_get(bbh_id, index, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_bytesotfddr_get(%u %u %u)\n", bbh_id, index,
                value);
        }
    }

    {
        uint8_t index = gtmv(m, 4);
        uint8_t value = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_cmdotfepsram_get(bbh_id, index, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_cmdotfepsram_get(%u %u %u)\n", bbh_id, index,
                value);
        }
    }

    {
        uint8_t index = gtmv(m, 4);
        uint16_t value = gtmv(m, 10);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_bytesotfepsram_get(bbh_id, index, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_bytesotfepsram_get(%u %u %u)\n", bbh_id, index,
                value);
        }
    }

    {
        uint8_t index = gtmv(m, 6);
        uint32_t entry = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_gnext_get(bbh_id, index, &entry);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_gnext_get(%u %u %u)\n", bbh_id, index,
                entry);
        }
    }

    {
        uint8_t index = gtmv(m, 5);
        uint32_t entry = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pdarray_get(bbh_id, index, &entry);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_pdarray_get(%u %u %u)\n", bbh_id, index,
                entry);
        }
    }

    {
        uint8_t index = gtmv(m, 2);
        uint32_t entry = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_curpd_get(bbh_id, index, &entry);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_curpd_get(%u %u %u)\n", bbh_id, index,
                entry);
        }
    }

    {
        uint8_t resps = gtmv(m, 8);
        uint8_t target_mem = gtmv(m, 8);
        uint8_t fe = gtmv(m, 8);
        uint8_t dp = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_space4req_get(bbh_id, &resps, &target_mem, &fe, &dp);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_space4req_get(%u %u %u %u %u)\n", bbh_id,
                resps, target_mem, fe, dp);
        }
    }

    {
        uint8_t pdempty = gtmv(m, 8);
        uint8_t gnrdy0 = gtmv(m, 8);
        uint8_t gnrdy1 = gtmv(m, 8);
        uint8_t qrdy = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_seginfo_get(bbh_id, &pdempty, &gnrdy0, &gnrdy1, &qrdy);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_seginfo_get(%u %u %u %u %u)\n", bbh_id,
                pdempty, gnrdy0, gnrdy1, qrdy);
        }
    }

    {
        uint8_t val = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ehdatval_get(bbh_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ehdatval_get(%u %u)\n", bbh_id,
                val);
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ehdat_get(bbh_id, index, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_ehdat_get(%u %u %u)\n", bbh_id, index,
                data);
        }
    }

    {
        uint16_t occ1 = gtmv(m, 9);
        uint16_t occ2 = gtmv(m, 9);
        uint16_t occ3 = gtmv(m, 9);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rmtpdocc_get(bbh_id, &occ1, &occ2, &occ3);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rmtpdocc_get(%u %u %u %u)\n", bbh_id,
                occ1, occ2, occ3);
        }
    }

    {
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rmtbytes_get(bbh_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rmtbytes_get(%u %u)\n", bbh_id,
                val);
        }
    }

    {
        uint8_t index = gtmv(m, 2);
        uint32_t val = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rmtlbcnt_get(bbh_id, index, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rmtlbcnt_get(%u %u %u)\n", bbh_id, index,
                val);
        }
    }

    {
        uint8_t index = gtmv(m, 4);
        uint8_t credit = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_respspd_get(bbh_id, index, &credit);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_respspd_get(%u %u %u)\n", bbh_id, index,
                credit);
        }
    }

    {
        uint8_t index = gtmv(m, 4);
        uint8_t credit = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_respsdata_get(bbh_id, index, &credit);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_respsdata_get(%u %u %u)\n", bbh_id, index,
                credit);
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint32_t value = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rnrsmpdcnt_get(bbh_id, index, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rnrsmpdcnt_get(%u %u %u)\n", bbh_id, index,
                value);
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint32_t value = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rnrsmbytecnt_get(bbh_id, index, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_rnrsmbytecnt_get(%u %u %u)\n", bbh_id, index,
                value);
        }
    }

    {
        uint8_t index = gtmv(m, 4);
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_portdbg_get(bbh_id, index, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_portdbg_get(%u %u %u)\n", bbh_id, index,
                data);
        }
    }

    {
        uint8_t index = gtmv(m, 4);
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_respdbg_get(bbh_id, index, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_debug_counters_respdbg_get(%u %u %u)\n", bbh_id, index,
                data);
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t flow = gtmv(m, 13);
        uint16_t gem = gtmv(m, 16);
        bdmf_boolean valid = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0_set(%u %u %u %u %u)\n", bbh_id, index,
            flow, gem, valid);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0_set(bbh_id, index, flow, gem, valid);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0_get(bbh_id, index, &flow, &gem, &valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0_get(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (flow != gtmv(m, 13) || gem != gtmv(m, 16) || valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t flow = gtmv(m, 13);
        uint16_t gem = gtmv(m, 16);
        bdmf_boolean valid = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1_set(%u %u %u %u %u)\n", bbh_id, index,
            flow, gem, valid);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1_set(bbh_id, index, flow, gem, valid);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1_get(bbh_id, index, &flow, &gem, &valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1_get(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (flow != gtmv(m, 13) || gem != gtmv(m, 16) || valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t flow = gtmv(m, 13);
        uint16_t gem = gtmv(m, 16);
        bdmf_boolean valid = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2_set(%u %u %u %u %u)\n", bbh_id, index,
            flow, gem, valid);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2_set(bbh_id, index, flow, gem, valid);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2_get(bbh_id, index, &flow, &gem, &valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2_get(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (flow != gtmv(m, 13) || gem != gtmv(m, 16) || valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t flow = gtmv(m, 13);
        uint16_t gem = gtmv(m, 16);
        bdmf_boolean valid = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3_set(%u %u %u %u %u)\n", bbh_id, index,
            flow, gem, valid);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3_set(bbh_id, index, flow, gem, valid);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3_get(bbh_id, index, &flow, &gem, &valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3_get(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (flow != gtmv(m, 13) || gem != gtmv(m, 16) || valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t flow = gtmv(m, 13);
        uint16_t gem = gtmv(m, 16);
        bdmf_boolean valid = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4_set(%u %u %u %u %u)\n", bbh_id, index,
            flow, gem, valid);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4_set(bbh_id, index, flow, gem, valid);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4_get(bbh_id, index, &flow, &gem, &valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4_get(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (flow != gtmv(m, 13) || gem != gtmv(m, 16) || valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t flow = gtmv(m, 13);
        uint16_t gem = gtmv(m, 16);
        bdmf_boolean valid = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5_set(%u %u %u %u %u)\n", bbh_id, index,
            flow, gem, valid);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5_set(bbh_id, index, flow, gem, valid);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5_get(bbh_id, index, &flow, &gem, &valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5_get(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (flow != gtmv(m, 13) || gem != gtmv(m, 16) || valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t flow = gtmv(m, 13);
        uint16_t gem = gtmv(m, 16);
        bdmf_boolean valid = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6_set(%u %u %u %u %u)\n", bbh_id, index,
            flow, gem, valid);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6_set(bbh_id, index, flow, gem, valid);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6_get(bbh_id, index, &flow, &gem, &valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6_get(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (flow != gtmv(m, 13) || gem != gtmv(m, 16) || valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 3);
        uint16_t flow = gtmv(m, 13);
        uint16_t gem = gtmv(m, 16);
        bdmf_boolean valid = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7_set(%u %u %u %u %u)\n", bbh_id, index,
            flow, gem, valid);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7_set(bbh_id, index, flow, gem, valid);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7_get(bbh_id, index, &flow, &gem, &valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7_get(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (flow != gtmv(m, 13) || gem != gtmv(m, 16) || valid != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 7);
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val_set(%u %u %u)\n", bbh_id, index,
            val);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val_set(bbh_id, index, val);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val_get(bbh_id, index, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val_get(%u %u %u)\n", bbh_id, index,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t index = gtmv(m, 11);
        uint32_t enable = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en_set(%u %u %u)\n", bbh_id, index,
            enable);
        ag_err = ag_drv_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en_set(bbh_id, index, enable);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en_get(bbh_id, index, &enable);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en_get(%u %u %u)\n", bbh_id, index,
                enable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (enable != gtmv(m, 32))
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
static int ag_drv_crdp_ds_cmn_bbh_tx_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m, input_method = parm[0].value.unumber;
    uint8_t bbh_id = parm[2].value.unumber;
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
    case cli_crdp_ds_cmn_bbh_tx_mac_type:
    {
        uint8_t max_zero = 8;
        uint8_t zero = gtmv(m, 3);
        uint8_t type = gtmv(m, 4);

        if ((start_idx >= max_zero) || (stop_idx >= max_zero))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_zero);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (zero = 0; zero < max_zero; zero++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_mac_type_set(%u %u %u)\n", bbh_id, zero,
                type);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_mac_type_set(bbh_id, zero, type);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", zero);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        type = gtmv(m, 4);

        for (zero = start_idx; zero <= stop_idx; zero++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_mac_type_set(%u %u %u)\n", bbh_id, zero,
                type);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_mac_type_set(bbh_id, zero, type);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", zero);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (zero = 0; zero < max_zero; zero++)
        {
            if (zero < start_idx || zero > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_mac_type_get(bbh_id, zero, &type);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_mac_type_get(%u %u %u)\n", bbh_id, zero,
                type);

            if (type != gtmv(m, 4) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", zero);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of mac_type completed. Number of tested entries %u.\n", max_zero);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg:
    {
        uint8_t max_pd_rnr_cfg_index = 8;
        uint8_t pd_rnr_cfg_index = gtmv(m, 3);
        uint16_t wkpaddr = gtmv(m, 16);
        uint8_t task = gtmv(m, 4);
        uint16_t bbsrcid = gtmv(m, 9);

        if ((start_idx >= max_pd_rnr_cfg_index) || (stop_idx >= max_pd_rnr_cfg_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_pd_rnr_cfg_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (pd_rnr_cfg_index = 0; pd_rnr_cfg_index < max_pd_rnr_cfg_index; pd_rnr_cfg_index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg_set(%u %u %u %u %u)\n", bbh_id, pd_rnr_cfg_index,
                wkpaddr, task, bbsrcid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg_set(bbh_id, pd_rnr_cfg_index, wkpaddr, task, bbsrcid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", pd_rnr_cfg_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        wkpaddr = gtmv(m, 16);
        task = gtmv(m, 4);
        bbsrcid = gtmv(m, 9);

        for (pd_rnr_cfg_index = start_idx; pd_rnr_cfg_index <= stop_idx; pd_rnr_cfg_index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg_set(%u %u %u %u %u)\n", bbh_id, pd_rnr_cfg_index,
                wkpaddr, task, bbsrcid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg_set(bbh_id, pd_rnr_cfg_index, wkpaddr, task, bbsrcid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", pd_rnr_cfg_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (pd_rnr_cfg_index = 0; pd_rnr_cfg_index < max_pd_rnr_cfg_index; pd_rnr_cfg_index++)
        {
            if (pd_rnr_cfg_index < start_idx || pd_rnr_cfg_index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg_get(bbh_id, pd_rnr_cfg_index, &wkpaddr, &task, &bbsrcid);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg_get(%u %u %u %u %u)\n", bbh_id, pd_rnr_cfg_index,
                wkpaddr, task, bbsrcid);

            if (wkpaddr != gtmv(m, 16) || task != gtmv(m, 4) || bbsrcid != gtmv(m, 9) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", pd_rnr_cfg_index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of common_configurations_pdrnrcfg completed. Number of tested entries %u.\n", max_pd_rnr_cfg_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel:
    {
        uint8_t max_ddr_fpmini_basel_index = 2;
        uint8_t ddr_fpmini_basel_index = gtmv(m, 1);
        uint32_t ddrfpminibase = gtmv(m, 32);

        if ((start_idx >= max_ddr_fpmini_basel_index) || (stop_idx >= max_ddr_fpmini_basel_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_ddr_fpmini_basel_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (ddr_fpmini_basel_index = 0; ddr_fpmini_basel_index < max_ddr_fpmini_basel_index; ddr_fpmini_basel_index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel_set(%u %u %u)\n", bbh_id, ddr_fpmini_basel_index,
                ddrfpminibase);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel_set(bbh_id, ddr_fpmini_basel_index, ddrfpminibase);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", ddr_fpmini_basel_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ddrfpminibase = gtmv(m, 32);

        for (ddr_fpmini_basel_index = start_idx; ddr_fpmini_basel_index <= stop_idx; ddr_fpmini_basel_index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel_set(%u %u %u)\n", bbh_id, ddr_fpmini_basel_index,
                ddrfpminibase);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel_set(bbh_id, ddr_fpmini_basel_index, ddrfpminibase);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", ddr_fpmini_basel_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (ddr_fpmini_basel_index = 0; ddr_fpmini_basel_index < max_ddr_fpmini_basel_index; ddr_fpmini_basel_index++)
        {
            if (ddr_fpmini_basel_index < start_idx || ddr_fpmini_basel_index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel_get(bbh_id, ddr_fpmini_basel_index, &ddrfpminibase);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel_get(%u %u %u)\n", bbh_id, ddr_fpmini_basel_index,
                ddrfpminibase);

            if (ddrfpminibase != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", ddr_fpmini_basel_index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of common_configurations_ddrfpminibasel completed. Number of tested entries %u.\n", max_ddr_fpmini_basel_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh:
    {
        uint8_t max_ddr_fpmini_baseh_index = 2;
        uint8_t ddr_fpmini_baseh_index = gtmv(m, 1);
        uint8_t ddrfpminibase = gtmv(m, 8);

        if ((start_idx >= max_ddr_fpmini_baseh_index) || (stop_idx >= max_ddr_fpmini_baseh_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_ddr_fpmini_baseh_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (ddr_fpmini_baseh_index = 0; ddr_fpmini_baseh_index < max_ddr_fpmini_baseh_index; ddr_fpmini_baseh_index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh_set(%u %u %u)\n", bbh_id, ddr_fpmini_baseh_index,
                ddrfpminibase);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh_set(bbh_id, ddr_fpmini_baseh_index, ddrfpminibase);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", ddr_fpmini_baseh_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ddrfpminibase = gtmv(m, 8);

        for (ddr_fpmini_baseh_index = start_idx; ddr_fpmini_baseh_index <= stop_idx; ddr_fpmini_baseh_index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh_set(%u %u %u)\n", bbh_id, ddr_fpmini_baseh_index,
                ddrfpminibase);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh_set(bbh_id, ddr_fpmini_baseh_index, ddrfpminibase);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", ddr_fpmini_baseh_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (ddr_fpmini_baseh_index = 0; ddr_fpmini_baseh_index < max_ddr_fpmini_baseh_index; ddr_fpmini_baseh_index++)
        {
            if (ddr_fpmini_baseh_index < start_idx || ddr_fpmini_baseh_index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh_get(bbh_id, ddr_fpmini_baseh_index, &ddrfpminibase);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh_get(%u %u %u)\n", bbh_id, ddr_fpmini_baseh_index,
                ddrfpminibase);

            if (ddrfpminibase != gtmv(m, 8) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", ddr_fpmini_baseh_index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of common_configurations_ddrfpminibaseh completed. Number of tested entries %u.\n", max_ddr_fpmini_baseh_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel:
    {
        uint8_t max_tm_basel_index = 4;
        uint8_t tm_basel_index = gtmv(m, 2);
        uint32_t ddrtmbase = gtmv(m, 32);

        if ((start_idx >= max_tm_basel_index) || (stop_idx >= max_tm_basel_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_tm_basel_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (tm_basel_index = 0; tm_basel_index < max_tm_basel_index; tm_basel_index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel_set(%u %u %u)\n", bbh_id, tm_basel_index,
                ddrtmbase);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel_set(bbh_id, tm_basel_index, ddrtmbase);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", tm_basel_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ddrtmbase = gtmv(m, 32);

        for (tm_basel_index = start_idx; tm_basel_index <= stop_idx; tm_basel_index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel_set(%u %u %u)\n", bbh_id, tm_basel_index,
                ddrtmbase);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel_set(bbh_id, tm_basel_index, ddrtmbase);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", tm_basel_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (tm_basel_index = 0; tm_basel_index < max_tm_basel_index; tm_basel_index++)
        {
            if (tm_basel_index < start_idx || tm_basel_index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel_get(bbh_id, tm_basel_index, &ddrtmbase);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel_get(%u %u %u)\n", bbh_id, tm_basel_index,
                ddrtmbase);

            if (ddrtmbase != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", tm_basel_index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of common_configurations_tmbasel completed. Number of tested entries %u.\n", max_tm_basel_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh:
    {
        uint8_t max_tm_baseh_index = 4;
        uint8_t tm_baseh_index = gtmv(m, 2);
        uint8_t ddrtmbase = gtmv(m, 8);

        if ((start_idx >= max_tm_baseh_index) || (stop_idx >= max_tm_baseh_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_tm_baseh_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (tm_baseh_index = 0; tm_baseh_index < max_tm_baseh_index; tm_baseh_index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh_set(%u %u %u)\n", bbh_id, tm_baseh_index,
                ddrtmbase);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh_set(bbh_id, tm_baseh_index, ddrtmbase);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", tm_baseh_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ddrtmbase = gtmv(m, 8);

        for (tm_baseh_index = start_idx; tm_baseh_index <= stop_idx; tm_baseh_index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh_set(%u %u %u)\n", bbh_id, tm_baseh_index,
                ddrtmbase);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh_set(bbh_id, tm_baseh_index, ddrtmbase);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", tm_baseh_index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (tm_baseh_index = 0; tm_baseh_index < max_tm_baseh_index; tm_baseh_index++)
        {
            if (tm_baseh_index < start_idx || tm_baseh_index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh_get(bbh_id, tm_baseh_index, &ddrtmbase);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh_get(%u %u %u)\n", bbh_id, tm_baseh_index,
                ddrtmbase);

            if (ddrtmbase != gtmv(m, 8) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", tm_baseh_index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of common_configurations_tmbaseh completed. Number of tested entries %u.\n", max_tm_baseh_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize:
    {
        uint8_t max_index = 24;
        uint8_t index = gtmv(m, 4);
        uint16_t size = gtmv(m, 10);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize_set(%u %u %u)\n", bbh_id, index,
                size);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize_set(bbh_id, index, size);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        size = gtmv(m, 10);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize_set(%u %u %u)\n", bbh_id, index,
                size);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize_set(bbh_id, index, size);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize_get(bbh_id, index, &size);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize_get(%u %u %u)\n", bbh_id, index,
                size);

            if (size != gtmv(m, 10) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of common_configurations_reorderfifosize completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits:
    {
        uint8_t max_index = 24;
        uint8_t index = gtmv(m, 4);
        uint8_t credits = gtmv(m, 8);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits_set(%u %u %u)\n", bbh_id, index,
                credits);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits_set(bbh_id, index, credits);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        credits = gtmv(m, 8);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits_set(%u %u %u)\n", bbh_id, index,
                credits);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits_set(bbh_id, index, credits);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits_get(bbh_id, index, &credits);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits_get(%u %u %u)\n", bbh_id, index,
                credits);

            if (credits != gtmv(m, 8) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of common_configurations_ubuscredits completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t wkpaddr = gtmv(m, 16);
        uint8_t task = gtmv(m, 4);
        uint16_t bbsrcid = gtmv(m, 9);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg_set(%u %u %u %u %u)\n", bbh_id, index,
                wkpaddr, task, bbsrcid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg_set(bbh_id, index, wkpaddr, task, bbsrcid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        wkpaddr = gtmv(m, 16);
        task = gtmv(m, 4);
        bbsrcid = gtmv(m, 9);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg_set(%u %u %u %u %u)\n", bbh_id, index,
                wkpaddr, task, bbsrcid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg_set(bbh_id, index, wkpaddr, task, bbsrcid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg_get(bbh_id, index, &wkpaddr, &task, &bbsrcid);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg_get(%u %u %u %u %u)\n", bbh_id, index,
                wkpaddr, task, bbsrcid);

            if (wkpaddr != gtmv(m, 16) || task != gtmv(m, 4) || bbsrcid != gtmv(m, 9) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of common_configurations_mirrorrnrcfg completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint8_t wkpgran = gtmv(m, 8);
        bdmf_boolean ptrrst = gtmv(m, 1);
        bdmf_boolean enable = gtmv(m, 1);
        uint8_t timer = gtmv(m, 8);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg_set(%u %u %u %u %u %u)\n", bbh_id, index,
                wkpgran, ptrrst, enable, timer);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg_set(bbh_id, index, wkpgran, ptrrst, enable, timer);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        wkpgran = gtmv(m, 8);
        ptrrst = gtmv(m, 1);
        enable = gtmv(m, 1);
        timer = gtmv(m, 8);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg_set(%u %u %u %u %u %u)\n", bbh_id, index,
                wkpgran, ptrrst, enable, timer);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg_set(bbh_id, index, wkpgran, ptrrst, enable, timer);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg_get(bbh_id, index, &wkpgran, &ptrrst, &enable, &timer);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg_get(%u %u %u %u %u %u)\n", bbh_id, index,
                wkpgran, ptrrst, enable, timer);

            if (wkpgran != gtmv(m, 8) || ptrrst != gtmv(m, 1) || enable != gtmv(m, 1) || timer != gtmv(m, 8) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of common_configurations_mirrorcfg completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc:
    {
        uint8_t max_index = 3;
        uint8_t index = gtmv(m, 1);
        uint16_t bbsrcid = gtmv(m, 9);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc_set(%u %u %u)\n", bbh_id, index,
                bbsrcid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc_set(bbh_id, index, bbsrcid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        bbsrcid = gtmv(m, 9);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc_set(%u %u %u)\n", bbh_id, index,
                bbsrcid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc_set(bbh_id, index, bbsrcid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc_get(bbh_id, index, &bbsrcid);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc_get(%u %u %u)\n", bbh_id, index,
                bbsrcid);

            if (bbsrcid != gtmv(m, 9) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of common_configurations_multibbsrc completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t wkpaddr = gtmv(m, 16);
        uint8_t task = gtmv(m, 4);
        uint16_t bbsrcid = gtmv(m, 9);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg_set(%u %u %u %u %u)\n", bbh_id, index,
                wkpaddr, task, bbsrcid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg_set(bbh_id, index, wkpaddr, task, bbsrcid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        wkpaddr = gtmv(m, 16);
        task = gtmv(m, 4);
        bbsrcid = gtmv(m, 9);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg_set(%u %u %u %u %u)\n", bbh_id, index,
                wkpaddr, task, bbsrcid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg_set(bbh_id, index, wkpaddr, task, bbsrcid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg_get(bbh_id, index, &wkpaddr, &task, &bbsrcid);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg_get(%u %u %u %u %u)\n", bbh_id, index,
                wkpaddr, task, bbsrcid);

            if (wkpaddr != gtmv(m, 16) || task != gtmv(m, 4) || bbsrcid != gtmv(m, 9) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of common_configurations_fcrnrcfg completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t fifosize = gtmv(m, 14);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize_set(%u %u %u)\n", bbh_id, index,
                fifosize);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize_set(bbh_id, index, fifosize);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        fifosize = gtmv(m, 14);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize_set(%u %u %u)\n", bbh_id, index,
                fifosize);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize_set(bbh_id, index, fifosize);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize_get(bbh_id, index, &fifosize);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize_get(%u %u %u)\n", bbh_id, index,
                fifosize);

            if (fifosize != gtmv(m, 14) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of per_port_configurations_pdsize completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t wkupthresh = gtmv(m, 14);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph_set(%u %u %u)\n", bbh_id, index,
                wkupthresh);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph_set(bbh_id, index, wkupthresh);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        wkupthresh = gtmv(m, 14);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph_set(%u %u %u)\n", bbh_id, index,
                wkupthresh);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph_set(bbh_id, index, wkupthresh);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph_get(bbh_id, index, &wkupthresh);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph_get(%u %u %u)\n", bbh_id, index,
                wkupthresh);

            if (wkupthresh != gtmv(m, 14) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of per_port_configurations_pdwkuph completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t empty = gtmv(m, 14);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty_set(%u %u %u)\n", bbh_id, index,
                empty);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty_set(bbh_id, index, empty);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        empty = gtmv(m, 14);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty_set(%u %u %u)\n", bbh_id, index,
                empty);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty_set(bbh_id, index, empty);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty_get(bbh_id, index, &empty);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty_get(%u %u %u)\n", bbh_id, index,
                empty);

            if (empty != gtmv(m, 14) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of per_port_configurations_pdempty completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint32_t bytelimit = gtmv(m, 22);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_set(%u %u %u)\n", bbh_id, index,
                bytelimit);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_set(bbh_id, index, bytelimit);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        bytelimit = gtmv(m, 22);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_set(%u %u %u)\n", bbh_id, index,
                bytelimit);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_set(bbh_id, index, bytelimit);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_get(bbh_id, index, &bytelimit);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_get(%u %u %u)\n", bbh_id, index,
                bytelimit);

            if (bytelimit != gtmv(m, 22) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of per_port_configurations_pd_byte_th completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t pdlimit = gtmv(m, 14);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th_set(%u %u %u)\n", bbh_id, index,
                pdlimit);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th_set(bbh_id, index, pdlimit);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        pdlimit = gtmv(m, 14);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th_set(%u %u %u)\n", bbh_id, index,
                pdlimit);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th_set(bbh_id, index, pdlimit);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th_get(bbh_id, index, &pdlimit);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th_get(%u %u %u)\n", bbh_id, index,
                pdlimit);

            if (pdlimit != gtmv(m, 14) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of per_port_configurations_max_pd_th completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t offset0 = gtmv(m, 16);
        uint16_t offset1 = gtmv(m, 16);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset_set(%u %u %u %u)\n", bbh_id, index,
                offset0, offset1);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset_set(bbh_id, index, offset0, offset1);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        offset0 = gtmv(m, 16);
        offset1 = gtmv(m, 16);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset_set(%u %u %u %u)\n", bbh_id, index,
                offset0, offset1);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset_set(bbh_id, index, offset0, offset1);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset_get(bbh_id, index, &offset0, &offset1);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset_get(%u %u %u %u)\n", bbh_id, index,
                offset0, offset1);

            if (offset0 != gtmv(m, 16) || offset1 != gtmv(m, 16) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of per_port_configurations_flowtogemoffset completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t thr = gtmv(m, 13);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr_set(%u %u %u)\n", bbh_id, index,
                thr);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr_set(bbh_id, index, thr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        thr = gtmv(m, 13);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr_set(%u %u %u)\n", bbh_id, index,
                thr);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr_set(bbh_id, index, thr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr_get(bbh_id, index, &thr);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr_get(%u %u %u)\n", bbh_id, index,
                thr);

            if (thr != gtmv(m, 13) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of per_port_configurations_flowtogemthr completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_febase:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t fifobase0 = gtmv(m, 12);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_febase_set(%u %u %u)\n", bbh_id, index,
                fifobase0);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_febase_set(bbh_id, index, fifobase0);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        fifobase0 = gtmv(m, 12);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_febase_set(%u %u %u)\n", bbh_id, index,
                fifobase0);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_febase_set(bbh_id, index, fifobase0);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_febase_get(bbh_id, index, &fifobase0);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_febase_get(%u %u %u)\n", bbh_id, index,
                fifobase0);

            if (fifobase0 != gtmv(m, 12) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of per_port_configurations_febase completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t fifosize = gtmv(m, 12);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize_set(%u %u %u)\n", bbh_id, index,
                fifosize);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize_set(bbh_id, index, fifosize);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        fifosize = gtmv(m, 12);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize_set(%u %u %u)\n", bbh_id, index,
                fifosize);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize_set(bbh_id, index, fifosize);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize_get(bbh_id, index, &fifosize);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize_get(%u %u %u)\n", bbh_id, index,
                fifosize);

            if (fifosize != gtmv(m, 12) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of per_port_configurations_fesize completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t fifobase0 = gtmv(m, 9);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase_set(%u %u %u)\n", bbh_id, index,
                fifobase0);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase_set(bbh_id, index, fifobase0);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        fifobase0 = gtmv(m, 9);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase_set(%u %u %u)\n", bbh_id, index,
                fifobase0);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase_set(bbh_id, index, fifobase0);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase_get(bbh_id, index, &fifobase0);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase_get(%u %u %u)\n", bbh_id, index,
                fifobase0);

            if (fifobase0 != gtmv(m, 9) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of per_port_configurations_fepdbase completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t fifosize0 = gtmv(m, 9);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize_set(%u %u %u)\n", bbh_id, index,
                fifosize0);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize_set(bbh_id, index, fifosize0);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        fifosize0 = gtmv(m, 9);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize_set(%u %u %u)\n", bbh_id, index,
                fifosize0);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize_set(bbh_id, index, fifosize0);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize_get(bbh_id, index, &fifosize0);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize_get(%u %u %u)\n", bbh_id, index,
                fifosize0);

            if (fifosize0 != gtmv(m, 9) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of per_port_configurations_fepdsize completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint8_t w0 = gtmv(m, 4);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr_set(%u %u %u)\n", bbh_id, index,
                w0);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr_set(bbh_id, index, w0);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        w0 = gtmv(m, 4);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr_set(%u %u %u)\n", bbh_id, index,
                w0);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr_set(bbh_id, index, w0);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr_get(bbh_id, index, &w0);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr_get(%u %u %u)\n", bbh_id, index,
                w0);

            if (w0 != gtmv(m, 4) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of per_port_configurations_sgmtwrr completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t ddr = gtmv(m, 10);
        uint16_t psram0 = gtmv(m, 10);
        uint16_t psram1 = gtmv(m, 10);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes_set(%u %u %u %u %u)\n", bbh_id, index,
                ddr, psram0, psram1);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes_set(bbh_id, index, ddr, psram0, psram1);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ddr = gtmv(m, 10);
        psram0 = gtmv(m, 10);
        psram1 = gtmv(m, 10);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes_set(%u %u %u %u %u)\n", bbh_id, index,
                ddr, psram0, psram1);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes_set(bbh_id, index, ddr, psram0, psram1);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes_get(bbh_id, index, &ddr, &psram0, &psram1);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes_get(%u %u %u %u %u)\n", bbh_id, index,
                ddr, psram0, psram1);

            if (ddr != gtmv(m, 10) || psram0 != gtmv(m, 10) || psram1 != gtmv(m, 10) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of per_port_configurations_otfbytes completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint8_t ddr = gtmv(m, 8);
        uint8_t psram0 = gtmv(m, 8);
        uint8_t psram1 = gtmv(m, 8);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd_set(%u %u %u %u %u)\n", bbh_id, index,
                ddr, psram0, psram1);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd_set(bbh_id, index, ddr, psram0, psram1);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ddr = gtmv(m, 8);
        psram0 = gtmv(m, 8);
        psram1 = gtmv(m, 8);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd_set(%u %u %u %u %u)\n", bbh_id, index,
                ddr, psram0, psram1);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd_set(bbh_id, index, ddr, psram0, psram1);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd_get(bbh_id, index, &ddr, &psram0, &psram1);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd_get(%u %u %u %u %u)\n", bbh_id, index,
                ddr, psram0, psram1);

            if (ddr != gtmv(m, 8) || psram0 != gtmv(m, 8) || psram1 != gtmv(m, 8) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of per_port_configurations_otfcmd completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t flow = gtmv(m, 13);
        uint16_t gem = gtmv(m, 16);
        bdmf_boolean valid = gtmv(m, 1);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0_set(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0_set(bbh_id, index, flow, gem, valid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        flow = gtmv(m, 13);
        gem = gtmv(m, 16);
        valid = gtmv(m, 1);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0_set(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0_set(bbh_id, index, flow, gem, valid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0_get(bbh_id, index, &flow, &gem, &valid);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0_get(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);

            if (flow != gtmv(m, 13) || gem != gtmv(m, 16) || valid != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of idx_to_gem_translate_table_entry_0 completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t flow = gtmv(m, 13);
        uint16_t gem = gtmv(m, 16);
        bdmf_boolean valid = gtmv(m, 1);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1_set(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1_set(bbh_id, index, flow, gem, valid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        flow = gtmv(m, 13);
        gem = gtmv(m, 16);
        valid = gtmv(m, 1);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1_set(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1_set(bbh_id, index, flow, gem, valid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1_get(bbh_id, index, &flow, &gem, &valid);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1_get(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);

            if (flow != gtmv(m, 13) || gem != gtmv(m, 16) || valid != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of idx_to_gem_translate_table_entry_1 completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t flow = gtmv(m, 13);
        uint16_t gem = gtmv(m, 16);
        bdmf_boolean valid = gtmv(m, 1);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2_set(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2_set(bbh_id, index, flow, gem, valid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        flow = gtmv(m, 13);
        gem = gtmv(m, 16);
        valid = gtmv(m, 1);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2_set(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2_set(bbh_id, index, flow, gem, valid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2_get(bbh_id, index, &flow, &gem, &valid);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2_get(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);

            if (flow != gtmv(m, 13) || gem != gtmv(m, 16) || valid != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of idx_to_gem_translate_table_entry_2 completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t flow = gtmv(m, 13);
        uint16_t gem = gtmv(m, 16);
        bdmf_boolean valid = gtmv(m, 1);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3_set(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3_set(bbh_id, index, flow, gem, valid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        flow = gtmv(m, 13);
        gem = gtmv(m, 16);
        valid = gtmv(m, 1);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3_set(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3_set(bbh_id, index, flow, gem, valid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3_get(bbh_id, index, &flow, &gem, &valid);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3_get(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);

            if (flow != gtmv(m, 13) || gem != gtmv(m, 16) || valid != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of idx_to_gem_translate_table_entry_3 completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t flow = gtmv(m, 13);
        uint16_t gem = gtmv(m, 16);
        bdmf_boolean valid = gtmv(m, 1);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4_set(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4_set(bbh_id, index, flow, gem, valid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        flow = gtmv(m, 13);
        gem = gtmv(m, 16);
        valid = gtmv(m, 1);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4_set(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4_set(bbh_id, index, flow, gem, valid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4_get(bbh_id, index, &flow, &gem, &valid);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4_get(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);

            if (flow != gtmv(m, 13) || gem != gtmv(m, 16) || valid != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of idx_to_gem_translate_table_entry_4 completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t flow = gtmv(m, 13);
        uint16_t gem = gtmv(m, 16);
        bdmf_boolean valid = gtmv(m, 1);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5_set(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5_set(bbh_id, index, flow, gem, valid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        flow = gtmv(m, 13);
        gem = gtmv(m, 16);
        valid = gtmv(m, 1);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5_set(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5_set(bbh_id, index, flow, gem, valid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5_get(bbh_id, index, &flow, &gem, &valid);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5_get(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);

            if (flow != gtmv(m, 13) || gem != gtmv(m, 16) || valid != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of idx_to_gem_translate_table_entry_5 completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t flow = gtmv(m, 13);
        uint16_t gem = gtmv(m, 16);
        bdmf_boolean valid = gtmv(m, 1);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6_set(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6_set(bbh_id, index, flow, gem, valid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        flow = gtmv(m, 13);
        gem = gtmv(m, 16);
        valid = gtmv(m, 1);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6_set(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6_set(bbh_id, index, flow, gem, valid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6_get(bbh_id, index, &flow, &gem, &valid);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6_get(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);

            if (flow != gtmv(m, 13) || gem != gtmv(m, 16) || valid != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of idx_to_gem_translate_table_entry_6 completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7:
    {
        uint8_t max_index = 8;
        uint8_t index = gtmv(m, 3);
        uint16_t flow = gtmv(m, 13);
        uint16_t gem = gtmv(m, 16);
        bdmf_boolean valid = gtmv(m, 1);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7_set(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7_set(bbh_id, index, flow, gem, valid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        flow = gtmv(m, 13);
        gem = gtmv(m, 16);
        valid = gtmv(m, 1);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7_set(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7_set(bbh_id, index, flow, gem, valid);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7_get(bbh_id, index, &flow, &gem, &valid);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7_get(%u %u %u %u %u)\n", bbh_id, index,
                flow, gem, valid);

            if (flow != gtmv(m, 13) || gem != gtmv(m, 16) || valid != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of idx_to_gem_translate_table_entry_7 completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val:
    {
        uint8_t max_index = 128;
        uint8_t index = gtmv(m, 7);
        uint32_t val = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val_set(%u %u %u)\n", bbh_id, index,
                val);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val_set(bbh_id, index, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        val = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val_set(%u %u %u)\n", bbh_id, index,
                val);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val_set(bbh_id, index, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val_get(bbh_id, index, &val);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val_get(%u %u %u)\n", bbh_id, index,
                val);

            if (val != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of gem_trans_tbl_gem_trns_val completed. Number of tested entries %u.\n", max_index);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en:
    {
        uint8_t max_index = 2048;
        uint8_t index = gtmv(m, 11);
        uint32_t enable = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en_set(%u %u %u)\n", bbh_id, index,
                enable);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en_set(bbh_id, index, enable);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        enable = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en_set(%u %u %u)\n", bbh_id, index,
                enable);
            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en_set(bbh_id, index, enable);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en_get(bbh_id, index, &enable);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en_get(%u %u %u)\n", bbh_id, index,
                enable);

            if (enable != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of mac_flow_en_table_gem_en completed. Number of tested entries %u.\n", max_index);
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
static int ag_drv_crdp_ds_cmn_bbh_tx_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_common_configurations_mactype: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MACTYPE); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_bbcfg_tx: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBCFG_TX); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_sbpmcfg: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_bbsrcid: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBSRCID); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_ddrcfg_tx: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_pdrnrcfg: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_PDRNRCFG); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_txrstcmd: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_dbgsel: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DBGSEL); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_clk_gate_cntrl: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_gpr: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GPR); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_general_cfg: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_GENERAL_CFG); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_ecncfg: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ECNCFG); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_ddrfpminibasel: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRFPMINIBASEL); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_ddrfpminibaseh: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_DDRFPMINIBASEH); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_bbovrdroute: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBOVRDROUTE); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_bbovrdctrl: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_BBOVRDCTRL); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_tmbasel: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TMBASEL); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_tmbaseh: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_TMBASEH); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_reorderfifosize: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_REORDERFIFOSIZE); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_ubuscredits: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_UBUSCREDITS); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_creditinit: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_CREDITINIT); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_mirrorrnrcfg: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORRNRCFG); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_mirrorcfg: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MIRRORCFG); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_multibbhcfg: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MULTIBBHCFG); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_multibbsrc: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_MULTIBBSRC); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_addr_spreader: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_ADDR_SPREADER); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_flowctrcfg: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FLOWCTRCFG); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_fcrnrcfg: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FCRNRCFG); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_common_configurations_fccfg: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, COMMON_CONFIGURATIONS_FCCFG); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_pdack: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDACK); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_pdsize: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDSIZE); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_pdwkuph: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDWKUPH); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_pdempty: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PDEMPTY); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_pd_byte_th: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PD_BYTE_TH); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_max_pd_th: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_MAX_PD_TH); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_pd_byte_th_en: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_flowtogemoffset: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_flowtogemthr: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_febase: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FEBASE); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_fesize: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FESIZE); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_fepdbase: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FEPDBASE); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_fepdsize: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FEPDSIZE); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_sgmtwrr: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_SGMTWRR); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_fe_credits: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_FE_CREDITS); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_gem_en_init: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_GEM_EN_INIT); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_gem_trns_init: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_otfbytes: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFBYTES); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_otfcmd: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_OTFCMD); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_per_port_configurations_autootfcredits: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_txsram0pd: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXSRAM0PD); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_txsram1pd: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXSRAM1PD); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_txddrpd: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXDDRPD); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_pddrop: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PDDROP); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_getnextnull: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_GETNEXTNULL); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_getnextdrop: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_GETNEXTDROP); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_inputfifodrop: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_INPUTFIFODROP); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_sram0pkt: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SRAM0PKT); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_sram1pkt: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SRAM1PKT); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_ddrpkt: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DDRPKT); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_txsram0byte: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXSRAM0BYTE); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_txsram1byte: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXSRAM1BYTE); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_txddrbyte: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_TXDDRBYTE); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_swrden: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDEN); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_swrdaddr: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDADDR); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_swrddata: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SWRDDATA); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_unifiedpkt: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_UNIFIEDPKT); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_unifiedbyte: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_UNIFIEDBYTE); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_dbgoutreg: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DBGOUTREG); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_in_segmentation: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_IN_SEGMENTATION); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_unified_data_credits: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_UNIFIED_DATA_CREDITS); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_unified_pd_credits: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_UNIFIED_PD_CREDITS); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_disflowdroppkt: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DISFLOWDROPPKT); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_disflowdropbyte: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_DISFLOWDROPBYTE); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_ecnpkt: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_ECNPKT); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_ecnbytes: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_ECNBYTES); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_ecnmarkerr: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_ECNMARKERR); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_ecnlenerr: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_ECNLENERR); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_flush_info: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_FLUSH_INFO); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_flush_info2: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_FLUSH_INFO2); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_pd_eh_mismatch: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PD_EH_MISMATCH); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_pd_eh_mismatch_data: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PD_EH_MISMATCH_DATA); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_outofrange_errors: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_OUTOFRANGE_ERRORS); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_gemdroppkt: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_GEMDROPPKT); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_gemdropbytes: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_GEMDROPBYTES); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_cmdotfddr: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_CMDOTFDDR); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_bytesotfddr: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_BYTESOTFDDR); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_cmdotfepsram: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_CMDOTFEPSRAM); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_bytesotfepsram: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_BYTESOTFEPSRAM); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_gnext: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_GNEXT); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_pdarray: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PDARRAY); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_curpd: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_CURPD); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_space4req: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SPACE4REQ); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_seginfo: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_SEGINFO); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_ehdatval: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_EHDATVAL); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_ehdat: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_EHDAT); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_rmtpdocc: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RMTPDOCC); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_rmtbytes: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RMTBYTES); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_rmtlbcnt: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RMTLBCNT); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_respspd: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RESPSPD); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_respsdata: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RESPSDATA); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_rnrsmpdcnt: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RNRSMPDCNT); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_rnrsmbytecnt: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RNRSMBYTECNT); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_portdbg: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_PORTDBG); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_debug_counters_respdbg: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, DEBUG_COUNTERS_RESPDBG); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_idx_to_gem_translate_table_entry_0: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_idx_to_gem_translate_table_entry_1: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_idx_to_gem_translate_table_entry_2: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_idx_to_gem_translate_table_entry_3: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_idx_to_gem_translate_table_entry_4: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_idx_to_gem_translate_table_entry_5: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_idx_to_gem_translate_table_entry_6: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_idx_to_gem_translate_table_entry_7: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_gem_trans_tbl_gem_trns_val: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, GEM_TRANS_TBL_GEM_TRNS_VAL); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
    case bdmf_address_mac_flow_en_table_gem_en: reg = &RU_REG(CRDP_DS_CMN_BBH_TX, MAC_FLOW_EN_TABLE_GEM_EN); blk = &RU_BLK(CRDP_DS_CMN_BBH_TX); break;
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

bdmfmon_handle_t ag_drv_crdp_ds_cmn_bbh_tx_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "crdp_ds_cmn_bbh_tx", "crdp_ds_cmn_bbh_tx", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_mac_type[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("type", "type", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_bbcfg_tx[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sbpmsrc", "sbpmsrc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpmsrc", "fpmsrc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_sbpmcfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("freenocntxt", "freenocntxt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mc_type_3", "mc_type_3", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mc_type_2", "mc_type_2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("second_bn_dis", "second_bn_dis", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bufsize", "bufsize", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enrrspread", "enrrspread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("maxgn", "maxgn", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("listfreetimer", "listfreetimer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("listfreethr", "listfreethr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("smid", "smid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_bbsrcid[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bbsrcid", "bbsrcid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("byp", "byp", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_ddrcfg_tx[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bufsize0", "bufsize0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bufsize1", "bufsize1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("hnsize", "hnsize", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_pdrnrcfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pd_rnr_cfg_index", "pd_rnr_cfg_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("wkpaddr", "wkpaddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("task", "task", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bbsrcid", "bbsrcid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_txrstcmd[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cntxtrst", "cntxtrst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pdfiforst", "pdfiforst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bpmfiforst", "bpmfiforst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sbpmfiforst", "sbpmfiforst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddrfiforst", "ddrfiforst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sramfiforst", "sramfiforst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("skbptrrst", "skbptrrst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gnxtfiforst", "gnxtfiforst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fbnfiforst", "fbnfiforst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("port", "port", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_dbgsel[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dbgsel", "dbgsel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_clk_gate_cntrl[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intrvl", "keep_alive_intrvl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_gpr[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gpr", "gpr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_general_cfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("macdrop", "macdrop", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("discopycrdt", "discopycrdt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ecnrmrk", "ecnrmrk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cntraddcrc", "cntraddcrc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("crcforward", "crcforward", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("multi_tcont", "multi_tcont", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("distargetperport", "distargetperport", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("discrdtchk", "discrdtchk", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_ecncfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ecnbyteipv4", "ecnbyteipv4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ecnbitipv4", "ecnbitipv4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ecnbyteipv6", "ecnbyteipv6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ecnbitipv6", "ecnbitipv6", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("chksumbyteipv4", "chksumbyteipv4", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("assignedqcnt", "assignedqcnt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_ddrfpminibasel[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_fpmini_basel_index", "ddr_fpmini_basel_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddrfpminibase", "ddrfpminibase", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_ddrfpminibaseh[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_fpmini_baseh_index", "ddr_fpmini_baseh_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddrfpminibase", "ddrfpminibase", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_bbovrdroute[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("route", "route", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_bbovrdctrl[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dest", "dest", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_tmbasel[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tm_basel_index", "tm_basel_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddrtmbase", "ddrtmbase", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_tmbaseh[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tm_baseh_index", "tm_baseh_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddrtmbase", "ddrtmbase", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_reorderfifosize[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("size", "size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_ubuscredits[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("credits", "credits", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_creditinit[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("init", "init", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_mirrorrnrcfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("wkpaddr", "wkpaddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("task", "task", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bbsrcid", "bbsrcid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_mirrorcfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("wkpgran", "wkpgran", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ptrrst", "ptrrst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("timer", "timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_multibbhcfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sninitval", "sninitval", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sninit", "sninit", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("lbctrrst", "lbctrrst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mode", "mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_multibbsrc[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bbsrcid", "bbsrcid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_addr_spreader[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("num_active_dram", "num_active_dram", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en_ddr", "en_ddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en_epsr", "en_epsr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("simple_spread", "simple_spread", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("active_dram_0", "active_dram_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("active_dram_1", "active_dram_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("block_spread_zone_en", "block_spread_zone_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("block_spread_zone", "block_spread_zone", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("spare", "spare", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_flowctrcfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("numports", "numports", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("numctrs", "numctrs", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("macflowcnten", "macflowcnten", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ctridxcnten", "ctridxcnten", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_fcrnrcfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("wkpaddr", "wkpaddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("task", "task", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bbsrcid", "bbsrcid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_fccfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fc_stblfilter", "fc_stblfilter", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fc_en", "fc_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_pdack[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("q", "q", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_pdsize[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fifosize", "fifosize", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_pdwkuph[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("wkupthresh", "wkupthresh", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_pdempty[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("empty", "empty", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_pd_byte_th[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bytelimit", "bytelimit", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_max_pd_th[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pdlimit", "pdlimit", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_pd_byte_th_en[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pdbytelimiten", "pdbytelimiten", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pdmaxlimiten", "pdmaxlimiten", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("disfesf", "disfesf", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_flowtogemoffset[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("offset0", "offset0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("offset1", "offset1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_flowtogemthr[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("thr", "thr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_febase[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fifobase0", "fifobase0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_fesize[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fifosize", "fifosize", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_fepdbase[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fifobase0", "fifobase0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_fepdsize[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fifosize0", "fifosize0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_sgmtwrr[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("w0", "w0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_fe_credits[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("init", "init", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("min_pd", "min_pd", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("min_data", "min_data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_gem_en_init[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("init", "init", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_gem_trns_init[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("init", "init", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_otfbytes[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram0", "psram0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram1", "psram1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_otfcmd[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr", "ddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram0", "psram0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psram1", "psram1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_per_port_configurations_autootfcredits[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("numports", "numports", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_counters_swrden[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pdsel", "pdsel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pdvsel", "pdvsel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pdemptysel", "pdemptysel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pdfullsel", "pdfullsel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pdbemptysel", "pdbemptysel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pdffwkpsel", "pdffwkpsel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fbn_0_sel", "fbn_0_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fbnv_0_sel", "fbnv_0_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fbnempty_0_sel", "fbnempty_0_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fbnfull_0_sel", "fbnfull_0_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("getnext_0_sel", "getnext_0_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("getnextv_0_sel", "getnextv_0_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("getnextempty_0_sel", "getnextempty_0_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("getnextfull_0_sel", "getnextfull_0_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_0_sel", "fpm_0_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sbpm_0_sel", "sbpm_0_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fbnv_1_sel", "fbnv_1_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fbnempty_1_sel", "fbnempty_1_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fbnfull_1_sel", "fbnfull_1_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("getnext_1_sel", "getnext_1_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("getnextv_1_sel", "getnextv_1_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("getnextempty_1_sel", "getnextempty_1_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("getnextfull_1_sel", "getnextfull_1_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fpm_1_sel", "fpm_1_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sbpm_1_sel", "sbpm_1_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fbn_1_sel", "fbn_1_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_counters_swrdaddr[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rdaddr", "rdaddr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_idx_to_gem_translate_table_entry_0[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow", "flow", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gem", "gem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_idx_to_gem_translate_table_entry_1[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow", "flow", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gem", "gem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_idx_to_gem_translate_table_entry_2[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow", "flow", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gem", "gem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_idx_to_gem_translate_table_entry_3[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow", "flow", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gem", "gem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_idx_to_gem_translate_table_entry_4[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow", "flow", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gem", "gem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_idx_to_gem_translate_table_entry_5[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow", "flow", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gem", "gem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_idx_to_gem_translate_table_entry_6[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow", "flow", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gem", "gem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_idx_to_gem_translate_table_entry_7[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flow", "flow", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gem", "gem", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gem_trans_tbl_gem_trns_val[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_flow_en_table_gem_en[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "mac_type", .val = cli_crdp_ds_cmn_bbh_tx_mac_type, .parms = set_mac_type },
            { .name = "common_configurations_bbcfg_tx", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_bbcfg_tx, .parms = set_common_configurations_bbcfg_tx },
            { .name = "common_configurations_sbpmcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg, .parms = set_common_configurations_sbpmcfg },
            { .name = "common_configurations_bbsrcid", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_bbsrcid, .parms = set_common_configurations_bbsrcid },
            { .name = "common_configurations_ddrcfg_tx", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrcfg_tx, .parms = set_common_configurations_ddrcfg_tx },
            { .name = "common_configurations_pdrnrcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg, .parms = set_common_configurations_pdrnrcfg },
            { .name = "common_configurations_txrstcmd", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd, .parms = set_common_configurations_txrstcmd },
            { .name = "common_configurations_dbgsel", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_dbgsel, .parms = set_common_configurations_dbgsel },
            { .name = "common_configurations_clk_gate_cntrl", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl, .parms = set_common_configurations_clk_gate_cntrl },
            { .name = "common_configurations_gpr", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_gpr, .parms = set_common_configurations_gpr },
            { .name = "common_configurations_general_cfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_general_cfg, .parms = set_common_configurations_general_cfg },
            { .name = "common_configurations_ecncfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_ecncfg, .parms = set_common_configurations_ecncfg },
            { .name = "common_configurations_ddrfpminibasel", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel, .parms = set_common_configurations_ddrfpminibasel },
            { .name = "common_configurations_ddrfpminibaseh", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh, .parms = set_common_configurations_ddrfpminibaseh },
            { .name = "common_configurations_bbovrdroute", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdroute, .parms = set_common_configurations_bbovrdroute },
            { .name = "common_configurations_bbovrdctrl", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdctrl, .parms = set_common_configurations_bbovrdctrl },
            { .name = "common_configurations_tmbasel", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel, .parms = set_common_configurations_tmbasel },
            { .name = "common_configurations_tmbaseh", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh, .parms = set_common_configurations_tmbaseh },
            { .name = "common_configurations_reorderfifosize", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize, .parms = set_common_configurations_reorderfifosize },
            { .name = "common_configurations_ubuscredits", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits, .parms = set_common_configurations_ubuscredits },
            { .name = "common_configurations_creditinit", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_creditinit, .parms = set_common_configurations_creditinit },
            { .name = "common_configurations_mirrorrnrcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg, .parms = set_common_configurations_mirrorrnrcfg },
            { .name = "common_configurations_mirrorcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg, .parms = set_common_configurations_mirrorcfg },
            { .name = "common_configurations_multibbhcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_multibbhcfg, .parms = set_common_configurations_multibbhcfg },
            { .name = "common_configurations_multibbsrc", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc, .parms = set_common_configurations_multibbsrc },
            { .name = "common_configurations_addr_spreader", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader, .parms = set_common_configurations_addr_spreader },
            { .name = "common_configurations_flowctrcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_flowctrcfg, .parms = set_common_configurations_flowctrcfg },
            { .name = "common_configurations_fcrnrcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg, .parms = set_common_configurations_fcrnrcfg },
            { .name = "common_configurations_fccfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_fccfg, .parms = set_common_configurations_fccfg },
            { .name = "per_port_configurations_pdack", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdack, .parms = set_per_port_configurations_pdack },
            { .name = "per_port_configurations_pdsize", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize, .parms = set_per_port_configurations_pdsize },
            { .name = "per_port_configurations_pdwkuph", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph, .parms = set_per_port_configurations_pdwkuph },
            { .name = "per_port_configurations_pdempty", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty, .parms = set_per_port_configurations_pdempty },
            { .name = "per_port_configurations_pd_byte_th", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th, .parms = set_per_port_configurations_pd_byte_th },
            { .name = "per_port_configurations_max_pd_th", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th, .parms = set_per_port_configurations_max_pd_th },
            { .name = "per_port_configurations_pd_byte_th_en", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_en, .parms = set_per_port_configurations_pd_byte_th_en },
            { .name = "per_port_configurations_flowtogemoffset", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset, .parms = set_per_port_configurations_flowtogemoffset },
            { .name = "per_port_configurations_flowtogemthr", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr, .parms = set_per_port_configurations_flowtogemthr },
            { .name = "per_port_configurations_febase", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_febase, .parms = set_per_port_configurations_febase },
            { .name = "per_port_configurations_fesize", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize, .parms = set_per_port_configurations_fesize },
            { .name = "per_port_configurations_fepdbase", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase, .parms = set_per_port_configurations_fepdbase },
            { .name = "per_port_configurations_fepdsize", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize, .parms = set_per_port_configurations_fepdsize },
            { .name = "per_port_configurations_sgmtwrr", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr, .parms = set_per_port_configurations_sgmtwrr },
            { .name = "per_port_configurations_fe_credits", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fe_credits, .parms = set_per_port_configurations_fe_credits },
            { .name = "per_port_configurations_gem_en_init", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_en_init, .parms = set_per_port_configurations_gem_en_init },
            { .name = "per_port_configurations_gem_trns_init", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_trns_init, .parms = set_per_port_configurations_gem_trns_init },
            { .name = "per_port_configurations_otfbytes", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes, .parms = set_per_port_configurations_otfbytes },
            { .name = "per_port_configurations_otfcmd", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd, .parms = set_per_port_configurations_otfcmd },
            { .name = "per_port_configurations_autootfcredits", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_autootfcredits, .parms = set_per_port_configurations_autootfcredits },
            { .name = "debug_counters_swrden", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_swrden, .parms = set_debug_counters_swrden },
            { .name = "debug_counters_swrdaddr", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_swrdaddr, .parms = set_debug_counters_swrdaddr },
            { .name = "idx_to_gem_translate_table_entry_0", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0, .parms = set_idx_to_gem_translate_table_entry_0 },
            { .name = "idx_to_gem_translate_table_entry_1", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1, .parms = set_idx_to_gem_translate_table_entry_1 },
            { .name = "idx_to_gem_translate_table_entry_2", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2, .parms = set_idx_to_gem_translate_table_entry_2 },
            { .name = "idx_to_gem_translate_table_entry_3", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3, .parms = set_idx_to_gem_translate_table_entry_3 },
            { .name = "idx_to_gem_translate_table_entry_4", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4, .parms = set_idx_to_gem_translate_table_entry_4 },
            { .name = "idx_to_gem_translate_table_entry_5", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5, .parms = set_idx_to_gem_translate_table_entry_5 },
            { .name = "idx_to_gem_translate_table_entry_6", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6, .parms = set_idx_to_gem_translate_table_entry_6 },
            { .name = "idx_to_gem_translate_table_entry_7", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7, .parms = set_idx_to_gem_translate_table_entry_7 },
            { .name = "gem_trans_tbl_gem_trns_val", .val = cli_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val, .parms = set_gem_trans_tbl_gem_trns_val },
            { .name = "mac_flow_en_table_gem_en", .val = cli_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en, .parms = set_mac_flow_en_table_gem_en },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_crdp_ds_cmn_bbh_tx_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_mac_type[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_common_configurations_pdrnrcfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pd_rnr_cfg_index", "pd_rnr_cfg_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_common_configurations_ddrfpminibasel[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_fpmini_basel_index", "ddr_fpmini_basel_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_common_configurations_ddrfpminibaseh[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_fpmini_baseh_index", "ddr_fpmini_baseh_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_common_configurations_tmbasel[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tm_basel_index", "tm_basel_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_common_configurations_tmbaseh[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tm_baseh_index", "tm_baseh_index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_common_configurations_reorderfifosize[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_common_configurations_ubuscredits[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_common_configurations_mirrorrnrcfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_common_configurations_mirrorcfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_common_configurations_multibbsrc[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_common_configurations_fcrnrcfg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_per_port_configurations_pdsize[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_per_port_configurations_pdwkuph[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_per_port_configurations_pdempty[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_per_port_configurations_pd_byte_th[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_per_port_configurations_max_pd_th[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_per_port_configurations_flowtogemoffset[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_per_port_configurations_flowtogemthr[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_per_port_configurations_febase[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_per_port_configurations_fesize[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_per_port_configurations_fepdbase[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_per_port_configurations_fepdsize[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_per_port_configurations_sgmtwrr[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_per_port_configurations_otfbytes[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_per_port_configurations_otfcmd[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_unifiedpkt[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("debug_unified_pkt_ctr_idx", "debug_unified_pkt_ctr_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_unifiedbyte[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("debug_unified_byte_ctr_idx", "debug_unified_byte_ctr_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_dbgoutreg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_unified_data_credits[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bbh_tx_debug_counters_unified_data_credits_byte_ctr_idx", "bbh_tx_debug_counters_unified_data_credits_byte_ctr_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_unified_pd_credits[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bbh_tx_debug_counters_unified_pd_credits_byte_ctr_idx", "bbh_tx_debug_counters_unified_pd_credits_byte_ctr_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_gemdroppkt[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_gemdropbytes[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_cmdotfddr[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_bytesotfddr[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_cmdotfepsram[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_bytesotfepsram[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_gnext[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_pdarray[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_curpd[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_ehdat[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_rmtlbcnt[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_respspd[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_respsdata[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_rnrsmpdcnt[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_rnrsmbytecnt[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_portdbg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_debug_counters_respdbg[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_idx_to_gem_translate_table_entry_0[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_idx_to_gem_translate_table_entry_1[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_idx_to_gem_translate_table_entry_2[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_idx_to_gem_translate_table_entry_3[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_idx_to_gem_translate_table_entry_4[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_idx_to_gem_translate_table_entry_5[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_idx_to_gem_translate_table_entry_6[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_idx_to_gem_translate_table_entry_7[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_gem_trans_tbl_gem_trns_val[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_mac_flow_en_table_gem_en[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "mac_type", .val = cli_crdp_ds_cmn_bbh_tx_mac_type, .parms = get_mac_type },
            { .name = "debug_counters_ddrbyte", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_ddrbyte, .parms = get_default },
            { .name = "debug_counters_sram0byte", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_sram0byte, .parms = get_default },
            { .name = "debug_counters_sram1byte", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_sram1byte, .parms = get_default },
            { .name = "common_configurations_bbcfg_tx", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_bbcfg_tx, .parms = get_default },
            { .name = "common_configurations_sbpmcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_sbpmcfg, .parms = get_default },
            { .name = "common_configurations_bbsrcid", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_bbsrcid, .parms = get_default },
            { .name = "common_configurations_ddrcfg_tx", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrcfg_tx, .parms = get_default },
            { .name = "common_configurations_pdrnrcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg, .parms = get_common_configurations_pdrnrcfg },
            { .name = "common_configurations_txrstcmd", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_txrstcmd, .parms = get_default },
            { .name = "common_configurations_dbgsel", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_dbgsel, .parms = get_default },
            { .name = "common_configurations_clk_gate_cntrl", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_clk_gate_cntrl, .parms = get_default },
            { .name = "common_configurations_gpr", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_gpr, .parms = get_default },
            { .name = "common_configurations_general_cfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_general_cfg, .parms = get_default },
            { .name = "common_configurations_ecncfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_ecncfg, .parms = get_default },
            { .name = "common_configurations_ddrfpminibasel", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel, .parms = get_common_configurations_ddrfpminibasel },
            { .name = "common_configurations_ddrfpminibaseh", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh, .parms = get_common_configurations_ddrfpminibaseh },
            { .name = "common_configurations_bbovrdroute", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdroute, .parms = get_default },
            { .name = "common_configurations_bbovrdctrl", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_bbovrdctrl, .parms = get_default },
            { .name = "common_configurations_tmbasel", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel, .parms = get_common_configurations_tmbasel },
            { .name = "common_configurations_tmbaseh", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh, .parms = get_common_configurations_tmbaseh },
            { .name = "common_configurations_reorderfifosize", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize, .parms = get_common_configurations_reorderfifosize },
            { .name = "common_configurations_ubuscredits", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits, .parms = get_common_configurations_ubuscredits },
            { .name = "common_configurations_creditinit", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_creditinit, .parms = get_default },
            { .name = "common_configurations_mirrorrnrcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg, .parms = get_common_configurations_mirrorrnrcfg },
            { .name = "common_configurations_mirrorcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg, .parms = get_common_configurations_mirrorcfg },
            { .name = "common_configurations_multibbhcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_multibbhcfg, .parms = get_default },
            { .name = "common_configurations_multibbsrc", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc, .parms = get_common_configurations_multibbsrc },
            { .name = "common_configurations_addr_spreader", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_addr_spreader, .parms = get_default },
            { .name = "common_configurations_flowctrcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_flowctrcfg, .parms = get_default },
            { .name = "common_configurations_fcrnrcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg, .parms = get_common_configurations_fcrnrcfg },
            { .name = "common_configurations_fccfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_fccfg, .parms = get_default },
            { .name = "per_port_configurations_pdack", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdack, .parms = get_default },
            { .name = "per_port_configurations_pdsize", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize, .parms = get_per_port_configurations_pdsize },
            { .name = "per_port_configurations_pdwkuph", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph, .parms = get_per_port_configurations_pdwkuph },
            { .name = "per_port_configurations_pdempty", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty, .parms = get_per_port_configurations_pdempty },
            { .name = "per_port_configurations_pd_byte_th", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th, .parms = get_per_port_configurations_pd_byte_th },
            { .name = "per_port_configurations_max_pd_th", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th, .parms = get_per_port_configurations_max_pd_th },
            { .name = "per_port_configurations_pd_byte_th_en", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th_en, .parms = get_default },
            { .name = "per_port_configurations_flowtogemoffset", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset, .parms = get_per_port_configurations_flowtogemoffset },
            { .name = "per_port_configurations_flowtogemthr", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr, .parms = get_per_port_configurations_flowtogemthr },
            { .name = "per_port_configurations_febase", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_febase, .parms = get_per_port_configurations_febase },
            { .name = "per_port_configurations_fesize", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize, .parms = get_per_port_configurations_fesize },
            { .name = "per_port_configurations_fepdbase", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase, .parms = get_per_port_configurations_fepdbase },
            { .name = "per_port_configurations_fepdsize", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize, .parms = get_per_port_configurations_fepdsize },
            { .name = "per_port_configurations_sgmtwrr", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr, .parms = get_per_port_configurations_sgmtwrr },
            { .name = "per_port_configurations_fe_credits", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fe_credits, .parms = get_default },
            { .name = "per_port_configurations_gem_en_init", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_en_init, .parms = get_default },
            { .name = "per_port_configurations_gem_trns_init", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_gem_trns_init, .parms = get_default },
            { .name = "per_port_configurations_otfbytes", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes, .parms = get_per_port_configurations_otfbytes },
            { .name = "per_port_configurations_otfcmd", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd, .parms = get_per_port_configurations_otfcmd },
            { .name = "per_port_configurations_autootfcredits", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_autootfcredits, .parms = get_default },
            { .name = "debug_counters_txsram0pd", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_txsram0pd, .parms = get_default },
            { .name = "debug_counters_txsram1pd", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_txsram1pd, .parms = get_default },
            { .name = "debug_counters_txddrpd", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_txddrpd, .parms = get_default },
            { .name = "debug_counters_pddrop", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_pddrop, .parms = get_default },
            { .name = "debug_counters_getnextnull", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_getnextnull, .parms = get_default },
            { .name = "debug_counters_getnextdrop", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_getnextdrop, .parms = get_default },
            { .name = "debug_counters_inputfifodrop", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_inputfifodrop, .parms = get_default },
            { .name = "debug_counters_sram0pkt", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_sram0pkt, .parms = get_default },
            { .name = "debug_counters_sram1pkt", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_sram1pkt, .parms = get_default },
            { .name = "debug_counters_ddrpkt", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_ddrpkt, .parms = get_default },
            { .name = "debug_counters_swrden", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_swrden, .parms = get_default },
            { .name = "debug_counters_swrdaddr", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_swrdaddr, .parms = get_default },
            { .name = "debug_counters_swrddata", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_swrddata, .parms = get_default },
            { .name = "debug_counters_unifiedpkt", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_unifiedpkt, .parms = get_debug_counters_unifiedpkt },
            { .name = "debug_counters_unifiedbyte", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_unifiedbyte, .parms = get_debug_counters_unifiedbyte },
            { .name = "debug_counters_dbgoutreg", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_dbgoutreg, .parms = get_debug_counters_dbgoutreg },
            { .name = "debug_counters_in_segmentation", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_in_segmentation, .parms = get_default },
            { .name = "debug_counters_unified_data_credits", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_unified_data_credits, .parms = get_debug_counters_unified_data_credits },
            { .name = "debug_counters_unified_pd_credits", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_unified_pd_credits, .parms = get_debug_counters_unified_pd_credits },
            { .name = "debug_counters_disflowdroppkt", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_disflowdroppkt, .parms = get_default },
            { .name = "debug_counters_disflowdropbyte", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_disflowdropbyte, .parms = get_default },
            { .name = "debug_counters_ecnpkt", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_ecnpkt, .parms = get_default },
            { .name = "debug_counters_ecnbytes", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_ecnbytes, .parms = get_default },
            { .name = "debug_counters_ecnmarkerr", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_ecnmarkerr, .parms = get_default },
            { .name = "debug_counters_ecnlenerr", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_ecnlenerr, .parms = get_default },
            { .name = "debug_counters_flush_info", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_flush_info, .parms = get_default },
            { .name = "debug_counters_flush_info2", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_flush_info2, .parms = get_default },
            { .name = "debug_counters_pd_eh_mismatch", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_pd_eh_mismatch, .parms = get_default },
            { .name = "debug_counters_pd_eh_mismatch_data", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_pd_eh_mismatch_data, .parms = get_default },
            { .name = "debug_counters_outofrange_errors", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_outofrange_errors, .parms = get_default },
            { .name = "debug_counters_gemdroppkt", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_gemdroppkt, .parms = get_debug_counters_gemdroppkt },
            { .name = "debug_counters_gemdropbytes", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_gemdropbytes, .parms = get_debug_counters_gemdropbytes },
            { .name = "debug_counters_cmdotfddr", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_cmdotfddr, .parms = get_debug_counters_cmdotfddr },
            { .name = "debug_counters_bytesotfddr", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_bytesotfddr, .parms = get_debug_counters_bytesotfddr },
            { .name = "debug_counters_cmdotfepsram", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_cmdotfepsram, .parms = get_debug_counters_cmdotfepsram },
            { .name = "debug_counters_bytesotfepsram", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_bytesotfepsram, .parms = get_debug_counters_bytesotfepsram },
            { .name = "debug_counters_gnext", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_gnext, .parms = get_debug_counters_gnext },
            { .name = "debug_counters_pdarray", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_pdarray, .parms = get_debug_counters_pdarray },
            { .name = "debug_counters_curpd", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_curpd, .parms = get_debug_counters_curpd },
            { .name = "debug_counters_space4req", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_space4req, .parms = get_default },
            { .name = "debug_counters_seginfo", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_seginfo, .parms = get_default },
            { .name = "debug_counters_ehdatval", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_ehdatval, .parms = get_default },
            { .name = "debug_counters_ehdat", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_ehdat, .parms = get_debug_counters_ehdat },
            { .name = "debug_counters_rmtpdocc", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_rmtpdocc, .parms = get_default },
            { .name = "debug_counters_rmtbytes", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_rmtbytes, .parms = get_default },
            { .name = "debug_counters_rmtlbcnt", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_rmtlbcnt, .parms = get_debug_counters_rmtlbcnt },
            { .name = "debug_counters_respspd", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_respspd, .parms = get_debug_counters_respspd },
            { .name = "debug_counters_respsdata", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_respsdata, .parms = get_debug_counters_respsdata },
            { .name = "debug_counters_rnrsmpdcnt", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_rnrsmpdcnt, .parms = get_debug_counters_rnrsmpdcnt },
            { .name = "debug_counters_rnrsmbytecnt", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_rnrsmbytecnt, .parms = get_debug_counters_rnrsmbytecnt },
            { .name = "debug_counters_portdbg", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_portdbg, .parms = get_debug_counters_portdbg },
            { .name = "debug_counters_respdbg", .val = cli_crdp_ds_cmn_bbh_tx_debug_counters_respdbg, .parms = get_debug_counters_respdbg },
            { .name = "idx_to_gem_translate_table_entry_0", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0, .parms = get_idx_to_gem_translate_table_entry_0 },
            { .name = "idx_to_gem_translate_table_entry_1", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1, .parms = get_idx_to_gem_translate_table_entry_1 },
            { .name = "idx_to_gem_translate_table_entry_2", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2, .parms = get_idx_to_gem_translate_table_entry_2 },
            { .name = "idx_to_gem_translate_table_entry_3", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3, .parms = get_idx_to_gem_translate_table_entry_3 },
            { .name = "idx_to_gem_translate_table_entry_4", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4, .parms = get_idx_to_gem_translate_table_entry_4 },
            { .name = "idx_to_gem_translate_table_entry_5", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5, .parms = get_idx_to_gem_translate_table_entry_5 },
            { .name = "idx_to_gem_translate_table_entry_6", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6, .parms = get_idx_to_gem_translate_table_entry_6 },
            { .name = "idx_to_gem_translate_table_entry_7", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7, .parms = get_idx_to_gem_translate_table_entry_7 },
            { .name = "gem_trans_tbl_gem_trns_val", .val = cli_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val, .parms = get_gem_trans_tbl_gem_trns_val },
            { .name = "mac_flow_en_table_gem_en", .val = cli_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en, .parms = get_mac_flow_en_table_gem_en },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_crdp_ds_cmn_bbh_tx_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_crdp_ds_cmn_bbh_tx_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "mac_type", .val = cli_crdp_ds_cmn_bbh_tx_mac_type, .parms = ext_test_default},
            { .name = "common_configurations_pdrnrcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_pdrnrcfg, .parms = ext_test_default},
            { .name = "common_configurations_ddrfpminibasel", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibasel, .parms = ext_test_default},
            { .name = "common_configurations_ddrfpminibaseh", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_ddrfpminibaseh, .parms = ext_test_default},
            { .name = "common_configurations_tmbasel", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_tmbasel, .parms = ext_test_default},
            { .name = "common_configurations_tmbaseh", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_tmbaseh, .parms = ext_test_default},
            { .name = "common_configurations_reorderfifosize", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_reorderfifosize, .parms = ext_test_default},
            { .name = "common_configurations_ubuscredits", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_ubuscredits, .parms = ext_test_default},
            { .name = "common_configurations_mirrorrnrcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_mirrorrnrcfg, .parms = ext_test_default},
            { .name = "common_configurations_mirrorcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_mirrorcfg, .parms = ext_test_default},
            { .name = "common_configurations_multibbsrc", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_multibbsrc, .parms = ext_test_default},
            { .name = "common_configurations_fcrnrcfg", .val = cli_crdp_ds_cmn_bbh_tx_common_configurations_fcrnrcfg, .parms = ext_test_default},
            { .name = "per_port_configurations_pdsize", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdsize, .parms = ext_test_default},
            { .name = "per_port_configurations_pdwkuph", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdwkuph, .parms = ext_test_default},
            { .name = "per_port_configurations_pdempty", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pdempty, .parms = ext_test_default},
            { .name = "per_port_configurations_pd_byte_th", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_pd_byte_th, .parms = ext_test_default},
            { .name = "per_port_configurations_max_pd_th", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_max_pd_th, .parms = ext_test_default},
            { .name = "per_port_configurations_flowtogemoffset", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemoffset, .parms = ext_test_default},
            { .name = "per_port_configurations_flowtogemthr", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_flowtogemthr, .parms = ext_test_default},
            { .name = "per_port_configurations_febase", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_febase, .parms = ext_test_default},
            { .name = "per_port_configurations_fesize", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fesize, .parms = ext_test_default},
            { .name = "per_port_configurations_fepdbase", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdbase, .parms = ext_test_default},
            { .name = "per_port_configurations_fepdsize", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_fepdsize, .parms = ext_test_default},
            { .name = "per_port_configurations_sgmtwrr", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_sgmtwrr, .parms = ext_test_default},
            { .name = "per_port_configurations_otfbytes", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_otfbytes, .parms = ext_test_default},
            { .name = "per_port_configurations_otfcmd", .val = cli_crdp_ds_cmn_bbh_tx_per_port_configurations_otfcmd, .parms = ext_test_default},
            { .name = "idx_to_gem_translate_table_entry_0", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_0, .parms = ext_test_default},
            { .name = "idx_to_gem_translate_table_entry_1", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_1, .parms = ext_test_default},
            { .name = "idx_to_gem_translate_table_entry_2", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_2, .parms = ext_test_default},
            { .name = "idx_to_gem_translate_table_entry_3", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_3, .parms = ext_test_default},
            { .name = "idx_to_gem_translate_table_entry_4", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_4, .parms = ext_test_default},
            { .name = "idx_to_gem_translate_table_entry_5", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_5, .parms = ext_test_default},
            { .name = "idx_to_gem_translate_table_entry_6", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_6, .parms = ext_test_default},
            { .name = "idx_to_gem_translate_table_entry_7", .val = cli_crdp_ds_cmn_bbh_tx_idx_to_gem_translate_table_entry_7, .parms = ext_test_default},
            { .name = "gem_trans_tbl_gem_trns_val", .val = cli_crdp_ds_cmn_bbh_tx_gem_trans_tbl_gem_trns_val, .parms = ext_test_default},
            { .name = "mac_flow_en_table_gem_en", .val = cli_crdp_ds_cmn_bbh_tx_mac_flow_en_table_gem_en, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_crdp_ds_cmn_bbh_tx_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "COMMON_CONFIGURATIONS_MACTYPE", .val = bdmf_address_common_configurations_mactype },
            { .name = "COMMON_CONFIGURATIONS_BBCFG_TX", .val = bdmf_address_common_configurations_bbcfg_tx },
            { .name = "COMMON_CONFIGURATIONS_SBPMCFG", .val = bdmf_address_common_configurations_sbpmcfg },
            { .name = "COMMON_CONFIGURATIONS_BBSRCID", .val = bdmf_address_common_configurations_bbsrcid },
            { .name = "COMMON_CONFIGURATIONS_DDRCFG_TX", .val = bdmf_address_common_configurations_ddrcfg_tx },
            { .name = "COMMON_CONFIGURATIONS_PDRNRCFG", .val = bdmf_address_common_configurations_pdrnrcfg },
            { .name = "COMMON_CONFIGURATIONS_TXRSTCMD", .val = bdmf_address_common_configurations_txrstcmd },
            { .name = "COMMON_CONFIGURATIONS_DBGSEL", .val = bdmf_address_common_configurations_dbgsel },
            { .name = "COMMON_CONFIGURATIONS_CLK_GATE_CNTRL", .val = bdmf_address_common_configurations_clk_gate_cntrl },
            { .name = "COMMON_CONFIGURATIONS_GPR", .val = bdmf_address_common_configurations_gpr },
            { .name = "COMMON_CONFIGURATIONS_GENERAL_CFG", .val = bdmf_address_common_configurations_general_cfg },
            { .name = "COMMON_CONFIGURATIONS_ECNCFG", .val = bdmf_address_common_configurations_ecncfg },
            { .name = "COMMON_CONFIGURATIONS_DDRFPMINIBASEL", .val = bdmf_address_common_configurations_ddrfpminibasel },
            { .name = "COMMON_CONFIGURATIONS_DDRFPMINIBASEH", .val = bdmf_address_common_configurations_ddrfpminibaseh },
            { .name = "COMMON_CONFIGURATIONS_BBOVRDROUTE", .val = bdmf_address_common_configurations_bbovrdroute },
            { .name = "COMMON_CONFIGURATIONS_BBOVRDCTRL", .val = bdmf_address_common_configurations_bbovrdctrl },
            { .name = "COMMON_CONFIGURATIONS_TMBASEL", .val = bdmf_address_common_configurations_tmbasel },
            { .name = "COMMON_CONFIGURATIONS_TMBASEH", .val = bdmf_address_common_configurations_tmbaseh },
            { .name = "COMMON_CONFIGURATIONS_REORDERFIFOSIZE", .val = bdmf_address_common_configurations_reorderfifosize },
            { .name = "COMMON_CONFIGURATIONS_UBUSCREDITS", .val = bdmf_address_common_configurations_ubuscredits },
            { .name = "COMMON_CONFIGURATIONS_CREDITINIT", .val = bdmf_address_common_configurations_creditinit },
            { .name = "COMMON_CONFIGURATIONS_MIRRORRNRCFG", .val = bdmf_address_common_configurations_mirrorrnrcfg },
            { .name = "COMMON_CONFIGURATIONS_MIRRORCFG", .val = bdmf_address_common_configurations_mirrorcfg },
            { .name = "COMMON_CONFIGURATIONS_MULTIBBHCFG", .val = bdmf_address_common_configurations_multibbhcfg },
            { .name = "COMMON_CONFIGURATIONS_MULTIBBSRC", .val = bdmf_address_common_configurations_multibbsrc },
            { .name = "COMMON_CONFIGURATIONS_ADDR_SPREADER", .val = bdmf_address_common_configurations_addr_spreader },
            { .name = "COMMON_CONFIGURATIONS_FLOWCTRCFG", .val = bdmf_address_common_configurations_flowctrcfg },
            { .name = "COMMON_CONFIGURATIONS_FCRNRCFG", .val = bdmf_address_common_configurations_fcrnrcfg },
            { .name = "COMMON_CONFIGURATIONS_FCCFG", .val = bdmf_address_common_configurations_fccfg },
            { .name = "PER_PORT_CONFIGURATIONS_PDACK", .val = bdmf_address_per_port_configurations_pdack },
            { .name = "PER_PORT_CONFIGURATIONS_PDSIZE", .val = bdmf_address_per_port_configurations_pdsize },
            { .name = "PER_PORT_CONFIGURATIONS_PDWKUPH", .val = bdmf_address_per_port_configurations_pdwkuph },
            { .name = "PER_PORT_CONFIGURATIONS_PDEMPTY", .val = bdmf_address_per_port_configurations_pdempty },
            { .name = "PER_PORT_CONFIGURATIONS_PD_BYTE_TH", .val = bdmf_address_per_port_configurations_pd_byte_th },
            { .name = "PER_PORT_CONFIGURATIONS_MAX_PD_TH", .val = bdmf_address_per_port_configurations_max_pd_th },
            { .name = "PER_PORT_CONFIGURATIONS_PD_BYTE_TH_EN", .val = bdmf_address_per_port_configurations_pd_byte_th_en },
            { .name = "PER_PORT_CONFIGURATIONS_FLOWTOGEMOFFSET", .val = bdmf_address_per_port_configurations_flowtogemoffset },
            { .name = "PER_PORT_CONFIGURATIONS_FLOWTOGEMTHR", .val = bdmf_address_per_port_configurations_flowtogemthr },
            { .name = "PER_PORT_CONFIGURATIONS_FEBASE", .val = bdmf_address_per_port_configurations_febase },
            { .name = "PER_PORT_CONFIGURATIONS_FESIZE", .val = bdmf_address_per_port_configurations_fesize },
            { .name = "PER_PORT_CONFIGURATIONS_FEPDBASE", .val = bdmf_address_per_port_configurations_fepdbase },
            { .name = "PER_PORT_CONFIGURATIONS_FEPDSIZE", .val = bdmf_address_per_port_configurations_fepdsize },
            { .name = "PER_PORT_CONFIGURATIONS_SGMTWRR", .val = bdmf_address_per_port_configurations_sgmtwrr },
            { .name = "PER_PORT_CONFIGURATIONS_FE_CREDITS", .val = bdmf_address_per_port_configurations_fe_credits },
            { .name = "PER_PORT_CONFIGURATIONS_GEM_EN_INIT", .val = bdmf_address_per_port_configurations_gem_en_init },
            { .name = "PER_PORT_CONFIGURATIONS_GEM_TRNS_INIT", .val = bdmf_address_per_port_configurations_gem_trns_init },
            { .name = "PER_PORT_CONFIGURATIONS_OTFBYTES", .val = bdmf_address_per_port_configurations_otfbytes },
            { .name = "PER_PORT_CONFIGURATIONS_OTFCMD", .val = bdmf_address_per_port_configurations_otfcmd },
            { .name = "PER_PORT_CONFIGURATIONS_AUTOOTFCREDITS", .val = bdmf_address_per_port_configurations_autootfcredits },
            { .name = "DEBUG_COUNTERS_TXSRAM0PD", .val = bdmf_address_debug_counters_txsram0pd },
            { .name = "DEBUG_COUNTERS_TXSRAM1PD", .val = bdmf_address_debug_counters_txsram1pd },
            { .name = "DEBUG_COUNTERS_TXDDRPD", .val = bdmf_address_debug_counters_txddrpd },
            { .name = "DEBUG_COUNTERS_PDDROP", .val = bdmf_address_debug_counters_pddrop },
            { .name = "DEBUG_COUNTERS_GETNEXTNULL", .val = bdmf_address_debug_counters_getnextnull },
            { .name = "DEBUG_COUNTERS_GETNEXTDROP", .val = bdmf_address_debug_counters_getnextdrop },
            { .name = "DEBUG_COUNTERS_INPUTFIFODROP", .val = bdmf_address_debug_counters_inputfifodrop },
            { .name = "DEBUG_COUNTERS_SRAM0PKT", .val = bdmf_address_debug_counters_sram0pkt },
            { .name = "DEBUG_COUNTERS_SRAM1PKT", .val = bdmf_address_debug_counters_sram1pkt },
            { .name = "DEBUG_COUNTERS_DDRPKT", .val = bdmf_address_debug_counters_ddrpkt },
            { .name = "DEBUG_COUNTERS_TXSRAM0BYTE", .val = bdmf_address_debug_counters_txsram0byte },
            { .name = "DEBUG_COUNTERS_TXSRAM1BYTE", .val = bdmf_address_debug_counters_txsram1byte },
            { .name = "DEBUG_COUNTERS_TXDDRBYTE", .val = bdmf_address_debug_counters_txddrbyte },
            { .name = "DEBUG_COUNTERS_SWRDEN", .val = bdmf_address_debug_counters_swrden },
            { .name = "DEBUG_COUNTERS_SWRDADDR", .val = bdmf_address_debug_counters_swrdaddr },
            { .name = "DEBUG_COUNTERS_SWRDDATA", .val = bdmf_address_debug_counters_swrddata },
            { .name = "DEBUG_COUNTERS_UNIFIEDPKT", .val = bdmf_address_debug_counters_unifiedpkt },
            { .name = "DEBUG_COUNTERS_UNIFIEDBYTE", .val = bdmf_address_debug_counters_unifiedbyte },
            { .name = "DEBUG_COUNTERS_DBGOUTREG", .val = bdmf_address_debug_counters_dbgoutreg },
            { .name = "DEBUG_COUNTERS_IN_SEGMENTATION", .val = bdmf_address_debug_counters_in_segmentation },
            { .name = "DEBUG_COUNTERS_UNIFIED_DATA_CREDITS", .val = bdmf_address_debug_counters_unified_data_credits },
            { .name = "DEBUG_COUNTERS_UNIFIED_PD_CREDITS", .val = bdmf_address_debug_counters_unified_pd_credits },
            { .name = "DEBUG_COUNTERS_DISFLOWDROPPKT", .val = bdmf_address_debug_counters_disflowdroppkt },
            { .name = "DEBUG_COUNTERS_DISFLOWDROPBYTE", .val = bdmf_address_debug_counters_disflowdropbyte },
            { .name = "DEBUG_COUNTERS_ECNPKT", .val = bdmf_address_debug_counters_ecnpkt },
            { .name = "DEBUG_COUNTERS_ECNBYTES", .val = bdmf_address_debug_counters_ecnbytes },
            { .name = "DEBUG_COUNTERS_ECNMARKERR", .val = bdmf_address_debug_counters_ecnmarkerr },
            { .name = "DEBUG_COUNTERS_ECNLENERR", .val = bdmf_address_debug_counters_ecnlenerr },
            { .name = "DEBUG_COUNTERS_FLUSH_INFO", .val = bdmf_address_debug_counters_flush_info },
            { .name = "DEBUG_COUNTERS_FLUSH_INFO2", .val = bdmf_address_debug_counters_flush_info2 },
            { .name = "DEBUG_COUNTERS_PD_EH_MISMATCH", .val = bdmf_address_debug_counters_pd_eh_mismatch },
            { .name = "DEBUG_COUNTERS_PD_EH_MISMATCH_DATA", .val = bdmf_address_debug_counters_pd_eh_mismatch_data },
            { .name = "DEBUG_COUNTERS_OUTOFRANGE_ERRORS", .val = bdmf_address_debug_counters_outofrange_errors },
            { .name = "DEBUG_COUNTERS_GEMDROPPKT", .val = bdmf_address_debug_counters_gemdroppkt },
            { .name = "DEBUG_COUNTERS_GEMDROPBYTES", .val = bdmf_address_debug_counters_gemdropbytes },
            { .name = "DEBUG_COUNTERS_CMDOTFDDR", .val = bdmf_address_debug_counters_cmdotfddr },
            { .name = "DEBUG_COUNTERS_BYTESOTFDDR", .val = bdmf_address_debug_counters_bytesotfddr },
            { .name = "DEBUG_COUNTERS_CMDOTFEPSRAM", .val = bdmf_address_debug_counters_cmdotfepsram },
            { .name = "DEBUG_COUNTERS_BYTESOTFEPSRAM", .val = bdmf_address_debug_counters_bytesotfepsram },
            { .name = "DEBUG_COUNTERS_GNEXT", .val = bdmf_address_debug_counters_gnext },
            { .name = "DEBUG_COUNTERS_PDARRAY", .val = bdmf_address_debug_counters_pdarray },
            { .name = "DEBUG_COUNTERS_CURPD", .val = bdmf_address_debug_counters_curpd },
            { .name = "DEBUG_COUNTERS_SPACE4REQ", .val = bdmf_address_debug_counters_space4req },
            { .name = "DEBUG_COUNTERS_SEGINFO", .val = bdmf_address_debug_counters_seginfo },
            { .name = "DEBUG_COUNTERS_EHDATVAL", .val = bdmf_address_debug_counters_ehdatval },
            { .name = "DEBUG_COUNTERS_EHDAT", .val = bdmf_address_debug_counters_ehdat },
            { .name = "DEBUG_COUNTERS_RMTPDOCC", .val = bdmf_address_debug_counters_rmtpdocc },
            { .name = "DEBUG_COUNTERS_RMTBYTES", .val = bdmf_address_debug_counters_rmtbytes },
            { .name = "DEBUG_COUNTERS_RMTLBCNT", .val = bdmf_address_debug_counters_rmtlbcnt },
            { .name = "DEBUG_COUNTERS_RESPSPD", .val = bdmf_address_debug_counters_respspd },
            { .name = "DEBUG_COUNTERS_RESPSDATA", .val = bdmf_address_debug_counters_respsdata },
            { .name = "DEBUG_COUNTERS_RNRSMPDCNT", .val = bdmf_address_debug_counters_rnrsmpdcnt },
            { .name = "DEBUG_COUNTERS_RNRSMBYTECNT", .val = bdmf_address_debug_counters_rnrsmbytecnt },
            { .name = "DEBUG_COUNTERS_PORTDBG", .val = bdmf_address_debug_counters_portdbg },
            { .name = "DEBUG_COUNTERS_RESPDBG", .val = bdmf_address_debug_counters_respdbg },
            { .name = "IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_0", .val = bdmf_address_idx_to_gem_translate_table_entry_0 },
            { .name = "IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_1", .val = bdmf_address_idx_to_gem_translate_table_entry_1 },
            { .name = "IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_2", .val = bdmf_address_idx_to_gem_translate_table_entry_2 },
            { .name = "IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_3", .val = bdmf_address_idx_to_gem_translate_table_entry_3 },
            { .name = "IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_4", .val = bdmf_address_idx_to_gem_translate_table_entry_4 },
            { .name = "IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_5", .val = bdmf_address_idx_to_gem_translate_table_entry_5 },
            { .name = "IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_6", .val = bdmf_address_idx_to_gem_translate_table_entry_6 },
            { .name = "IDX_TO_GEM_TRANSLATE_TABLE_ENTRY_7", .val = bdmf_address_idx_to_gem_translate_table_entry_7 },
            { .name = "GEM_TRANS_TBL_GEM_TRNS_VAL", .val = bdmf_address_gem_trans_tbl_gem_trns_val },
            { .name = "MAC_FLOW_EN_TABLE_GEM_EN", .val = bdmf_address_mac_flow_en_table_gem_en },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_crdp_ds_cmn_bbh_tx_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "bbh_id", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
