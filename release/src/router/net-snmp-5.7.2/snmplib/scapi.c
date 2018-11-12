/* Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright © 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

/*
 * scapi.c
 *
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>

#include <sys/types.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/utilities.h>

netsnmp_feature_child_of(usm_support, libnetsnmp)
netsnmp_feature_child_of(usm_scapi, usm_support)

#ifndef NETSNMP_FEATURE_REMOVE_USM_SCAPI

#ifdef NETSNMP_USE_INTERNAL_MD5
#include <net-snmp/library/md5.h>
#endif
#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/callback.h>
#include <net-snmp/library/snmp_secmod.h>
#include <net-snmp/library/snmpusm.h>
#include <net-snmp/library/keytools.h>
#include <net-snmp/library/scapi.h>
#include <net-snmp/library/mib.h>
#include <net-snmp/library/transform_oids.h>

#ifdef NETSNMP_USE_INTERNAL_CRYPTO
#include <net-snmp/library/openssl_md5.h>
#include <net-snmp/library/openssl_sha.h>
#include <net-snmp/library/openssl_des.h>
#include <net-snmp/library/openssl_aes.h>
#endif

#ifdef NETSNMP_USE_OPENSSL
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/des.h>
#ifdef HAVE_AES
#include <openssl/aes.h>
#endif

#ifndef NETSNMP_DISABLE_DES
#ifdef HAVE_STRUCT_DES_KS_STRUCT_WEAK_KEY
/* these are older names for newer structures that exist in openssl .9.7 */
#define DES_key_schedule    des_key_schedule 
#define DES_cblock          des_cblock 
#define DES_key_sched       des_key_sched 
#define DES_ncbc_encrypt    des_ncbc_encrypt
#define DES_cbc_encrypt    des_cbc_encrypt
#define OLD_DES
#endif
#endif

#endif /* HAVE_OPENSSL */

#ifdef NETSNMP_USE_INTERNAL_CRYPTO
#endif

#ifdef NETSNMP_USE_PKCS11
#include <security/cryptoki.h>
#endif

#ifdef QUITFUN
#undef QUITFUN
#define QUITFUN(e, l)					\
	if (e != SNMPERR_SUCCESS) {			\
		rval = SNMPERR_SC_GENERAL_FAILURE;	\
		goto l ;				\
	}
#endif

#ifdef NETSNMP_USE_INTERNAL_CRYPTO
static
int SHA1_hmac(const u_char * data, size_t len, u_char * mac, size_t maclen,
              const u_char * secret, size_t secretlen);

static
int MD5_hmac(const u_char * data, size_t len, u_char * mac, size_t maclen,
             const u_char * secret, size_t secretlen);
#endif

static netsnmp_auth_alg_info _auth_alg_info[] = {
    { NETSNMP_USMAUTH_NOAUTH, "usmNoAuthProtocol", usmNoAuthProtocol,
      OID_LENGTH(usmNoAuthProtocol), 0, 0 },
    { NETSNMP_USMAUTH_HMACSHA1, "usmHMACSHA1AuthProtocol",
      usmHMACSHA1AuthProtocol, OID_LENGTH(usmHMACSHA1AuthProtocol),
      BYTESIZE(SNMP_TRANS_AUTHLEN_HMACSHA1), USM_MD5_AND_SHA_AUTH_LEN },
#ifndef NETSNMP_DISABLE_MD5
    { NETSNMP_USMAUTH_HMACMD5, "usmHMACMD5AuthProtocol",
      usmHMACMD5AuthProtocol, OID_LENGTH(usmHMACMD5AuthProtocol),
      BYTESIZE(SNMP_TRANS_AUTHLEN_HMACMD5), USM_MD5_AND_SHA_AUTH_LEN },
#endif
#ifdef HAVE_EVP_SHA224
    { NETSNMP_USMAUTH_HMAC128SHA224, "usmHMAC128SHA224AuthProtocol",
      usmHMAC128SHA224AuthProtocol, OID_LENGTH(usmHMAC128SHA224AuthProtocol),
      BYTESIZE(SNMP_TRANS_AUTHLEN_HMAC128SHA224), USM_HMAC128SHA224_AUTH_LEN },
    { NETSNMP_USMAUTH_HMAC192SHA256, "usmHMAC192SHA256AuthProtocol",
      usmHMAC192SHA256AuthProtocol, OID_LENGTH(usmHMAC192SHA256AuthProtocol),
      BYTESIZE(SNMP_TRANS_AUTHLEN_HMAC192SHA256), USM_HMAC192SHA256_AUTH_LEN },
#endif
#ifdef HAVE_EVP_SHA384
    { NETSNMP_USMAUTH_HMAC256SHA384, "usmHMAC256SHA384AuthProtocol",
      usmHMAC256SHA384AuthProtocol, OID_LENGTH(usmHMAC256SHA384AuthProtocol),
      BYTESIZE(SNMP_TRANS_AUTHLEN_HMAC256SHA384), USM_HMAC256SHA384_AUTH_LEN },
    { NETSNMP_USMAUTH_HMAC384SHA512, "usmHMAC384SHA512AuthProtocol",
      usmHMAC384SHA512AuthProtocol, OID_LENGTH(usmHMAC384SHA512AuthProtocol),
      BYTESIZE(SNMP_TRANS_AUTHLEN_HMAC384SHA512), USM_HMAC384SHA512_AUTH_LEN },
#endif
    { -1, "unknown", NULL, 0, 0, 0 }
};

static netsnmp_priv_alg_info _priv_alg_info[] = {
    { USM_CREATE_USER_PRIV_NONE, "usmNoPrivProtocol",
      usmNoPrivProtocol, OID_LENGTH(usmNoPrivProtocol), 0, 0, 0 },
#ifndef NETSNMP_DISABLE_DES
    { USM_CREATE_USER_PRIV_DES, "usmDESPrivProtocol",
      usmDESPrivProtocol, OID_LENGTH(usmDESPrivProtocol),
      BYTESIZE(SNMP_TRANS_PRIVLEN_1DES),
      BYTESIZE(SNMP_TRANS_PRIVLEN_1DES_IV),
      BYTESIZE(SNMP_TRANS_PRIVLEN_1DES) },
#endif
#ifdef HAVE_AES
    { USM_CREATE_USER_PRIV_AES, "usmAESPrivProtocol",
      usmAESPrivProtocol, OID_LENGTH(usmAESPrivProtocol),
      BYTESIZE(SNMP_TRANS_PRIVLEN_AES),
      BYTESIZE(SNMP_TRANS_PRIVLEN_AES_IV),
      0 },
#ifdef NETSNMP_DRAFT_BLUMENTHAL_AES_04
    { USM_CREATE_USER_PRIV_AES192, "usmAES192PrivProtocol",
      usmAES192PrivProtocol, OID_LENGTH(usmAES192PrivProtocol),
      BYTESIZE(SNMP_TRANS_PRIVLEN_AES192),
      BYTESIZE(SNMP_TRANS_PRIVLEN_AES192_IV),
      0 },
    { USM_CREATE_USER_PRIV_AES192_CISCO, "usmAES192CiscoPrivProtocol",
      usmAES192CiscoPrivProtocol, OID_LENGTH(usmAES192CiscoPrivProtocol),
      BYTESIZE(SNMP_TRANS_PRIVLEN_AES192),
      BYTESIZE(SNMP_TRANS_PRIVLEN_AES192_IV),
      0 },
    { USM_CREATE_USER_PRIV_AES256, "usmAES256PrivProtocol",
      usmAES256PrivProtocol, OID_LENGTH(usmAES256PrivProtocol),
      BYTESIZE(SNMP_TRANS_PRIVLEN_AES256),
      BYTESIZE(SNMP_TRANS_PRIVLEN_AES256_IV),
      0 },
    { USM_CREATE_USER_PRIV_AES256_CISCO, "usmAES256CiscoPrivProtocol",
      usmAES256CiscoPrivProtocol, OID_LENGTH(usmAES256CiscoPrivProtocol),
      BYTESIZE(SNMP_TRANS_PRIVLEN_AES256),
      BYTESIZE(SNMP_TRANS_PRIVLEN_AES256_IV),
      0 },
#endif
#endif
    { -1, NULL, NULL, 0 , 0, 0, 0 },
};


/*
 * sc_get_priv_alg(oid *privoid, u_int len)
 *
 * returns a pointer to a netsnmp_priv_alg_info struct
 */
