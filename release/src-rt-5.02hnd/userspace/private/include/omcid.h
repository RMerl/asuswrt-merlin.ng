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


#ifndef _GPON_OMCI_H_
#define _GPON_OMCI_H_

#include "mdm_object.h"
#include "omci_api.h"
#include "omci_util.h"
#include "../../libs/cms_core/linux/rut_gpon.h"

/*
 * macros
 */
#define OMCI_FLAG_PROMISC         0x0001
#define OMCI_FLAG_SAFE_CAPTURE	  0x0002

#define FIVE_BYTE_MASK 0x000000FFFFFFFFFFLLU

#define OMCI_GET_INSTANCE_BY_NBR  0x80000000

#define OMCI_ATTR_MASK_MAX        16

#define OMCI_CREATE_OVERHEAD      0 /* attributes in the Create message start in byte 0
                                       (II.2.1/G.984.4) */

#define OMCI_SET_OVERHEAD         2 /* attributes in the Set message start in byte 2
                                       (II.2.9/G.984.4) */

#define OMCI_GET_OVERHEAD         3 /* attributes in the response message of a Get command
                                       start in byte 3 (II.2.11/G.984.4) */

#define OMCI_MIB_UPLOAD_OVERHEAD  6 /* attributes in the response message start
                                       in byte 6 (II.2.22/G.984.4) */
#define OMCI_MIB_UPLOAD_INST_MAX  256 /* maximum number of ME instances to upload */
#define OMCI_MIB_UPLOAD_FRAG_MAX  16

#define DECLARE_PGPONOMCI() gponOmci_t *pGponOmci = &gponOmci

#define OMCID_PROMISC_MODE() (gponOmci.flags & OMCI_FLAG_PROMISC)
//#define OMCID_PROMISC_MODE() (FALSE)
#define OMCID_SAFE_CAPTURE_MODE() (gponOmci.flags & OMCI_FLAG_SAFE_CAPTURE)

#define OMCID_DUMP_PACKET(...)                                       \
    do {                                                             \
        if(gponOmci.logLevel == 1 ||                                 \
           rutGpon_getOmciDebugModule(OMCI_DEBUG_MODULE_OMCI) == 1)  \
        {                                                            \
            FILE *fs = rutGpon_getOmciDebugFile();                   \
            if (fs != NULL)                                          \
                omciUtl_dumpPacketToFile(fs, __VA_ARGS__);           \
            else                                                     \
                omciUtl_dumpPacket(__VA_ARGS__);                     \
        }                                                            \
    } while (0)

#define OMCID_DUMP_OBJECT(_mdmObj)                                   \
    do {                                                             \
        UINT32 _headerMask = cmsLog_getHeaderMask();                 \
        if(gponOmci.logLevel == 1 ||                                 \
           rutGpon_getOmciDebugModule(OMCI_DEBUG_MODULE_OMCI) == 1)  \
        {                                                            \
            cmsLog_setHeaderMask(0);                                 \
            cmsObj_dumpObject((_mdmObj));                            \
            cmsLog_setHeaderMask(_headerMask);                       \
        }                                                            \
    } while (0)

#define OMCID_STATS_INC(_counter)               \
    do {                                        \
        gponOmci.stats._counter++;              \
    } while (0)

#define SHORT_NOTICE(...)                               \
    do {                                                \
        UINT32 _headerMask = cmsLog_getHeaderMask();    \
        cmsLog_setHeaderMask(0);                        \
        cmsLog_notice(__VA_ARGS__);                     \
        cmsLog_setHeaderMask(_headerMask);              \
    } while (0)

#define OMCID_GET_MDM_OBJ_OID(_mdmObj) ( (unsigned int)(*((MdmObjectId *) _mdmObj)) )


/*
 * structures
 */
typedef struct {
    int logLevel;
    UINT32 prevTcId[OMCI_PACKET_PRIORITY_MAX];
    omciPacket prevRespPacket[OMCI_PACKET_PRIORITY_MAX];
    _GponOmciStatsObject stats;
    UINT16 flags;
} gponOmci_t;

typedef struct {
    UINT16 meClass;
    void* mdmObj;
} mibUpload_t;


typedef enum
{
    STOP_ON_EOF=0,      // capture terminates when eof is reached
                        //
    WRAP_ON_EOF,        // capture wraps when eof is reached and overwrites the whole file
                        //
    HYBRID              // file divided into two capture buffers of 3/4 and 1/4 of the file size.
                        // Whole file is written first time thru, but once eof is reached capture
                        // continues in the 1/4 dedicated to overflow
    
} EofOption_t;


typedef struct OmciCaptureReplay
{    
EofOption_t     EofOption;                          // How to handle EOF...

FILE    *pSrcLogFile;
FILE    *pDstLogFile;

char    TagFormatStr[64];
char    Directory[16];
char    BaseFileName[16];
char    FullFileName[256];
char    TextBuffer[256];
char    StartTimeStamp[32];
char    StopTimeStamp[32];

UINT32          LogFileAllocSize;                   // in bytes
UINT32          MaxMsgCount;                    // value can change if user specifies a log file size larger than the default
UINT32          ActualMsgCnt;
UINT32          ReplayedMsgCount;

    struct
    {
        UINT32  MaxMsgCnt;
        UINT32  ActualMsgCnt;
        UINT32  LostMsgCnt;
        UINT32  OldestMsg;
        UINT32  NextMsg;
        
    } Section[2];

    struct 
    {
        UINT32  Capturing:1;
        UINT32  Replaying :1;
        UINT32  Viewing:1;
        UINT32  OverwriteFile:1;

    } Flags;

} OmciCaptureReplay_t;

//OmciCaptureReplay_t OmciCapture = {STOP_ON_EOF,NULL,NULL,"%010d.%010d:omci capture:","/var/","omci.msg","","","","",0,0,0,{0,0,0,0,0},{0,0,0,0,0},0};



typedef enum
{
    STOP=0,
    START,
    RESTART
    
} OmciCaptureState_t;


/*
 * global variables
 */
extern gponOmci_t gponOmci; /* main structure */


/*
 * function prototypes
 */
CmsRet omci_msg_init(void);
int omci_msg_handler(omciPacket *pPacket, int len);
CmsRet omci_msg_dump_me(UINT16 meClass, UINT16 meInst);
int omci_me_handler(omciPacket *pPacket, int len);


#endif /* _GPON_OMCI_H_ */

