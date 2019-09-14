/* net.c
 * Implementation of network-related stuff.
 *
 * File begun on 2007-07-20 by RGerhards (extracted from syslogd.c)
 * This file is under development and has not yet arrived at being fully
 * self-contained and a real object. So far, it is mostly an excerpt
 * of the "old" networking code without any modifications. However, it
 * helps to have things at the right place one we go to the meat of it.
 *
 * Starting 2007-12-24, I have begun to shuffle more network-related code
 * from syslogd.c to over here. I am not sure if it will stay here in the
 * long term, but it is good to have it out of syslogd.c. Maybe this here is
 * an interim location ;)
 *
 * Copyright 2007-2018 Rainer Gerhards and Adiscon GmbH.
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
#include "config.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <netdb.h>
#include <fnmatch.h>
#include <fcntl.h>
#include <unistd.h>
#if HAVE_GETIFADDRS
#include <ifaddrs.h>
#else
#include "compat/ifaddrs.h"
#endif /* HAVE_GETIFADDRS */
#include <sys/types.h>
#include <arpa/inet.h>

#include "rsyslog.h"
#include "syslogd-types.h"
#include "module-template.h"
#include "parse.h"
#include "srUtils.h"
#include "obj.h"
#include "errmsg.h"
#include "net.h"
#include "dnscache.h"
#include "prop.h"

#ifdef OS_SOLARIS
#include <arpa/nameser_compat.h>
#	define	s6_addr32	_S6_un._S6_u32
	typedef unsigned int	u_int32_t;
#endif

MODULE_TYPE_LIB
MODULE_TYPE_NOKEEP

/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)
DEFobjCurrIf(prop)

#ifndef HAVE_STRUCT_SOCKADDR_SA_LEN
extern size_t SALEN(struct sockaddr *sa);
#endif
/* support for defining allowed TCP and UDP senders. We use the same
 * structure to implement this (a linked list), but we define two different
 * list roots, one for UDP and one for TCP.
 * rgerhards, 2005-09-26
 */
/* All of the five below are read-only after startup */
struct AllowedSenders *pAllowedSenders_UDP = NULL; /* the roots of the allowed sender */
struct AllowedSenders *pAllowedSenders_TCP = NULL; /* lists. If NULL, all senders are ok! */
static struct AllowedSenders *pLastAllowedSenders_UDP = NULL; /* and now the pointers to the last */
static struct AllowedSenders *pLastAllowedSenders_TCP = NULL; /* element in the respective list */
#ifdef USE_GSSAPI
struct AllowedSenders *pAllowedSenders_GSS = NULL;
static struct AllowedSenders *pLastAllowedSenders_GSS = NULL;
#endif

int     ACLAddHostnameOnFail = 0; /* add hostname to acl when DNS resolving has failed */
int     ACLDontResolve = 0;       /* add hostname to acl instead of resolving it to IP(s) */


/* ------------------------------ begin permitted peers code ------------------------------ */


/* sets the correct allow root pointer based on provided type
 * rgerhards, 2008-12-01
 */
static rsRetVal
setAllowRoot(struct AllowedSenders **ppAllowRoot, uchar *pszType)
{
	DEFiRet;

	if(!strcmp((char*)pszType, "UDP"))
		*ppAllowRoot = pAllowedSenders_UDP;
	else if(!strcmp((char*)pszType, "TCP"))
		*ppAllowRoot = pAllowedSenders_TCP;
#ifdef USE_GSSAPI
	else if(!strcmp((char*)pszType, "GSS"))
		*ppAllowRoot = pAllowedSenders_GSS;
#endif
	else {
		dbgprintf("program error: invalid allowed sender ID '%s', denying...\n", pszType);
		ABORT_FINALIZE(RS_RET_CODE_ERR); /* everything is invalid for an invalid type */
	}

finalize_it:
	RETiRet;
}
/* re-initializes (sets to NULL) the correct allow root pointer
 * rgerhards, 2009-01-12
 */
static rsRetVal
reinitAllowRoot(uchar *pszType)
{
	DEFiRet;

	if(!strcmp((char*)pszType, "UDP"))
		pAllowedSenders_UDP = NULL;
	else if(!strcmp((char*)pszType, "TCP"))
		pAllowedSenders_TCP = NULL;
#ifdef USE_GSSAPI
	else if(!strcmp((char*)pszType, "GSS"))
		pAllowedSenders_GSS = NULL;
#endif
	else {
		dbgprintf("program error: invalid allowed sender ID '%s', denying...\n", pszType);
		ABORT_FINALIZE(RS_RET_CODE_ERR); /* everything is invalid for an invalid type */
	}

finalize_it:
	RETiRet;
}


/* add a wildcard entry to this permitted peer. Entries are always
 * added at the tail of the list. pszStr and lenStr identify the wildcard
 * entry to be added. Note that the string is NOT \0 terminated, so
 * we must rely on lenStr for when it is finished.
 * rgerhards, 2008-05-27
 */
static rsRetVal
AddPermittedPeerWildcard(permittedPeers_t *pPeer, uchar* pszStr, size_t lenStr)
{
	permittedPeerWildcard_t *pNew = NULL;
	size_t iSrc;
	size_t iDst;
	DEFiRet;

	assert(pPeer != NULL);
	assert(pszStr != NULL);

	CHKmalloc(pNew = calloc(1, sizeof(*pNew)));

	if(lenStr == 0) { /* empty domain components are permitted */
		pNew->wildcardType = PEER_WILDCARD_EMPTY_COMPONENT;
		FINALIZE;
	} else {
		/* alloc memory for the domain component. We may waste a byte or
		 * two, but that's ok.
		 */
		CHKmalloc(pNew->pszDomainPart = MALLOC(lenStr +1 ));
	}

	if(pszStr[0] == '*') {
		pNew->wildcardType = PEER_WILDCARD_AT_START;
		iSrc = 1; /* skip '*' */
	} else {
		iSrc = 0;
	}

	for(iDst = 0 ; iSrc < lenStr && pszStr[iSrc] != '*' ; ++iSrc, ++iDst)  {
		pNew->pszDomainPart[iDst] = pszStr[iSrc];
	}

	if(iSrc < lenStr) {
		if(iSrc + 1 == lenStr && pszStr[iSrc] == '*') {
			if(pNew->wildcardType == PEER_WILDCARD_AT_START) {
				ABORT_FINALIZE(RS_RET_INVALID_WILDCARD);
			} else {
				pNew->wildcardType = PEER_WILDCARD_AT_END;
			}
		} else {
			/* we have an invalid wildcard, something follows the asterisk! */
			ABORT_FINALIZE(RS_RET_INVALID_WILDCARD);
		}
	}

	if(lenStr == 1 && pNew->wildcardType == PEER_WILDCARD_AT_START) {
		pNew->wildcardType = PEER_WILDCARD_MATCH_ALL;
	}

	/* if we reach this point, we had a valid wildcard. We now need to
	 * properly terminate the domain component string.
	 */
	pNew->pszDomainPart[iDst] = '\0';
	pNew->lenDomainPart = strlen((char*)pNew->pszDomainPart);

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pNew != NULL) {
			if(pNew->pszDomainPart != NULL)
				free(pNew->pszDomainPart);
			free(pNew);
		}
	} else {
		/* enqueue the element */
		if(pPeer->pWildcardRoot == NULL) {
			pPeer->pWildcardRoot = pNew;
			pPeer->pWildcardLast = pNew;
		} else {
			pPeer->pWildcardLast->pNext = pNew;
		}
		pPeer->pWildcardLast = pNew;
	}

	RETiRet;
}


/* Destruct a permitted peer's wildcard list -- rgerhards, 2008-05-27 */
static rsRetVal
DestructPermittedPeerWildcards(permittedPeers_t *pPeer)
{
	permittedPeerWildcard_t *pCurr;
	permittedPeerWildcard_t *pDel;
	DEFiRet;

	assert(pPeer != NULL);

	for(pCurr = pPeer->pWildcardRoot ; pCurr != NULL ; /*EMPTY*/) {
		pDel = pCurr;
		pCurr = pCurr->pNext;
		free(pDel->pszDomainPart);
		free(pDel);
	}

	pPeer->pWildcardRoot = NULL;
	pPeer->pWildcardLast = NULL;

	RETiRet;
}