netsnmp_priv_alg_info *
sc_get_priv_alg_byoid(const oid *privoid, u_int len)
{
    int i = 0;

    DEBUGTRACE;

    if ((NULL == privoid) || (0 == len))
        return NULL;

    for( ; _priv_alg_info[i].type != -1; ++i) {
        if (len != _priv_alg_info[i].oid_len)
            continue;
        if (snmp_oid_compare(_priv_alg_info[i].alg_oid,
                             _priv_alg_info[i].oid_len,
                             privoid, len) == 0 )
            return(&_priv_alg_info[i]);
    }

    DEBUGMSGTL(("scapi", "no match for OID "));
    DEBUGMSGOID(("scapi", privoid, len));
    DEBUGMSG(("scapi", "\n"));
    return NULL;
}

/*
 * sc_get_priv_alg_bytype(u_int type)
 *
 * returns a pointer to a netsnmp_priv_alg_info struct
 */
netsnmp_priv_alg_info *
sc_get_priv_alg_bytype(u_int type)
{
    int i = 0;

    DEBUGTRACE;

    for( ; _priv_alg_info[i].type != -1; ++i) {
        if (type != _priv_alg_info[i].type)
            continue;
        return(&_priv_alg_info[i]);
    }

    return NULL;
}

/*
 * sc_find_auth_alg(oid *authoid, u_int len)
 *
 * returns a pointer to a netsnmp_auth_alg_info struct
 */
netsnmp_auth_alg_info *
sc_find_auth_alg_byoid(const oid *authoid, u_int len)
{
    int i = 0;

    DEBUGTRACE;

    if ((NULL == authoid) || (0 == len))
        return NULL;

    for( ; _auth_alg_info[i].type != -1; ++i) {
        if (len != _auth_alg_info[i].oid_len)
            continue;
        if (snmp_oid_compare(_auth_alg_info[i].alg_oid,
                             _auth_alg_info[i].oid_len,
                             authoid, len) == 0 )
            return(&_auth_alg_info[i]);
    }

/*    DEBUGMSGTL(("scapi", "No auth alg found for"));
      DEBUGMSGOID(("scapi", authoid, len ));*/

    return NULL;
}

/*
 * sc_get_auth_alg_byindex(u_int index)
 *
 * returns a pointer to a netsnmp_auth_alg_info struct
 */
netsnmp_auth_alg_info *
sc_get_auth_alg_byindex(u_int index)
{
    DEBUGTRACE;

    if (index >= (sizeof(_auth_alg_info)) / (sizeof(_auth_alg_info[0])) ||
        -1 == _auth_alg_info[index].type)
        return NULL;

    return(&_auth_alg_info[index]);
}

/*
 * sc_find_auth_alg_bytype(u_int type)
 *
 * returns a pointer to a netsnmp_auth_alg_info struct
 */
netsnmp_auth_alg_info *
sc_find_auth_alg_bytype(u_int type)
{
    int i = 0;

    DEBUGTRACE;

    for( ; _auth_alg_info[i].type != -1; ++i) {
        if (type != _auth_alg_info[i].type)
            continue;
        return(&_auth_alg_info[i]);
    }

    return NULL;
}

/*
 * sc_get_authtype(oid *hashtype, u_int hashtype_len):
 * 
 * Given a hashing type ("hashtype" and its length hashtype_len), return
 * its type (the last suboid). NETSNMP_USMAUTH_* constants are defined in
 * transform_oids.h.
 * 
 * Returns SNMPERR_GENERR for an unknown hashing type.
 */
int
sc_get_authtype(const oid * hashtype, u_int hashtype_len)
{
    netsnmp_auth_alg_info *aai;

    DEBUGTRACE;

    aai = sc_find_auth_alg_byoid(hashtype, hashtype_len);
    if (NULL == aai)
        return SNMPERR_GENERR;

    return aai->type;
}

int
sc_get_privtype(const oid * privtype, u_int privtype_len)
{
    netsnmp_priv_alg_info *pai;

    DEBUGTRACE;

    pai = sc_get_priv_alg_byoid(privtype, privtype_len);
    if (NULL == pai)
        return SNMPERR_GENERR;

    return pai->type;
}

/*
 * sc_get_auth_maclen(int hashtype):
 *
 * Given a hash type, return its MAC length, which may be shorter than
 * the full hash length.
 *
 * Returns 0 for an unknown hash type.
 */
int
sc_get_auth_maclen(int hashtype)
{
    netsnmp_auth_alg_info *aai;

    DEBUGTRACE;

    aai = sc_find_auth_alg_bytype(hashtype);
    if (NULL == aai)
        return 0;

    return aai->mac_length;
}

/*
 * sc_get_proper_auth_length_bytype(int hashtype):
 * 
 * Given a hashing type, return the length of the hash result.
 * 
 * Returns either the length or SNMPERR_GENERR for an unknown hashing type.
 */
int
sc_get_proper_auth_length_bytype(int hashtype)
{
    netsnmp_auth_alg_info *aai;

    DEBUGTRACE;

    aai = sc_find_auth_alg_bytype(hashtype);
    if (NULL == aai)
        return SNMPERR_GENERR;

    return aai->proper_length;
}

/*
 * sc_get_auth_oid(int hashtype, int *oid_len):
 * 
 * Given a type, return the OID and optionally set OID length.
 */
oid *
sc_get_auth_oid(int type, size_t *oid_len)
{
    netsnmp_auth_alg_info *ai;

    DEBUGTRACE;

    ai = sc_find_auth_alg_bytype(type);
    if (NULL == ai)
        return NULL;

    if (NULL != oid_len)
        *oid_len = ai->oid_len;

    return ai->alg_oid;
}

/*
 * sc_get_auth_name(int hashtype):
 * 
 * Given a type, return the name string
 */
const char*
sc_get_auth_name(int type)
{
    netsnmp_auth_alg_info *ai;

    DEBUGTRACE;

    ai = sc_find_auth_alg_bytype(type);
    if (NULL == ai)
        return NULL;

    return ai->name;
}

/*
 * sc_get_priv_oid(int type, int *oid_len):
 * 
 * Given a type, return the OID and optionally set OID length.
 */
oid *
sc_get_priv_oid(int type, size_t *oid_len)
{
    netsnmp_priv_alg_info *ai;

    DEBUGTRACE;

    ai = sc_get_priv_alg_bytype(type);
    if (NULL == ai)
        return NULL;

    if (NULL != oid_len)
        *oid_len = ai->oid_len;

    return ai->alg_oid;
}

/*
 * sc_get_properlength(oid *hashtype, u_int hashtype_len):
 * 
 * Given a hashing type ("hashtype" and its length hashtype_len), return
 * the length of the hash result.
 * 
 * Returns either the length or SNMPERR_GENERR for an unknown hashing type.
 */
int
sc_get_properlength(const oid * hashtype, u_int hashtype_len)
{
    DEBUGTRACE;
    /*
     * Determine transform type hash length.
     */
    return sc_get_proper_auth_length_bytype(
        sc_get_authtype(hashtype, hashtype_len));
}

netsnmp_feature_child_of(scapi_get_proper_priv_length, netsnmp_unused)
#ifndef NETSNMP_FEATURE_REMOVE_SCAPI_GET_PROPER_PRIV_LENGTH
int
sc_get_proper_priv_length(const oid * privtype, u_int privtype_len)
{
    netsnmp_priv_alg_info *pai;

    DEBUGTRACE;

    pai = sc_get_priv_alg_byoid(privtype, privtype_len);
    if (NULL == pai)
        return 0;

    return pai->proper_length;
}
#endif /* NETSNMP_FEATURE_REMOVE_SCAPI_GET_PROPER_PRIV_LENGTH */

/*
 * sc_get_priv_alg_byindex(u_int index)
 *
 * returns a pointer to a netsnmp_priv_alg_info struct
 */
netsnmp_priv_alg_info *
sc_get_priv_alg_byindex(u_int index)
{
    DEBUGTRACE;

    if (index >= (sizeof(_priv_alg_info)) / (sizeof(_priv_alg_info[0])) ||
        -1 == _priv_alg_info[index].type)
        return NULL;

    return(&_priv_alg_info[index]);
}

int
sc_get_proper_priv_length_bytype(int privtype)
{
    netsnmp_priv_alg_info *pai;

    DEBUGTRACE;

    pai = sc_get_priv_alg_bytype(privtype);
    if (NULL == pai)
        return 0;

    return pai->proper_length;
}


