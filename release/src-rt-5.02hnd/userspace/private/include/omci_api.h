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

#ifndef _GPON_OMCI_API_H_
#define _GPON_OMCI_API_H_

#include "os_defs.h"


/*
 * Macros
 */
#if defined(OMCI_API_CMS)

#include "cms.h"
#include "cms_log.h"

#define ERROR(...)  cmsLog_error(__VA_ARGS__)
#define NOTICE(...) cmsLog_notice(__VA_ARGS__)
#define DEBUG(...)  cmsLog_debug(__VA_ARGS__)

#else

#define ERROR(fmt, arg...) \
    printf("\nomci_api:ERROR <%s,%d>: " fmt "\n", __FUNCTION__, __LINE__, ##arg)

#if defined(OMCI_API_NOTICE)
#define NOTICE(fmt, arg...) \
    printf("omci_api: " fmt "\n", ##arg)
#else
#define NOTICE(fmt, arg...)
#endif

#if defined(OMCI_API_DEBUG)
#define DEBUG(fmt, arg...) \
    printf("omci_api:DEBUG <%s,%d>: " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#else
#define DEBUG(fmt, arg...)
#endif

#endif /* OMCI_API_CMS */

#ifdef GPON_LEGACY_API
#define BCMGPON_FILE "/dev/bcmgpon"
#else
#define BCMGPON_FILE "/dev/bcm_omci"
#endif   // GPON_LEGACY_API

#define OMCI_ENTRY_SIZE_52 52
#define OMCI_ENTRY_SIZE_48 48
#define OMCI_ENTRY_SIZE_36 36
#define OMCI_ENTRY_SIZE_32 32
#define OMCI_ENTRY_SIZE_24 24
#define OMCI_ENTRY_SIZE_20 20
#define OMCI_ENTRY_SIZE_16 16
#define OMCI_ENTRY_SIZE_8 8

#define OMCI_PACKET_A_HDR_SIZE 8 /* bytes */
#define OMCI_PACKET_A_MSG_SIZE 32 /* bytes */
#define OMCI_PACKET_A_TRAILER_SIZE 8 /* bytes */
#define OMCI_PACKET_CPCS_SDU_LEN 0x0028 /* OMCI_PACKET_A_HDR_SIZE + OMCI_PACKET_A_MSG_SIZE */
#define OMCI_PACKET_A_SIZE (OMCI_PACKET_A_HDR_SIZE + OMCI_PACKET_A_MSG_SIZE + \
    OMCI_PACKET_A_TRAILER_SIZE)
#define OMCI_PACKET_MIC_SIZE 4 /* bytes */
#define OMCI_PACKET_B_HDR_SIZE 10 /* bytes */
#define OMCI_PACKET_B_MSG_SIZE_MAX 1966 /* bytes */
#define OMCI_PACKET_B_SIZE_MAX (OMCI_PACKET_B_HDR_SIZE + OMCI_PACKET_B_MSG_SIZE_MAX + \
    OMCI_PACKET_MIC_SIZE)

#define OMCI_PACKET_PRIORITY(p) (UINT8)(((p)->tcId[0] & 0x80) >> 8)
#define OMCI_PACKET_TC_ID(p)    (UINT16)((((p)->tcId[0] & 0x7F) << 8) | ((p)->tcId[1]))
#define OMCI_PACKET_TC_ID_INIT  0x10000
#define OMCI_PACKET_DB(p)       ((p)->msgType & 0x80)
#define OMCI_PACKET_AR(p)       ((p)->msgType & 0x40)
#define OMCI_PACKET_ACK_REQ(p)  (((p)->msgType & 0x40)>>6)
#define OMCI_PACKET_AK(p)       ((p)->msgType & 0x20)
#define OMCI_PACKET_MT(p)       ((p)->msgType & 0x1F)
#define OMCI_PACKET_MT_MAX      32
#define OMCI_PACKET_DEV_ID_A 0x0A
#define OMCI_PACKET_DEV_ID_B 0x0B
#define OMCI_CHECK_DEV_ID_A(p) ((p)->devId == OMCI_PACKET_DEV_ID_A)
#define OMCI_CHECK_DEV_ID_B(p) ((p)->devId == OMCI_PACKET_DEV_ID_B)
#define OMCI_PACKET_ME_CLASS(p) (UINT16)(((p)->classNo[0] << 8) | ((p)->classNo[1]))
#define OMCI_PACKET_ME_INST(p)  (UINT16)(((p)->instId[0] << 8) | ((p)->instId[1]))
#define OMCI_PACKET_SW_IMG_INST(p)  (OMCI_PACKET_ME_INST(p) & 0xFF)

#define OMCI_ME_CLASS_MAX       348 /* Maximum ME class ID can be supported
                                       should be updated when more ME are added in omcid_me.h and omci_util.h */

#define OMCI_MSG_TYPE_AR(c)       ((c) | 0x40)
#define OMCI_MSG_TYPE_AK(c)       ((c) | 0x20)

#if __BYTE_ORDER == __BIG_ENDIAN
#define OMCI_NTOHS(addr)       (*(UINT16*)addr)
#define OMCI_HTONS(addr, val)  (*(UINT16*)(addr) = (val))
#define OMCI_NTOHL(addr)       (*(UINT32*)addr)
#define OMCI_HTONL(addr, val)  (*(UINT32*)(addr) = (val))

#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define OMCI_NTOHS(addr)\
  ((((*(UINT8*)(addr)) << 8) & 0xFF00) | (*((UINT8*)(addr) + 1)))
#define OMCI_HTONS(addr, val)\
{\
    *(UINT8*)(addr) = (UINT8)(((val) >> 8) & 0xFF);\
    *((UINT8*)(addr) + 1) = (UINT8)((val) & 0xFF);\
}
#define OMCI_NTOHL(addr)\
  (((((UINT32)*((UINT8*)addr)) << 24) & 0xFF000000)\
  | (((UINT32)*((UINT8*)(addr) + 1) << 16) & 0xFF0000)\
  | (((UINT32)*((UINT8*)(addr) + 2) << 8) & 0xFF00)\
  | ((UINT32)*((UINT8*)(addr) + 3)))
#define OMCI_HTONL(addr, val)\
{\
    *(UINT8*)(addr) = (UINT8)(((val) >> 24) & 0xFF);\
    *((UINT8*)(addr) + 1) = (UINT8)(((val) >> 16) & 0xFF);\
    *((UINT8*)(addr) + 2) = (UINT8)(((val) >> 8) & 0xFF);\
    *((UINT8*)(addr) + 3) = (UINT8)((val) & 0xFF);\
}
#else
#error cannot detect endianess
#endif

/* #define OMCI_SW_IMAGE_1_VERSION "01011a0c00020000000000000000" */
//#define OMCI_SW_IMAGE_0_VERSION "534F465457415245494D41474530" /* SOFTWAREIMAGE0 */
//#define OMCI_SW_IMAGE_1_VERSION "534F465457415245494D41474531" /* SOFTWAREIMAGE1 */
#define OMCI_SW_IMAGE_0_VERSION "534F465457415245494D41474530" /*  SOFTWAREIMAGE0*/
#define OMCI_SW_IMAGE_1_VERSION "534F465457415245494D41474531" /*  SOFTWAREIMAGE1*/
#define OMCI_SW_IMAGE_0_VERDFLT "SOFTWAREIMAGE0" /* SOFTWAREIMAGE0 */
#define OMCI_SW_IMAGE_1_VERDFLT "SOFTWAREIMAGE1" /* SOFTWAREIMAGE1 */
#define OMCI_SOFTWARE_DOWNLOAD_SECTION_SIZE (31)

/*
 * enumerations
 */
typedef enum {
    OMCI_ACCESS_TYPE_BLOCKING,
    OMCI_ACCESS_TYPE_NONBLOCKING,
    OMCI_ACCESS_TYPE_MAX,
} omciAccessType;

typedef enum {
    OMCI_MSG_TYPE_CREATE = 4,
    OMCI_MSG_TYPE_CREATECOMPLETECONNECTION,
    OMCI_MSG_TYPE_DELETE,
    OMCI_MSG_TYPE_DELETECOMPLETECONNECTION,
    OMCI_MSG_TYPE_SET,
    OMCI_MSG_TYPE_GET,
    OMCI_MSG_TYPE_GETCOMPLETECONNECTION,
    OMCI_MSG_TYPE_GETALLALARMS,
    OMCI_MSG_TYPE_GETALLALARMSNEXT,
    OMCI_MSG_TYPE_MIBUPLOAD,
    OMCI_MSG_TYPE_MIBUPLOADNEXT,
    OMCI_MSG_TYPE_MIBRESET,
    OMCI_MSG_TYPE_ALARM,
    OMCI_MSG_TYPE_ATTRIBUTEVALUECHANGE,
    OMCI_MSG_TYPE_TEST,
    OMCI_MSG_TYPE_STARTSOFTWAREDOWNLOAD,
    OMCI_MSG_TYPE_DOWNLOADSECTION,
    OMCI_MSG_TYPE_ENDSOFTWAREDOWNLOAD,
    OMCI_MSG_TYPE_ACTIVATESOFTWARE,
    OMCI_MSG_TYPE_COMMITSOFTWARE,
    OMCI_MSG_TYPE_SYNCHRONIZETIME,
    OMCI_MSG_TYPE_REBOOT,
    OMCI_MSG_TYPE_GETNEXT,
    OMCI_MSG_TYPE_TESTRESULT,
    OMCI_MSG_TYPE_GETCURRENTDATA
} omciMsgType;

typedef enum {
    OMCI_MSG_RESULT_SUCCESS=0,
    OMCI_MSG_RESULT_PROC_ERROR,
    OMCI_MSG_RESULT_NOT_SUPPORTED,
    OMCI_MSG_RESULT_PARM_ERROR,
    OMCI_MSG_RESULT_UNKNOWN_ME,
    OMCI_MSG_RESULT_UNKNOWN_ME_INST,
    OMCI_MSG_RESULT_DEV_BUSY,
    OMCI_MSG_RESULT_ME_INST_EXISTS,
    OMCI_MSG_RESULT_ATTR_FAILED=9,
    OMCI_MSG_RESULT_MAX
} omciMsgResult;

typedef enum {
    OMCI_PACKET_PRIORITY_LOW=0,
    OMCI_PACKET_PRIORITY_HIGH,
    OMCI_PACKET_PRIORITY_MAX
} omciPacketPriority;


typedef struct  _StartSoftwareDownloadMsg
{
	UINT8	WindowSize;
	UINT32	ImageSize;
	UINT8	CircuitPackUpdateCount;
	UINT8	SoftwareImageInstanceMsb;
	UINT8	SoftwareImageInstanceLsb;	
	UINT8	Padding[24];

} __attribute__ ((packed)) StartSoftwareDownloadMsg_t;


typedef union
{
	StartSoftwareDownloadMsg_t 	StartSoftwareDownload;	
} OmciReqMsgBody_t;

/*
 * structures
 */
struct omciPacket_struct {
    UINT8 tcId[2];
    UINT8 msgType;
    UINT8 devId;
    UINT8 classNo[2];
    UINT8 instId[2];
    union {
        struct {
            UINT8 msg[OMCI_PACKET_A_MSG_SIZE];
            union {
                UINT8 trailer[OMCI_PACKET_A_TRAILER_SIZE];
                struct {
                    UINT8 len[4];
                    UINT8 crc[4];
                } t;
            };
        } A;
        struct {
            UINT16 reserved:5;
            UINT16 msgLen:11;
            UINT8 msg[OMCI_PACKET_B_MSG_SIZE_MAX + OMCI_PACKET_MIC_SIZE];
        } B;
    };
    CmsEntityId src_eid; /* Not part of OMCI message format, used internally for CMS messages */
} __attribute__ ((packed));

#define OMCI_PACKET_SIZE(m) ((m)->devId == OMCI_PACKET_DEV_ID_A ? OMCI_PACKET_A_SIZE : \
                                  (OMCI_PACKET_B_HDR_SIZE + (m)->B.msgLen + \
                                  OMCI_PACKET_MIC_SIZE))
#define OMCI_PACKET_MSG(m) ((m)->devId == OMCI_PACKET_DEV_ID_A ? (m)->A.msg : (m)->B.msg)
#define OMCI_PACKET_MSG_LEN(m) ((m)->devId == OMCI_PACKET_DEV_ID_A ? OMCI_PACKET_A_MSG_SIZE : (m)->B.msgLen)
/* Return pointer to CRC for type A, or MIC location, right after B. msgLen bytes, should be set last before sending message */
#define OMCI_PACKET_CRC(m) ((m)->devId == OMCI_PACKET_DEV_ID_A ? &((m)->A.trailer[4]) : &((m)->B.msg[(m)->B.msgLen]))

typedef struct omciPacket_struct omciPacket;


/*
 * function prototypes
 */
int gpon_omci_api_init(omciAccessType accessType);

void gpon_omci_api_exit(void);

void gpon_omci_api_get_handle(int *pHandle);

int gpon_omci_api_transmit(omciPacket *pPacket, int size);

int gpon_omci_api_receive(omciPacket *pPacket, int size);

#endif /* _GPON_OMCI_API_H_ */
