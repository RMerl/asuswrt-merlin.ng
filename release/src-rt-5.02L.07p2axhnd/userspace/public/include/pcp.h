/*
 * Copyright (C) 2010-2011  Internet Systems Consortium, Inc. ("ISC")
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* $Id: pcp.h 1253 2011-07-04 14:54:05Z fdupont $ */

/*
 * PCP (port-control-protocol) client library include
 *
 * Francis_Dupont@isc.org, December 2010
 *
 * a la miniUPnP libnatpmp
 *
 * types: pcp_t, pcp_request_t, pcp_response_t
 */

/* API version */

#define __PCP_API_VERSION__		0

#include <sys/time.h>

/* Types */

#ifndef PCP_MAX_OPTIONS
#define PCP_MAX_OPTIONS	4
#endif
#define PCP_VECOPT_SIZE	(PCP_MAX_OPTIONS + 1)

typedef struct {			/* PCP handler (opaque) */
	int s;				/* server socket */
	uint8_t usable;			/* better than s < 0 */
	uint8_t tries;			/* retry counter */
	uint8_t reset;			/* reset flag */
	struct timeval retry;		/* next retry date */
	uint16_t reqlen;		/* current request length */
	uint16_t resplen;		/* current response length */
	uint8_t *request;		/* current request */
#define PCP_RESPLENGTH	512
	uint8_t *response;		/* current response */
} pcp_t;

typedef struct {			/* PCP option */
	uint8_t code;			/* option code */
	uint8_t res;			/* reserved field */
	uint16_t length;		/* option data length (per 32 bits) */
	uint8_t *data;			/* option data */
} pcp_option_t;

typedef struct {			/* PCP element of request */
	uint8_t ver;			/* version */
	uint8_t opcode; 		/* opcode */
	uint16_t res;			/* reserved field */
	uint32_t lifetime;		/* lifetime */
	uint8_t iaddr[16];		/* internal address */
    /* fields below are for MAP/PEER opcode specific */
	uint16_t iport;			/* internal port */
	uint16_t eport;			/* external port */
	uint8_t eaddr[16];		/* external address */
	uint8_t raddr[16];		/* remote address */
	uint16_t rport;			/* remote peer's port */
	uint8_t protocol;		/* protocol */
} pcp_request_t;

/*
 * PCP response Packet Format:
 *
 * 0        1       2         3
 * Version  Opcode  Reserved  Result
 *             Lifetime
 *            Epoch Time
 *             Reserved
 *             Reserved
 *             Reserved
 *    Opcode specific response data
 *              Options
 */
typedef struct {			/* PCP response */
	uint8_t ver;			/* version */
	uint8_t opcode;			/* opcode */
	uint8_t res;			/* reserved field */
	uint8_t result;			/* result-code */
	uint32_t lifetime;		/* lifetime */
	uint32_t epochtime;		/* second since start of epoch */
	uint8_t optcnt;			/* option count */
	pcp_request_t assigned;	/* assigned request */
	pcp_option_t options[PCP_VECOPT_SIZE];	/* returned options */
	unsigned int optoffs[PCP_VECOPT_SIZE];	/* options offsets */
} pcp_response_t;

typedef struct {			/* PCP MAP specific info */
	uint8_t proto;			/* protocol */
	uint8_t res[3]; 		/* reserved field */
	uint16_t iport;			/* internal port */
	uint16_t eport;			/* external port */
	uint8_t eaddr[16];		/* external address */
} pcp_map_t;

typedef struct {			/* PCP PEER specific info */
	uint8_t proto;			/* protocol */
	uint8_t res1[3]; 		/* reserved field */
	uint16_t iport;			/* internal port */
	uint16_t eport;			/* external port */
	uint8_t eaddr[16];		/* external address */
	uint16_t rport;			/* remote peer's port */
	uint16_t res2;			/* reserved field */
	uint8_t raddr[16];		/* remote peer's address */
} pcp_peer_t;

#if 0
typedef struct {			/* PCP third_party option */
	uint8_t code;			/* 1 */
	uint8_t res;			/* 0 */
	uint16_t length;		/* 16 */
	uint8_t iaddr[16];		/* LAN device's address */
} pcp_option_thirdparty_t;

typedef struct {			/* PCP prefer_failure option */
	uint8_t code;			/* 2 */
	uint8_t res;			/* 0 */
	uint16_t length;		/* 0 */
} pcp_option_preferfailure_t;

typedef struct {			/* PCP filter option */
	uint8_t code;			/* 3 */
	uint8_t res1;			/* 0 */
	uint16_t length;		/* 20 */
	uint8_t res2;			/* 0 */
	uint8_t plen;			/* prefix length */
	uint16_t rport; 		/* remote peer's port */
	uint8_t raddr[16];		/* remote peer's address */
} pcp_option_filter_t;
#endif

