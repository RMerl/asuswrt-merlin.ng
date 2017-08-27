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
 * To Be Defined:
 * - Do we need the ability to "remove" configured items in an FCB.
 *   reference: enum BpaConfigOper
 *
 */

#ifndef __BPAAPI_H__
#define __BPAAPI_H__

#if defined(__KERNEL__)
#include "bcm_OS_Deps.h"
#else
#include <stdint.h>             /**< ISO C99 7.18 Integer Types */
#endif

/*! \file bpaapi.h
 *  \brief Header file for Broadcom Packet Accelerator (BPA) APIs v1.02.
 *
 *  This header file defines the BPA APIs which allows an application
 *  to configure Anti-Spoofing tables, DSCP-to-Pbits mapping tables and
 *  the unicast & multicast flows for a GPON device.
 */


/** Block types inside FCB */
typedef enum BpaFcbBlk
{
    e_FCBBLK_FILT,               /**< Filtering Block    */
    e_FCBBLK_MOD,                /**< Modification Block */
    e_FCBBLK_FWD,                /**< Forwarding Block   */
    e_FCBBLK_MAX                 /**< Max # of Blocks    */
} BpaFcbBlk_t;

/** Flow header types
 *
 * These are the various headers in the packet based on which the filtering
 * and modification can be done.
 * Currently supports max 4 VLAN tags (VLAN0,1,2,3).
 */
typedef enum BpaFlowHdr
{
    e_FLOWHDR_ETH,
    e_FLOWHDR_VLAN0,            /**< Outermost VLAN */
    e_FLOWHDR_VLAN1,
    e_FLOWHDR_VLAN2,
    e_FLOWHDR_VLAN3,            /**< Innermost VLAN */
    e_FLOWHDR_PPPOE,
    e_FLOWHDR_MPLS,
    e_FLOWHDR_IPV4,
    e_FLOWHDR_IPV6,
/*   e_FLOWHDR_Raw, Protocol agnostic specification interms of <offset,value> */
    e_FLOWHDR_MAX
} BpaFlowHdr_t;


/** 6816 ports */
typedef enum BpaPort
{
    e_PORT_GEPHY0,
    e_PORT_GEPHY1,
    e_PORT_RGMII0,
    e_PORT_RGMII1,
    e_PORT_SERDES_GMII,
    e_PORT_MOCA,
    e_PORT_USB,
    e_PORT_GPON,
    e_PORT_MIPS,
    e_PORT_MAX
} BpaPort_t;

#define BPA_MAX_UNI             6

#define BPA_INVALID_GEMIDX      CONFIG_BCM_MAX_GEM_PORTS  /** Invalid GEM Index to use during GEM Index Reset */
#define BPA_MAX_DST_GEMIDX      1   /**< currently 1 GEM port in upstream */
#define BPA_MAX_DST_QID         8   /**< Limit 4 (IuDMA priority) to MIPS */

/** Dest Port mask info */
typedef struct BpaDstPortMask
{
#if defined(CONFIG_BCM96816)
    uint8_t  gemPortIdxQid[CONFIG_BCM_MAX_GEM_PORTS]; /**< Q on the dest GEM port. */
    uint32_t gemPortIdxMask[1]; /**< bitmap of GEM ports when GPON port bit
                                     is set in portMask                     */
#elif defined(CONFIG_BCM96818)
    uint8_t  gemPortIdxQid;     /** Q on the dest GEM port*/
    uint32_t gemPortIdx;        /** GEM port index when GPON port bit is set in 
                                     portMask*/
#endif 
    uint8_t  qid[e_PORT_MAX];   /**< Q on the dest port. except for GPON    */
    uint32_t portMask;          /**< bitmap of ports                        */
} BpaDstPortMask_t;


#if defined(CONFIG_BCM96818)
/* CAUTION: 
 * 1. When using gemPortIndex, the flow can specify either a single
 *    gemPortIndex or all gem port indices (i.e. BPA_ALL_GEMS).
 * 2. Multiple GEM port indices (other than BPA_ALL_GEMS) are not supported.
 */
#endif

/** Source Port mask info */
typedef struct BpaSrcPortMask
{
#if defined(CONFIG_BCM96816)
    uint32_t gemPortIdxMask[1]; /**< bitmap of GEM ports when GPON port bit
                                        is set in portMask      */
#elif defined(CONFIG_BCM96818)
#define BPA_ALL_GEMS    0xFFFF  /**< All GEM ports           */
    uint32_t gemPortIdx;        /**< GEM port index when GPON port bit
                                        is set in portMask      */
#endif
    uint32_t portMask;          /**< bitmap of ports         */
} BpaSrcPortMask_t;

/** VLAN tag number.
 *
 * Different levels of VLAN tags that will be supported. 
 * Max of 4 VLAN tags could be present in a packet.
 * The VLANs are numbered in the order:
 * VLAN TAG0 : Outermost
 * VLAN TAG1 : 2nd Outermost
 * VLAN TAG2 : ...
 * VLAN TAG3 : Innermost
 */
typedef enum BpaVlanTagNum
{
    e_VLAN_TAG0,                /**< Outermost tag     */
    e_VLAN_TAG1,                /**< 2nd Outermost tag */
/*  e_VLAN_TAG2,                     2nd Innermost tag */
/*  e_VLAN_TAG3,                     Innermost tag     */
    e_VLAN_TAG_MAX
} BpaVlanTagNum_t;


/** MAC Address */
typedef union BpaMacAddr
{
    uint8_t u8[6];
    struct 
    {
        uint16_t mshw;          /**< Most significant Half-word       */
        uint32_t lsw;           /**< Least significant word           */
    } __attribute__((packed)) s1;
} BpaMacAddr_t;

#define BPA_NULL_IPADDR    0    /**< 0.0.0.0 */

/** IPv4 Address Type */
typedef union BpaIpAddr
{
    uint8_t  u8[4];
    uint32_t u32;
} BpaIpAddr_t;


/** CIDR (Classless Inter-Domain Routing) IP Address */
typedef struct BpaCidrIpAddr
{
    BpaIpAddr_t addr;
    uint32_t    prefixLen;
} BpaCidrIpAddr_t;


/** IPv6 Address Type */
typedef union BpaIpv6Addr
{
    uint8_t  u8[16];
    uint16_t u16[8];
    uint32_t u32[4];
} BpaIpv6Addr_t;


/** CIDR (Classless Inter-Domain Routing) IPv6 Address */
typedef struct BpaCidrIpv6Addr
{
    BpaIpv6Addr_t addr;
    uint32_t      prefixLen;
} BpaCidrIpv6Addr_t;



/** Configuration Operations.
 *
 * Various operations that can add new filters in the
 * filtering block and actions for the modification block of a flow.
 * CONFIG_OPER_ADD:  adds the filter/action to the flow.
 * CONFIG_OPER_REM:  removes the exising filter/action from the flow.
 */
typedef enum BpaConfigOper
{
    e_CONFIG_OPER_ADD,          /**< Add operation       */
/**<   e_CONFIG_OPER_REM, */    /**< Remove operation    */
    e_CONFIG_OPER_MAX           /**< Max # of operations */
} BpaConfigOper_t;


/*
 * The source of a pbits may originate from a DSCP_1P, the Outer or Inner VLAN.
 */
typedef enum BpaPbitsSource
{
    e_PBITS_SRC_DSCP_1P,
    e_PBITS_SRC_VLANTAG0,
    e_PBITS_SRC_VLANTAG1,
    e_PBITS_SRC_MAX
} BpaPbitsSource_t;

/*
 * Raw Actions for miscellaneous (masked) Insert and Replaces.
 * These raw actions may be used in the pBits to Action table or in a FCB action
 * An example of a raw action is for VLAN TCI masked replace.
 */
#define BPA_RAWACT_MAXSIZ 2

typedef enum BpaRawActionType
{
    e_RAW_ACTION_NONE=0,
    e_RAW_ACTION_INSERT,
    e_RAW_ACTION_REPLACE,
    e_RAW_ACTION_MAX
} BpaRawActionType_t;

typedef struct BpaRawAction
{
    BpaRawActionType_t type;         /**< Modification type */
    uint8_t offset;                  /**< Byte offset in the packet (N/W order) */
    uint8_t size;                    /**< Value size, in bytes, up to BPA_RAWACT_MAXSIZ */
    uint8_t val[BPA_RAWACT_MAXSIZ];  /**< Byte Value array (N/W order) */
    uint8_t mask[BPA_RAWACT_MAXSIZ]; /**< Byte Mask array (N/W order) */
} BpaRawAction_t;


/** BPA API Return Values
 */
typedef enum BpaApiRet
{
    e_RET_SUCCESS,
    e_RET_ERR_FAILURE,
    e_RET_ERR_NULL_PTR,
    e_RET_ERR_INVALID_HDL,
    e_RET_ERR_INT_PROB,
    e_RET_ERR_NULL_REFCNT,
    e_RET_ERR_NO_SUCH_ENTRY,
    e_RET_ERR_OVERFLOW,
    e_RET_ERR_NOT_SUPPORTED,
    e_RET_ERR_REQOUTOFORDER,
    e_RET_ERR_HDROUTOFORDER,
    e_RET_ERR_HDRNOTCFG,
    e_RET_ERR_RESRC_UNAVAIL,    /**< Memory or some other resource    */
    e_RET_ERR_RESRC_BUSY,       /**< Resource Busy                    */
    e_RET_ERR_RESRC_NOT_ACTIVE,
    e_RET_ERR_RESRC_ALREADY_ACTIVE,
    e_RET_ERR_STATE_NOTALLOC,
    e_RET_ERR_STATE_NOTCFG,
    e_RET_ERR_STATE_NOT_ACTIVE,
    e_RET_ERR_STATE_ALREADY_ACTIVE,
    e_RET_ERR_OVRD_STATUS,
    e_RET_ERR_INVALID_CMPOPSEQ,
    e_RET_ERR_INVALID_FILTSEQ,
    e_RET_ERR_INVALID_ACTIONSEQ,
    e_RET_ERR_INVALID_VAL,
    e_RET_ERR_INVALID_PARAM,
    e_RET_ERR_INVALID_RANGE,
    e_RET_ERR_INVALID_SECTION,
    e_RET_ERR_INVALID_NUMFLOWS,
    e_RET_ERR_INVALID_GROUP,
    e_RET_ERR_INVALID_FWDACTION,
    e_RET_ERR_INVALID_VID,
    e_RET_ERR_INVALID_MACDA,
    e_RET_ERR_INVALID_MACSA,
    e_RET_ERR_INVALID_IPSA,
    e_RET_ERR_INVALID_IPDA,
    e_RET_ERR_INVALID_SRCPORT,
    e_RET_ERR_INVALID_SRCPORTMASK,
    e_RET_ERR_INVALID_DSTPORT,
    e_RET_ERR_INVALID_DSTPORTMASK,
    e_RET_ERR_INVALID_DSTQID,
    e_RET_ERR_INVALID_GEMPORTIDXMASK,
    e_RET_ERR_INVALID_HDRTYPE,
    e_RET_ERR_INVALID_BLOCKTYPE,
    e_RET_ERR_INVALID_TAGNUM,
    e_RET_ERR_INVALID_CONFIGOPER,
    e_RET_ERR_INVALID_CMPOPER,
    e_RET_ERR_INVALID_FILT,
    e_RET_ERR_INVALID_ACTION,
    e_RET_ERR_INVALID_ASENT,
    e_RET_ERR_INVALID_ASOVRD,
    e_RET_ERR_INVALID_ACLTYPE, 
    e_RET_ERR_INVALID_OVRD,
    e_RET_ERR_INVALID_ENTIDX,
    e_RET_ERR_INVALID_IPPREFIXLENVAL,
    e_RET_ERR_INVALID_PBITS,
    e_RET_ERR_INVALID_DSCPVAL,
    e_RET_ERR_INVALID_GEMPORTIDX,
    e_RET_ERR_INVALID_FWDTYPE,
    e_RET_ERR_INVALID_FLOWPRIO,
    e_RET_ERR_INVALID_TBLSTATUS,
    e_RET_ERR_INVALID_ETHTYPE,
    e_RET_ERR_INVALID_VLANTAG,
    e_RET_ERR_INVALID_PBITS_SOURCE,
    e_RET_ERR_INVALID_RAWACTION_TYPE,
    e_RET_ERR_INVALID_RAWACTION_OFFSET,
    e_RET_ERR_INVALID_RAWACTION_SIZE,
    e_RET_ERR_INVALID_PADLEN, 
    e_RET_ERR_INVALID_RLSTATUS, 
    e_RET_ERR_INVALID_RLINDEX, 
    e_RET_ERR_INVALID_RLDIR, 
    e_RET_ERR_INVALID_RLUNIT, 
    e_RET_ERR_INVALID_RLRATE,
    e_RET_ERR_INVALID_PAC_PBITS2DESTQ_COMBO,
#if defined(CONFIG_BCM96818)
    e_RET_ERR_INVALID_FWDCOLOR,
#endif
    e_RET_MAX
} BpaApiRet_t;


/*
 *------------------------------------------------------------------------------
 * Library calls
 *------------------------------------------------------------------------------
 */

/*
 *******************************************************************************
 * Global APIs
 *******************************************************************************
 */

/** Initializes the BPA API library.
 * This should be the first BPA API invoked, and should be invoked only once.
 * 
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 */
extern BpaApiRet_t BPA_Init_Sys( void );


typedef enum {
    e_FLOW_SECTION_DEFINED,     /**< default size of defined section is 0    */
    e_FLOW_SECTION_PROVISIONED, /**< default size of provisioned sect is max */
    e_FLOW_SECTION_DYNAMIC,     /**< default size of dynamic section is 0    */
    e_FLOW_SECTION_MAX,         
} BpaFlowSection_t;


/** Sets the maximum number of flows in a Flow Section.
 *
 * @param  section   (IN)  The section type
 * @param  numFlows  (IN)  The number of flows in this section.
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_INVALID_SECTION
 *     e_RET_ERR_INVALID_NUMFLOWS
 *     e_RET_ERR_RESRC_UNAVAIL
 *     e_RET_ERR_RESRC_BUSY
 */
extern BpaApiRet_t BPA_Set_SectionMaxFlows( BpaFlowSection_t section,
                uint16_t numFlows );

/** Gets the value of maximum number of flows in a section.
 * 
 * @param  section    IN)  The section type
 * @param  numFlows_p (OUT) The numer of flows in the section.
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_SECTION
 */
extern BpaApiRet_t BPA_Get_SectionMaxFlows( BpaFlowSection_t section,
                uint16_t *numFlows_p );


/** Initially when the FCB(s) are not created all the packets will encounter
 * miss condition. Even after creation of FCB(s) in the system,  some other
 * flows for which FCB is not created will encounter the miss condition. The
 * APIs allows the application to configure the action for this missed flow
 * packets. The action configured is applicable to all the missed flow packets.
 * The three allowed actions for missed packets are:
 * - drop the packets
 * - send to the MIPS
 * - pass the packet along the flow path
 */
typedef enum {
    e_FLOW_MISS_ACTION_DROP,    /**< Drop packet                            */
    e_FLOW_MISS_ACTION_SEND2MIPS, /**< Send to MIPS                         */
    e_FLOW_MISS_ACTION_PASS,    /**< pass the packet to the switch          */
    e_FLOW_MISS_ACTION_MAX,         
} BpaFlowMissAction_t;


