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

/*
 * IEEE1905 Trace
 */

#include "stdio.h"
#include "ieee1905_trace.h"

typedef struct {
  int depth;
} i5Trace_type;

typedef struct {
  int depth;
  unsigned int ifindex;
} i5Trace_packet_type;

i5Trace_type i5TraceModules[i5TraceLast] = {{0}};
i5Trace_packet_type i5TracePacketInfo = {0, 0};

long lastTraceTimeMs=0;
int  i5TimeTraces=0;

int i5TraceGet(int module_id)
{
  return i5TraceModules[module_id].depth;
}

int i5TracePacketGetDepth(void)
{
  return i5TracePacketInfo.depth;
}

int i5TracePacketGetIndex(void)
{
  return i5TracePacketInfo.ifindex;
}

void i5TraceSet(int module_id, unsigned int depth, unsigned int ifindex, unsigned char *ifmacaddr)
{
  int i;

  if ( i5TracePacket == module_id ) {
    i5TracePacketInfo.depth = depth;
    i5TracePacketInfo.ifindex = ifindex;
    if ( (ifmacaddr != NULL) && (0 == ifindex) ) {
      i5_socket_type *socket = i5SocketFindDevSocketByAddr(ifmacaddr, NULL);
      if (NULL != socket) {
        i5TracePacketInfo.ifindex = socket->u.sll.sa.sll_ifindex;
      }
    }
  }
  else if (module_id < i5TraceLast) {
    i5TraceModules[module_id].depth = depth;
  }
  else {
    for (i = 0; i < i5TraceLast; ++i) {
      i5TraceModules[i].depth = depth;
    }
  }
}

void i5TraceTimestampSet(int value)
{
    if (value == 2)
        i5TimeTraces = !i5TimeTraces;
    else
        i5TimeTraces = !!value;
}
