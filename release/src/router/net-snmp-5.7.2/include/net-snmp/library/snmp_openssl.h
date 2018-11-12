/*
 * Header file for the OpenSSL Wrapper
 */

#ifndef SNMP_OPENSSL_H
#define SNMP_OPENSSL_H

#ifdef NETSNMP_USE_OPENSSL
#include <openssl/dh.h>
#include <openssl/ssl.h>

#ifdef __cplusplus
extern          "C" {
#endif

    struct netsnmp_cert_map_s;

    NETSNMP_IMPORT
    void netsnmp_init_openssl(void);

    /*
     * cert fields
     */
    void netsnmp_openssl_cert_dump_names(X509 *ocert);
    void netsnmp_openssl_cert_dump_extensions(X509 *ocert);

    char *netsnmp_openssl_cert_get_commonName(X509 *, char **buf, int *len);
    char *netsnmp_openssl_cert_get_subjectName(X509 *, char **buf, int *len);
    char *netsnmp_openssl_cert_get_fingerprint(X509 *ocert, int alg);
    int netsnmp_openssl_cert_get_hash_type(X509 *ocert);

    int netsnmp_openssl_cert_issued_by(X509 *issuer, X509 *cert);

    char *netsnmp_openssl_extract_secname(struct netsnmp_cert_map_s *cert_map,
                                          struct netsnmp_cert_map_s *peer_cert);

    char *netsnmp_openssl_cert_get_subjectAltName(X509 *, char **buf, int *len);

    /*
     * ssl cert chains
     */
    netsnmp_container *netsnmp_openssl_get_cert_chain(SSL *ssl);

    /*
     * misc
     */
    void netsnmp_openssl_err_log(const char *prefix);
    void netsnmp_openssl_null_checks(SSL *ssl, int *nullAuth, int *nullCipher);

    /*
     * backports
     */
#ifndef HAVE_DH_SET0_PQG
    NETSNMP_IMPORT
    int DH_set0_pqg(DH *dh, BIGNUM *p, BIGNUM *q, BIGNUM *g);
#endif
#ifndef HAVE_DH_GET0_PQG
    NETSNMP_IMPORT
    void DH_get0_pqg(const DH *dh, const BIGNUM **p, const BIGNUM **q,
                     const BIGNUM **g);
#endif
#ifndef HAVE_DH_GET0_KEY
    NETSNMP_IMPORT
    void DH_get0_key(const DH *dh, const BIGNUM **pub_key,
                     const BIGNUM **priv_key);
#endif
#ifndef HAVE_ASN1_STRING_GET0_DATA
    NETSNMP_IMPORT
    const unsigned char *ASN1_STRING_get0_data(const ASN1_STRING *x);
#endif
#ifndef HAVE_X509_NAME_ENTRY_GET_OBJECT
    NETSNMP_IMPORT
    ASN1_OBJECT *X509_NAME_ENTRY_get_object(const X509_NAME_ENTRY *ne);
#endif
#ifndef HAVE_X509_NAME_ENTRY_GET_DATA
    NETSNMP_IMPORT
    ASN1_STRING *X509_NAME_ENTRY_get_data(const X509_NAME_ENTRY *ne);
#endif
#ifndef HAVE_X509_GET_SIGNATURE_NID
    NETSNMP_IMPORT
    int X509_get_signature_nid(const X509 *req);
#endif
#ifndef HAVE_TLS_METHOD
    NETSNMP_IMPORT
    const SSL_METHOD *TLS_method(void);
#endif
#ifndef HAVE_DTLS_METHOD
    NETSNMP_IMPORT
    const SSL_METHOD *DTLS_method(void);
#endif
#endif

#ifdef __cplusplus
}
#endif
#endif                          /* SNMP_OPENSSL_H */
