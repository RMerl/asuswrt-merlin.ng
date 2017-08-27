
/*
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * This source code was modified by Broadcom. It is distributed under the
 * original license terms described below.
 *
 * Copyright (c) 2002-2006 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */
/*
 * $Id: wl_mdns_common.h 467328 2014-04-03 01:23:40Z $
 *
 * Description: Mulicast DNS Service Discovery offload common defines.
 *
 */


/* FILE-CSTYLED */

#ifndef _MDNS_SD_COMMON_H_
#define _MDNS_SD_COMMON_H_

typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;
typedef int32 s32;
#define PACKED_STRUCT  __attribute__ ((packed))

#define DNS_TIME(...)
#define IS_WL_TRACE_ON() 0
#define DNS_QUERY(...)

//#define DNS_TRACE(...)
//#define DNS_ERROR(...) 	printf(__VA_ARGS__)

#define UNUSED_PARAMETER(x) (void)(x)

/*
    Old ethernet card references, to be replaced/removed
*/
#define MAX_PKT_SIZE 1584
#define SYSTEM_VAR_START 0

#define ERR_NO_ERROR            0
#define ERR_ADDR_FAILED         -1
#define ERR_PROT_UNHANDLED      -2
#define ERR_PKT_TOO_SHORT       -3
#define ERR_PROT_UNKNOWN        -4

/*
    DNS stuff
*/

typedef enum					/* From RFC 1035 */
{
	kDNSClass_IN               = 1,		/* Internet */
	kDNSClass_CS               = 2,		/* CSNET */
	kDNSClass_CH               = 3,		/* CHAOS */
	kDNSClass_HS               = 4,		/* Hesiod */
	kDNSClass_NONE             = 254,	/* Used in DNS UPDATE [RFC 2136] */

	kDNSClass_Mask             = 0x7FFF,	/* Multicast DNS uses the bottom 15 bits to
							identify the record class...
						*/
	kDNSClass_UniqueRRSet      = 0x8000,	/* ... and the top bit indicates that all other
							cached records are now invalid
						*/

	kDNSQClass_ANY             = 255,	/* Not a DNS class, but a DNS query class,
							meaning "all classes"
						*/
	kDNSQClass_UnicastResponse = 0x8000	/* Top bit set in a question means
							"unicast response acceptable"
						*/
} DNS_ClassValues;

typedef enum				/* From RFC 1035 */
{
	kDNSType_A = 1,			/*  1 Address */
	kDNSType_NS,			/*  2 Name Server */
	kDNSType_MD,			/*  3 Mail Destination */
	kDNSType_MF,			/*  4 Mail Forwarder */
	kDNSType_CNAME,			/*  5 Canonical Name */
	kDNSType_SOA,			/*  6 Start of Authority */
	kDNSType_MB,			/*  7 Mailbox */
	kDNSType_MG,			/*  8 Mail Group */
	kDNSType_MR,			/*  9 Mail Rename */
	kDNSType_NULL,			/* 10 NULL RR */
	kDNSType_WKS,			/* 11 Well-known-service */
	kDNSType_PTR,			/* 12 Domain name pointer */
	kDNSType_HINFO,			/* 13 Host information */
	kDNSType_MINFO,			/* 14 Mailbox information */
	kDNSType_MX,			/* 15 Mail Exchanger */
	kDNSType_TXT,			/* 16 Arbitrary text string */
	kDNSType_RP,			/* 17 Responsible person */
	kDNSType_AFSDB,			/* 18 AFS cell database */
	kDNSType_X25,			/* 19 X_25 calling address */
	kDNSType_ISDN,			/* 20 ISDN calling address */
	kDNSType_RT,			/* 21 Router */
	kDNSType_NSAP,			/* 22 NSAP address */
	kDNSType_NSAP_PTR,		/* 23 Reverse NSAP lookup (deprecated) */
	kDNSType_SIG,			/* 24 Security signature */
	kDNSType_KEY,			/* 25 Security key */
	kDNSType_PX,			/* 26 X.400 mail mapping */
	kDNSType_GPOS,			/* 27 Geographical position (withdrawn) */
	kDNSType_AAAA,			/* 28 IPv6 Address */
	kDNSType_LOC,			/* 29 Location Information */
	kDNSType_NXT,			/* 30 Next domain (security) */
	kDNSType_EID,			/* 31 Endpoint identifier */
	kDNSType_NIMLOC,		/* 32 Nimrod Locator */
	kDNSType_SRV,			/* 33 Service record */
	kDNSType_ATMA,			/* 34 ATM Address */
	kDNSType_NAPTR,			/* 35 Naming Authority PoinTeR */
	kDNSType_KX,			/* 36 Key Exchange */
	kDNSType_CERT,			/* 37 Certification record */
	kDNSType_A6,			/* 38 IPv6 Address (deprecated) */
	kDNSType_DNAME,			/* 39 Non-terminal DNAME (for IPv6) */
	kDNSType_SINK,			/* 40 Kitchen sink (experimental) */
	kDNSType_OPT,			/* 41 EDNS0 option (meta-RR) */
	kDNSType_APL,			/* 42 Address Prefix List */
	kDNSType_DS,			/* 43 Delegation Signer */
	kDNSType_SSHFP,			/* 44 SSH Key Fingerprint */
	kDNSType_IPSECKEY,		/* 45 IPSECKEY */
	kDNSType_RRSIG,			/* 46 RRSIG */
	kDNSType_NSEC,			/* 47 Denial of Existence */
	kDNSType_DNSKEY,		/* 48 DNSKEY */
	kDNSType_DHCID,			/* 49 DHCP Client Identifier */
	kDNSType_NSEC3,			/* 50 Hashed Authenticated Denial of Existence */
	kDNSType_NSEC3PARAM,	/* 51 Hashed Authenticated Denial of Existence */

	kDNSType_HIP = 55,		/* 55 Host Identity Protocol */

	kDNSType_SPF = 99,		/* 99 Sender Policy Framework for E-Mail */
	kDNSType_UINFO,			/* 100 IANA-Reserved */
	kDNSType_UID,			/* 101 IANA-Reserved */
	kDNSType_GID,			/* 102 IANA-Reserved */
	kDNSType_UNSPEC,		/* 103 IANA-Reserved */

	kDNSType_TKEY = 249,	/* 249 Transaction key */
	kDNSType_TSIG,			/* 250 Transaction signature */
	kDNSType_IXFR,			/* 251 Incremental zone transfer */
	kDNSType_AXFR,			/* 252 Transfer zone of authority */
	kDNSType_MAILB,			/* 253 Transfer mailbox records */
	kDNSType_MAILA,			/* 254 Transfer mail agent records */
	kDNSQType_ANY		/* Not a DNS type, but a DNS query type, meaning "all types" */
} DNS_TypeValues;

