#include "log.h"
#include "http.h"
#include "ssl.h"

#define xstr(s) str(s)
#define str(s) #s

int ssl_init(void) { return 0; }

void ssl_exit(void) {}

int ssl_open(http_t *client, char *msg)
{
	int port = 0;
	int rc;

	if (!client->ssl_enabled)
		return tcp_init(&client->tcp, msg);

	http_get_port(client, &port);
	if (!port)
		http_set_port(client, HTTPS_DEFAULT_PORT);
	rc = tcp_init(&client->tcp, msg);
	if (rc)
		return rc;

	logit(LOG_INFO, "%s, initiating HTTPS ...", msg);

	mbedtls_ssl_init       (&client->ssl);
	mbedtls_net_init       (&client->server_fd);
	mbedtls_x509_crt_init  (&client->cacert);
	mbedtls_ssl_config_init(&client->conf);
	mbedtls_ctr_drbg_init  (&client->ctr_drbg);
	mbedtls_entropy_init   (&client->entropy);

	rc = mbedtls_ctr_drbg_seed(&client->ctr_drbg, mbedtls_entropy_func, &client->entropy, PACKAGE_STRING, strlen(PACKAGE_STRING));
	if (rc) {
		logit(LOG_DEBUG, "mbedtls_ctr_drbg_seed:%d", rc);
		ssl_close(client);
		return RC_HTTPS_OUT_OF_MEMORY;
	}

	rc = mbedtls_x509_crt_parse_file(&client->cacert, CAFILE1);
	if (rc) {
		rc = mbedtls_x509_crt_parse_file(&client->cacert, CAFILE2);
	}
	if (rc) {
		logit(LOG_DEBUG, "mbedtls_x509_crt_parse_file: %d", rc);
		ssl_close(client);
		return RC_HTTPS_NO_TRUSTED_CA_STORE;
	}

	rc = mbedtls_net_connect(&client->server_fd, client->tcp.remote_host, xstr(HTTPS_DEFAULT_PORT), MBEDTLS_NET_PROTO_TCP);
	if (rc) {
		logit(LOG_DEBUG, "mbedtls_net_connect:%d '%s'", rc, client->tcp.remote_host);
		ssl_close(client);
		return RC_TCP_CONNECT_FAILED;
	}

	rc = mbedtls_ssl_config_defaults(&client->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
	if (rc) {
		logit(LOG_DEBUG, "mbedtls_ssl_config_defaults:%d", rc);
		ssl_close(client);
		return RC_HTTPS_OUT_OF_MEMORY;
	}

	mbedtls_ssl_conf_ca_chain(&client->conf, &client->cacert, 0);
	mbedtls_ssl_conf_rng(&client->conf, mbedtls_ctr_drbg_random, &client->ctr_drbg);

	rc = mbedtls_ssl_setup(&client->ssl, &client->conf);
	if (rc) {
		logit(LOG_DEBUG, "mbedtls_ssl_setup:%d", rc);
		ssl_close(client);
		return RC_HTTPS_OUT_OF_MEMORY;
	}

	rc = mbedtls_ssl_set_hostname(&client->ssl, client->tcp.remote_host);
	if (rc) {
		logit(LOG_DEBUG, "mbedtls_ssl_set_hostname:%d '%s'", rc, client->tcp.remote_host);
		ssl_close(client);
		return RC_HTTPS_SNI_ERROR;
	}

	mbedtls_ssl_set_bio(&client->ssl, &client->server_fd, mbedtls_net_send, mbedtls_net_recv, 0);

	while (1) {
		rc = mbedtls_ssl_handshake(&client->ssl);
		if (!rc) { break; }
		if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE) {
			logit(LOG_DEBUG, "mbedtls_ssl_handshake:%d", rc);
			ssl_close(client);
			return RC_HTTPS_OUT_OF_MEMORY;
		}
	}

	client->connected = 1;
	logit(LOG_INFO, "SSL connection using protocol %s and ciphersuite %s", mbedtls_ssl_get_version(&client->ssl), mbedtls_ssl_get_ciphersuite(&client->ssl));

	return 0;
}

int ssl_close(http_t *client)
{
	if (client->ssl_enabled) {
		if (client->connected)
			mbedtls_ssl_close_notify(&client->ssl);
		mbedtls_ssl_free       (&client->ssl      );
		mbedtls_net_free       (&client->server_fd);
		mbedtls_x509_crt_free  (&client->cacert   );
		mbedtls_ssl_config_free(&client->conf     );
		mbedtls_ctr_drbg_free  (&client->ctr_drbg );
		mbedtls_entropy_free   (&client->entropy  );
	}
	client->connected = 0;

	return tcp_exit(&client->tcp);
}

int ssl_send(http_t *client, const char *buf, int len)
{
	int err;

	if (!client->ssl_enabled)
		return tcp_send(&client->tcp, buf, len);

	do {
		err = mbedtls_ssl_write(&client->ssl, buf, len);
	} while (err == MBEDTLS_ERR_SSL_WANT_WRITE);

	if (err <= 0) {
		return RC_HTTPS_SEND_ERROR;
	}

	logit(LOG_DEBUG, "Successfully sent HTTPS request!");

	return 0;
}

int ssl_recv(http_t *client, char *buf, int buf_len, int *recv_len)
{
	int err, len = 0;

	if (!client->ssl_enabled)
		return tcp_recv(&client->tcp, buf, buf_len, recv_len);

	do {
		err = mbedtls_ssl_read(&client->ssl, buf + len, buf_len - len);
		if (err == 0 || err == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
			err = 0;
			client->connected = 0;
			break;
		}
		if (err > 0)
			len += err;
	} while (err > 0 || err == MBEDTLS_ERR_SSL_WANT_READ);

	if (err < 0) {
		return RC_HTTPS_RECV_ERROR;
	}

	*recv_len = len;

	logit(LOG_DEBUG, "Successfully received HTTPS response (%d/%d bytes)!", *recv_len, buf_len);

	return 0;
}
