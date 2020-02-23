/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
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
*/



/*
 * RDPA utilities
 */

#ifndef _RDPA_UTILS_H_
#define _RDPA_UTILS_H_

#include "rdd_defs.h"
#include "rdd_utils.h"

/* 
 * Macro utilities for reading from trace buffer 
 * You pass in a 16-bit or 32-bit pointer to these macros
 */
#ifdef RUNNER_FWTRACE_32BIT
        /* The layout of the buffer in 32-bit mode is:
                 Word 0: bits 15:0 - Event Num index0, bits 31:16 - Thread Num index0
                 Word 1: Time Count for index0
                 Word 2: bits 15:0  - Event Num index1, bits 31:16 - Thread Num index1
                 Word 3: Time Count for index1
                 etc
             */
        #define RDPA_FWTRACE_READ_TIME_CNTR(x) ((ntohl(x[i+1]) & 0x0FFFFFFF))
        #define RDPA_FWTRACE_READ_EVENT(x) (ntohl(x[i]) & 0xFFFF)
        #define RDPA_FWTRACE_READ_THREAD(x) ((ntohl(x[i]) & 0xFF0000) >> 16)
#else
        /* The layout of the buffer in 16-bit mode is:
                 Word 0: bits 15:0 - Time Count for index0.  bits 23:16 Event Num for index 0, bits 31:24 Thread Num for index 0
                 Word 1: bits 15:0 - Time Count for index1.  bits 23:16 Event Num for index 1, bits 31:24 Thread Num for index 1
                 etc
             */
        #define RDPA_FWTRACE_READ_TIME_CNTR(x) (ntohl(x[i]) & 0xFFFF)
        #define RDPA_FWTRACE_READ_EVENT(x) ((ntohl(x[i]) & 0xFF0000) >> 16)
        #define RDPA_FWTRACE_READ_THREAD(x) ((ntohl(x[i]) & 0xFF000000) >> 24)
#endif


/** Clear the FW Trace
 */
void rdpa_fwtrace_clear(void);

/** FW Enable or Disable
 * \param[in]   enable      1 to enable FW Trace, 0 to disable
 * \returnVal    0 on success, -1 on failure
 */
int rdpa_fwtrace_enable_set(uint32_t enable);

/** Read the last FW Trace for a given processor
 * \param[in]   runner_id      Runner processor's FW Trace to read
 * \param[out]   trace_length - function will populate pointer data with trace length
 * \param[out]   trace_buffer - function will populate with trace buffer
 * \returnVal    0 on success, -1 on failure
 */
int rdpa_fwtrace_get(LILAC_RDD_RUNNER_INDEX_DTS runner_id,
                            uint32_t *trace_length,
                            uint32_t *trace_buffer);

/** Get Thread Name from Runner A
 * \param[in]   threadId    Numerical thread ID 
 * \param[out]   pName       pointer to location to return thread name
 * \returnVal    0 on success, -1 on failure
 */
int rdpa_fwtrace_rnr_a_thread_name_get(int thread_id, char *p_mame);

/** Get Thread Name from Runner B
 * \param[in]   threadId    Numerical thread ID 
 * \param[out]   pName       pointer to location to return thread name
 * \returnVal    0 on success, -1 on failure
 */
int rdpa_fwtrace_rnr_b_thread_name_get(int thread_id, char *p_mame);

/** Get FW Trace Event Name from eventId
 * \param[in]   eventId    Numerical event ID
 * \param[out]   pName     pointer to location to return event name
 * \returnVal    0 on success, -1 on failure
 */
int rdpa_fwtrace_event_name_get(int event_id, char *p_name);

#endif
