/*
<:copyright-BRCM:2009:proprietary:standard

   Copyright (c) 2009 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/

/*
 *******************************************************************************
 * File Name  : fapProtocol.c
 *
 * Description: This implementation supports the dynamically learnt NAT Flows
 *
 *******************************************************************************
 */
#include <linux/module.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <linux/ppp_defs.h>
#include <linux/version.h>
#include <net/ip.h>
#include <linux/bcm_skb_defines.h>
#include <linux/bcm_log.h>
#include <linux/blog.h>
#include <linux/vlanctl_bind.h>
#include "pktHdr.h"
#include "fap_hw.h"
#include "fap.h"
#include "fap_task.h"
#include "bcmtypes.h"
#include "fap_dqm.h"
#include "fap_dqmHost.h"
#include "fap_local.h"
#include "fap_packet.h"
#include "fap_protocol.h"
#include "fap_mcast.h"
#include "fap_l2flow.h"
#include "fap4ke_iopDma.h"
#include "fcache.h"
#include "fcachehw.h"
#include "bcmnet.h"             /* SKBMARK_GET_Q_PRIO */
#include "bcmxtmrt.h"
#include "bcmenet.h"
#include "bcmPktDma.h"
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
#include <net/ipv6.h>
#endif

#define protoLogDebug bcmLog_logIsEnabled(BCM_LOG_ID_FAPPROTO, BCM_LOG_LEVEL_DEBUG)
#define protoDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_FAPPROTO, fmt, ##arg)
#define protoInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_FAPPROTO, fmt, ##arg)
#define protoNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_FAPPROTO, fmt, ##arg)
#define protoError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_FAPPROTO, fmt, ##arg)
#define protoPrint(fmt, arg...)   printk(fmt, ##arg)
#define PROTODBG(prt, fmt,arg...) BCM_LOGCODE(if(protoLogDebug)         \
                                                  prt(fmt, ##arg);)

#define CHECK_CMD_LIST(_cmdList_p, _index)                              \
    do {                                                                \
        BCM_LOGCODE(                                                    \
            if(protoLogDebug) {                                         \
                printk("%s, %u : Command List\n", __FUNCTION__, __LINE__); \
                dumpMemHost(_cmdList_p, _index);                        \
            } );                                                        \
        if(_index >= FAP4KE_PKT_CMD_LIST_SIZE) {                        \
            protoError("CMD List Overflow : index %d, size %u\n",       \
                       (int)_index, FAP4KE_PKT_CMD_LIST_SIZE);          \
            return FAP_ERROR;                                           \
        }                                                               \
    } while(0)

void dumpMemHost(uint8 *mem_p, uint32 length)
{
    int i;

    printk("addr <0x%08X>, length <%d>\n",
           (uint32_t)mem_p, length);

    printk("%04X : ", 0);

    for(i=0; i<length; ++i)
    {
        if((i != 0) && ((i % 16) == 0))
        {
            printk("\n%04X : ", i);
        }
        printk("%02X ", *(mem_p + i));
    }

    printk("\n");
}

/*
 *------------------------------------------------------------------------------
 * Protocol layer global statistics and active flow list
 *------------------------------------------------------------------------------
 */
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
DEFINE_SPINLOCK(fapProto_lock_g);
#else
spinlock_t fapProto_lock_g = SPIN_LOCK_UNLOCKED;
#endif
#endif

typedef struct {
    uint32_t status;        /* status: Enable=1 or Disable=0 */

    uint32_t activates;     /* number of activate (downcalls)   */
    uint32_t failures;      /* number of activate failures      */
    uint32_t deactivates;   /* number of deactivate (downcalls) */
    uint32_t flushes;       /* number of clear (upcalls)        */
    uint32_t active;
#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
    /*
     * FAP makes upcalls into ARL layer via theses hooks
     */
    HOOKP      fap_activate_arl_hook _FCALIGN_;
    HOOK4PARM  fap_deactivate_arl_hook;
    HOOK3PARM  fap_refresh_arl_hook;
    HOOK32     fap_reset_stats_arl_hook;
    HOOK32     fap_clear_arl_hook;

    uint32_t fap_arlBindCnt;         /* number of ARL programmed flows */
#endif
} fapProtoState_t;

static fapProtoState_t fapState_g;   /* Protocol layer global context */

static int fapTcpAckPrioStatus;

int fapTcpAckPrioConfig(int status)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

    fapMsg.generic.word[0]= status;

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_CONFIG_TCP_ACK_MFLOWS, &fapMsg);
    }
    return 0;
}

/* These functions updating fapState should be called
 * with FAP_PROTO_LOCK */
void decrement_Fapfailures(void)
{
    fapState_g.failures--;
}
void update_Fapdeactivates(void)
{
    fapState_g.deactivates++;
    fapState_g.active--;
}

uint8_t fapGetHwEngine(uint32_t hwTuple)
{
    uint8_t eng;

#if defined(CONFIG_BCM_ARL_MODULE) || defined(CONFIG_BCM_ARL)
    if (hwTuple & FAP_HW_TUPLE_ARL_MASK)
        eng = FAP_HW_ENGINE_ARL;
    else
#endif
    if (hwTuple & FAP_HW_TUPLE_MCAST_MASK)
        eng = FAP_HW_ENGINE_MCAST;
    else if (hwTuple & FAP_HW_TUPLE_L2FLOW_MASK)
        eng = FAP_HW_ENGINE_L2FLOW;
    else 
        eng = FAP_HW_ENGINE_SWC;

    return eng;
}

uint16_t fapGetHwEntIx(uint32_t hwTuple)
{
    return (uint16_t)(hwTuple & ((FAP_MAX_FLOWS * NUM_FAPS)-1));
}

#if defined(CONFIG_BCM_FHW)
static FC_CLEAR_HOOK fhw_clear_hook_fp = (FC_CLEAR_HOOK)NULL;
#endif

/*
 *------------------------------------------------------------------------------
 * Function prototype
 *------------------------------------------------------------------------------
 */
#if defined(CONFIG_BCM_FAP_LAYER2)
void fap_bind_vlanctl(int enable_flag);
uint32_t fap_config(Blog_t * blog_p, BlogTraffic_t traffic);
Blog_t * fap_deconf(uint32_t key_info, BlogTraffic_t traffic);
#endif


/*
 *------------------------------------------------------------------------------
 * Wrapper to a BLOG. Storage for a blog's Rx L2 header with BRCM_TAG stripped.
 *------------------------------------------------------------------------------
 */
typedef struct Clog
{
    Blog_t  * blog_p;
    uint8_t * rx_l2hdr_p;
    uint8_t rx_l2hdr[BLOG_HDRSZ_MAX] __attribute__((aligned(4)));
    uint8_t rx_length;
} Clog_t;

/*
 *------------------------------------------------------------------------------
 * Function   : buildClog
 * Description: This function strips the BCM_SWC (BRCM TAG) if present in the
 *              Blog Rx l2 header into a  Clog_t structure.
 *------------------------------------------------------------------------------
 */
static void buildClog(Blog_t *blog_p, Clog_t *clog_p)
{
    clog_p->blog_p = blog_p;

    {
        clog_p->rx_l2hdr_p = &blog_p->rx.l2hdr[0];
        clog_p->rx_length = blog_p->rx.length;
    }
}

