#ifndef __ARL_H_INCLUDED__
#define __ARL_H_INCLUDED__

/*
* <:copyright-BRCM:2009:proprietary:standard
* 
*    Copyright (c) 2009 Broadcom 
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

/*
 *******************************************************************************
 * File Name  : arl.h
 *
 * Description: This file contains the specification of some common definitions
 *      and interfaces to other modules. This file may be included by both
 *      Kernel and userapp (C only).
 *
 *******************************************************************************
 */


/*----- Defines -----*/

#define ARL_VERSION              "0.1"
#define ARL_VER_STR              "v" ARL_VERSION
#define ARL_MODNAME              "Broadcom Address Resolution Logic Processor (ARL)"

#define ARL_NAME                 "bcmarl"

#ifndef ARL_ERROR
#define ARL_ERROR                (-1)
#endif
#ifndef ARL_SUCCESS
#define ARL_SUCCESS              0
#endif

/* ARL Character Device */
#define ARLDRV_MAJOR             305
#define ARLDRV_NAME              ARL_NAME
#define ARLDRV_DEVICE_NAME       "/dev/" ARLDRV_NAME

/* ARL Control Utility Executable */
#define ARL_CTL_UTILITY_PATH     "/bin/arlctl"

/* ARL Proc FS Directory Path */
#define ARL_PROC_FS_DIR_PATH     ARL_NAME

/* Menuconfig: BRCM_DRIVER_PKTFLOW_DEBUG selection will cause -DPKTDBG C Flags*/
#ifdef PKTDBG
#define CC_ARL_DEBUG
#define CC_ARL_ASSERT
#endif

#if defined( __KERNEL__ )
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0)
#include <asm/system.h>
#else
#include <asm/cmpxchg.h>
#endif
#define KERNEL_LOCK(level)          local_irq_save(level)
#define KERNEL_UNLOCK(level)        local_irq_restore(level)
#endif

/*
 *------------------------------------------------------------------------------
 * Common defines for ARL layers.
 *------------------------------------------------------------------------------
 */
#define ARL_DECL(x)                 #x, /* for string declaration in C file */
#undef ARL_DECL
#define ARL_DECL(x)                 x,  /* for enum declaration in H file */

/*
 *------------------------------------------------------------------------------
 *              Packet CFM character device driver IOCTL enums
 * A character device and the associated userspace utility for design debug.
 * Include arlParser.h for ACTIVATE/DEACTIVATE IOCTLs
 *------------------------------------------------------------------------------
 */
typedef enum {
/* IOCTL cmd values 1 and 2 are mapped to FIBMAP and FIGETBSZ on ARM
   processor. Hence start all IOCTL values from 100 to prevent conflicts */
    ARL_IOC_DUMMY=99,
    ARL_DECL(ARL_IOC_INIT)
    ARL_DECL(ARL_IOC_SHOW)
    ARL_DECL(ARL_IOC_FLUSH)
    ARL_DECL(ARL_IOC_DEBUG)
    ARL_DECL(ARL_IOC_MAX)
} arlIoctl_t;


typedef enum {
    ARL_PKT_CONTINUE,
    ARL_PKT_DONE,
    ARL_PKT_MAX
} arlAction_t;

typedef enum {
    ARL_DECL(ARL_HW_ENGINE_SWC)
    ARL_DECL(ARL_HW_ENGINE_MCAST)
    ARL_DECL(ARL_HW_ENGINE_ARL)
    ARL_DECL(ARL_HW_ENGINE_L2FLOW)
    ARL_DECL(ARL_HW_ENGINE_ALL) /* max number of ARL_HW enum */
} ArlHwEngine_t;

#define ARL_HW_TUPLE_MCAST_MASK    (1<<12)  /* must be an integer power of 2 */
#define ARL_HW_TUPLE_ARL_MASK      (1<<13)  /* must be an integer power of 2 */

/* Construct a 16bit tuple from the Engine and matched FlowInfo Element. */
#define ARL_HW_TUPLE(eng,entIx)                                        \
    ( (eng == ARL_HW_ENGINE_MCAST) ?                                   \
      (__force uint16_t)(entIx | ARL_HW_TUPLE_MCAST_MASK) :            \
      ( (eng == ARL_HW_ENGINE_ARL) ?                                   \
        (__force uint16_t)(ARL_HW_TUPLE_ARL_MASK) :                    \
          (__force uint16_t)(entIx) ) )
#define ARL_HW_TUPLE_INVALID         FHW_TUPLE_INVALID


#endif  /* defined(__ARL_H_INCLUDED__) */