typedef enum
{
	kDNSFlag0_QR_Mask     = 0x80,		/* Query or response? */
	kDNSFlag0_QR_Query    = 0x00,
	kDNSFlag0_QR_Response = 0x80,
	
	kDNSFlag0_OP_Mask     = 0x78,		/* Operation type */
	kDNSFlag0_OP_StdQuery = 0x00,
	kDNSFlag0_OP_Iquery   = 0x08,
	kDNSFlag0_OP_Status   = 0x10,
	kDNSFlag0_OP_Unused3  = 0x18,
	kDNSFlag0_OP_Notify   = 0x20,
	kDNSFlag0_OP_Update   = 0x28,
	
	kDNSFlag0_QROP_Mask   = kDNSFlag0_QR_Mask | kDNSFlag0_OP_Mask,
	
	kDNSFlag0_AA          = 0x04,		/* Authoritative Answer? */
	kDNSFlag0_TC          = 0x02,		/* Truncated? */
	kDNSFlag0_RD          = 0x01,		/* Recursion Desired? */
	kDNSFlag1_RA          = 0x80,		/* Recursion Available? */
	
	kDNSFlag1_Zero        = 0x40,		/* Reserved; must be zero */
	kDNSFlag1_AD          = 0x20,		/* Authentic Data [RFC 2535] */
	kDNSFlag1_CD          = 0x10,		/* Checking Disabled [RFC 2535] */

	kDNSFlag1_RC          = 0x0F,		/* Response code */
	kDNSFlag1_RC_NoErr    = 0x00,
	kDNSFlag1_RC_FmtErr   = 0x01,
	kDNSFlag1_RC_SrvErr   = 0x02,
	kDNSFlag1_RC_NXDomain = 0x03,
	kDNSFlag1_RC_NotImpl  = 0x04,
	kDNSFlag1_RC_Refused  = 0x05,
	kDNSFlag1_RC_YXDomain = 0x06,
	kDNSFlag1_RC_YXRRSet  = 0x07,
	kDNSFlag1_RC_NXRRSet  = 0x08,
	kDNSFlag1_RC_NotAuth  = 0x09,
	kDNSFlag1_RC_NotZone  = 0x0A
} DNS_Flags;

typedef struct mDNSInterfaceID_dummystruct { void *dummy; } *mDNSInterfaceID;

/* mDNS defines its own names for these common types to simplify portability across */
/* multiple platforms that may each have their own (different) names for these types. */
typedef          bool  mDNSBool;
typedef   signed char  mDNSs8;
typedef unsigned char  mDNSu8;
typedef   signed short mDNSs16;
typedef unsigned short mDNSu16;
/* typedef   signed long  mDNSs32; */
/* typedef unsigned long  mDNSu32; */
typedef   signed int  mDNSs32;
typedef unsigned int  mDNSu32;

