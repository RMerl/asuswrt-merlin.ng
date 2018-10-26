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
#include "xrdp_drv_dma_ag.h"

#define BLOCK_ADDR_COUNT_BITS 1
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_dma_debug_info_set(uint8_t dma_id, const dma_debug_info *debug_info)
{
    uint32_t reg_debug_nempty=0;
    uint32_t reg_debug_urgnt=0;
    uint32_t reg_debug_selsrc=0;
    uint32_t reg_debug_rdadd=0;
    uint32_t reg_debug_rdvalid=0;
    uint32_t reg_debug_rddatardy=0;

#ifdef VALIDATE_PARMS
    if(!debug_info)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (debug_info->sel_src >= _6BITS_MAX_VAL_) ||
       (debug_info->address >= _10BITS_MAX_VAL_) ||
       (debug_info->datacs >= _1BITS_MAX_VAL_) ||
       (debug_info->cdcs >= _1BITS_MAX_VAL_) ||
       (debug_info->rrcs >= _1BITS_MAX_VAL_) ||
       (debug_info->valid >= _1BITS_MAX_VAL_) ||
       (debug_info->ready >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_nempty = RU_FIELD_SET(dma_id, DMA, DEBUG_NEMPTY, NEMPTY, reg_debug_nempty, debug_info->nempty);
    reg_debug_urgnt = RU_FIELD_SET(dma_id, DMA, DEBUG_URGNT, URGNT, reg_debug_urgnt, debug_info->urgnt);
    reg_debug_selsrc = RU_FIELD_SET(dma_id, DMA, DEBUG_SELSRC, SEL_SRC, reg_debug_selsrc, debug_info->sel_src);
    reg_debug_rdadd = RU_FIELD_SET(dma_id, DMA, DEBUG_RDADD, ADDRESS, reg_debug_rdadd, debug_info->address);
    reg_debug_rdadd = RU_FIELD_SET(dma_id, DMA, DEBUG_RDADD, DATACS, reg_debug_rdadd, debug_info->datacs);
    reg_debug_rdadd = RU_FIELD_SET(dma_id, DMA, DEBUG_RDADD, CDCS, reg_debug_rdadd, debug_info->cdcs);
    reg_debug_rdadd = RU_FIELD_SET(dma_id, DMA, DEBUG_RDADD, RRCS, reg_debug_rdadd, debug_info->rrcs);
    reg_debug_rdvalid = RU_FIELD_SET(dma_id, DMA, DEBUG_RDVALID, VALID, reg_debug_rdvalid, debug_info->valid);
    reg_debug_rddatardy = RU_FIELD_SET(dma_id, DMA, DEBUG_RDDATARDY, READY, reg_debug_rddatardy, debug_info->ready);

    RU_REG_WRITE(dma_id, DMA, DEBUG_NEMPTY, reg_debug_nempty);
    RU_REG_WRITE(dma_id, DMA, DEBUG_URGNT, reg_debug_urgnt);
    RU_REG_WRITE(dma_id, DMA, DEBUG_SELSRC, reg_debug_selsrc);
    RU_REG_WRITE(dma_id, DMA, DEBUG_RDADD, reg_debug_rdadd);
    RU_REG_WRITE(dma_id, DMA, DEBUG_RDVALID, reg_debug_rdvalid);
    RU_REG_WRITE(dma_id, DMA, DEBUG_RDDATARDY, reg_debug_rddatardy);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_debug_info_get(uint8_t dma_id, dma_debug_info *debug_info)
{
    uint32_t reg_debug_nempty;
    uint32_t reg_debug_urgnt;
    uint32_t reg_debug_selsrc;
    uint32_t reg_debug_rdadd;
    uint32_t reg_debug_rdvalid;
    uint32_t reg_debug_rddatardy;

#ifdef VALIDATE_PARMS
    if(!debug_info)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dma_id, DMA, DEBUG_NEMPTY, reg_debug_nempty);
    RU_REG_READ(dma_id, DMA, DEBUG_URGNT, reg_debug_urgnt);
    RU_REG_READ(dma_id, DMA, DEBUG_SELSRC, reg_debug_selsrc);
    RU_REG_READ(dma_id, DMA, DEBUG_RDADD, reg_debug_rdadd);
    RU_REG_READ(dma_id, DMA, DEBUG_RDVALID, reg_debug_rdvalid);
    RU_REG_READ(dma_id, DMA, DEBUG_RDDATARDY, reg_debug_rddatardy);

    debug_info->nempty = RU_FIELD_GET(dma_id, DMA, DEBUG_NEMPTY, NEMPTY, reg_debug_nempty);
    debug_info->urgnt = RU_FIELD_GET(dma_id, DMA, DEBUG_URGNT, URGNT, reg_debug_urgnt);
    debug_info->sel_src = RU_FIELD_GET(dma_id, DMA, DEBUG_SELSRC, SEL_SRC, reg_debug_selsrc);
    debug_info->address = RU_FIELD_GET(dma_id, DMA, DEBUG_RDADD, ADDRESS, reg_debug_rdadd);
    debug_info->datacs = RU_FIELD_GET(dma_id, DMA, DEBUG_RDADD, DATACS, reg_debug_rdadd);
    debug_info->cdcs = RU_FIELD_GET(dma_id, DMA, DEBUG_RDADD, CDCS, reg_debug_rdadd);
    debug_info->rrcs = RU_FIELD_GET(dma_id, DMA, DEBUG_RDADD, RRCS, reg_debug_rdadd);
    debug_info->valid = RU_FIELD_GET(dma_id, DMA, DEBUG_RDVALID, VALID, reg_debug_rdvalid);
    debug_info->ready = RU_FIELD_GET(dma_id, DMA, DEBUG_RDDATARDY, READY, reg_debug_rddatardy);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_bbrouteovrd_set(uint8_t dma_id, uint8_t dest, uint16_t route, bdmf_boolean ovrd)
{
    uint32_t reg_config_bbrouteovrd=0;

#ifdef VALIDATE_PARMS
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (dest >= _6BITS_MAX_VAL_) ||
       (route >= _10BITS_MAX_VAL_) ||
       (ovrd >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_config_bbrouteovrd = RU_FIELD_SET(dma_id, DMA, CONFIG_BBROUTEOVRD, DEST, reg_config_bbrouteovrd, dest);
    reg_config_bbrouteovrd = RU_FIELD_SET(dma_id, DMA, CONFIG_BBROUTEOVRD, ROUTE, reg_config_bbrouteovrd, route);
    reg_config_bbrouteovrd = RU_FIELD_SET(dma_id, DMA, CONFIG_BBROUTEOVRD, OVRD, reg_config_bbrouteovrd, ovrd);

    RU_REG_WRITE(dma_id, DMA, CONFIG_BBROUTEOVRD, reg_config_bbrouteovrd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_bbrouteovrd_get(uint8_t dma_id, uint8_t *dest, uint16_t *route, bdmf_boolean *ovrd)
{
    uint32_t reg_config_bbrouteovrd;

#ifdef VALIDATE_PARMS
    if(!dest || !route || !ovrd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dma_id, DMA, CONFIG_BBROUTEOVRD, reg_config_bbrouteovrd);

    *dest = RU_FIELD_GET(dma_id, DMA, CONFIG_BBROUTEOVRD, DEST, reg_config_bbrouteovrd);
    *route = RU_FIELD_GET(dma_id, DMA, CONFIG_BBROUTEOVRD, ROUTE, reg_config_bbrouteovrd);
    *ovrd = RU_FIELD_GET(dma_id, DMA, CONFIG_BBROUTEOVRD, OVRD, reg_config_bbrouteovrd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_num_of_writes_set(uint8_t dma_id, uint8_t emac_index, uint8_t numofbuff)
{
    uint32_t reg_config_num_of_writes=0;

#ifdef VALIDATE_PARMS
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 8) ||
       (numofbuff >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_config_num_of_writes = RU_FIELD_SET(dma_id, DMA, CONFIG_NUM_OF_WRITES, NUMOFBUFF, reg_config_num_of_writes, numofbuff);

    RU_REG_RAM_WRITE(dma_id, emac_index, DMA, CONFIG_NUM_OF_WRITES, reg_config_num_of_writes);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_num_of_writes_get(uint8_t dma_id, uint8_t emac_index, uint8_t *numofbuff)
{
    uint32_t reg_config_num_of_writes;

#ifdef VALIDATE_PARMS
    if(!numofbuff)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(dma_id, emac_index, DMA, CONFIG_NUM_OF_WRITES, reg_config_num_of_writes);

    *numofbuff = RU_FIELD_GET(dma_id, DMA, CONFIG_NUM_OF_WRITES, NUMOFBUFF, reg_config_num_of_writes);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_num_of_reads_set(uint8_t dma_id, uint8_t emac_index, uint8_t rr_num)
{
    uint32_t reg_config_num_of_reads=0;

#ifdef VALIDATE_PARMS
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 8) ||
       (rr_num >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_config_num_of_reads = RU_FIELD_SET(dma_id, DMA, CONFIG_NUM_OF_READS, RR_NUM, reg_config_num_of_reads, rr_num);

    RU_REG_RAM_WRITE(dma_id, emac_index, DMA, CONFIG_NUM_OF_READS, reg_config_num_of_reads);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_num_of_reads_get(uint8_t dma_id, uint8_t emac_index, uint8_t *rr_num)
{
    uint32_t reg_config_num_of_reads;

#ifdef VALIDATE_PARMS
    if(!rr_num)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(dma_id, emac_index, DMA, CONFIG_NUM_OF_READS, reg_config_num_of_reads);

    *rr_num = RU_FIELD_GET(dma_id, DMA, CONFIG_NUM_OF_READS, RR_NUM, reg_config_num_of_reads);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_u_thresh_set(uint8_t dma_id, uint8_t emac_index, uint8_t into_u, uint8_t out_of_u)
{
    uint32_t reg_config_u_thresh=0;

#ifdef VALIDATE_PARMS
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 8) ||
       (into_u >= _6BITS_MAX_VAL_) ||
       (out_of_u >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_config_u_thresh = RU_FIELD_SET(dma_id, DMA, CONFIG_U_THRESH, INTO_U, reg_config_u_thresh, into_u);
    reg_config_u_thresh = RU_FIELD_SET(dma_id, DMA, CONFIG_U_THRESH, OUT_OF_U, reg_config_u_thresh, out_of_u);

    RU_REG_RAM_WRITE(dma_id, emac_index, DMA, CONFIG_U_THRESH, reg_config_u_thresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_u_thresh_get(uint8_t dma_id, uint8_t emac_index, uint8_t *into_u, uint8_t *out_of_u)
{
    uint32_t reg_config_u_thresh;

#ifdef VALIDATE_PARMS
    if(!into_u || !out_of_u)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(dma_id, emac_index, DMA, CONFIG_U_THRESH, reg_config_u_thresh);

    *into_u = RU_FIELD_GET(dma_id, DMA, CONFIG_U_THRESH, INTO_U, reg_config_u_thresh);
    *out_of_u = RU_FIELD_GET(dma_id, DMA, CONFIG_U_THRESH, OUT_OF_U, reg_config_u_thresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_pri_set(uint8_t dma_id, uint8_t emac_index, uint8_t rxpri, uint8_t txpri)
{
    uint32_t reg_config_pri=0;

#ifdef VALIDATE_PARMS
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 8) ||
       (rxpri >= _4BITS_MAX_VAL_) ||
       (txpri >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_config_pri = RU_FIELD_SET(dma_id, DMA, CONFIG_PRI, RXPRI, reg_config_pri, rxpri);
    reg_config_pri = RU_FIELD_SET(dma_id, DMA, CONFIG_PRI, TXPRI, reg_config_pri, txpri);

    RU_REG_RAM_WRITE(dma_id, emac_index, DMA, CONFIG_PRI, reg_config_pri);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_pri_get(uint8_t dma_id, uint8_t emac_index, uint8_t *rxpri, uint8_t *txpri)
{
    uint32_t reg_config_pri;

#ifdef VALIDATE_PARMS
    if(!rxpri || !txpri)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(dma_id, emac_index, DMA, CONFIG_PRI, reg_config_pri);

    *rxpri = RU_FIELD_GET(dma_id, DMA, CONFIG_PRI, RXPRI, reg_config_pri);
    *txpri = RU_FIELD_GET(dma_id, DMA, CONFIG_PRI, TXPRI, reg_config_pri);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_periph_source_set(uint8_t dma_id, uint8_t emac_index, uint8_t rxsource, uint8_t txsource)
{
    uint32_t reg_config_periph_source=0;

#ifdef VALIDATE_PARMS
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 8) ||
       (rxsource >= _6BITS_MAX_VAL_) ||
       (txsource >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_config_periph_source = RU_FIELD_SET(dma_id, DMA, CONFIG_PERIPH_SOURCE, RXSOURCE, reg_config_periph_source, rxsource);
    reg_config_periph_source = RU_FIELD_SET(dma_id, DMA, CONFIG_PERIPH_SOURCE, TXSOURCE, reg_config_periph_source, txsource);

    RU_REG_RAM_WRITE(dma_id, emac_index, DMA, CONFIG_PERIPH_SOURCE, reg_config_periph_source);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_periph_source_get(uint8_t dma_id, uint8_t emac_index, uint8_t *rxsource, uint8_t *txsource)
{
    uint32_t reg_config_periph_source;

#ifdef VALIDATE_PARMS
    if(!rxsource || !txsource)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(dma_id, emac_index, DMA, CONFIG_PERIPH_SOURCE, reg_config_periph_source);

    *rxsource = RU_FIELD_GET(dma_id, DMA, CONFIG_PERIPH_SOURCE, RXSOURCE, reg_config_periph_source);
    *txsource = RU_FIELD_GET(dma_id, DMA, CONFIG_PERIPH_SOURCE, TXSOURCE, reg_config_periph_source);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_weight_set(uint8_t dma_id, uint8_t emac_index, uint8_t rxweight, uint8_t txweight)
{
    uint32_t reg_config_weight=0;

#ifdef VALIDATE_PARMS
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 8) ||
       (rxweight >= _3BITS_MAX_VAL_) ||
       (txweight >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_config_weight = RU_FIELD_SET(dma_id, DMA, CONFIG_WEIGHT, RXWEIGHT, reg_config_weight, rxweight);
    reg_config_weight = RU_FIELD_SET(dma_id, DMA, CONFIG_WEIGHT, TXWEIGHT, reg_config_weight, txweight);

    RU_REG_RAM_WRITE(dma_id, emac_index, DMA, CONFIG_WEIGHT, reg_config_weight);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_weight_get(uint8_t dma_id, uint8_t emac_index, uint8_t *rxweight, uint8_t *txweight)
{
    uint32_t reg_config_weight;

#ifdef VALIDATE_PARMS
    if(!rxweight || !txweight)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(dma_id, emac_index, DMA, CONFIG_WEIGHT, reg_config_weight);

    *rxweight = RU_FIELD_GET(dma_id, DMA, CONFIG_WEIGHT, RXWEIGHT, reg_config_weight);
    *txweight = RU_FIELD_GET(dma_id, DMA, CONFIG_WEIGHT, TXWEIGHT, reg_config_weight);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_ptrrst_set(uint8_t dma_id, uint16_t rstvec)
{
    uint32_t reg_config_ptrrst=0;

#ifdef VALIDATE_PARMS
    if((dma_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_config_ptrrst = RU_FIELD_SET(dma_id, DMA, CONFIG_PTRRST, RSTVEC, reg_config_ptrrst, rstvec);

    RU_REG_WRITE(dma_id, DMA, CONFIG_PTRRST, reg_config_ptrrst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_ptrrst_get(uint8_t dma_id, uint16_t *rstvec)
{
    uint32_t reg_config_ptrrst;

#ifdef VALIDATE_PARMS
    if(!rstvec)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dma_id, DMA, CONFIG_PTRRST, reg_config_ptrrst);

    *rstvec = RU_FIELD_GET(dma_id, DMA, CONFIG_PTRRST, RSTVEC, reg_config_ptrrst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_max_otf_set(uint8_t dma_id, uint8_t max)
{
    uint32_t reg_config_max_otf=0;

#ifdef VALIDATE_PARMS
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (max >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_config_max_otf = RU_FIELD_SET(dma_id, DMA, CONFIG_MAX_OTF, MAX, reg_config_max_otf, max);

    RU_REG_WRITE(dma_id, DMA, CONFIG_MAX_OTF, reg_config_max_otf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_max_otf_get(uint8_t dma_id, uint8_t *max)
{
    uint32_t reg_config_max_otf;

#ifdef VALIDATE_PARMS
    if(!max)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dma_id, DMA, CONFIG_MAX_OTF, reg_config_max_otf);

    *max = RU_FIELD_GET(dma_id, DMA, CONFIG_MAX_OTF, MAX, reg_config_max_otf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_clk_gate_cntrl_set(uint8_t dma_id, const dma_config_clk_gate_cntrl *config_clk_gate_cntrl)
{
    uint32_t reg_config_clk_gate_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!config_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (config_clk_gate_cntrl->bypass_clk_gate >= _1BITS_MAX_VAL_) ||
       (config_clk_gate_cntrl->keep_alive_en >= _1BITS_MAX_VAL_) ||
       (config_clk_gate_cntrl->keep_alive_intrvl >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_config_clk_gate_cntrl = RU_FIELD_SET(dma_id, DMA, CONFIG_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_config_clk_gate_cntrl, config_clk_gate_cntrl->bypass_clk_gate);
    reg_config_clk_gate_cntrl = RU_FIELD_SET(dma_id, DMA, CONFIG_CLK_GATE_CNTRL, TIMER_VAL, reg_config_clk_gate_cntrl, config_clk_gate_cntrl->timer_val);
    reg_config_clk_gate_cntrl = RU_FIELD_SET(dma_id, DMA, CONFIG_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_config_clk_gate_cntrl, config_clk_gate_cntrl->keep_alive_en);
    reg_config_clk_gate_cntrl = RU_FIELD_SET(dma_id, DMA, CONFIG_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_config_clk_gate_cntrl, config_clk_gate_cntrl->keep_alive_intrvl);
    reg_config_clk_gate_cntrl = RU_FIELD_SET(dma_id, DMA, CONFIG_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_config_clk_gate_cntrl, config_clk_gate_cntrl->keep_alive_cyc);

    RU_REG_WRITE(dma_id, DMA, CONFIG_CLK_GATE_CNTRL, reg_config_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_clk_gate_cntrl_get(uint8_t dma_id, dma_config_clk_gate_cntrl *config_clk_gate_cntrl)
{
    uint32_t reg_config_clk_gate_cntrl;

#ifdef VALIDATE_PARMS
    if(!config_clk_gate_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dma_id, DMA, CONFIG_CLK_GATE_CNTRL, reg_config_clk_gate_cntrl);

    config_clk_gate_cntrl->bypass_clk_gate = RU_FIELD_GET(dma_id, DMA, CONFIG_CLK_GATE_CNTRL, BYPASS_CLK_GATE, reg_config_clk_gate_cntrl);
    config_clk_gate_cntrl->timer_val = RU_FIELD_GET(dma_id, DMA, CONFIG_CLK_GATE_CNTRL, TIMER_VAL, reg_config_clk_gate_cntrl);
    config_clk_gate_cntrl->keep_alive_en = RU_FIELD_GET(dma_id, DMA, CONFIG_CLK_GATE_CNTRL, KEEP_ALIVE_EN, reg_config_clk_gate_cntrl);
    config_clk_gate_cntrl->keep_alive_intrvl = RU_FIELD_GET(dma_id, DMA, CONFIG_CLK_GATE_CNTRL, KEEP_ALIVE_INTRVL, reg_config_clk_gate_cntrl);
    config_clk_gate_cntrl->keep_alive_cyc = RU_FIELD_GET(dma_id, DMA, CONFIG_CLK_GATE_CNTRL, KEEP_ALIVE_CYC, reg_config_clk_gate_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_dbg_sel_set(uint8_t dma_id, uint8_t dbgsel)
{
    uint32_t reg_config_dbg_sel=0;

#ifdef VALIDATE_PARMS
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (dbgsel >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_config_dbg_sel = RU_FIELD_SET(dma_id, DMA, CONFIG_DBG_SEL, DBGSEL, reg_config_dbg_sel, dbgsel);

    RU_REG_WRITE(dma_id, DMA, CONFIG_DBG_SEL, reg_config_dbg_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_dbg_sel_get(uint8_t dma_id, uint8_t *dbgsel)
{
    uint32_t reg_config_dbg_sel;

#ifdef VALIDATE_PARMS
    if(!dbgsel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(dma_id, DMA, CONFIG_DBG_SEL, reg_config_dbg_sel);

    *dbgsel = RU_FIELD_GET(dma_id, DMA, CONFIG_DBG_SEL, DBGSEL, reg_config_dbg_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_debug_req_cnt_rx_get(uint8_t dma_id, uint8_t emac_index, uint8_t *req_cnt)
{
    uint32_t reg_debug_req_cnt_rx;

#ifdef VALIDATE_PARMS
    if(!req_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(dma_id, emac_index, DMA, DEBUG_REQ_CNT_RX, reg_debug_req_cnt_rx);

    *req_cnt = RU_FIELD_GET(dma_id, DMA, DEBUG_REQ_CNT_RX, REQ_CNT, reg_debug_req_cnt_rx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_debug_req_cnt_tx_get(uint8_t dma_id, uint8_t emac_index, uint8_t *req_cnt)
{
    uint32_t reg_debug_req_cnt_tx;

#ifdef VALIDATE_PARMS
    if(!req_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(dma_id, emac_index, DMA, DEBUG_REQ_CNT_TX, reg_debug_req_cnt_tx);

    *req_cnt = RU_FIELD_GET(dma_id, DMA, DEBUG_REQ_CNT_TX, REQ_CNT, reg_debug_req_cnt_tx);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_debug_req_cnt_rx_acc_get(uint8_t dma_id, uint8_t emac_index, uint32_t *req_cnt)
{
    uint32_t reg_debug_req_cnt_rx_acc;

#ifdef VALIDATE_PARMS
    if(!req_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(dma_id, emac_index, DMA, DEBUG_REQ_CNT_RX_ACC, reg_debug_req_cnt_rx_acc);

    *req_cnt = RU_FIELD_GET(dma_id, DMA, DEBUG_REQ_CNT_RX_ACC, REQ_CNT, reg_debug_req_cnt_rx_acc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_debug_req_cnt_tx_acc_get(uint8_t dma_id, uint8_t emac_index, uint32_t *req_cnt)
{
    uint32_t reg_debug_req_cnt_tx_acc;

#ifdef VALIDATE_PARMS
    if(!req_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(dma_id, emac_index, DMA, DEBUG_REQ_CNT_TX_ACC, reg_debug_req_cnt_tx_acc);

    *req_cnt = RU_FIELD_GET(dma_id, DMA, DEBUG_REQ_CNT_TX_ACC, REQ_CNT, reg_debug_req_cnt_tx_acc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_debug_rddata_get(uint8_t dma_id, uint8_t word_index, uint32_t *data)
{
    uint32_t reg_debug_rddata;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (word_index >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(dma_id, word_index, DMA, DEBUG_RDDATA, reg_debug_rddata);

    *data = RU_FIELD_GET(dma_id, DMA, DEBUG_RDDATA, DATA, reg_debug_rddata);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_config_bbrouteovrd,
    bdmf_address_config_num_of_writes,
    bdmf_address_config_num_of_reads,
    bdmf_address_config_u_thresh,
    bdmf_address_config_pri,
    bdmf_address_config_periph_source,
    bdmf_address_config_weight,
    bdmf_address_config_ptrrst,
    bdmf_address_config_max_otf,
    bdmf_address_config_clk_gate_cntrl,
    bdmf_address_config_dbg_sel,
    bdmf_address_debug_nempty,
    bdmf_address_debug_urgnt,
    bdmf_address_debug_selsrc,
    bdmf_address_debug_req_cnt_rx,
    bdmf_address_debug_req_cnt_tx,
    bdmf_address_debug_req_cnt_rx_acc,
    bdmf_address_debug_req_cnt_tx_acc,
    bdmf_address_debug_rdadd,
    bdmf_address_debug_rdvalid,
    bdmf_address_debug_rddata,
    bdmf_address_debug_rddatardy,
}
bdmf_address;

static int bcm_dma_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_dma_debug_info:
    {
        dma_debug_info debug_info = { .nempty=parm[2].value.unumber, .urgnt=parm[3].value.unumber, .sel_src=parm[4].value.unumber, .address=parm[5].value.unumber, .datacs=parm[6].value.unumber, .cdcs=parm[7].value.unumber, .rrcs=parm[8].value.unumber, .valid=parm[9].value.unumber, .ready=parm[10].value.unumber};
        err = ag_drv_dma_debug_info_set(parm[1].value.unumber, &debug_info);
        break;
    }
    case cli_dma_config_bbrouteovrd:
        err = ag_drv_dma_config_bbrouteovrd_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dma_config_num_of_writes:
        err = ag_drv_dma_config_num_of_writes_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dma_config_num_of_reads:
        err = ag_drv_dma_config_num_of_reads_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_dma_config_u_thresh:
        err = ag_drv_dma_config_u_thresh_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dma_config_pri:
        err = ag_drv_dma_config_pri_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dma_config_periph_source:
        err = ag_drv_dma_config_periph_source_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dma_config_weight:
        err = ag_drv_dma_config_weight_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dma_config_ptrrst:
        err = ag_drv_dma_config_ptrrst_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dma_config_max_otf:
        err = ag_drv_dma_config_max_otf_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dma_config_clk_gate_cntrl:
    {
        dma_config_clk_gate_cntrl config_clk_gate_cntrl = { .bypass_clk_gate=parm[2].value.unumber, .timer_val=parm[3].value.unumber, .keep_alive_en=parm[4].value.unumber, .keep_alive_intrvl=parm[5].value.unumber, .keep_alive_cyc=parm[6].value.unumber};
        err = ag_drv_dma_config_clk_gate_cntrl_set(parm[1].value.unumber, &config_clk_gate_cntrl);
        break;
    }
    case cli_dma_config_dbg_sel:
        err = ag_drv_dma_config_dbg_sel_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_dma_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_dma_debug_info:
    {
        dma_debug_info debug_info;
        err = ag_drv_dma_debug_info_get(parm[1].value.unumber, &debug_info);
        bdmf_session_print(session, "nempty = %u (0x%x)\n", debug_info.nempty, debug_info.nempty);
        bdmf_session_print(session, "urgnt = %u (0x%x)\n", debug_info.urgnt, debug_info.urgnt);
        bdmf_session_print(session, "sel_src = %u (0x%x)\n", debug_info.sel_src, debug_info.sel_src);
        bdmf_session_print(session, "address = %u (0x%x)\n", debug_info.address, debug_info.address);
        bdmf_session_print(session, "datacs = %u (0x%x)\n", debug_info.datacs, debug_info.datacs);
        bdmf_session_print(session, "cdcs = %u (0x%x)\n", debug_info.cdcs, debug_info.cdcs);
        bdmf_session_print(session, "rrcs = %u (0x%x)\n", debug_info.rrcs, debug_info.rrcs);
        bdmf_session_print(session, "valid = %u (0x%x)\n", debug_info.valid, debug_info.valid);
        bdmf_session_print(session, "ready = %u (0x%x)\n", debug_info.ready, debug_info.ready);
        break;
    }
    case cli_dma_config_bbrouteovrd:
    {
        uint8_t dest;
        uint16_t route;
        bdmf_boolean ovrd;
        err = ag_drv_dma_config_bbrouteovrd_get(parm[1].value.unumber, &dest, &route, &ovrd);
        bdmf_session_print(session, "dest = %u (0x%x)\n", dest, dest);
        bdmf_session_print(session, "route = %u (0x%x)\n", route, route);
        bdmf_session_print(session, "ovrd = %u (0x%x)\n", ovrd, ovrd);
        break;
    }
    case cli_dma_config_num_of_writes:
    {
        uint8_t numofbuff;
        err = ag_drv_dma_config_num_of_writes_get(parm[1].value.unumber, parm[2].value.unumber, &numofbuff);
        bdmf_session_print(session, "numofbuff = %u (0x%x)\n", numofbuff, numofbuff);
        break;
    }
    case cli_dma_config_num_of_reads:
    {
        uint8_t rr_num;
        err = ag_drv_dma_config_num_of_reads_get(parm[1].value.unumber, parm[2].value.unumber, &rr_num);
        bdmf_session_print(session, "rr_num = %u (0x%x)\n", rr_num, rr_num);
        break;
    }
    case cli_dma_config_u_thresh:
    {
        uint8_t into_u;
        uint8_t out_of_u;
        err = ag_drv_dma_config_u_thresh_get(parm[1].value.unumber, parm[2].value.unumber, &into_u, &out_of_u);
        bdmf_session_print(session, "into_u = %u (0x%x)\n", into_u, into_u);
        bdmf_session_print(session, "out_of_u = %u (0x%x)\n", out_of_u, out_of_u);
        break;
    }
    case cli_dma_config_pri:
    {
        uint8_t rxpri;
        uint8_t txpri;
        err = ag_drv_dma_config_pri_get(parm[1].value.unumber, parm[2].value.unumber, &rxpri, &txpri);
        bdmf_session_print(session, "rxpri = %u (0x%x)\n", rxpri, rxpri);
        bdmf_session_print(session, "txpri = %u (0x%x)\n", txpri, txpri);
        break;
    }
    case cli_dma_config_periph_source:
    {
        uint8_t rxsource;
        uint8_t txsource;
        err = ag_drv_dma_config_periph_source_get(parm[1].value.unumber, parm[2].value.unumber, &rxsource, &txsource);
        bdmf_session_print(session, "rxsource = %u (0x%x)\n", rxsource, rxsource);
        bdmf_session_print(session, "txsource = %u (0x%x)\n", txsource, txsource);
        break;
    }
    case cli_dma_config_weight:
    {
        uint8_t rxweight;
        uint8_t txweight;
        err = ag_drv_dma_config_weight_get(parm[1].value.unumber, parm[2].value.unumber, &rxweight, &txweight);
        bdmf_session_print(session, "rxweight = %u (0x%x)\n", rxweight, rxweight);
        bdmf_session_print(session, "txweight = %u (0x%x)\n", txweight, txweight);
        break;
    }
    case cli_dma_config_ptrrst:
    {
        uint16_t rstvec;
        err = ag_drv_dma_config_ptrrst_get(parm[1].value.unumber, &rstvec);
        bdmf_session_print(session, "rstvec = %u (0x%x)\n", rstvec, rstvec);
        break;
    }
    case cli_dma_config_max_otf:
    {
        uint8_t max;
        err = ag_drv_dma_config_max_otf_get(parm[1].value.unumber, &max);
        bdmf_session_print(session, "max = %u (0x%x)\n", max, max);
        break;
    }
    case cli_dma_config_clk_gate_cntrl:
    {
        dma_config_clk_gate_cntrl config_clk_gate_cntrl;
        err = ag_drv_dma_config_clk_gate_cntrl_get(parm[1].value.unumber, &config_clk_gate_cntrl);
        bdmf_session_print(session, "bypass_clk_gate = %u (0x%x)\n", config_clk_gate_cntrl.bypass_clk_gate, config_clk_gate_cntrl.bypass_clk_gate);
        bdmf_session_print(session, "timer_val = %u (0x%x)\n", config_clk_gate_cntrl.timer_val, config_clk_gate_cntrl.timer_val);
        bdmf_session_print(session, "keep_alive_en = %u (0x%x)\n", config_clk_gate_cntrl.keep_alive_en, config_clk_gate_cntrl.keep_alive_en);
        bdmf_session_print(session, "keep_alive_intrvl = %u (0x%x)\n", config_clk_gate_cntrl.keep_alive_intrvl, config_clk_gate_cntrl.keep_alive_intrvl);
        bdmf_session_print(session, "keep_alive_cyc = %u (0x%x)\n", config_clk_gate_cntrl.keep_alive_cyc, config_clk_gate_cntrl.keep_alive_cyc);
        break;
    }
    case cli_dma_config_dbg_sel:
    {
        uint8_t dbgsel;
        err = ag_drv_dma_config_dbg_sel_get(parm[1].value.unumber, &dbgsel);
        bdmf_session_print(session, "dbgsel = %u (0x%x)\n", dbgsel, dbgsel);
        break;
    }
    case cli_dma_debug_req_cnt_rx:
    {
        uint8_t req_cnt;
        err = ag_drv_dma_debug_req_cnt_rx_get(parm[1].value.unumber, parm[2].value.unumber, &req_cnt);
        bdmf_session_print(session, "req_cnt = %u (0x%x)\n", req_cnt, req_cnt);
        break;
    }
    case cli_dma_debug_req_cnt_tx:
    {
        uint8_t req_cnt;
        err = ag_drv_dma_debug_req_cnt_tx_get(parm[1].value.unumber, parm[2].value.unumber, &req_cnt);
        bdmf_session_print(session, "req_cnt = %u (0x%x)\n", req_cnt, req_cnt);
        break;
    }
    case cli_dma_debug_req_cnt_rx_acc:
    {
        uint32_t req_cnt;
        err = ag_drv_dma_debug_req_cnt_rx_acc_get(parm[1].value.unumber, parm[2].value.unumber, &req_cnt);
        bdmf_session_print(session, "req_cnt = %u (0x%x)\n", req_cnt, req_cnt);
        break;
    }
    case cli_dma_debug_req_cnt_tx_acc:
    {
        uint32_t req_cnt;
        err = ag_drv_dma_debug_req_cnt_tx_acc_get(parm[1].value.unumber, parm[2].value.unumber, &req_cnt);
        bdmf_session_print(session, "req_cnt = %u (0x%x)\n", req_cnt, req_cnt);
        break;
    }
    case cli_dma_debug_rddata:
    {
        uint32_t data;
        err = ag_drv_dma_debug_rddata_get(parm[1].value.unumber, parm[2].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_dma_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t dma_id = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        dma_debug_info debug_info = {.nempty=gtmv(m, 16), .urgnt=gtmv(m, 16), .sel_src=gtmv(m, 6), .address=gtmv(m, 10), .datacs=gtmv(m, 1), .cdcs=gtmv(m, 1), .rrcs=gtmv(m, 1), .valid=gtmv(m, 1), .ready=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_dma_debug_info_set(%u %u %u %u %u %u %u %u %u %u)\n", dma_id, debug_info.nempty, debug_info.urgnt, debug_info.sel_src, debug_info.address, debug_info.datacs, debug_info.cdcs, debug_info.rrcs, debug_info.valid, debug_info.ready);
        if(!err) ag_drv_dma_debug_info_set(dma_id, &debug_info);
        if(!err) ag_drv_dma_debug_info_get( dma_id, &debug_info);
        if(!err) bdmf_session_print(session, "ag_drv_dma_debug_info_get(%u %u %u %u %u %u %u %u %u %u)\n", dma_id, debug_info.nempty, debug_info.urgnt, debug_info.sel_src, debug_info.address, debug_info.datacs, debug_info.cdcs, debug_info.rrcs, debug_info.valid, debug_info.ready);
        if(err || debug_info.nempty!=gtmv(m, 16) || debug_info.urgnt!=gtmv(m, 16) || debug_info.sel_src!=gtmv(m, 6) || debug_info.address!=gtmv(m, 10) || debug_info.datacs!=gtmv(m, 1) || debug_info.cdcs!=gtmv(m, 1) || debug_info.rrcs!=gtmv(m, 1) || debug_info.valid!=gtmv(m, 1) || debug_info.ready!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t dest=gtmv(m, 6);
        uint16_t route=gtmv(m, 10);
        bdmf_boolean ovrd=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_bbrouteovrd_set(%u %u %u %u)\n", dma_id, dest, route, ovrd);
        if(!err) ag_drv_dma_config_bbrouteovrd_set(dma_id, dest, route, ovrd);
        if(!err) ag_drv_dma_config_bbrouteovrd_get( dma_id, &dest, &route, &ovrd);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_bbrouteovrd_get(%u %u %u %u)\n", dma_id, dest, route, ovrd);
        if(err || dest!=gtmv(m, 6) || route!=gtmv(m, 10) || ovrd!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint8_t numofbuff=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_num_of_writes_set(%u %u %u)\n", dma_id, emac_index, numofbuff);
        if(!err) ag_drv_dma_config_num_of_writes_set(dma_id, emac_index, numofbuff);
        if(!err) ag_drv_dma_config_num_of_writes_get( dma_id, emac_index, &numofbuff);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_num_of_writes_get(%u %u %u)\n", dma_id, emac_index, numofbuff);
        if(err || numofbuff!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint8_t rr_num=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_num_of_reads_set(%u %u %u)\n", dma_id, emac_index, rr_num);
        if(!err) ag_drv_dma_config_num_of_reads_set(dma_id, emac_index, rr_num);
        if(!err) ag_drv_dma_config_num_of_reads_get( dma_id, emac_index, &rr_num);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_num_of_reads_get(%u %u %u)\n", dma_id, emac_index, rr_num);
        if(err || rr_num!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint8_t into_u=gtmv(m, 6);
        uint8_t out_of_u=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_u_thresh_set(%u %u %u %u)\n", dma_id, emac_index, into_u, out_of_u);
        if(!err) ag_drv_dma_config_u_thresh_set(dma_id, emac_index, into_u, out_of_u);
        if(!err) ag_drv_dma_config_u_thresh_get( dma_id, emac_index, &into_u, &out_of_u);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_u_thresh_get(%u %u %u %u)\n", dma_id, emac_index, into_u, out_of_u);
        if(err || into_u!=gtmv(m, 6) || out_of_u!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint8_t rxpri=gtmv(m, 4);
        uint8_t txpri=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_pri_set(%u %u %u %u)\n", dma_id, emac_index, rxpri, txpri);
        if(!err) ag_drv_dma_config_pri_set(dma_id, emac_index, rxpri, txpri);
        if(!err) ag_drv_dma_config_pri_get( dma_id, emac_index, &rxpri, &txpri);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_pri_get(%u %u %u %u)\n", dma_id, emac_index, rxpri, txpri);
        if(err || rxpri!=gtmv(m, 4) || txpri!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint8_t rxsource=gtmv(m, 6);
        uint8_t txsource=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_periph_source_set(%u %u %u %u)\n", dma_id, emac_index, rxsource, txsource);
        if(!err) ag_drv_dma_config_periph_source_set(dma_id, emac_index, rxsource, txsource);
        if(!err) ag_drv_dma_config_periph_source_get( dma_id, emac_index, &rxsource, &txsource);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_periph_source_get(%u %u %u %u)\n", dma_id, emac_index, rxsource, txsource);
        if(err || rxsource!=gtmv(m, 6) || txsource!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint8_t rxweight=gtmv(m, 3);
        uint8_t txweight=gtmv(m, 3);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_weight_set(%u %u %u %u)\n", dma_id, emac_index, rxweight, txweight);
        if(!err) ag_drv_dma_config_weight_set(dma_id, emac_index, rxweight, txweight);
        if(!err) ag_drv_dma_config_weight_get( dma_id, emac_index, &rxweight, &txweight);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_weight_get(%u %u %u %u)\n", dma_id, emac_index, rxweight, txweight);
        if(err || rxweight!=gtmv(m, 3) || txweight!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t rstvec=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_ptrrst_set(%u %u)\n", dma_id, rstvec);
        if(!err) ag_drv_dma_config_ptrrst_set(dma_id, rstvec);
        if(!err) ag_drv_dma_config_ptrrst_get( dma_id, &rstvec);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_ptrrst_get(%u %u)\n", dma_id, rstvec);
        if(err || rstvec!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t max=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_max_otf_set(%u %u)\n", dma_id, max);
        if(!err) ag_drv_dma_config_max_otf_set(dma_id, max);
        if(!err) ag_drv_dma_config_max_otf_get( dma_id, &max);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_max_otf_get(%u %u)\n", dma_id, max);
        if(err || max!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        dma_config_clk_gate_cntrl config_clk_gate_cntrl = {.bypass_clk_gate=gtmv(m, 1), .timer_val=gtmv(m, 8), .keep_alive_en=gtmv(m, 1), .keep_alive_intrvl=gtmv(m, 3), .keep_alive_cyc=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_clk_gate_cntrl_set(%u %u %u %u %u %u)\n", dma_id, config_clk_gate_cntrl.bypass_clk_gate, config_clk_gate_cntrl.timer_val, config_clk_gate_cntrl.keep_alive_en, config_clk_gate_cntrl.keep_alive_intrvl, config_clk_gate_cntrl.keep_alive_cyc);
        if(!err) ag_drv_dma_config_clk_gate_cntrl_set(dma_id, &config_clk_gate_cntrl);
        if(!err) ag_drv_dma_config_clk_gate_cntrl_get( dma_id, &config_clk_gate_cntrl);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_clk_gate_cntrl_get(%u %u %u %u %u %u)\n", dma_id, config_clk_gate_cntrl.bypass_clk_gate, config_clk_gate_cntrl.timer_val, config_clk_gate_cntrl.keep_alive_en, config_clk_gate_cntrl.keep_alive_intrvl, config_clk_gate_cntrl.keep_alive_cyc);
        if(err || config_clk_gate_cntrl.bypass_clk_gate!=gtmv(m, 1) || config_clk_gate_cntrl.timer_val!=gtmv(m, 8) || config_clk_gate_cntrl.keep_alive_en!=gtmv(m, 1) || config_clk_gate_cntrl.keep_alive_intrvl!=gtmv(m, 3) || config_clk_gate_cntrl.keep_alive_cyc!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t dbgsel=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_dbg_sel_set(%u %u)\n", dma_id, dbgsel);
        if(!err) ag_drv_dma_config_dbg_sel_set(dma_id, dbgsel);
        if(!err) ag_drv_dma_config_dbg_sel_get( dma_id, &dbgsel);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_dbg_sel_get(%u %u)\n", dma_id, dbgsel);
        if(err || dbgsel!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint8_t req_cnt=gtmv(m, 6);
        if(!err) ag_drv_dma_debug_req_cnt_rx_get( dma_id, emac_index, &req_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_dma_debug_req_cnt_rx_get(%u %u %u)\n", dma_id, emac_index, req_cnt);
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint8_t req_cnt=gtmv(m, 6);
        if(!err) ag_drv_dma_debug_req_cnt_tx_get( dma_id, emac_index, &req_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_dma_debug_req_cnt_tx_get(%u %u %u)\n", dma_id, emac_index, req_cnt);
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint32_t req_cnt=gtmv(m, 32);
        if(!err) ag_drv_dma_debug_req_cnt_rx_acc_get( dma_id, emac_index, &req_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_dma_debug_req_cnt_rx_acc_get(%u %u %u)\n", dma_id, emac_index, req_cnt);
    }
    {
        uint8_t emac_index=gtmv(m, 3);
        uint32_t req_cnt=gtmv(m, 32);
        if(!err) ag_drv_dma_debug_req_cnt_tx_acc_get( dma_id, emac_index, &req_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_dma_debug_req_cnt_tx_acc_get(%u %u %u)\n", dma_id, emac_index, req_cnt);
    }
    {
        uint8_t word_index=gtmv(m, 2);
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_dma_debug_rddata_get( dma_id, word_index, &data);
        if(!err) bdmf_session_print(session, "ag_drv_dma_debug_rddata_get(%u %u %u)\n", dma_id, word_index, data);
    }
    return err;
}

static int bcm_dma_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_config_bbrouteovrd : reg = &RU_REG(DMA, CONFIG_BBROUTEOVRD); blk = &RU_BLK(DMA); break;
    case bdmf_address_config_num_of_writes : reg = &RU_REG(DMA, CONFIG_NUM_OF_WRITES); blk = &RU_BLK(DMA); break;
    case bdmf_address_config_num_of_reads : reg = &RU_REG(DMA, CONFIG_NUM_OF_READS); blk = &RU_BLK(DMA); break;
    case bdmf_address_config_u_thresh : reg = &RU_REG(DMA, CONFIG_U_THRESH); blk = &RU_BLK(DMA); break;
    case bdmf_address_config_pri : reg = &RU_REG(DMA, CONFIG_PRI); blk = &RU_BLK(DMA); break;
    case bdmf_address_config_periph_source : reg = &RU_REG(DMA, CONFIG_PERIPH_SOURCE); blk = &RU_BLK(DMA); break;
    case bdmf_address_config_weight : reg = &RU_REG(DMA, CONFIG_WEIGHT); blk = &RU_BLK(DMA); break;
    case bdmf_address_config_ptrrst : reg = &RU_REG(DMA, CONFIG_PTRRST); blk = &RU_BLK(DMA); break;
    case bdmf_address_config_max_otf : reg = &RU_REG(DMA, CONFIG_MAX_OTF); blk = &RU_BLK(DMA); break;
    case bdmf_address_config_clk_gate_cntrl : reg = &RU_REG(DMA, CONFIG_CLK_GATE_CNTRL); blk = &RU_BLK(DMA); break;
    case bdmf_address_config_dbg_sel : reg = &RU_REG(DMA, CONFIG_DBG_SEL); blk = &RU_BLK(DMA); break;
    case bdmf_address_debug_nempty : reg = &RU_REG(DMA, DEBUG_NEMPTY); blk = &RU_BLK(DMA); break;
    case bdmf_address_debug_urgnt : reg = &RU_REG(DMA, DEBUG_URGNT); blk = &RU_BLK(DMA); break;
    case bdmf_address_debug_selsrc : reg = &RU_REG(DMA, DEBUG_SELSRC); blk = &RU_BLK(DMA); break;
    case bdmf_address_debug_req_cnt_rx : reg = &RU_REG(DMA, DEBUG_REQ_CNT_RX); blk = &RU_BLK(DMA); break;
    case bdmf_address_debug_req_cnt_tx : reg = &RU_REG(DMA, DEBUG_REQ_CNT_TX); blk = &RU_BLK(DMA); break;
    case bdmf_address_debug_req_cnt_rx_acc : reg = &RU_REG(DMA, DEBUG_REQ_CNT_RX_ACC); blk = &RU_BLK(DMA); break;
    case bdmf_address_debug_req_cnt_tx_acc : reg = &RU_REG(DMA, DEBUG_REQ_CNT_TX_ACC); blk = &RU_BLK(DMA); break;
    case bdmf_address_debug_rdadd : reg = &RU_REG(DMA, DEBUG_RDADD); blk = &RU_BLK(DMA); break;
    case bdmf_address_debug_rdvalid : reg = &RU_REG(DMA, DEBUG_RDVALID); blk = &RU_BLK(DMA); break;
    case bdmf_address_debug_rddata : reg = &RU_REG(DMA, DEBUG_RDDATA); blk = &RU_BLK(DMA); break;
    case bdmf_address_debug_rddatardy : reg = &RU_REG(DMA, DEBUG_RDDATARDY); blk = &RU_BLK(DMA); break;
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

bdmfmon_handle_t ag_drv_dma_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "dma"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "dma", "dma", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_debug_info[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("nempty", "nempty", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("urgnt", "urgnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sel_src", "sel_src", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("address", "address", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("datacs", "datacs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cdcs", "cdcs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rrcs", "rrcs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ready", "ready", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_bbrouteovrd[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("dest", "dest", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("route", "route", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ovrd", "ovrd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_num_of_writes[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("numofbuff", "numofbuff", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_num_of_reads[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rr_num", "rr_num", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_u_thresh[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("into_u", "into_u", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("out_of_u", "out_of_u", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_pri[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxpri", "rxpri", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txpri", "txpri", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_periph_source[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxsource", "rxsource", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txsource", "txsource", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_weight[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxweight", "rxweight", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txweight", "txweight", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_ptrrst[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("rstvec", "rstvec", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_max_otf[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("max", "max", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_clk_gate_cntrl[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("bypass_clk_gate", "bypass_clk_gate", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("timer_val", "timer_val", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_en", "keep_alive_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_intrvl", "keep_alive_intrvl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("keep_alive_cyc", "keep_alive_cyc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_dbg_sel[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("dbgsel", "dbgsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="debug_info", .val=cli_dma_debug_info, .parms=set_debug_info },
            { .name="config_bbrouteovrd", .val=cli_dma_config_bbrouteovrd, .parms=set_config_bbrouteovrd },
            { .name="config_num_of_writes", .val=cli_dma_config_num_of_writes, .parms=set_config_num_of_writes },
            { .name="config_num_of_reads", .val=cli_dma_config_num_of_reads, .parms=set_config_num_of_reads },
            { .name="config_u_thresh", .val=cli_dma_config_u_thresh, .parms=set_config_u_thresh },
            { .name="config_pri", .val=cli_dma_config_pri, .parms=set_config_pri },
            { .name="config_periph_source", .val=cli_dma_config_periph_source, .parms=set_config_periph_source },
            { .name="config_weight", .val=cli_dma_config_weight, .parms=set_config_weight },
            { .name="config_ptrrst", .val=cli_dma_config_ptrrst, .parms=set_config_ptrrst },
            { .name="config_max_otf", .val=cli_dma_config_max_otf, .parms=set_config_max_otf },
            { .name="config_clk_gate_cntrl", .val=cli_dma_config_clk_gate_cntrl, .parms=set_config_clk_gate_cntrl },
            { .name="config_dbg_sel", .val=cli_dma_config_dbg_sel, .parms=set_config_dbg_sel },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_dma_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_num_of_writes[]={
            BDMFMON_MAKE_PARM("dma_id", "dma_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_num_of_reads[]={
            BDMFMON_MAKE_PARM("dma_id", "dma_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_u_thresh[]={
            BDMFMON_MAKE_PARM("dma_id", "dma_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_pri[]={
            BDMFMON_MAKE_PARM("dma_id", "dma_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_periph_source[]={
            BDMFMON_MAKE_PARM("dma_id", "dma_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_weight[]={
            BDMFMON_MAKE_PARM("dma_id", "dma_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_req_cnt_rx[]={
            BDMFMON_MAKE_PARM("dma_id", "dma_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_req_cnt_tx[]={
            BDMFMON_MAKE_PARM("dma_id", "dma_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_req_cnt_rx_acc[]={
            BDMFMON_MAKE_PARM("dma_id", "dma_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_req_cnt_tx_acc[]={
            BDMFMON_MAKE_PARM("dma_id", "dma_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_rddata[]={
            BDMFMON_MAKE_PARM("dma_id", "dma_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("word_index", "word_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="debug_info", .val=cli_dma_debug_info, .parms=set_default },
            { .name="config_bbrouteovrd", .val=cli_dma_config_bbrouteovrd, .parms=set_default },
            { .name="config_num_of_writes", .val=cli_dma_config_num_of_writes, .parms=set_config_num_of_writes },
            { .name="config_num_of_reads", .val=cli_dma_config_num_of_reads, .parms=set_config_num_of_reads },
            { .name="config_u_thresh", .val=cli_dma_config_u_thresh, .parms=set_config_u_thresh },
            { .name="config_pri", .val=cli_dma_config_pri, .parms=set_config_pri },
            { .name="config_periph_source", .val=cli_dma_config_periph_source, .parms=set_config_periph_source },
            { .name="config_weight", .val=cli_dma_config_weight, .parms=set_config_weight },
            { .name="config_ptrrst", .val=cli_dma_config_ptrrst, .parms=set_default },
            { .name="config_max_otf", .val=cli_dma_config_max_otf, .parms=set_default },
            { .name="config_clk_gate_cntrl", .val=cli_dma_config_clk_gate_cntrl, .parms=set_default },
            { .name="config_dbg_sel", .val=cli_dma_config_dbg_sel, .parms=set_default },
            { .name="debug_req_cnt_rx", .val=cli_dma_debug_req_cnt_rx, .parms=set_debug_req_cnt_rx },
            { .name="debug_req_cnt_tx", .val=cli_dma_debug_req_cnt_tx, .parms=set_debug_req_cnt_tx },
            { .name="debug_req_cnt_rx_acc", .val=cli_dma_debug_req_cnt_rx_acc, .parms=set_debug_req_cnt_rx_acc },
            { .name="debug_req_cnt_tx_acc", .val=cli_dma_debug_req_cnt_tx_acc, .parms=set_debug_req_cnt_tx_acc },
            { .name="debug_rddata", .val=cli_dma_debug_rddata, .parms=set_debug_rddata },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_dma_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_dma_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="CONFIG_BBROUTEOVRD" , .val=bdmf_address_config_bbrouteovrd },
            { .name="CONFIG_NUM_OF_WRITES" , .val=bdmf_address_config_num_of_writes },
            { .name="CONFIG_NUM_OF_READS" , .val=bdmf_address_config_num_of_reads },
            { .name="CONFIG_U_THRESH" , .val=bdmf_address_config_u_thresh },
            { .name="CONFIG_PRI" , .val=bdmf_address_config_pri },
            { .name="CONFIG_PERIPH_SOURCE" , .val=bdmf_address_config_periph_source },
            { .name="CONFIG_WEIGHT" , .val=bdmf_address_config_weight },
            { .name="CONFIG_PTRRST" , .val=bdmf_address_config_ptrrst },
            { .name="CONFIG_MAX_OTF" , .val=bdmf_address_config_max_otf },
            { .name="CONFIG_CLK_GATE_CNTRL" , .val=bdmf_address_config_clk_gate_cntrl },
            { .name="CONFIG_DBG_SEL" , .val=bdmf_address_config_dbg_sel },
            { .name="DEBUG_NEMPTY" , .val=bdmf_address_debug_nempty },
            { .name="DEBUG_URGNT" , .val=bdmf_address_debug_urgnt },
            { .name="DEBUG_SELSRC" , .val=bdmf_address_debug_selsrc },
            { .name="DEBUG_REQ_CNT_RX" , .val=bdmf_address_debug_req_cnt_rx },
            { .name="DEBUG_REQ_CNT_TX" , .val=bdmf_address_debug_req_cnt_tx },
            { .name="DEBUG_REQ_CNT_RX_ACC" , .val=bdmf_address_debug_req_cnt_rx_acc },
            { .name="DEBUG_REQ_CNT_TX_ACC" , .val=bdmf_address_debug_req_cnt_tx_acc },
            { .name="DEBUG_RDADD" , .val=bdmf_address_debug_rdadd },
            { .name="DEBUG_RDVALID" , .val=bdmf_address_debug_rdvalid },
            { .name="DEBUG_RDDATA" , .val=bdmf_address_debug_rddata },
            { .name="DEBUG_RDDATARDY" , .val=bdmf_address_debug_rddatardy },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_dma_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

