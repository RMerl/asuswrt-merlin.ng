/*

	Minimal CyaSSL/OpenSSL Helper
	Copyright (C) 2006-2009 Jonathan Zarate
	Copyright (C) 2010 Fedor Kozhevnikov

	Licensed under GNU GPL v2 or later

*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/file.h>
#include <sys/stat.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

#define _dprintf(args...)	while (0) {}

// refer https://mozilla.github.io/server-side-tls/ssl-config-generator/ w/o DES ciphers
#define SERVER_CIPHERS "ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:DHE-RSA-AES128-SHA256:DHE-RSA-AES256-SHA256:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:!DSS"

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
// use reasonable defaults
#define CLIENT_CIPHERS NULL
#else
#define CLIENT_CIPHERS "ALL:!EXPORT:!EXPORT40:!EXPORT56:!aNULL:!LOW:!RC4:@STRENGTH"
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#define X509_getm_notBefore X509_get_notBefore
#define X509_getm_notAfter X509_get_notAfter
#define RSA_get0_n(d) ((d)->n)
#define DSA_get0_pub_key(d) ((d)->pub_key)
#endif

#if !defined(__GLIBC__) && !defined(__UCLIBC__) /* musl */
// introduce musl fopencookie patch & copy some musl definition here (musl-1.1.16)
typedef ssize_t (cookie_read_function_t)(void *, char *, size_t);
typedef ssize_t (cookie_write_function_t)(void *, const char *, size_t);
typedef int (cookie_seek_function_t)(void *, off_t *, int);
typedef int (cookie_close_function_t)(void *);

typedef struct {
	cookie_read_function_t *read;
	cookie_write_function_t *write;
	cookie_seek_function_t *seek;
	cookie_close_function_t *close;
} cookie_io_functions_t;

struct fcookie {
	void *cookie;
	cookie_io_functions_t iofuncs;
};

typedef struct _IO_FILE FILE;
struct _IO_FILE {
        unsigned flags;
        unsigned char *rpos, *rend;
        int (*close)(FILE *);
        unsigned char *wend, *wpos;
        unsigned char *mustbezero_1;
        unsigned char *wbase;
        size_t (*read)(FILE *, unsigned char *, size_t);
        size_t (*write)(FILE *, const unsigned char *, size_t);
        off_t (*seek)(FILE *, off_t, int);
        unsigned char *buf;
        size_t buf_size;
        FILE *prev, *next;
        int fd;
        int pipe_pid;
        long lockcount;
        short dummy3;
        signed char mode;
        signed char lbf;
        volatile int lock;
        volatile int waiters;
        void *cookie;
        off_t off;
        char *getln_buf;
        void *mustbezero_2;
        unsigned char *shend;
        off_t shlim, shcnt;
        FILE *prev_locked, *next_locked;
        struct __locale_struct *locale;
};
#define UNGET 8
#define F_PERM 1
#define F_NORD 4
#define F_NOWR 8
#define F_EOF 16
#define F_ERR 32
#define F_SVB 64
#define F_APP 128
FILE *__ofl_add(FILE *f);

struct cookie_FILE {
	FILE f;
	struct fcookie fc;
	unsigned char buf[UNGET+BUFSIZ];
};

static size_t cookieread(FILE *f, unsigned char *buf, size_t len)
{
	struct fcookie *fc = f->cookie;
	ssize_t ret = -1;
	size_t remain = len, readlen = 0;
	size_t len2 = len - !!f->buf_size;

	if (!fc->iofuncs.read) goto bail;

	if (len2) {
		ret = fc->iofuncs.read(fc->cookie, (char *) buf, len2);
		if (ret <= 0) goto bail;

		readlen += ret;
		remain -= ret;
	}

	if (!f->buf_size || remain > !!f->buf_size) return readlen;

	f->rpos = f->buf;
	ret = fc->iofuncs.read(fc->cookie, (char *) f->rpos, f->buf_size);
	if (ret <= 0) goto bail;
	f->rend = f->rpos + ret;

	buf[readlen++] = *f->rpos++;

	return readlen;

bail:
	f->flags |= ret == 0 ? F_EOF : F_ERR;
	f->rpos = f->rend = f->buf;
	return readlen;
}