/* Routines */

extern int pcp_init(struct sockaddr *server,
		    struct sockaddr *source,
		    pcp_t *pcp, int isRenew);
extern int pcp_close(pcp_t *pcp, int isRenew);
extern int pcp_getsocket(pcp_t *pcp, int *sock);
extern int pcp_gettries(pcp_t *pcp, int *tries);
extern int pcp_gettimeout(pcp_t *pcp, struct timeval *timeout, int relative);
extern int pcp_setreset(pcp_t *pcp, int reset, int *old);
extern int pcp_third_party(pcp_option_t ***options, const uint8_t *addr);
extern int pcp_prefer_failure(pcp_option_t ***options);
extern int pcp_filter4(pcp_option_t ***options, uint32_t addr, uint16_t port);
extern int pcp_filter6(pcp_option_t ***options, uint8_t *addr, uint16_t port);
extern int pcp_makerequest(pcp_t *pcp,
			   pcp_request_t *request,
			   pcp_option_t **options);
extern int pcp_sendrequest(pcp_t *pcp);
extern int pcp_recvresponse(pcp_t *pcp, pcp_response_t *response);
extern const char *pcp_strerror(int error);
extern int pcp_fillintaddr(pcp_t *pcp, pcp_request_t *request);
extern int pcp_getmapping(pcp_t *pcp,
			  pcp_request_t *request,
			  pcp_option_t **options,
			  pcp_response_t *response);
extern int pcp_renewmapping(pcp_t *pcp, pcp_response_t *response);
extern int pcp_delmapping(pcp_t *pcp, pcp_response_t *response);
extern int _pcp_setrequest(pcp_t *pcp, uint8_t *request, uint16_t reqlen);
extern int _pcp_getrequest(pcp_t *pcp, uint8_t **request, uint16_t *reqlen);
extern int _pcp_setresponse(pcp_t *pcp, uint8_t *response, uint16_t resplen);
extern int _pcp_getresponse(pcp_t *pcp, uint8_t **response, uint16_t *resplen);

/* Support PCP version */
#define PCP_VERSION_IMPL     1

/* UDP port number */
#define PCP_LISTEN_PORT1    5350
#define PCP_LISTEN_PORT2    5351
#define PCP_RELINQUISH_PORT    44323

/* opcode values */
#define PCP_OPCODE_ANNOUNCE 0
#define PCP_OPCODE_MAP		1
#define PCP_OPCODE_PEER     2
#define PCP_OPCODE_RESPONSE	128	/* response bit */

/* offsets in the common header */
#define PCP_OPCODE_HDR		24

/* offsets in the MAP header */
#define PCP_MAP_PROTOCOL	(PCP_OPCODE_HDR + 0)
#define PCP_MAP_INTERNAL_PORT	(PCP_OPCODE_HDR + 4)
#define PCP_MAP_EXTERNAL_PORT	(PCP_OPCODE_HDR + 6)
#define PCP_MAP_EXTERNAL_ADDR	(PCP_OPCODE_HDR + 8)

/* offsets in the PEER header */
#define PCP_PEER_PROTOCOL	(PCP_OPCODE_HDR + 0)
#define PCP_PEER_INTERNAL_PORT	(PCP_OPCODE_HDR + 4)
#define PCP_PEER_EXTERNAL_PORT	(PCP_OPCODE_HDR + 6)
#define PCP_PEER_EXTERNAL_ADDR	(PCP_OPCODE_HDR + 8)
#define PCP_PEER_REMOTE_PORT	(PCP_OPCODE_HDR + 24)
#define PCP_PEER_REMOTE_ADDR	(PCP_OPCODE_HDR + 28)

/* lengths */
#define PCP_LEN_MAP     48
#define PCP_LEN_PEER    68
#define PCP_LEN_THIRDPARTY_OPTION    16
#define PCP_LEN_FILTER_OPTION    20

/* PCP Result Code */
#define PCP_OK		    	(0)	/* SUCCESS */
#define PCP_UNSUPP_VERSION	(1)	/* Unsupported protocol version */
#define PCP_NOT_AUTHORIZED	(2)	/* Unauthorized req */
#define PCP_MALFORMED_REQ  	(3)	/* Request cannot be parsed */
#define PCP_UNSUPP_OPCODE  	(4)	/* Unsupported opcode */
#define PCP_UNSUPP_OPTION  	(5)	/* Unsupported option */
#define PCP_MALFORMED_OPT  	(6)	/* Malformed option */
#define PCP_NETWORK_FAIL  	(7)	/* Network failure */
#define PCP_NO_RESOURCE    	(8)	/* NO_RESOURCE */
#define PCP_UNSUPP_PROTO   	(9)	/* Unsupported proto */
#define PCP_EX_QUOTA    	(10)/* Exceed user's port quota */
#define PCP_NO_EXTERNAL    	(11)/* Cannot provide external */
#define PCP_ADDR_MISMATCH  	(12)/* Address mismatch */


