/* omsnmp.h
 * These are the definitions for the build-in MySQL output module.
 *
 * Copyright 2007-2012 Rainer Gerhards and Adiscon GmbH.
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
#ifndef	OMSNMP_H_INCLUDED
#define	OMSNMP_H_INCLUDED 1

#define OMSNMP_MAXTRANSPORLENGTH 10
#define OMSNMP_MAXPORTLENGHT 5
#define OMSNMP_MAXCOMMUNITYLENGHT 255
#define OMSNMP_MAXOIDLENGHT 255


#endif /* #ifndef OMMYSQL_H_INCLUDED */
/*
 * vi:set ai:
 */

#include <net-snmp/library/snmp_api.h>

static const char *api_errors[-SNMPERR_MAX + 1] = {
	"No error",                 /* SNMPERR_SUCCESS */
	"Generic error",            /* SNMPERR_GENERR */
	"Invalid local port",       /* SNMPERR_BAD_LOCPORT */
	"Unknown host",             /* SNMPERR_BAD_ADDRESS */
	"Unknown session",          /* SNMPERR_BAD_SESSION */
	"Too long",                 /* SNMPERR_TOO_LONG */
	"No socket",                /* SNMPERR_NO_SOCKET */
	"Cannot send V2 PDU on V1 session", /* SNMPERR_V2_IN_V1 */
	"Cannot send V1 PDU on V2 session", /* SNMPERR_V1_IN_V2 */
	"Bad value for non-repeaters",      /* SNMPERR_BAD_REPEATERS */
	"Bad value for max-repetitions",    /* SNMPERR_BAD_REPETITIONS */
	"Error building ASN.1 representation",      /* SNMPERR_BAD_ASN1_BUILD */
	"Failure in sendto",        /* SNMPERR_BAD_SENDTO */
	"Bad parse of ASN.1 type",  /* SNMPERR_BAD_PARSE */
	"Bad version specified",    /* SNMPERR_BAD_VERSION */
	"Bad source party specified",       /* SNMPERR_BAD_SRC_PARTY */
	"Bad destination party specified",  /* SNMPERR_BAD_DST_PARTY */
	"Bad context specified",    /* SNMPERR_BAD_CONTEXT */
	"Bad community specified",  /* SNMPERR_BAD_COMMUNITY */
	"Cannot send noAuth/Priv",       /* SNMPERR_NOAUTH_DESPRIV */
	"Bad ACL definition",       /* SNMPERR_BAD_ACL */
	"Bad Party definition",     /* SNMPERR_BAD_PARTY */
	"Session abort failure",    /* SNMPERR_ABORT */
	"Unknown PDU type",         /* SNMPERR_UNKNOWN_PDU */
	"Timeout",                  /* SNMPERR_TIMEOUT */
	"Failure in recvfrom",      /* SNMPERR_BAD_RECVFROM */
	"Unable to determine contextEngineID",      /* SNMPERR_BAD_ENG_ID */
	"No securityName specified",        /* SNMPERR_BAD_SEC_NAME */
	"Unable to determine securityLevel",        /* SNMPERR_BAD_SEC_LEVEL  */
	"ASN.1 parse error in message",     /* SNMPERR_ASN_PARSE_ERR */
	"Unknown security model in message",        /* SNMPERR_UNKNOWN_SEC_MODEL */
	"Invalid message (e.g. msgFlags)",  /* SNMPERR_INVALID_MSG */
	"Unknown engine ID",        /* SNMPERR_UNKNOWN_ENG_ID */
	"Unknown user name",        /* SNMPERR_UNKNOWN_USER_NAME */
	"Unsupported security level",       /* SNMPERR_UNSUPPORTED_SEC_LEVEL */
	"Authentication failure (incorrect password, community or key)",    /* SNMPERR_AUTHENTICATION_FAILURE */
	"Not in time window",       /* SNMPERR_NOT_IN_TIME_WINDOW */
	"Decryption error",         /* SNMPERR_DECRYPTION_ERR */
	"SCAPI general failure",    /* SNMPERR_SC_GENERAL_FAILURE */
	"SCAPI sub-system not configured",  /* SNMPERR_SC_NOT_CONFIGURED */
	"Key tools not available",  /* SNMPERR_KT_NOT_AVAILABLE */
	"Unknown Report message",   /* SNMPERR_UNKNOWN_REPORT */
	"USM generic error",        /* SNMPERR_USM_GENERICERROR */
	"USM unknown security name (no such user exists)",  /* SNMPERR_USM_UNKNOWNSECURITYNAME */
	"USM unsupported security level (this user has not been configured for that level of security)",
	/* SNMPERR_USM_UNSUPPORTEDSECURITYLEVEL */
	"USM encryption error",     /* SNMPERR_USM_ENCRYPTIONERROR */
	"USM authentication failure (incorrect password or key)",   /* SNMPERR_USM_AUTHENTICATIONFAILURE */
	"USM parse error",          /* SNMPERR_USM_PARSEERROR */
	"USM unknown engineID",     /* SNMPERR_USM_UNKNOWNENGINEID */
	"USM not in time window",   /* SNMPERR_USM_NOTINTIMEWINDOW */
	"USM decryption error",     /* SNMPERR_USM_DECRYPTIONERROR */
	"MIB not initialized",      /* SNMPERR_NOMIB */
	"Value out of range",       /* SNMPERR_RANGE */
	"Sub-id out of range",      /* SNMPERR_MAX_SUBID */
	"Bad sub-id in object identifier",  /* SNMPERR_BAD_SUBID */
	"Object identifier too long",       /* SNMPERR_LONG_OID */
	"Bad value name",           /* SNMPERR_BAD_NAME */
	"Bad value notation",       /* SNMPERR_VALUE */
	"Unknown Object Identifier",        /* SNMPERR_UNKNOWN_OBJID */
	"No PDU in snmp_send",      /* SNMPERR_NULL_PDU */
	"Missing variables in PDU", /* SNMPERR_NO_VARS */
	"Bad variable type",        /* SNMPERR_VAR_TYPE */
	"Out of memory (malloc failure)",   /* SNMPERR_MALLOC */
	"Kerberos related error",   /* SNMPERR_KRB5 */
};