static size_t cookiewrite(FILE *f, const unsigned char *buf, size_t len)
{
	struct fcookie *fc = f->cookie;
	ssize_t ret;
	size_t len2 = f->wpos - f->wbase;
	if (!fc->iofuncs.write) return len;
	if (len2) {
		f->wpos = f->wbase;
		if (cookiewrite(f, f->wpos, len2) < len2) return 0;
	}
	ret = fc->iofuncs.write(fc->cookie, (const char *) buf, len);
	if (ret < 0) {
		f->wpos = f->wbase = f->wend = 0;
		f->flags |= F_ERR;
		return 0;
	}
	return ret;
}

static off_t cookieseek(FILE *f, off_t off, int whence)
{
	struct fcookie *fc = f->cookie;
	int res;
	if (whence > 2U) {
		errno = EINVAL;
		return -1;
	}
	if (!fc->iofuncs.seek) {
		errno = ENOTSUP;
		return -1;
	}
	res = fc->iofuncs.seek(fc->cookie, &off, whence);
	if (res < 0)
		return res;
	return off;
}

static int cookieclose(FILE *f)
{
	struct fcookie *fc = f->cookie;
	if (fc->iofuncs.close) return fc->iofuncs.close(fc->cookie);
	return 0;
}

FILE *fopencookie(void *cookie, const char *mode, cookie_io_functions_t iofuncs)
{
	struct cookie_FILE *f;

	/* Check for valid initial mode character */
	if (!strchr("rwa", *mode)) {
		errno = EINVAL;
		return 0;
	}

	/* Allocate FILE+fcookie+buffer or fail */
	if (!(f=malloc(sizeof *f))) return 0;

	/* Zero-fill only the struct, not the buffer */
	memset(&f->f, 0, sizeof f->f);

	/* Impose mode restrictions */
	if (!strchr(mode, '+')) f->f.flags = (*mode == 'r') ? F_NOWR : F_NORD;

	/* Set up our fcookie */
	f->fc.cookie = cookie;
	f->fc.iofuncs.read = iofuncs.read;
	f->fc.iofuncs.write = iofuncs.write;
	f->fc.iofuncs.seek = iofuncs.seek;
	f->fc.iofuncs.close = iofuncs.close;

	f->f.fd = -1;
	f->f.cookie = &f->fc;
	f->f.buf = f->buf + UNGET;
	f->f.buf_size = BUFSIZ;
	f->f.lbf = EOF;

	/* Initialize op ptrs. No problem if some are unneeded. */
	f->f.read = cookieread;
	f->f.write = cookiewrite;
	f->f.seek = cookieseek;
	f->f.close = cookieclose;

	/* Add new FILE to open file list */
	return __ofl_add(&f->f);
}
#endif

typedef struct {
	SSL* ssl;
	int sd;
} mssl_cookie_t;

static SSL_CTX* ctx;

static inline void mssl_print_err(SSL* ssl)
{
	ERR_print_errors_fp(stderr);
}

static inline void mssl_cleanup(int err)
{
	if (err) mssl_print_err(NULL);
	SSL_CTX_free(ctx);
	ctx = NULL;
}

static ssize_t mssl_read(void *cookie, char *buf, size_t len)
{
	_dprintf("%s()\n", __FUNCTION__);

	mssl_cookie_t *kuki = cookie;
	int total = 0;
	int n, err;

	do {
		n = SSL_read(kuki->ssl, &(buf[total]), len - total);
		_dprintf("SSL_read(max=%d) returned %d\n", len - total, n);

		err = SSL_get_error(kuki->ssl, n);
		switch (err) {
		case SSL_ERROR_NONE:
			total += n;
			break;
		case SSL_ERROR_ZERO_RETURN:
			total += n;
			goto OUT;
		case SSL_ERROR_WANT_WRITE:
		case SSL_ERROR_WANT_READ:
			break;
		default:
			_dprintf("%s(): SSL error %d\n", __FUNCTION__, err);
			mssl_print_err(kuki->ssl);
			if (total == 0) total = -1;
			goto OUT;
		}
	} while ((len - total > 0) && SSL_pending(kuki->ssl));

OUT:
	_dprintf("%s() returns %d\n", __FUNCTION__, total);
	return total;
}

