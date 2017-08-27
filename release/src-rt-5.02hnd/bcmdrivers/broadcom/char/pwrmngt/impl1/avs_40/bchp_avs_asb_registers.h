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

#ifndef BCHP_AVS_ASB_H__
#define BCHP_AVS_ASB_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BCHP_AVS_ASB_REGISTERS_ASB_COMMAND                   (AVS_BASE + 0x00000200) // Command to initiate read/write transaction on ASB master
#define BCHP_AVS_ASB_REGISTERS_ASB_COMMAND_start                (1<<0)  
#define BCHP_AVS_ASB_REGISTERS_ASB_COMMAND_read_write           (1<<1)  
#define BCHP_AVS_ASB_REGISTERS_ASB_ADDRESS                   (AVS_BASE + 0x00000204) // Identify which slave and register the transaction will take place on
#define BCHP_AVS_ASB_REGISTERS_ASB_ADDRESS_vreg_1p0             (0x103)
#define BCHP_AVS_ASB_REGISTERS_ASB_DATA_PWD_CONFIG_0         (AVS_BASE + 0x00000208) // Data corresponding to start and cfg field of PWD config register
#define BCHP_AVS_ASB_REGISTERS_ASB_DATA_PWD_CONFIG_1         (AVS_BASE + 0x0000020c) // Data corresponding to rsel and clearcfg field of PWD config register
#define BCHP_AVS_ASB_REGISTERS_ASB_DATA_WRAPPER_CONFIG       (AVS_BASE + 0x00000210) // Data corresponding to AVS remote sensor wrapper config registers, dde and pos
#define BCHP_AVS_ASB_REGISTERS_ASB_DATA_PWD_EN               (AVS_BASE + 0x00000214) // Data corresponding to AVS remote sensor wrapper config registers, Enable
#define BCHP_AVS_ASB_REGISTERS_ASB_BUSY                      (AVS_BASE + 0x00000218) // Indicate a transaction is ongoing in the ASB

#ifdef __cplusplus
}
#endif

#endif