/*******************************************************************-o-******
 * sc_init
 *
 * Returns:
 *	SNMPERR_SUCCESS			Success.
 */
int
sc_init(void)
{
    int             rval = SNMPERR_SUCCESS;

#if !defined(NETSNMP_USE_OPENSSL)
#if defined(NETSNMP_USE_INTERNAL_MD5) || defined(NETSNMP_USE_INTERNAL_CRYPTO)
    struct timeval  tv;

    DEBUGTRACE;

    gettimeofday(&tv, (struct timezone *) 0);

    netsnmp_srandom((unsigned)(tv.tv_sec ^ tv.tv_usec));
#elif NETSNMP_USE_PKCS11
    DEBUGTRACE;
    rval = pkcs_init();
#else
    rval = SNMPERR_SC_NOT_CONFIGURED;
#endif                           /* NETSNMP_USE_INTERNAL_MD5 */
    /*
     * XXX ogud: The only reason to do anything here with openssl is to 
     * * XXX ogud: seed random number generator 
     */
#endif                          /* ifndef NETSNMP_USE_OPENSSL */

    return rval;
}                               /* end sc_init() */

/*******************************************************************-o-******
 * sc_random
 *
 * Parameters:
 *	*buf		Pre-allocated buffer.
 *	*buflen 	Size of buffer.
 *      
 * Returns:
 *	SNMPERR_SUCCESS			Success.
 */
int
sc_random(u_char * buf, size_t * buflen)
#if defined(NETSNMP_USE_INTERNAL_MD5) || defined(NETSNMP_USE_OPENSSL) || defined(NETSNMP_USE_PKCS11) || defined(NETSNMP_USE_INTERNAL_CRYPTO)
{
    int             rval = SNMPERR_SUCCESS;
#if !defined(NETSNMP_USE_OPENSSL) && !defined(NETSNMP_USE_PKCS11)
    int             i;
    int             rndval;
    u_char         *ucp = buf;
#endif

    DEBUGTRACE;

#ifdef NETSNMP_USE_OPENSSL
    RAND_bytes(buf, *buflen);   /* will never fail */
#elif NETSNMP_USE_PKCS11			/* NETSNMP_USE_PKCS11 */
    pkcs_random(buf, *buflen);
#else                           /* NETSNMP_USE_INTERNAL_MD5 */
    /*
     * fill the buffer with random integers.  Note that random()
     * is defined in config.h and may not be truly the random()
     * system call if something better existed 
     */
    rval = *buflen - *buflen % sizeof(rndval);
    for (i = 0; i < rval; i += sizeof(rndval)) {
        rndval = netsnmp_random();
        memcpy(ucp, &rndval, sizeof(rndval));
        ucp += sizeof(rndval);
    }

    rndval = netsnmp_random();
    memcpy(ucp, &rndval, *buflen % sizeof(rndval));

    rval = SNMPERR_SUCCESS;
#endif                          /* NETSNMP_USE_OPENSSL */
    return rval;

}                               /* end sc_random() */

#else
_SCAPI_NOT_CONFIGURED
#endif                          /*  */


#ifdef NETSNMP_USE_OPENSSL
const EVP_MD   *
sc_get_openssl_hashfn(int auth_type)
{
    const EVP_MD   *hashfn = NULL;

    DEBUGTRACE;

    switch (auth_type) {
#ifndef NETSNMP_DISABLE_MD5
        case NETSNMP_USMAUTH_HMACMD5:
            hashfn = (const EVP_MD *) EVP_md5();
            break;
#endif
        case NETSNMP_USMAUTH_HMACSHA1:
            hashfn = (const EVP_MD *) EVP_sha1();
            break;

#ifdef HAVE_EVP_SHA224
        case NETSNMP_USMAUTH_HMAC128SHA224:
            hashfn = (const EVP_MD *) EVP_sha224();
            break;

        case NETSNMP_USMAUTH_HMAC192SHA256:
            hashfn = (const EVP_MD *) EVP_sha256();
            break;
#endif /* HAVE_EVP_SHA224 */

#ifdef HAVE_EVP_SHA384
        case NETSNMP_USMAUTH_HMAC256SHA384:
            hashfn = (const EVP_MD *) EVP_sha384();
            break;

        case NETSNMP_USMAUTH_HMAC384SHA512:
            hashfn = (const EVP_MD *) EVP_sha512();
            break;
#endif /* HAVE_EVP_SHA384 */
    }

    return hashfn;
}

const EVP_CIPHER *
sc_get_openssl_privfn(int priv_type)
{
    const EVP_CIPHER *fn = NULL;

    DEBUGTRACE;

    switch(priv_type & (USM_PRIV_MASK_ALG | USM_PRIV_MASK_VARIANT)) {
#ifdef HAVE_AES
        case USM_CREATE_USER_PRIV_AES:
            fn = (const EVP_CIPHER *)EVP_aes_128_cfb();
            break;
#ifdef NETSNMP_DRAFT_BLUMENTHAL_AES_04
        case USM_CREATE_USER_PRIV_AES192:
            fn = (const void*)EVP_aes_192_cfb();
            break;
        case USM_CREATE_USER_PRIV_AES256:
            fn = (const void*)EVP_aes_256_cfb();
            break;
#endif
#endif /* HAVE_AES */
    }

    return fn;
}
#endif /* openssl */


/*******************************************************************-o-******
 * sc_generate_keyed_hash
 *
 * Parameters:
 *	 authtype	Type of authentication transform.
 *	 authtypelen
 *	*key		Pointer to key (Kul) to use in keyed hash.
 *	 keylen		Length of key in bytes.
 *	*message	Pointer to the message to hash.
 *	 msglen		Length of the message.
 *	*MAC		Will be returned with allocated bytes containg hash.
 *	*maclen		Length of the hash buffer in bytes; also indicates
 *				whether the MAC should be truncated.
 *      
 * Returns:
 *	SNMPERR_SUCCESS			Success.
 *	SNMPERR_GENERR			All errs
 *
 *
 * A hash of the first msglen bytes of message using a keyed hash defined
 * by authtype is created and stored in MAC.  MAC is ASSUMED to be a buffer
 * of at least maclen bytes.  If the length of the hash is greater than
 * maclen, it is truncated to fit the buffer.  If the length of the hash is
 * less than maclen, maclen set to the number of hash bytes generated.
 *
 * ASSUMED that the number of hash bits is a multiple of 8.
 */
int
sc_generate_keyed_hash(const oid * authtypeOID, size_t authtypeOIDlen,
                       const u_char * key, u_int keylen,
                       const u_char * message, u_int msglen,
                       u_char * MAC, size_t * maclen)