/* add a permitted peer. PermittedPeers is an interim solution until we can provide
 * access control via enhanced RainerScript methods.
 * Note: the provided string is handed over to this function, caller must
 * no longer access it. -- rgerhards, 2008-05-19
 */
static rsRetVal
AddPermittedPeer(permittedPeers_t **ppRootPeer, uchar* pszID)
{
	permittedPeers_t *pNew = NULL;
	DEFiRet;

	assert(ppRootPeer != NULL);
	assert(pszID != NULL);

	CHKmalloc(pNew = calloc(1, sizeof(permittedPeers_t))); /* we use calloc() for consistency with "real" objects */
	CHKmalloc(pNew->pszID = (uchar*)strdup((char*)pszID));

	if(*ppRootPeer != NULL) {
		pNew->pNext = *ppRootPeer;
	}
	*ppRootPeer = pNew;

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pNew != NULL)
			free(pNew);
	}
	RETiRet;
}


/* Destruct a permitted peers list -- rgerhards, 2008-05-19 */
static rsRetVal
DestructPermittedPeers(permittedPeers_t **ppRootPeer)
{
	permittedPeers_t *pCurr;
	permittedPeers_t *pDel;
	DEFiRet;

	assert(ppRootPeer != NULL);

	for(pCurr = *ppRootPeer ; pCurr != NULL ; /*EMPTY*/) {
		pDel = pCurr;
		pCurr = pCurr->pNext;
		DestructPermittedPeerWildcards(pDel);
		free(pDel->pszID);
		free(pDel);
	}

	*ppRootPeer = NULL;

	RETiRet;
}


/* Compile a wildcard. The function first checks if there is a wildcard
 * present and compiles it only if so ;) It sets the etryType status
 * accordingly.
 * rgerhards, 2008-05-27
 */
static rsRetVal
PermittedPeerWildcardCompile(permittedPeers_t *pPeer)
{
	uchar *pC;
	uchar *pStart;
	DEFiRet;

	assert(pPeer != NULL);
	assert(pPeer->pszID != NULL);

	/* first check if we have a wildcard */
	for(pC = pPeer->pszID ; *pC != '\0' && *pC != '*' ; ++pC)
		/*EMPTY, just skip*/;

	if(*pC == '\0') {
		/* no wildcard found, we are mostly done */
		pPeer->etryType = PERM_PEER_TYPE_PLAIN;
		FINALIZE;
	}

	/* if we reach this point, the string contains wildcards. So let's
	 * compile the structure. To do so, we must parse from dot to dot
	 * and create a wildcard entry for each domain component we find.
	 * We must also flag problems if we have an asterisk in the middle
	 * of the text (it is supported at the start or end only).
	 */
	pPeer->etryType = PERM_PEER_TYPE_WILDCARD;
	pC = pPeer->pszID;
	while(*pC != '\0') {
		pStart = pC;
		/* find end of domain component */
		for( ; *pC != '\0' && *pC != '.' ; ++pC)
			/*EMPTY, just skip*/;
		CHKiRet(AddPermittedPeerWildcard(pPeer, pStart, pC - pStart));
		/* now check if we have an empty component at end of string */
		if(*pC == '.' && *(pC + 1) == '\0') {
			/* pStart is a dummy, it is not used if length is 0 */
			CHKiRet(AddPermittedPeerWildcard(pPeer, pStart, 0));
		}
		if(*pC != '\0')
			++pC;
	}

finalize_it:
	if(iRet != RS_RET_OK) {
		LogError(0, iRet, "error compiling wildcard expression '%s'",
			 pPeer->pszID);
	}
	RETiRet;
}


/* Do a (potential) wildcard match. The function first checks if the wildcard
 * has already been compiled and, if not, compiles it. If the peer entry in
 * question does NOT contain a wildcard, a simple strcmp() is done.
 * *pbIsMatching is set to 0 if there is no match and something else otherwise.
 * rgerhards, 2008-05-27 */
static rsRetVal
PermittedPeerWildcardMatch(permittedPeers_t *const pPeer,
	const uchar *pszNameToMatch,
	int *const pbIsMatching)
{
	const permittedPeerWildcard_t *pWildcard;
	const uchar *pC;
	size_t iWildcard, iName; /* work indexes for backward comparisons */
	DEFiRet;

	assert(pPeer != NULL);
	assert(pszNameToMatch != NULL);
	assert(pbIsMatching != NULL);

	if(pPeer->etryType == PERM_PEER_TYPE_UNDECIDED) {
		PermittedPeerWildcardCompile(pPeer);
	}

	if(pPeer->etryType == PERM_PEER_TYPE_PLAIN) {
		*pbIsMatching = !strcmp((char*)pPeer->pszID, (char*)pszNameToMatch);
		FINALIZE;
	}

	/* we have a wildcard, so we need to extract the domain components and
	 * check then against the provided wildcards.
	 */
	pWildcard = pPeer->pWildcardRoot;
	pC = pszNameToMatch;
	while(*pC != '\0') {
		if(pWildcard == NULL) {
			/* we have more domain components than we have wildcards --> no match */
			*pbIsMatching = 0;
			FINALIZE;
		}
		const uchar *const pStart = pC; /* start of current domain component */
		while(*pC != '\0' && *pC != '.') {
			++pC;
		}

		/* got the component, now do the match */
		switch(pWildcard->wildcardType) {
			case PEER_WILDCARD_NONE:
				if(   pWildcard->lenDomainPart != (size_t) (pC - pStart)
				   || strncmp((char*)pStart, (char*)pWildcard->pszDomainPart, pC - pStart)) {
					*pbIsMatching = 0;
					FINALIZE;
				}
				break;
			case PEER_WILDCARD_AT_START:
				/* we need to do the backwards-matching manually */
				if(pWildcard->lenDomainPart > (size_t) (pC - pStart)) {
					*pbIsMatching = 0;
					FINALIZE;
				}
				iName = (size_t) (pC - pStart) - pWildcard->lenDomainPart;
				iWildcard = 0;
				while(iWildcard < pWildcard->lenDomainPart) {
					// I give up, I see now way to remove false positive -- rgerhards, 2017-10-23
					#ifndef __clang_analyzer__
					if(pWildcard->pszDomainPart[iWildcard] != pStart[iName]) {
						*pbIsMatching = 0;
						FINALIZE;
					}
					#endif
					++iName;
					++iWildcard;
				}
				break;
			case PEER_WILDCARD_AT_END:
				if(   pWildcard->lenDomainPart > (size_t) (pC - pStart)
				   || strncmp((char*)pStart, (char*)pWildcard->pszDomainPart,
				   	pWildcard->lenDomainPart)) {
					*pbIsMatching = 0;
					FINALIZE;
				}
				break;
			case PEER_WILDCARD_MATCH_ALL:
				/* everything is OK, just continue */
				break;
			case PEER_WILDCARD_EMPTY_COMPONENT:
				if(pC - pStart > 0) {
				   	/* if it is not empty, it is no match... */
					*pbIsMatching = 0;
					FINALIZE;
				}
				break;
		}
		pWildcard =  pWildcard->pNext; /* we processed this entry */

		/* skip '.' if we had it and so prepare for next iteration */
		if(*pC == '.')
			++pC;
	}

	if(pWildcard != NULL) {
		/* we have more domain components than in the name to be
		 * checked. So this is no match.
		 */
		*pbIsMatching = 0;
		FINALIZE;
	}

	*pbIsMatching = 1; /* finally... it matches ;) */

finalize_it:
	RETiRet;
}


/* ------------------------------ end permitted peers code ------------------------------ */


/* Code for handling allowed/disallowed senders
 */
