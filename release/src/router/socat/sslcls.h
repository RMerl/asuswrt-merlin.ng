/* source: sslcls.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __sslcls_h_included
#define __sslcls_h_included 1

#if WITH_SYCLS
#if WITH_OPENSSL

void sycSSL_load_error_strings(void);
int sycSSL_library_init(void);
const SSL_METHOD *sycSSLv2_client_method(void);
const SSL_METHOD *sycSSLv2_server_method(void);
const SSL_METHOD *sycSSLv3_client_method(void);
const SSL_METHOD *sycSSLv3_server_method(void);
const SSL_METHOD *sycSSLv23_client_method(void);
const SSL_METHOD *sycSSLv23_server_method(void);
const SSL_METHOD *sycTLSv1_client_method(void);
const SSL_METHOD *sycTLSv1_server_method(void);
const SSL_METHOD *sycTLSv1_1_client_method(void);
const SSL_METHOD *sycTLSv1_1_server_method(void);
const SSL_METHOD *sycTLSv1_2_client_method(void);
const SSL_METHOD *sycTLSv1_2_server_method(void);
const SSL_METHOD *sycDTLSv1_client_method(void);
const SSL_METHOD *sycDTLSv1_server_method(void);
SSL_CTX *sycSSL_CTX_new(const SSL_METHOD *method);
SSL *sycSSL_new(SSL_CTX *ctx);
int sycSSL_CTX_load_verify_locations(SSL_CTX *ctx, const char *CAfile,
				     const char *CApath);
int sycSSL_CTX_use_certificate_file(SSL_CTX *ctx, const char *file, int type);
int sycSSL_CTX_use_certificate_chain_file(SSL_CTX *ctx, const char *file);
int sycSSL_CTX_use_PrivateKey_file(SSL_CTX *ctx, const char *file, int type);
void sycSSL_CTX_set_verify(SSL_CTX *ctx, int mode,
			   int (*verify_callback)(int, X509_STORE_CTX *));
int sycSSL_CTX_set_tmp_dh(SSL_CTX *ctx, DH *dh);
int sycSSL_CTX_set_cipher_list(SSL_CTX *ctx, const char *str);
int sycSSL_set_cipher_list(SSL *ssl, const char *str);
long sycSSL_get_verify_result(SSL *ssl);
int sycSSL_set_fd(SSL *ssl, int fd);
int sycSSL_connect(SSL *ssl);
int sycSSL_accept(SSL *ssl);
int sycSSL_read(SSL *ssl, void *buf, int num);
int sycSSL_pending(SSL *ssl);
int sycSSL_write(SSL *ssl, const void *buf, int num);
X509 *sycSSL_get_peer_certificate(SSL *ssl);
int sycSSL_shutdown(SSL *ssl);
void sycSSL_CTX_free(SSL_CTX *ctx);
void sycSSL_free(SSL *ssl);
int sycRAND_egd(const char *path);

DH *sycPEM_read_bio_DHparams(BIO *bp, DH **x, pem_password_cb *cb, void *u);

BIO *sycBIO_new_file(const char *filename, const char *mode);

int sycFIPS_mode_set(int onoff);

#if OPENSSL_VERSION_NUMBER >= 0x00908000L && !defined(OPENSSL_NO_COMP)
const COMP_METHOD *sycSSL_get_current_compression(SSL *ssl);
const COMP_METHOD *sycSSL_get_current_expansion(SSL *ssl);
const char *sycSSL_COMP_get_name(const COMP_METHOD *comp);
#endif

#endif /* WITH_OPENSSL */

#else /* !WITH_SYCLS */

#if WITH_OPENSSL

#define sycSSL_load_error_strings() SSL_load_error_strings()
#define sycSSL_library_init() SSL_library_init()
#define sycSSLv2_client_method() SSLv2_client_method()
#define sycSSLv2_server_method() SSLv2_server_method()
#define sycSSLv3_client_method() SSLv3_client_method()
#define sycSSLv3_server_method() SSLv3_server_method()
#define sycSSLv23_client_method() SSLv23_client_method()
#define sycSSLv23_server_method() SSLv23_server_method()
#define sycTLSv1_client_method() TLSv1_client_method()
#define sycTLSv1_server_method() TLSv1_server_method()
#define sycTLSv1_1_client_method() TLSv1_1_client_method()
#define sycTLSv1_1_server_method() TLSv1_1_server_method()
#define sycTLSv1_2_client_method() TLSv1_2_client_method()
#define sycTLSv1_2_server_method() TLSv1_2_server_method()
#define sycDTLSv1_client_method() DTLSv1_client_method()
#define sycDTLSv1_server_method() DTLSv1_server_method()
#define sycSSL_CTX_new(m) SSL_CTX_new(m)
#define sycSSL_new(c) SSL_new(c)
#define sycSSL_CTX_load_verify_locations(c,f,p) SSL_CTX_load_verify_locations(c,f,p)
#define sycSSL_CTX_use_certificate_file(c,f,t) SSL_CTX_use_certificate_file(c,f,t)
#define sycSSL_CTX_use_certificate_chain_file(c,f) SSL_CTX_use_certificate_chain_file(c,f)
#define sycSSL_CTX_use_PrivateKey_file(c,f,t) SSL_CTX_use_PrivateKey_file(c,f,t)
#define sycSSL_CTX_set_verify(c,m,v) SSL_CTX_set_verify(c,m,v)
#define sycSSL_CTX_set_tmp_dh(c,d) SSL_CTX_set_tmp_dh(c,d)
#define sycSSL_CTX_set_cipher_list(c,s) SSL_CTX_set_cipher_list(c,s)
#define sycSSL_set_cipher_list(s,t) SSL_set_cipher_list(s,t)
#define sycSSL_get_verify_result(s) SSL_get_verify_result(s)
#define sycSSL_set_fd(s,f) SSL_set_fd(s,f)
#define sycSSL_connect(s) SSL_connect(s)
#define sycSSL_accept(s) SSL_accept(s)
#define sycSSL_read(s,b,n) SSL_read(s,b,n)
#define sycSSL_pending(s) SSL_pending(s)
#define sycSSL_write(s,b,n) SSL_write(s,b,n)
#define sycSSL_get_peer_certificate(s) SSL_get_peer_certificate(s)
#define sycSSL_shutdown(s) SSL_shutdown(s)
#define sycSSL_CTX_free(c) SSL_CTX_free(c)
#define sycSSL_free(s) SSL_free(s)
#define sycRAND_egd(p) RAND_egd(p)

#define sycPEM_read_bio_DHparams(b,x,p,u) PEM_read_bio_DHparams(b,x,p,u)

#define sycBIO_new_file(f,m) BIO_new_file(f,m)

#define sycSSL_get_current_compression(s) SSL_get_current_compression(s)
#define sycSSL_get_current_expansion(s) SSL_get_current_expansion(s)
#define sycSSL_COMP_get_name(c) SSL_COMP_get_name(c)

#endif /* WITH_OPENSSL */

#define sycFIPS_mode_set(o) FIPS_mode_set(o)

#endif /* !WITH_SYCLS */

#endif /* !defined(__sslcls_h_included) */

