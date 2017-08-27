/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

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
 *
 ************************************************************************/


#ifndef _OMCI_UTIL_H_
#define _OMCI_UTIL_H_

#include "cms.h"
#include "cms_mdm.h"
#include "mdm_objectid.h"
#include "omci_api.h"

#define OMCID_DEBUG_ON  1
#define OMCID_DEBUG_OFF 0

typedef enum
{
    OMCI_ERR_NONE = 0,
    OMCI_ERR_SWDL_SECTION_HOLE,
    OMCI_ERR_SWDL_SECTION_RSP,
    OMCI_ERR_SWDL_IMAGE_CRC
} OmciDbgErr_t;

typedef enum
{
    OMCI_ETH_PORT_TYPE_NONE = 0,
    OMCI_ETH_PORT_TYPE_RG_ONT,
    OMCI_ETH_PORT_TYPE_ONT,
    OMCI_ETH_PORT_TYPE_RG
} OmciEthPortType;

typedef struct {
    union {
        UINT32 all;
        struct {
            UINT32 eth0:2;
            UINT32 eth1:2;
            UINT32 eth2:2;
            UINT32 eth3:2;
            UINT32 eth4:2;
            UINT32 eth5:2;
            UINT32 eth6:2;
            UINT32 eth7:2;
            UINT32 unused:16;
        } ports;
    } types;
} OmciEthPortType_t;

typedef struct {
    union {
        UINT32 all;
        struct {
            UINT32 omci:1;
            UINT32 model:1;
            UINT32 vlan:1;
            UINT32 cmf:1;
            UINT32 flow:1;
            UINT32 rule:1;
            UINT32 mcast:1;
            UINT32 voice:1;
            UINT32 file:1;
            UINT32 unused:23;
        } bits;
    } flags;
} omciDebug_t;

typedef struct
{
   UINT16 mdmOid;
   UINT16 classId;
} OmciMdmOidClassId_t;

