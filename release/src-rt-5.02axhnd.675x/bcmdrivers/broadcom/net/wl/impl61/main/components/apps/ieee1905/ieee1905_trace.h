/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2013:proprietary:standard
 *
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 *
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 *
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :>
 *
 * $Change: 111969 $
 ***********************************************************************/

#ifndef _IEEE1905_TRACE_H_
#define _IEEE1905_TRACE_H_

#include <time.h>
#include "ieee1905_utils.h"

/*
 * IEEE1905 Trace
 */

enum {
  i5TraceMessage   = 0,
  i5TraceTlv       = 1,
  i5TraceDm        = 2,
  i5TraceInterface = 3,
  i5TraceFlow      = 4,
  i5TraceTimer     = 5,
  i5TraceSocket    = 6,
  i5TraceMain      = 7,
  i5TraceSecurity  = 8,
  i5TraceUdpSocket = 9,
  i5TracePlc       = 10,
  i5TraceWlcfg     = 11,
  i5TraceControl   = 12,
  i5TraceNetlink   = 13,
  i5TraceJson      = 14,
  i5TraceEthStat   = 15,
  i5TraceBrUtil    = 16,
  i5TraceGlue      = 17,
  i5TraceCmsUtil   = 18,
  i5TraceNoMod     = 19,
  i5TraceLast,
  /* special handling */
  i5TracePacket,

};

enum {
  i5TraceLevelError  = 1,
  i5TraceLevelNormal = 2,
  i5TraceLevelInfo   = 3,
} ;

extern long lastTraceTimeMs;
extern int  i5TimeTraces;

#define MAP_MAX_PROCESS_NAME		30	/* Maximum length of process NAME */
extern char g_map_process_name[MAP_MAX_PROCESS_NAME];

#define i5TracePrint(level, module, fmt, arg...) \
    do { \
        if (i5TraceGet(module) >= level) { \
            if (i5TimeTraces) { \
                struct timespec traceTime; \
                long traceTimeMs; \
                clock_gettime(CLOCK_REALTIME,&traceTime); \
                traceTimeMs=traceTime.tv_nsec / 1000000L + traceTime.tv_sec*1000L; \
                printf("%s >> {+%5ld} %s [line %d] " fmt, g_map_process_name, traceTimeMs-lastTraceTimeMs, __FUNCTION__, __LINE__, ##arg); \
                lastTraceTimeMs=traceTimeMs; \
            } else { \
                printf("%s >> %s [line %d] " fmt, g_map_process_name, __FUNCTION__, __LINE__, ##arg); \
            } \
        } \
    } while(0)

#define i5TraceModuleInfo(module, fmt, arg...)   \
    i5TracePrint(i5TraceLevelInfo, module, fmt, ##arg )

#define i5TraceModule(module,fmt, arg...)   \
    i5TracePrint(i5TraceLevelNormal, module, fmt, ##arg)

#define i5TraceModuleError(module,fmt, arg...)   \
    i5TracePrint(i5TraceLevelError, module, fmt, ##arg)

#define i5TraceInfo(fmt, arg...)    i5TraceModuleInfo(I5_TRACE_MODULE, fmt, ##arg)
#define i5Trace(fmt, arg...)        i5TraceModule(I5_TRACE_MODULE, fmt, ##arg)
#define i5TraceError(fmt, arg...)   i5TraceModuleError(I5_TRACE_MODULE, fmt, ##arg)

#define i5Debug(fmt, arg...)        i5TraceModule(i5TraceNoMod, fmt, ##arg)

#define i5TraceAssert(cond)   \
    if (!(cond)) { \
       fprintf(stderr, "%s [line %d] ASSERTION FAILURE: " #cond "\n", __FUNCTION__, __LINE__); \
    }

#define i5TracePacket(dir, idx, fmt, arg...)   \
  if (i5TracePacketGetDepth() & dir) { \
    unsigned int index = i5TracePacketGetIndex(); \
    if ( (0 == index) || (idx == index)) { \
      printf(fmt, ##arg); \
    } \
  }

#define i5TraceDirPrint(fmt, arg...) \
  printf("IEEE1905-%s >> %s(%d) : "fmt, g_map_process_name, __FUNCTION__, __LINE__, ##arg)

int i5TraceGet(int module_id);
int i5TracePacketGetDepth(void);
int i5TracePacketGetIndex(void);
void i5TraceSet(int module_id, unsigned int depth, unsigned int ifindex, unsigned char *ifmacaddr);
void i5TraceTimestampSet(int value);

#endif // endif
