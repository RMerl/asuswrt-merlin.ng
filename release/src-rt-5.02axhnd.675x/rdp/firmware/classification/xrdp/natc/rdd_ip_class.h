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

#ifndef _RDD_IP_CLASS_H
#define _RDD_IP_CLASS_H

#include "rdd.h"
#include "rdd_common.h"
#include "rdd_ip_class_inline.h"
#include "rdd_defs.h"
#include "rdd_data_structures_auto.h"

/* metadata_1_0 - 10 bits (unioned, doesn't have own auto-gens) */
#define METADATA_1_0_WIDTH 10
#define METADATA_1_0_MASK ((1 << METADATA_1_0_WIDTH) - 1)
/* metadata_1_1 */
#define METADATA_1_1_WIDTH (FLOW_CACHE_CONTEXT_ENTRY_FLOW_PRIO_OR_METADATA_1_1_F_WIDTH)
#define METADATA_1_1_MASK ((1 << METADATA_1_1_WIDTH) - 1)
/* metadata_1_2 */
#define METADATA_1_2_WIDTH (FLOW_CACHE_CONTEXT_ENTRY_METADATA_1_2_OR_TCPSPDTEST_STREAM_ID_F_WIDTH)
#define METADATA_1_2_MASK ((1 << METADATA_1_2_WIDTH) - 1)

void rdd_ip_class_result_entry_compose(rdd_fc_context_t *ctx, uint8_t *entry, rdpa_traffic_dir dir);
void rdd_ip_class_result_entry_decompose(rdd_fc_context_t *ctx, uint8_t *entry);

#endif /* RDD_IP_CLASS_H_ */