static const OmciMdmOidClassId_t omciMdmOidClassIdTable[] =
{
    {MDMOID_ONT_DATA, 2},
    {MDMOID_CARD_HOLDER, 5},
    {MDMOID_CIRCUIT_PACK, 6},
    {MDMOID_SOFTWARE_IMAGE, 7},
    {MDMOID_PPTP_ETHERNET_UNI, 11},
    {MDMOID_ETHERNET_PM_HISTORY_DATA, 24},
    {MDMOID_MAC_BRIDGE_SERVICE_PROFILE, 45},
    {MDMOID_MAC_BRIDGE_CONFIG_DATA, 46},
    {MDMOID_MAC_BRIDGE_PORT_CONFIG_DATA, 47},
    {MDMOID_MAC_BRIDGE_PORT_DESIGNATION_DATA, 48},
    {MDMOID_MAC_BRIDGE_PORT_FILTER_TABLE_DATA, 49},
    {MDMOID_MAC_BRIDGE_PORT_BRIDGE_TABLE_DATA, 50},
    {MDMOID_MAC_BRIDGE_PM_HISTORY_DATA, 51},
    {MDMOID_MAC_BRIDGE_PORT_PM_HISTORY_DATA, 52},
    {MDMOID_PPTP_POTS_UNI, 53},
    {MDMOID_VOICE_SERVICE, 58},
    {MDMOID_VLAN_TAGGING_OPERATION_CONFIGURATION_DATA, 78},
    {MDMOID_MAC_BRIDGE_PORT_FILTER_PRE_ASSIGN_TABLE, 79},
    {MDMOID_VLAN_TAGGING_FILTER_DATA, 84},
    {MDMOID_ETHERNET_PM_HISTORY_DATA2, 89},
    {MDMOID_MAPPER_SERVICE_PROFILE, 130},
    {MDMOID_OLT_G, 131},
    {MDMOID_POWER_SHEDDING, 133},
    {MDMOID_IP_HOST_CONFIG_DATA, 134},
    {MDMOID_IP_HOST_PM_HISTORY_DATA, 135},
    {MDMOID_TCP_UDP_CONFIG_DATA, 136},
    {MDMOID_NETWORK_ADDRESS, 137},
    {MDMOID_VO_IP_CONFIG_DATA, 138},
    {MDMOID_VO_IP_VOICE_CTP, 139},
    {MDMOID_CALL_CONTROL_PM_HISTORY_DATA, 140},
    {MDMOID_VO_IP_LINE_STATUS, 141},
    {MDMOID_VO_IP_MEDIA_PROFILE, 142},
    {MDMOID_RTP_PROFILE_DATA, 143},
    {MDMOID_RTP_PM_HISTORY_DATA, 144},
    {MDMOID_NETWORK_DIAL_PLAN_TABLE, 145},
    {MDMOID_VO_IP_APP_SERVICE_PROFILE, 146},
    {MDMOID_VOICE_FEATURE_ACCESS_CODES, 147},
    {MDMOID_AUTHENTICATION_SECURITY_METHOD, 148},
    {MDMOID_SIP_AGENT_CONFIG_DATA, 150},
    {MDMOID_SIP_AGENT_PM_HISTORY_DATA, 151},
    {MDMOID_SIP_CALL_INIT_PM_HISTORY_DATA, 152},
    {MDMOID_SIP_USER_DATA, 153},
    {MDMOID_MGC_CONFIG_DATA, 155},
    {MDMOID_MGC_PM_HISTORY_DATA, 156},
    {MDMOID_LARGE_STRING, 157},
    {MDMOID_PORT_MAPPING_PACKAGE_G, 297},
    {MDMOID_PPTP_MOCA_UNI, 162},
    {MDMOID_MOCA_ETHERNET_PM_HISTORY_DATA, 163},
    {MDMOID_MOCA_INTERFACE_PM_HISTORY_DATA, 164},
    {MDMOID_EXTENDED_VLAN_TAGGING_OPERATION_CONFIGURATION_DATA, 171},
    {MDMOID_BRCM_PPTP_MOCA_UNI, 240},
    {MDMOID_MOCA_STATUS, 241},
    {MDMOID_MOCA_STATS, 242},
    {MDMOID_ONT_G, 256},
    {MDMOID_ONT2_G, 257},
    {MDMOID_T_CONT, 262},
    {MDMOID_ANI_G, 263},
    {MDMOID_UNI_G, 264},
    {MDMOID_GEM_INTERWORKING_TP, 266},
    {MDMOID_GEM_PORT_NETWORK_CTP, 268},
    {MDMOID_GAL_ETHERNET_PROFILE, 272},
    {MDMOID_THRESHOLD_DATA1, 273},
    {MDMOID_THRESHOLD_DATA2, 274},
    {MDMOID_GAL_ETHERNET_PM_HISTORY_DATA, 276},
    {MDMOID_PRIORITY_QUEUE_G, 277},
    {MDMOID_TRAFFIC_SCHEDULER_G, 278},
    {MDMOID_GEM_TRAFFIC_DESCRIPTOR, 280},
    {MDMOID_MULTICAST_GEM_INTERWORKING_TP, 281},
    {MDMOID_OMCI, 287},
    {MDMOID_ETHERNET_PM_HISTORY_DATA3, 296},
    {MDMOID_MULTICAST_OPERATIONS_PROFILE, 309},
    {MDMOID_MULTICAST_SUBSCRIBER_CONFIG_INFO, 310},
    {MDMOID_MULTICAST_SUBSCRIBER_MONITOR, 311},
    {MDMOID_FEC_PM_HISTORY_DATA, 312},
    {MDMOID_DOWNSTREAM_ETHERNET_FRAME_PM_HISTORY_DATA, 321},
    {MDMOID_UPSTREAM_ETHERNET_FRAME_PM_HISTORY_DATA, 322},
    {MDMOID_VIRTUAL_ETHERNET_INTERFACE_POINT, 329},
    {MDMOID_ETHERNET_FRAME_EXTENDED_P_M, 334},
    {MDMOID_TR069_MANAGEMENT_SERVER, 340},
    {MDMOID_GEM_PORT_PM_HISTORY_DATA, 341},
    {MDMOID_IPV6_HOST_CONFIG_DATA, 347}
};