/* Errors */
/* bad API use */
#define PCP_ERR_INVAL		(-1)	/* invalid arguments */
/* OS errors */
#define PCP_ERR_NOMEM		(-10)	/* malloc() failed */
#define PCP_ERR_SOCKET		(-11)	/* socket() syscall failed */
#define PCP_ERR_BIND		(-12)	/* bind() syscall failed */
#define PCP_ERR_CONNECT		(-13)	/* connect() syscall failed */
#define PCP_ERR_SEND		(-14)	/* send() syscall failed */
#define PCP_ERR_RECV		(-15)	/* recv() syscall failed */
#define PCP_ERR_SYSCALL		(-16)	/* miscellaneous syscall failed */
/* routine errors */
#define PCP_ERR_NOREQUEST	(-20)	/* no current request */
#define PCP_ERR_RECVBAD		(-21)	/* received a bad response */
#define PCP_ERR_TOOMANYOPTS	(-22)	/* too many options */
/* *mapping failure */
#define PCP_ERR_FAILURE		(-30)	/* *mapping() internal failure */
/* user application */
#define PCP_ERR_APP0		(-50)	/* user application error 0 */
#define PCP_ERR_APP1		(-51)	/* user application error 1 */
#define PCP_ERR_APP2		(-52)	/* user application error 2 */
/* protocol errors */
#define PCP_ERR_PROTOBASE	(-100)	/* base for protocol errors */
#define PCP_ERR_UNSUPVERSION	(-101)	/* unsupported version */
#define PCP_ERR_BADREQUEST	(-102)	/* malformed request */
#define PCP_ERR_UNSUPOPCODE	(-103)	/* unsupported opcode */
#define PCP_ERR_UNSUPOPTION	(-104)	/* unsupported option */
#define PCP_ERR_BADOPTION	(-105)	/* malformed option */
#define PCP_ERR_PROCERROR	(-106)	/* processing error */
#define PCP_ERR_SRVOVERLOAD	(-107)	/* overloaded server */
#define PCP_ERR_NETFAILURE	(-120)	/* network failure */
#define PCP_ERR_NORESOURCES	(-121)	/* out of resources */
#define PCP_ERR_UNSUPPROTO	(-122)	/* unsupported protocol */
#define PCP_ERR_NOTAUTH		(-123)	/* not authorized */
#define PCP_ERR_EXQUOTA		(-124)	/* user exceeded quota */
#define PCP_ERR_CANTPROVIDE	(-125)	/* cannot provide external port */
#define PCP_ERR_TOOMANYPEER	(-126)	/* excessive number of remote peers */
#define PCP_ERR_UNAUTH3PTY	(-151)	/* unauthorized third party */
#define PCP_ERR_IMPLICITMAP	(-228)	/* collides with implicit mapping */

/* Options */

#define PCP_OPTION_THIRD_PARTY	1	/* third-party */
#define PCP_OPTION_PREF_FAIL	2	/* prefer failure */
#define PCP_OPTION_FILTER	3	/* remote peer filter */
#define PCP_OPTION_OPTIONAL	128	/* flag bit */

/* Others */

/* Default port of PCP service */

#define PCP_PORT		5351

/* Default retry timer (in seconds) */

#define PCP_RETRY_TIMER		2

/* Maximum number of tries */

#define PCP_MAX_TRIES		4

/* Default reset threshold (in seconds) */

#ifndef PCP_RESET_THRESHOLD
#define PCP_RESET_THRESHOLD		60
#endif

/* Tunables & co */

/* weak definition so you can overwrite them */

extern void *pcp_malloc(size_t size);
extern void pcp_free(void *ptr);

/* inline utility to free allocated options */

static inline void pcp_freeoptions(pcp_option_t **options) {
	if (options != NULL) {
		unsigned int i;

		for (i = 0; options[i] != NULL; i++)
			pcp_free(options[i]);
		pcp_free(options);
		options = NULL;
	}
}

extern const char *err_app0_msg;
extern const char *err_app1_msg;
extern const char *err_app2_msg;

/* for bad response debugging */

#define LIBSPEC

// LIBSPEC int pcp_debug_offset;
// LIBSPEC int pcp_debug_line;