static void MaskIP6 (struct in6_addr *addr, uint8_t bits) {
	register uint8_t i;
	
	assert (addr != NULL);
	assert (bits <= 128);
	
	i = bits/32;
	if (bits%32)
		addr->s6_addr32[i++] &= htonl(0xffffffff << (32 - (bits % 32)));
	for (; i < (sizeof addr->s6_addr32)/4; i++)
		addr->s6_addr32[i] = 0;
}

static void MaskIP4 (struct in_addr  *addr, uint8_t bits) {
	
	assert (addr != NULL);
	assert (bits <=32 );
	
	addr->s_addr &= htonl(0xffffffff << (32 - bits));
}

#define SIN(sa)  ((struct sockaddr_in  *)(void*)(sa))
#define SIN6(sa) ((struct sockaddr_in6 *)(void*)(sa))


/* This is a cancel-safe getnameinfo() version, because we learned
 * (via drd/valgrind) that getnameinfo() seems to have some issues
 * when being cancelled, at least if the module was dlloaded.
 * rgerhards, 2008-09-30
 */
static int
mygetnameinfo(const struct sockaddr *sa, socklen_t salen,
			char *host, size_t hostlen,
			char *serv, size_t servlen, int flags)
{
	int iCancelStateSave;
	int i;

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &iCancelStateSave);
	i = getnameinfo(sa, salen, host, hostlen, serv, servlen, flags);
	pthread_setcancelstate(iCancelStateSave, NULL);
	return i;
}


/* This function adds an allowed sender entry to the ACL linked list.
 * In any case, a single entry is added. If an error occurs, the
 * function does its error reporting itself. All validity checks
 * must already have been done by the caller.
 * This is a helper to AddAllowedSender().
 * rgerhards, 2007-07-17
 */
static rsRetVal AddAllowedSenderEntry(struct AllowedSenders **ppRoot, struct AllowedSenders **ppLast,
		     		      struct NetAddr *iAllow, uint8_t iSignificantBits)
{
	struct AllowedSenders *pEntry = NULL;

	assert(ppRoot != NULL);
	assert(ppLast != NULL);
	assert(iAllow != NULL);

	if((pEntry = (struct AllowedSenders*) calloc(1, sizeof(struct AllowedSenders))) == NULL) {
		return RS_RET_OUT_OF_MEMORY; /* no options left :( */
	}
	
	memcpy(&(pEntry->allowedSender), iAllow, sizeof (struct NetAddr));
	pEntry->pNext = NULL;
	pEntry->SignificantBits = iSignificantBits;
	
	/* enqueue */
	if(*ppRoot == NULL) {
		*ppRoot = pEntry;
	} else {
		(*ppLast)->pNext = pEntry;
	}
	*ppLast = pEntry;
	
	return RS_RET_OK;
}

/* function to clear the allowed sender structure in cases where
 * it must be freed (occurs most often when HUPed).
 * rgerhards, 2008-12-02: revamped this code when we fixed the interface
 * definition. Now an iterative algorithm is used.
 */
static void
clearAllowedSenders(uchar *pszType)
{
	struct AllowedSenders *pPrev;
	struct AllowedSenders *pCurr = NULL;

	if(setAllowRoot(&pCurr, pszType) != RS_RET_OK)
		return;	/* if something went wrong, so let's leave */
	
	while(pCurr != NULL) {
		pPrev = pCurr;
		pCurr = pCurr->pNext;
		/* now delete the entry we are right now processing */
		if(F_ISSET(pPrev->allowedSender.flags, ADDR_NAME))
			free(pPrev->allowedSender.addr.HostWildcard);
		else
			free(pPrev->allowedSender.addr.NetAddr);
		free(pPrev);
	}

	/* indicate root pointer is de-init (was forgotten previously, resulting in
	 * all kinds of interesting things) -- rgerhards, 2009-01-12
	 */
	reinitAllowRoot(pszType);
}


/* function to add an allowed sender to the allowed sender list. The
 * root of the list is caller-provided, so it can be used for all
 * supported lists. The caller must provide a pointer to the root,
 * as it eventually needs to be updated. Also, a pointer to the
 * pointer to the last element must be provided (to speed up adding
 * list elements).
 * rgerhards, 2005-09-26
 * If a hostname is given there are possible multiple entries
 * added (all addresses from that host).
 */
static rsRetVal AddAllowedSender(struct AllowedSenders **ppRoot, struct AllowedSenders **ppLast,
		     		 struct NetAddr *iAllow, uint8_t iSignificantBits)
{
	struct addrinfo *restmp = NULL;
	DEFiRet;

	assert(ppRoot != NULL);
	assert(ppLast != NULL);
	assert(iAllow != NULL);

	if (!F_ISSET(iAllow->flags, ADDR_NAME)) {
		if(iSignificantBits == 0)
			/* we handle this seperatly just to provide a better
			 * error message.
			 */
			LogError(0, NO_ERRCODE, "You can not specify 0 bits of the netmask, this would "
				 "match ALL systems. If you really intend to do that, "
				 "remove all $AllowedSender directives.");

		switch (iAllow->addr.NetAddr->sa_family) {
		case AF_INET:
			if((iSignificantBits < 1) || (iSignificantBits > 32)) {
				LogError(0, NO_ERRCODE, "Invalid number of bits (%d) in IPv4 address - adjusted to 32",
					    (int)iSignificantBits);
				iSignificantBits = 32;
			}

			MaskIP4 (&(SIN(iAllow->addr.NetAddr)->sin_addr), iSignificantBits);
			break;
		case AF_INET6:
			if((iSignificantBits < 1) || (iSignificantBits > 128)) {
				LogError(0, NO_ERRCODE, "Invalid number of bits (%d) in IPv6 address - adjusted to 128",
					    iSignificantBits);
				iSignificantBits = 128;
			}

			MaskIP6 (&(SIN6(iAllow->addr.NetAddr)->sin6_addr), iSignificantBits);
			break;
		default:
			/* rgerhards, 2007-07-16: We have an internal program error in this
			 * case. However, there is not much we can do against it right now. Of
			 * course, we could abort, but that would probably cause more harm
			 * than good. So we continue to run. We simply do not add this line - the
			 * worst thing that happens is that one host will not be allowed to
			 * log.
			 */
			LogError(0, NO_ERRCODE, "Internal error caused AllowedSender to be ignored, AF = %d",
				    iAllow->addr.NetAddr->sa_family);
			ABORT_FINALIZE(RS_RET_ERR);
		}
		/* OK, entry constructed, now lets add it to the ACL list */
		iRet = AddAllowedSenderEntry(ppRoot, ppLast, iAllow, iSignificantBits);
	} else {
		/* we need to process a hostname ACL */
		if(glbl.GetDisableDNS()) {
			LogError(0, NO_ERRCODE, "Ignoring hostname based ACLs because DNS is disabled.");
			ABORT_FINALIZE(RS_RET_OK);
		}

		if (!strchr (iAllow->addr.HostWildcard, '*') &&
		    !strchr (iAllow->addr.HostWildcard, '?') &&
		    ACLDontResolve == 0) {
			/* single host - in this case, we pull its IP addresses from DNS
			* and add IP-based ACLs.
			*/
			struct addrinfo hints, *res;
			struct NetAddr allowIP;

			memset (&hints, 0, sizeof (struct addrinfo));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_DGRAM;
#			ifdef AI_ADDRCONFIG /* seems not to be present on all systems */
				hints.ai_flags  = AI_ADDRCONFIG;
#			endif

			if (getaddrinfo (iAllow->addr.HostWildcard, NULL, &hints, &res) != 0) {
			        LogError(0, NO_ERRCODE, "DNS error: Can't resolve \"%s\"", iAllow->addr.HostWildcard);

				if (ACLAddHostnameOnFail) {
				        LogError(0, NO_ERRCODE, "Adding hostname \"%s\" to ACL as a wildcard "
					"entry.", iAllow->addr.HostWildcard);
				        iRet = AddAllowedSenderEntry(ppRoot, ppLast, iAllow, iSignificantBits);
					FINALIZE;
				} else {
				        LogError(0, NO_ERRCODE, "Hostname \"%s\" WON\'T be added to ACL.",
							iAllow->addr.HostWildcard);
				        ABORT_FINALIZE(RS_RET_NOENTRY);
				}
			}

			restmp = res;
			for ( ; res != NULL ; res = res->ai_next) {
				switch (res->ai_family) {
				case AF_INET: /* add IPv4 */
					iSignificantBits = 32;
					allowIP.flags = 0;
					if((allowIP.addr.NetAddr = MALLOC(res->ai_addrlen)) == NULL) {
						ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
					}
					memcpy(allowIP.addr.NetAddr, res->ai_addr, res->ai_addrlen);

					if((iRet = AddAllowedSenderEntry(ppRoot, ppLast, &allowIP, iSignificantBits))
						!= RS_RET_OK) {
						free(allowIP.addr.NetAddr);
						FINALIZE;
					}
					break;
				case AF_INET6: /* IPv6 - but need to check if it is a v6-mapped IPv4 */
					if(IN6_IS_ADDR_V4MAPPED (&SIN6(res->ai_addr)->sin6_addr)) {
						/* extract & add IPv4 */

						iSignificantBits = 32;
						allowIP.flags = 0;
						if((allowIP.addr.NetAddr = (struct sockaddr *)
						MALLOC(sizeof(struct sockaddr))) == NULL) {
							ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
						}
						SIN(allowIP.addr.NetAddr)->sin_family = AF_INET;
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
						SIN(allowIP.addr.NetAddr)->sin_len    = sizeof (struct sockaddr_in);
#endif
						SIN(allowIP.addr.NetAddr)->sin_port   = 0;
						memcpy(&(SIN(allowIP.addr.NetAddr)->sin_addr.s_addr),
							&(SIN6(res->ai_addr)->sin6_addr.s6_addr32[3]),
							sizeof (in_addr_t));

						if((iRet = AddAllowedSenderEntry(ppRoot, ppLast, &allowIP,
								iSignificantBits))
							!= RS_RET_OK) {
							free(allowIP.addr.NetAddr);
							FINALIZE;
						}
					} else {
						/* finally add IPv6 */

						iSignificantBits = 128;
						allowIP.flags = 0;
						if((allowIP.addr.NetAddr = MALLOC(res->ai_addrlen)) == NULL) {
							ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
						}
						memcpy(allowIP.addr.NetAddr, res->ai_addr, res->ai_addrlen);

						if((iRet = AddAllowedSenderEntry(ppRoot, ppLast, &allowIP,
								iSignificantBits))
							!= RS_RET_OK) {
							free(allowIP.addr.NetAddr);
							FINALIZE;
						}
					}
					break;
				}
			}
		} else {
			/* wildcards in hostname - we need to add a text-based ACL.
			 * For this, we already have everything ready and just need
			 * to pass it along...
			 */
			iRet =  AddAllowedSenderEntry(ppRoot, ppLast, iAllow, iSignificantBits);
		}
	}

finalize_it:
	if(restmp != NULL) {
		freeaddrinfo (restmp);
	}
	RETiRet;
}


