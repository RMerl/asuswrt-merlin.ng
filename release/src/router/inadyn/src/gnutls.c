/* GnuTLS interface for optional HTTPS functions
 *
 * Copyright (C) 2014-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <stdint.h>
#include <gnutls/x509.h>

#include "log.h"
#include "http.h"
#include "ssl.h"

extern char *prognm;
static gnutls_certificate_credentials_t xcred;


/* This function will verify the peer's certificate, and check
 * if the hostname matches, as well as the activation, expiration dates.
 */
static int verify_certificate_callback(gnutls_session_t session)
{
	unsigned int status;
	const gnutls_datum_t *cert_list;
	unsigned int cert_list_size;
	int ret;
	gnutls_x509_crt_t cert;
	const char *hostname;

	/* read hostname */
	hostname = gnutls_session_get_ptr(session);

	/* This verification function uses the trusted CAs in the credentials
	 * structure. So you must have installed one or more CA certificates.
	 */
	ret = gnutls_certificate_verify_peers2(session, &status);
	if (ret < 0) {
		logit(LOG_ERR, "Failed verifying certificate peers.");
		goto error;
	}

	if (status & GNUTLS_CERT_SIGNER_NOT_FOUND)
		logit(LOG_WARNING, "The certificate does not have a known issuer.");

	if (status & GNUTLS_CERT_REVOKED)
		logit(LOG_WARNING, "The certificate has been revoked.");

	if (status & GNUTLS_CERT_EXPIRED)
		logit(LOG_WARNING, "The certificate has expired.");

	if (status & GNUTLS_CERT_NOT_ACTIVATED) {
		logit(LOG_WARNING, "The certificate is not yet activated.");
		if (broken_rtc && (status &= ~GNUTLS_CERT_NOT_ACTIVATED) == GNUTLS_CERT_INVALID)
			status = 0;
	}

	if (status & GNUTLS_CERT_INVALID) {
		logit(LOG_ERR, "The certificate is not trusted.");
		goto error;
	}

	/* Up to here the process is the same for X.509 certificates and
	 * OpenPGP keys. From now on X.509 certificates are assumed. This can
	 * be easily extended to work with openpgp keys as well.
	 */
	if (gnutls_certificate_type_get(session) != GNUTLS_CRT_X509) {
		logit(LOG_ERR, "Not a valid X.509 certificate");
		goto error;
	}

	if (gnutls_x509_crt_init(&cert) < 0) {
		logit(LOG_ERR, "Failed init of X.509 cert engine");
		goto error;
	}

	cert_list = gnutls_certificate_get_peers(session, &cert_list_size);
	if (cert_list == NULL) {
		logit(LOG_ERR, "No certificate was found!");
		goto error;
	}

	if (gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER) < 0) {
		logit(LOG_ERR, "Error while parsing certificate.");
		goto error;
	}


	if (!gnutls_x509_crt_check_hostname(cert, hostname)) {
		logit(LOG_ERR, "The certificate's owner does not match the hostname '%s'", hostname);
		goto error;
	}

	gnutls_x509_crt_deinit(cert);

	/* notify gnutls to continue handshake normally */
	logit(LOG_DEBUG, "Certificate OK");
	return 0;

error:
	if (secure_ssl)
		return GNUTLS_E_CERTIFICATE_ERROR;

	return 0;
}

static int ssl_set_ca_location(void)
{
	int num = 0;

	/* A user defined CA PEM bundle overrides any built-ins or fall-backs */
	if (ca_trust_file) {
		num = gnutls_certificate_set_x509_trust_file(xcred, ca_trust_file, GNUTLS_X509_FMT_PEM);
		goto done;
	}

#ifdef gnutls_certificate_set_x509_system_trust /* Since 3.0.20 */
	num = gnutls_certificate_set_x509_system_trust(xcred);
#endif
	if (num <= 0)
		num = gnutls_certificate_set_x509_trust_file(xcred, CAFILE1, GNUTLS_X509_FMT_PEM);
	if (num <= 0)
		num = gnutls_certificate_set_x509_trust_file(xcred, CAFILE2, GNUTLS_X509_FMT_PEM);
done:
	if (num <= 0)
		return 1;

	return 0;
}

int ssl_init(void)
{
	if (!gnutls_check_version("3.1.4")) {
		logit(LOG_ERR, "%s requires GnuTLS 3.1.4 or later for SSL", prognm);
		exit(1);
	}

	/* for backwards compatibility with gnutls < 3.3.0 */
	gnutls_global_init();

	/* X509 stuff */
	gnutls_certificate_allocate_credentials(&xcred);

	/* Try to figure out location of trusted CA certs on system */
	if (ssl_set_ca_location())
		return RC_HTTPS_NO_TRUSTED_CA_STORE;

	gnutls_certificate_set_verify_function(xcred, verify_certificate_callback);

	return 0;
}


void ssl_exit(void)
{
	gnutls_certificate_free_credentials(xcred);
	gnutls_global_deinit();
}

void ssl_get_info(http_t *client)
{
#ifndef gnutls_session_get_desc
	(void)client;
#else
	char *info;

	/* Available since 3.1.10  */
	info = gnutls_session_get_desc(client->ssl);
	logit(LOG_INFO, "SSL connection using: %s", info);
	gnutls_free(info);
#endif
}


