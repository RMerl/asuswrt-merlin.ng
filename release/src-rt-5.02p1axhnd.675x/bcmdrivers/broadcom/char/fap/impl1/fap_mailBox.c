/*
 * <:copyright-BRCM:2007:proprietary:standard
 * 
 *    Copyright (c) 2007 Broadcom 
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

//**************************************************************************
// File Name  : fap_mailBox.c
//
// Description: This is the Host side mailbox implementation for the 63268 FAP.
//
//**************************************************************************

#include <bcm_intr.h>
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include "fap_hw.h"
#include "fap_local.h"
#include "fap4ke_mailBox.h"


/* host global variables */
static uint16 hostPrintCount = 0;
static uint32 hostKeepAliveCount[NUM_FAPS] = {0};

static inline void hostToFap_xmit(uint32 fapIdx, fapMailBox_msg_t msg)
{
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->host_mbox_out, msg.u32);
}

static inline void fapToHost_recv(uint32 fapIdx, fapMailBox_msg_t *msg)
{
    msg->u32 = FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->host_mbox_in);
}

#define FAP_MAILBOX_PRINT_Q_SIZE        32768 /* bytes */

void fapPrint4keDoTasklet(unsigned long unused);
DECLARE_TASKLET(fapPrint4keTasklet, fapPrint4keDoTasklet, 0);

char gFapPrintQ[FAP_MAILBOX_PRINT_Q_SIZE];
char gFapPrintBufferBh[FAP_MAILBOX_PRINTBUF_SIZE];
char * gFapPrintQHead;
char * gFapPrintQTail;
uint16 gHostCount[NUM_FAPS];

void fapPrint4keTaskletInit( void )
{
    gFapPrintQTail = gFapPrintQ;
    gFapPrintQHead = gFapPrintQ;
    *(uint16 *)gFapPrintQ = 0xFFFF;    
}



static void print4ke(uint32 fapIdx, fapMailBox_msg_t msg)
{    
    uint32 spaceToEoq;
    uint16 strLen;
    uint16 spaceLeft;  //total space left in Q
    char * eoq;

    /* invalidate print buffer in the D$ */
    fap_cacheInvFlush(gFap[fapIdx].pPrintBuffer,
                      &gFap[fapIdx].pPrintBuffer[FAP_MAILBOX_PRINTBUF_SIZE-1],
                      0);

    eoq = gFapPrintQ + FAP_MAILBOX_PRINT_Q_SIZE;
    //strLen = (uint16)((strlen(gFap[fapIdx].pPrintBuffer) + 1 + 1) & ~0x1);  // TBD: is there a faster way of getting strlen?
    strLen = (uint16)((msg.data + 1 + 1) & ~0x1);
    spaceToEoq = eoq - gFapPrintQHead;    

    if (gFapPrintQTail > gFapPrintQHead) 
        spaceLeft = gFapPrintQTail - gFapPrintQHead;
    else
        spaceLeft = gFapPrintQTail - gFapPrintQ + spaceToEoq;
    
    if (spaceLeft < strLen + 8 + 2)
    {
        printk("FAP%d print buffer overflow! [%s]\n", fapIdx, gFap[fapIdx].pPrintBuffer);
        // note: cannot call fapPrint4keDoTasklet(fapIdx) directly due to race condition
        // reschedule the tasklet: shouldn't need to do this, but it doesn't hurt:
        tasklet_schedule(&fapPrint4keTasklet);
        return;
    }     
    

    BCM_ASSERT(*((uint16 *)gFapPrintQHead) == 0xFFFF);
    *(uint16 *)gFapPrintQHead = msg.msgId;  // non-concurent -- we can write this now
    gFapPrintQHead = (gFapPrintQHead < eoq - 2) ? gFapPrintQHead + 2 : gFapPrintQ;

    *(uint16 *)gFapPrintQHead = strLen;
    gFapPrintQHead = (gFapPrintQHead < eoq - 2) ? gFapPrintQHead + 2 : gFapPrintQ;
    
    *(uint16 *)gFapPrintQHead = msg.data;
    gFapPrintQHead = (gFapPrintQHead < eoq - 2) ? gFapPrintQHead + 2 : gFapPrintQ;

    *(uint16 *)gFapPrintQHead = fapIdx;
    gFapPrintQHead = (gFapPrintQHead < eoq - 2) ? gFapPrintQHead + 2 : gFapPrintQ;
    
    spaceToEoq = eoq - gFapPrintQHead;


    if (spaceToEoq < strLen)
    {
        uint16 leftover = strLen - spaceToEoq;
        memcpy(gFapPrintQHead, gFap[fapIdx].pPrintBuffer, spaceToEoq);
        memcpy(gFapPrintQ, gFap[fapIdx].pPrintBuffer + spaceToEoq, leftover);
        gFapPrintQHead = gFapPrintQ + leftover;
    }
    else
    {
        memcpy(gFapPrintQHead, gFap[fapIdx].pPrintBuffer, strLen);
        gFapPrintQHead += strLen;
    }

    if (gFapPrintQHead >= gFapPrintQ + FAP_MAILBOX_PRINT_Q_SIZE)
        gFapPrintQHead = gFapPrintQ;

    *(uint16 *)gFapPrintQHead = 0xFFFF;
    
    tasklet_schedule(&fapPrint4keTasklet);
}

