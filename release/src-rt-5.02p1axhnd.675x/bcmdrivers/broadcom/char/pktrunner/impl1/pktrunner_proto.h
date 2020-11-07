#ifndef __PKT_RUNNER_H_INCLUDED__
#define __PKT_RUNNER_H_INCLUDED__

/*
<:copyright-BRCM:2013:proprietary:standard

   Copyright (c) 2013 Broadcom 
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

#if !defined(RDP_SIM)
#define CC_PKTRUNNER_PROCFS
#include <linux/seq_file.h>
#endif

#define PKTRNR_MAX_FHW_ACCEL    (2)  /* Max number of HW accelerators towards FHW */


#define PKTRUNNER_ACCEL_UCAST   (0)
#define PKTRUNNER_ACCEL_MCAST   (1)

#ifndef CONFIG_BCM_RUNNER_MAX_FLOWS
#define PKTRUNNER_MAX_FLOWS       (16384)
#else
#define PKTRUNNER_MAX_FLOWS       (CONFIG_BCM_RUNNER_MAX_FLOWS) 
#endif
#define PKTRUNNER_MAX_FLOWS_MCAST 1024


#define MAX_TUNNELS_NUM  2
#define GRE_TUNNEL_INDEX 0
#define DS_LITE_TUNNEL_INDEX 1


/* TODO - going forward, common stuff between impl1 & impl2 should be moved into common file */
typedef enum {
    PKTRNR_FLOW_TYPE_INV    = 0,
    PKTRNR_FLOW_TYPE_IP     = 1,
    PKTRNR_FLOW_TYPE_L2     = 2,
    PKTRNR_FLOW_TYPE_MC     = 3, /* Mcast on PON used separate accelerator, so type is not
                                  * mandatory but keep it to stay similar with DSL. In future
                                  * PON/DSL should have common file to do pktrunner flow mgmt. */
}e_PKTRNR_FLOW_TYPE;


#define protoLogDebug bcmLog_logIsEnabled(BCM_LOG_ID_PKTRUNNER, BCM_LOG_LEVEL_DEBUG)
#define protoDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define protoInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define protoNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define protoError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define protoPrint(fmt, arg...)   bcm_printk(fmt, ##arg)


int __init runnerHost_construct(void);
void __exit runnerHost_destruct(void);

int __init runnerProto_construct(void);
void __exit runnerProto_destruct(void);

#if defined(CC_PKTRUNNER_PROCFS)
uint32_t pktRunnerGetState(struct seq_file *, uint32_t accel);
#endif

#endif  /* defined(__PKT_RUNNER_H_INCLUDED__) */
