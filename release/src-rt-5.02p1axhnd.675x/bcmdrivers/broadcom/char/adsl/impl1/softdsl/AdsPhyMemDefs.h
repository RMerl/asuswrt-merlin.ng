/* 
* <:copyright-BRCM:2002:DUAL/GPL:standard
* 
*    Copyright (c) 2002 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :>
*/

#ifndef _ADSL_PHY_DEFS_H
#define _ADSL_PHY_DEFS_H

#if !defined(CONFIG_BCM96362) && (defined(BOARD_bcm96362) || defined(_BCM96362_))
#define CONFIG_BCM96362	1
#endif

#if !defined(CONFIG_BCM96328) && (defined(BOARD_bcm96328) || defined(_BCM96328_))
#define CONFIG_BCM96328	1
#endif

#if !defined(CONFIG_BCM963268) && (defined(BOARD_bcm963268) || defined(_BCM963268_))
#define	CONFIG_BCM963268	1
#endif

#if !defined(CONFIG_BCM96318) && (defined(BOARD_bcm96318) || defined(_BCM96318_))
#define	CONFIG_BCM96318	1
#endif

#if !defined(CONFIG_BCM963138) && (defined(BOARD_bcm963138) || defined(_BCM963138_))
#define	CONFIG_BCM963138	1
#endif

#if !defined(CONFIG_BCM963381) && (defined(BOARD_bcm963381) || defined(_BCM963381_))
#define	CONFIG_BCM963381	1
#endif

#if !defined(CONFIG_BCM963148) && (defined(BOARD_bcm963148) || defined(_BCM963148_))
#define	CONFIG_BCM963148	1
#endif

#if !defined(CONFIG_BCM963158) && (defined(BOARD_bcm963158) || defined(_BCM963158_))
#define	CONFIG_BCM963158	1
#endif

#if !defined(CONFIG_BCM963178) && (defined(BOARD_bcm963178) || defined(_BCM963178_))
#define	CONFIG_BCM963178	1
#endif

#if !defined(CONFIG_BCM963146) && (defined(BOARD_bcm963146) || defined(_BCM963146_))
#define	CONFIG_BCM963146	1
#endif

#if defined(CONFIG_BCM96362) ||      \
	defined(CONFIG_BCM96328) ||      \
	defined(CONFIG_BCM963268) ||     \
	defined(CONFIG_BCM96318) ||      \
	defined(CONFIG_BCM963138) ||     \
	defined(CONFIG_BCM963381) ||     \
	defined(CONFIG_BCM963148) ||     \
	defined(CONFIG_BCM963158) ||     \
	defined(CONFIG_BCM963178) ||     \
	defined(CONFIG_BCM963146)
#define	CONFIG_BCM963x8
#endif

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
#define	CONFIG_PHY_PARAM
#endif

#if defined(CONFIG_BCM963x8)

#if defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328)

#ifdef ADSL_ANNEXB
#include "../adslcore6362B/adsl_defs.h"
#else
#include "../adslcore6362/adsl_defs.h"
#endif

#elif defined(CONFIG_BCM963268)

#ifdef PHY_LOOPBACK
#include "../adslcore63268LB/adsl_defs.h"
#else
#ifdef ADSL_ANNEXB
#include "../adslcore63268B/adsl_defs.h"
#elif defined(SUPPORT_DSL_BONDING) && !defined(SUPPORT_2CHIP_BONDING)
#ifdef SUPPORT_DSL_BONDING5B
#include "../adslcore63268bnd5/adsl_defs.h"
#else
#include "../adslcore63268bnd/adsl_defs.h"
#endif
#else
#include "../adslcore63268/adsl_defs.h"
#endif
#endif

#elif defined(CONFIG_BCM96318)

#ifdef ADSL_ANNEXB
#include "../adslcore6318B/adsl_defs.h"
#else
#include "../adslcore6318/adsl_defs.h"
#endif