/** Sets a flow miss action globally (i.e. for all flows). The
 *  action taken when a flow miss occurs.
 * 
 * @param action_p   (IN)  Flow Miss action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_ACTION
 */
extern BpaApiRet_t BPA_Set_FlowMissAction( BpaFlowMissAction_t action ); 


/** Gets the configured flow miss action.
 * 
 * @param action_p   (IN)  Flow Miss action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 */
extern BpaApiRet_t BPA_Get_FlowMissAction( BpaFlowMissAction_t *action_p ); 

/*
 *------------------------------------------------------------------------------
 * Rate Limiter (RL) APIs
 *
 * Description:
 *    Rate Limiter APIs allow an application to control the rate of a unicast
 *    flow. But, L2 multicast and broadcast flows are not rate limited. 
 *
 *    Every time a EV* (EV, EVV, EVP, ...) packet is received, the TCI field
 *    (outermost VLAN tag) from the packet is ANDed with TCI mask and searched
 *    in the RL table for match against all the configured TCIs.
 *
 *    Match:
 *      The packet flow is rate limited to the configured parameters 
 *      (CIR, CBS) for that flow. 
 *      - Conforming Action: Accept the packet
 *      - Exceed Action    : Drop the packet
 *
 *    NO Match:  Accept the packet.
 *
 *    TCI Mask
 *    It is a 16-bit mask applied across all the RLs in a direction. An
 *    application can have different mask for upstream and downstream.
 *
 *    TCI
 *    The TCI field in RL is matched against the result of ANDing of TCI field
 *    in received packet and TCI mask.
 *
 *    CIR: The CIR rate can be specified with a particular granularity
 *    depending on the CIR rate. If the CIR value is not a multiple of the
 *    specified granularity, the CIR value is rounded up to the next higher
 *    value, e.g. 100Kbps is rounded to 128Kbps. 
 *    - KBPS: Rate is rounded up to the nearest multiple of 64Kbps 
 *       +  64 Kbps for  1 <= Rate <=  8064
 *    - MBPS: Rate is rounded up to the nearest multiple of granularity:
 *       +  1 Mbps for   1 <= Rate <   255
 *       +  2 Mbps for 256 <= Rate <   512
 *       +  4 Mbps for 512 <= Rate <= 1000
 *
 * CBS: CBS (Committed burst size) is specified in bytes.
 *    The application should configure a suitable value for CBS.
 *
 * NOTE:
 *    1. 1Kbps = 1000bps, and 1Mbps = 1024Kbps.
 *    2. While calculating pps, take the following points into consideration:
 *       - Ethernet physical layer overhead (IFG + Preamble) is not included.
 *       - for medium and larger size packets (pktLen >= 256B), pktLen is
 *         calculated based on IPv4/6 Length and assumes the EVI packet. 
 *         If a packet of different matchTag type (say EVVI, EVPI, etc) is
 *         received by RL then a slight error in the pps rate will be observed.
 *
 *
 * CAUTION!!!:
 *    1. printk() from within kernel MAY cause packets to drop. 
 *       prink() disables the IRQs, which causes the delayed handling
 *       of the RL timer interrupt. If a lot of packets are received while
 *       the prink() was running, the RL may run out of tokens and hence drop
 *       the packets.
 *    2. Configure a bigger CBS value, specially for higher rates. 
 *    3. If you see packet drops, make sure there were no printk() active, 
 *       and then try to increase the CBS and check again.
 *
 *
 * USAGE SCENARIO:
 *     1. Upstream direction:
 *        - In upstream direction the incoming port and RL port are different.
 *          So, the application needs to direct the incoming traffic to 
 *          RL port (USB).
 *        - Application should put the USB port in loopback mode.
 *        - Application should allocate and activate an upstream RL with the
 *          required TCI value
 *        - Application should add the required TCI value (VLAN tag) to the 
 *          incoming flow (if not already present) and then direct the flow to 
 *          USB port. This is achieved by creating an FCB with dest port as USB.
 *          flow (if not already present) and then direct the flow to 
 *        - Application should direct the flow received on USB port to the
 *          outgoing port. This is achieved by creating an FCB with 
 *          source port as USB and the required dest port.
 *
  *     2. Downstream direction:
 *        - In downstream direction the RL is on incoming port.
 *        - Application should allocate and activate a downstream RL with the
 *          required TCI value
 *        - The incoming flow should already have TCI value (VLAN tag).
 *          The FCBs are created similar to non-RL scenario.
 *
 * Programming Notes:
 *    1. Enable/disable of RL global status can be done any number of times.
 *       On disabling the RL global status, all RLs are disabled.
 *
 *    Once per direction:
 *    1. Set the RL TCI Cfg (unused TCI and TCI mask).
 *
 *    To use each Rate Limiter:
 *    1. Allocate a Rate limiter.
 *    2. Set the values in the above allocated Rate limiter.
 *    3. Activate the Rate limiter.
 *
 *    To free a Rate Limiter:
 *    1. Deactivate the Rate limiter.
 *    2. Free a Rate limiter.
 *------------------------------------------------------------------------------
 */
#define BPA_MAX_RL               16     /* MAX # of RL in each direction */
#define BPA_MAX_KBPS_RLRATE    8064     /* 8064 Kbps */
#define BPA_MAX_MBPS_RLRATE    1000     /* 1G = 1000M */           

typedef enum {
    e_RL_DIR_US,          /**< Upstream */
    e_RL_DIR_DS,          /**< Downstream */
    e_RL_DIR_MAX,         
} BpaRLDir_t;

typedef enum {
    e_RL_STATUS_DISABLE,  /**< Disable RL Status */
    e_RL_STATUS_ENABLE,   /**< Enable RL Status */
    e_RL_STATUS_MAX,         
} BpaRLStatus_t;

/* CIR unit */
typedef enum {
    e_RL_CIR_UNIT_KBPS,   /** Kbps */
    e_RL_CIR_UNIT_MBPS,   /** Mbps */
    e_RL_CIR_UNIT_MAX,
} BpaRLCirUnit_t;

typedef struct BpaRLCir_t {
    uint32_t        rate;   /** Rate in terms of unit                   */ 
    BpaRLCirUnit_t  unit;   /** Unit in which rate value is interpreted */ 
} BpaRLCir_t;

/** Rate Limiter Entry */
typedef struct BpaRLEnt
{
    uint32_t    tci;        /**< outer TCI (Pbits + VID) in the RL flows */
    BpaRLCir_t  cir;        /**< CIR (rate of the flow ) */
    uint32_t    cbs;        /**< CBS (committed burst size in bytes) */
} BpaRLEnt_t;

typedef struct BpaRLTciCfg
{
    uint16_t    unusedTci;  /**< Unused TCI value */
    uint16_t    mask;       /**< TCI in packet is ANDed with TCI Mask to
                                 identify the RL */
} BpaRLTciCfg_t;


/** Sets the RL TCI configuration (unused TCI and TCI mask) for a 
 *  RL direction. 
 *  - Unused TCI indicates a TCI value which will never be used as a valid
 *    TCI value in the received packets.
 *  - TCI Mask is ANDed with TCI field (from outermost VLAN tag) in 
 *    received packet to find out the matching RL. The TCI mask gives the
 *    flexibility to use just VID or (Pbits + VID) as a rate limiter key.
 *
 * @param rlDir      (IN)  Rate Limiter direction
 * @param rlRLTciCfg (IN) TCI default configuration
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_INVALID_RLDIR
 */
extern BpaApiRet_t BPA_Set_RLTciCfg( BpaRLDir_t rlDir, 
            BpaRLTciCfg_t rlRLTciCfg );

/** Gets the default TCI configuration (unused TCI and TCI mask) for a 
 *  RL direction. 
 *
 * @param rlDir      (IN)  Rate Limiter direction
 * @param rlTciMask_p (OUT) Pointer to TCI default configuraton
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_INVALID_RLDIR
 */
extern BpaApiRet_t BPA_Get_RLTciCfg( BpaRLDir_t rlDir, 
            BpaRLTciCfg_t *rlRLTciCfg_p );


/** Sets the Global RL status 
 * @param rlStatus   (IN)  RL Global status
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_INVALID_RLSTATUS
 *     e_RET_ERR_RESRC_ALREADY_ACTIVE
 *     e_RET_ERR_RESRC_NOT_ACTIVE
 */
extern BpaApiRet_t BPA_Set_RLStatus( BpaRLStatus_t rlStatus );

/** Gets the Global RL status 
 *
 * @param rlStatus_p (OUT) Pointer to RL Global status
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 */
extern BpaApiRet_t BPA_Get_RLStatus( BpaRLStatus_t *rlStatus_p );


/** Allocates a Rate Limiter
 *
 * @param rlDir      (IN)  Rate Limiter direction
 * @param rlIndex    (IN)  Index for the current rate limiter
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_INVALID_RLDIR
 *     e_RET_ERR_INVALID_RLINDEX 
 *     e_RET_ERR_RESRC_UNAVAIL
 */
extern BpaApiRet_t BPA_Alloc_RL( BpaRLDir_t rlDir, uint32_t rlIndex );

/** Frees the Rate Limiter.
 *
 * @param rlDir      (IN)  Rate Limiter direction
 * @param rlIndex    (IN)  Index for the current rate limiter
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_INVALID_RLDIR
 *     e_RET_ERR_INVALID_RLINDEX 
 *     e_RET_ERR_RESRC_BUSY
 */
extern BpaApiRet_t BPA_Free_RL( BpaRLDir_t rlDir, uint32_t rlIndex );

/** Activates the Rate Limiter
 *
 * @param rlDir      (IN)  Rate Limiter direction
 * @param rlIndex    (IN)  Index for the current rate limiter
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_INVALID_RLDIR
 *     e_RET_ERR_INVALID_RLINDEX 
 *     e_RET_ERR_RESRC_ALREADY_ACTIVE
 */
extern BpaApiRet_t BPA_Activate_RL( BpaRLDir_t rlDir, uint32_t rlIndex );

/** Deactivates the Rate Limiter
 *
 * @param rlDir      (IN)  Rate Limiter direction
 * @param rlIndex    (IN)  Index for the current rate limiter
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_INVALID_RLDIR
 *     e_RET_ERR_INVALID_RLINDEX 
 *     e_RET_ERR_RESRC_NOT_ACTIVE
 */
extern BpaApiRet_t BPA_Deactivate_RL( BpaRLDir_t rlDir, uint32_t rlIndex );

/** Set a TCI based Rate Limiter (RL)
 * 
 * @param rlDir      (IN)  Rate Limiter direction
 * @param rlIndex    (IN)  Index for the current rate limiter
 * @param rlEnt      (IN)  Rate Limiter configuration
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_INVALID_RLDIR
 *     e_RET_ERR_INVALID_RLINDEX 
 *     e_RET_ERR_INVALID_RLUNIT
 *     e_RET_ERR_INVALID_RLRATE
 */
extern BpaApiRet_t BPA_Set_RL( BpaRLDir_t rlDir, uint32_t rlIndex, 
            BpaRLEnt_t rlEnt );

/** Gets the info for Rate Limiter.
 * 
 * @param rlDir      (IN)  Rate Limiter direction
 * @param rlIndex    (IN)  Index for the current rate limiter
 * @param rlEnt_p    (OUT) pointer to Rate Limiter configuration
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_RLDIR
 *     e_RET_ERR_INVALID_RLINDEX 
 */
extern BpaApiRet_t BPA_Get_RL( BpaRLDir_t rlDir, uint32_t rlIndex, 
            BpaRLEnt_t *rlEnt_p ); 


/** Updates the new values for active Rate Limiter.
 * CAUTION: The rate limiter should be active for this API to be successful.
 *
 * @param rlDir      (IN)  Rate Limiter direction
 * @param rlIndex    (IN)  Index for the current rate limiter
 * @param rlEnt      (IN)  Rate Limiter configuration
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_INVALID_RLDIR
 *     e_RET_ERR_INVALID_RLINDEX 
 *     e_RET_ERR_RESRC_NOT_ACTIVE
 */
extern BpaApiRet_t BPA_Update_RL( BpaRLDir_t  rlDir, uint32_t rlIndex, 
            BpaRLEnt_t rlEnt );

#define BPA_TPID_MAXNUM             4

typedef struct
{
    BpaPort_t   portId;
    uint16_t    tpid[BPA_TPID_MAXNUM];
} BpaTpidCfg_t ;

/** Sets the TPID configuration 
 *
 * @param  tpidCfg   (IN)  TPID configuration
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_SRCPORT
 */
extern BpaApiRet_t BPA_Set_Tpid( BpaTpidCfg_t *tpidCfg_p );

/** Gets TPID configuration 
 *
 * @param  tpidCfg   (OUT)  TPID configuration
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_SRCPORT
 */
extern BpaApiRet_t BPA_Get_Tpid( BpaTpidCfg_t *tpidCfg_p );

/* 
 * padLen = 0 to 63 (disable padding)
 * padLen = 64 to 255 (enable padding)
 * padLen > 255 (invalid value)
 */
/** Sets the minimum padding length of a packet 
 *
 * @param  srcPort   (IN)  The source port
 * @param  padLen    (IN)  The minumum padding length
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 */
extern BpaApiRet_t BPA_Set_PortPadLen( BpaPort_t srcPort, uint32_t padLen );

/** Gets the minimum padding length of a packet 
 *
 * @param  srcPort   (IN)  The source port
 * @param  padLen    (OUT) pointer to minimum padding length
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 */
extern BpaApiRet_t BPA_Get_PortPadLen( BpaPort_t srcPort, uint32_t *padLen_p );


/** Access control type for antispoofing */
typedef enum BpaAclType
{
    e_ACL_REJECT,           /**< Deny packet when match occurs  */
    e_ACL_ACCEPT,           /**< Allow packet when match occurs */
#if defined(CONFIG_BCM96818)
    e_ACL_SEND2MIPS,        /**< Send to MIPS when match occurs */
#endif
    e_ACL_MAX               /**< Max # of ACL types        */
} BpaAclType_t;

/** Sets the Port Default Access Control type (ACCEPT/REJECT)
 *
 * @param  srcport   (IN)  The source port
 * @param  aclType   (IN)  The Access Control type
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 */
extern BpaApiRet_t BPA_Set_PortDefAcl( BpaPort_t srcport, BpaAclType_t aclType);

/** Gets the port default Access Control type.
 * 
 * @param  srcport   (IN)  The source port
 * @param  aclType_p (OUT)  pointer to Access Control type
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 */
extern BpaApiRet_t BPA_Get_PortDefAcl( BpaPort_t srcport, 
                BpaAclType_t *aclType_p );



