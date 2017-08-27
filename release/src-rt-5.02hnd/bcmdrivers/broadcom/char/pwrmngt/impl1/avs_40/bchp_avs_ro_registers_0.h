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

#ifndef BCHP_AVS_RO_REGISTERS_0_H__
#define BCHP_AVS_RO_REGISTERS_0_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS  (AVS_BASE + 0x00000300) // Indicate PVT monitor sel 000(Temperature Monitoring) measurements data, validity of data and measurement done status
#define BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS_data_MASK (0x3FF << 0)

#define BCHP_AVS_RO_REGISTERS_0_PVT_PROCESS_MNTR_STATUS      (AVS_BASE + 0x00000304) // Indicate PVT monitor sel 001(Process Monitoring) measurements data, validity of data and measurement done status
#define BCHP_AVS_RO_REGISTERS_0_PVT_PROCESS_MNTR_STATUS_data_MASK     (0x3FF << 0)

#define BCHP_AVS_RO_REGISTERS_0_PVT_0P99V_MNTR_STATUS        (AVS_BASE + 0x00000308) // Indicate PVT monitor sel 010(0p99V Monitoring) measurements data, validity of data and measurement done status
#define BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS      (AVS_BASE + 0x0000030c) // Indicate PVT monitor sel 011(1p10V_0 Monitoring) measurements data, validity of data and measurement done status
#define BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_0_MNTR_STATUS_data_MASK     (0x3FF << 0)

#define BCHP_AVS_RO_REGISTERS_0_PVT_1P10V_1_MNTR_STATUS      (AVS_BASE + 0x00000310) // Indicate PVT monitor sel 100(1p10V_1 Monitoring) measurements data, validity of data and measurement done status
#define BCHP_AVS_RO_REGISTERS_0_PVT_2p75V_MNTR_STATUS        (AVS_BASE + 0x00000314) // Indicate PVT monitor sel 101(2p75V Monitoring) measurements data, validity of data and measurement done status
#define BCHP_AVS_RO_REGISTERS_0_PVT_3p63V_MNTR_STATUS        (AVS_BASE + 0x00000318) // Indicate PVT monitor sel 110(3p63V Monitoring) measurements data, validity of data and measurement done status
#define BCHP_AVS_RO_REGISTERS_0_PVT_TESTMODE_MNTR_STATUS     (AVS_BASE + 0x0000031c) // Indicate PVT monitor sel 111(Testmode Monitoring) measurements data, validity of data and measurement done status
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_0            (AVS_BASE + 0x00000320) // Indicate central rosc 0 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_0_data_MASK           (0x7FFF << 0)

#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_1            (AVS_BASE + 0x00000324) // Indicate central rosc 1 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_2            (AVS_BASE + 0x00000328) // Indicate central rosc 2 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_3            (AVS_BASE + 0x0000032c) // Indicate central rosc 3 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_4            (AVS_BASE + 0x00000330) // Indicate central rosc 4 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_5            (AVS_BASE + 0x00000334) // Indicate central rosc 5 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_6            (AVS_BASE + 0x00000338) // Indicate central rosc 6 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_7            (AVS_BASE + 0x0000033c) // Indicate central rosc 7 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_8            (AVS_BASE + 0x00000340) // Indicate central rosc 8 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_9            (AVS_BASE + 0x00000344) // Indicate central rosc 9 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_10           (AVS_BASE + 0x00000348) // Indicate central rosc 10 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_11           (AVS_BASE + 0x0000034c) // Indicate central rosc 11 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_12           (AVS_BASE + 0x00000350) // Indicate central rosc 12 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_13           (AVS_BASE + 0x00000354) // Indicate central rosc 13 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_14           (AVS_BASE + 0x00000358) // Indicate central rosc 14 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_15           (AVS_BASE + 0x0000035c) // Indicate central rosc 15 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_16           (AVS_BASE + 0x00000360) // Indicate central rosc 16 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_17           (AVS_BASE + 0x00000364) // Indicate central rosc 17 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_18           (AVS_BASE + 0x00000368) // Indicate central rosc 18 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_19           (AVS_BASE + 0x0000036c) // Indicate central rosc 19 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_20           (AVS_BASE + 0x00000370) // Indicate central rosc 20 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_21           (AVS_BASE + 0x00000374) // Indicate central rosc 21 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_22           (AVS_BASE + 0x00000378) // Indicate central rosc 22 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_23           (AVS_BASE + 0x0000037c) // Indicate central rosc 23 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_24           (AVS_BASE + 0x00000380) // Indicate central rosc 24 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_25           (AVS_BASE + 0x00000384) // Indicate central rosc 25 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_26           (AVS_BASE + 0x00000388) // Indicate central rosc 26 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_27           (AVS_BASE + 0x0000038c) // Indicate central rosc 27 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_28           (AVS_BASE + 0x00000390) // Indicate central rosc 28 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_29           (AVS_BASE + 0x00000394) // Indicate central rosc 29 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_30           (AVS_BASE + 0x00000398) // Indicate central rosc 30 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_31           (AVS_BASE + 0x0000039c) // Indicate central rosc 31 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_32           (AVS_BASE + 0x000003a0) // Indicate central rosc 32 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_33           (AVS_BASE + 0x000003a4) // Indicate central rosc 33 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_34           (AVS_BASE + 0x000003a8) // Indicate central rosc 34 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_35           (AVS_BASE + 0x000003ac) // Indicate central rosc 35 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_36           (AVS_BASE + 0x000003b0) // Indicate central rosc 36 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_37           (AVS_BASE + 0x000003b4) // Indicate central rosc 37 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_38           (AVS_BASE + 0x000003b8) // Indicate central rosc 38 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_39           (AVS_BASE + 0x000003bc) // Indicate central rosc 39 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_40           (AVS_BASE + 0x000003c0) // Indicate central rosc 40 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_41           (AVS_BASE + 0x000003c4) // Indicate central rosc 41 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_42           (AVS_BASE + 0x000003c8) // Indicate central rosc 42 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_43           (AVS_BASE + 0x000003cc) // Indicate central rosc 43 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_44           (AVS_BASE + 0x000003d0) // Indicate central rosc 44 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_45           (AVS_BASE + 0x000003d4) // Indicate central rosc 45 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_46           (AVS_BASE + 0x000003d8) // Indicate central rosc 46 measurement data and validity of data
#define BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_47           (AVS_BASE + 0x000003dc) // Indicate central rosc 47 measurement data and validity of data

#ifdef __cplusplus
}
#endif

#endif