void fapPrint4keDoTasklet(unsigned long unused)
{

    /* get print buffer: */
    
    fapMailBox_msgId_t msgType;
    uint16             strLen;
    uint16             msgData;
    uint16             spaceToEoq;
    char *             pMsg;
    char *             pNewTail;
    char *             eoq;
    uint16             overlap;
    uint32             fapIdx;

    eoq = gFapPrintQ + FAP_MAILBOX_PRINT_Q_SIZE;
    while( *((uint16 *)(gFapPrintQTail)) != 0xFFFF)
    {
        BCM_ASSERT(gFapPrintQTail != eoq);
            
        msgType = *((uint16 *)(gFapPrintQTail));
        gFapPrintQTail = (gFapPrintQTail < eoq - 2) ? gFapPrintQTail + 2 : gFapPrintQ;

        strLen = *((uint16 *)(gFapPrintQTail));
        gFapPrintQTail = (gFapPrintQTail < eoq - 2) ? gFapPrintQTail + 2 : gFapPrintQ;        
        
        msgData = *((uint16 *)(gFapPrintQTail));
        gFapPrintQTail = (gFapPrintQTail < eoq - 2) ? gFapPrintQTail + 2 : gFapPrintQ; 
        
        fapIdx = *((uint16 *)(gFapPrintQTail));
        gFapPrintQTail = (gFapPrintQTail < eoq - 2) ? gFapPrintQTail + 2 : gFapPrintQ; 
        
        spaceToEoq = eoq - gFapPrintQTail;
        
        BCM_ASSERT(msgType >= FAP_MAILBOX_MSGID_PRINT && msgType <= FAP_MAILBOX_MSGID_LOG_DEBUG);
        BCM_ASSERT(((strLen & 0x1) == 0) && strLen < FAP_MAILBOX_PRINTBUF_SIZE);
        
        if ( gFapPrintQTail + strLen >= eoq )
        {
            overlap = strLen - spaceToEoq;
            memcpy( gFapPrintBufferBh, gFapPrintQTail, spaceToEoq);
            memcpy( gFapPrintBufferBh + spaceToEoq, gFapPrintQ, overlap);
            pMsg = gFapPrintBufferBh;
            pNewTail = gFapPrintQ + overlap;
        }
        else
        {
            pMsg = gFapPrintQTail;
            pNewTail = gFapPrintQTail + strLen;
        }

        gHostCount[fapIdx]++;

        pMsg[strLen-1] = '\0';
        
        switch(msgType)
        {
            case FAP_MAILBOX_MSGID_PRINT:
                printk(FAP_IDX_FMT "%s", fapIdx, pMsg);
                break;
        
            case FAP_MAILBOX_MSGID_LOG_ASSERT:
                BCM_LOG_ERROR(BCM_LOG_ID_FAP4KE, "[%d.%05d] ASSERTION %s", fapIdx, gHostCount[fapIdx], pMsg);
                BCM_ASSERT(0);
                break;
        
            case FAP_MAILBOX_MSGID_LOG_ERROR:
                BCM_LOG_ERROR(BCM_LOG_ID_FAP4KE, "[%d.%05d] %s", fapIdx, gHostCount[fapIdx], pMsg);
                break;
        
            case FAP_MAILBOX_MSGID_LOG_NOTICE:
                BCM_LOG_NOTICE(BCM_LOG_ID_FAP4KE, "[%d.%05d] %s", fapIdx, gHostCount[fapIdx], pMsg);
                break;
        
            case FAP_MAILBOX_MSGID_LOG_INFO:
                BCM_LOG_INFO(BCM_LOG_ID_FAP4KE, "[%d.%05d] %s",fapIdx,  gHostCount[fapIdx], pMsg);
                break;
        
            case FAP_MAILBOX_MSGID_LOG_DEBUG:
                BCM_LOG_DEBUG(BCM_LOG_ID_FAP4KE, "[%d.%05d] %s", fapIdx, gHostCount[fapIdx], pMsg);
                break;
        
            default:
                BCM_LOG_ERROR(BCM_LOG_ID_FAP, "[%d] Invalid FAP LOG Level <%d>", fapIdx, msgType);
                break;
        }  
        gFapPrintQTail = pNewTail;
    }
}

