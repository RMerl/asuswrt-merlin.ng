/* Definitions for network-related stuff.
 *
 * Copyright 2007-2016 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * The rsyslog runtime library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The rsyslog runtime library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the rsyslog runtime library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 * A copy of the LGPL can be found in the file "COPYING.LESSER" in this distribution.
 */

#ifndef INCLUDED_NET_H
#define INCLUDED_NET_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h> /* this is needed on HP UX -- rgerhards, 2008-03-04 */

typedef enum _TCPFRAMINGMODE {
		TCP_FRAMING_OCTET_STUFFING = 0, /* traditional LF-delimited */
		TCP_FRAMING_OCTET_COUNTING = 1  /* -transport-tls like octet count */
	} TCPFRAMINGMODE;

#define   F_SET(where, flag) ((where)|=(flag))
#define F_ISSET(where, flag) (((where)&(flag))==(flag))
#define F_UNSET(where, flag) ((where)&=~(flag))

#define ADDR_NAME 0x01 /* address is hostname wildcard) */
#define ADDR_PRI6 0x02 /* use IPv6 address prior to IPv4 when resolving */

/* portability: incase IP_FREEBIND is not defined */
#ifndef IP_FREEBIND
#define IP_FREEBIND 0
#endif
/* defines for IP_FREEBIND, currently being used in imudp */
#define IPFREEBIND_DISABLED 0x00 /* don't enable IP_FREEBIND in  sock option */
#define IPFREEBIND_ENABLED_NO_LOG 0x01 /* enable IP_FREEBIND but no warn on success */
#define IPFREEBIND_ENABLED_WITH_LOG 0x02 /* enable IP_FREEBIND and warn on success */

#ifdef OS_BSD
#	ifndef _KERNEL
#		define s6_addr32 __u6_addr.__u6_addr32
#	endif
#endif

struct NetAddr {
	uint8_t flags;
	union {
		struct sockaddr *NetAddr;
		char *HostWildcard;
	} addr;
};

#ifndef SO_BSDCOMPAT
	/* this shall prevent compiler errors due to undefined name */
#	define SO_BSDCOMPAT 0
#endif


/* IPv6 compatibility layer for older platforms
 * We need to handle a few things different if we are running
 * on an older platform which does not support all the glory
 * of IPv6. We try to limit toll on features and reliability,
 * but obviously it is better to run rsyslog on a platform that
 * supports everything...
 * rgerhards, 2007-06-22
 */
#ifndef AI_NUMERICSERV
#  define AI_NUMERICSERV 0
#endif


#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
#define SALEN(sa) ((sa)->sa_len)
#else
static inline size_t __attribute__((unused))
SALEN(struct sockaddr *sa) {
	switch (sa->sa_family) {
	case AF_INET:  return (sizeof (struct sockaddr_in));
	case AF_INET6: return (sizeof (struct sockaddr_in6));
	default:       return 0;
	}
}
#endif

struct AllowedSenders {
	struct NetAddr allowedSender; /* ip address allowed */
	uint8_t SignificantBits;      /* defines how many bits should be discarded (eqiv to mask) */
	struct AllowedSenders *pNext;
};


/* this structure is a helper to implement wildcards in permittedPeers_t. It specifies
 * the domain component and the matching mode.
 * rgerhards, 2008-05-27
 */
struct permittedPeerWildcard_s {
	uchar *pszDomainPart;
	size_t lenDomainPart;
	enum {
		PEER_WILDCARD_NONE = 0,		/**< no wildcard in this entry */
		PEER_WILDCARD_AT_START = 1,	/**< wildcard at start of entry (*name) */
		PEER_WILDCARD_AT_END = 2,	/**< wildcard at end of entry (name*) */
		PEER_WILDCARD_MATCH_ALL = 3,	/**< only * wildcard, matches all values */
		PEER_WILDCARD_EMPTY_COMPONENT = 4/**< special case: domain component empty (e.g. "..") */
	} wildcardType;
	permittedPeerWildcard_t *pNext;
};