#if defined(CONFIG_BCM_PORTS_ON_INT_EXT_SW)
static int32 addL2Commands(fap4kePkt_flowInfo_t *flowInfo_p, uint8 *cmdList, int32 cmdListIx, Clog_t *clog_p, uint8 isExtSwitch, uint8 extSwPort)
#else
static int32 addL2Commands(fap4kePkt_flowInfo_t *flowInfo_p, uint8 *cmdList, int32 cmdListIx, Clog_t *clog_p)
#endif
{
    Blog_t *blog_p = clog_p->blog_p;
    int32 cmdSize;
    uint32_t rxOffset;
    uint32_t rxL2Len;
    uint32_t txL2Len;
    uint8 *txData_p;
    uint16 data;
    int size;
    int ix;

    rxL2Len = blog_p->rx.length;
    txL2Len = blog_p->tx.length;
    txData_p = blog_p->tx.l2hdr;

    flowInfo_p->txAdjust += (rxL2Len - txL2Len);

    size = 0;
    rxOffset = 0;

    for(ix = 0; ix < blog_p->tx.count; ix++)
    {
        switch(blog_p->tx.encap[ix])
        {
            case ETH_802x:
            {
                protoDebug("Insert ENET Header: MAC DA/SA");
                size = ETH_ALEN * 2;

                PROTODBG(protoPrint, "%s INSERT: ix <%d> offset <%u> length <%u>\n",
                         __FUNCTION__, (int)cmdListIx, rxOffset, size);
                cmdSize = iopDma_CmdListAddInsert(&cmdList[cmdListIx],
                                                  rxOffset, txData_p, size);
                if(cmdSize == FAP_ERROR)
                {
                    protoError("Command List Error (%d)", __LINE__);
                    return FAP_ERROR;
                }
                cmdListIx += cmdSize;

                CHECK_CMD_LIST(cmdList, cmdListIx);

                txData_p += size;

#if defined(CONFIG_BCM_EXT_SWITCH)
#if defined(CONFIG_BCM_PORTS_ON_INT_EXT_SW)
                if((flowInfo_p->dest.phy == FAP4KE_PKT_PHY_ENET) && isExtSwitch)
#else
                if(flowInfo_p->dest.phy == FAP4KE_PKT_PHY_ENET)
#endif
                {
                    uint32_t tag;

                    flowInfo_p->dest.phy = FAP4KE_PKT_PHY_ENET_EXT;
                    protoDebug("Insert Type 2 Broadcom Tag");
                    size = sizeof(uint32_t);

                    /*
                     * txAdjust:
                     * ETH -> ETH: FC strips the tag (+4), FAP replaces the tag (-4) = 0
                     * XTM -> ETH: FC doesn't see the tag (0), FAP inserts the tag (-4) = -4
                     */
                    flowInfo_p->txAdjust -= size;

                    tag = (BRCM_TAG2_EGRESS | ((flowInfo_p->dest.queue << 10) & DMA_PRIO)) << 16;
#if defined(CONFIG_BCM_PORTS_ON_INT_EXT_SW)
                    tag |= (uint16_t)(1 << extSwPort);
#else
                    tag |= (uint16_t)(flowInfo_p->dest.channelMask);
#endif
                    PROTODBG(protoPrint, "%s INSERT: ix <%d> offset <%u> length <%u> tag 0x%x channelMask 0x%x\n",
                             __FUNCTION__, (int)cmdListIx, rxOffset, size, tag, flowInfo_p->dest.channelMask);
                    cmdSize = iopDma_CmdListAddInsert(&cmdList[cmdListIx],
                                                      rxOffset, (uint8_t *)&tag, size);
                    if(cmdSize == FAP_ERROR)
                    {
                        protoError("Command List Error (%d)", __LINE__);
                        return FAP_ERROR;
                    }
                    cmdListIx += cmdSize;

                    CHECK_CMD_LIST(cmdList, cmdListIx);
                }
#endif
                protoDebug("Insert ENET Header: Ethertype");
                size = sizeof(uint16_t);

                PROTODBG(protoPrint, "%s INSERT: ix <%d> offset <%u> length <%u>\n",
                         __FUNCTION__, (int)cmdListIx, rxOffset, size);
                cmdSize = iopDma_CmdListAddInsert(&cmdList[cmdListIx],
                                                  rxOffset, txData_p, size);
                if(cmdSize == FAP_ERROR)
                {
                    protoError("Command List Error (%d)", __LINE__);
                    return FAP_ERROR;
                }
                cmdListIx += cmdSize;

                CHECK_CMD_LIST(cmdList, cmdListIx);

                txData_p += size;
            }
            break;

            case VLAN_8021Q:
            {
                protoDebug("Insert VLAN Header");
                size = sizeof(struct vlan_hdr);

                PROTODBG(protoPrint, "%s INSERT: ix <%d> offset <%u> length <%u>\n",
                         __FUNCTION__, (int)cmdListIx, rxOffset, size);
                cmdSize = iopDma_CmdListAddInsert(&cmdList[cmdListIx],
                                                  rxOffset, txData_p, size);
                if(cmdSize == FAP_ERROR)
                {
                    protoError("Command List Error (%d)", __LINE__);
                    return FAP_ERROR;
                }
                cmdListIx += cmdSize;

                CHECK_CMD_LIST(cmdList, cmdListIx);

                txData_p += size;
            }
            break;

            case PPP_1661:
            {
                protoDebug("Insert PPP Header");
                size = sizeof(uint16_t); /* ppp length */

                PROTODBG(protoPrint, "%s INSERT: ix <%d> offset <%u> length <%u>\n",
                         __FUNCTION__, (int)cmdListIx, rxOffset, size);
                cmdSize = iopDma_CmdListAddInsert(&cmdList[cmdListIx],
                                                  rxOffset, txData_p, size);
                if(cmdSize == FAP_ERROR)
                {
                    protoError("Command List Error (%d)", __LINE__);
                    return FAP_ERROR;
                }
                cmdListIx += cmdSize;

                CHECK_CMD_LIST(cmdList, cmdListIx);

                txData_p += size;
            }
            break;

            case PPPoE_2516:
            {
                protoDebug("Insert PPPoE Header");

                /* Insert PPPoE Header up to length field, plus all preceeding headers */

                size = OFFSETOF(pppoe_hdr, length);

                PROTODBG(protoPrint, "%s INSERT: ix <%d> offset <%u> length <%u>\n",
                         __FUNCTION__, (int)cmdListIx, rxOffset, size);
                cmdSize = iopDma_CmdListAddInsert(&cmdList[cmdListIx],
                                                  rxOffset, txData_p, size);
                if(cmdSize == FAP_ERROR)
                {
                    protoError("Command List Error (%d)", __LINE__);
                    return FAP_ERROR;
                }
                cmdListIx += cmdSize;

                CHECK_CMD_LIST(cmdList, cmdListIx);

                txData_p += size;

                /*
                 * Insert PPPoE length field
                 * pppoe_hdr.length = ip_hdr.tot_len + sizeof(PPP_1661 header)
                 * The PDU length will be set to ip_hdr.tot_len in the FAP, for each packet
                 * We have to set the PPP header length as a constant in the command
                 */
                size = sizeof(uint16_t); /* ppp length */

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
                /* IPv6: pppoe_hdr.length = ip6_hdr.payload_len + sizeof(PPP_1661 header) + sizeof(fap4kePkt_ipv6Header_t) */
                /* The PDU length will be set to ip6_hdr.payload_len in the FAP for IPv6 rx packets */
                /* otherwise it will be ip_hdr.tot_len */

                if ( T4in6DN(blog_p) )        /* rx IPv6:IPv4; tx IPv4 */
                    data  = sizeof(uint16_t); 
                else if ( T4in6UP(blog_p) )   /* rx IPv4; tx IPv6:IPv4 */
                    data  = sizeof(uint16_t) + sizeof(fap4kePkt_ipv6Header_t);
                else if(T6in4DN(blog_p))    /* rx IPv4:IPv6; tx IPv6 */
                    data  = sizeof(uint16_t) + sizeof(fap4kePkt_ipv6Header_t);
                else if(T6in4UP(blog_p))    /* rx IPv6; tx IPv4:IPv6 */
                    data  = sizeof(uint16_t) + sizeof(fap4kePkt_ipv6Header_t) + sizeof(struct iphdr);
                else if ( blog_p->tx.info.bmap.PLD_IPv6 )   /* rx IPv6; tx IPv6 */
                    data  = sizeof(uint16_t) + sizeof(fap4kePkt_ipv6Header_t); 
                else                          /* rx IPv4; tx IPv4 */
#endif
                    data  = sizeof(uint16_t); 

                PROTODBG(protoPrint, "%s INSERT LENGTH: ix <%d> offset <%u> data <%u>\n",
                         __FUNCTION__, (int)cmdListIx, rxOffset, size);
                cmdSize = iopDma_CmdListAddInsertLength(&cmdList[cmdListIx],
                                                        rxOffset, data);
                if(cmdSize == FAP_ERROR)
                {
                    protoError("Command List Error (%d)", __LINE__);
                    return FAP_ERROR;
                }
                cmdListIx += cmdSize;

                CHECK_CMD_LIST(cmdList, cmdListIx);

                txData_p += size;

                /*** Insert PPP_1661 ***/
                /* This will correctly handle the T4in6DN & T4in6UP cases */
                /* by pulling the PPP_1661 from the tx l2hdr data */

                size = sizeof(uint16_t); /* sizeof(PPP_1661 header) */

                PROTODBG(protoPrint, "%s INSERT: ix <%d> offset <%u> length <%u>\n",
                         __FUNCTION__, (int)cmdListIx, rxOffset, size);
                cmdSize = iopDma_CmdListAddInsert(&cmdList[cmdListIx],
                                                  rxOffset, txData_p, size);
                if(cmdSize == FAP_ERROR)
                {
                    protoError("Command List Error (%d)", __LINE__);
                    return FAP_ERROR;
                }
                cmdListIx += cmdSize;

                CHECK_CMD_LIST(cmdList, cmdListIx);

                txData_p += size;

                /* Check that PPPoE hdr is the last hdr in tx L2 */
                if((ix + 1) != blog_p->tx.count)
                {
                    protoError("Command List Error (%d)", __LINE__);
                    return FAP_ERROR;
                }
            }
            break;

            default:
            {
                protoError("Invalid encapsulation <%d>", blog_p->tx.encap[ix]);
                return FAP_ERROR;
            }
        } /* loop through TX L2 headers */
    }

    if(blog_p->rx.info.bmap.BCM_XPHY)  /* Received from SAR */
    {
        rxL2Len += rfc2684HdrLength[blog_p->rx.info.phyHdrLen];

        /* Modify txAdjust by the same amount (PPPoA LLC bug) - June 2010 */
        flowInfo_p->txAdjust += rfc2684HdrLength[blog_p->rx.info.phyHdrLen];
    }

    if(rxL2Len != 0)
    {
        protoDebug("Delete Rx L2 headers");

        /*** Delete Rx L2 headers, including BRCM_TAG, up to IP offset ***/
        size = rxL2Len;

        PROTODBG(protoPrint, "%s DELETE CMD: ix <%d> offset <%u> length <%u>\n",
                 __FUNCTION__, (int)cmdListIx, rxOffset, size);
        cmdSize = iopDma_CmdListAddDelete(&cmdList[cmdListIx],
                                          rxOffset, size);
        if(cmdSize == FAP_ERROR)
        {
            protoError("Command List Error (%d)", __LINE__);
            return FAP_ERROR;
        }
        cmdListIx += cmdSize;

        CHECK_CMD_LIST(cmdList, cmdListIx);
    }
    else
    {
        protoDebug("No Rx L2 headers to Delete");
    }

    return cmdListIx;
}

