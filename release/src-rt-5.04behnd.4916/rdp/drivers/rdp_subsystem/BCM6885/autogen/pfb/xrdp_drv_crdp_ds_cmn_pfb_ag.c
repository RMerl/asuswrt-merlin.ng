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
#include "xrdp_drv_crdp_ds_cmn_pfb_ag.h"

#define BLOCK_ADDR_COUNT_BITS 3
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_cfg_ing_set(uint8_t pfb_id, uint8_t idx, const crdp_ds_cmn_pfb_cfg_ing *cfg_ing)
{
    uint32_t reg_cfg_ing = 0;

#ifdef VALIDATE_PARMS
    if(!cfg_ing)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pfb_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 8) ||
       (cfg_ing->fifo_en >= _1BITS_MAX_VAL_) ||
       (cfg_ing->to_val >= _4BITS_MAX_VAL_) ||
       (cfg_ing->my_ch_addr >= _3BITS_MAX_VAL_) ||
       (cfg_ing->egr_ch_addr >= _3BITS_MAX_VAL_) ||
       (cfg_ing->egr_bb_id >= _7BITS_MAX_VAL_) ||
       (cfg_ing->wrr_val >= _4BITS_MAX_VAL_) ||
       (cfg_ing->init_addr_en >= _1BITS_MAX_VAL_) ||
       (cfg_ing->if_width >= _2BITS_MAX_VAL_) ||
       (cfg_ing->eop_en >= _1BITS_MAX_VAL_) ||
       (cfg_ing->iso_input >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_ing = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, FIFO_EN, reg_cfg_ing, cfg_ing->fifo_en);
    reg_cfg_ing = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, TO_VAL, reg_cfg_ing, cfg_ing->to_val);
    reg_cfg_ing = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, MY_CH_ADDR, reg_cfg_ing, cfg_ing->my_ch_addr);
    reg_cfg_ing = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, EGR_CH_ADDR, reg_cfg_ing, cfg_ing->egr_ch_addr);
    reg_cfg_ing = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, EGR_BB_ID, reg_cfg_ing, cfg_ing->egr_bb_id);
    reg_cfg_ing = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, WRR_VAL, reg_cfg_ing, cfg_ing->wrr_val);
    reg_cfg_ing = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, INIT_ADDR_EN, reg_cfg_ing, cfg_ing->init_addr_en);
    reg_cfg_ing = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, IF_WIDTH, reg_cfg_ing, cfg_ing->if_width);
    reg_cfg_ing = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, EOP_EN, reg_cfg_ing, cfg_ing->eop_en);
    reg_cfg_ing = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, ISO_INPUT, reg_cfg_ing, cfg_ing->iso_input);

    RU_REG_RAM_WRITE(pfb_id, idx, CRDP_DS_CMN_PFB, CFG_ING, reg_cfg_ing);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_cfg_ing_get(uint8_t pfb_id, uint8_t idx, crdp_ds_cmn_pfb_cfg_ing *cfg_ing)
{
    uint32_t reg_cfg_ing;

#ifdef VALIDATE_PARMS
    if (!cfg_ing)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pfb_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(pfb_id, idx, CRDP_DS_CMN_PFB, CFG_ING, reg_cfg_ing);

    cfg_ing->fifo_en = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, FIFO_EN, reg_cfg_ing);
    cfg_ing->to_val = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, TO_VAL, reg_cfg_ing);
    cfg_ing->my_ch_addr = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, MY_CH_ADDR, reg_cfg_ing);
    cfg_ing->egr_ch_addr = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, EGR_CH_ADDR, reg_cfg_ing);
    cfg_ing->egr_bb_id = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, EGR_BB_ID, reg_cfg_ing);
    cfg_ing->wrr_val = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, WRR_VAL, reg_cfg_ing);
    cfg_ing->init_addr_en = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, INIT_ADDR_EN, reg_cfg_ing);
    cfg_ing->if_width = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, IF_WIDTH, reg_cfg_ing);
    cfg_ing->eop_en = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, EOP_EN, reg_cfg_ing);
    cfg_ing->iso_input = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING, ISO_INPUT, reg_cfg_ing);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_cfg_egr_set(uint8_t pfb_id, uint8_t idx, const crdp_ds_cmn_pfb_cfg_egr *cfg_egr)
{
    uint32_t reg_cfg_egr = 0;

#ifdef VALIDATE_PARMS
    if(!cfg_egr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pfb_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 8) ||
       (cfg_egr->fifo_en >= _1BITS_MAX_VAL_) ||
       (cfg_egr->cr_rep_gran >= _4BITS_MAX_VAL_) ||
       (cfg_egr->my_ch_addr >= _3BITS_MAX_VAL_) ||
       (cfg_egr->ing_ch_addr >= _3BITS_MAX_VAL_) ||
       (cfg_egr->init_cr >= _4BITS_MAX_VAL_) ||
       (cfg_egr->ing_bb_id >= _7BITS_MAX_VAL_) ||
       (cfg_egr->init_addr_en >= _1BITS_MAX_VAL_) ||
       (cfg_egr->if_width >= _2BITS_MAX_VAL_) ||
       (cfg_egr->flush_en >= _1BITS_MAX_VAL_) ||
       (cfg_egr->iso_input >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_egr = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, FIFO_EN, reg_cfg_egr, cfg_egr->fifo_en);
    reg_cfg_egr = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, CR_REP_GRAN, reg_cfg_egr, cfg_egr->cr_rep_gran);
    reg_cfg_egr = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, MY_CH_ADDR, reg_cfg_egr, cfg_egr->my_ch_addr);
    reg_cfg_egr = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, ING_CH_ADDR, reg_cfg_egr, cfg_egr->ing_ch_addr);
    reg_cfg_egr = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, INIT_CR, reg_cfg_egr, cfg_egr->init_cr);
    reg_cfg_egr = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, ING_BB_ID, reg_cfg_egr, cfg_egr->ing_bb_id);
    reg_cfg_egr = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, INIT_ADDR_EN, reg_cfg_egr, cfg_egr->init_addr_en);
    reg_cfg_egr = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, IF_WIDTH, reg_cfg_egr, cfg_egr->if_width);
    reg_cfg_egr = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, FLUSH_EN, reg_cfg_egr, cfg_egr->flush_en);
    reg_cfg_egr = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, ISO_INPUT, reg_cfg_egr, cfg_egr->iso_input);

    RU_REG_RAM_WRITE(pfb_id, idx, CRDP_DS_CMN_PFB, CFG_EGR, reg_cfg_egr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_cfg_egr_get(uint8_t pfb_id, uint8_t idx, crdp_ds_cmn_pfb_cfg_egr *cfg_egr)
{
    uint32_t reg_cfg_egr;

#ifdef VALIDATE_PARMS
    if (!cfg_egr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pfb_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(pfb_id, idx, CRDP_DS_CMN_PFB, CFG_EGR, reg_cfg_egr);

    cfg_egr->fifo_en = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, FIFO_EN, reg_cfg_egr);
    cfg_egr->cr_rep_gran = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, CR_REP_GRAN, reg_cfg_egr);
    cfg_egr->my_ch_addr = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, MY_CH_ADDR, reg_cfg_egr);
    cfg_egr->ing_ch_addr = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, ING_CH_ADDR, reg_cfg_egr);
    cfg_egr->init_cr = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, INIT_CR, reg_cfg_egr);
    cfg_egr->ing_bb_id = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, ING_BB_ID, reg_cfg_egr);
    cfg_egr->init_addr_en = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, INIT_ADDR_EN, reg_cfg_egr);
    cfg_egr->if_width = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, IF_WIDTH, reg_cfg_egr);
    cfg_egr->flush_en = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, FLUSH_EN, reg_cfg_egr);
    cfg_egr->iso_input = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR, ISO_INPUT, reg_cfg_egr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_cfg_gbl_set(uint8_t pfb_id, const crdp_ds_cmn_pfb_cfg_gbl *cfg_gbl)
{
    uint32_t reg_cfg_gbl = 0;

#ifdef VALIDATE_PARMS
    if(!cfg_gbl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pfb_id >= BLOCK_ADDR_COUNT) ||
       (cfg_gbl->arb_scheme >= _1BITS_MAX_VAL_) ||
       (cfg_gbl->route_addr_overwr_en >= _1BITS_MAX_VAL_) ||
       (cfg_gbl->src_bb_id >= _7BITS_MAX_VAL_) ||
       (cfg_gbl->src_bb_id_ovr_en >= _1BITS_MAX_VAL_) ||
       (cfg_gbl->flush_pace_timeout >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_gbl = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_GBL, ARB_SCHEME, reg_cfg_gbl, cfg_gbl->arb_scheme);
    reg_cfg_gbl = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_GBL, ROUTE_ADDR_OVERWR_EN, reg_cfg_gbl, cfg_gbl->route_addr_overwr_en);
    reg_cfg_gbl = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_GBL, SRC_BB_ID, reg_cfg_gbl, cfg_gbl->src_bb_id);
    reg_cfg_gbl = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_GBL, SRC_BB_ID_OVR_EN, reg_cfg_gbl, cfg_gbl->src_bb_id_ovr_en);
    reg_cfg_gbl = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_GBL, FLUSH_PACE_TIMEOUT, reg_cfg_gbl, cfg_gbl->flush_pace_timeout);
    reg_cfg_gbl = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_GBL, ROUTE_ADDR_OVERWR_VAL, reg_cfg_gbl, cfg_gbl->route_addr_overwr_val);

    RU_REG_WRITE(pfb_id, CRDP_DS_CMN_PFB, CFG_GBL, reg_cfg_gbl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_cfg_gbl_get(uint8_t pfb_id, crdp_ds_cmn_pfb_cfg_gbl *cfg_gbl)
{
    uint32_t reg_cfg_gbl;

#ifdef VALIDATE_PARMS
    if (!cfg_gbl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pfb_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(pfb_id, CRDP_DS_CMN_PFB, CFG_GBL, reg_cfg_gbl);

    cfg_gbl->arb_scheme = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_GBL, ARB_SCHEME, reg_cfg_gbl);
    cfg_gbl->route_addr_overwr_en = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_GBL, ROUTE_ADDR_OVERWR_EN, reg_cfg_gbl);
    cfg_gbl->src_bb_id = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_GBL, SRC_BB_ID, reg_cfg_gbl);
    cfg_gbl->src_bb_id_ovr_en = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_GBL, SRC_BB_ID_OVR_EN, reg_cfg_gbl);
    cfg_gbl->flush_pace_timeout = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_GBL, FLUSH_PACE_TIMEOUT, reg_cfg_gbl);
    cfg_gbl->route_addr_overwr_val = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_GBL, ROUTE_ADDR_OVERWR_VAL, reg_cfg_gbl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_cfg_ing_pwire_set(uint8_t pfb_id, uint8_t idx, uint8_t mask, uint8_t to_val, uint8_t ing_spare_cfg, uint16_t pwire_val)
{
    uint32_t reg_cfg_ing_pwire = 0;

#ifdef VALIDATE_PARMS
    if ((pfb_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 8) ||
       (mask >= _4BITS_MAX_VAL_) ||
       (to_val >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_ing_pwire = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING_PWIRE, MASK, reg_cfg_ing_pwire, mask);
    reg_cfg_ing_pwire = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING_PWIRE, TO_VAL, reg_cfg_ing_pwire, to_val);
    reg_cfg_ing_pwire = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING_PWIRE, ING_SPARE_CFG, reg_cfg_ing_pwire, ing_spare_cfg);
    reg_cfg_ing_pwire = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING_PWIRE, PWIRE_VAL, reg_cfg_ing_pwire, pwire_val);

    RU_REG_RAM_WRITE(pfb_id, idx, CRDP_DS_CMN_PFB, CFG_ING_PWIRE, reg_cfg_ing_pwire);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_cfg_ing_pwire_get(uint8_t pfb_id, uint8_t idx, uint8_t *mask, uint8_t *to_val, uint8_t *ing_spare_cfg, uint16_t *pwire_val)
{
    uint32_t reg_cfg_ing_pwire;

#ifdef VALIDATE_PARMS
    if (!mask || !to_val || !ing_spare_cfg || !pwire_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pfb_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(pfb_id, idx, CRDP_DS_CMN_PFB, CFG_ING_PWIRE, reg_cfg_ing_pwire);

    *mask = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING_PWIRE, MASK, reg_cfg_ing_pwire);
    *to_val = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING_PWIRE, TO_VAL, reg_cfg_ing_pwire);
    *ing_spare_cfg = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING_PWIRE, ING_SPARE_CFG, reg_cfg_ing_pwire);
    *pwire_val = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING_PWIRE, PWIRE_VAL, reg_cfg_ing_pwire);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_cfg_egr_pwire_set(uint8_t pfb_id, uint8_t idx, uint8_t mask, uint8_t to_val, uint8_t egr_spare_cfg, uint16_t pwire_val)
{
    uint32_t reg_cfg_egr_pwire = 0;

#ifdef VALIDATE_PARMS
    if ((pfb_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 8) ||
       (mask >= _4BITS_MAX_VAL_) ||
       (to_val >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_egr_pwire = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR_PWIRE, MASK, reg_cfg_egr_pwire, mask);
    reg_cfg_egr_pwire = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR_PWIRE, TO_VAL, reg_cfg_egr_pwire, to_val);
    reg_cfg_egr_pwire = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR_PWIRE, EGR_SPARE_CFG, reg_cfg_egr_pwire, egr_spare_cfg);
    reg_cfg_egr_pwire = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR_PWIRE, PWIRE_VAL, reg_cfg_egr_pwire, pwire_val);

    RU_REG_RAM_WRITE(pfb_id, idx, CRDP_DS_CMN_PFB, CFG_EGR_PWIRE, reg_cfg_egr_pwire);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_cfg_egr_pwire_get(uint8_t pfb_id, uint8_t idx, uint8_t *mask, uint8_t *to_val, uint8_t *egr_spare_cfg, uint16_t *pwire_val)
{
    uint32_t reg_cfg_egr_pwire;

#ifdef VALIDATE_PARMS
    if (!mask || !to_val || !egr_spare_cfg || !pwire_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pfb_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(pfb_id, idx, CRDP_DS_CMN_PFB, CFG_EGR_PWIRE, reg_cfg_egr_pwire);

    *mask = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR_PWIRE, MASK, reg_cfg_egr_pwire);
    *to_val = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR_PWIRE, TO_VAL, reg_cfg_egr_pwire);
    *egr_spare_cfg = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR_PWIRE, EGR_SPARE_CFG, reg_cfg_egr_pwire);
    *pwire_val = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_EGR_PWIRE, PWIRE_VAL, reg_cfg_egr_pwire);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_cfg_ing_eop_mask_set(uint8_t pfb_id, uint8_t idx, uint16_t mask, uint16_t eop_msb_mask)
{
    uint32_t reg_cfg_ing_eop_mask = 0;

#ifdef VALIDATE_PARMS
    if ((pfb_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_ing_eop_mask = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING_EOP_MASK, MASK, reg_cfg_ing_eop_mask, mask);
    reg_cfg_ing_eop_mask = RU_FIELD_SET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING_EOP_MASK, EOP_MSB_MASK, reg_cfg_ing_eop_mask, eop_msb_mask);

    RU_REG_RAM_WRITE(pfb_id, idx, CRDP_DS_CMN_PFB, CFG_ING_EOP_MASK, reg_cfg_ing_eop_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_cfg_ing_eop_mask_get(uint8_t pfb_id, uint8_t idx, uint16_t *mask, uint16_t *eop_msb_mask)
{
    uint32_t reg_cfg_ing_eop_mask;

#ifdef VALIDATE_PARMS
    if (!mask || !eop_msb_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pfb_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(pfb_id, idx, CRDP_DS_CMN_PFB, CFG_ING_EOP_MASK, reg_cfg_ing_eop_mask);

    *mask = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING_EOP_MASK, MASK, reg_cfg_ing_eop_mask);
    *eop_msb_mask = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, CFG_ING_EOP_MASK, EOP_MSB_MASK, reg_cfg_ing_eop_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_dbg_fifo_status_get(uint8_t pfb_id, uint8_t *ing_full, uint8_t *ing_empty, uint8_t *egr_full, uint8_t *egr_empty)
{
    uint32_t reg_dbg_fifo_status;

#ifdef VALIDATE_PARMS
    if (!ing_full || !ing_empty || !egr_full || !egr_empty)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pfb_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(pfb_id, CRDP_DS_CMN_PFB, DBG_FIFO_STATUS, reg_dbg_fifo_status);

    *ing_full = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, DBG_FIFO_STATUS, ING_FULL, reg_dbg_fifo_status);
    *ing_empty = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, DBG_FIFO_STATUS, ING_EMPTY, reg_dbg_fifo_status);
    *egr_full = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, DBG_FIFO_STATUS, EGR_FULL, reg_dbg_fifo_status);
    *egr_empty = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, DBG_FIFO_STATUS, EGR_EMPTY, reg_dbg_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_dbg_ingress_used_words_get(uint8_t pfb_id, uint8_t idx, uint32_t *used_words)
{
    uint32_t reg_dbg_ingress_used_words;

#ifdef VALIDATE_PARMS
    if (!used_words)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pfb_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(pfb_id, idx, CRDP_DS_CMN_PFB, DBG_INGRESS_USED_WORDS, reg_dbg_ingress_used_words);

    *used_words = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, DBG_INGRESS_USED_WORDS, USED_WORDS, reg_dbg_ingress_used_words);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_dbg_egress_used_words_get(uint8_t pfb_id, uint8_t idx, uint32_t *used_words)
{
    uint32_t reg_dbg_egress_used_words;

#ifdef VALIDATE_PARMS
    if (!used_words)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pfb_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(pfb_id, idx, CRDP_DS_CMN_PFB, DBG_EGRESS_USED_WORDS, reg_dbg_egress_used_words);

    *used_words = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, DBG_EGRESS_USED_WORDS, USED_WORDS, reg_dbg_egress_used_words);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_dbg_ingress_bb_msg_trans_get(uint8_t pfb_id, uint8_t idx, uint32_t *bb_msg_trans)
{
    uint32_t reg_dbg_ingress_bb_msg_trans;

#ifdef VALIDATE_PARMS
    if (!bb_msg_trans)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pfb_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(pfb_id, idx, CRDP_DS_CMN_PFB, DBG_INGRESS_BB_MSG_TRANS, reg_dbg_ingress_bb_msg_trans);

    *bb_msg_trans = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, DBG_INGRESS_BB_MSG_TRANS, BB_MSG_TRANS, reg_dbg_ingress_bb_msg_trans);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_dbg_egress_bb_msg_trans_get(uint8_t pfb_id, uint8_t idx, uint32_t *bb_msg_trans)
{
    uint32_t reg_dbg_egress_bb_msg_trans;

#ifdef VALIDATE_PARMS
    if (!bb_msg_trans)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pfb_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(pfb_id, idx, CRDP_DS_CMN_PFB, DBG_EGRESS_BB_MSG_TRANS, reg_dbg_egress_bb_msg_trans);

    *bb_msg_trans = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, DBG_EGRESS_BB_MSG_TRANS, BB_MSG_TRANS, reg_dbg_egress_bb_msg_trans);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_crdp_ds_cmn_pfb_dbg_ingress_credit_cnt_get(uint8_t pfb_id, uint8_t idx, uint32_t *ingress_credit_cnt)
{
    uint32_t reg_dbg_ingress_credit_cnt;

#ifdef VALIDATE_PARMS
    if (!ingress_credit_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((pfb_id >= BLOCK_ADDR_COUNT) ||
       (idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(pfb_id, idx, CRDP_DS_CMN_PFB, DBG_INGRESS_CREDIT_CNT, reg_dbg_ingress_credit_cnt);

    *ingress_credit_cnt = RU_FIELD_GET(pfb_id, CRDP_DS_CMN_PFB, DBG_INGRESS_CREDIT_CNT, INGRESS_CREDIT_CNT, reg_dbg_ingress_credit_cnt);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_cfg_ing,
    bdmf_address_cfg_egr,
    bdmf_address_cfg_gbl,
    bdmf_address_cfg_ing_pwire,
    bdmf_address_cfg_egr_pwire,
    bdmf_address_cfg_ing_eop_mask,
    bdmf_address_dbg_fifo_status,
    bdmf_address_dbg_ingress_used_words,
    bdmf_address_dbg_egress_used_words,
    bdmf_address_dbg_ingress_bb_msg_trans,
    bdmf_address_dbg_egress_bb_msg_trans,
    bdmf_address_dbg_ingress_credit_cnt,
}
bdmf_address;

static int ag_drv_crdp_ds_cmn_pfb_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_crdp_ds_cmn_pfb_cfg_ing:
    {
        crdp_ds_cmn_pfb_cfg_ing cfg_ing = { .fifo_en = parm[3].value.unumber, .to_val = parm[4].value.unumber, .my_ch_addr = parm[5].value.unumber, .egr_ch_addr = parm[6].value.unumber, .egr_bb_id = parm[7].value.unumber, .wrr_val = parm[8].value.unumber, .init_addr_en = parm[9].value.unumber, .if_width = parm[10].value.unumber, .eop_en = parm[11].value.unumber, .iso_input = parm[12].value.unumber};
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_set(parm[1].value.unumber, parm[2].value.unumber, &cfg_ing);
        break;
    }
    case cli_crdp_ds_cmn_pfb_cfg_egr:
    {
        crdp_ds_cmn_pfb_cfg_egr cfg_egr = { .fifo_en = parm[3].value.unumber, .cr_rep_gran = parm[4].value.unumber, .my_ch_addr = parm[5].value.unumber, .ing_ch_addr = parm[6].value.unumber, .init_cr = parm[7].value.unumber, .ing_bb_id = parm[8].value.unumber, .init_addr_en = parm[9].value.unumber, .if_width = parm[10].value.unumber, .flush_en = parm[11].value.unumber, .iso_input = parm[12].value.unumber};
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_egr_set(parm[1].value.unumber, parm[2].value.unumber, &cfg_egr);
        break;
    }
    case cli_crdp_ds_cmn_pfb_cfg_gbl:
    {
        crdp_ds_cmn_pfb_cfg_gbl cfg_gbl = { .arb_scheme = parm[2].value.unumber, .route_addr_overwr_en = parm[3].value.unumber, .src_bb_id = parm[4].value.unumber, .src_bb_id_ovr_en = parm[5].value.unumber, .flush_pace_timeout = parm[6].value.unumber, .route_addr_overwr_val = parm[7].value.unumber};
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_gbl_set(parm[1].value.unumber, &cfg_gbl);
        break;
    }
    case cli_crdp_ds_cmn_pfb_cfg_ing_pwire:
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_pwire_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber);
        break;
    case cli_crdp_ds_cmn_pfb_cfg_egr_pwire:
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_egr_pwire_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber);
        break;
    case cli_crdp_ds_cmn_pfb_cfg_ing_eop_mask:
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_eop_mask_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_crdp_ds_cmn_pfb_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_crdp_ds_cmn_pfb_cfg_ing:
    {
        crdp_ds_cmn_pfb_cfg_ing cfg_ing;
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_get(parm[1].value.unumber, parm[2].value.unumber, &cfg_ing);
        bdmf_session_print(session, "fifo_en = %u = 0x%x\n", cfg_ing.fifo_en, cfg_ing.fifo_en);
        bdmf_session_print(session, "to_val = %u = 0x%x\n", cfg_ing.to_val, cfg_ing.to_val);
        bdmf_session_print(session, "my_ch_addr = %u = 0x%x\n", cfg_ing.my_ch_addr, cfg_ing.my_ch_addr);
        bdmf_session_print(session, "egr_ch_addr = %u = 0x%x\n", cfg_ing.egr_ch_addr, cfg_ing.egr_ch_addr);
        bdmf_session_print(session, "egr_bb_id = %u = 0x%x\n", cfg_ing.egr_bb_id, cfg_ing.egr_bb_id);
        bdmf_session_print(session, "wrr_val = %u = 0x%x\n", cfg_ing.wrr_val, cfg_ing.wrr_val);
        bdmf_session_print(session, "init_addr_en = %u = 0x%x\n", cfg_ing.init_addr_en, cfg_ing.init_addr_en);
        bdmf_session_print(session, "if_width = %u = 0x%x\n", cfg_ing.if_width, cfg_ing.if_width);
        bdmf_session_print(session, "eop_en = %u = 0x%x\n", cfg_ing.eop_en, cfg_ing.eop_en);
        bdmf_session_print(session, "iso_input = %u = 0x%x\n", cfg_ing.iso_input, cfg_ing.iso_input);
        break;
    }
    case cli_crdp_ds_cmn_pfb_cfg_egr:
    {
        crdp_ds_cmn_pfb_cfg_egr cfg_egr;
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_egr_get(parm[1].value.unumber, parm[2].value.unumber, &cfg_egr);
        bdmf_session_print(session, "fifo_en = %u = 0x%x\n", cfg_egr.fifo_en, cfg_egr.fifo_en);
        bdmf_session_print(session, "cr_rep_gran = %u = 0x%x\n", cfg_egr.cr_rep_gran, cfg_egr.cr_rep_gran);
        bdmf_session_print(session, "my_ch_addr = %u = 0x%x\n", cfg_egr.my_ch_addr, cfg_egr.my_ch_addr);
        bdmf_session_print(session, "ing_ch_addr = %u = 0x%x\n", cfg_egr.ing_ch_addr, cfg_egr.ing_ch_addr);
        bdmf_session_print(session, "init_cr = %u = 0x%x\n", cfg_egr.init_cr, cfg_egr.init_cr);
        bdmf_session_print(session, "ing_bb_id = %u = 0x%x\n", cfg_egr.ing_bb_id, cfg_egr.ing_bb_id);
        bdmf_session_print(session, "init_addr_en = %u = 0x%x\n", cfg_egr.init_addr_en, cfg_egr.init_addr_en);
        bdmf_session_print(session, "if_width = %u = 0x%x\n", cfg_egr.if_width, cfg_egr.if_width);
        bdmf_session_print(session, "flush_en = %u = 0x%x\n", cfg_egr.flush_en, cfg_egr.flush_en);
        bdmf_session_print(session, "iso_input = %u = 0x%x\n", cfg_egr.iso_input, cfg_egr.iso_input);
        break;
    }
    case cli_crdp_ds_cmn_pfb_cfg_gbl:
    {
        crdp_ds_cmn_pfb_cfg_gbl cfg_gbl;
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_gbl_get(parm[1].value.unumber, &cfg_gbl);
        bdmf_session_print(session, "arb_scheme = %u = 0x%x\n", cfg_gbl.arb_scheme, cfg_gbl.arb_scheme);
        bdmf_session_print(session, "route_addr_overwr_en = %u = 0x%x\n", cfg_gbl.route_addr_overwr_en, cfg_gbl.route_addr_overwr_en);
        bdmf_session_print(session, "src_bb_id = %u = 0x%x\n", cfg_gbl.src_bb_id, cfg_gbl.src_bb_id);
        bdmf_session_print(session, "src_bb_id_ovr_en = %u = 0x%x\n", cfg_gbl.src_bb_id_ovr_en, cfg_gbl.src_bb_id_ovr_en);
        bdmf_session_print(session, "flush_pace_timeout = %u = 0x%x\n", cfg_gbl.flush_pace_timeout, cfg_gbl.flush_pace_timeout);
        bdmf_session_print(session, "route_addr_overwr_val = %u = 0x%x\n", cfg_gbl.route_addr_overwr_val, cfg_gbl.route_addr_overwr_val);
        break;
    }
    case cli_crdp_ds_cmn_pfb_cfg_ing_pwire:
    {
        uint8_t mask;
        uint8_t to_val;
        uint8_t ing_spare_cfg;
        uint16_t pwire_val;
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_pwire_get(parm[1].value.unumber, parm[2].value.unumber, &mask, &to_val, &ing_spare_cfg, &pwire_val);
        bdmf_session_print(session, "mask = %u = 0x%x\n", mask, mask);
        bdmf_session_print(session, "to_val = %u = 0x%x\n", to_val, to_val);
        bdmf_session_print(session, "ing_spare_cfg = %u = 0x%x\n", ing_spare_cfg, ing_spare_cfg);
        bdmf_session_print(session, "pwire_val = %u = 0x%x\n", pwire_val, pwire_val);
        break;
    }
    case cli_crdp_ds_cmn_pfb_cfg_egr_pwire:
    {
        uint8_t mask;
        uint8_t to_val;
        uint8_t egr_spare_cfg;
        uint16_t pwire_val;
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_egr_pwire_get(parm[1].value.unumber, parm[2].value.unumber, &mask, &to_val, &egr_spare_cfg, &pwire_val);
        bdmf_session_print(session, "mask = %u = 0x%x\n", mask, mask);
        bdmf_session_print(session, "to_val = %u = 0x%x\n", to_val, to_val);
        bdmf_session_print(session, "egr_spare_cfg = %u = 0x%x\n", egr_spare_cfg, egr_spare_cfg);
        bdmf_session_print(session, "pwire_val = %u = 0x%x\n", pwire_val, pwire_val);
        break;
    }
    case cli_crdp_ds_cmn_pfb_cfg_ing_eop_mask:
    {
        uint16_t mask;
        uint16_t eop_msb_mask;
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_eop_mask_get(parm[1].value.unumber, parm[2].value.unumber, &mask, &eop_msb_mask);
        bdmf_session_print(session, "mask = %u = 0x%x\n", mask, mask);
        bdmf_session_print(session, "eop_msb_mask = %u = 0x%x\n", eop_msb_mask, eop_msb_mask);
        break;
    }
    case cli_crdp_ds_cmn_pfb_dbg_fifo_status:
    {
        uint8_t ing_full;
        uint8_t ing_empty;
        uint8_t egr_full;
        uint8_t egr_empty;
        ag_err = ag_drv_crdp_ds_cmn_pfb_dbg_fifo_status_get(parm[1].value.unumber, &ing_full, &ing_empty, &egr_full, &egr_empty);
        bdmf_session_print(session, "ing_full = %u = 0x%x\n", ing_full, ing_full);
        bdmf_session_print(session, "ing_empty = %u = 0x%x\n", ing_empty, ing_empty);
        bdmf_session_print(session, "egr_full = %u = 0x%x\n", egr_full, egr_full);
        bdmf_session_print(session, "egr_empty = %u = 0x%x\n", egr_empty, egr_empty);
        break;
    }
    case cli_crdp_ds_cmn_pfb_dbg_ingress_used_words:
    {
        uint32_t used_words;
        ag_err = ag_drv_crdp_ds_cmn_pfb_dbg_ingress_used_words_get(parm[1].value.unumber, parm[2].value.unumber, &used_words);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        break;
    }
    case cli_crdp_ds_cmn_pfb_dbg_egress_used_words:
    {
        uint32_t used_words;
        ag_err = ag_drv_crdp_ds_cmn_pfb_dbg_egress_used_words_get(parm[1].value.unumber, parm[2].value.unumber, &used_words);
        bdmf_session_print(session, "used_words = %u = 0x%x\n", used_words, used_words);
        break;
    }
    case cli_crdp_ds_cmn_pfb_dbg_ingress_bb_msg_trans:
    {
        uint32_t bb_msg_trans;
        ag_err = ag_drv_crdp_ds_cmn_pfb_dbg_ingress_bb_msg_trans_get(parm[1].value.unumber, parm[2].value.unumber, &bb_msg_trans);
        bdmf_session_print(session, "bb_msg_trans = %u = 0x%x\n", bb_msg_trans, bb_msg_trans);
        break;
    }
    case cli_crdp_ds_cmn_pfb_dbg_egress_bb_msg_trans:
    {
        uint32_t bb_msg_trans;
        ag_err = ag_drv_crdp_ds_cmn_pfb_dbg_egress_bb_msg_trans_get(parm[1].value.unumber, parm[2].value.unumber, &bb_msg_trans);
        bdmf_session_print(session, "bb_msg_trans = %u = 0x%x\n", bb_msg_trans, bb_msg_trans);
        break;
    }
    case cli_crdp_ds_cmn_pfb_dbg_ingress_credit_cnt:
    {
        uint32_t ingress_credit_cnt;
        ag_err = ag_drv_crdp_ds_cmn_pfb_dbg_ingress_credit_cnt_get(parm[1].value.unumber, parm[2].value.unumber, &ingress_credit_cnt);
        bdmf_session_print(session, "ingress_credit_cnt = %u = 0x%x\n", ingress_credit_cnt, ingress_credit_cnt);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_crdp_ds_cmn_pfb_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t pfb_id = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint8_t idx = gtmv(m, 3);
        crdp_ds_cmn_pfb_cfg_ing cfg_ing = {.fifo_en = gtmv(m, 1), .to_val = gtmv(m, 4), .my_ch_addr = gtmv(m, 3), .egr_ch_addr = gtmv(m, 3), .egr_bb_id = gtmv(m, 7), .wrr_val = gtmv(m, 4), .init_addr_en = gtmv(m, 1), .if_width = gtmv(m, 2), .eop_en = gtmv(m, 1), .iso_input = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_ing_set(%u %u %u %u %u %u %u %u %u %u %u %u)\n", pfb_id, idx,
            cfg_ing.fifo_en, cfg_ing.to_val, cfg_ing.my_ch_addr, cfg_ing.egr_ch_addr, 
            cfg_ing.egr_bb_id, cfg_ing.wrr_val, cfg_ing.init_addr_en, cfg_ing.if_width, 
            cfg_ing.eop_en, cfg_ing.iso_input);
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_set(pfb_id, idx, &cfg_ing);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_get(pfb_id, idx, &cfg_ing);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_ing_get(%u %u %u %u %u %u %u %u %u %u %u %u)\n", pfb_id, idx,
                cfg_ing.fifo_en, cfg_ing.to_val, cfg_ing.my_ch_addr, cfg_ing.egr_ch_addr, 
                cfg_ing.egr_bb_id, cfg_ing.wrr_val, cfg_ing.init_addr_en, cfg_ing.if_width, 
                cfg_ing.eop_en, cfg_ing.iso_input);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cfg_ing.fifo_en != gtmv(m, 1) || cfg_ing.to_val != gtmv(m, 4) || cfg_ing.my_ch_addr != gtmv(m, 3) || cfg_ing.egr_ch_addr != gtmv(m, 3) || cfg_ing.egr_bb_id != gtmv(m, 7) || cfg_ing.wrr_val != gtmv(m, 4) || cfg_ing.init_addr_en != gtmv(m, 1) || cfg_ing.if_width != gtmv(m, 2) || cfg_ing.eop_en != gtmv(m, 1) || cfg_ing.iso_input != gtmv(m, 1))
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
        crdp_ds_cmn_pfb_cfg_egr cfg_egr = {.fifo_en = gtmv(m, 1), .cr_rep_gran = gtmv(m, 4), .my_ch_addr = gtmv(m, 3), .ing_ch_addr = gtmv(m, 3), .init_cr = gtmv(m, 4), .ing_bb_id = gtmv(m, 7), .init_addr_en = gtmv(m, 1), .if_width = gtmv(m, 2), .flush_en = gtmv(m, 1), .iso_input = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_egr_set(%u %u %u %u %u %u %u %u %u %u %u %u)\n", pfb_id, idx,
            cfg_egr.fifo_en, cfg_egr.cr_rep_gran, cfg_egr.my_ch_addr, cfg_egr.ing_ch_addr, 
            cfg_egr.init_cr, cfg_egr.ing_bb_id, cfg_egr.init_addr_en, cfg_egr.if_width, 
            cfg_egr.flush_en, cfg_egr.iso_input);
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_egr_set(pfb_id, idx, &cfg_egr);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_egr_get(pfb_id, idx, &cfg_egr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_egr_get(%u %u %u %u %u %u %u %u %u %u %u %u)\n", pfb_id, idx,
                cfg_egr.fifo_en, cfg_egr.cr_rep_gran, cfg_egr.my_ch_addr, cfg_egr.ing_ch_addr, 
                cfg_egr.init_cr, cfg_egr.ing_bb_id, cfg_egr.init_addr_en, cfg_egr.if_width, 
                cfg_egr.flush_en, cfg_egr.iso_input);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cfg_egr.fifo_en != gtmv(m, 1) || cfg_egr.cr_rep_gran != gtmv(m, 4) || cfg_egr.my_ch_addr != gtmv(m, 3) || cfg_egr.ing_ch_addr != gtmv(m, 3) || cfg_egr.init_cr != gtmv(m, 4) || cfg_egr.ing_bb_id != gtmv(m, 7) || cfg_egr.init_addr_en != gtmv(m, 1) || cfg_egr.if_width != gtmv(m, 2) || cfg_egr.flush_en != gtmv(m, 1) || cfg_egr.iso_input != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        crdp_ds_cmn_pfb_cfg_gbl cfg_gbl = {.arb_scheme = gtmv(m, 1), .route_addr_overwr_en = gtmv(m, 1), .src_bb_id = gtmv(m, 7), .src_bb_id_ovr_en = gtmv(m, 1), .flush_pace_timeout = gtmv(m, 4), .route_addr_overwr_val = gtmv(m, 16)};
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_gbl_set(%u %u %u %u %u %u %u)\n", pfb_id,
            cfg_gbl.arb_scheme, cfg_gbl.route_addr_overwr_en, cfg_gbl.src_bb_id, cfg_gbl.src_bb_id_ovr_en, 
            cfg_gbl.flush_pace_timeout, cfg_gbl.route_addr_overwr_val);
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_gbl_set(pfb_id, &cfg_gbl);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_gbl_get(pfb_id, &cfg_gbl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_gbl_get(%u %u %u %u %u %u %u)\n", pfb_id,
                cfg_gbl.arb_scheme, cfg_gbl.route_addr_overwr_en, cfg_gbl.src_bb_id, cfg_gbl.src_bb_id_ovr_en, 
                cfg_gbl.flush_pace_timeout, cfg_gbl.route_addr_overwr_val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cfg_gbl.arb_scheme != gtmv(m, 1) || cfg_gbl.route_addr_overwr_en != gtmv(m, 1) || cfg_gbl.src_bb_id != gtmv(m, 7) || cfg_gbl.src_bb_id_ovr_en != gtmv(m, 1) || cfg_gbl.flush_pace_timeout != gtmv(m, 4) || cfg_gbl.route_addr_overwr_val != gtmv(m, 16))
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
        uint8_t mask = gtmv(m, 4);
        uint8_t to_val = gtmv(m, 4);
        uint8_t ing_spare_cfg = gtmv(m, 8);
        uint16_t pwire_val = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_ing_pwire_set(%u %u %u %u %u %u)\n", pfb_id, idx,
            mask, to_val, ing_spare_cfg, pwire_val);
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_pwire_set(pfb_id, idx, mask, to_val, ing_spare_cfg, pwire_val);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_pwire_get(pfb_id, idx, &mask, &to_val, &ing_spare_cfg, &pwire_val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_ing_pwire_get(%u %u %u %u %u %u)\n", pfb_id, idx,
                mask, to_val, ing_spare_cfg, pwire_val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mask != gtmv(m, 4) || to_val != gtmv(m, 4) || ing_spare_cfg != gtmv(m, 8) || pwire_val != gtmv(m, 16))
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
        uint8_t mask = gtmv(m, 4);
        uint8_t to_val = gtmv(m, 4);
        uint8_t egr_spare_cfg = gtmv(m, 8);
        uint16_t pwire_val = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_egr_pwire_set(%u %u %u %u %u %u)\n", pfb_id, idx,
            mask, to_val, egr_spare_cfg, pwire_val);
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_egr_pwire_set(pfb_id, idx, mask, to_val, egr_spare_cfg, pwire_val);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_egr_pwire_get(pfb_id, idx, &mask, &to_val, &egr_spare_cfg, &pwire_val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_egr_pwire_get(%u %u %u %u %u %u)\n", pfb_id, idx,
                mask, to_val, egr_spare_cfg, pwire_val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mask != gtmv(m, 4) || to_val != gtmv(m, 4) || egr_spare_cfg != gtmv(m, 8) || pwire_val != gtmv(m, 16))
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
        uint16_t mask = gtmv(m, 16);
        uint16_t eop_msb_mask = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_ing_eop_mask_set(%u %u %u %u)\n", pfb_id, idx,
            mask, eop_msb_mask);
        ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_eop_mask_set(pfb_id, idx, mask, eop_msb_mask);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_eop_mask_get(pfb_id, idx, &mask, &eop_msb_mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_ing_eop_mask_get(%u %u %u %u)\n", pfb_id, idx,
                mask, eop_msb_mask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mask != gtmv(m, 16) || eop_msb_mask != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t ing_full = gtmv(m, 8);
        uint8_t ing_empty = gtmv(m, 8);
        uint8_t egr_full = gtmv(m, 8);
        uint8_t egr_empty = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_pfb_dbg_fifo_status_get(pfb_id, &ing_full, &ing_empty, &egr_full, &egr_empty);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_dbg_fifo_status_get(%u %u %u %u %u)\n", pfb_id,
                ing_full, ing_empty, egr_full, egr_empty);
        }
    }

    {
        uint8_t idx = gtmv(m, 3);
        uint32_t used_words = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_pfb_dbg_ingress_used_words_get(pfb_id, idx, &used_words);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_dbg_ingress_used_words_get(%u %u %u)\n", pfb_id, idx,
                used_words);
        }
    }

    {
        uint8_t idx = gtmv(m, 3);
        uint32_t used_words = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_pfb_dbg_egress_used_words_get(pfb_id, idx, &used_words);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_dbg_egress_used_words_get(%u %u %u)\n", pfb_id, idx,
                used_words);
        }
    }

    {
        uint8_t idx = gtmv(m, 3);
        uint32_t bb_msg_trans = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_pfb_dbg_ingress_bb_msg_trans_get(pfb_id, idx, &bb_msg_trans);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_dbg_ingress_bb_msg_trans_get(%u %u %u)\n", pfb_id, idx,
                bb_msg_trans);
        }
    }

    {
        uint8_t idx = gtmv(m, 3);
        uint32_t bb_msg_trans = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_pfb_dbg_egress_bb_msg_trans_get(pfb_id, idx, &bb_msg_trans);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_dbg_egress_bb_msg_trans_get(%u %u %u)\n", pfb_id, idx,
                bb_msg_trans);
        }
    }

    {
        uint8_t idx = gtmv(m, 3);
        uint32_t ingress_credit_cnt = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_crdp_ds_cmn_pfb_dbg_ingress_credit_cnt_get(pfb_id, idx, &ingress_credit_cnt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_dbg_ingress_credit_cnt_get(%u %u %u)\n", pfb_id, idx,
                ingress_credit_cnt);
        }
    }

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_crdp_ds_cmn_pfb_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m, input_method = parm[0].value.unumber;
    uint8_t pfb_id = parm[2].value.unumber;
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
    case cli_crdp_ds_cmn_pfb_cfg_ing:
    {
        uint8_t max_idx = 8;
        uint8_t idx = gtmv(m, 3);
        crdp_ds_cmn_pfb_cfg_ing cfg_ing = {
            .fifo_en = gtmv(m, 1), 
            .to_val = gtmv(m, 4), 
            .my_ch_addr = gtmv(m, 3), 
            .egr_ch_addr = gtmv(m, 3), 
            .egr_bb_id = gtmv(m, 7), 
            .wrr_val = gtmv(m, 4), 
            .init_addr_en = gtmv(m, 1), 
            .if_width = gtmv(m, 2), 
            .eop_en = gtmv(m, 1), 
            .iso_input = gtmv(m, 1) };

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_ing_set(%u %u %u %u %u %u %u %u %u %u %u %u)\n", pfb_id, idx,
                cfg_ing.fifo_en, cfg_ing.to_val, cfg_ing.my_ch_addr, cfg_ing.egr_ch_addr, 
                cfg_ing.egr_bb_id, cfg_ing.wrr_val, cfg_ing.init_addr_en, cfg_ing.if_width, 
                cfg_ing.eop_en, cfg_ing.iso_input);
            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_set(pfb_id, idx, &cfg_ing);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        cfg_ing.fifo_en = gtmv(m, 1);
        cfg_ing.to_val = gtmv(m, 4);
        cfg_ing.my_ch_addr = gtmv(m, 3);
        cfg_ing.egr_ch_addr = gtmv(m, 3);
        cfg_ing.egr_bb_id = gtmv(m, 7);
        cfg_ing.wrr_val = gtmv(m, 4);
        cfg_ing.init_addr_en = gtmv(m, 1);
        cfg_ing.if_width = gtmv(m, 2);
        cfg_ing.eop_en = gtmv(m, 1);
        cfg_ing.iso_input = gtmv(m, 1);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_ing_set(%u %u %u %u %u %u %u %u %u %u %u %u)\n", pfb_id, idx,
                cfg_ing.fifo_en, cfg_ing.to_val, cfg_ing.my_ch_addr, cfg_ing.egr_ch_addr, 
                cfg_ing.egr_bb_id, cfg_ing.wrr_val, cfg_ing.init_addr_en, cfg_ing.if_width, 
                cfg_ing.eop_en, cfg_ing.iso_input);
            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_set(pfb_id, idx, &cfg_ing);
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

            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_get(pfb_id, idx, &cfg_ing);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_ing_get(%u %u %u %u %u %u %u %u %u %u %u %u)\n", pfb_id, idx,
                cfg_ing.fifo_en, cfg_ing.to_val, cfg_ing.my_ch_addr, cfg_ing.egr_ch_addr, 
                cfg_ing.egr_bb_id, cfg_ing.wrr_val, cfg_ing.init_addr_en, cfg_ing.if_width, 
                cfg_ing.eop_en, cfg_ing.iso_input);

            if (cfg_ing.fifo_en != gtmv(m, 1) || 
                cfg_ing.to_val != gtmv(m, 4) || 
                cfg_ing.my_ch_addr != gtmv(m, 3) || 
                cfg_ing.egr_ch_addr != gtmv(m, 3) || 
                cfg_ing.egr_bb_id != gtmv(m, 7) || 
                cfg_ing.wrr_val != gtmv(m, 4) || 
                cfg_ing.init_addr_en != gtmv(m, 1) || 
                cfg_ing.if_width != gtmv(m, 2) || 
                cfg_ing.eop_en != gtmv(m, 1) || 
                cfg_ing.iso_input != gtmv(m, 1) || 
                0)
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
        bdmf_session_print(session, "Test of cfg_ing completed. Number of tested entries %u.\n", max_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_pfb_cfg_egr:
    {
        uint8_t max_idx = 8;
        uint8_t idx = gtmv(m, 3);
        crdp_ds_cmn_pfb_cfg_egr cfg_egr = {
            .fifo_en = gtmv(m, 1), 
            .cr_rep_gran = gtmv(m, 4), 
            .my_ch_addr = gtmv(m, 3), 
            .ing_ch_addr = gtmv(m, 3), 
            .init_cr = gtmv(m, 4), 
            .ing_bb_id = gtmv(m, 7), 
            .init_addr_en = gtmv(m, 1), 
            .if_width = gtmv(m, 2), 
            .flush_en = gtmv(m, 1), 
            .iso_input = gtmv(m, 1) };

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_egr_set(%u %u %u %u %u %u %u %u %u %u %u %u)\n", pfb_id, idx,
                cfg_egr.fifo_en, cfg_egr.cr_rep_gran, cfg_egr.my_ch_addr, cfg_egr.ing_ch_addr, 
                cfg_egr.init_cr, cfg_egr.ing_bb_id, cfg_egr.init_addr_en, cfg_egr.if_width, 
                cfg_egr.flush_en, cfg_egr.iso_input);
            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_egr_set(pfb_id, idx, &cfg_egr);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        cfg_egr.fifo_en = gtmv(m, 1);
        cfg_egr.cr_rep_gran = gtmv(m, 4);
        cfg_egr.my_ch_addr = gtmv(m, 3);
        cfg_egr.ing_ch_addr = gtmv(m, 3);
        cfg_egr.init_cr = gtmv(m, 4);
        cfg_egr.ing_bb_id = gtmv(m, 7);
        cfg_egr.init_addr_en = gtmv(m, 1);
        cfg_egr.if_width = gtmv(m, 2);
        cfg_egr.flush_en = gtmv(m, 1);
        cfg_egr.iso_input = gtmv(m, 1);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_egr_set(%u %u %u %u %u %u %u %u %u %u %u %u)\n", pfb_id, idx,
                cfg_egr.fifo_en, cfg_egr.cr_rep_gran, cfg_egr.my_ch_addr, cfg_egr.ing_ch_addr, 
                cfg_egr.init_cr, cfg_egr.ing_bb_id, cfg_egr.init_addr_en, cfg_egr.if_width, 
                cfg_egr.flush_en, cfg_egr.iso_input);
            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_egr_set(pfb_id, idx, &cfg_egr);
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

            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_egr_get(pfb_id, idx, &cfg_egr);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_egr_get(%u %u %u %u %u %u %u %u %u %u %u %u)\n", pfb_id, idx,
                cfg_egr.fifo_en, cfg_egr.cr_rep_gran, cfg_egr.my_ch_addr, cfg_egr.ing_ch_addr, 
                cfg_egr.init_cr, cfg_egr.ing_bb_id, cfg_egr.init_addr_en, cfg_egr.if_width, 
                cfg_egr.flush_en, cfg_egr.iso_input);

            if (cfg_egr.fifo_en != gtmv(m, 1) || 
                cfg_egr.cr_rep_gran != gtmv(m, 4) || 
                cfg_egr.my_ch_addr != gtmv(m, 3) || 
                cfg_egr.ing_ch_addr != gtmv(m, 3) || 
                cfg_egr.init_cr != gtmv(m, 4) || 
                cfg_egr.ing_bb_id != gtmv(m, 7) || 
                cfg_egr.init_addr_en != gtmv(m, 1) || 
                cfg_egr.if_width != gtmv(m, 2) || 
                cfg_egr.flush_en != gtmv(m, 1) || 
                cfg_egr.iso_input != gtmv(m, 1) || 
                0)
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
        bdmf_session_print(session, "Test of cfg_egr completed. Number of tested entries %u.\n", max_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_pfb_cfg_ing_pwire:
    {
        uint8_t max_idx = 8;
        uint8_t idx = gtmv(m, 3);
        uint8_t mask = gtmv(m, 4);
        uint8_t to_val = gtmv(m, 4);
        uint8_t ing_spare_cfg = gtmv(m, 8);
        uint16_t pwire_val = gtmv(m, 16);

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_ing_pwire_set(%u %u %u %u %u %u)\n", pfb_id, idx,
                mask, to_val, ing_spare_cfg, pwire_val);
            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_pwire_set(pfb_id, idx, mask, to_val, ing_spare_cfg, pwire_val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        mask = gtmv(m, 4);
        to_val = gtmv(m, 4);
        ing_spare_cfg = gtmv(m, 8);
        pwire_val = gtmv(m, 16);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_ing_pwire_set(%u %u %u %u %u %u)\n", pfb_id, idx,
                mask, to_val, ing_spare_cfg, pwire_val);
            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_pwire_set(pfb_id, idx, mask, to_val, ing_spare_cfg, pwire_val);
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

            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_pwire_get(pfb_id, idx, &mask, &to_val, &ing_spare_cfg, &pwire_val);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_ing_pwire_get(%u %u %u %u %u %u)\n", pfb_id, idx,
                mask, to_val, ing_spare_cfg, pwire_val);

            if (mask != gtmv(m, 4) || to_val != gtmv(m, 4) || ing_spare_cfg != gtmv(m, 8) || pwire_val != gtmv(m, 16) || 0)
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
        bdmf_session_print(session, "Test of cfg_ing_pwire completed. Number of tested entries %u.\n", max_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_pfb_cfg_egr_pwire:
    {
        uint8_t max_idx = 8;
        uint8_t idx = gtmv(m, 3);
        uint8_t mask = gtmv(m, 4);
        uint8_t to_val = gtmv(m, 4);
        uint8_t egr_spare_cfg = gtmv(m, 8);
        uint16_t pwire_val = gtmv(m, 16);

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_egr_pwire_set(%u %u %u %u %u %u)\n", pfb_id, idx,
                mask, to_val, egr_spare_cfg, pwire_val);
            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_egr_pwire_set(pfb_id, idx, mask, to_val, egr_spare_cfg, pwire_val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        mask = gtmv(m, 4);
        to_val = gtmv(m, 4);
        egr_spare_cfg = gtmv(m, 8);
        pwire_val = gtmv(m, 16);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_egr_pwire_set(%u %u %u %u %u %u)\n", pfb_id, idx,
                mask, to_val, egr_spare_cfg, pwire_val);
            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_egr_pwire_set(pfb_id, idx, mask, to_val, egr_spare_cfg, pwire_val);
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

            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_egr_pwire_get(pfb_id, idx, &mask, &to_val, &egr_spare_cfg, &pwire_val);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_egr_pwire_get(%u %u %u %u %u %u)\n", pfb_id, idx,
                mask, to_val, egr_spare_cfg, pwire_val);

            if (mask != gtmv(m, 4) || to_val != gtmv(m, 4) || egr_spare_cfg != gtmv(m, 8) || pwire_val != gtmv(m, 16) || 0)
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
        bdmf_session_print(session, "Test of cfg_egr_pwire completed. Number of tested entries %u.\n", max_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_crdp_ds_cmn_pfb_cfg_ing_eop_mask:
    {
        uint8_t max_idx = 8;
        uint8_t idx = gtmv(m, 3);
        uint16_t mask = gtmv(m, 16);
        uint16_t eop_msb_mask = gtmv(m, 16);

        if ((start_idx >= max_idx) || (stop_idx >= max_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (idx = 0; idx < max_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_ing_eop_mask_set(%u %u %u %u)\n", pfb_id, idx,
                mask, eop_msb_mask);
            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_eop_mask_set(pfb_id, idx, mask, eop_msb_mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        mask = gtmv(m, 16);
        eop_msb_mask = gtmv(m, 16);

        for (idx = start_idx; idx <= stop_idx; idx++)
        {
            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_ing_eop_mask_set(%u %u %u %u)\n", pfb_id, idx,
                mask, eop_msb_mask);
            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_eop_mask_set(pfb_id, idx, mask, eop_msb_mask);
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

            ag_err = ag_drv_crdp_ds_cmn_pfb_cfg_ing_eop_mask_get(pfb_id, idx, &mask, &eop_msb_mask);

            bdmf_session_print(session, "ag_drv_crdp_ds_cmn_pfb_cfg_ing_eop_mask_get(%u %u %u %u)\n", pfb_id, idx,
                mask, eop_msb_mask);

            if (mask != gtmv(m, 16) || eop_msb_mask != gtmv(m, 16) || 0)
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
        bdmf_session_print(session, "Test of cfg_ing_eop_mask completed. Number of tested entries %u.\n", max_idx);
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
static int ag_drv_crdp_ds_cmn_pfb_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_cfg_ing: reg = &RU_REG(CRDP_DS_CMN_PFB, CFG_ING); blk = &RU_BLK(CRDP_DS_CMN_PFB); break;
    case bdmf_address_cfg_egr: reg = &RU_REG(CRDP_DS_CMN_PFB, CFG_EGR); blk = &RU_BLK(CRDP_DS_CMN_PFB); break;
    case bdmf_address_cfg_gbl: reg = &RU_REG(CRDP_DS_CMN_PFB, CFG_GBL); blk = &RU_BLK(CRDP_DS_CMN_PFB); break;
    case bdmf_address_cfg_ing_pwire: reg = &RU_REG(CRDP_DS_CMN_PFB, CFG_ING_PWIRE); blk = &RU_BLK(CRDP_DS_CMN_PFB); break;
    case bdmf_address_cfg_egr_pwire: reg = &RU_REG(CRDP_DS_CMN_PFB, CFG_EGR_PWIRE); blk = &RU_BLK(CRDP_DS_CMN_PFB); break;
    case bdmf_address_cfg_ing_eop_mask: reg = &RU_REG(CRDP_DS_CMN_PFB, CFG_ING_EOP_MASK); blk = &RU_BLK(CRDP_DS_CMN_PFB); break;
    case bdmf_address_dbg_fifo_status: reg = &RU_REG(CRDP_DS_CMN_PFB, DBG_FIFO_STATUS); blk = &RU_BLK(CRDP_DS_CMN_PFB); break;
    case bdmf_address_dbg_ingress_used_words: reg = &RU_REG(CRDP_DS_CMN_PFB, DBG_INGRESS_USED_WORDS); blk = &RU_BLK(CRDP_DS_CMN_PFB); break;
    case bdmf_address_dbg_egress_used_words: reg = &RU_REG(CRDP_DS_CMN_PFB, DBG_EGRESS_USED_WORDS); blk = &RU_BLK(CRDP_DS_CMN_PFB); break;
    case bdmf_address_dbg_ingress_bb_msg_trans: reg = &RU_REG(CRDP_DS_CMN_PFB, DBG_INGRESS_BB_MSG_TRANS); blk = &RU_BLK(CRDP_DS_CMN_PFB); break;
    case bdmf_address_dbg_egress_bb_msg_trans: reg = &RU_REG(CRDP_DS_CMN_PFB, DBG_EGRESS_BB_MSG_TRANS); blk = &RU_BLK(CRDP_DS_CMN_PFB); break;
    case bdmf_address_dbg_ingress_credit_cnt: reg = &RU_REG(CRDP_DS_CMN_PFB, DBG_INGRESS_CREDIT_CNT); blk = &RU_BLK(CRDP_DS_CMN_PFB); break;
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

bdmfmon_handle_t ag_drv_crdp_ds_cmn_pfb_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "crdp_ds_cmn_pfb", "crdp_ds_cmn_pfb", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_cfg_ing[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fifo_en", "fifo_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("to_val", "to_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("my_ch_addr", "my_ch_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("egr_ch_addr", "egr_ch_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("egr_bb_id", "egr_bb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("wrr_val", "wrr_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("init_addr_en", "init_addr_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("if_width", "if_width", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("eop_en", "eop_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("iso_input", "iso_input", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_egr[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fifo_en", "fifo_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cr_rep_gran", "cr_rep_gran", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("my_ch_addr", "my_ch_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ing_ch_addr", "ing_ch_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("init_cr", "init_cr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ing_bb_id", "ing_bb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("init_addr_en", "init_addr_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("if_width", "if_width", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flush_en", "flush_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("iso_input", "iso_input", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_gbl[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("arb_scheme", "arb_scheme", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("route_addr_overwr_en", "route_addr_overwr_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("src_bb_id", "src_bb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("src_bb_id_ovr_en", "src_bb_id_ovr_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flush_pace_timeout", "flush_pace_timeout", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("route_addr_overwr_val", "route_addr_overwr_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_ing_pwire[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mask", "mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("to_val", "to_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ing_spare_cfg", "ing_spare_cfg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pwire_val", "pwire_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_egr_pwire[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mask", "mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("to_val", "to_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("egr_spare_cfg", "egr_spare_cfg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pwire_val", "pwire_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_ing_eop_mask[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mask", "mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("eop_msb_mask", "eop_msb_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "cfg_ing", .val = cli_crdp_ds_cmn_pfb_cfg_ing, .parms = set_cfg_ing },
            { .name = "cfg_egr", .val = cli_crdp_ds_cmn_pfb_cfg_egr, .parms = set_cfg_egr },
            { .name = "cfg_gbl", .val = cli_crdp_ds_cmn_pfb_cfg_gbl, .parms = set_cfg_gbl },
            { .name = "cfg_ing_pwire", .val = cli_crdp_ds_cmn_pfb_cfg_ing_pwire, .parms = set_cfg_ing_pwire },
            { .name = "cfg_egr_pwire", .val = cli_crdp_ds_cmn_pfb_cfg_egr_pwire, .parms = set_cfg_egr_pwire },
            { .name = "cfg_ing_eop_mask", .val = cli_crdp_ds_cmn_pfb_cfg_ing_eop_mask, .parms = set_cfg_ing_eop_mask },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_crdp_ds_cmn_pfb_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_cfg_ing[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_cfg_egr[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_cfg_ing_pwire[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_cfg_egr_pwire[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_cfg_ing_eop_mask[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_dbg_ingress_used_words[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_dbg_egress_used_words[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_dbg_ingress_bb_msg_trans[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_dbg_egress_bb_msg_trans[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_dbg_ingress_credit_cnt[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "cfg_ing", .val = cli_crdp_ds_cmn_pfb_cfg_ing, .parms = get_cfg_ing },
            { .name = "cfg_egr", .val = cli_crdp_ds_cmn_pfb_cfg_egr, .parms = get_cfg_egr },
            { .name = "cfg_gbl", .val = cli_crdp_ds_cmn_pfb_cfg_gbl, .parms = get_default },
            { .name = "cfg_ing_pwire", .val = cli_crdp_ds_cmn_pfb_cfg_ing_pwire, .parms = get_cfg_ing_pwire },
            { .name = "cfg_egr_pwire", .val = cli_crdp_ds_cmn_pfb_cfg_egr_pwire, .parms = get_cfg_egr_pwire },
            { .name = "cfg_ing_eop_mask", .val = cli_crdp_ds_cmn_pfb_cfg_ing_eop_mask, .parms = get_cfg_ing_eop_mask },
            { .name = "dbg_fifo_status", .val = cli_crdp_ds_cmn_pfb_dbg_fifo_status, .parms = get_default },
            { .name = "dbg_ingress_used_words", .val = cli_crdp_ds_cmn_pfb_dbg_ingress_used_words, .parms = get_dbg_ingress_used_words },
            { .name = "dbg_egress_used_words", .val = cli_crdp_ds_cmn_pfb_dbg_egress_used_words, .parms = get_dbg_egress_used_words },
            { .name = "dbg_ingress_bb_msg_trans", .val = cli_crdp_ds_cmn_pfb_dbg_ingress_bb_msg_trans, .parms = get_dbg_ingress_bb_msg_trans },
            { .name = "dbg_egress_bb_msg_trans", .val = cli_crdp_ds_cmn_pfb_dbg_egress_bb_msg_trans, .parms = get_dbg_egress_bb_msg_trans },
            { .name = "dbg_ingress_credit_cnt", .val = cli_crdp_ds_cmn_pfb_dbg_ingress_credit_cnt, .parms = get_dbg_ingress_credit_cnt },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_crdp_ds_cmn_pfb_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_crdp_ds_cmn_pfb_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_MAKE_PARM("pfb_id", "pfb_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "cfg_ing", .val = cli_crdp_ds_cmn_pfb_cfg_ing, .parms = ext_test_default},
            { .name = "cfg_egr", .val = cli_crdp_ds_cmn_pfb_cfg_egr, .parms = ext_test_default},
            { .name = "cfg_ing_pwire", .val = cli_crdp_ds_cmn_pfb_cfg_ing_pwire, .parms = ext_test_default},
            { .name = "cfg_egr_pwire", .val = cli_crdp_ds_cmn_pfb_cfg_egr_pwire, .parms = ext_test_default},
            { .name = "cfg_ing_eop_mask", .val = cli_crdp_ds_cmn_pfb_cfg_ing_eop_mask, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_crdp_ds_cmn_pfb_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "CFG_ING", .val = bdmf_address_cfg_ing },
            { .name = "CFG_EGR", .val = bdmf_address_cfg_egr },
            { .name = "CFG_GBL", .val = bdmf_address_cfg_gbl },
            { .name = "CFG_ING_PWIRE", .val = bdmf_address_cfg_ing_pwire },
            { .name = "CFG_EGR_PWIRE", .val = bdmf_address_cfg_egr_pwire },
            { .name = "CFG_ING_EOP_MASK", .val = bdmf_address_cfg_ing_eop_mask },
            { .name = "DBG_FIFO_STATUS", .val = bdmf_address_dbg_fifo_status },
            { .name = "DBG_INGRESS_USED_WORDS", .val = bdmf_address_dbg_ingress_used_words },
            { .name = "DBG_EGRESS_USED_WORDS", .val = bdmf_address_dbg_egress_used_words },
            { .name = "DBG_INGRESS_BB_MSG_TRANS", .val = bdmf_address_dbg_ingress_bb_msg_trans },
            { .name = "DBG_EGRESS_BB_MSG_TRANS", .val = bdmf_address_dbg_egress_bb_msg_trans },
            { .name = "DBG_INGRESS_CREDIT_CNT", .val = bdmf_address_dbg_ingress_credit_cnt },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_crdp_ds_cmn_pfb_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "pfb_id", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