#elif defined(CONFIG_BCM963138)

#if defined(PHY_LOOPBACK)
#include "../adslcore63138LB/adsl_defs.h"
#elif defined(ADSL_ANNEXB)
#include "../adslcore63138B/adsl_defs.h"
#else
#include "../adslcore63138/adsl_defs.h"
#endif

#elif defined(CONFIG_BCM963381)

#ifdef PHY_LOOPBACK
#include "../adslcore63381LB/adsl_defs.h"
#else
#ifdef ADSL_ANNEXB
#include "../adslcore63381B/adsl_defs.h"
#else
#include "../adslcore63381/adsl_defs.h"
#endif
#endif

#elif defined(CONFIG_BCM963148)

#ifdef PHY_LOOPBACK
#include "../adslcore63148LB/adsl_defs.h"
#else
#ifdef ADSL_ANNEXB
#include "../adslcore63148B/adsl_defs.h"
#else
#include "../adslcore63148/adsl_defs.h"
#endif
#endif

#elif defined(CONFIG_BCM963158)

#ifdef PHY_LOOPBACK
#include "../adslcore63158LB/adsl_defs.h"
#elif defined(PHY_CO)
#include "../adslcore63158CO/adsl_defs.h"
#elif defined(ADSL_ANNEXB)
#include "../adslcore63158B/adsl_defs.h"
#elif (CONFIG_BRCM_CHIP_REV==0x63158A0)
#include "../adslcore63158_A0/adsl_defs.h"
#else
#include "../adslcore63158/adsl_defs.h"
#endif

#elif defined(CONFIG_BCM963178)

#ifdef PHY_LOOPBACK
#include "../adslcore63178LB/adsl_defs.h"
#elif defined(ADSL_ANNEXB)
#include "../adslcore63178B/adsl_defs.h"
#else
#include "../adslcore63178/adsl_defs.h"
#endif

#elif defined(CONFIG_BCM963146)

#ifdef PHY_LOOPBACK
#include "../adslcore63146LB/adsl_defs.h"
#elif defined(ADSL_ANNEXB)
#include "../adslcore63146B/adsl_defs.h"
#else
#include "../adslcore63146/adsl_defs.h"
#endif

#endif
#endif /* defined(CONFIG_BCM963x8) */

//#define SDRAM4G_SUPPORT1
#ifdef SDRAM4G_SUPPORT1
#define SDRAM4G_FULL_SUPPORT    /* indicates that both PHY and DSL driver support SDRAM4G */
#endif
#if defined(CONFIG_BCM963146)
#define SDRAM_EXCL_ADDR_LOW     0x19000000    /* LMEM & RMEM */
#define SDRAM_EXCL_ADDR_HIGH    0x1A000000    /* registers @ 0x19E0_0000  */
#else
#define SDRAM_EXCL_ADDR_LOW     0x19000000    /* LMEM & RMEM */
#define SDRAM_EXCL_ADDR_HIGH    0x20000000    /* registers @ 0x1FE0_0000  */
#endif

#ifndef	ADSL_PHY_SDRAM_LINK_OFFSET
#define ADSL_PHY_SDRAM_LINK_OFFSET		0x00040000
#endif

#ifndef ADSL_PHY_SDRAM_BIAS
#define ADSL_PHY_SDRAM_BIAS				0x00040000
#endif

#ifndef	ADSL_PHY_SDRAM_PAGE_SIZE
#define ADSL_PHY_SDRAM_PAGE_SIZE		0x00080000
#endif

#ifdef ADSL_PHY_SDRAM_BIAS
#define ADSL_SDRAM_IMAGE_SIZE			(ADSL_PHY_SDRAM_PAGE_SIZE - ADSL_PHY_SDRAM_BIAS)
#else
#define ADSL_SDRAM_IMAGE_SIZE			(256*1024)
#endif

//#define ADSL_SDRAM_RESERVE_SIZE			0x20000000


#endif

