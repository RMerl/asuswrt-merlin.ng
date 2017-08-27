/*
 * eap_types.h  Header file containing the interfaces for all EAP types.
 *
 * most contents moved from modules/rlm_eap/eap.h
 *
 * Version:     $Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2001  hereUare Communications, Inc. <raghud@hereuare.com>
 * Copyright 2003  Alan DeKok <aland@freeradius.org>
 * Copyright 2006  The FreeRADIUS server project
 */
#ifndef _EAP_TYPES_H
#define _EAP_TYPES_H

RCSIDH(eap_methods_h, "$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>

/* Field length and other arbitrary things */
#define EAP_HEADER_LEN 		4

/* base for dictionary values */
#define ATTRIBUTE_EAP_ID	1020
#define ATTRIBUTE_EAP_CODE      1021
#define ATTRIBUTE_EAP_BASE      1280

typedef enum eap_code {
	PW_EAP_REQUEST = 1,
	PW_EAP_RESPONSE,
	PW_EAP_SUCCESS,
	PW_EAP_FAILURE,
	PW_EAP_MAX_CODES
} eap_code_t;

typedef enum eap_method {
	PW_EAP_INVALID = 0,		/* 0 */
	PW_EAP_IDENTITY,		/* 1 */
	PW_EAP_NOTIFICATION,		/* 2 */
	PW_EAP_NAK,			/* 3 */
	PW_EAP_MD5,			/* 4 */
	PW_EAP_OTP,			/* 5 */
	PW_EAP_GTC,			/* 6 */
	PW_EAP_7,			/* 7  - unused */
	PW_EAP_8,			/* 8  - unused */
	PW_EAP_RSA_PUBLIC_KEY,		/* 9 */
	PW_EAP_DSS_UNILATERAL,		/* 10 */
	PW_EAP_KEA,			/* 11 */
	PW_EAP_KEA_VALIDATE,		/* 12 */
	PW_EAP_TLS,			/* 13 */
	PW_EAP_DEFENDER_TOKEN,		/* 14 */
	PW_EAP_RSA_SECURID,		/* 15 */
	PW_EAP_ARCOT_SYSTEMS,		/* 16 */
	PW_EAP_LEAP,			/* 17 */
	PW_EAP_SIM,			/* 18 */
	PW_EAP_SRP_SHA1,		/* 19 */
	PW_EAP_20,			/* 20 - unassigned */
	PW_EAP_TTLS,			/* 21 */
	PW_EAP_REMOTE_ACCESS_SERVICE,	/* 22 */
	PW_EAP_AKA,			/* 23 */
	PW_EAP_3COM,			/* 24 - should this be EAP-HP now? */
	PW_EAP_PEAP,			/* 25 */
	PW_EAP_MSCHAPV2,		/* 26 */
	PW_EAP_MAKE,			/* 27 */
	PW_EAP_CRYPTOCARD,		/* 28 */
	PW_EAP_CISCO_MSCHAPV2,		/* 29 */
	PW_EAP_DYNAMID,			/* 30 */
	PW_EAP_ROB,			/* 31 */
	PW_EAP_POTP,			/* 32 */
	PW_EAP_MS_ATLV,			/* 33 */
	PW_EAP_SENTRINET,		/* 34 */
	PW_EAP_ACTIONTEC,		/* 35 */
	PW_EAP_COGENT_BIOMETRIC,	/* 36 */
	PW_EAP_AIRFORTRESS,		/* 37 */
	PW_EAP_TNC,			/* 38 - fixme conflicts with HTTP DIGEST */
//	PW_EAP_HTTP_DIGEST,		/* 38 */
	PW_EAP_SECURISUITE,		/* 39 */
	PW_EAP_DEVICECONNECT,		/* 40 */
	PW_EAP_SPEKE,			/* 41 */
	PW_EAP_MOBAC,			/* 42 */
	PW_EAP_FAST,			/* 43 */
	PW_EAP_ZONELABS,		/* 44 */
	PW_EAP_LINK,			/* 45 */
	PW_EAP_PAX,			/* 46 */
	PW_EAP_PSK,			/* 47 */
	PW_EAP_SAKE,			/* 48 */
	PW_EAP_IKEV2,			/* 49 */
	PW_EAP_AKA2,			/* 50 */
	PW_EAP_GPSK,			/* 51 */
	PW_EAP_PWD,			/* 52 */
	PW_EAP_EKE,			/* 53 */
	PW_EAP_MAX_TYPES		/* 54 - for validation */
} eap_type_t;

typedef enum eap_rcode {
	EAP_NOTFOUND,    	//!< EAP handler data not found.
	EAP_FOUND,       	//!< EAP handler data found, continue.
	EAP_OK,		 	//!< Ok, continue.
	EAP_FAIL,		//!< Failed, don't reply.
	EAP_NOOP,		//!< Succeeded without doing anything.
	EAP_INVALID,     	//!< Invalid, don't reply.
	EAP_VALID,		//!< Valid, continue.
	EAP_MAX_RCODES
} eap_rcode_t;

extern const FR_NAME_NUMBER eap_rcode_table[];

/*
 * EAP-Type specific data.
 */
typedef struct eap_type_data {
	eap_type_t	num;
	size_t		length;
	uint8_t		*data;
} eap_type_data_t;

/*
 * Structure to hold EAP data.
 *
 * length = code + id + length + type + type.data
 *	=  1   +  1 +   2    +  1   +  X
 */
typedef struct eap_packet {
	eap_code_t	code;
	uint8_t		id;
	size_t		length;
	eap_type_data_t	type;

	uint8_t		*packet;
} eap_packet_t;

/*
 * Structure to represent packet format of eap *on wire*
 */
typedef struct eap_packet_raw {
	uint8_t		code;
	uint8_t		id;
	uint8_t		length[2];
	uint8_t		data[1];
} eap_packet_raw_t;


/*
 * interfaces in eapcommon.c
 */
extern eap_type_t eap_name2type(char const *name);
extern char const *eap_type2name(eap_type_t method);
extern int eap_wireformat(eap_packet_t *reply);
extern int eap_basic_compose(RADIUS_PACKET *packet, eap_packet_t *reply);
extern VALUE_PAIR *eap_packet2vp(RADIUS_PACKET *packet, eap_packet_raw_t const *reply);
extern eap_packet_raw_t *eap_vp2packet(TALLOC_CTX *ctx, VALUE_PAIR *vps);
void eap_add_reply(REQUEST *request,
		   char const *name, uint8_t const *value, int len);

#endif /* _EAP_TYPES_H */