static int32 addL3Commands(fap4kePkt_flowInfo_t *flowInfo_p, uint8 *cmdList, int32 cmdListIx, Blog_t *blog_p)
{
    BlogTuple_t *rxIp_p;
    BlogTuple_t *txIp_p;
    uint32 ipOffset;
    uint16 offset;
    uint8 len;
    int32 cmdSize;
#if 0
    int32 cmdListIxOrig = cmdListIx;
#endif

    ipOffset = blog_p->rx.length;

    if(blog_p->rx.info.bmap.BCM_XPHY)  /* Received from SAR */
    {
        /* Modify ipOffset by rfc2684HdrLength (PPPoA LLC bug) - June 2010 */
        ipOffset += rfc2684HdrLength[blog_p->rx.info.phyHdrLen];
    }

    rxIp_p = &blog_p->rx.tuple;
    txIp_p = &blog_p->tx.tuple;

    flowInfo_p->tunnelType = FAP4KE_TUNNEL_NONE;

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
    /* Do IPv6 Layer 3 changes first */
    /* Followed by IPv4 Layer 3 changes if T4in6DN or T4in6UP */

    if ( T4in6DN(blog_p) )  /* rx IPv6:IPv4; tx IPv4 */
    {
        int valSz;
        uint16 ethType = ETH_P_IP;

        if(blog_p->tos_mode_ds == BLOG_TOS_INHERIT)
        {
            protoDebug("T4in6DN: Inherit Tos");

            flowInfo_p->inheritTos = 1;
        }

        /* Replace IPv6 etherType with IPv4 etherType */
        ipOffset -= ETHERTYPE_LENGTH;
        len = ETHERTYPE_LENGTH;
        PROTODBG(protoPrint, "%s REPLACE: ix <%d> offset <%u> length <%u>\n",
                 __FUNCTION__, (int)cmdListIx, (unsigned int)ipOffset, (unsigned int)len);

        cmdSize = iopDma_CmdListAddReplace(&cmdList[cmdListIx],
                                           ipOffset, (uint8 *)(&ethType), len);
        if(cmdSize == FAP_ERROR)
        {
            protoError("Command List Error (%d)", __LINE__);
            return FAP_ERROR;
        }
        cmdListIx += cmdSize;
        ipOffset += ETHERTYPE_LENGTH;

        CHECK_CMD_LIST(cmdList, cmdListIx);

        /* Remove the entire IPv6 hdr */
        valSz = sizeof(fap4kePkt_ipv6Header_t);

        PROTODBG(protoPrint, "%s DELETE CMD: ix <%d> offset <%u> length <%u>\n",
                 __FUNCTION__, (int)cmdListIx, (unsigned int)ipOffset, (unsigned int)valSz);
        cmdSize = iopDma_CmdListAddDelete(&cmdList[cmdListIx],
                                          ipOffset, valSz);
        if(cmdSize == FAP_ERROR)
        {
            protoError("Command List Error (%d)", __LINE__);
            return FAP_ERROR;
        }
        cmdListIx += cmdSize;
        flowInfo_p->txAdjust += valSz;

        flowInfo_p->ipv4.tuple.tosOrig = rxIp_p->tos;

        /* adjust ipOffset to start of IPv4 hdr in the rx packet */
        ipOffset += sizeof(fap4kePkt_ipv6Header_t);

        CHECK_CMD_LIST(cmdList, cmdListIx);

        flowInfo_p->tunnelType = FAP4KE_TUNNEL_4in6_DN;
    } 
    else if ( T4in6UP(blog_p) ) /* rx IPv4; tx IPv6:IPv4 */
    {
        uint8 * data_p;
        int valSz;
        uint16 data;

        if(blog_p->tos_mode_us == BLOG_TOS_INHERIT)
        {
            protoDebug("T4in6UP: Inherit Tos");

            flowInfo_p->inheritTos = 1;
        }
        else
        {
            protoDebug("T4in6UP: Fixed ToS <%u>", PKT_IPV6_GET_TOS_WORD(blog_p->tupleV6.word0));
        }

        /* Insert the IPv6 hdr from blog.tupleV6 adjusting payload_len */
        valSz = OFFSETOF(ipv6hdr, payload_len);
        data_p = (uint8 *)&blog_p->tupleV6.word0;

        PROTODBG(protoPrint, "%s INSERT: ix <%d> offset <%u> length <%u>\n",
                  __FUNCTION__, (int)cmdListIx, (unsigned int)ipOffset, (unsigned int)valSz);
        cmdSize = iopDma_CmdListAddInsert(&cmdList[cmdListIx],
                                          ipOffset, data_p, valSz);
        if(cmdSize == FAP_ERROR)
        {
            protoError("Command List Error (%d)", __LINE__);
            return FAP_ERROR;
        }
        cmdListIx += cmdSize;

        CHECK_CMD_LIST(cmdList, cmdListIx);

        data_p += valSz;

        /*
         * Insert IPv6 payload_len field
         * ipv6hdr.payload_len = ip_hdr.tot_len
         * The PDU length will be set to ip_hdr.tot_len in the FAP, for each packet
         * We have to set the PPP header length as a constant (0) in this command
         */
        valSz = sizeof(uint16_t); /* payload_len length */
        data  = 0; 

        PROTODBG(protoPrint, "%s INSERT LENGTH: ix <%d> offset <%u> data <%u>\n",
                 __FUNCTION__, (int)cmdListIx, (unsigned int)ipOffset, (unsigned int)valSz);
        cmdSize = iopDma_CmdListAddInsertLength(&cmdList[cmdListIx],
                                                ipOffset, data);
        if(cmdSize == FAP_ERROR)
        {
             protoError("Command List Error (%d)", __LINE__);
             return FAP_ERROR;
        }
        cmdListIx += cmdSize;

        CHECK_CMD_LIST(cmdList, cmdListIx);

        data_p += sizeof(uint16_t);

        /* Insert the remainder of the IPv6 hdr */
        valSz = sizeof(fap4kePkt_ipv6Header_t) - OFFSETOF(ipv6hdr, nexthdr);

        PROTODBG(protoPrint, "%s INSERT: ix <%d> offset <%u> length <%u>\n",
                  __FUNCTION__, (int)cmdListIx, (unsigned int)ipOffset, (unsigned int)valSz);
        cmdSize = iopDma_CmdListAddInsert(&cmdList[cmdListIx],
                                          ipOffset, data_p, valSz);
        if(cmdSize == FAP_ERROR)
        {
            protoError("Command List Error (%d)", __LINE__);
            return FAP_ERROR;
        }
        cmdListIx += cmdSize;
        flowInfo_p->txAdjust -= sizeof(fap4kePkt_ipv6Header_t);    /* increasing the size of the packet */

        CHECK_CMD_LIST(cmdList, cmdListIx);

        flowInfo_p->tunnelType = FAP4KE_TUNNEL_4in6_UP;

        flowInfo_p->ipv4.tuple.tosOrig = rxIp_p->tos;

        /* ipOffset already points to start of IPv4 hdr in rx packet */
    } 
    else if(T6in4DN(blog_p))    /* rx IPv4:IPv6; tx IPv6 */
    {
        int valSz;
        uint16 ethType = ETH_P_IPV6;

        if(blog_p->tos_mode_ds == BLOG_TOS_INHERIT)
        {
            protoDebug("T6in4DN: Inherit Tos");

            flowInfo_p->inheritTos = 1;
        }
        else if(blog_p->rx.tuple.tos != PKT_IPV6_GET_TOS_WORD(blog_p->tupleV6.word0))
        {
            flowInfo_p->ipv6.tuple.flags.mangleTos = 1;
            flowInfo_p->ipv6.tuple.flags.tos = PKT_IPV6_GET_TOS_WORD(blog_p->tupleV6.word0);

            protoDebug("Mangle IPv6 Traffic Class %u", flowInfo_p->ipv6.tuple.flags.tos);
        }
        flowInfo_p->ipv6.tuple.tosOrig = rxIp_p->tos;

        /* Step1: Replace IPv4 etherType with IPv6 etherType */
        ipOffset -= ETHERTYPE_LENGTH;
        len = ETHERTYPE_LENGTH;
        PROTODBG(protoPrint, "%s REPLACE: ix <%d> offset <%u> length <%u>\n",
                 __FUNCTION__, (int)cmdListIx, (unsigned int)ipOffset, (unsigned int)len);

        cmdSize = iopDma_CmdListAddReplace(&cmdList[cmdListIx],
                                           ipOffset, (uint8 *)(&ethType), len);
        if(cmdSize == FAP_ERROR)
        {
            protoError("Command List Error (%d)", __LINE__);
            return FAP_ERROR;
        }
        cmdListIx += cmdSize;
        ipOffset += ETHERTYPE_LENGTH;

        CHECK_CMD_LIST(cmdList, cmdListIx);

        /* Delete the entire IPv4 hdr */
        valSz = sizeof(struct iphdr);

        PROTODBG(protoPrint, "%s DELETE CMD: ix <%d> offset <%u> length <%u>\n",
                 __FUNCTION__, (int)cmdListIx, (unsigned int)ipOffset, (unsigned int)valSz);
        cmdSize = iopDma_CmdListAddDelete(&cmdList[cmdListIx],
                                          ipOffset, valSz);
        if(cmdSize == FAP_ERROR)
        {
            protoError("Command List Error (%d)", __LINE__);
            return FAP_ERROR;
        }
        cmdListIx += cmdSize;
        flowInfo_p->txAdjust += valSz;

        /* adjust ipOffset to start of IPv6 hdr in the rx packet */
        ipOffset += sizeof(struct iphdr);

        CHECK_CMD_LIST(cmdList, cmdListIx);

        flowInfo_p->tunnelType = FAP4KE_TUNNEL_6in4_DN;
    }
    else if(T6in4UP(blog_p))    /* rx IPv6; tx IPv4:IPv6 */
    {
        /* Insert IPv4 Header, checksum cmdlist is generated in setFwdAndFilters() */
        struct iphdr ipv4Hdr;
        int valSz;

        if(blog_p->tos_mode_us == BLOG_TOS_INHERIT)
        {
            protoDebug("T6in4UP: Inherit Tos");

            flowInfo_p->inheritTos = 1;
        }
        else
        {
            protoDebug("T6in4UP: Fixed ToS <%u>", blog_p->tx.tuple.tos);
        }

        ipv4Hdr.version = 4;
        ipv4Hdr.ihl = 5;
        ipv4Hdr.tos = blog_p->tx.tuple.tos;
        flowInfo_p->ipv6.tuple.tosOrig = rxIp_p->tos;

        /* Insert IPv4 Header till length field */ 
        valSz = OFFSETOF(iphdr, tot_len);
        PROTODBG(protoPrint, "%s INSERT LENGTH: ix <%d> offset <%u> data <%u>\n",
                 __FUNCTION__, (int)cmdListIx, (unsigned int)ipOffset, (unsigned int)valSz);
        cmdSize = iopDma_CmdListAddInsert(&cmdList[cmdListIx],
                                           ipOffset, (uint8 *)(&ipv4Hdr), valSz);
        if(cmdSize == FAP_ERROR)
        {
            protoError("Command List Error (%d)", __LINE__);
            return FAP_ERROR;
        }
        cmdListIx += cmdSize;
        CHECK_CMD_LIST(cmdList, cmdListIx);

        /* Insert Length Field of IPv4 Header */
        valSz =  sizeof(struct iphdr) + sizeof(struct ipv6hdr);
        PROTODBG(protoPrint, "%s INSERT LENGTH: ix <%d> offset <%u> data <%u>\n",
                 __FUNCTION__, (int)cmdListIx, (unsigned int)ipOffset, (unsigned int)valSz);
        cmdSize = iopDma_CmdListAddInsertLength(&cmdList[cmdListIx],
                                                ipOffset, (uint16)valSz);
        if(cmdSize == FAP_ERROR)
        {
             protoError("Command List Error (%d)", __LINE__);
             return FAP_ERROR;
        }
        cmdListIx += cmdSize;
        CHECK_CMD_LIST(cmdList, cmdListIx);

        /* Insert remaining fields of IPv4 Header */
        ipv4Hdr.id = blog_p->tupleV6.ipid;
        ipv4Hdr.frag_off = blog_p->tupleV6.fragflag?  BLOG_IP_FLAG_DF : 0;
        ipv4Hdr.ttl = blog_p->tx.tuple.ttl;
        ipv4Hdr.protocol = BLOG_IPPROTO_IPV6;
        ipv4Hdr.check = 0;
        memcpy(&(ipv4Hdr.saddr), &blog_p->tx.tuple.saddr, sizeof(blog_p->tx.tuple.saddr) + sizeof(blog_p->tx.tuple.daddr));

        valSz = sizeof(struct iphdr) - OFFSETOF(iphdr, id);

        PROTODBG(protoPrint, "%s INSERT LENGTH: ix <%d> offset <%u> data <%u>\n",
                 __FUNCTION__, (int)cmdListIx, (unsigned int)ipOffset, (unsigned int)valSz);
        cmdSize = iopDma_CmdListAddInsert(&cmdList[cmdListIx],
                                           ipOffset, (uint8 *)(&(ipv4Hdr.id)), valSz);
        if(cmdSize == FAP_ERROR)
        {
            protoError("Command List Error (%d)", __LINE__);
            return FAP_ERROR;
        }
        cmdListIx += cmdSize;
        CHECK_CMD_LIST(cmdList, cmdListIx);

        flowInfo_p->txAdjust -= sizeof(struct iphdr);

        flowInfo_p->tunnelType = FAP4KE_TUNNEL_6in4_UP;

        return(cmdListIx);
    }
    else if(blog_p->rx.info.bmap.PLD_IPv6)
    {   /* rx IPv6; tx IPv6 */

        if(blog_p->tupleV6.rx_hop_limit != blog_p->tupleV6.tx_hop_limit)
            flowInfo_p->ipv6.tuple.flags.isRouted = 1;

        /* Note: Rx IPv6 TOS is stored in the blog ipv4 tuple to save memory */
        flowInfo_p->ipv6.tuple.tosOrig = rxIp_p->tos;
        if (( blog_p->rx.tuple.tos != PKT_IPV6_GET_TOS_WORD(blog_p->tupleV6.word0) ))
        {
            flowInfo_p->ipv6.tuple.flags.mangleTos = 1;
            flowInfo_p->ipv6.tuple.flags.tos = PKT_IPV6_GET_TOS_WORD(blog_p->tupleV6.word0);

            protoDebug("Mangle IPv6 Traffic Class %u", flowInfo_p->ipv6.tuple.flags.tos);
        }
    }

    if(blog_p->tx.info.bmap.PLD_IPv4 == 0) /* tx is IPv6  */
        return(cmdListIx);   /* L3 IPv6 fields taken care of. OK to return */
    
#endif
    /* also need to fix mangle_ip6tos above for this case */

    flowInfo_p->ipv4.tuple.tosOrig = rxIp_p->tos;
    if ((rxIp_p->tos != txIp_p->tos) &&
        (!T4in6DN(blog_p) || blog_p->tos_mode_ds == BLOG_TOS_FIXED))
    {
        flowInfo_p->ipv4.tuple.flags.mangleTos = 1;
        flowInfo_p->ipv4.tuple.flags.tos = txIp_p->tos;

        protoDebug("Mangle IPv4 ToS %u", flowInfo_p->ipv4.tuple.flags.tos);
    }

    if(rxIp_p->saddr != txIp_p->saddr)
    {
        protoDebug("Replace IpSa");

        offset = ipOffset + OFFSETOF(iphdr, saddr);
        len = sizeof(uint32);

        PROTODBG(protoPrint, "%s REPLACE: ix <%d> offset <%u> length <%u>\n",
                 __FUNCTION__, (int)cmdListIx, offset, len);

        cmdSize = iopDma_CmdListAddReplace(&cmdList[cmdListIx],
                                           offset, (uint8 *)(&txIp_p->saddr), len);
        if(cmdSize == FAP_ERROR)
        {
            protoError("Command List Error (%d)", __LINE__);
            return FAP_ERROR;
        }
        cmdListIx += cmdSize;

        CHECK_CMD_LIST(cmdList, cmdListIx);
    }

    if(rxIp_p->daddr != txIp_p->daddr)
    {
        protoDebug("Replace IpDa");

        offset = ipOffset + OFFSETOF(iphdr, daddr);
        len = sizeof(uint32);

        PROTODBG(protoPrint, "%s REPLACE: ix <%d> offset <%u> length <%u>\n",
                 __FUNCTION__, (int)cmdListIx, offset, len);

        cmdSize = iopDma_CmdListAddReplace(&cmdList[cmdListIx],
                                           offset, (uint8 *)(&txIp_p->daddr), len);
        if(cmdSize == FAP_ERROR)
        {
            protoError("Command List Error (%d)", __LINE__);
            return FAP_ERROR;
        }
        cmdListIx += cmdSize;

        CHECK_CMD_LIST(cmdList, cmdListIx);
    }

    if(rxIp_p->port.source != txIp_p->port.source)
    {
        protoDebug("Replace Sport");

        offset = ipOffset + sizeof(struct iphdr) + OFFSETOF(tcphdr, source);
        len = sizeof(uint16);

        PROTODBG(protoPrint, "%s REPLACE: ix <%d> offset <%u> length <%u>\n",
                 __FUNCTION__, (int)cmdListIx, offset, len);

        cmdSize = iopDma_CmdListAddReplace(&cmdList[cmdListIx],
                                           offset, (uint8 *)(&txIp_p->port.source), len);
        if(cmdSize == FAP_ERROR)
        {
            protoError("Command List Error (%d)", __LINE__);
            return FAP_ERROR;
        }
        cmdListIx += cmdSize;

        CHECK_CMD_LIST(cmdList, cmdListIx);
    }

    if(rxIp_p->port.dest != txIp_p->port.dest)
    {
        protoDebug("Replace Dport");

        offset = ipOffset + sizeof(struct iphdr) + OFFSETOF(tcphdr, dest);
        len = sizeof(uint16);

        PROTODBG(protoPrint, "%s REPLACE: ix <%d> offset <%u> length <%u>\n",
                 __FUNCTION__, (int)cmdListIx, offset, len);

        cmdSize = iopDma_CmdListAddReplace(&cmdList[cmdListIx],
                                           offset, (uint8 *)(&txIp_p->port.dest), len);
        if(cmdSize == FAP_ERROR)
        {
            protoError("Command List Error (%d)", __LINE__);
            return FAP_ERROR;
        }
        cmdListIx += cmdSize;

        CHECK_CMD_LIST(cmdList, cmdListIx);
    }
    
#if 0
    if(cmdListIxOrig == cmdListIx)
    {
        protoDebug("Only NATed flows supported");
        return FAP_ERROR;
    }
#endif

    return cmdListIx;
}