/*
 *------------------------------------------------------------------------------
 * Anti-spoofing APIs
 *
 * Description:
 *    Anti-spoofing tables provides security controls. ALLOW (ACCEPT packet) or
 *    DENY (REJECT packet) access controls based on packet parameters may be
 *    configured.
 *
 *    One or more antispoof tables may be associated with a source port.
 *    An antispoof table may be applied to one or more source ports.
 *
 *    An anti-spoofing table may have one more entries. Each entry includes
 *    - a list of packet parameters that may serve as access control fields:
 *      + Ethernet MAC source address (MAC SA)
 *      + Ethernet MAC dest address (MAC DA): Not allowed for IPv6
 *      + Ether Type
 *      + Outer VLAN ID (12 bit)
 *      + Inner VLAN ID (12 bit)
 *      + IP
 *        # IPv4 Source Address
 *          or IPv6 Source Address
 *        # Layer 4 Destination Port
 *        # IP Protocol (Internet layer 4 protocol, e.g. TCP, UDP,)
 *      + Non-IPv4
 *      + Non-IPv6
 *
 *      To signify non-IP (v4 or v6), set both Non-IPv4 and Non-IPv6.
 *      When either Non-IPv4 or Non-IPv6 are set, the IP Source Address and
 *      TU Dport settings are ignored (no warning).
 *
 *    - a bit mask of which parameters are configured in an access control list.
 *      All selected fields must match for a successful table element match.
 *    - Access Control Type: ACCEPT or REJECT access control
 *      ALLOW ACL : ACCEPT access control resulting in normal processing.
 *      DENY  ACL : REJECT access control resulting in packet drop.
 *
 *    A packet received on a source port would be applied against all elements
 *    of the AS table(s) associated with the source port. A match on all
 *    selected fields of any one element of the table(s) constitutes a pass. A
 *    match failure across all elements of all tables associated with a port
 *    constitutes a miss. Upon a miss, the source port global action of ACCEPT
 *    or REJECT will be applied unless explicitly overriden by the matched flow.
 *
 *    A flow may override the application of an antispoof result from a matched
 *    antispoof entry or the port global.
 *
 * Programming Note:
 *    1. The user should first allocate an Anti-spoofing table, which
 *       returns a handle to be used in all subsequent calls for this
 *       table. There is a pool of Anti-spoofing tables in the system.
 *    2. Set the source port(s) associated with this table.
 *    3. Configure the anti-spoofing table entries, to include, the
 *       desired fields values, mask (1b1 signifies field is selected),
 *       and access control type (ACCEPT/REJECT)
 *
 *------------------------------------------------------------------------------
 */

#define BPA_ASTBL_INVALID_HDL   0xFFFFFFFF
#define BPA_ASTBL_MAXNUM        16
#define BPA_ASENT_MAXNUM        8

/** Anti-spoofing table handle 
 *
 * Application should treat the handle type as opaque type.
 */
typedef union BpaAsTblHdl
{
    uint32_t u32;
    struct 
    {
        uint32_t   resvd: 24;   /**< can be used for incarnation      */
        uint32_t   id   :  8;
    } f1;
} BpaAsTblHdl_t;

/** Anti-spoofing table configuration of source port(s) and ACL type */
typedef struct BpaAsTblCfg {
    uint32_t       srcPortMask; /**< source port mask                 */
} BpaAsTblCfg_t;


/** Selection of access control fields in an Anti-spoofing table entry */
typedef enum BpaAsEntAclMask
{
    e_ASENT_MACSA       = 1<<0, /**< Access Control based on MAC SA     */
    e_ASENT_IP4SA       = 1<<1, /**< Access Control based on IPv4 SAddr */
    e_ASENT_ETHTYPE     = 1<<2, /**< Access Control based on Ether Type */
    e_ASENT_VLAN_TAG0   = 1<<3, /**< Access Control based on VLAN TAG0  */
    e_ASENT_VLAN_TAG1   = 1<<4, /**< Access Control based on VLAN TAG1  */
    e_ASENT_L4_DESTPORT = 1<<5, /**< Access Control based on L4 DestPort*/
    e_ASENT_IP_PROTO    = 1<<6, /**< Access Control based on IP Protocol*/
    e_ASENT_NONIPV4     = 1<<7, /**< Access Control based on non-IPv4   */
    e_ASENT_NONIPV6     = 1<<8, /**< Access Control based on non-IPv6   */
    e_ASENT_MACDA       = 1<<9, /**< Access Control based on MAC DA     */
    e_ASENT_IP6SA       = 1<<10,/**< Access Control based on IPv6 SAddr */
    e_ASENT_MAX         = 1<<11,/**< Access Control MAX                 */
} BpaAsEntAclMask_t;

/** MAC Address and the mask 
 *
 * The mask is treated similar to IP mask i.e. a bit=1 in mask means 
 * the addr bit is significant, and a bit=0 in mask means don't care
 * (ignore) the addr bit.
 */
typedef struct BpaMacAddrMask_t
{
    BpaMacAddr_t    addr;      /**< MAC address value */
    BpaMacAddr_t    mask;      /**< MAC address mask  */
} BpaMacAddrMask_t;

typedef struct BpaAsEnt
{
    BpaMacAddrMask_t macda;     /**< Entry MACDA AS Entry           */
    BpaMacAddrMask_t macsa;     /**< Entry MACSA AS Entry           */
    BpaCidrIpAddr_t ipv4sa;     /**< Entry IPv4 SAddr value         */
    uint16_t        ethType;    /**< Entry Ether Type value         */
    uint16_t        vlanTag0;   /**< Entry VLAN Outer Tag value     */
    uint16_t        vlanTag1;   /**< Entry VLAN Inner Tag value     */
    uint16_t        destPort;   /**< Entry L4 Dest Port             */ 
    uint8_t         ipProto;    /**< Entry IP Protocol value        */
    BpaCidrIpv6Addr_t ipv6sa;   /**< Entry IPv6 SAddr value         */
    uint32_t        aclMask;    /**< bit field of selected fields   */
    BpaAclType_t    aclType;    /**< ACL type ( ACCEPT, REJECT).    */
} BpaAsEnt_t;


/** Allocates an Anti-spoofing table
 *
 * @param asTblHdl_p (OUT) Pointer to Anti-spoofing table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_RESRC_UNAVAIL
 */
extern BpaApiRet_t BPA_Alloc_AsTable( BpaAsTblHdl_t *asTblHdl_p );

/** Frees an Anti-spoofing table.
 *
 * @param asTblHdl_p (IN)  pointer to anti-spoofing table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_BUSY
 */
extern BpaApiRet_t BPA_Free_AsTable( const BpaAsTblHdl_t *asTblHdl_p );

/** Activates an Anti-spoofing table.
 *
 * @param asTblHdl_p (IN)  pointer to anti-spoofing table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_ALREADY_ACTIVE
 */
extern BpaApiRet_t BPA_Activate_AsTable( const BpaAsTblHdl_t *asTblHdl_p );

/** Deactivates an Anti-spoofing table.
 *
 * @param asTblHdl_p (IN)  pointer to anti-spoofing table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_NOT_ACTIVE
 */
extern BpaApiRet_t BPA_Deactivate_AsTable( const BpaAsTblHdl_t *asTblHdl_p );



/** Sets Anti-spoofing table configuration of source port(s) and ACL type
 *
 * @param asTblHdl_p (IN)  Anti-spoofing table handle
 * @param asTblCfg_p (IN)  Pointer to the Anti-spoofing table configuration
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_SRCPORT
 *     e_RET_ERR_INVALID_ASTBLTYPE
 *     e_RET_ERR_INVALID_ASMODE
 *     e_RET_ERR_INVALID_ASFAILACTION
 */
extern BpaApiRet_t BPA_Set_AsTableCfg( const BpaAsTblHdl_t *asTblHdl_p, 
                const BpaAsTblCfg_t *asTblCfg_p );

/** Gets Anti-spoofing table configuration.
 *
 * @param asTblHdl_p (IN)  pointer to anti-spoofing table handle
 * @param asTblCfg_p (OUT) Pointer to the Anti-spoofing table configuration
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 */
extern BpaApiRet_t BPA_Get_AsTableCfg( const BpaAsTblHdl_t *asTblHdl_p, 
                BpaAsTblCfg_t *asTblCfg_p );

/** Sets Anti-spoofing table entry at a given index.
 *
 * @param asTblHdl_p (IN)  pointer to anti-spoofing table handle
 * @param asEntIdx   (IN)  Index of the AS Entry to be set
 * @param asEnt_p    (IN)  pointer to the AS Entry 
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_ENTIDX
 *     e_RET_ERR_INVALID_ASENT
 *     e_RET_ERR_INVALID_IPPREFIXLENVAL
 *     e_RET_ERR_RESRC_UNAVAIL
 */
extern BpaApiRet_t BPA_Set_AsTableEntry( const BpaAsTblHdl_t *asTblHdl_p, 
                uint8_t asEntIdx,
                const BpaAsEnt_t *asEnt_p );

/** Gets the configured entry of Anti-spoofing table at a given index.
 *
 * @param asTblHdl_p (IN)  pointer to anti-spoofing table handle
 * @param asEntIdx   (IN)  Index of the AS Entry to get
 * @param asEnt_p    (OUT) pointer to the AS Entry
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_ENTIDX
 */
extern BpaApiRet_t BPA_Get_AsTableEntry( const BpaAsTblHdl_t *asTblHdl_p,
                uint8_t asEntIdx,
                BpaAsEnt_t *asEnt_p );

/** Deletes an Anti-spoofing table entry at a given index.
 *
 * @param asTblHdl_p (IN)  pointer to anti-spoofing table handle
 * @param asEntIdx   (IN)  Index of the AS Entry to delete
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_NO_SUCH_ENTRY
 */
extern BpaApiRet_t BPA_Delete_AsTableEntry( const BpaAsTblHdl_t *asTblHdl_p, 
                uint8_t asEntIdx);

/*
 *------------------------------------------------------------------------------
 * DSCP to PBits Mapping APIs
 *------------------------------------------------------------------------------
 *
 * Programming Note:
 *    1. In most cases there will be one DSCP-to-Pbits mapping table per port.
 *    2. A Dscp Hdl is returned whenever a DSCP table is allocated.
 *    3. The returned Dscp Hdl should be used in all subsequent calls 
 *       referring to above created DSCP table.
 *    4. Dscp Hdl is destroyed once the free API for the DSCP is called.
 *    5. An API has been provided to set the default Pbits value
 *       for all the undefined DSCP values.
 *------------------------------------------------------------------------------
 */

/** DSCP table handle
 *
 * Application should treat the handle type as opaque type.
 */

#define BPA_PBITS_MAX               8
#define BPA_DSCP_MAX                64
#define BPA_DSCPTBL_INVALID_HDL     0xFFFFFFFF
#define BPA_DSCPTBL_MAXNUM          BPA_MAX_UNI
#define BPA_DSCPENT_MAXNUM          64

typedef union BpaDscpTblHdl
{
    uint32_t u32;
    struct 
    {
        uint32_t   resvd: 24;   /**< can be used for incarnation      */
        uint32_t   id   :  8;   /**< right now max of 4 DSCP Tables  */
    } f1;
} BpaDscpTblHdl_t;

typedef struct BpaDefPbits
{
    BpaPort_t srcPort; 
    uint8_t   pbits;
} BpaDefPbits_t;

typedef struct BpaDscpToPbitsMapping
{
    uint8_t resvd; 
    uint8_t startDscp; 
    uint8_t endDscp; 
    uint8_t pbits;
} BpaDscpToPbitsMapping_t;

/** Allocates a DSCP-to-Pbits mapping table.
 *
 * @param dscpHdl_p  (OUT) pointer to DSCP table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_RESRC_UNAVAIL
 */
extern BpaApiRet_t BPA_Alloc_DscpToPbitsTable( BpaDscpTblHdl_t *dscpHdl_p );

/** Frees a DSCP-to-Pbits mapping table.
 *
 * @param dscpHdl_p  (IN)  pointer to DSCP table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_BUSY
 */
extern BpaApiRet_t BPA_Free_DscpToPbitsTable( const BpaDscpTblHdl_t *dscpHdl_p);

/** Activates a DSCP-to-Pbits mapping table.
 *
 * @param dscpHdl_p  (IN)  pointer to DSCP table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_UNAVAIL
 *     e_RET_ERR_RESRC_ALREADY_ACTIVE
 */
extern BpaApiRet_t BPA_Activate_DscpToPbitsTable( 
                const BpaDscpTblHdl_t *dscpHdl_p );

/** Deactivates a DSCP-to-Pbits mapping table.
 *
 * @param dscpHdl_p  (IN)  pointer to DSCP table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_NOT_ACTIVE
 */
extern BpaApiRet_t BPA_Deactivate_DscpToPbitsTable( 
                const BpaDscpTblHdl_t *dscpHdl_p );

/** Sets the default Pbits value for the unmapped DSCP values.
 *
 * @param dscpHdl_p  (IN)  pointer to DSCP table handle
 * @param defPbits_p (IN)  pointer to defPbits
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_INVALID_SRCPORT
 *     e_RET_ERR_INVALID_INVALID_PBITS
 */
extern BpaApiRet_t BPA_Set_DscpToPbitsTable_DefPbits(
                const BpaDscpTblHdl_t *dscpHdl_p,
                const BpaDefPbits_t *defPbits_p); 

/** Gets the configured default Pbits value for the unmapped DSCP values.
 *
 * @param dscpHdl_p  (IN)  ponter to DSCP table handle
 * @param defPbits_p (OUT) pointer to defPbits
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 */
extern BpaApiRet_t BPA_Get_DscpToPbitsTable_DefPbits(
                const BpaDscpTblHdl_t *dscpHdl_p,
                BpaDefPbits_t *defPbits_p); 

/** Sets the DSCP-to-Pbits mapping.
 *
 *  A range of DSCP values (using startDscp and endDscp parameters)
 *  can be mapped to a pbits value.
 *  To map an individual DSCP value use the startDscp value equal to
 *  endDscp value.
 *
 * @param dscpHdl_p  (IN)  pointer to DSCP table handle
 * @param dscpMapping_p (IN)  pointer to DSCP to Pbits mapping
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_RANGE
 */
extern BpaApiRet_t BPA_Set_DscpToPbitsTable( 
                const BpaDscpTblHdl_t *dscpHdl_p, 
                const BpaDscpToPbitsMapping_t *dscpMapping_p );

/** Gets the configured Pbits value for the requested DSCP value.
 *
 * This function may be used to retrieve the pBits value corresponding to DSCP
 * values that may have been configured using a range based specification. The
 * DSCP range information may not be retained as the underlying layer may use
 * an entry per DSCP value.
 *
 * @param dscpHdl_p  (IN)  pointer to DSCP table handle
 * @param dscp       (IN)  the DSCP value
 * @param pbits_p    (OUT) pointer to the pbits value
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_DSCPVAL
 */
extern BpaApiRet_t BPA_Get_DscpToPbitsTable( 
                const BpaDscpTblHdl_t *dscpHdl_p, 
                uint8_t dscp, 
                uint8_t *pbits_p );

/*
 *------------------------------------------------------------------------------
 * Pbits to DestQ Mapping Table
 *------------------------------------------------------------------------------
 * One Pbits2DestQ mapping table may be configured per switch port. Currently
 * only GPON port is supported. One or more Flows may be associated with a Pbits2DestQ table,
 * where a unique destination-Q per pbits may be specified. 
 *
 * When a flow is associated with a Pbits2DestQ table, the source of the pbits must always be
 * from outter most VLAN Tag, hence this table can only be associated with EV.. type of flows.
 * 
 * When a table is created, a default configuration must be applied to all table
 * entries, see BPA_Set_Pbits2DestQTable_DefDestQ(). This API is used to reinitialize the entire table.
 * Later per pBit, a unique destination-Q may be configured, see BPA_Set_Pbits2DestQTable() and
 * BPA_Get_Pbits2DestQTable().
 * 
 * When a Pbits2DestQ table is associated with a flow, the destination-Q specified in the flow
 * entry will be overridden by the destination-Q selected based on Pbits on the incoming packet.
 * 
 * CAUTION:
 * Active Flows associated with a Pbits2DestQ table must
 * be deactivated prior to deactivation of Pbits2DestQ table.
 *
 *------------------------------------------------------------------------------
 */
