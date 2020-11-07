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

#ifndef _RDP_DRV_PROJ_POLICER_H_
#define _RDP_DRV_PROJ_POLICER_H_

#include "rdp_drv_proj_cntr.h"

/****************************************************************************** 
  * 
  * POLICER GROUP 0 :
  * --------------
  * Structure: 8B 
  * Parameters: 8B 
  *  
  * Usage: 
  *   Policer   | amount
  *   -------------------
  *   Double    | 80
  *  
  *  
*******************************************************************************/
#define CNPL_POLICER_NUM  80
#define CNPL_POLICER_SIZE  8

/* POLICER GROUP 0: Double */
#define CNPL_POLICER_BASE_ADDR  (CNTR_END_ADDR)
#define CNPL_POLICER_END_ADDR  (CNPL_POLICER_BASE_ADDR + ((CNPL_POLICER_NUM * CNPL_POLICER_SIZE) >> 3))

#define CNPL_POLICER_PARAM_BASE_ADDR  (CNPL_POLICER_END_ADDR)
#define CNPL_POLICER_PARAM_END_ADDR  (CNPL_POLICER_PARAM_BASE_ADDR + ((CNPL_POLICER_NUM * CNPL_POLICER_SIZE) >> 3))
#if ((!defined(CONFIG_RNR_BRIDGE)) && ((CNPL_POLICER_PARAM_END_ADDR * 8) > CNPL_MEMORY_END_ADDR))
    #error " CNPL is exceed HW limit size in policer configuration "
#endif

#endif

