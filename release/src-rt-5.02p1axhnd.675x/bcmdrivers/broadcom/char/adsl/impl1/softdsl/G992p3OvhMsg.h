/* 
* <:copyright-BRCM:2002:proprietary:standard
* 
*    Copyright (c) 2002 Broadcom 
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

/****************************************************************************
 *
 * G992p3OvhMsg.h 
 *
 * Description:
 *	This file contains the exported functions and definitions for G992p3 
 *  overhead channel messages
 *
 *
 * Copyright (c) 1999-2003 BroadCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: G992p3OvhMsg.h,v 1.1 2003/07/18 19:39:18 ilyas Exp $
 *
 * $Log: G992p3OvhMsg.h,v $
 * Revision 1.1  2003/07/18 19:39:18  ilyas
 * Initial G.992.3 overhead channel message implementation (from ADSL driver)
 *
 *
 *****************************************************************************/

#ifndef	G992p3OvhMsgFramerHeader
#define	G992p3OvhMsgFramerHeader

#define	kG992p3OvhMsgFrameBufCnt	-1

extern Boolean  G992p3OvhMsgInit(
		void					*gDslVars, 
		bitMap					setup,
		dslFrameHandlerType		rxReturnFramePtr,
		dslFrameHandlerType		txSendFramePtr,
		dslFrameHandlerType		txSendCompletePtr,
		dslCommandHandlerType	commandHandler,
		dslStatusHandlerType	statusHandler);

extern void		G992p3OvhMsgReset(void *gDslVars);
extern void		G992p3OvhMsgClose(void *gDslVars);
extern void		G992p3OvhMsgUpdateTimer(void *gDslVars, uint timeMs);
extern void		G992p3OvhMsgTimer(void *gDslVars);
extern Boolean	G992p3OvhMsgCommandHandler (void *gDslVars, dslCommandStruct *cmd);
extern void		G992p3OvhMsgStatusSnooper (void *gDslVars, dslStatusStruct *status);

extern	int		G992p3OvhMsgSendCompleteFrame(void *gDslVars, void *pVc, ulong mid, dslFrame *pFrame);
extern  int		G992p3OvhMsgIndicateRcvFrame(void *gDslVars, void *pVc, ulong mid, dslFrame *pFrame);
extern  void	G992p3OvhMsgReturnFrame(void *gDslVars, void *pVc, ulong mid, dslFrame *pFrame);

extern void		G992p3OvhMsgSetL3(void *gDslVars);
extern void		G992p3OvhMsgSetL0(void *gDslVars);
extern Boolean G992p3OvhMsgIsL3RspPending(void *gDslVars);

extern Boolean	G992p3OvhMsgSendClearEocFrame(void *gDslVars, dslFrame *pFrame);
extern Boolean G992p3OvhMsgSendNonStdFacFrame(void *gDslVars, dslFrame *pFrame);
extern Boolean G992p3OvhMsgSendDatagramEocFrame(void *gDslVars, dslFrame *pFrame);
extern int G992p3OvhMsgSendHmiEocFrame(void *gDslVars, dslFrame *pFrame);

extern void G992p3OvhMsgSetRateChangeFlag(void *gDslVars);
#endif	/* G992p3OvhMsgFramerHeader */
