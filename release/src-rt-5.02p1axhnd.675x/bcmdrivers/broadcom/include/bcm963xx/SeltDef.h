/*
* <:copyright-BRCM:2015:proprietary:standard
* 
*    Copyright (c) 2015 Broadcom 
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

#ifndef __SELTDEF_H__
#define __SELTDEF_H__

#define SELT_STATE_IDLE                    0
#define SELT_STATE_MEASURING               1
#define SELT_STATE_POSTPROCESSING          2
#define SELT_STATE_COMPLETE                3

#define SELT_STATE_MEASUREMENT_SHIFT       8
#define SELT_STATE_MASK                    ((1<<SELT_STATE_MEASUREMENT_SHIFT)-1)

#define SELT_STATE_WAITING                 ((1<<SELT_STATE_MEASUREMENT_SHIFT)|SELT_STATE_MEASURING)
#define SELT_STATE_MEASURING_QLN           ((2<<SELT_STATE_MEASUREMENT_SHIFT)|SELT_STATE_MEASURING)
#define SELT_STATE_MEASURING_ENR           ((4<<SELT_STATE_MEASUREMENT_SHIFT)|SELT_STATE_MEASURING)
#define SELT_STATE_MEASURING_SELT          ((8<<SELT_STATE_MEASUREMENT_SHIFT)|SELT_STATE_MEASURING)

#define SELT_STATE_STEP_WAIT               0x01
#define SELT_STATE_STEP_QLN                0x02
#define SELT_STATE_STEP_ENR                0x04
#define SELT_STATE_STEP_SELT               0x08
#define SELT_STATE_STEP_POSTPROCESSING     0x10

#define SELT_STATE_ALL_STEPS               0x1F

typedef struct SeltData {
#if defined(LINUX_FW_EXTRAVERSION) && (LINUX_FW_EXTRAVERSION >= 50204)
    unsigned int seltCfg;
#else
    unsigned long seltCfg;
#endif
    int          seltState;
    unsigned char seltSteps;
    int           seltAgc;
} SeltData;

#endif