#if  defined(NETSNMP_USE_INTERNAL_MD5) || defined(NETSNMP_USE_OPENSSL) || defined(NETSNMP_USE_PKCS11) || defined(NETSNMP_USE_INTERNAL_CRYPTO)
{
    int             rval = SNMPERR_SUCCESS, auth_type;
    int             iproperlength;
    size_t          properlength;
    u_char          buf[SNMP_MAXBUF_SMALL];
#if  defined(NETSNMP_USE_OPENSSL) || defined(NETSNMP_USE_PKCS11)
    unsigned int    buf_len = sizeof(buf);
#endif
#ifdef NETSNMP_USE_OPENSSL
    const EVP_MD   *hashfn;
#elif defined(NETSNMP_USE_PKCS11)
    u_long          ck_type;
#endif

    DEBUGTRACE;

#ifdef NETSNMP_ENABLE_TESTING_CODE
    {
        int             i;
        DEBUGMSG(("sc_generate_keyed_hash",
                  "sc_generate_keyed_hash(): key=0x"));
        for (i = 0; i < keylen; i++)
            DEBUGMSG(("sc_generate_keyed_hash", "%02x", key[i] & 0xff));
        DEBUGMSG(("sc_generate_keyed_hash", " (%d)\n", keylen));
    }
#endif                          /* NETSNMP_ENABLE_TESTING_CODE */

    /*
     * Sanity check.
     */
    if (!authtypeOID || !key || !message || !MAC || !maclen
        || (keylen <= 0) || (msglen <= 0) || (*maclen <= 0)) {
        QUITFUN(SNMPERR_GENERR, sc_generate_keyed_hash_quit);
    }

    auth_type = sc_get_authtype(authtypeOID, authtypeOIDlen);
    iproperlength = sc_get_auth_maclen(auth_type);
    if (iproperlength == SNMPERR_GENERR)
        return SNMPERR_GENERR;
    properlength = (size_t)iproperlength;
    if (keylen < properlength) {
        QUITFUN(SNMPERR_GENERR, sc_generate_keyed_hash_quit);
    }
#ifdef NETSNMP_ENABLE_TESTING_CODE
    DEBUGMSGTL(("scapi", "iproperlength: %d, maclen:%" NETSNMP_PRIz "d\n", iproperlength,
                *maclen));
#endif
#ifdef NETSNMP_USE_OPENSSL
    /** get hash function */
    hashfn = sc_get_openssl_hashfn(auth_type);
    if (NULL == hashfn) {
        QUITFUN(SNMPERR_GENERR, sc_generate_keyed_hash_quit);
    }

    HMAC(hashfn, key, keylen, message, msglen, buf, &buf_len);
    if (buf_len != properlength) {
        QUITFUN(rval, sc_generate_keyed_hash_quit);
    }
    if (*maclen > buf_len)
        *maclen = buf_len;
    memcpy(MAC, buf, *maclen);

#elif NETSNMP_USE_PKCS11                    /* NETSNMP_USE_PKCS11 */

#ifndef NETSNMP_DISABLE_MD5
    if (NETSNMP_USMAUTH_HMACMD5 == auth_type) {
	if (pkcs_sign(CKM_MD5_HMAC,key, keylen, message,
			msglen, buf, &buf_len) != SNMPERR_SUCCESS) {
            QUITFUN(SNMPERR_GENERR, sc_generate_keyed_hash_quit);
        }
    } else
#endif
        if (NETSNMP_USMAUTH_HMACSHA1 == auth_type) {
	if (pkcs_sign(CKM_SHA_1_HMAC,key, keylen, message,
			msglen, buf, &buf_len) != SNMPERR_SUCCESS) {
            QUITFUN(SNMPERR_GENERR, sc_generate_keyed_hash_quit);
        }
    } else {
        QUITFUN(SNMPERR_GENERR, sc_generate_keyed_hash_quit);
    }

    if (buf_len != properlength) {
        QUITFUN(rval, sc_generate_keyed_hash_quit);
    }
    if (*maclen > buf_len)
        *maclen = buf_len;
    memcpy(MAC, buf, *maclen);

#elif NETSNMP_USE_INTERNAL_CRYPTO
    if (*maclen > properlength)
        *maclen = properlength;
#ifndef NETSNMP_DISABLE_MD5
    if (NETSNMP_USMAUTH_HMACMD5 == auth_type)
        rval = MD5_hmac(message, msglen, MAC, *maclen, key, keylen);
    else
#endif
         if (NETSNMP_USMAUTH_HMACSHA1 == auth_type)
        rval = SHA1_hmac(message, msglen, MAC, *maclen, key, keylen);
    else {
        QUITFUN(SNMPERR_GENERR, sc_generate_keyed_hash_quit);
    }
    if (rval != 0) {
        rval = SNMPERR_GENERR;
        goto sc_generate_keyed_hash_quit;
    }    
#else                            /* NETSNMP_USE_INTERNAL_MD5 */
    if (*maclen > properlength)
        *maclen = properlength;
    if (MDsign(message, msglen, MAC, *maclen, key, keylen)) {
        rval = SNMPERR_GENERR;
        goto sc_generate_keyed_hash_quit;
    }
#endif                          /* NETSNMP_USE_OPENSSL */

#ifdef NETSNMP_ENABLE_TESTING_CODE
    {
        char           *s;
        int             len = binary_to_hex(MAC, *maclen, &s);

        DEBUGMSGTL(("scapi", "Full v3 message hash: %s\n", s));
        SNMP_ZERO(s, len);
        SNMP_FREE(s);
    }
#endif                          /* NETSNMP_ENABLE_TESTING_CODE */

  sc_generate_keyed_hash_quit:
    memset(buf, 0, SNMP_MAXBUF_SMALL);
    return rval;
}                               /* end sc_generate_keyed_hash() */

#else
                _SCAPI_NOT_CONFIGURED
#endif                          /* */
/*******************************************************************-o-******
 * sc_hash(): a generic wrapper around whatever hashing package we are using.
 * 
 * IN:
 * hashtype    - oid pointer to a hash type
 * hashtypelen - length of oid pointer
 * buf         - u_char buffer to be hashed
 * buf_len     - integer length of buf data
 * MAC_len     - length of the passed MAC buffer size.
 * 
 * OUT:    
 * MAC         - pre-malloced space to store hash output.
 * MAC_len     - length of MAC output to the MAC buffer.
 * 
 * Returns:
 * SNMPERR_SUCCESS              Success.
 * SNMP_SC_GENERAL_FAILURE      Any error.
 * SNMPERR_SC_NOT_CONFIGURED    Hash type not supported.
 */
int
sc_hash(const oid * hashtype, size_t hashtypelen, const u_char * buf,
        size_t buf_len, u_char * MAC, size_t * MAC_len)
{
    int auth_type;

    DEBUGTRACE;

    if (hashtype == NULL)
        return (SNMPERR_GENERR);

    auth_type = sc_get_authtype(hashtype, hashtypelen);
    if (auth_type < 0 )
        return (SNMPERR_GENERR);

    return sc_hash_type(auth_type, buf, buf_len, MAC, MAC_len);
}

/*******************************************************************-o-******
 * sc_hash_type():
 *    a generic wrapper around whatever hashing package we are using.
 *
 * IN:
 * hashtype    - oid pointer to a hash type
 * hashtypelen - length of oid pointer
 * buf         - u_char buffer to be hashed
 * buf_len     - integer length of buf data
 * MAC_len     - length of the passed MAC buffer size.
 *
 * OUT:
 * MAC         - pre-malloced space to store hash output.
 * MAC_len     - length of MAC output to the MAC buffer.
 *
 * Returns:
 * SNMPERR_SUCCESS              Success.
 * SNMP_SC_GENERAL_FAILURE      Any error.
 * SNMPERR_SC_NOT_CONFIGURED    Hash type not supported.
 */
int
sc_hash_type(int auth_type, const u_char * buf, size_t buf_len, u_char * MAC,
             size_t * MAC_len)
