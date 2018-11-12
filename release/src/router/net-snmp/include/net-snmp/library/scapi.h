/*
 * scapi.h
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#ifndef _SCAPI_H
#define _SCAPI_H

#ifdef NETSNMP_USE_OPENSSL
#include <openssl/ossl_typ.h> /* EVP_MD */
#endif

#ifdef __cplusplus
extern          "C" {
#endif

    /*
     * Authentication/privacy transform bitlengths.
     */
#define SNMP_TRANS_AUTHLEN_HMACMD5	128
#define SNMP_TRANS_AUTHLEN_HMACSHA1	160
#define SNMP_TRANS_AUTHLEN_HMAC128SHA224   224 /* OPTIONAL */
#define SNMP_TRANS_AUTHLEN_HMAC192SHA256   256 /* MUST */
#define SNMP_TRANS_AUTHLEN_HMAC256SHA384   384 /* OPTIONAL */
#define SNMP_TRANS_AUTHLEN_HMAC384SHA512   512 /* SHOULD */

#define SNMP_TRANS_AUTHLEN_HMAC96	96

#define SNMP_TRANS_PRIVLEN_1DES		64
#define SNMP_TRANS_PRIVLEN_1DES_IV	64

#ifdef NETSNMP_DRAFT_REEDER_3DES
#define SNMP_TRANS_PRIVLEN_3DESEDE      256
#define SNMP_TRANS_PRIVLEN_3DESEDE_IV   64
#endif /* NETSNMP_DRAFT_REEDER_3DES */

#define SNMP_TRANS_PRIVLEN_AES          128
#define SNMP_TRANS_PRIVLEN_AES_IV       128

#define SNMP_TRANS_PRIVLEN_AES128       SNMP_TRANS_PRIVLEN_AES
#define SNMP_TRANS_PRIVLEN_AES128_IV    SNMP_TRANS_PRIVLEN_AES_IV

#define SNMP_TRANS_PRIVLEN_AES192       192
#define SNMP_TRANS_PRIVLEN_AES192_IV    128 /* 192 */

#define SNMP_TRANS_PRIVLEN_AES256       256
#define SNMP_TRANS_PRIVLEN_AES256_IV    128 /* 256 */


typedef struct netsnmp_auth_alg_info_s {
    int          type;
    const char * name;
    oid *        alg_oid;
    int          oid_len;
    int          proper_length;
    int          mac_length;
} netsnmp_auth_alg_info;

typedef struct netsnmp_priv_alg_info_s {
    int          type;
    const char * name;
    oid *        alg_oid;
    int          oid_len;
    int          proper_length;
    int         iv_length;
    int         pad_size;
#ifdef NETSNMP_USE_OPENSSL
    const EVP_CIPHER *  cipher;
#endif
} netsnmp_priv_alg_info;

    /*
     * Prototypes.
     */
    NETSNMP_IMPORT
    int             sc_get_authtype(const oid * hashoid, u_int hashoid_len);
    NETSNMP_IMPORT
    int             sc_get_proper_auth_length_bytype(int auth_type);
    NETSNMP_IMPORT
    int             sc_get_auth_maclen(int auth_type);
    NETSNMP_IMPORT
    const char*     sc_get_auth_name(int auth_type);
    NETSNMP_IMPORT
    oid *           sc_get_auth_oid(int auth_type, size_t *oid_len);
    NETSNMP_IMPORT
    netsnmp_auth_alg_info * sc_get_auth_alg_byoid(const oid *oid, u_int len);
    NETSNMP_IMPORT
    netsnmp_auth_alg_info * sc_get_auth_alg_bytype(u_int type);
    NETSNMP_IMPORT
    netsnmp_auth_alg_info * sc_get_auth_alg_byindex(u_int index);

    /** deprectated, use
     *        sc_get_authtype() + sc_get_proper_auth_length_bytype() */
    NETSNMP_IMPORT
    int             sc_get_properlength(const oid * hashtype,
                                        u_int hashtype_len);

#ifdef NETSNMP_USE_OPENSSL
    NETSNMP_IMPORT
    const EVP_MD *sc_get_openssl_hashfn(int auth_type);
    NETSNMP_IMPORT
    const EVP_CIPHER *sc_get_openssl_privfn(int priv_type);
#endif

    NETSNMP_IMPORT
    int             sc_get_privtype(const oid * privtype, u_int privtype_len);
    NETSNMP_IMPORT
    oid *           sc_get_priv_oid(int type, size_t *oid_len);
    NETSNMP_IMPORT
    int             sc_get_proper_priv_length(const oid * privtype,
                                              u_int privtype_len);
    NETSNMP_IMPORT
    int             sc_get_proper_priv_length_bytype(int privtype);
    NETSNMP_IMPORT
    netsnmp_priv_alg_info * sc_get_priv_alg_byoid(const oid *oid, u_int len);
    NETSNMP_IMPORT
    netsnmp_priv_alg_info * sc_get_priv_alg_bytype(u_int type);
    NETSNMP_IMPORT
    netsnmp_priv_alg_info * sc_get_priv_alg_byindex(u_int index);

    NETSNMP_IMPORT
    int             sc_init(void);
    NETSNMP_IMPORT
    int             sc_shutdown(int majorID, int minorID, void *serverarg,
                                void *clientarg);

    NETSNMP_IMPORT
    int             sc_random(u_char * buf, size_t * buflen);

    NETSNMP_IMPORT
    int             sc_generate_keyed_hash(const oid * authtype,
                                           size_t authtypelen,
                                           const u_char * key, u_int keylen,
                                           const u_char * message, u_int msglen,
                                           u_char * MAC, size_t * maclen);

    NETSNMP_IMPORT
    int             sc_check_keyed_hash(const oid * authtype,
                                        size_t authtypelen, const u_char * key,
                                        u_int keylen, const u_char * message,
                                        u_int msglen, const u_char * MAC,
                                        u_int maclen);

    NETSNMP_IMPORT
    int             sc_encrypt(const oid * privtype, size_t privtypelen,
                               u_char * key, u_int keylen,
                               u_char * iv, u_int ivlen,
                               const u_char * plaintext, u_int ptlen,
                               u_char * ciphertext, size_t * ctlen);

    NETSNMP_IMPORT
    int             sc_decrypt(const oid * privtype, size_t privtypelen,
                               u_char * key, u_int keylen,
                               u_char * iv, u_int ivlen,
                               u_char * ciphertext, u_int ctlen,
                               u_char * plaintext, size_t * ptlen);

    NETSNMP_IMPORT
    int             sc_hash_type(int auth_type, const u_char * buf,
                                 size_t buf_len, u_char * MAC,
                                 size_t * MAC_len);

    NETSNMP_IMPORT
    int             sc_hash(const oid * hashtype, size_t hashtypelen,
                            const u_char * buf, size_t buf_len,
                            u_char * MAC, size_t * MAC_len);

    NETSNMP_IMPORT
    int             sc_get_transform_type(oid * hashtype,
                                          u_int hashtype_len,
                                          int (**hash_fn) (const int mode,
                                                           void **context,
                                                           const u_char *
                                                           data,
                                                           const int
                                                           data_len,
                                                           u_char **
                                                           digest,
                                                           size_t *
                                                           digest_len));


    /*
     * All functions devolve to the following block if we can't do cryptography
     */
#define	_SCAPI_NOT_CONFIGURED					\
{								\
        snmp_log(LOG_ERR, "Encryption support not enabled.\n"); \
        DEBUGMSGTL(("scapi", "SCAPI not configured"));		\
	return SNMPERR_SC_NOT_CONFIGURED;			\
}

    /*
     * define a transform type if we're using the internal md5 support 
     */
#ifdef NETSNMP_USE_INTERNAL_MD5
#define INTERNAL_MD5 1
#endif

#ifdef __cplusplus
}
#endif
#endif                          /* _SCAPI_H */
