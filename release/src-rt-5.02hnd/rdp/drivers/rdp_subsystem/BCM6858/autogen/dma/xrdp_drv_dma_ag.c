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

#define BLOCK_ADDR_COUNT_BITS 2
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
       (debug_info->eth0rxne >= _1BITS_MAX_VAL_) ||
       (debug_info->eth1rxne >= _1BITS_MAX_VAL_) ||
       (debug_info->eth2rxne >= _1BITS_MAX_VAL_) ||
       (debug_info->eth3rxne >= _1BITS_MAX_VAL_) ||
       (debug_info->eth4rxne >= _1BITS_MAX_VAL_) ||
       (debug_info->gponrxne >= _1BITS_MAX_VAL_) ||
       (debug_info->eth0txne >= _1BITS_MAX_VAL_) ||
       (debug_info->eth1txne >= _1BITS_MAX_VAL_) ||
       (debug_info->eth2txne >= _1BITS_MAX_VAL_) ||
       (debug_info->eth3txne >= _1BITS_MAX_VAL_) ||
       (debug_info->eth4txne >= _1BITS_MAX_VAL_) ||
       (debug_info->gpontxne >= _1BITS_MAX_VAL_) ||
       (debug_info->eth0rxu >= _1BITS_MAX_VAL_) ||
       (debug_info->eth1rxu >= _1BITS_MAX_VAL_) ||
       (debug_info->eth2rxu >= _1BITS_MAX_VAL_) ||
       (debug_info->eth3rxu >= _1BITS_MAX_VAL_) ||
       (debug_info->eth4rxu >= _1BITS_MAX_VAL_) ||
       (debug_info->gponrxu >= _1BITS_MAX_VAL_) ||
       (debug_info->eth0txu >= _1BITS_MAX_VAL_) ||
       (debug_info->eth1txu >= _1BITS_MAX_VAL_) ||
       (debug_info->eth2txu >= _1BITS_MAX_VAL_) ||
       (debug_info->eth3txu >= _1BITS_MAX_VAL_) ||
       (debug_info->eth4txu >= _1BITS_MAX_VAL_) ||
       (debug_info->gpontxu >= _1BITS_MAX_VAL_) ||
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

    reg_debug_nempty = RU_FIELD_SET(dma_id, DMA, DEBUG_NEMPTY, ETH0RXNE, reg_debug_nempty, debug_info->eth0rxne);
    reg_debug_nempty = RU_FIELD_SET(dma_id, DMA, DEBUG_NEMPTY, ETH1RXNE, reg_debug_nempty, debug_info->eth1rxne);
    reg_debug_nempty = RU_FIELD_SET(dma_id, DMA, DEBUG_NEMPTY, ETH2RXNE, reg_debug_nempty, debug_info->eth2rxne);
    reg_debug_nempty = RU_FIELD_SET(dma_id, DMA, DEBUG_NEMPTY, ETH3RXNE, reg_debug_nempty, debug_info->eth3rxne);
    reg_debug_nempty = RU_FIELD_SET(dma_id, DMA, DEBUG_NEMPTY, ETH4RXNE, reg_debug_nempty, debug_info->eth4rxne);
    reg_debug_nempty = RU_FIELD_SET(dma_id, DMA, DEBUG_NEMPTY, GPONRXNE, reg_debug_nempty, debug_info->gponrxne);
    reg_debug_nempty = RU_FIELD_SET(dma_id, DMA, DEBUG_NEMPTY, ETH0TXNE, reg_debug_nempty, debug_info->eth0txne);
    reg_debug_nempty = RU_FIELD_SET(dma_id, DMA, DEBUG_NEMPTY, ETH1TXNE, reg_debug_nempty, debug_info->eth1txne);
    reg_debug_nempty = RU_FIELD_SET(dma_id, DMA, DEBUG_NEMPTY, ETH2TXNE, reg_debug_nempty, debug_info->eth2txne);
    reg_debug_nempty = RU_FIELD_SET(dma_id, DMA, DEBUG_NEMPTY, ETH3TXNE, reg_debug_nempty, debug_info->eth3txne);
    reg_debug_nempty = RU_FIELD_SET(dma_id, DMA, DEBUG_NEMPTY, ETH4TXNE, reg_debug_nempty, debug_info->eth4txne);
    reg_debug_nempty = RU_FIELD_SET(dma_id, DMA, DEBUG_NEMPTY, GPONTXNE, reg_debug_nempty, debug_info->gpontxne);
    reg_debug_urgnt = RU_FIELD_SET(dma_id, DMA, DEBUG_URGNT, ETH0RXU, reg_debug_urgnt, debug_info->eth0rxu);
    reg_debug_urgnt = RU_FIELD_SET(dma_id, DMA, DEBUG_URGNT, ETH1RXU, reg_debug_urgnt, debug_info->eth1rxu);
    reg_debug_urgnt = RU_FIELD_SET(dma_id, DMA, DEBUG_URGNT, ETH2RXU, reg_debug_urgnt, debug_info->eth2rxu);
    reg_debug_urgnt = RU_FIELD_SET(dma_id, DMA, DEBUG_URGNT, ETH3RXU, reg_debug_urgnt, debug_info->eth3rxu);
    reg_debug_urgnt = RU_FIELD_SET(dma_id, DMA, DEBUG_URGNT, ETH4RXU, reg_debug_urgnt, debug_info->eth4rxu);
    reg_debug_urgnt = RU_FIELD_SET(dma_id, DMA, DEBUG_URGNT, GPONRXU, reg_debug_urgnt, debug_info->gponrxu);
    reg_debug_urgnt = RU_FIELD_SET(dma_id, DMA, DEBUG_URGNT, ETH0TXU, reg_debug_urgnt, debug_info->eth0txu);
    reg_debug_urgnt = RU_FIELD_SET(dma_id, DMA, DEBUG_URGNT, ETH1TXU, reg_debug_urgnt, debug_info->eth1txu);
    reg_debug_urgnt = RU_FIELD_SET(dma_id, DMA, DEBUG_URGNT, ETH2TXU, reg_debug_urgnt, debug_info->eth2txu);
    reg_debug_urgnt = RU_FIELD_SET(dma_id, DMA, DEBUG_URGNT, ETH3TXU, reg_debug_urgnt, debug_info->eth3txu);
    reg_debug_urgnt = RU_FIELD_SET(dma_id, DMA, DEBUG_URGNT, ETH4TXU, reg_debug_urgnt, debug_info->eth4txu);
    reg_debug_urgnt = RU_FIELD_SET(dma_id, DMA, DEBUG_URGNT, GPONTXU, reg_debug_urgnt, debug_info->gpontxu);
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

    debug_info->eth0rxne = RU_FIELD_GET(dma_id, DMA, DEBUG_NEMPTY, ETH0RXNE, reg_debug_nempty);
    debug_info->eth1rxne = RU_FIELD_GET(dma_id, DMA, DEBUG_NEMPTY, ETH1RXNE, reg_debug_nempty);
    debug_info->eth2rxne = RU_FIELD_GET(dma_id, DMA, DEBUG_NEMPTY, ETH2RXNE, reg_debug_nempty);
    debug_info->eth3rxne = RU_FIELD_GET(dma_id, DMA, DEBUG_NEMPTY, ETH3RXNE, reg_debug_nempty);
    debug_info->eth4rxne = RU_FIELD_GET(dma_id, DMA, DEBUG_NEMPTY, ETH4RXNE, reg_debug_nempty);
    debug_info->gponrxne = RU_FIELD_GET(dma_id, DMA, DEBUG_NEMPTY, GPONRXNE, reg_debug_nempty);
    debug_info->eth0txne = RU_FIELD_GET(dma_id, DMA, DEBUG_NEMPTY, ETH0TXNE, reg_debug_nempty);
    debug_info->eth1txne = RU_FIELD_GET(dma_id, DMA, DEBUG_NEMPTY, ETH1TXNE, reg_debug_nempty);
    debug_info->eth2txne = RU_FIELD_GET(dma_id, DMA, DEBUG_NEMPTY, ETH2TXNE, reg_debug_nempty);
    debug_info->eth3txne = RU_FIELD_GET(dma_id, DMA, DEBUG_NEMPTY, ETH3TXNE, reg_debug_nempty);
    debug_info->eth4txne = RU_FIELD_GET(dma_id, DMA, DEBUG_NEMPTY, ETH4TXNE, reg_debug_nempty);
    debug_info->gpontxne = RU_FIELD_GET(dma_id, DMA, DEBUG_NEMPTY, GPONTXNE, reg_debug_nempty);
    debug_info->eth0rxu = RU_FIELD_GET(dma_id, DMA, DEBUG_URGNT, ETH0RXU, reg_debug_urgnt);
    debug_info->eth1rxu = RU_FIELD_GET(dma_id, DMA, DEBUG_URGNT, ETH1RXU, reg_debug_urgnt);
    debug_info->eth2rxu = RU_FIELD_GET(dma_id, DMA, DEBUG_URGNT, ETH2RXU, reg_debug_urgnt);
    debug_info->eth3rxu = RU_FIELD_GET(dma_id, DMA, DEBUG_URGNT, ETH3RXU, reg_debug_urgnt);
    debug_info->eth4rxu = RU_FIELD_GET(dma_id, DMA, DEBUG_URGNT, ETH4RXU, reg_debug_urgnt);
    debug_info->gponrxu = RU_FIELD_GET(dma_id, DMA, DEBUG_URGNT, GPONRXU, reg_debug_urgnt);
    debug_info->eth0txu = RU_FIELD_GET(dma_id, DMA, DEBUG_URGNT, ETH0TXU, reg_debug_urgnt);
    debug_info->eth1txu = RU_FIELD_GET(dma_id, DMA, DEBUG_URGNT, ETH1TXU, reg_debug_urgnt);
    debug_info->eth2txu = RU_FIELD_GET(dma_id, DMA, DEBUG_URGNT, ETH2TXU, reg_debug_urgnt);
    debug_info->eth3txu = RU_FIELD_GET(dma_id, DMA, DEBUG_URGNT, ETH3TXU, reg_debug_urgnt);
    debug_info->eth4txu = RU_FIELD_GET(dma_id, DMA, DEBUG_URGNT, ETH4TXU, reg_debug_urgnt);
    debug_info->gpontxu = RU_FIELD_GET(dma_id, DMA, DEBUG_URGNT, GPONTXU, reg_debug_urgnt);
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
       (emac_index >= 6) ||
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
       (emac_index >= 6))
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
       (emac_index >= 6) ||
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
       (emac_index >= 6))
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
       (emac_index >= 6) ||
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
       (emac_index >= 6))
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
       (emac_index >= 6))
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
       (emac_index >= 6))
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

