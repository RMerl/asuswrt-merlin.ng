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