/* These types are for opaque two- and four-byte identifiers. */
/* The "NotAnInteger" fields of the unions allow the value to be conveniently passed around in a */
/* register for the sake of efficiency, and compared for equality or inequality, but don't forget -- */
/* just because it is in a register doesn't mean it is an integer. Operations like greater than, */
/* less than, add, multiply, increment, decrement, etc., are undefined for opaque identifiers, */
/* and if you make the mistake of trying to do those using the NotAnInteger field, then you'll */
/* find you get code that doesn't work consistently on big-endian and little-endian machines. */
typedef union { mDNSu8 b[ 2]; mDNSu16 NotAnInteger; } PACKED_STRUCT mDNSOpaque16;
typedef union { mDNSu8 b[ 4]; mDNSu32 NotAnInteger; } PACKED_STRUCT mDNSOpaque32;
typedef union { mDNSu8 b[ 6]; mDNSu16 w[3]; mDNSu32 l[1]; } PACKED_STRUCT mDNSOpaque48;
typedef union { mDNSu8 b[ 8]; mDNSu16 w[4]; mDNSu32 l[2]; } PACKED_STRUCT mDNSOpaque64;
typedef union { mDNSu8 b[16]; mDNSu16 w[8]; mDNSu32 l[4]; } PACKED_STRUCT mDNSOpaque128;

typedef mDNSOpaque32  mDNSv4Addr; /* IP address is a four-byte opaque identifier (not an integer) */
typedef mDNSOpaque128 mDNSv6Addr; /* IPv6 address is a 16-byte opaque identifier (not an integer) */
typedef mDNSOpaque16  mDNSIPPort; /* IP port is a two-byte opaque identifier (not an integer) */
typedef mDNSOpaque48  mDNSEthAddr; /* Ethernet is a six-byte opaque identifier (not an integer) */

enum
{
	mDNSAddrType_None    = 0,
	mDNSAddrType_IPv4    = 4,
	mDNSAddrType_IPv6    = 6,
	mDNSAddrType_Unknown = ~0 /* Special marker value used in known answer list recording */
};

typedef struct
{
	mDNSs32 type;
	union {
		mDNSv6Addr v6;
		mDNSv4Addr v4;
	} ip;
} mDNSAddr;

#define mDNSfalse FALSE
#define mDNStrue TRUE

#define mDNSNULL NULL

#define AllDNSLinkGroupv4 (AllDNSLinkGroup_v4.ip.v4)
#define AllDNSLinkGroupv6 (AllDNSLinkGroup_v6.ip.v6)

#define MulticastDNSPortAsNumber	5353

typedef struct
{
	mDNSOpaque16	id;
	mDNSOpaque16 	flags;
	u16 			numQuestions;
	u16 			numAnswers;
	u16 			numAuthorities;
	u16 			numAdditionals;
} PACKED_STRUCT DNSMessageHeader;

#define AbsoluteMaxDNSMessageData 8940
#define NormalMaxDNSMessageData 1440

typedef struct
{
	DNSMessageHeader h;			/* Note: Size 12 bytes */
	/* 40 (IPv6) + 8 (UDP) + 12 (DNS header) + 8940 (data) = 9000 */
	/* u8 data[AbsoluteMaxDNSMessageData]; */
	u8 data[NormalMaxDNSMessageData];
} PACKED_STRUCT DNSMessage;

enum
{
	kDNSRecordTypeUnregistered     = 0x00,	/* Not currently in any list */
	kDNSRecordTypeDeregistering    = 0x01,	/* Shared record about to announce its departure and leave the list */

	kDNSRecordTypeUnique           = 0x02,	/* Will become a kDNSRecordTypeVerified when probing is complete */

	kDNSRecordTypeAdvisory         = 0x04,	/* Like Shared, but no goodbye packet */
	kDNSRecordTypeShared           = 0x08,	/* Shared means record name does not have to be unique -- use random delay on responses */

	kDNSRecordTypeVerified         = 0x10,	/* Unique means mDNS should check that name is unique (and then send immediate responses) */
	kDNSRecordTypeKnownUnique      = 0x20,	/* Known Unique means mDNS can assume name is unique without checking */
	                                        /* For Dynamic Update records, Known Unique means the record must already exist on the server. */
	kDNSRecordTypeUniqueMask       = (kDNSRecordTypeUnique | kDNSRecordTypeVerified | kDNSRecordTypeKnownUnique),
	kDNSRecordTypeActiveMask       = (kDNSRecordTypeAdvisory | kDNSRecordTypeShared | kDNSRecordTypeVerified | kDNSRecordTypeKnownUnique),

	kDNSRecordTypePacketAdd        = 0x80,	/* Received in the Additional  Section of a DNS Response */
	kDNSRecordTypePacketAddUnique  = 0x90,	/* Received in the Additional  Section of a DNS Response with kDNSClass_UniqueRRSet set */
	kDNSRecordTypePacketAuth       = 0xA0,	/* Received in the Authorities Section of a DNS Response */
	kDNSRecordTypePacketAuthUnique = 0xB0,	/* Received in the Authorities Section of a DNS Response with kDNSClass_UniqueRRSet set */
	kDNSRecordTypePacketAns        = 0xC0,	/* Received in the Answer      Section of a DNS Response */
	kDNSRecordTypePacketAnsUnique  = 0xD0,	/* Received in the Answer      Section of a DNS Response with kDNSClass_UniqueRRSet set */