static ssize_t mssl_write(void *cookie, const char *buf, size_t len)
{
	_dprintf("%s()\n", __FUNCTION__);

	mssl_cookie_t *kuki = cookie;
	int total = 0;
	int n, err;

	while (total < len) {
		n = SSL_write(kuki->ssl, &(buf[total]), len - total);
		_dprintf("SSL_write(max=%d) returned %d\n", len - total, n);
		err = SSL_get_error(kuki->ssl, n);
		switch (err) {
		case SSL_ERROR_NONE:
			total += n;
			break;
		case SSL_ERROR_ZERO_RETURN:
			total += n;
			goto OUT;
		case SSL_ERROR_WANT_WRITE:
		case SSL_ERROR_WANT_READ:
			break;
		default:
			_dprintf("%s(): SSL error %d\n", __FUNCTION__, err);
			mssl_print_err(kuki->ssl);
			if (total == 0) total = -1;
			goto OUT;
		}
	}

OUT:
	_dprintf("%s() returns %d\n", __FUNCTION__, total);
	return total;
}

static int mssl_seek(void *cookie, off64_t *pos, int whence)
{
	_dprintf("%s()\n", __FUNCTION__);
	errno = EIO;
	return -1;
}

static int mssl_close(void *cookie)
{
	_dprintf("%s()\n", __FUNCTION__);

	mssl_cookie_t *kuki = cookie;
	if (!kuki) return 0;

	if (kuki->ssl) {
		SSL_shutdown(kuki->ssl);
		SSL_free(kuki->ssl);
	}

	free(kuki);
	return 0;
}

static const cookie_io_functions_t mssl = {
	mssl_read, mssl_write, mssl_seek, mssl_close
};