/* for fingerprints and hostnames, we need to have a temporary linked list of
 * permitted values. Unforutnately, we must also duplicate this in the netstream
 * drivers. However, this is the best interim solution (with the least effort).
 * A clean implementation requires that we have more capable variables and the
 * full-fledged scripting engine available. So we have opted to do the interim
 * solution so that our users can begin to enjoy authenticated TLS. The next step
 * (hopefully) is to enhance RainerScript. -- rgerhards, 2008-05-19
 */
struct permittedPeers_s {
	uchar *pszID;
	enum {
		PERM_PEER_TYPE_UNDECIDED = 0,	/**< we have not yet decided the type (fine in some auth modes) */
		PERM_PEER_TYPE_PLAIN = 1,	/**< just plain text contained */
		PERM_PEER_TYPE_WILDCARD = 2,	/**< wildcards are contained, wildcard struture is filled */
	} etryType;
	permittedPeers_t *pNext;
	permittedPeerWildcard_t *pWildcardRoot;	/**< root of the wildcard, NULL if not initialized */
	permittedPeerWildcard_t *pWildcardLast;	/**< end of the wildcard list, NULL if not initialized */
};


/* interfaces */
BEGINinterface(net) /* name must also be changed in ENDinterface macro! */
	rsRetVal (*cvthname)(struct sockaddr_storage *f, prop_t **localName, prop_t **fqdn, prop_t **ip);
	/* things to go away after proper modularization */
	rsRetVal (*addAllowedSenderLine)(char* pName, uchar** ppRestOfConfLine);
	void (*PrintAllowedSenders)(int iListToPrint);
	void (*clearAllowedSenders)(uchar*);
	void (*debugListenInfo)(int fd, char *type);
	int *(*create_udp_socket)(uchar *hostname, uchar *LogPort, int bIsServer, int rcvbuf, int sndbuf,
		int ipfreebind, char *device);
	void (*closeUDPListenSockets)(int *finet);
	int (*isAllowedSender)(uchar *pszType, struct sockaddr *pFrom, const char *pszFromHost); /* deprecated! */
	rsRetVal (*getLocalHostname)(uchar**);
	int (*should_use_so_bsdcompat)(void);
	/* permitted peer handling should be replaced by something better (see comments above) */
	rsRetVal (*AddPermittedPeer)(permittedPeers_t **ppRootPeer, uchar *pszID);
	rsRetVal (*DestructPermittedPeers)(permittedPeers_t **ppRootPeer);
	rsRetVal (*PermittedPeerWildcardMatch)(permittedPeers_t *pPeer, const uchar *pszNameToMatch, int *pbIsMatching);
	/* v5 interface additions */
	int (*CmpHost)(struct sockaddr_storage *, struct sockaddr_storage*, size_t);
	/* v6 interface additions - 2009-11-16 */
	rsRetVal (*HasRestrictions)(uchar *, int *bHasRestrictions);
	int (*isAllowedSender2)(uchar *pszType, struct sockaddr *pFrom, const char *pszFromHost, int bChkDNS);
	/* v7 interface additions - 2012-03-06 */
	rsRetVal (*GetIFIPAddr)(uchar *szif, int family, uchar *pszbuf, int lenBuf);
	/* data members - these should go away over time... TODO */
	int    *pACLAddHostnameOnFail; /* add hostname to acl when DNS resolving has failed */
	int    *pACLDontResolve;       /* add hostname to acl instead of resolving it to IP(s) */
	/* v8 cvthname() signature change -- rgerhards, 2013-01-18 */
	/* v9 create_udp_socket() signature change -- dsahern, 2016-11-11 */
ENDinterface(net)
#define netCURR_IF_VERSION 9 /* increment whenever you change the interface structure! */

/* prototypes */
PROTOTYPEObj(net);

/* the name of our library binary */
#define LM_NET_FILENAME "lmnet"

#endif /* #ifndef INCLUDED_NET_H */