void fapMailBox_irqHandler(uint32 fapIdx)
{
    fapMailBox_msg_t msg;
    static uint32 mailBoxWord = 0xDEADBEEF;
    int doPrint4ke = 1;

    /* clear mailbox interrupt before processing, to make sure we do not miss interrupts */
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->gp_status, IRQ_FAP_GP_MBOX_IN);

    fapToHost_recv(fapIdx, &msg);

    switch(msg.msgId)
    {
        case FAP_MAILBOX_MSGID_SUCCESS:
            BCM_LOG_DEBUG(BCM_LOG_ID_FAP, "FAP Success <%05d>", msg.data);
            break;

        case FAP_MAILBOX_MSGID_ERROR:
            BCM_LOG_NOTICE(BCM_LOG_ID_FAP4KE, "FAP Mailbox Error <%05d>", msg.data);
            break;
            
        case FAP_MAILBOX_MSGID_LOG_NOTICE:
        case FAP_MAILBOX_MSGID_LOG_INFO:
        case FAP_MAILBOX_MSGID_LOG_DEBUG:
        case FAP_MAILBOX_MSGID_PRINT:
        case FAP_MAILBOX_MSGID_LOG_ASSERT:
        case FAP_MAILBOX_MSGID_LOG_ERROR:
            switch (msg.msgId)
            {
                case FAP_MAILBOX_MSGID_LOG_ERROR:
                    doPrint4ke = bcmLog_logIsEnabled(BCM_LOG_ID_FAP4KE, BCM_LOG_LEVEL_ERROR) ? 1 : 0;
                    break;
                case FAP_MAILBOX_MSGID_LOG_NOTICE:
                    doPrint4ke = bcmLog_logIsEnabled(BCM_LOG_ID_FAP4KE, BCM_LOG_LEVEL_NOTICE) ? 1 : 0;
                    break;
                case FAP_MAILBOX_MSGID_LOG_INFO:
                    doPrint4ke = bcmLog_logIsEnabled(BCM_LOG_ID_FAP4KE, BCM_LOG_LEVEL_INFO) ? 1 : 0;
                    break;
                case FAP_MAILBOX_MSGID_LOG_DEBUG:
                    doPrint4ke = bcmLog_logIsEnabled(BCM_LOG_ID_FAP4KE, BCM_LOG_LEVEL_DEBUG) ? 1 : 0;
                    break;
            }
            if(doPrint4ke)
            {
                print4ke(fapIdx, msg);
            }
            msg.msgId = FAP_MAILBOX_MSGID_SUCCESS;
            msg.data = ++hostPrintCount;
            hostToFap_xmit(fapIdx, msg);
            break;

        case FAP_MAILBOX_MSGID_KEEPALIVE:
            if(msg.data == (uint16)(hostKeepAliveCount[fapIdx] & 0xFFFF))
            {
                BCM_LOG_DEBUG(BCM_LOG_ID_FAP, "Keep-Alive (%d): Host <%d>, 4ke <%d>",
                             fapIdx, hostKeepAliveCount[fapIdx], msg.data);
                fap_Updatejiffies(fapIdx);

                hostKeepAliveCount[fapIdx]++;
                msg.msgId = FAP_MAILBOX_MSGID_SUCCESS;
            }
            else
            {
                BCM_LOG_NOTICE(BCM_LOG_ID_FAP,
                               "Keep-Alive out of Sync: Fap <%d>, Host <%d>, 4ke <%d>",
                               fapIdx, hostKeepAliveCount[fapIdx], msg.data);

                hostKeepAliveCount[fapIdx] = msg.data + 1;

                msg.msgId = FAP_MAILBOX_MSGID_ERROR;
            }

            hostToFap_xmit(fapIdx, msg);

            break;

        case FAP_MAILBOX_MSGID_WORD_HI:
            mailBoxWord = (uint32)(msg.data << 16);
            msg.msgId = FAP_MAILBOX_MSGID_SUCCESS;
            hostToFap_xmit(fapIdx, msg);
//            BCM_LOG_NOTICE(BCM_LOG_ID_FAP, "Word <HIGH>");
            break;

        case FAP_MAILBOX_MSGID_WORD_LO:
            mailBoxWord |= (uint32)(msg.data);
            msg.msgId = FAP_MAILBOX_MSGID_SUCCESS;
            hostToFap_xmit(fapIdx, msg);
            if(mailBoxWord == 0xFFFFFFFF)
            {
                msg.msgId = FAP_MAILBOX_MSGID_LOG_NOTICE;
                msg.data = 0xFFFF;
                print4ke(fapIdx, msg);
            }
            else
            {
                BCM_LOG_NOTICE(BCM_LOG_ID_FAP, "Word <0x%08X>", mailBoxWord);
            }
            break;

        case 0x00C0:
            /* Test word written by 4ke initialization vector */
            BCM_LOG_DEBUG(BCM_LOG_ID_FAP, "0x%08X", msg.u32);
            break;

        default:
            /* FIXME: Add error counter here */
            BCM_LOG_ERROR(BCM_LOG_ID_FAP,
                          "Invalid FAP Mailbox message <0x%08X>", msg.u32);
            break;
    }

    fapIrq_enable(fapIdx, IRQ_FAP_HOST_MBOX_IN);
}

void fapMailBox_sysRq(uint32 fapIdx)
{
    fapMailBox_msg_t msg;

    msg.msgId = FAP_MAILBOX_MSGID_SYSRQ;

    hostToFap_xmit(fapIdx, msg);
}

void fapMailBox_hostInit(uint32 fapIdx)
{
    unsigned long flags;

    /* inititialize the print Q */   
    if (fapIdx == 0) 
       fapPrint4keTaskletInit();

    local_irq_save(flags);

    /* clear and unmask Mailbox interrupt */
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->gp_status, IRQ_FAP_GP_MBOX_IN);

    /* IMPORTANT: Both Host and GP Masks must be unmasked */
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->gp_mask,
                    (FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->gp_mask) | IRQ_FAP_GP_MBOX_IN));

    fapIrq_enable(fapIdx, IRQ_FAP_HOST_MBOX_IN);

    local_irq_restore(flags);

    hostKeepAliveCount[fapIdx] = 1;
}
