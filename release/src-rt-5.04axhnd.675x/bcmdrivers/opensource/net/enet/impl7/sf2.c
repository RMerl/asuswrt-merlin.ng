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

/*
 *  Created on: Dec/2016
 *      Author: steven.hsieh@broadcom.com
 */

#include <bcmnet.h>
#include "phy_drv_dsl_phy.h"
#include "crossbar_dev.h"
#include <bcm/bcmswapitypes.h>

#include <linux/bcm_skb_defines.h>
#include <linux/rtnetlink.h>

#include "linux/bcm_log.h"

#ifdef PKTC
#include <osl.h>
#endif

#include "port.h"
#include "enet.h"
#include "mux_index.h"
#include "sf2.h"
#include "sf2_common.h"
#include "pmc_switch.h"
#include "phy_drv_dsl_serdes.h"
#include "bcmenet_common.h"

#undef OFFSETOF
#define OFFSETOF(STYPE, MEMBER)     ((size_t) &((STYPE *)0)->MEMBER)

#include "bcm_map_part.h"
#include "bcmmii.h"
#include "bcmmii_xtn.h"

// =========== global/static variables ====================
enetx_port_t *sf2_sw;       /* 1st SF2 switch */
enetx_port_t *sf2_sw_ext;   /* 2nd SF2 switch */
uint32_t sf2_unit_bmap;

#define SF2SW_RREG      extsw_rreg_wrap
#define SF2SW_WREG      extsw_wreg_wrap

#include "sf2_platform.h"

#if defined(CONFIG_NET_SWITCHDEV)
#define SW_SET_HW_FWD(s)      (s)->n.flags |= PORT_FLAG_HW_FWD
#define SW_CLR_HW_FWD(s)      (s)->n.flags &= ~PORT_FLAG_HW_FWD
#define SW_IS_HW_FWD(s)       ((s)->n.flags & PORT_FLAG_HW_FWD)
#else
static uint8_t  sf2_hw_switching_state = HW_SWITCHING_ENABLED;
#define SW_SET_HW_FWD(s)      sf2_hw_switching_state = HW_SWITCHING_ENABLED
#define SW_CLR_HW_FWD(s)      sf2_hw_switching_state = HW_SWITCHING_DISABLED
#define SW_IS_HW_FWD(s)       sf2_hw_switching_state
#endif

/* ETHSWPRIOCONTROL, ETHSWQUEMAP related variables */
static int sf2LanUpPorts_g, sf2WanUpPorts_g;

#if defined(CONFIG_BCM947622)
#define DefaultWANQueBitMap 0xff
#define DefaultQueNoRemap 0x76543210 /* No remapping constant */
static uint32_t wanQueMap = DefaultWANQueBitMap;
uint32_t queRemap = DefaultQueNoRemap;
#else
#define DefaultWANQueBitMap 0xaa
#define DefaultQueRemap 0x77553311 /* Default Map CPU Traffic from Queue 0 to 1 to get basic WAN gurantee */
static uint32_t wanQueMap = DefaultWANQueBitMap;
uint32_t queRemap = DefaultQueRemap;
#endif

#define MaxStreamNumber 40                                              // MaxStreamNumber
static uint32_t maxStreamNumber = MaxStreamNumber, curStreams, queThreConfMode;
#if !defined(SF2_EXTERNAL)
static uint16_t sf2_sw_port_thred[FC_THRED_TOTAL_TYPES][FC_LAN_TXQ_QUEUES];     // sf2_sw_port_thred
static uint16_t sf2_imp0_thred[FC_THRED_TOTAL_TYPES][FC_LAN_TXQ_QUEUES];        // sf2_imp0_thred
static uint16_t sf2_wan_imp1_thred[FC_THRED_TOTAL_TYPES][FC_LAN_TXQ_QUEUES];    // sf2_wan_imp1_thred
#endif

/* 
    ACTIVE_QUEUE_NUMBER_PER_PORT defines actuall actively used 
    queue number in run time for threshold calculation 
*/
#define ACTIVE_QUEUE_NUMBER_PER_PORT (2)
static int active_queue_number_per_port = ACTIVE_QUEUE_NUMBER_PER_PORT;


/* Deep Green Mode enabled flag currently tied to the WebGUI's "Advanced Setup -> Power Management -> Ethernet Auto Power Down & Sleep" checkbox */
#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
static int deep_green_mode_enabled = 1;      //Keep track of whether Deep Green Mode feature is enabled/disabled
static int deep_green_mode_activated = 0;    //Keep track of whether Deep Green Mode feature is activated/deactivated (DGM is activated when the feature is enabled and all ports are disconnected)

static uint32 reg_low_power_exp1;  /* Store REG_LOW_POWER_EXP1 register value before enabling Deep Green Mode so that we can restore value when disabling Deep Green Mode */
#endif

// ----------- static SF2 functions -----------------------
static inline void dumpHexData1(uint8_t *pHead, uint32_t len)
{
    uint32_t i;
    uint8_t *c = pHead;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 0)
            printk("\n");
        printk("0x%02X, ", *c++);
    }
    printk("\n");
}

#if !defined(SF2_EXTERNAL)
static int _get_next_queue(int cur_que, int for_wan, int reverse)
{
    int q;

    if(reverse == 0)
    {
        q = 0;
        if (cur_que != -1) q = cur_que + 1;

        for (; q < FC_LAN_TXQ_QUEUES; q++)
        {
            if (for_wan)
            {
                if (wanQueMap & (1<<q)) return q;
            }
            else
            {
                if ((wanQueMap & (1<<q)) == 0) return q;
            }
        }
    }
    else
    {
        q = FC_LAN_TXQ_QUEUES - 1;
        if (cur_que != -1) q = cur_que - 1;
        for (; q >= 0; q--)
        {
            if (for_wan)
            {
                if (wanQueMap & (1<<q)) return q;
            }
            else
            {
                if ((wanQueMap & (1<<q)) == 0) return q;
            }
        }
    }
    return -1;
}
#define GET_NEXT_WAN_QUE(CUR_QUE) _get_next_queue(CUR_QUE, 1, 0)
#define GET_NEXT_LAN_QUE(CUR_QUE) _get_next_queue(CUR_QUE, 0, 0)
#define GET_PREV_WAN_QUE(CUR_QUE) _get_next_queue(CUR_QUE, 1, 1)
#define GET_PREV_LAN_QUE(CUR_QUE) _get_next_queue(CUR_QUE, 0, 1)

static void _sf2_conf_thred_2reg(int unit, int page, int reg, uint16_t tbl[][FC_LAN_TXQ_QUEUES])
{
    int t, q;

    for (t = 0; t < FC_THRED_TOTAL_TYPES; t++)
    {
        for (q = 0; q < FC_LAN_TXQ_QUEUES; q++)
        {
            SF2SW_WREG(unit, page, reg + t*0x10 + q*2, &tbl[t][q], sizeof(tbl[0][0]));
        }
    }
}
#endif //!SF2_EXTERNAL

static void _sf2_conf_que_thred(int unit);

void link_change_sf2_conf_que_thread(enetx_port_t *port, int up)
{
    int unit = PORT_ON_ROOT_SW(port)?0:1;

    if (port->p.parent_sw != sf2_sw)
        return;

    if (PORT_ROLE_IS_WAN(port))
        if (up) sf2WanUpPorts_g++; else sf2WanUpPorts_g--;
    else
        if (up) sf2LanUpPorts_g++; else sf2LanUpPorts_g--;

    _sf2_conf_que_thred(unit);
}

static int tr_port_for_imp_cnt(enetx_port_t *port, void *ctx)
{
    int *count = ctx;

    if (port->p.port_cap == PORT_CAP_MGMT)
        (*count)++;
    return 0;
}

static int tr_port_for_nonimp_cnt(enetx_port_t *port, void *ctx)
{
    int *count = ctx;

    if (port->has_interface && port->p.port_cap != PORT_CAP_MGMT)
        (*count)++;
    return 0;
}

static int sf2_get_imp_port_cnt(void)
{
    int cnt = 0;
    port_traverse_ports(sf2_sw, tr_port_for_imp_cnt, PORT_CLASS_PORT, &cnt);
    return cnt;
}

static int sf2_get_nonimp_port_cnt(void)
{
    int cnt = 0;
    port_traverse_ports(sf2_sw, tr_port_for_nonimp_cnt, PORT_CLASS_PORT, &cnt);
    return cnt;
}

int inline sf2_imp_port_cnt (void)
{
    static int imp_port_cnt;

    if (imp_port_cnt == 0)
        imp_port_cnt = sf2_get_imp_port_cnt();
    return imp_port_cnt;
}

int inline sf2_nonimp_port_cnt (void)
{
    static int nonimp_port_cnt;

    if (nonimp_port_cnt == 0)
        nonimp_port_cnt = sf2_get_nonimp_port_cnt(); 
    return nonimp_port_cnt;
}

#if defined(SF2_EXTERNAL) || defined(SF2_DUAL)
#if defined(SF2_DUAL)
static void _sf2_ext_conf_que_thred(int unit)
#else
static void _sf2_conf_que_thred(int unit)
#endif
{
    static int sf2_fc_configured = 0;
    uint16_t val;
    int page, offset;
    int p, t, q;
    
    if (sf2_fc_configured) return;

    SF2SW_RREG(unit, PAGE_MANAGEMENT, REG_DEV_ID, &val, sizeof(val));

    if (val==DEV_ID_BCM53134_A0 || val==DEV_ID_BCM53134_B0_B1)
    {
        for (p =0; p < 3; p++)
        {
            page = (p==0)?PAGE_FC_LAN_TXQ:(p==1)?PAGE_FC_IMP0_TXQ:PAGE_FC_IMP1_TXQ;
            for (t = 0; t < FC_THRED_TOTAL_TYPES; t++)
            {
                for (q = 0; q < FC_LAN_TXQ_QUEUES; q++)
                {
                    offset = REG_FC_LAN_TXQ_THD_RSV_QN0 + t*0x10 + q*2;
                    // override chip default with calculated nums from Predrag
                    switch (t) {
                    case FC_THRED_QUE_RSRVD_TYPE:   val = 18;       break;
                    case FC_THRED_QUE_HYSTR_TYPE:   val = 54 + 4*q; break;
                    case FC_THRED_QUE_PAUSE_TYPE:   val = 108+ 8*q; break;
                    case FC_THRED_QUE_DROP_TYPE:    val = 498;      break;
                    case FC_THRED_TTL_HYSTR_TYPE:   val = 179+ 4*q; break;
                    case FC_THRED_TTL_PAUSE_TYPE:   val = 357+ 8*q; break;
                    case FC_THRED_TTL_DROP_TYPE:    val = 498;      break;
                    }
                    SF2SW_WREG(unit, page, offset, &val, sizeof(val));
                }
            }
        }
    }

    sf2_fc_configured = 1;
}
#endif
#if !defined(SF2_EXTERNAL)
static uint32_t acb_xoff_threshold;
static int acb_port_xoff_threshold;

#define TTL_DROP_MAX_FACTOR  53/100     /* Max Percentage of Drop Thred to total buffer */
#define PAUSE_DELAY_BUF_IN_PAGE  33     /* IEEE requested Pause buffering */
#define TTL_PAUSE_MAX_FACTOR  10/100    /* Max Total Pause thread reduction from Drop Total Pause Thread in percentage of total buffer */
#define PORT_PAUSE_MAX_FACTOR 3/100    /* Max Port Pause Thread reduction from Port Pause Thread in percentage of total buffer */
#define TXQ_RSRVD_MAX_FACTOR  5/100     /* Max TXQ Reserved Thread percentage to total buffer */

static void _sf2_conf_que_thred(int unit)
{
    int q, q1, t;
    int thredBase[FC_THRED_TOTAL_TYPES],
        maxFrameLength = 4096, lastLanQue, lastWanQue, wanUpPorts, lanUpPorts;
    int available_switch_buf = SF2_MAX_BUFFER_IN_PAGE;
    int impPortCnt;
    int acb_xoff = acb_port_xoff_threshold > acb_xoff_threshold? acb_port_xoff_threshold: acb_xoff_threshold;
#if (defined(CONFIG_BCM96756) && defined(RTAX58U_V2)) || defined(RTAX3000N) || defined(BR63)
    uint8_t val8;
#endif

#if defined(SF2_DUAL)
    if (unit == 1)
    {
        _sf2_ext_conf_que_thred(unit);
        return;
    }
#endif 
    /*
       The percentage of various factors for different condition
       from base computation result. Don't use parantheses to avoid underflow.
       The design goal is:
       o WAN queues have higher thredsholds than all LAN queues.
       o Higher queues have higher thresholds than lower queues in the same group.
     */

    /* Define IMP Port over LAN port threshold factor */
#define IMP_OVER_WAN_QUE_FACTOR 100/100

    /*
       Define Threshold increment factor with higher priority queue to
       guarantee higher priority queue get slight more chance of resource.
       The value should not caused WAN/LAN threshold reserved when scaled up.
     */
#define LOW_QUE_OVER_HIGH_QUE_FACTOR 100/101

    /* Define WAN Queue over LAN queu factor so that WAN queue gets higher resource chance */
#define LAN_QUE_OVER_WAN_QUE_FACTOR 100/105

    lanUpPorts = sf2LanUpPorts_g;
    wanUpPorts = sf2WanUpPorts_g;

    impPortCnt = sf2_imp_port_cnt();
    if (impPortCnt > lanUpPorts)
        impPortCnt = lanUpPorts;

    /* curStreams is used to calculat Total Pause(Not Drop) to
       reserve head room in buffer to guarantee the minimum buffer by queue reserved threshold
       when stream number is below this number. Term "stream" means traffic to ONE queue. */
    if (queThreConfMode == ThreModeDynamic)
    {
        curStreams = (lanUpPorts + wanUpPorts + impPortCnt) * active_queue_number_per_port;
    }
    else
    {
        curStreams = (sf2_nonimp_port_cnt() + sf2_imp_port_cnt()) * active_queue_number_per_port;
    }

    /* Set a system level cap of max stream number to matching real world */
    if (curStreams > maxStreamNumber)
    {
        curStreams = maxStreamNumber;
    }

    /* Unify all computation to page unit */
    maxFrameLength /= SF2_BYTES_PER_PAGE;

    /* Set Reserved Threshold to two frame size to create hard guarantee for each queue */
    thredBase[FC_THRED_QUE_RSRVD_TYPE] = maxFrameLength * 2;
    if (thredBase[FC_THRED_QUE_RSRVD_TYPE] > SF2_MAX_BUFFER_IN_PAGE * TXQ_RSRVD_MAX_FACTOR)
        thredBase[FC_THRED_QUE_RSRVD_TYPE] = SF2_MAX_BUFFER_IN_PAGE * TXQ_RSRVD_MAX_FACTOR;
        

    /*
       Total Drop Threshold:
       When total queue length exceeds Total Drop, all packet will be dropped even
       for queues under Reserved Threshold. This can only happen when external device
       ignores Pause frame. As the values for protocol violation case or misconfiguration case,
       the value is designed as high as possible to minimum the impact of overrun above
       the total Pause and as final guard to total buffer.  Thus set to to one packet size room
       below hard limits.
     */
    available_switch_buf -= acb_xoff * (lanUpPorts + wanUpPorts);
    //thredBase[FC_THRED_TTL_DROP_TYPE] = available_switch_buf - maxFrameLength;
    thredBase[FC_THRED_TTL_DROP_TYPE] = available_switch_buf;

    if ( thredBase[FC_THRED_TTL_DROP_TYPE] < SF2_MAX_BUFFER_IN_PAGE * TTL_DROP_MAX_FACTOR)
        thredBase[FC_THRED_TTL_DROP_TYPE] = SF2_MAX_BUFFER_IN_PAGE * TTL_DROP_MAX_FACTOR;


    /*
       Compute Total Pause Threshold:
       Need to guarantee hardware reserved threshold in EACH queuue.
       The value depends on simultaneous buffer requesting streams and frame length.
       The value is set to guarantee no drop
     */
    thredBase[FC_THRED_TTL_PAUSE_TYPE] = thredBase[FC_THRED_TTL_DROP_TYPE] - PAUSE_DELAY_BUF_IN_PAGE * curStreams;
    if ((thredBase[FC_THRED_TTL_DROP_TYPE] - thredBase[FC_THRED_TTL_PAUSE_TYPE]) >
        (SF2_MAX_BUFFER_IN_PAGE * TTL_PAUSE_MAX_FACTOR))
        thredBase[FC_THRED_TTL_PAUSE_TYPE] = thredBase[FC_THRED_TTL_DROP_TYPE] -
            (SF2_MAX_BUFFER_IN_PAGE * TTL_PAUSE_MAX_FACTOR);

    /*
       Total Hysteresis:
       Hysteresis will reflect the hop count from this device to the source and
       the latency of each hop's resume operation. Set this as high as possible
       related to PAUSE, set it to half of TTL_PAUSE
     */
    thredBase[FC_THRED_TTL_HYSTR_TYPE] = thredBase[FC_THRED_TTL_PAUSE_TYPE]/2;
    if (thredBase[FC_THRED_TTL_HYSTR_TYPE] < thredBase[FC_THRED_QUE_RSRVD_TYPE])
        thredBase[FC_THRED_TTL_HYSTR_TYPE] = thredBase[FC_THRED_QUE_RSRVD_TYPE] + maxFrameLength/2;

    /* Set per-queue drop threshold to 1/3 of total drop threshold */
    thredBase[FC_THRED_QUE_DROP_TYPE] = thredBase[FC_THRED_TTL_DROP_TYPE]/3;

    /* Set per-queue pause threshould one frame size lower than per-q drop threshold */
    thredBase[FC_THRED_QUE_PAUSE_TYPE] = thredBase[FC_THRED_QUE_DROP_TYPE] - PAUSE_DELAY_BUF_IN_PAGE;
    if ((thredBase[FC_THRED_QUE_DROP_TYPE] - thredBase[FC_THRED_QUE_PAUSE_TYPE]) >
        SF2_MAX_BUFFER_IN_PAGE * PORT_PAUSE_MAX_FACTOR)
    thredBase[FC_THRED_QUE_PAUSE_TYPE] = thredBase[FC_THRED_QUE_DROP_TYPE] - SF2_MAX_BUFFER_IN_PAGE * PORT_PAUSE_MAX_FACTOR;

    /* Set per queue's Hysteresis to be helf of per-q pause threshold */
    thredBase[FC_THRED_QUE_HYSTR_TYPE] = thredBase[FC_THRED_QUE_PAUSE_TYPE]/2;

    /* Compute base WAN queue threashold */
    lastWanQue = GET_PREV_WAN_QUE(-1);

    /* Now Scale WAN Queue (ACB Queue) Thredshold up */
    /* ACB queue should never exceed Hardware Reserved Thread because ACB is monitoring the queuing */
    q1 = lastWanQue;
    for (q = q1; q != -1; q1 = q, q = GET_PREV_WAN_QUE(q))
    {
        for (t = FC_THRED_TOTAL_TYPES - 1; t >= 0; t--)
        {
            switch (t)
            {
                case FC_THRED_QUE_RSRVD_TYPE:
                    /* Set reserve threshold to be the same */
                    sf2_sw_port_thred[t][q] = thredBase[FC_THRED_QUE_RSRVD_TYPE];
                    break;
                default:
                    /* Set all rest thredsholds to 2 frames lower than maximum buffer. */
                    sf2_sw_port_thred[t][q] = SF2_MAX_BUFFER_IN_PAGE - 2 * maxFrameLength;
            }
        }
    }

    /* Now Scale LAN Queue Thredshold down */
    lastLanQue = GET_PREV_LAN_QUE(-1);
    q1 = lastLanQue;
    for (q = q1; q != -1; q1 = q, q = GET_PREV_LAN_QUE(q1))
    {
        for (t = FC_THRED_TOTAL_TYPES - 1; t >= 0; t--)
        {
            switch (t)
            {
                case FC_THRED_QUE_RSRVD_TYPE:
                    /* Set reserve threshold to be the same */
                    sf2_sw_port_thred[t][q] = thredBase[FC_THRED_QUE_RSRVD_TYPE];
                    break;
                default:
                    if (q == lastLanQue)
                    {
                        sf2_sw_port_thred[t][q] = thredBase[t];

                        /* If there is WAN Queue configured, scale down LAN queue with a factor */
                        if (lastWanQue != -1)
                        {
                            sf2_sw_port_thred[t][q] = sf2_sw_port_thred[t][q] * LAN_QUE_OVER_WAN_QUE_FACTOR;
                        }
                    }
                    else
                    {
                        sf2_sw_port_thred[t][q] = sf2_sw_port_thred[t][q1] * LOW_QUE_OVER_HIGH_QUE_FACTOR;
                    }
            }
        }
    }

    /* Configure IMP port */
    for (q = FC_LAN_TXQ_QUEUES - 1; q >= 0; q--)
    {
        for (t = FC_THRED_TOTAL_TYPES - 1; t >= 0; t--)
        {
            switch (t)
            {
                case FC_THRED_QUE_RSRVD_TYPE:
                    sf2_imp0_thred[t][q] = thredBase[t];
                    break;
                default:
                    if (q == FC_LAN_TXQ_QUEUES - 1)
                    {
                        sf2_imp0_thred[t][q] = thredBase[t] * IMP_OVER_WAN_QUE_FACTOR;
                    }
                    else
                    {
                        sf2_imp0_thred[t][q] = sf2_imp0_thred[t][q + 1] * LOW_QUE_OVER_HIGH_QUE_FACTOR;
                    }
            }
        }
    }

    /* Configure IMP1 port for WAN port if it is used or Dual IMP port is used */
    for (q = FC_LAN_TXQ_QUEUES - 1; q >= 0; q--)
    {
        for (t = FC_THRED_TOTAL_TYPES - 1; t >= 0; t--)
        {
            switch (t)
            {
                case FC_THRED_QUE_RSRVD_TYPE:
                    /* Set reserve threshold to be the same */
                    sf2_wan_imp1_thred[t][q] = thredBase[FC_THRED_QUE_RSRVD_TYPE];
                    break;
                default:
                    /* Set all rest thredsholds to 2 frames lower than maximum buffer. */
                    sf2_wan_imp1_thred[t][q] = SF2_MAX_BUFFER_IN_PAGE - 2 * maxFrameLength;
            }
        }
    }

    _sf2_conf_thred_2reg(unit, PAGE_FC_LAN_TXQ, REG_FC_LAN_TXQ_THD_RSV_QN0, sf2_sw_port_thred);
    _sf2_conf_thred_2reg(unit, PAGE_FC_IMP0_TXQ, REG_FC_IMP0_TXQ_THD_RSV_QN0, sf2_imp0_thred);
    _sf2_conf_thred_2reg(unit, PAGE_FC_IMP1_TXQ, REG_FC_IMP0_TXQ_THD_RSV_QN0, sf2_wan_imp1_thred);

#if (defined(CONFIG_BCM96756) && defined(RTAX58U_V2)) || defined(RTAX3000N) || defined(BR63)
#define P8_PORT_ID     8
    SF2SW_RREG(unit, PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(P8_PORT_ID), &val8, sizeof(val8));
    val8 |= REG_PORT_STATE_TX_FLOWCTL | REG_PORT_STATE_RX_FLOWCTL;
    SF2SW_WREG(unit, PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(P8_PORT_ID), &val8, sizeof(val8));

    SF2SW_RREG(unit, PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(P5_PORT_ID), &val8, sizeof(val8));
    val8 |= REG_PORT_STATE_TX_FLOWCTL | REG_PORT_STATE_RX_FLOWCTL;
    SF2SW_WREG(unit, PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(P5_PORT_ID), &val8, sizeof(val8));
#endif /* CONFIG_BCM96756 */
}
#endif //!SF2_EXTERNAL

