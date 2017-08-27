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

#ifndef BCHP_AVS_PVT_MNTR_CONFIG_H__
#define BCHP_AVS_PVT_MNTR_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL               (AVS_BASE + 0x00000100) // Control bits for PVT monitor
#define BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_TP_MODE_ENABLE     (AVS_BASE + 0x00000108) // Enabling TP mode to use data from tp_in/out pins
#define BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_PROGRAMMING_ENABLE (AVS_BASE + 0x00000110) // SW must set this bit to 1 to modify DAC_CODE, MIN_DAC_CODE and MAX_DAC_CODE
#define BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_PROGRAMMING_ENABLE_code_program_en_MASK (1 << 0)
#define BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE                    (AVS_BASE + 0x00000114) // Represents the input code of the DAC
#define BCHP_AVS_PVT_MNTR_CONFIG_DAC_CODE_dac_code_MASK                           (0x1FF)
#define BCHP_AVS_PVT_MNTR_CONFIG_MIN_DAC_CODE                (AVS_BASE + 0x00000118) // SW may set this to a lower value to prevent the DAC from driving too high of a voltage
#define BCHP_AVS_PVT_MNTR_CONFIG_MAX_DAC_CODE                (AVS_BASE + 0x0000011c) // SW may set this to a higher value to prevent the DAC from driving too low of a voltage

#ifdef __cplusplus
}
#endif

#endif