	kDNSRecordTypePacketAnsMask    = 0x40,	/* True for PacketAns and    PacketAnsUnique */
	kDNSRecordTypePacketUniqueMask = 0x10	/* True for PacketAddUnique, PacketAnsUnique, PacketAuthUnique */
};

enum
{
	mStatus_Waiting           = 1,
	mStatus_NoError           = 0,

	/* mDNS return values are in the range FFFE FF00 (-65792) to FFFE FFFF (-65537) */
	/* The top end of the range (FFFE FFFF) is used for error codes; */
	/* the bottom end of the range (FFFE FF00) is used for non-error values; */

	/* Error codes: */
	mStatus_UnknownErr        = -65537,		/* First value: 0xFFFE FFFF */
	mStatus_NoSuchNameErr     = -65538,
	mStatus_NoMemoryErr       = -65539,
	mStatus_BadParamErr       = -65540,
	mStatus_BadReferenceErr   = -65541,
	mStatus_BadStateErr       = -65542,
	mStatus_BadFlagsErr       = -65543,
	mStatus_UnsupportedErr    = -65544,
	mStatus_NotInitializedErr = -65545,
	mStatus_NoCache           = -65546,
	mStatus_AlreadyRegistered = -65547,
	mStatus_NameConflict      = -65548,
	mStatus_Invalid           = -65549,
	mStatus_Firewall          = -65550,
	mStatus_Incompatible      = -65551,
	mStatus_BadInterfaceErr   = -65552,
	mStatus_Refused           = -65553,
	mStatus_NoSuchRecord      = -65554,
	mStatus_NoAuth            = -65555,
	mStatus_NoSuchKey         = -65556,
	mStatus_NATTraversal      = -65557,
	mStatus_DoubleNAT         = -65558,
	mStatus_BadTime           = -65559,
	mStatus_BadSig            = -65560, /* while we define this per RFC 2845, BIND 9
						returns Refused for bad/missing signatures
						*/
	mStatus_BadKey            = -65561,
	mStatus_TransientErr      = -65562, /* transient failures, e.g. sending packets
					shortly after a network transition or wake from sleep
					*/

	/* -65563 to -65786 currently unused; available for allocation */

	/* tcp connection status */
	mStatus_ConnPending       = -65787,
	mStatus_ConnFailed        = -65788,
	mStatus_ConnEstablished   = -65789,

	/* Non-error values: */
	mStatus_GrowCache         = -65790,
	mStatus_ConfigChanged     = -65791,
	mStatus_MemFree           = -65792		/* Last value: 0xFFFE FF00 */
	
	/* mStatus_MemFree is the last legal mDNS error code, at the end
	   of the range allocated for mDNS
	*/
};

typedef u32 mStatus;

/* RFC 1034/1035 specify that a domain label consists of a length byte plus up to 63 characters */
#define MAX_DOMAIN_LABEL 63
typedef struct { u8 c[ 64]; } domainlabel; /* One label: length byte and up to 63 characters */

/* RFC 1034/1035 specify that a domain name, including length bytes,
   data bytes, and terminating zero, may be up to 255 bytes long
*/
#define MAX_DOMAIN_NAME 256
typedef struct { u8 c[256]; } domainname; /* Up to 255 bytes of length-prefixed domainlabels */

typedef struct { u8 c[256]; } UTF8str255; /* Null-terminated C string */

#define mdnsIsDigit(X)     ((X) >= '0' && (X) <= '9')
#define mDNSIsUpperCase(X) ((X) >= 'A' && (X) <= 'Z')
#define mDNSIsLowerCase(X) ((X) >= 'a' && (X) <= 'z')
#define mdnsIsLetter(X)    (mDNSIsUpperCase(X) || mDNSIsLowerCase(X))
	
#define mDNSSameIPv4Address(A, B) ((A).NotAnInteger == (B).NotAnInteger)
#define mDNSSameIPv6Address(A, B) (!memcmp((A).b, (B).b, sizeof(mDNSv6Addr)))
#define mDNSSameEthAddress(A, B)  (!memcmp((A).b, (B).b), 6)

#define mDNSIPPortIsZero(A)      ((A).NotAnInteger                            == 0)
#define mDNSOpaque16IsZero(A)    ((A).NotAnInteger                            == 0)
#define mDNSIPv4AddressIsZero(A) ((A).NotAnInteger                            == 0)
#define mDNSIPv6AddressIsZero(A) mDNSSameIPv6Address((A), zerov6Addr)

#define mDNSIPv4AddressIsOnes(A) mDNSSameIPv4Address((A), onesIPv4Addr)
#define mDNSIPv6AddressIsOnes(A) mDNSSameIPv6Address((A), onesIPv6Addr)