#define NUM_OMCI_CLASS_ID (sizeof(omciMdmOidClassIdTable) / sizeof(OmciMdmOidClassId_t))

#define OMCI_CLASS_ID_TO_MDM_OID(obj)                                 \
    do {                                                              \
        UINT32 i = 0;                                                 \
        for (i = 0; i < NUM_OMCI_CLASS_ID; i++)                       \
        {                                                             \
            if (omciMdmOidClassIdTable[i].classId == (obj)->classId)  \
                break;                                                \
        }                                                             \
        if (i < NUM_OMCI_CLASS_ID)                                    \
            (obj)->mdmOid = omciMdmOidClassIdTable[i].mdmOid;         \
        else                                                          \
            (obj)->mdmOid = 0;                                        \
    } while (0)

#define OMCI_MDM_OID_TO_CLASS_ID(obj)                                 \
    do {                                                              \
        UINT32 i = 0;                                                 \
        for (i = 0; i < NUM_OMCI_CLASS_ID; i++)                       \
        {                                                             \
            if (omciMdmOidClassIdTable[i].mdmOid == (obj)->mdmOid)    \
                break;                                                \
        }                                                             \
        if (i < NUM_OMCI_CLASS_ID)                                    \
            (obj)->classId = omciMdmOidClassIdTable[i].classId;       \
        else                                                          \
            (obj)->classId = 0;                                       \
    } while (0)

static const UINT32 GroupObjectsTable[] = {
   MDMOID_EQUIPMENT_MANAGEMENT,
   MDMOID_ANI_MANAGEMENT,
   MDMOID_LAYER2_DATA_SERVICES,
   MDMOID_LAYER3_DATA_SERVICES,
   MDMOID_ETHERNET_SERVICES,
#ifdef DMP_X_ITU_ORG_VOICE_1
   MDMOID_VOICE_SERVICES,
#endif /* DMP_X_ITU_ORG_VOICE_1 */
   MDMOID_MOCA_SERVICES,
   MDMOID_TRAFFIC_MANAGEMENT,
   MDMOID_GENERAL
   };

#define NUM_GROUP_OBJECTS (sizeof(GroupObjectsTable) / sizeof(UINT32))

#define OMCI_CRC32_POLYNOMIAL 0x04c11db7L /* Standard CRC-32 ppolynomial */

#define OMCI_DEFAULT_TCONT_MEID 0

void omciUtl_initCrc32Table(void);
UINT32 omciUtl_getCrc32(UINT32 crcAccum, char *pBuf, UINT32 size);
void omciUtl_getCrc32Staged(UINT32 stage, UINT32 *crcAccumP, char *pBuf,
  UINT32 size);
UINT32 omciUtl_getParamSize(MdmParamTypes type, UINT32 maxVal);
void omciUtl_dumpMem(unsigned char *pBuf, int len);
void omciUtl_dumpMemWidth(unsigned char *pBuf, int len, int width);
void omciUtl_dumpMemWidthToFile(FILE *fs,
   unsigned char *pBuf, int len, int width);
void omciUtl_dumpPacket(omciPacket *pPacket, int len);
void omciUtl_dumpPacketToFile(FILE *fs, omciPacket *pPacket, int len);
CmsRet omciUtl_sendAttributeValueChange(UINT32 oid, UINT16 meId, UINT16 attrMask,
   UINT8 *attrValue, UINT16 attrSize);
CmsRet omciUtl_HexToAscii(char *string, const char *binString, int len);
CmsRet omciUtl_AsciiToHex(char *binString, const char *string, int len);
int omciUtil_ioctlWrap(int fd, int req, void *argp);

OmciEthPortType omciUtil_getPortType(UINT8 port, UINT32 typesAll);

#endif /* _OMCI_UTIL_H_ */