int ssl_open(http_t *client, char *msg)
{
	int ret;
	char buf[256];
	size_t len;
	const char *sn, *err;
	const gnutls_datum_t *cert_list;
	unsigned int cert_list_size = 0;
	gnutls_x509_crt_t cert;

	if (!client->ssl_enabled)
		return tcp_init(&client->tcp, msg);

	/* Initialize TLS session */
	logit(LOG_INFO, "%s, initiating HTTPS ...", msg);
	gnutls_init(&client->ssl, GNUTLS_CLIENT);

	/* SSL SNI support: tell the servername we want to speak to */
	http_get_remote_name(client, &sn);
	gnutls_session_set_ptr(client->ssl, (void *)sn);
	if (gnutls_server_name_set(client->ssl, GNUTLS_NAME_DNS, sn, strlen(sn)))
		return RC_HTTPS_SNI_ERROR;

	/* Use default priorities */
	ret = gnutls_priority_set_direct(client->ssl, "NORMAL", &err);
	if (ret < 0) {
		if (ret == GNUTLS_E_INVALID_REQUEST)
			logit(LOG_ERR, "Syntax error at: %s", err);

		return RC_HTTPS_INVALID_REQUEST;
	}

	/* put the x509 credentials to the current session */
	gnutls_credentials_set(client->ssl, GNUTLS_CRD_CERTIFICATE, xcred);

	/* connect to the peer */
	tcp_set_port(&client->tcp, HTTPS_DEFAULT_PORT);
	DO(tcp_init(&client->tcp, msg));

	/* Forward TCP socket to GnuTLS, the set_int() API is perhaps too new still ... since 3.1.9 */
//	gnutls_transport_set_int(client->ssl, client->tcp.socket);
	gnutls_transport_set_ptr(client->ssl, (gnutls_transport_ptr_t)(intptr_t)client->tcp.socket);

	/* Perform the TLS handshake, ignore non-fatal errors. */
	do {
		ret = gnutls_handshake(client->ssl);
	}
	while (ret != 0 && !gnutls_error_is_fatal(ret));

	if (gnutls_error_is_fatal(ret)) {
		logit(LOG_ERR, "SSL handshake with %s failed: %s", sn, gnutls_strerror(ret));
		return RC_HTTPS_FAILED_CONNECT;
	}

	client->connected = 1;
	ssl_get_info(client);

	/* Get server's certificate (note: beware of dynamic allocation) - opt */
	cert_list = gnutls_certificate_get_peers(client->ssl, &cert_list_size);
	if (cert_list_size > 0) {
		if (gnutls_x509_crt_init(&cert))
			return RC_HTTPS_FAILED_GETTING_CERT;

		gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER);

		len = sizeof(buf);
		gnutls_x509_crt_get_dn(cert, buf, &len);
		logit(LOG_INFO, "SSL server cert subject: %s", buf);

		len = sizeof(buf);
		gnutls_x509_crt_get_issuer_dn(cert, buf, &len);
		logit(LOG_INFO, "SSL server cert issuer: %s", buf);

		gnutls_x509_crt_deinit(cert);
	}

	return 0;
}

int ssl_close(http_t *client)
{
	if (client->ssl_enabled) {
		if (client->connected)
			gnutls_bye(client->ssl, GNUTLS_SHUT_WR);
		gnutls_deinit(client->ssl);
	}
	client->connected = 0;

	return tcp_exit(&client->tcp);
}

int ssl_send(http_t *client, const char *buf, int len)
{
	int ret;

	if (!client->ssl_enabled)
		return tcp_send(&client->tcp, buf, len);

	do {
		ret = gnutls_record_send(client->ssl, buf, len);
	} while (ret == GNUTLS_E_INTERRUPTED || ret == GNUTLS_E_AGAIN);

	if (ret < 0)
		return RC_HTTPS_SEND_ERROR;

	logit(LOG_DEBUG, "Successfully sent HTTPS request!");

	return 0;
}

int ssl_recv(http_t *client, char *buf, int buf_len, int *recv_len)
{
	int ret, len = 0;

	if (!client->ssl_enabled)
		return tcp_recv(&client->tcp, buf, buf_len, recv_len);

	do {
		ret = gnutls_record_recv(client->ssl, buf + len, buf_len - len);
		if (ret > 0) {
			len += ret;
		}
	} while (ret > 0 || ret == GNUTLS_E_INTERRUPTED || ret == GNUTLS_E_AGAIN);

	/*
	 * We may get GNUTLS_E_PREMATURE_TERMINATION here.  It happens
	 * when a server closes the TCP connection without a prior TLS
	 * shutdown alert.  Probably indicates a bug in the server's
	 * TLS handling.  OpenSSL seems to ignore this so we do too.
	 *                       -- André Colomb
	 */
	if (ret < 0 && ret != GNUTLS_E_PREMATURE_TERMINATION) {
		logit(LOG_WARNING, "Failed receiving HTTPS response: %s", gnutls_strerror(ret));
		return RC_HTTPS_RECV_ERROR;
	}

	*recv_len = len;

	logit(LOG_DEBUG, "Successfully received HTTPS response (%d/%d bytes)!", *recv_len, buf_len);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