/* Print an allowed sender list. The caller must tell us which one.
 * iListToPrint = 1 means UDP, 2 means TCP
 * rgerhards, 2005-09-27
 */
static void
PrintAllowedSenders(int iListToPrint)
{
	struct AllowedSenders *pSender;
	uchar szIP[64];
	
#ifdef _AIX
#ifdef USE_GSSAPI
	assert((iListToPrint == 1) || (iListToPrint == 2) || (iListToPrint == 3));
	dbgprintf("Allowed %s Senders:\n",
	       (iListToPrint == 1) ? "UDP" :
	       (iListToPrint == 3) ? "GSS" :
	       "TCP");
#else
	assert((iListToPrint == 1) || (iListToPrint == 2));
	dbgprintf("Allowed %s Senders:\n",
	       (iListToPrint == 1) ? "UDP" :
	       "TCP");
#endif /* USE_GSSAPI */
#else /* _AIX */
	assert((iListToPrint == 1) || (iListToPrint == 2)
#ifdef USE_GSSAPI
	       || (iListToPrint == 3)
#endif
	       );

	dbgprintf("Allowed %s Senders:\n",
	       (iListToPrint == 1) ? "UDP" :
#ifdef USE_GSSAPI
	       (iListToPrint == 3) ? "GSS" :
#endif
	       "TCP");
#endif /* End of _AIX */

	pSender = (iListToPrint == 1) ? pAllowedSenders_UDP :
#ifdef USE_GSSAPI
		(iListToPrint == 3) ? pAllowedSenders_GSS :
#endif
		pAllowedSenders_TCP;
	if(pSender == NULL) {
		dbgprintf("\tNo restrictions set.\n");
	} else {
		while(pSender != NULL) {
			if (F_ISSET(pSender->allowedSender.flags, ADDR_NAME))
				dbgprintf ("\t%s\n", pSender->allowedSender.addr.HostWildcard);
			else {
				if(mygetnameinfo (pSender->allowedSender.addr.NetAddr,
						     SALEN(pSender->allowedSender.addr.NetAddr),
						     (char*)szIP, 64, NULL, 0, NI_NUMERICHOST) == 0) {
					dbgprintf ("\t%s/%u\n", szIP, pSender->SignificantBits);
				} else {
					/* getnameinfo() failed - but as this is only a
					 * debug function, we simply spit out an error and do
					 * not care much about it.
					 */
					dbgprintf("\tERROR in getnameinfo() - something may be wrong "
						"- ignored for now\n");
				}
			}
			pSender = pSender->pNext;
		}
	}
}


/* parse an allowed sender config line and add the allowed senders
 * (if the line is correct).
 * rgerhards, 2005-09-27
 */
static rsRetVal
addAllowedSenderLine(char* pName, uchar** ppRestOfConfLine)
{
	struct AllowedSenders **ppRoot;
	struct AllowedSenders **ppLast;
	rsParsObj *pPars;
	rsRetVal iRet;
	struct NetAddr *uIP = NULL;
	int iBits;

	assert(pName != NULL);
	assert(ppRestOfConfLine != NULL);
	assert(*ppRestOfConfLine != NULL);

	if(!strcasecmp(pName, "udp")) {
		ppRoot = &pAllowedSenders_UDP;
		ppLast = &pLastAllowedSenders_UDP;
	} else if(!strcasecmp(pName, "tcp")) {
		ppRoot = &pAllowedSenders_TCP;
		ppLast = &pLastAllowedSenders_TCP;
#ifdef USE_GSSAPI
	} else if(!strcasecmp(pName, "gss")) {
		ppRoot = &pAllowedSenders_GSS;
		ppLast = &pLastAllowedSenders_GSS;
#endif
	} else {
		LogError(0, RS_RET_ERR, "Invalid protocol '%s' in allowed sender "
		           "list, line ignored", pName);
		return RS_RET_ERR;
	}

	/* OK, we now know the protocol and have valid list pointers.
	 * So let's process the entries. We are using the parse class
	 * for this.
	 */
	/* create parser object starting with line string without leading colon */
	if((iRet = rsParsConstructFromSz(&pPars, (uchar*) *ppRestOfConfLine) != RS_RET_OK)) {
		LogError(0, iRet, "Error %d constructing parser object - ignoring allowed sender list", iRet);
		return(iRet);
	}

	while(!parsIsAtEndOfParseString(pPars)) {
		if(parsPeekAtCharAtParsPtr(pPars) == '#')
			break; /* a comment-sign stops processing of line */
		/* now parse a single IP address */
		if((iRet = parsAddrWithBits(pPars, &uIP, &iBits)) != RS_RET_OK) {
			LogError(0, iRet, "Error %d parsing address in allowed sender"
				    "list - ignoring.", iRet);
			rsParsDestruct(pPars);
			return(iRet);
		}
		if((iRet = AddAllowedSender(ppRoot, ppLast, uIP, iBits)) != RS_RET_OK) {
		        if(iRet == RS_RET_NOENTRY) {
			        LogError(0, iRet, "Error %d adding allowed sender entry "
					    "- ignoring.", iRet);
		        } else {
			        LogError(0, iRet, "Error %d adding allowed sender entry "
					    "- terminating, nothing more will be added.", iRet);
				rsParsDestruct(pPars);
				free(uIP);
				return(iRet);
		        }
		}
		free (uIP); /* copy stored in AllowedSenders list */
	}

	/* cleanup */
	*ppRestOfConfLine += parsGetCurrentPosition(pPars);
	return rsParsDestruct(pPars);
}