#if defined(NETSNMP_USE_INTERNAL_MD5) || defined(NETSNMP_USE_OPENSSL) || defined(NETSNMP_USE_PKCS11) || defined(NETSNMP_USE_INTERNAL_CRYPTO)
{
#if defined(NETSNMP_USE_OPENSSL) || defined(NETSNMP_USE_PKCS11) || defined(NETSNMP_USE_INTERNAL_CRYPTO)
    int            rval = SNMPERR_SUCCESS;
#endif
#if defined(NETSNMP_USE_OPENSSL) || defined(NETSNMP_USE_PKCS11)
    unsigned int   tmp_len;
#endif
    int            ret;

#ifdef NETSNMP_USE_OPENSSL
    const EVP_MD   *hashfn;
    EVP_MD_CTX     *cptr;
#endif
#ifdef NETSNMP_USE_INTERNAL_CRYPTO
    MD5_CTX        cmd5;
    SHA_CTX        csha1;
#endif
    DEBUGTRACE;

    if (buf == NULL || buf_len <= 0 || MAC == NULL || MAC_len == NULL )
        return (SNMPERR_GENERR);

    ret = sc_get_proper_auth_length_bytype(auth_type);
    if (( ret < 0 ) || (*MAC_len < (size_t)ret ))
        return (SNMPERR_GENERR);

#ifdef NETSNMP_USE_OPENSSL
    /*
     * Determine transform type.
     */
    hashfn = sc_get_openssl_hashfn(auth_type);
    if (NULL == hashfn)
        return SNMPERR_GENERR;

/** initialize the pointer */
#if defined(HAVE_EVP_MD_CTX_NEW)
    cptr = EVP_MD_CTX_new();
#elif defined(HAVE_EVP_MD_CTX_CREATE)
    cptr = EVP_MD_CTX_create();
#else
    cptr = malloc(sizeof(*cptr));
#if defined(OLD_DES)
    memset(cptr, 0, sizeof(*cptr));
#else
    EVP_MD_CTX_init(cptr);
#endif
#endif
    if (!EVP_DigestInit(cptr, hashfn)) {
        /* requested hash function is not available */
        return SNMPERR_SC_NOT_CONFIGURED;
    }

/** pass the data */
    EVP_DigestUpdate(cptr, buf, buf_len);

/** do the final pass */
    EVP_DigestFinal(cptr, MAC, &tmp_len);
    *MAC_len = tmp_len;
#if defined(HAVE_EVP_MD_CTX_FREE)
    EVP_MD_CTX_free(cptr);
#elif defined(HAVE_EVP_MD_CTX_DESTROY)
    EVP_MD_CTX_destroy(cptr);
#else
#if !defined(OLD_DES)
    EVP_MD_CTX_cleanup(cptr);
#endif
    free(cptr);
#endif
    return (rval);

#elif NETSNMP_USE_INTERNAL_CRYPTO
#ifndef NETSNMP_DISABLE_MD5
    if (NETSNMP_USMAUTH_HMACMD5 == auth_type) {
        if (*MAC_len < MD5_DIGEST_LENGTH)
            return (SNMPERR_GENERR);      /* the buffer isn't big enough */
        MD5_Init(&cmd5);
        MD5_Update(&cmd5, buf, buf_len);
        MD5_Final(MAC, &cmd5);
        *MAC_len = MD5_DIGEST_LENGTH;
    } else 
#endif
    if (NETSNMP_USMAUTH_HMACSHA1 == auth_type) {
        if (*MAC_len < SHA_DIGEST_LENGTH)
            return (SNMPERR_GENERR);      /* the buffer isn't big enough */
        SHA1_Init(&csha1);
        SHA1_Update(&csha1, buf, buf_len);
        SHA1_Final(MAC, &csha1);
        *MAC_len = SHA_DIGEST_LENGTH;
            
    } else {
        return (SNMPERR_GENERR);
    }
    return (rval);
#elif NETSNMP_USE_PKCS11                  /* NETSNMP_USE_PKCS11 */

#ifndef NETSNMP_DISABLE_MD5
    if (NETSNMP_USMAUTH_HMACMD5 == auth_type) {
        rval = pkcs_digest(CKM_MD5, buf, buf_len, MAC, &tmp_len);
        *MAC_len = tmp_len;
    } else
#endif
    if (NETSNMP_USMAUTH_HMACSHA1 == auth_type) {
       rval = pkcs_digest(CKM_SHA_1, buf, buf_len, MAC, &tmp_len);
        *MAC_len = tmp_len;
    } else {
        return (SNMPERR_GENERR);
    }

     return (rval);

#else                           /* NETSNMP_USE_INTERNAL_MD5 */

    if (MDchecksum(buf, buf_len, MAC, *MAC_len)) {
        return SNMPERR_GENERR;
    }
    if (*MAC_len > 16)
        *MAC_len = 16;
    return SNMPERR_SUCCESS;

#endif                          /* NETSNMP_USE_OPENSSL */
}
#else                           /* !defined(NETSNMP_USE_OPENSSL) && !defined(NETSNMP_USE_INTERNAL_MD5) */
_SCAPI_NOT_CONFIGURED
#endif                          /* !defined(NETSNMP_USE_OPENSSL) && !defined(NETSNMP_USE_INTERNAL_MD5) */
/*******************************************************************-o-******
 * sc_check_keyed_hash
 *
 * Parameters:
 *	 authtype	Transform type of authentication hash.
 *	*key		Key bits in a string of bytes.
 *	 keylen		Length of key in bytes.
 *	*message	Message for which to check the hash.
 *	 msglen		Length of message.
 *	*MAC		Given hash.
 *	 maclen		Length of given hash; indicates truncation if it is
 *				shorter than the normal size of output for
 *				given hash transform.
 * Returns:
 *	SNMPERR_SUCCESS		Success.
 *	SNMP_SC_GENERAL_FAILURE	Any error
 *
 *
 * Check the hash given in MAC against the hash of message.  If the length
 * of MAC is less than the length of the transform hash output, only maclen
 * bytes are compared.  The length of MAC cannot be greater than the
 * length of the hash transform output.
 */
int
sc_check_keyed_hash(const oid * authtypeOID, size_t authtypeOIDlen,
                    const u_char * key, u_int keylen,
                    const u_char * message, u_int msglen,
                    const u_char * MAC, u_int maclen)
#if defined(NETSNMP_USE_INTERNAL_MD5) || defined(NETSNMP_USE_OPENSSL) || defined(NETSNMP_USE_PKCS11) || defined(NETSNMP_USE_INTERNAL_CRYPTO)
{
    int             rval = SNMPERR_SUCCESS, auth_type, auth_size;
    size_t          buf_len = SNMP_MAXBUF_SMALL;

    u_char          buf[SNMP_MAXBUF_SMALL];

    DEBUGTRACE;

#ifdef NETSNMP_ENABLE_TESTING_CODE
    {
        int             i;
        DEBUGMSG(("scapi", "sc_check_keyed_hash():    key=0x"));
        for (i = 0; i < keylen; i++)
            DEBUGMSG(("scapi", "%02x", key[i] & 0xff));
        DEBUGMSG(("scapi", " (%d)\n", keylen));
    }
#endif                          /* NETSNMP_ENABLE_TESTING_CODE */

    /*
     * Sanity check.
     */
    if (!authtypeOID || !key || !message || !MAC
        || (keylen <= 0) || (msglen <= 0) || (maclen <= 0)) {
        QUITFUN(SNMPERR_GENERR, sc_check_keyed_hash_quit);
    }

    auth_type = sc_get_authtype(authtypeOID, authtypeOIDlen);
    if (auth_type < 0 )
        return (SNMPERR_GENERR);

    auth_size = sc_get_auth_maclen(auth_type);
    if (0 == auth_size || maclen != auth_size) {
        QUITFUN(SNMPERR_GENERR, sc_check_keyed_hash_quit);
    }

    /*
     * Generate a full hash of the message, then compare
     * the result with the given MAC which may be shorter than
     * the full hash length.
     */
    rval = sc_generate_keyed_hash(authtypeOID, authtypeOIDlen, key, keylen,
                                  message, msglen, buf, &buf_len);
    QUITFUN(rval, sc_check_keyed_hash_quit);

    if (maclen > msglen) {
        QUITFUN(SNMPERR_GENERR, sc_check_keyed_hash_quit);

    } else if (memcmp(buf, MAC, maclen) != 0) {
        QUITFUN(SNMPERR_GENERR, sc_check_keyed_hash_quit);
    }


  sc_check_keyed_hash_quit:
    memset(buf, 0, SNMP_MAXBUF_SMALL);

    return rval;

}                               /* end sc_check_keyed_hash() */

#else
_SCAPI_NOT_CONFIGURED
#endif                          /* NETSNMP_USE_INTERNAL_MD5 */
/*******************************************************************-o-******
 * sc_encrypt
 *
 * Parameters:
 *	 privtype	Type of privacy cryptographic transform.
 *	*key		Key bits for crypting.
 *	 keylen		Length of key (buffer) in bytes.
 *	*iv		IV bits for crypting.
 *	 ivlen		Length of iv (buffer) in bytes.
 *	*plaintext	Plaintext to crypt.
 *	 ptlen		Length of plaintext.
 *	*ciphertext	Ciphertext to crypt.
 *	*ctlen		Length of ciphertext.
 *      
 * Returns:
 *	SNMPERR_SUCCESS			Success.
 *	SNMPERR_SC_NOT_CONFIGURED	Encryption is not supported.
 *	SNMPERR_SC_GENERAL_FAILURE	Any other error
 *
 *
 * Encrypt plaintext into ciphertext using key and iv.
 *
 * ctlen contains actual number of crypted bytes in ciphertext upon
 * successful return.
 */
int
sc_encrypt(const oid * privtype, size_t privtypelen,
           u_char * key, u_int keylen,
           u_char * iv, u_int ivlen,
           const u_char * plaintext, u_int ptlen,
           u_char * ciphertext, size_t * ctlen)
