#ifndef __FAP4KE_SWQ_H_INCLUDED__
#define __FAP4KE_SWQ_H_INCLUDED__

/*
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
/* SW Queue Data */


#define SWQ_HOST2FAP_GSO_LOOPBACK_BUDGET 32

#define SWQ_HOST2FAP_GSO_LOOPBACK_Q  0
#define SWQ_HOST2FAP_GSO_LOOPBACK_Q_MSG_SIZE 4   /*words*/
#define SWQ_HOST2FAP_GSO_LOOPBACK_Q_DEPTH 1024

#define SWQ_HOST2FAP_GSO_LOOPBACK_Q_MEM_SIZE  \
        (SWQ_HOST2FAP_GSO_LOOPBACK_Q_MSG_SIZE * SWQ_HOST2FAP_GSO_LOOPBACK_Q_DEPTH)
        

#define SWQ_FAP2HOST_GSO_LOOPBACK_Q  1
#define SWQ_FAP2HOST_GSO_LOOPBACK_Q_MSG_SIZE 2   /*words*/
#define SWQ_FAP2HOST_GSO_LOOPBACK_Q_DEPTH 2048

#define SWQ_FAP2HOST_GSO_LOOPBACK_Q_MEM_SIZE   \
        (SWQ_FAP2HOST_GSO_LOOPBACK_Q_MSG_SIZE * SWQ_FAP2HOST_GSO_LOOPBACK_Q_DEPTH)

#define SWQ_FAP2HOST_WFD_BASE_Q 2
#define SWQ_FAP2HOST_WFD_Q_MSG_SIZE 3   /*words*/
#define SWQ_FAP2HOST_WFD_Q_DEPTH 600    /* This number has been chosen to be same as 
                                           number of enet bds as the WLAN packets were previously 
                                           sent via Ethernet driver. This value can be changed 
                                           if the Q depth turns out low */
#define SWQ_FAP2HOST_WFD_Q_MEM_SIZE   \
        (SWQ_FAP2HOST_WFD_Q_MSG_SIZE * SWQ_FAP2HOST_WFD_Q_DEPTH)

#define SWQ_FAP2HOST_WFD_Q0          SWQ_FAP2HOST_WFD_BASE_Q
#define SWQ_FAP2HOST_WFD_Q0_MSG_SIZE SWQ_FAP2HOST_WFD_Q_MSG_SIZE
#define SWQ_FAP2HOST_WFD_Q0_DEPTH    SWQ_FAP2HOST_WFD_Q_DEPTH

#define SWQ_FAP2HOST_WFD_Q1          (SWQ_FAP2HOST_WFD_BASE_Q + 1)
#define SWQ_FAP2HOST_WFD_Q1_MSG_SIZE SWQ_FAP2HOST_WFD_Q_MSG_SIZE
#define SWQ_FAP2HOST_WFD_Q1_DEPTH    SWQ_FAP2HOST_WFD_Q_DEPTH

#define SWQ_MAX  4

#if 0
typedef struct {
  uint32 gsoLoopBackH2FSwqMem[SWQ_HOST2FAP_GSO_LOOPBACK_Q_MEM_SIZE];
  uint32 gsoLoopBackF2HSwqMem[SWQ_FAP2HOST_GSO_LOOPBACK_Q_MEM_SIZE];
} fap4ke_Sdram_Swq_t;
#endif

typedef struct SWQDataMsg_t
{
    uint32    data[4];   
} SWQDataMsg_t;

#define FAP4KE_MAX_IC_ENABLED_SWQS 4
#define FAP4KE_WFD_IC_MAX_PKT_CNT 32
#define FAP4KE_WFD_IC_MAX_TIMEOUT_IN_US 400
typedef struct fap4ke_SWQueue_t{
    struct{
        uint8  dqm:6;         /* assoicated DQM for interrupt mgmt */
        uint8  dqmDir:1;
        uint8  fapId:1;
    };
    uint8  msgSize;    /* size of each message */
    uint16 maxDepth;

    uint16 rsvd;
    uint16 count;    /* number of messages in Queue */

    uint32 *qStart;   /* pointer to the start of the Queue in DDR */
    uint32 *qEnd;     /* pointer to the end of the Queue in DDR */

    volatile uint32 *rdPtr;   /* current read pointer */
    volatile uint32 *wrPtr;   /* current write pointer */

    uint32 dropped;   /* number of message dropped due to queue full */
    uint32 processed; /* total number of messages passed */

	 struct{
		 uint8 ic_enable:1; /* 1 - Enable. 0 - Disable */
		 uint8 ic_timerActive:1;
#define FAP4KE_IC_MAX_PKT_CNT       63
		 uint8 ic_max_pktcnt:6;
	 };
#define FAP4KE_IC_MAX_TIMEOUT_IN_US 1023
#define FAP4KE_IC_TIMEOUT_RES_IN_US 200
    uint16 ic_timeout_us;    /* The hardware timer period is configured to 
                                200 us. This means that the timeout period
                                will round up to the next 200 us.  
                                For example, if the timeout is 201 us, 
                                a timeout will not be identified until 400us.
                                This means for practical purposes, the 
                                timeout value is 200 us, 400 us, 600 us, 
                                800 us and 1000 us.  
                             */
	 uint8 rsvd2;
	 uint32 ic_expiry_jiffies;
	 uint32 interrupts;
} fap4ke_SWQueue_t;


#ifdef FAP_4KE
extern int swqXmitAvailable4ke(fap4ke_SWQueue_t *swq);
extern void swqXmitMsg4ke(fap4ke_SWQueue_t *swq, SWQDataMsg_t *msg);
extern int swqRecvAvailable4ke(fap4ke_SWQueue_t *swq);
extern void swqRecvMsg4ke(fap4ke_SWQueue_t *swq, SWQDataMsg_t *msg);
extern void swqInit4ke(fap4ke_SWQueue_t *swq, uint32 *qmem, uint32 msgSize,
								uint32 maxDepth, uint32 dqm, uint8 dqmDir,
								uint8 ic_enable, uint8 ic_max_pktcnt, uint16 ic_timeout_us);
extern void swqDump4ke(fap4ke_SWQueue_t *swq);
extern fapRet swqTimerHandler(uint32 swq);
#endif  

static inline int swqGetCurrQDepth(fap4ke_SWQueue_t *swq)
{
	if (swq->wrPtr >= swq->rdPtr)
	{
		return (swq->wrPtr - swq->rdPtr) / swq->msgSize;
	}
	else
	{
		return ((swq->qEnd - swq->rdPtr) + (swq->wrPtr - swq->qStart)) / swq->msgSize;
	}
}

#endif  /* defined(__FAP4KE_SWQ_H_INCLUDED__) */
