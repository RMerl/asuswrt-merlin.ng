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
#include "drv_epon_lif_ag.h"
#include "drv_epon_xif_ag.h"
#include "drv_epon_epn_ag.h"
#include "drv_epon_link_array_ag.h"

typedef struct {
    bdmf_error_t (*set) (uint32_t burstcap);  /* set callback */
    bdmf_error_t (*get) (uint32_t *burstcap); /* get callback */
}AG_DRV_EPN_BURST_CAP_OP_T; /* ag_drv_epn_burst_cap operation table */
 
static AG_DRV_EPN_BURST_CAP_OP_T ag_drv_epn_burst_cap_op[] =
{
#define EPN_BURST_CAP_SET(x) ag_drv_epn_burst_cap_##x##_set
#define EPN_BURST_CAP_GET(x) ag_drv_epn_burst_cap_##x##_get
#define EPN_BURST_CAP_OP(x) [x]   = {.set=EPN_BURST_CAP_SET(x), .get=EPN_BURST_CAP_GET(x)}
 
    EPN_BURST_CAP_OP(0),
    EPN_BURST_CAP_OP(1),
    EPN_BURST_CAP_OP(2),
    EPN_BURST_CAP_OP(3),
    EPN_BURST_CAP_OP(4),
    EPN_BURST_CAP_OP(5),
    EPN_BURST_CAP_OP(6),
    EPN_BURST_CAP_OP(7),
    EPN_BURST_CAP_OP(8),
    EPN_BURST_CAP_OP(9),
    EPN_BURST_CAP_OP(10),
    EPN_BURST_CAP_OP(11),
    EPN_BURST_CAP_OP(12),
    EPN_BURST_CAP_OP(13),
    EPN_BURST_CAP_OP(14),
    EPN_BURST_CAP_OP(15),
    EPN_BURST_CAP_OP(16),
    EPN_BURST_CAP_OP(17),
    EPN_BURST_CAP_OP(18),
    EPN_BURST_CAP_OP(19),
    EPN_BURST_CAP_OP(20),
    EPN_BURST_CAP_OP(21),
    EPN_BURST_CAP_OP(22),
    EPN_BURST_CAP_OP(23),
    EPN_BURST_CAP_OP(24),
    EPN_BURST_CAP_OP(25),
    EPN_BURST_CAP_OP(26),
    EPN_BURST_CAP_OP(27),
    EPN_BURST_CAP_OP(28),
    EPN_BURST_CAP_OP(29),
    EPN_BURST_CAP_OP(30),
    EPN_BURST_CAP_OP(31)
};