#if defined(CC_FAP4KE_PKT_HW_ICSUM)
static int genChecksumCmdList(uint8 *checksum, uint16 offset, uint16 icsum)
{
    int32 cmdSize;
    int32 checksumIx = 0;

    cmdSize = iopDma_ChecksumCmdListAdd(&checksum[checksumIx],
                                        IOPDMA_HCS_OPCODE_FIRST_WITH_CONST,
                                        0, icsum);
    if(cmdSize == FAP_ERROR)
    {
        protoError("Command List Error (%d)", __LINE__);

        return FAP_ERROR;
    }
    checksumIx += cmdSize;


    cmdSize = iopDma_ChecksumCmdListAdd(&checksum[checksumIx],
                                        IOPDMA_HCS_OPCODE_CONTINUE,
                                        offset, 2);
    if(cmdSize == FAP_ERROR)
    {
        protoError("Command List Error (%d)", __LINE__);

        return FAP_ERROR;
    }
    checksumIx += cmdSize;

    cmdSize = iopDma_ChecksumCmdListAdd(&checksum[checksumIx],
                                        IOPDMA_HCS_OPCODE_LAST,
                                        offset, 0);
    if(cmdSize == FAP_ERROR)
    {
        protoError("Command List Error (%d)", __LINE__);

        return FAP_ERROR;
    }
    checksumIx += cmdSize;

    cmdSize = iopDma_ChecksumCmdListAdd(&checksum[checksumIx],
                                        IOPDMA_HCS_OPCODE_END,
                                        0, 0);
    if(cmdSize == FAP_ERROR)
    {
        protoError("Command List Error (%d)", __LINE__);

        return FAP_ERROR;
    }
    checksumIx += cmdSize;

    if(checksumIx >= FAP4KE_PKT_CSUM_CMD_LIST_SIZE)
    {
        protoError("Checksum Command List Overflow: checksumIx <%d>", checksumIx);

        return FAP_ERROR;
    }

    return FAP_SUCCESS;
}
#endif /* CC_FAP4KE_PKT_HW_ICSUM */

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
static int buildIPv4CsumList(uint8 *csumList, uint32 ipOffset)
{
    int csumListIx;
    int cmdSize;
    const uint32 csumOffset = OFFSETOF(iphdr, check);

    csumListIx = 0;

    /* run checksum until HCS field */
    cmdSize = iopDma_ChecksumCmdListAdd(&csumList[csumListIx],
                                        IOPDMA_HCS_OPCODE_FIRST,
                                        ipOffset, csumOffset);
    if(cmdSize == FAP_ERROR)
    {
        protoError("Command List Error (%d)", __LINE__);
        return FAP_ERROR;
    }
    csumListIx += cmdSize;

    /* continue checksum claculation, skipping over the HCS field */
    cmdSize = iopDma_ChecksumCmdListAdd(&csumList[csumListIx],
                                        IOPDMA_HCS_OPCODE_CONTINUE,
                                        ipOffset + csumOffset + 2,
                                        sizeof(struct iphdr) - csumOffset - 2);
    if(cmdSize == FAP_ERROR)
    {
        protoError("Command List Error (%d)", __LINE__);
        return FAP_ERROR;
    }
    csumListIx += cmdSize;

    /* Update the HCS field with the calculated checksum */
    cmdSize = iopDma_ChecksumCmdListAdd(&csumList[csumListIx],
                                        IOPDMA_HCS_OPCODE_LAST,
                                        ipOffset + csumOffset, 0);
    if(cmdSize == FAP_ERROR)
    {
        protoError("Command List Error (%d)", __LINE__);
        return FAP_ERROR;
    }
    csumListIx += cmdSize;

    cmdSize = iopDma_ChecksumCmdListAdd(&csumList[csumListIx],
                                        IOPDMA_HCS_OPCODE_END,
                                        0, 0);
    if(cmdSize == FAP_ERROR)
    {
        protoError("Command List Error (%d)", __LINE__);
        return FAP_ERROR;
    }
    csumListIx += cmdSize;

    if(csumListIx > FAP4KE_PKT_CSUM_CMD_LIST_SIZE)
    {
        protoError("Checksum Command List Overflow: csumListIx <%d>", csumListIx);
        return FAP_ERROR;
    }

    return FAP_SUCCESS;
}
#endif

