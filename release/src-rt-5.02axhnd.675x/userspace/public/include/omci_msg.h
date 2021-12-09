/***********************************************************************
 *
 * Copyright (c) 2017 Broadcom
 * All Rights Reserved
 *
 * <:label-BRCM:2017:DUAL/GPL:standard
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
 * :>
 *
 ************************************************************************/

/*****************************************************************************
*    Description:
*
*      OMCI Inter Process Communication related definitions.
*      The OMCI processs exchanges the following messages.
*        CMS_MSG_OMCI_IGMP_ADMISSION_CONTROL
*        CMS_MSG_OMCI_GPON_WAN_SERVICE_STATUS_CHANGE
*        CMS_MSG_OMCI_RG_WAN_SERVICE_STAUTS_CHANGE
*        CMS_MSG_PING_DATA
*        CMS_MSG_TRACERT_DATA
*
*****************************************************************************/

#ifndef _OMCI_MSG_DEFS_H
#define _OMCI_MSG_DEFS_H

/* ---- Include Files ----------------------------------------------------- */

#include "os_defs.h"


/* ---- Constants and Types ----------------------------------------------- */

#define OMCI_DEBUG_MSG_DATA_LEN_MAX 128

typedef enum
{
    OMCI_DUMP_MIB_ALL   = 0,
    OMCI_DUMP_DM        = 1,
    /* Between OMCI_DUMP_DM and OMCI_DUMP_MIB_RESET: dump OMCI ME. */
    OMCI_DUMP_MIB_RESET = 0x0ffff,
    OMCI_DUMP_OMCID     = 0x10000,
    OMCI_DUMP_PM_MIN    = 0x10001,
    OMCI_DUMP_PM_THR    = OMCI_DUMP_PM_MIN,
    OMCI_DUMP_PM_ALARM,
    OMCI_DUMP_PM_ADMIN,
    OMCI_DUMP_PM_MAX    = 0x10020
} OmciDumpInfoCmd;

typedef enum
{
    OMCI_FLOW_NONE = 0,
    OMCI_FLOW_UPSTREAM,
    OMCI_FLOW_DOWNSTREAM,
    OMCI_FLOW_BOTH,
} OmciFLowDirection;

typedef enum
{
    OMCI_IGMP_PHY_NONE = 0,
    OMCI_IGMP_PHY_ETHERNET,
    OMCI_IGMP_PHY_MOCA,
    OMCI_IGMP_PHY_WIRELESS,
    OMCI_IGMP_PHY_POTS,
    OMCI_IGMP_PHY_GPON
} OmciIgmpPhyType;

typedef enum
{
    OMCI_IGMP_MSG_JOIN = 0,
    OMCI_IGMP_MSG_RE_JOIN,
    OMCI_IGMP_MSG_LEAVE
} OmciIgmpMsgType;

typedef enum
{
    OMCI_DEBUG_DATA_NONE = 0,
    OMCI_DEBUG_DATA_CMAC_MSG,
    OMCI_DEBUG_DATA_GET_MSK,
    OMCI_DEBUG_DATA_CLR_MSK,
    OMCI_DEBUG_DATA_DDI,
    OMCI_DEBUG_DATA_MAX
} OmciDebugDataType;

/** Data body for CMS_MSG_OMCI_IGMP_ADMISSION_CONTROL message type.
 *
 */
typedef struct
{
    UINT16  tci;
    UINT32  sourceIpAddress;
    UINT32  groupIpAddress;
    UINT32  clientIpAddress;
    UINT16  phyPort;
    OmciIgmpPhyType phyType;
    OmciIgmpMsgType msgType;
    UINT32  igmpVersion;
} OmciIgmpMsgBody;

typedef struct {
   SINT32 pbits;             /**< L2 gpon ifname vlan p-bits */
   SINT32 vlanId;            /**< L2 gpon ifname vlan ID */
   UBOOL8 noMcastVlanFilter; /**< noMcastVlanFilter */
   UBOOL8 serviceStatus;     /**< If TRUE, gpon link or RG-WAN-service is created already */
   UBOOL8 igmpEnabled;       /**< Is IGMP Snooping/Proxy enabled on this WAN service. */
} GponWanServiceParams;

typedef enum
{
    OMCI_SERVICE_UNICAST = 0,
    OMCI_SERVICE_MULTICAST,
    OMCI_SERVICE_BROADCAST
} OmciGponWanServiceType;

typedef struct {
   UINT32 gemPortIndex;               /**< GEM port index - RG-Full only. RG-Light uses the gemIdxArrayStruct in gponInterfaceArray  */
   UINT16 portID;                     /**< It is a logical port ID which maps o the gem port index. eg. 3000 maps to gem port 0 */
   OmciFLowDirection flowDirection;   /**< Gem Port flow direction */
   OmciGponWanServiceType serviceType;/**< Type of GemPort - Bi-directional, Multicast or Broadcast  */
} GponWanLinkParams;

typedef struct {
    char   l2Ifname[CMS_IFNAME_LENGTH];
    GponWanServiceParams serviceParams;
    GponWanLinkParams    linkParams;
} OmciServiceMsgBody;

/** Data body for CMS_MSG_PING_STATE_CHANGED message type of OMCI.
 *
 */
typedef struct
{
   UINT16 tcid;                /**< transaction id of omci ME*/
   UINT16 result;              /**< ping result */
   union
   {
     UINT16 responseTime[15];  /**< response time */
     UINT8 icmpReply[30];      /**< pkt of unexpected ICMP response */
   } msg;
} OmciPingDataMsgBody;

/** Data body for CMS_MSG_TRACERT_STATE_CHANGED message type of OMCI.
 *
 */
typedef struct
{
   UINT16 tcid;                /**< transaction id of omci ME*/
   UINT16 result;              /**< traceroute result */
   UINT16 hops;                /**< neighbour count */
   union
   {
     UINT32 neighbour[7];      /**< neighbour list */
     UINT8 icmpReply[30];      /**< pkt of unexpected ICMP response */
   } msg;
} OmciTracertDataMsgBody;

/** Data body for the CMS_MSG_GET_NTH_GPON_WAN_LINK_IF_NAME
 *
 */
typedef struct
{
   UINT32 linkEntryIdx;       /**< Nth WAN link entry - send in request */
   char   L2Ifname[CMS_IFNAME_LENGTH];/**< L2 gpon ifname - send in response */
   GponWanServiceParams serviceParams;
} GponNthWanLinkInfo;

/** Data body for the CMS_MSG_OMCI_DEBUG_OMCI_MSG_GEN
 *
 */
typedef struct
{
   UINT16 msgType;
   UINT16 meClass;
   UINT16 meInst;
   UINT16 attrMask;
   UINT16 alarmNum;
   UBOOL8 flagB;
} OmciMsgGenCmd;

/** Data body for the OMCI_IPC_SET_DEBUG_DATA_REQ
 *
 */
typedef struct
{
   UINT16 msgType; /* OmciDebugDataType */
   UINT16 dataLen;
   UINT8  data[OMCI_DEBUG_MSG_DATA_LEN_MAX];
} OmciDebugData;


/* ---- Variable Externs -------------------------------------------------- */


/* ---- Function Prototypes ----------------------------------------------- */


#endif /* _OMCI_MSG_DEFS_H */