bdmf_error_t ag_drv_epn_burst_cap_set(uint8_t link_idx, uint32_t burstcap)
{
#ifdef VALIDATE_PARMS
    if((link_idx >= 32) ||
       (burstcap >= _20BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_epn_burst_cap_op[link_idx].set(burstcap);
}

bdmf_error_t ag_drv_epn_burst_cap_get(uint8_t link_idx, uint32_t *burstcap)
{
#ifdef VALIDATE_PARMS
    if(!burstcap)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((link_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_epn_burst_cap_op[link_idx].get(burstcap);
}

typedef struct {
    bdmf_error_t (*set) (uint8_t quellidmap);  /* set callback */
    bdmf_error_t (*get) (uint8_t *quellidmap); /* get callback */
}AG_DRV_EPN_QUEUE_LLID_MAP_OP_T; /* ag_drv_epn_queue_llid_map operation table */
 
static AG_DRV_EPN_QUEUE_LLID_MAP_OP_T ag_drv_epn_queue_llid_map_op[] =
{
#define EPN_QUEUE_LLID_MAP_SET(x) ag_drv_epn_queue_llid_map_##x##_set
#define EPN_QUEUE_LLID_MAP_GET(x) ag_drv_epn_queue_llid_map_##x##_get
#define EPN_QUEUE_LLID_MAP_OP(x) [x]   = {.set=EPN_QUEUE_LLID_MAP_SET(x), .get=EPN_QUEUE_LLID_MAP_GET(x)}
 
    EPN_QUEUE_LLID_MAP_OP(0),
    EPN_QUEUE_LLID_MAP_OP(1),
    EPN_QUEUE_LLID_MAP_OP(2),
    EPN_QUEUE_LLID_MAP_OP(3),
    EPN_QUEUE_LLID_MAP_OP(4),
    EPN_QUEUE_LLID_MAP_OP(5),
    EPN_QUEUE_LLID_MAP_OP(6),
    EPN_QUEUE_LLID_MAP_OP(7),
    EPN_QUEUE_LLID_MAP_OP(8),
    EPN_QUEUE_LLID_MAP_OP(9),
    EPN_QUEUE_LLID_MAP_OP(10),
    EPN_QUEUE_LLID_MAP_OP(11),
    EPN_QUEUE_LLID_MAP_OP(12),
    EPN_QUEUE_LLID_MAP_OP(13),
    EPN_QUEUE_LLID_MAP_OP(14),
    EPN_QUEUE_LLID_MAP_OP(15),
    EPN_QUEUE_LLID_MAP_OP(16),
    EPN_QUEUE_LLID_MAP_OP(17),
    EPN_QUEUE_LLID_MAP_OP(18),
    EPN_QUEUE_LLID_MAP_OP(19),
    EPN_QUEUE_LLID_MAP_OP(20),
    EPN_QUEUE_LLID_MAP_OP(21),
    EPN_QUEUE_LLID_MAP_OP(22),
    EPN_QUEUE_LLID_MAP_OP(23),
    EPN_QUEUE_LLID_MAP_OP(24),
    EPN_QUEUE_LLID_MAP_OP(25),
    EPN_QUEUE_LLID_MAP_OP(26),
    EPN_QUEUE_LLID_MAP_OP(27),
    EPN_QUEUE_LLID_MAP_OP(28),
    EPN_QUEUE_LLID_MAP_OP(29),
    EPN_QUEUE_LLID_MAP_OP(30),
    EPN_QUEUE_LLID_MAP_OP(31)
};


bdmf_error_t ag_drv_epn_queue_llid_map_set(uint8_t que_idx, uint8_t quellidmap)
{
#ifdef VALIDATE_PARMS
    if((que_idx >= 32) ||
       (quellidmap >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_epn_queue_llid_map_op[que_idx].set(quellidmap);
}

bdmf_error_t ag_drv_epn_queue_llid_map_get(uint8_t que_idx, uint8_t *quellidmap)
{
#ifdef VALIDATE_PARMS
    if(!quellidmap)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((que_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_epn_queue_llid_map_op[que_idx].get(quellidmap);
}

typedef struct {
    bdmf_error_t (*set) (uint32_t unusedtqcnt);  /* set callback */
    bdmf_error_t (*get) (uint32_t *unusedtqcnt); /* get callback */
}AG_DRV_EPN_UNUSED_TQ_CNT_OP_T; /* ag_drv_epn_unused_tq_cnt operation table */
 
static AG_DRV_EPN_UNUSED_TQ_CNT_OP_T ag_drv_epn_unused_tq_cnt_op[] =
{
#define  EPN_UNUSED_TQ_CNT_SET(x) ag_drv_epn_unused_tq_cnt##x##_set
#define  EPN_UNUSED_TQ_CNT_GET(x) ag_drv_epn_unused_tq_cnt##x##_get
#define  EPN_UNUSED_TQ_CNT_OP(x) [x]   = {.set= EPN_UNUSED_TQ_CNT_SET(x), .get= EPN_UNUSED_TQ_CNT_GET(x)}
  
      EPN_UNUSED_TQ_CNT_OP(0),
      EPN_UNUSED_TQ_CNT_OP(1),
      EPN_UNUSED_TQ_CNT_OP(2),
      EPN_UNUSED_TQ_CNT_OP(3),
      EPN_UNUSED_TQ_CNT_OP(4),
      EPN_UNUSED_TQ_CNT_OP(5),
      EPN_UNUSED_TQ_CNT_OP(6),
      EPN_UNUSED_TQ_CNT_OP(7),
      EPN_UNUSED_TQ_CNT_OP(8),
      EPN_UNUSED_TQ_CNT_OP(9),
      EPN_UNUSED_TQ_CNT_OP(10),
      EPN_UNUSED_TQ_CNT_OP(11),
      EPN_UNUSED_TQ_CNT_OP(12),
      EPN_UNUSED_TQ_CNT_OP(13),
      EPN_UNUSED_TQ_CNT_OP(14),
      EPN_UNUSED_TQ_CNT_OP(15),
      EPN_UNUSED_TQ_CNT_OP(16),
      EPN_UNUSED_TQ_CNT_OP(17),
      EPN_UNUSED_TQ_CNT_OP(18),
      EPN_UNUSED_TQ_CNT_OP(19),
      EPN_UNUSED_TQ_CNT_OP(20),
      EPN_UNUSED_TQ_CNT_OP(21),
      EPN_UNUSED_TQ_CNT_OP(22),
      EPN_UNUSED_TQ_CNT_OP(23),
      EPN_UNUSED_TQ_CNT_OP(24),
      EPN_UNUSED_TQ_CNT_OP(25),
      EPN_UNUSED_TQ_CNT_OP(26),
      EPN_UNUSED_TQ_CNT_OP(27),
      EPN_UNUSED_TQ_CNT_OP(28),
      EPN_UNUSED_TQ_CNT_OP(29),
      EPN_UNUSED_TQ_CNT_OP(30),
      EPN_UNUSED_TQ_CNT_OP(31)
 };

bdmf_error_t ag_drv_epn_unused_tq_cnt_set(uint8_t link_idx, uint32_t unusedtqcnt)
{
#ifdef VALIDATE_PARMS
    if((link_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_epn_unused_tq_cnt_op[link_idx].set(unusedtqcnt);
}

bdmf_error_t ag_drv_epn_unused_tq_cnt_get(uint8_t link_idx, uint32_t *unusedtqcnt)
{
#ifdef VALIDATE_PARMS
    if(!unusedtqcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((link_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_epn_unused_tq_cnt_op[link_idx].get(unusedtqcnt);
}


typedef struct {
    bdmf_error_t (*set) (uint32_t cfgshpmask);  /* set callback */
    bdmf_error_t (*get) (uint32_t *cfgshpmask); /* get callback */
}AG_DRV_EPN_TX_L1S_SHP_QUE_MASK_OP_T; /* ag_drv_epn_tx_l1s_shp_que_mask operation table */
 
static AG_DRV_EPN_TX_L1S_SHP_QUE_MASK_OP_T ag_drv_epn_tx_l1s_shp_que_mask_op[] =
{
#define  EPN_TX_L1S_SHP_QUE_MASK_SET(x) ag_drv_epn_tx_l1s_shp_que_mask_##x##_set
#define  EPN_TX_L1S_SHP_QUE_MASK_GET(x) ag_drv_epn_tx_l1s_shp_que_mask_##x##_get
#define  EPN_TX_L1S_SHP_QUE_MASK_OP(x) [x]   = {.set= EPN_TX_L1S_SHP_QUE_MASK_SET(x), .get= EPN_TX_L1S_SHP_QUE_MASK_GET(x)}
  
      EPN_TX_L1S_SHP_QUE_MASK_OP(0),
      EPN_TX_L1S_SHP_QUE_MASK_OP(1),
      EPN_TX_L1S_SHP_QUE_MASK_OP(2),
      EPN_TX_L1S_SHP_QUE_MASK_OP(3),
      EPN_TX_L1S_SHP_QUE_MASK_OP(4),
      EPN_TX_L1S_SHP_QUE_MASK_OP(5),
      EPN_TX_L1S_SHP_QUE_MASK_OP(6),
      EPN_TX_L1S_SHP_QUE_MASK_OP(7),
      EPN_TX_L1S_SHP_QUE_MASK_OP(8),
      EPN_TX_L1S_SHP_QUE_MASK_OP(9),
      EPN_TX_L1S_SHP_QUE_MASK_OP(10),
      EPN_TX_L1S_SHP_QUE_MASK_OP(11),
      EPN_TX_L1S_SHP_QUE_MASK_OP(12),
      EPN_TX_L1S_SHP_QUE_MASK_OP(13),
      EPN_TX_L1S_SHP_QUE_MASK_OP(14),
      EPN_TX_L1S_SHP_QUE_MASK_OP(15),
      EPN_TX_L1S_SHP_QUE_MASK_OP(16),
      EPN_TX_L1S_SHP_QUE_MASK_OP(17),
      EPN_TX_L1S_SHP_QUE_MASK_OP(18),
      EPN_TX_L1S_SHP_QUE_MASK_OP(19),
      EPN_TX_L1S_SHP_QUE_MASK_OP(20),
      EPN_TX_L1S_SHP_QUE_MASK_OP(21),
      EPN_TX_L1S_SHP_QUE_MASK_OP(22),
      EPN_TX_L1S_SHP_QUE_MASK_OP(23),
      EPN_TX_L1S_SHP_QUE_MASK_OP(24),
      EPN_TX_L1S_SHP_QUE_MASK_OP(25),
      EPN_TX_L1S_SHP_QUE_MASK_OP(26),
      EPN_TX_L1S_SHP_QUE_MASK_OP(27),
      EPN_TX_L1S_SHP_QUE_MASK_OP(28),
      EPN_TX_L1S_SHP_QUE_MASK_OP(29),
      EPN_TX_L1S_SHP_QUE_MASK_OP(30),
      EPN_TX_L1S_SHP_QUE_MASK_OP(31)
 };

bdmf_error_t ag_drv_epn_tx_l1s_shp_que_mask_set(uint8_t shaper_idx, uint32_t cfgshpmask)
{
#ifdef VALIDATE_PARMS
    if((shaper_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_epn_tx_l1s_shp_que_mask_op[shaper_idx].set(cfgshpmask);
}

bdmf_error_t ag_drv_epn_tx_l1s_shp_que_mask_get(uint8_t shaper_idx, uint32_t *cfgshpmask)
{
#ifdef VALIDATE_PARMS
    if(!cfgshpmask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((shaper_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_epn_tx_l1s_shp_que_mask_op[shaper_idx].get(cfgshpmask);
}


typedef struct {
    bdmf_error_t (*set) (uint16_t cfgl2squeend, uint16_t cfgl2squestart);  /* set callback */
    bdmf_error_t (*get) (uint16_t * cfgl2squeend, uint16_t * cfgl2squestart); /* get callback */
}AG_DRV_EPN_TX_L2S_QUEUE_CONFIG_OP_T; /* ag_drv_epn_tx_l2s_queue_config operation table */
 
static AG_DRV_EPN_TX_L2S_QUEUE_CONFIG_OP_T ag_drv_epn_tx_l2s_que_config_op[] =
{
#define  EPN_TX_L2S_QUEUE_CONFIG_SET(x) ag_drv_epn_tx_l2s_que_config_##x##_set
#define  EPN_TX_L2S_QUEUE_CONFIG_GET(x) ag_drv_epn_tx_l2s_que_config_##x##_get
#define  EPN_TX_L2S_QUEUE_CONFIG_OP(x) [x]   = {.set= EPN_TX_L2S_QUEUE_CONFIG_SET(x), .get= EPN_TX_L2S_QUEUE_CONFIG_GET(x)}
  
      EPN_TX_L2S_QUEUE_CONFIG_OP(0),
      EPN_TX_L2S_QUEUE_CONFIG_OP(1),
      EPN_TX_L2S_QUEUE_CONFIG_OP(2),
      EPN_TX_L2S_QUEUE_CONFIG_OP(3),
      EPN_TX_L2S_QUEUE_CONFIG_OP(4),
      EPN_TX_L2S_QUEUE_CONFIG_OP(5),
      EPN_TX_L2S_QUEUE_CONFIG_OP(6),
      EPN_TX_L2S_QUEUE_CONFIG_OP(7),
      EPN_TX_L2S_QUEUE_CONFIG_OP(8),
      EPN_TX_L2S_QUEUE_CONFIG_OP(9),
      EPN_TX_L2S_QUEUE_CONFIG_OP(10),
      EPN_TX_L2S_QUEUE_CONFIG_OP(11),
      EPN_TX_L2S_QUEUE_CONFIG_OP(12),
      EPN_TX_L2S_QUEUE_CONFIG_OP(13),
      EPN_TX_L2S_QUEUE_CONFIG_OP(14),
      EPN_TX_L2S_QUEUE_CONFIG_OP(15),
      EPN_TX_L2S_QUEUE_CONFIG_OP(16),
      EPN_TX_L2S_QUEUE_CONFIG_OP(17),
      EPN_TX_L2S_QUEUE_CONFIG_OP(18),
      EPN_TX_L2S_QUEUE_CONFIG_OP(19),
      EPN_TX_L2S_QUEUE_CONFIG_OP(20),
      EPN_TX_L2S_QUEUE_CONFIG_OP(21),
      EPN_TX_L2S_QUEUE_CONFIG_OP(22),
      EPN_TX_L2S_QUEUE_CONFIG_OP(23),
      EPN_TX_L2S_QUEUE_CONFIG_OP(24),
      EPN_TX_L2S_QUEUE_CONFIG_OP(25),
      EPN_TX_L2S_QUEUE_CONFIG_OP(26),
      EPN_TX_L2S_QUEUE_CONFIG_OP(27),
      EPN_TX_L2S_QUEUE_CONFIG_OP(28),
      EPN_TX_L2S_QUEUE_CONFIG_OP(29),
      EPN_TX_L2S_QUEUE_CONFIG_OP(30),
      EPN_TX_L2S_QUEUE_CONFIG_OP(31)
 };


bdmf_error_t ag_drv_epn_tx_l2s_queue_config_set(uint8_t que_idx, uint16_t cfgl2squeend, uint16_t cfgl2squestart)
{
#ifdef VALIDATE_PARMS
    if((que_idx >= 32) ||
       (cfgl2squeend >= _12BITS_MAX_VAL_) ||
       (cfgl2squestart >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_epn_tx_l2s_que_config_op[que_idx].set(cfgl2squeend, cfgl2squestart);
}

bdmf_error_t ag_drv_epn_tx_l2s_queue_config_get(uint8_t que_idx, uint16_t *cfgl2squeend, uint16_t *cfgl2squestart)
{
#ifdef VALIDATE_PARMS
    if(!cfgl2squeend || !cfgl2squestart)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((que_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_epn_tx_l2s_que_config_op[que_idx].get(cfgl2squeend, cfgl2squestart);
}


typedef struct {
    bdmf_error_t (*set) (uint32_t prvburstlimit);  /* set callback */
    bdmf_error_t (*get) (uint32_t *prvburstlimit); /* get callback */
}AG_DRV_EPN_TX_CTC_BURST_LIMIT_OP_T; /* ag_drv_epn_tx_ctc_burst_limit operation table */

static AG_DRV_EPN_TX_CTC_BURST_LIMIT_OP_T ag_drv_epn_tx_ctc_burst_limit_op[] =
{
#define  EPN_TX_CTC_BURST_LIMIT_SET(x) ag_drv_epn_tx_ctc_burst_limit_##x##_set
#define  EPN_TX_CTC_BURST_LIMIT_GET(x) ag_drv_epn_tx_ctc_burst_limit_##x##_get
#define  EPN_TX_CTC_BURST_LIMIT_OP(x) [x]   = {.set= EPN_TX_CTC_BURST_LIMIT_SET(x), .get= EPN_TX_CTC_BURST_LIMIT_GET(x)}
  
      EPN_TX_CTC_BURST_LIMIT_OP(0),
      EPN_TX_CTC_BURST_LIMIT_OP(1),
      EPN_TX_CTC_BURST_LIMIT_OP(2),
      EPN_TX_CTC_BURST_LIMIT_OP(3),
      EPN_TX_CTC_BURST_LIMIT_OP(4),
      EPN_TX_CTC_BURST_LIMIT_OP(5),
      EPN_TX_CTC_BURST_LIMIT_OP(6),
      EPN_TX_CTC_BURST_LIMIT_OP(7),
      EPN_TX_CTC_BURST_LIMIT_OP(8),
      EPN_TX_CTC_BURST_LIMIT_OP(9),
      EPN_TX_CTC_BURST_LIMIT_OP(10),
      EPN_TX_CTC_BURST_LIMIT_OP(11),
      EPN_TX_CTC_BURST_LIMIT_OP(12),
      EPN_TX_CTC_BURST_LIMIT_OP(13),
      EPN_TX_CTC_BURST_LIMIT_OP(14),
      EPN_TX_CTC_BURST_LIMIT_OP(15),
      EPN_TX_CTC_BURST_LIMIT_OP(16),
      EPN_TX_CTC_BURST_LIMIT_OP(17),
      EPN_TX_CTC_BURST_LIMIT_OP(18),
      EPN_TX_CTC_BURST_LIMIT_OP(19),
      EPN_TX_CTC_BURST_LIMIT_OP(20),
      EPN_TX_CTC_BURST_LIMIT_OP(21),
      EPN_TX_CTC_BURST_LIMIT_OP(22),
      EPN_TX_CTC_BURST_LIMIT_OP(23),
      EPN_TX_CTC_BURST_LIMIT_OP(24),
      EPN_TX_CTC_BURST_LIMIT_OP(25),
      EPN_TX_CTC_BURST_LIMIT_OP(26),
      EPN_TX_CTC_BURST_LIMIT_OP(27),
      EPN_TX_CTC_BURST_LIMIT_OP(28),
      EPN_TX_CTC_BURST_LIMIT_OP(29),
      EPN_TX_CTC_BURST_LIMIT_OP(30),
      EPN_TX_CTC_BURST_LIMIT_OP(31)
};

bdmf_error_t ag_drv_epn_tx_ctc_burst_limit_set(uint8_t l2_queue_idx, uint32_t prvburstlimit)
{
#ifdef VALIDATE_PARMS
    if((l2_queue_idx >= 32) ||
       (prvburstlimit >= _18BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_epn_tx_ctc_burst_limit_op[l2_queue_idx].set(prvburstlimit);
}

bdmf_error_t ag_drv_epn_tx_ctc_burst_limit_get(uint8_t l2_queue_idx, uint32_t *prvburstlimit)
{
#ifdef VALIDATE_PARMS
    if(!prvburstlimit)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((l2_queue_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_epn_tx_ctc_burst_limit_op[l2_queue_idx].get(prvburstlimit);
}

typedef struct {
    bdmf_error_t (*set) (uint32_t cfgllid);  /* set callback */
    bdmf_error_t (*get) (uint32_t *cfgllid); /* get callback */
}AG_DRV_LIF_LLID_OP_T; /* ag_drv_lif_llid operation table */


static AG_DRV_LIF_LLID_OP_T ag_drv_lif_llid_op[] =
{
#define  LIF_LLID_SET(x) ag_drv_lif_llid_##x##_set
#define  LIF_LLID_GET(x) ag_drv_lif_llid_##x##_get
#define  LIF_LLID_OP(x) [x]   = {.set= LIF_LLID_SET(x), .get= LIF_LLID_GET(x)}
  
      LIF_LLID_OP(0),
      LIF_LLID_OP(1),
      LIF_LLID_OP(2),
      LIF_LLID_OP(3),
      LIF_LLID_OP(4),
      LIF_LLID_OP(5),
      LIF_LLID_OP(6),
      LIF_LLID_OP(7),
      LIF_LLID_OP(8),
      LIF_LLID_OP(9),
      LIF_LLID_OP(10),
      LIF_LLID_OP(11),
      LIF_LLID_OP(12),
      LIF_LLID_OP(13),
      LIF_LLID_OP(14),
      LIF_LLID_OP(15),
      LIF_LLID_OP(16),
      LIF_LLID_OP(17),
      LIF_LLID_OP(18),
      LIF_LLID_OP(19),
      LIF_LLID_OP(20),
      LIF_LLID_OP(21),
      LIF_LLID_OP(22),
      LIF_LLID_OP(23),
      LIF_LLID_OP(24),
      LIF_LLID_OP(25),
      LIF_LLID_OP(26),
      LIF_LLID_OP(27),
      LIF_LLID_OP(28),
      LIF_LLID_OP(29),
      LIF_LLID_OP(30),
      LIF_LLID_OP(31)
};

bdmf_error_t ag_drv_lif_llid_set(uint8_t llid_index, uint32_t cfgllid)
{
#ifdef VALIDATE_PARMS
    if((llid_index >= 32) ||
       (cfgllid >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_lif_llid_op[llid_index].set(cfgllid);
}

bdmf_error_t ag_drv_lif_llid_get(uint8_t llid_index, uint32_t *cfgllid)
{
#ifdef VALIDATE_PARMS
    if(!cfgllid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((llid_index >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_lif_llid_op[llid_index].get(cfgllid);
}

typedef struct {
    bdmf_error_t (*set) (uint32_t cfgllid);  /* set callback */
    bdmf_error_t (*get) (uint32_t *cfgllid); /* get callback */
}AG_DRV_XIF_LLID_OP_T; /* ag_drv_xif_llid operation table */

static AG_DRV_XIF_LLID_OP_T ag_drv_xif_llid_op[] =
{
#define  XIF_LLID_SET(x) ag_drv_xif_llid_##x##_set
#define  XIF_LLID_GET(x) ag_drv_xif_llid_##x##_get
#define  XIF_LLID_OP(x) [x]   = {.set= XIF_LLID_SET(x), .get= XIF_LLID_GET(x)}
  
      XIF_LLID_OP(0),
      XIF_LLID_OP(1),
      XIF_LLID_OP(2),
      XIF_LLID_OP(3),
      XIF_LLID_OP(4),
      XIF_LLID_OP(5),
      XIF_LLID_OP(6),
      XIF_LLID_OP(7),
      XIF_LLID_OP(8),
      XIF_LLID_OP(9),
      XIF_LLID_OP(10),
      XIF_LLID_OP(11),
      XIF_LLID_OP(12),
      XIF_LLID_OP(13),
      XIF_LLID_OP(14),
      XIF_LLID_OP(15),
      XIF_LLID_OP(16),
      XIF_LLID_OP(17),
      XIF_LLID_OP(18),
      XIF_LLID_OP(19),
      XIF_LLID_OP(20),
      XIF_LLID_OP(21),
      XIF_LLID_OP(22),
      XIF_LLID_OP(23),
      XIF_LLID_OP(24),
      XIF_LLID_OP(25),
      XIF_LLID_OP(26),
      XIF_LLID_OP(27),
      XIF_LLID_OP(28),
      XIF_LLID_OP(29),
      XIF_LLID_OP(30),
      XIF_LLID_OP(31)
};

bdmf_error_t ag_drv_xif_llid__set(uint8_t llid_index, uint32_t cfgonullid)
{
#ifdef VALIDATE_PARMS
    if((llid_index >= 32) ||
       (cfgonullid >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_xif_llid_op[llid_index].set(cfgonullid);
}

bdmf_error_t ag_drv_xif_llid__get(uint8_t llid_index, uint32_t *cfgonullid)
{
#ifdef VALIDATE_PARMS
    if(!cfgonullid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((llid_index >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_xif_llid_op[llid_index].get(cfgonullid);
}

typedef struct {
    bdmf_error_t (*set) (uint32_t cfgp2psci_lo);  /* set callback */
    bdmf_error_t (*get) (uint32_t *cfgp2psci_lo); /* get callback */
}AG_DRV_LIF_P2P_AE_SCI_LO_OP_T; /* ag_drv_LIF_P2P_AE_SCI_LO operation table */

static AG_DRV_LIF_P2P_AE_SCI_LO_OP_T ag_drv_lif_p2p_ae_sci_lo_op[] =
{
#define  LIF_P2P_AE_SCI_LO_SET(x) ag_drv_lif_p2p_ae_sci_lo_##x##_set
#define  LIF_P2P_AE_SCI_LO_GET(x) ag_drv_lif_p2p_ae_sci_lo_##x##_get
#define  LIF_P2P_AE_SCI_LO_OP(x) [x]   = {.set= LIF_P2P_AE_SCI_LO_SET(x), .get= LIF_P2P_AE_SCI_LO_GET(x)}
  
      LIF_P2P_AE_SCI_LO_OP(0),
      LIF_P2P_AE_SCI_LO_OP(1),
      LIF_P2P_AE_SCI_LO_OP(2),
      LIF_P2P_AE_SCI_LO_OP(3),
      LIF_P2P_AE_SCI_LO_OP(4),
      LIF_P2P_AE_SCI_LO_OP(5),
      LIF_P2P_AE_SCI_LO_OP(6),
      LIF_P2P_AE_SCI_LO_OP(7),
      LIF_P2P_AE_SCI_LO_OP(8),
      LIF_P2P_AE_SCI_LO_OP(9),
      LIF_P2P_AE_SCI_LO_OP(10),
      LIF_P2P_AE_SCI_LO_OP(11),
      LIF_P2P_AE_SCI_LO_OP(12),
      LIF_P2P_AE_SCI_LO_OP(13),
      LIF_P2P_AE_SCI_LO_OP(14),
      LIF_P2P_AE_SCI_LO_OP(15)
};

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_set(uint8_t llid_index, uint32_t cfgp2psci_lo)
{
#ifdef VALIDATE_PARMS
if((llid_index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_lif_p2p_ae_sci_lo_op[llid_index].set(cfgp2psci_lo);
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_get(uint8_t llid_index, uint32_t *cfgp2psci_lo)
{
#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }

    if((llid_index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_lif_p2p_ae_sci_lo_op[llid_index].get(cfgp2psci_lo);
}

typedef struct {
    bdmf_error_t (*set) (uint32_t cfgp2psci_hi);  /* set callback */
    bdmf_error_t (*get) (uint32_t *cfgp2psci_hi); /* get callback */
}AG_DRV_LIF_P2P_AE_SCI_HI_OP_T; /* ag_drv_LIF_P2P_AE_SCI_HI operation table */

static AG_DRV_LIF_P2P_AE_SCI_HI_OP_T ag_drv_lif_p2p_ae_sci_hi_op[] =
{
#define  LIF_P2P_AE_SCI_HI_SET(x) ag_drv_lif_p2p_ae_sci_hi_##x##_set
#define  LIF_P2P_AE_SCI_HI_GET(x) ag_drv_lif_p2p_ae_sci_hi_##x##_get
#define  LIF_P2P_AE_SCI_HI_OP(x) [x]   = {.set= LIF_P2P_AE_SCI_HI_SET(x), .get= LIF_P2P_AE_SCI_HI_GET(x)}
  
      LIF_P2P_AE_SCI_HI_OP(0),
      LIF_P2P_AE_SCI_HI_OP(1),
      LIF_P2P_AE_SCI_HI_OP(2),
      LIF_P2P_AE_SCI_HI_OP(3),
      LIF_P2P_AE_SCI_HI_OP(4),
      LIF_P2P_AE_SCI_HI_OP(5),
      LIF_P2P_AE_SCI_HI_OP(6),
      LIF_P2P_AE_SCI_HI_OP(7),
      LIF_P2P_AE_SCI_HI_OP(8),
      LIF_P2P_AE_SCI_HI_OP(9),
      LIF_P2P_AE_SCI_HI_OP(10),
      LIF_P2P_AE_SCI_HI_OP(11),
      LIF_P2P_AE_SCI_HI_OP(12),
      LIF_P2P_AE_SCI_HI_OP(13),
      LIF_P2P_AE_SCI_HI_OP(14),
      LIF_P2P_AE_SCI_HI_OP(15)
};

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_set(uint8_t llid_index, uint32_t cfgp2psci_hi)
{
#ifdef VALIDATE_PARMS
    if((llid_index >= 16))
        {
            bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
            return BDMF_ERR_RANGE;
        }
#endif

    return ag_drv_lif_p2p_ae_sci_hi_op[llid_index].set(cfgp2psci_hi);
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_get(uint8_t llid_index, uint32_t *cfgp2psci_hi)
{
#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }

    if((llid_index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_lif_p2p_ae_sci_hi_op[llid_index].get(cfgp2psci_hi);
}

typedef struct {
    bdmf_error_t (*set) (uint32_t cfgp2psci_lo);  /* set callback */
    bdmf_error_t (*get) (uint32_t *cfgp2psci_lo); /* get callback */
}AG_DRV_XIF_P2P_AE_SCI_LO_OP_T; /* ag_drv_xif_p2p_ae_sci_lo operation table */

static AG_DRV_XIF_P2P_AE_SCI_LO_OP_T ag_drv_xif_p2p_ae_sci_lo_op[] =
{
#define  XIF_P2P_AE_SCI_LO_SET(x) ag_drv_xif_p2p_ae_sci_lo_##x##_set
#define  XIF_P2P_AE_SCI_LO_GET(x) ag_drv_xif_p2p_ae_sci_lo_##x##_get
#define  XIF_P2P_AE_SCI_LO_OP(x) [x]   = {.set= XIF_P2P_AE_SCI_LO_SET(x), .get= XIF_P2P_AE_SCI_LO_GET(x)}
  
      XIF_P2P_AE_SCI_LO_OP(0),
      XIF_P2P_AE_SCI_LO_OP(1),
      XIF_P2P_AE_SCI_LO_OP(2),
      XIF_P2P_AE_SCI_LO_OP(3),
      XIF_P2P_AE_SCI_LO_OP(4),
      XIF_P2P_AE_SCI_LO_OP(5),
      XIF_P2P_AE_SCI_LO_OP(6),
      XIF_P2P_AE_SCI_LO_OP(7),
      XIF_P2P_AE_SCI_LO_OP(8),
      XIF_P2P_AE_SCI_LO_OP(9),
      XIF_P2P_AE_SCI_LO_OP(10),
      XIF_P2P_AE_SCI_LO_OP(11),
      XIF_P2P_AE_SCI_LO_OP(12),
      XIF_P2P_AE_SCI_LO_OP(13),
      XIF_P2P_AE_SCI_LO_OP(14),
      XIF_P2P_AE_SCI_LO_OP(15)
};


bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_set(uint8_t llid_index, uint32_t cfgp2psci_lo)
{
#ifdef VALIDATE_PARMS
    if((llid_index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_xif_p2p_ae_sci_lo_op[llid_index].set(cfgp2psci_lo);
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_get(uint8_t llid_index, uint32_t *cfgp2psci_lo)
{
#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }

    if((llid_index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_xif_p2p_ae_sci_lo_op[llid_index].get(cfgp2psci_lo);
}

typedef struct {
    bdmf_error_t (*set) (uint32_t cfgp2psci_hi);  /* set callback */
    bdmf_error_t (*get) (uint32_t *cfgp2psci_hi); /* get callback */
}AG_DRV_XIF_P2P_AE_SCI_HI_OP_T; /*ag_drv_xif_p2p_ae_sci_hi operation table */

static AG_DRV_XIF_P2P_AE_SCI_HI_OP_T ag_drv_xif_p2p_ae_sci_hi_op[] =
{
#define  XIF_P2P_AE_SCI_HI_SET(x) ag_drv_xif_p2p_ae_sci_hi_##x##_set
#define  XIF_P2P_AE_SCI_HI_GET(x) ag_drv_xif_p2p_ae_sci_hi_##x##_get
#define  XIF_P2P_AE_SCI_HI_OP(x) [x]   = {.set= XIF_P2P_AE_SCI_HI_SET(x), .get= XIF_P2P_AE_SCI_HI_GET(x)}
  
      XIF_P2P_AE_SCI_HI_OP(0),
      XIF_P2P_AE_SCI_HI_OP(1),
      XIF_P2P_AE_SCI_HI_OP(2),
      XIF_P2P_AE_SCI_HI_OP(3),
      XIF_P2P_AE_SCI_HI_OP(4),
      XIF_P2P_AE_SCI_HI_OP(5),
      XIF_P2P_AE_SCI_HI_OP(6),
      XIF_P2P_AE_SCI_HI_OP(7),
      XIF_P2P_AE_SCI_HI_OP(8),
      XIF_P2P_AE_SCI_HI_OP(9),
      XIF_P2P_AE_SCI_HI_OP(10),
      XIF_P2P_AE_SCI_HI_OP(11),
      XIF_P2P_AE_SCI_HI_OP(12),
      XIF_P2P_AE_SCI_HI_OP(13),
      XIF_P2P_AE_SCI_HI_OP(14),
      XIF_P2P_AE_SCI_HI_OP(15)
};

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_set(uint8_t llid_index, uint32_t cfgp2psci_hi)
{
#ifdef VALIDATE_PARMS
    if((llid_index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_xif_p2p_ae_sci_hi_op[llid_index].set(cfgp2psci_hi);
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_get(uint8_t llid_index, uint32_t *cfgp2psci_hi)
{
#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }

    if((llid_index >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    return ag_drv_xif_p2p_ae_sci_hi_op[llid_index].get(cfgp2psci_hi);
}