static int32 setFwdAndFilters(fap4kePkt_flowInfo_t *flowInfo_p, uint8 *cmdList, int32 cmdListIx,
                              uint8 **checksum1_p, uint8 **checksum2_p, Blog_t *blog_p)
{
    uint8 isRouted=0;

    /* set destination */
    if(blog_p->tx.info.bmap.BCM_XPHY)
    {
        /* Send to SAR */
        flowInfo_p->dest.phy = FAP4KE_PKT_PHY_XTM;
        flowInfo_p->dest.channel = blog_p->tx.info.channel;
    }
    else if (blog_p->tx.info.phyHdrType == BLOG_WLANPHY)
    {
        flowInfo_p->dest.phy = FAP4KE_PKT_PHY_WLAN;
        flowInfo_p->dest.channel = blog_p->tx.info.channel;
    }
    else
    {
        /* Send to ETH */
        flowInfo_p->dest.phy = FAP4KE_PKT_PHY_ENET;
#if defined(CONFIG_BCM_PORTS_ON_INT_EXT_SW)  /* Internal and external switch */
        if (IsExternalSwitchPort(blog_p->tx.info.channel))
        {   /* Is the destination port on external switch ? 
             * If yes - then dest.channelMask is the connected port to ext_sw.*/
            int extSwConnPort = BpGetPortConnectedToExtSwitch();
            flowInfo_p->dest.channelMask = 1 << extSwConnPort;
        } 
        else
#endif
        {   /* Internal switch port or (external switch port w/o internal-tag);*/
            flowInfo_p->dest.channelMask = (1 << LOGICAL_PORT_TO_PHYSICAL_PORT(blog_p->tx.info.channel));
        }
    }
    flowInfo_p->dest.queue   = (uint8_t)(SKBMARK_GET_Q_PRIO(blog_p->mark));   //blog_p->priority;

    /* set source */
    if(blog_p->rx.info.bmap.BCM_XPHY)
    {
        /* Received from SAR */
        flowInfo_p->source.phy = FAP4KE_PKT_PHY_XTM;
    }
    else
    {
        /* Received from ETH */
        flowInfo_p->source.phy = FAP4KE_PKT_PHY_ENET;
    }
    flowInfo_p->source.queue = 0;
    flowInfo_p->source.channel = blog_p->rx.info.channel;

    flowInfo_p->source.tcp_pure_ack = blog_p->key.tcp_pure_ack;

    /* Check if the flow is routed or bridged */
    isRouted = 0;
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
    if (CHK4in6(blog_p) || CHK6in4(blog_p)) 
    {
        isRouted = 1;
    }
    else if(CHK6to6(blog_p))
    {
        if (blog_p->tupleV6.rx_hop_limit != blog_p->tupleV6.tx_hop_limit)
            isRouted = 1;
    }
    else
#endif
    if (CHK4to4(blog_p))
    {
        if (blog_p->rx.tuple.ttl != blog_p->tx.tuple.ttl)
        {
            isRouted = 1;
        }
    }
    else
    {
        protoError("Unable to determine if the flow is routed/bridged (%d)", __LINE__);

        return FAP_ERROR;
    }

    if(isRouted) /* Routed */
    {
        uint32 csum;
#if defined(CC_FAP4KE_PKT_HW_ICSUM) || defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
        int32 cmdSize;
#endif
#if defined(CC_FAP4KE_PKT_HW_ICSUM)
        int ret;
        uint16 ipOffset = blog_p->tx.length;
        uint16 ipChecksumOffset = ipOffset + OFFSETOF(iphdr, check);
        uint32 tuChecksumOffset = ipOffset + sizeof(struct iphdr);
        uint16 icsum;
#endif /* CC_FAP4KE_PKT_HW_ICSUM */


        /* FlowCache builds incremental checksums for L3 and L4 */

        csum  = (__force uint32_t)blog_p->rx.tuple.check;
        csum += (__force uint32_t)htons(0x0100);
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
        if (flowInfo_p->type == FAP4KE_PKT_FT_IPV6)
        {
            flowInfo_p->ipv6.tuple.flags.isRouted = 1;
            flowInfo_p->ipv6.tuple.icsum.ip = (__force __sum16)(csum + (csum >= 0xFFFF));
            flowInfo_p->ipv6.tuple.icsum.tu = blog_p->tx.tuple.check; /* Precomputed incremental L4 checksum */

            protoDebug("icsum.ipv6 : 0x%X / 0x%X\n", flowInfo_p->ipv6.tuple.icsum.ip, (__force uint32_t)blog_p->rx.tuple.check);
        }
        else
#endif
        {
            flowInfo_p->ipv4.tuple.flags.isRouted = 1;
            flowInfo_p->ipv4.tuple.icsum.ip = (__force __sum16)(csum + (csum >= 0xFFFF));
            flowInfo_p->ipv4.tuple.icsum.tu = blog_p->tx.tuple.check; /* Precomputed incremental L4 checksum */

            protoDebug("icsum.ipv4 : 0x%X / 0x%X\n", flowInfo_p->ipv4.tuple.icsum.ip, (__force uint32_t)blog_p->rx.tuple.check);
        }

#if defined(CC_FAP4KE_PKT_HW_ICSUM)
        if(blog_p->tx.info.bmap.PLD_IPv4)
        {
            /* add checksum1 command placeholder */
            cmdSize = iopDma_CmdListAddChecksum1(&cmdList[cmdListIx], 0);
            if(cmdSize == FAP_ERROR)
            {
                protoError("Command List Error (%d)", __LINE__);

                return FAP_ERROR;
            }
            cmdListIx += cmdSize;

            CHECK_CMD_LIST(cmdList, cmdListIx);

            /* generate IP checksum command list */

            /* FIXME: icsum 1's complement for 6362 only!!! */
            icsum = ~flowInfo_p->ipv4.tuple.icsum.ip;
            ret = genChecksumCmdList(*checksum1_p, ipChecksumOffset, icsum);
            if(ret == FAP_ERROR)
            {
                protoError("Checksum Command List Error (%d)", __LINE__);

                return FAP_ERROR;
            }
        }
        else
        {
            *checksum1_p = NULL;
        }

        if(blog_p->tx.info.bmap.PLD_IPv4)
        {
            /* FIXME: icsum 1's complement for 6362 only!!! */
            icsum = ~flowInfo_p->ipv4.tuple.icsum.tu;
        }
        else
        {
            /* FIXME: icsum 1's complement for 6362 only!!! */
            icsum = ~flowInfo_p->ipv6.tuple.icsum.tu;
        }

        if (blog_p->key.protocol == IPPROTO_TCP ||
            (blog_p->key.protocol == IPPROTO_UDP && icsum != 0xFFFF))
        {
            /* add checksum2 command placeholder */
            cmdSize = iopDma_CmdListAddChecksum2(&cmdList[cmdListIx], 0);
            if(cmdSize == FAP_ERROR)
            {
                protoError("Command List Error (%d)", __LINE__);

                return FAP_ERROR;
            }
            cmdListIx += cmdSize;

            CHECK_CMD_LIST(cmdList, cmdListIx);

            /* generate TCP/UDP checksum command list */
            if (blog_p->key.protocol == IPPROTO_TCP)
            {
                tuChecksumOffset += OFFSETOF(tcphdr, check);
            }
            else
            {
                tuChecksumOffset += OFFSETOF(udphdr, check);
            }

            ret = genChecksumCmdList(*checksum2_p, tuChecksumOffset, icsum);
            if(ret == FAP_ERROR)
            {
                protoError("Checksum Command List Error (%d)", __LINE__);

                return FAP_ERROR;
            }
        }
        else
        {
            *checksum2_p = NULL;
        }
#else
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
        if(T6in4UP(blog_p))
        {   /* We need to calculate the IP header for inserted IPv4 header */
            cmdSize = iopDma_CmdListAddChecksum1(&cmdList[cmdListIx], 0);
            if(cmdSize == FAP_ERROR)
            {
                protoError("Command List Error (%d)", __LINE__);
                return FAP_ERROR;
            }
            cmdListIx += cmdSize;
            CHECK_CMD_LIST(cmdList, cmdListIx);

            /* build IP checksum command list */
            if( FAP_ERROR == buildIPv4CsumList(*checksum1_p, blog_p->tx.length))
            {
                protoError("Checksum Command List Error (%d)", __LINE__);
                return FAP_ERROR;
            }
        }
        else
#endif
        {
            *checksum1_p = NULL;
        }
        *checksum2_p = NULL;
#endif /* CC_FAP4KE_PKT_HW_ICSUM */
    }
    else
    {
        /* Bridged flow */

        if(blog_p->rx.tuple.tos != blog_p->tx.tuple.tos)
        {
            /* ToS Mangling, need to re-compute IP checksum */

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
            if (flowInfo_p->type == FAP4KE_PKT_FT_IPV6)
            {
                flowInfo_p->ipv6.tuple.flags.mangleTos = 1;
                flowInfo_p->ipv6.tuple.icsum.ip = blog_p->rx.tuple.check;
            }
            else
#endif
            {
                flowInfo_p->ipv4.tuple.flags.mangleTos = 1;
                flowInfo_p->ipv4.tuple.icsum.ip = blog_p->rx.tuple.check;
            }
        }
        /* No Checksum to calculate - incremental checksum will be applied */
        *checksum1_p = NULL;
        *checksum2_p = NULL;
    }

    /* L4 protocol is set in the Blog Key for both IPv4 and IPv6 */
    flowInfo_p->source.protocol = blog_p->key.protocol;

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
    if( blog_p->rx.info.bmap.PLD_IPv6 && !(T4in6DN(blog_p)) )
    {
        memcpy(flowInfo_p->ipv6.tuple.ipSa6.u8, blog_p->tupleV6.saddr.p8, sizeof(ip6_addr_t));
        memcpy(flowInfo_p->ipv6.tuple.ipDa6.u8, blog_p->tupleV6.daddr.p8, sizeof(ip6_addr_t));
        flowInfo_p->ipv6.tuple.sPort = blog_p->tupleV6.port.source;
        flowInfo_p->ipv6.tuple.dPort = blog_p->tupleV6.port.dest;
        if(T6in4DN(blog_p))
        {
            /* RFC 5969: Section(s): 9.2, 12 
             * 6rd tunnels: check if tunnel saddr is the same as it was learnt */
            flowInfo_p->ipv6.tuple.tunnelIpSa4 = blog_p->rx.tuple.saddr;

            protoDebug("6rd: tunnelIpSa4 %pI4", &flowInfo_p->ipv6.tuple.tunnelIpSa4);
        }
    }
    else
#endif
    {
        flowInfo_p->ipv4.tuple.ipSa4 = blog_p->rx.tuple.saddr;
        flowInfo_p->ipv4.tuple.ipDa4 = blog_p->rx.tuple.daddr;
        flowInfo_p->ipv4.tuple.sPort = blog_p->rx.tuple.port.source;
        flowInfo_p->ipv4.tuple.dPort = blog_p->rx.tuple.port.dest;
    }

    return cmdListIx;
}

#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
/*
 *------------------------------------------------------------------------------
 * Flow cache interface to ARL hardware
 *------------------------------------------------------------------------------
 */
/*
 *------------------------------------------------------------------------------
 * Function   : fap_clear
 *              ARL to FAP downcall function.
 * Description: Clear FAP entry(s) association with HW. 
 * Parameters :
 *      mcast : multicast group address
 *      port  : port identifiers
 *
 * Returns    : Number of associations cleared.
 *------------------------------------------------------------------------------
 */
