/***********************************************************************
 *
 * Copyright (c) 2017 Broadcom
 * All Rights Reserved
 *
 * <:label-BRCM:2017:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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

#define OMCI_DEBUG_MSG_DATA_LEN_MAX 512
#define OMCI_INVALID_VLAN 0xFFFF

typedef enum
{
    OMCI_DUMP_MIB_ALL   = 0,
    OMCI_DUMP_DM        = 1,
    /* Between OMCI_DUMP_DM and OMCI_DUMP_MIB_RESET: dump OMCI ME. */
    OMCI_DUMP_MIB_RESET = 0x0ffff,
    OMCI_DUMP_OMCID     = 0x10000,
    OMCI_DUMP_PM_MIN    = 0x10001,
    OMCI_DUMP_PM_ALL    = OMCI_DUMP_PM_MIN,
    OMCI_DUMP_PM_THR,
    OMCI_DUMP_PM_ALARM,
    OMCI_DUMP_PM_ADMIN,
    OMCI_DUMP_PM_COUNTER,
    OMCI_DUMP_PM_MAX    = 0x10020,
    OMCI_DUMP_PAL_MIN   = 0x10021,
    OMCI_DUMP_PAL_TC    = OMCI_DUMP_PAL_MIN,
    OMCI_DUMP_PAL_MAX   = 0x10030
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
    OMCI_DEBUG_DATA_DDI_OMCID,
    OMCI_DEBUG_DATA_MAX
} OmciDebugDataType;

/** Data body for CMS_MSG_OMCI_IGMP_ADMISSION_CONTROL message type.
 *
 */
typedef struct
{
    UINT16  lanTci;
    UINT16  wanTci;
    UINT32  sourceIpAddress;
    UINT32  groupIpAddress;
    UINT32  clientIpAddress;
    UINT16  phyPort;
    OmciIgmpPhyType phyType;
    OmciIgmpMsgType msgType;
    UINT32  igmpVersion;
} OmciIgmpMsgBody;

typedef struct {
   SINT8 nbrOfTags;          /**< Number of VLAN tags */
   SINT32 pbits;             /**< VLAN p-bits associated with the WAN service */
   SINT32 vlanId;            /**< VLAN ID associated with the WAN service */
   UBOOL8 serviceStatus;     /**< If TRUE, the WAN service is created */
   UBOOL8 igmpEnabled;       /**< Is IGMP Snooping/Proxy enabled on this WAN service. */
} GponWanServiceParams;

typedef enum
{
    OMCI_SERVICE_UNICAST = 0,
    OMCI_SERVICE_MULTICAST,
    OMCI_SERVICE_BROADCAST
} OmciGponWanServiceType;

/*
 * Both gemPortIndex and portID will become deprecated. A GEM port does not
 * represent a link or an interface. In BBF TR-156, a GEM port represents
 * a traffic class or a queue.
 */
typedef struct {
   UINT32 gemPortIndex;               /**< GEM port index */
   UINT16 portID;                     /**< The TC layer GEM port ID that maps to the GEM port index. e.g. GEM port 3000 maps to index 0 */
   OmciFLowDirection flowDirection;   /**< Gem Port flow direction */
   OmciGponWanServiceType serviceType;/**< Type of GemPort - Bi-directional, Multicast or Broadcast  */
} GponWanLinkParams;

typedef struct {
    char   l2Ifname[CMS_IFNAME_LENGTH];
    GponWanServiceParams serviceParams;
    GponWanLinkParams    linkParams;
} OmciServiceMsgBody;

/** Data body for CMS_MSG_PING_STATE_CHANGED message type for OMCI.
 *
 */
typedef struct
{
   UINT16 tcid;                /**< transaction id provided by omcid */
   UINT16 result;              /**< ping result */
   union
   {
     UINT16 responseTime[15];  /**< response time */
     UINT8 icmpReply[30];      /**< pkt of unexpected ICMP response */
   } msg;
} OmciPingDataMsgBody;

/** Data body for CMS_MSG_TRACERT_STATE_CHANGED message type for OMCI.
 *  TODO: update the neighbour definition to support IPv6.
 *  Note the IPv4 neighbour data from busybox is in network order.
 *
 */
typedef struct
{
   UINT16 tcid;                /**< transaction id provided by omcid */
   UINT16 result;              /**< traceroute result */
   UINT16 hops;                /**< neighbour count */
   union
   {
     UINT32 neighbour[7];      /**< neighbour list */
     UINT8 icmpReply[30];      /**< pkt of unexpected ICMP response */
   } msg;
} OmciTracertDataMsgBody;

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
