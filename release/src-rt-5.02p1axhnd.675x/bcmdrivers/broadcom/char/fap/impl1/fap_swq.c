/*
<:copyright-BRCM:2009:proprietary:standard

   Copyright (c) 2009 Broadcom 
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

#include "fap4ke_memory.h"
#include "fap_dqm.h"
#include "fap_dqmHost.h"
#include "fap4ke_swq.h"  
#include "bcm_OS_Deps.h"

void swqRecvMsgHost(fap4ke_SWQueue_t *swq, SWQDataMsg_t *msg, uint32 msgSize, uint32 *qStart, uint32 *qEnd)
{
    int i;
    volatile uint32 *rdPtr = swq->rdPtr;

    for(i=0; i < msgSize; i++)
    {
        msg->data[i] = rdPtr[i] ;
    }

    rdPtr += msgSize;
    if(rdPtr == qEnd)
    {
        rdPtr = qStart;
    }
    swq->rdPtr = rdPtr;
}

void swqDumpHost(fap4ke_SWQueue_t *swq)
{
    printk(">>>>------------------\n");

    printk("swq =%p msgSize =%u words , maxDepth=%u\n",swq, swq->msgSize, swq->maxDepth);
    printk("qStart =%p qEnd=%p swq->currdepth %d\n", swq->qStart, swq->qEnd, swqGetCurrQDepth(swq));
    printk("rdPtr =%p wrPtr=%p dropped %d\n", swq->rdPtr, swq->wrPtr, (int)swq->dropped);
    printk("interrupts %d processed %d pkt/int %d\n", (int)swq->interrupts, (int)swq->processed, 
		   (int)(swq->processed / swq->interrupts));
    printk("Associated DQM=%u dir %s fapId=%d\n",swq->dqm, swq->dqmDir?"FAP2HOST": "HOST2FAP", swq->fapId);
    if(swq->dqmDir) /*FAP2HOST */
    {
        printk("DQM recv available = %d, notEmptyirqMask=%d notEmptyIrqStatus=%d \n",
                dqmRecvAvailableHost(swq->fapId,swq->dqm)? 1: 0,
                dqmReadNotEmptyIrqMskHost(swq->fapId) & (1<< swq->dqm)? 1: 0,
                dqmReadNotEmptyIrqStsHost(swq->fapId) & (1<< swq->dqm)? 1: 0 );  

    }
    else
    {
        printk("DQM xmit available = %d,\n",dqmXmitAvailableHost(swq->fapId,swq->dqm));  
    }
    printk("------------------<<<<\n");
}
