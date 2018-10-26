#ifndef __PERIODICSTAT_H__
#define __PERIODICSTAT_H__

/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2013:proprietary:standard

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
#include <cms.h>

#define PERIODIC_STATS_CHECK_INTERVAL 5
#define PERIODIC_STATS_DIRECTORY "/var/periodicstat/"
#define PERIODIC_STATS_SAMPLE_FILENAME_PREFIX "/var/periodicstat/smpset_"

#if defined(DMP_PERIODICSTATSADV_1) || defined(DMP_DEVICE2_PERIODICSTATSADV_1)
#define SUPPORT_PERIODICSTATSADV 1
#endif

#define CHECK_ALLOC_FAIL3(a,b,c) (!(a) || !(b) || !(c))


#define SAFE_FREE(a)\
do{\
   if ( (a) != NULL )\
      free((a));\
   a=NULL;\
}while(0)\

enum {
   SAMPLEMODE_CURRENT,
   SAMPLEMODE_CHANGE,
};


typedef struct _SampleParameterInfo
{
   char    name[BUFLEN_256];
   char  **values;
   UINT32  id;
   UINT32 *sampleSeconds;
   UINT32 *suspectData;
   UINT32  valueCount;
   UINT32  previousTime;
   struct _SampleParameterInfo *next;
#if SUPPORT_PERIODICSTATSADV
   UINT16  sampleMode;
   UINT16  calculationMode;
   SINT32  lowThreshold;
   SINT32  highThreshold;
   UINT32  failure;
   UINT32  previousValue;
#endif
} SampleParameterInfo;

typedef struct _SampleSetInfo
{ 
   char            name[BUFLEN_128];
   char            startTime[BUFLEN_64];
   char            endTime[BUFLEN_64];
   UINT32          id;
   UINT32          sampleInterval;
   UINT32          reportSamples;
   UINT32          sampleCount;
   UINT32          previousTime;
   UINT32         *sampleSeconds;
   SampleParameterInfo   *parameters;
   struct _SampleSetInfo *next;
#if SUPPORT_PERIODICSTATSADV
   UINT32          fetchSamples;
#endif
} SampleSetInfo;


#endif //__PERIODICSTAT_H__