#if defined(NETSNMP_USE_OPENSSL) || defined(NETSNMP_USE_INTERNAL_CRYPTO)
{
    int             rval = SNMPERR_SUCCESS;
    u_char          pad_block[128];      /* bigger than anything I need */
    u_char          my_iv[128];  /* ditto */
    netsnmp_priv_alg_info *pai = NULL;
#ifndef NETSNMP_DISABLE_DES
    int             pad, plast, pad_size = 0;
#ifdef OLD_DES
    DES_key_schedule key_sch;
#else
    DES_key_schedule key_sched_store;
    DES_key_schedule *key_sch = &key_sched_store;
#endif /* OLD_DES */
    DES_cblock       key_struct;
#endif /* NETSNMP_DISABLE_DES */

    DEBUGTRACE;

    /*
     * Sanity check.
     */
#if	!defined(NETSNMP_ENABLE_SCAPI_AUTHPRIV)
    snmp_log(LOG_ERR, "Encryption support not enabled.(2)\n");
    return SNMPERR_SC_NOT_CONFIGURED;
#endif

    if (!privtype || !key || !iv || !plaintext || !ciphertext || !ctlen
        || (keylen <= 0) || (ivlen <= 0) || (ivlen > sizeof(my_iv))
        || (ptlen <= 0) || (*ctlen <= 0)) {
        DEBUGMSGTL(("scapi:encrypt", "bad arguments 1\n"));
        QUITFUN(SNMPERR_GENERR, sc_encrypt_quit);
    } else if (ptlen > *ctlen) {
        DEBUGMSGTL(("scapi:encrypt", "bad arguments 2\n"));
        QUITFUN(SNMPERR_GENERR, sc_encrypt_quit);
    }
#ifdef NETSNMP_ENABLE_TESTING_CODE
    {
        size_t          buf_len = 128, out_len = 0;
        u_char         *buf = (u_char *) malloc(buf_len);

        if (buf != NULL) {
            if (sprint_realloc_hexstring(&buf, &buf_len, &out_len, 1,
                                         iv, ivlen)) {
                DEBUGMSGTL(("scapi", "encrypt: IV: %s\n", buf));
            } else {
                DEBUGMSGTL(("scapi", "encrypt: IV: %s [TRUNCATED]\n", buf));
            }
            out_len = 0;
            if (sprint_realloc_hexstring(&buf, &buf_len, &out_len, 1,
                                         key, keylen)) {
                DEBUGMSG(("scapi", "key: %s\n", buf));
            } else {
                DEBUGMSG(("scapi", "key: %s [TRUNCATED]\n", buf));
            }
            out_len = 0;
            if (sprint_realloc_hexstring(&buf, &buf_len, &out_len, 1,
                                         plaintext, 16)) {
                DEBUGMSGTL(("scapi", "encrypt: string: %s\n", buf));
            } else {
                DEBUGMSGTL(("scapi", "encrypt: string: %s [TRUNCATED]\n",
                            buf));
            }
            free(buf);
        } else {
            DEBUGMSGTL(("scapi",
                        "encrypt: malloc fail for debug output\n"));
        }
    }
#endif                          /* NETSNMP_ENABLE_TESTING_CODE */


    /*
     * Determine privacy transform.
     */
    pai = sc_get_priv_alg_byoid(privtype, privtypelen);
    if (NULL == pai ||
        (keylen < pai->proper_length) || (ivlen < pai->iv_length)) {
        DEBUGMSGTL(("scapi:encrypt",
                    "bad arguments 3 pai %p, keylen %d ivlen %d\n",
                    pai, keylen, ivlen));
        if (pai)
            DEBUGMSGTL(("scapi:encrypt", " pai: properlen %d, ivlen %d\n",
                        pai->proper_length, pai->iv_length));

        QUITFUN(SNMPERR_GENERR, sc_encrypt_quit);
    }
    pad_size = pai->pad_size;

    memset(my_iv, 0, sizeof(my_iv));

#ifndef NETSNMP_DISABLE_DES
    if (USM_CREATE_USER_PRIV_DES == (pai->type & USM_PRIV_MASK_ALG)) {

        /*
         * now calculate the padding needed 
         */
        pad = pad_size - (ptlen % pad_size);
        plast = (int) ptlen - (pad_size - pad);
        if (pad == pad_size)
            pad = 0;
        if (ptlen + pad > *ctlen) {
            DEBUGMSGTL(("scapi:encrypt", "not enough space\n"));
            QUITFUN(SNMPERR_GENERR, sc_encrypt_quit);    /* not enough space */
        }
        if (pad > 0) {              /* copy data into pad block if needed */
            memcpy(pad_block, plaintext + plast, pad_size - pad);
            memset(&pad_block[pad_size - pad], pad, pad);   /* filling in padblock */
        }

        memcpy(key_struct, key, sizeof(key_struct));
        (void) DES_key_sched(&key_struct, key_sch);

        memcpy(my_iv, iv, ivlen);
        /*
         * encrypt the data 
         */
        DES_ncbc_encrypt(plaintext, ciphertext, plast, key_sch,
                         (DES_cblock *) my_iv, DES_ENCRYPT);
        if (pad > 0) {
            /*
             * then encrypt the pad block 
             */
            DES_ncbc_encrypt(pad_block, ciphertext + plast, pad_size,
                             key_sch, (DES_cblock *) my_iv, DES_ENCRYPT);
            *ctlen = plast + pad_size;
        } else {
            *ctlen = plast;
        }
    }
#endif
#if defined(NETSNMP_USE_OPENSSL) && defined(HAVE_AES)
    if (USM_CREATE_USER_PRIV_AES == (pai->type & USM_PRIV_MASK_ALG)) {
        EVP_CIPHER_CTX *ctx;
        const EVP_CIPHER *cipher;
        int len, rc, enclen;

        cipher = sc_get_openssl_privfn(pai->type);
        if (NULL == cipher) {
            DEBUGMSGTL(("scapi:encrypt", "cipher not found\n"));
            QUITFUN(SNMPERR_GENERR, sc_encrypt_quit);
        }

        memcpy(my_iv, iv, ivlen);
        /*
         * encrypt the data 
         */
        ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            DEBUGMSGTL(("scapi:encrypt", "openssl error: ctx_new\n"));
            QUITFUN(SNMPERR_GENERR, sc_encrypt_quit);
        }
        rc = EVP_EncryptInit(ctx, cipher, key, my_iv);
        if (rc != 1) {
            DEBUGMSGTL(("scapi:encrypt", "openssl error: init\n"));
            EVP_CIPHER_CTX_free(ctx);
            QUITFUN(SNMPERR_GENERR, sc_encrypt_quit);
        }
        rc = EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, ptlen);
        if (rc != 1) {
            DEBUGMSGTL(("scapi:encrypt", "openssl error: update\n"));
            EVP_CIPHER_CTX_free(ctx);
            QUITFUN(SNMPERR_GENERR, sc_encrypt_quit);
        }
        enclen = len;
        rc = EVP_EncryptFinal(ctx, ciphertext + len, &len);
        if (rc != 1) {
            DEBUGMSGTL(("scapi:encrypt", "openssl error: final\n"));
            EVP_CIPHER_CTX_free(ctx);
            QUITFUN(SNMPERR_GENERR, sc_encrypt_quit);
        }
        enclen += len;
        ptlen = enclen;
        /* Clean up */
        EVP_CIPHER_CTX_free(ctx);
        *ctlen = ptlen;
    }
#endif
  sc_encrypt_quit:
    /*
     * clear memory just in case 
     */
    memset(my_iv, 0, sizeof(my_iv));
    memset(pad_block, 0, sizeof(pad_block));
#ifndef NETSNMP_DISABLE_DES
    memset(key_struct, 0, sizeof(key_struct));
#ifdef OLD_DES
    memset(&key_sch, 0, sizeof(key_sch));
#else
    memset(&key_sched_store, 0, sizeof(key_sched_store));