#define mDNSAddressIsAllDNSLinkGroup(X) (                                                     \
	((X)->type == mDNSAddrType_IPv4 && mDNSSameIPv4Address((X)->ip.v4, AllDNSLinkGroupv4)) || \
	((X)->type == mDNSAddrType_IPv6 && mDNSSameIPv6Address((X)->ip.v6, AllDNSLinkGroupv6))    )

#define mDNSAddressIsZero(X) (                                                \
	((X)->type == mDNSAddrType_IPv4 && mDNSIPv4AddressIsZero((X)->ip.v4))  || \
	((X)->type == mDNSAddrType_IPv6 && mDNSIPv6AddressIsZero((X)->ip.v6))     )

#define mDNSAddressIsValidNonZero(X) (                                        \
	((X)->type == mDNSAddrType_IPv4 && !mDNSIPv4AddressIsZero((X)->ip.v4)) || \
	((X)->type == mDNSAddrType_IPv6 && !mDNSIPv6AddressIsZero((X)->ip.v6))    )

#define mDNSAddressIsOnes(X) (                                                \
	((X)->type == mDNSAddrType_IPv4 && mDNSIPv4AddressIsOnes((X)->ip.v4))  || \
	((X)->type == mDNSAddrType_IPv6 && mDNSIPv6AddressIsOnes((X)->ip.v6))     )

#define mDNSAddressIsValid(X) (                                                                                             \
	((X)->type == mDNSAddrType_IPv4) ? !(mDNSIPv4AddressIsZero((X)->ip.v4) || mDNSIPv4AddressIsOnes((X)->ip.v4)) :          \
	((X)->type == mDNSAddrType_IPv6) ? !(mDNSIPv6AddressIsZero((X)->ip.v6) || mDNSIPv6AddressIsOnes((X)->ip.v6)) : mDNSfalse)

/* MAX_REVERSE_MAPPING_NAME */
/* For IPv4: "123.123.123.123.in-addr.arpa."  30 bytes including terminating NUL */
/* For IPv6: "x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.ip6.arpa."  74 bytes including terminating NUL */

#define MAX_REVERSE_MAPPING_NAME_V4 30
#define MAX_REVERSE_MAPPING_NAME_V6 74
#define MAX_REVERSE_MAPPING_NAME    74

/* Most records have a TTL of 75 minutes, so that their 80% cache-renewal query occurs once per hour. */
/* For records containing a hostname (in the name on the left, or in the rdata on the right), */
/* like A, AAAA, reverse-mapping PTR, and SRV, we use a two-minute TTL by default, because we don't want */
/* them to hang around for too long in the cache if the host in question crashes or otherwise goes away. */
/* Wide-area service discovery records have a very short TTL to avoid poluting intermediate caches with */
/* dynamic records.  When discovered via Long Lived Queries (with change notifications), resource record */
/* TTLs can be safely ignored. */
	
#define kStandardTTL (3600UL * 100 / 80)
#define kHostNameTTL 120UL
#define kWideAreaTTL 3

#define DefaultTTLforRRType(X) \
(((X) == kDNSType_A || (X) == kDNSType_AAAA || (X) == kDNSType_SRV) ? kHostNameTTL : kStandardTTL)

/* Assignment */
/* A simple C structure assignment of a domainname can cause a protection fault by accessing unmapped memory, */
/* because that object is defined to be 256 bytes long, but not all domainname objects are truly the full size. */
/* This macro uses mDNSPlatformMemCopy() to make sure it only touches the actual bytes that are valid. */
#define AssignDomainName(DST, SRC) memcpy((DST)->c, (SRC)->c, DomainNameLength((SRC)))

#define ResourceRecordIsValidAnswer(RR) ( ((RR)->resrec.RecordType & kDNSRecordTypeActiveMask) )

#define mDNSPlatformOneSecond  (1000)	/*Using TSF Timer */

/* A given RRType answers a QuestionType if RRType is CNAME, or types match, or QuestionType is ANY, */
/* or the RRType is NSEC and positively asserts the nonexistence of the type being requested */
#define RRTypeAnswersQuestionType(R, T) \
	((R)->rrtype == kDNSType_CNAME || (R)->rrtype == (T) || \
	(T) == kDNSQType_ANY || RRAssertsNonexistence((R), (T)))
#define RRAssertsNonexistence(R, T) \
	((R)->rrtype == kDNSType_NSEC && (T) < kDNSQType_ANY &&\
	!((R)->rdata->u.nsec->bitmap[(T)>>3] & (128 >> ((T)&7))))

typedef struct AuthRecord_struct AuthRecord;
typedef struct CacheRecord_struct CacheRecord;
typedef struct mDNS_struct mDNS;
typedef struct DNSQuestion_struct DNSQuestion;

/* A maximal NSEC record is: */
/*   256 bytes domainname 'nextname' */
/* + 256 * 34 = 8704 bytes of bitmap data */
/* = 8960 bytes total */
/* For now we only support NSEC records encoding DNS types 0-255 and ignore */
/* the nextname (we always set it to be the same as the rrname), */
/* which gives us a fixed in-memory size of 32 bytes (256 bits) */
typedef struct
{
	mDNSu8 bitmap[32];
} rdataNSEC;