#define IMP_PORTS_ENABLE    1
#define IMP_PORTS_DISABLE   2
#define IMP_PORTS_SETUP     3

static int tr_imp_ports_op(enetx_port_t *port, void *ctx)
{
    int unit = PORT_ON_ROOT_SW(port)?0:1;
    int *op = ctx;
    int id = port->p.mac->mac_id;
    uint8_t val8, reg, bit;

#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
    if (port->p.port_cap != PORT_CAP_MGMT)
        return 0;
#else
    if ((port->p.port_cap != PORT_CAP_MGMT) || (id != platform_imp_port(unit)))
        return 0;
#endif
    switch (*op) 
    {
    case IMP_PORTS_SETUP:
        // enable BRCM TAG
        reg = (id < P5_PORT_ID) ? REG_BRCM_HDR_CTRL2 : REG_BRCM_HDR_CTRL;
        bit = (id < P5_PORT_ID) ? 1 << id : (id == IMP_PORT_ID) ? BRCM_HDR_EN_IMP_PORT : (id == P7_PORT_ID) ? BRCM_HDR_EN_GMII_PORT_7 : BRCM_HDR_EN_GMII_PORT_5;
        SF2SW_RREG(unit, PAGE_MANAGEMENT, reg, &val8, sizeof(val8));
        val8 |= bit;
        SF2SW_WREG(unit, PAGE_MANAGEMENT, reg, &val8, sizeof(val8));
        
        // enable link - port override register
        if (id == IMP_PORT_ID)
            val8 = IMP_LINK_OVERRIDE_2000FDX /*| REG_CONTROL_MPSO_FLOW_CONTROL*/; /* FIXME : Enabling flow control creates some issues */
        else 
            val8 = (id == P4_PORT_ID) ? LINK_OVERRIDE_1000FDX : LINK_OVERRIDE_1000FDX | REG_PORT_GMII_SPEED_UP_2G;
        SF2SW_WREG(unit, PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(id), &val8, sizeof(val8));
        __attribute__((fallthrough));
        
        // intentional fall thru to _ENABLE
    case IMP_PORTS_ENABLE:
        SF2SW_RREG(unit, PAGE_CONTROL, REG_PORT_CTRL+id, &val8, 1);
        val8 &= ~REG_PORT_CTRL_DISABLE;
        if (id != IMP_PORT_ID)
        {   // set STP state to forward 
            val8 &= ~REG_PORT_STP_MASK;
            val8 |= REG_PORT_STP_STATE_FORWARDING;
        }
        SF2SW_WREG(unit, PAGE_CONTROL, REG_PORT_CTRL+id, &val8, 1);
        break;
    case IMP_PORTS_DISABLE:
        SF2SW_RREG(unit, PAGE_CONTROL, REG_PORT_CTRL+id, &val8, 1);
        val8 |= REG_PORT_CTRL_DISABLE;
        SF2SW_WREG(unit, PAGE_CONTROL, REG_PORT_CTRL+id, &val8, 1);
        break;
    }
    return 0;
}

static void _imp_ports_op(int unit, int op)
{
    int operation = op;
    _port_traverse_ports((unit && sf2_sw_ext) ? sf2_sw_ext : sf2_sw, tr_imp_ports_op, PORT_CLASS_PORT,  &operation, 1);
}

static void _extsw_setup_imp_ports(int unit)
{
    // based on impl5:extsw_setup_imp_ports()
    uint8_t  val8;
    uint16_t val16;

    /* Assumption : External switch is always in MANAGED Mode w/ TAG enabled.
     * but it is not deterministic when the userspace app for external switch
     * will run. When it gets delayed and the device is already getting traffic,
     * all those packets are sent to CPU without external switch TAG.
     * To avoid the race condition - it is better to enable BRCM_TAG during driver init. */
    _imp_ports_op(unit, IMP_PORTS_SETUP);

    /* Enable IMP Port */
    val8 = ENABLE_MII_PORT | RECEIVE_BPDU;
#if defined(SF2_DUAL)
    if (unit && (ext_sw_imp_port != IMP_PORT_ID)) val8 |= ENABLE_DUAL_IMP_PORTS;
#endif
    SF2SW_WREG(unit, PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, sizeof(val8));

    /* management mode, enable forwarding */
    SF2SW_RREG(unit, PAGE_CONTROL, REG_SWITCH_MODE, &val8, sizeof(val8));
    val8 |= REG_SWITCH_MODE_FRAME_MANAGE_MODE | REG_SWITCH_MODE_SW_FWDG_EN;
    SF2SW_WREG(unit, PAGE_CONTROL, REG_SWITCH_MODE, &val8, sizeof(val8));

    /* clear dumb mode */
    val16 = 0;
    SF2SW_WREG(unit, PAGE_CONTROL, REG_SWITCH_CONTROL, &val16, sizeof(val16));

    /* enable rx bcast, ucast and mcast of imp port */
    val8 = REG_MII_PORT_CONTROL_RX_UCST_EN | REG_MII_PORT_CONTROL_RX_MCST_EN |
           REG_MII_PORT_CONTROL_RX_BCST_EN;
    SF2SW_WREG(unit, PAGE_CONTROL, REG_MII_PORT_CONTROL, &val8, sizeof(val8));

    /* Forward lookup failure to use ULF/MLF/IPMC lookup fail registers */
    val8 = REG_PORT_FORWARD_MCST | REG_PORT_FORWARD_UCST | REG_PORT_FORWARD_IP_MCST;
    val8 |= REG_PORT_FORWARD_INRANGEERR_DISCARD | REG_PORT_FORWARD_OUTRANGEERR_DISCARD;
    SF2SW_WREG(unit, PAGE_CONTROL, REG_PORT_FORWARD, &val8, sizeof(val8));

    /* Forward unlearned unicast and unresolved mcast to the MIPS */
    val16 = 1 << platform_imp_port(unit); 
    SF2SW_WREG(unit, PAGE_CONTROL, REG_UCST_LOOKUP_FAIL, &val16, sizeof(val16));
    SF2SW_WREG(unit, PAGE_CONTROL, REG_MCST_LOOKUP_FAIL, &val16, sizeof(val16));
    SF2SW_WREG(unit, PAGE_CONTROL, REG_IPMC_LOOKUP_FAIL, &val16, sizeof(val16));

    /* Disable learning on MIPS*/
    SF2SW_WREG(unit, PAGE_CONTROL, REG_DISABLE_LEARNING, &val16, sizeof(val16));
}

#include "bcm_chip_arch.h"

static void _extsw_set_pbvlan(int unit, int port, uint16_t fwdMap)
{
    SF2SW_WREG(unit, PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (port * 2), (uint8_t *)&fwdMap, 2);
}

#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)

static uint16_t _extsw_get_pbvlan(int unit, int port)
{
    uint16_t val16;

    SF2SW_RREG(unit, PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (port * 2), (uint8_t *)&val16, 2);
    return val16;
}

/*
    Use CFP to force Reserved Multicast Address to be received by
    IMP port correctly overriding Port Based VLAN set for load balancing.
*/
static int _bcmsw_add_cfp_rsvd_multicast_support(int unit)
{
    // based on impl5:bcmsw_add_cfp_rsvd_multicast_support()
    struct ethswctl_data _e, *e = &_e;
    cfpArg_t *cfpArg = &e->cfpArgs;

    memset(e, 0, sizeof(*e));
    e->unit = unit;

    cfpArg->da = 0x0180c2000000LL;
    cfpArg->da_mask = 0xffffff000000;
    cfpArg->argFlag |= CFP_ARG_DA_M;
    cfpArg->l3_framing= CfpL3NoIP;
    cfpArg->argFlag |= CFP_ARG_L3_FRAMING_M;
    cfpArg->op = CFPOP_APPEND;
    cfpArg->argFlag |= CFP_ARG_OP_M;
    cfpArg->chg_fpmap_ib = 2;
    cfpArg->argFlag |= CFP_ARG_CHG_FPMAP_IB_M;
    cfpArg->fpmap_ib = PBMAP_MIPS;
    cfpArg->argFlag |= CFP_ARG_FPMAP_IB_M;
    cfpArg->priority = 2;
    cfpArg->argFlag |= CFP_ARG_PRIORITY_M;

    return ioctl_extsw_cfp(e);
}

static void _bcmsw_print_imp_port_grouping(unsigned long port_map, int port_imp_map[])
{
    // based on impl5:bcmsw_print_imp_port_grouping()
    int port, imp_port, new_grp = 0;
    printk("NOTE: Using Port Grouping for IMP ports : ");
    for (imp_port = 0; imp_port <= MAX_SWITCH_PORTS; imp_port++)
    {
        /* Not an IMP port -- continue */
        if (! ( (1<<imp_port) & DEFAULT_IMP_PBMAP ) ) continue;
        new_grp = 1;
        for (port = 0; port < MAX_SWITCH_PORTS; port++) 
        {
            if ( ((1<<port) & port_map) && 
                 port_imp_map[port] == imp_port )
            {
                if (new_grp)
                {
                    pr_cont("[");
                    new_grp = 0;
                }
                else
                {
                    pr_cont(",");
                }
                pr_cont(" %d",port);
            }
        }
        if (!new_grp)
        {
            pr_cont(" --> %d ] ",imp_port);
        }
    }
    pr_cont("\n");
}

static int *cur_port_imp_map;
static int tr_imp_grp_pmap_get(enetx_port_t *port, void *ctx)
{
    unsigned long *port_map = ctx;
    if (port->has_interface)
        *port_map |= 1 << port->p.mac->mac_id;
    return 0; 
}

static void _extsw_cfg_port_imp_grouping(int unit, int port_imp_map[])
{
    // based on impl5:extsw_cfg_port_imp_grouping()
    unsigned char port;
    unsigned long port_map = 0;
    uint16 v16;

    if (cur_port_imp_map == port_imp_map) return;

    cur_port_imp_map = port_imp_map;
    /* Configure forwarding based on Port Grouping
     * By default all port's pbvlan is 0x1FF */
    _port_traverse_ports(sf2_sw, tr_imp_grp_pmap_get, PORT_CLASS_PORT, &port_map, 1);
    /* NOTE : ports are scanned to cover last IMP port as well -- see details below */
    for (port = 0; port < MAX_SWITCH_PORTS+1; port++)
    {
        v16 = 0;
        if ( !(DEFAULT_IMP_PBMAP & (1<<port)) && (port_imp_map[port] != -1) && ( (1<<port) & port_map ) )
        {
            v16 = _extsw_get_pbvlan(unit, port) & chip_arch_all_portmap[1]; /* Get current PBVLAN Map */
            v16 &= ~(imp_pbmap[1]); /* Remove all IMP Ports from PBVLAN Map for external switch*/
            v16 |= (1<<port_imp_map[port]); /* Add back the desired IMP Port */
            _extsw_set_pbvlan(unit, port, v16);
            port_imp_emac_map[port] = imp_to_emac[port_imp_map[port]];
        }
        else if ( DEFAULT_IMP_PBMAP & (1<<port) ) 
        { /* IMP Port - Block IMP to IMP forwarding */
            /* As such there is no need to block IMP-IMP forwarding because it should NEVER happen
             * But during initial runner development, it was noticed that runner was adding incorrect
             * Broadcom tag (that has destination port as other IMP), this results in packet getting 
             * looped back; In order to avoid this issue temporarily, following is done. 
             * Below change could be kept as permanent, though not needed. */ 
            v16 = _extsw_get_pbvlan(unit, port) & chip_arch_all_portmap[1]; /* Get current PBVLAN Map */
            v16 &= ~(imp_pbmap[1]); /* Remove all IMP Ports from PBVLAN Map for external switch*/
            v16 |= (1<<port); /* Add back this IMP Port - Not required though */
            _extsw_set_pbvlan(unit, port, v16);
        }
    }
    
    _bcmsw_print_imp_port_grouping(port_map, port_imp_map);
}

void _extsw_set_port_imp_map_2_5g(int unit)
{
    _extsw_cfg_port_imp_grouping(unit, port_imp_map_2_5g);
}

void _extsw_set_port_imp_map_non_2_5g(int unit)
{
    _extsw_cfg_port_imp_grouping(unit, port_imp_map_non_2_5g);
}


static void _extsw_setup_imp_fwding(int unit)
{
    // based on impl5:extsw_setup_imp_fwding()
    uint16 v16;

    /* Configure the Lookup failure registers to all IMP ports */
    v16 = DEFAULT_IMP_PBMAP;
    SF2SW_WREG(unit, PAGE_CONTROL, REG_UCST_LOOKUP_FAIL, &v16, sizeof(v16));
    SF2SW_WREG(unit, PAGE_CONTROL, REG_MCST_LOOKUP_FAIL, &v16, sizeof(v16));
    SF2SW_WREG(unit, PAGE_CONTROL, REG_IPMC_LOOKUP_FAIL, &v16, sizeof(v16));
    /* Disable learning on all IMP ports */
    SF2SW_WREG(unit, PAGE_CONTROL, REG_DISABLE_LEARNING, &v16, sizeof(v16));

    _extsw_set_port_imp_map_non_2_5g(unit); /* By default we start with assuming no 2.5G port */

    _bcmsw_add_cfp_rsvd_multicast_support(unit);
}
#else
static void _extsw_setup_imp_fwding(int unit) {}
#endif //!defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)