#endif
#endif
    return rval;

}                               /* end sc_encrypt() */
#elif defined(NETSNMP_USE_PKCS11)
{
    int             rval = SNMPERR_SUCCESS, priv_type
    u_char	    pkcs_des_key[8];
    netsnmp_priv_alg_info *pai;

    DEBUGTRACE;

    /*
     * Sanity check.
     */
#if	!defined(NETSNMP_ENABLE_SCAPI_AUTHPRIV)
    snmp_log(LOG_ERR, "Encryption support not enabled.(1)\n");
    return SNMPERR_SC_NOT_CONFIGURED;
#endif

    if (!privtype || !key || !iv || !plaintext || !ciphertext || !ctlen
        || (keylen <= 0) || (ivlen <= 0) || (ptlen <= 0) || (*ctlen <= 0)) {
        QUITFUN(SNMPERR_GENERR, sc_encrypt_quit);
    } else if (ptlen > *ctlen) {
        QUITFUN(SNMPERR_GENERR, sc_encrypt_quit);
    }

    /*
     * Determine privacy transform.
     */
    pai = sc_get_priv_alg_byoid(privtype, privtypelen);
    if (NULL == pai || USM_CREATE_USER_PRIV_DES != priv_type ||
        (keylen < pai->proper_length) || (ivlen < pai->iv_length)) {
        QUITFUN(SNMPERR_GENERR, sc_encrypt_quit);
    }

    memset(pkcs_des_key, 0, sizeof(pkcs_des_key));
    memcpy(pkcs_des_key, key, sizeof(pkcs_des_key));
    rval = pkcs_encrpyt(CKM_DES_CBC, pkcs_des_key,
                        sizeof(pkcs_des_key), iv, ivlen, plaintext, ptlen,
                        ciphertext, ctlen);

  sc_encrypt_quit:
    return rval;
}
#else
{
#	if NETSNMP_USE_INTERNAL_MD5
    {
        snmp_log(LOG_ERR, "Encryption support not enabled.(3)\n");
        DEBUGMSGTL(("scapi", "Encrypt function not defined.\n"));
        return SNMPERR_SC_GENERAL_FAILURE;
    }

#	else
    _SCAPI_NOT_CONFIGURED
#	endif                   /* NETSNMP_USE_INTERNAL_MD5 */
}
#endif                          /* */



/*******************************************************************-o-******
 * sc_decrypt
 *
 * Parameters:
 *	 privtype
 *	*key
 *	 keylen
 *	*iv
 *	 ivlen
 *	*ciphertext
 *	 ctlen
 *	*plaintext
 *	*ptlen
 *      
 * Returns:
 *	SNMPERR_SUCCESS			Success.
 *	SNMPERR_SC_NOT_CONFIGURED	Encryption is not supported.
 *      SNMPERR_SC_GENERAL_FAILURE      Any other error
 *
 *
 * Decrypt ciphertext into plaintext using key and iv.
 *
 * ptlen contains actual number of plaintext bytes in plaintext upon
 * successful return.
 */
int
sc_decrypt(const oid * privtype, size_t privtypelen,
           u_char * key, u_int keylen,
           u_char * iv, u_int ivlen,
           u_char * ciphertext, u_int ctlen,
           u_char * plaintext, size_t * ptlen)
#if defined(NETSNMP_USE_OPENSSL) || defined(NETSNMP_USE_INTERNAL_CRYPTO)
{

    int             rval = SNMPERR_SUCCESS;
    u_char          my_iv[128];
#ifndef NETSNMP_DISABLE_DES
#ifdef OLD_DES
    DES_key_schedule key_sch;
#else
    DES_key_schedule key_sched_store;
    DES_key_schedule *key_sch = &key_sched_store;
#endif
    DES_cblock      key_struct;
#endif
    netsnmp_priv_alg_info *pai = NULL;

    DEBUGTRACE;

    if (!privtype || !key || !iv || !plaintext || !ciphertext || !ptlen
        || (ctlen <= 0) || (*ptlen <= 0) || (*ptlen < ctlen)) {
        DEBUGMSGTL(("scapi", "decrypt: arg sanity checks failed\n"));
        QUITFUN(SNMPERR_GENERR, sc_decrypt_quit);
    }
#ifdef NETSNMP_ENABLE_TESTING_CODE
    {
        size_t          buf_len = 128, out_len = 0;
        u_char         *buf = (u_char *) malloc(buf_len);

        if (buf != NULL) {
            if (sprint_realloc_hexstring(&buf, &buf_len, &out_len, 1,
                                         iv, ivlen)) {
                DEBUGMSGTL(("scapi", "decrypt: IV: %s\n", buf));
            } else {
                DEBUGMSGTL(("scapi", "decrypt: IV: %s [TRUNCATED]\n", buf));
            }
            out_len = 0;
            if (sprint_realloc_hexstring(&buf, &buf_len, &out_len, 1,
                                         key, keylen)) {
                DEBUGMSG(("scapi", "key: %s\n", buf));
            } else {
                DEBUGMSG(("scapi", "key: %s [TRUNCATED]\n", buf));
            }
            free(buf);
        } else {
            DEBUGMSGTL(("scapi",
                        "decrypt: malloc fail for debug output\n"));
        }
    }
#endif                          /* NETSNMP_ENABLE_TESTING_CODE */

    /*
     * Determine privacy transform.
     */
    pai = sc_get_priv_alg_byoid(privtype, privtypelen);
    if (NULL == pai ||
        (keylen < pai->proper_length) || (ivlen < pai->iv_length)) {
        QUITFUN(SNMPERR_GENERR, sc_decrypt_quit);
    }

    memset(my_iv, 0, sizeof(my_iv));
#ifndef NETSNMP_DISABLE_DES
    if (USM_CREATE_USER_PRIV_DES == (pai->type & USM_PRIV_MASK_ALG)) {
        memcpy(key_struct, key, sizeof(key_struct));
        (void) DES_key_sched(&key_struct, key_sch);

        memcpy(my_iv, iv, ivlen);
        DES_cbc_encrypt(ciphertext, plaintext, ctlen, key_sch,
                        (DES_cblock *) my_iv, DES_DECRYPT);
        *ptlen = ctlen;
    }
#endif
#if defined(NETSNMP_USE_OPENSSL) && defined(HAVE_AES)
    if (USM_CREATE_USER_PRIV_AES == (pai->type & USM_PRIV_MASK_ALG)) {
        EVP_CIPHER_CTX *ctx;
        const EVP_CIPHER *cipher;
        int len, rc;

        cipher = sc_get_openssl_privfn(pai->type);
        if (NULL == cipher)
            QUITFUN(SNMPERR_GENERR, sc_decrypt_quit);

        memcpy(my_iv, iv, ivlen);
        /*
         * decrypt the data
         */
        ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            QUITFUN(SNMPERR_GENERR, sc_decrypt_quit);
        }
        rc = EVP_DecryptInit(ctx, cipher, key, my_iv);
        if (rc != 1) {
            EVP_CIPHER_CTX_free(ctx);
            QUITFUN(SNMPERR_GENERR, sc_decrypt_quit);
        }
        rc = EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ctlen);
        if (rc != 1) {
            EVP_CIPHER_CTX_free(ctx);
            QUITFUN(SNMPERR_GENERR, sc_decrypt_quit);
        }
        rc = EVP_DecryptFinal(ctx, plaintext + len, &len);
        if (rc != 1) {
            EVP_CIPHER_CTX_free(ctx);
            QUITFUN(SNMPERR_GENERR, sc_decrypt_quit);
        }
        /* Clean up */
        EVP_CIPHER_CTX_free(ctx);
        *ptlen = ctlen;
    }
#endif

    /*
     * exit cond 
     */
  sc_decrypt_quit:
#ifndef NETSNMP_DISABLE_DES
#ifdef OLD_DES
    memset(&key_sch, 0, sizeof(key_sch));
#else
    memset(&key_sched_store, 0, sizeof(key_sched_store));
#endif
    memset(key_struct, 0, sizeof(key_struct));
#endif
    memset(my_iv, 0, sizeof(my_iv));
    return rval;
}				/* USE OPEN_SSL */
#elif NETSNMP_USE_PKCS11                  /* USE PKCS */
{
    int             rval = SNMPERR_SUCCESS;
    u_char	    pkcs_des_key[8];
    netsnmp_priv_alg_info *pai;

    DEBUGTRACE;

    if (!privtype || !key || !iv || !plaintext || !ciphertext || !ptlen
        || (ctlen <= 0) || (*ptlen <= 0) || (*ptlen < ctlen)) {
        QUITFUN(SNMPERR_GENERR, sc_decrypt_quit);
    }

    /*
     * Determine privacy transform.
     */
    pai = sc_get_priv_alg_byoid(privtype, privtypelen);
    if (NULL == pai || USM_CREATE_USER_PRIV_DES != priv_type ||
        (keylen < pai->proper_length) || (ivlen < pai->iv_length)) {
        QUITFUN(SNMPERR_GENERR, sc_decrypt_quit);
    }

    memset(pkcs_des_key, 0, sizeof(pkcs_des_key));
    memcpy(pkcs_des_key, key, sizeof(pkcs_des_key));
    rval = pkcs_decrpyt(CKM_DES_CBC, pkcs_des_key, 
                        sizeof(pkcs_des_key), iv, ivlen, ciphertext,
                        ctlen, plaintext, ptlen);
    *ptlen = ctlen;

  sc_decrypt_quit:
    return rval;
}				/* USE PKCS */
#else
{
#if	!defined(NETSNMP_ENABLE_SCAPI_AUTHPRIV)
    snmp_log(LOG_ERR, "Encryption support not enabled.(4)\n");
    return SNMPERR_SC_NOT_CONFIGURED;
#else
#	if NETSNMP_USE_INTERNAL_MD5
    {
        DEBUGMSGTL(("scapi", "Decryption function not defined.\n"));
        return SNMPERR_SC_GENERAL_FAILURE;
    }

#	else
    _SCAPI_NOT_CONFIGURED
#	endif                   /* NETSNMP_USE_INTERNAL_MD5 */
#endif                          /*  */
}
#endif                          /* NETSNMP_USE_OPENSSL */

