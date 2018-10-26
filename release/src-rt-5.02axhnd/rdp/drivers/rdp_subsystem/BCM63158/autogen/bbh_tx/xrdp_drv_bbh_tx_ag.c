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
#include "xrdp_drv_bbh_tx_ag.h"

#define BLOCK_ADDR_COUNT_BITS 3
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_bbh_tx_mac_type_set(uint8_t bbh_id, uint8_t type)
{
    uint32_t reg_common_configurations_mactype=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (type >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_mactype = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_MACTYPE, TYPE, reg_common_configurations_mactype, type);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_MACTYPE, reg_common_configurations_mactype);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_mac_type_get(uint8_t bbh_id, uint8_t *type)
{
    uint32_t reg_common_configurations_mactype;

#ifdef VALIDATE_PARMS
    if(!type)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_MACTYPE, reg_common_configurations_mactype);

    *type = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_MACTYPE, TYPE, reg_common_configurations_mactype);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_cfg_src_id_set(uint8_t bbh_id, const bbh_tx_cfg_src_id *cfg_src_id)
{
    uint32_t reg_common_configurations_bbcfg_1_tx=0;
    uint32_t reg_common_configurations_bbcfg_2_tx=0;

#ifdef VALIDATE_PARMS
    if(!cfg_src_id)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (cfg_src_id->fpmsrc >= _6BITS_MAX_VAL_) ||
       (cfg_src_id->sbpmsrc >= _6BITS_MAX_VAL_) ||
       (cfg_src_id->stsrnrsrc >= _6BITS_MAX_VAL_) ||
       (cfg_src_id->msgrnrsrc >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_common_configurations_bbcfg_1_tx);
    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_2_TX, reg_common_configurations_bbcfg_2_tx);

    reg_common_configurations_bbcfg_1_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, FPMSRC, reg_common_configurations_bbcfg_1_tx, cfg_src_id->fpmsrc);
    reg_common_configurations_bbcfg_1_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, SBPMSRC, reg_common_configurations_bbcfg_1_tx, cfg_src_id->sbpmsrc);
    reg_common_configurations_bbcfg_2_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_2_TX, STSRNRSRC, reg_common_configurations_bbcfg_2_tx, cfg_src_id->stsrnrsrc);
    reg_common_configurations_bbcfg_2_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_2_TX, MSGRNRSRC, reg_common_configurations_bbcfg_2_tx, cfg_src_id->msgrnrsrc);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_common_configurations_bbcfg_1_tx);
    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_2_TX, reg_common_configurations_bbcfg_2_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_cfg_src_id_get(uint8_t bbh_id, bbh_tx_cfg_src_id *cfg_src_id)
{
    uint32_t reg_common_configurations_bbcfg_1_tx;
    uint32_t reg_common_configurations_bbcfg_2_tx;

#ifdef VALIDATE_PARMS
    if(!cfg_src_id)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_common_configurations_bbcfg_1_tx);
    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_2_TX, reg_common_configurations_bbcfg_2_tx);

    cfg_src_id->fpmsrc = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, FPMSRC, reg_common_configurations_bbcfg_1_tx);
    cfg_src_id->sbpmsrc = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, SBPMSRC, reg_common_configurations_bbcfg_1_tx);
    cfg_src_id->stsrnrsrc = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_2_TX, STSRNRSRC, reg_common_configurations_bbcfg_2_tx);
    cfg_src_id->msgrnrsrc = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_2_TX, MSGRNRSRC, reg_common_configurations_bbcfg_2_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_rnr_src_id_set(uint8_t bbh_id, uint8_t pdrnr0src, uint8_t pdrnr1src)
{
    uint32_t reg_common_configurations_bbcfg_2_tx=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (pdrnr0src >= _6BITS_MAX_VAL_) ||
       (pdrnr1src >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_2_TX, reg_common_configurations_bbcfg_2_tx);

    reg_common_configurations_bbcfg_2_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_2_TX, PDRNR0SRC, reg_common_configurations_bbcfg_2_tx, pdrnr0src);
    reg_common_configurations_bbcfg_2_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_2_TX, PDRNR1SRC, reg_common_configurations_bbcfg_2_tx, pdrnr1src);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_2_TX, reg_common_configurations_bbcfg_2_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_rnr_src_id_get(uint8_t bbh_id, uint8_t *pdrnr0src, uint8_t *pdrnr1src)
{
    uint32_t reg_common_configurations_bbcfg_2_tx;

#ifdef VALIDATE_PARMS
    if(!pdrnr0src || !pdrnr1src)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_2_TX, reg_common_configurations_bbcfg_2_tx);

    *pdrnr0src = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_2_TX, PDRNR0SRC, reg_common_configurations_bbcfg_2_tx);
    *pdrnr1src = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_2_TX, PDRNR1SRC, reg_common_configurations_bbcfg_2_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_bbh_dma_cfg_set(uint8_t bbh_id, const bbh_tx_bbh_dma_cfg *bbh_dma_cfg)
{
    uint32_t reg_common_configurations_bbcfg_1_tx=0;
    uint32_t reg_common_configurations_dmacfg_tx=0;

#ifdef VALIDATE_PARMS
    if(!bbh_dma_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (bbh_dma_cfg->dmasrc >= _6BITS_MAX_VAL_) ||
       (bbh_dma_cfg->descbase >= _6BITS_MAX_VAL_) ||
       (bbh_dma_cfg->descsize >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_common_configurations_bbcfg_1_tx);
    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, reg_common_configurations_dmacfg_tx);

    reg_common_configurations_bbcfg_1_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, DMASRC, reg_common_configurations_bbcfg_1_tx, bbh_dma_cfg->dmasrc);
    reg_common_configurations_dmacfg_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, DESCBASE, reg_common_configurations_dmacfg_tx, bbh_dma_cfg->descbase);
    reg_common_configurations_dmacfg_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, DESCSIZE, reg_common_configurations_dmacfg_tx, bbh_dma_cfg->descsize);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_common_configurations_bbcfg_1_tx);
    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, reg_common_configurations_dmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_bbh_dma_cfg_get(uint8_t bbh_id, bbh_tx_bbh_dma_cfg *bbh_dma_cfg)
{
    uint32_t reg_common_configurations_bbcfg_1_tx;
    uint32_t reg_common_configurations_dmacfg_tx;

#ifdef VALIDATE_PARMS
    if(!bbh_dma_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_common_configurations_bbcfg_1_tx);
    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, reg_common_configurations_dmacfg_tx);

    bbh_dma_cfg->dmasrc = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, DMASRC, reg_common_configurations_bbcfg_1_tx);
    bbh_dma_cfg->descbase = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, DESCBASE, reg_common_configurations_dmacfg_tx);
    bbh_dma_cfg->descsize = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, DESCSIZE, reg_common_configurations_dmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_dma_max_otf_read_request_set(uint8_t bbh_id, uint8_t maxreq)
{
    uint32_t reg_common_configurations_dmacfg_tx=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (maxreq >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, reg_common_configurations_dmacfg_tx);

    reg_common_configurations_dmacfg_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, MAXREQ, reg_common_configurations_dmacfg_tx, maxreq);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, reg_common_configurations_dmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_dma_max_otf_read_request_get(uint8_t bbh_id, uint8_t *maxreq)
{
    uint32_t reg_common_configurations_dmacfg_tx;

#ifdef VALIDATE_PARMS
    if(!maxreq)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, reg_common_configurations_dmacfg_tx);

    *maxreq = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, MAXREQ, reg_common_configurations_dmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_dma_epon_urgent_set(uint8_t bbh_id, bdmf_boolean epnurgnt)
{
    uint32_t reg_common_configurations_dmacfg_tx=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (epnurgnt >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, reg_common_configurations_dmacfg_tx);

    reg_common_configurations_dmacfg_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, EPNURGNT, reg_common_configurations_dmacfg_tx, epnurgnt);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, reg_common_configurations_dmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_dma_epon_urgent_get(uint8_t bbh_id, bdmf_boolean *epnurgnt)
{
    uint32_t reg_common_configurations_dmacfg_tx;

#ifdef VALIDATE_PARMS
    if(!epnurgnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, reg_common_configurations_dmacfg_tx);

    *epnurgnt = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX, EPNURGNT, reg_common_configurations_dmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_bbh_sdma_cfg_set(uint8_t bbh_id, const bbh_tx_bbh_sdma_cfg *bbh_sdma_cfg)
{
    uint32_t reg_common_configurations_bbcfg_1_tx=0;
    uint32_t reg_common_configurations_sdmacfg_tx=0;

#ifdef VALIDATE_PARMS
    if(!bbh_sdma_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (bbh_sdma_cfg->sdmasrc >= _6BITS_MAX_VAL_) ||
       (bbh_sdma_cfg->descbase >= _6BITS_MAX_VAL_) ||
       (bbh_sdma_cfg->descsize >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_common_configurations_bbcfg_1_tx);
    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, reg_common_configurations_sdmacfg_tx);

    reg_common_configurations_bbcfg_1_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, SDMASRC, reg_common_configurations_bbcfg_1_tx, bbh_sdma_cfg->sdmasrc);
    reg_common_configurations_sdmacfg_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, DESCBASE, reg_common_configurations_sdmacfg_tx, bbh_sdma_cfg->descbase);
    reg_common_configurations_sdmacfg_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, DESCSIZE, reg_common_configurations_sdmacfg_tx, bbh_sdma_cfg->descsize);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_common_configurations_bbcfg_1_tx);
    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, reg_common_configurations_sdmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_bbh_sdma_cfg_get(uint8_t bbh_id, bbh_tx_bbh_sdma_cfg *bbh_sdma_cfg)
{
    uint32_t reg_common_configurations_bbcfg_1_tx;
    uint32_t reg_common_configurations_sdmacfg_tx;

#ifdef VALIDATE_PARMS
    if(!bbh_sdma_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, reg_common_configurations_bbcfg_1_tx);
    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, reg_common_configurations_sdmacfg_tx);

    bbh_sdma_cfg->sdmasrc = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX, SDMASRC, reg_common_configurations_bbcfg_1_tx);
    bbh_sdma_cfg->descbase = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, DESCBASE, reg_common_configurations_sdmacfg_tx);
    bbh_sdma_cfg->descsize = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, DESCSIZE, reg_common_configurations_sdmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_sdma_max_otf_read_request_set(uint8_t bbh_id, uint8_t maxreq)
{
    uint32_t reg_common_configurations_sdmacfg_tx=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (maxreq >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, reg_common_configurations_sdmacfg_tx);

    reg_common_configurations_sdmacfg_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, MAXREQ, reg_common_configurations_sdmacfg_tx, maxreq);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, reg_common_configurations_sdmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_sdma_max_otf_read_request_get(uint8_t bbh_id, uint8_t *maxreq)
{
    uint32_t reg_common_configurations_sdmacfg_tx;

#ifdef VALIDATE_PARMS
    if(!maxreq)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, reg_common_configurations_sdmacfg_tx);

    *maxreq = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, MAXREQ, reg_common_configurations_sdmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_sdma_epon_urgent_set(uint8_t bbh_id, bdmf_boolean epnurgnt)
{
    uint32_t reg_common_configurations_sdmacfg_tx=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (epnurgnt >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, reg_common_configurations_sdmacfg_tx);

    reg_common_configurations_sdmacfg_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, EPNURGNT, reg_common_configurations_sdmacfg_tx, epnurgnt);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, reg_common_configurations_sdmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_sdma_epon_urgent_get(uint8_t bbh_id, bdmf_boolean *epnurgnt)
{
    uint32_t reg_common_configurations_sdmacfg_tx;

#ifdef VALIDATE_PARMS
    if(!epnurgnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, reg_common_configurations_sdmacfg_tx);

    *epnurgnt = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX, EPNURGNT, reg_common_configurations_sdmacfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_bbh_ddr_cfg_set(uint8_t bbh_id, const bbh_tx_bbh_ddr_cfg *bbh_ddr_cfg)
{
    uint32_t reg_common_configurations_ddrcfg_tx=0;

#ifdef VALIDATE_PARMS
    if(!bbh_ddr_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (bbh_ddr_cfg->bufsize >= _3BITS_MAX_VAL_) ||
       (bbh_ddr_cfg->byteresul >= _1BITS_MAX_VAL_) ||
       (bbh_ddr_cfg->ddrtxoffset >= _9BITS_MAX_VAL_) ||
       (bbh_ddr_cfg->hnsize0 >= _7BITS_MAX_VAL_) ||
       (bbh_ddr_cfg->hnsize1 >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_ddrcfg_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, BUFSIZE, reg_common_configurations_ddrcfg_tx, bbh_ddr_cfg->bufsize);
    reg_common_configurations_ddrcfg_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, BYTERESUL, reg_common_configurations_ddrcfg_tx, bbh_ddr_cfg->byteresul);
    reg_common_configurations_ddrcfg_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, DDRTXOFFSET, reg_common_configurations_ddrcfg_tx, bbh_ddr_cfg->ddrtxoffset);
    reg_common_configurations_ddrcfg_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, HNSIZE0, reg_common_configurations_ddrcfg_tx, bbh_ddr_cfg->hnsize0);
    reg_common_configurations_ddrcfg_tx = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, HNSIZE1, reg_common_configurations_ddrcfg_tx, bbh_ddr_cfg->hnsize1);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, reg_common_configurations_ddrcfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_bbh_ddr_cfg_get(uint8_t bbh_id, bbh_tx_bbh_ddr_cfg *bbh_ddr_cfg)
{
    uint32_t reg_common_configurations_ddrcfg_tx;

#ifdef VALIDATE_PARMS
    if(!bbh_ddr_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, reg_common_configurations_ddrcfg_tx);

    bbh_ddr_cfg->bufsize = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, BUFSIZE, reg_common_configurations_ddrcfg_tx);
    bbh_ddr_cfg->byteresul = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, BYTERESUL, reg_common_configurations_ddrcfg_tx);
    bbh_ddr_cfg->ddrtxoffset = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, DDRTXOFFSET, reg_common_configurations_ddrcfg_tx);
    bbh_ddr_cfg->hnsize0 = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, HNSIZE0, reg_common_configurations_ddrcfg_tx);
    bbh_ddr_cfg->hnsize1 = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX, HNSIZE1, reg_common_configurations_ddrcfg_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_debug_counters_get(uint8_t bbh_id, bbh_tx_debug_counters *debug_counters)
{
    uint32_t reg_debug_counters_srampd;
    uint32_t reg_debug_counters_ddrpd;
    uint32_t reg_debug_counters_pddrop;
    uint32_t reg_debug_counters_stscnt;
    uint32_t reg_debug_counters_stsdrop;
    uint32_t reg_debug_counters_msgcnt;
    uint32_t reg_debug_counters_msgdrop;
    uint32_t reg_debug_counters_getnextnull;
    uint32_t reg_debug_counters_lenerr;
    uint32_t reg_debug_counters_aggrlenerr;
    uint32_t reg_debug_counters_srampkt;
    uint32_t reg_debug_counters_ddrpkt;
    uint32_t reg_debug_counters_flushpkts;

#ifdef VALIDATE_PARMS
    if(!debug_counters)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_SRAMPD, reg_debug_counters_srampd);
    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_DDRPD, reg_debug_counters_ddrpd);
    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_PDDROP, reg_debug_counters_pddrop);
    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_STSCNT, reg_debug_counters_stscnt);
    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_STSDROP, reg_debug_counters_stsdrop);
    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_MSGCNT, reg_debug_counters_msgcnt);
    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_MSGDROP, reg_debug_counters_msgdrop);
    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_GETNEXTNULL, reg_debug_counters_getnextnull);
    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_LENERR, reg_debug_counters_lenerr);
    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_AGGRLENERR, reg_debug_counters_aggrlenerr);
    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_SRAMPKT, reg_debug_counters_srampkt);
    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_DDRPKT, reg_debug_counters_ddrpkt);
    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_FLUSHPKTS, reg_debug_counters_flushpkts);

    debug_counters->srampd = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SRAMPD, SRAMPD, reg_debug_counters_srampd);
    debug_counters->ddrpd = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_DDRPD, DDRPD, reg_debug_counters_ddrpd);
    debug_counters->pddrop = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_PDDROP, PDDROP, reg_debug_counters_pddrop);
    debug_counters->stscnt = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_STSCNT, STSCNT, reg_debug_counters_stscnt);
    debug_counters->stsdrop = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_STSDROP, STSDROP, reg_debug_counters_stsdrop);
    debug_counters->msgcnt = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_MSGCNT, MSGCNT, reg_debug_counters_msgcnt);
    debug_counters->msgdrop = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_MSGDROP, MSGDROP, reg_debug_counters_msgdrop);
    debug_counters->getnextnull = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_GETNEXTNULL, GETNEXTNULL, reg_debug_counters_getnextnull);
    debug_counters->lenerr = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_LENERR, LENERR, reg_debug_counters_lenerr);
    debug_counters->aggrlenerr = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_AGGRLENERR, AGGRLENERR, reg_debug_counters_aggrlenerr);
    debug_counters->srampkt = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SRAMPKT, SRAMPKT, reg_debug_counters_srampkt);
    debug_counters->ddrpkt = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_DDRPKT, DDRPKT, reg_debug_counters_ddrpkt);
    debug_counters->flshpkts = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_FLUSHPKTS, FLSHPKTS, reg_debug_counters_flushpkts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_rnrcfg_1_set(uint8_t bbh_id, uint8_t rnr_cfg_index_1, uint16_t tcontaddr, uint16_t skbaddr)
{
    uint32_t reg_common_configurations_rnrcfg_1=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (rnr_cfg_index_1 >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_rnrcfg_1 = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_RNRCFG_1, TCONTADDR, reg_common_configurations_rnrcfg_1, tcontaddr);
    reg_common_configurations_rnrcfg_1 = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_RNRCFG_1, SKBADDR, reg_common_configurations_rnrcfg_1, skbaddr);

    RU_REG_RAM_WRITE(bbh_id, rnr_cfg_index_1, BBH_TX, COMMON_CONFIGURATIONS_RNRCFG_1, reg_common_configurations_rnrcfg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_rnrcfg_1_get(uint8_t bbh_id, uint8_t rnr_cfg_index_1, uint16_t *tcontaddr, uint16_t *skbaddr)
{
    uint32_t reg_common_configurations_rnrcfg_1;

#ifdef VALIDATE_PARMS
    if(!tcontaddr || !skbaddr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (rnr_cfg_index_1 >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, rnr_cfg_index_1, BBH_TX, COMMON_CONFIGURATIONS_RNRCFG_1, reg_common_configurations_rnrcfg_1);

    *tcontaddr = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_RNRCFG_1, TCONTADDR, reg_common_configurations_rnrcfg_1);
    *skbaddr = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_RNRCFG_1, SKBADDR, reg_common_configurations_rnrcfg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_rnrcfg_2_set(uint8_t bbh_id, uint16_t rnr_cfg_index_2, uint16_t ptraddr, uint8_t task)
{
    uint32_t reg_common_configurations_rnrcfg_2=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (rnr_cfg_index_2 >= 2) ||
       (task >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_rnrcfg_2 = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_RNRCFG_2, PTRADDR, reg_common_configurations_rnrcfg_2, ptraddr);
    reg_common_configurations_rnrcfg_2 = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_RNRCFG_2, TASK, reg_common_configurations_rnrcfg_2, task);

    RU_REG_RAM_WRITE(bbh_id, rnr_cfg_index_2, BBH_TX, COMMON_CONFIGURATIONS_RNRCFG_2, reg_common_configurations_rnrcfg_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_rnrcfg_2_get(uint8_t bbh_id, uint16_t rnr_cfg_index_2, uint16_t *ptraddr, uint8_t *task)
{
    uint32_t reg_common_configurations_rnrcfg_2;

#ifdef VALIDATE_PARMS
    if(!ptraddr || !task)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (rnr_cfg_index_2 >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, rnr_cfg_index_2, BBH_TX, COMMON_CONFIGURATIONS_RNRCFG_2, reg_common_configurations_rnrcfg_2);

    *ptraddr = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_RNRCFG_2, PTRADDR, reg_common_configurations_rnrcfg_2);
    *task = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_RNRCFG_2, TASK, reg_common_configurations_rnrcfg_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_sbpmcfg_set(uint8_t bbh_id, bdmf_boolean freenocntxt, bdmf_boolean specialfree, uint8_t maxgn)
{
    uint32_t reg_common_configurations_sbpmcfg=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (freenocntxt >= _1BITS_MAX_VAL_) ||
       (specialfree >= _1BITS_MAX_VAL_) ||
       (maxgn >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_sbpmcfg = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, FREENOCNTXT, reg_common_configurations_sbpmcfg, freenocntxt);
    reg_common_configurations_sbpmcfg = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, SPECIALFREE, reg_common_configurations_sbpmcfg, specialfree);
    reg_common_configurations_sbpmcfg = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, MAXGN, reg_common_configurations_sbpmcfg, maxgn);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, reg_common_configurations_sbpmcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_sbpmcfg_get(uint8_t bbh_id, bdmf_boolean *freenocntxt, bdmf_boolean *specialfree, uint8_t *maxgn)
{
    uint32_t reg_common_configurations_sbpmcfg;

#ifdef VALIDATE_PARMS
    if(!freenocntxt || !specialfree || !maxgn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, reg_common_configurations_sbpmcfg);

    *freenocntxt = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, FREENOCNTXT, reg_common_configurations_sbpmcfg);
    *specialfree = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, SPECIALFREE, reg_common_configurations_sbpmcfg);
    *maxgn = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG, MAXGN, reg_common_configurations_sbpmcfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrtmbasel_set(uint8_t bbh_id, uint8_t zero, const bbh_tx_common_configurations_ddrtmbasel *common_configurations_ddrtmbasel)
{
#ifdef VALIDATE_PARMS
    if(!common_configurations_ddrtmbasel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_WRITE(bbh_id, zero *2 + 0, BBH_TX, COMMON_CONFIGURATIONS_DDRTMBASEL, common_configurations_ddrtmbasel->addr[0]);
    RU_REG_RAM_WRITE(bbh_id, zero *2 + 1, BBH_TX, COMMON_CONFIGURATIONS_DDRTMBASEL, common_configurations_ddrtmbasel->addr[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrtmbasel_get(uint8_t bbh_id, uint8_t zero, bbh_tx_common_configurations_ddrtmbasel *common_configurations_ddrtmbasel)
{
#ifdef VALIDATE_PARMS
    if(!common_configurations_ddrtmbasel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero *2 + 0, BBH_TX, COMMON_CONFIGURATIONS_DDRTMBASEL, common_configurations_ddrtmbasel->addr[0]);
    RU_REG_RAM_READ(bbh_id, zero *2 + 1, BBH_TX, COMMON_CONFIGURATIONS_DDRTMBASEL, common_configurations_ddrtmbasel->addr[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrtmbaseh_set(uint8_t bbh_id, uint8_t zero, const bbh_tx_common_configurations_ddrtmbaseh *common_configurations_ddrtmbaseh)
{
#ifdef VALIDATE_PARMS
    if(!common_configurations_ddrtmbaseh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_WRITE(bbh_id, zero *2 + 0, BBH_TX, COMMON_CONFIGURATIONS_DDRTMBASEH, common_configurations_ddrtmbaseh->addr[0]);
    RU_REG_RAM_WRITE(bbh_id, zero *2 + 1, BBH_TX, COMMON_CONFIGURATIONS_DDRTMBASEH, common_configurations_ddrtmbaseh->addr[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrtmbaseh_get(uint8_t bbh_id, uint8_t zero, bbh_tx_common_configurations_ddrtmbaseh *common_configurations_ddrtmbaseh)
{
#ifdef VALIDATE_PARMS
    if(!common_configurations_ddrtmbaseh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero *2 + 0, BBH_TX, COMMON_CONFIGURATIONS_DDRTMBASEH, common_configurations_ddrtmbaseh->addr[0]);
    RU_REG_RAM_READ(bbh_id, zero *2 + 1, BBH_TX, COMMON_CONFIGURATIONS_DDRTMBASEH, common_configurations_ddrtmbaseh->addr[1]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_dfifoctrl_set(uint8_t bbh_id, uint16_t psramsize, uint16_t ddrsize, uint16_t psrambase)
{
    uint32_t reg_common_configurations_dfifoctrl=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (psramsize >= _10BITS_MAX_VAL_) ||
       (ddrsize >= _10BITS_MAX_VAL_) ||
       (psrambase >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_dfifoctrl = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DFIFOCTRL, PSRAMSIZE, reg_common_configurations_dfifoctrl, psramsize);
    reg_common_configurations_dfifoctrl = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DFIFOCTRL, DDRSIZE, reg_common_configurations_dfifoctrl, ddrsize);
    reg_common_configurations_dfifoctrl = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DFIFOCTRL, PSRAMBASE, reg_common_configurations_dfifoctrl, psrambase);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DFIFOCTRL, reg_common_configurations_dfifoctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_dfifoctrl_get(uint8_t bbh_id, uint16_t *psramsize, uint16_t *ddrsize, uint16_t *psrambase)
{
    uint32_t reg_common_configurations_dfifoctrl;

#ifdef VALIDATE_PARMS
    if(!psramsize || !ddrsize || !psrambase)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DFIFOCTRL, reg_common_configurations_dfifoctrl);

    *psramsize = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DFIFOCTRL, PSRAMSIZE, reg_common_configurations_dfifoctrl);
    *ddrsize = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DFIFOCTRL, DDRSIZE, reg_common_configurations_dfifoctrl);
    *psrambase = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DFIFOCTRL, PSRAMBASE, reg_common_configurations_dfifoctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_arb_cfg_set(uint8_t bbh_id, bdmf_boolean hightrxq)
{
    uint32_t reg_common_configurations_arb_cfg=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (hightrxq >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_arb_cfg = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_ARB_CFG, HIGHTRXQ, reg_common_configurations_arb_cfg, hightrxq);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_ARB_CFG, reg_common_configurations_arb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_arb_cfg_get(uint8_t bbh_id, bdmf_boolean *hightrxq)
{
    uint32_t reg_common_configurations_arb_cfg;

#ifdef VALIDATE_PARMS
    if(!hightrxq)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_ARB_CFG, reg_common_configurations_arb_cfg);

    *hightrxq = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_ARB_CFG, HIGHTRXQ, reg_common_configurations_arb_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_bbroute_set(uint8_t bbh_id, uint16_t route, uint8_t dest, bdmf_boolean en)
{
    uint32_t reg_common_configurations_bbroute=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (route >= _10BITS_MAX_VAL_) ||
       (dest >= _6BITS_MAX_VAL_) ||
       (en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_bbroute = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBROUTE, ROUTE, reg_common_configurations_bbroute, route);
    reg_common_configurations_bbroute = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBROUTE, DEST, reg_common_configurations_bbroute, dest);
    reg_common_configurations_bbroute = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBROUTE, EN, reg_common_configurations_bbroute, en);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBROUTE, reg_common_configurations_bbroute);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_bbroute_get(uint8_t bbh_id, uint16_t *route, uint8_t *dest, bdmf_boolean *en)
{
    uint32_t reg_common_configurations_bbroute;

#ifdef VALIDATE_PARMS
    if(!route || !dest || !en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBROUTE, reg_common_configurations_bbroute);

    *route = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBROUTE, ROUTE, reg_common_configurations_bbroute);
    *dest = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBROUTE, DEST, reg_common_configurations_bbroute);
    *en = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_BBROUTE, EN, reg_common_configurations_bbroute);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_q2rnr_set(uint8_t bbh_id, uint8_t q_2_rnr_index, bdmf_boolean q0, bdmf_boolean q1)
{
    uint32_t reg_common_configurations_q2rnr=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (q_2_rnr_index >= 20) ||
       (q0 >= _1BITS_MAX_VAL_) ||
       (q1 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_q2rnr = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_Q2RNR, Q0, reg_common_configurations_q2rnr, q0);
    reg_common_configurations_q2rnr = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_Q2RNR, Q1, reg_common_configurations_q2rnr, q1);

    RU_REG_RAM_WRITE(bbh_id, q_2_rnr_index, BBH_TX, COMMON_CONFIGURATIONS_Q2RNR, reg_common_configurations_q2rnr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_q2rnr_get(uint8_t bbh_id, uint8_t q_2_rnr_index, bdmf_boolean *q0, bdmf_boolean *q1)
{
    uint32_t reg_common_configurations_q2rnr;

#ifdef VALIDATE_PARMS
    if(!q0 || !q1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (q_2_rnr_index >= 20))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, q_2_rnr_index, BBH_TX, COMMON_CONFIGURATIONS_Q2RNR, reg_common_configurations_q2rnr);

    *q0 = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_Q2RNR, Q0, reg_common_configurations_q2rnr);
    *q1 = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_Q2RNR, Q1, reg_common_configurations_q2rnr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_perqtask_set(uint8_t bbh_id, const bbh_tx_common_configurations_perqtask *common_configurations_perqtask)
{
    uint32_t reg_common_configurations_perqtask=0;

#ifdef VALIDATE_PARMS
    if(!common_configurations_perqtask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (common_configurations_perqtask->task0 >= _4BITS_MAX_VAL_) ||
       (common_configurations_perqtask->task1 >= _4BITS_MAX_VAL_) ||
       (common_configurations_perqtask->task2 >= _4BITS_MAX_VAL_) ||
       (common_configurations_perqtask->task3 >= _4BITS_MAX_VAL_) ||
       (common_configurations_perqtask->task4 >= _4BITS_MAX_VAL_) ||
       (common_configurations_perqtask->task5 >= _4BITS_MAX_VAL_) ||
       (common_configurations_perqtask->task6 >= _4BITS_MAX_VAL_) ||
       (common_configurations_perqtask->task7 >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_perqtask = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, TASK0, reg_common_configurations_perqtask, common_configurations_perqtask->task0);
    reg_common_configurations_perqtask = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, TASK1, reg_common_configurations_perqtask, common_configurations_perqtask->task1);
    reg_common_configurations_perqtask = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, TASK2, reg_common_configurations_perqtask, common_configurations_perqtask->task2);
    reg_common_configurations_perqtask = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, TASK3, reg_common_configurations_perqtask, common_configurations_perqtask->task3);
    reg_common_configurations_perqtask = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, TASK4, reg_common_configurations_perqtask, common_configurations_perqtask->task4);
    reg_common_configurations_perqtask = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, TASK5, reg_common_configurations_perqtask, common_configurations_perqtask->task5);
    reg_common_configurations_perqtask = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, TASK6, reg_common_configurations_perqtask, common_configurations_perqtask->task6);
    reg_common_configurations_perqtask = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, TASK7, reg_common_configurations_perqtask, common_configurations_perqtask->task7);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, reg_common_configurations_perqtask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_perqtask_get(uint8_t bbh_id, bbh_tx_common_configurations_perqtask *common_configurations_perqtask)
{
    uint32_t reg_common_configurations_perqtask;

#ifdef VALIDATE_PARMS
    if(!common_configurations_perqtask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, reg_common_configurations_perqtask);

    common_configurations_perqtask->task0 = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, TASK0, reg_common_configurations_perqtask);
    common_configurations_perqtask->task1 = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, TASK1, reg_common_configurations_perqtask);
    common_configurations_perqtask->task2 = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, TASK2, reg_common_configurations_perqtask);
    common_configurations_perqtask->task3 = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, TASK3, reg_common_configurations_perqtask);
    common_configurations_perqtask->task4 = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, TASK4, reg_common_configurations_perqtask);
    common_configurations_perqtask->task5 = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, TASK5, reg_common_configurations_perqtask);
    common_configurations_perqtask->task6 = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, TASK6, reg_common_configurations_perqtask);
    common_configurations_perqtask->task7 = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_PERQTASK, TASK7, reg_common_configurations_perqtask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_txrstcmd_set(uint8_t bbh_id, const bbh_tx_common_configurations_txrstcmd *common_configurations_txrstcmd)
{
    uint32_t reg_common_configurations_txrstcmd=0;

#ifdef VALIDATE_PARMS
    if(!common_configurations_txrstcmd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (common_configurations_txrstcmd->cntxtrst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->pdfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->dmaptrrst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->sdmaptrrst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->bpmfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->sbpmfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->okfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->ddrfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->sramfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->skbptrrst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->stsfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->reqfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->msgfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->gnxtfiforst >= _1BITS_MAX_VAL_) ||
       (common_configurations_txrstcmd->fbnfiforst >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, CNTXTRST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->cntxtrst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, PDFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->pdfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, DMAPTRRST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->dmaptrrst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, SDMAPTRRST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->sdmaptrrst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, BPMFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->bpmfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, SBPMFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->sbpmfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, OKFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->okfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, DDRFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->ddrfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, SRAMFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->sramfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, SKBPTRRST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->skbptrrst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, STSFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->stsfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, REQFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->reqfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, MSGFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->msgfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, GNXTFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->gnxtfiforst);
    reg_common_configurations_txrstcmd = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, FBNFIFORST, reg_common_configurations_txrstcmd, common_configurations_txrstcmd->fbnfiforst);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, reg_common_configurations_txrstcmd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_txrstcmd_get(uint8_t bbh_id, bbh_tx_common_configurations_txrstcmd *common_configurations_txrstcmd)
{
    uint32_t reg_common_configurations_txrstcmd;

#ifdef VALIDATE_PARMS
    if(!common_configurations_txrstcmd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, reg_common_configurations_txrstcmd);

    common_configurations_txrstcmd->cntxtrst = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, CNTXTRST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->pdfiforst = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, PDFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->dmaptrrst = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, DMAPTRRST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->sdmaptrrst = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, SDMAPTRRST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->bpmfiforst = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, BPMFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->sbpmfiforst = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, SBPMFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->okfiforst = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, OKFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->ddrfiforst = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, DDRFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->sramfiforst = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, SRAMFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->skbptrrst = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, SKBPTRRST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->stsfiforst = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, STSFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->reqfiforst = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, REQFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->msgfiforst = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, MSGFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->gnxtfiforst = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, GNXTFIFORST, reg_common_configurations_txrstcmd);
    common_configurations_txrstcmd->fbnfiforst = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD, FBNFIFORST, reg_common_configurations_txrstcmd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_dbgsel_set(uint8_t bbh_id, uint8_t dbgsel)
{
    uint32_t reg_common_configurations_dbgsel=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (dbgsel >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_dbgsel = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DBGSEL, DBGSEL, reg_common_configurations_dbgsel, dbgsel);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DBGSEL, reg_common_configurations_dbgsel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_dbgsel_get(uint8_t bbh_id, uint8_t *dbgsel)
{
    uint32_t reg_common_configurations_dbgsel;

#ifdef VALIDATE_PARMS
    if(!dbgsel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DBGSEL, reg_common_configurations_dbgsel);

    *dbgsel = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_DBGSEL, DBGSEL, reg_common_configurations_dbgsel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_clk_gate_cntrl_set(uint8_t bbh_id, const bbh_tx_common_configurations_clk_gate_cntrl *common_configurations_clk_gate_cntrl)
{
    uint32_t reg_common_configurations_clk_gate_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!common_configurations_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (common_configurations_clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
       (common_configurations_clk_gate_cntrl->keep_alive_en >= _1BITS_MAX_VAL_) ||
       (common_configurations_clk_gate_cntrl->keep_alive_intrvl >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_common_configurations_clk_gate_cntrl = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_common_configurations_clk_gate_cntrl, common_configurations_clk_gate_cntrl->bypass_clk_gate);
    reg_common_configurations_clk_gate_cntrl = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, TIMER_VAL, reg_common_configurations_clk_gate_cntrl, common_configurations_clk_gate_cntrl->timer_val);
    reg_common_configurations_clk_gate_cntrl = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_common_configurations_clk_gate_cntrl, common_configurations_clk_gate_cntrl->keep_alive_en);
    reg_common_configurations_clk_gate_cntrl = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_common_configurations_clk_gate_cntrl, common_configurations_clk_gate_cntrl->keep_alive_intrvl);
    reg_common_configurations_clk_gate_cntrl = RU_FIELD_SET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_common_configurations_clk_gate_cntrl, common_configurations_clk_gate_cntrl->keep_alive_cyc);

    RU_REG_WRITE(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, reg_common_configurations_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_common_configurations_clk_gate_cntrl_get(uint8_t bbh_id, bbh_tx_common_configurations_clk_gate_cntrl *common_configurations_clk_gate_cntrl)
{
    uint32_t reg_common_configurations_clk_gate_cntrl;

#ifdef VALIDATE_PARMS
    if(!common_configurations_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, reg_common_configurations_clk_gate_cntrl);

    common_configurations_clk_gate_cntrl->bypass_clk_gate = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_common_configurations_clk_gate_cntrl);
    common_configurations_clk_gate_cntrl->timer_val = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, TIMER_VAL, reg_common_configurations_clk_gate_cntrl);
    common_configurations_clk_gate_cntrl->keep_alive_en = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_common_configurations_clk_gate_cntrl);
    common_configurations_clk_gate_cntrl->keep_alive_intrvl = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_common_configurations_clk_gate_cntrl);
    common_configurations_clk_gate_cntrl->keep_alive_cyc = RU_FIELD_GET(bbh_id, BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_common_configurations_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdbase_set(uint8_t bbh_id, uint8_t wan_pd_base_index, uint16_t fifobase0, uint16_t fifobase1)
{
    uint32_t reg_wan_configurations_pdbase=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wan_pd_base_index >= 20) ||
       (fifobase0 >= _9BITS_MAX_VAL_) ||
       (fifobase1 >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wan_configurations_pdbase = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PDBASE, FIFOBASE0, reg_wan_configurations_pdbase, fifobase0);
    reg_wan_configurations_pdbase = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PDBASE, FIFOBASE1, reg_wan_configurations_pdbase, fifobase1);

    RU_REG_RAM_WRITE(bbh_id, wan_pd_base_index, BBH_TX, WAN_CONFIGURATIONS_PDBASE, reg_wan_configurations_pdbase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdbase_get(uint8_t bbh_id, uint8_t wan_pd_base_index, uint16_t *fifobase0, uint16_t *fifobase1)
{
    uint32_t reg_wan_configurations_pdbase;

#ifdef VALIDATE_PARMS
    if(!fifobase0 || !fifobase1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wan_pd_base_index >= 20))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, wan_pd_base_index, BBH_TX, WAN_CONFIGURATIONS_PDBASE, reg_wan_configurations_pdbase);

    *fifobase0 = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PDBASE, FIFOBASE0, reg_wan_configurations_pdbase);
    *fifobase1 = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PDBASE, FIFOBASE1, reg_wan_configurations_pdbase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdsize_set(uint8_t bbh_id, uint8_t wan_pd_size_index, uint16_t fifosize0, uint16_t fifosize1)
{
    uint32_t reg_wan_configurations_pdsize=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wan_pd_size_index >= 20) ||
       (fifosize0 >= _9BITS_MAX_VAL_) ||
       (fifosize1 >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wan_configurations_pdsize = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PDSIZE, FIFOSIZE0, reg_wan_configurations_pdsize, fifosize0);
    reg_wan_configurations_pdsize = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PDSIZE, FIFOSIZE1, reg_wan_configurations_pdsize, fifosize1);

    RU_REG_RAM_WRITE(bbh_id, wan_pd_size_index, BBH_TX, WAN_CONFIGURATIONS_PDSIZE, reg_wan_configurations_pdsize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdsize_get(uint8_t bbh_id, uint8_t wan_pd_size_index, uint16_t *fifosize0, uint16_t *fifosize1)
{
    uint32_t reg_wan_configurations_pdsize;

#ifdef VALIDATE_PARMS
    if(!fifosize0 || !fifosize1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wan_pd_size_index >= 20))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, wan_pd_size_index, BBH_TX, WAN_CONFIGURATIONS_PDSIZE, reg_wan_configurations_pdsize);

    *fifosize0 = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PDSIZE, FIFOSIZE0, reg_wan_configurations_pdsize);
    *fifosize1 = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PDSIZE, FIFOSIZE1, reg_wan_configurations_pdsize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdwkuph_set(uint8_t bbh_id, uint8_t wan_pd_wkup_index, uint8_t wkupthresh0, uint8_t wkupthresh1)
{
    uint32_t reg_wan_configurations_pdwkuph=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wan_pd_wkup_index >= 20))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wan_configurations_pdwkuph = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PDWKUPH, WKUPTHRESH0, reg_wan_configurations_pdwkuph, wkupthresh0);
    reg_wan_configurations_pdwkuph = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PDWKUPH, WKUPTHRESH1, reg_wan_configurations_pdwkuph, wkupthresh1);

    RU_REG_RAM_WRITE(bbh_id, wan_pd_wkup_index, BBH_TX, WAN_CONFIGURATIONS_PDWKUPH, reg_wan_configurations_pdwkuph);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdwkuph_get(uint8_t bbh_id, uint8_t wan_pd_wkup_index, uint8_t *wkupthresh0, uint8_t *wkupthresh1)
{
    uint32_t reg_wan_configurations_pdwkuph;

#ifdef VALIDATE_PARMS
    if(!wkupthresh0 || !wkupthresh1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wan_pd_wkup_index >= 20))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, wan_pd_wkup_index, BBH_TX, WAN_CONFIGURATIONS_PDWKUPH, reg_wan_configurations_pdwkuph);

    *wkupthresh0 = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PDWKUPH, WKUPTHRESH0, reg_wan_configurations_pdwkuph);
    *wkupthresh1 = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PDWKUPH, WKUPTHRESH1, reg_wan_configurations_pdwkuph);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_pd_byte_th_set(uint8_t bbh_id, uint8_t wan_pd_byte_th_index, uint16_t pdlimit0, uint16_t pdlimit1)
{
    uint32_t reg_wan_configurations_pd_byte_th=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wan_pd_byte_th_index >= 20))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wan_configurations_pd_byte_th = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PD_BYTE_TH, PDLIMIT0, reg_wan_configurations_pd_byte_th, pdlimit0);
    reg_wan_configurations_pd_byte_th = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PD_BYTE_TH, PDLIMIT1, reg_wan_configurations_pd_byte_th, pdlimit1);

    RU_REG_RAM_WRITE(bbh_id, wan_pd_byte_th_index, BBH_TX, WAN_CONFIGURATIONS_PD_BYTE_TH, reg_wan_configurations_pd_byte_th);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_pd_byte_th_get(uint8_t bbh_id, uint8_t wan_pd_byte_th_index, uint16_t *pdlimit0, uint16_t *pdlimit1)
{
    uint32_t reg_wan_configurations_pd_byte_th;

#ifdef VALIDATE_PARMS
    if(!pdlimit0 || !pdlimit1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wan_pd_byte_th_index >= 20))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, wan_pd_byte_th_index, BBH_TX, WAN_CONFIGURATIONS_PD_BYTE_TH, reg_wan_configurations_pd_byte_th);

    *pdlimit0 = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PD_BYTE_TH, PDLIMIT0, reg_wan_configurations_pd_byte_th);
    *pdlimit1 = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PD_BYTE_TH, PDLIMIT1, reg_wan_configurations_pd_byte_th);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_pd_byte_th_en_set(uint8_t bbh_id, bdmf_boolean pdlimiten)
{
    uint32_t reg_wan_configurations_pd_byte_th_en=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (pdlimiten >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wan_configurations_pd_byte_th_en = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PD_BYTE_TH_EN, PDLIMITEN, reg_wan_configurations_pd_byte_th_en, pdlimiten);

    RU_REG_WRITE(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PD_BYTE_TH_EN, reg_wan_configurations_pd_byte_th_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_pd_byte_th_en_get(uint8_t bbh_id, bdmf_boolean *pdlimiten)
{
    uint32_t reg_wan_configurations_pd_byte_th_en;

#ifdef VALIDATE_PARMS
    if(!pdlimiten)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PD_BYTE_TH_EN, reg_wan_configurations_pd_byte_th_en);

    *pdlimiten = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PD_BYTE_TH_EN, PDLIMITEN, reg_wan_configurations_pd_byte_th_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdempty_set(uint8_t bbh_id, uint8_t empty)
{
    uint32_t reg_wan_configurations_pdempty=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wan_configurations_pdempty = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PDEMPTY, EMPTY, reg_wan_configurations_pdempty, empty);

    RU_REG_WRITE(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PDEMPTY, reg_wan_configurations_pdempty);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdempty_get(uint8_t bbh_id, uint8_t *empty)
{
    uint32_t reg_wan_configurations_pdempty;

#ifdef VALIDATE_PARMS
    if(!empty)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PDEMPTY, reg_wan_configurations_pdempty);

    *empty = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_PDEMPTY, EMPTY, reg_wan_configurations_pdempty);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_stsrnrcfg_1_set(uint8_t bbh_id, uint8_t wan_sts_rnr_cfg_1_index, uint16_t tcontaddr)
{
    uint32_t reg_wan_configurations_stsrnrcfg_1=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wan_sts_rnr_cfg_1_index >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wan_configurations_stsrnrcfg_1 = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_STSRNRCFG_1, TCONTADDR, reg_wan_configurations_stsrnrcfg_1, tcontaddr);

    RU_REG_RAM_WRITE(bbh_id, wan_sts_rnr_cfg_1_index, BBH_TX, WAN_CONFIGURATIONS_STSRNRCFG_1, reg_wan_configurations_stsrnrcfg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_stsrnrcfg_1_get(uint8_t bbh_id, uint8_t wan_sts_rnr_cfg_1_index, uint16_t *tcontaddr)
{
    uint32_t reg_wan_configurations_stsrnrcfg_1;

#ifdef VALIDATE_PARMS
    if(!tcontaddr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wan_sts_rnr_cfg_1_index >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, wan_sts_rnr_cfg_1_index, BBH_TX, WAN_CONFIGURATIONS_STSRNRCFG_1, reg_wan_configurations_stsrnrcfg_1);

    *tcontaddr = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_STSRNRCFG_1, TCONTADDR, reg_wan_configurations_stsrnrcfg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_stsrnrcfg_2_set(uint8_t bbh_id, uint8_t wan_sts_rnr_cfg_2_index, uint16_t ptraddr, uint8_t task)
{
    uint32_t reg_wan_configurations_stsrnrcfg_2=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wan_sts_rnr_cfg_2_index >= 2) ||
       (task >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wan_configurations_stsrnrcfg_2 = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_STSRNRCFG_2, PTRADDR, reg_wan_configurations_stsrnrcfg_2, ptraddr);
    reg_wan_configurations_stsrnrcfg_2 = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_STSRNRCFG_2, TASK, reg_wan_configurations_stsrnrcfg_2, task);

    RU_REG_RAM_WRITE(bbh_id, wan_sts_rnr_cfg_2_index, BBH_TX, WAN_CONFIGURATIONS_STSRNRCFG_2, reg_wan_configurations_stsrnrcfg_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_stsrnrcfg_2_get(uint8_t bbh_id, uint8_t wan_sts_rnr_cfg_2_index, uint16_t *ptraddr, uint8_t *task)
{
    uint32_t reg_wan_configurations_stsrnrcfg_2;

#ifdef VALIDATE_PARMS
    if(!ptraddr || !task)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wan_sts_rnr_cfg_2_index >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, wan_sts_rnr_cfg_2_index, BBH_TX, WAN_CONFIGURATIONS_STSRNRCFG_2, reg_wan_configurations_stsrnrcfg_2);

    *ptraddr = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_STSRNRCFG_2, PTRADDR, reg_wan_configurations_stsrnrcfg_2);
    *task = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_STSRNRCFG_2, TASK, reg_wan_configurations_stsrnrcfg_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_msgrnrcfg_1_set(uint8_t bbh_id, uint8_t wan_msg_rnr_cfg_1_index, uint16_t tcontaddr)
{
    uint32_t reg_wan_configurations_msgrnrcfg_1=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wan_msg_rnr_cfg_1_index >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wan_configurations_msgrnrcfg_1 = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_MSGRNRCFG_1, TCONTADDR, reg_wan_configurations_msgrnrcfg_1, tcontaddr);

    RU_REG_RAM_WRITE(bbh_id, wan_msg_rnr_cfg_1_index, BBH_TX, WAN_CONFIGURATIONS_MSGRNRCFG_1, reg_wan_configurations_msgrnrcfg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_msgrnrcfg_1_get(uint8_t bbh_id, uint8_t wan_msg_rnr_cfg_1_index, uint16_t *tcontaddr)
{
    uint32_t reg_wan_configurations_msgrnrcfg_1;

#ifdef VALIDATE_PARMS
    if(!tcontaddr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wan_msg_rnr_cfg_1_index >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, wan_msg_rnr_cfg_1_index, BBH_TX, WAN_CONFIGURATIONS_MSGRNRCFG_1, reg_wan_configurations_msgrnrcfg_1);

    *tcontaddr = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_MSGRNRCFG_1, TCONTADDR, reg_wan_configurations_msgrnrcfg_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_msgrnrcfg_2_set(uint8_t bbh_id, uint8_t wan_msg_rnr_cfg_2_index, uint16_t ptraddr, uint8_t task)
{
    uint32_t reg_wan_configurations_msgrnrcfg_2=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wan_msg_rnr_cfg_2_index >= 2) ||
       (task >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wan_configurations_msgrnrcfg_2 = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_MSGRNRCFG_2, PTRADDR, reg_wan_configurations_msgrnrcfg_2, ptraddr);
    reg_wan_configurations_msgrnrcfg_2 = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_MSGRNRCFG_2, TASK, reg_wan_configurations_msgrnrcfg_2, task);

    RU_REG_RAM_WRITE(bbh_id, wan_msg_rnr_cfg_2_index, BBH_TX, WAN_CONFIGURATIONS_MSGRNRCFG_2, reg_wan_configurations_msgrnrcfg_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_msgrnrcfg_2_get(uint8_t bbh_id, uint8_t wan_msg_rnr_cfg_2_index, uint16_t *ptraddr, uint8_t *task)
{
    uint32_t reg_wan_configurations_msgrnrcfg_2;

#ifdef VALIDATE_PARMS
    if(!ptraddr || !task)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wan_msg_rnr_cfg_2_index >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, wan_msg_rnr_cfg_2_index, BBH_TX, WAN_CONFIGURATIONS_MSGRNRCFG_2, reg_wan_configurations_msgrnrcfg_2);

    *ptraddr = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_MSGRNRCFG_2, PTRADDR, reg_wan_configurations_msgrnrcfg_2);
    *task = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_MSGRNRCFG_2, TASK, reg_wan_configurations_msgrnrcfg_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_epncfg_set(uint8_t bbh_id, bdmf_boolean stplenerr, bdmf_boolean cmp_width, bdmf_boolean considerfull, bdmf_boolean addcrc)
{
    uint32_t reg_wan_configurations_epncfg=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (stplenerr >= _1BITS_MAX_VAL_) ||
       (cmp_width >= _1BITS_MAX_VAL_) ||
       (considerfull >= _1BITS_MAX_VAL_) ||
       (addcrc >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wan_configurations_epncfg = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_EPNCFG, STPLENERR, reg_wan_configurations_epncfg, stplenerr);
    reg_wan_configurations_epncfg = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_EPNCFG, CMP_WIDTH, reg_wan_configurations_epncfg, cmp_width);
    reg_wan_configurations_epncfg = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_EPNCFG, CONSIDERFULL, reg_wan_configurations_epncfg, considerfull);
    reg_wan_configurations_epncfg = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_EPNCFG, ADDCRC, reg_wan_configurations_epncfg, addcrc);

    RU_REG_WRITE(bbh_id, BBH_TX, WAN_CONFIGURATIONS_EPNCFG, reg_wan_configurations_epncfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_epncfg_get(uint8_t bbh_id, bdmf_boolean *stplenerr, bdmf_boolean *cmp_width, bdmf_boolean *considerfull, bdmf_boolean *addcrc)
{
    uint32_t reg_wan_configurations_epncfg;

#ifdef VALIDATE_PARMS
    if(!stplenerr || !cmp_width || !considerfull || !addcrc)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, WAN_CONFIGURATIONS_EPNCFG, reg_wan_configurations_epncfg);

    *stplenerr = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_EPNCFG, STPLENERR, reg_wan_configurations_epncfg);
    *cmp_width = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_EPNCFG, CMP_WIDTH, reg_wan_configurations_epncfg);
    *considerfull = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_EPNCFG, CONSIDERFULL, reg_wan_configurations_epncfg);
    *addcrc = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_EPNCFG, ADDCRC, reg_wan_configurations_epncfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_flow2port_set(uint8_t bbh_id, uint32_t wdata, uint8_t a, bdmf_boolean cmd)
{
    uint32_t reg_wan_configurations_flow2port=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (wdata >= _18BITS_MAX_VAL_) ||
       (cmd >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wan_configurations_flow2port = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_FLOW2PORT, WDATA, reg_wan_configurations_flow2port, wdata);
    reg_wan_configurations_flow2port = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_FLOW2PORT, A, reg_wan_configurations_flow2port, a);
    reg_wan_configurations_flow2port = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_FLOW2PORT, CMD, reg_wan_configurations_flow2port, cmd);

    RU_REG_WRITE(bbh_id, BBH_TX, WAN_CONFIGURATIONS_FLOW2PORT, reg_wan_configurations_flow2port);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_flow2port_get(uint8_t bbh_id, uint32_t *wdata, uint8_t *a, bdmf_boolean *cmd)
{
    uint32_t reg_wan_configurations_flow2port;

#ifdef VALIDATE_PARMS
    if(!wdata || !a || !cmd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, WAN_CONFIGURATIONS_FLOW2PORT, reg_wan_configurations_flow2port);

    *wdata = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_FLOW2PORT, WDATA, reg_wan_configurations_flow2port);
    *a = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_FLOW2PORT, A, reg_wan_configurations_flow2port);
    *cmd = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_FLOW2PORT, CMD, reg_wan_configurations_flow2port);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_ts_set(uint8_t bbh_id, bdmf_boolean en)
{
    uint32_t reg_wan_configurations_ts=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wan_configurations_ts = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_TS, EN, reg_wan_configurations_ts, en);

    RU_REG_WRITE(bbh_id, BBH_TX, WAN_CONFIGURATIONS_TS, reg_wan_configurations_ts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_ts_get(uint8_t bbh_id, bdmf_boolean *en)
{
    uint32_t reg_wan_configurations_ts;

#ifdef VALIDATE_PARMS
    if(!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, WAN_CONFIGURATIONS_TS, reg_wan_configurations_ts);

    *en = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_TS, EN, reg_wan_configurations_ts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_maxwlen_set(uint8_t bbh_id, uint16_t maxwlen)
{
    uint32_t reg_wan_configurations_maxwlen=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wan_configurations_maxwlen = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_MAXWLEN, MAXWLEN, reg_wan_configurations_maxwlen, maxwlen);

    RU_REG_WRITE(bbh_id, BBH_TX, WAN_CONFIGURATIONS_MAXWLEN, reg_wan_configurations_maxwlen);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_maxwlen_get(uint8_t bbh_id, uint16_t *maxwlen)
{
    uint32_t reg_wan_configurations_maxwlen;

#ifdef VALIDATE_PARMS
    if(!maxwlen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, WAN_CONFIGURATIONS_MAXWLEN, reg_wan_configurations_maxwlen);

    *maxwlen = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_MAXWLEN, MAXWLEN, reg_wan_configurations_maxwlen);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_flush_set(uint8_t bbh_id, uint16_t flush, bdmf_boolean srst_n)
{
    uint32_t reg_wan_configurations_flush=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (srst_n >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_wan_configurations_flush = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_FLUSH, FLUSH, reg_wan_configurations_flush, flush);
    reg_wan_configurations_flush = RU_FIELD_SET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_FLUSH, SRST_N, reg_wan_configurations_flush, srst_n);

    RU_REG_WRITE(bbh_id, BBH_TX, WAN_CONFIGURATIONS_FLUSH, reg_wan_configurations_flush);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_wan_configurations_flush_get(uint8_t bbh_id, uint16_t *flush, bdmf_boolean *srst_n)
{
    uint32_t reg_wan_configurations_flush;

#ifdef VALIDATE_PARMS
    if(!flush || !srst_n)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, WAN_CONFIGURATIONS_FLUSH, reg_wan_configurations_flush);

    *flush = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_FLUSH, FLUSH, reg_wan_configurations_flush);
    *srst_n = RU_FIELD_GET(bbh_id, BBH_TX, WAN_CONFIGURATIONS_FLUSH, SRST_N, reg_wan_configurations_flush);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_pdbase_set(uint8_t bbh_id, uint16_t fifobase0, uint16_t fifobase1)
{
    uint32_t reg_lan_configurations_pdbase=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (fifobase0 >= _9BITS_MAX_VAL_) ||
       (fifobase1 >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lan_configurations_pdbase = RU_FIELD_SET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDBASE, FIFOBASE0, reg_lan_configurations_pdbase, fifobase0);
    reg_lan_configurations_pdbase = RU_FIELD_SET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDBASE, FIFOBASE1, reg_lan_configurations_pdbase, fifobase1);

    RU_REG_WRITE(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDBASE, reg_lan_configurations_pdbase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_pdbase_get(uint8_t bbh_id, uint16_t *fifobase0, uint16_t *fifobase1)
{
    uint32_t reg_lan_configurations_pdbase;

#ifdef VALIDATE_PARMS
    if(!fifobase0 || !fifobase1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDBASE, reg_lan_configurations_pdbase);

    *fifobase0 = RU_FIELD_GET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDBASE, FIFOBASE0, reg_lan_configurations_pdbase);
    *fifobase1 = RU_FIELD_GET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDBASE, FIFOBASE1, reg_lan_configurations_pdbase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_pdsize_set(uint8_t bbh_id, uint16_t fifosize0, uint16_t fifosize1)
{
    uint32_t reg_lan_configurations_pdsize=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (fifosize0 >= _9BITS_MAX_VAL_) ||
       (fifosize1 >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lan_configurations_pdsize = RU_FIELD_SET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDSIZE, FIFOSIZE0, reg_lan_configurations_pdsize, fifosize0);
    reg_lan_configurations_pdsize = RU_FIELD_SET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDSIZE, FIFOSIZE1, reg_lan_configurations_pdsize, fifosize1);

    RU_REG_WRITE(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDSIZE, reg_lan_configurations_pdsize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_pdsize_get(uint8_t bbh_id, uint16_t *fifosize0, uint16_t *fifosize1)
{
    uint32_t reg_lan_configurations_pdsize;

#ifdef VALIDATE_PARMS
    if(!fifosize0 || !fifosize1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDSIZE, reg_lan_configurations_pdsize);

    *fifosize0 = RU_FIELD_GET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDSIZE, FIFOSIZE0, reg_lan_configurations_pdsize);
    *fifosize1 = RU_FIELD_GET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDSIZE, FIFOSIZE1, reg_lan_configurations_pdsize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_pdwkuph_set(uint8_t bbh_id, uint8_t wkupthresh0, uint8_t wkupthresh1)
{
    uint32_t reg_lan_configurations_pdwkuph=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lan_configurations_pdwkuph = RU_FIELD_SET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDWKUPH, WKUPTHRESH0, reg_lan_configurations_pdwkuph, wkupthresh0);
    reg_lan_configurations_pdwkuph = RU_FIELD_SET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDWKUPH, WKUPTHRESH1, reg_lan_configurations_pdwkuph, wkupthresh1);

    RU_REG_WRITE(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDWKUPH, reg_lan_configurations_pdwkuph);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_pdwkuph_get(uint8_t bbh_id, uint8_t *wkupthresh0, uint8_t *wkupthresh1)
{
    uint32_t reg_lan_configurations_pdwkuph;

#ifdef VALIDATE_PARMS
    if(!wkupthresh0 || !wkupthresh1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDWKUPH, reg_lan_configurations_pdwkuph);

    *wkupthresh0 = RU_FIELD_GET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDWKUPH, WKUPTHRESH0, reg_lan_configurations_pdwkuph);
    *wkupthresh1 = RU_FIELD_GET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDWKUPH, WKUPTHRESH1, reg_lan_configurations_pdwkuph);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_pd_byte_th_set(uint8_t bbh_id, uint16_t pdlimit0, uint16_t pdlimit1)
{
    uint32_t reg_lan_configurations_pd_byte_th=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lan_configurations_pd_byte_th = RU_FIELD_SET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PD_BYTE_TH, PDLIMIT0, reg_lan_configurations_pd_byte_th, pdlimit0);
    reg_lan_configurations_pd_byte_th = RU_FIELD_SET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PD_BYTE_TH, PDLIMIT1, reg_lan_configurations_pd_byte_th, pdlimit1);

    RU_REG_WRITE(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PD_BYTE_TH, reg_lan_configurations_pd_byte_th);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_pd_byte_th_get(uint8_t bbh_id, uint16_t *pdlimit0, uint16_t *pdlimit1)
{
    uint32_t reg_lan_configurations_pd_byte_th;

#ifdef VALIDATE_PARMS
    if(!pdlimit0 || !pdlimit1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PD_BYTE_TH, reg_lan_configurations_pd_byte_th);

    *pdlimit0 = RU_FIELD_GET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PD_BYTE_TH, PDLIMIT0, reg_lan_configurations_pd_byte_th);
    *pdlimit1 = RU_FIELD_GET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PD_BYTE_TH, PDLIMIT1, reg_lan_configurations_pd_byte_th);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_pd_byte_th_en_set(uint8_t bbh_id, bdmf_boolean pdlimiten)
{
    uint32_t reg_lan_configurations_pd_byte_th_en=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (pdlimiten >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lan_configurations_pd_byte_th_en = RU_FIELD_SET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PD_BYTE_TH_EN, PDLIMITEN, reg_lan_configurations_pd_byte_th_en, pdlimiten);

    RU_REG_WRITE(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PD_BYTE_TH_EN, reg_lan_configurations_pd_byte_th_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_pd_byte_th_en_get(uint8_t bbh_id, bdmf_boolean *pdlimiten)
{
    uint32_t reg_lan_configurations_pd_byte_th_en;

#ifdef VALIDATE_PARMS
    if(!pdlimiten)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PD_BYTE_TH_EN, reg_lan_configurations_pd_byte_th_en);

    *pdlimiten = RU_FIELD_GET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PD_BYTE_TH_EN, PDLIMITEN, reg_lan_configurations_pd_byte_th_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_pdempty_set(uint8_t bbh_id, uint8_t empty)
{
    uint32_t reg_lan_configurations_pdempty=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lan_configurations_pdempty = RU_FIELD_SET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDEMPTY, EMPTY, reg_lan_configurations_pdempty, empty);

    RU_REG_WRITE(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDEMPTY, reg_lan_configurations_pdempty);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_pdempty_get(uint8_t bbh_id, uint8_t *empty)
{
    uint32_t reg_lan_configurations_pdempty;

#ifdef VALIDATE_PARMS
    if(!empty)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDEMPTY, reg_lan_configurations_pdempty);

    *empty = RU_FIELD_GET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_PDEMPTY, EMPTY, reg_lan_configurations_pdempty);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_txthresh_set(uint8_t bbh_id, uint16_t ddrthresh, uint16_t sramthresh)
{
    uint32_t reg_lan_configurations_txthresh=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (ddrthresh >= _9BITS_MAX_VAL_) ||
       (sramthresh >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lan_configurations_txthresh = RU_FIELD_SET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_TXTHRESH, DDRTHRESH, reg_lan_configurations_txthresh, ddrthresh);
    reg_lan_configurations_txthresh = RU_FIELD_SET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_TXTHRESH, SRAMTHRESH, reg_lan_configurations_txthresh, sramthresh);

    RU_REG_WRITE(bbh_id, BBH_TX, LAN_CONFIGURATIONS_TXTHRESH, reg_lan_configurations_txthresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_txthresh_get(uint8_t bbh_id, uint16_t *ddrthresh, uint16_t *sramthresh)
{
    uint32_t reg_lan_configurations_txthresh;

#ifdef VALIDATE_PARMS
    if(!ddrthresh || !sramthresh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, LAN_CONFIGURATIONS_TXTHRESH, reg_lan_configurations_txthresh);

    *ddrthresh = RU_FIELD_GET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_TXTHRESH, DDRTHRESH, reg_lan_configurations_txthresh);
    *sramthresh = RU_FIELD_GET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_TXTHRESH, SRAMTHRESH, reg_lan_configurations_txthresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_eee_set(uint8_t bbh_id, bdmf_boolean en)
{
    uint32_t reg_lan_configurations_eee=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lan_configurations_eee = RU_FIELD_SET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_EEE, EN, reg_lan_configurations_eee, en);

    RU_REG_WRITE(bbh_id, BBH_TX, LAN_CONFIGURATIONS_EEE, reg_lan_configurations_eee);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_eee_get(uint8_t bbh_id, bdmf_boolean *en)
{
    uint32_t reg_lan_configurations_eee;

#ifdef VALIDATE_PARMS
    if(!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, LAN_CONFIGURATIONS_EEE, reg_lan_configurations_eee);

    *en = RU_FIELD_GET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_EEE, EN, reg_lan_configurations_eee);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_ts_set(uint8_t bbh_id, bdmf_boolean en)
{
    uint32_t reg_lan_configurations_ts=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lan_configurations_ts = RU_FIELD_SET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_TS, EN, reg_lan_configurations_ts, en);

    RU_REG_WRITE(bbh_id, BBH_TX, LAN_CONFIGURATIONS_TS, reg_lan_configurations_ts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_lan_configurations_ts_get(uint8_t bbh_id, bdmf_boolean *en)
{
    uint32_t reg_lan_configurations_ts;

#ifdef VALIDATE_PARMS
    if(!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, LAN_CONFIGURATIONS_TS, reg_lan_configurations_ts);

    *en = RU_FIELD_GET(bbh_id, BBH_TX, LAN_CONFIGURATIONS_TS, EN, reg_lan_configurations_ts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdbase_set(uint8_t bbh_id, uint8_t unified_pd_base_index, uint16_t fifobase0, uint16_t fifobase1)
{
    uint32_t reg_unified_configurations_pdbase=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_pd_base_index >= 4) ||
       (fifobase0 >= _9BITS_MAX_VAL_) ||
       (fifobase1 >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unified_configurations_pdbase = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PDBASE, FIFOBASE0, reg_unified_configurations_pdbase, fifobase0);
    reg_unified_configurations_pdbase = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PDBASE, FIFOBASE1, reg_unified_configurations_pdbase, fifobase1);

    RU_REG_RAM_WRITE(bbh_id, unified_pd_base_index, BBH_TX, UNIFIED_CONFIGURATIONS_PDBASE, reg_unified_configurations_pdbase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdbase_get(uint8_t bbh_id, uint8_t unified_pd_base_index, uint16_t *fifobase0, uint16_t *fifobase1)
{
    uint32_t reg_unified_configurations_pdbase;

#ifdef VALIDATE_PARMS
    if(!fifobase0 || !fifobase1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_pd_base_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, unified_pd_base_index, BBH_TX, UNIFIED_CONFIGURATIONS_PDBASE, reg_unified_configurations_pdbase);

    *fifobase0 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PDBASE, FIFOBASE0, reg_unified_configurations_pdbase);
    *fifobase1 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PDBASE, FIFOBASE1, reg_unified_configurations_pdbase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdsize_set(uint8_t bbh_id, uint8_t unified_pd_size_index, uint16_t fifosize0, uint16_t fifosize1)
{
    uint32_t reg_unified_configurations_pdsize=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_pd_size_index >= 4) ||
       (fifosize0 >= _9BITS_MAX_VAL_) ||
       (fifosize1 >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unified_configurations_pdsize = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PDSIZE, FIFOSIZE0, reg_unified_configurations_pdsize, fifosize0);
    reg_unified_configurations_pdsize = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PDSIZE, FIFOSIZE1, reg_unified_configurations_pdsize, fifosize1);

    RU_REG_RAM_WRITE(bbh_id, unified_pd_size_index, BBH_TX, UNIFIED_CONFIGURATIONS_PDSIZE, reg_unified_configurations_pdsize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdsize_get(uint8_t bbh_id, uint8_t unified_pd_size_index, uint16_t *fifosize0, uint16_t *fifosize1)
{
    uint32_t reg_unified_configurations_pdsize;

#ifdef VALIDATE_PARMS
    if(!fifosize0 || !fifosize1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_pd_size_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, unified_pd_size_index, BBH_TX, UNIFIED_CONFIGURATIONS_PDSIZE, reg_unified_configurations_pdsize);

    *fifosize0 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PDSIZE, FIFOSIZE0, reg_unified_configurations_pdsize);
    *fifosize1 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PDSIZE, FIFOSIZE1, reg_unified_configurations_pdsize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdwkuph_set(uint8_t bbh_id, uint8_t unified_pd_wkup_index, uint8_t wkupthresh0, uint8_t wkupthresh1)
{
    uint32_t reg_unified_configurations_pdwkuph=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_pd_wkup_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unified_configurations_pdwkuph = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PDWKUPH, WKUPTHRESH0, reg_unified_configurations_pdwkuph, wkupthresh0);
    reg_unified_configurations_pdwkuph = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PDWKUPH, WKUPTHRESH1, reg_unified_configurations_pdwkuph, wkupthresh1);

    RU_REG_RAM_WRITE(bbh_id, unified_pd_wkup_index, BBH_TX, UNIFIED_CONFIGURATIONS_PDWKUPH, reg_unified_configurations_pdwkuph);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdwkuph_get(uint8_t bbh_id, uint8_t unified_pd_wkup_index, uint8_t *wkupthresh0, uint8_t *wkupthresh1)
{
    uint32_t reg_unified_configurations_pdwkuph;

#ifdef VALIDATE_PARMS
    if(!wkupthresh0 || !wkupthresh1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_pd_wkup_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, unified_pd_wkup_index, BBH_TX, UNIFIED_CONFIGURATIONS_PDWKUPH, reg_unified_configurations_pdwkuph);

    *wkupthresh0 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PDWKUPH, WKUPTHRESH0, reg_unified_configurations_pdwkuph);
    *wkupthresh1 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PDWKUPH, WKUPTHRESH1, reg_unified_configurations_pdwkuph);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_pd_byte_th_set(uint8_t bbh_id, uint8_t unified_pd_byte_th_index, uint16_t pdlimit0, uint16_t pdlimit1)
{
    uint32_t reg_unified_configurations_pd_byte_th=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_pd_byte_th_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unified_configurations_pd_byte_th = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PD_BYTE_TH, PDLIMIT0, reg_unified_configurations_pd_byte_th, pdlimit0);
    reg_unified_configurations_pd_byte_th = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PD_BYTE_TH, PDLIMIT1, reg_unified_configurations_pd_byte_th, pdlimit1);

    RU_REG_RAM_WRITE(bbh_id, unified_pd_byte_th_index, BBH_TX, UNIFIED_CONFIGURATIONS_PD_BYTE_TH, reg_unified_configurations_pd_byte_th);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_pd_byte_th_get(uint8_t bbh_id, uint8_t unified_pd_byte_th_index, uint16_t *pdlimit0, uint16_t *pdlimit1)
{
    uint32_t reg_unified_configurations_pd_byte_th;

#ifdef VALIDATE_PARMS
    if(!pdlimit0 || !pdlimit1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_pd_byte_th_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, unified_pd_byte_th_index, BBH_TX, UNIFIED_CONFIGURATIONS_PD_BYTE_TH, reg_unified_configurations_pd_byte_th);

    *pdlimit0 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PD_BYTE_TH, PDLIMIT0, reg_unified_configurations_pd_byte_th);
    *pdlimit1 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PD_BYTE_TH, PDLIMIT1, reg_unified_configurations_pd_byte_th);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_pd_byte_th_en_set(uint8_t bbh_id, bdmf_boolean pdlimiten)
{
    uint32_t reg_unified_configurations_pd_byte_th_en=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (pdlimiten >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unified_configurations_pd_byte_th_en = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN, PDLIMITEN, reg_unified_configurations_pd_byte_th_en, pdlimiten);

    RU_REG_WRITE(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN, reg_unified_configurations_pd_byte_th_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_pd_byte_th_en_get(uint8_t bbh_id, bdmf_boolean *pdlimiten)
{
    uint32_t reg_unified_configurations_pd_byte_th_en;

#ifdef VALIDATE_PARMS
    if(!pdlimiten)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN, reg_unified_configurations_pd_byte_th_en);

    *pdlimiten = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN, PDLIMITEN, reg_unified_configurations_pd_byte_th_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdempty_set(uint8_t bbh_id, uint8_t empty)
{
    uint32_t reg_unified_configurations_pdempty=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unified_configurations_pdempty = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PDEMPTY, EMPTY, reg_unified_configurations_pdempty, empty);

    RU_REG_WRITE(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PDEMPTY, reg_unified_configurations_pdempty);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdempty_get(uint8_t bbh_id, uint8_t *empty)
{
    uint32_t reg_unified_configurations_pdempty;

#ifdef VALIDATE_PARMS
    if(!empty)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PDEMPTY, reg_unified_configurations_pdempty);

    *empty = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_PDEMPTY, EMPTY, reg_unified_configurations_pdempty);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_gtxthresh_set(uint8_t bbh_id, uint16_t ddrthresh, uint16_t sramthresh)
{
    uint32_t reg_unified_configurations_gtxthresh=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (ddrthresh >= _9BITS_MAX_VAL_) ||
       (sramthresh >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unified_configurations_gtxthresh = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_GTXTHRESH, DDRTHRESH, reg_unified_configurations_gtxthresh, ddrthresh);
    reg_unified_configurations_gtxthresh = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_GTXTHRESH, SRAMTHRESH, reg_unified_configurations_gtxthresh, sramthresh);

    RU_REG_WRITE(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_GTXTHRESH, reg_unified_configurations_gtxthresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_gtxthresh_get(uint8_t bbh_id, uint16_t *ddrthresh, uint16_t *sramthresh)
{
    uint32_t reg_unified_configurations_gtxthresh;

#ifdef VALIDATE_PARMS
    if(!ddrthresh || !sramthresh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_GTXTHRESH, reg_unified_configurations_gtxthresh);

    *ddrthresh = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_GTXTHRESH, DDRTHRESH, reg_unified_configurations_gtxthresh);
    *sramthresh = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_GTXTHRESH, SRAMTHRESH, reg_unified_configurations_gtxthresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_eee_set(uint8_t bbh_id, uint8_t en)
{
    uint32_t reg_unified_configurations_eee=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unified_configurations_eee = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_EEE, EN, reg_unified_configurations_eee, en);

    RU_REG_WRITE(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_EEE, reg_unified_configurations_eee);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_eee_get(uint8_t bbh_id, uint8_t *en)
{
    uint32_t reg_unified_configurations_eee;

#ifdef VALIDATE_PARMS
    if(!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_EEE, reg_unified_configurations_eee);

    *en = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_EEE, EN, reg_unified_configurations_eee);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_ts_set(uint8_t bbh_id, uint8_t en)
{
    uint32_t reg_unified_configurations_ts=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unified_configurations_ts = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_TS, EN, reg_unified_configurations_ts, en);

    RU_REG_WRITE(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_TS, reg_unified_configurations_ts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_ts_get(uint8_t bbh_id, uint8_t *en)
{
    uint32_t reg_unified_configurations_ts;

#ifdef VALIDATE_PARMS
    if(!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_TS, reg_unified_configurations_ts);

    *en = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_TS, EN, reg_unified_configurations_ts);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_febase_set(uint8_t bbh_id, uint8_t unified_fe_base_index, uint16_t fifobase0, uint16_t fifobase1)
{
    uint32_t reg_unified_configurations_febase=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_fe_base_index >= 4) ||
       (fifobase0 >= _11BITS_MAX_VAL_) ||
       (fifobase1 >= _11BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unified_configurations_febase = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_FEBASE, FIFOBASE0, reg_unified_configurations_febase, fifobase0);
    reg_unified_configurations_febase = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_FEBASE, FIFOBASE1, reg_unified_configurations_febase, fifobase1);

    RU_REG_RAM_WRITE(bbh_id, unified_fe_base_index, BBH_TX, UNIFIED_CONFIGURATIONS_FEBASE, reg_unified_configurations_febase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_febase_get(uint8_t bbh_id, uint8_t unified_fe_base_index, uint16_t *fifobase0, uint16_t *fifobase1)
{
    uint32_t reg_unified_configurations_febase;

#ifdef VALIDATE_PARMS
    if(!fifobase0 || !fifobase1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_fe_base_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, unified_fe_base_index, BBH_TX, UNIFIED_CONFIGURATIONS_FEBASE, reg_unified_configurations_febase);

    *fifobase0 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_FEBASE, FIFOBASE0, reg_unified_configurations_febase);
    *fifobase1 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_FEBASE, FIFOBASE1, reg_unified_configurations_febase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_fesize_set(uint8_t bbh_id, uint8_t unified_fe_size_index, uint16_t fifosize0, uint16_t fifosize1)
{
    uint32_t reg_unified_configurations_fesize=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_fe_size_index >= 4) ||
       (fifosize0 >= _11BITS_MAX_VAL_) ||
       (fifosize1 >= _11BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unified_configurations_fesize = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_FESIZE, FIFOSIZE0, reg_unified_configurations_fesize, fifosize0);
    reg_unified_configurations_fesize = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_FESIZE, FIFOSIZE1, reg_unified_configurations_fesize, fifosize1);

    RU_REG_RAM_WRITE(bbh_id, unified_fe_size_index, BBH_TX, UNIFIED_CONFIGURATIONS_FESIZE, reg_unified_configurations_fesize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_fesize_get(uint8_t bbh_id, uint8_t unified_fe_size_index, uint16_t *fifosize0, uint16_t *fifosize1)
{
    uint32_t reg_unified_configurations_fesize;

#ifdef VALIDATE_PARMS
    if(!fifosize0 || !fifosize1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_fe_size_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, unified_fe_size_index, BBH_TX, UNIFIED_CONFIGURATIONS_FESIZE, reg_unified_configurations_fesize);

    *fifosize0 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_FESIZE, FIFOSIZE0, reg_unified_configurations_fesize);
    *fifosize1 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_FESIZE, FIFOSIZE1, reg_unified_configurations_fesize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_fepdbase_set(uint8_t bbh_id, uint8_t unified_fe_pd_base_index, uint8_t fifobase0, uint8_t fifobase1)
{
    uint32_t reg_unified_configurations_fepdbase=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_fe_pd_base_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unified_configurations_fepdbase = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_FEPDBASE, FIFOBASE0, reg_unified_configurations_fepdbase, fifobase0);
    reg_unified_configurations_fepdbase = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_FEPDBASE, FIFOBASE1, reg_unified_configurations_fepdbase, fifobase1);

    RU_REG_RAM_WRITE(bbh_id, unified_fe_pd_base_index, BBH_TX, UNIFIED_CONFIGURATIONS_FEPDBASE, reg_unified_configurations_fepdbase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_fepdbase_get(uint8_t bbh_id, uint8_t unified_fe_pd_base_index, uint8_t *fifobase0, uint8_t *fifobase1)
{
    uint32_t reg_unified_configurations_fepdbase;

#ifdef VALIDATE_PARMS
    if(!fifobase0 || !fifobase1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_fe_pd_base_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, unified_fe_pd_base_index, BBH_TX, UNIFIED_CONFIGURATIONS_FEPDBASE, reg_unified_configurations_fepdbase);

    *fifobase0 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_FEPDBASE, FIFOBASE0, reg_unified_configurations_fepdbase);
    *fifobase1 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_FEPDBASE, FIFOBASE1, reg_unified_configurations_fepdbase);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_fepdsize_set(uint8_t bbh_id, uint8_t unified_fe_pd_size_index, uint8_t fifosize0, uint8_t fifosize1)
{
    uint32_t reg_unified_configurations_fepdsize=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_fe_pd_size_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unified_configurations_fepdsize = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_FEPDSIZE, FIFOSIZE0, reg_unified_configurations_fepdsize, fifosize0);
    reg_unified_configurations_fepdsize = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_FEPDSIZE, FIFOSIZE1, reg_unified_configurations_fepdsize, fifosize1);

    RU_REG_RAM_WRITE(bbh_id, unified_fe_pd_size_index, BBH_TX, UNIFIED_CONFIGURATIONS_FEPDSIZE, reg_unified_configurations_fepdsize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_fepdsize_get(uint8_t bbh_id, uint8_t unified_fe_pd_size_index, uint8_t *fifosize0, uint8_t *fifosize1)
{
    uint32_t reg_unified_configurations_fepdsize;

#ifdef VALIDATE_PARMS
    if(!fifosize0 || !fifosize1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_fe_pd_size_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, unified_fe_pd_size_index, BBH_TX, UNIFIED_CONFIGURATIONS_FEPDSIZE, reg_unified_configurations_fepdsize);

    *fifosize0 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_FEPDSIZE, FIFOSIZE0, reg_unified_configurations_fepdsize);
    *fifosize1 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_FEPDSIZE, FIFOSIZE1, reg_unified_configurations_fepdsize);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_txwrr_set(uint8_t bbh_id, uint8_t unified_tx_wrr_index, uint8_t w0, uint8_t w1)
{
    uint32_t reg_unified_configurations_txwrr=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_tx_wrr_index >= 4) ||
       (w0 >= _4BITS_MAX_VAL_) ||
       (w1 >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unified_configurations_txwrr = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_TXWRR, W0, reg_unified_configurations_txwrr, w0);
    reg_unified_configurations_txwrr = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_TXWRR, W1, reg_unified_configurations_txwrr, w1);

    RU_REG_RAM_WRITE(bbh_id, unified_tx_wrr_index, BBH_TX, UNIFIED_CONFIGURATIONS_TXWRR, reg_unified_configurations_txwrr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_txwrr_get(uint8_t bbh_id, uint8_t unified_tx_wrr_index, uint8_t *w0, uint8_t *w1)
{
    uint32_t reg_unified_configurations_txwrr;

#ifdef VALIDATE_PARMS
    if(!w0 || !w1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_tx_wrr_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, unified_tx_wrr_index, BBH_TX, UNIFIED_CONFIGURATIONS_TXWRR, reg_unified_configurations_txwrr);

    *w0 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_TXWRR, W0, reg_unified_configurations_txwrr);
    *w1 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_TXWRR, W1, reg_unified_configurations_txwrr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_txthresh_set(uint8_t bbh_id, uint8_t unified_tx_thr_index, uint16_t thresh0, uint16_t thresh1)
{
    uint32_t reg_unified_configurations_txthresh=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_tx_thr_index >= 4) ||
       (thresh0 >= _9BITS_MAX_VAL_) ||
       (thresh1 >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unified_configurations_txthresh = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_TXTHRESH, THRESH0, reg_unified_configurations_txthresh, thresh0);
    reg_unified_configurations_txthresh = RU_FIELD_SET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_TXTHRESH, THRESH1, reg_unified_configurations_txthresh, thresh1);

    RU_REG_RAM_WRITE(bbh_id, unified_tx_thr_index, BBH_TX, UNIFIED_CONFIGURATIONS_TXTHRESH, reg_unified_configurations_txthresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_unified_configurations_txthresh_get(uint8_t bbh_id, uint8_t unified_tx_thr_index, uint16_t *thresh0, uint16_t *thresh1)
{
    uint32_t reg_unified_configurations_txthresh;

#ifdef VALIDATE_PARMS
    if(!thresh0 || !thresh1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (unified_tx_thr_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, unified_tx_thr_index, BBH_TX, UNIFIED_CONFIGURATIONS_TXTHRESH, reg_unified_configurations_txthresh);

    *thresh0 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_TXTHRESH, THRESH0, reg_unified_configurations_txthresh);
    *thresh1 = RU_FIELD_GET(bbh_id, BBH_TX, UNIFIED_CONFIGURATIONS_TXTHRESH, THRESH1, reg_unified_configurations_txthresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_debug_counters_srambyte_get(uint8_t bbh_id, uint32_t *srambyte)
{
    uint32_t reg_debug_counters_srambyte;

#ifdef VALIDATE_PARMS
    if(!srambyte)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_SRAMBYTE, reg_debug_counters_srambyte);

    *srambyte = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SRAMBYTE, SRAMBYTE, reg_debug_counters_srambyte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_debug_counters_ddrbyte_get(uint8_t bbh_id, uint32_t *ddrbyte)
{
    uint32_t reg_debug_counters_ddrbyte;

#ifdef VALIDATE_PARMS
    if(!ddrbyte)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_DDRBYTE, reg_debug_counters_ddrbyte);

    *ddrbyte = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_DDRBYTE, DDRBYTE, reg_debug_counters_ddrbyte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_debug_counters_swrden_set(uint8_t bbh_id, const bbh_tx_debug_counters_swrden *debug_counters_swrden)
{
    uint32_t reg_debug_counters_swrden=0;

#ifdef VALIDATE_PARMS
    if(!debug_counters_swrden)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (debug_counters_swrden->pdsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->pdvsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->pdemptysel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->pdfullsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->pdbemptysel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->pdffwkpsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->fbnsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->fbnvsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->fbnemptysel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->fbnfullsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->getnextsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->getnextvsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->getnextemptysel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->getnextfullsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->gpncntxtsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->bpmsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->bpmfsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->sbpmsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->sbpmfsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->stssel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->stsvsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->stsemptysel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->stsfullsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->stsbemptysel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->stsffwkpsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->msgsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->msgvsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->epnreqsel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->datasel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->reordersel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->tsinfosel >= _1BITS_MAX_VAL_) ||
       (debug_counters_swrden->mactxsel >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, PDSEL, reg_debug_counters_swrden, debug_counters_swrden->pdsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, PDVSEL, reg_debug_counters_swrden, debug_counters_swrden->pdvsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, PDEMPTYSEL, reg_debug_counters_swrden, debug_counters_swrden->pdemptysel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, PDFULLSEL, reg_debug_counters_swrden, debug_counters_swrden->pdfullsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, PDBEMPTYSEL, reg_debug_counters_swrden, debug_counters_swrden->pdbemptysel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, PDFFWKPSEL, reg_debug_counters_swrden, debug_counters_swrden->pdffwkpsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNSEL, reg_debug_counters_swrden, debug_counters_swrden->fbnsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNVSEL, reg_debug_counters_swrden, debug_counters_swrden->fbnvsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNEMPTYSEL, reg_debug_counters_swrden, debug_counters_swrden->fbnemptysel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNFULLSEL, reg_debug_counters_swrden, debug_counters_swrden->fbnfullsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTSEL, reg_debug_counters_swrden, debug_counters_swrden->getnextsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTVSEL, reg_debug_counters_swrden, debug_counters_swrden->getnextvsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTEMPTYSEL, reg_debug_counters_swrden, debug_counters_swrden->getnextemptysel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTFULLSEL, reg_debug_counters_swrden, debug_counters_swrden->getnextfullsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, GPNCNTXTSEL, reg_debug_counters_swrden, debug_counters_swrden->gpncntxtsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, BPMSEL, reg_debug_counters_swrden, debug_counters_swrden->bpmsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, BPMFSEL, reg_debug_counters_swrden, debug_counters_swrden->bpmfsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, SBPMSEL, reg_debug_counters_swrden, debug_counters_swrden->sbpmsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, SBPMFSEL, reg_debug_counters_swrden, debug_counters_swrden->sbpmfsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, STSSEL, reg_debug_counters_swrden, debug_counters_swrden->stssel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, STSVSEL, reg_debug_counters_swrden, debug_counters_swrden->stsvsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, STSEMPTYSEL, reg_debug_counters_swrden, debug_counters_swrden->stsemptysel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, STSFULLSEL, reg_debug_counters_swrden, debug_counters_swrden->stsfullsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, STSBEMPTYSEL, reg_debug_counters_swrden, debug_counters_swrden->stsbemptysel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, STSFFWKPSEL, reg_debug_counters_swrden, debug_counters_swrden->stsffwkpsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, MSGSEL, reg_debug_counters_swrden, debug_counters_swrden->msgsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, MSGVSEL, reg_debug_counters_swrden, debug_counters_swrden->msgvsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, EPNREQSEL, reg_debug_counters_swrden, debug_counters_swrden->epnreqsel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, DATASEL, reg_debug_counters_swrden, debug_counters_swrden->datasel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, REORDERSEL, reg_debug_counters_swrden, debug_counters_swrden->reordersel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, TSINFOSEL, reg_debug_counters_swrden, debug_counters_swrden->tsinfosel);
    reg_debug_counters_swrden = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, MACTXSEL, reg_debug_counters_swrden, debug_counters_swrden->mactxsel);

    RU_REG_WRITE(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, reg_debug_counters_swrden);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_debug_counters_swrden_get(uint8_t bbh_id, bbh_tx_debug_counters_swrden *debug_counters_swrden)
{
    uint32_t reg_debug_counters_swrden;

#ifdef VALIDATE_PARMS
    if(!debug_counters_swrden)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, reg_debug_counters_swrden);

    debug_counters_swrden->pdsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, PDSEL, reg_debug_counters_swrden);
    debug_counters_swrden->pdvsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, PDVSEL, reg_debug_counters_swrden);
    debug_counters_swrden->pdemptysel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, PDEMPTYSEL, reg_debug_counters_swrden);
    debug_counters_swrden->pdfullsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, PDFULLSEL, reg_debug_counters_swrden);
    debug_counters_swrden->pdbemptysel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, PDBEMPTYSEL, reg_debug_counters_swrden);
    debug_counters_swrden->pdffwkpsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, PDFFWKPSEL, reg_debug_counters_swrden);
    debug_counters_swrden->fbnsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNSEL, reg_debug_counters_swrden);
    debug_counters_swrden->fbnvsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNVSEL, reg_debug_counters_swrden);
    debug_counters_swrden->fbnemptysel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNEMPTYSEL, reg_debug_counters_swrden);
    debug_counters_swrden->fbnfullsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, FBNFULLSEL, reg_debug_counters_swrden);
    debug_counters_swrden->getnextsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTSEL, reg_debug_counters_swrden);
    debug_counters_swrden->getnextvsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTVSEL, reg_debug_counters_swrden);
    debug_counters_swrden->getnextemptysel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTEMPTYSEL, reg_debug_counters_swrden);
    debug_counters_swrden->getnextfullsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, GETNEXTFULLSEL, reg_debug_counters_swrden);
    debug_counters_swrden->gpncntxtsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, GPNCNTXTSEL, reg_debug_counters_swrden);
    debug_counters_swrden->bpmsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, BPMSEL, reg_debug_counters_swrden);
    debug_counters_swrden->bpmfsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, BPMFSEL, reg_debug_counters_swrden);
    debug_counters_swrden->sbpmsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, SBPMSEL, reg_debug_counters_swrden);
    debug_counters_swrden->sbpmfsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, SBPMFSEL, reg_debug_counters_swrden);
    debug_counters_swrden->stssel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, STSSEL, reg_debug_counters_swrden);
    debug_counters_swrden->stsvsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, STSVSEL, reg_debug_counters_swrden);
    debug_counters_swrden->stsemptysel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, STSEMPTYSEL, reg_debug_counters_swrden);
    debug_counters_swrden->stsfullsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, STSFULLSEL, reg_debug_counters_swrden);
    debug_counters_swrden->stsbemptysel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, STSBEMPTYSEL, reg_debug_counters_swrden);
    debug_counters_swrden->stsffwkpsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, STSFFWKPSEL, reg_debug_counters_swrden);
    debug_counters_swrden->msgsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, MSGSEL, reg_debug_counters_swrden);
    debug_counters_swrden->msgvsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, MSGVSEL, reg_debug_counters_swrden);
    debug_counters_swrden->epnreqsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, EPNREQSEL, reg_debug_counters_swrden);
    debug_counters_swrden->datasel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, DATASEL, reg_debug_counters_swrden);
    debug_counters_swrden->reordersel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, REORDERSEL, reg_debug_counters_swrden);
    debug_counters_swrden->tsinfosel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, TSINFOSEL, reg_debug_counters_swrden);
    debug_counters_swrden->mactxsel = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDEN, MACTXSEL, reg_debug_counters_swrden);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_debug_counters_swrdaddr_set(uint8_t bbh_id, uint16_t rdaddr)
{
    uint32_t reg_debug_counters_swrdaddr=0;

#ifdef VALIDATE_PARMS
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (rdaddr >= _11BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_counters_swrdaddr = RU_FIELD_SET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDADDR, RDADDR, reg_debug_counters_swrdaddr, rdaddr);

    RU_REG_WRITE(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDADDR, reg_debug_counters_swrdaddr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_debug_counters_swrdaddr_get(uint8_t bbh_id, uint16_t *rdaddr)
{
    uint32_t reg_debug_counters_swrdaddr;

#ifdef VALIDATE_PARMS
    if(!rdaddr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDADDR, reg_debug_counters_swrdaddr);

    *rdaddr = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDADDR, RDADDR, reg_debug_counters_swrdaddr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_debug_counters_swrddata_get(uint8_t bbh_id, uint32_t *data)
{
    uint32_t reg_debug_counters_swrddata;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDDATA, reg_debug_counters_swrddata);

    *data = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_SWRDDATA, DATA, reg_debug_counters_swrddata);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_debug_counters_unifiedpkt_get(uint8_t bbh_id, uint8_t debug_unified_pkt_ctr_idx, uint32_t *ddrbyte)
{
    uint32_t reg_debug_counters_unifiedpkt;

#ifdef VALIDATE_PARMS
    if(!ddrbyte)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (debug_unified_pkt_ctr_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, debug_unified_pkt_ctr_idx, BBH_TX, DEBUG_COUNTERS_UNIFIEDPKT, reg_debug_counters_unifiedpkt);

    *ddrbyte = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_UNIFIEDPKT, DDRBYTE, reg_debug_counters_unifiedpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_debug_counters_unifiedbyte_get(uint8_t bbh_id, uint8_t debug_unified_byte_ctr_idx, uint32_t *ddrbyte)
{
    uint32_t reg_debug_counters_unifiedbyte;

#ifdef VALIDATE_PARMS
    if(!ddrbyte)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (debug_unified_byte_ctr_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, debug_unified_byte_ctr_idx, BBH_TX, DEBUG_COUNTERS_UNIFIEDBYTE, reg_debug_counters_unifiedbyte);

    *ddrbyte = RU_FIELD_GET(bbh_id, BBH_TX, DEBUG_COUNTERS_UNIFIEDBYTE, DDRBYTE, reg_debug_counters_unifiedbyte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_bbh_tx_debug_counters_dbgoutreg_get(uint8_t bbh_id, uint8_t zero, bbh_tx_debug_counters_dbgoutreg *debug_counters_dbgoutreg)
{
#ifdef VALIDATE_PARMS
    if(!debug_counters_dbgoutreg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((bbh_id >= BLOCK_ADDR_COUNT) ||
       (zero >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(bbh_id, zero *8 + 0, BBH_TX, DEBUG_COUNTERS_DBGOUTREG, debug_counters_dbgoutreg->debug_out_reg[0]);
    RU_REG_RAM_READ(bbh_id, zero *8 + 1, BBH_TX, DEBUG_COUNTERS_DBGOUTREG, debug_counters_dbgoutreg->debug_out_reg[1]);
    RU_REG_RAM_READ(bbh_id, zero *8 + 2, BBH_TX, DEBUG_COUNTERS_DBGOUTREG, debug_counters_dbgoutreg->debug_out_reg[2]);
    RU_REG_RAM_READ(bbh_id, zero *8 + 3, BBH_TX, DEBUG_COUNTERS_DBGOUTREG, debug_counters_dbgoutreg->debug_out_reg[3]);
    RU_REG_RAM_READ(bbh_id, zero *8 + 4, BBH_TX, DEBUG_COUNTERS_DBGOUTREG, debug_counters_dbgoutreg->debug_out_reg[4]);
    RU_REG_RAM_READ(bbh_id, zero *8 + 5, BBH_TX, DEBUG_COUNTERS_DBGOUTREG, debug_counters_dbgoutreg->debug_out_reg[5]);
    RU_REG_RAM_READ(bbh_id, zero *8 + 6, BBH_TX, DEBUG_COUNTERS_DBGOUTREG, debug_counters_dbgoutreg->debug_out_reg[6]);
    RU_REG_RAM_READ(bbh_id, zero *8 + 7, BBH_TX, DEBUG_COUNTERS_DBGOUTREG, debug_counters_dbgoutreg->debug_out_reg[7]);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_common_configurations_mactype,
    bdmf_address_common_configurations_bbcfg_1_tx,
    bdmf_address_common_configurations_bbcfg_2_tx,
    bdmf_address_common_configurations_ddrcfg_tx,
    bdmf_address_common_configurations_rnrcfg_1,
    bdmf_address_common_configurations_rnrcfg_2,
    bdmf_address_common_configurations_dmacfg_tx,
    bdmf_address_common_configurations_sdmacfg_tx,
    bdmf_address_common_configurations_sbpmcfg,
    bdmf_address_common_configurations_ddrtmbasel,
    bdmf_address_common_configurations_ddrtmbaseh,
    bdmf_address_common_configurations_dfifoctrl,
    bdmf_address_common_configurations_arb_cfg,
    bdmf_address_common_configurations_bbroute,
    bdmf_address_common_configurations_q2rnr,
    bdmf_address_common_configurations_perqtask,
    bdmf_address_common_configurations_txrstcmd,
    bdmf_address_common_configurations_dbgsel,
    bdmf_address_common_configurations_clk_gate_cntrl,
    bdmf_address_wan_configurations_pdbase,
    bdmf_address_wan_configurations_pdsize,
    bdmf_address_wan_configurations_pdwkuph,
    bdmf_address_wan_configurations_pd_byte_th,
    bdmf_address_wan_configurations_pd_byte_th_en,
    bdmf_address_wan_configurations_pdempty,
    bdmf_address_wan_configurations_stsrnrcfg_1,
    bdmf_address_wan_configurations_stsrnrcfg_2,
    bdmf_address_wan_configurations_msgrnrcfg_1,
    bdmf_address_wan_configurations_msgrnrcfg_2,
    bdmf_address_wan_configurations_epncfg,
    bdmf_address_wan_configurations_flow2port,
    bdmf_address_wan_configurations_ts,
    bdmf_address_wan_configurations_maxwlen,
    bdmf_address_wan_configurations_flush,
    bdmf_address_lan_configurations_pdbase,
    bdmf_address_lan_configurations_pdsize,
    bdmf_address_lan_configurations_pdwkuph,
    bdmf_address_lan_configurations_pd_byte_th,
    bdmf_address_lan_configurations_pd_byte_th_en,
    bdmf_address_lan_configurations_pdempty,
    bdmf_address_lan_configurations_txthresh,
    bdmf_address_lan_configurations_eee,
    bdmf_address_lan_configurations_ts,
    bdmf_address_unified_configurations_pdbase,
    bdmf_address_unified_configurations_pdsize,
    bdmf_address_unified_configurations_pdwkuph,
    bdmf_address_unified_configurations_pd_byte_th,
    bdmf_address_unified_configurations_pd_byte_th_en,
    bdmf_address_unified_configurations_pdempty,
    bdmf_address_unified_configurations_gtxthresh,
    bdmf_address_unified_configurations_eee,
    bdmf_address_unified_configurations_ts,
    bdmf_address_unified_configurations_febase,
    bdmf_address_unified_configurations_fesize,
    bdmf_address_unified_configurations_fepdbase,
    bdmf_address_unified_configurations_fepdsize,
    bdmf_address_unified_configurations_txwrr,
    bdmf_address_unified_configurations_txthresh,
    bdmf_address_debug_counters_srampd,
    bdmf_address_debug_counters_ddrpd,
    bdmf_address_debug_counters_pddrop,
    bdmf_address_debug_counters_stscnt,
    bdmf_address_debug_counters_stsdrop,
    bdmf_address_debug_counters_msgcnt,
    bdmf_address_debug_counters_msgdrop,
    bdmf_address_debug_counters_getnextnull,
    bdmf_address_debug_counters_flushpkts,
    bdmf_address_debug_counters_lenerr,
    bdmf_address_debug_counters_aggrlenerr,
    bdmf_address_debug_counters_srampkt,
    bdmf_address_debug_counters_ddrpkt,
    bdmf_address_debug_counters_srambyte,
    bdmf_address_debug_counters_ddrbyte,
    bdmf_address_debug_counters_swrden,
    bdmf_address_debug_counters_swrdaddr,
    bdmf_address_debug_counters_swrddata,
    bdmf_address_debug_counters_unifiedpkt,
    bdmf_address_debug_counters_unifiedbyte,
    bdmf_address_debug_counters_dbgoutreg,
}
bdmf_address;

static int bcm_bbh_tx_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_bbh_tx_mac_type:
        err = ag_drv_bbh_tx_mac_type_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_cfg_src_id:
    {
        bbh_tx_cfg_src_id cfg_src_id = { .fpmsrc=parm[2].value.unumber, .sbpmsrc=parm[3].value.unumber, .stsrnrsrc=parm[4].value.unumber, .msgrnrsrc=parm[5].value.unumber};
        err = ag_drv_bbh_tx_cfg_src_id_set(parm[1].value.unumber, &cfg_src_id);
        break;
    }
    case cli_bbh_tx_rnr_src_id:
        err = ag_drv_bbh_tx_rnr_src_id_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_tx_bbh_dma_cfg:
    {
        bbh_tx_bbh_dma_cfg bbh_dma_cfg = { .dmasrc=parm[2].value.unumber, .descbase=parm[3].value.unumber, .descsize=parm[4].value.unumber};
        err = ag_drv_bbh_tx_bbh_dma_cfg_set(parm[1].value.unumber, &bbh_dma_cfg);
        break;
    }
    case cli_bbh_tx_dma_max_otf_read_request:
        err = ag_drv_bbh_tx_dma_max_otf_read_request_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_dma_epon_urgent:
        err = ag_drv_bbh_tx_dma_epon_urgent_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_bbh_sdma_cfg:
    {
        bbh_tx_bbh_sdma_cfg bbh_sdma_cfg = { .sdmasrc=parm[2].value.unumber, .descbase=parm[3].value.unumber, .descsize=parm[4].value.unumber};
        err = ag_drv_bbh_tx_bbh_sdma_cfg_set(parm[1].value.unumber, &bbh_sdma_cfg);
        break;
    }
    case cli_bbh_tx_sdma_max_otf_read_request:
        err = ag_drv_bbh_tx_sdma_max_otf_read_request_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_sdma_epon_urgent:
        err = ag_drv_bbh_tx_sdma_epon_urgent_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_bbh_ddr_cfg:
    {
        bbh_tx_bbh_ddr_cfg bbh_ddr_cfg = { .bufsize=parm[2].value.unumber, .byteresul=parm[3].value.unumber, .ddrtxoffset=parm[4].value.unumber, .hnsize0=parm[5].value.unumber, .hnsize1=parm[6].value.unumber};
        err = ag_drv_bbh_tx_bbh_ddr_cfg_set(parm[1].value.unumber, &bbh_ddr_cfg);
        break;
    }
    case cli_bbh_tx_common_configurations_rnrcfg_1:
        err = ag_drv_bbh_tx_common_configurations_rnrcfg_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_common_configurations_rnrcfg_2:
        err = ag_drv_bbh_tx_common_configurations_rnrcfg_2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_common_configurations_sbpmcfg:
        err = ag_drv_bbh_tx_common_configurations_sbpmcfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_common_configurations_ddrtmbasel:
    {
        bbh_tx_common_configurations_ddrtmbasel common_configurations_ddrtmbasel = { .addr = { parm[3].value.unumber, parm[4].value.unumber}};
        err = ag_drv_bbh_tx_common_configurations_ddrtmbasel_set(parm[1].value.unumber, parm[2].value.unumber, &common_configurations_ddrtmbasel);
        break;
    }
    case cli_bbh_tx_common_configurations_ddrtmbaseh:
    {
        bbh_tx_common_configurations_ddrtmbaseh common_configurations_ddrtmbaseh = { .addr = { parm[3].value.unumber, parm[4].value.unumber}};
        err = ag_drv_bbh_tx_common_configurations_ddrtmbaseh_set(parm[1].value.unumber, parm[2].value.unumber, &common_configurations_ddrtmbaseh);
        break;
    }
    case cli_bbh_tx_common_configurations_dfifoctrl:
        err = ag_drv_bbh_tx_common_configurations_dfifoctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_common_configurations_arb_cfg:
        err = ag_drv_bbh_tx_common_configurations_arb_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_common_configurations_bbroute:
        err = ag_drv_bbh_tx_common_configurations_bbroute_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_common_configurations_q2rnr:
        err = ag_drv_bbh_tx_common_configurations_q2rnr_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_common_configurations_perqtask:
    {
        bbh_tx_common_configurations_perqtask common_configurations_perqtask = { .task0=parm[2].value.unumber, .task1=parm[3].value.unumber, .task2=parm[4].value.unumber, .task3=parm[5].value.unumber, .task4=parm[6].value.unumber, .task5=parm[7].value.unumber, .task6=parm[8].value.unumber, .task7=parm[9].value.unumber};
        err = ag_drv_bbh_tx_common_configurations_perqtask_set(parm[1].value.unumber, &common_configurations_perqtask);
        break;
    }
    case cli_bbh_tx_common_configurations_txrstcmd:
    {
        bbh_tx_common_configurations_txrstcmd common_configurations_txrstcmd = { .cntxtrst=parm[2].value.unumber, .pdfiforst=parm[3].value.unumber, .dmaptrrst=parm[4].value.unumber, .sdmaptrrst=parm[5].value.unumber, .bpmfiforst=parm[6].value.unumber, .sbpmfiforst=parm[7].value.unumber, .okfiforst=parm[8].value.unumber, .ddrfiforst=parm[9].value.unumber, .sramfiforst=parm[10].value.unumber, .skbptrrst=parm[11].value.unumber, .stsfiforst=parm[12].value.unumber, .reqfiforst=parm[13].value.unumber, .msgfiforst=parm[14].value.unumber, .gnxtfiforst=parm[15].value.unumber, .fbnfiforst=parm[16].value.unumber};
        err = ag_drv_bbh_tx_common_configurations_txrstcmd_set(parm[1].value.unumber, &common_configurations_txrstcmd);
        break;
    }
    case cli_bbh_tx_common_configurations_dbgsel:
        err = ag_drv_bbh_tx_common_configurations_dbgsel_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_common_configurations_clk_gate_cntrl:
    {
        bbh_tx_common_configurations_clk_gate_cntrl common_configurations_clk_gate_cntrl = { .bypass_clk_gate=parm[2].value.unumber, .timer_val=parm[3].value.unumber, .keep_alive_en=parm[4].value.unumber, .keep_alive_intrvl=parm[5].value.unumber, .keep_alive_cyc=parm[6].value.unumber};
        err = ag_drv_bbh_tx_common_configurations_clk_gate_cntrl_set(parm[1].value.unumber, &common_configurations_clk_gate_cntrl);
        break;
    }
    case cli_bbh_tx_wan_configurations_pdbase:
        err = ag_drv_bbh_tx_wan_configurations_pdbase_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_wan_configurations_pdsize:
        err = ag_drv_bbh_tx_wan_configurations_pdsize_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_wan_configurations_pdwkuph:
        err = ag_drv_bbh_tx_wan_configurations_pdwkuph_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_wan_configurations_pd_byte_th:
        err = ag_drv_bbh_tx_wan_configurations_pd_byte_th_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_wan_configurations_pd_byte_th_en:
        err = ag_drv_bbh_tx_wan_configurations_pd_byte_th_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_wan_configurations_pdempty:
        err = ag_drv_bbh_tx_wan_configurations_pdempty_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_wan_configurations_stsrnrcfg_1:
        err = ag_drv_bbh_tx_wan_configurations_stsrnrcfg_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_tx_wan_configurations_stsrnrcfg_2:
        err = ag_drv_bbh_tx_wan_configurations_stsrnrcfg_2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_wan_configurations_msgrnrcfg_1:
        err = ag_drv_bbh_tx_wan_configurations_msgrnrcfg_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_tx_wan_configurations_msgrnrcfg_2:
        err = ag_drv_bbh_tx_wan_configurations_msgrnrcfg_2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_wan_configurations_epncfg:
        err = ag_drv_bbh_tx_wan_configurations_epncfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_bbh_tx_wan_configurations_flow2port:
        err = ag_drv_bbh_tx_wan_configurations_flow2port_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_wan_configurations_ts:
        err = ag_drv_bbh_tx_wan_configurations_ts_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_wan_configurations_maxwlen:
        err = ag_drv_bbh_tx_wan_configurations_maxwlen_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_wan_configurations_flush:
        err = ag_drv_bbh_tx_wan_configurations_flush_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_tx_lan_configurations_pdbase:
        err = ag_drv_bbh_tx_lan_configurations_pdbase_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_tx_lan_configurations_pdsize:
        err = ag_drv_bbh_tx_lan_configurations_pdsize_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_tx_lan_configurations_pdwkuph:
        err = ag_drv_bbh_tx_lan_configurations_pdwkuph_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_tx_lan_configurations_pd_byte_th:
        err = ag_drv_bbh_tx_lan_configurations_pd_byte_th_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_tx_lan_configurations_pd_byte_th_en:
        err = ag_drv_bbh_tx_lan_configurations_pd_byte_th_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_lan_configurations_pdempty:
        err = ag_drv_bbh_tx_lan_configurations_pdempty_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_lan_configurations_txthresh:
        err = ag_drv_bbh_tx_lan_configurations_txthresh_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_tx_lan_configurations_eee:
        err = ag_drv_bbh_tx_lan_configurations_eee_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_lan_configurations_ts:
        err = ag_drv_bbh_tx_lan_configurations_ts_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_unified_configurations_pdbase:
        err = ag_drv_bbh_tx_unified_configurations_pdbase_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_unified_configurations_pdsize:
        err = ag_drv_bbh_tx_unified_configurations_pdsize_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_unified_configurations_pdwkuph:
        err = ag_drv_bbh_tx_unified_configurations_pdwkuph_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_unified_configurations_pd_byte_th:
        err = ag_drv_bbh_tx_unified_configurations_pd_byte_th_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_unified_configurations_pd_byte_th_en:
        err = ag_drv_bbh_tx_unified_configurations_pd_byte_th_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_unified_configurations_pdempty:
        err = ag_drv_bbh_tx_unified_configurations_pdempty_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_unified_configurations_gtxthresh:
        err = ag_drv_bbh_tx_unified_configurations_gtxthresh_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_bbh_tx_unified_configurations_eee:
        err = ag_drv_bbh_tx_unified_configurations_eee_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_unified_configurations_ts:
        err = ag_drv_bbh_tx_unified_configurations_ts_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_bbh_tx_unified_configurations_febase:
        err = ag_drv_bbh_tx_unified_configurations_febase_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_unified_configurations_fesize:
        err = ag_drv_bbh_tx_unified_configurations_fesize_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_unified_configurations_fepdbase:
        err = ag_drv_bbh_tx_unified_configurations_fepdbase_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_unified_configurations_fepdsize:
        err = ag_drv_bbh_tx_unified_configurations_fepdsize_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_unified_configurations_txwrr:
        err = ag_drv_bbh_tx_unified_configurations_txwrr_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_unified_configurations_txthresh:
        err = ag_drv_bbh_tx_unified_configurations_txthresh_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_bbh_tx_debug_counters_swrden:
    {
        bbh_tx_debug_counters_swrden debug_counters_swrden = { .pdsel=parm[2].value.unumber, .pdvsel=parm[3].value.unumber, .pdemptysel=parm[4].value.unumber, .pdfullsel=parm[5].value.unumber, .pdbemptysel=parm[6].value.unumber, .pdffwkpsel=parm[7].value.unumber, .fbnsel=parm[8].value.unumber, .fbnvsel=parm[9].value.unumber, .fbnemptysel=parm[10].value.unumber, .fbnfullsel=parm[11].value.unumber, .getnextsel=parm[12].value.unumber, .getnextvsel=parm[13].value.unumber, .getnextemptysel=parm[14].value.unumber, .getnextfullsel=parm[15].value.unumber, .gpncntxtsel=parm[16].value.unumber, .bpmsel=parm[17].value.unumber, .bpmfsel=parm[18].value.unumber, .sbpmsel=parm[19].value.unumber, .sbpmfsel=parm[20].value.unumber, .stssel=parm[21].value.unumber, .stsvsel=parm[22].value.unumber, .stsemptysel=parm[23].value.unumber, .stsfullsel=parm[24].value.unumber, .stsbemptysel=parm[25].value.unumber, .stsffwkpsel=parm[26].value.unumber, .msgsel=parm[27].value.unumber, .msgvsel=parm[28].value.unumber, .epnreqsel=parm[29].value.unumber, .datasel=parm[30].value.unumber, .reordersel=parm[31].value.unumber, .tsinfosel=parm[32].value.unumber, .mactxsel=parm[33].value.unumber};
        err = ag_drv_bbh_tx_debug_counters_swrden_set(parm[1].value.unumber, &debug_counters_swrden);
        break;
    }
    case cli_bbh_tx_debug_counters_swrdaddr:
        err = ag_drv_bbh_tx_debug_counters_swrdaddr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_bbh_tx_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_bbh_tx_mac_type:
    {
        uint8_t type;
        err = ag_drv_bbh_tx_mac_type_get(parm[1].value.unumber, &type);
        bdmf_session_print(session, "type = %u (0x%x)\n", type, type);
        break;
    }
    case cli_bbh_tx_cfg_src_id:
    {
        bbh_tx_cfg_src_id cfg_src_id;
        err = ag_drv_bbh_tx_cfg_src_id_get(parm[1].value.unumber, &cfg_src_id);
        bdmf_session_print(session, "fpmsrc = %u (0x%x)\n", cfg_src_id.fpmsrc, cfg_src_id.fpmsrc);
        bdmf_session_print(session, "sbpmsrc = %u (0x%x)\n", cfg_src_id.sbpmsrc, cfg_src_id.sbpmsrc);
        bdmf_session_print(session, "stsrnrsrc = %u (0x%x)\n", cfg_src_id.stsrnrsrc, cfg_src_id.stsrnrsrc);
        bdmf_session_print(session, "msgrnrsrc = %u (0x%x)\n", cfg_src_id.msgrnrsrc, cfg_src_id.msgrnrsrc);
        break;
    }
    case cli_bbh_tx_rnr_src_id:
    {
        uint8_t pdrnr0src;
        uint8_t pdrnr1src;
        err = ag_drv_bbh_tx_rnr_src_id_get(parm[1].value.unumber, &pdrnr0src, &pdrnr1src);
        bdmf_session_print(session, "pdrnr0src = %u (0x%x)\n", pdrnr0src, pdrnr0src);
        bdmf_session_print(session, "pdrnr1src = %u (0x%x)\n", pdrnr1src, pdrnr1src);
        break;
    }
    case cli_bbh_tx_bbh_dma_cfg:
    {
        bbh_tx_bbh_dma_cfg bbh_dma_cfg;
        err = ag_drv_bbh_tx_bbh_dma_cfg_get(parm[1].value.unumber, &bbh_dma_cfg);
        bdmf_session_print(session, "dmasrc = %u (0x%x)\n", bbh_dma_cfg.dmasrc, bbh_dma_cfg.dmasrc);
        bdmf_session_print(session, "descbase = %u (0x%x)\n", bbh_dma_cfg.descbase, bbh_dma_cfg.descbase);
        bdmf_session_print(session, "descsize = %u (0x%x)\n", bbh_dma_cfg.descsize, bbh_dma_cfg.descsize);
        break;
    }
    case cli_bbh_tx_dma_max_otf_read_request:
    {
        uint8_t maxreq;
        err = ag_drv_bbh_tx_dma_max_otf_read_request_get(parm[1].value.unumber, &maxreq);
        bdmf_session_print(session, "maxreq = %u (0x%x)\n", maxreq, maxreq);
        break;
    }
    case cli_bbh_tx_dma_epon_urgent:
    {
        bdmf_boolean epnurgnt;
        err = ag_drv_bbh_tx_dma_epon_urgent_get(parm[1].value.unumber, &epnurgnt);
        bdmf_session_print(session, "epnurgnt = %u (0x%x)\n", epnurgnt, epnurgnt);
        break;
    }
    case cli_bbh_tx_bbh_sdma_cfg:
    {
        bbh_tx_bbh_sdma_cfg bbh_sdma_cfg;
        err = ag_drv_bbh_tx_bbh_sdma_cfg_get(parm[1].value.unumber, &bbh_sdma_cfg);
        bdmf_session_print(session, "sdmasrc = %u (0x%x)\n", bbh_sdma_cfg.sdmasrc, bbh_sdma_cfg.sdmasrc);
        bdmf_session_print(session, "descbase = %u (0x%x)\n", bbh_sdma_cfg.descbase, bbh_sdma_cfg.descbase);
        bdmf_session_print(session, "descsize = %u (0x%x)\n", bbh_sdma_cfg.descsize, bbh_sdma_cfg.descsize);
        break;
    }
    case cli_bbh_tx_sdma_max_otf_read_request:
    {
        uint8_t maxreq;
        err = ag_drv_bbh_tx_sdma_max_otf_read_request_get(parm[1].value.unumber, &maxreq);
        bdmf_session_print(session, "maxreq = %u (0x%x)\n", maxreq, maxreq);
        break;
    }
    case cli_bbh_tx_sdma_epon_urgent:
    {
        bdmf_boolean epnurgnt;
        err = ag_drv_bbh_tx_sdma_epon_urgent_get(parm[1].value.unumber, &epnurgnt);
        bdmf_session_print(session, "epnurgnt = %u (0x%x)\n", epnurgnt, epnurgnt);
        break;
    }
    case cli_bbh_tx_bbh_ddr_cfg:
    {
        bbh_tx_bbh_ddr_cfg bbh_ddr_cfg;
        err = ag_drv_bbh_tx_bbh_ddr_cfg_get(parm[1].value.unumber, &bbh_ddr_cfg);
        bdmf_session_print(session, "bufsize = %u (0x%x)\n", bbh_ddr_cfg.bufsize, bbh_ddr_cfg.bufsize);
        bdmf_session_print(session, "byteresul = %u (0x%x)\n", bbh_ddr_cfg.byteresul, bbh_ddr_cfg.byteresul);
        bdmf_session_print(session, "ddrtxoffset = %u (0x%x)\n", bbh_ddr_cfg.ddrtxoffset, bbh_ddr_cfg.ddrtxoffset);
        bdmf_session_print(session, "hnsize0 = %u (0x%x)\n", bbh_ddr_cfg.hnsize0, bbh_ddr_cfg.hnsize0);
        bdmf_session_print(session, "hnsize1 = %u (0x%x)\n", bbh_ddr_cfg.hnsize1, bbh_ddr_cfg.hnsize1);
        break;
    }
    case cli_bbh_tx_debug_counters:
    {
        bbh_tx_debug_counters debug_counters;
        err = ag_drv_bbh_tx_debug_counters_get(parm[1].value.unumber, &debug_counters);
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
    case cli_bbh_tx_common_configurations_rnrcfg_1:
    {
        uint16_t tcontaddr;
        uint16_t skbaddr;
        err = ag_drv_bbh_tx_common_configurations_rnrcfg_1_get(parm[1].value.unumber, parm[2].value.unumber, &tcontaddr, &skbaddr);
        bdmf_session_print(session, "tcontaddr = %u (0x%x)\n", tcontaddr, tcontaddr);
        bdmf_session_print(session, "skbaddr = %u (0x%x)\n", skbaddr, skbaddr);
        break;
    }
    case cli_bbh_tx_common_configurations_rnrcfg_2:
    {
        uint16_t ptraddr;
        uint8_t task;
        err = ag_drv_bbh_tx_common_configurations_rnrcfg_2_get(parm[1].value.unumber, parm[2].value.unumber, &ptraddr, &task);
        bdmf_session_print(session, "ptraddr = %u (0x%x)\n", ptraddr, ptraddr);
        bdmf_session_print(session, "task = %u (0x%x)\n", task, task);
        break;
    }
    case cli_bbh_tx_common_configurations_sbpmcfg:
    {
        bdmf_boolean freenocntxt;
        bdmf_boolean specialfree;
        uint8_t maxgn;
        err = ag_drv_bbh_tx_common_configurations_sbpmcfg_get(parm[1].value.unumber, &freenocntxt, &specialfree, &maxgn);
        bdmf_session_print(session, "freenocntxt = %u (0x%x)\n", freenocntxt, freenocntxt);
        bdmf_session_print(session, "specialfree = %u (0x%x)\n", specialfree, specialfree);
        bdmf_session_print(session, "maxgn = %u (0x%x)\n", maxgn, maxgn);
        break;
    }
    case cli_bbh_tx_common_configurations_ddrtmbasel:
    {
        bbh_tx_common_configurations_ddrtmbasel common_configurations_ddrtmbasel;
        err = ag_drv_bbh_tx_common_configurations_ddrtmbasel_get(parm[1].value.unumber, parm[2].value.unumber, &common_configurations_ddrtmbasel);
        bdmf_session_print(session, "addr[0] = %u (0x%x)\n", common_configurations_ddrtmbasel.addr[0], common_configurations_ddrtmbasel.addr[0]);
        bdmf_session_print(session, "addr[1] = %u (0x%x)\n", common_configurations_ddrtmbasel.addr[1], common_configurations_ddrtmbasel.addr[1]);
        break;
    }
    case cli_bbh_tx_common_configurations_ddrtmbaseh:
    {
        bbh_tx_common_configurations_ddrtmbaseh common_configurations_ddrtmbaseh;
        err = ag_drv_bbh_tx_common_configurations_ddrtmbaseh_get(parm[1].value.unumber, parm[2].value.unumber, &common_configurations_ddrtmbaseh);
        bdmf_session_print(session, "addr[0] = %u (0x%x)\n", common_configurations_ddrtmbaseh.addr[0], common_configurations_ddrtmbaseh.addr[0]);
        bdmf_session_print(session, "addr[1] = %u (0x%x)\n", common_configurations_ddrtmbaseh.addr[1], common_configurations_ddrtmbaseh.addr[1]);
        break;
    }
    case cli_bbh_tx_common_configurations_dfifoctrl:
    {
        uint16_t psramsize;
        uint16_t ddrsize;
        uint16_t psrambase;
        err = ag_drv_bbh_tx_common_configurations_dfifoctrl_get(parm[1].value.unumber, &psramsize, &ddrsize, &psrambase);
        bdmf_session_print(session, "psramsize = %u (0x%x)\n", psramsize, psramsize);
        bdmf_session_print(session, "ddrsize = %u (0x%x)\n", ddrsize, ddrsize);
        bdmf_session_print(session, "psrambase = %u (0x%x)\n", psrambase, psrambase);
        break;
    }
    case cli_bbh_tx_common_configurations_arb_cfg:
    {
        bdmf_boolean hightrxq;
        err = ag_drv_bbh_tx_common_configurations_arb_cfg_get(parm[1].value.unumber, &hightrxq);
        bdmf_session_print(session, "hightrxq = %u (0x%x)\n", hightrxq, hightrxq);
        break;
    }
    case cli_bbh_tx_common_configurations_bbroute:
    {
        uint16_t route;
        uint8_t dest;
        bdmf_boolean en;
        err = ag_drv_bbh_tx_common_configurations_bbroute_get(parm[1].value.unumber, &route, &dest, &en);
        bdmf_session_print(session, "route = %u (0x%x)\n", route, route);
        bdmf_session_print(session, "dest = %u (0x%x)\n", dest, dest);
        bdmf_session_print(session, "en = %u (0x%x)\n", en, en);
        break;
    }
    case cli_bbh_tx_common_configurations_q2rnr:
    {
        bdmf_boolean q0;
        bdmf_boolean q1;
        err = ag_drv_bbh_tx_common_configurations_q2rnr_get(parm[1].value.unumber, parm[2].value.unumber, &q0, &q1);
        bdmf_session_print(session, "q0 = %u (0x%x)\n", q0, q0);
        bdmf_session_print(session, "q1 = %u (0x%x)\n", q1, q1);
        break;
    }
    case cli_bbh_tx_common_configurations_perqtask:
    {
        bbh_tx_common_configurations_perqtask common_configurations_perqtask;
        err = ag_drv_bbh_tx_common_configurations_perqtask_get(parm[1].value.unumber, &common_configurations_perqtask);
        bdmf_session_print(session, "task0 = %u (0x%x)\n", common_configurations_perqtask.task0, common_configurations_perqtask.task0);
        bdmf_session_print(session, "task1 = %u (0x%x)\n", common_configurations_perqtask.task1, common_configurations_perqtask.task1);
        bdmf_session_print(session, "task2 = %u (0x%x)\n", common_configurations_perqtask.task2, common_configurations_perqtask.task2);
        bdmf_session_print(session, "task3 = %u (0x%x)\n", common_configurations_perqtask.task3, common_configurations_perqtask.task3);
        bdmf_session_print(session, "task4 = %u (0x%x)\n", common_configurations_perqtask.task4, common_configurations_perqtask.task4);
        bdmf_session_print(session, "task5 = %u (0x%x)\n", common_configurations_perqtask.task5, common_configurations_perqtask.task5);
        bdmf_session_print(session, "task6 = %u (0x%x)\n", common_configurations_perqtask.task6, common_configurations_perqtask.task6);
        bdmf_session_print(session, "task7 = %u (0x%x)\n", common_configurations_perqtask.task7, common_configurations_perqtask.task7);
        break;
    }
    case cli_bbh_tx_common_configurations_txrstcmd:
    {
        bbh_tx_common_configurations_txrstcmd common_configurations_txrstcmd;
        err = ag_drv_bbh_tx_common_configurations_txrstcmd_get(parm[1].value.unumber, &common_configurations_txrstcmd);
        bdmf_session_print(session, "cntxtrst = %u (0x%x)\n", common_configurations_txrstcmd.cntxtrst, common_configurations_txrstcmd.cntxtrst);
        bdmf_session_print(session, "pdfiforst = %u (0x%x)\n", common_configurations_txrstcmd.pdfiforst, common_configurations_txrstcmd.pdfiforst);
        bdmf_session_print(session, "dmaptrrst = %u (0x%x)\n", common_configurations_txrstcmd.dmaptrrst, common_configurations_txrstcmd.dmaptrrst);
        bdmf_session_print(session, "sdmaptrrst = %u (0x%x)\n", common_configurations_txrstcmd.sdmaptrrst, common_configurations_txrstcmd.sdmaptrrst);
        bdmf_session_print(session, "bpmfiforst = %u (0x%x)\n", common_configurations_txrstcmd.bpmfiforst, common_configurations_txrstcmd.bpmfiforst);
        bdmf_session_print(session, "sbpmfiforst = %u (0x%x)\n", common_configurations_txrstcmd.sbpmfiforst, common_configurations_txrstcmd.sbpmfiforst);
        bdmf_session_print(session, "okfiforst = %u (0x%x)\n", common_configurations_txrstcmd.okfiforst, common_configurations_txrstcmd.okfiforst);
        bdmf_session_print(session, "ddrfiforst = %u (0x%x)\n", common_configurations_txrstcmd.ddrfiforst, common_configurations_txrstcmd.ddrfiforst);
        bdmf_session_print(session, "sramfiforst = %u (0x%x)\n", common_configurations_txrstcmd.sramfiforst, common_configurations_txrstcmd.sramfiforst);
        bdmf_session_print(session, "skbptrrst = %u (0x%x)\n", common_configurations_txrstcmd.skbptrrst, common_configurations_txrstcmd.skbptrrst);
        bdmf_session_print(session, "stsfiforst = %u (0x%x)\n", common_configurations_txrstcmd.stsfiforst, common_configurations_txrstcmd.stsfiforst);
        bdmf_session_print(session, "reqfiforst = %u (0x%x)\n", common_configurations_txrstcmd.reqfiforst, common_configurations_txrstcmd.reqfiforst);
        bdmf_session_print(session, "msgfiforst = %u (0x%x)\n", common_configurations_txrstcmd.msgfiforst, common_configurations_txrstcmd.msgfiforst);
        bdmf_session_print(session, "gnxtfiforst = %u (0x%x)\n", common_configurations_txrstcmd.gnxtfiforst, common_configurations_txrstcmd.gnxtfiforst);
        bdmf_session_print(session, "fbnfiforst = %u (0x%x)\n", common_configurations_txrstcmd.fbnfiforst, common_configurations_txrstcmd.fbnfiforst);
        break;
    }
    case cli_bbh_tx_common_configurations_dbgsel:
    {
        uint8_t dbgsel;
        err = ag_drv_bbh_tx_common_configurations_dbgsel_get(parm[1].value.unumber, &dbgsel);
        bdmf_session_print(session, "dbgsel = %u (0x%x)\n", dbgsel, dbgsel);
        break;
    }
    case cli_bbh_tx_common_configurations_clk_gate_cntrl:
    {
        bbh_tx_common_configurations_clk_gate_cntrl common_configurations_clk_gate_cntrl;
        err = ag_drv_bbh_tx_common_configurations_clk_gate_cntrl_get(parm[1].value.unumber, &common_configurations_clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u (0x%x)\n", common_configurations_clk_gate_cntrl.bypass_clk_gate, common_configurations_clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u (0x%x)\n", common_configurations_clk_gate_cntrl.timer_val, common_configurations_clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u (0x%x)\n", common_configurations_clk_gate_cntrl.keep_alive_en, common_configurations_clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intrvl = %u (0x%x)\n", common_configurations_clk_gate_cntrl.keep_alive_intrvl, common_configurations_clk_gate_cntrl.keep_alive_intrvl);
        bdmf_session_print(session, "keep_alive_cyc = %u (0x%x)\n", common_configurations_clk_gate_cntrl.keep_alive_cyc, common_configurations_clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    case cli_bbh_tx_wan_configurations_pdbase:
    {
        uint16_t fifobase0;
        uint16_t fifobase1;
        err = ag_drv_bbh_tx_wan_configurations_pdbase_get(parm[1].value.unumber, parm[2].value.unumber, &fifobase0, &fifobase1);
        bdmf_session_print(session, "fifobase0 = %u (0x%x)\n", fifobase0, fifobase0);
        bdmf_session_print(session, "fifobase1 = %u (0x%x)\n", fifobase1, fifobase1);
        break;
    }
    case cli_bbh_tx_wan_configurations_pdsize:
    {
        uint16_t fifosize0;
        uint16_t fifosize1;
        err = ag_drv_bbh_tx_wan_configurations_pdsize_get(parm[1].value.unumber, parm[2].value.unumber, &fifosize0, &fifosize1);
        bdmf_session_print(session, "fifosize0 = %u (0x%x)\n", fifosize0, fifosize0);
        bdmf_session_print(session, "fifosize1 = %u (0x%x)\n", fifosize1, fifosize1);
        break;
    }
    case cli_bbh_tx_wan_configurations_pdwkuph:
    {
        uint8_t wkupthresh0;
        uint8_t wkupthresh1;
        err = ag_drv_bbh_tx_wan_configurations_pdwkuph_get(parm[1].value.unumber, parm[2].value.unumber, &wkupthresh0, &wkupthresh1);
        bdmf_session_print(session, "wkupthresh0 = %u (0x%x)\n", wkupthresh0, wkupthresh0);
        bdmf_session_print(session, "wkupthresh1 = %u (0x%x)\n", wkupthresh1, wkupthresh1);
        break;
    }
    case cli_bbh_tx_wan_configurations_pd_byte_th:
    {
        uint16_t pdlimit0;
        uint16_t pdlimit1;
        err = ag_drv_bbh_tx_wan_configurations_pd_byte_th_get(parm[1].value.unumber, parm[2].value.unumber, &pdlimit0, &pdlimit1);
        bdmf_session_print(session, "pdlimit0 = %u (0x%x)\n", pdlimit0, pdlimit0);
        bdmf_session_print(session, "pdlimit1 = %u (0x%x)\n", pdlimit1, pdlimit1);
        break;
    }
    case cli_bbh_tx_wan_configurations_pd_byte_th_en:
    {
        bdmf_boolean pdlimiten;
        err = ag_drv_bbh_tx_wan_configurations_pd_byte_th_en_get(parm[1].value.unumber, &pdlimiten);
        bdmf_session_print(session, "pdlimiten = %u (0x%x)\n", pdlimiten, pdlimiten);
        break;
    }
    case cli_bbh_tx_wan_configurations_pdempty:
    {
        uint8_t empty;
        err = ag_drv_bbh_tx_wan_configurations_pdempty_get(parm[1].value.unumber, &empty);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        break;
    }
    case cli_bbh_tx_wan_configurations_stsrnrcfg_1:
    {
        uint16_t tcontaddr;
        err = ag_drv_bbh_tx_wan_configurations_stsrnrcfg_1_get(parm[1].value.unumber, parm[2].value.unumber, &tcontaddr);
        bdmf_session_print(session, "tcontaddr = %u (0x%x)\n", tcontaddr, tcontaddr);
        break;
    }
    case cli_bbh_tx_wan_configurations_stsrnrcfg_2:
    {
        uint16_t ptraddr;
        uint8_t task;
        err = ag_drv_bbh_tx_wan_configurations_stsrnrcfg_2_get(parm[1].value.unumber, parm[2].value.unumber, &ptraddr, &task);
        bdmf_session_print(session, "ptraddr = %u (0x%x)\n", ptraddr, ptraddr);
        bdmf_session_print(session, "task = %u (0x%x)\n", task, task);
        break;
    }
    case cli_bbh_tx_wan_configurations_msgrnrcfg_1:
    {
        uint16_t tcontaddr;
        err = ag_drv_bbh_tx_wan_configurations_msgrnrcfg_1_get(parm[1].value.unumber, parm[2].value.unumber, &tcontaddr);
        bdmf_session_print(session, "tcontaddr = %u (0x%x)\n", tcontaddr, tcontaddr);
        break;
    }
    case cli_bbh_tx_wan_configurations_msgrnrcfg_2:
    {
        uint16_t ptraddr;
        uint8_t task;
        err = ag_drv_bbh_tx_wan_configurations_msgrnrcfg_2_get(parm[1].value.unumber, parm[2].value.unumber, &ptraddr, &task);
        bdmf_session_print(session, "ptraddr = %u (0x%x)\n", ptraddr, ptraddr);
        bdmf_session_print(session, "task = %u (0x%x)\n", task, task);
        break;
    }
    case cli_bbh_tx_wan_configurations_epncfg:
    {
        bdmf_boolean stplenerr;
        bdmf_boolean cmp_width;
        bdmf_boolean considerfull;
        bdmf_boolean addcrc;
        err = ag_drv_bbh_tx_wan_configurations_epncfg_get(parm[1].value.unumber, &stplenerr, &cmp_width, &considerfull, &addcrc);
        bdmf_session_print(session, "stplenerr = %u (0x%x)\n", stplenerr, stplenerr);
        bdmf_session_print(session, "cmp_width = %u (0x%x)\n", cmp_width, cmp_width);
        bdmf_session_print(session, "considerfull = %u (0x%x)\n", considerfull, considerfull);
        bdmf_session_print(session, "addcrc = %u (0x%x)\n", addcrc, addcrc);
        break;
    }
    case cli_bbh_tx_wan_configurations_flow2port:
    {
        uint32_t wdata;
        uint8_t a;
        bdmf_boolean cmd;
        err = ag_drv_bbh_tx_wan_configurations_flow2port_get(parm[1].value.unumber, &wdata, &a, &cmd);
        bdmf_session_print(session, "wdata = %u (0x%x)\n", wdata, wdata);
        bdmf_session_print(session, "a = %u (0x%x)\n", a, a);
        bdmf_session_print(session, "cmd = %u (0x%x)\n", cmd, cmd);
        break;
    }
    case cli_bbh_tx_wan_configurations_ts:
    {
        bdmf_boolean en;
        err = ag_drv_bbh_tx_wan_configurations_ts_get(parm[1].value.unumber, &en);
        bdmf_session_print(session, "en = %u (0x%x)\n", en, en);
        break;
    }
    case cli_bbh_tx_wan_configurations_maxwlen:
    {
        uint16_t maxwlen;
        err = ag_drv_bbh_tx_wan_configurations_maxwlen_get(parm[1].value.unumber, &maxwlen);
        bdmf_session_print(session, "maxwlen = %u (0x%x)\n", maxwlen, maxwlen);
        break;
    }
    case cli_bbh_tx_wan_configurations_flush:
    {
        uint16_t flush;
        bdmf_boolean srst_n;
        err = ag_drv_bbh_tx_wan_configurations_flush_get(parm[1].value.unumber, &flush, &srst_n);
        bdmf_session_print(session, "flush = %u (0x%x)\n", flush, flush);
        bdmf_session_print(session, "srst_n = %u (0x%x)\n", srst_n, srst_n);
        break;
    }
    case cli_bbh_tx_lan_configurations_pdbase:
    {
        uint16_t fifobase0;
        uint16_t fifobase1;
        err = ag_drv_bbh_tx_lan_configurations_pdbase_get(parm[1].value.unumber, &fifobase0, &fifobase1);
        bdmf_session_print(session, "fifobase0 = %u (0x%x)\n", fifobase0, fifobase0);
        bdmf_session_print(session, "fifobase1 = %u (0x%x)\n", fifobase1, fifobase1);
        break;
    }
    case cli_bbh_tx_lan_configurations_pdsize:
    {
        uint16_t fifosize0;
        uint16_t fifosize1;
        err = ag_drv_bbh_tx_lan_configurations_pdsize_get(parm[1].value.unumber, &fifosize0, &fifosize1);
        bdmf_session_print(session, "fifosize0 = %u (0x%x)\n", fifosize0, fifosize0);
        bdmf_session_print(session, "fifosize1 = %u (0x%x)\n", fifosize1, fifosize1);
        break;
    }
    case cli_bbh_tx_lan_configurations_pdwkuph:
    {
        uint8_t wkupthresh0;
        uint8_t wkupthresh1;
        err = ag_drv_bbh_tx_lan_configurations_pdwkuph_get(parm[1].value.unumber, &wkupthresh0, &wkupthresh1);
        bdmf_session_print(session, "wkupthresh0 = %u (0x%x)\n", wkupthresh0, wkupthresh0);
        bdmf_session_print(session, "wkupthresh1 = %u (0x%x)\n", wkupthresh1, wkupthresh1);
        break;
    }
    case cli_bbh_tx_lan_configurations_pd_byte_th:
    {
        uint16_t pdlimit0;
        uint16_t pdlimit1;
        err = ag_drv_bbh_tx_lan_configurations_pd_byte_th_get(parm[1].value.unumber, &pdlimit0, &pdlimit1);
        bdmf_session_print(session, "pdlimit0 = %u (0x%x)\n", pdlimit0, pdlimit0);
        bdmf_session_print(session, "pdlimit1 = %u (0x%x)\n", pdlimit1, pdlimit1);
        break;
    }
    case cli_bbh_tx_lan_configurations_pd_byte_th_en:
    {
        bdmf_boolean pdlimiten;
        err = ag_drv_bbh_tx_lan_configurations_pd_byte_th_en_get(parm[1].value.unumber, &pdlimiten);
        bdmf_session_print(session, "pdlimiten = %u (0x%x)\n", pdlimiten, pdlimiten);
        break;
    }
    case cli_bbh_tx_lan_configurations_pdempty:
    {
        uint8_t empty;
        err = ag_drv_bbh_tx_lan_configurations_pdempty_get(parm[1].value.unumber, &empty);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        break;
    }
    case cli_bbh_tx_lan_configurations_txthresh:
    {
        uint16_t ddrthresh;
        uint16_t sramthresh;
        err = ag_drv_bbh_tx_lan_configurations_txthresh_get(parm[1].value.unumber, &ddrthresh, &sramthresh);
        bdmf_session_print(session, "ddrthresh = %u (0x%x)\n", ddrthresh, ddrthresh);
        bdmf_session_print(session, "sramthresh = %u (0x%x)\n", sramthresh, sramthresh);
        break;
    }
    case cli_bbh_tx_lan_configurations_eee:
    {
        bdmf_boolean en;
        err = ag_drv_bbh_tx_lan_configurations_eee_get(parm[1].value.unumber, &en);
        bdmf_session_print(session, "en = %u (0x%x)\n", en, en);
        break;
    }
    case cli_bbh_tx_lan_configurations_ts:
    {
        bdmf_boolean en;
        err = ag_drv_bbh_tx_lan_configurations_ts_get(parm[1].value.unumber, &en);
        bdmf_session_print(session, "en = %u (0x%x)\n", en, en);
        break;
    }
    case cli_bbh_tx_unified_configurations_pdbase:
    {
        uint16_t fifobase0;
        uint16_t fifobase1;
        err = ag_drv_bbh_tx_unified_configurations_pdbase_get(parm[1].value.unumber, parm[2].value.unumber, &fifobase0, &fifobase1);
        bdmf_session_print(session, "fifobase0 = %u (0x%x)\n", fifobase0, fifobase0);
        bdmf_session_print(session, "fifobase1 = %u (0x%x)\n", fifobase1, fifobase1);
        break;
    }
    case cli_bbh_tx_unified_configurations_pdsize:
    {
        uint16_t fifosize0;
        uint16_t fifosize1;
        err = ag_drv_bbh_tx_unified_configurations_pdsize_get(parm[1].value.unumber, parm[2].value.unumber, &fifosize0, &fifosize1);
        bdmf_session_print(session, "fifosize0 = %u (0x%x)\n", fifosize0, fifosize0);
        bdmf_session_print(session, "fifosize1 = %u (0x%x)\n", fifosize1, fifosize1);
        break;
    }
    case cli_bbh_tx_unified_configurations_pdwkuph:
    {
        uint8_t wkupthresh0;
        uint8_t wkupthresh1;
        err = ag_drv_bbh_tx_unified_configurations_pdwkuph_get(parm[1].value.unumber, parm[2].value.unumber, &wkupthresh0, &wkupthresh1);
        bdmf_session_print(session, "wkupthresh0 = %u (0x%x)\n", wkupthresh0, wkupthresh0);
        bdmf_session_print(session, "wkupthresh1 = %u (0x%x)\n", wkupthresh1, wkupthresh1);
        break;
    }
    case cli_bbh_tx_unified_configurations_pd_byte_th:
    {
        uint16_t pdlimit0;
        uint16_t pdlimit1;
        err = ag_drv_bbh_tx_unified_configurations_pd_byte_th_get(parm[1].value.unumber, parm[2].value.unumber, &pdlimit0, &pdlimit1);
        bdmf_session_print(session, "pdlimit0 = %u (0x%x)\n", pdlimit0, pdlimit0);
        bdmf_session_print(session, "pdlimit1 = %u (0x%x)\n", pdlimit1, pdlimit1);
        break;
    }
    case cli_bbh_tx_unified_configurations_pd_byte_th_en:
    {
        bdmf_boolean pdlimiten;
        err = ag_drv_bbh_tx_unified_configurations_pd_byte_th_en_get(parm[1].value.unumber, &pdlimiten);
        bdmf_session_print(session, "pdlimiten = %u (0x%x)\n", pdlimiten, pdlimiten);
        break;
    }
    case cli_bbh_tx_unified_configurations_pdempty:
    {
        uint8_t empty;
        err = ag_drv_bbh_tx_unified_configurations_pdempty_get(parm[1].value.unumber, &empty);
        bdmf_session_print(session, "empty = %u (0x%x)\n", empty, empty);
        break;
    }
    case cli_bbh_tx_unified_configurations_gtxthresh:
    {
        uint16_t ddrthresh;
        uint16_t sramthresh;
        err = ag_drv_bbh_tx_unified_configurations_gtxthresh_get(parm[1].value.unumber, &ddrthresh, &sramthresh);
        bdmf_session_print(session, "ddrthresh = %u (0x%x)\n", ddrthresh, ddrthresh);
        bdmf_session_print(session, "sramthresh = %u (0x%x)\n", sramthresh, sramthresh);
        break;
    }
    case cli_bbh_tx_unified_configurations_eee:
    {
        uint8_t en;
        err = ag_drv_bbh_tx_unified_configurations_eee_get(parm[1].value.unumber, &en);
        bdmf_session_print(session, "en = %u (0x%x)\n", en, en);
        break;
    }
    case cli_bbh_tx_unified_configurations_ts:
    {
        uint8_t en;
        err = ag_drv_bbh_tx_unified_configurations_ts_get(parm[1].value.unumber, &en);
        bdmf_session_print(session, "en = %u (0x%x)\n", en, en);
        break;
    }
    case cli_bbh_tx_unified_configurations_febase:
    {
        uint16_t fifobase0;
        uint16_t fifobase1;
        err = ag_drv_bbh_tx_unified_configurations_febase_get(parm[1].value.unumber, parm[2].value.unumber, &fifobase0, &fifobase1);
        bdmf_session_print(session, "fifobase0 = %u (0x%x)\n", fifobase0, fifobase0);
        bdmf_session_print(session, "fifobase1 = %u (0x%x)\n", fifobase1, fifobase1);
        break;
    }
    case cli_bbh_tx_unified_configurations_fesize:
    {
        uint16_t fifosize0;
        uint16_t fifosize1;
        err = ag_drv_bbh_tx_unified_configurations_fesize_get(parm[1].value.unumber, parm[2].value.unumber, &fifosize0, &fifosize1);
        bdmf_session_print(session, "fifosize0 = %u (0x%x)\n", fifosize0, fifosize0);
        bdmf_session_print(session, "fifosize1 = %u (0x%x)\n", fifosize1, fifosize1);
        break;
    }
    case cli_bbh_tx_unified_configurations_fepdbase:
    {
        uint8_t fifobase0;
        uint8_t fifobase1;
        err = ag_drv_bbh_tx_unified_configurations_fepdbase_get(parm[1].value.unumber, parm[2].value.unumber, &fifobase0, &fifobase1);
        bdmf_session_print(session, "fifobase0 = %u (0x%x)\n", fifobase0, fifobase0);
        bdmf_session_print(session, "fifobase1 = %u (0x%x)\n", fifobase1, fifobase1);
        break;
    }
    case cli_bbh_tx_unified_configurations_fepdsize:
    {
        uint8_t fifosize0;
        uint8_t fifosize1;
        err = ag_drv_bbh_tx_unified_configurations_fepdsize_get(parm[1].value.unumber, parm[2].value.unumber, &fifosize0, &fifosize1);
        bdmf_session_print(session, "fifosize0 = %u (0x%x)\n", fifosize0, fifosize0);
        bdmf_session_print(session, "fifosize1 = %u (0x%x)\n", fifosize1, fifosize1);
        break;
    }
    case cli_bbh_tx_unified_configurations_txwrr:
    {
        uint8_t w0;
        uint8_t w1;
        err = ag_drv_bbh_tx_unified_configurations_txwrr_get(parm[1].value.unumber, parm[2].value.unumber, &w0, &w1);
        bdmf_session_print(session, "w0 = %u (0x%x)\n", w0, w0);
        bdmf_session_print(session, "w1 = %u (0x%x)\n", w1, w1);
        break;
    }
    case cli_bbh_tx_unified_configurations_txthresh:
    {
        uint16_t thresh0;
        uint16_t thresh1;
        err = ag_drv_bbh_tx_unified_configurations_txthresh_get(parm[1].value.unumber, parm[2].value.unumber, &thresh0, &thresh1);
        bdmf_session_print(session, "thresh0 = %u (0x%x)\n", thresh0, thresh0);
        bdmf_session_print(session, "thresh1 = %u (0x%x)\n", thresh1, thresh1);
        break;
    }
    case cli_bbh_tx_debug_counters_srambyte:
    {
        uint32_t srambyte;
        err = ag_drv_bbh_tx_debug_counters_srambyte_get(parm[1].value.unumber, &srambyte);
        bdmf_session_print(session, "srambyte = %u (0x%x)\n", srambyte, srambyte);
        break;
    }
    case cli_bbh_tx_debug_counters_ddrbyte:
    {
        uint32_t ddrbyte;
        err = ag_drv_bbh_tx_debug_counters_ddrbyte_get(parm[1].value.unumber, &ddrbyte);
        bdmf_session_print(session, "ddrbyte = %u (0x%x)\n", ddrbyte, ddrbyte);
        break;
    }
    case cli_bbh_tx_debug_counters_swrden:
    {
        bbh_tx_debug_counters_swrden debug_counters_swrden;
        err = ag_drv_bbh_tx_debug_counters_swrden_get(parm[1].value.unumber, &debug_counters_swrden);
        bdmf_session_print(session, "pdsel = %u (0x%x)\n", debug_counters_swrden.pdsel, debug_counters_swrden.pdsel);
        bdmf_session_print(session, "pdvsel = %u (0x%x)\n", debug_counters_swrden.pdvsel, debug_counters_swrden.pdvsel);
        bdmf_session_print(session, "pdemptysel = %u (0x%x)\n", debug_counters_swrden.pdemptysel, debug_counters_swrden.pdemptysel);
        bdmf_session_print(session, "pdfullsel = %u (0x%x)\n", debug_counters_swrden.pdfullsel, debug_counters_swrden.pdfullsel);
        bdmf_session_print(session, "pdbemptysel = %u (0x%x)\n", debug_counters_swrden.pdbemptysel, debug_counters_swrden.pdbemptysel);
        bdmf_session_print(session, "pdffwkpsel = %u (0x%x)\n", debug_counters_swrden.pdffwkpsel, debug_counters_swrden.pdffwkpsel);
        bdmf_session_print(session, "fbnsel = %u (0x%x)\n", debug_counters_swrden.fbnsel, debug_counters_swrden.fbnsel);
        bdmf_session_print(session, "fbnvsel = %u (0x%x)\n", debug_counters_swrden.fbnvsel, debug_counters_swrden.fbnvsel);
        bdmf_session_print(session, "fbnemptysel = %u (0x%x)\n", debug_counters_swrden.fbnemptysel, debug_counters_swrden.fbnemptysel);
        bdmf_session_print(session, "fbnfullsel = %u (0x%x)\n", debug_counters_swrden.fbnfullsel, debug_counters_swrden.fbnfullsel);
        bdmf_session_print(session, "getnextsel = %u (0x%x)\n", debug_counters_swrden.getnextsel, debug_counters_swrden.getnextsel);
        bdmf_session_print(session, "getnextvsel = %u (0x%x)\n", debug_counters_swrden.getnextvsel, debug_counters_swrden.getnextvsel);
        bdmf_session_print(session, "getnextemptysel = %u (0x%x)\n", debug_counters_swrden.getnextemptysel, debug_counters_swrden.getnextemptysel);
        bdmf_session_print(session, "getnextfullsel = %u (0x%x)\n", debug_counters_swrden.getnextfullsel, debug_counters_swrden.getnextfullsel);
        bdmf_session_print(session, "gpncntxtsel = %u (0x%x)\n", debug_counters_swrden.gpncntxtsel, debug_counters_swrden.gpncntxtsel);
        bdmf_session_print(session, "bpmsel = %u (0x%x)\n", debug_counters_swrden.bpmsel, debug_counters_swrden.bpmsel);
        bdmf_session_print(session, "bpmfsel = %u (0x%x)\n", debug_counters_swrden.bpmfsel, debug_counters_swrden.bpmfsel);
        bdmf_session_print(session, "sbpmsel = %u (0x%x)\n", debug_counters_swrden.sbpmsel, debug_counters_swrden.sbpmsel);
        bdmf_session_print(session, "sbpmfsel = %u (0x%x)\n", debug_counters_swrden.sbpmfsel, debug_counters_swrden.sbpmfsel);
        bdmf_session_print(session, "stssel = %u (0x%x)\n", debug_counters_swrden.stssel, debug_counters_swrden.stssel);
        bdmf_session_print(session, "stsvsel = %u (0x%x)\n", debug_counters_swrden.stsvsel, debug_counters_swrden.stsvsel);
        bdmf_session_print(session, "stsemptysel = %u (0x%x)\n", debug_counters_swrden.stsemptysel, debug_counters_swrden.stsemptysel);
        bdmf_session_print(session, "stsfullsel = %u (0x%x)\n", debug_counters_swrden.stsfullsel, debug_counters_swrden.stsfullsel);
        bdmf_session_print(session, "stsbemptysel = %u (0x%x)\n", debug_counters_swrden.stsbemptysel, debug_counters_swrden.stsbemptysel);
        bdmf_session_print(session, "stsffwkpsel = %u (0x%x)\n", debug_counters_swrden.stsffwkpsel, debug_counters_swrden.stsffwkpsel);
        bdmf_session_print(session, "msgsel = %u (0x%x)\n", debug_counters_swrden.msgsel, debug_counters_swrden.msgsel);
        bdmf_session_print(session, "msgvsel = %u (0x%x)\n", debug_counters_swrden.msgvsel, debug_counters_swrden.msgvsel);
        bdmf_session_print(session, "epnreqsel = %u (0x%x)\n", debug_counters_swrden.epnreqsel, debug_counters_swrden.epnreqsel);
        bdmf_session_print(session, "datasel = %u (0x%x)\n", debug_counters_swrden.datasel, debug_counters_swrden.datasel);
        bdmf_session_print(session, "reordersel = %u (0x%x)\n", debug_counters_swrden.reordersel, debug_counters_swrden.reordersel);
        bdmf_session_print(session, "tsinfosel = %u (0x%x)\n", debug_counters_swrden.tsinfosel, debug_counters_swrden.tsinfosel);
        bdmf_session_print(session, "mactxsel = %u (0x%x)\n", debug_counters_swrden.mactxsel, debug_counters_swrden.mactxsel);
        break;
    }
    case cli_bbh_tx_debug_counters_swrdaddr:
    {
        uint16_t rdaddr;
        err = ag_drv_bbh_tx_debug_counters_swrdaddr_get(parm[1].value.unumber, &rdaddr);
        bdmf_session_print(session, "rdaddr = %u (0x%x)\n", rdaddr, rdaddr);
        break;
    }
    case cli_bbh_tx_debug_counters_swrddata:
    {
        uint32_t data;
        err = ag_drv_bbh_tx_debug_counters_swrddata_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_bbh_tx_debug_counters_unifiedpkt:
    {
        uint32_t ddrbyte;
        err = ag_drv_bbh_tx_debug_counters_unifiedpkt_get(parm[1].value.unumber, parm[2].value.unumber, &ddrbyte);
        bdmf_session_print(session, "ddrbyte = %u (0x%x)\n", ddrbyte, ddrbyte);
        break;
    }
    case cli_bbh_tx_debug_counters_unifiedbyte:
    {
        uint32_t ddrbyte;
        err = ag_drv_bbh_tx_debug_counters_unifiedbyte_get(parm[1].value.unumber, parm[2].value.unumber, &ddrbyte);
        bdmf_session_print(session, "ddrbyte = %u (0x%x)\n", ddrbyte, ddrbyte);
        break;
    }
    case cli_bbh_tx_debug_counters_dbgoutreg:
    {
        bbh_tx_debug_counters_dbgoutreg debug_counters_dbgoutreg;
        err = ag_drv_bbh_tx_debug_counters_dbgoutreg_get(parm[1].value.unumber, parm[2].value.unumber, &debug_counters_dbgoutreg);
        bdmf_session_print(session, "debug_out_reg[0] = %u (0x%x)\n", debug_counters_dbgoutreg.debug_out_reg[0], debug_counters_dbgoutreg.debug_out_reg[0]);
        bdmf_session_print(session, "debug_out_reg[1] = %u (0x%x)\n", debug_counters_dbgoutreg.debug_out_reg[1], debug_counters_dbgoutreg.debug_out_reg[1]);
        bdmf_session_print(session, "debug_out_reg[2] = %u (0x%x)\n", debug_counters_dbgoutreg.debug_out_reg[2], debug_counters_dbgoutreg.debug_out_reg[2]);
        bdmf_session_print(session, "debug_out_reg[3] = %u (0x%x)\n", debug_counters_dbgoutreg.debug_out_reg[3], debug_counters_dbgoutreg.debug_out_reg[3]);
        bdmf_session_print(session, "debug_out_reg[4] = %u (0x%x)\n", debug_counters_dbgoutreg.debug_out_reg[4], debug_counters_dbgoutreg.debug_out_reg[4]);
        bdmf_session_print(session, "debug_out_reg[5] = %u (0x%x)\n", debug_counters_dbgoutreg.debug_out_reg[5], debug_counters_dbgoutreg.debug_out_reg[5]);
        bdmf_session_print(session, "debug_out_reg[6] = %u (0x%x)\n", debug_counters_dbgoutreg.debug_out_reg[6], debug_counters_dbgoutreg.debug_out_reg[6]);
        bdmf_session_print(session, "debug_out_reg[7] = %u (0x%x)\n", debug_counters_dbgoutreg.debug_out_reg[7], debug_counters_dbgoutreg.debug_out_reg[7]);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_bbh_tx_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t bbh_id = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        uint8_t type=gtmv(m, 3);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_mac_type_set(%u %u)\n", bbh_id, type);
        if(!err) ag_drv_bbh_tx_mac_type_set(bbh_id, type);
        if(!err) ag_drv_bbh_tx_mac_type_get( bbh_id, &type);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_mac_type_get(%u %u)\n", bbh_id, type);
        if(err || type!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bbh_tx_cfg_src_id cfg_src_id = {.fpmsrc=gtmv(m, 6), .sbpmsrc=gtmv(m, 6), .stsrnrsrc=gtmv(m, 6), .msgrnrsrc=gtmv(m, 6)};
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_cfg_src_id_set(%u %u %u %u %u)\n", bbh_id, cfg_src_id.fpmsrc, cfg_src_id.sbpmsrc, cfg_src_id.stsrnrsrc, cfg_src_id.msgrnrsrc);
        if(!err) ag_drv_bbh_tx_cfg_src_id_set(bbh_id, &cfg_src_id);
        if(!err) ag_drv_bbh_tx_cfg_src_id_get( bbh_id, &cfg_src_id);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_cfg_src_id_get(%u %u %u %u %u)\n", bbh_id, cfg_src_id.fpmsrc, cfg_src_id.sbpmsrc, cfg_src_id.stsrnrsrc, cfg_src_id.msgrnrsrc);
        if(err || cfg_src_id.fpmsrc!=gtmv(m, 6) || cfg_src_id.sbpmsrc!=gtmv(m, 6) || cfg_src_id.stsrnrsrc!=gtmv(m, 6) || cfg_src_id.msgrnrsrc!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t pdrnr0src=gtmv(m, 6);
        uint8_t pdrnr1src=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_rnr_src_id_set(%u %u %u)\n", bbh_id, pdrnr0src, pdrnr1src);
        if(!err) ag_drv_bbh_tx_rnr_src_id_set(bbh_id, pdrnr0src, pdrnr1src);
        if(!err) ag_drv_bbh_tx_rnr_src_id_get( bbh_id, &pdrnr0src, &pdrnr1src);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_rnr_src_id_get(%u %u %u)\n", bbh_id, pdrnr0src, pdrnr1src);
        if(err || pdrnr0src!=gtmv(m, 6) || pdrnr1src!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bbh_tx_bbh_dma_cfg bbh_dma_cfg = {.dmasrc=gtmv(m, 6), .descbase=gtmv(m, 6), .descsize=gtmv(m, 6)};
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_bbh_dma_cfg_set(%u %u %u %u)\n", bbh_id, bbh_dma_cfg.dmasrc, bbh_dma_cfg.descbase, bbh_dma_cfg.descsize);
        if(!err) ag_drv_bbh_tx_bbh_dma_cfg_set(bbh_id, &bbh_dma_cfg);
        if(!err) ag_drv_bbh_tx_bbh_dma_cfg_get( bbh_id, &bbh_dma_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_bbh_dma_cfg_get(%u %u %u %u)\n", bbh_id, bbh_dma_cfg.dmasrc, bbh_dma_cfg.descbase, bbh_dma_cfg.descsize);
        if(err || bbh_dma_cfg.dmasrc!=gtmv(m, 6) || bbh_dma_cfg.descbase!=gtmv(m, 6) || bbh_dma_cfg.descsize!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t maxreq=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_dma_max_otf_read_request_set(%u %u)\n", bbh_id, maxreq);
        if(!err) ag_drv_bbh_tx_dma_max_otf_read_request_set(bbh_id, maxreq);
        if(!err) ag_drv_bbh_tx_dma_max_otf_read_request_get( bbh_id, &maxreq);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_dma_max_otf_read_request_get(%u %u)\n", bbh_id, maxreq);
        if(err || maxreq!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean epnurgnt=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_dma_epon_urgent_set(%u %u)\n", bbh_id, epnurgnt);
        if(!err) ag_drv_bbh_tx_dma_epon_urgent_set(bbh_id, epnurgnt);
        if(!err) ag_drv_bbh_tx_dma_epon_urgent_get( bbh_id, &epnurgnt);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_dma_epon_urgent_get(%u %u)\n", bbh_id, epnurgnt);
        if(err || epnurgnt!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bbh_tx_bbh_sdma_cfg bbh_sdma_cfg = {.sdmasrc=gtmv(m, 6), .descbase=gtmv(m, 6), .descsize=gtmv(m, 6)};
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_bbh_sdma_cfg_set(%u %u %u %u)\n", bbh_id, bbh_sdma_cfg.sdmasrc, bbh_sdma_cfg.descbase, bbh_sdma_cfg.descsize);
        if(!err) ag_drv_bbh_tx_bbh_sdma_cfg_set(bbh_id, &bbh_sdma_cfg);
        if(!err) ag_drv_bbh_tx_bbh_sdma_cfg_get( bbh_id, &bbh_sdma_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_bbh_sdma_cfg_get(%u %u %u %u)\n", bbh_id, bbh_sdma_cfg.sdmasrc, bbh_sdma_cfg.descbase, bbh_sdma_cfg.descsize);
        if(err || bbh_sdma_cfg.sdmasrc!=gtmv(m, 6) || bbh_sdma_cfg.descbase!=gtmv(m, 6) || bbh_sdma_cfg.descsize!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t maxreq=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_sdma_max_otf_read_request_set(%u %u)\n", bbh_id, maxreq);
        if(!err) ag_drv_bbh_tx_sdma_max_otf_read_request_set(bbh_id, maxreq);
        if(!err) ag_drv_bbh_tx_sdma_max_otf_read_request_get( bbh_id, &maxreq);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_sdma_max_otf_read_request_get(%u %u)\n", bbh_id, maxreq);
        if(err || maxreq!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean epnurgnt=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_sdma_epon_urgent_set(%u %u)\n", bbh_id, epnurgnt);
        if(!err) ag_drv_bbh_tx_sdma_epon_urgent_set(bbh_id, epnurgnt);
        if(!err) ag_drv_bbh_tx_sdma_epon_urgent_get( bbh_id, &epnurgnt);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_sdma_epon_urgent_get(%u %u)\n", bbh_id, epnurgnt);
        if(err || epnurgnt!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bbh_tx_bbh_ddr_cfg bbh_ddr_cfg = {.bufsize=gtmv(m, 3), .byteresul=gtmv(m, 1), .ddrtxoffset=gtmv(m, 9), .hnsize0=gtmv(m, 7), .hnsize1=gtmv(m, 7)};
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_bbh_ddr_cfg_set(%u %u %u %u %u %u)\n", bbh_id, bbh_ddr_cfg.bufsize, bbh_ddr_cfg.byteresul, bbh_ddr_cfg.ddrtxoffset, bbh_ddr_cfg.hnsize0, bbh_ddr_cfg.hnsize1);
        if(!err) ag_drv_bbh_tx_bbh_ddr_cfg_set(bbh_id, &bbh_ddr_cfg);
        if(!err) ag_drv_bbh_tx_bbh_ddr_cfg_get( bbh_id, &bbh_ddr_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_bbh_ddr_cfg_get(%u %u %u %u %u %u)\n", bbh_id, bbh_ddr_cfg.bufsize, bbh_ddr_cfg.byteresul, bbh_ddr_cfg.ddrtxoffset, bbh_ddr_cfg.hnsize0, bbh_ddr_cfg.hnsize1);
        if(err || bbh_ddr_cfg.bufsize!=gtmv(m, 3) || bbh_ddr_cfg.byteresul!=gtmv(m, 1) || bbh_ddr_cfg.ddrtxoffset!=gtmv(m, 9) || bbh_ddr_cfg.hnsize0!=gtmv(m, 7) || bbh_ddr_cfg.hnsize1!=gtmv(m, 7))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bbh_tx_debug_counters debug_counters = {.srampd=gtmv(m, 32), .ddrpd=gtmv(m, 32), .pddrop=gtmv(m, 16), .stscnt=gtmv(m, 32), .stsdrop=gtmv(m, 16), .msgcnt=gtmv(m, 32), .msgdrop=gtmv(m, 16), .getnextnull=gtmv(m, 16), .lenerr=gtmv(m, 16), .aggrlenerr=gtmv(m, 16), .srampkt=gtmv(m, 32), .ddrpkt=gtmv(m, 32), .flshpkts=gtmv(m, 16)};
        if(!err) ag_drv_bbh_tx_debug_counters_get( bbh_id, &debug_counters);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_debug_counters_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", bbh_id, debug_counters.srampd, debug_counters.ddrpd, debug_counters.pddrop, debug_counters.stscnt, debug_counters.stsdrop, debug_counters.msgcnt, debug_counters.msgdrop, debug_counters.getnextnull, debug_counters.lenerr, debug_counters.aggrlenerr, debug_counters.srampkt, debug_counters.ddrpkt, debug_counters.flshpkts);
    }
    {
        uint8_t rnr_cfg_index_1=gtmv(m, 1);
        uint16_t tcontaddr=gtmv(m, 16);
        uint16_t skbaddr=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_rnrcfg_1_set(%u %u %u %u)\n", bbh_id, rnr_cfg_index_1, tcontaddr, skbaddr);
        if(!err) ag_drv_bbh_tx_common_configurations_rnrcfg_1_set(bbh_id, rnr_cfg_index_1, tcontaddr, skbaddr);
        if(!err) ag_drv_bbh_tx_common_configurations_rnrcfg_1_get( bbh_id, rnr_cfg_index_1, &tcontaddr, &skbaddr);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_rnrcfg_1_get(%u %u %u %u)\n", bbh_id, rnr_cfg_index_1, tcontaddr, skbaddr);
        if(err || tcontaddr!=gtmv(m, 16) || skbaddr!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t rnr_cfg_index_2=gtmv(m, 1);
        uint16_t ptraddr=gtmv(m, 16);
        uint8_t task=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_rnrcfg_2_set(%u %u %u %u)\n", bbh_id, rnr_cfg_index_2, ptraddr, task);
        if(!err) ag_drv_bbh_tx_common_configurations_rnrcfg_2_set(bbh_id, rnr_cfg_index_2, ptraddr, task);
        if(!err) ag_drv_bbh_tx_common_configurations_rnrcfg_2_get( bbh_id, rnr_cfg_index_2, &ptraddr, &task);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_rnrcfg_2_get(%u %u %u %u)\n", bbh_id, rnr_cfg_index_2, ptraddr, task);
        if(err || ptraddr!=gtmv(m, 16) || task!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean freenocntxt=gtmv(m, 1);
        bdmf_boolean specialfree=gtmv(m, 1);
        uint8_t maxgn=gtmv(m, 5);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_sbpmcfg_set(%u %u %u %u)\n", bbh_id, freenocntxt, specialfree, maxgn);
        if(!err) ag_drv_bbh_tx_common_configurations_sbpmcfg_set(bbh_id, freenocntxt, specialfree, maxgn);
        if(!err) ag_drv_bbh_tx_common_configurations_sbpmcfg_get( bbh_id, &freenocntxt, &specialfree, &maxgn);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_sbpmcfg_get(%u %u %u %u)\n", bbh_id, freenocntxt, specialfree, maxgn);
        if(err || freenocntxt!=gtmv(m, 1) || specialfree!=gtmv(m, 1) || maxgn!=gtmv(m, 5))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t zero=gtmv(m, 0);
        bbh_tx_common_configurations_ddrtmbasel common_configurations_ddrtmbasel = {.addr={gtmv(m, 32), gtmv(m, 32)}};
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_ddrtmbasel_set(%u %u %u %u)\n", bbh_id, zero, common_configurations_ddrtmbasel.addr[0], common_configurations_ddrtmbasel.addr[1]);
        if(!err) ag_drv_bbh_tx_common_configurations_ddrtmbasel_set(bbh_id, zero, &common_configurations_ddrtmbasel);
        if(!err) ag_drv_bbh_tx_common_configurations_ddrtmbasel_get( bbh_id, zero, &common_configurations_ddrtmbasel);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_ddrtmbasel_get(%u %u %u %u)\n", bbh_id, zero, common_configurations_ddrtmbasel.addr[0], common_configurations_ddrtmbasel.addr[1]);
        if(err || common_configurations_ddrtmbasel.addr[0]!=gtmv(m, 32) || common_configurations_ddrtmbasel.addr[1]!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t zero=gtmv(m, 0);
        bbh_tx_common_configurations_ddrtmbaseh common_configurations_ddrtmbaseh = {.addr={gtmv(m, 32), gtmv(m, 32)}};
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_ddrtmbaseh_set(%u %u %u %u)\n", bbh_id, zero, common_configurations_ddrtmbaseh.addr[0], common_configurations_ddrtmbaseh.addr[1]);
        if(!err) ag_drv_bbh_tx_common_configurations_ddrtmbaseh_set(bbh_id, zero, &common_configurations_ddrtmbaseh);
        if(!err) ag_drv_bbh_tx_common_configurations_ddrtmbaseh_get( bbh_id, zero, &common_configurations_ddrtmbaseh);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_ddrtmbaseh_get(%u %u %u %u)\n", bbh_id, zero, common_configurations_ddrtmbaseh.addr[0], common_configurations_ddrtmbaseh.addr[1]);
        if(err || common_configurations_ddrtmbaseh.addr[0]!=gtmv(m, 32) || common_configurations_ddrtmbaseh.addr[1]!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t psramsize=gtmv(m, 10);
        uint16_t ddrsize=gtmv(m, 10);
        uint16_t psrambase=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_dfifoctrl_set(%u %u %u %u)\n", bbh_id, psramsize, ddrsize, psrambase);
        if(!err) ag_drv_bbh_tx_common_configurations_dfifoctrl_set(bbh_id, psramsize, ddrsize, psrambase);
        if(!err) ag_drv_bbh_tx_common_configurations_dfifoctrl_get( bbh_id, &psramsize, &ddrsize, &psrambase);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_dfifoctrl_get(%u %u %u %u)\n", bbh_id, psramsize, ddrsize, psrambase);
        if(err || psramsize!=gtmv(m, 10) || ddrsize!=gtmv(m, 10) || psrambase!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean hightrxq=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_arb_cfg_set(%u %u)\n", bbh_id, hightrxq);
        if(!err) ag_drv_bbh_tx_common_configurations_arb_cfg_set(bbh_id, hightrxq);
        if(!err) ag_drv_bbh_tx_common_configurations_arb_cfg_get( bbh_id, &hightrxq);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_arb_cfg_get(%u %u)\n", bbh_id, hightrxq);
        if(err || hightrxq!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t route=gtmv(m, 10);
        uint8_t dest=gtmv(m, 6);
        bdmf_boolean en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_bbroute_set(%u %u %u %u)\n", bbh_id, route, dest, en);
        if(!err) ag_drv_bbh_tx_common_configurations_bbroute_set(bbh_id, route, dest, en);
        if(!err) ag_drv_bbh_tx_common_configurations_bbroute_get( bbh_id, &route, &dest, &en);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_bbroute_get(%u %u %u %u)\n", bbh_id, route, dest, en);
        if(err || route!=gtmv(m, 10) || dest!=gtmv(m, 6) || en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t q_2_rnr_index=gtmv(m, 2);
        bdmf_boolean q0=gtmv(m, 1);
        bdmf_boolean q1=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_q2rnr_set(%u %u %u %u)\n", bbh_id, q_2_rnr_index, q0, q1);
        if(!err) ag_drv_bbh_tx_common_configurations_q2rnr_set(bbh_id, q_2_rnr_index, q0, q1);
        if(!err) ag_drv_bbh_tx_common_configurations_q2rnr_get( bbh_id, q_2_rnr_index, &q0, &q1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_q2rnr_get(%u %u %u %u)\n", bbh_id, q_2_rnr_index, q0, q1);
        if(err || q0!=gtmv(m, 1) || q1!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bbh_tx_common_configurations_perqtask common_configurations_perqtask = {.task0=gtmv(m, 4), .task1=gtmv(m, 4), .task2=gtmv(m, 4), .task3=gtmv(m, 4), .task4=gtmv(m, 4), .task5=gtmv(m, 4), .task6=gtmv(m, 4), .task7=gtmv(m, 4)};
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_perqtask_set(%u %u %u %u %u %u %u %u %u)\n", bbh_id, common_configurations_perqtask.task0, common_configurations_perqtask.task1, common_configurations_perqtask.task2, common_configurations_perqtask.task3, common_configurations_perqtask.task4, common_configurations_perqtask.task5, common_configurations_perqtask.task6, common_configurations_perqtask.task7);
        if(!err) ag_drv_bbh_tx_common_configurations_perqtask_set(bbh_id, &common_configurations_perqtask);
        if(!err) ag_drv_bbh_tx_common_configurations_perqtask_get( bbh_id, &common_configurations_perqtask);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_perqtask_get(%u %u %u %u %u %u %u %u %u)\n", bbh_id, common_configurations_perqtask.task0, common_configurations_perqtask.task1, common_configurations_perqtask.task2, common_configurations_perqtask.task3, common_configurations_perqtask.task4, common_configurations_perqtask.task5, common_configurations_perqtask.task6, common_configurations_perqtask.task7);
        if(err || common_configurations_perqtask.task0!=gtmv(m, 4) || common_configurations_perqtask.task1!=gtmv(m, 4) || common_configurations_perqtask.task2!=gtmv(m, 4) || common_configurations_perqtask.task3!=gtmv(m, 4) || common_configurations_perqtask.task4!=gtmv(m, 4) || common_configurations_perqtask.task5!=gtmv(m, 4) || common_configurations_perqtask.task6!=gtmv(m, 4) || common_configurations_perqtask.task7!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bbh_tx_common_configurations_txrstcmd common_configurations_txrstcmd = {.cntxtrst=gtmv(m, 1), .pdfiforst=gtmv(m, 1), .dmaptrrst=gtmv(m, 1), .sdmaptrrst=gtmv(m, 1), .bpmfiforst=gtmv(m, 1), .sbpmfiforst=gtmv(m, 1), .okfiforst=gtmv(m, 1), .ddrfiforst=gtmv(m, 1), .sramfiforst=gtmv(m, 1), .skbptrrst=gtmv(m, 1), .stsfiforst=gtmv(m, 1), .reqfiforst=gtmv(m, 1), .msgfiforst=gtmv(m, 1), .gnxtfiforst=gtmv(m, 1), .fbnfiforst=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_txrstcmd_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", bbh_id, common_configurations_txrstcmd.cntxtrst, common_configurations_txrstcmd.pdfiforst, common_configurations_txrstcmd.dmaptrrst, common_configurations_txrstcmd.sdmaptrrst, common_configurations_txrstcmd.bpmfiforst, common_configurations_txrstcmd.sbpmfiforst, common_configurations_txrstcmd.okfiforst, common_configurations_txrstcmd.ddrfiforst, common_configurations_txrstcmd.sramfiforst, common_configurations_txrstcmd.skbptrrst, common_configurations_txrstcmd.stsfiforst, common_configurations_txrstcmd.reqfiforst, common_configurations_txrstcmd.msgfiforst, common_configurations_txrstcmd.gnxtfiforst, common_configurations_txrstcmd.fbnfiforst);
        if(!err) ag_drv_bbh_tx_common_configurations_txrstcmd_set(bbh_id, &common_configurations_txrstcmd);
        if(!err) ag_drv_bbh_tx_common_configurations_txrstcmd_get( bbh_id, &common_configurations_txrstcmd);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_txrstcmd_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", bbh_id, common_configurations_txrstcmd.cntxtrst, common_configurations_txrstcmd.pdfiforst, common_configurations_txrstcmd.dmaptrrst, common_configurations_txrstcmd.sdmaptrrst, common_configurations_txrstcmd.bpmfiforst, common_configurations_txrstcmd.sbpmfiforst, common_configurations_txrstcmd.okfiforst, common_configurations_txrstcmd.ddrfiforst, common_configurations_txrstcmd.sramfiforst, common_configurations_txrstcmd.skbptrrst, common_configurations_txrstcmd.stsfiforst, common_configurations_txrstcmd.reqfiforst, common_configurations_txrstcmd.msgfiforst, common_configurations_txrstcmd.gnxtfiforst, common_configurations_txrstcmd.fbnfiforst);
        if(err || common_configurations_txrstcmd.cntxtrst!=gtmv(m, 1) || common_configurations_txrstcmd.pdfiforst!=gtmv(m, 1) || common_configurations_txrstcmd.dmaptrrst!=gtmv(m, 1) || common_configurations_txrstcmd.sdmaptrrst!=gtmv(m, 1) || common_configurations_txrstcmd.bpmfiforst!=gtmv(m, 1) || common_configurations_txrstcmd.sbpmfiforst!=gtmv(m, 1) || common_configurations_txrstcmd.okfiforst!=gtmv(m, 1) || common_configurations_txrstcmd.ddrfiforst!=gtmv(m, 1) || common_configurations_txrstcmd.sramfiforst!=gtmv(m, 1) || common_configurations_txrstcmd.skbptrrst!=gtmv(m, 1) || common_configurations_txrstcmd.stsfiforst!=gtmv(m, 1) || common_configurations_txrstcmd.reqfiforst!=gtmv(m, 1) || common_configurations_txrstcmd.msgfiforst!=gtmv(m, 1) || common_configurations_txrstcmd.gnxtfiforst!=gtmv(m, 1) || common_configurations_txrstcmd.fbnfiforst!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t dbgsel=gtmv(m, 5);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_dbgsel_set(%u %u)\n", bbh_id, dbgsel);
        if(!err) ag_drv_bbh_tx_common_configurations_dbgsel_set(bbh_id, dbgsel);
        if(!err) ag_drv_bbh_tx_common_configurations_dbgsel_get( bbh_id, &dbgsel);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_dbgsel_get(%u %u)\n", bbh_id, dbgsel);
        if(err || dbgsel!=gtmv(m, 5))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bbh_tx_common_configurations_clk_gate_cntrl common_configurations_clk_gate_cntrl = {.bypass_clk_gate=gtmv(m, 1), .timer_val=gtmv(m, 8), .keep_alive_en=gtmv(m, 1), .keep_alive_intrvl=gtmv(m, 3), .keep_alive_cyc=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_clk_gate_cntrl_set(%u %u %u %u %u %u)\n", bbh_id, common_configurations_clk_gate_cntrl.bypass_clk_gate, common_configurations_clk_gate_cntrl.timer_val, common_configurations_clk_gate_cntrl.keep_alive_en, common_configurations_clk_gate_cntrl.keep_alive_intrvl, common_configurations_clk_gate_cntrl.keep_alive_cyc);
        if(!err) ag_drv_bbh_tx_common_configurations_clk_gate_cntrl_set(bbh_id, &common_configurations_clk_gate_cntrl);
        if(!err) ag_drv_bbh_tx_common_configurations_clk_gate_cntrl_get( bbh_id, &common_configurations_clk_gate_cntrl);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_common_configurations_clk_gate_cntrl_get(%u %u %u %u %u %u)\n", bbh_id, common_configurations_clk_gate_cntrl.bypass_clk_gate, common_configurations_clk_gate_cntrl.timer_val, common_configurations_clk_gate_cntrl.keep_alive_en, common_configurations_clk_gate_cntrl.keep_alive_intrvl, common_configurations_clk_gate_cntrl.keep_alive_cyc);
        if(err || common_configurations_clk_gate_cntrl.bypass_clk_gate!=gtmv(m, 1) || common_configurations_clk_gate_cntrl.timer_val!=gtmv(m, 8) || common_configurations_clk_gate_cntrl.keep_alive_en!=gtmv(m, 1) || common_configurations_clk_gate_cntrl.keep_alive_intrvl!=gtmv(m, 3) || common_configurations_clk_gate_cntrl.keep_alive_cyc!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t wan_pd_base_index=gtmv(m, 2);
        uint16_t fifobase0=gtmv(m, 9);
        uint16_t fifobase1=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_pdbase_set(%u %u %u %u)\n", bbh_id, wan_pd_base_index, fifobase0, fifobase1);
        if(!err) ag_drv_bbh_tx_wan_configurations_pdbase_set(bbh_id, wan_pd_base_index, fifobase0, fifobase1);
        if(!err) ag_drv_bbh_tx_wan_configurations_pdbase_get( bbh_id, wan_pd_base_index, &fifobase0, &fifobase1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_pdbase_get(%u %u %u %u)\n", bbh_id, wan_pd_base_index, fifobase0, fifobase1);
        if(err || fifobase0!=gtmv(m, 9) || fifobase1!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t wan_pd_size_index=gtmv(m, 2);
        uint16_t fifosize0=gtmv(m, 9);
        uint16_t fifosize1=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_pdsize_set(%u %u %u %u)\n", bbh_id, wan_pd_size_index, fifosize0, fifosize1);
        if(!err) ag_drv_bbh_tx_wan_configurations_pdsize_set(bbh_id, wan_pd_size_index, fifosize0, fifosize1);
        if(!err) ag_drv_bbh_tx_wan_configurations_pdsize_get( bbh_id, wan_pd_size_index, &fifosize0, &fifosize1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_pdsize_get(%u %u %u %u)\n", bbh_id, wan_pd_size_index, fifosize0, fifosize1);
        if(err || fifosize0!=gtmv(m, 9) || fifosize1!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t wan_pd_wkup_index=gtmv(m, 2);
        uint8_t wkupthresh0=gtmv(m, 8);
        uint8_t wkupthresh1=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_pdwkuph_set(%u %u %u %u)\n", bbh_id, wan_pd_wkup_index, wkupthresh0, wkupthresh1);
        if(!err) ag_drv_bbh_tx_wan_configurations_pdwkuph_set(bbh_id, wan_pd_wkup_index, wkupthresh0, wkupthresh1);
        if(!err) ag_drv_bbh_tx_wan_configurations_pdwkuph_get( bbh_id, wan_pd_wkup_index, &wkupthresh0, &wkupthresh1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_pdwkuph_get(%u %u %u %u)\n", bbh_id, wan_pd_wkup_index, wkupthresh0, wkupthresh1);
        if(err || wkupthresh0!=gtmv(m, 8) || wkupthresh1!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t wan_pd_byte_th_index=gtmv(m, 2);
        uint16_t pdlimit0=gtmv(m, 16);
        uint16_t pdlimit1=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_pd_byte_th_set(%u %u %u %u)\n", bbh_id, wan_pd_byte_th_index, pdlimit0, pdlimit1);
        if(!err) ag_drv_bbh_tx_wan_configurations_pd_byte_th_set(bbh_id, wan_pd_byte_th_index, pdlimit0, pdlimit1);
        if(!err) ag_drv_bbh_tx_wan_configurations_pd_byte_th_get( bbh_id, wan_pd_byte_th_index, &pdlimit0, &pdlimit1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_pd_byte_th_get(%u %u %u %u)\n", bbh_id, wan_pd_byte_th_index, pdlimit0, pdlimit1);
        if(err || pdlimit0!=gtmv(m, 16) || pdlimit1!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean pdlimiten=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_pd_byte_th_en_set(%u %u)\n", bbh_id, pdlimiten);
        if(!err) ag_drv_bbh_tx_wan_configurations_pd_byte_th_en_set(bbh_id, pdlimiten);
        if(!err) ag_drv_bbh_tx_wan_configurations_pd_byte_th_en_get( bbh_id, &pdlimiten);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_pd_byte_th_en_get(%u %u)\n", bbh_id, pdlimiten);
        if(err || pdlimiten!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t empty=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_pdempty_set(%u %u)\n", bbh_id, empty);
        if(!err) ag_drv_bbh_tx_wan_configurations_pdempty_set(bbh_id, empty);
        if(!err) ag_drv_bbh_tx_wan_configurations_pdempty_get( bbh_id, &empty);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_pdempty_get(%u %u)\n", bbh_id, empty);
        if(err || empty!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t wan_sts_rnr_cfg_1_index=gtmv(m, 1);
        uint16_t tcontaddr=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_stsrnrcfg_1_set(%u %u %u)\n", bbh_id, wan_sts_rnr_cfg_1_index, tcontaddr);
        if(!err) ag_drv_bbh_tx_wan_configurations_stsrnrcfg_1_set(bbh_id, wan_sts_rnr_cfg_1_index, tcontaddr);
        if(!err) ag_drv_bbh_tx_wan_configurations_stsrnrcfg_1_get( bbh_id, wan_sts_rnr_cfg_1_index, &tcontaddr);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_stsrnrcfg_1_get(%u %u %u)\n", bbh_id, wan_sts_rnr_cfg_1_index, tcontaddr);
        if(err || tcontaddr!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t wan_sts_rnr_cfg_2_index=gtmv(m, 1);
        uint16_t ptraddr=gtmv(m, 16);
        uint8_t task=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_stsrnrcfg_2_set(%u %u %u %u)\n", bbh_id, wan_sts_rnr_cfg_2_index, ptraddr, task);
        if(!err) ag_drv_bbh_tx_wan_configurations_stsrnrcfg_2_set(bbh_id, wan_sts_rnr_cfg_2_index, ptraddr, task);
        if(!err) ag_drv_bbh_tx_wan_configurations_stsrnrcfg_2_get( bbh_id, wan_sts_rnr_cfg_2_index, &ptraddr, &task);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_stsrnrcfg_2_get(%u %u %u %u)\n", bbh_id, wan_sts_rnr_cfg_2_index, ptraddr, task);
        if(err || ptraddr!=gtmv(m, 16) || task!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t wan_msg_rnr_cfg_1_index=gtmv(m, 1);
        uint16_t tcontaddr=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_msgrnrcfg_1_set(%u %u %u)\n", bbh_id, wan_msg_rnr_cfg_1_index, tcontaddr);
        if(!err) ag_drv_bbh_tx_wan_configurations_msgrnrcfg_1_set(bbh_id, wan_msg_rnr_cfg_1_index, tcontaddr);
        if(!err) ag_drv_bbh_tx_wan_configurations_msgrnrcfg_1_get( bbh_id, wan_msg_rnr_cfg_1_index, &tcontaddr);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_msgrnrcfg_1_get(%u %u %u)\n", bbh_id, wan_msg_rnr_cfg_1_index, tcontaddr);
        if(err || tcontaddr!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t wan_msg_rnr_cfg_2_index=gtmv(m, 1);
        uint16_t ptraddr=gtmv(m, 16);
        uint8_t task=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_msgrnrcfg_2_set(%u %u %u %u)\n", bbh_id, wan_msg_rnr_cfg_2_index, ptraddr, task);
        if(!err) ag_drv_bbh_tx_wan_configurations_msgrnrcfg_2_set(bbh_id, wan_msg_rnr_cfg_2_index, ptraddr, task);
        if(!err) ag_drv_bbh_tx_wan_configurations_msgrnrcfg_2_get( bbh_id, wan_msg_rnr_cfg_2_index, &ptraddr, &task);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_msgrnrcfg_2_get(%u %u %u %u)\n", bbh_id, wan_msg_rnr_cfg_2_index, ptraddr, task);
        if(err || ptraddr!=gtmv(m, 16) || task!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean stplenerr=gtmv(m, 1);
        bdmf_boolean cmp_width=gtmv(m, 1);
        bdmf_boolean considerfull=gtmv(m, 1);
        bdmf_boolean addcrc=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_epncfg_set(%u %u %u %u %u)\n", bbh_id, stplenerr, cmp_width, considerfull, addcrc);
        if(!err) ag_drv_bbh_tx_wan_configurations_epncfg_set(bbh_id, stplenerr, cmp_width, considerfull, addcrc);
        if(!err) ag_drv_bbh_tx_wan_configurations_epncfg_get( bbh_id, &stplenerr, &cmp_width, &considerfull, &addcrc);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_epncfg_get(%u %u %u %u %u)\n", bbh_id, stplenerr, cmp_width, considerfull, addcrc);
        if(err || stplenerr!=gtmv(m, 1) || cmp_width!=gtmv(m, 1) || considerfull!=gtmv(m, 1) || addcrc!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t wdata=gtmv(m, 18);
        uint8_t a=gtmv(m, 8);
        bdmf_boolean cmd=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_flow2port_set(%u %u %u %u)\n", bbh_id, wdata, a, cmd);
        if(!err) ag_drv_bbh_tx_wan_configurations_flow2port_set(bbh_id, wdata, a, cmd);
        if(!err) ag_drv_bbh_tx_wan_configurations_flow2port_get( bbh_id, &wdata, &a, &cmd);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_flow2port_get(%u %u %u %u)\n", bbh_id, wdata, a, cmd);
        if(err || wdata!=gtmv(m, 18) || a!=gtmv(m, 8) || cmd!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_ts_set(%u %u)\n", bbh_id, en);
        if(!err) ag_drv_bbh_tx_wan_configurations_ts_set(bbh_id, en);
        if(!err) ag_drv_bbh_tx_wan_configurations_ts_get( bbh_id, &en);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_ts_get(%u %u)\n", bbh_id, en);
        if(err || en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t maxwlen=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_maxwlen_set(%u %u)\n", bbh_id, maxwlen);
        if(!err) ag_drv_bbh_tx_wan_configurations_maxwlen_set(bbh_id, maxwlen);
        if(!err) ag_drv_bbh_tx_wan_configurations_maxwlen_get( bbh_id, &maxwlen);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_maxwlen_get(%u %u)\n", bbh_id, maxwlen);
        if(err || maxwlen!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t flush=gtmv(m, 16);
        bdmf_boolean srst_n=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_flush_set(%u %u %u)\n", bbh_id, flush, srst_n);
        if(!err) ag_drv_bbh_tx_wan_configurations_flush_set(bbh_id, flush, srst_n);
        if(!err) ag_drv_bbh_tx_wan_configurations_flush_get( bbh_id, &flush, &srst_n);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_wan_configurations_flush_get(%u %u %u)\n", bbh_id, flush, srst_n);
        if(err || flush!=gtmv(m, 16) || srst_n!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t fifobase0=gtmv(m, 9);
        uint16_t fifobase1=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_pdbase_set(%u %u %u)\n", bbh_id, fifobase0, fifobase1);
        if(!err) ag_drv_bbh_tx_lan_configurations_pdbase_set(bbh_id, fifobase0, fifobase1);
        if(!err) ag_drv_bbh_tx_lan_configurations_pdbase_get( bbh_id, &fifobase0, &fifobase1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_pdbase_get(%u %u %u)\n", bbh_id, fifobase0, fifobase1);
        if(err || fifobase0!=gtmv(m, 9) || fifobase1!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t fifosize0=gtmv(m, 9);
        uint16_t fifosize1=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_pdsize_set(%u %u %u)\n", bbh_id, fifosize0, fifosize1);
        if(!err) ag_drv_bbh_tx_lan_configurations_pdsize_set(bbh_id, fifosize0, fifosize1);
        if(!err) ag_drv_bbh_tx_lan_configurations_pdsize_get( bbh_id, &fifosize0, &fifosize1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_pdsize_get(%u %u %u)\n", bbh_id, fifosize0, fifosize1);
        if(err || fifosize0!=gtmv(m, 9) || fifosize1!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t wkupthresh0=gtmv(m, 8);
        uint8_t wkupthresh1=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_pdwkuph_set(%u %u %u)\n", bbh_id, wkupthresh0, wkupthresh1);
        if(!err) ag_drv_bbh_tx_lan_configurations_pdwkuph_set(bbh_id, wkupthresh0, wkupthresh1);
        if(!err) ag_drv_bbh_tx_lan_configurations_pdwkuph_get( bbh_id, &wkupthresh0, &wkupthresh1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_pdwkuph_get(%u %u %u)\n", bbh_id, wkupthresh0, wkupthresh1);
        if(err || wkupthresh0!=gtmv(m, 8) || wkupthresh1!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t pdlimit0=gtmv(m, 16);
        uint16_t pdlimit1=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_pd_byte_th_set(%u %u %u)\n", bbh_id, pdlimit0, pdlimit1);
        if(!err) ag_drv_bbh_tx_lan_configurations_pd_byte_th_set(bbh_id, pdlimit0, pdlimit1);
        if(!err) ag_drv_bbh_tx_lan_configurations_pd_byte_th_get( bbh_id, &pdlimit0, &pdlimit1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_pd_byte_th_get(%u %u %u)\n", bbh_id, pdlimit0, pdlimit1);
        if(err || pdlimit0!=gtmv(m, 16) || pdlimit1!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean pdlimiten=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_pd_byte_th_en_set(%u %u)\n", bbh_id, pdlimiten);
        if(!err) ag_drv_bbh_tx_lan_configurations_pd_byte_th_en_set(bbh_id, pdlimiten);
        if(!err) ag_drv_bbh_tx_lan_configurations_pd_byte_th_en_get( bbh_id, &pdlimiten);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_pd_byte_th_en_get(%u %u)\n", bbh_id, pdlimiten);
        if(err || pdlimiten!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t empty=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_pdempty_set(%u %u)\n", bbh_id, empty);
        if(!err) ag_drv_bbh_tx_lan_configurations_pdempty_set(bbh_id, empty);
        if(!err) ag_drv_bbh_tx_lan_configurations_pdempty_get( bbh_id, &empty);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_pdempty_get(%u %u)\n", bbh_id, empty);
        if(err || empty!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t ddrthresh=gtmv(m, 9);
        uint16_t sramthresh=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_txthresh_set(%u %u %u)\n", bbh_id, ddrthresh, sramthresh);
        if(!err) ag_drv_bbh_tx_lan_configurations_txthresh_set(bbh_id, ddrthresh, sramthresh);
        if(!err) ag_drv_bbh_tx_lan_configurations_txthresh_get( bbh_id, &ddrthresh, &sramthresh);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_txthresh_get(%u %u %u)\n", bbh_id, ddrthresh, sramthresh);
        if(err || ddrthresh!=gtmv(m, 9) || sramthresh!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_eee_set(%u %u)\n", bbh_id, en);
        if(!err) ag_drv_bbh_tx_lan_configurations_eee_set(bbh_id, en);
        if(!err) ag_drv_bbh_tx_lan_configurations_eee_get( bbh_id, &en);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_eee_get(%u %u)\n", bbh_id, en);
        if(err || en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_ts_set(%u %u)\n", bbh_id, en);
        if(!err) ag_drv_bbh_tx_lan_configurations_ts_set(bbh_id, en);
        if(!err) ag_drv_bbh_tx_lan_configurations_ts_get( bbh_id, &en);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_lan_configurations_ts_get(%u %u)\n", bbh_id, en);
        if(err || en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t unified_pd_base_index=gtmv(m, 2);
        uint16_t fifobase0=gtmv(m, 9);
        uint16_t fifobase1=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_pdbase_set(%u %u %u %u)\n", bbh_id, unified_pd_base_index, fifobase0, fifobase1);
        if(!err) ag_drv_bbh_tx_unified_configurations_pdbase_set(bbh_id, unified_pd_base_index, fifobase0, fifobase1);
        if(!err) ag_drv_bbh_tx_unified_configurations_pdbase_get( bbh_id, unified_pd_base_index, &fifobase0, &fifobase1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_pdbase_get(%u %u %u %u)\n", bbh_id, unified_pd_base_index, fifobase0, fifobase1);
        if(err || fifobase0!=gtmv(m, 9) || fifobase1!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t unified_pd_size_index=gtmv(m, 2);
        uint16_t fifosize0=gtmv(m, 9);
        uint16_t fifosize1=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_pdsize_set(%u %u %u %u)\n", bbh_id, unified_pd_size_index, fifosize0, fifosize1);
        if(!err) ag_drv_bbh_tx_unified_configurations_pdsize_set(bbh_id, unified_pd_size_index, fifosize0, fifosize1);
        if(!err) ag_drv_bbh_tx_unified_configurations_pdsize_get( bbh_id, unified_pd_size_index, &fifosize0, &fifosize1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_pdsize_get(%u %u %u %u)\n", bbh_id, unified_pd_size_index, fifosize0, fifosize1);
        if(err || fifosize0!=gtmv(m, 9) || fifosize1!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t unified_pd_wkup_index=gtmv(m, 2);
        uint8_t wkupthresh0=gtmv(m, 8);
        uint8_t wkupthresh1=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_pdwkuph_set(%u %u %u %u)\n", bbh_id, unified_pd_wkup_index, wkupthresh0, wkupthresh1);
        if(!err) ag_drv_bbh_tx_unified_configurations_pdwkuph_set(bbh_id, unified_pd_wkup_index, wkupthresh0, wkupthresh1);
        if(!err) ag_drv_bbh_tx_unified_configurations_pdwkuph_get( bbh_id, unified_pd_wkup_index, &wkupthresh0, &wkupthresh1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_pdwkuph_get(%u %u %u %u)\n", bbh_id, unified_pd_wkup_index, wkupthresh0, wkupthresh1);
        if(err || wkupthresh0!=gtmv(m, 8) || wkupthresh1!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t unified_pd_byte_th_index=gtmv(m, 2);
        uint16_t pdlimit0=gtmv(m, 16);
        uint16_t pdlimit1=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_pd_byte_th_set(%u %u %u %u)\n", bbh_id, unified_pd_byte_th_index, pdlimit0, pdlimit1);
        if(!err) ag_drv_bbh_tx_unified_configurations_pd_byte_th_set(bbh_id, unified_pd_byte_th_index, pdlimit0, pdlimit1);
        if(!err) ag_drv_bbh_tx_unified_configurations_pd_byte_th_get( bbh_id, unified_pd_byte_th_index, &pdlimit0, &pdlimit1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_pd_byte_th_get(%u %u %u %u)\n", bbh_id, unified_pd_byte_th_index, pdlimit0, pdlimit1);
        if(err || pdlimit0!=gtmv(m, 16) || pdlimit1!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean pdlimiten=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_pd_byte_th_en_set(%u %u)\n", bbh_id, pdlimiten);
        if(!err) ag_drv_bbh_tx_unified_configurations_pd_byte_th_en_set(bbh_id, pdlimiten);
        if(!err) ag_drv_bbh_tx_unified_configurations_pd_byte_th_en_get( bbh_id, &pdlimiten);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_pd_byte_th_en_get(%u %u)\n", bbh_id, pdlimiten);
        if(err || pdlimiten!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t empty=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_pdempty_set(%u %u)\n", bbh_id, empty);
        if(!err) ag_drv_bbh_tx_unified_configurations_pdempty_set(bbh_id, empty);
        if(!err) ag_drv_bbh_tx_unified_configurations_pdempty_get( bbh_id, &empty);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_pdempty_get(%u %u)\n", bbh_id, empty);
        if(err || empty!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t ddrthresh=gtmv(m, 9);
        uint16_t sramthresh=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_gtxthresh_set(%u %u %u)\n", bbh_id, ddrthresh, sramthresh);
        if(!err) ag_drv_bbh_tx_unified_configurations_gtxthresh_set(bbh_id, ddrthresh, sramthresh);
        if(!err) ag_drv_bbh_tx_unified_configurations_gtxthresh_get( bbh_id, &ddrthresh, &sramthresh);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_gtxthresh_get(%u %u %u)\n", bbh_id, ddrthresh, sramthresh);
        if(err || ddrthresh!=gtmv(m, 9) || sramthresh!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t en=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_eee_set(%u %u)\n", bbh_id, en);
        if(!err) ag_drv_bbh_tx_unified_configurations_eee_set(bbh_id, en);
        if(!err) ag_drv_bbh_tx_unified_configurations_eee_get( bbh_id, &en);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_eee_get(%u %u)\n", bbh_id, en);
        if(err || en!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t en=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_ts_set(%u %u)\n", bbh_id, en);
        if(!err) ag_drv_bbh_tx_unified_configurations_ts_set(bbh_id, en);
        if(!err) ag_drv_bbh_tx_unified_configurations_ts_get( bbh_id, &en);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_ts_get(%u %u)\n", bbh_id, en);
        if(err || en!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t unified_fe_base_index=gtmv(m, 2);
        uint16_t fifobase0=gtmv(m, 11);
        uint16_t fifobase1=gtmv(m, 11);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_febase_set(%u %u %u %u)\n", bbh_id, unified_fe_base_index, fifobase0, fifobase1);
        if(!err) ag_drv_bbh_tx_unified_configurations_febase_set(bbh_id, unified_fe_base_index, fifobase0, fifobase1);
        if(!err) ag_drv_bbh_tx_unified_configurations_febase_get( bbh_id, unified_fe_base_index, &fifobase0, &fifobase1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_febase_get(%u %u %u %u)\n", bbh_id, unified_fe_base_index, fifobase0, fifobase1);
        if(err || fifobase0!=gtmv(m, 11) || fifobase1!=gtmv(m, 11))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t unified_fe_size_index=gtmv(m, 2);
        uint16_t fifosize0=gtmv(m, 11);
        uint16_t fifosize1=gtmv(m, 11);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_fesize_set(%u %u %u %u)\n", bbh_id, unified_fe_size_index, fifosize0, fifosize1);
        if(!err) ag_drv_bbh_tx_unified_configurations_fesize_set(bbh_id, unified_fe_size_index, fifosize0, fifosize1);
        if(!err) ag_drv_bbh_tx_unified_configurations_fesize_get( bbh_id, unified_fe_size_index, &fifosize0, &fifosize1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_fesize_get(%u %u %u %u)\n", bbh_id, unified_fe_size_index, fifosize0, fifosize1);
        if(err || fifosize0!=gtmv(m, 11) || fifosize1!=gtmv(m, 11))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t unified_fe_pd_base_index=gtmv(m, 2);
        uint8_t fifobase0=gtmv(m, 8);
        uint8_t fifobase1=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_fepdbase_set(%u %u %u %u)\n", bbh_id, unified_fe_pd_base_index, fifobase0, fifobase1);
        if(!err) ag_drv_bbh_tx_unified_configurations_fepdbase_set(bbh_id, unified_fe_pd_base_index, fifobase0, fifobase1);
        if(!err) ag_drv_bbh_tx_unified_configurations_fepdbase_get( bbh_id, unified_fe_pd_base_index, &fifobase0, &fifobase1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_fepdbase_get(%u %u %u %u)\n", bbh_id, unified_fe_pd_base_index, fifobase0, fifobase1);
        if(err || fifobase0!=gtmv(m, 8) || fifobase1!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t unified_fe_pd_size_index=gtmv(m, 2);
        uint8_t fifosize0=gtmv(m, 8);
        uint8_t fifosize1=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_fepdsize_set(%u %u %u %u)\n", bbh_id, unified_fe_pd_size_index, fifosize0, fifosize1);
        if(!err) ag_drv_bbh_tx_unified_configurations_fepdsize_set(bbh_id, unified_fe_pd_size_index, fifosize0, fifosize1);
        if(!err) ag_drv_bbh_tx_unified_configurations_fepdsize_get( bbh_id, unified_fe_pd_size_index, &fifosize0, &fifosize1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_fepdsize_get(%u %u %u %u)\n", bbh_id, unified_fe_pd_size_index, fifosize0, fifosize1);
        if(err || fifosize0!=gtmv(m, 8) || fifosize1!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t unified_tx_wrr_index=gtmv(m, 2);
        uint8_t w0=gtmv(m, 4);
        uint8_t w1=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_txwrr_set(%u %u %u %u)\n", bbh_id, unified_tx_wrr_index, w0, w1);
        if(!err) ag_drv_bbh_tx_unified_configurations_txwrr_set(bbh_id, unified_tx_wrr_index, w0, w1);
        if(!err) ag_drv_bbh_tx_unified_configurations_txwrr_get( bbh_id, unified_tx_wrr_index, &w0, &w1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_txwrr_get(%u %u %u %u)\n", bbh_id, unified_tx_wrr_index, w0, w1);
        if(err || w0!=gtmv(m, 4) || w1!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t unified_tx_thr_index=gtmv(m, 2);
        uint16_t thresh0=gtmv(m, 9);
        uint16_t thresh1=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_txthresh_set(%u %u %u %u)\n", bbh_id, unified_tx_thr_index, thresh0, thresh1);
        if(!err) ag_drv_bbh_tx_unified_configurations_txthresh_set(bbh_id, unified_tx_thr_index, thresh0, thresh1);
        if(!err) ag_drv_bbh_tx_unified_configurations_txthresh_get( bbh_id, unified_tx_thr_index, &thresh0, &thresh1);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_unified_configurations_txthresh_get(%u %u %u %u)\n", bbh_id, unified_tx_thr_index, thresh0, thresh1);
        if(err || thresh0!=gtmv(m, 9) || thresh1!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t srambyte=gtmv(m, 32);
        if(!err) ag_drv_bbh_tx_debug_counters_srambyte_get( bbh_id, &srambyte);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_debug_counters_srambyte_get(%u %u)\n", bbh_id, srambyte);
    }
    {
        uint32_t ddrbyte=gtmv(m, 32);
        if(!err) ag_drv_bbh_tx_debug_counters_ddrbyte_get( bbh_id, &ddrbyte);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_debug_counters_ddrbyte_get(%u %u)\n", bbh_id, ddrbyte);
    }
    {
        bbh_tx_debug_counters_swrden debug_counters_swrden = {.pdsel=gtmv(m, 1), .pdvsel=gtmv(m, 1), .pdemptysel=gtmv(m, 1), .pdfullsel=gtmv(m, 1), .pdbemptysel=gtmv(m, 1), .pdffwkpsel=gtmv(m, 1), .fbnsel=gtmv(m, 1), .fbnvsel=gtmv(m, 1), .fbnemptysel=gtmv(m, 1), .fbnfullsel=gtmv(m, 1), .getnextsel=gtmv(m, 1), .getnextvsel=gtmv(m, 1), .getnextemptysel=gtmv(m, 1), .getnextfullsel=gtmv(m, 1), .gpncntxtsel=gtmv(m, 1), .bpmsel=gtmv(m, 1), .bpmfsel=gtmv(m, 1), .sbpmsel=gtmv(m, 1), .sbpmfsel=gtmv(m, 1), .stssel=gtmv(m, 1), .stsvsel=gtmv(m, 1), .stsemptysel=gtmv(m, 1), .stsfullsel=gtmv(m, 1), .stsbemptysel=gtmv(m, 1), .stsffwkpsel=gtmv(m, 1), .msgsel=gtmv(m, 1), .msgvsel=gtmv(m, 1), .epnreqsel=gtmv(m, 1), .datasel=gtmv(m, 1), .reordersel=gtmv(m, 1), .tsinfosel=gtmv(m, 1), .mactxsel=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_debug_counters_swrden_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", bbh_id, debug_counters_swrden.pdsel, debug_counters_swrden.pdvsel, debug_counters_swrden.pdemptysel, debug_counters_swrden.pdfullsel, debug_counters_swrden.pdbemptysel, debug_counters_swrden.pdffwkpsel, debug_counters_swrden.fbnsel, debug_counters_swrden.fbnvsel, debug_counters_swrden.fbnemptysel, debug_counters_swrden.fbnfullsel, debug_counters_swrden.getnextsel, debug_counters_swrden.getnextvsel, debug_counters_swrden.getnextemptysel, debug_counters_swrden.getnextfullsel, debug_counters_swrden.gpncntxtsel, debug_counters_swrden.bpmsel, debug_counters_swrden.bpmfsel, debug_counters_swrden.sbpmsel, debug_counters_swrden.sbpmfsel, debug_counters_swrden.stssel, debug_counters_swrden.stsvsel, debug_counters_swrden.stsemptysel, debug_counters_swrden.stsfullsel, debug_counters_swrden.stsbemptysel, debug_counters_swrden.stsffwkpsel, debug_counters_swrden.msgsel, debug_counters_swrden.msgvsel, debug_counters_swrden.epnreqsel, debug_counters_swrden.datasel, debug_counters_swrden.reordersel, debug_counters_swrden.tsinfosel, debug_counters_swrden.mactxsel);
        if(!err) ag_drv_bbh_tx_debug_counters_swrden_set(bbh_id, &debug_counters_swrden);
        if(!err) ag_drv_bbh_tx_debug_counters_swrden_get( bbh_id, &debug_counters_swrden);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_debug_counters_swrden_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", bbh_id, debug_counters_swrden.pdsel, debug_counters_swrden.pdvsel, debug_counters_swrden.pdemptysel, debug_counters_swrden.pdfullsel, debug_counters_swrden.pdbemptysel, debug_counters_swrden.pdffwkpsel, debug_counters_swrden.fbnsel, debug_counters_swrden.fbnvsel, debug_counters_swrden.fbnemptysel, debug_counters_swrden.fbnfullsel, debug_counters_swrden.getnextsel, debug_counters_swrden.getnextvsel, debug_counters_swrden.getnextemptysel, debug_counters_swrden.getnextfullsel, debug_counters_swrden.gpncntxtsel, debug_counters_swrden.bpmsel, debug_counters_swrden.bpmfsel, debug_counters_swrden.sbpmsel, debug_counters_swrden.sbpmfsel, debug_counters_swrden.stssel, debug_counters_swrden.stsvsel, debug_counters_swrden.stsemptysel, debug_counters_swrden.stsfullsel, debug_counters_swrden.stsbemptysel, debug_counters_swrden.stsffwkpsel, debug_counters_swrden.msgsel, debug_counters_swrden.msgvsel, debug_counters_swrden.epnreqsel, debug_counters_swrden.datasel, debug_counters_swrden.reordersel, debug_counters_swrden.tsinfosel, debug_counters_swrden.mactxsel);
        if(err || debug_counters_swrden.pdsel!=gtmv(m, 1) || debug_counters_swrden.pdvsel!=gtmv(m, 1) || debug_counters_swrden.pdemptysel!=gtmv(m, 1) || debug_counters_swrden.pdfullsel!=gtmv(m, 1) || debug_counters_swrden.pdbemptysel!=gtmv(m, 1) || debug_counters_swrden.pdffwkpsel!=gtmv(m, 1) || debug_counters_swrden.fbnsel!=gtmv(m, 1) || debug_counters_swrden.fbnvsel!=gtmv(m, 1) || debug_counters_swrden.fbnemptysel!=gtmv(m, 1) || debug_counters_swrden.fbnfullsel!=gtmv(m, 1) || debug_counters_swrden.getnextsel!=gtmv(m, 1) || debug_counters_swrden.getnextvsel!=gtmv(m, 1) || debug_counters_swrden.getnextemptysel!=gtmv(m, 1) || debug_counters_swrden.getnextfullsel!=gtmv(m, 1) || debug_counters_swrden.gpncntxtsel!=gtmv(m, 1) || debug_counters_swrden.bpmsel!=gtmv(m, 1) || debug_counters_swrden.bpmfsel!=gtmv(m, 1) || debug_counters_swrden.sbpmsel!=gtmv(m, 1) || debug_counters_swrden.sbpmfsel!=gtmv(m, 1) || debug_counters_swrden.stssel!=gtmv(m, 1) || debug_counters_swrden.stsvsel!=gtmv(m, 1) || debug_counters_swrden.stsemptysel!=gtmv(m, 1) || debug_counters_swrden.stsfullsel!=gtmv(m, 1) || debug_counters_swrden.stsbemptysel!=gtmv(m, 1) || debug_counters_swrden.stsffwkpsel!=gtmv(m, 1) || debug_counters_swrden.msgsel!=gtmv(m, 1) || debug_counters_swrden.msgvsel!=gtmv(m, 1) || debug_counters_swrden.epnreqsel!=gtmv(m, 1) || debug_counters_swrden.datasel!=gtmv(m, 1) || debug_counters_swrden.reordersel!=gtmv(m, 1) || debug_counters_swrden.tsinfosel!=gtmv(m, 1) || debug_counters_swrden.mactxsel!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t rdaddr=gtmv(m, 11);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_debug_counters_swrdaddr_set(%u %u)\n", bbh_id, rdaddr);
        if(!err) ag_drv_bbh_tx_debug_counters_swrdaddr_set(bbh_id, rdaddr);
        if(!err) ag_drv_bbh_tx_debug_counters_swrdaddr_get( bbh_id, &rdaddr);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_debug_counters_swrdaddr_get(%u %u)\n", bbh_id, rdaddr);
        if(err || rdaddr!=gtmv(m, 11))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_bbh_tx_debug_counters_swrddata_get( bbh_id, &data);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_debug_counters_swrddata_get(%u %u)\n", bbh_id, data);
    }
    {
        uint8_t debug_unified_pkt_ctr_idx=gtmv(m, 3);
        uint32_t ddrbyte=gtmv(m, 32);
        if(!err) ag_drv_bbh_tx_debug_counters_unifiedpkt_get( bbh_id, debug_unified_pkt_ctr_idx, &ddrbyte);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_debug_counters_unifiedpkt_get(%u %u %u)\n", bbh_id, debug_unified_pkt_ctr_idx, ddrbyte);
    }
    {
        uint8_t debug_unified_byte_ctr_idx=gtmv(m, 3);
        uint32_t ddrbyte=gtmv(m, 32);
        if(!err) ag_drv_bbh_tx_debug_counters_unifiedbyte_get( bbh_id, debug_unified_byte_ctr_idx, &ddrbyte);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_debug_counters_unifiedbyte_get(%u %u %u)\n", bbh_id, debug_unified_byte_ctr_idx, ddrbyte);
    }
    {
        uint8_t zero=gtmv(m, 2);
        bbh_tx_debug_counters_dbgoutreg debug_counters_dbgoutreg = {.debug_out_reg={gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        if(!err) ag_drv_bbh_tx_debug_counters_dbgoutreg_get( bbh_id, zero, &debug_counters_dbgoutreg);
        if(!err) bdmf_session_print(session, "ag_drv_bbh_tx_debug_counters_dbgoutreg_get(%u %u %u %u %u %u %u %u %u %u)\n", bbh_id, zero, debug_counters_dbgoutreg.debug_out_reg[0], debug_counters_dbgoutreg.debug_out_reg[1], debug_counters_dbgoutreg.debug_out_reg[2], debug_counters_dbgoutreg.debug_out_reg[3], debug_counters_dbgoutreg.debug_out_reg[4], debug_counters_dbgoutreg.debug_out_reg[5], debug_counters_dbgoutreg.debug_out_reg[6], debug_counters_dbgoutreg.debug_out_reg[7]);
    }
    return err;
}

static int bcm_bbh_tx_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_common_configurations_mactype : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_MACTYPE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_bbcfg_1_tx : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_BBCFG_1_TX); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_bbcfg_2_tx : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_BBCFG_2_TX); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_ddrcfg_tx : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_DDRCFG_TX); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_rnrcfg_1 : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_RNRCFG_1); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_rnrcfg_2 : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_RNRCFG_2); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_dmacfg_tx : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_DMACFG_TX); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_sdmacfg_tx : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_SDMACFG_TX); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_sbpmcfg : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_SBPMCFG); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_ddrtmbasel : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_DDRTMBASEL); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_ddrtmbaseh : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_DDRTMBASEH); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_dfifoctrl : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_DFIFOCTRL); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_arb_cfg : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_ARB_CFG); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_bbroute : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_BBROUTE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_q2rnr : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_Q2RNR); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_perqtask : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_PERQTASK); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_txrstcmd : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_TXRSTCMD); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_dbgsel : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_DBGSEL); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_common_configurations_clk_gate_cntrl : reg = &RU_REG(BBH_TX, COMMON_CONFIGURATIONS_CLK_GATE_CNTRL); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_wan_configurations_pdbase : reg = &RU_REG(BBH_TX, WAN_CONFIGURATIONS_PDBASE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_wan_configurations_pdsize : reg = &RU_REG(BBH_TX, WAN_CONFIGURATIONS_PDSIZE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_wan_configurations_pdwkuph : reg = &RU_REG(BBH_TX, WAN_CONFIGURATIONS_PDWKUPH); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_wan_configurations_pd_byte_th : reg = &RU_REG(BBH_TX, WAN_CONFIGURATIONS_PD_BYTE_TH); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_wan_configurations_pd_byte_th_en : reg = &RU_REG(BBH_TX, WAN_CONFIGURATIONS_PD_BYTE_TH_EN); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_wan_configurations_pdempty : reg = &RU_REG(BBH_TX, WAN_CONFIGURATIONS_PDEMPTY); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_wan_configurations_stsrnrcfg_1 : reg = &RU_REG(BBH_TX, WAN_CONFIGURATIONS_STSRNRCFG_1); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_wan_configurations_stsrnrcfg_2 : reg = &RU_REG(BBH_TX, WAN_CONFIGURATIONS_STSRNRCFG_2); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_wan_configurations_msgrnrcfg_1 : reg = &RU_REG(BBH_TX, WAN_CONFIGURATIONS_MSGRNRCFG_1); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_wan_configurations_msgrnrcfg_2 : reg = &RU_REG(BBH_TX, WAN_CONFIGURATIONS_MSGRNRCFG_2); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_wan_configurations_epncfg : reg = &RU_REG(BBH_TX, WAN_CONFIGURATIONS_EPNCFG); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_wan_configurations_flow2port : reg = &RU_REG(BBH_TX, WAN_CONFIGURATIONS_FLOW2PORT); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_wan_configurations_ts : reg = &RU_REG(BBH_TX, WAN_CONFIGURATIONS_TS); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_wan_configurations_maxwlen : reg = &RU_REG(BBH_TX, WAN_CONFIGURATIONS_MAXWLEN); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_wan_configurations_flush : reg = &RU_REG(BBH_TX, WAN_CONFIGURATIONS_FLUSH); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_lan_configurations_pdbase : reg = &RU_REG(BBH_TX, LAN_CONFIGURATIONS_PDBASE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_lan_configurations_pdsize : reg = &RU_REG(BBH_TX, LAN_CONFIGURATIONS_PDSIZE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_lan_configurations_pdwkuph : reg = &RU_REG(BBH_TX, LAN_CONFIGURATIONS_PDWKUPH); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_lan_configurations_pd_byte_th : reg = &RU_REG(BBH_TX, LAN_CONFIGURATIONS_PD_BYTE_TH); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_lan_configurations_pd_byte_th_en : reg = &RU_REG(BBH_TX, LAN_CONFIGURATIONS_PD_BYTE_TH_EN); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_lan_configurations_pdempty : reg = &RU_REG(BBH_TX, LAN_CONFIGURATIONS_PDEMPTY); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_lan_configurations_txthresh : reg = &RU_REG(BBH_TX, LAN_CONFIGURATIONS_TXTHRESH); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_lan_configurations_eee : reg = &RU_REG(BBH_TX, LAN_CONFIGURATIONS_EEE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_lan_configurations_ts : reg = &RU_REG(BBH_TX, LAN_CONFIGURATIONS_TS); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_unified_configurations_pdbase : reg = &RU_REG(BBH_TX, UNIFIED_CONFIGURATIONS_PDBASE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_unified_configurations_pdsize : reg = &RU_REG(BBH_TX, UNIFIED_CONFIGURATIONS_PDSIZE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_unified_configurations_pdwkuph : reg = &RU_REG(BBH_TX, UNIFIED_CONFIGURATIONS_PDWKUPH); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_unified_configurations_pd_byte_th : reg = &RU_REG(BBH_TX, UNIFIED_CONFIGURATIONS_PD_BYTE_TH); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_unified_configurations_pd_byte_th_en : reg = &RU_REG(BBH_TX, UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_unified_configurations_pdempty : reg = &RU_REG(BBH_TX, UNIFIED_CONFIGURATIONS_PDEMPTY); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_unified_configurations_gtxthresh : reg = &RU_REG(BBH_TX, UNIFIED_CONFIGURATIONS_GTXTHRESH); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_unified_configurations_eee : reg = &RU_REG(BBH_TX, UNIFIED_CONFIGURATIONS_EEE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_unified_configurations_ts : reg = &RU_REG(BBH_TX, UNIFIED_CONFIGURATIONS_TS); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_unified_configurations_febase : reg = &RU_REG(BBH_TX, UNIFIED_CONFIGURATIONS_FEBASE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_unified_configurations_fesize : reg = &RU_REG(BBH_TX, UNIFIED_CONFIGURATIONS_FESIZE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_unified_configurations_fepdbase : reg = &RU_REG(BBH_TX, UNIFIED_CONFIGURATIONS_FEPDBASE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_unified_configurations_fepdsize : reg = &RU_REG(BBH_TX, UNIFIED_CONFIGURATIONS_FEPDSIZE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_unified_configurations_txwrr : reg = &RU_REG(BBH_TX, UNIFIED_CONFIGURATIONS_TXWRR); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_unified_configurations_txthresh : reg = &RU_REG(BBH_TX, UNIFIED_CONFIGURATIONS_TXTHRESH); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_srampd : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_SRAMPD); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_ddrpd : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_DDRPD); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_pddrop : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_PDDROP); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_stscnt : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_STSCNT); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_stsdrop : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_STSDROP); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_msgcnt : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_MSGCNT); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_msgdrop : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_MSGDROP); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_getnextnull : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_GETNEXTNULL); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_flushpkts : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_FLUSHPKTS); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_lenerr : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_LENERR); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_aggrlenerr : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_AGGRLENERR); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_srampkt : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_SRAMPKT); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_ddrpkt : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_DDRPKT); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_srambyte : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_SRAMBYTE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_ddrbyte : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_DDRBYTE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_swrden : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_SWRDEN); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_swrdaddr : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_SWRDADDR); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_swrddata : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_SWRDDATA); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_unifiedpkt : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_UNIFIEDPKT); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_unifiedbyte : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_UNIFIEDBYTE); blk = &RU_BLK(BBH_TX); break;
    case bdmf_address_debug_counters_dbgoutreg : reg = &RU_REG(BBH_TX, DEBUG_COUNTERS_DBGOUTREG); blk = &RU_BLK(BBH_TX); break;
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

bdmfmon_handle_t ag_drv_bbh_tx_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "bbh_tx"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "bbh_tx", "bbh_tx", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_mac_type[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("type", "type", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_src_id[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("fpmsrc", "fpmsrc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sbpmsrc", "sbpmsrc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("stsrnrsrc", "stsrnrsrc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("msgrnrsrc", "msgrnrsrc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rnr_src_id[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pdrnr0src", "pdrnr0src", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pdrnr1src", "pdrnr1src", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_dma_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("dmasrc", "dmasrc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("descbase", "descbase", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("descsize", "descsize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_max_otf_read_request[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("maxreq", "maxreq", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dma_epon_urgent[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("epnurgnt", "epnurgnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_sdma_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("sdmasrc", "sdmasrc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("descbase", "descbase", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("descsize", "descsize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sdma_max_otf_read_request[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("maxreq", "maxreq", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sdma_epon_urgent[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("epnurgnt", "epnurgnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_ddr_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("bufsize", "bufsize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("byteresul", "byteresul", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddrtxoffset", "ddrtxoffset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hnsize0", "hnsize0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hnsize1", "hnsize1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_rnrcfg_1[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("rnr_cfg_index_1", "rnr_cfg_index_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tcontaddr", "tcontaddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("skbaddr", "skbaddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_rnrcfg_2[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("rnr_cfg_index_2", "rnr_cfg_index_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ptraddr", "ptraddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("task", "task", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_sbpmcfg[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("freenocntxt", "freenocntxt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("specialfree", "specialfree", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maxgn", "maxgn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_ddrtmbasel[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("addr0", "addr0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("addr1", "addr1", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_ddrtmbaseh[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("addr0", "addr0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("addr1", "addr1", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_dfifoctrl[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("psramsize", "psramsize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ddrsize", "ddrsize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("psrambase", "psrambase", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_arb_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("hightrxq", "hightrxq", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_bbroute[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("route", "route", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dest", "dest", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_q2rnr[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("q_2_rnr_index", "q_2_rnr_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q0", "q0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q1", "q1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_perqtask[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
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
        static bdmfmon_cmd_parm_t set_common_configurations_txrstcmd[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
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
        static bdmfmon_cmd_parm_t set_common_configurations_dbgsel[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("dbgsel", "dbgsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_clk_gate_cntrl[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intrvl", "keep_alive_intrvl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_pdbase[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("wan_pd_base_index", "wan_pd_base_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifobase0", "fifobase0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifobase1", "fifobase1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_pdsize[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("wan_pd_size_index", "wan_pd_size_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifosize0", "fifosize0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifosize1", "fifosize1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_pdwkuph[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("wan_pd_wkup_index", "wan_pd_wkup_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wkupthresh0", "wkupthresh0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wkupthresh1", "wkupthresh1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_pd_byte_th[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("wan_pd_byte_th_index", "wan_pd_byte_th_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pdlimit0", "pdlimit0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pdlimit1", "pdlimit1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_pd_byte_th_en[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pdlimiten", "pdlimiten", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_pdempty[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("empty", "empty", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_stsrnrcfg_1[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("wan_sts_rnr_cfg_1_index", "wan_sts_rnr_cfg_1_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tcontaddr", "tcontaddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_stsrnrcfg_2[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("wan_sts_rnr_cfg_2_index", "wan_sts_rnr_cfg_2_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ptraddr", "ptraddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("task", "task", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_msgrnrcfg_1[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("wan_msg_rnr_cfg_1_index", "wan_msg_rnr_cfg_1_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tcontaddr", "tcontaddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_msgrnrcfg_2[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("wan_msg_rnr_cfg_2_index", "wan_msg_rnr_cfg_2_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ptraddr", "ptraddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("task", "task", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_epncfg[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("stplenerr", "stplenerr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cmp_width", "cmp_width", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("considerfull", "considerfull", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("addcrc", "addcrc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_flow2port[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("wdata", "wdata", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("a", "a", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cmd", "cmd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_ts[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_maxwlen[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("maxwlen", "maxwlen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_flush[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("flush", "flush", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("srst_n", "srst_n", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lan_configurations_pdbase[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("fifobase0", "fifobase0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifobase1", "fifobase1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lan_configurations_pdsize[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("fifosize0", "fifosize0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifosize1", "fifosize1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lan_configurations_pdwkuph[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("wkupthresh0", "wkupthresh0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wkupthresh1", "wkupthresh1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lan_configurations_pd_byte_th[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pdlimit0", "pdlimit0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pdlimit1", "pdlimit1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lan_configurations_pd_byte_th_en[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pdlimiten", "pdlimiten", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lan_configurations_pdempty[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("empty", "empty", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lan_configurations_txthresh[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("ddrthresh", "ddrthresh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sramthresh", "sramthresh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lan_configurations_eee[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lan_configurations_ts[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_pdbase[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("unified_pd_base_index", "unified_pd_base_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifobase0", "fifobase0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifobase1", "fifobase1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_pdsize[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("unified_pd_size_index", "unified_pd_size_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifosize0", "fifosize0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifosize1", "fifosize1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_pdwkuph[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("unified_pd_wkup_index", "unified_pd_wkup_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wkupthresh0", "wkupthresh0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wkupthresh1", "wkupthresh1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_pd_byte_th[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("unified_pd_byte_th_index", "unified_pd_byte_th_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pdlimit0", "pdlimit0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pdlimit1", "pdlimit1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_pd_byte_th_en[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pdlimiten", "pdlimiten", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_pdempty[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("empty", "empty", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_gtxthresh[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("ddrthresh", "ddrthresh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sramthresh", "sramthresh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_eee[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_ts[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_febase[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("unified_fe_base_index", "unified_fe_base_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifobase0", "fifobase0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifobase1", "fifobase1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_fesize[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("unified_fe_size_index", "unified_fe_size_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifosize0", "fifosize0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifosize1", "fifosize1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_fepdbase[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("unified_fe_pd_base_index", "unified_fe_pd_base_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifobase0", "fifobase0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifobase1", "fifobase1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_fepdsize[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("unified_fe_pd_size_index", "unified_fe_pd_size_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifosize0", "fifosize0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifosize1", "fifosize1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_txwrr[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("unified_tx_wrr_index", "unified_tx_wrr_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("w0", "w0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("w1", "w1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_txthresh[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("unified_tx_thr_index", "unified_tx_thr_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("thresh0", "thresh0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("thresh1", "thresh1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_counters_swrden[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
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
        static bdmfmon_cmd_parm_t set_debug_counters_swrdaddr[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("rdaddr", "rdaddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="mac_type", .val=cli_bbh_tx_mac_type, .parms=set_mac_type },
            { .name="cfg_src_id", .val=cli_bbh_tx_cfg_src_id, .parms=set_cfg_src_id },
            { .name="rnr_src_id", .val=cli_bbh_tx_rnr_src_id, .parms=set_rnr_src_id },
            { .name="bbh_dma_cfg", .val=cli_bbh_tx_bbh_dma_cfg, .parms=set_bbh_dma_cfg },
            { .name="dma_max_otf_read_request", .val=cli_bbh_tx_dma_max_otf_read_request, .parms=set_dma_max_otf_read_request },
            { .name="dma_epon_urgent", .val=cli_bbh_tx_dma_epon_urgent, .parms=set_dma_epon_urgent },
            { .name="bbh_sdma_cfg", .val=cli_bbh_tx_bbh_sdma_cfg, .parms=set_bbh_sdma_cfg },
            { .name="sdma_max_otf_read_request", .val=cli_bbh_tx_sdma_max_otf_read_request, .parms=set_sdma_max_otf_read_request },
            { .name="sdma_epon_urgent", .val=cli_bbh_tx_sdma_epon_urgent, .parms=set_sdma_epon_urgent },
            { .name="bbh_ddr_cfg", .val=cli_bbh_tx_bbh_ddr_cfg, .parms=set_bbh_ddr_cfg },
            { .name="common_configurations_rnrcfg_1", .val=cli_bbh_tx_common_configurations_rnrcfg_1, .parms=set_common_configurations_rnrcfg_1 },
            { .name="common_configurations_rnrcfg_2", .val=cli_bbh_tx_common_configurations_rnrcfg_2, .parms=set_common_configurations_rnrcfg_2 },
            { .name="common_configurations_sbpmcfg", .val=cli_bbh_tx_common_configurations_sbpmcfg, .parms=set_common_configurations_sbpmcfg },
            { .name="common_configurations_ddrtmbasel", .val=cli_bbh_tx_common_configurations_ddrtmbasel, .parms=set_common_configurations_ddrtmbasel },
            { .name="common_configurations_ddrtmbaseh", .val=cli_bbh_tx_common_configurations_ddrtmbaseh, .parms=set_common_configurations_ddrtmbaseh },
            { .name="common_configurations_dfifoctrl", .val=cli_bbh_tx_common_configurations_dfifoctrl, .parms=set_common_configurations_dfifoctrl },
            { .name="common_configurations_arb_cfg", .val=cli_bbh_tx_common_configurations_arb_cfg, .parms=set_common_configurations_arb_cfg },
            { .name="common_configurations_bbroute", .val=cli_bbh_tx_common_configurations_bbroute, .parms=set_common_configurations_bbroute },
            { .name="common_configurations_q2rnr", .val=cli_bbh_tx_common_configurations_q2rnr, .parms=set_common_configurations_q2rnr },
            { .name="common_configurations_perqtask", .val=cli_bbh_tx_common_configurations_perqtask, .parms=set_common_configurations_perqtask },
            { .name="common_configurations_txrstcmd", .val=cli_bbh_tx_common_configurations_txrstcmd, .parms=set_common_configurations_txrstcmd },
            { .name="common_configurations_dbgsel", .val=cli_bbh_tx_common_configurations_dbgsel, .parms=set_common_configurations_dbgsel },
            { .name="common_configurations_clk_gate_cntrl", .val=cli_bbh_tx_common_configurations_clk_gate_cntrl, .parms=set_common_configurations_clk_gate_cntrl },
            { .name="wan_configurations_pdbase", .val=cli_bbh_tx_wan_configurations_pdbase, .parms=set_wan_configurations_pdbase },
            { .name="wan_configurations_pdsize", .val=cli_bbh_tx_wan_configurations_pdsize, .parms=set_wan_configurations_pdsize },
            { .name="wan_configurations_pdwkuph", .val=cli_bbh_tx_wan_configurations_pdwkuph, .parms=set_wan_configurations_pdwkuph },
            { .name="wan_configurations_pd_byte_th", .val=cli_bbh_tx_wan_configurations_pd_byte_th, .parms=set_wan_configurations_pd_byte_th },
            { .name="wan_configurations_pd_byte_th_en", .val=cli_bbh_tx_wan_configurations_pd_byte_th_en, .parms=set_wan_configurations_pd_byte_th_en },
            { .name="wan_configurations_pdempty", .val=cli_bbh_tx_wan_configurations_pdempty, .parms=set_wan_configurations_pdempty },
            { .name="wan_configurations_stsrnrcfg_1", .val=cli_bbh_tx_wan_configurations_stsrnrcfg_1, .parms=set_wan_configurations_stsrnrcfg_1 },
            { .name="wan_configurations_stsrnrcfg_2", .val=cli_bbh_tx_wan_configurations_stsrnrcfg_2, .parms=set_wan_configurations_stsrnrcfg_2 },
            { .name="wan_configurations_msgrnrcfg_1", .val=cli_bbh_tx_wan_configurations_msgrnrcfg_1, .parms=set_wan_configurations_msgrnrcfg_1 },
            { .name="wan_configurations_msgrnrcfg_2", .val=cli_bbh_tx_wan_configurations_msgrnrcfg_2, .parms=set_wan_configurations_msgrnrcfg_2 },
            { .name="wan_configurations_epncfg", .val=cli_bbh_tx_wan_configurations_epncfg, .parms=set_wan_configurations_epncfg },
            { .name="wan_configurations_flow2port", .val=cli_bbh_tx_wan_configurations_flow2port, .parms=set_wan_configurations_flow2port },
            { .name="wan_configurations_ts", .val=cli_bbh_tx_wan_configurations_ts, .parms=set_wan_configurations_ts },
            { .name="wan_configurations_maxwlen", .val=cli_bbh_tx_wan_configurations_maxwlen, .parms=set_wan_configurations_maxwlen },
            { .name="wan_configurations_flush", .val=cli_bbh_tx_wan_configurations_flush, .parms=set_wan_configurations_flush },
            { .name="lan_configurations_pdbase", .val=cli_bbh_tx_lan_configurations_pdbase, .parms=set_lan_configurations_pdbase },
            { .name="lan_configurations_pdsize", .val=cli_bbh_tx_lan_configurations_pdsize, .parms=set_lan_configurations_pdsize },
            { .name="lan_configurations_pdwkuph", .val=cli_bbh_tx_lan_configurations_pdwkuph, .parms=set_lan_configurations_pdwkuph },
            { .name="lan_configurations_pd_byte_th", .val=cli_bbh_tx_lan_configurations_pd_byte_th, .parms=set_lan_configurations_pd_byte_th },
            { .name="lan_configurations_pd_byte_th_en", .val=cli_bbh_tx_lan_configurations_pd_byte_th_en, .parms=set_lan_configurations_pd_byte_th_en },
            { .name="lan_configurations_pdempty", .val=cli_bbh_tx_lan_configurations_pdempty, .parms=set_lan_configurations_pdempty },
            { .name="lan_configurations_txthresh", .val=cli_bbh_tx_lan_configurations_txthresh, .parms=set_lan_configurations_txthresh },
            { .name="lan_configurations_eee", .val=cli_bbh_tx_lan_configurations_eee, .parms=set_lan_configurations_eee },
            { .name="lan_configurations_ts", .val=cli_bbh_tx_lan_configurations_ts, .parms=set_lan_configurations_ts },
            { .name="unified_configurations_pdbase", .val=cli_bbh_tx_unified_configurations_pdbase, .parms=set_unified_configurations_pdbase },
            { .name="unified_configurations_pdsize", .val=cli_bbh_tx_unified_configurations_pdsize, .parms=set_unified_configurations_pdsize },
            { .name="unified_configurations_pdwkuph", .val=cli_bbh_tx_unified_configurations_pdwkuph, .parms=set_unified_configurations_pdwkuph },
            { .name="unified_configurations_pd_byte_th", .val=cli_bbh_tx_unified_configurations_pd_byte_th, .parms=set_unified_configurations_pd_byte_th },
            { .name="unified_configurations_pd_byte_th_en", .val=cli_bbh_tx_unified_configurations_pd_byte_th_en, .parms=set_unified_configurations_pd_byte_th_en },
            { .name="unified_configurations_pdempty", .val=cli_bbh_tx_unified_configurations_pdempty, .parms=set_unified_configurations_pdempty },
            { .name="unified_configurations_gtxthresh", .val=cli_bbh_tx_unified_configurations_gtxthresh, .parms=set_unified_configurations_gtxthresh },
            { .name="unified_configurations_eee", .val=cli_bbh_tx_unified_configurations_eee, .parms=set_unified_configurations_eee },
            { .name="unified_configurations_ts", .val=cli_bbh_tx_unified_configurations_ts, .parms=set_unified_configurations_ts },
            { .name="unified_configurations_febase", .val=cli_bbh_tx_unified_configurations_febase, .parms=set_unified_configurations_febase },
            { .name="unified_configurations_fesize", .val=cli_bbh_tx_unified_configurations_fesize, .parms=set_unified_configurations_fesize },
            { .name="unified_configurations_fepdbase", .val=cli_bbh_tx_unified_configurations_fepdbase, .parms=set_unified_configurations_fepdbase },
            { .name="unified_configurations_fepdsize", .val=cli_bbh_tx_unified_configurations_fepdsize, .parms=set_unified_configurations_fepdsize },
            { .name="unified_configurations_txwrr", .val=cli_bbh_tx_unified_configurations_txwrr, .parms=set_unified_configurations_txwrr },
            { .name="unified_configurations_txthresh", .val=cli_bbh_tx_unified_configurations_txthresh, .parms=set_unified_configurations_txthresh },
            { .name="debug_counters_swrden", .val=cli_bbh_tx_debug_counters_swrden, .parms=set_debug_counters_swrden },
            { .name="debug_counters_swrdaddr", .val=cli_bbh_tx_debug_counters_swrdaddr, .parms=set_debug_counters_swrdaddr },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_bbh_tx_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_rnrcfg_1[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_cfg_index_1", "rnr_cfg_index_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_rnrcfg_2[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rnr_cfg_index_2", "rnr_cfg_index_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_ddrtmbasel[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_ddrtmbaseh[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_common_configurations_q2rnr[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("q_2_rnr_index", "q_2_rnr_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_pdbase[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wan_pd_base_index", "wan_pd_base_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_pdsize[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wan_pd_size_index", "wan_pd_size_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_pdwkuph[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wan_pd_wkup_index", "wan_pd_wkup_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_pd_byte_th[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wan_pd_byte_th_index", "wan_pd_byte_th_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_stsrnrcfg_1[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wan_sts_rnr_cfg_1_index", "wan_sts_rnr_cfg_1_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_stsrnrcfg_2[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wan_sts_rnr_cfg_2_index", "wan_sts_rnr_cfg_2_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_msgrnrcfg_1[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wan_msg_rnr_cfg_1_index", "wan_msg_rnr_cfg_1_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_wan_configurations_msgrnrcfg_2[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wan_msg_rnr_cfg_2_index", "wan_msg_rnr_cfg_2_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_pdbase[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unified_pd_base_index", "unified_pd_base_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_pdsize[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unified_pd_size_index", "unified_pd_size_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_pdwkuph[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unified_pd_wkup_index", "unified_pd_wkup_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_pd_byte_th[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unified_pd_byte_th_index", "unified_pd_byte_th_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_febase[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unified_fe_base_index", "unified_fe_base_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_fesize[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unified_fe_size_index", "unified_fe_size_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_fepdbase[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unified_fe_pd_base_index", "unified_fe_pd_base_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_fepdsize[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unified_fe_pd_size_index", "unified_fe_pd_size_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_txwrr[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unified_tx_wrr_index", "unified_tx_wrr_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unified_configurations_txthresh[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unified_tx_thr_index", "unified_tx_thr_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_counters_unifiedpkt[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("debug_unified_pkt_ctr_idx", "debug_unified_pkt_ctr_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_counters_unifiedbyte[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("debug_unified_byte_ctr_idx", "debug_unified_byte_ctr_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_counters_dbgoutreg[]={
            BDMFMON_MAKE_PARM("bbh_id", "bbh_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="mac_type", .val=cli_bbh_tx_mac_type, .parms=set_default },
            { .name="cfg_src_id", .val=cli_bbh_tx_cfg_src_id, .parms=set_default },
            { .name="rnr_src_id", .val=cli_bbh_tx_rnr_src_id, .parms=set_default },
            { .name="bbh_dma_cfg", .val=cli_bbh_tx_bbh_dma_cfg, .parms=set_default },
            { .name="dma_max_otf_read_request", .val=cli_bbh_tx_dma_max_otf_read_request, .parms=set_default },
            { .name="dma_epon_urgent", .val=cli_bbh_tx_dma_epon_urgent, .parms=set_default },
            { .name="bbh_sdma_cfg", .val=cli_bbh_tx_bbh_sdma_cfg, .parms=set_default },
            { .name="sdma_max_otf_read_request", .val=cli_bbh_tx_sdma_max_otf_read_request, .parms=set_default },
            { .name="sdma_epon_urgent", .val=cli_bbh_tx_sdma_epon_urgent, .parms=set_default },
            { .name="bbh_ddr_cfg", .val=cli_bbh_tx_bbh_ddr_cfg, .parms=set_default },
            { .name="debug_counters", .val=cli_bbh_tx_debug_counters, .parms=set_default },
            { .name="common_configurations_rnrcfg_1", .val=cli_bbh_tx_common_configurations_rnrcfg_1, .parms=set_common_configurations_rnrcfg_1 },
            { .name="common_configurations_rnrcfg_2", .val=cli_bbh_tx_common_configurations_rnrcfg_2, .parms=set_common_configurations_rnrcfg_2 },
            { .name="common_configurations_sbpmcfg", .val=cli_bbh_tx_common_configurations_sbpmcfg, .parms=set_default },
            { .name="common_configurations_ddrtmbasel", .val=cli_bbh_tx_common_configurations_ddrtmbasel, .parms=set_common_configurations_ddrtmbasel },
            { .name="common_configurations_ddrtmbaseh", .val=cli_bbh_tx_common_configurations_ddrtmbaseh, .parms=set_common_configurations_ddrtmbaseh },
            { .name="common_configurations_dfifoctrl", .val=cli_bbh_tx_common_configurations_dfifoctrl, .parms=set_default },
            { .name="common_configurations_arb_cfg", .val=cli_bbh_tx_common_configurations_arb_cfg, .parms=set_default },
            { .name="common_configurations_bbroute", .val=cli_bbh_tx_common_configurations_bbroute, .parms=set_default },
            { .name="common_configurations_q2rnr", .val=cli_bbh_tx_common_configurations_q2rnr, .parms=set_common_configurations_q2rnr },
            { .name="common_configurations_perqtask", .val=cli_bbh_tx_common_configurations_perqtask, .parms=set_default },
            { .name="common_configurations_txrstcmd", .val=cli_bbh_tx_common_configurations_txrstcmd, .parms=set_default },
            { .name="common_configurations_dbgsel", .val=cli_bbh_tx_common_configurations_dbgsel, .parms=set_default },
            { .name="common_configurations_clk_gate_cntrl", .val=cli_bbh_tx_common_configurations_clk_gate_cntrl, .parms=set_default },
            { .name="wan_configurations_pdbase", .val=cli_bbh_tx_wan_configurations_pdbase, .parms=set_wan_configurations_pdbase },
            { .name="wan_configurations_pdsize", .val=cli_bbh_tx_wan_configurations_pdsize, .parms=set_wan_configurations_pdsize },
            { .name="wan_configurations_pdwkuph", .val=cli_bbh_tx_wan_configurations_pdwkuph, .parms=set_wan_configurations_pdwkuph },
            { .name="wan_configurations_pd_byte_th", .val=cli_bbh_tx_wan_configurations_pd_byte_th, .parms=set_wan_configurations_pd_byte_th },
            { .name="wan_configurations_pd_byte_th_en", .val=cli_bbh_tx_wan_configurations_pd_byte_th_en, .parms=set_default },
            { .name="wan_configurations_pdempty", .val=cli_bbh_tx_wan_configurations_pdempty, .parms=set_default },
            { .name="wan_configurations_stsrnrcfg_1", .val=cli_bbh_tx_wan_configurations_stsrnrcfg_1, .parms=set_wan_configurations_stsrnrcfg_1 },
            { .name="wan_configurations_stsrnrcfg_2", .val=cli_bbh_tx_wan_configurations_stsrnrcfg_2, .parms=set_wan_configurations_stsrnrcfg_2 },
            { .name="wan_configurations_msgrnrcfg_1", .val=cli_bbh_tx_wan_configurations_msgrnrcfg_1, .parms=set_wan_configurations_msgrnrcfg_1 },
            { .name="wan_configurations_msgrnrcfg_2", .val=cli_bbh_tx_wan_configurations_msgrnrcfg_2, .parms=set_wan_configurations_msgrnrcfg_2 },
            { .name="wan_configurations_epncfg", .val=cli_bbh_tx_wan_configurations_epncfg, .parms=set_default },
            { .name="wan_configurations_flow2port", .val=cli_bbh_tx_wan_configurations_flow2port, .parms=set_default },
            { .name="wan_configurations_ts", .val=cli_bbh_tx_wan_configurations_ts, .parms=set_default },
            { .name="wan_configurations_maxwlen", .val=cli_bbh_tx_wan_configurations_maxwlen, .parms=set_default },
            { .name="wan_configurations_flush", .val=cli_bbh_tx_wan_configurations_flush, .parms=set_default },
            { .name="lan_configurations_pdbase", .val=cli_bbh_tx_lan_configurations_pdbase, .parms=set_default },
            { .name="lan_configurations_pdsize", .val=cli_bbh_tx_lan_configurations_pdsize, .parms=set_default },
            { .name="lan_configurations_pdwkuph", .val=cli_bbh_tx_lan_configurations_pdwkuph, .parms=set_default },
            { .name="lan_configurations_pd_byte_th", .val=cli_bbh_tx_lan_configurations_pd_byte_th, .parms=set_default },
            { .name="lan_configurations_pd_byte_th_en", .val=cli_bbh_tx_lan_configurations_pd_byte_th_en, .parms=set_default },
            { .name="lan_configurations_pdempty", .val=cli_bbh_tx_lan_configurations_pdempty, .parms=set_default },
            { .name="lan_configurations_txthresh", .val=cli_bbh_tx_lan_configurations_txthresh, .parms=set_default },
            { .name="lan_configurations_eee", .val=cli_bbh_tx_lan_configurations_eee, .parms=set_default },
            { .name="lan_configurations_ts", .val=cli_bbh_tx_lan_configurations_ts, .parms=set_default },
            { .name="unified_configurations_pdbase", .val=cli_bbh_tx_unified_configurations_pdbase, .parms=set_unified_configurations_pdbase },
            { .name="unified_configurations_pdsize", .val=cli_bbh_tx_unified_configurations_pdsize, .parms=set_unified_configurations_pdsize },
            { .name="unified_configurations_pdwkuph", .val=cli_bbh_tx_unified_configurations_pdwkuph, .parms=set_unified_configurations_pdwkuph },
            { .name="unified_configurations_pd_byte_th", .val=cli_bbh_tx_unified_configurations_pd_byte_th, .parms=set_unified_configurations_pd_byte_th },
            { .name="unified_configurations_pd_byte_th_en", .val=cli_bbh_tx_unified_configurations_pd_byte_th_en, .parms=set_default },
            { .name="unified_configurations_pdempty", .val=cli_bbh_tx_unified_configurations_pdempty, .parms=set_default },
            { .name="unified_configurations_gtxthresh", .val=cli_bbh_tx_unified_configurations_gtxthresh, .parms=set_default },
            { .name="unified_configurations_eee", .val=cli_bbh_tx_unified_configurations_eee, .parms=set_default },
            { .name="unified_configurations_ts", .val=cli_bbh_tx_unified_configurations_ts, .parms=set_default },
            { .name="unified_configurations_febase", .val=cli_bbh_tx_unified_configurations_febase, .parms=set_unified_configurations_febase },
            { .name="unified_configurations_fesize", .val=cli_bbh_tx_unified_configurations_fesize, .parms=set_unified_configurations_fesize },
            { .name="unified_configurations_fepdbase", .val=cli_bbh_tx_unified_configurations_fepdbase, .parms=set_unified_configurations_fepdbase },
            { .name="unified_configurations_fepdsize", .val=cli_bbh_tx_unified_configurations_fepdsize, .parms=set_unified_configurations_fepdsize },
            { .name="unified_configurations_txwrr", .val=cli_bbh_tx_unified_configurations_txwrr, .parms=set_unified_configurations_txwrr },
            { .name="unified_configurations_txthresh", .val=cli_bbh_tx_unified_configurations_txthresh, .parms=set_unified_configurations_txthresh },
            { .name="debug_counters_srambyte", .val=cli_bbh_tx_debug_counters_srambyte, .parms=set_default },
            { .name="debug_counters_ddrbyte", .val=cli_bbh_tx_debug_counters_ddrbyte, .parms=set_default },
            { .name="debug_counters_swrden", .val=cli_bbh_tx_debug_counters_swrden, .parms=set_default },
            { .name="debug_counters_swrdaddr", .val=cli_bbh_tx_debug_counters_swrdaddr, .parms=set_default },
            { .name="debug_counters_swrddata", .val=cli_bbh_tx_debug_counters_swrddata, .parms=set_default },
            { .name="debug_counters_unifiedpkt", .val=cli_bbh_tx_debug_counters_unifiedpkt, .parms=set_debug_counters_unifiedpkt },
            { .name="debug_counters_unifiedbyte", .val=cli_bbh_tx_debug_counters_unifiedbyte, .parms=set_debug_counters_unifiedbyte },
            { .name="debug_counters_dbgoutreg", .val=cli_bbh_tx_debug_counters_dbgoutreg, .parms=set_debug_counters_dbgoutreg },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_bbh_tx_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_bbh_tx_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("bbh_id", "bbh_id", bbh_id_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="COMMON_CONFIGURATIONS_MACTYPE" , .val=bdmf_address_common_configurations_mactype },
            { .name="COMMON_CONFIGURATIONS_BBCFG_1_TX" , .val=bdmf_address_common_configurations_bbcfg_1_tx },
            { .name="COMMON_CONFIGURATIONS_BBCFG_2_TX" , .val=bdmf_address_common_configurations_bbcfg_2_tx },
            { .name="COMMON_CONFIGURATIONS_DDRCFG_TX" , .val=bdmf_address_common_configurations_ddrcfg_tx },
            { .name="COMMON_CONFIGURATIONS_RNRCFG_1" , .val=bdmf_address_common_configurations_rnrcfg_1 },
            { .name="COMMON_CONFIGURATIONS_RNRCFG_2" , .val=bdmf_address_common_configurations_rnrcfg_2 },
            { .name="COMMON_CONFIGURATIONS_DMACFG_TX" , .val=bdmf_address_common_configurations_dmacfg_tx },
            { .name="COMMON_CONFIGURATIONS_SDMACFG_TX" , .val=bdmf_address_common_configurations_sdmacfg_tx },
            { .name="COMMON_CONFIGURATIONS_SBPMCFG" , .val=bdmf_address_common_configurations_sbpmcfg },
            { .name="COMMON_CONFIGURATIONS_DDRTMBASEL" , .val=bdmf_address_common_configurations_ddrtmbasel },
            { .name="COMMON_CONFIGURATIONS_DDRTMBASEH" , .val=bdmf_address_common_configurations_ddrtmbaseh },
            { .name="COMMON_CONFIGURATIONS_DFIFOCTRL" , .val=bdmf_address_common_configurations_dfifoctrl },
            { .name="COMMON_CONFIGURATIONS_ARB_CFG" , .val=bdmf_address_common_configurations_arb_cfg },
            { .name="COMMON_CONFIGURATIONS_BBROUTE" , .val=bdmf_address_common_configurations_bbroute },
            { .name="COMMON_CONFIGURATIONS_Q2RNR" , .val=bdmf_address_common_configurations_q2rnr },
            { .name="COMMON_CONFIGURATIONS_PERQTASK" , .val=bdmf_address_common_configurations_perqtask },
            { .name="COMMON_CONFIGURATIONS_TXRSTCMD" , .val=bdmf_address_common_configurations_txrstcmd },
            { .name="COMMON_CONFIGURATIONS_DBGSEL" , .val=bdmf_address_common_configurations_dbgsel },
            { .name="COMMON_CONFIGURATIONS_CLK_GATE_CNTRL" , .val=bdmf_address_common_configurations_clk_gate_cntrl },
            { .name="WAN_CONFIGURATIONS_PDBASE" , .val=bdmf_address_wan_configurations_pdbase },
            { .name="WAN_CONFIGURATIONS_PDSIZE" , .val=bdmf_address_wan_configurations_pdsize },
            { .name="WAN_CONFIGURATIONS_PDWKUPH" , .val=bdmf_address_wan_configurations_pdwkuph },
            { .name="WAN_CONFIGURATIONS_PD_BYTE_TH" , .val=bdmf_address_wan_configurations_pd_byte_th },
            { .name="WAN_CONFIGURATIONS_PD_BYTE_TH_EN" , .val=bdmf_address_wan_configurations_pd_byte_th_en },
            { .name="WAN_CONFIGURATIONS_PDEMPTY" , .val=bdmf_address_wan_configurations_pdempty },
            { .name="WAN_CONFIGURATIONS_STSRNRCFG_1" , .val=bdmf_address_wan_configurations_stsrnrcfg_1 },
            { .name="WAN_CONFIGURATIONS_STSRNRCFG_2" , .val=bdmf_address_wan_configurations_stsrnrcfg_2 },
            { .name="WAN_CONFIGURATIONS_MSGRNRCFG_1" , .val=bdmf_address_wan_configurations_msgrnrcfg_1 },
            { .name="WAN_CONFIGURATIONS_MSGRNRCFG_2" , .val=bdmf_address_wan_configurations_msgrnrcfg_2 },
            { .name="WAN_CONFIGURATIONS_EPNCFG" , .val=bdmf_address_wan_configurations_epncfg },
            { .name="WAN_CONFIGURATIONS_FLOW2PORT" , .val=bdmf_address_wan_configurations_flow2port },
            { .name="WAN_CONFIGURATIONS_TS" , .val=bdmf_address_wan_configurations_ts },
            { .name="WAN_CONFIGURATIONS_MAXWLEN" , .val=bdmf_address_wan_configurations_maxwlen },
            { .name="WAN_CONFIGURATIONS_FLUSH" , .val=bdmf_address_wan_configurations_flush },
            { .name="LAN_CONFIGURATIONS_PDBASE" , .val=bdmf_address_lan_configurations_pdbase },
            { .name="LAN_CONFIGURATIONS_PDSIZE" , .val=bdmf_address_lan_configurations_pdsize },
            { .name="LAN_CONFIGURATIONS_PDWKUPH" , .val=bdmf_address_lan_configurations_pdwkuph },
            { .name="LAN_CONFIGURATIONS_PD_BYTE_TH" , .val=bdmf_address_lan_configurations_pd_byte_th },
            { .name="LAN_CONFIGURATIONS_PD_BYTE_TH_EN" , .val=bdmf_address_lan_configurations_pd_byte_th_en },
            { .name="LAN_CONFIGURATIONS_PDEMPTY" , .val=bdmf_address_lan_configurations_pdempty },
            { .name="LAN_CONFIGURATIONS_TXTHRESH" , .val=bdmf_address_lan_configurations_txthresh },
            { .name="LAN_CONFIGURATIONS_EEE" , .val=bdmf_address_lan_configurations_eee },
            { .name="LAN_CONFIGURATIONS_TS" , .val=bdmf_address_lan_configurations_ts },
            { .name="UNIFIED_CONFIGURATIONS_PDBASE" , .val=bdmf_address_unified_configurations_pdbase },
            { .name="UNIFIED_CONFIGURATIONS_PDSIZE" , .val=bdmf_address_unified_configurations_pdsize },
            { .name="UNIFIED_CONFIGURATIONS_PDWKUPH" , .val=bdmf_address_unified_configurations_pdwkuph },
            { .name="UNIFIED_CONFIGURATIONS_PD_BYTE_TH" , .val=bdmf_address_unified_configurations_pd_byte_th },
            { .name="UNIFIED_CONFIGURATIONS_PD_BYTE_TH_EN" , .val=bdmf_address_unified_configurations_pd_byte_th_en },
            { .name="UNIFIED_CONFIGURATIONS_PDEMPTY" , .val=bdmf_address_unified_configurations_pdempty },
            { .name="UNIFIED_CONFIGURATIONS_GTXTHRESH" , .val=bdmf_address_unified_configurations_gtxthresh },
            { .name="UNIFIED_CONFIGURATIONS_EEE" , .val=bdmf_address_unified_configurations_eee },
            { .name="UNIFIED_CONFIGURATIONS_TS" , .val=bdmf_address_unified_configurations_ts },
            { .name="UNIFIED_CONFIGURATIONS_FEBASE" , .val=bdmf_address_unified_configurations_febase },
            { .name="UNIFIED_CONFIGURATIONS_FESIZE" , .val=bdmf_address_unified_configurations_fesize },
            { .name="UNIFIED_CONFIGURATIONS_FEPDBASE" , .val=bdmf_address_unified_configurations_fepdbase },
            { .name="UNIFIED_CONFIGURATIONS_FEPDSIZE" , .val=bdmf_address_unified_configurations_fepdsize },
            { .name="UNIFIED_CONFIGURATIONS_TXWRR" , .val=bdmf_address_unified_configurations_txwrr },
            { .name="UNIFIED_CONFIGURATIONS_TXTHRESH" , .val=bdmf_address_unified_configurations_txthresh },
            { .name="DEBUG_COUNTERS_SRAMPD" , .val=bdmf_address_debug_counters_srampd },
            { .name="DEBUG_COUNTERS_DDRPD" , .val=bdmf_address_debug_counters_ddrpd },
            { .name="DEBUG_COUNTERS_PDDROP" , .val=bdmf_address_debug_counters_pddrop },
            { .name="DEBUG_COUNTERS_STSCNT" , .val=bdmf_address_debug_counters_stscnt },
            { .name="DEBUG_COUNTERS_STSDROP" , .val=bdmf_address_debug_counters_stsdrop },
            { .name="DEBUG_COUNTERS_MSGCNT" , .val=bdmf_address_debug_counters_msgcnt },
            { .name="DEBUG_COUNTERS_MSGDROP" , .val=bdmf_address_debug_counters_msgdrop },
            { .name="DEBUG_COUNTERS_GETNEXTNULL" , .val=bdmf_address_debug_counters_getnextnull },
            { .name="DEBUG_COUNTERS_FLUSHPKTS" , .val=bdmf_address_debug_counters_flushpkts },
            { .name="DEBUG_COUNTERS_LENERR" , .val=bdmf_address_debug_counters_lenerr },
            { .name="DEBUG_COUNTERS_AGGRLENERR" , .val=bdmf_address_debug_counters_aggrlenerr },
            { .name="DEBUG_COUNTERS_SRAMPKT" , .val=bdmf_address_debug_counters_srampkt },
            { .name="DEBUG_COUNTERS_DDRPKT" , .val=bdmf_address_debug_counters_ddrpkt },
            { .name="DEBUG_COUNTERS_SRAMBYTE" , .val=bdmf_address_debug_counters_srambyte },
            { .name="DEBUG_COUNTERS_DDRBYTE" , .val=bdmf_address_debug_counters_ddrbyte },
            { .name="DEBUG_COUNTERS_SWRDEN" , .val=bdmf_address_debug_counters_swrden },
            { .name="DEBUG_COUNTERS_SWRDADDR" , .val=bdmf_address_debug_counters_swrdaddr },
            { .name="DEBUG_COUNTERS_SWRDDATA" , .val=bdmf_address_debug_counters_swrddata },
            { .name="DEBUG_COUNTERS_UNIFIEDPKT" , .val=bdmf_address_debug_counters_unifiedpkt },
            { .name="DEBUG_COUNTERS_UNIFIEDBYTE" , .val=bdmf_address_debug_counters_unifiedbyte },
            { .name="DEBUG_COUNTERS_DBGOUTREG" , .val=bdmf_address_debug_counters_dbgoutreg },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_bbh_tx_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "bbh_id", bbh_id_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

