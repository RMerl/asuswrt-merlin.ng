/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2003 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __UDNS_H_
#define __UDNS_H_

#include "mDNSEmbeddedAPI.h"
#include "DNSCommon.h"

#ifdef	__cplusplus
	extern "C" {
#endif

#define RESTART_GOODBYE_DELAY    (6 * mDNSPlatformOneSecond) // delay after restarting LLQ before nuking previous known answers (avoids flutter if we restart before we have networking up)
#define INIT_UCAST_POLL_INTERVAL (3 * mDNSPlatformOneSecond) // this interval is used after send failures on network transitions
	                                                         // which typically heal quickly, so we start agressively and exponentially back off
#define MAX_UCAST_POLL_INTERVAL (60 * 60 * mDNSPlatformOneSecond)
//#define MAX_UCAST_POLL_INTERVAL (1 * 60 * mDNSPlatformOneSecond)
#define LLQ_POLL_INTERVAL       (15 * 60 * mDNSPlatformOneSecond) // Polling interval for zones w/ an advertised LLQ port (ie not static zones) if LLQ fails due to NAT, etc.
#define RESPONSE_WINDOW (60 * mDNSPlatformOneSecond)         // require server responses within one minute of request
#define MAX_UCAST_UNANSWERED_QUERIES 2                       // the number of unanswered queries from any one uDNS server before trying another server
#define DNSSERVER_PENALTY_TIME (60 * mDNSPlatformOneSecond) // number of seconds for which new questions don't pick this server

#define DEFAULT_UPDATE_LEASE 7200

#define QuestionIntervalStep 3
#define QuestionIntervalStep2 (QuestionIntervalStep*QuestionIntervalStep)
#define QuestionIntervalStep3 (QuestionIntervalStep*QuestionIntervalStep*QuestionIntervalStep)
#define InitialQuestionInterval ((mDNSPlatformOneSecond + QuestionIntervalStep-1) / QuestionIntervalStep)

// For Unicast record registrations, we initialize the interval to 1 second. When we send any query for
// the record registration e.g., GetZoneData, we always back off by QuestionIntervalStep
// so that the first retry does not happen until 3 seconds which should be enough for TCP/TLS to be done.
#define INIT_RECORD_REG_INTERVAL (1 * mDNSPlatformOneSecond)
#define MAX_RECORD_REG_INTERVAL	(15 * 60 * mDNSPlatformOneSecond)
#define MERGE_DELAY_TIME	(1 * mDNSPlatformOneSecond)

// If we are refreshing, we do it at least 5 times with a min update frequency of 
// 5 minutes
#define MAX_UPDATE_REFRESH_COUNT	5
#define MIN_UPDATE_REFRESH_TIME		(5 * 60 * mDNSPlatformOneSecond)

// For questions that use kDNSServiceFlagsTimeout and we don't have a matching resolver e.g., no dns servers,
// then use the default value of 30 seconds
#define DEFAULT_UDNS_TIMEOUT	30 // in seconds

// Entry points into unicast-specific routines

extern void LLQGotZoneData(mDNS *const m, mStatus err, const ZoneData *zoneInfo);
extern void startLLQHandshake(mDNS *m, DNSQuestion *q);
extern void sendLLQRefresh(mDNS *m, DNSQuestion *q);

extern void SleepRecordRegistrations(mDNS *m);

// uDNS_UpdateRecord
// following fields must be set, and the update validated, upon entry.
// rr->NewRData
// rr->newrdlength
// rr->UpdateCallback

extern mStatus uDNS_UpdateRecord(mDNS *m, AuthRecord *rr);

extern void SetNextQueryTime(mDNS *const m, const DNSQuestion *const q);
extern CacheGroup *CacheGroupForName(const mDNS *const m, const mDNSu32 slot, const mDNSu32 namehash, const domainname *const name);
extern mStatus mDNS_Register_internal(mDNS *const m, AuthRecord *const rr);
extern mStatus mDNS_Deregister_internal(mDNS *const m, AuthRecord *const rr, mDNS_Dereg_type drt);
extern mStatus mDNS_StartQuery_internal(mDNS *const m, DNSQuestion *const question);
extern mStatus mDNS_StopQuery_internal(mDNS *const m, DNSQuestion *const question);
extern mStatus mDNS_StartNATOperation_internal(mDNS *const m, NATTraversalInfo *traversal);

extern void RecordRegistrationGotZoneData(mDNS *const m, mStatus err, const ZoneData *zoneData);
extern mStatus uDNS_DeregisterRecord(mDNS *const m, AuthRecord *const rr);
extern const domainname *GetServiceTarget(mDNS *m, AuthRecord *const rr);
extern void uDNS_CheckCurrentQuestion(mDNS *const m);

// integer fields of msg header must be in HOST byte order before calling this routine
extern void uDNS_ReceiveMsg(mDNS *const m, DNSMessage *const msg, const mDNSu8 *const end,
	const mDNSAddr *const srcaddr, const mDNSIPPort srcport);

extern void uDNS_Tasks(mDNS *const m);
extern void UpdateAllSRVRecords(mDNS *m);
extern void CheckNATMappings(mDNS *m);

extern mStatus         uDNS_SetupDNSConfig(mDNS *const m);

// uDNS_SetupSearchDomains by default adds search domains. It also can be called with one or
// more values for "action" which does the following:
//
// -UDNS_START_WAB_QUERY - start Wide Area Bonjour (domain enumeration) queries

#define UDNS_START_WAB_QUERY    0x00000001

extern mStatus         uDNS_SetupSearchDomains(mDNS *const m, int action);
extern domainname      *uDNS_GetNextSearchDomain(mDNS *const m, mDNSInterfaceID InterfaceID, mDNSs8 *searchIndex, mDNSBool ignoreDotLocal);

typedef enum
	{
	uDNS_LLQ_Not = 0,	// Normal uDNS answer: Flush any stale records from cache, and respect record TTL
	uDNS_LLQ_Ignore,	// LLQ initial challenge packet: ignore -- has no useful records for us
	uDNS_LLQ_Entire,	// LLQ initial set of answers: Flush any stale records from cache, but assume TTL is 2 x LLQ refresh interval
	uDNS_LLQ_Events		// LLQ event packet: don't flush cache; assume TTL is 2 x LLQ refresh interval
	} uDNS_LLQType;

extern uDNS_LLQType    uDNS_recvLLQResponse(mDNS *const m, const DNSMessage *const msg, const mDNSu8 *const end, const mDNSAddr *const srcaddr, const mDNSIPPort srcport, DNSQuestion **matchQuestion);
extern DomainAuthInfo *GetAuthInfoForName_internal(mDNS *m, const domainname *const name);
extern DomainAuthInfo *GetAuthInfoForQuestion(mDNS *m, const DNSQuestion *const q);
extern void DisposeTCPConn(struct tcpInfo_t *tcp);

// NAT traversal
extern void	uDNS_ReceiveNATPMPPacket(mDNS *m, const mDNSInterfaceID InterfaceID, mDNSu8 *pkt, mDNSu16 len);	// Called for each received NAT-PMP packet
extern void	natTraversalHandleAddressReply(mDNS *const m, mDNSu16 err, mDNSv4Addr ExtAddr);
extern void	natTraversalHandlePortMapReply(mDNS *const m, NATTraversalInfo *n, const mDNSInterfaceID InterfaceID, mDNSu16 err, mDNSIPPort extport, mDNSu32 lease);

#ifdef	__cplusplus
	}
#endif

#endif // __UDNS_H_
