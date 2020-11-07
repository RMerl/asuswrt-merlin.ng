/*
  <:copyright-BRCM:2015:proprietary:standard
  
     Copyright (c) 2015 Broadcom 
     All Rights Reserved
  
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
*/

/****************************************************************************
 *
 * pon_drv.h -- Bcm Wan Driver Head File
 *
 * Description:
 *      This file contains the common definitions for BCM Wan Driver
 *  Note: internal head file
 *
 * Authors: Fuguo Xu, Akiva Sadovski
 *
 * $Revision: 1.1 $
 *
 * $Id: pon_drv.h,v 1.1 2015/12/21 Fuguo Exp $
 *
 * $Log: pon_drv.h,v $
 * Revision 1.1  2015/12/21 Fuguo
 * Initial version.
 *
 ****************************************************************************/

#ifndef __PON_DRV_H_INCLUDED__
#define __PON_DRV_H_INCLUDED__

#include <linux/bcm_log.h>

extern uint16_t optics_type;

#define __logDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_GPON_SERDES, "SERDES: "fmt, ##arg)
#define __logInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_GPON_SERDES, "SERDES: "fmt, ##arg)
#define __logNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_GPON_SERDES, "SERDES: "fmt, ##arg)
#define __logError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_GPON_SERDES, "SERDES: "fmt, ##arg)
#define __logLevelSet(level)      bcmLog_setLogLevel(BCM_LOG_ID_GPON_SERDES, level)
#define __logLevelGet()           bcmLog_getLogLevel(BCM_LOG_ID_GPON_SERDES)

int serdes_debug_init(void);
void wan_serdes_temp_init(void);

#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878)
#define CHOP(A, B)     B
#define WAN_SERDES     SERDES_STATUS
#define EPON_GBOX_PON_RX_WIDTH_MODE     EPON_GBOX_RX_WIDTH_MODE
#define TOP_OSR        SERDES_STATUS
#define CONTROL        OVERSAMPLE_CTRL
#define STATUS_0        STATUS0
#elif defined(CONFIG_BCM96856)
#define CHOP(A, B)     B
#else
#define CHOP(A, B)     A##B
#endif

#endif  /* __PON_DRV_H_INCLUDED__ */

