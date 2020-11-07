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
:>
*/

#ifndef __FBOND_H_INCLUDED__
#define __FBOND_H_INCLUDED__

#define FLOWBOND_VERSION             "v1.0"

#define FLOWBOND_VER_STR             FLOWBOND_VERSION
#define FLOWBOND_MODNAME             "Broadcom Packet Flow Bond"
#define FLOWBOND_NAME                "fbond"
#define FBOND_PROCFS_DIR_PATH        FLOWBOND_NAME    /* dir: /procfs/fbond    */

/* Flow Cache Character Device */
#define FBOND_DRV_MAJOR              312
#define FBOND_DRV_NAME               FLOWBOND_NAME
#define FBOND_DRV_DEVICE_NAME        "/dev/" FBOND_DRV_NAME

#define FLOWBOND_MAX_GROUP          1
#define FLOWBOND_MAX_DEV_IN_GROUP   8

/*
 * Conditional compilation of cache aligned declaration of flow members
 */
#define CC_FBOND_ALIGNED_DECLARE
#if defined(CC_FBOND_ALIGNED_DECLARE)
#define _FBALIGN_     ____cacheline_aligned
#else
#define _FBALIGN_
#endif

/* LAB ONLY: Design development */
/* Menuconfig: BRCM_DRIVER_FBOND_DEBUG selection will add -DPKTDBG to C Flags*/
#ifdef PKTDBG
#define CC_CONFIG_FBOND_DEBUG

/* LAB ONLY: Design development */
#define CC_CONFIG_FBOND_COLOR          /* Color highlighted debugging     */
#define CC_CONFIG_FBOND_DBGLVL     0   /* DBG_BASIC Level                 */
#define CC_CONFIG_FBOND_DRV_DBGLVL 0   /* DBG_BASIC Level Basic           */
#endif

/* Functional interface return status */
#define FBOND_ERROR                (-1)    /* Functional interface error     */
#define FBOND_SUCCESS              0       /* Functional interface success   */

/* fb_error: unconditionally compiled */
#define fb_error(fmt, arg...)      \
        printk( CLRerr DBGsys "%-10s ERROR: " fmt CLRnl, __FUNCTION__, ##arg )

#undef FBOND_DECL
#define FBOND_DECL(x)      x,  /* for enum declaration in H file */

typedef enum
{
    FBOND_DECL(FBOND_DBG_DRV_LAYER)
    FBOND_DECL(FBOND_DBG_FB_LAYER)
    FBOND_DECL(FBOND_DBG_LAYER_MAX)
} FbondDbgLayer_t;


/*
 *------------------------------------------------------------------------------
 *              Flow Bond character device driver IOCTL enums
 *------------------------------------------------------------------------------
 */
typedef enum FbondIoctl
{
/* IOCTL cmd values 1 and 2 are mapped to FIBMAP and FIGETBSZ on ARM
   processor. Hence start all IOCTL values from 100 to prevent conflicts */
    FBOND_IOCTL_DUMMY=99,
    FBOND_DECL(FBOND_IOCTL_STATUS)
    FBOND_DECL(FBOND_IOCTL_ENABLE)
    FBOND_DECL(FBOND_IOCTL_DISABLE)
    FBOND_DECL(FBOND_IOCTL_INTERVAL)
    FBOND_DECL(FBOND_IOCTL_ADDIF)
    FBOND_DECL(FBOND_IOCTL_DELETEIF)
    FBOND_DECL(FBOND_IOCTL_TOKENS)
    FBOND_DECL(FBOND_IOCTL_TEST)
    FBOND_DECL(FBOND_IOCTL_DEBUG)
    FBOND_DECL(FBOND_IOCTL_INVALID)
} FbondIoctl_t;

typedef struct
{
    unsigned int groupindex;
    unsigned int ifindex;
} FbondIoctlAddDeleteIf_t;

typedef struct
{
    unsigned int groupindex;
    unsigned int ifindex;
    unsigned int tokens;
    unsigned int max_tokens;
} FbondIoctlTokens_t;

 #include <pktHdr.h>

#define FBOND_HTABLE_SIZE         8192     /* Must not be greater than 16384 */

/* Flow bond system periodic timer */
#define FBOND_REFRESH              ( 8 )           /* Poll timer interval   */
#define FBOND_SLICE_MAX_ENT_COUNT  128
#define FBOND_REFRESH_INTERVAL     ( FBOND_REFRESH SECONDS )

#endif  /* defined(__FBOND_H_INCLUDED__) */