/* compares a host to an allowed sender list entry. Handles all subleties
 * including IPv4/v6 as well as domain name wildcards.
 * This is a helper to isAllowedSender.
 * Returns 0 if they do not match, 1 if they match and 2 if a DNS name would have been required.
 * contributed 2007-07-16 by mildew@gmail.com
 */
static int
MaskCmp(struct NetAddr *pAllow, uint8_t bits, struct sockaddr *pFrom, const char *pszFromHost, int bChkDNS)
{
	assert(pAllow != NULL);
	assert(pFrom != NULL);

	if(F_ISSET(pAllow->flags, ADDR_NAME)) {
		if(bChkDNS == 0)
			return 2;
		dbgprintf("MaskCmp: host=\"%s\"; pattern=\"%s\"\n", pszFromHost, pAllow->addr.HostWildcard);

#		if !defined(FNM_CASEFOLD)
			/* TODO: I don't know if that then works, seen on HP UX, what I have not in lab... ;) */
			return(fnmatch(pAllow->addr.HostWildcard, pszFromHost, FNM_NOESCAPE) == 0);
#		else
			return(fnmatch(pAllow->addr.HostWildcard, pszFromHost, FNM_NOESCAPE|FNM_CASEFOLD) == 0);
#		endif
	} else {/* We need to compare an IP address */
		switch (pFrom->sa_family) {
		case AF_INET:
			if (AF_INET == pAllow->addr.NetAddr->sa_family)
				return(( SIN(pFrom)->sin_addr.s_addr & htonl(0xffffffff << (32 - bits)) )
				       == SIN(pAllow->addr.NetAddr)->sin_addr.s_addr);
			else
				return 0;
			break;
		case AF_INET6:
			switch (pAllow->addr.NetAddr->sa_family) {
			case AF_INET6: {
				struct in6_addr ip, net;
				register uint8_t i;

				memcpy (&ip,  &(SIN6(pFrom))->sin6_addr, sizeof (struct in6_addr));
				memcpy (&net, &(SIN6(pAllow->addr.NetAddr))->sin6_addr, sizeof (struct in6_addr));

				i = bits/32;
				if (bits % 32)
					ip.s6_addr32[i++] &= htonl(0xffffffff << (32 - (bits % 32)));
				for (; i < (sizeof ip.s6_addr32)/4; i++)
					ip.s6_addr32[i] = 0;

				return (memcmp (ip.s6_addr, net.s6_addr, sizeof ip.s6_addr) == 0 &&
					(SIN6(pAllow->addr.NetAddr)->sin6_scope_id != 0 ?
					 SIN6(pFrom)->sin6_scope_id == SIN6(pAllow->addr.NetAddr)->sin6_scope_id : 1));
			}
			case AF_INET: {
				struct in6_addr *ip6 = &(SIN6(pFrom))->sin6_addr;
				struct in_addr  *net = &(SIN(pAllow->addr.NetAddr))->sin_addr;

				if ((ip6->s6_addr32[3] & (u_int32_t)
					htonl((0xffffffff << (32 - bits)))) == net->s_addr &&
#if BYTE_ORDER == LITTLE_ENDIAN
				    (ip6->s6_addr32[2] == (u_int32_t)0xffff0000) &&
#else
				    (ip6->s6_addr32[2] == (u_int32_t)0x0000ffff) &&
#endif
				    (ip6->s6_addr32[1] == 0) && (ip6->s6_addr32[0] == 0))
					return 1;
				else
					return 0;
			}
			default:
				/* Unsupported AF */
				return 0;
			}
		default:
			/* Unsupported AF */
			return 0;
		}
	}
}


/* check if a sender is allowed. The root of the the allowed sender.
 * list must be proveded by the caller. As such, this function can be
 * used to check both UDP and TCP allowed sender lists.
 * returns 1, if the sender is allowed, 0 if not and 2 if we could not
 * obtain a result because we would need a dns name, which we don't have
 * (2 was added rgerhards, 2009-11-16).
 * rgerhards, 2005-09-26
 */
static int isAllowedSender2(uchar *pszType, struct sockaddr *pFrom, const char *pszFromHost, int bChkDNS)
{
	struct AllowedSenders *pAllow;
	struct AllowedSenders *pAllowRoot = NULL;
	int bNeededDNS = 0;	/* partial check because we could not resolve DNS? */
	int ret;

	assert(pFrom != NULL);
	
	if(setAllowRoot(&pAllowRoot, pszType) != RS_RET_OK)
		return 0;	/* if something went wrong, we deny access - that's the better choice... */

	if(pAllowRoot == NULL)
		return 1; /* checking disabled, everything is valid! */
	
	/* now we loop through the list of allowed senders. As soon as
	 * we find a match, we return back (indicating allowed). We loop
	 * until we are out of allowed senders. If so, we fall through the
	 * loop and the function's terminal return statement will indicate
	 * that the sender is disallowed.
	 */
	for(pAllow = pAllowRoot ; pAllow != NULL ; pAllow = pAllow->pNext) {
		ret = MaskCmp (&(pAllow->allowedSender), pAllow->SignificantBits, pFrom, pszFromHost, bChkDNS);
		if(ret == 1)
			return 1;
		else if(ret == 2)
			bNeededDNS = 2;
	}
	return bNeededDNS;
}


/* legacy API, not to be used any longer */
static int
isAllowedSender(uchar *pszType, struct sockaddr *pFrom, const char *pszFromHost) {
	return isAllowedSender2(pszType, pFrom, pszFromHost, 1);
}


/* The following #ifdef sequence is a small compatibility
 * layer. It tries to work around the different availality
 * levels of SO_BSDCOMPAT on linuxes...
 * I borrowed this code from
 *    http://www.erlang.org/ml-archive/erlang-questions/200307/msg00037.html
 * It still needs to be a bit better adapted to rsyslog.
 * rgerhards 2005-09-19
 */
#include <sys/utsname.h>
static int
should_use_so_bsdcompat(void)
{
#ifndef OS_BSD
	static int init_done = 0;
	static int so_bsdcompat_is_obsolete = 0;

	if (!init_done) {
		struct utsname myutsname;
		unsigned int version, patchlevel;

		init_done = 1;
		if (uname(&myutsname) < 0) {
			char errStr[1024];
			dbgprintf("uname: %s\r\n", rs_strerror_r(errno, errStr, sizeof(errStr)));
			return 1;
		}
		/* Format is <version>.<patchlevel>.<sublevel><extraversion>
		 * where the first three are unsigned integers and the last
		 * is an arbitrary string. We only care about the first two. */
		if (sscanf(myutsname.release, "%u.%u", &version, &patchlevel) != 2) {
		    dbgprintf("uname: unexpected release '%s'\r\n",
			    myutsname.release);
		    return 1;
		}
		/* SO_BSCOMPAT is deprecated and triggers warnings in 2.5
		   kernels. It is a no-op in 2.4 but not in 2.2 kernels. */
		if (version > 2 || (version == 2 && patchlevel >= 5))
		    so_bsdcompat_is_obsolete = 1;
	}
	return !so_bsdcompat_is_obsolete;
#else	/* #ifndef OS_BSD */
	return 1;
#endif	/* #ifndef OS_BSD */
}
#ifndef SO_BSDCOMPAT
/* this shall prevent compiler errors due to undfined name */
#define SO_BSDCOMPAT 0
#endif


