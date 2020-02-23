/*
* <:copyright-BRCM:2008-2014:proprietary:standard
* 
*    Copyright (c) 2008-2014 Broadcom 
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
#ifndef _BCMPWRMNGTDRV_H
#define _BCMPWRMNGTDRV_H

#include <linux/ioctl.h>
#include "bcmpwrmngtcfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PWRMNGT_DRV_MAJOR 300

#define PWRMNGT_IOCTL_INITIALIZE \
    _IOWR(PWRMNGT_DRV_MAJOR, 0, PWRMNGT_DRV_INITIALIZE)
#define PWRMNGT_IOCTL_UNINITIALIZE \
    _IOR(PWRMNGT_DRV_MAJOR, 1, PWRMNGT_DRV_STATUS_ONLY)
#define PWRMNGT_IOCTL_SET_CONFIG \
    _IOWR(PWRMNGT_DRV_MAJOR, 2, PWRMNGT_DRV_CONFIG_PARAMS)
#define PWRMNGT_IOCTL_GET_CONFIG \
    _IOWR(PWRMNGT_DRV_MAJOR, 3, PWRMNGT_DRV_CONFIG_PARAMS)

#define MAX_PWRMNGT_DRV_IOCTL_COMMANDS 4


/* Typedefs. */
typedef struct
{
    PWRMNGT_STATUS status;
} PWRMNGT_DRV_STATUS_ONLY, *PPWRMNGT_DRV_STATUS_ONLY;

typedef struct
{
   PWRMNGT_CONFIG_PARAMS     init;
   PWRMNGT_STATUS            status;
} PWRMNGT_DRV_INITIALIZE, *PPWRMNGT_DRV_INITIALIZE;


typedef struct
{
   PWRMNGT_CONFIG_PARAMS        configParams;
   ui32                         configMask;
   PWRMNGT_STATUS               status;
} PWRMNGT_DRV_CONFIG_PARAMS, *PPWRMNGT_DRV_CONFIG_PARAMS ;


#ifdef __cplusplus
}
#endif

#endif /* _BCMPWRMNGTDRV_H */