/* StandardAuthRDSize is 264 (256+8), which is large enough to hold a maximum-sized SRV record */
#define StandardAuthRDSize 264
#define MaximumRDSize 768

#define InlineCacheRDSize 768

typedef struct
{
	u16 		priority;
	u16 		weight;
	mDNSIPPort	port;
	domainname 	*target;
} PACKED_STRUCT rdataSRV;

typedef struct
{
	u16 		priority;
	u16 		weight;
	mDNSIPPort	port;
	domainname 	target;
} PACKED_STRUCT rdataSRV_t;

typedef union
{
	u8          data[StandardAuthRDSize];
	mDNSv4Addr  ipv4;		/* For 'A' record */
	mDNSv6Addr  ipv6;		/* For 'AAAA' record */
	domainname  name;		/* For PTR, NS, and CNAME records */
	UTF8str255  txt;		/* For TXT record */
	rdataSRV_t  srv;		/* For SRV record */
	rdataNSEC   nsec;
} RDataBody_t;

typedef union
{
	u8          *data;
	mDNSv4Addr  *ipv4;		/* For 'A' record */
	mDNSv6Addr  *ipv6;		/* For 'AAA' record */
	domainname  *name;		/* For PTR, NS, and CNAME records */
	UTF8str255  *txt;		/* For TXT record */
	rdataSRV    *srv;		/* For SRV record */
	rdataNSEC   *nsec;
} RDataBody;

typedef struct
{
	u16   MaxRDLength; /* Amount of storage allocated for rdata (usually sizeof(RDataBody)) */
	RDataBody  	u;
} RData;
#define sizeofRDataHeader (sizeof(RData) - sizeof(RDataBody))

typedef struct
{
	u8          RecordType;		/* See enum above */
	domainname  *name;
	u16         rrtype;
	u16         rrclass;
	u32         rroriginalttl;	/* In seconds */
	u16         rdlength;		/* Size of the raw rdata, in bytes */
	u16         rdestimate;		/* Upper bound on size of rdata after name compression */
	RData       *rdata;		/* Pointer to storage for this rdata */
} ResourceRecord;

struct AuthRecord_struct
{
/* For examples of how to set up this structure for use in mDNS_Register(), */
/* see mDNS_AdvertiseInterface() or mDNS_RegisterService(). */
/* Basically, resrec and persistent metadata need to be set up before calling mDNS_Register(). */

	/* mDNS_SetupResourceRecord() is avaliable as a helper routine to set up most
		fields to sensible default values for you
	*/

	AuthRecord   *next;   /* Next in list; first element of structure for efficiency reasons */
	/* Field Group 1: Common ResourceRecord fields */
	ResourceRecord  resrec;

	/* Field Group 2: Persistent metadata for Authoritative Records */
	AuthRecord   *Additional1;  /* Recommended additional record to include in response */
	AuthRecord   *Additional2;  /* Another additional */
	AuthRecord   *DependentOn;  /* This record depends on another for its uniqueness checking */

	/* Field Group 3: Transient state for Authoritative Records */


	/* Set if this record was selected by virtue of being a direct answer to a question */
	u8              *NR_AnswerTo;
	/* Link to the next element in the chain of responses to generate */
	AuthRecord     *NextResponse;
	/* Set if this record was selected by virtue of being additional to another */
	AuthRecord     *NR_AdditionalTo;
	/* Last time we multicast this record (used to guard against packet-storm attacks) */
	s32         	LastMCTime;
	/* Set if we need to generate associated NSEC data for this rrname */
	mDNSInterfaceID SendNSECNow;
	/* Someone on this interface issued a query need to answer (all-ones for all interfaces) */
	mDNSInterfaceID ImmedAnswer;
	/* Set if we may send our response directly via unicast to the requester */
	u8          	ImmedUnicast;
	/* Hint that we might want to also send this record, just to be helpful */
	mDNSInterfaceID ImmedAdditional;
	/* The interface this query is being sent on right now */
	mDNSInterfaceID SendRNow;
	/* Recent v4 query for this record, or all-ones if more than one recent query */
	mDNSv4Addr      v4Requester;
	/* Recent v6 query for this record, or all-ones if more than one recent query */
	mDNSv6Addr      v6Requester;
	u8		macAddrRequester[6];

	RData           rdatastorage; /*	Normally the storage is right here, except for
		oversized records rdatastorage MUST be the last thing in the structure --
		when using oversized AuthRecords, extra bytes are appended after the end of the
		AuthRecord, logically augmenting the size of the rdatastorage
		*/

	/* DO NOT ADD ANY MORE FIELDS HERE */
};

struct CacheRecord_struct
{
	CacheRecord    *next; /* Next in list; first element of structure for efficiency reasons */
	ResourceRecord  resrec;