/*
 *------------------------------------------------------------------------------
 * Pbits to DestQ Mapping APIs
 *------------------------------------------------------------------------------
 *
 * Programming Note:
 *    1. There will be only one Pbits-to-DestQ mapping table per port.
 *    2. A Pbits2DestQ Hdl is returned whenever a PbitsToDestQ table is allocated.
 *    3. The returned Pbits2DestQ Hdl should be used in all subsequent calls 
 *       referring to above created PbitsToDestQ table.
 *    4. Pbits2DestQ Hdl is destroyed once the free API for the PbitsToDestQ table is called.
 *    5. An API has been provided to set the default DestQ value
 *       for all the undefined Pbits values.
 *    6. DestQ:1 cannot be selected. See 681x errata document.
 *    7. DestQ:0 cannot be selected due to switch limitation.
 *    8. Activation of Pbits2DestQ table on a port does not have any effect until it is
 *       bound to a flow.
 *    9. Only the flows that are bound with Pbits2DestQ table will override the
 *       switch or flow destination-Q selection.
 *   10. This table can be associated with flows that have only ONE ingress srcport bitmask
 *       and that srcport should match with the sourceport of this table.
 *------------------------------------------------------------------------------
 */

/** Pbits2DestQ table handle
 *
 * Application should treat the handle type as opaque type.
 */

#define BPA_PBITS2DESTQTBL_MAXNUM       (1) /* Only for GPON; Need to optimize to avoid memory usage */

typedef union BpaPbits2DestQTblHdl
{
    uint32_t u32;
    struct 
    {
        uint32_t   resvd: 29;   /**< can be used for incarnation      */
        uint32_t   id   :  3;   /**< right now max of 8 Pbits2DestQ Tables  */
    } f1;
} BpaPbits2DestQTblHdl_t;

typedef struct BpaDefDestQ
{
    BpaPort_t srcPort; 
    uint8_t   destQ;
} BpaDefDestQ_t;

typedef struct BpaPbits2DestQMapping
{
    uint8_t resvd1; 
    uint8_t resvd2; 
    uint8_t pbits; 
    uint8_t destQ;
} BpaPbits2DestQMapping_t;

/** Allocates a Pbits-to-DestQ mapping table.
 *
 * @param pbits2DestQHdl_p (OUT) pointer to Pbits2DestQ table 
 *                          handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_RESRC_UNAVAIL
 */
extern BpaApiRet_t BPA_Alloc_Pbits2DestQTable( BpaPbits2DestQTblHdl_t *pbits2DestQHdl_p );

/** Frees a Pbits-to-DestQ mapping table.
 *
 * @param pbits2DestQHdl_p  (IN)  pointer to Pbits2DestQ table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_BUSY
 */
extern BpaApiRet_t BPA_Free_Pbits2DestQTable( const BpaPbits2DestQTblHdl_t *pbits2DestQHdl_p);

/** Activates a Pbits-to-DestQ mapping table.
 *
 * @param pbits2DestQHdl_p  (IN)  pointer to Pbits2DestQ table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_UNAVAIL
 *     e_RET_ERR_RESRC_ALREADY_ACTIVE
 */
extern BpaApiRet_t BPA_Activate_Pbits2DestQTable( 
                const BpaPbits2DestQTblHdl_t *pbits2DestQHdl_p );

/** Deactivates a Pbits-to-DestQ mapping table.
 *
 * @param pbits2DestQHdl_p  (IN)  pointer to Pbits2DestQ table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_NOT_ACTIVE
 */
extern BpaApiRet_t BPA_Deactivate_Pbits2DestQTable( 
                const BpaPbits2DestQTblHdl_t *pbits2DestQHdl_p );

/** Sets the default DestQ value for the unmapped Pbits values.
 *
 * @param pbits2DestQHdl_p  (IN)  pointer to Pbits2DestQ table handle
 * @param defDestQ_p (IN)  pointer to defDestQ
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_INVALID_SRCPORT
 *     e_RET_ERR_INVALID_INVALID_DESTQ
 */
extern BpaApiRet_t BPA_Set_Pbits2DestQTable_DefDestQ(
                const BpaPbits2DestQTblHdl_t *pbits2DestQHdl_p,
                const BpaDefDestQ_t *defDestQ_p); 

/** Gets the configured default DestQ value for the unmapped
 *  Pbit values.
 *
 * @param pbits2DestQHdl_p  (IN)  ponter to Pbits2DestQ table handle
 * @param defDestQ_p (OUT) pointer to defDestQ
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 */
extern BpaApiRet_t BPA_Get_Pbits2DestQTable_DefDestQ(
                const BpaPbits2DestQTblHdl_t *pbits2DestQHdl_p,
                BpaDefDestQ_t *defDestQ_p); 

/** Sets the Pbits-to-DestQ mapping.
 *
 * @param pbits2DestQHdl_p  (IN)  pointer to Pbits2DestQ table handle
 * @param pbits2DestQMapping_p (IN)  pointer to Pbits to DestQ 
 *                         mapping
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_RANGE
 */
extern BpaApiRet_t BPA_Set_Pbits2DestQTable( 
                const BpaPbits2DestQTblHdl_t *pbits2DestQHdl_p, 
                const BpaPbits2DestQMapping_t *pbits2DestQMapping_p );

/** Gets the configured DestQ value for the requested Pbits
 *  value.
 *
 * @param pbits2DestQHdl_p  (IN)  pointer to Pbits2DestQ table handle
 * @param pbits       (IN)  the Pbits value
 * @param destQ_p    (OUT) pointer to the destQ value
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_PBITSVAL
 */
extern BpaApiRet_t BPA_Get_Pbits2DestQTable( 
                const BpaPbits2DestQTblHdl_t *pbits2DestQHdl_p, 
                uint8_t pbits, 
                uint8_t *destQ_p );


#define BPA_EPTBL_INVALID_HDL       0xFFFFFFFF
#define BPA_EPTBL_MAXNUM            BPA_MAX_UNI
#define BPA_EPENT_MAXNUM            8

typedef union BpaEpTblHdl
{
    uint32_t u32;
    struct 
    {
        uint32_t   resvd: 24;   /**< can be used for incarnation      */
        uint32_t   id   :  8;   /**< right now max of 4 Tables        */
    } f1;
} BpaEpTblHdl_t;

typedef struct BpaEpTblCfg
{
    BpaPort_t srcPort; 
} BpaEpTblCfg_t;

typedef struct BpaEpMapping
{
    uint8_t ethType; 
    uint8_t pbits;
} BpaEpMapping_t;


/** Allocates a EthType-to-Pbits mapping table.
 *
 * @param epTblHdl_p (OUT) pointer to EthType table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_RESRC_UNAVAIL
 */
extern BpaApiRet_t BPA_Alloc_EpTable( BpaEpTblHdl_t *epTblHdl_p );

/** Frees a EthType-to-Pbits mapping table.
 *
 * @param epTblHdl_p (IN)  pointer to EthType table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_BUSY
 */
extern BpaApiRet_t BPA_Free_EpTable( const BpaEpTblHdl_t *epTblHdl_p);

/** Activates a EthType-to-Pbits mapping table.
 *
 * @param epTblHdl_p (IN)  pointer to EthType table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_ALREADY_ACTIVE
 */
extern BpaApiRet_t BPA_Activate_EpTable( const BpaEpTblHdl_t *epTblHdl_p );

/** Deactivates a EthType-to-Pbits mapping table.
 *
 * @param epTblHdl_p (IN)  pointer to EthType table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_ALREADY_NOT_ACTIVE
 */
extern BpaApiRet_t BPA_Deactivate_EpTable( 
                const BpaEpTblHdl_t *epTblHdl_p );

/** Sets the configuration for the ethType table.
 *
 * @param epTblHdl_p (IN)  pointer to EthType table handle
 * @param epTblCfg_p (IN)  pointer to EthType table configuration
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 */
extern BpaApiRet_t BPA_Set_EpTableCfg(
                const BpaEpTblHdl_t *epTblHdl_p,
                const BpaEpTblCfg_t *epTblCfg_p); 

/** Gets the configuration for the ethType table.
 *
 * @param epTblHdl_p (IN)  pointer to EthType table handle
 * @param epTblCfg_p (OUT) pointer to EthType table configuration
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 */
extern BpaApiRet_t BPA_Get_EpTableCfg(
                const BpaEpTblHdl_t *epTblHdl_p,
                BpaEpTblCfg_t *epTblCfg_p); 

/** Sets the EthType-to-Pbits mapping table entry.
 *
 * @param epTblHdl_p (IN)  pointer to EthType table handle
 * @param epEntIdx   (IN)  the EthType table entry index
 * @param epMapping_p (IN)  pointer to EthType to Pbits mapping
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 */
extern BpaApiRet_t BPA_Set_EpTableEntry( 
                const BpaEpTblHdl_t *epTblHdl_p, 
                uint8_t epEntIdx,
                const BpaEpMapping_t *epMapping_p );

/** Gets the EthType-to-Pbits mapping table entry.
 *
 * @param epTblHdl_p (IN)  pointer to EthType table handle
 * @param epEntIdx   (IN)  the EthType table entry index
 * @param epMapping_p (OUT) pointer to EthType to Pbits mapping
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 */
extern BpaApiRet_t BPA_Get_EpTableEntry( 
                const BpaEpTblHdl_t *epTblHdl_p, 
                const uint8_t epEntIdx,
                BpaEpMapping_t *epMapping_p );


/*
 *------------------------------------------------------------------------------
 * PBits to Action APIs
 *------------------------------------------------------------------------------
 *
 * One or more Flows may be associated with a PBits to Action(PAC) table, where
 * a unique action per pbits may be specified. The table may contain a max of 8
 * action entries, one per pBit value.
 *
 * When a flow is associated with a PAC table, the source of the pbits must be
 * specified during association, see BPA_Set_PacInfo().
 * 
 * When a table is created, a default configuration may be applied to all table
 * entries, see BPA_Set_PacTableCfg() and BPA_Get_PacTableCfg(). This API may be
 * used to reinitialize the entire table.
 *
 * Per pBit, a unique action may be configured, see BPA_Set_PacTableEntry() and
 * BPA_Get_PacTableEntry().
 *
 * The following actions may be specified per Pbit. These actions will override
 * the configuration specified in associated Flow(s) FCB.
 *  - Override the AS Disable configuration 
 *  - Override the DROP configuration
 *  - Override the Dest Port map configuration and Send to MIPS.
 *  - Override the Dest GemPort (need to specifiy GemPort value) configuration
 *  - Override the Dest queue priority (need to specify the Dest QId value)
 *  - Override the Raw Actions (if any) in the associated Flow(s).
 *
 * PS. The DestQ priority may be overriden independently of the Dest GemPort.
 *
 * CAUTION:
 * Activation of the PBits 2 Action table must preceed the activation of any
 * associated flows. Active Flows associated with a PBits 2 Action table must
 * be deactivated prior to deactivation of PBits 2 Action table.
 *
 *------------------------------------------------------------------------------
 */

#define BPA_PACACT_MAXNUM       2 /**< Maximum number of actions per entry */

/* When these control bits are set, it overrides the associated FCB's corresponding configuration */
typedef enum BpaPacEntOvrd
{
    e_PACTBL_OVRD_NONE      = 0,    /**< NO Override */
    e_PACTBL_OVRD_ASDIS     = 1<<0, /**< Override FCB's configuration of disable AS */
    e_PACTBL_OVRD_DROP      = 1<<1, /**< Override FCB's forwarding information and drop packet */
    e_PACTBL_OVRD_SEND2MIPS = 1<<2, /**< Override FCB's destination port mask and send to MIPS */
    e_PACTBL_OVRD_DSTGEM    = 1<<3, /**< Override FCB's destination GEM Port */
    e_PACTBL_OVRD_DSTQID    = 1<<4, /**< Override FCB's GEM Port Qid OR Port Qid */
    e_PACTBL_OVRD_MAX       = 1<<5, /**< Override MAX */
} BpaPacEntOvrd_t;

typedef struct BpaPacEnt
{
    uint16_t ovrd;                  /**< Bit mask of action overrides */
    uint8_t dstGemPort;
    uint8_t dstQid;
    BpaRawAction_t action[BPA_PACACT_MAXNUM];
} BpaPacEnt_t;


#define BPA_PACTBL_INVALID_HDL  0xFFFFFFFF
#define BPA_PACTBL_MAXNUM       16
#define BPA_PACENT_MAXNUM       BPA_PBITS_MAX

typedef union BpaPacTblHdl
{
    uint32_t u32;
    struct 
    {
        uint32_t   resvd: 24;   /**< can be used for incarnation      */
        uint32_t   id   :  8;   /**< right now max of 4 Tables        */
    } f1;
} BpaPacTblHdl_t;

/** Allocates a Pbits-to-Action Table.
 *
 * @param pacTblHdl_p (OUT) pointer to Pbits-to-Action Table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_RESRC_UNAVAIL
 */
extern BpaApiRet_t BPA_Alloc_PacTable( BpaPacTblHdl_t *pacTblHdl_p );

/** Frees a Pbits-to-Action Table.
 *
 * @param pacTblHdl_p (IN)  pointer to Pbits-to-Action Table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_BUSY
 */
extern BpaApiRet_t BPA_Free_PacTable( const BpaPacTblHdl_t *pacTblHdl_p );

/** Activates a Pbits-to-Action table.
 *
 * @param pacTblHdl_p (IN)  pointer to Pbits-to-Action Table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_ALREADY_ACTIVE
 */
extern BpaApiRet_t BPA_Activate_PacTable( const BpaPacTblHdl_t *pacTblHdl_p );

/** Deactivates a Pbits-to-Action table.
 *
 * @param pacTblHdl_p (IN)  pointer to Pbits-to-Action Table handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_NOT_ACTIVE
 */
extern BpaApiRet_t BPA_Deactivate_PacTable( const BpaPacTblHdl_t *pacTblHdl_p );

/** Sets Pbits-to-Action table configuration.
 *
 *  This API may be used to define the default common configuration to be used
 *  for all table entries that have not been explicitly configured using the API
 *  BPA_Set_PacTableEntry().
 *  If the default common configuration is not explicitly set, a configuration
 *  of overrides=0 and actions=none will be used for entries not explicitly set.
 *
 * @param pacTblHdl_p (IN)  Pbits-to-Action table handle
 * @param pacTblCfg_p (IN)  Pointer to the Pbits-to-Action port table configuration
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_SRCPORT
 *     e_RET_ERR_INVALID_GEMPORTIDX
 */
extern BpaApiRet_t BPA_Set_PacTableCfg( const BpaPacTblHdl_t *pacTblHdl_p, 
                                        const BpaPacEnt_t *pacTblCfg_p );

/** Gets Pbits-to-Action table configuration.
 *
 * @param pacTblHdl_p (IN)  pointer to the Pbits-to-Action table handle
 * @param pacTblCfg_p (OUT) Pointer to the Pbits-to-Action table configuration
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 */
extern BpaApiRet_t BPA_Get_PacTableCfg( const BpaPacTblHdl_t *pacTblHdl_p, 
                                        BpaPacEnt_t *pacTblCfg_p );