bdmf_error_t ag_drv_dma_config_weight_set(uint8_t dma_id, uint8_t emac_index, uint8_t rxweight, uint8_t txweight)
{
    uint32_t reg_config_weight=0;

#ifdef VALIDATE_PARMS
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 6) ||
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
       (emac_index >= 6))
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

bdmf_error_t ag_drv_dma_config_periph_source_set(uint8_t dma_id, uint8_t emac_index, uint8_t rxsource, uint8_t txsource)
{
    uint32_t reg_config_periph_source=0;

#ifdef VALIDATE_PARMS
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (emac_index >= 6) ||
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
       (emac_index >= 6))
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

bdmf_error_t ag_drv_dma_config_ptrrst_set(uint8_t dma_id, const dma_config_ptrrst *config_ptrrst)
{
    uint32_t reg_config_ptrrst=0;

#ifdef VALIDATE_PARMS
    if(!config_ptrrst)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((dma_id >= BLOCK_ADDR_COUNT) ||
       (config_ptrrst->eth0rx >= _1BITS_MAX_VAL_) ||
       (config_ptrrst->eth0tx >= _1BITS_MAX_VAL_) ||
       (config_ptrrst->eth1rx >= _1BITS_MAX_VAL_) ||
       (config_ptrrst->eth1tx >= _1BITS_MAX_VAL_) ||
       (config_ptrrst->eth2rx >= _1BITS_MAX_VAL_) ||
       (config_ptrrst->eth2tx >= _1BITS_MAX_VAL_) ||
       (config_ptrrst->eth3rx >= _1BITS_MAX_VAL_) ||
       (config_ptrrst->eth3tx >= _1BITS_MAX_VAL_) ||
       (config_ptrrst->eth4rx >= _1BITS_MAX_VAL_) ||
       (config_ptrrst->eth4tx >= _1BITS_MAX_VAL_) ||
       (config_ptrrst->gponrx >= _1BITS_MAX_VAL_) ||
       (config_ptrrst->gpontx >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_config_ptrrst = RU_FIELD_SET(dma_id, DMA, CONFIG_PTRRST, ETH0RX, reg_config_ptrrst, config_ptrrst->eth0rx);
    reg_config_ptrrst = RU_FIELD_SET(dma_id, DMA, CONFIG_PTRRST, ETH0TX, reg_config_ptrrst, config_ptrrst->eth0tx);
    reg_config_ptrrst = RU_FIELD_SET(dma_id, DMA, CONFIG_PTRRST, ETH1RX, reg_config_ptrrst, config_ptrrst->eth1rx);
    reg_config_ptrrst = RU_FIELD_SET(dma_id, DMA, CONFIG_PTRRST, ETH1TX, reg_config_ptrrst, config_ptrrst->eth1tx);
    reg_config_ptrrst = RU_FIELD_SET(dma_id, DMA, CONFIG_PTRRST, ETH2RX, reg_config_ptrrst, config_ptrrst->eth2rx);
    reg_config_ptrrst = RU_FIELD_SET(dma_id, DMA, CONFIG_PTRRST, ETH2TX, reg_config_ptrrst, config_ptrrst->eth2tx);
    reg_config_ptrrst = RU_FIELD_SET(dma_id, DMA, CONFIG_PTRRST, ETH3RX, reg_config_ptrrst, config_ptrrst->eth3rx);
    reg_config_ptrrst = RU_FIELD_SET(dma_id, DMA, CONFIG_PTRRST, ETH3TX, reg_config_ptrrst, config_ptrrst->eth3tx);
    reg_config_ptrrst = RU_FIELD_SET(dma_id, DMA, CONFIG_PTRRST, ETH4RX, reg_config_ptrrst, config_ptrrst->eth4rx);
    reg_config_ptrrst = RU_FIELD_SET(dma_id, DMA, CONFIG_PTRRST, ETH4TX, reg_config_ptrrst, config_ptrrst->eth4tx);
    reg_config_ptrrst = RU_FIELD_SET(dma_id, DMA, CONFIG_PTRRST, GPONRX, reg_config_ptrrst, config_ptrrst->gponrx);
    reg_config_ptrrst = RU_FIELD_SET(dma_id, DMA, CONFIG_PTRRST, GPONTX, reg_config_ptrrst, config_ptrrst->gpontx);

    RU_REG_WRITE(dma_id, DMA, CONFIG_PTRRST, reg_config_ptrrst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dma_config_ptrrst_get(uint8_t dma_id, dma_config_ptrrst *config_ptrrst)
{
    uint32_t reg_config_ptrrst;

#ifdef VALIDATE_PARMS
    if(!config_ptrrst)
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

    config_ptrrst->eth0rx = RU_FIELD_GET(dma_id, DMA, CONFIG_PTRRST, ETH0RX, reg_config_ptrrst);
    config_ptrrst->eth0tx = RU_FIELD_GET(dma_id, DMA, CONFIG_PTRRST, ETH0TX, reg_config_ptrrst);
    config_ptrrst->eth1rx = RU_FIELD_GET(dma_id, DMA, CONFIG_PTRRST, ETH1RX, reg_config_ptrrst);
    config_ptrrst->eth1tx = RU_FIELD_GET(dma_id, DMA, CONFIG_PTRRST, ETH1TX, reg_config_ptrrst);
    config_ptrrst->eth2rx = RU_FIELD_GET(dma_id, DMA, CONFIG_PTRRST, ETH2RX, reg_config_ptrrst);
    config_ptrrst->eth2tx = RU_FIELD_GET(dma_id, DMA, CONFIG_PTRRST, ETH2TX, reg_config_ptrrst);
    config_ptrrst->eth3rx = RU_FIELD_GET(dma_id, DMA, CONFIG_PTRRST, ETH3RX, reg_config_ptrrst);
    config_ptrrst->eth3tx = RU_FIELD_GET(dma_id, DMA, CONFIG_PTRRST, ETH3TX, reg_config_ptrrst);
    config_ptrrst->eth4rx = RU_FIELD_GET(dma_id, DMA, CONFIG_PTRRST, ETH4RX, reg_config_ptrrst);
    config_ptrrst->eth4tx = RU_FIELD_GET(dma_id, DMA, CONFIG_PTRRST, ETH4TX, reg_config_ptrrst);
    config_ptrrst->gponrx = RU_FIELD_GET(dma_id, DMA, CONFIG_PTRRST, GPONRX, reg_config_ptrrst);
    config_ptrrst->gpontx = RU_FIELD_GET(dma_id, DMA, CONFIG_PTRRST, GPONTX, reg_config_ptrrst);

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
       (emac_index >= 6))
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
       (emac_index >= 6))
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
       (emac_index >= 6))
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
       (emac_index >= 6))
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
    bdmf_address_config_weight,
    bdmf_address_config_periph_source,
    bdmf_address_config_ptrrst,
    bdmf_address_config_max_otf,
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
        dma_debug_info debug_info = { .eth0rxne=parm[2].value.unumber, .eth1rxne=parm[3].value.unumber, .eth2rxne=parm[4].value.unumber, .eth3rxne=parm[5].value.unumber, .eth4rxne=parm[6].value.unumber, .gponrxne=parm[7].value.unumber, .eth0txne=parm[8].value.unumber, .eth1txne=parm[9].value.unumber, .eth2txne=parm[10].value.unumber, .eth3txne=parm[11].value.unumber, .eth4txne=parm[12].value.unumber, .gpontxne=parm[13].value.unumber, .eth0rxu=parm[14].value.unumber, .eth1rxu=parm[15].value.unumber, .eth2rxu=parm[16].value.unumber, .eth3rxu=parm[17].value.unumber, .eth4rxu=parm[18].value.unumber, .gponrxu=parm[19].value.unumber, .eth0txu=parm[20].value.unumber, .eth1txu=parm[21].value.unumber, .eth2txu=parm[22].value.unumber, .eth3txu=parm[23].value.unumber, .eth4txu=parm[24].value.unumber, .gpontxu=parm[25].value.unumber, .sel_src=parm[26].value.unumber, .address=parm[27].value.unumber, .datacs=parm[28].value.unumber, .cdcs=parm[29].value.unumber, .rrcs=parm[30].value.unumber, .valid=parm[31].value.unumber, .ready=parm[32].value.unumber};
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
    case cli_dma_config_weight:
        err = ag_drv_dma_config_weight_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dma_config_periph_source:
        err = ag_drv_dma_config_periph_source_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_dma_config_ptrrst:
    {
        dma_config_ptrrst config_ptrrst = { .eth0rx=parm[2].value.unumber, .eth0tx=parm[3].value.unumber, .eth1rx=parm[4].value.unumber, .eth1tx=parm[5].value.unumber, .eth2rx=parm[6].value.unumber, .eth2tx=parm[7].value.unumber, .eth3rx=parm[8].value.unumber, .eth3tx=parm[9].value.unumber, .eth4rx=parm[10].value.unumber, .eth4tx=parm[11].value.unumber, .gponrx=parm[12].value.unumber, .gpontx=parm[13].value.unumber};
        err = ag_drv_dma_config_ptrrst_set(parm[1].value.unumber, &config_ptrrst);
        break;
    }
    case cli_dma_config_max_otf:
        err = ag_drv_dma_config_max_otf_set(parm[1].value.unumber, parm[2].value.unumber);
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
        bdmf_session_print(session, "eth0rxne = %u (0x%x)\n", debug_info.eth0rxne, debug_info.eth0rxne);
        bdmf_session_print(session, "eth1rxne = %u (0x%x)\n", debug_info.eth1rxne, debug_info.eth1rxne);
        bdmf_session_print(session, "eth2rxne = %u (0x%x)\n", debug_info.eth2rxne, debug_info.eth2rxne);
        bdmf_session_print(session, "eth3rxne = %u (0x%x)\n", debug_info.eth3rxne, debug_info.eth3rxne);
        bdmf_session_print(session, "eth4rxne = %u (0x%x)\n", debug_info.eth4rxne, debug_info.eth4rxne);
        bdmf_session_print(session, "gponrxne = %u (0x%x)\n", debug_info.gponrxne, debug_info.gponrxne);
        bdmf_session_print(session, "eth0txne = %u (0x%x)\n", debug_info.eth0txne, debug_info.eth0txne);
        bdmf_session_print(session, "eth1txne = %u (0x%x)\n", debug_info.eth1txne, debug_info.eth1txne);
        bdmf_session_print(session, "eth2txne = %u (0x%x)\n", debug_info.eth2txne, debug_info.eth2txne);
        bdmf_session_print(session, "eth3txne = %u (0x%x)\n", debug_info.eth3txne, debug_info.eth3txne);
        bdmf_session_print(session, "eth4txne = %u (0x%x)\n", debug_info.eth4txne, debug_info.eth4txne);
        bdmf_session_print(session, "gpontxne = %u (0x%x)\n", debug_info.gpontxne, debug_info.gpontxne);
        bdmf_session_print(session, "eth0rxu = %u (0x%x)\n", debug_info.eth0rxu, debug_info.eth0rxu);
        bdmf_session_print(session, "eth1rxu = %u (0x%x)\n", debug_info.eth1rxu, debug_info.eth1rxu);
        bdmf_session_print(session, "eth2rxu = %u (0x%x)\n", debug_info.eth2rxu, debug_info.eth2rxu);
        bdmf_session_print(session, "eth3rxu = %u (0x%x)\n", debug_info.eth3rxu, debug_info.eth3rxu);
        bdmf_session_print(session, "eth4rxu = %u (0x%x)\n", debug_info.eth4rxu, debug_info.eth4rxu);
        bdmf_session_print(session, "gponrxu = %u (0x%x)\n", debug_info.gponrxu, debug_info.gponrxu);
        bdmf_session_print(session, "eth0txu = %u (0x%x)\n", debug_info.eth0txu, debug_info.eth0txu);
        bdmf_session_print(session, "eth1txu = %u (0x%x)\n", debug_info.eth1txu, debug_info.eth1txu);
        bdmf_session_print(session, "eth2txu = %u (0x%x)\n", debug_info.eth2txu, debug_info.eth2txu);
        bdmf_session_print(session, "eth3txu = %u (0x%x)\n", debug_info.eth3txu, debug_info.eth3txu);
        bdmf_session_print(session, "eth4txu = %u (0x%x)\n", debug_info.eth4txu, debug_info.eth4txu);
        bdmf_session_print(session, "gpontxu = %u (0x%x)\n", debug_info.gpontxu, debug_info.gpontxu);
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
    case cli_dma_config_weight:
    {
        uint8_t rxweight;
        uint8_t txweight;
        err = ag_drv_dma_config_weight_get(parm[1].value.unumber, parm[2].value.unumber, &rxweight, &txweight);
        bdmf_session_print(session, "rxweight = %u (0x%x)\n", rxweight, rxweight);
        bdmf_session_print(session, "txweight = %u (0x%x)\n", txweight, txweight);
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
    case cli_dma_config_ptrrst:
    {
        dma_config_ptrrst config_ptrrst;
        err = ag_drv_dma_config_ptrrst_get(parm[1].value.unumber, &config_ptrrst);
        bdmf_session_print(session, "eth0rx = %u (0x%x)\n", config_ptrrst.eth0rx, config_ptrrst.eth0rx);
        bdmf_session_print(session, "eth0tx = %u (0x%x)\n", config_ptrrst.eth0tx, config_ptrrst.eth0tx);
        bdmf_session_print(session, "eth1rx = %u (0x%x)\n", config_ptrrst.eth1rx, config_ptrrst.eth1rx);
        bdmf_session_print(session, "eth1tx = %u (0x%x)\n", config_ptrrst.eth1tx, config_ptrrst.eth1tx);
        bdmf_session_print(session, "eth2rx = %u (0x%x)\n", config_ptrrst.eth2rx, config_ptrrst.eth2rx);
        bdmf_session_print(session, "eth2tx = %u (0x%x)\n", config_ptrrst.eth2tx, config_ptrrst.eth2tx);
        bdmf_session_print(session, "eth3rx = %u (0x%x)\n", config_ptrrst.eth3rx, config_ptrrst.eth3rx);
        bdmf_session_print(session, "eth3tx = %u (0x%x)\n", config_ptrrst.eth3tx, config_ptrrst.eth3tx);
        bdmf_session_print(session, "eth4rx = %u (0x%x)\n", config_ptrrst.eth4rx, config_ptrrst.eth4rx);
        bdmf_session_print(session, "eth4tx = %u (0x%x)\n", config_ptrrst.eth4tx, config_ptrrst.eth4tx);
        bdmf_session_print(session, "gponrx = %u (0x%x)\n", config_ptrrst.gponrx, config_ptrrst.gponrx);
        bdmf_session_print(session, "gpontx = %u (0x%x)\n", config_ptrrst.gpontx, config_ptrrst.gpontx);
        break;
    }
    case cli_dma_config_max_otf:
    {
        uint8_t max;
        err = ag_drv_dma_config_max_otf_get(parm[1].value.unumber, &max);
        bdmf_session_print(session, "max = %u (0x%x)\n", max, max);
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
        dma_debug_info debug_info = {.eth0rxne=gtmv(m, 1), .eth1rxne=gtmv(m, 1), .eth2rxne=gtmv(m, 1), .eth3rxne=gtmv(m, 1), .eth4rxne=gtmv(m, 1), .gponrxne=gtmv(m, 1), .eth0txne=gtmv(m, 1), .eth1txne=gtmv(m, 1), .eth2txne=gtmv(m, 1), .eth3txne=gtmv(m, 1), .eth4txne=gtmv(m, 1), .gpontxne=gtmv(m, 1), .eth0rxu=gtmv(m, 1), .eth1rxu=gtmv(m, 1), .eth2rxu=gtmv(m, 1), .eth3rxu=gtmv(m, 1), .eth4rxu=gtmv(m, 1), .gponrxu=gtmv(m, 1), .eth0txu=gtmv(m, 1), .eth1txu=gtmv(m, 1), .eth2txu=gtmv(m, 1), .eth3txu=gtmv(m, 1), .eth4txu=gtmv(m, 1), .gpontxu=gtmv(m, 1), .sel_src=gtmv(m, 6), .address=gtmv(m, 10), .datacs=gtmv(m, 1), .cdcs=gtmv(m, 1), .rrcs=gtmv(m, 1), .valid=gtmv(m, 1), .ready=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_dma_debug_info_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", dma_id, debug_info.eth0rxne, debug_info.eth1rxne, debug_info.eth2rxne, debug_info.eth3rxne, debug_info.eth4rxne, debug_info.gponrxne, debug_info.eth0txne, debug_info.eth1txne, debug_info.eth2txne, debug_info.eth3txne, debug_info.eth4txne, debug_info.gpontxne, debug_info.eth0rxu, debug_info.eth1rxu, debug_info.eth2rxu, debug_info.eth3rxu, debug_info.eth4rxu, debug_info.gponrxu, debug_info.eth0txu, debug_info.eth1txu, debug_info.eth2txu, debug_info.eth3txu, debug_info.eth4txu, debug_info.gpontxu, debug_info.sel_src, debug_info.address, debug_info.datacs, debug_info.cdcs, debug_info.rrcs, debug_info.valid, debug_info.ready);
        if(!err) ag_drv_dma_debug_info_set(dma_id, &debug_info);
        if(!err) ag_drv_dma_debug_info_get( dma_id, &debug_info);
        if(!err) bdmf_session_print(session, "ag_drv_dma_debug_info_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", dma_id, debug_info.eth0rxne, debug_info.eth1rxne, debug_info.eth2rxne, debug_info.eth3rxne, debug_info.eth4rxne, debug_info.gponrxne, debug_info.eth0txne, debug_info.eth1txne, debug_info.eth2txne, debug_info.eth3txne, debug_info.eth4txne, debug_info.gpontxne, debug_info.eth0rxu, debug_info.eth1rxu, debug_info.eth2rxu, debug_info.eth3rxu, debug_info.eth4rxu, debug_info.gponrxu, debug_info.eth0txu, debug_info.eth1txu, debug_info.eth2txu, debug_info.eth3txu, debug_info.eth4txu, debug_info.gpontxu, debug_info.sel_src, debug_info.address, debug_info.datacs, debug_info.cdcs, debug_info.rrcs, debug_info.valid, debug_info.ready);
        if(err || debug_info.eth0rxne!=gtmv(m, 1) || debug_info.eth1rxne!=gtmv(m, 1) || debug_info.eth2rxne!=gtmv(m, 1) || debug_info.eth3rxne!=gtmv(m, 1) || debug_info.eth4rxne!=gtmv(m, 1) || debug_info.gponrxne!=gtmv(m, 1) || debug_info.eth0txne!=gtmv(m, 1) || debug_info.eth1txne!=gtmv(m, 1) || debug_info.eth2txne!=gtmv(m, 1) || debug_info.eth3txne!=gtmv(m, 1) || debug_info.eth4txne!=gtmv(m, 1) || debug_info.gpontxne!=gtmv(m, 1) || debug_info.eth0rxu!=gtmv(m, 1) || debug_info.eth1rxu!=gtmv(m, 1) || debug_info.eth2rxu!=gtmv(m, 1) || debug_info.eth3rxu!=gtmv(m, 1) || debug_info.eth4rxu!=gtmv(m, 1) || debug_info.gponrxu!=gtmv(m, 1) || debug_info.eth0txu!=gtmv(m, 1) || debug_info.eth1txu!=gtmv(m, 1) || debug_info.eth2txu!=gtmv(m, 1) || debug_info.eth3txu!=gtmv(m, 1) || debug_info.eth4txu!=gtmv(m, 1) || debug_info.gpontxu!=gtmv(m, 1) || debug_info.sel_src!=gtmv(m, 6) || debug_info.address!=gtmv(m, 10) || debug_info.datacs!=gtmv(m, 1) || debug_info.cdcs!=gtmv(m, 1) || debug_info.rrcs!=gtmv(m, 1) || debug_info.valid!=gtmv(m, 1) || debug_info.ready!=gtmv(m, 1))
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
        uint8_t emac_index=gtmv(m, 1);
        uint8_t numofbuff=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_num_of_writes_set(%u %u %u)\n", dma_id, emac_index, numofbuff);
        if(!err) ag_drv_dma_config_num_of_writes_set(dma_id, emac_index, numofbuff);
        if(!err) ag_drv_dma_config_num_of_writes_get( dma_id, emac_index, &numofbuff);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_num_of_writes_get(%u %u %u)\n", dma_id, emac_index, numofbuff);
        if(err || numofbuff!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 1);
        uint8_t rr_num=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_num_of_reads_set(%u %u %u)\n", dma_id, emac_index, rr_num);
        if(!err) ag_drv_dma_config_num_of_reads_set(dma_id, emac_index, rr_num);
        if(!err) ag_drv_dma_config_num_of_reads_get( dma_id, emac_index, &rr_num);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_num_of_reads_get(%u %u %u)\n", dma_id, emac_index, rr_num);
        if(err || rr_num!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 1);
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
        uint8_t emac_index=gtmv(m, 1);
        uint8_t rxpri=gtmv(m, 8);
        uint8_t txpri=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_pri_set(%u %u %u %u)\n", dma_id, emac_index, rxpri, txpri);
        if(!err) ag_drv_dma_config_pri_set(dma_id, emac_index, rxpri, txpri);
        if(!err) ag_drv_dma_config_pri_get( dma_id, emac_index, &rxpri, &txpri);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_pri_get(%u %u %u %u)\n", dma_id, emac_index, rxpri, txpri);
        if(err || rxpri!=gtmv(m, 8) || txpri!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t emac_index=gtmv(m, 1);
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
        uint8_t emac_index=gtmv(m, 1);
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
        dma_config_ptrrst config_ptrrst = {.eth0rx=gtmv(m, 1), .eth0tx=gtmv(m, 1), .eth1rx=gtmv(m, 1), .eth1tx=gtmv(m, 1), .eth2rx=gtmv(m, 1), .eth2tx=gtmv(m, 1), .eth3rx=gtmv(m, 1), .eth3tx=gtmv(m, 1), .eth4rx=gtmv(m, 1), .eth4tx=gtmv(m, 1), .gponrx=gtmv(m, 1), .gpontx=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_ptrrst_set(%u %u %u %u %u %u %u %u %u %u %u %u %u)\n", dma_id, config_ptrrst.eth0rx, config_ptrrst.eth0tx, config_ptrrst.eth1rx, config_ptrrst.eth1tx, config_ptrrst.eth2rx, config_ptrrst.eth2tx, config_ptrrst.eth3rx, config_ptrrst.eth3tx, config_ptrrst.eth4rx, config_ptrrst.eth4tx, config_ptrrst.gponrx, config_ptrrst.gpontx);
        if(!err) ag_drv_dma_config_ptrrst_set(dma_id, &config_ptrrst);
        if(!err) ag_drv_dma_config_ptrrst_get( dma_id, &config_ptrrst);
        if(!err) bdmf_session_print(session, "ag_drv_dma_config_ptrrst_get(%u %u %u %u %u %u %u %u %u %u %u %u %u)\n", dma_id, config_ptrrst.eth0rx, config_ptrrst.eth0tx, config_ptrrst.eth1rx, config_ptrrst.eth1tx, config_ptrrst.eth2rx, config_ptrrst.eth2tx, config_ptrrst.eth3rx, config_ptrrst.eth3tx, config_ptrrst.eth4rx, config_ptrrst.eth4tx, config_ptrrst.gponrx, config_ptrrst.gpontx);
        if(err || config_ptrrst.eth0rx!=gtmv(m, 1) || config_ptrrst.eth0tx!=gtmv(m, 1) || config_ptrrst.eth1rx!=gtmv(m, 1) || config_ptrrst.eth1tx!=gtmv(m, 1) || config_ptrrst.eth2rx!=gtmv(m, 1) || config_ptrrst.eth2tx!=gtmv(m, 1) || config_ptrrst.eth3rx!=gtmv(m, 1) || config_ptrrst.eth3tx!=gtmv(m, 1) || config_ptrrst.eth4rx!=gtmv(m, 1) || config_ptrrst.eth4tx!=gtmv(m, 1) || config_ptrrst.gponrx!=gtmv(m, 1) || config_ptrrst.gpontx!=gtmv(m, 1))
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
        uint8_t emac_index=gtmv(m, 1);
        uint8_t req_cnt=gtmv(m, 6);
        if(!err) ag_drv_dma_debug_req_cnt_rx_get( dma_id, emac_index, &req_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_dma_debug_req_cnt_rx_get(%u %u %u)\n", dma_id, emac_index, req_cnt);
    }
    {
        uint8_t emac_index=gtmv(m, 1);
        uint8_t req_cnt=gtmv(m, 6);
        if(!err) ag_drv_dma_debug_req_cnt_tx_get( dma_id, emac_index, &req_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_dma_debug_req_cnt_tx_get(%u %u %u)\n", dma_id, emac_index, req_cnt);
    }
    {
        uint8_t emac_index=gtmv(m, 1);
        uint32_t req_cnt=gtmv(m, 32);
        if(!err) ag_drv_dma_debug_req_cnt_rx_acc_get( dma_id, emac_index, &req_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_dma_debug_req_cnt_rx_acc_get(%u %u %u)\n", dma_id, emac_index, req_cnt);
    }
    {
        uint8_t emac_index=gtmv(m, 1);
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
    case bdmf_address_config_weight : reg = &RU_REG(DMA, CONFIG_WEIGHT); blk = &RU_BLK(DMA); break;
    case bdmf_address_config_periph_source : reg = &RU_REG(DMA, CONFIG_PERIPH_SOURCE); blk = &RU_BLK(DMA); break;
    case bdmf_address_config_ptrrst : reg = &RU_REG(DMA, CONFIG_PTRRST); blk = &RU_BLK(DMA); break;
    case bdmf_address_config_max_otf : reg = &RU_REG(DMA, CONFIG_MAX_OTF); blk = &RU_BLK(DMA); break;
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
            BDMFMON_MAKE_PARM("eth0rxne", "eth0rxne", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth1rxne", "eth1rxne", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth2rxne", "eth2rxne", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth3rxne", "eth3rxne", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth4rxne", "eth4rxne", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gponrxne", "gponrxne", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth0txne", "eth0txne", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth1txne", "eth1txne", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth2txne", "eth2txne", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth3txne", "eth3txne", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth4txne", "eth4txne", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gpontxne", "gpontxne", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth0rxu", "eth0rxu", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth1rxu", "eth1rxu", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth2rxu", "eth2rxu", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth3rxu", "eth3rxu", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth4rxu", "eth4rxu", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gponrxu", "gponrxu", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth0txu", "eth0txu", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth1txu", "eth1txu", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth2txu", "eth2txu", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth3txu", "eth3txu", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth4txu", "eth4txu", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gpontxu", "gpontxu", BDMFMON_PARM_NUMBER, 0),
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
        static bdmfmon_cmd_parm_t set_config_weight[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxweight", "rxweight", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txweight", "txweight", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_periph_source[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxsource", "rxsource", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txsource", "txsource", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_ptrrst[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("eth0rx", "eth0rx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth0tx", "eth0tx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth1rx", "eth1rx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth1tx", "eth1tx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth2rx", "eth2rx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth2tx", "eth2tx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth3rx", "eth3rx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth3tx", "eth3tx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth4rx", "eth4rx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth4tx", "eth4tx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gponrx", "gponrx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gpontx", "gpontx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_max_otf[]={
            BDMFMON_MAKE_PARM_ENUM("dma_id", "dma_id", dma_id_enum_table, 0),
            BDMFMON_MAKE_PARM("max", "max", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="debug_info", .val=cli_dma_debug_info, .parms=set_debug_info },
            { .name="config_bbrouteovrd", .val=cli_dma_config_bbrouteovrd, .parms=set_config_bbrouteovrd },
            { .name="config_num_of_writes", .val=cli_dma_config_num_of_writes, .parms=set_config_num_of_writes },
            { .name="config_num_of_reads", .val=cli_dma_config_num_of_reads, .parms=set_config_num_of_reads },
            { .name="config_u_thresh", .val=cli_dma_config_u_thresh, .parms=set_config_u_thresh },
            { .name="config_pri", .val=cli_dma_config_pri, .parms=set_config_pri },
            { .name="config_weight", .val=cli_dma_config_weight, .parms=set_config_weight },
            { .name="config_periph_source", .val=cli_dma_config_periph_source, .parms=set_config_periph_source },
            { .name="config_ptrrst", .val=cli_dma_config_ptrrst, .parms=set_config_ptrrst },
            { .name="config_max_otf", .val=cli_dma_config_max_otf, .parms=set_config_max_otf },
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
        static bdmfmon_cmd_parm_t set_config_weight[]={
            BDMFMON_MAKE_PARM("dma_id", "dma_id", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("emac_index", "emac_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_config_periph_source[]={
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
            { .name="config_weight", .val=cli_dma_config_weight, .parms=set_config_weight },
            { .name="config_periph_source", .val=cli_dma_config_periph_source, .parms=set_config_periph_source },
            { .name="config_ptrrst", .val=cli_dma_config_ptrrst, .parms=set_default },
            { .name="config_max_otf", .val=cli_dma_config_max_otf, .parms=set_default },
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
            { .name="CONFIG_WEIGHT" , .val=bdmf_address_config_weight },
            { .name="CONFIG_PERIPH_SOURCE" , .val=bdmf_address_config_periph_source },
            { .name="CONFIG_PTRRST" , .val=bdmf_address_config_ptrrst },
            { .name="CONFIG_MAX_OTF" , .val=bdmf_address_config_max_otf },
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