static void _extsw_port_trunk_init(int unit)
{
    // based on impl5:extsw_port_runk_init()
    int enable_trunk = 0;
#if defined(CONFIG_BCM_KERNEL_BONDING)
    enable_trunk |= 1;
#endif

    if (enable_trunk)
    {
        unsigned char v8;
        SF2SW_RREG(unit, PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        v8 |= ( (1 & TRUNK_EN_LOCAL_M) << TRUNK_EN_LOCAL_S ); /* Enable Trunking */
        v8 |= ( (TRUNK_HASH_DA_SA_VID & TRUNK_HASH_SEL_M) << TRUNK_HASH_SEL_S ); /* Default VID+DA+SA Hash */
        SF2SW_WREG(unit, PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        SF2SW_RREG(unit, PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        printk("LAG/Trunking enabled <0x%02x>\n",v8);
    }
}

static void _sf2_tc_to_cos_default(int unit)
{
    int i, j;
    uint16_t reg_addr;
    uint32_t val32;

    for (i = 0; i <= SF2_IMP0_PORT; i++) // all ports except 6
    {
        if (!(chip_arch_all_portmap[unit] & (1<<i))) continue; // skip undefined ports
        reg_addr = SF2_REG_PORTN_TC_TO_COS + i * 4;
        val32 = 0;
        for (j = 0; j <= SF2_QOS_TC_MAX; j++) // all TC s
        {
            //  TC to COS one-one mapping
            val32 |= (j & SF2_QOS_COS_MASK) << (j * SF2_QOS_COS_SHIFT);
        }
        SF2SW_WREG(unit, PAGE_QOS, reg_addr, &val32, 4);
    }
}

static void _sf2_qos_default(int unit)
{
    uint32_t val32;
    uint32_t port;
    /* Set Global QoS Control */
    SF2SW_RREG(unit, PAGE_QOS, SF2_REG_QOS_GLOBAL_CTRL, &val32, 4);
    val32 |= SF2_QOS_P8_AGGREGATION_MODE;
    SF2SW_WREG(unit, PAGE_QOS, SF2_REG_QOS_GLOBAL_CTRL, &val32, 4);
    /* set SP scheduling on all ports (including IMP) by default */
    for (port=0; port <= SF2_IMP0_PORT;  port++)
    {
        if (!(chip_arch_all_portmap[unit] & (1<<port))) continue; // skip undefined ports
        SF2SW_RREG(unit, PAGE_QOS_SCHEDULER, REG_PN_QOS_PRI_CTL_PORT_0 + (port), &val32, 4);
        val32 &= ~(PN_QOS_SCHED_SEL_M<<PN_QOS_SCHED_SEL_S); /* Clear Bits */
        val32 |= (SF2_ALL_Q_SP<<PN_QOS_SCHED_SEL_S); /* Set SP for all */
        SF2SW_WREG(unit, PAGE_QOS_SCHEDULER, REG_PN_QOS_PRI_CTL_PORT_0 + (port), &val32, 4);
    }
    /* Set default TC to COS config */
    _sf2_tc_to_cos_default(unit);
}

static void config_wan_queue_acb_map(void)
{
#if defined(SF2_ACB_CONTROL_QUE_MAP_0)
    int q;
    volatile uint32_t *sf2_acb_control_que_map_0 = (void *)SF2_ACB_CONTROL_QUE_MAP_0;
    volatile uint32_t *sf2_acb_control_que_map_1 = (void *)SF2_ACB_CONTROL_QUE_MAP_1;
    /* 
       Enable all port's and WAN queues for PORT Based ACB packet count
     */
    *sf2_acb_control_que_map_0 = *sf2_acb_control_que_map_1 = 0;
    for (q=0; q<FC_LAN_TXQ_QUEUES; q++) {
        if ((wanQueMap & (1<<q)) == 0) continue;
        *sf2_acb_control_que_map_0 = *sf2_acb_control_que_map_1 |= 
            (1<<q)|(1<<(q+8))|(1<<(q+16))|(1<<(q+24));
    }
#endif
}

#if !defined(SF2_EXTERNAL)
typedef struct acb_config_s {
    uint16 total_xon_hyst;
    uint16 xon_hyst;
    acb_queue_config_t acb_queue_config[1/*64*/];
} acb_config_t;

static acb_config_t acb_profiles [] = {
    // profile 1
    {
        .total_xon_hyst = 6,
        .xon_hyst = 4,
        {
            // queue 0, (port 0) for LAN->LAN
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
            {
                .pessimistic_mode = 0,
                .total_xon_en = 0,
                .pkt_len = 0,
                .xoff_threshold = 16,
            },
#else // For 148
            {
                .pessimistic_mode = 1,
                .total_xon_en = 1,
                .pkt_len = 6,
                .xoff_threshold = 16,
            },
#endif
        },

    },
};

void _sf2_conf_acb_conges_profile(int profile)
{
    /* based on impl5:sf2_conf_acb_conges_profile() */
    volatile uint32_t *sf2_acb_control    = (void *) (SF2_ACB_CONTROL_REG);
    volatile uint32_t *sf2_acb_xon_thresh = (void *) (sf2_acb_control + 1);
    volatile uint32_t *sf2_acb_que_config = (void *) (sf2_acb_control + 2);
#if defined(SF2_ACB_PORT0_CONFIG_REG)
    volatile uint32_t *sf2_acb_port_config = (void *) (SF2_ACB_PORT0_CONFIG_REG);
#endif
    acb_config_t *p = &acb_profiles [profile];
    int q, val32;
    acb_queue_config_t *qp;

    // acb disable
    *sf2_acb_control &= ~SF2_ACB_EN;
    *sf2_acb_xon_thresh = (p->total_xon_hyst & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_TOTAL_XON_BUFS_S |
                          (p->xon_hyst & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_XON_BUFS_S;

    // compute proper acb_xoff_threshold based on MTU size a buffer is 256 bytes
#if defined(CONFIG_BCM963178) || defined(CONFIG_BCM96756)
    acb_xoff_threshold = 32;    // fixed threshold as 178 has limited number of buffers
    acb_port_xoff_threshold = acb_xoff_threshold * 3/2;     /* Scale up per port XOFF thread to 1.5 times of per queue */
#else
    acb_xoff_threshold = (ENET_MAX_MTU_SIZE + SF2_BYTES_PER_PAGE - 1) / SF2_BYTES_PER_PAGE;  // get number of buffers needed
    acb_xoff_threshold = ((acb_xoff_threshold < 8) ? 8 : acb_xoff_threshold) * 2;   // need to be at least 2 times max packet
    acb_port_xoff_threshold = 0; /* For switch threadshold calculation */
#endif


    for (q = 0; q <= SF2_ACB_QUE_MAX; q++)
    {

        /*
         * We have made room to configure each of the 64 queues with differently
         * defined per q values.
         * Here, we are however, duplicating q0 profiled ACB  config on every queue so we do not
         * leave queues congested for ever when ACB is enabled by default.
         */
        val32 = 0;
        qp = &p->acb_queue_config[0];
        val32 |= (qp->pessimistic_mode & SF2_ACB_QUE_PESSIMISTIC_M)
                                        << SF2_ACB_QUE_PESSIMISTIC_S;
        val32 |= (qp->total_xon_en & SF2_ACB_QUE_TOTAL_XON_M) << SF2_ACB_QUE_TOTAL_XON_S;
        val32 |= (qp->xon_en & SF2_ACB_QUE_XON_M) << SF2_ACB_QUE_XON_S;
        val32 |= (qp->total_xoff_en & SF2_ACB_QUE_TOTAL_XOFF_M) << SF2_ACB_QUE_TOTAL_XOFF_S;
        val32 |= (qp->pkt_len & SF2_ACB_QUE_PKT_LEN_M) << SF2_ACB_QUE_PKT_LEN_S;
        val32 |= (acb_xoff_threshold/*qp->xoff_threshold*/ & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_QUE_XOFF_BUFS_S;
        *(sf2_acb_que_config + q) =  val32;
    }
    // acb enable
#if defined(ACB_ALGORITHM2) && !defined(CONFIG_BCM963178) && !defined(CONFIG_BCM96756)   // For 138, Algorithm2 is applied
    *sf2_acb_control |= SF2_ACB_EN | (SF2_ACB_ALGORITHM_M << SF2_ACB_ALGORITHM_S);
#else
    *sf2_acb_control |= SF2_ACB_EN;
#endif

#if defined(SF2_ACB_PORT0_CONFIG_REG)
    for (q = 0; q < MAX_SWITCH_PORTS; q++) {
        // due to jira37197 strict priorty use acb_xoff_threshold * 3 queues + extra as port xoff limit
        // instead of using acb_port_xoff_threshold which is still used in Congestion calculation.
#if defined(PORT_WITH_8TXQ)
        if (q == PORT_WITH_8TXQ)
            // for port with 8 txqs use 7.5 multiplier to support 8 SP.
            *(sf2_acb_port_config + q) = SF2_ACB_PORT_XOFF_EN | (acb_xoff_threshold*15/2);
        else
#endif
            *(sf2_acb_port_config + q) = SF2_ACB_PORT_XOFF_EN | (acb_xoff_threshold*7/2);
    }
#endif

    config_wan_queue_acb_map();
}
#endif //!SF2_EXTERNAL


// =========== public ioctl functions =====================
// ----------- SIOCETHSWCTLOPS ETHSWINFO functions ---
typedef struct sw_info_s {
    phy_dev_t *gphy;
    int pbmap;
} sw_info_t;

static int tr_sw_info_get(enetx_port_t *port, void *ctx)
{
    sw_info_t *info = ctx;
    phy_dev_t *phy = get_active_phy(port->p.phy);

    if (port->has_interface)
        info->pbmap |= 1 << port->p.mac->mac_id;
    if (!info->gphy && phy->mii_type == PHY_MII_TYPE_GMII)
        info->gphy = phy;
    return 0;
}

int ioctl_extsw_info(struct ethswctl_data *e)
{
    enetx_port_t *sw;
    sw_info_t info;
    int bus_type;
    unsigned int vend_id = 0, dev_id = 0, rev_id = 0;
    u16 v16;
    phy_drv_t *phy_drv;

    if (!IS_UNIT_SF2(e->val))
        return -EFAULT;

    sw = (sf2_sw_ext && e->val) ? sf2_sw_ext : sf2_sw;

#if defined(SF2_EXTERNAL)
    bus_type = MBUS_MDIO;
#else
    bus_type = (sw == sf2_sw_ext) ? MBUS_MDIO : MBUS_MMAP;
#endif

    info.gphy = NULL; 
    info.pbmap = 0;
    _port_traverse_ports(sw, tr_sw_info_get, PORT_CLASS_PORT, &info, 1);

    if (info.gphy) {
        phy_drv = info.gphy->phy_drv;
        phy_bus_read(info.gphy, 2, &v16);
        vend_id = v16; vend_id = __le32_to_cpu(vend_id);
        phy_bus_read(info.gphy, 3, &v16);
        dev_id = v16;  dev_id = __le32_to_cpu(dev_id);
        enet_dbgv("vendor=%x dev=%x\n", vend_id, dev_id);
        if (dev_id >= 0xb000) {
            rev_id = dev_id & 0xf;
            dev_id &= 0xfff0;
        }
    } else {
        enet_err("Error: No integrated PHY defined for device ID in this board design.\n");
        return -EFAULT;
    }

    e->ret_val = bus_type;
    e->vendor_id = vend_id;
    e->dev_id = dev_id;
    e->rev_id = rev_id;
    e->phy_portmap = 0;  // mdk is not used anymore
    e->port_map = info.pbmap;

    return BCM_E_NONE;
}

// ----------- SIOCETHSWCTLOPS ETHSWCOSSCHED functions ---
/*
 * Get/Set StarFighter port scheduling policy
 *** Input params
 * e->type  GET/SET
 * e->port_qos_sched.num_spq  Tells SP/WRR policy to use on the port's queues
 * e->port_qos_sched.wrr_type Granularity packet or 256 byte
 * e->port  per port
 *** Output params
 * e->val has current sched policy - GET
 * Returns 0 for Success, Negative value for failure.
 */
int ioctl_extsw_cosq_sched(enetx_port_t *self, struct ethswctl_data *e)
{
    // based on impl5:sf2_cosq_sched()
    int reg;
    int i, j;
    uint8_t data[8];
    uint8_t val8 = 0;

    down(&self->p.parent_sw->s.conf_sem);

    reg = REG_PN_QOS_PRI_CTL_PORT_0 + e->port * REG_PN_QOS_PRI_CTL_SZ;
    SF2SW_RREG(e->unit, PAGE_QOS_SCHEDULER, reg, &val8, REG_PN_QOS_PRI_CTL_SZ);
    if (e->type == TYPE_GET) {
        switch ((val8 >> PN_QOS_SCHED_SEL_S ) & PN_QOS_SCHED_SEL_M)
        {
            case 0:
                e->port_qos_sched.sched_mode = BCM_COSQ_STRICT;
                break;
            case 5:
                e->port_qos_sched.sched_mode = BCM_COSQ_WRR;
                break;
            default:
                e->port_qos_sched.sched_mode = BCM_COSQ_COMBO;
                e->port_qos_sched.num_spq    = (val8 & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
                break;
        }
        e->port_qos_sched.port_qos_caps = QOS_SCHED_SP_CAP | QOS_SCHED_WRR_CAP | QOS_SCHED_WDR_CAP |
                                        QOS_SCHED_COMBO | QOS_PORT_SHAPER_CAP | QOS_QUEUE_SHAPER_CAP;
        e->port_qos_sched.max_egress_q = NUM_EGRESS_QUEUES;
        e->port_qos_sched.max_egress_spq = MAX_EGRESS_SPQ;
        e->port_qos_sched.wrr_type = (val8 >> PN_QOS_WDRR_GRAN_S) & PN_QOS_WDRR_GRAN_M?
                       QOS_ENUM_WRR_PKT: QOS_ENUM_WDRR_PKT;
        SF2SW_RREG(e->unit, PAGE_QOS_SCHEDULER, REG_PN_QOS_WEIGHT_PORT_0 +
                       e->port * REG_PN_QOS_WEIGHTS, data, DATA_TYPE_HOST_ENDIAN|REG_PN_QOS_WEIGHTS);
        for (i = 0; i < BCM_COS_COUNT; i++) {
            e->weights[i] = data[i];
        }
    } else { // TYPE_SET
        val8 &= ~(PN_QOS_SCHED_SEL_M << PN_QOS_SCHED_SEL_S);
        if (e->port_qos_sched.sched_mode == BCM_COSQ_WRR) {
                val8 |= (SF2_ALL_Q_WRR & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
        } else if ((e->port_qos_sched.sched_mode == BCM_COSQ_SP) &&
                           (e->port_qos_sched.num_spq == 0)) {
                val8 |= (SF2_ALL_Q_SP & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;

        } else {
            switch (e->port_qos_sched.num_spq) {
                case 1:
                    val8 |= (SF2_Q7_SP & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
                    break;
                case 2:
                    val8 |= (SF2_Q7_Q6_SP & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
                    break;
                case 3:
                    val8 |= (SF2_Q7_Q5_SP & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
                    break;
                case 4:
                    val8 |= (SF2_Q7_Q4_SP & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
                    break;
                default:
                    enet_dbg("Incorrect num_spq param %d", e->port_qos_sched.num_spq);
                    up(&self->p.parent_sw->s.conf_sem);
                    return -BCM_E_PARAM;
                    break;
            }
        }
        if (e->port_qos_sched.wrr_type == QOS_ENUM_WRR_PKT) {
            val8 |= SF2_WRR_PKT << PN_QOS_WDRR_GRAN_S;
        } else if (e->port_qos_sched.wrr_type == QOS_ENUM_WDRR_PKT) {
            val8 &= ~(SF2_WRR_PKT << PN_QOS_WDRR_GRAN_S);
        }
        SF2SW_WREG(e->unit, PAGE_QOS_SCHEDULER, reg, &val8, REG_PN_QOS_PRI_CTL_SZ);
 // programming queue weights.
        if (e->port_qos_sched.num_spq || e->port_qos_sched.sched_mode == BCM_COSQ_WRR) {
                      // some or all queues in weighted mode.
            SF2SW_RREG(e->unit, PAGE_QOS_SCHEDULER, REG_PN_QOS_WEIGHT_PORT_0 +
                           e->port * REG_PN_QOS_WEIGHTS, data, DATA_TYPE_HOST_ENDIAN | REG_PN_QOS_WEIGHTS);
            i = e->port_qos_sched.weights_upper? (BCM_COS_COUNT/2): 0;
            for (j = 0; j < BCM_COS_COUNT/2; i++, j++) {
                data[i] = e->weights[i];
            }
            SF2SW_WREG(e->unit, PAGE_QOS_SCHEDULER, REG_PN_QOS_WEIGHT_PORT_0 + e->port * REG_PN_QOS_WEIGHTS,
                            data, DATA_TYPE_HOST_ENDIAN | REG_PN_QOS_WEIGHTS);
        }
    } // SET
    up(&self->p.parent_sw->s.conf_sem);
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWPSEUDOMDIOACCESS functions ---
int ioctl_extsw_pmdioaccess(struct ethswctl_data *e)
{
    int page, reg;
    uint8 data[8] = {0};

    page = (e->offset >> 8) & 0xFF;
    reg = e->offset & 0xFF;
    if (e->type == TYPE_GET) {
        sf2_pseudo_mdio_switch_read(page, reg, data, e->length);
        memcpy((void*)(e->data), (void*)data, e->length);
    } else {
        sf2_pseudo_mdio_switch_write(page, reg, e->data, e->length);
    }
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWPRIOCONTROL functions ---
/*
 * Get/Set StarFighter switch Flowcontrol thresholds.
 *** Input params
 * e->type  GET/SET
 * e->sw_ctrl_type buffer threshold type
 * e->port that determines LAN/IMP0/IMP1 to pick the register set
 * e->val  buffer threshold value to write
 *** Output params
 * e->val has buffer threshold value read for GET
 * Returns 0 for Success, Negative value for failure.
 */
static int _sf2_prio_control(struct ethswctl_data *e)
{
    /* based on impl5:sf2_prio_control() */
    uint16_t val16;
    int reg = 0, page, curThreConfMode, curMaxStreamNumber;

    down(&sf2_sw->s.conf_sem);
    switch (e->sw_ctrl_type)
    {
        case bcmSwitchTxQThresholdConfigMode:
        case bcmSwitchTotalPorts:
        case bcmSwitchLinkUpLanPorts:
        case bcmSwitchLinkUpWanPorts:
        case bcmSwitchMaxStreams:
        case bcmSwitchCurStreams:
        case bcmSwitchActQuePerPort:
            break;
        case bcmSwitchTxQHiReserveThreshold:
            reg = REG_FC_LAN_TXQ_THD_RSV_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTxQHiHysteresisThreshold:
            reg = REG_FC_LAN_TXQ_THD_HYST_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTxQHiPauseThreshold:
            reg = REG_FC_LAN_TXQ_THD_PAUSE_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTxQHiDropThreshold:
            reg = REG_FC_LAN_TXQ_THD_DROP_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTotalHysteresisThreshold:
            reg = REG_FC_LAN_TOTAL_THD_HYST_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTotalPauseThreshold:
            reg = REG_FC_LAN_TOTAL_THD_PAUSE_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTotalDropThreshold:
            reg = REG_FC_LAN_TOTAL_THD_DROP_QN0 + (e->priority * 2);
            break;
        default:
            enet_err("Unknown threshold type \n");
            up(&sf2_sw->s.conf_sem);
            return -BCM_E_PARAM;
    }

    if(e->port == SF2_IMP0_PORT)
    {
        page = PAGE_FC_IMP0_TXQ;
    } else if(e->port == SF2_WAN_IMP1_PORT) {
        page = PAGE_FC_IMP1_TXQ;
    } else if ((e->port < SF2_IMP0_PORT) && (chip_arch_all_portmap[e->unit] & (1<<e->port))) {
        page = PAGE_FC_LAN_TXQ;
    } else {
        enet_err("port # %d error \n", e->port);
        up(&sf2_sw->s.conf_sem);
        return -BCM_E_PARAM;
    }

    //enet_dbg("Threshold: page %d  register offset = %#4x", page, reg);
    /* select port if port based threshold configuration in force */
    if (page == PAGE_FC_LAN_TXQ)
    {
        SF2SW_RREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_CTRL_MODE, &val16, 2);
        if (val16 & FC_CTRL_MODE_PORT) {
            /* port number to port select register */
            val16 = 1 << (REG_FC_CTRL_PORT_P0 + e->port);
            SF2SW_WREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_CTRL_PORT_SEL, &val16, 2);
        }
    }

    if (e->type == TYPE_GET)
    {
        switch (e->sw_ctrl_type)
        {
            case bcmSwitchTxQThresholdConfigMode:
                e->val = queThreConfMode;
                break;
            case bcmSwitchTotalPorts:
                e->val = sf2_nonimp_port_cnt() + sf2_imp_port_cnt();
                break;
            case bcmSwitchLinkUpLanPorts:
                e->val = sf2LanUpPorts_g;
                break;
            case bcmSwitchLinkUpWanPorts:
                e->val = sf2WanUpPorts_g;
                break;
            case bcmSwitchMaxStreams:
                e->val = maxStreamNumber;
                break;
            case bcmSwitchCurStreams:
                e->val = curStreams;
                break;
            case bcmSwitchActQuePerPort:
                e->val = active_queue_number_per_port;
                break;
            default:
                SF2SW_RREG(e->unit, page, reg, &val16, 2);
                //enet_dbg("Threshold read = %4x", val16);
                e->val = val16;
        }
    }
    else
    {
        switch (e->sw_ctrl_type)
        {
            case bcmSwitchTxQThresholdConfigMode:
                curThreConfMode = queThreConfMode;
                queThreConfMode = (uint32_t)e->val;
                if (curThreConfMode != queThreConfMode && queThreConfMode != ThreModeManual )
                {
                    _sf2_conf_que_thred(e->unit);
                }
                break;
            case bcmSwitchMaxStreams:
                curMaxStreamNumber = maxStreamNumber;
                maxStreamNumber = e->val;
                if (maxStreamNumber == -1) maxStreamNumber = MaxStreamNumber;
                if (curMaxStreamNumber != maxStreamNumber && queThreConfMode != ThreModeManual )
                {
                    _sf2_conf_que_thred(e->unit);
                }
                break;
            case bcmSwitchActQuePerPort:
                if(active_queue_number_per_port != e->val)
                {
                    active_queue_number_per_port = e->val;
                    _sf2_conf_que_thred(e->unit);
                }
                break;
            default:
                val16 = (uint32_t)e->val;
                //ent_dbg("e->val is = %4x", e->val);
                SF2SW_WREG(e->unit, page, reg, &val16, 2);
        }
    }
    up(&sf2_sw->s.conf_sem);
    return 0;
}

int ioctl_extsw_prio_control(struct ethswctl_data *e)
{
    /* based on impl5:bcmeapi_ioctl_extsw_prio_control() */
    int ret = 0;
    if ((ret =  _sf2_prio_control(e)) >= 0) {
        if (e->type == TYPE_GET) {
            e->ret_val = e->val;
            //enet_dbg("e->ret_val is = %4x", e->ret_val);
        }
    }
    return ret;
}

// ----------- SIOCETHSWCTLOPS ETHSWQUEMAP functions ---
#if defined(ARCHER_DEVICE)
static int tr_port_q_map(enetx_port_t *port, void *ctxt)
{
    uint32_t *queRemap = (uint32_t *)ctxt;
    bcmFun_t *enet_sysport_q_map = bcmFun_get(BCM_FUN_ID_ENET_SYSPORT_QUEUE_MAP);
    int ndx;
    bcmSysport_QueueMap_t map;

    if (!port->has_interface) return 0;

    if (!enet_sysport_q_map)
        return -EOPNOTSUPP;

    map.blog_chnl = port->n.blog_chnl;

    for (ndx = 0; ndx < BCM_ENET_SYSPORT_QUEUE_MAP_PRIORITY_MAX; ndx++) {
#if defined(PORT_WITH_8TXQ)
        if ((port->p.mac->mac_id == PORT_WITH_8TXQ) && PORT_ON_ROOT_SW(port) &&
            (port->n.port_netdev_role == PORT_NETDEV_ROLE_WAN))
            map.priority_to_switch_queue[ndx] = ndx;
        else
#endif
            map.priority_to_switch_queue[ndx] = (*queRemap >> (ndx *4)) & 0xf;
    }

    return enet_sysport_q_map(&map) ? -EINVAL : 0;
}

int bcmenet_sysport_q_map(uint32_t queRemap)
{
    uint32_t map = queRemap;
    return port_traverse_ports(root_sw, tr_port_q_map, PORT_CLASS_PORT, &map);
}
#endif // ARCHER_DEVICE

int ioctl_extsw_que_map(struct ethswctl_data *e)
{
    /* based on impl5:bcmeapi_ioctl_que_map() */
    if (e->type == TYPE_SET) {
        if (e->val != -1) {
            wanQueMap = e->val;
            config_wan_queue_acb_map();
        }
        if (e->priority != -1) {
#if defined(ARCHER_DEVICE)
            int res = bcmenet_sysport_q_map(e->priority);
            if (res) return res;
#endif
            queRemap = e->priority;
        }
        _sf2_conf_que_thred(e->unit);
    }
    e->val = wanQueMap;
    e->priority = queRemap;

    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWQUEMON functions ---
static void _check_que_mon_port(int unit, int port)
{
    /* based on impl5: check_que_mon_port() */
    static uint16 last_port = -1;

    if (last_port == port) return;
    last_port = port;
    SF2SW_WREG(unit, PAGE_FLOW_CTRL_XTN, REG_FC_DIAG_CTRL, &last_port, 2);
    return;
}

int ioctl_extsw_que_mon(struct ethswctl_data *e)
{
    /* based on impl5:bcmeapi_ioctl_que_mon() */
    int port = e->port,
        que = e->priority,
        type = e->sw_ctrl_type,
        val, err = 0;
    uint16_t v16 = 0;

    switch(type)
    {
        case QUE_CUR_COUNT:
            _check_que_mon_port(e->unit, port);
            SF2SW_RREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_QUE_CUR_COUNT + que*2, &v16, 2);
            break;
        case QUE_PEAK_COUNT:
            _check_que_mon_port(e->unit, port);
            SF2SW_RREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_QUE_PEAK_COUNT + que*2, &v16, 2);
            break;
        case SYS_TOTAL_PEAK_COUNT:
            SF2SW_RREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_SYS_TOTAL_PEAK_COUNT, &v16, 2);
            break;
        case SYS_TOTAL_USED_COUNT:
            SF2SW_RREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_SYS_TOTAL_USED_COUNT, &v16, 2);
            break;

        case PORT_PEAK_RX_BUFFER:
            _check_que_mon_port(e->unit, port);
            SF2SW_RREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_PORT_PEAK_RX_BUFFER, &v16, 2);
            break;
        case QUE_FINAL_CONGESTED_STATUS:
            SF2SW_RREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_QUE_FINAL_CONG_STAT + 2*port, &v16, 2);
            break;
        case PORT_PAUSE_HISTORY:
            SF2SW_RREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_PORT_PAUSE_HISTORY, &v16, 2);
            break;
        case PORT_PAUSE_QUAN_HISTORY:
            SF2SW_RREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_PORT_PAUSE_QUAN_HISTORY, &v16, 2);
            break;

        case PORT_RX_BASE_PAUSE_HISTORY:
            SF2SW_RREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_PORT_RXBASE_PAUSE_HISTORY, &v16, 2);
            break;
        case PORT_RX_BUFFER_ERROR_HISTORY:
            SF2SW_RREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_PORT_RX_BUFFER_ERR_HISTORY, &v16, 2);
            break;
        case QUE_CONGESTED_STATUS:
            SF2SW_RREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_QUE_CONG_STATUS + 2*port, &v16, 2);
            break;
        case QUE_TOTAL_CONGESTED_STATUS:
            SF2SW_RREG(e->unit, PAGE_FLOW_CTRL_XTN, REG_FC_QUE_TOTAL_CONG_STATUS + 2*port, &v16, 2);
            break;
    }

    val = v16;
    e->val = val;
    e->ret_val = err;
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWMACLMT functions ---
int ioctl_extsw_maclmt(struct ethswctl_data *e)
{
    int port = e->port,
        type = e->sw_ctrl_type;
    uint32_t val32;

    if (type < GLOBAL_LIMIT && (port < 0 || port > SF2_IMP0_PORT) )
        return -BCM_E_PARAM;
    if (e->type == TYPE_SET) {      // SET
        switch(type) {
        case PORT_LIMIT_EN:
            SF2SW_RREG(e->unit, PAGE_SA_LIMIT, REG_SA_LIMIT_EN, &val32, 4);
            val32 = (e->val)? val32 | (1<<port) : val32 & ~(1<<port);
            SF2SW_WREG(e->unit, PAGE_SA_LIMIT, REG_SA_LIMIT_EN, &val32, 4);
            break;
        case PORT_LIMIT:
            SF2SW_RREG(e->unit, PAGE_SA_LIMIT, REG_SA_LIMIT_CTRL_P(port), &val32, 4);
            val32 &= ~(MACLMT_LIMIT_M <<MACLMT_LIMIT_S);
            val32 |= (e->val & MACLMT_LIMIT_M) << MACLMT_LIMIT_S;
            SF2SW_WREG(e->unit, PAGE_SA_LIMIT, REG_SA_LIMIT_CTRL_P(port), &val32, 4);
            break;
       case PORT_ACTION:
            SF2SW_RREG(e->unit, PAGE_SA_LIMIT, REG_SA_LIMIT_CTRL_P(port), &val32, 4);
            val32 &= ~(MACLMT_ACTION_M <<MACLMT_ACTION_S);
            val32 |= (e->val & MACLMT_ACTION_M) << MACLMT_ACTION_S;
            SF2SW_WREG(e->unit, PAGE_SA_LIMIT, REG_SA_LIMIT_CTRL_P(port), &val32, 4);
            break;
        case PORT_RST_OVER_LIMIT_PKT_COUNT:
            val32 = 1 << port;
            SF2SW_WREG(e->unit, PAGE_SA_LIMIT, REG_SA_OVRLIMIT_CNTR_RST, &val32, 4);
            break;
        case GLOBAL_RST_OVER_LIMIT_PKT_COUNT:
            val32 = (1 << SF2_IMP0_PORT)-1;
            SF2SW_WREG(e->unit, PAGE_SA_LIMIT, REG_SA_OVRLIMIT_CNTR_RST, &val32, 4);
            break;
        case GLOBAL_LIMIT:
            SF2SW_RREG(e->unit, PAGE_SA_LIMIT, REG_TOTAL_SA_LIMIT_CTL, &val32, 4);
            val32 &= ~(MACLMT_LIMIT_M <<MACLMT_LIMIT_S);
            val32 |= (e->val & MACLMT_LIMIT_M) << MACLMT_LIMIT_S;
            SF2SW_WREG(e->unit, PAGE_SA_LIMIT, REG_TOTAL_SA_LIMIT_CTL, &val32, 4);
            break;
        default: return -BCM_E_PARAM;
        }
        
    } else {                        // GET
        switch(type) {
        case PORT_LIMIT_EN:
            SF2SW_RREG(e->unit, PAGE_SA_LIMIT, REG_SA_LIMIT_EN, &val32, 4);
            e->val = (val32 & 1<<port) ? 1:0;
            break;
        case PORT_LIMIT:
            SF2SW_RREG(e->unit, PAGE_SA_LIMIT, REG_SA_LIMIT_CTRL_P(port), &val32, 4);
            e->val = (val32 >> MACLMT_LIMIT_S) & MACLMT_LIMIT_M;
            break;
        case PORT_ACTION:
            SF2SW_RREG(e->unit, PAGE_SA_LIMIT, REG_SA_LIMIT_CTRL_P(port), &val32, 4);
            e->val = (val32 >> MACLMT_ACTION_S) & MACLMT_ACTION_M;
            break;
        case PORT_LEARNED_COUNT:
            SF2SW_RREG(e->unit, PAGE_SA_LIMIT, REG_SA_LRN_CNTR_P(port), &val32, 4);
            e->val = (val32 >> MACLMT_LIMIT_S) & MACLMT_LIMIT_M;
            break;
        case PORT_OVER_LIMIT_PKT_COUNT:
            SF2SW_RREG(e->unit, PAGE_SA_LIMIT, REG_SA_OVRLIMIT_CNTR_P(port), &val32, 4);
            e->val = val32;
            break;
        case GLOBAL_LIMIT:
            SF2SW_RREG(e->unit, PAGE_SA_LIMIT, REG_TOTAL_SA_LIMIT_CTL, &val32, 4);
            e->val = (val32 >> MACLMT_LIMIT_S) & MACLMT_LIMIT_M;
            break;
        case GLOBAL_LEARNED_COUNT:
            SF2SW_RREG(e->unit, PAGE_SA_LIMIT, REG_TOTAL_SA_LRN_CNTR, &val32, 4);
            e->val = (val32 >> MACLMT_LIMIT_S) & MACLMT_LIMIT_M;
            break;
        default: return -BCM_E_PARAM;
        }
    }
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWCOSPORTMAP functions ---
/*
 * Get/Set cos(queue) mapping, given priority (TC)
 *** Input params
 * e->type  GET/SET
 * e->queue - target queue
 * e->port  per port
 *** Output params
 * Returns 0 for Success, Negative value for failure.
 */
int ioctl_extsw_cosq_port_mapping(struct ethswctl_data *e)
{
    /* based on impl5:bcmeapi_ioctl_extsw_cosq_port_mapping() */
    union {
        uint32_t val32;
        uint16_t val16;
    }val;
    int queue;
    int retval = 0;
    uint16_t reg_addr;
    uint16_t cos_shift;
    uint16_t cos_mask;
    uint16_t reg_len;

    //enet_dbg("Given port: %02d priority: %02d \n ", e->port, e->priority);
    if (e->port >= TOTAL_SWITCH_PORTS || !(chip_arch_all_portmap[e->unit] & (1<<e->port))) {
        printk("Invalid Switch Port %02d \n", e->port);
        return -BCM_E_ERROR;
    }
    if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
        printk("Invalid Priority \n");
        return -BCM_E_ERROR;
    }
    reg_addr  = SF2_REG_PORTN_TC_TO_COS + e->port * 4;
    cos_shift = SF2_QOS_COS_SHIFT;
    cos_mask  = SF2_QOS_COS_MASK;
    reg_len   = 4;

    down(&sf2_sw->s.conf_sem);

    if (e->type == TYPE_GET) {
        SF2SW_RREG(e->unit, PAGE_QOS, reg_addr, (uint8_t *)&val, reg_len);
        //enet_dbg("REG_QOS_PORT_PRIO_MAP_Px = %px\n", (void*)&val);
        /* Get the queue */
        queue = (val.val32 >> (e->priority * cos_shift)) & cos_mask;
        retval = queue & SF2_QOS_COS_MASK;
        //enet_dbg("%s queue is = %4x\n", __FUNCTION__, retval);
    } else {
        //enet_dbg("Given queue: 0x%02x \n ", e->queue);
        SF2SW_RREG(e->unit, PAGE_QOS, reg_addr, (uint8_t *)&val, reg_len);
        val.val32 &= ~(cos_mask << (e->priority * cos_shift));
        val.val32 |= (e->queue & cos_mask) << (e->priority * cos_shift);
        SF2SW_WREG(e->unit, PAGE_QOS, reg_addr, (uint8_t *)&val, reg_len);
    }
    up(&sf2_sw->s.conf_sem);
    return retval;
}

#if !defined(SF2_EXTERNAL)
// ----------- SIOCETHSWCTLOPS ETHSWACBCONTROL functions ---
#if defined(ACB_ALGORITHM2) && !defined(CONFIG_BCM963178) && !defined(CONFIG_BCM96756)
static void _sf2_set_acb_algorithm(int algorithm)
{
    volatile uint32_t *sf2_acb_control    = (void *) (SF2_ACB_CONTROL_REG);
    volatile uint32_t *sf2_acb_que_config = (void *) (sf2_acb_control + 2);
    volatile uint32_t *sf2_acb_que_in_flight = (void *)(SF2_ACB_QUE0_PKTS_IN_FLIGHT);
    uint32_t val32, q;

    if (algorithm) // ACB_ALGORITHM2
    {
        *sf2_acb_control &= ~(SF2_ACB_EN|(SF2_ACB_ALGORITHM_M<<SF2_ACB_ALGORITHM_S)|
                (SF2_ACB_FLUSH_M<<SF2_ACB_FLUSH_S)|(SF2_ACB_EOP_DELAY_M<<SF2_ACB_EOP_DELAY_S));

        for (q = 0; q <= SF2_ACB_QUE_MAX; q++)
        {
            val32 = *(sf2_acb_que_config + q);
            val32 &= ~((SF2_ACB_QUE_PESSIMISTIC_M << SF2_ACB_QUE_PESSIMISTIC_S)|
                    (SF2_ACB_QUE_PKT_LEN_M<<SF2_ACB_QUE_PKT_LEN_S));
            *(sf2_acb_que_config + q) =  val32;
            *(sf2_acb_que_in_flight + q) = 0;
        }

        *sf2_acb_control |= SF2_ACB_EN|(SF2_ACB_ALGORITHM_M<<SF2_ACB_ALGORITHM_S)|
            (0x32<<SF2_ACB_EOP_DELAY_S);
    }
    else
    {
        *sf2_acb_control &= ~(SF2_ACB_EN|(SF2_ACB_ALGORITHM_M<<SF2_ACB_ALGORITHM_S)|
                (SF2_ACB_FLUSH_M<<SF2_ACB_FLUSH_S)|(SF2_ACB_EOP_DELAY_M<<SF2_ACB_EOP_DELAY_S));
        *sf2_acb_control &= ~SF2_ACB_EN;
        for (q = 0; q <= SF2_ACB_QUE_MAX; q++)
        {
            val32 = *(sf2_acb_que_config + q);
            val32 &= ~(SF2_ACB_QUE_PKT_LEN_M << SF2_ACB_QUE_PKT_LEN_S);
            val32 |= 6 << SF2_ACB_QUE_PKT_LEN_S;
            val32 |= (SF2_ACB_QUE_PESSIMISTIC_M << SF2_ACB_QUE_PESSIMISTIC_S);
            *(sf2_acb_que_config + q) =  val32;
            *(sf2_acb_que_in_flight + q) = 0;
        }
        *sf2_acb_control |= SF2_ACB_EN;
    }
}
#endif /* defined(ACB_ALGORITHM2) */

static void dump_sf2_fc_info(int unit)
{
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
    volatile uint32_t *sf2_acb_que_in_flight = (void *)(SF2_ACB_QUE0_PKTS_IN_FLIGHT);
#endif
    uint32_t diag_ctrl, val, i, j;
    SF2SW_RREG(unit, PAGE_FLOW_CTRL_XTN, REG_FC_DIAG_CTRL, &diag_ctrl, 4);  // save current diag control

    printk("Note: Congested Status fields: F-final(@0a-60) C-congest(@0a-80) T-total(@0a-9a)\n"
           "                       level: .< reserved_thd < R < hyst_thd < pause_thd < P < drop_thd < D\n\n");
    SF2SW_RREG(unit, PAGE_FLOW_CTRL_XTN, REG_FC_SYS_TOTAL_PEAK_COUNT, &i, 4);
    SF2SW_RREG(unit, PAGE_FLOW_CTRL_XTN, REG_FC_SYS_TOTAL_USED_COUNT, &j, 4);
    printk(
#if defined(ACB_ALGORITHM2)
           "        in flight         |    mon TX    total=%4d|    peak TX        total=%4d| Congested Status FCT\n"
           "   q0 q1 q2 q3 q4 q5 q6 q7| q0 q1 q2 q3 q4 q5 q6 q7| q0 q1 q2 q3 q4 q5 q6 q7 port| q0  q1  q2  q3  q4  q5  q6  q7\n",
#else
           "        mon TX    total=%4d|    peak TX        total=%4d| Congested Status FCT\n"
           "   q0 q1 q2 q3 q4 q5 q6 q7| q0 q1 q2 q3 q4 q5 q6 q7 port| q0  q1  q2  q3  q4  q5  q6  q7\n",
#endif
           j & 0x7ff, i & 0x7ff);

    for (i = 0; i <= 8; i++)
    {
        uint32_t final, congest, total;
#if defined(ACB_ALGORITHM2)
        uint32_t inflight[8];
#endif
        uint32_t mon[8];

        if (!(chip_arch_all_portmap[unit] & (1<<i))) continue; // skip undefined ports

        SF2SW_WREG(unit, PAGE_FLOW_CTRL_XTN, REG_FC_DIAG_CTRL, &i, 4);
        printk("p%d", i);
        // read in flight & mon values right next to each other to get more accurate values
        for (j = 0; j < 8; j++)
        {
#if defined(ACB_ALGORITHM2)
            if (i < 8)
                inflight[j]= *(sf2_acb_que_in_flight + i*8 + j);
#endif
            SF2SW_RREG(unit, PAGE_FLOW_CTRL_XTN, REG_FC_QUE_CUR_COUNT+j*2, &(mon[j]), 4);
        }
#if defined(ACB_ALGORITHM2)
        // dump in flight regs
        for (j = 0; j < 8; j++)
        {
            if (i<8)
            {
                pr_cont("%3d", inflight[j] & 0x7ff);
            }
            else
                pr_cont("   ");
        }
        pr_cont("|");
#endif
        // dump monitor tx regs
        for (j = 0; j < 8; j++)
        {
            pr_cont("%3d", mon[j] & 0x7ff);
        }
        pr_cont("|");

        // clear peak tx regs
        for (j = 0; j < 8; j++)
        {
            SF2SW_RREG(unit, PAGE_FLOW_CTRL_XTN, REG_FC_QUE_PEAK_COUNT+j*2, &val, 4);
        }
        SF2SW_RREG(unit, PAGE_FLOW_CTRL_XTN, REG_FC_PORT_PEAK_RX_BUFFER, &val, 4);
        SF2SW_RREG(unit, PAGE_FLOW_CTRL_XTN,REG_FC_QUE_FINAL_CONG_STAT+i*2, &final, 4);
        SF2SW_RREG(unit, PAGE_FLOW_CTRL_XTN,REG_FC_QUE_CONG_STATUS+i*2, &congest, 4);
        SF2SW_RREG(unit, PAGE_FLOW_CTRL_XTN,REG_FC_QUE_TOTAL_CONG_STATUS+i*2, &total, 4);
        
        mdelay(500);
        // dump peak tx regs
        for (j = 0; j < 8; j++)
        {
            SF2SW_RREG(unit, PAGE_FLOW_CTRL_XTN, REG_FC_QUE_PEAK_COUNT+j*2, &val, 4);
            pr_cont("%3d", val & 0x7ff);
        }
        SF2SW_RREG(unit, PAGE_FLOW_CTRL_XTN, REG_FC_PORT_PEAK_RX_BUFFER, &val, 4);
        pr_cont(" %4d|", val & 0x7ff);

        SF2SW_RREG(unit, PAGE_FLOW_CTRL_XTN,REG_FC_QUE_FINAL_CONG_STAT+i*2, &final, 4);
        SF2SW_RREG(unit, PAGE_FLOW_CTRL_XTN,REG_FC_QUE_CONG_STATUS+i*2, &congest, 4);
        SF2SW_RREG(unit, PAGE_FLOW_CTRL_XTN,REG_FC_QUE_TOTAL_CONG_STATUS+i*2, &total, 4);
        for (j = 0; j < 8; j++)
        {
            char *lvl_str=".RPD";
            #define LVL_CHAR(v,q) lvl_str[(v>>q*2) & 3]
            pr_cont("%c%c%c ", LVL_CHAR(final,j), LVL_CHAR(congest,j), LVL_CHAR(total,j));
        }
        pr_cont("\n");
    }

    SF2SW_WREG(unit, PAGE_FLOW_CTRL_XTN, REG_FC_DIAG_CTRL, &diag_ctrl, 4);  // restore current diag control
}

int ioctl_extsw_cfg_acb(struct ethswctl_data *e)
{
    /* based on impl5:sf2_config_acb() */
    uint32_t val32, val;
    acb_q_params_t acb_conf_info;
    volatile uint32_t *sf2_acb_control    = (void *) (SF2_ACB_CONTROL_REG);
    volatile uint32_t *sf2_acb_xon_thresh = (void *) (sf2_acb_control + 1);
    volatile uint32_t *sf2_acb_que_config = (void *) (sf2_acb_control + 2);
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
    volatile uint32_t *sf2_acb_que_in_flight = (void *)(SF2_ACB_QUE0_PKTS_IN_FLIGHT);
#endif

    if (e->queue < 0 || e->queue > SF2_ACB_QUE_MAX) {
        printk("%s parameter error, queue 0x%x \n", 	__FUNCTION__, e->queue);
        return BCM_E_PARAM;
    }
    val   = *sf2_acb_xon_thresh;
    val32 = *(sf2_acb_que_config + e->queue);
    if (e->type == TYPE_GET) {
        switch (e->sw_ctrl_type) {
            case acb_en:
                acb_conf_info.acb_en =  *sf2_acb_control & SF2_ACB_EN;
                break;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
            case acb_eop_delay:
                acb_conf_info.eop_delay =  (*sf2_acb_control >> SF2_ACB_EOP_DELAY_S) & SF2_ACB_EOP_DELAY_M;
                break;
            case acb_flush:
                acb_conf_info.flush =  (*sf2_acb_control >> SF2_ACB_FLUSH_S) & SF2_ACB_FLUSH_M;
                break;
            case acb_algorithm:
#if !defined(CONFIG_BCM963178) && !defined(CONFIG_BCM96756)
                acb_conf_info.algorithm =  (*sf2_acb_control >> SF2_ACB_ALGORITHM_S) & SF2_ACB_ALGORITHM_M;
#else
                acb_conf_info.algorithm = 1;
#endif
                break;
#endif
            case acb_tot_xon_hyst:
                acb_conf_info.total_xon_hyst =  (val >> SF2_ACB_TOTAL_XON_BUFS_S)
                    & SF2_ACB_BUFS_THRESH_M;
                break;
            case acb_xon_hyst:
                acb_conf_info.xon_hyst =  (val >> SF2_ACB_XON_BUFS_S)
                                                        & SF2_ACB_BUFS_THRESH_M;
                break;
            case acb_q_pessimistic_mode:
                acb_conf_info.acb_queue_config.pessimistic_mode = (val32  >> SF2_ACB_QUE_PESSIMISTIC_S)
                                                        & SF2_ACB_QUE_PESSIMISTIC_M;
                break;
            case acb_q_total_xon_en:
                acb_conf_info.acb_queue_config.total_xon_en = (val32  >> SF2_ACB_QUE_TOTAL_XON_S)
                                                         & SF2_ACB_QUE_TOTAL_XON_M;
                break;
            case acb_q_xon_en:
                acb_conf_info.acb_queue_config.xon_en = (val32  >> SF2_ACB_QUE_XON_S)
                                                         & SF2_ACB_QUE_XON_M;
                break;
            case acb_q_total_xoff_en:
                acb_conf_info.acb_queue_config.total_xoff_en = (val32  >> SF2_ACB_QUE_TOTAL_XOFF_S)
                                                         & SF2_ACB_QUE_TOTAL_XOFF_M;
                break;
            case acb_q_pkt_len:
                acb_conf_info.acb_queue_config.pkt_len = (val32  >> SF2_ACB_QUE_PKT_LEN_S)
                                                         & SF2_ACB_QUE_PKT_LEN_M;
                break;
            case acb_q_tot_xoff_thresh:
                acb_conf_info.acb_queue_config.total_xoff_threshold = (val32  >> SF2_ACB_QUE_TOTOAL_XOFF_BUFS_S)
                                                         & SF2_ACB_BUFS_THRESH_M;
                break;
            case acb_q_xoff_thresh:
                acb_conf_info.acb_queue_config.xoff_threshold = (val32  >> SF2_ACB_QUE_XOFF_BUFS_S)
                                                         & SF2_ACB_BUFS_THRESH_M;
                break;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
            case acb_q_pkts_in_flight:
                acb_conf_info.pkts_in_flight = *(sf2_acb_que_in_flight + e->queue) & SF2_ACB_QUE_PKTS_IN_FLIGHT_M;
                break;
#endif
            case acb_parms_all:
                acb_conf_info.acb_en =  *sf2_acb_control & SF2_ACB_EN;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
                acb_conf_info.eop_delay =  (*sf2_acb_control >> SF2_ACB_EOP_DELAY_S) & SF2_ACB_EOP_DELAY_M;
                acb_conf_info.flush =  (*sf2_acb_control >> SF2_ACB_FLUSH_S) & SF2_ACB_FLUSH_M;
#if !defined(CONFIG_BCM963178) && !defined(CONFIG_BCM96756)
                acb_conf_info.algorithm =  (*sf2_acb_control >> SF2_ACB_ALGORITHM_S) & SF2_ACB_ALGORITHM_M;
#else
                acb_conf_info.algorithm = 1;
#endif
#endif
                acb_conf_info.total_xon_hyst =  (val >> SF2_ACB_TOTAL_XON_BUFS_S)
                                                        & SF2_ACB_BUFS_THRESH_M;
                acb_conf_info.xon_hyst =  (val >> SF2_ACB_XON_BUFS_S)
                                                        & SF2_ACB_BUFS_THRESH_M;
                acb_conf_info.acb_queue_config.pessimistic_mode = (val32  >> SF2_ACB_QUE_PESSIMISTIC_S)
                                                        & SF2_ACB_QUE_PESSIMISTIC_M;
                acb_conf_info.acb_queue_config.total_xon_en = (val32  >> SF2_ACB_QUE_TOTAL_XON_S)
                                                         & SF2_ACB_QUE_TOTAL_XON_M;
                acb_conf_info.acb_queue_config.xon_en = (val32  >> SF2_ACB_QUE_XON_S)
                                                         & SF2_ACB_QUE_XON_M;
                acb_conf_info.acb_queue_config.total_xoff_en = (val32  >> SF2_ACB_QUE_TOTAL_XOFF_S)
                                                         & SF2_ACB_QUE_TOTAL_XOFF_M;
                acb_conf_info.acb_queue_config.pkt_len = (val32  >> SF2_ACB_QUE_PKT_LEN_S)
                                                         & SF2_ACB_QUE_PKT_LEN_M;
                acb_conf_info.acb_queue_config.total_xoff_threshold = (val32  >> SF2_ACB_QUE_TOTOAL_XOFF_BUFS_S)
                                                         & SF2_ACB_BUFS_THRESH_M;
                acb_conf_info.acb_queue_config.xoff_threshold = (val32  >> SF2_ACB_QUE_XOFF_BUFS_S)
                                                         & SF2_ACB_BUFS_THRESH_M;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
                acb_conf_info.pkts_in_flight = *(sf2_acb_que_in_flight + e->queue) & SF2_ACB_QUE_PKTS_IN_FLIGHT_M;
#endif
                break;
            case 100:
                dump_sf2_fc_info(e->unit);
                break;
            default:
                printk("%s: Get op %#x Unsupported \n", __FUNCTION__, e->sw_ctrl_type);
                return BCM_E_PARAM;
                break;
        }
        if (copy_to_user (e->vptr, &acb_conf_info, sizeof(acb_q_params_t))) {
            return -EFAULT;
        }
    } else {  // SET
        switch (e->sw_ctrl_type) {
            case acb_en:
                if (e->val)
                    *sf2_acb_control |= SF2_ACB_EN;
                else
                    *sf2_acb_control &= ~SF2_ACB_EN;
                return 0;
                break;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
            case acb_eop_delay:
                *sf2_acb_control &= ~SF2_ACB_EN;
                *sf2_acb_control &= ~(SF2_ACB_EOP_DELAY_M << SF2_ACB_EOP_DELAY_S);
                *sf2_acb_control |= (e->val & SF2_ACB_EOP_DELAY_M) << SF2_ACB_EOP_DELAY_S;
                *sf2_acb_control |= SF2_ACB_EN;
                return 0;
            case acb_flush:
                *sf2_acb_control &= ~SF2_ACB_EN;
                *sf2_acb_control &= ~(SF2_ACB_FLUSH_M << SF2_ACB_FLUSH_S);
                *sf2_acb_control |= (e->val & SF2_ACB_FLUSH_M) << SF2_ACB_FLUSH_S;
                *sf2_acb_control |= SF2_ACB_EN;
                return 0;
            case acb_algorithm:
#if !defined(CONFIG_BCM963178) && !defined(CONFIG_BCM96756)
                _sf2_set_acb_algorithm(e->val);
#endif
                return 0;
#endif
            case acb_tot_xon_hyst:
                *sf2_acb_control &= ~SF2_ACB_EN;
                *sf2_acb_xon_thresh = val | (e->val & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_TOTAL_XON_BUFS_S;
                *sf2_acb_control |= SF2_ACB_EN;
                return 0;
                break;
            case acb_xon_hyst:
                *sf2_acb_control &= ~SF2_ACB_EN;
                *sf2_acb_xon_thresh = val | (e->val & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_XON_BUFS_S;
                *sf2_acb_control |= SF2_ACB_EN;
                return 0;
                break;
            case acb_q_pessimistic_mode:
                val32 &= ~(SF2_ACB_QUE_PESSIMISTIC_M << SF2_ACB_QUE_PESSIMISTIC_S);
                val32 |= (e->val & SF2_ACB_QUE_PESSIMISTIC_M) << SF2_ACB_QUE_PESSIMISTIC_S;
                break;
            case acb_q_total_xon_en:
                val32 &= ~(SF2_ACB_QUE_TOTAL_XON_M << SF2_ACB_QUE_TOTAL_XON_S);
                val32 |= (e->val & SF2_ACB_QUE_TOTAL_XON_M) << SF2_ACB_QUE_TOTAL_XON_S;
                break;
            case acb_q_xon_en:
                val32 &= ~(SF2_ACB_QUE_XON_M << SF2_ACB_QUE_XON_S);
                val32 |= (e->val & SF2_ACB_QUE_XON_M) << SF2_ACB_QUE_XON_S;
                break;
            case acb_q_total_xoff_en:
                val32 &= ~(SF2_ACB_QUE_TOTAL_XOFF_M << SF2_ACB_QUE_TOTAL_XOFF_S);
                val32 |= (e->val & SF2_ACB_QUE_TOTAL_XON_M) << SF2_ACB_QUE_TOTAL_XON_S;
                break;
            case acb_q_pkt_len:
                val32 |=  (e->val & SF2_ACB_QUE_PKT_LEN_M) << SF2_ACB_QUE_PKT_LEN_S;
                break;
            case acb_q_tot_xoff_thresh:
                val32 |=  (e->val & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_QUE_TOTOAL_XOFF_BUFS_S;
                break;
            case acb_q_xoff_thresh:
                val32 |=  (e->val & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_QUE_XOFF_BUFS_S;
                break;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
            case acb_q_pkts_in_flight:
                printk("Warning: This register should only be set by HW, but set here.\n");
                *sf2_acb_control &= ~SF2_ACB_EN;
                *(sf2_acb_que_in_flight + e->queue) = e->val & SF2_ACB_QUE_PKTS_IN_FLIGHT_M;
                *sf2_acb_control |= SF2_ACB_EN;
                return BCM_E_PARAM;
#endif
            default:
                printk("%s: Set op %#x Unsupported \n", __FUNCTION__, e->sw_ctrl_type);
                return BCM_E_PARAM;
                break;
        }
        *sf2_acb_control &= ~SF2_ACB_EN;
        *(sf2_acb_que_config + e->queue) = val32;
        *sf2_acb_control |= SF2_ACB_EN;
        return 0;
    }// Set
    return 0;
}
#endif //!SF2_EXTERNAL

// ----------- SIOCETHSWCTLOPS ETHSWCOSPRIORITYMETHOD functions ---
/* This function just serves Star Fighter. Legacy External switch
 * goes with Robo.
 *
 * Get/Set StarFighter cos priority method
 *** Input params
 * e->type  GET/SET
 * e->pkt_type_mask - ipv4/ipv6:802.1p:static mac destination or port Id based
 * e->val - ingress classifier TC src selection -- DSCP, vlan pri,
 *        -  MAC addr, PORT based (default vlan tag)
 * e->port  per port
 *** Output params
 * Returns 0 for Success, Negative value for failure.
 */
static int _qos_dscp_is_enabled(int unit, int port)
{
    u16 val16;
    SF2SW_RREG(unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&val16, 2);
    return (val16 >> port) & 1;
}

static int _qos_8021p_is_enabled(int unit, int port)
{
    u16 val16;
    SF2SW_RREG(unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&val16, 2);
    return (val16 >> port) & 1;
}

static void _enable_dscp_qos(int unit, int port, int enable)
{
    u16 val16;
    SF2SW_RREG(unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&val16, 2);
    val16 &= ~(1 << port);
    val16 |= enable << port;
    SF2SW_WREG(unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&val16, 2);
}

static void _enable_8021p_qos(int unit, int port, int enable)
{
    u16 val16;
    SF2SW_RREG(unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&val16, 2);
    val16 &= ~(1 << port);
    val16 |= enable << port;
    SF2SW_WREG(unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&val16, 2);
}

/* Note: Method values are UAPI definition */
static int _isQoSMethodEnabled(int unit, int port, int method)
{
    switch(method)
    {
        case PORT_QOS:
        case MAC_QOS:
            return 1;
        case IEEE8021P_QOS:
            return _qos_8021p_is_enabled(unit, port);
        case DIFFSERV_QOS:
            return _qos_dscp_is_enabled(unit, port);
    }
    return 0;
}

/* Note: Method values are UAPI definition */
static void _enableQosMethod(int unit, int port, int method, int enable)
{
    switch(method)
    {
        case MAC_QOS:
        case PORT_QOS:
            return;
        case IEEE8021P_QOS:
            if (enable)
            {
                _enable_8021p_qos(unit, port, 1); // Enable PCP for the port
            }
            else
            {
                _enable_8021p_qos(unit, port, 0);
            }
            return;
        case DIFFSERV_QOS:
            if (enable)
            {
                _enable_dscp_qos(unit, port, 1); // Enable DSCP for the port
            }
            else
            {
                _enable_dscp_qos(unit, port, 0);
            }
            return;
    }
}

#define QOS_METHOD_CNVT_UAPI_AND_REG(regQoS)  (~(regQoS) & SF2_QOS_TC_SRC_SEL_VAL_MASK)
#define QOS_METHODS_CNVT_UAPI_AND_REG(regQoSPorts)  (~(regQoSPorts) & 0xffff)
int ioctl_extsw_cos_priority_method_cfg(struct ethswctl_data *e)
{
    // based on impl5:sf2_cos_priority_method_config()
    uint16_t val16, reg_addr, pkt_type_mask, tc_sel_src ;
    uint32_t val32;
    int i, enable_qos;

    reg_addr = SF2_REG_PORTN_TC_SELECT_TABLE + e->port * 2;
#if defined(CONFIG_BCM96756)
    if (e->unit) reg_addr -= QOS_REG_SHIFT;   // external switch 
#endif
    down(&sf2_sw->s.conf_sem);

    //enet_dbg("%s port %d pkt_type 0x%x Given method: %02d \n ",__FUNCTION__,
    //        e->port, e->pkt_type_mask, e->val);
    if (e->port < 0 || e->port > SF2_IMP0_PORT ||  !(chip_arch_all_portmap[e->unit] & (1<<e->port))) {
        enet_dbg("parameter error, port %d \n", e->port);
        return -BCM_E_PARAM;
    }
    pkt_type_mask = e->pkt_type_mask;
    if (e->type == TYPE_GET) {
        SF2SW_RREG(e->unit, PAGE_QOS, reg_addr, &val16, 2);
        if (e->pkt_type_mask == SF2_QOS_TC_SRC_SEL_PKT_TYPE_ALL)
        {
            val32 = QOS_METHODS_CNVT_UAPI_AND_REG(val16);
            for (i = 0; i < e->pkt_type_mask; i++)
            {
                tc_sel_src = (val16 >> (i * 2)) & SF2_QOS_TC_SRC_SEL_VAL_MASK;
                enable_qos = _isQoSMethodEnabled(e->unit, e->port, QOS_METHOD_CNVT_UAPI_AND_REG(tc_sel_src));
                val32 |= !enable_qos << (16+i);
            }
        } else {
            pkt_type_mask &=  SF2_QOS_TC_SRC_SEL_PKT_TYPE_MASK;
            val16 =  (val16 >> (pkt_type_mask * 2)) & SF2_QOS_TC_SRC_SEL_VAL_MASK;
            val16 = QOS_METHOD_CNVT_UAPI_AND_REG(val16);
            enable_qos = _isQoSMethodEnabled(e->unit, e->port, val16);
            val32 = (!enable_qos << 16) | val16;
        }
        // bits programmed in TC Select Table registers and software notion are bit inversed.
        e->ret_val = val32;
    } else { // TYPE_SET

        /* when pkt_type_mask is NOT SF2_QOS_TC_SRC_SEL_PKT_TYPE_ALL,
            tc_sel_src b0-1 2 bit show of TC.  b16-17 2bits contains disable bit.  */
        if (e->pkt_type_mask != SF2_QOS_TC_SRC_SEL_PKT_TYPE_ALL)
        {
            tc_sel_src = QOS_METHOD_CNVT_UAPI_AND_REG(e->val);
            SF2SW_RREG(e->unit, PAGE_QOS, reg_addr, &val16, 2);
            pkt_type_mask = e->pkt_type_mask & SF2_QOS_TC_SRC_SEL_PKT_TYPE_MASK;
            val16 &= ~(SF2_QOS_TC_SRC_SEL_VAL_MASK << (pkt_type_mask * 2));
            val16 |=  (tc_sel_src & SF2_QOS_TC_SRC_SEL_VAL_MASK ) << (pkt_type_mask * 2);
            enable_qos = !((e->val >> 16) & 1);
            //enet_dbg("%s: Write to: len %d page 0x%x reg 0x%x val 0x%x\n",
            //        __FUNCTION__, 2, PAGE_QOS, reg_addr, val16);
            _enableQosMethod(e->unit, e->port, e->val & SF2_QOS_TC_SRC_SEL_VAL_MASK, enable_qos);
        }
        else    /* SF2_QOS_TC_SRC_SEL_PKT_TYPE_ALL */
        /* when pkt_type_mask is SF2_QOS_TC_SRC_SEL_PKT_TYPE_ALL,
            tc_sel_src's lower 16 bits contains TC selections for all 8 packet types.
            higher 8 bits contains disable bit for corresponding methods in lower 16bits.  */
        {
            val16 = QOS_METHODS_CNVT_UAPI_AND_REG(e->val);
            for (i = 0; i < e->pkt_type_mask; i++)
            {
                tc_sel_src = ((e->val) >> (i * 2)) & SF2_QOS_TC_SRC_SEL_VAL_MASK;
                enable_qos = !((e->val >> (16 + i)) & 1);
                _enableQosMethod(e->unit, e->port, tc_sel_src, enable_qos);
            }
        }
        SF2SW_WREG(e->unit, PAGE_QOS, reg_addr, &val16, 2);
    }

    up(&sf2_sw->s.conf_sem);
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWCOSPCPPRIOMAP functions ---
/*
 * Get/Set PCP to TC mapping Tabe entry given 802.1p priotity (PCP)
 * and mapped priority
 *** Input params
 * e->type  GET/SET
 * e->val -  pcp
 * e->priority - mapped TC value, case of SET
 *** Output params
 * e->priority - mapped TC value, case of GET
 * Returns 0 for Success, Negative value for failure.
 */
int ioctl_extsw_pcp_to_priority_mapping(struct ethswctl_data *e)
{
    // based on impl5:bcmeapi_ioctl_extsw_pcp_to_priority_mapping()
    uint32_t val32;
    uint16_t reg_addr;
    int adjust = 0;

    enet_dbg("Given pcp: %02d \n ", e->val);
    if (e->val > MAX_PRIORITY_VALUE) {
        enet_err("Invalid PCP Value %02d \n", e->val);
        return BCM_E_ERROR;
    }

#if defined(CONFIG_BCM96756)
    if (e->unit) adjust = -QOS_REG_SHIFT;   // external switch 
#endif

    if (e->port < 0 || e->port > SF2_IMP0_PORT ||  !(chip_arch_all_portmap[e->unit] & (1<<e->port))) {
        enet_err("Invalid port number %02d \n", e->port);
        return BCM_E_ERROR;
    }
    reg_addr = e->port == SF2_IMP0_PORT? SF2_REG_QOS_PCP_IMP0 + adjust:
               e->port == SF2_P7? SF2_REG_QOS_PCP_P7 + adjust:
                          REG_QOS_8021P_PRIO_MAP + e->port * QOS_PCP_MAP_REG_SZ;

    down(&sf2_sw->s.conf_sem);

    if (e->type == TYPE_GET) {
        SF2SW_RREG(e->unit, PAGE_QOS, reg_addr, (void *)&val32, 4);
        e->priority = (val32 >> (e->val * QOS_TC_S)) & QOS_TC_M;
        enet_dbg("pcp %d is mapped to priority: %d \n ", e->val, e->priority);
    } else {
        enet_dbg("Given pcp: %02d priority: %02d \n ", e->val, e->priority);
        if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
            enet_err("Invalid Priority \n");
            up(&sf2_sw->s.conf_sem);
            return BCM_E_ERROR;
        }
        SF2SW_RREG(e->unit, PAGE_QOS, reg_addr, (void *)&val32, 4);
        val32 &= ~(QOS_TC_M << (e->val * QOS_TC_S));
        val32 |= (e->priority & QOS_TC_M) << (e->val * QOS_TC_S);
        SF2SW_WREG(e->unit, PAGE_QOS, reg_addr, (void *)&val32, 4);
    }

    up(&sf2_sw->s.conf_sem);
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWCOSPIDPRIOMAP functions ---
/*
 * Get/Set PID to TC mapping Table entry given ingress port
 * and mapped priority
 *** Input params
 * e->type  GET/SET
 * e->priority - mapped TC value, case of SET
 *** Output params
 * e->priority - mapped TC value, case of GET
 * Returns 0 for Success, Negative value for failure.
 */
int ioctl_extsw_pid_to_priority_mapping(struct ethswctl_data *e)
{
    // based on impl5:sf2_pid_to_priority_mapping()
    uint32_t val32;
    int reg_port_id_prio_map = SF2_REG_PORT_ID_PRIO_MAP;

    enet_dbg("Given uint %02d port %02d \n ", e->unit, e->port);

    if (e->port < 0 || e->port > SF2_IMP0_PORT || !(chip_arch_all_portmap[e->unit] & (1<<e->port))) {
        enet_err("Invalid port number %02d \n", e->port);
        return BCM_E_ERROR;
    }
#if defined(CONFIG_BCM96756)
    if (e->unit) reg_port_id_prio_map -= QOS_REG_SHIFT;   // external switch 
#endif
    down(&sf2_sw->s.conf_sem);

    if (e->type == TYPE_GET) {
        SF2SW_RREG(e->unit, PAGE_QOS, reg_port_id_prio_map, (void *)&val32, 4);
        e->priority = (val32 >> (e->port * QOS_TC_S)) & QOS_TC_M;
        enet_dbg("port %d is mapped to priority: %d \n ", e->port, e->priority);
    } else {
        enet_dbg("Given port: %02d priority: %02d \n ", e->port, e->priority);
        SF2SW_RREG(e->unit, PAGE_QOS, reg_port_id_prio_map, (void *)&val32, 4);
        val32 &= ~(QOS_TC_M << (e->port * QOS_TC_S));
        val32 |= (e->priority & QOS_TC_M) << (e->port * QOS_TC_S);
        SF2SW_WREG(e->unit, PAGE_QOS, reg_port_id_prio_map, (void *)&val32, 4);
    }

    up(&sf2_sw->s.conf_sem);
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWCOSDSCPPRIOMAP functions ---
/*
 * Get/Set DSCP to TC mapping Tabe entry given dscp value and priority
 *** Input params
 * e->type  GET/SET
 * e->val -  dscp
 * e->priority - mapped TC value, case of SET
 *** Output params
 * e->priority - mapped TC value, case of GET
 * Returns 0 for Success, Negative value for failure.
 */
int ioctl_extsw_dscp_to_priority_mapping(struct ethswctl_data *e)
{
    // based on impl5:bcmeapi_ioctl_extsw_dscp_to_priority_mapping()
    uint64 val64 = 0;
    uint32_t mapnum;
    int dscplsbs;
    int reg_qos_dscp_prio_map0lo = REG_QOS_DSCP_PRIO_MAP0LO;

    enet_dbg("Given dscp: %02d \n ", e->val);
    if (e->val > QOS_DSCP_M) {
        enet_err("Invalid DSCP Value \n");
        return BCM_E_ERROR;
    }

#if defined(CONFIG_BCM96756)
    if (e->unit) reg_qos_dscp_prio_map0lo -= QOS_REG_SHIFT;   // external switch 
#endif
    down(&sf2_sw->s.conf_sem);

    dscplsbs = e->val & QOS_DSCP_MAP_LSBITS_M;
    mapnum = (e->val >> QOS_DSCP_MAP_S) & QOS_DSCP_MAP_M;

    if (e->type == TYPE_GET) {
        SF2SW_RREG(e->unit, PAGE_QOS, reg_qos_dscp_prio_map0lo + mapnum * QOS_DSCP_MAP_REG_SZ,
                                 (void *)&val64, QOS_DSCP_MAP_REG_SZ | DATA_TYPE_HOST_ENDIAN);
        e->priority = (val64 >> (dscplsbs * QOS_TC_S)) & QOS_TC_M;
        enet_dbg("dscp %d is mapped to priority: %d \n ", e->val, e->priority);
    } else {
        enet_dbg("Given priority: %02d \n ", e->priority);
        if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
            enet_err("Invalid Priority \n");
            up(&sf2_sw->s.conf_sem);
            return BCM_E_ERROR;
        }
        // LE assumptions below, TODO
        SF2SW_RREG(e->unit, PAGE_QOS, reg_qos_dscp_prio_map0lo + mapnum * QOS_DSCP_MAP_REG_SZ,
                                     (void *)&val64, QOS_DSCP_MAP_REG_SZ | DATA_TYPE_HOST_ENDIAN);
        val64 &= ~(((uint64)(QOS_TC_M)) << (dscplsbs * QOS_TC_S));
        val64 |= ((uint64)(e->priority & QOS_TC_M)) << (dscplsbs * QOS_TC_S);
        enet_dbg(" @ addr %#x val64 to write = 0x%llx \n",
                                (reg_qos_dscp_prio_map0lo + mapnum * QOS_DSCP_MAP_REG_SZ),
                                (uint64) val64);

        SF2SW_WREG(e->unit, PAGE_QOS, reg_qos_dscp_prio_map0lo + mapnum * QOS_DSCP_MAP_REG_SZ,
                                            (void *)&val64, QOS_DSCP_MAP_REG_SZ | DATA_TYPE_HOST_ENDIAN);
    }

    up(&sf2_sw->s.conf_sem);
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWPORTTXRATE functions ---
int shaper_byte_unit[] = {8, 16, 32, 64, 128, 256, 512}; // *kbps
int shaper_pkt_unit[] = {125, 250, 500, 1000, 2000, 4000, 8000}; // */8 pps 
#define DIVIDE_RU(x,y)  ((x)+(y)-1)/(y)
/*
 * Get/Set StarFighter Egress shaper control
 *** Input params
 * e->port  egress port that is configured
 * e->unit  switch unit
 * e->type  GET/SET
 * e->limit egress rate control limit in
 *          64 kbps(Byte mode) 125 pps packet mode
 * e->burst_size egress burst in 64 Byte units(Byte mode)
 *          or in packets (packet mode)
 * e->queue egress queue if specified (per queue config)
 *          or -1 is not specified
 *** Output params
 * e->vptr has result copied for GET
 * Returns 0 for Success, Negative value for failure.
 */
int ioctl_extsw_port_erc_config(struct ethswctl_data *e)
{
    // based on impl5:sf2_port_erc_config()
    uint16_t val16, page, reg;
    uint32_t val32;
    uint32_t pkt_flag;
    unsigned char q_shaper = e->queue >= 0;
    uint32_t tick = 3; // 128Khz default

    // get current shaper tick
#if defined(CONFIG_BCM96756)
    if (e->unit == 0) {
        SF2SW_RREG(e->unit, PAGE_AVB_TICK_CTRL, SF2_REG_AVB_GLOBAL_TICK_CTRL, &tick, 4);
        SF2SW_RREG(e->unit, PAGE_AVB_TICK_CTRL, SF2_REG_AVB_TICK_CTRL_P0_OVERRIDE+e->port, &val32, 4);
        if (val32 < 7 /* use global */) tick = val32;
    }
#endif

    if (e->type == TYPE_SET) {

        /* find queue or port page*/
        page = q_shaper? PAGE_Q0_EGRESS_SHAPER + e->queue:
                                    PAGE_PORT_EGRESS_SHAPER;
        /* find the shaper mode */
        pkt_flag =  (e->sub_type == SHAPER_PACKET_MODE) ? SHAPER_PACKET_MODE : 0;

        /* configure shaper rate limit; limit = 0 means disable shaper */
        val32 = 0; /* reset rate limiting by default */
        if (e->limit)
        {
            if (pkt_flag == SHAPER_PACKET_MODE)
            {
                val32 = DIVIDE_RU(e->limit*8,shaper_pkt_unit[tick]); /* shaper rate config in 125pps units */
            }
            else {
                val32 = DIVIDE_RU(e->limit,shaper_byte_unit[tick]); /* shaper rate config in 64Kbps units */
            }
            if (!val32) {
                val32 = 1; /* At least 64Kbps */
            }
            val32 = val32 & SHAPER_RATE_BURST_VAL_MASK;
        }
        reg  =   pkt_flag == SHAPER_PACKET_MODE? SF2_REG_PN_SHAPER_RATE_PKT:
                                                  SF2_REG_PN_SHAPER_RATE_BYTE;
        SF2SW_WREG(e->unit, page, reg + e->port * 4, &val32, 4);

        /* configure shaper burst size */
        val32 = 0; /* reset burst size by default */
        if (e->limit) { /* Only set burst size if shaper is getting enabled */
            if (pkt_flag == SHAPER_PACKET_MODE) {
                val32 = e->burst_size; /* shaper burst config in 1 packet units */
            }
            else {
                if (!e->burst_size)
                    val32 = DIVIDE_RU(e->limit,shaper_byte_unit[tick]*2*8*64);  // formula provided by Alex ROUNDUP((rate Mbps * 1000/ N KHz)/8/64)
                else
                    val32 = DIVIDE_RU(e->burst_size /* Kbits */ * 1000, 8*64); /* shaper burst config in 64Byte units */
            }
            if (!val32) {
                val32 = 1;
            }
            val32 = val32 & SHAPER_RATE_BURST_VAL_MASK;
        }
        reg  =   pkt_flag == SHAPER_PACKET_MODE? SF2_REG_PN_SHAPER_BURST_SZ_PKT:
                                                  SF2_REG_PN_SHAPER_BURST_SZ_BYTE;
        SF2SW_WREG(e->unit, page, reg + e->port * 4, &val32, 4);

        /* enable shaper for byte mode or pkt mode as the case may be. */
        SF2SW_RREG(e->unit, page, SF2_REG_SHAPER_ENB_PKT_BASED, &val16, 2);
        val16 &= ~(1 << e->port);
        val16 |= pkt_flag == SHAPER_PACKET_MODE? (1 << e->port): 0;
        SF2SW_WREG(e->unit, page, SF2_REG_SHAPER_ENB_PKT_BASED, &val16, 2);

        /* Enable/disable shaper */
        SF2SW_RREG(e->unit, page, SF2_REG_SHAPER_ENB, &val16, 2);
        val16 &= ~(1 << e->port); /* Disable Shaper */
        val16 |= e->limit? (1 << e->port): 0; /* Enable Shaper, if needed */
        SF2SW_WREG(e->unit, page, SF2_REG_SHAPER_ENB, &val16, 2);

        return 0;
    } else {
        /* Egress shaper stats*/
        egress_shaper_stats_t stats;

        page = q_shaper? PAGE_Q0_EGRESS_SHAPER + e->queue:
                                    PAGE_PORT_EGRESS_SHAPER;
        SF2SW_RREG(e->unit, page, SF2_REG_SHAPER_ENB_PKT_BASED, &val16, 2);
        pkt_flag = (val16 & (1 << e->port))? 1: 0;
        stats.egress_shaper_flags = 0;
        stats.egress_shaper_flags |= pkt_flag? SHAPER_RATE_PACKET_MODE: 0;

        reg = pkt_flag? SF2_REG_PN_SHAPER_RATE_PKT: SF2_REG_PN_SHAPER_RATE_BYTE;
        SF2SW_RREG(e->unit, page, reg + e->port * 4, &val32, 4);
        stats.egress_rate_cfg = val32 & SHAPER_RATE_BURST_VAL_MASK;

        reg = pkt_flag? SF2_REG_PN_SHAPER_BURST_SZ_PKT: SF2_REG_PN_SHAPER_BURST_SZ_BYTE;
        SF2SW_RREG(e->unit, page, reg + e->port * 4, &val32, 4);
        stats.egress_burst_sz_cfg = val32 & SHAPER_RATE_BURST_VAL_MASK;

        reg = SF2_REG_PN_SHAPER_STAT;
        SF2SW_RREG(e->unit, page, reg + e->port * 4, &val32, 4);
        stats.egress_cur_tokens = val32 & SHAPER_STAT_COUNT_MASK;
        stats.egress_shaper_flags |= val32 & SHAPER_STAT_OVF_MASK? SHAPER_OVF_FLAG: 0;
        stats.egress_shaper_flags |= val32 & SHAPER_STAT_INPF_MASK? SHAPER_INPF_FLAG: 0;

        SF2SW_RREG(e->unit, page, SF2_REG_SHAPER_ENB, &val16, 2);
        stats.egress_shaper_flags |= (val16 & (1 << e->port))? SHAPER_ENABLE: 0;

        SF2SW_RREG(e->unit, page, SF2_REG_SHAPER_BLK_CTRL_ENB, &val16, 2);
        stats.egress_shaper_flags |= (val16 & (1 << e->port))? SHAPER_BLOCKING_MODE: 0;

        // applies only for port shaper
        if (!q_shaper) {
            SF2SW_RREG(e->unit, page, SF2_REG_SHAPER_IFG_BYTES, &val16, 2);
            stats.egress_shaper_flags |= (val16 & (1 << e->port))? SHAPER_INCLUDE_IFG: 0;
        }

        SF2SW_RREG(e->unit, page, SF2_REG_SHAPER_ENB_AVB, &val16, 2);
        stats.egress_shaper_flags |= (val16 & (1 << e->port))? SHAPER_AVB_MODE: 0;

        /* Convert the return values based on mode */
        if (pkt_flag)
        {
            stats.egress_rate_cfg = stats.egress_rate_cfg * shaper_pkt_unit[tick] / 8; /* Shaper rate in 125pps unit */
            /* stats.egress_burst_sz_cfg  - burst unit in packets */
        }
        else {
            stats.egress_rate_cfg *= shaper_byte_unit[tick]; /* Shaper rate in 64Kbps unit */
            stats.egress_burst_sz_cfg = (stats.egress_burst_sz_cfg*8*64)/1000; /* Shaper burst is in 64Byte unit - convert into kbits */
        }
        if (e->vptr) {
            if (copy_to_user (e->vptr, &stats, sizeof(egress_shaper_stats_t))) {
                return -EFAULT;
            }
        } else {
            // Just support Legacy API
            e->limit = stats.egress_rate_cfg;
            e->burst_size =  stats.egress_burst_sz_cfg;
        }
    }
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWPORTSHAPERCFG functions ---
inline static void _extsw_reg16_bit_ops(int unit, uint16 page, uint16 reg, int bit, int on)
{
    uint16 val16;

    SF2SW_RREG(unit, page, reg, &val16, 2);
    val16 &= ~(1 << bit);
    val16 |= on << bit;
    SF2SW_WREG(unit, page, reg, &val16, 2);
}


/*
 * As part of setting StarFighter Egress shaper
 * configuration, turn on/off various shaper modes.
 *** Input params
 * e->port  egress port that is configured
 * e->unit  switch unit
 * e->type  SET
 * e->queue egress queue if specified (per queue config)
 *          or -1 is not specified
 * e->sub_type - Or'ed Flags
 * e->val = 1 | 0  for On or Off
 *  Output params None
 * Returns 0 for Success, Negative value for failure.
 */
int ioctl_extsw_port_shaper_config(struct ethswctl_data *e)
{
    // based on impl5:sf2_port_shaper_config()
    uint16 page, reg;
    unsigned char q_shaper;

    if (e->type == TYPE_SET) {
        /* confiure requested shaper parameters.
         * Notice: each q has its separate page.
         */
        q_shaper = e->queue >= 0;
        page = q_shaper? PAGE_Q0_EGRESS_SHAPER + e->queue:
                                    PAGE_PORT_EGRESS_SHAPER;
        if (e->sub_type & SHAPER_ENABLE) {
            reg = SF2_REG_SHAPER_ENB;
            _extsw_reg16_bit_ops(e->unit, page, reg, e->port, (e->val & SHAPER_ENABLE)?1:0);
        }
        if (e->sub_type & SHAPER_RATE_PACKET_MODE) {
            reg = SF2_REG_SHAPER_ENB_PKT_BASED;
            _extsw_reg16_bit_ops(e->unit, page, reg, e->port, (e->val & SHAPER_RATE_PACKET_MODE)?1:0);
        }
        if (e->sub_type & SHAPER_BLOCKING_MODE) {
            reg = SF2_REG_SHAPER_BLK_CTRL_ENB;
            _extsw_reg16_bit_ops(e->unit, page, reg, e->port, (e->val & SHAPER_BLOCKING_MODE)?1:0);
        }
        if (e->sub_type & SHAPER_INCLUDE_IFG) {
            // applies only for port shaper
            if (!q_shaper) {
                reg = SF2_REG_SHAPER_IFG_BYTES;
                _extsw_reg16_bit_ops(e->unit, page, reg, e->port, (e->val & SHAPER_INCLUDE_IFG)?1:0);
            }
        }
        if (e->sub_type & SHAPER_AVB_MODE) {
            reg = SF2_REG_SHAPER_ENB_AVB;
            _extsw_reg16_bit_ops(e->unit, page, reg, e->port, (e->val & SHAPER_AVB_MODE)?1:0);
        }
    }

    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWMULTIPORT functions ---
int ioctl_extsw_set_multiport_address(int unit, uint8_t *addr)
{
    // based on impl5:bcmsw_set_multiport_address_ext()
    int i;
    uint32 v32;
    uint16 v16;
    uint8 v64[8];
    uint8 cur64[8];

    *(uint16*)(&v64[0]) = 0;
    memcpy(&v64[2], addr, 6);
    /* check if address is set already */
    for ( i = 0; i < MULTIPORT_CTRL_COUNT; i++ )
    {
       SF2SW_RREG(unit, PAGE_ARLCTRL, (REG_MULTIPORT_ADDR1_LO + (i * 0x10)), (uint8 *)&cur64, sizeof(cur64)|DATA_TYPE_VID_MAC);
       if ( 0 == memcmp(&v64[0], &cur64[0], 8) )
       {
           return 0;
       }
    }

    /* add new entry */
    for ( i = 0; i < MULTIPORT_CTRL_COUNT; i++ )
    {
        SF2SW_RREG(unit, PAGE_ARLCTRL, REG_MULTIPORT_CTRL, (uint8 *)&v16, 2);
        if ( 0 == (v16 & (MULTIPORT_CTRL_EN_M << (i << 1))))
        {
            v16 |= (1<<MULTIPORT_CTRL_DA_HIT_EN) | (MULTIPORT_CTRL_ADDR_CMP << (i << 1));
            SF2SW_WREG(unit, PAGE_ARLCTRL, REG_MULTIPORT_CTRL, (uint8 *)&v16, 2);
            *(uint16*)(&v64[0]) = 0;
            memcpy(&v64[2], addr, 6);
            SF2SW_WREG(unit, PAGE_ARLCTRL, (REG_MULTIPORT_ADDR1_LO + (i * 0x10)), (uint8 *)&v64, sizeof(v64)|DATA_TYPE_VID_MAC);
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
            v32 = imp_pbmap[1];
#else
            v32 = 1 << platform_imp_port(unit);
#endif
            SF2SW_WREG(unit, PAGE_ARLCTRL, (REG_MULTIPORT_VECTOR1 + (i * 0x10)), (uint8 *)&v32, sizeof(v32));

            /* Set multiport VLAN control based on U/V_FWD_MAP;
               This is required so that VLAN tagged frames matching Multiport Address are forwarded according to V/U forwarding map */
            SF2SW_RREG(unit, PAGE_8021Q_VLAN, REG_VLAN_MULTI_PORT_ADDR_CTL, &v16, sizeof(v16));
            v16 |=  (EN_MPORT_V_FWD_MAP | EN_MPORT_U_FWD_MAP) << (i*EN_MPORT_V_U_FWD_MAP_S) ;
            SF2SW_WREG(unit, PAGE_8021Q_VLAN, REG_VLAN_MULTI_PORT_ADDR_CTL, &v16, sizeof(v16));

            return 0;
        }
    }
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWDEEPGREENMODE functions ---
#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)

static int tr_imp_port_down(enetx_port_t *self, void *_ctx)
{
    if (self->p.mac && self->p.mac->mac_id == IMP_PORT_ID && self->dev != NULL)
    {
        netif_carrier_off(self->dev);
        printk((KERN_CRIT "%s (%s switch port: %d) (Logical Port: %d) Virtual link DOWN\n"),
               self->dev->name, PORT_ON_ROOT_SW(self)?"Int":"Ext", self->p.mac->mac_id, 
               PHYSICAL_PORT_TO_LOGICAL_PORT(self->p.mac->mac_id, PORT_ON_ROOT_SW(self)?0:1));
        return 1;
    }
    return 0;
}
static int tr_imp_port_up(enetx_port_t *self, void *_ctx)
{
    if (self->p.mac && self->p.mac->mac_id == IMP_PORT_ID && self->dev != NULL)
    {
        netif_carrier_on(self->dev);
        printk((KERN_CRIT "%s (%s switch port: %d) (Logical Port: %d) Virtual link UP\n"),
               self->dev->name, PORT_ON_ROOT_SW(self)?"Int":"Ext", self->p.mac->mac_id,
               PHYSICAL_PORT_TO_LOGICAL_PORT(self->p.mac->mac_id, PORT_ON_ROOT_SW(self)?0:1));
        return 1;
    }
    return 0;
}
static void _ethsw_deep_green_mode_activate(void)
{
    // based on impl5:ethsw_deep_green_mode_activate()
    uint32 reg_val32;
    int unit = IS_ROOT_SW(sf2_sw)?0:1;

    printk("===> Activate Deep Green Mode\n");

    port_traverse_ports(sf2_sw, tr_imp_port_down, PORT_CLASS_PORT, NULL);

    /* Disable IMP port */
    _imp_ports_op(unit, IMP_PORTS_DISABLE);

    /* Disable all ports' MAC TX/RX clocks (IMPORTANT: prevent all traffic into Switch while its clock is lowered) */
    SF2SW_RREG(unit, PAGE_CONTROL, REG_LOW_POWER_EXP1, &reg_val32, 4);
    reg_low_power_exp1 = reg_val32;    //Store register's value so that we can restore this value when we disable Deep Green Mode
    reg_val32 |= REG_LOW_POWER_EXP1_SLEEP_MACCLK_PORT_MASK;
    SF2SW_WREG(unit, PAGE_CONTROL, REG_LOW_POWER_EXP1, &reg_val32, 4);

    platform_set_clock_slow(unit);
}

static void _ethsw_deep_green_mode_deactivate(void)
{
    // based on impl5:ethsw_deep_green_mode_deactivate()
    uint32 reg_val32;
    int unit = IS_ROOT_SW(sf2_sw)?0:1;

    printk("<=== Deactivate Deep Green Mode\n");

    platform_set_clock_normal(unit);

    /* Enable IMP port */
    _imp_ports_op(unit, IMP_PORTS_ENABLE);

    /* Set IMP port to link up */
    SF2SW_RREG(unit, PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(IMP_PORT_ID), &reg_val32, 4);
    reg_val32 |= REG_CONTROL_MPSO_LINKPASS;
    SF2SW_WREG(unit, PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(IMP_PORT_ID), &reg_val32, 4);

    /* Re-enable in-use ports' MAC TX/RX clocks (leave unused port's MAC TX/RX clock disabled) */
    reg_val32 = reg_low_power_exp1;    //Restore register's previous value from before we enabled Deep Green Mode
    SF2SW_WREG(unit, PAGE_CONTROL, REG_LOW_POWER_EXP1, &reg_val32, 4);

    port_traverse_ports(sf2_sw, tr_imp_port_up, PORT_CLASS_PORT, NULL);
}

static int tr_phy_link_up(enetx_port_t *port, void *_ctx)
{
    int *any_link_up = (int *)_ctx;
    phy_dev_t *phy = get_active_phy(port->p.phy);

    // port is outward facing port and is linked up
    if (port->dev && phy && phy->link)
    {
        *any_link_up = 1;    // found one port with link up
        return 1;           // stop scanning
    }
    return 0;
}

void port_sf2_deep_green_mode_handler(void)
{
    // based on impl5:ethsw_deep_green_mode_handler()
    int any_link_up = 0;
    
    // check if any port is linked up?
    port_traverse_ports(root_sw, tr_phy_link_up, PORT_CLASS_PORT, &any_link_up);

    /* (special case) If user uses WebGUI to disable Deep Green Mode feature then deactivate Deep Green Mode if necessary and exit function */
    if ( !deep_green_mode_enabled )  {
        //printk("Deep Green Mode feature is disabled in WebGUI.  Do nothing.\n");
        if (deep_green_mode_activated) {
            deep_green_mode_activated = 0;
            //printk("Deep Green Mode was activated.  Deactivating Deep Green Mode now...\n");
            _ethsw_deep_green_mode_deactivate();
        }
        return;
    }

    /* Only activate Deep Green Mode if all ports are linked down and Deep Green Mode wasn't already enabled */
    if ( (!any_link_up) && (!deep_green_mode_activated) ) {
        deep_green_mode_activated = 1;
        _ethsw_deep_green_mode_activate();
    /* Only deactivate Deep Green Mode if some ports are linked up and Deep Green Mode is currently enabled */
    } else if ( any_link_up && (deep_green_mode_activated) )  {
        deep_green_mode_activated = 0;
        _ethsw_deep_green_mode_deactivate();
    }
}

int ioctl_pwrmngt_get_deepgreenmode(int mode)
{
    // based on impl5:BcmPwrMngtGetDeepGreenMode()
    if (!mode) {
        return (deep_green_mode_enabled);
    } else {
        return (deep_green_mode_activated);
    }
}

int ioctl_pwrmngt_set_deepgreenmode(int enable)
{
    // based on impl5:BcmPwrMngtSetDeepGreenMode()
    if (deep_green_mode_enabled != enable) {
        deep_green_mode_enabled = enable;
        port_sf2_deep_green_mode_handler();
        printk("Deep Green Mode feature changed to %s (DGM status: %s)\n", enable?"enabled":"disabled", deep_green_mode_activated?"activated":"deactivated");
    }
    return 0;
}

#endif /* defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE) */

// =========== sf2 public functions =======================

void link_change_sf2_led_config(enetx_port_t *port, int linkstatus, int speed)
{
#if defined(CONFIG_BCM963158)
    // workaround sf2.p6 link LED issue
    if (port == unit_port_array[1][6])
    {
        volatile u32 *p6_led_ctrl_reg = (void*)(SWITCH_P6_LED_CTRL_REG);
        u32 val = *p6_led_ctrl_reg & ~ETHSW_LED_CTRL_LNK_SPD_MASK;
        
        switch (speed) {
        case 2500:    val |= ETHSW_LED_CTRL_SPD_OVRD_2P5G;   break;
        case 1000:    val |= ETHSW_LED_CTRL_SPD_OVRD_1G;     break;
        case 100:     val |= ETHSW_LED_CTRL_SPD_OVRD_100M;   break;
        default:      val |= ETHSW_LED_CTRL_SPD_OVRD_10M;
        }
        *p6_led_ctrl_reg = ETHSW_LED_CTRL_SPD_OVRD_EN | val |
                           ETHSW_LED_CTRL_LNK_OVRD_EN | (linkstatus ? ETHSW_LED_CTRL_LNK_STATUS_OVRD : 0);
    }
#endif
}

void extsw_set_mac_address(enetx_port_t *port)
{
    uint8_t *addr;
    int unit = 1;
    // only operate if port is on sf2
    if (port->port_type != PORT_TYPE_SF2_PORT && port->port_type != PORT_TYPE_SF2_SW)
        return;
    addr = port->dev->dev_addr;
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
    /* Static MAC works for all scenarios, so just add multiport
     * MAC only when multiple IMP ports are in use. */
    ioctl_extsw_set_multiport_address(unit, addr);
#else
    if (port->port_class == PORT_CLASS_SW)
        unit = IS_ROOT_SW(port)?0:1;
    else
        unit = PORT_ON_ROOT_SW(port)?0:1;
    extsw_arl_write_ext(unit, addr, 0, ARL_DATA_ENTRY_VALID|ARL_DATA_ENTRY_STATIC|platform_imp_port(unit));
#endif
}

#if defined(CRB_5X3_QGPHY3_WORKAROUND) /* 5x3 crossbar */
/* these functions are for qgphy3 workaround */
int sf2_set_mac_port_state(int phy_port, int link, int speed, int duplex)
{
    // based on imp5\bcmsw.c:bcmsw_set_mac_port_state()
    uint8 v8;

    v8 = REG_PORT_STATE_OVERRIDE;
    v8 |= (link != 0)? REG_PORT_STATE_LNK: 0;
    v8 |= (duplex != 0)? REG_PORT_STATE_FDX: 0;

    if (speed == 1000)
        v8 |= REG_PORT_STATE_1000;
    else if (speed == 100)
        v8 |= REG_PORT_STATE_100;

    down(&sf2_sw->s.conf_sem);
    SF2SW_WREG(1, PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(phy_port), &v8, 1);
    up(&sf2_sw->s.conf_sem);

    return 0;
}

void sf2_force_mac_up(int port)
{
    // based on imp5\eth_pwrmngt.c:ethsw_force_mac_up()
    uint32 reg_val32 = 0;
    
    SF2SW_RREG(1, PAGE_CONTROL, REG_LOW_POWER_EXP1, &reg_val32, 4);
    reg_val32 &= ~((1<<port) << REG_LOW_POWER_EXP1_SLEEP_MACCLK_PORT_SHIFT);
    SF2SW_WREG(1, PAGE_CONTROL, REG_LOW_POWER_EXP1, &reg_val32, 4);
#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
    reg_low_power_exp1 &= ~((1<<port) << REG_LOW_POWER_EXP1_SLEEP_MACCLK_PORT_SHIFT);
#endif
}

#endif /* defined(CRB_5X3_QGPHY3_WORKAROUND) */

//#define CC_PREPEND_DATA_TEST

#if defined(CC_PREPEND_DATA_TEST)
#define PREPEND_DATA_TEST_SIZE 16

static int __prepend_data_test(void *arg_p)
{
    BCM_runnerPrepend_t *prepend_p = arg_p;
    int i;

    for(i=0; i<PREPEND_DATA_TEST_SIZE; ++i)
    {
        prepend_p->data[i] = i;
    }

    prepend_p->size = PREPEND_DATA_TEST_SIZE;

    return 0;
}
#endif



#if defined(ARCHER_DEVICE)
static enetx_port_t *blog_chnl_array[BCM_ENET_SYSPORT_BLOG_CHNL_MAX] = {};

static inline enetx_port_t *port_by_blog_chnl(int blog_chnl)
{
    // Archer platforms blog_chnl is not unit_port based.
    return (blog_chnl >= BCM_ENET_SYSPORT_BLOG_CHNL_MAX) ? NULL : blog_chnl_array[blog_chnl];
}

static int tr_fill_sysp_conf(enetx_port_t *port, void *ctxt)
{
    bcmSysport_Config_t *conf = (bcmSysport_Config_t*)ctxt;

    if (port->has_interface) {
        bcmSysport_BlogChnl_t *ch = &(conf->blog_chnl[port->n.blog_chnl]);
        conf->nbr_of_blog_channels++;
#if defined(CONFIG_BCM963178) || defined(CONFIG_BCM963158)
        ch->sysport = 0;
        ch->switch_id = 0;
        ch->port = port->p.mac->mac_id;
#if defined(PORT_WITH_8TXQ)
        ch->nbr_of_queues = (port->p.mac->mac_id == PORT_WITH_8TXQ) ? 8 : 4;
#else
        ch->nbr_of_queues = 4;
#endif
#elif defined(CONFIG_BCM947622)
        if (PORT_ON_ROOT_SW(port)) {
            ch->sysport = port->p.mac->mac_id;
            ch->port = 0;
        } else {
            ch->sysport = port->p.parent_sw->s.parent_port->p.mac->mac_id;
            ch->port = port->p.mac->mac_id;
        }
        ch->switch_id = 0;
        ch->nbr_of_queues = 8;
#elif defined(CONFIG_BCM96756)
        ch->sysport = 0;
        ch->switch_id = PORT_ON_ROOT_SW(port)?0:1;
        ch->port = port->p.mac->mac_id;
#if defined(PORT_WITH_8TXQ)
        ch->nbr_of_queues = (port->p.mac->mac_id == PORT_WITH_8TXQ && PORT_ON_ROOT_SW(port)) ? 8 : 4;
#else
        ch->nbr_of_queues = 4;
#endif
#endif
        blog_chnl_array[port->n.blog_chnl] = port;
        ch->dev = port->dev;

        enet_dbgv("dev %s nbr_of_blog_ch=%d blog_chnl=%d[%d/%d]\n", port->dev->name, conf->nbr_of_blog_channels, port->n.blog_chnl, ch->sysport, ch->port);
    }
    return 0;
}

static int bcmenet_sysport_config(void)
{
    bcmFun_t *enet_sysport_config = bcmFun_get(BCM_FUN_ID_ENET_SYSPORT_CONFIG);
    bcmSysport_Config_t conf;

    if(!enet_sysport_config)
    {
        enet_err("Sysport Configuration is not available\n");
        return -1;
    }

    memset(&conf, 0, sizeof(conf));
#if defined(CONFIG_BCM963178) || defined(CONFIG_BCM963158)
    conf.nbr_of_sysports = 1;
    conf.sysport[0].mode = BCM_ENET_SYSPORT_MODE_INTERNAL_BRCM_SW;
    conf.switch_parent_port = NO_EXT_SWITCH;
    enet_dbgv(" sysport0 mode= internal_sw\n");
#elif defined(CONFIG_BCM947622)
    conf.nbr_of_sysports = 2;
    conf.sysport[0].mode = unit_port_array[0][0]->p.child_sw ? BCM_ENET_SYSPORT_MODE_EXTERNAL_BRCM_SW : BCM_ENET_SYSPORT_MODE_PORT;
    conf.sysport[1].mode = unit_port_array[0][1]->p.child_sw ? BCM_ENET_SYSPORT_MODE_EXTERNAL_BRCM_SW : BCM_ENET_SYSPORT_MODE_PORT;
    conf.switch_parent_port = NO_EXT_SWITCH;
    enet_dbgv(" sysport0 mode= %s\n", conf.sysport[0].mode == BCM_ENET_SYSPORT_MODE_PORT ? "port" :"ext_sw" );
    enet_dbgv(" sysport1 mode= %s\n", conf.sysport[1].mode == BCM_ENET_SYSPORT_MODE_PORT ? "port" :"ext_sw" );
#elif defined(CONFIG_BCM96756)
    conf.nbr_of_sysports = 1;
    conf.sysport[0].mode =    (sf2_sw_ext) ? BCM_ENET_SYSPORT_MODE_STACKED_BRCM_SW : BCM_ENET_SYSPORT_MODE_INTERNAL_BRCM_SW;
    conf.switch_parent_port = (sf2_sw_ext) ? sf2_sw_ext->s.parent_port->p.mac->mac_id : NO_EXT_SWITCH;
    enet_dbgv(" sysport0 mode= internal_sw %s\n", (sf2_sw_ext)? "with external_sw":"");
    conf.ls_port_q_offset = (sf2_sw_ext) ? LS_PORT_TXQ_OFFSET : 0;
#endif
    port_traverse_ports(root_sw, tr_fill_sysp_conf, PORT_CLASS_PORT, &conf);

    enet_sysport_config(&conf);
    bcmenet_sysport_q_map(queRemap);
    return 0;
}
static int bcmenet_is_bonded_lan_wan_port(void *ctxt)
{
    /* based on impl5\bcmenet.c:bcmenet_is_bonded_lan_wan_port() */
    int ret_val = 0;
#if defined(CONFIG_BCM_KERNEL_BONDING)
    int blog_chnl = *((int*)ctxt);
    enetx_port_t *port = port_by_blog_chnl(blog_chnl);

    if (port && port->p.bond_grp)
    {
        if (port->p.bond_grp->is_lan_wan_cfg &&
            port->p.bond_grp->lan_wan_port == port )
        {
            ret_val = 1;
        }
    }
#endif
    return ret_val;
}

static int bcmenet_is_wan_port(void *ctxt)
{
    int blog_chnl = *((int*)ctxt);
    enetx_port_t *port = port_by_blog_chnl(blog_chnl);
    
    return (port && !bcmenet_is_bonded_lan_wan_port(ctxt)) ? PORT_ROLE_IS_WAN(port) : 0;
}
#endif //ARCHER_DEVICE

int enetxapi_post_sf2_config(void)
{
    uint8_t ieee1905_multicast_mac[6] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x13};
#if defined(SF2_DUAL)
    uint8_t spt_mac[6] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x00};
#endif

#if defined(PHY_138CLASS_SERDES)
    serdes_work_around(NULL); 
#endif

    print_mac_phy_info_all();

#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
#if defined(CONFIG_BCM963178)
    /* Preprogram switch variable speed clock but don't switch to it yet */
    pmc_switch_clock_lowpower_mode (1);
#endif
    port_sf2_deep_green_mode_handler();
#endif

    if (IS_UNIT_SF2(0)) ioctl_extsw_set_multiport_address(0, ieee1905_multicast_mac);
    if (IS_UNIT_SF2(1)) ioctl_extsw_set_multiport_address(1, ieee1905_multicast_mac);

#if defined(SF2_DUAL)
    // in 6756+53134 thru SGMII and SGMII_P8_SEL is low, imp port is not 8, add STP MAC to multiport
    if (ext_sw_imp_port != IMP_PORT_ID)
        ioctl_extsw_set_multiport_address(1, spt_mac);
#endif

#if defined(ARCHER_DEVICE)
    bcmFun_reg(BCM_FUN_ID_ENET_IS_WAN_PORT, bcmenet_is_wan_port);
    bcmFun_reg(BCM_FUN_ID_ENET_IS_BONDED_LAN_WAN_PORT, bcmenet_is_bonded_lan_wan_port);
    bcmenet_sysport_config();
#endif
#if defined(CC_PREPEND_DATA_TEST)
    bcmFun_reg(BCM_FUN_ID_RUNNER_PREPEND, __prepend_data_test);
#endif

#if defined(CONFIG_NET_SWITCHDEV)
    sf2_switchdev_init();
#endif
    return 0;
}

static int tr_find_sf2_sw(enetx_port_t *sw, void *_ctx)
{
    int unit = IS_ROOT_SW(sw)?0:1;

    if (sw->port_type != PORT_TYPE_SF2_SW)
        return 0;

    if (sf2_unit_bmap)
        sf2_sw_ext = sw;
    else
        sf2_sw = sw_p = sw;
    sf2_unit_bmap |= 1 << unit;

    if (sw->s.reset_gpiod)
    {
        struct gpio_desc *gpio = sw->s.reset_gpiod;
#if defined(CONFIG_BCM94908)
	uint8_t val8;
	uint16_t val16;
#endif
        printk("Lift external switch (%s) out of Reset\n", sw->obj_name);
        gpiod_direction_output(gpio, 1);
        gpiod_set_value(gpio, 1);   /* reset active */
        mdelay(100);
        gpiod_set_value(gpio, 0);   /* reset clear */
#if defined(CONFIG_BCM94908)
	// lifting external unmanged out of reset
	mdelay(100);

	// also configure unmanaged mode (53134)
	/* Enable RX_UCST_EN, RX_MCST_EN, RX_BCST_EN */
	val8 = REG_MII_PORT_CONTROL_RX_UCST_EN | REG_MII_PORT_CONTROL_RX_MCST_EN | REG_MII_PORT_CONTROL_RX_BCST_EN;
	sf2_pseudo_mdio_switch_write(PAGE_CONTROL, REG_MII_PORT_CONTROL, &val8, 1);

	/* Force IMP port link up: Enable LINK_STS */
	val8 = IMP_LINK_OVERRIDE_1000FDX;
	sf2_pseudo_mdio_switch_write(PAGE_CONTROL, REG_CONTROL_MII1_PORT_STATE_OVERRIDE, &val8, 1);

	/* unmanged mode, enable forwarding: Enable MII_DUMP_FEDG_EN */
	val16 = 0x0040;
	sf2_pseudo_mdio_switch_write(PAGE_CONTROL, REG_SWITCH_CONTROL, &val16, 2);
#endif
    }
    return 0;
}

int enetxapi_post_sf2_parse(void)
{
    port_traverse_ports(root_sw, tr_find_sf2_sw, PORT_CLASS_SW, NULL);
    return 0;
}

#if defined(ARCHER_DEVICE) && defined(SF2_EXTERNAL)
static int bcmenet_tm_enable_set(void *ctxt)        //TODO_DUAL: how to set this for 6756?
{
    uint16_t val;
    uint8_t  val8;

    int enable = *((int*)ctxt);
    
    if (enable) {
        // turn off 53134 to 47622 flowcontrol
        SF2SW_RREG(1, PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(IMP_PORT_ID), &val8, sizeof(val8));
        val8 &= ~REG_PORT_STATE_TX_FLOWCTL;
        SF2SW_WREG(1, PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(IMP_PORT_ID), &val8, sizeof(val8));
        
    } else {
        // turn on 53134 to 47622 flowcontrol
        SF2SW_RREG(1, PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(IMP_PORT_ID), &val8, sizeof(val8));
        val8 |= REG_PORT_STATE_TX_FLOWCTL;
        SF2SW_WREG(1, PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(IMP_PORT_ID), &val8, sizeof(val8));

        // 53134 only gen IMP0 tx pause using total pause mechanism
        SF2SW_RREG(1, PAGE_FLOW_CTRL, REG_FC_PAUSE_DROP_CTRL, &val, sizeof(val));
        val &= ~FC_PAUSE_TX_IMP0_TXQ_EN;
        SF2SW_WREG(1, PAGE_FLOW_CTRL, REG_FC_PAUSE_DROP_CTRL, &val, sizeof(val));
    }
    return 0;
}
#endif

// =========== sf2 switch ops =============================
int port_sf2_sw_init(enetx_port_t *self)
{
    int unit = IS_ROOT_SW(self)?0:1;
    uint8_t val8;
    uint16_t val16;

    SW_SET_HW_FWD(self);	/* initially hw fwd for all ports on switch */
    platform_enable_p8_rdp_sel();
    platform_set_imp_speed(self);

    _extsw_setup_imp_ports(unit);
    _extsw_setup_imp_fwding(unit);

    // configure trunk groups if required.
    _extsw_port_trunk_init(unit);

    // set ARL AGE_DYNAMIC bit for aging operations
    port_sw_fast_age(self);

    // set VLAN_CTRL1.EN_RSV_MCAST_UNTAG
    SF2SW_RREG(unit, PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_CTRL1, &val8, 1);
    val8 |= (VLAN_EN_RSV_MCAST_UNTAG_M << VLAN_EN_RSV_MCAST_UNTAG_S);
    SF2SW_WREG(unit, PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_CTRL1, &val8, 1);

    // set tx shaper to L1 mode (+IFG_BYTES) by default, use "txratecfg" to overwrite
    val16 = 0x1ff;
#if defined(SF2_DUAL)
    // set lightstacking port to L2 rate
    if ((unit == 0) && sf2_sw_ext)
        val16 &= ~(1<< sf2_sw_ext->s.parent_port->p.mac->mac_id); 
#endif
    SF2SW_WREG(unit, PAGE_PORT_EGRESS_SHAPER, SF2_REG_SHAPER_IFG_BYTES, &val16, 2);

    _sf2_qos_default(unit);
#if !defined(SF2_EXTERNAL)
    _sf2_conf_acb_conges_profile(0);
#endif
    if (queThreConfMode != ThreModeManual)
    {
        _sf2_conf_que_thred(unit); // for acb testing
    }

#if !defined(CONFIG_NET_SWITCHDEV)
    /* Register ARL Entry clear routine */
    bcmFun_reg(BCM_FUN_IN_ENET_CLEAR_ARL_ENTRY, remove_arl_entry_wrapper);
#endif

#if defined(ARCHER_DEVICE) && defined(SF2_EXTERNAL)
    {
        int tm_en = 0;
        bcmenet_tm_enable_set(&tm_en);
        bcmFun_reg(BCM_FUN_ID_ENET_TM_EN_SET, bcmenet_tm_enable_set);
    }
#endif
    return 0;
}

int port_sf2_sw_uninit(enetx_port_t *self)
{
#if !defined(CONFIG_NET_SWITCHDEV)
    bcmFun_dereg(BCM_FUN_IN_ENET_CLEAR_ARL_ENTRY);
#endif
    return 0;
}


#if defined(CONFIG_BLOG) && defined(CONFIG_BCM_KERNEL_BONDING)

static int tr_update_trunk_chnl_rx(enetx_port_t *port, void *_ctx)
{
    bond_info_t *bond_grp = (bond_info_t *)_ctx;
    bcmFun_t *enet_bond_rx_port_map = bcmFun_get(BCM_FUN_ID_ENET_BOND_RX_PORT_MAP);
    bcmEnet_BondRxPortMap_t map;

    if (!bond_grp || port->p.bond_grp == bond_grp) {
        map.blog_chnl = port->n.blog_chnl;
        map.blog_chnl_rx = port->n.blog_chnl_rx;
        enet_bond_rx_port_map(&map);
    }
    return 0;
}

extern bond_info_t bond_grps[];
int sw_config_trunk_chnl_rx(enetx_port_t *sw, enetx_port_t *port, int grp_no, int add)
{
    bond_info_t *bond_grp = &bond_grps[grp_no];

    if (bcmFun_get(BCM_FUN_ID_ENET_BOND_RX_PORT_MAP)) {
        if (bond_grp->port_count)
            port_traverse_ports(root_sw, tr_update_trunk_chnl_rx, PORT_CLASS_PORT, bond_grp);

        if (!add)       // update removed member
            tr_update_trunk_chnl_rx(port, NULL);
    }
    return 0;
}
#else // !CONFIG_BLOG
int sw_config_trunk_chnl_rx(enetx_port_t *sw, enetx_port_t *port, int grp_no, int add)
{
    return 0;
}
#endif //!CONFIG_BLOG

int port_sf2_sw_config_trunk(enetx_port_t *sw, enetx_port_t *port, int grp_no, int add)
{
    int unit = IS_ROOT_SW(sw)?0:1;
    uint16_t v16;

    down(&sf2_sw->s.conf_sem);
    SF2SW_RREG(unit, PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL + (2*grp_no), &v16, 2);
    if (add)
    {
        v16 |= ( ( (1<< port->p.mac->mac_id) & TRUNK_EN_GRP_M ) << TRUNK_EN_GRP_S );
        enet_dbg("ADD : port %s to group <%d>; New pmap <0x%02x>\n", port->obj_name, grp_no, v16);
    }
    else
    {
        v16 &= ~( ( (1<< port->p.mac->mac_id) & TRUNK_EN_GRP_M ) << TRUNK_EN_GRP_S );
        enet_dbg("REM : port %s to group <%d>; New pmap <0x%02x>\n", port->obj_name, grp_no, v16);
    }
    SF2SW_WREG(unit, PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL + (2*grp_no), &v16, 2);
    up(&sf2_sw->s.conf_sem);

    return sw_config_trunk_chnl_rx(sw, port, grp_no, add);
}

int port_sf2_sw_update_pbvlan(enetx_port_t *sw, unsigned int pmap)
{
    int unit = IS_ROOT_SW(sw)?0:1;
    int i;

    if (pmap == 0) return 0;    //nothing to do

    for (i=0; i < MAX_SWITCH_PORTS; i++ )
    {
        if (pmap & (1<<i))
        {
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
            _extsw_set_pbvlan(unit, i, pmap | (1 << cur_port_imp_map[i]));
#else
            _extsw_set_pbvlan(unit, i, pmap | (1 << platform_imp_port(unit)));
#endif
        }
    }
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWSWITCHING functions ---

static uint16_t dis_learning_ext = 0x0100; /* This default value does not matter */

int port_sf2_sw_hw_sw_state_set(enetx_port_t *sw, unsigned long state)
{
    uint16_t val;
    int unit = IS_ROOT_SW(sw)?0:1;

    if (SW_IS_HW_FWD(sw) == state) {
        return 0;
    }

    down(&sw->s.conf_sem);
    if (state == HW_SWITCHING_ENABLED) {
        /* restore disable learning register */
        SF2SW_WREG(unit, PAGE_CONTROL, REG_DISABLE_LEARNING, &dis_learning_ext, 2);
        SW_SET_HW_FWD(sw);
    }
    else {
        /* Save disable_learning_reg setting */
        SF2SW_RREG(unit, PAGE_CONTROL, REG_DISABLE_LEARNING, &dis_learning_ext, 2);

        /* disable learning on all ports */
        val = PBMAP_ALL;
        SF2SW_WREG(unit, PAGE_CONTROL, REG_DISABLE_LEARNING, &val, 2);

        /* flush arl table dynamic entries */
        port_sw_fast_age(sw);
        SW_CLR_HW_FWD(sw);
    }

    up(&sw->s.conf_sem);
    return 0;
}

int port_sf2_sw_hw_sw_state_get(enetx_port_t *sw)
{
    return SW_IS_HW_FWD(sw)?HW_SWITCHING_ENABLED:HW_SWITCHING_DISABLED;
}


// =========== sf2 port ops =============================
int port_sf2_port_init(enetx_port_t *self)
{
    if (self->has_interface) {
        self->n.blog_phy = BLOG_ENETPHY;
        self->n.blog_chnl = self->n.blog_chnl_rx = root_sw->n.blog_chnl++;
        enet_dbgv("%s port_id=%d blog_chnl=%d\n", self->obj_name, self->p.port_id, self->n.blog_chnl);

        if (mux_set_rx_index(self->p.parent_sw, self->n.blog_chnl, self))
            return -1;

        /* also register demux at root for receive processing if port not on root sw */
        if (!PORT_ON_ROOT_SW(self))
            if (mux_set_rx_index(root_sw, self->n.blog_chnl, self))
                return -1;
    }

    enet_dbg("Initialized %s role %s\n", self->obj_name, (self->n.port_netdev_role==PORT_NETDEV_ROLE_WAN)?"wan":"lan" );

    return 0;
}

void port_sf2_generic_open(enetx_port_t *self)
{
    if (self->p.parent_sw->s.parent_port)
    {
        PORT_SET_EXT_SW(self);
        // port is on external switch, also enable connected runner/sysp port
        port_open(self->p.parent_sw->s.parent_port);
    }

    extlh_mac2mac_port_handle(self);
    port_generic_open(self); 
}

#ifdef EMBEDDED_BRCMTAG_TX_INSERT
// based on impl5:bcm63xx_fkb_put_tag()
static inline void enet_fkb_put_tag(FkBuff_t * fkb_p,
        struct net_device * dev, unsigned int port_map)
{
    int i;
    int tailroom;
    uint16 *from = (uint16*)fkb_p->data;
    BcmEnet_hdr2 *pHdr = (BcmEnet_hdr2 *)from;

    if (ntohs(pHdr->brcm_type) != BRCM_TYPE2)
    {
        uint16 * to = (uint16*)fkb_push(fkb_p, BRCM_TAG_TYPE2_LEN);
        pHdr = (BcmEnet_hdr2 *)to;
        for ( i=0; i<ETH_ALEN; *to++ = *from++, i++ ); /* memmove 2 * ETH_ALEN */
        /* set port of ingress brcm tag */
        pHdr->brcm_tag = htons(port_map);

    }
    /* set ingress brcm tag and TC bit */
    pHdr->brcm_type = htons(BRCM_TAG2_EGRESS | (SKBMARK_GET_Q_PRIO(fkb_p->mark) << 10));
    tailroom = ETH_ZLEN + BRCM_TAG_TYPE2_LEN - fkb_p->len;
    if (tailroom > 0)
    {
        fkb_pad(fkb_p, tailroom);
        fkb_p->dirty_p = _to_dptr_from_kptr_(fkb_p->data + fkb_p->len);
    }
}

// based on impl5:bcm63xx_skb_put_tag()
static struct sk_buff *enet_skb_put_tag(struct sk_buff *skb,
        struct net_device *dev, unsigned int port_map)
{
    BcmEnet_hdr2 *pHdr = (BcmEnet_hdr2 *)skb->data;
    int i, headroom;
    int tailroom;

    if (ntohs(pHdr->brcm_type) == BRCM_TYPE2)
    {
        headroom = 0;
        tailroom = ETH_ZLEN + BRCM_TAG_TYPE2_LEN - skb->len;
    }
    else
    {
        headroom = BRCM_TAG_TYPE2_LEN;
        tailroom = ETH_ZLEN - skb->len;
    }

    if (tailroom < 0)
    {
        tailroom = 0;
    }

#if defined(CONFIG_BCM_USBNET_ACCELERATION)
    if ((skb_writable_headroom(skb) < headroom) || (skb_tailroom(skb) < tailroom))
#else
    if ((skb_headroom(skb) < headroom) || (skb_tailroom(skb) < tailroom))
#endif
    {
        struct sk_buff *oskb = skb;
        skb = skb_copy_expand(oskb, headroom, tailroom, GFP_ATOMIC);
        kfree_skb(oskb);
        if (!skb)
        {
            return NULL;
        }
    }
#if defined(CONFIG_BCM_USBNET_ACCELERATION)
    else if ((headroom != 0) && (skb->bcm_ext.clone_wr_head == NULL))
#else
    else if ((headroom != 0) && !(skb_clone_writable(skb, headroom)))
#endif
    {
        skb = skb_unshare(skb, GFP_ATOMIC);
        if (!skb)
        {
            return NULL;
        }
    }

    if (tailroom > 0)
    {
        if (skb_is_nonlinear(skb))
        {
            /* Non linear skb whose skb->len is < minimum Ethernet Packet Length
               (ETHZLEN or ETH_ZLEN + BroadcomMgmtTag Length) */
            if (skb_linearize(skb))
            {
                return NULL;
            }
        }
        memset(skb->data + skb->len, 0, tailroom);  /* padding to 0 */
        skb_put(skb, tailroom);
    }

    if (headroom != 0)
    {
        uint16 *to, *from;
        BcmEnet_hdr2 *pHdr2 = (BcmEnet_hdr2 *)skb_push(skb, headroom);
        to = (uint16*)pHdr2;
        from = (uint16*)(skb->data + headroom);
        for ( i=0; i<ETH_ALEN; *to++ = *from++, i++ ); /* memmove 2 * ETH_ALEN */
        /* set ingress brcm tag and TC bit */
        pHdr2->brcm_type = htons(BRCM_TAG2_EGRESS | (SKBMARK_GET_Q_PRIO(skb->mark) << 10));
        pHdr2->brcm_tag  = htons(port_map);
        if (skb_mac_header_was_set(skb))
            skb->mac_header -= headroom;
        /* network_header and transport_header are unchanged */
    }
    return skb;
}

/* insert broadcom tag for external switch port */
int port_sf2_tx_pkt_mod(enetx_port_t *port, pNBuff_t *pNBuff, uint8_t **data, uint32_t *len, unsigned int port_map)
{
    FkBuff_t *pFkb = 0;
    struct sk_buff *skb = 0;

    if (IS_FKBUFF_PTR(*pNBuff))
    {
        FkBuff_t * pFkbOrig = PNBUFF_2_FKBUFF(*pNBuff);

#if defined(CONFIG_BCM_ARCHER_SIM)
        if(SKBMARK_GET_IQPRIO_MARK(pFkbOrig->mark))
        {
            return 0;
        }
#endif
//enet_err("len=%d dirty=%d\n", pFkbOrig->len, (uint32)(pFkbOrig->dirty_p - pFkbOrig->data));
        pFkb = fkb_unshare(pFkbOrig);

        if (pFkb == FKB_NULL)
        {
            fkb_free(pFkbOrig);
            INC_STAT_TX_DROP(port,tx_dropped_no_fkb);
            return -1;
        }
        enet_fkb_put_tag(pFkb, port->dev, port_map); /* Portmap for external switch */
        *data = (void *)pFkb->data;
        *len  = pFkb->len;
        *pNBuff = PBUF_2_PNBUFF((void*)pFkb,FKBUFF_PTR);
    }
    else
    {
        skb = PNBUFF_2_SKBUFF(*pNBuff);
        skb = enet_skb_put_tag(skb, port->dev, port_map);    /* Portmap for external switch and also pads to 0 */
        if (skb == NULL) {
            INC_STAT_TX_DROP(port,tx_dropped_no_skb);
            return -1;
        }
        *data = (void *)skb->data;   /* Re-encode pNBuff for adjusted data and len */
        *len  = skb->len;
        *pNBuff = PBUF_2_PNBUFF((void*)skb,SKBUFF_PTR);
    }
    return 0;
}
#endif // EMBEDDED_BRCMTAG_TX_INSERT

#ifdef EMBEDDED_BRCMTAG_RX_REMOVE
static inline int bcm_mvtags_len(char *ethHdr)
{
    unsigned int end_offset = 0;
    BcmEnet_hdr2* bhd;
    uint16 brcm_type;

    bhd = (BcmEnet_hdr2*)ethHdr;
    brcm_type = ntohs(bhd->brcm_type);
    if (brcm_type == BRCM_TYPE2)
    {
        end_offset += BRCM_TAG_TYPE2_LEN;
    }

    return end_offset;
}

/* based on impl5:bcm_type_trans(), bcm_mvtags_len() */
int port_sf2_rx_pkt_mod(enetx_port_t *port, struct sk_buff *skb)
{
    unsigned int end_offset = 0, from_offset = 0;
    uint16 *to, *end, *from;

    skb_reset_mac_header(skb);
    end_offset = bcm_mvtags_len(skb->data);
    if (end_offset)
    {
        from_offset = OFFSETOF(struct ethhdr, h_proto);

        to = (uint16*)(skb->data + from_offset + end_offset) - 1;
        end = (uint16*)(skb->data + end_offset) - 1;
        from = (uint16*)(skb->data + from_offset) - 1;

        while ( to != end )
            *to-- = *from--;
    }

    skb_set_mac_header(skb, end_offset);

    skb_pull(skb, end_offset);
    return 0;
}
#endif // EMBEDDED_BRCMTAG_RX_REMOVE

uint32_t port_sf2_tx_q_remap(enetx_port_t *port, uint32_t txq)
{
    // based on impl5:bcmeapi_enet_prepare_xmit()
#if defined(PORT_WITH_8TXQ)
    // no remap if port has 8 txqs and is WAN
    if ((port->p.mac->mac_id == PORT_WITH_8TXQ) &&
        (port->n.port_netdev_role == PORT_NETDEV_ROLE_WAN))
        return txq;
#endif
    return (queRemap >> (txq * 4)) & 0xf;
}

#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
uint16_t port_sf2_tx_lb_imp(enetx_port_t *port, uint16_t port_id, void* pHdr)
{
    return port_imp_emac_map[port_id];
}
#endif /* defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT) */

#if defined(ARCHER_DEVICE) && defined(SF2_DUAL)
void port_sf2_dual_tx_shape(enetx_port_t *port, int speed)
{
    if (port->p.parent_sw == sf2_sw_ext)
    {
        struct ethswctl_data e;
        uint16_t val;

        e.unit = 0;
        e.port = sf2_sw_ext->s.parent_port->p.mac->mac_id;
        e.queue = port->p.mac->mac_id + LS_PORT_TXQ_OFFSET;
        
        // set AVB mode
        SF2SW_RREG(e.unit, PAGE_Q0_EGRESS_SHAPER + e.queue, SF2_REG_SHAPER_ENB_AVB, &val, sizeof(val));
        val |= 1 << e.port;
        SF2SW_WREG(e.unit, PAGE_Q0_EGRESS_SHAPER + e.queue, SF2_REG_SHAPER_ENB_AVB, &val, sizeof(val));
        
        e.type = TYPE_SET;
        e.sub_type = 0; /* BYTE_MODE */
        e.limit = speed * 1000;
        e.burst_size = 0;   // calculate in function below
        ioctl_extsw_port_erc_config(&e);
        enet_dbgv("set tx shaper unit%d port%d q%d to %dkbps burst%dkbps\n", e.unit, e.port, e.queue, e.limit, e.burst_size);
    }
}
#endif //SF2_DUAL

#if defined(CONFIG_BCM963138)

#include "bcm_led.h"
#include "bcm_pinmux.h"

static void _handle_phy_move_led(enetx_port_t *port, unsigned short gpionum, enum bp_id id)
{
    // assume when GPHY4 when configured for LAN is on sf2 p4
    short lkup_port = PORT_ON_ROOT_SW(port) ? 0 : 4;
    short reverse_lkup = PORT_ON_ROOT_SW(port) ? 4 : 0;
    unsigned int mux_info, mux_cur;

    if ((gpionum != BP_GPIO_NONE) && (BpGrepPinmuxListByPort(id, lkup_port, gpionum, &mux_info) == 0)) {
        // update new pinmux and update optled map
        bcm_set_pinmux(gpionum, (mux_info & BP_PINMUX_VAL_MASK) >> BP_PINMUX_VAL_SHIFT);
        bcm_pinmux_update_optled_map(gpionum, mux_info);

        if (BpGrepPinmuxListByPort(id, reverse_lkup, gpionum, &mux_cur) == 0) {
            unsigned short led_cur, led_new;
            led_cur = (mux_cur & BP_PINMUX_OPTLED_VALID) ? ((mux_cur & BP_PINMUX_OPTLED_MASK) >> BP_PINMUX_OPTLED_SHIFT): gpionum;
            led_new = (mux_info & BP_PINMUX_OPTLED_VALID) ? ((mux_info & BP_PINMUX_OPTLED_MASK) >> BP_PINMUX_OPTLED_SHIFT): gpionum;
            if ((led_cur != led_new) && (mux_cur & BP_PINMUX_HWLED) && (mux_info & BP_PINMUX_HWLED)) {
                // update LED mapping and disable flashing
                bcm_led_update_source(0, 0, 1UL<<led_new, 1UL<<led_cur);
                bcm_led_zero_flash_rate(led_new);
            }
            
        }
    }
}

void handle_phy_move(enetx_port_t *port, phy_dev_t *phy_dev, int external_endpoint)
{
    // handle GPHY4 LAN/WAN move updating link/speed LED pinmux
    if (external_endpoint == 1 && phy_dev->priv) {
        LED_INFO *led = phy_dev->priv;

        _handle_phy_move_led(port, led->LedLink & BP_GPIO_NUM_MASK, bp_usLinkLed);
        _handle_phy_move_led(port, led->speedLed100 & BP_GPIO_NUM_MASK, bp_usSpeedLed100);
        _handle_phy_move_led(port, led->speedLed1000 & BP_GPIO_NUM_MASK, bp_usSpeedLed1000);
    }
}
#endif // 63138

#if defined(enet_dbg_sf2_proc)
#include "sf2_dbg_proc.c"
#endif
