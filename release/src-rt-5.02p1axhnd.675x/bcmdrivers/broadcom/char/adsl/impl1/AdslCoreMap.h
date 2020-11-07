/* 
* <:copyright-BRCM:2002:proprietary:standard
* 
*    Copyright (c) 2002 Broadcom 
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
/****************************************************************************
 *
 * AdslCoreMap.h -- Definitions for ADSL core hardware
 *
 * Description:
 *	Definitions for ADSL core hardware
 *
 *
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: AdslCoreMap.h,v 1.1 2004/04/08 21:24:49 ilyas Exp $
 *
 * $Log: AdslCoreMap.h,v $
 * Revision 1.1  2004/04/08 21:24:49  ilyas
 * Initial CVS checkin. Version A2p014
 *
 ****************************************************************************/

#if defined(_NOOS)

#elif defined(__KERNEL__)

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
#include <generated/autoconf.h>
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21))
#include <linux/autoconf.h>
#else
#include <linux/config.h>
#endif

#endif

#if !defined(_NOOS)
#include "bcm_map.h"
#endif
#include "softdsl/AdslCoreDefs.h"

#if defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)
#define XDSL_ENUM_BASE		0xB0D56000	/* DHIF */
#elif defined(CONFIG_BCM963268)
#define XDSL_ENUM_BASE		0xB0756000	/* DHIF */
#elif defined(CONFIG_BCM96318)
#define XDSL_ENUM_BASE		0xB0156000	/* DHIF */
#elif defined(CONFIG_BCM963138) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963148)
#define XDSL_ENUM_BASE		(DSLPHY_BASE + 0x00056000)
#elif defined(CONFIG_BCM963158) || defined(CONFIG_BCM963146)
#ifdef _NOOS
#ifdef CONFIG_BCM963158
#define XDSL_ENUM_BASE		0x80657000
#else
#define XDSL_ENUM_BASE		0x80757000
#endif
#else
#define XDSL_ENUM_BASE		(DSLPHY_BASE + 0x00007000)
#endif
#elif defined(CONFIG_BCM963178)
#define XDSL_ENUM_BASE		(DSLPHY_BASE + 0x00006000)
#else
#error  No definition for XDSL_ENUM_BASE
#endif

#ifndef HOST_LMEM_BASE
#if defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)
 #define HOST_LMEM_BASE          0xB0D80000
#elif defined(CONFIG_BCM963268)
 #define HOST_LMEM_BASE          0xB0780000
#elif defined(CONFIG_BCM96318)
 #define HOST_LMEM_BASE          0xB0180000
#elif defined(CONFIG_BCM963138) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM963146)
 #if defined(_NOOS) && defined(CONFIG_BCM963158)
 #define HOST_LMEM_BASE          0x80800000
 #else
 #define HOST_LMEM_BASE          DSLLMEM_BASE
 #endif
#else
 #define HOST_LMEM_BASE          0xFFF00000
#endif
#endif	/* !HOST_LMEM_BASE */


/* Common for: CONFIG_BCM96368, CONFIG_BCM96362, CONFIG_BCM96328, CONFIG_BCM963268 */
#define MSGINT_BIT			0x00000100
#define MSGINT_MASK_BIT		0x00000001

#define ADSL_Core2HostMsg		(0x034 / 4)
#define ADSL_INTMASK_I		(0x03c / 4)
#define ADSL_INTSTATUS_I		ADSL_INTMASK_I
#define ADSL_HOSTMESSAGE	(0x038 / 4)