/** Sets a Pbits-to-Action table entry.
 *
 * The Pbits-to-Action table will use final Pbit value after packet processing
 * for forwarding. In other words, if DSCP to 1P, or Pbit translation, or just
 * plain Pbit classification is performed, the resulting Pbit value is used to
 * determine the GEM port ID.
 *
 * @param pacTblHdl_p (IN)  pointer to Pbits-to-Action Table handle
 * @param pacTblEntry_p (IN)  pointer to PAC to Pbits mapping
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_PBITS
 *     e_RET_ERR_INVALID_GEMPORTIDX
 */
extern BpaApiRet_t BPA_Set_PacTableEntry( const BpaPacTblHdl_t *pacTblHdl_p,
                                          uint8_t pbits, 
                                          const BpaPacEnt_t *pacEnt_p );

/** Gets the Pbits-to-Action port mapping table entry.
 *
 * @param pacTblHdl_p (IN)  pointer to Pbits-to-Action Table handle
 * @param pacTblEntry_p (OUT) pointer to Pbits-to-Action port mapping
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_PBITS
 */
extern BpaApiRet_t BPA_Get_PacTableEntry( const BpaPacTblHdl_t *pacTblHdl_p,
                                          uint8_t pbits,
                                          BpaPacEnt_t *pacEnt_p );

/*
 *******************************************************************************
 * Flow APIs
 *******************************************************************************
 *
 * Description:
 *    The system can have a number of packet flows, and each packet flow can
 *    be characterised by a set of fields of packets, and/or some system 
 *    attribute like port. The flows can be identified by setting up 
 *    filters for all the required header fields. Once the flow is 
 *    identified, it is possible to modify a few of the fields in the 
 *    packet header before forwarding the modified packet to the 
 *    destination port(s). 
 *
 *    A flow may be optionally associated with an Anti-spoofing table,
 *    which decides whether to accept or reject the access to the ingress 
 *    packet. If the packet fails the table criterion it is possible to 
 *    configure a fail action on the failed packet.
 *
 *    There is a common set of APIs for unicast and multicast flows. 
 *    Each packet flow (unicast and multicast) is represented by
 *    an FCB throughout the life of the flow.  
 *
 *    The available total flow memory is divided into three flow 
 *    sections: defined, provisioned, and dynamic. User can dimension
 *    the size of each of these sections. The defined flows are well-known 
 *    flows which get created at initialization time and remain active. 
 *    The provisioned flows are created using CLI or through library API 
 *    calls and remain active for long time after creation. The dynamic 
 *    flows are created by the network software automatically as and when 
 *    new flows are learnt and torn down when the flow is inactive for 
 *    specified period of time.
 *
 *    A flow has three blocks: filter, modify, and forwarding. 
 *    Filter Block:
 *       The filter block allocates all the headers of interest and then 
 *       configures the filters for the flow headers. It is allowed to
 *       configure filter on multiple fields. The net effect of these
 *       multiple filters is an AND of all the filters configured on
 *       the flow.
 *
 *    Modify Block:
 *       The modify block allocates all the headers of interest and then 
 *       configures the modify actions for the flow headers. It is also
 *       possible to associate (optional) a DSCP-to-Pbit mapping table,
 *       with the flow, for ingress untagged packets.
 *
 *    Forwarding Block:
 *       The forwarding block has the flow priority, source, destination 
 *       port(s) and IP multicast group configuration information. Each
 *       flow is assigned a flow priority by the caller of the API.
 *
 *    Flow Headers:
 *       Each Flow can have different combination of headers and user 
 *       needs to allocate the flow headers. Some of the possible
 *       combinations could be: EP, EVP, EVVP, etc., where E is Eth
 *       header and V is vlan tag and P is PPPoE.
 *
 * Programming Note:
 *    Unicast flow:
 *    1. User must call allocate FCB once in the life of flow to
 *       create the FCB, which returns a flow handle to be used in all
 *       subsequent calls to refer to this flow.
 *    2. User must specify the source port mask, destination port mask
 *       and the forwarding action using the fowarding API.
 *    3. User must allocate flow headers for each header of interest in
 *       the filter and modification blocks.
 *    4. User may call one or more of the filter APIs to identify the 
 *       flow. If no filter is specified by these APIs then the packets
 *       are automatically filtered based on the source port configured
 *       in forwarding block. 
 *    5. Then the user may call one of more the modify action APIs
 *       to change the packet headers in the outgoing packets of the flow. 
 *    6. If the Anti-spoofing is required then an already allocated
 *       and configured Anti-spoofing table is linked to the flow.
 *    7. If the DSCP-to-Pbits mapping is required then an already allocated
 *       and configured DSCP table is linked to the flow.
 *    8. Once the configuration has been completed flow needs to be
 *       activated.
 *    9. The user can free/delete a flow when it is no more required.
 *
 *    Multicast flow:
 *    1. A multicast flow can be configured similar to the unicast flow
 *       with a few additional changes. 
 *    2. When calling the forwarding API the destMask parameter has a 
 *       port mask for all the ports on which the packet needs to be 
 *       replicated and impg parameter has the IP multicast group 
 *       information.
 *    3. A hardware accelerator may have a limitation wherein for the insertion
 *       of a per-port VLAN tag, the port may expect a TPID of 0x8100 and a
 *       unique (unused) VLAN-ID of 0xFFF. The switch port recongnizes such a
 *       combination of TPID=0x8100 and VLAN-ID=0xFFF to trigger a per
 *       port VLAN-ID to be replaced.
 *       Configuration of the per switch port VLAN-ID is not covered by the
 *       BPA APIs, rather it would be done via the ethernet control utility or
 *       ethernet userspace ioctl APIs.
 *       For instance, if a tagged packet was received, a modification to
 *       replace the TPID to 0x8100 (if it was different) and the VLAN-ID to
 *       0xFFF could be configured in the FCB. Alternatively, a untagged flow
 *       could have been configured to insert the above combination.
 *
 *******************************************************************************
 */

/** Flow Handle
 *
 * Application should treat the handle type as opaque type.
 */

#define BPA_FLOW_INVALID_HDL    0xFFFFFFFF
#define BPA_MAX_FLOWS           512

typedef union BpaFlowHdl
{
    uint32_t   u32;
    struct 
    {
        uint32_t   resvd: 16;   /**< can be used for incarnation  */
        uint32_t   id   : 16;   /**< right now max of 512 flows   */
    } f1;
} BpaFlowHdl_t;

 /* The FCB can have 1 or 8 flows depending on the FCB configuation.
  * When PacTbl is not used, there is only 1 flow (numFlows=1) and the hits
  * for the flow is available at hits[0].
  *
  * When PacTbl is used the FCB explodes to 8 flows (numFlows), and hits for 
  * each flow can be accesed using pbits as index in hits[].
  */
typedef struct BpaFCBHits
{
    uint32_t   numFlows;        /**< number of valid flows (1 or 8) of an FCB */
    uint32_t   hits[BPA_PBITS_MAX];  /**< hits count array for FCB flows      */
} BpaFCBHits_t;

/*
 *------------------------------------------------------------------------------
 * FCB APIs
 *------------------------------------------------------------------------------
 *
 * A flow is represented by one FCB. The FCB APIs allow the user to allocate,
 * activate and free the FCB.
 *------------------------------------------------------------------------------
 */

/** Allocates an FCB (Flow Control Block).
 *
 *  This is the first API to be called for the flow. The result of this API is 
 *  the allocation of an FCB and the flowHdl returned to the 
 *  caller. This flowHdl is the handle for the flow for all the future calls 
 *  to identify a particular flow.
 *  NOTE:- This API should be called only once for a flow at the beginning
 *         to allocate the FCB for the flow.
 *
 * @param section    (IN)  Flow section types
 * @param flowHdl_p  (OUT) pointer to flow handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_RESRC_UNAVAIL
 */
extern BpaApiRet_t BPA_Alloc_FCB( BpaFlowSection_t section,
                BpaFlowHdl_t *flowHdl_p);


#if 0
/** Query an FCB.
 *
 * This API internally calls the QUERY_FCB ioctl. The ioctl updates 
 * the FCB with the previously configured info in the HAL for the flow.
 *
 * @param flowHdl_p   (IN) pointer to flow handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 */
extern BpaApiRet_t BPA_Query_FCB( const BpaFlowHdl_t *flowHdl_p );
#endif


/** Activates an FCB.
 *
 * This API activates the flow after configuration has been done by the 
 * previous set of APIs. After this API the flow is active and packets
 * can pass for the flow.
 *
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_ALREADY_ACTIVE
 *     e_RET_ERR_INVALID_PAC_PBITS2DESTQ_COMBO
 */
extern BpaApiRet_t BPA_Activate_FCB( const BpaFlowHdl_t *flowHdl_p );


/** Deactivates an FCB.
 *
 * This API deactivates the flow.  After this API the flow is inactive and 
 * packets cannot pass for the flow.
 *
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_NOT_ACTIVE
 */
extern BpaApiRet_t BPA_Deactivate_FCB( const BpaFlowHdl_t *flowHdl_p );


/** Frees an FCB
 *
 * The BPA_Free_FCB() is the last API to be called before a flow is
 * deleted. The result of this API is the free of an FCB, and flowHdl is
 * available for reuse by the APIs. The flowHdl should not be reused until 
 * BPA_Alloc_FCB() API is successfully invoked again.
 * This API will also free all the associations (if there is any) with the 
 * tables like: AsTable, DscpToPbitsTable, EpTable, etc.
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_BUSY
 */
extern BpaApiRet_t BPA_Free_FCB( const BpaFlowHdl_t *flowHdl_p );


/** Gets FCB Hits.
 *
 * This API gets the FCB hits since the last invocation of this API. The FCB
 * should be in activated state for the API to return valid values.
 *
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param fcbHits_p  (OUT) pointer to FCB Hits struct
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_RESRC_NOT_ACTIVE
 */
extern BpaApiRet_t BPA_Get_FCBHits( const BpaFlowHdl_t *flowHdl_p, 
                BpaFCBHits_t *fcbHits_p );



/*
 *------------------------------------------------------------------------------
 * Flow Header APIs
 *------------------------------------------------------------------------------
 *
 * Allocates space for a packet header inside the FCB, it should be called 
 * once for each header of interest (e.g. Eth, VLAN0, PPPoE, IPv4, etc.) 
 * after BPA_Alloc_FCB() has been successful.
 * Basically this set of APIs indicates what all headers the API should 
 * expect to filter and/or modify in the packets. 
 *
 * In case of insertion of vlan tags the number of flow headers for filter
 * and modification blocks will be different. 
 *
 * CAUTION:
 *    VLAN TAG PUSH/INSERT
 *       In case of push/insert of VLAN tag the user should allocate extra 
 *       flow header for the new tag (just added) in the modification 
 *       block. VLAN tags are always inserted as the outermost VLAN tag
 *       (VLAN TAG0). After insertion all the existing tags are shifted 
 *       inside by one level. It is possible to push multiple tags.
 *       The push action should be acted immediately before the 
 *       first VLAN action.
 *
  *    VLAN TAG POP/DELETE
 *       In case of pop/delete of VLAN tag the user does not need to
 *       allocate extra flow header for the poped tag (deleted) in 
 *       the modification block. VLAN tags are always poped or deleted
 *       at the outermost VLAN tag (VLAN TAG0). After popping all the 
 *       existing tags are shifted outside by one level.
 *       The pop action will be always acted after VLAN actions have
 *       completed.
 *
 * Note:- There is no API to free flow headers. In case the header needs to be
 * be freed, it is better to free FCB and reconfigure the flow.
 *------------------------------------------------------------------------------
 */

typedef struct BpaAllocHdr
{
    BpaFcbBlk_t  blk;
    BpaFlowHdr_t flowHdr;
} BpaAllocHdr_t;

/** Allocates a packet header inside the FCB.
 * 
 * Allocates space for a packet header inside the FCB, it should be called 
 * once for each header of interest (e.g. Eth, VLAN0, PPPoE, IPv4, etc.) 
 * after BPA_Alloc_FCB() has been successful.
 * Basically this call indicates what all headers the API should expect to
 * filter and/or modify in the packets.
 * There is an order in allocation of headers which is used for validation. 
 * The Ethernet header should be always allocated first. There are some 
 * examples given below for different configurations, which shows the order
 * of flow header allocations:
 * EV Config: 
 *    The order is: E, VLAN0
 * EVVVV Config: 
 *    The order is: E, VLAN0, VLAN1, VLAN2, VLAN3
 * EVVP Config: 
 *    The order is: E, VLAN0, VLAN1, PPPOE
 * EI Config: 
 *    The order is: E, IPV4
 * EVI Config: 
 *    The order is: E, VLAN0, IPV4 
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param hdr_p      (IN)  pointer to the Alloc header
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_HDRTYPE
 *     e_RET_ERR_INVALID_BLOCKTYPE
 *     e_RET_ERR_HDROUTOFORDER
 */
extern BpaApiRet_t BPA_Alloc_Hdr( const BpaFlowHdl_t *flowHdl_p, 
                const BpaAllocHdr_t *hdr_p );

/** Dumps all the configured requests for a header.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param hdr_p      (IN)  pointer to the Alloc header
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_HDRTYPE
 *     e_RET_ERR_INVALID_BLOCKTYPE
 */
extern BpaApiRet_t BPA_Dump_HdrReqList( const BpaFlowHdl_t *flowHdl_p, 
                const BpaAllocHdr_t *hdr_p );



/*
 *------------------------------------------------------------------------------
 * Forwarding Block APIs
 *------------------------------------------------------------------------------
 *
 * Configures source, destination port mask information and the forwarding
 * action for the flow.
 * In case of multicast flow destination port mask  will have bits set for all
 * the destination ports.
 *
 * Flow Priority determines the flow priority/precedence among different flows
 * in the system. A group of flows may have the same priority. The lower 
 * numerical value indicates higher priority.
 *
 * These are the forwarding actions:
 *  e_FWD_ACTION_DROP: 
 *      When this bit is 1, the dstMask of BpaFwdInfo is ignored and 
 *      packet is dropped. Drop action is the highest priority action 
 *      and all other forward actions are ignored. The packet is NOT 
 *      forwarded to switch.
 *      When this bit is 0, the packet is forwarded to switch and
 *      other forward actions define the forwarding behavior.
 *  e_FWD_ACTION_ARL_OVRD: 
 *      When this bit is 1, the packet is forwarded to switch with ARL
 *      override, and the portMask specified in the dstMask of the 
 *      BpaFwdInfo is used. 
 *      When this bit is 0, the packet is forwarded to switch. The 
 *      destination portMap is determined by ANDing the portMask field in
 *      dstMask with the forward map from switch ARL lookup, and the
 *      Port Based VLAN forward map of Rx port.
 *  e_FWD_ACTION_LRN_ENABLE: 
 *      When this bit is 1, the packet is forwarded to switch with MAC SA
 *      learning enabled.
 *      When this bit is 0, the packet is forwarded to switch with MAC SA
 *      learning disabled.
 *------------------------------------------------------------------------------
 */

#define BPA_FLOWPRIO_MAX    32

/** forwarding action for a flow */
typedef enum BpaFwdAction
{
    e_FWD_ACTION_DROP        = 1<<0, /**< Drop the packet                */
    e_FWD_ACTION_ARL_OVRD    = 1<<1, /**< Fwd to switch but override ARL */
    e_FWD_ACTION_LRN_ENABLE  = 1<<2, /**< Fwd to switch and learn MAC SA */
    e_FWD_ACTION_MAX         = 1<<3, /**< Max # of Fwd Actions bits      */
} BpaFwdAction_t;