/* print out which socket we are listening on. This is only
 * a debug aid. rgerhards, 2007-07-02
 */
static void
debugListenInfo(int fd, char *type)
{
	const char *szFamily;
	int port;
	struct sockaddr_storage sa;
	socklen_t saLen = sizeof(sa);

	if(getsockname(fd, (struct sockaddr *) &sa, &saLen) == 0) {
		switch(sa.ss_family) {
		case PF_INET:
			szFamily = "IPv4";
			port = ntohs(((struct sockaddr_in *) &sa)->sin_port);
			break;
		case PF_INET6:
			szFamily = "IPv6";
			port = ntohs(((struct sockaddr_in6 *) &sa)->sin6_port);
			break;
		default:
			szFamily = "other";
			port = -1;
			break;
		}
		dbgprintf("Listening on %s syslogd socket %d (%s/port %d).\n",
			type, fd, szFamily, port);
		return;
	}

	/* we can not obtain peer info. We are just providing
	 * debug info, so this is no reason to break the program
	 * or do any serious error reporting.
	 */
	dbgprintf("Listening on syslogd socket %d - could not obtain peer info.\n", fd);
}


/* Return a printable representation of a host addresses. If
 * a parameter is NULL, it is not set.  rgerhards, 2013-01-22
 */
static rsRetVal
cvthname(struct sockaddr_storage *f, prop_t **localName, prop_t **fqdn, prop_t **ip)
{
	DEFiRet;
	assert(f != NULL);
	iRet = dnscacheLookup(f, fqdn, NULL, localName, ip);
	RETiRet;
}


/* get the name of the local host. A pointer to a character pointer is passed
 * in, which on exit points to the local hostname. This buffer is dynamically
 * allocated and must be free()ed by the caller. If the functions returns an
 * error, the pointer is NULL.
 * This function always tries to return a FQDN, even so be quering DNS. So it
 * is safe to assume for the caller that when the function does not return
 * a FQDN, it simply is not available. The domain part of that string is
 * normalized to lower case. The hostname is kept in mixed case for historic
 * reasons.
 */
#define EMPTY_HOSTNAME_REPLACEMENT "localhost-empty-hostname"
static rsRetVal
getLocalHostname(uchar **ppName)
{
	DEFiRet;
	char hnbuf[8192];
	uchar *fqdn = NULL;
	int empty_hostname = 1;

	if(gethostname(hnbuf, sizeof(hnbuf)) != 0) {
		strcpy(hnbuf, EMPTY_HOSTNAME_REPLACEMENT);
	} else {
		/* now guard against empty hostname
		 * see https://github.com/rsyslog/rsyslog/issues/1040
		 */
		if(hnbuf[0] == '\0') {
			strcpy(hnbuf, EMPTY_HOSTNAME_REPLACEMENT);
		} else {
			empty_hostname = 0;
			hnbuf[sizeof(hnbuf)-1] = '\0'; /* be on the safe side... */
		}
	}

	char *dot = strstr(hnbuf, ".");
	struct addrinfo *res = NULL;
	if(!empty_hostname && dot == NULL) {
		/* we need to (try) to find the real name via resolver */
		struct addrinfo flags;
		memset(&flags, 0, sizeof(flags));
		flags.ai_flags = AI_CANONNAME;
		int error = getaddrinfo((char*)hnbuf, NULL, &flags, &res);
		if (error != 0 &&
		    error != EAI_NONAME && error != EAI_AGAIN && error != EAI_FAIL) {
			/* If we get one of errors above, network is probably
			 * not working yet, so we fall back to local hostname below
			 */
			LogError(0, RS_RET_ERR, "getaddrinfo failed obtaining local "
				"hostname - using '%s' instead; error: %s",
				hnbuf, gai_strerror(error));
		}
		if (res != NULL) {
			/* When AI_CANONNAME is set first member of res linked-list */
			/* should contain what we need */
			if (res->ai_canonname != NULL && res->ai_canonname[0] != '\0') {
				CHKmalloc(fqdn = (uchar*)strdup(res->ai_canonname));
				dot = strstr((char*)fqdn, ".");
			}
		}
	}

	if(fqdn == NULL) {
		/* already was FQDN or we could not obtain a better one */
		CHKmalloc(fqdn = (uchar*) strdup(hnbuf));
	}

	if(dot != NULL)
		for(char *p = dot+1 ; *p ; ++p)
			*p = tolower(*p);

	*ppName = fqdn;
finalize_it:
	if (res != NULL) {
		freeaddrinfo(res);
	}
	RETiRet;
}


/* closes the UDP listen sockets (if they exist) and frees
 * all dynamically assigned memory.
 */
static void
closeUDPListenSockets(int *pSockArr)
{
	register int i;

	assert(pSockArr != NULL);
	if(pSockArr != NULL) {
	        for (i = 0; i < *pSockArr; i++)
	                close(pSockArr[i+1]);
		free(pSockArr);
	}
}


/* create a single UDP socket and bail out if an error occurs.
 * This is called from a loop inside create_udp_socket which
 * iterates through potentially multiple sockets. NOT to be
 * used elsewhere.
 */
static rsRetVal ATTR_NONNULL(1, 2)
create_single_udp_socket(int *const s, /* socket */
	struct addrinfo *const r,
	const uchar *const hostname,
	const int bIsServer,
	const int rcvbuf,
	const int sndbuf,
	const int ipfreebind,
	const char *const device
	)
{
	const int on = 1;
	int sockflags;
	int actrcvbuf;
	int actsndbuf;
	socklen_t optlen;
	char errStr[1024];
	DEFiRet;

	assert(r != NULL); // does NOT work with -O2 or higher due to ATTR_NONNULL!
	assert(s != NULL);

#	if defined (_AIX)
	/* AIXPORT : socktype will be SOCK_DGRAM, as set in hints before */
	*s = socket(r->ai_family, SOCK_DGRAM, r->ai_protocol);
#	else
	*s = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
#	endif
	if (*s < 0) {
		if(!(r->ai_family == PF_INET6 && errno == EAFNOSUPPORT)) {
			LogError(errno, NO_ERRCODE, "create_udp_socket(), socket");
			/* it is debateble if PF_INET with EAFNOSUPPORT should
			 * also be ignored...
			 */
		}
		ABORT_FINALIZE(RS_RET_ERR);
	}

#	ifdef IPV6_V6ONLY
	if (r->ai_family == AF_INET6) {
		int ion = 1;
		if (setsockopt(*s, IPPROTO_IPV6, IPV6_V6ONLY,
		      (char *)&ion, sizeof (ion)) < 0) {
			LogError(errno, RS_RET_ERR, "error creating UDP socket - setsockopt");
			ABORT_FINALIZE(RS_RET_ERR);
		}
	}
#	endif

	if(device) {
#		if defined(SO_BINDTODEVICE)
		if(setsockopt(*s, SOL_SOCKET, SO_BINDTODEVICE, device, strlen(device) + 1) < 0)
#		endif
		{
			LogError(errno, RS_RET_ERR, "create UDP socket bound to device failed");
			ABORT_FINALIZE(RS_RET_ERR);
		}
	}

	if(setsockopt(*s, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on)) < 0 ) {
		LogError(errno, RS_RET_ERR, "create UDP socket failed to set REUSEADDR");
		ABORT_FINALIZE(RS_RET_ERR);
	}

	/* We need to enable BSD compatibility. Otherwise an attacker
	 * could flood our log files by sending us tons of ICMP errors.
	 */
	/* AIXPORT : SO_BSDCOMPAT socket option is depricated, and its usage
	* has been discontinued on most unixes, AIX does not support this option,
	* hence avoid the call.
	*/
