/*
* <:copyright-BRCM:2013:proprietary:standard
* 
*    Copyright (c) 2013 Broadcom 
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

#ifndef RDPA_PLATFORM_H_
#define RDPA_PLATFORM_H_

#ifdef RDP_SIM
#define INTERRUPT_ID_RDP_RUNNER 0
#else
#include "bcm_OS_Deps.h"
#endif

#define RDPA_RX_MAIN_INTERRUPT_NUM_IN_RDD 0   /**< Main interrupt for CPU RX interface */
#define RDPA_PCI_MAIN_INTERRUPT_NUM_IN_RDD 1  /**< Main interrupt for PCI TX interface (WiFi acceleration) */
#define RDPA_PCI_SUB_INTERRUPT_NUM_IN_RDD 0   /**< Sub-interrupt for PCI TX interface */

/* These constants define the min/max packet size selection indices */
#define RDPA_BBH_RX_ETH_MIN_PKT_SIZE_SELECTION_INDEX 0
#define RDPA_BBH_RX_OMCI_MIN_PKT_SIZE_SELECTION_INDEX 1
#define RDPA_BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX 0

#define RDPA_IC_CPU_RX_IRQ INTERRUPT_ID_RDP_RUNNER   /**< CPU rx IRQ in IC */
#define RDPA_IC_WLAN0_IRQ (RDPA_IC_CPU_RX_IRQ + 1)   /**< Wlan0 IRQ in IC */
#define RDPA_IC_WLAN1_IRQ (RDPA_IC_CPU_RX_IRQ + 1)   /**< Wlan1 IRQ in IC */

#define DS_RATE_LIMITER_TIMER_PERIOD_IN_USEC                    1000
#define US_RATE_LIMITER_TIMER_PERIOD_IN_USEC                    4000

#endif /* RDPA_PLATFORM_H_ */