#if defined(CONFIG_BCM96818)
/** packet coloring of a flow */
typedef enum BpaFwdColor
{
    e_FWD_COLOR_GREEN        = 0,    /**< Green Color                    */
    e_FWD_COLOR_YELLOW       = 1,    /**< Yellow Color                   */
    e_FWD_COLOR_MAX                  /**< Max # of colors                */
} BpaFwdColor_t;
#endif

/** Forwarding configuration info */
typedef struct BpaFwdInfo
{
    uint32_t         flowPrio; 
    BpaSrcPortMask_t srcMask; 
    BpaDstPortMask_t dstMask;
    uint32_t         fwdAction;
#if defined(CONFIG_BCM96818)
    BpaFwdColor_t    fwdColor;
#endif
} BpaFwdInfo_t;

/** Sets the FCB forwarding information.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param fwdInfo_p  (IN)  pointer to forwarding configuration
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_FWDTYPE
 *     e_RET_ERR_INVALID_SRCPORTMASK
 *     e_RET_ERR_INVALID_DSTPORTMASK
 *     e_RET_ERR_INVALID_FLOWPRIO
 *     e_RET_ERR_INVALID_FWDACTION
 *     e_RET_ERR_INVALID_FWDCOLOR
 */
extern BpaApiRet_t BPA_Set_FwdInfo( const BpaFlowHdl_t *flowHdl_p,  
                const BpaFwdInfo_t *fwdInfo_p );

/** Add port(s) to the FCB forwarding information.
 * NOTE: Forwarding Info should be already configured before this API. 
 *       The forwarding info should have already one dest port configured.
 *       Qid info is taken from previously configured ports in Forwarding
 *       information.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param dstMask_p  (IN)  pointer to dest port mask (ports to add)
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_DSTPORTMASK
 */
extern BpaApiRet_t BPA_Add_DstPortInfo( const BpaFlowHdl_t *flowHdl_p,
                BpaDstPortMask_t *dstMask_p );

/** Remove port(s) from the FCB forwarding information.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param dstMask_p  (IN)  pointer to dest port mask (ports to remove)
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_DSTPORTMASK
 */
extern BpaApiRet_t BPA_Rem_DstPortInfo( const BpaFlowHdl_t *flowHdl_p,
                BpaDstPortMask_t *dstMask_p );


/** Gets the configured forwarding information for the flow.
 *     
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param fwdInfo_p  (OUT) pointer to forwarding configuration
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 */
extern BpaApiRet_t BPA_Get_FwdInfo( const BpaFlowHdl_t *flowHdl_p, 
                BpaFwdInfo_t *fwdInfo_p );


/** Antispoof result override control for a flow */
typedef enum BpaAsOverride
{
    e_AS_OVERRIDE_DISABLE,  /**< Disable override: Perform Antispoof action */
    e_AS_OVERRIDE_ENABLE,   /**< Enable override:  Ignore  Antispoof action */
    e_AS_OVERRIDE_MAX       /**< Max # of Antispoof overrides               */
} BpaAsOverride_t;

#define BPA_AS_OVERRIDE_DEFAULT e_AS_OVERRIDE_DISABLE

/** Configures Anti-spoofing override control for the flow.
 *
 * Configures the Anti-spoofing override to be used for the flow.
 * By default, the override control is e_AS_OVERRIDE_DISABLE.
 * This is an OPTIONAL API and may not be invoked for the flow which 
 * donot require Anti-spoofing.
 *
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param asOverride (IN)  Antispoof access control override
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 */
extern BpaApiRet_t BPA_Set_AsOverride( const BpaFlowHdl_t *flowHdl_p, 
                BpaAsOverride_t asOverride );

/** Gets the configured Anti-spoofing override control for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param asOverride_p (OUT) pointer to table linked status
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 */
extern BpaApiRet_t BPA_Get_AsOverride( const BpaFlowHdl_t *flowHdl_p, 
                BpaAsOverride_t *asOverride_p );


/** table link status with a flow */
typedef enum BpaTblStatus
{
    e_TBLSTATUS_DISABLE,        /**< Disable Table link status  */
    e_TBLSTATUS_ENABLE,         /**< Enable Table link status   */
    e_TBLSTATUS_MAX             /**< Max # of Fwd actions       */
} BpaTblStatus_t;



/** Configures PBits to Action table for the flow.
 *
 * Configures the PBits to Action table to be used for the flow. It is assumed that 
 * the PAC table has been already configured before it can be
 * used to set for the flow.
 * This is an Optional API and may not be invoked for the flows which 
 * donot use PBits to Action mapping.
 *
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param pacTblSts  (IN)  table linked status
 * @param pbitsSource(IN)  source of pBits
 * @param pacHdl_p   (IN)  PAC table handle to be used for the flow
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_TBLSTATUS
 */
extern BpaApiRet_t BPA_Set_PacInfo( const BpaFlowHdl_t *flowHdl_p, 
                BpaTblStatus_t pacTblSts, BpaPbitsSource_t pbitsSource, 
                const BpaPacTblHdl_t *pacHdl_p );

/** Gets the configured PAC table handle for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param pacTblSts_p (OUT) pointer to table linked status
 * @param pbitsSource (OUT)  source of pBits
 * @param pacHdl_p  (OUT) pointer to PAC table handle used for the flow
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 */
extern BpaApiRet_t BPA_Get_PacInfo( const BpaFlowHdl_t *flowHdl_p, 
                BpaTblStatus_t * pacTblSts, BpaPbitsSource_t * pbitsSource, 
                BpaPacTblHdl_t *pacHdl_p );


/** Configures EthType-to-Pbits mapping table for the flow.
 *
 * Configures the EthType-to-Pbits mapping table to be used for the flow. It is 
 * assumed that the EthType-to-Pbits mapping table has been already configured 
 * before it can be used to set for the flow.
 * This is an OPTIONAL API and may not be invoked for the flow which 
 * donot require EthType-to-Pbits mapping.
 *
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param epTblSts   (IN)  table linked status
 * @param epTblHdl_p (IN)  pointer to EthType-to-Pbits mapping table 
 *                         handle to be used for
 *                         the flow
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_TBLSTATUS
 */
extern BpaApiRet_t BPA_Set_EpInfo( const BpaFlowHdl_t *flowHdl_p, 
                BpaTblStatus_t epTblSts, 
                const BpaEpTblHdl_t *epTblHdl_p );

/** Gets the configured EthType-to-Pbits mapping table handle for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param epTblSts_p (OUT) pointer to table linked status
 * @param epTblHdl_p (OUT) pointer to EthType-to-Pbits mapping table 
 *                         handle for the flow
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_NULL_PTR
 */
extern BpaApiRet_t BPA_Get_EpInfo( const BpaFlowHdl_t *flowHdl_p, 
                BpaTblStatus_t *epTblSts_p, 
                BpaEpTblHdl_t *epTblHdl_p );

/** Configures PBits to DestQ table for the flow.
 *
 * Configures the PBits to DestQ table to be used for the flow. 
 * It is assumed that the Pbits2DestQ table has been already 
 * configured before it can be used to set for the flow. 
 * This is an Optional API and may not be invoked for the flows 
 * which do not use PBits to DestQ mapping. 
 *  
 * CAUTION: It is assumed that user take appropriate precaution 
 * to make sure that this table is associated with only those 
 * flows that are specific to the switch ingress port this table 
 * is associated with. 
 *
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param pbits2DestQTblSts  (IN)  table link status
 * @param pbits2DestQHdl_p   (IN)  Pbits2DestQ table handle to 
 *                           be used for the flow
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_TBLSTATUS
 */
extern BpaApiRet_t BPA_Set_Pbits2DestQInfo( const BpaFlowHdl_t *flowHdl_p, 
                                            BpaTblStatus_t pbits2DestQTblSts, 
                                            const BpaPbits2DestQTblHdl_t *pbits2DestQHdl_p );

/** Gets the configured Pbits2DestQ table handle for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param pbits2DestQTblSts_p (OUT) pointer to table linked 
 *                            status
 * @param pbits2DestQHdl_p  (OUT) pointer to Pbits2DestQ table 
 *                          handle used for the flow
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 */
extern BpaApiRet_t BPA_Get_Pbits2DestQInfo( const BpaFlowHdl_t *flowHdl_p, 
                                            BpaTblStatus_t * pbits2DestQTblSts, 
                                            BpaPbits2DestQTblHdl_t *pbits2DestQHdl_p );


/*
 *------------------------------------------------------------------------------
 * Filter Block APIs
 *------------------------------------------------------------------------------
 *
 * The Filter APIs can be called more than once. But each filter will result 
 * in the AND operation. E.g. if e_ETH_FILT_DA and e_ETH_FILT_ETHTYPE 
 * are configured, the resulting flow needs to match both the filters.
 * When a filter is already configured and the Filter API is again called for
 * the same Filter, one more filter is added with the new value. The combinened
 * filter for the field is formed by ANDing all the filters for the field.
 *------------------------------------------------------------------------------
 */

/** Compare Operators.
 *
 * Various comparision operators that can be used with filters.
 * For all the comparision operators (most of them except range) which 
 * need one operand, only val1 field of val parameter is used.
 *
 * The e_CMP_IN_RANGE operator checks the value to be in the given range 
 * and the range values are inclusive. The val1 and val2 fields of the 
 * val parameter are used as range start and end values respectively.
 *
 * e_CMP_NEQ operator:
 * The NEQ operation is supported for a few filters like:
 * e_ETH_FILT_ETHTYPE, e_VLAN_FILT_TPID, e_VLAN_FILT_VID, e_PPPOE_FILT_PROTO,
 * e_IPV4_FILT_PROTO, e_IPV6_FILT_NXT_HDR, e_TCP_FILT_SPORT, e_TCP_FILT_DPORT 
 * CAUTION!!!: Although an NEQ filter may be used multiple times, but only 
 * a limited number (may be total of 4) of NEQ filters are supported per port.
 */
typedef enum BpaCmpOper
{
    e_CMP_EQ,                   /**< checks for equal           */
    e_CMP_NEQ,                  /**< checks for not equal       */
    e_CMP_IN_RANGE,             /**< checks for value in range  */
#if 0
    e_CMP_LT,                   /**< checks for less than       */
    e_CMP_GT,                   /**< checks for greater than    */
    e_CMP_LE,                   /**< checks for less or equal   */
    e_CMP_GE,                   /**< checks for greater of equal*/
#endif
    e_CMP_MAX                   /**< Max # of operations        */
} BpaCmpOper_t;


/** Ethernet Header Filters 
 *
 * The Ethernet filters allow a common set of filter rules (independent of
 * IPv4 or IPv6) for filtering based on DA, SA, MCAST, BCAST fields. For 
 * these common Eth filters donot specify the EthType as 0x0800 (IPv4) or
 * 0x86DD (IPv6).
 *
 * CAUTION: 
 * 1. The filter e_ETH_FILT_ETHTYPE refers to Type field at offset 12 in 
 * untagged and at offset 16 in single tagged, and at offset 20 in double 
 * tagged Ethernet frames.
 * 2. Following Ethernet filters cannot be used with IPV6 filters for the
 * same flow: e_ETH_FILT_MCAST/BCAST/DA/SA
 */
typedef enum BpaEthFilterType
{
    e_ETH_FILT_MCAST,           /**< Filter Ethernet Multicast frames  */
    e_ETH_FILT_BCAST,           /**< Filter Ethernet Broadcast frames  */
    e_ETH_FILT_DA,              /**< Filter on MAC Destination Address */
    e_ETH_FILT_SA,              /**< Filter on MAC Source Address      */
    e_ETH_FILT_ETHTYPE,         /**< Filter on Ethernet Type           */
    e_ETH_FILT_MAX              /**< Max # of Ethernet header filters  */
} BpaEthFilterType_t;

/** Ethernet Values */
typedef union BpaEthValue
{
    uint8_t      mcast;
    uint8_t      bcast;
    uint16_t     ethType;
    BpaMacAddrMask_t sa;
    BpaMacAddrMask_t da;
} BpaEthValue_t;

/** Ethernet Filter Value */
typedef struct BpaEthFilterValue
{
    BpaEthValue_t  val1;        /**< Used as value for range from       */
    BpaEthValue_t  val2;        /**< Used as value for range to         */
} BpaEthFilterValue_t;

/** Ethernet Filter
 * Config API 
 *     op and type are used as index 
 * Get API 
 *     op is ignored and type is used as index 
 */
typedef struct BpaEthFilter
{
    BpaConfigOper_t     op;     /**< configuration operation */   
    BpaEthFilterType_t  type;   /**< field to filter on      */
    BpaCmpOper_t        cmpOp;  /**< compare operation       */
    BpaEthFilterValue_t val;    /**< value of filter         */ 
} BpaEthFilter_t;


/** Configures an Ethernet filter for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param filter_p   (IN)  pointer to Ethernet Filter
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_CONFIGOPER
 *     e_RET_ERR_INVALID_CMPOPER
 *     e_RET_ERR_INVALID_FILT
 *     e_RET_ERR_INVALID_VAL
 */
extern BpaApiRet_t BPA_Config_EthFilter( const BpaFlowHdl_t *flowHdl_p, 
                const BpaEthFilter_t *filter_p );

/** Gets the configuration of an Ethernet filter.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param filter_p   (IN/OUT)  Ethernet filter array (allocated by caller)
 *        type       (IN)  field to filter on
 *        cmpOp      (OUT) compare operation 
 *        val        (OUT) value of filter
 * @param size       (IN)  size of filter array
 * @param elemCount_p (OUT) pointer to number of elements returned in 
 *                          filter array (filter_p).
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_FILT
 *     e_RET_ERR_NO_SUCH_ENTRY
 *     e_RET_ERR_OVERFLOW
 */
extern BpaApiRet_t BPA_Get_EthFilter( const BpaFlowHdl_t *flowHdl_p, 
                BpaEthFilter_t *filter_p, 
                uint8_t size, 
                uint8_t *elemCount_p );

/** VLAN Filter Types */
typedef enum BpaVlanFilterType
{
    e_VLAN_FILT_TAG,
    e_VLAN_FILT_TPID,
    e_VLAN_FILT_TCI,
    e_VLAN_FILT_PBITS,
    e_VLAN_FILT_DEI,
    e_VLAN_FILT_VID,
    e_VLAN_FILT_MAX
} BpaVlanFilterType_t;

/** VLAN Filter values */
typedef union BpaVlanValue
{
    uint8_t         pbits;
    uint8_t         dei;
    uint16_t        vid;
    uint16_t        tci;
    uint16_t        tpid;
    uint32_t        tag;
    BpaVlanTagNum_t fromTagNum;
} BpaVlanValue_t;

typedef struct BpaVlanFilterValue
{
    BpaVlanValue_t val1;
    BpaVlanValue_t val2;
} BpaVlanFilterValue_t;

typedef struct BpaVlanFilter
{
    BpaConfigOper_t      op;      /**< configuration operation */  
    BpaVlanTagNum_t      toTagNum;/**< Tag number to filter    */
    BpaVlanFilterType_t  type;    /**< field to filter on      */ 
    BpaCmpOper_t         cmpOp;   /**< compare operation       */  
    BpaVlanFilterValue_t val;     /**< value of filter         */ 
} BpaVlanFilter_t;