#ifdef NETSNMP_USE_INTERNAL_CRYPTO

/* These functions are basically copies of the MDSign() routine in
   md5.c modified to be used with the OpenSSL hashing functions.  The
   copyright below is from the md5.c file that these functions were
   taken from: */

/*
 * ** **************************************************************************
 * ** md5.c -- Implementation of MD5 Message Digest Algorithm                 **
 * ** Updated: 2/16/90 by Ronald L. Rivest                                    **
 * ** (C) 1990 RSA Data Security, Inc.                                        **
 * ** **************************************************************************
 */

/*
 * MD5_hmac(data, len, MD5): do a checksum on an arbirtrary amount
 * of data, and prepended with a secret in the standard fashion 
 */
static int
MD5_hmac(const u_char * data, size_t len, u_char * mac, size_t maclen,
         const u_char * secret, size_t secretlen)
{
#define MD5_HASHKEYLEN 64
#define MD5_SECRETKEYLEN 16

    MD5_CTX         cmd5;
    u_char          K1[MD5_HASHKEYLEN];
    u_char          K2[MD5_HASHKEYLEN];
    u_char          extendedAuthKey[MD5_HASHKEYLEN];
    u_char          buf[MD5_HASHKEYLEN];
    size_t          i;
    const u_char   *cp;
    u_char         *newdata = NULL;
    int             rc = 0;

    DEBUGTRACE;

    /*
     * memset(K1,0,MD5_HASHKEYLEN);
     * memset(K2,0,MD5_HASHKEYLEN);
     * memset(buf,0,MD5_HASHKEYLEN);
     * memset(extendedAuthKey,0,MD5_HASHKEYLEN);
     */

    if (secretlen != MD5_SECRETKEYLEN || secret == NULL ||
        mac == NULL || data == NULL ||
        len <= 0 || maclen <= 0) {
        /*
         * DEBUGMSGTL(("md5","MD5 signing not properly initialized")); 
         */
        return -1;
    }

    memset(extendedAuthKey, 0, MD5_HASHKEYLEN);
    memcpy(extendedAuthKey, secret, secretlen);
    for (i = 0; i < MD5_HASHKEYLEN; i++) {
        K1[i] = extendedAuthKey[i] ^ 0x36;
        K2[i] = extendedAuthKey[i] ^ 0x5c;
    }

    MD5_Init(&cmd5);
    rc = !MD5_Update(&cmd5, K1, MD5_HASHKEYLEN);
    if (rc)
        goto update_end;

    i = len;
    if (((uintptr_t) data) % sizeof(long) != 0) {
        /*
         * this relies on the ability to use integer math and thus we
         * must rely on data that aligns on 32-bit-word-boundries 
         */
        newdata = netsnmp_memdup(data, len);
        cp = newdata;
    } else {
        cp = data;
    }

    while (i >= 64) {
        rc = !MD5_Update(&cmd5, cp, 64);
        if (rc)
            goto update_end;
        cp += 64;
        i -= 64;
    }

    rc = !MD5_Update(&cmd5, cp, i);
    if (rc)
        goto update_end;

    memset(buf, 0, MD5_HASHKEYLEN);
    MD5_Final(buf, &cmd5);

    MD5_Init(&cmd5);
    rc = !MD5_Update(&cmd5, K2, MD5_HASHKEYLEN);
    if (rc)
        goto update_end;
    rc = !MD5_Update(&cmd5, buf, MD5_SECRETKEYLEN);
    if (rc)
        goto update_end;

    /*
     * copy the sign checksum to the outgoing pointer 
     */
    MD5_Final(buf, &cmd5);
    memcpy(mac, buf, maclen);

  update_end:
    memset(buf, 0, MD5_HASHKEYLEN);
    memset(K1, 0, MD5_HASHKEYLEN);
    memset(K2, 0, MD5_HASHKEYLEN);
    memset(extendedAuthKey, 0, MD5_HASHKEYLEN);
    memset(&cmd5, 0, sizeof(cmd5));

    if (newdata)
        free(newdata);
    return rc;
}

static int
SHA1_hmac(const u_char * data, size_t len, u_char * mac, size_t maclen,
          const u_char * secret, size_t secretlen)
{
#define SHA1_HASHKEYLEN   64
#define SHA1_SECRETKEYLEN 20

    SHA_CTX         csha1;
    u_char          K1[SHA1_HASHKEYLEN];
    u_char          K2[SHA1_HASHKEYLEN];
    u_char          extendedAuthKey[SHA1_HASHKEYLEN];
    u_char          buf[SHA1_HASHKEYLEN];
    size_t          i;
    const u_char   *cp;
    u_char         *newdata = NULL;
    int             rc = 0;

    DEBUGTRACE;

    /*
     * memset(K1,0,SHA1_HASHKEYLEN);
     * memset(K2,0,SHA1_HASHKEYLEN);
     * memset(buf,0,SHA1_HASHKEYLEN);
     * memset(extendedAuthKey,0,SHA1_HASHKEYLEN);
     */

    if (secretlen != SHA1_SECRETKEYLEN || secret == NULL ||
        mac == NULL || data == NULL ||
        len <= 0 || maclen <= 0) {
        /*
         * DEBUGMSGTL(("sha1","SHA1 signing not properly initialized")); 
         */
        return -1;
    }

    memset(extendedAuthKey, 0, SHA1_HASHKEYLEN);
    memcpy(extendedAuthKey, secret, secretlen);
    for (i = 0; i < SHA1_HASHKEYLEN; i++) {
        K1[i] = extendedAuthKey[i] ^ 0x36;
        K2[i] = extendedAuthKey[i] ^ 0x5c;
    }

    SHA1_Init(&csha1);
    rc = !SHA1_Update(&csha1, K1, SHA1_HASHKEYLEN);
    if (rc)
        goto update_end;

    i = len;
    if (((uintptr_t) data) % sizeof(long) != 0) {
        /*
         * this relies on the ability to use integer math and thus we
         * must rely on data that aligns on 32-bit-word-boundries 
         */
        newdata = netsnmp_memdup(data, len);
        cp = newdata;
    } else {
        cp = data;
    }

    while (i >= 64) {
        rc = !SHA1_Update(&csha1, cp, 64);
        if (rc)
            goto update_end;
        cp += 64;
        i -= 64;
    }

    rc = !SHA1_Update(&csha1, cp, i);
    if (rc)
        goto update_end;

    memset(buf, 0, SHA1_HASHKEYLEN);
    SHA1_Final(buf, &csha1);

    SHA1_Init(&csha1);
    rc = !SHA1_Update(&csha1, K2, SHA1_HASHKEYLEN);
    if (rc)
        goto update_end;
    rc = !SHA1_Update(&csha1, buf, SHA1_SECRETKEYLEN);
    if (rc)
        goto update_end;

    /*
     * copy the sign checksum to the outgoing pointer 
     */
    SHA1_Final(buf, &csha1);
    memcpy(mac, buf, maclen);

  update_end:
    memset(buf, 0, SHA1_HASHKEYLEN);
    memset(K1, 0, SHA1_HASHKEYLEN);
    memset(K2, 0, SHA1_HASHKEYLEN);
    memset(extendedAuthKey, 0, SHA1_HASHKEYLEN);
    memset(&csha1, 0, sizeof(csha1));

    if (newdata)
        free(newdata);
    return rc;
}
#endif /* NETSNMP_USE_INTERNAL_CRYPTO */
#endif /*  NETSNMP_FEATURE_REMOVE_USM_SCAPI  */