static FILE *_ssl_fopen(int sd, int client, const char *name)
{
	int r = 0;
	mssl_cookie_t *kuki;
	FILE *f;

	_dprintf("%s()\n", __FUNCTION__);

	if ((kuki = calloc(1, sizeof(*kuki))) == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	kuki->sd = sd;

	// Create new SSL object
	if ((kuki->ssl = SSL_new(ctx)) == NULL) {
		_dprintf("%s: SSL_new failed\n", __FUNCTION__);
		goto ERROR;
	}

	// SSL structure for client authenticate after SSL_new()
	SSL_set_verify(kuki->ssl, SSL_VERIFY_NONE, NULL);
	SSL_set_mode(kuki->ssl, SSL_MODE_AUTO_RETRY);

	if (client) {
		// Setup SNI
#if OPENSSL_VERSION_NUMBER >= 0x0090806fL && !defined(OPENSSL_NO_TLSEXT)
		if (name && *name) {
			struct addrinfo *res, hint = { .ai_flags = AI_NUMERICHOST };
			if (getaddrinfo(name, NULL, &hint, &res) == 0)
				freeaddrinfo(res);
			else if (SSL_set_tlsext_host_name(kuki->ssl, name) != 1) {
				_dprintf("%s: SSL_set_tlsext_host_name failed\n", __FUNCTION__);
				mssl_print_err(kuki->ssl);
				goto ERROR;
			}
		}
#endif
	}

	// Bind the socket to SSL structure
	// kuki->ssl : SSL structure
	// kuki->sd  : socket_fd
	r = SSL_set_fd(kuki->ssl, kuki->sd);

	if (!client) {
		// Do the SSL Handshake
		r = SSL_accept(kuki->ssl);
	} else {
		// Connect to the server, SSL layer
		r = SSL_connect(kuki->ssl);
	}
	// r = 0 show unknown CA, but we don't have any CA, so ignore.
	if (r < 0) {
		// Check error in connect or accept
		_dprintf(client ? "%s: SSL_connect failed\n" : "%s: SSL_accept failed\n", __FUNCTION__);
		mssl_print_err(kuki->ssl);
		goto ERROR;
	}
	
	_dprintf("SSL connection using %s cipher\n", SSL_get_cipher(kuki->ssl));

	if ((f = fopencookie(kuki, "r+", mssl)) == NULL) {
		_dprintf("%s: fopencookie failed\n", __FUNCTION__);
		goto ERROR;
	}

	_dprintf("%s() success\n", __FUNCTION__);
	return f;

ERROR:
	mssl_close(kuki);
	return NULL;
}

FILE *ssl_server_fopen(int sd)
{
	_dprintf("%s()\n", __FUNCTION__);
	return _ssl_fopen(sd, 0, NULL);
}

FILE *ssl_client_fopen(int sd)
{
	_dprintf("%s()\n", __FUNCTION__);
	return _ssl_fopen(sd, 1, NULL);
}

FILE *ssl_client_fopen_name(int sd, const char *name)
{
	_dprintf("%s()\n", __FUNCTION__);
	return _ssl_fopen(sd, 1, name);
}

#ifndef SSL_OP_NO_RENEGOTIATION
#if OPENSSL_VERSION_NUMBER < 0x10100000L && defined(SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS)
static void ssl_info_cb(const SSL *ssl, int where, int ret)
{
	if ((where & SSL_CB_HANDSHAKE_DONE) != 0 && SSL_is_server((SSL *) ssl)) {
		// disable renegotiation (CVE-2009-3555)
		ssl->s3->flags |= SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS;
	}
}
#endif
#endif

int mssl_init_ex(char *cert, char *priv, char *ciphers)
{
	int server;

	_dprintf("%s()\n", __FUNCTION__);

	server = (cert != NULL);

	// Register error strings for libcrypto and libssl functions
	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();

	// Create the new CTX with the method 
	// If server=1, use TLSv1_server_method() or SSLv23_server_method()
	// else 	use TLSv1_client_method() or SSLv23_client_method()
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	ctx = SSL_CTX_new(server ? TLS_server_method() : TLS_client_method());
#else
	ctx = SSL_CTX_new(server ? SSLv23_server_method() : SSLv23_client_method());
#endif
	if (!ctx) {
		_dprintf("SSL_CTX_new() failed\n");
		mssl_print_err(NULL);
		return 0;
	}

	// Setup common modes
	SSL_CTX_set_mode(ctx,
#ifdef SSL_MODE_RELEASE_BUFFERS
				 SSL_MODE_RELEASE_BUFFERS |
#endif
				 SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

	// Setup common options
	SSL_CTX_set_options(ctx, SSL_OP_ALL |
#ifdef SSL_OP_NO_TICKET
				 SSL_OP_NO_TICKET |
#endif
#ifdef SSL_OP_NO_COMPRESSION
				 SSL_OP_NO_COMPRESSION |
#endif
#ifdef SSL_OP_SINGLE_DH_USE
				 SSL_OP_SINGLE_DH_USE |
#endif
				 SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);

	// Setup EC support
#ifdef NID_X9_62_prime256v1
	EC_KEY *ecdh = NULL;
	if ((ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1)) != NULL) {
		SSL_CTX_set_tmp_ecdh(ctx, ecdh);
		EC_KEY_free(ecdh);
#ifdef SSL_OP_SINGLE_ECDH_USE
		SSL_CTX_set_options(ctx, SSL_OP_SINGLE_ECDH_USE);
#endif
	}
#endif

	// Setup available ciphers
	if (ciphers == NULL)
		ciphers = server ? SERVER_CIPHERS : CLIENT_CIPHERS;
	if (ciphers && SSL_CTX_set_cipher_list(ctx, ciphers) != 1) {
		_dprintf("%s: SSL_CTX_set_cipher_list failed\n", __FUNCTION__);
		mssl_cleanup(1);
		return 0;
	}

	if (server) {
		// Enforce server cipher order
		SSL_CTX_set_options(ctx, SSL_OP_CIPHER_SERVER_PREFERENCE);

		// Set the certificate to be used
		_dprintf("SSL_CTX_use_certificate_chain_file(%s)\n", cert);
		if (SSL_CTX_use_certificate_chain_file(ctx, cert) <= 0) {
			_dprintf("SSL_CTX_use_certificate_chain_file() failed\n");
			mssl_cleanup(1);
			return 0;
		}
		// Indicate the key file to be used
		_dprintf("SSL_CTX_use_PrivateKey_file(%s)\n", priv);
		if (SSL_CTX_use_PrivateKey_file(ctx, priv, SSL_FILETYPE_PEM) <= 0) {
			_dprintf("SSL_CTX_use_PrivateKey_file() failed\n");
			mssl_cleanup(1);
			return 0;
		}
		// Make sure the key and certificate file match
		if (!SSL_CTX_check_private_key(ctx)) {
			_dprintf("Private key does not match the certificate public key\n");
			mssl_cleanup(0);
			return 0;
		}

		// Disable renegotiation
#ifdef SSL_OP_NO_RENGOTIATION
		SSL_CTX_set_options(ctx, SSL_OP_NO_RENGOTIATION);
#elif OPENSSL_VERSION_NUMBER < 0x10100000L && defined(SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS)
		SSL_CTX_set_info_callback(ctx, ssl_info_cb);
#endif
	}

	SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

	_dprintf("%s() success\n", __FUNCTION__);
	return 1;
}

int mssl_init(char *cert, char *priv)
{
	return mssl_init_ex(cert, priv, NULL);
}

void mssl_ctx_free()
{
	mssl_cleanup(0);
}

static int mssl_f_exists(const char *path)
{
	struct stat st;
	return (stat(path, &st) == 0) && (!S_ISDIR(st.st_mode));
}

/*
 * compare the modulus of public key and private key
 */
int mssl_cert_key_match(const char *cert_path, const char *key_path)
{
	FILE *fp;
	X509 *x509data = NULL;
	EVP_PKEY *pkey = NULL;
	RSA *rsa_pub = NULL;
	RSA *rsa_pri = NULL;
	DSA *dsa_pub = NULL;
	DSA *dsa_pri = NULL;
	int pem = 1;
	int ret = 0;

	if(!mssl_f_exists(cert_path) || !mssl_f_exists(key_path))
	{
		return 0;
	}

	//get x509 from cert file
	fp = fopen(cert_path, "r");
	if(!fp)
	{
		return 0;
	}
	if(!PEM_read_X509(fp, &x509data, NULL, NULL))
	{
		_dprintf("[mssl] Try to read DER format certificate\n");
		pem = 0;
		fseek(fp, 0, SEEK_SET);
		d2i_X509_fp(fp, &x509data);
	}
	else
	{
		_dprintf("[mssl] PEM format certificate\n");
	}

	fclose(fp);
	if(x509data == NULL)
	{
		_dprintf("[mssl] Load certificate failed\n");
		ret = 0;
		goto end;
	}

	//get pubic key from x509
	pkey = X509_get_pubkey(x509data);
	if(pkey == NULL)
	{
		ret = 0;
		goto end;
	}
	X509_free(x509data);
	x509data = NULL;

	if(EVP_PKEY_id(pkey) == EVP_PKEY_RSA)
	{
		//_dprintf("RSA public key\n");
		rsa_pub = EVP_PKEY_get1_RSA(pkey);
	}
	else if(EVP_PKEY_id(pkey) == EVP_PKEY_DSA)
	{
		//_dprintf("DSA public key\n");
		dsa_pub = EVP_PKEY_get1_DSA(pkey);
	}
	EVP_PKEY_free(pkey);
	pkey = NULL;

	//get private key from key file
	fp = fopen(key_path, "r");
	if(!fp)
	{
		ret = 0;
		goto end;
	}
	if(pem)
	{
		//_dprintf("PEM format private key\n");
		pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
	}
	else
	{
		//_dprintf("DER format private key\n");
		pkey = d2i_PrivateKey_fp(fp, NULL);
	}
	fclose(fp);

	if(pkey == NULL)
	{
		_dprintf("[mssl] Load private key failed\n");
		ret = 0;
		goto end;
	}

	if(EVP_PKEY_id(pkey) == EVP_PKEY_RSA)
	{
		//_dprintf("RSA private key\n");
		rsa_pri = EVP_PKEY_get1_RSA(pkey);
	}
	else if(EVP_PKEY_id(pkey) == EVP_PKEY_DSA)
	{
		//_dprintf("DSA private key\n");
		dsa_pri = EVP_PKEY_get1_DSA(pkey);
	}
	EVP_PKEY_free(pkey);
	pkey = NULL;

	//compare modulus
	if(rsa_pub && rsa_pri)
	{
		if(BN_cmp(RSA_get0_n(rsa_pub), RSA_get0_n(rsa_pri)))
		{
			_dprintf("[mssl] rsa n not match\n");
			ret = 0;
		}
		else
		{
			_dprintf("[mssl] rsa n match\n");
			ret = 1;
		}
	}
	else if(dsa_pub && dsa_pri)
	{
		if(BN_cmp(DSA_get0_pub_key(dsa_pub), DSA_get0_pub_key(dsa_pri)))
		{
			_dprintf("[mssl] dsa modulus not match\n");
			ret = 0;
		}
		else
		{
			_dprintf("[mssl] dsa modulus match\n");
			ret = 1;
		}
	}
	else
	{
		_dprintf("[mssl] compare failed");
	}

end:
	if(x509data)
		X509_free(x509data);
	if(pkey)
		EVP_PKEY_free(pkey);
	if(rsa_pub)
		RSA_free(rsa_pub);
	if(dsa_pub)
		DSA_free(dsa_pub);
	if(rsa_pri)
		RSA_free(rsa_pri);
	if(dsa_pri)
		DSA_free(dsa_pri);

	return ret;
}