/** Configures a VLAN filter for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param filter_p   (IN)  pointer to VLAN Filter
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_CONFIGOPER
 *     e_RET_ERR_INVALID_CMPOPER
 *     e_RET_ERR_INVALID_FILT
 *     e_RET_ERR_INVALID_VAL
 *     e_RET_ERR_INVALID_TAGNUM
 */
extern BpaApiRet_t BPA_Config_VlanFilter( const BpaFlowHdl_t *flowHdl_p, 
                const BpaVlanFilter_t *filter_p );

/** Gets the configuration of a VLAN filter.
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param filter_p   (IN/OUT)  VLAN filter array
 *        toTagNum   (IN)  vlan tag TO which the filter applies
 *        type       (IN)  field to filter on
 *        cmpOp      (OUT) compare operation 
 *        val        (OUT) value of filter
 * @param size       (IN)  size of filter array
 * @param elemCount_p (OUT) pointer to number of elements returned in 
 *                          filter array (filter_p).
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_FILT
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_TAGNUM
 *     e_RET_ERR_OVERFLOW
 */
extern BpaApiRet_t BPA_Get_VlanFilter( const BpaFlowHdl_t *flowHdl_p, 
                BpaVlanFilter_t *filter_p, 
                uint8_t size, 
                uint8_t *elemCount_p );


/** PPPoE Filter types */
typedef enum BpaPppoeFilterType
{
    e_PPPOE_FILT_VER,
    e_PPPOE_FILT_TYPE,
    e_PPPOE_FILT_CODE,
    e_PPPOE_FILT_SESID,
    e_PPPOE_FILT_PROTO,
    e_PPPOE_FILT_MAX
} BpaPppoeFilterType_t;

/** PPPoE Filter values */
typedef union BpaPppoeValue
{
    uint8_t  ver;
    uint8_t  type;
    uint8_t  code;
    uint16_t sesid;
    uint16_t proto;
} BpaPppoeValue_t;

typedef struct BpaPppoeFilterValue
{
    BpaPppoeValue_t val1;
    BpaPppoeValue_t val2;
} BpaPppoeFilterValue_t;

typedef struct BpaPppoeFilter
{
    BpaConfigOper_t       op;   /**< configuration operation     */ 
    BpaPppoeFilterType_t  type; /**< field to filter on          */   
    BpaCmpOper_t          cmpOp;/**< compare operation           */    
    BpaPppoeFilterValue_t val;  /**< value of filter             */ 
} BpaPppoeFilter_t;

/** Configures a PPPoE filter for the flow.
 *
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param filter_p   (IN)  pointer to PPPoE Filter
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_CONFIGOPER
 *     e_RET_ERR_INVALID_FILT
 *     e_RET_ERR_INVALID_VAL
 */
extern BpaApiRet_t BPA_Config_PppoeFilter( const BpaFlowHdl_t *flowHdl_p, 
                const BpaPppoeFilter_t *filter_p );


/** Gets the configuration of a PPPoE filter.
 *
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param filter_p   (IN/OUT)  PPPoE filter array
 *        type       (IN)  field to filter on
 *        cmpOp      (OUT) compare operation 
 *        val        (OUT) value of filter
 * @param size       (IN)  size of filter array
 * @param elemCount_p (OUT) pointer to number of elements returned in 
 *                          filter array (filter_p).
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_FILT
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_OVERFLOW
 */
extern BpaApiRet_t BPA_Get_PppoeFilter( const BpaFlowHdl_t *flowHdl_p, 
                BpaPppoeFilter_t *filter_p,
                uint8_t size, 
                uint8_t *elemCount_p );


/** MPLS filter types */
typedef enum BpaMplsFilterType
{
    e_MPLS_FILT_TTL,
    e_MPLS_FILT_SBIT,
    e_MPLS_FILT_LABEL,
    e_MPLS_FILT_MAX
} BpaMplsFilterType_t;

/** MPLS filter values */
typedef union BpaMplsValue
{
    uint8_t  ttl;
    uint8_t  sbit;
    uint32_t label;
} BpaMplsValue_t;

typedef struct BpaMplsFilterValue 
{
    BpaMplsValue_t val1;
    BpaMplsValue_t val2;
} BpaMplsFilterValue_t;

typedef struct BpaMplsFilter
{
     BpaConfigOper_t      op;   /**< configuration operation */ 
     BpaMplsFilterType_t  type; /**< field to filter on      */    
     BpaCmpOper_t         cmpOp;/**< compare operation       */     
     BpaMplsFilterValue_t val;  /**< value of filter         */ 
} BpaMplsFilter_t;

/** Configures an MPLS filter for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param filter_p   (IN)  pointer to MPLS Filter
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_CONFIGOPER
 *     e_RET_ERR_INVALID_FILT
 *     e_RET_ERR_INVALID_VAL
 */
extern BpaApiRet_t BPA_Config_MplsFilter( const BpaFlowHdl_t *flowHdl_p, 
                const BpaMplsFilter_t *filter_p );

/** Gets the configuration of an MPLS filter.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param filter_p   (IN/OUT)  MPLS filter array
 *        type       (IN)  field to filter on
 *        cmpOp      (OUT) compare operation 
 *        val        (OUT) value of filter
 * @param size       (IN)  size of filter array
 * @param elemCount_p (OUT) pointer to number of elements returned in 
 *                          filter array (filter_p).
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_FILT
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_OVERFLOW
 */
extern BpaApiRet_t BPA_Get_MplsFilter( const BpaFlowHdl_t *flowHdl_p, 
                BpaMplsFilter_t *filter_p,
                uint8_t size, 
                uint8_t *elemCount_p );


/** IPv4 filter types */
typedef enum BpaIpv4FilterType
{
    e_IPV4_FILT_VER,
    e_IPV4_FILT_IHL,
    e_IPV4_FILT_SA,
    e_IPV4_FILT_DA,
    e_IPV4_FILT_MCAST,
    e_IPV4_FILT_TOS,
    e_IPV4_FILT_DSCP,
    e_IPV4_FILT_DSCP_1P,        /**< Filter on Pbits computed from DSCP2PBits table */
    e_IPV4_FILT_TTL_GT1,
    e_IPV4_FILT_PROTO,
    e_IPV4_FILT_FRAG,
    e_IPV4_FILT_MAX
} BpaIpv4FilterType_t;

/** IPv4 filter values */
typedef union BpaIpv4Value
{
    uint8_t     ver;
    uint8_t     ihl;
    uint8_t     tos;
    uint8_t     dscp;
    uint8_t     dscp_1p;
    BpaCidrIpAddr_t sa;
    BpaCidrIpAddr_t da;
    uint8_t     mcast;
    uint8_t     ttlGt1;
    uint16_t    proto;
    uint16_t    frag;
} BpaIpv4Value_t;

typedef struct BpaIpv4FilterValue_t
{
    BpaIpv4Value_t val1;
    BpaIpv4Value_t val2;
} BpaIpv4FilterValue_t;

typedef struct BpaIpv4Filter
{
    BpaConfigOper_t      op;    /**< configuration operation */ 
    BpaIpv4FilterType_t  type;  /**< field to filter on      */   
    BpaCmpOper_t         cmpOp; /**< compare operation       */    
    BpaIpv4FilterValue_t val;   /**< value of filter         */ 
} BpaIpv4Filter_t;

/** Configures an IPv4 filter for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param filter_p   (IN)  pointer to IPv4 Filter
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_CONFIGOPER
 *     e_RET_ERR_INVALID_FILT
 *     e_RET_ERR_INVALID_VAL
 */
extern BpaApiRet_t BPA_Config_Ipv4Filter( const BpaFlowHdl_t *flowHdl_p, 
                const BpaIpv4Filter_t *filter_p );

/** Gets the configuration of an IPv4 filter.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param filter_p   (IN/OUT)  IPv4 filter array
 *        type       (IN)  field to filter on
 *        cmpOp      (OUT) compare operation 
 *        val        (OUT) value of filter
 * @param size       (IN)  size of filter array
 * @param elemCount_p (OUT) pointer to number of elements returned in 
 *                          filter array (filter_p).
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_FILT
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_OVERFLOW
 */
extern BpaApiRet_t BPA_Get_Ipv4Filter( const BpaFlowHdl_t *flowHdl_p, 
                BpaIpv4Filter_t *filter_p,
                uint8_t size, 
                uint8_t *elemCount_p );


/** IPv6 Header Filters 
 *
 * The IPv6 filters allow a flow to be filtered on one or more fields of
 * IPv6 fixed header.
 *
 * e_IPV6_FILT_NXT_HDR filters based on the value of next header field in 
 * fixed header. 
 * e_IPV6_FILT_EXT_HDR filters the packets based on one or more
 * extension headers.
 *
 * CAUTION: 
 * 1. The packet matching Eth filter e_ETH_FILT_ETHTYPE with value of 0x86DD
 * is treated as IPV6 packet.
 * 2. Following Ethernet filters cannot be used with IPV6 filters for the
 * same flow: e_ETH_FILT_MCAST/BCAST/DA/SA
 * 3. When e_IPV6_FILT_EXT_HDR filter is used, then filtering cannot be 
 * based on TCP or UDP headers because we donot know L4 protocol.
 */

/** IPv6 filter types */
typedef enum BpaIpv6FilterType
{
    e_IPV6_FILT_SA,
    e_IPV6_FILT_DA,
    e_IPV6_FILT_MCAST,
    e_IPV6_FILT_TCLASS,
    e_IPV6_FILT_DSCP,
    e_IPV6_FILT_DSCP_1P,  /**< Filter on Pbits computed from DSCP2PBits table */
    e_IPV6_FILT_NXT_HDR,
    e_IPV6_FILT_EXT_HDR, /**< Filter based on IPv6 Extension Header present */
    e_IPV6_FILT_HOPLIMIT_GT1,
    e_IPV6_FILT_MAX
} BpaIpv6FilterType_t;

/** IPv6 filter values */
typedef union BpaIpv6Value
{
    BpaCidrIpv6Addr_t sa;
    BpaCidrIpv6Addr_t da;
    uint8_t     mcast;
    uint8_t     tclass;
    uint8_t     dscp;
    uint8_t     dscp_1p;
    uint16_t    nxtHdr;
    uint8_t     hopLimitGt1;
    uint8_t     extHdr;
} BpaIpv6Value_t;

typedef struct BpaIpv6FilterValue_t
{
    BpaIpv6Value_t val1;
    BpaIpv6Value_t val2;
} BpaIpv6FilterValue_t;

typedef struct BpaIpv6Filter
{
    BpaConfigOper_t      op;    /**< configuration operation */ 
    BpaIpv6FilterType_t  type;  /**< field to filter on      */   
    BpaCmpOper_t         cmpOp; /**< compare operation       */    
    BpaIpv6FilterValue_t val;   /**< value of filter         */ 
} BpaIpv6Filter_t;

/** Configures an IPv6 filter for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param filter_p   (IN)  pointer to IPv6 Filter
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_CONFIGOPER
 *     e_RET_ERR_INVALID_FILT
 *     e_RET_ERR_INVALID_VAL
 */
extern BpaApiRet_t BPA_Config_Ipv6Filter( const BpaFlowHdl_t *flowHdl_p, 
                const BpaIpv6Filter_t *filter_p );

/** Gets the configuration of an IPv6 filter.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param filter_p   (IN/OUT)  IPv6 filter array
 *        type       (IN)  field to filter on
 *        cmpOp      (OUT) compare operation 
 *        val        (OUT) value of filter
 * @param size       (IN)  size of filter array
 * @param elemCount_p (OUT) pointer to number of elements returned in 
 *                          filter array (filter_p).
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_FILT
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_OVERFLOW
 */
extern BpaApiRet_t BPA_Get_Ipv6Filter( const BpaFlowHdl_t *flowHdl_p, 
                BpaIpv6Filter_t *filter_p,
                uint8_t size, 
                uint8_t *elemCount_p );


/** TCP filter types */
typedef enum BpaTcpFilterType
{
    e_TCP_FILT_SPORT,
    e_TCP_FILT_DPORT,
    e_TCP_FILT_SYN,
    e_TCP_FILT_ACK,
    e_TCP_FILT_FIN,
    e_TCP_FILT_RST,
    e_TCP_FILT_CTRL,
    e_TCP_FILT_MAX
} BpaTcpFilterType_t;

/** TCP filter values */
typedef union BpaTcpValue
{
    uint16_t sport;
    uint16_t dport;
    uint8_t  syn;
    uint8_t  ack;
    uint8_t  fin;
    uint8_t  rst;
    uint8_t  ctrl;
} BpaTcpValue_t;

typedef struct BpaTcpFilterValue
{
    BpaTcpValue_t val1;
    BpaTcpValue_t val2;
} BpaTcpFilterValue_t;

typedef struct BpaTcpFilter
{
    BpaConfigOper_t     op;     /**< configuration operation */ 
    BpaTcpFilterType_t  type;   /**< field to filter on      */   
    BpaCmpOper_t        cmpOp;  /**< compare operation       */    
    BpaTcpFilterValue_t val;    /**< value of filter         */ 
} BpaTcpFilter_t;

/** Configures a TCP filter for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param filter_p   (IN)  pointer to TCP Filter
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_CONFIGOPER
 *     e_RET_ERR_INVALID_FILT
 *     e_RET_ERR_INVALID_VAL
 */
extern BpaApiRet_t BPA_Config_TcpFilter( const BpaFlowHdl_t *flowHdl_p, 
                const BpaTcpFilter_t *filter_p );

/** Gets the configuration of an TCP filter.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param filter_p   (IN/OUT)  TCP filter array
 *        type       (IN)  field to filter on
 *        cmpOp      (OUT) compare operation 
 *        val        (OUT) value of filter
 * @param size       (IN)  size of filter array
 * @param elemCount_p (OUT) pointer to number of elements returned in 
 *                          filter array (filter_p).
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_FILT
 *     e_RET_ERR_OVERFLOW
 */
extern BpaApiRet_t BPA_Get_TcpFilter( const BpaFlowHdl_t *flowHdl_p, 
                BpaTcpFilter_t *filter_p,
                uint8_t size, 
                uint8_t *elemCount_p );


/** UDP filter types */
typedef enum BpaUdpFilterType
{
    e_UDP_FILT_SPORT,
    e_UDP_FILT_DPORT,
    e_UDP_FILT_MAX
} BpaUdpFilterType_t;

/** UDP filter values */
typedef union BpaUdpValue
{
    uint16_t sport;
    uint16_t dport;
} BpaUdpValue_t;

typedef struct BpaUdpFilterValue
{
    BpaUdpValue_t val1;
    BpaUdpValue_t val2;
} BpaUdpFilterValue_t;

typedef struct BpaUdpFilter
{
    BpaConfigOper_t     op;     /**< configuration operation */ 
    BpaUdpFilterType_t  type;   /**< field to filter on      */ 
    BpaCmpOper_t        cmpOp;  /**< compare operation       */ 
    BpaUdpFilterValue_t val;    /**< value of filter         */ 
} BpaUdpFilter_t;

/** Configures a UDP filter for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param filter_p   (IN)  pointer to TCP Filter
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_CONFIGOPER
 *     e_RET_ERR_INVALID_FILT
 *     e_RET_ERR_INVALID_VAL
 */
extern BpaApiRet_t BPA_Config_UdpFilter( const BpaFlowHdl_t *flowHdl_p, 
                const BpaUdpFilter_t *filter_p );