	struct {
		u16 MaxRDLength;
		void *ptr;		/* Padding for use with 64 bit machines */
		RDataBody_t u;
	} rdatastorage;			/* Storage for small records is right here */
	u8 data[InlineCacheRDSize];
};

typedef struct
{
	CacheRecord r;
	domainname namestorage;		/* Needs to go *after* the extra rdata bytes */
} LargeCacheRecord;

typedef struct NetworkInterfaceInfo_struct NetworkInterfaceInfo;

struct NetworkInterfaceInfo_struct
{
	/* Internal state fields. These are used internally by mDNSCore;
	   the client layer needn't be concerned with them.
	*/
	NetworkInterfaceInfo *next;

	/* Set if interface is sending & receiving packets (see comment above) */
	bool        InterfaceActive;

	/* Standard AuthRecords that every Responder host should have (one per active IP address) */
	AuthRecord RR_A;	/* 'A' or 'AAAA' (address) record for our ".local" name */
	AuthRecord RR_PTR;	/* PTR (reverse lookup) record */

	/* Client API fields: The client must set up these fields *before*
		calling mDNS_RegisterInterface()
	*/
	mDNSAddr        ip;		/* The IPv4 or IPv6 address to advertise */
	mDNSAddr        mask;
	mDNSBool        McastTxRx; 	/* Send/Receive multicast on this
						{ InterfaceID, address family } ?
					*/
};

struct mDNS_struct
{
	/* Value = BONJOUR_PROXY_SIG if mDNS is initialized */
	u32 signature;

	mDNSBool CanReceiveUnicastOn5353;

	/* For debugging: To catch and report locking failures */
	u32 mDNS_busy;			/* Incremented between mDNS_Lock/mDNS_Unlock section */

	/* Task Scheduling variables */
	s32  timenow_adjust;	/* Correction applied if we ever discover time went backwards */
	s32  timenow;	/* The time that this particular activation of the mDNS code started */
	s32  timenow_last;	/* The time the last time we ran */
	s32  NextScheduledEvent;	/* Derived from values below */
	s32  SuppressSending;		/* Don't send *any* packets during this time */
	s32  NextScheduledResponse;	/* Next time to send authoritative record(s) in responses */
	s32  PktNum;		/* Unique sequence number assigned to each received packet */

	u8   hostIpv4Addr[4];
	u8   hostIpv6Addr[16];

	u16  numIpv4Addr;
	u16  numIpv6Addr;
	u16  numIpv6SolAddr;
	u8  *ipv6SolAddr;

	u16  numSecondaryIpv4Addr;
	u8   *secondaryIpv4Addr;
	u16  numSecondaryIpv6Addr;
	u8  *secondaryIpv6Addr;

	u16  numUdpWakeUpPorts;
	u16 *udpWakeUpPorts;
	u16  numTcpWakeUpPorts;
	u16 *tcpWakeUpPorts;

	u16  numServices;
	u16  numServicesFound;
	u16  numTxt;
	u16  numTxtFound;

	u32  OffloadFlags;
#define MDNS_OFFLOAD_FLAGS_ENABLE_ICMP_ECHO		0x1

	bool  IPv4Available;	/* If InterfaceActive, set if v4 available on this InterfaceID */
	bool  IPv6Available;	/* If InterfaceActive, set if v6 available on this InterfaceID */

	/* Fields below only required for mDNS Responder... */
	domainname  MulticastHostname;	/* Fully Qualified "dot-local" Host Name,
						e.g. "Foo.local."
					*/
	AuthRecord  DeviceInfo;
	AuthRecord *ResourceRecords;
	AuthRecord *NewLocalRecords;	/* Fresh local-only records not yet delivered to
						local-only questions
					*/
	AuthRecord *CurrentRecord;	/* Next AuthRecord about to be examined */

	NetworkInterfaceInfo *HostInterfaces;

	rdataNSEC   nsecStorage;

	DNSMessage *omsg;	/* Outgoing message we're building */
	LargeCacheRecord rec;	/* Resource Record extracted from received message */
};

struct DNSQuestion_struct
{
	/* Internal state fields. These are used internally by mDNSCore;
		the client layer needn't be concerned with them.
	*/
	DNSQuestion  *next;
	s32  DelayAnswering;	/* Set if we want to defer answering this question until
					the cache settles
				*/
	s32  LastQTime;	/* Last scheduled transmission of this Q on *all* applicable interfaces */
	s32  ThisQInterval;
		/* LastQTime + ThisQInterval is the next scheduled transmission of this Q */
		/* ThisQInterval > 0 for an active question; */
		/* ThisQInterval = 0 for a suspended question that's still in the list */
		/* ThisQInterval = -1 for a cancelled question that's been removed from the list */
	s32  LastAnswerPktNum;	/* The sequence number of the last response packet containing
					an answer to this Q
				*/
	u32  RecentAnswerPkts;	/* Number of answers since the last time we sent this query */
	u32  CurrentAnswers;	/* Number of records currently in the cache that answer
					this question
				*/
	u32  LargeAnswers;	/* Number of answers with rdata > 1024 bytes */
	u32  UniqueAnswers;	/* Number of answers received with kDNSClass_UniqueRRSet bit set */
	DNSQuestion  *DuplicateOf;
	DNSQuestion  *NextInDQList;
	bool SendOnAll;		/* Set if we're sending this question on all active interfaces */
	s32  LastQTxTime;	/* Last time this Q was sent on one (but not necessarily all)
					interfaces
				*/
	domainname        qname;
	u16  qtype;
	u16  qclass;
};