#	if !defined(OS_BSD) && !defined(__hpux)  && !defined(_AIX)
	if (should_use_so_bsdcompat()) {
		if (setsockopt(*s, SOL_SOCKET, SO_BSDCOMPAT, (char *) &on, sizeof(on)) < 0) {
			LogError(errno, RS_RET_ERR, "create UDP socket failed to set BSDCOMPAT");
			ABORT_FINALIZE(RS_RET_ERR);
		}
	}
#	endif
	if(bIsServer) {
		DBGPRINTF("net.c: trying to set server socket %d to non-blocking mode\n", *s);
		if ((sockflags = fcntl(*s, F_GETFL)) != -1) {
			sockflags |= O_NONBLOCK;
			/* SETFL could fail too, so get it caught by the subsequent
			 * error check.
			 */
			sockflags = fcntl(*s, F_SETFL, sockflags);
		}
		if (sockflags == -1) {
			LogError(errno, RS_RET_ERR, "net.c: socket %d fcntl(O_NONBLOCK)", *s);
			ABORT_FINALIZE(RS_RET_ERR);
		}
	}

	if(sndbuf != 0) {
#		if defined(SO_SNDBUFFORCE)
		if(setsockopt(*s, SOL_SOCKET, SO_SNDBUFFORCE, &sndbuf, sizeof(sndbuf)) < 0)
#		endif
		{
			/* if we fail, try to do it the regular way. Experiments show that at
			 * least some platforms do not return an error here, but silently set
			 * it to the max permitted value. So we do our error check a bit
			 * differently by querying the size below.
			 */
			if(setsockopt(*s, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf)) != 0) {
				/* keep Coverity happy */
				DBGPRINTF("setsockopt in %s:%d failed - this is expected and "
					"handled at later stages\n", __FILE__, __LINE__);
			}
		}
		/* report socket buffer sizes */
		optlen = sizeof(actsndbuf);
		if(getsockopt(*s, SOL_SOCKET, SO_SNDBUF, &actsndbuf, &optlen) == 0) {
			LogMsg(0, NO_ERRCODE, LOG_INFO,
				"socket %d, actual os socket sndbuf size is %d", *s, actsndbuf);
			if(sndbuf != 0 && actsndbuf/2 != sndbuf) {
				LogError(errno, NO_ERRCODE,
					"could not set os socket sndbuf size %d for socket %d, "
					"value now is %d", sndbuf, *s, actsndbuf/2);
			}
		} else {
			DBGPRINTF("could not obtain os socket rcvbuf size for socket %d: %s\n",
				*s, rs_strerror_r(errno, errStr, sizeof(errStr)));
		}
	}

	if(rcvbuf != 0) {
#		if defined(SO_RCVBUFFORCE)
		if(setsockopt(*s, SOL_SOCKET, SO_RCVBUFFORCE, &rcvbuf, sizeof(rcvbuf)) < 0)
#		endif
		{
			/* if we fail, try to do it the regular way. Experiments show that at
			 * least some platforms do not return an error here, but silently set
			 * it to the max permitted value. So we do our error check a bit
			 * differently by querying the size below.
			 */
			if(setsockopt(*s, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) != 0) {
				/* keep Coverity happy */
				DBGPRINTF("setsockopt in %s:%d failed - this is expected and "
					"handled at later stages\n", __FILE__, __LINE__);
			}
		}
		optlen = sizeof(actrcvbuf);
		if(getsockopt(*s, SOL_SOCKET, SO_RCVBUF, &actrcvbuf, &optlen) == 0) {
			LogMsg(0, NO_ERRCODE, LOG_INFO,
				"socket %d, actual os socket rcvbuf size %d\n", *s, actrcvbuf);
			if(rcvbuf != 0 && actrcvbuf/2 != rcvbuf) {
				LogError(errno, NO_ERRCODE,
					"cannot set os socket rcvbuf size %d for socket %d, value now is %d",
					rcvbuf, *s, actrcvbuf/2);
			}
		} else {
			DBGPRINTF("could not obtain os socket rcvbuf size for socket %d: %s\n",
				*s, rs_strerror_r(errno, errStr, sizeof(errStr)));
		}
	}

	if(bIsServer) {
		/* rgerhards, 2007-06-22: if we run on a kernel that does not support
		 * the IPV6_V6ONLY socket option, we need to use a work-around. On such
		 * systems the IPv6 socket does also accept IPv4 sockets. So an IPv4
		 * socket can not listen on the same port as an IPv6 socket. The only
		 * workaround is to ignore the "socket in use" error. This is what we
		 * do if we have to.
		 */
		if(     (bind(*s, r->ai_addr, r->ai_addrlen) < 0)
#		ifndef IPV6_V6ONLY
		     && (errno != EADDRINUSE)
#		endif
		   ) {
			if (errno == EADDRNOTAVAIL && ipfreebind != IPFREEBIND_DISABLED) {
				if (setsockopt(*s, IPPROTO_IP, IP_FREEBIND, &on, sizeof(on)) < 0) {
					LogError(errno, RS_RET_ERR, "setsockopt(IP_FREEBIND)");
				} else if (bind(*s, r->ai_addr, r->ai_addrlen) < 0) {
					LogError(errno, RS_RET_ERR, "bind with IP_FREEBIND");
				} else {
					if (ipfreebind >= IPFREEBIND_ENABLED_WITH_LOG)
						LogMsg(0, RS_RET_OK_WARN, LOG_WARNING,
							"bound address %s IP free", hostname);
					FINALIZE;
				}
			}
			ABORT_FINALIZE(RS_RET_ERR);
		}
	}

finalize_it:
	if(iRet != RS_RET_OK) {
		if(*s != -1) {
			close(*s);
			*s = -1;
		}
	}
	RETiRet;
}

/* creates the UDP listen sockets
 * hostname and/or pszPort may be NULL, but not both!
 * bIsServer indicates if a server socket should be created
 * 1 - server, 0 - client
 * Note: server sockets are created in non-blocking mode, client ones
 * are blocking.
 * param rcvbuf indicates desired rcvbuf size; 0 means OS default,
 * similar for sndbuf.
 */
static int *
create_udp_socket(uchar *hostname,
	uchar *pszPort,
	const int bIsServer,
	const int rcvbuf,
	const int sndbuf,
	const int ipfreebind,
	char *device)
{
	struct addrinfo hints, *res, *r;
	int error, maxs, *s, *socks;
	rsRetVal localRet;

	assert(!((pszPort == NULL) && (hostname == NULL))); /* one of them must be non-NULL */
	memset(&hints, 0, sizeof(hints));
	if(bIsServer)
		hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
	else
		hints.ai_flags = AI_NUMERICSERV;
	hints.ai_family = glbl.GetDefPFFamily();
	hints.ai_socktype = SOCK_DGRAM;
#	if defined (_AIX)
	/* AIXPORT : SOCK_DGRAM has the protocol IPPROTO_UDP
	 *           getaddrinfo needs this hint on AIX
	 */
	hints.ai_protocol = IPPROTO_UDP;
#	endif
	error = getaddrinfo((char*) hostname, (char*) pszPort, &hints, &res);
	if(error) {
		LogError(0, NO_ERRCODE, "%s",  gai_strerror(error));
		LogError(0, NO_ERRCODE, "UDP message reception disabled due to error logged in last message.\n");
	       return NULL;
	}

	/* Count max number of sockets we may open */
	for (maxs = 0, r = res; r != NULL ; r = r->ai_next, maxs++)
		/* EMPTY */;
	socks = MALLOC((maxs+1) * sizeof(int));
	if (socks == NULL) {
		LogError(0, RS_RET_OUT_OF_MEMORY, "couldn't allocate memory for UDP "
			"sockets, suspending UDP message reception");
		freeaddrinfo(res);
		return NULL;
	}

	*socks = 0;   /* num of sockets counter at start of array */
	s = socks + 1;
	for (r = res; r != NULL ; r = r->ai_next) {
		localRet = create_single_udp_socket(s, r, hostname, bIsServer, rcvbuf,
			sndbuf, ipfreebind, device);
		if(localRet == RS_RET_OK) {
			(*socks)++;
			s++;
		}
	}

	if(res != NULL)
		freeaddrinfo(res);

	if(Debug && *socks != maxs)
		dbgprintf("We could initialize %d UDP listen sockets out of %d we received "
		 	"- this may or may not be an error indication.\n", *socks, maxs);

	if(*socks == 0) {
		LogError(0, NO_ERRCODE, "No UDP socket could successfully be initialized, "
			 "some functionality may be disabled.\n");
		/* we do NOT need to close any sockets, because there were none... */
		free(socks);
		return(NULL);
	}

	return(socks);
}


