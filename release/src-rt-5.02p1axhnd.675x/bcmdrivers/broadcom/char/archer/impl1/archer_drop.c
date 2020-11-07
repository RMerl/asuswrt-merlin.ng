/*
  <:copyright-BRCM:2019:proprietary:standard

  Copyright (c) 2019 Broadcom 
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

/*
*******************************************************************************
*
* File Name  : archer_drop.c
*
* Description: Archer packet drop algortithms for queue congestion management
*
*******************************************************************************
*/

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/bcm_log.h>

#include "archer.h"
#include "archer_drop.h"

#define CC_ARCHER_DRIVER_BASIC
#include "archer_driver.h"

static inline uint32_t archer_drop_rand(void)
{
    static uint32_t rand_seed = 1;

    rand_seed = 214013 * rand_seed + 2531011;

    return rand_seed;
}

int __archer_drop_packet(archer_drop_config_t *config_p,
                         archer_drop_profile_index_t drop_profile,
                         int queue_level)
{
    archer_drop_profile_index_t profile_index =
        (likely(ARCHER_DROP_ALGORITHM_WRED == config_p->algorithm)) ?
        drop_profile : ARCHER_DROP_PROFILE_LOW;
    archer_drop_profile_t *profile_p = &config_p->profile[profile_index];

    if(profile_p->dropProb)
    {
        if(queue_level > profile_p->maxThres)
        {
            return 1;
        }

        if(queue_level > profile_p->minThres)
        {
            /* current count in the queue is between the min/max threshold, we will drop based on
             * the following calculation:
             * p = drop_p * (Qinst - min_th) / (max_th - min_th) */
            uint32_t dropProb = profile_p->dropProb * (queue_level - profile_p->minThres);

            dropProb = dropProb / (profile_p->maxThres - profile_p->minThres);

            if((archer_drop_rand() % 100) < dropProb)
            {
                return 1;
            }
        }
    }

    return 0;
}
