/* This file is an aid to support non-modular object accesses
 * while we do not have fully modularized everything. Once this is
 * done, this file can (and should) be deleted. Presence of it
 * also somewhat indicates that the runtime library is not really
 * yet a runtime library, because it depends on some functionality
 * residing somewhere else.
 *
 * Copyright 2007-2014 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef	DIRTY_H_INCLUDED
#define	DIRTY_H_INCLUDED 1

rsRetVal __attribute__((deprecated)) multiSubmitMsg(multi_submit_t *pMultiSub);
rsRetVal ATTR_NONNULL() multiSubmitMsg2(multi_submit_t *const pMultiSub); /* friends only! */
rsRetVal submitMsg2(smsg_t *pMsg);
rsRetVal __attribute__((deprecated)) submitMsg(smsg_t *pMsg);
rsRetVal multiSubmitFlush(multi_submit_t *pMultiSub);
rsRetVal logmsgInternal(const int iErr, const syslog_pri_t pri, const uchar *const msg, int flags);
rsRetVal __attribute__((deprecated)) parseAndSubmitMessage(const uchar *hname,
	const uchar *hnameIP, const uchar *msg, const int len,
	const int flags, const flowControl_t flowCtlType,
	prop_t *pInputName, const struct syslogTime *stTime,
	const time_t ttGenTime, ruleset_t *pRuleset);
rsRetVal createMainQueue(qqueue_t **ppQueue, uchar *pszQueueName, struct nvlst *lst);
rsRetVal startMainQueue(qqueue_t *pQueue);

extern int MarkInterval;
extern qqueue_t *pMsgQueue;			/* the main message queue */
#define CONF_VERIFY_PARTIAL_CONF 0x02		/* bit: partial configuration to be checked */
extern int iConfigVerify;			/* is this just a config verify run? */
extern int bHaveMainQueue;

#endif /* #ifndef DIRTY_H_INCLUDED */