/* check if two provided socket addresses point to the same host. Note that the
 * length of the sockets must be provided as third parameter. This is necessary to
 * compare non IPv4/v6 hosts, in which case we do a simple memory compare of the
 * address structure (in that case, the same host may not reliably be detected).
 * Note that we need to do the comparison not on the full structure, because it contains things
 * like the port, which we do not need to look at when thinking about hostnames. So we look
 * at the relevant fields, what means a somewhat more complicated processing.
 * Also note that we use a non-standard calling interface, as this is much more natural and
 * it looks extremely unlikely that we get an exception of any kind here. What we
 * return is mimiced after memcmp(), and as such useful for building binary trees
 * (the order relation may be a bit arbritrary, but at least it is consistent).
 * rgerhards, 2009-09-03
 */
static int CmpHost(struct sockaddr_storage *s1, struct sockaddr_storage* s2, size_t socklen)
{
	int ret;

	if(((struct sockaddr*) s1)->sa_family != ((struct sockaddr*) s2)->sa_family) {
		ret = memcmp(s1, s2, socklen);
		goto finalize_it;
	}

	if(((struct sockaddr*) s1)->sa_family == AF_INET) {
		if(((struct sockaddr_in *) s1)->sin_addr.s_addr == ((struct sockaddr_in*)s2)->sin_addr.s_addr) {
			ret = 0;
		} else if(((struct sockaddr_in *) s1)->sin_addr.s_addr < ((struct sockaddr_in*)s2)->sin_addr.s_addr) {
			ret = -1;
		} else {
			ret = 1;
		}
	} else if(((struct sockaddr*) s1)->sa_family == AF_INET6) {
		/* IPv6 addresses are always 16 octets long */
		ret = memcmp(((struct sockaddr_in6 *)s1)->sin6_addr.s6_addr,
			((struct sockaddr_in6*)s2)->sin6_addr.s6_addr, 16);
	} else {
		ret = memcmp(s1, s2, socklen);
	}

finalize_it:
	return ret;
}



/* check if restrictions (ALCs) exists. The goal of this function is to disable the
 * somewhat time-consuming ACL checks if no restrictions are defined (the usual case).
 * This also permits to gain some speedup by using firewall-based ACLs instead of
 * rsyslog ACLs (the recommended method.
 * rgerhards, 2009-11-16
 */
static rsRetVal
HasRestrictions(uchar *pszType, int *bHasRestrictions) {
	struct AllowedSenders *pAllowRoot = NULL;
	DEFiRet;

	CHKiRet(setAllowRoot(&pAllowRoot, pszType));

	*bHasRestrictions = (pAllowRoot == NULL) ? 0 : 1;

finalize_it:
	if(iRet != RS_RET_OK) {
		*bHasRestrictions = 1; /* in this case it is better to check individually */
		DBGPRINTF("Error %d trying to obtain ACL restriction state of '%s'\n", iRet, pszType);
	}
	RETiRet;
}


/* return the IP address (IPv4/6) for the provided interface. Returns
 * RS_RET_NOT_FOUND if interface can not be found in interface list.
 * The family must be correct (AF_INET vs. AF_INET6, AF_UNSPEC means
 * either of *these two*).
 * The function re-queries the interface list (at least in theory).
 * However, it caches entries in order to avoid too-frequent requery.
 * rgerhards, 2012-03-06
 */
#if !defined(_AIX)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align" /* TODO: how can we fix these warnings? */
/* Problem with the warnings: they seem to stem back from the way the API is structured */
#endif
static rsRetVal
getIFIPAddr(uchar *szif, int family, uchar *pszbuf, int lenBuf)
{
#ifdef _AIX
	struct ifaddrs_rsys * ifaddrs = NULL;
	struct ifaddrs_rsys * ifa;
#else
	struct ifaddrs * ifaddrs = NULL;
	struct ifaddrs * ifa;
#endif
	void * pAddr;
	DEFiRet;

	if(getifaddrs(&ifaddrs) != 0) {
		ABORT_FINALIZE(RS_RET_ERR);
	}

	for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
		if(strcmp(ifa->ifa_name, (char*)szif))
			continue;
		if(   (family == AF_INET6 || family == AF_UNSPEC)
		   && ifa->ifa_addr->sa_family == AF_INET6) {
			pAddr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
			inet_ntop(AF_INET6, pAddr, (char*)pszbuf, lenBuf);
			break;
		} else if(/*   (family == AF_INET || family == AF_UNSPEC)
		         &&*/ ifa->ifa_addr->sa_family == AF_INET) {
			pAddr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			inet_ntop(AF_INET, pAddr, (char*)pszbuf, lenBuf);
			break;
		}
	}

	if(ifaddrs != NULL)
		freeifaddrs(ifaddrs);

	if(ifa == NULL)
		iRet = RS_RET_NOT_FOUND;

finalize_it:
	RETiRet;

}
#if !defined(_AIX)
#pragma GCC diagnostic pop
#endif


/* queryInterface function
 * rgerhards, 2008-03-05
 */
BEGINobjQueryInterface(net)
CODESTARTobjQueryInterface(net)
	if(pIf->ifVersion != netCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->cvthname = cvthname;
	/* things to go away after proper modularization */
	pIf->addAllowedSenderLine = addAllowedSenderLine;
	pIf->PrintAllowedSenders = PrintAllowedSenders;
	pIf->clearAllowedSenders = clearAllowedSenders;
	pIf->debugListenInfo = debugListenInfo;
	pIf->create_udp_socket = create_udp_socket;
	pIf->closeUDPListenSockets = closeUDPListenSockets;
	pIf->isAllowedSender = isAllowedSender;
	pIf->isAllowedSender2 = isAllowedSender2;
	pIf->should_use_so_bsdcompat = should_use_so_bsdcompat;
	pIf->getLocalHostname = getLocalHostname;
	pIf->AddPermittedPeer = AddPermittedPeer;
	pIf->DestructPermittedPeers = DestructPermittedPeers;
	pIf->PermittedPeerWildcardMatch = PermittedPeerWildcardMatch;
	pIf->CmpHost = CmpHost;
	pIf->HasRestrictions = HasRestrictions;
	pIf->GetIFIPAddr = getIFIPAddr;
	/* data members */
	pIf->pACLAddHostnameOnFail = &ACLAddHostnameOnFail;
	pIf->pACLDontResolve = &ACLDontResolve;
finalize_it:
ENDobjQueryInterface(net)


/* exit our class
 * rgerhards, 2008-03-10
 */
BEGINObjClassExit(net, OBJ_IS_LOADABLE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(net)
	/* release objects we no longer need */
	objRelease(glbl, CORE_COMPONENT);
	objRelease(prop, CORE_COMPONENT);
ENDObjClassExit(net)


/* Initialize the net class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-19
 */
BEGINAbstractObjClassInit(net, 1, OBJ_IS_CORE_MODULE) /* class, version */
	/* request objects we use */
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(prop, CORE_COMPONENT));

	/* set our own handlers */
ENDObjClassInit(net)


/* --------------- here now comes the plumbing that makes as a library module --------------- */


BEGINmodExit
CODESTARTmodExit
	netClassExit();
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_LIB_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */

	/* Initialize all classes that are in our module - this includes ourselfs */
	CHKiRet(netClassInit(pModInfo)); /* must be done after tcps_sess, as we use it */
ENDmodInit
/* vi:set ai:
 */