typedef struct ServiceRecordSet_struct ServiceRecordSet;

struct ServiceRecordSet_struct
{
	/* Internal state fields. These are used internally by mDNSCore; the client layer
		needn't be concerned with them.
	*/
	/* No fields need to be set up by the client prior to calling mDNS_RegisterService(); */
	/* all required data is passed as parameters to that function. */
	ServiceRecordSet    *next;

	mDNSu32     NumSubTypes;
	AuthRecord  *SubTypes;

	AuthRecord  RR_ADV;	/* e.g. _services._dns-sd._udp.local. PTR _printer._tcp.local. */
	AuthRecord  RR_PTR;	/* e.g. _printer._tcp.local.        PTR Name._printer._tcp.local. */
	AuthRecord  RR_SRV;	/* e.g. Name._printer._tcp.local.   SRV 0 0 port target */
	AuthRecord  RR_TXT;	/* e.g. Name._printer._tcp.local.   TXT PrintQueueName */
	/* Don't add any fields after AuthRecord RR_TXT. */
	/* This is where the implicit extra space goes if we allocate a ServiceRecordSet
		containing an oversized RR_TXT record
	*/
};

#define PutResourceRecordTTL(msg, ptr, count, rr, ttl) \
	PutResourceRecordTTLWithLimit((msg), (ptr), (count), (rr), (ttl), \
		(msg)->data + NormalMaxDNSMessageData)
#define PutResourceRecord(MSG, P, C, RR) \
	PutResourceRecordTTL((MSG), (P), (C), (RR), (RR)->rroriginalttl)

#define GetRRDomainNameTarget(RR) (\
((RR)->rrtype == kDNSType_CNAME || (RR)->rrtype == kDNSType_PTR || (RR)->rrtype == kDNSType_NS) \
? &(RR)->rdata->u.name       : NULL)

#define ValidateDomainName(N) (DomainNameLength(N) <= MAX_DOMAIN_NAME)

#define TicksTTL(RR) ((s32)(RR)->resrec.rroriginalttl * mDNSPlatformOneSecond)

#define localdomain 	(*(domainname *)"\x5" "local")
#define DeviceInfoName	(*(domainname *)"\xC" "_device-info" "\x4" "_tcp")

#define SameResourceRecordSignature(A, B) \
	(A)->resrec.rrtype == (B)->resrec.rrtype && SameResourceRecordNameClassInterface((A), (B))


/* Main struct to pull together all pieces */
typedef struct {
	u32			signature;	/* Signature */
	mDNS 			*m;
	ServiceRecordSet 	*mdns_sr;	/* Service Records */
	NetworkInterfaceInfo	*mdns_intf;	/* Interface Info from host */
} mdns_share_mem_block_t;

extern mdns_share_mem_block_t *mdns_share_blk;

#define BONJOUR_PROXY_SIG	0x424A5030	/* "BJP0" */

extern u16 CompressedDomainNameLength(domainname *name, domainname *parent);
extern u16 DomainNameLength(domainname *name);
extern u8 *getDomainName(DNSMessage *msg, u8 *ptr, u8 *end, domainname *name);
extern u8 *GetLargeResourceRecord(DNSMessage *msg, u8 *ptr, u8 *end, u8 RecordType,
	LargeCacheRecord *largecr);
extern u16 GetRDLength(ResourceRecord *rr, bool estimate);
extern bool IdenticalResourceRecord(ResourceRecord *r1, ResourceRecord *r2);
extern bool mDNSPlatformMemSame(void *src, void *dst, u32 len);
extern s32 NonZeroTime(s32 t);
extern bool SameDomainLabel(u8 *a, u8 *b);
extern bool SameDomainName(domainname *d1, domainname *d2);
extern bool SameRData(ResourceRecord *r1, ResourceRecord *r2);
extern bool SameRDataBody(ResourceRecord *r1, RDataBody *r2);
extern mDNSBool SameResourceRecordNameClassInterface(AuthRecord *r1, AuthRecord *r2);
extern void SetNewRData(ResourceRecord *const rr, RData *NewRData, u16 rdlength);
extern void mDNS_SetupResourceRecord(AuthRecord *rr, u16 rrtype, u32 ttl, u8 RecordType);
extern u32 simple_divide(u32 dividend, u32 divisor);
extern void print_domainname(domainname *name);

#endif  /* _MDNS_SD_COMMON_H_ */
