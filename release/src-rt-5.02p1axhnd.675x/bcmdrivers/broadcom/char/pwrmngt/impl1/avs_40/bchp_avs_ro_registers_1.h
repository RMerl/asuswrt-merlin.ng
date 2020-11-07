/***************************************************************************
 *     (c)2008-2012 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *   
 *  Except as expressly set forth in the Authorized License,
 *   
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *   
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *  
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 *  ANY LIMITED REMEDY.
 * 
 *
 * Module Description:
 *
 * Revision History:
 *
 * 
 ***************************************************************************/

#ifndef BCHP_AVS_RO_REGISTERS_1_H__
#define BCHP_AVS_RO_REGISTERS_1_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BCHP_AVS_RO_REGISTERS_1_POW_WDOG_FAILURE_STATUS          (AVS_BASE + 0x00000400) // Indicate power watchdog failure measurement data and validity of the data
#define BCHP_AVS_RO_REGISTERS_1_INTERRUPT_STATUS_FAULTY_POW_WDOG (AVS_BASE + 0x00000404) // Indicate power watchdogs' status for static monitoring by software
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_0                (AVS_BASE + 0x00000410) // Indicate remote rosc 0 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_0_data_MASK           (0x1FFF << 0)

#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_1                (AVS_BASE + 0x00000414) // Indicate remote rosc 1 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_2                (AVS_BASE + 0x00000418) // Indicate remote rosc 2 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_3                (AVS_BASE + 0x0000041c) // Indicate remote rosc 3 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_4                (AVS_BASE + 0x00000420) // Indicate remote rosc 4 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_5                (AVS_BASE + 0x00000424) // Indicate remote rosc 5 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_6                (AVS_BASE + 0x00000428) // Indicate remote rosc 6 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_7                (AVS_BASE + 0x0000042c) // Indicate remote rosc 7 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_8                (AVS_BASE + 0x00000430) // Indicate remote rosc 8 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_9                (AVS_BASE + 0x00000434) // Indicate remote rosc 9 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_10               (AVS_BASE + 0x00000438) // Indicate remote rosc 10 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_11               (AVS_BASE + 0x0000043c) // Indicate remote rosc 11 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_12               (AVS_BASE + 0x00000440) // Indicate remote rosc 12 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_13               (AVS_BASE + 0x00000444) // Indicate remote rosc 13 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_14               (AVS_BASE + 0x00000448) // Indicate remote rosc 14 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_15               (AVS_BASE + 0x0000044c) // Indicate remote rosc 15 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_16               (AVS_BASE + 0x00000450) // Indicate remote rosc 16 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_17               (AVS_BASE + 0x00000454) // Indicate remote rosc 17 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_18               (AVS_BASE + 0x00000458) // Indicate remote rosc 18 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_19               (AVS_BASE + 0x0000045c) // Indicate remote rosc 19 measurement data and validity of data

#ifdef __cplusplus
}
#endif

#endif