static int fap_clear( uint32_t mcast, uint32_t port_map)
{
    return FAP_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function     : fap_bind_arl
 * Description  : Binds the ARL hooks to activate, deactivate and refresh.
 *------------------------------------------------------------------------------
 */

void fap_bind_arl(HOOKP activate_fn, HOOK4PARM deactivate_fn,
                  HOOK3PARM refresh_fn, HOOK32 reset_stats_fn, HOOK32 clear_fn, 
                  FAP_CLEAR_HOOK *fap_clear_fn)
{
    fapState_g.fap_activate_arl_hook   = activate_fn;  /* upcall hook into ARL */
    fapState_g.fap_deactivate_arl_hook = deactivate_fn;/* upcall hook into ARL */
    fapState_g.fap_refresh_arl_hook    = refresh_fn;   /* upcall hook into ARL */
    fapState_g.fap_reset_stats_arl_hook= reset_stats_fn;/* upcall hook into ARL */
    fapState_g.fap_clear_arl_hook      = clear_fn;     /* upcall hook into ARL */

    if ( activate_fn != (HOOKP) NULL )
        *fap_clear_fn = fap_clear;                /* downcall hook from FHW */
    else
        *fap_clear_fn = (FC_CLEAR_HOOK)NULL;

}
#endif

#if defined(CONFIG_BCM_FAP_LAYER2)
/*
 *------------------------------------------------------------------------------
 * Function:
 *   void fap_bind_vlanctl(int enable_flag)
 * Description:
 *   Permits manual enabling|disabling of vlanctl bind for FAP.
 * Parameter:
 *   To enable:    enable_flag > 0
 *   To disable:   enable_flag = 0
 *------------------------------------------------------------------------------
 */
void fap_bind_vlanctl(int enable_flag)
{
    vlanctl_bind_t hook_info;

    if (enable_flag)
    {
        hook_info.hook_info = 0xFF;
        /* Bind FAP handlers */
        vlanctl_bind_config((vlanctl_bind_ScHook_t)fap_config, (vlanctl_bind_SdHook_t)fap_deconf, 
            (vlanctl_bind_SnHook_t)NULL, VLANCTL_BIND_CLIENT_FAP, hook_info);
        protoInfo("FAP vlanctl bind enabled.");
    }
    else
    {
        hook_info.hook_info = 0xFF;
        /* Unbind to vlanctl */
        vlanctl_bind_config((vlanctl_bind_ScHook_t)NULL, (vlanctl_bind_SdHook_t)NULL,
            (vlanctl_bind_SnHook_t)NULL, VLANCTL_BIND_CLIENT_FAP, hook_info);
        protoInfo("FAP vlanctl bind disabled.");
   }

}  /* fap_bind_vlanctl() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   uint32_t fap_config(Blog_t * blog_p, BlogTraffic_t traffic)
 * Description:
 *   A kernel module creates one blog at a time and fills in 
 *   necessary fields to construct a flow hash. This function will
 *   not only create/hashin a flow based on the blog but also 
 *   activate HW acceleration immediately. Currently, this function
 *   only supports layer 2 flow acceleration.
 * Parameter:
 *   blog_p:   pointer to a blog.
 *   traffic:  traffic type.
 * Returns:
 *   blog key
 *------------------------------------------------------------------------------
 */
uint32_t fap_config(Blog_t * blog_p, BlogTraffic_t traffic)
{
   uint32_t ret = BLOG_KEY_INVALID;

   BCM_ASSERT( ((blog_p!=BLOG_NULL)&&(traffic<BlogTraffic_MAX)) );

   BCM_LOGCODE(if(protoLogDebug)
                { protoPrint("\n::: fap_config :::\n"); blog_dump(blog_p); });

   switch(traffic)
   {
      case BlogTraffic_Layer2_Flow:
      
         FAP_PROTO_LOCK();

         ret = fapL2flow_activate(blog_p);
         if(ret == FAP_ERROR)
         {
            fapState_g.failures++;
            ret = BLOG_KEY_INVALID;
         }
         else
         {
            fapState_g.active++;
            fapState_g.activates++;
         }

         FAP_PROTO_UNLOCK();

         break;
         
      default:
         protoError( "static fap config is only available for layer 2 traffic" );
   }

   return ret;
    
}  /* fap_config() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   Blog_t fap_deconf(uint32_t key_info, BlogTraffic_t traffic)
 * Description:
 *   blog_deactivate() is triggered by a kernel module to 
 *   deactivate a blog (or a flow) entry in fap.
 * Parameter:
 *   key_info: the blog flow handle.
 *   traffic:  traffic type.
 * Returns:
 *   BLOG_NULL
 *------------------------------------------------------------------------------
 */
Blog_t * fap_deconf(uint32_t key_info, BlogTraffic_t traffic)
{
   fapPkt_flowHandle_t flowHandle;
            
   BCM_ASSERT( (traffic<BlogTraffic_MAX) );

   BCM_LOGCODE(if(protoLogDebug)
                { protoPrint("\n::: fap_deconf :::\n"); });

   switch(traffic)
   {
      case BlogTraffic_Layer2_Flow:
      
         FAP_PROTO_LOCK();
            
         flowHandle.u16 = (key_info & ~FAP_HW_TUPLE_L2FLOW_MASK) & 0xFFFF;
         if(fapL2flow_deactivate(flowHandle) == FAP_ERROR)
         {
               fapState_g.failures++;
         }
         else
         {
            fapState_g.deactivates++;
            fapState_g.active--;
         }

         FAP_PROTO_UNLOCK();

         break;
         
      default:
         protoError( "static fap deconf is only available for layer 2 traffic" );
   }

   return BLOG_NULL;
    
}  /* fap_deconf() */
#endif

/*
 *------------------------------------------------------------------------------
 *
 * Function   : fapActivate
 * Description: This function is bound to the Flow Cache subsytem for the
 *              configuration of dynamically learnt flows.
 *              When a new flow is added to the flow cache hash table a
 *              request to configure this flow in hardware is made.
 *              A Flow_t object contains all Rx and Tx header information.
 *
 * Parameters :
 *    blog_p  : Pointer to a Blog_t object.
 *
 * Returns    : 16bit FAP_HW_TUPLE (i.e. FlowIx) or ~0.
 *------------------------------------------------------------------------------
 */
int fapActivate(Blog_t *blog_p, uint32_t key_in)
{
    Clog_t  clog;
    fap4kePkt_flowInfo_t flowInfo;
    /* FAP_DM_P2: instead of storing cmdList on stack, simply allocate it ahead of time with fapDm_alloc... */
    /* this would save a mem copy, and allow the flow to come up faster */
    uint8 cmdList[FAP4KE_PKT_CMD_LIST_SIZE];
#if defined(CC_FAP4KE_PKT_HW_ICSUM) || defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
    uint8 checksum1[FAP4KE_PKT_CSUM_CMD_LIST_SIZE];
    uint8 *checksum1_p = checksum1;
#else
    uint8 *checksum1_p = NULL;
#endif
#if defined(CC_FAP4KE_PKT_HW_ICSUM)
    uint8 checksum2[FAP4KE_PKT_CSUM_CMD_LIST_SIZE];
    uint8 *checksum2_p = checksum2;
#else
    uint8 *checksum2_p = NULL;
#endif
    int32 cmdSize;
    int32 cmdListIx;
    fapPkt_flowHandle_t flowHandle;
    uint32 fapIdx;


    /* this is kind of a hack to ensure both fcache/fap are in sync
     * with tcp_ack_mflows feature
     */
    if(fapTcpAckPrioStatus != blog_support_tcp_ack_mflows_g)
    {
        fapTcpAckPrioConfig(blog_support_tcp_ack_mflows_g);
        fapTcpAckPrioStatus = blog_support_tcp_ack_mflows_g;
    }


    BCM_ASSERT((blog_p!=BLOG_NULL));

    BCM_LOGCODE(if(protoLogDebug)
                { protoPrint("\n::: fapActivate :::\n"); blog_dump(blog_p); });

    memset(&flowInfo, 0, sizeof(fap4kePkt_flowInfo_t));
    memset(cmdList, ~0, FAP4KE_PKT_CMD_LIST_SIZE);
#if defined(CC_FAP4KE_PKT_HW_ICSUM)
    memset(checksum1, ~0, FAP4KE_PKT_CSUM_CMD_LIST_SIZE);
    memset(checksum2, ~0, FAP4KE_PKT_CSUM_CMD_LIST_SIZE);
#endif

    /* Set flow type: */
    if ( blog_p->rx.info.bmap.PLD_IPv6 && !T4in6DN(blog_p))
    {
        flowInfo.type = FAP4KE_PKT_FT_IPV6;
    }
    else if ( blog_p->rx.info.bmap.PLD_IPv4 )
    {
        flowInfo.type = FAP4KE_PKT_FT_IPV4;
    }
    else
    {
        protoInfo("Flow Type is not supported");
        goto abort_activate;
    }

    if(blog_p->rx.multicast)
    {
        int ret;
        int isActivation;

        cmdListIx = 0;

        FAP_PROTO_LOCK();

        if (blog_p->key.l1_tuple.channel != BLOG_CHAN_INVALID)
        {
            ret = fapMcast_activate(blog_p, &flowInfo, &isActivation);
            if(ret == FAP_ERROR)
            {
                ret = FLOW_HW_INVALID;
                fapState_g.failures++;
            }
            else
            {
                if(isActivation)
                {
                    fapState_g.active++;
                    fapState_g.activates++;
                }
            }
        }
        else
        {
#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
            if( likely(fapState_g.fap_activate_arl_hook != (HOOKP)NULL) )
            {
                ret = fapState_g.fap_activate_arl_hook( (void*)blog_p );
                /* if the activation was successful then compute hw handle */
                if ( 0 == ret )
                   ret = FAP_HW_TUPLE(FAP_HW_ENGINE_ARL, 0);
                else
                {    
                    protoInfo("fap_activate_arl_hook failed ret=%d\n", ret);
                }
            }
            else
#endif
            {
                protoInfo("Invalid blog_p->key.l1_tuple.channel<0x%x>\n",blog_p->key.l1_tuple.channel);
                ret = FLOW_HW_INVALID;
            }
        }

        FAP_PROTO_UNLOCK();
        return ret;
    }

    /* Determine which FAP to send the flow to */
    if(blog_p->rx.info.bmap.BCM_XPHY)
        fapIdx = getFapIdxFromXtmRxPort(blog_p->rx.info.channel);
    else if (blog_p->rx.info.phyHdrType == BLOG_GPONPHY) 
        fapIdx = getFapIdxForGPONPort();
    else
        fapIdx = getFapIdxFromEthRxPort(blog_p->rx.info.channel);

#if NUM_FAPS > 1
    if(!blog_p->rx.info.bmap.BCM_XPHY)
    {
        /* We've received on the Ethernet */
        if(blog_p->tx.info.bmap.BCM_XPHY)
        {
            /* We are to transmit on the SAR */
            if (fapIdx != getFapIdxFromXtmTxPort(blog_p->tx.info.channel))
            {
                /* The fapIdx of the recieve side is not the same as the
                   fapIdx of the transmit side, and the transmit side is
                   SAR.  Because both FAPs cannot do SAR transmission,
                   we are marking this as invalid */
                fapIdx = FAP_INVALID_IDX;
            }
        }
    }
#endif
    /* Do not push flows to FAP for traffic received on the Host */
    if( FAP_INVALID_IDX == fapIdx )
    {
        protoInfo("No FAP to activate");

        /* This should not be accounted as a failure (split iuDMA) */
        fapState_g.failures--;

        goto abort_activate;
    }

    /* strip BCM Tag from L2 header and store L2 header in clog */
    buildClog(blog_p, &clog);

    cmdListIx = 0;
    flowInfo.txAdjust = 0;

    cmdListIx = setFwdAndFilters(&flowInfo, cmdList, cmdListIx,
                                 &checksum1_p, &checksum2_p, blog_p);
    if(cmdListIx == FAP_ERROR)
    {
        protoInfo("Failed to add L2 Commands");
        goto abort_activate;
    }

#if defined(CONFIG_BCM_PORTS_ON_INT_EXT_SW)
    {
        uint8 isExtSwitch = IsExternalSwitchPort(blog_p->tx.info.channel);
        uint8 extSwPort = LOGICAL_PORT_TO_PHYSICAL_PORT(blog_p->tx.info.channel);

        if (flowInfo.dest.phy == FAP4KE_PKT_PHY_WLAN) //WLAN flow. Set ExtSwitch flag to 0 so that no TAGs are added/replaced by FAP
        {
            isExtSwitch = 0;
            extSwPort = FAP_INVALID_SWITCH_PORT;
        }

        cmdListIx = addL2Commands(&flowInfo, cmdList, cmdListIx, &clog, isExtSwitch, extSwPort);
    }
#else
    cmdListIx = addL2Commands(&flowInfo, cmdList, cmdListIx, &clog);
#endif
    if(cmdListIx == FAP_ERROR)
    {
        protoInfo("Failed to add L2 Commands");
        goto abort_activate;
    }

#if defined(CC_FAP4KE_TM)
    fapTm_setFlowInfo(&flowInfo, (1 << blog_p->tx.info.channel));
    if(fapTm_checkHighPrio(flowInfo.dest.phy, flowInfo.virtDestPort,
                           flowInfo.dest.queue, flowInfo.dest.channel,
                           SKBMARK_GET_TC_ID(blog_p->mark)) == 1)
        flowInfo.dest.hiPrio = 1;
    else
        flowInfo.dest.hiPrio = 0;
#endif

    cmdListIx = addL3Commands(&flowInfo, cmdList, cmdListIx, blog_p);
    if(cmdListIx == FAP_ERROR)
    {
        protoInfo("Failed to add L3 Commands");
        goto abort_activate;
    }

    cmdSize = iopDma_CmdListAddEndOfCommands(&cmdList[cmdListIx]);
    if(cmdSize == FAP_ERROR)
    {
        protoError("Command List End: cmdListIx <%d>", cmdListIx);
        goto abort_activate;
    }
    cmdListIx += cmdSize;

    CHECK_CMD_LIST(cmdList, cmdListIx);

    if(cmdListIx >= FAP4KE_PKT_CMD_LIST_SIZE)
    {
        protoError("Command List Overflow: cmdListIx <%d>", cmdListIx);
        goto abort_activate;
    }

    protoDebug("Command List Size <%d>, Tx Adjust <%d>", cmdListIx, flowInfo.txAdjust);

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    flowInfo.iq.prio = blog_p->iq_prio;
#endif

    if (pHostPsmGbl(fapIdx)->mtuOverride)
        flowInfo.fapMtu = pHostPsmGbl(fapIdx)->mtuOverride;
    else
        flowInfo.fapMtu = blog_getTxMtu(blog_p);

    if (flowInfo.dest.phy == FAP4KE_PKT_PHY_WLAN) //WLAN flow. Set ChainIdx
    {
        if (!blog_p->wfd.nic_ucast.is_wfd) /* FAP only supports WFD based flows */
        {
            goto abort_activate;
        }
        flowInfo.word = 0; /* clear the metadata */
        flowInfo.nic.is_chain = blog_p->wfd.nic_ucast.is_chain;
        if (blog_p->wfd.nic_ucast.is_chain) /* is_chain position is same in both nic or dhd */
        {
            flowInfo.nic.wfd_idx = blog_p->wfd.nic_ucast.wfd_idx;
            flowInfo.nic.priority = blog_p->wfd.nic_ucast.priority;
            flowInfo.nic.chain_idx = blog_p->wfd.nic_ucast.chain_idx;
            protoDebug("Adding NIC WLAN Flow Metadata <%d:%d:%d> Channel 0x%x WLAN Priority 0x%x",  
                       flowInfo.nic.wfd_idx,flowInfo.nic.priority,flowInfo.nic.chain_idx, blog_p->tx.info.channel, flowInfo.dest.queue);
        }
        else /* DHD */
        {
            flowInfo.dhd.ssid = blog_p->wfd.dhd_ucast.ssid;
            flowInfo.dhd.wfd_idx = blog_p->wfd.dhd_ucast.wfd_idx;
            flowInfo.dhd.priority = blog_p->wfd.dhd_ucast.priority;
            flowInfo.dhd.flow_ring_idx = blog_p->wfd.dhd_ucast.flowring_idx;
            protoDebug("Adding DHD WLAN Flow Metadata <%d:%d:%d:%d> Channel 0x%x WLAN Priority 0x%x", 
                       flowInfo.dhd.wfd_idx,flowInfo.dhd.priority,flowInfo.dhd.flow_ring_idx,flowInfo.dhd.ssid,blog_p->tx.info.channel, 
                       flowInfo.dest.queue);
        }
    }
    FAP_PROTO_LOCK();

    /* activate flow into the FAP */
    flowHandle = fapPkt_activate(fapIdx, &flowInfo, cmdList, cmdListIx, checksum1_p, checksum2_p, NULL);
    if(flowHandle.u16 == FAP4KE_PKT_INVALID_FLOWHANDLE)
    {
        protoInfo("Could not allocate flow");
        FAP_PROTO_UNLOCK();
        goto abort_activate;
    }

    fapState_g.activates++;

    fapState_g.active++;

    PROTODBG(protoPrint, FAP_IDX_FMT "::: fapActivate flowId<%03u> cumm_activates<%u> :::\n\n",
             (uint32)flowHandle.fapIdx, flowHandle.flowId, fapState_g.activates);

    FAP_PROTO_UNLOCK();

    return FAP_HW_TUPLE(FAP_HW_ENGINE_SWC, flowHandle.u16);

abort_activate:
    FAP_PROTO_LOCK();
    fapState_g.failures++;

    protoInfo("cumm_failures<%u>", fapState_g.failures);
    FAP_PROTO_UNLOCK();

    return FLOW_HW_INVALID;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fapRefresh
 * Description: This function is invoked to check activity for a NATed flow
 * Parameters :
 *  tuple : 16bit index to refer to a FAP flow
 * Returns    : Total hits on this connection.
 *------------------------------------------------------------------------------
 */
int fapRefresh(uint16_t tuple, uint32_t *pktsCnt_p, uint32_t *octetsCnt_p)
{
    int ret = FAP_SUCCESS;

    FAP_PROTO_LOCK();

    if(fapGetHwEngine(tuple) == FAP_HW_ENGINE_ARL)
    {
#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
        if ( likely(fapState_g.fap_refresh_arl_hook != (HOOK3PARM)NULL) )
        {
            ret = fapState_g.fap_refresh_arl_hook( (uint32_t)tuple,
                                    (uint32_t)(pktsCnt_p),
                                    (uint32_t)(octetsCnt_p) );
            
            /* switch HW includes CRC length for octet info */
            if (*octetsCnt_p)
                *octetsCnt_p -= (*pktsCnt_p * BLOG_ETH_FCS_LEN);
        }
        else
        {
            ret = FAP_ERROR;
        }
#endif
    }
    else if(fapGetHwEngine(tuple) == FAP_HW_ENGINE_MCAST)
    {
        ret = fapMcast_refresh(tuple, pktsCnt_p, octetsCnt_p);
    }
    else
    {
        fapPkt_flowHandle_t flowHandle;
        fap4kePkt_flowStats_t *flowStats;

        flowHandle.u16 = fapGetHwEntIx(tuple);

        flowStats = fapPkt_getFlowStats(flowHandle);

        if(flowStats == NULL)
        {
            protoError(FAP_IDX_FMT "Could not get flowId<%d> stats",
                       (uint32)flowHandle.fapIdx, flowHandle.flowId);

            ret = FAP_ERROR;
            goto out;
        }

        *pktsCnt_p = flowStats->hits;
        *octetsCnt_p = flowStats->bytes;

        /* switch HW includes CRC length for octet info */
        if (*octetsCnt_p)
            *octetsCnt_p -= (*pktsCnt_p * BLOG_ETH_FCS_LEN);

        protoDebug(FAP_IDX_FMT "flowId<%d> cumm_hits<%u> cumm_bytes<%u>",
            (uint32)flowHandle.fapIdx, flowHandle.flowId, 
            *octetsCnt_p, *pktsCnt_p);
    }

out:
    FAP_PROTO_UNLOCK();

    return ret; 
}

/*
 *------------------------------------------------------------------------------
 * Function   : fapResetStats
 * Description: This function is invoked to reset stats for a flow
 * Parameters :
 *  hwTuple: 16bit index to refer to a FAP flow
 * Returns    : Total hits on this connection.
 *------------------------------------------------------------------------------
 */
int fapResetStats(uint16_t hwTuple)
{
    int ret = FAP_SUCCESS;

    FAP_PROTO_LOCK();

    if(fapGetHwEngine(hwTuple) == FAP_HW_ENGINE_ARL)
    {
#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
        if ( likely(fapState_g.fap_reset_stats_arl_hook != (HOOK32)NULL) )
        {
            ret = fapState_g.fap_reset_stats_arl_hook( (uint32_t)hwTuple );
        }
        else
        {
            ret = FAP_ERROR;
        }
#endif
    }
    else if(fapGetHwEngine(hwTuple) == FAP_HW_ENGINE_ARL)
    {
        ret = fapMcast_resetStats(hwTuple);
    }
    else
    {
        fapPkt_flowHandle_t flowHandle;

        flowHandle.u16 = fapGetHwEntIx(hwTuple);
        fapPkt_resetStats(flowHandle);
    }

    FAP_PROTO_UNLOCK();

    return ret; 
}


#if defined(CONFIG_BCM_FHW)
/*
 *------------------------------------------------------------------------------
 * Function   : __clearFCache
 * Description: Clears FlowCache association(s) to FAP entries.
 *              This local function MUST be called with the Protocol Layer
 *              Lock taken.
 *------------------------------------------------------------------------------
 */
static int __clearFCache(uint32_t key, const FlowScope_t scope)
{
    int count;

    /* Upcall into FlowCache */
    if(fhw_clear_hook_fp != (FC_CLEAR_HOOK)NULL)
    {
        fapState_g.flushes += fhw_clear_hook_fp(key, scope);
    }

    count = fapMcast_deactivateAll();
    count += fapPkt_deactivateAll();

    protoDebug("key<%03u> scope<%s> cumm_flushes<%u>",
               key,
               (scope == System_e) ? "System" : "Match",
               fapState_g.flushes);

    return count;
}
#endif

/*
 *------------------------------------------------------------------------------
 * Function   : fapDeactivate
 * Description: This function is invoked when a flow in the FAP needs to be
 *              deactivated.
 * Parameters :
 *  flowIx    : 16bit index to refer to a NATed flow in HW
 *  blog_p    : pointer to a blog object (for multicast only)
 * Returns    : Remaining number of active port (for multicast only)
 *------------------------------------------------------------------------------
 */
int fapDeactivate(uint16_t tuple, uint32_t *pktsCnt_p,
                  uint32_t *octetsCnt_p, struct blog_t * blog_p)
{
    int ret;
    int isDeactivation = 0;

    fapRefresh(tuple, pktsCnt_p, octetsCnt_p);

    FAP_PROTO_LOCK();

    if(blog_p->rx.multicast)
    {
        if (blog_p->key.l1_tuple.channel != BLOG_CHAN_INVALID) 
        {
            ret = fapMcast_deactivate(blog_p, &isDeactivation);

            // Only for multicast we need to adjust the octets when the last
            // flow gets evicted. For Unicast, this is done by fapRefresh
            if (isDeactivation)
            {
                /* switch HW includes CRC length for octet info */
                if (*octetsCnt_p)
                    *octetsCnt_p -= (*pktsCnt_p * BLOG_ETH_FCS_LEN);
            }
        }
        else
        {
#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
            if(likely(fapState_g.fap_deactivate_arl_hook != (HOOK4PARM)NULL) )
            {
                ret = fapState_g.fap_deactivate_arl_hook(tuple,
                    (uint32_t)pktsCnt_p, 
                    (uint32_t)octetsCnt_p, (uint32_t )blog_p);
            }
            else
#endif
            {
                ret = FAP_ERROR;
            }
        }
    }
    else
    {
        fapPkt_flowHandle_t flowHandle;

        flowHandle.u16 = fapGetHwEntIx(tuple);

        ret = fapPkt_deactivate(flowHandle);
        if(ret == FAP_ERROR)
        {
            goto out_unlock;
        }
        isDeactivation = 1;
    }

    if(isDeactivation)
    {
        fapState_g.deactivates++;
        fapState_g.active--;
    }


    PROTODBG(protoPrint,
             "::: fapDeactivate flowIx<%03u> hits<%u> bytes<%u> cumm_deactivates<%u> :::\n",
             tuple, *pktsCnt_p, *octetsCnt_p, fapState_g.deactivates);

out_unlock:
    FAP_PROTO_UNLOCK();

    return ret;
}

/*
 *------------------------------------------------------------------------------
 * Function   : fapClear
 * Description: This function is invoked when all entries pertaining to
 *              a entIx in FAP need to be cleared.
 * Parameters :
 *  tuple: FHW Engine instance and match index
 * Returns    : success
 *------------------------------------------------------------------------------
 */
int fapClear(uint16_t tuple)
{
    return FAP_SUCCESS;
}

/* 
 *------------------------------------------------------------------------------
 * Function   : fapStatus()
 * Description: Display FAP Protocol learning status, summary
 *------------------------------------------------------------------------------
 */
void fapStatus(void)
{
    FAP_PROTO_LOCK();

    protoPrint("FAP:\n"
               "\tAcceleration %s, Active <%u>, IPv6 %s\n"
               "\tActivates   : %u\n"
               "\tFailures    : %u\n"
               "\tDeactivates : %u\n"
               "\tFlushes     : %u\n\n",
               (fapState_g.status == 1) ?  "Enabled" : "Disabled",
               fapState_g.active, 
#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
               "Enabled",
#else
               "Disabled",
#endif
               fapState_g.activates, fapState_g.failures,
               fapState_g.deactivates, fapState_g.flushes);

    FAP_PROTO_UNLOCK();
}

/*
 *------------------------------------------------------------------------------
 * Function   : fapPrint()
 * Description: Display Statistics and dump all active flows with hits
 *------------------------------------------------------------------------------
 */
void fapPrint(int16 sourceChannel, int16 destChannel)
{
    fapStatus();

    fapPkt_printAllFlows(sourceChannel, destChannel);
}

/*
 *------------------------------------------------------------------------------
 * Function   : fapEnable
 * Description: Binds the FAP Protocol Layer handler functions to Flow Cache hooks.
 *------------------------------------------------------------------------------
 */
void fapEnable(void)
{
#if defined(CONFIG_BCM_FHW)
    FhwBindHwHooks_t hwHooks = {};
    FhwHwAccPrio_t prioIx;

    prioIx = FHW_PRIO_0;

    blog_lock(); /* Block flow-cache from packet processing and try to push the flows */
    FAP_PROTO_LOCK();

    hwHooks.activate_fn = (HOOKP32)fapActivate;
    hwHooks.deactivate_fn = (HOOK4PARM)fapDeactivate;
    hwHooks.update_fn = (HOOK3PARM)NULL;
    hwHooks.refresh_fn = (HOOK3PARM)fapRefresh;
    hwHooks.fhw_clear_fn = &fhw_clear_hook_fp;
    hwHooks.reset_stats_fn =(HOOK32) fapResetStats; 
    hwHooks.cap = (1<<HW_CAP_IPV4_UCAST);
    hwHooks.cap |= (1<<HW_CAP_IPV4_MCAST);

#if defined(CC_FAP4KE_PKT_IPV6_SUPPORT)
    hwHooks.cap |= (1<<HW_CAP_IPV6_UCAST) | (1<<HW_CAP_IPV6_MCAST) |
                   (1<<HW_CAP_IPV6_TUNNEL);
#endif

    /* TODO: Ideally this should just be FAP_MAX_FLOWS * NUM_FAPS */
    hwHooks.max_ent = FAP_MAX_FLOWS * NUM_FAPS * FAP_HW_ENGINE_ALL;

    /* Bind to fc HW layer for learning connection configurations dynamically */
    hwHooks.clear_fn = (HOOK32)fapClear;

    fhw_bind_hw(prioIx, &hwHooks);

    BCM_ASSERT((fhw_clear_hook_fp != (FC_CLEAR_HOOK)NULL));

    fapState_g.status = 1;

    FAP_PROTO_UNLOCK();
    blog_unlock();

    /* configure TCP ACK PRIO feature */
    fapTcpAckPrioConfig(blog_support_tcp_ack_mflows_g);
    fapTcpAckPrioStatus = blog_support_tcp_ack_mflows_g;

    if(bcmLog_getLogLevel(BCM_LOG_ID_FAPPROTO) >= BCM_LOG_LEVEL_INFO)
    {
        fapStatus();
    }

    protoNotice("Enabled FAP binding to Flow Cache");
#else
    protoNotice("Flow Cache not built.");
#endif

#if defined(CONFIG_BCM_FAP_LAYER2)
    /* Bind to vlanctl for layer2 acceleration. */
    fap_bind_vlanctl(1);
#endif    
}

/*
 *------------------------------------------------------------------------------
 * Function   : fapDisable
 * Description: Clears all active Flow Cache associations with FAP.
 *              Unbind all flow cache to FAP hooks.
 *------------------------------------------------------------------------------
 */
void fapDisable(void)
{
#if defined(CONFIG_BCM_FHW)
    FhwBindHwHooks_t hwHooks = {};
    FhwHwAccPrio_t prioIx;

    prioIx = FHW_PRIO_0;

    blog_lock(); /* Block flow-cache from packet processing and try to push the flows */
    FAP_PROTO_LOCK();

    /* Clear system wide active FlowCache associations, and disable learning. */
    __clearFCache(FAP_HW_ENGINE_ALL, System_e);

    hwHooks.activate_fn = (HOOKP32)NULL;
    hwHooks.deactivate_fn = (HOOK4PARM)NULL;
    hwHooks.update_fn = (HOOK3PARM)NULL;
    hwHooks.refresh_fn = (HOOK3PARM)NULL;
    hwHooks.fhw_clear_fn = &fhw_clear_hook_fp;
    hwHooks.reset_stats_fn =(HOOK32) NULL; 
    hwHooks.cap = 0;
    hwHooks.max_ent = 0;
    hwHooks.clear_fn = (HOOK32)NULL;

    fhw_bind_hw(prioIx, &hwHooks);

    fhw_clear_hook_fp = (FC_CLEAR_HOOK)NULL;

    fapState_g.status = 0;

    FAP_PROTO_UNLOCK();
    blog_unlock();

    if(bcmLog_getLogLevel(BCM_LOG_ID_FAPPROTO) >= BCM_LOG_LEVEL_INFO)
    {
        fapStatus();
    }

    protoNotice("Disabled FAP binding to Flow Cache");
#else
    protoNotice("Flow Cache not built.");
#endif

#if defined(CONFIG_BCM_FAP_LAYER2)
    /* Unbind to vlanctl for layer2 acceleration. */
    fap_bind_vlanctl(0);
#endif    
}

/*
 *------------------------------------------------------------------------------
 * Function   : fapReset
 * Description: Resets Flow Cache to have no reference to FAP NATed flows
 *              and pre-initializes statistics and global state.
 *------------------------------------------------------------------------------
 */
void fapReset(void)
{
    FAP_PROTO_LOCK();

    if(fapState_g.status == 1)
    {
        fapDisable();
    }

    fapState_g.activates = fapState_g.failures
    = fapState_g.deactivates = fapState_g.flushes
    = fapState_g.active = 0;

    protoNotice("Reset FAP Protocol layer");

    FAP_PROTO_UNLOCK();
}

/*------------------------------------------------------------------------------
 * Function   : fapDebug
 * Description: Sets the FAP Protocol Layer log level
 *------------------------------------------------------------------------------
 */
fapRet fapDebug(int logLevel)
{
    if(logLevel >= 0 && logLevel < BCM_LOG_LEVEL_MAX)
    {
        bcmLog_setLogLevel(BCM_LOG_ID_FAPPROTO, logLevel);
    }
    else
    {
        protoError("Invalid Log level %d (max %d)",
                   logLevel, BCM_LOG_LEVEL_MAX);

        return FAP_ERROR;
    }

    return FAP_SUCCESS;
}

/*
 *******************************************************************************
 * Function   : fapProtoConstruct
 * Description: Construct the FAP Protocol layer
 *******************************************************************************
 */
void __init fapProtoConstruct(void)
{
    fapState_g.status = 0;          /* not initialized yet */
    fapReset();               /* Resets and Initializes */

    bcmLog_setLogLevel(BCM_LOG_ID_FAPPROTO, BCM_LOG_LEVEL_ERROR);

    fapMcast_construct();

    protoNotice("Reset and initialized FAP Protocol Layer");

    fapEnable();
}


/*
 *******************************************************************************
 * Function   : fapProtoDestruct
 * Description: Destruct the FAP Protocol layer
 *******************************************************************************
 */
void __exit fapProtoDestruct(void)
{
    fapMcast_destruct();
}