/** Gets the configuration of a UDP filter.
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param filter_p   (IN/OUT)  UDP filter array
 *        type       (IN)  field to filter on
 *        cmpOp      (OUT) compare operation 
 *        val        (OUT) value of filter
 * @param size       (IN)  size of filter array
 * @param elemCount_p (OUT) pointer to number of elements returned in 
 *                          filter array (filter_p).
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_FILT
 *     e_RET_ERR_OVERFLOW
 */
extern BpaApiRet_t BPA_Get_UdpFilter( const BpaFlowHdl_t *flowHdl_p, 
                BpaUdpFilter_t *filter_p,
                uint8_t size, 
                uint8_t *elemCount_p );


/*
 *------------------------------------------------------------------------------
 * Modification Block APIs (Action)
 *------------------------------------------------------------------------------
 *
 * The Action APIs can be called more than once. The result will be the 
 * combined action of all the actions configured. E.g. if e_ETH_ACTION_DA and 
 * e_ETH_ACTION_ETHTYPE are configured, the resulting flow will have the DA and
 * type modified as per actions.
 *
 * When an action is already configured and the action API is again called for
 * the same action, the action is updated with the new value. This means in
 * this case the configuration operation is treated as modify operation.
 *
 *  e_ETH_ACTION_PUSH_TAG & e_ETH_ACTION_POP_TAG commands do not take any
 *  value parameter. These actions always insert or delete the outermost
 *  vlan tag. It is NOT allowed to insert/remove a tag in between the tags.
 *------------------------------------------------------------------------------
 */

/** Ethernet Modify Actions */
typedef enum BpaEthActionType
{
    e_ETH_ACTION_DA,
    e_ETH_ACTION_SA,
    e_ETH_ACTION_ETHTYPE,
    e_ETH_ACTION_PUSH_TAG,      /**< Push/insert an outermost VLAN tag */
    e_ETH_ACTION_POP_TAG,       /**< Pop/remove an outermost VLAN tag  */
    e_ETH_ACTION_MAX
} BpaEthActionType_t;

typedef struct BpaEthAction
{
    BpaConfigOper_t    op; 
    BpaEthActionType_t type; 
    BpaEthValue_t      val;
} BpaEthAction_t;

/** Configures an Ethernet modify Action for the flow.
 *
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param action_p   (IN)  pointer to Ethernet action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_CONFIGOPER
 *     e_RET_ERR_INVALID_ACTION
 *     e_RET_ERR_INVALID_VAL
 *     e_RET_ERR_INVALID_DSTPORT
 */
extern BpaApiRet_t BPA_Config_EthAction( const BpaFlowHdl_t *flowHdl_p, 
                const BpaEthAction_t *action_p );

/** Gets the configured value for an Ethernet modify Action.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param action_p   (IN/OUT)  pointer to Ethernet action
 *        type       (IN)  action type
 *        val        (OUT) value of action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_ACTION
 *     e_RET_ERR_INVALID_DSTPORT
 */
extern BpaApiRet_t BPA_Get_EthAction( const BpaFlowHdl_t *flowHdl_p, 
                BpaEthAction_t *action_p );

/*
 *------------------------------------------------------------------------------
 * CAUTION: - 
 *     Modification of vlag tags are done from outermost to innermost.
 *
 * VLAN_ACTION_COPY_XX_FROM_TAG: the val_p parameter points to the copy FROM 
 *     vlan tag num, and vlanTagNum parameter defines the copy TO vlan tag num.
 *
 * VLAN_ACTION_XLATE_PBITS_FROM_DSCP: the val_p parameter points to
 *     toTagNum, and the DSCP field value from the packet is used to map to 
 *     the Pbits using the DscpToPbits. The toTagNum field specifies the 
 *     VLAN tag num whose pbit field shall be modified.
 *
 * VLAN_ACTION_XLATE_PBITS_FROM_ETHTYPE: the val_p parameter is ignored, and
 *     the ethType field value from the packet is used to map to 
 *     the Pbits using the EP Map table for the incoming port.
 *
 * Pass Thru: No Vlan Action is required.
 *------------------------------------------------------------------------------
 */
/** VLAN Modify Actions */
typedef enum BpaVlanActionType
{
    e_VLAN_ACTION_SET_TAG,                /**< Set TAG                   */
    e_VLAN_ACTION_SET_TPID,               /**< Set TPID                  */
    e_VLAN_ACTION_SET_TCI,                /**< Set TCI                   */
    e_VLAN_ACTION_SET_PBITS,              /**< Set PBITS                 */
    e_VLAN_ACTION_SET_DEI,                /**< Set DEI                   */
    e_VLAN_ACTION_SET_VID,                /**< Set VID                   */
    e_VLAN_ACTION_COPY_TAG_FROM_TAG,      /**< Copy TAG from a TAG       */
    e_VLAN_ACTION_COPY_TPID_FROM_TAG,     /**< Copy TPID from a TAG      */
    e_VLAN_ACTION_COPY_TCI_FROM_TAG,      /**< Copy TCI from a TAG       */
    e_VLAN_ACTION_COPY_PBITS_FROM_TAG,    /**< Copy PBits from a TAG     */
    e_VLAN_ACTION_COPY_VID_FROM_TAG,      /**< Copy VID from a TAG       */
    e_VLAN_ACTION_COPY_DEI_FROM_TAG,      /**< Copy DEI from a TAG       */
    e_VLAN_ACTION_XLATE_PBITS_FROM_DSCP,  /**< Translate PBits from DSCP */
    e_VLAN_ACTION_XLATE_PBITS_FROM_ETHTYPE,  /**< Translate PBits from 
                                                  ETHTYPE */
    e_VLAN_ACTION_MAX                     /**< Max # of VLAN actions     */
} BpaVlanActionType_t;

typedef struct BpaVlanAction
{
    BpaConfigOper_t     op; 
    BpaVlanActionType_t type; 
    BpaVlanValue_t      val;
    BpaVlanTagNum_t     toTagNum; 
} BpaVlanAction_t;

/** Configures VLAN modify Action for the flow.
 *
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param action_p   (IN)  pointer to VLAN action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_CONFIGOPER
 *     e_RET_ERR_INVALID_ACTION
 *     e_RET_ERR_INVALID_VAL
 *     e_RET_ERR_INVALID_DSTPORT
 *     e_RET_ERR_INVALID_TAGNUM
 */
extern BpaApiRet_t BPA_Config_VlanAction( const BpaFlowHdl_t *flowHdl_p, 
                const BpaVlanAction_t *action_p ); 

/** Gets the configured value for a VLAN tag modify Action.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param action_p   (IN/OUT)  pointer to Ethernet action
 *        type       (IN)  action type
 *        val        (OUT) value of action
 *        tagNum     (IN)  Vlan tag TO which the Action applies
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_ACTION
 *     e_RET_ERR_INVALID_DSTPORT
 *     e_RET_ERR_INVALID_TAGNUM
 */
extern BpaApiRet_t BPA_Get_VlanAction( const BpaFlowHdl_t *flowHdl_p, 
                BpaVlanAction_t *action_p ); 

/** PPPoE Modify Actions */
typedef enum BpaPppoeActionType
{
    e_PPPOE_ACTION_REMOVE_HDR,
    e_PPPOE_ACTION_MAX
} BpaPppoeActionType_t;

typedef struct BpaPppoeAction
{
    BpaConfigOper_t      op; 
    BpaPppoeActionType_t type; 
    BpaPppoeValue_t      val;
} BpaPppoeAction_t;

/** Configures a PPPoE modify Action for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param action_p   (IN)  pointer to PPPoE action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_CONFIGOPER
 *     e_RET_ERR_INVALID_ACTION
 *     e_RET_ERR_INVALID_VAL
 *     e_RET_ERR_INVALID_DSTPORT
 */
extern BpaApiRet_t BPA_Config_PppoeAction( const BpaFlowHdl_t *flowHdl_p, 
                const BpaPppoeAction_t *action_p ); 

/** Gets the configured value for an PPPoE modify Action.
 *
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param action_p   (IN/OUT)  pointer to PPPoE action
 *        type       (IN)  action type
 *        val        (OUT) value of action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_ACTION
 *     e_RET_ERR_INVALID_DSTPORT
 */
extern BpaApiRet_t BPA_Get_PppoeAction( const BpaFlowHdl_t *flowHdl_p, 
                BpaPppoeAction_t *action_p ); 


/** MPLS Modify Actions */
typedef enum BpaMplsActionType
{
    e_MPLS_ACTION_TTL,
    e_MPLS_ACTION_SBIT,
    e_MPLS_ACTION_LABEL,
    e_MPLS_ACTION_MAX
} BpaMplsActionType_t;

typedef struct BpaMplsAction
{
    BpaConfigOper_t      op; 
    BpaMplsActionType_t  type; 
    BpaMplsValue_t       val;
} BpaMplsAction_t;

/** Configures an MPLS modify Action for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param action_p   (IN)  pointer to MPLS action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_CONFIGOPER
 *     e_RET_ERR_INVALID_ACTION
 *     e_RET_ERR_INVALID_VAL
 *     e_RET_ERR_INVALID_DSTPORT
 */
extern BpaApiRet_t BPA_Config_MplsAction( const BpaFlowHdl_t *flowHdl_p, 
                const BpaMplsAction_t *action_p );

/** Gets the configured value for an MPLS modify Action.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param action_p   (IN/OUT)  pointer to MPLS action
 *        type       (IN)  action type
 *        val        (OUT) value of action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_ACTION
 *     e_RET_ERR_INVALID_DSTPORT
 */
extern BpaApiRet_t BPA_Get_MplsAction( const BpaFlowHdl_t *flowHdl_p, 
                BpaMplsAction_t *action_p );

/** IPv4 Modify Actions */
typedef enum BpaIpv4ActionType
{
    e_IPV4_ACTION_SA,           /**< may be for NATing */
    e_IPV4_ACTION_DA,           /**< may be for NATing */
    e_IPV4_ACTION_TOS,
    e_IPV4_ACTION_DSCP,
    e_IPV4_ACTION_TTL,
    e_IPV4_ACTION_MAX
} BpaIpv4ActionType_t;

typedef struct BpaIpv4Action
{
    BpaConfigOper_t      op; 
    BpaIpv4ActionType_t  type; 
    BpaIpv4Value_t       val;
} BpaIpv4Action_t;

/** Configures an IPv4 modify Action for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param action_p   (IN)  pointer to IPv4 action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_CONFIGOPER
 *     e_RET_ERR_INVALID_ACTION
 *     e_RET_ERR_INVALID_VAL
 *     e_RET_ERR_INVALID_DSTPORT
 */
extern BpaApiRet_t BPA_Config_Ipv4Action( const BpaFlowHdl_t *flowHdl_p, 
                const BpaIpv4Action_t *action_p );

/** Gets the configured value for an IPv4 modify Action.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param action_p   (IN/OUT)  pointer to IPv4 action
 *        type       (IN)  action type
 *        val        (OUT) value of action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_ACTION
 *     e_RET_ERR_INVALID_DSTPORT
 */
extern BpaApiRet_t BPA_Get_Ipv4Action( const BpaFlowHdl_t *flowHdl_p, 
                BpaIpv4Action_t *action_p );

/** IPv6 Modify Actions */
typedef enum BpaIpv6ActionType
{
    e_IPV6_ACTION_SA,           /**< may be for NATing */
    e_IPV6_ACTION_DA,           /**< may be for NATing */
    e_IPV6_ACTION_TCLASS,
    e_IPV6_ACTION_DSCP,
    e_IPV6_ACTION_HOPLIMIT,
    e_IPV6_ACTION_MAX
} BpaIpv6ActionType_t;

typedef struct BpaIpv6Action
{
    BpaConfigOper_t      op; 
    BpaIpv6ActionType_t  type; 
    BpaIpv6Value_t       val;
} BpaIpv6Action_t;

/** Configures an IPv6 modify Action for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param action_p   (IN)  pointer to IPv6 action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_CONFIGOPER
 *     e_RET_ERR_INVALID_ACTION
 *     e_RET_ERR_INVALID_VAL
 */
extern BpaApiRet_t BPA_Config_Ipv6Action( const BpaFlowHdl_t *flowHdl_p, 
                const BpaIpv6Action_t *action_p );

/** Gets the configured value for an IPv6 modify Action.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param action_p   (IN/OUT)  pointer to IPv6 action
 *        type       (IN)  action type
 *        val        (OUT) value of action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_ACTION
 */
extern BpaApiRet_t BPA_Get_Ipv6Action( const BpaFlowHdl_t *flowHdl_p, 
                BpaIpv6Action_t *action_p );


/** TCP Modify Actions */
typedef enum BpaTcpActionType
{
    e_TCP_ACTION_SPORT,
    e_TCP_ACTION_DPORT,
    e_TCP_ACTION_MAX
} BpaTcpActionType_t;

typedef struct BpaTcpAction
{
    BpaConfigOper_t      op; 
    BpaTcpActionType_t   type; 
    BpaTcpValue_t        val;
} BpaTcpAction_t;

/** Configures an TCP modify Action for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param action_p   (IN)  pointer to TCP action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_CONFIGOPER
 *     e_RET_ERR_INVALID_ACTION
 *     e_RET_ERR_INVALID_VAL
 *     e_RET_ERR_INVALID_DSTPORT
 */
extern BpaApiRet_t BPA_Config_TcpAction( const BpaFlowHdl_t *flowHdl_p, 
                const BpaTcpAction_t *action_p ); 

/** Gets the configured value for a TCP modify Action.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param action_p   (IN/OUT)  pointer to TCP action
 *        type       (IN)  action type
 *        val        (OUT) value of action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_ACTION
 *     e_RET_ERR_INVALID_DSTPORT
 */
extern BpaApiRet_t BPA_Get_TcpAction( const BpaFlowHdl_t *flowHdl_p, 
                BpaTcpAction_t *action_p );

/** UDP Modify Actions */
typedef enum BpaUdpActionType
{
    e_UDP_ACTION_SPORT,
    e_UDP_ACTION_DPORT,
    e_UDP_ACTION_MAX
} BpaUdpActionType_t;

typedef struct BpaUdpAction
{
    BpaConfigOper_t      op; 
    BpaUdpActionType_t   type; 
    BpaUdpValue_t        val;
} BpaUdpAction_t;

/** Configures a UDP modify Action for the flow.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param action_p   (IN)  pointer to UDP action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_CONFIGOPER
 *     e_RET_ERR_INVALID_ACTION
 *     e_RET_ERR_INVALID_VAL
 *     e_RET_ERR_INVALID_DSTPORT
 */
extern BpaApiRet_t BPA_Config_UdpAction( const BpaFlowHdl_t *flowHdl_p, 
                const BpaUdpAction_t *action_p ); 

/** Gets the configured value for UDP modify Action.
 * 
 * @param flowHdl_p  (IN)  pointer to flow handle
 * @param action_p   (IN/OUT)  pointer to UDP action
 *        type       (IN)  action type
 *        val        (OUT) value of action
 * @return BpaApiRet_t     enum
 *     e_RET_SUCCESS
 *     e_RET_ERR_FAILURE
 *     e_RET_ERR_NULL_PTR
 *     e_RET_ERR_INVALID_HDL
 *     e_RET_ERR_INVALID_ACTION
 *     e_RET_ERR_INVALID_DSTPORT
 */
extern BpaApiRet_t BPA_Get_UdpAction( const BpaFlowHdl_t *flowHdl_p, 
                BpaUdpAction_t *action_p ); 

#endif /* __BPAAPI_H__ */

