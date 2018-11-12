#ifndef _net_snmp_transform_oids_h
#define _net_snmp_transform_oids_h

#ifdef __cplusplus
extern          "C" {
#endif
/*
 * transform_oids.h
 *
 * Numeric MIB names for auth and priv transforms.
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

/** auth */
#define NETSNMP_USMAUTH_BASE_OID 1,3,6,1,6,3,10,1,1
#define NETSNMP_USMAUTH_NOAUTH            1
#define NETSNMP_USMAUTH_HMACMD5           2
#define NETSNMP_USMAUTH_HMACSHA1          3
#define NETSNMP_USMAUTH_HMACSHA           NETSNMP_USMAUTH_HMACSHA1
#define NETSNMP_USMAUTH_HMAC128SHA224     4 /* RFC 7860; OPTIONAL */
#define NETSNMP_USMAUTH_HMAC192SHA256     5 /* RFC 7860; MUST */
#define NETSNMP_USMAUTH_HMAC256SHA384     6 /* RFC 7860; OPTIONAL */
#define NETSNMP_USMAUTH_HMAC384SHA512     7 /* RFC 7860; SHOULD */

NETSNMP_IMPORT oid      usmNoAuthProtocol[10];

#ifndef NETSNMP_DISABLE_MD5
NETSNMP_IMPORT oid      usmHMACMD5AuthProtocol[10];
#endif

NETSNMP_IMPORT oid      usmHMACSHA1AuthProtocol[10];

NETSNMP_IMPORT oid      usmHMAC128SHA224AuthProtocol[10];
NETSNMP_IMPORT oid      usmHMAC192SHA256AuthProtocol[10];
NETSNMP_IMPORT oid      usmHMAC256SHA384AuthProtocol[10];
NETSNMP_IMPORT oid      usmHMAC384SHA512AuthProtocol[10];

/** priv */
NETSNMP_IMPORT oid      usmNoPrivProtocol[10];

#ifndef NETSNMP_DISABLE_DES
NETSNMP_IMPORT oid      usmDESPrivProtocol[10];
#endif

NETSNMP_IMPORT oid      usmAESPrivProtocol[10];
NETSNMP_IMPORT oid      *usmAES128PrivProtocol; /* backwards compat */

/*
 * backwards compatibility. should not be used in internal code.
 */
#define USM_AUTH_PROTO_NOAUTH_LEN 10
#define USM_AUTH_PROTO_MD5_LEN 10
#define USM_AUTH_PROTO_SHA_LEN 10
#define USM_PRIV_PROTO_NOPRIV_LEN 10
#define USM_PRIV_PROTO_DES_LEN 10

#define USM_PRIV_PROTO_AES_LEN 10
#define USM_PRIV_PROTO_AES128_LEN 10 /* backwards compat */

#ifdef NETSNMP_DRAFT_BLUMENTHAL_AES_04
NETSNMP_IMPORT oid      usmAES192PrivProtocol[9];
NETSNMP_IMPORT oid      usmAES256PrivProtocol[9];

NETSNMP_IMPORT oid      usmAES192CiscoPrivProtocol[11];
NETSNMP_IMPORT oid      usmAES256CiscoPrivProtocol[11];

NETSNMP_IMPORT oid      usmAES192Cisco2PrivProtocol[11];
NETSNMP_IMPORT oid      usmAES256Cisco2PrivProtocol[11];
#endif /* NETSNMP_DRAFT_BLUMENTHAL_AES_04 */

#ifdef __cplusplus
}
#endif
#endif
