/*
 * Testing tool for TLSv1 client/server routines
 * Copyright (c) 2019, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "includes.h"

#include "common.h"
#include "crypto/tls.h"

static void usage(void) {
	wpa_printf(MSG_INFO,
		   "usage: test-tls <server/client> <read/write> <file>");
	exit(-1);
}

static void write_msg(FILE *f, struct wpabuf *msg)
{
	u8 len[2];

	wpa_printf(MSG_DEBUG, "TEST: Write message to file (msg_len=%u)",
		   (unsigned int) wpabuf_len(msg));
	WPA_PUT_BE16(len, wpabuf_len(msg));
	fwrite(len, 2, 1, f);
	fwrite(wpabuf_head(msg), wpabuf_len(msg), 1, f);
}

static struct wpabuf * read_msg(FILE *f)
{
	u8 len[2];
	u16 msg_len;
	struct wpabuf *msg;

	if (fread(len, 2, 1, f) != 1) {
		wpa_printf(MSG_ERROR, "TEST-ERROR: Could not read msg len");
		return NULL;
	}
	msg_len = WPA_GET_BE16(len);

	msg = wpabuf_alloc(msg_len);
	if (!msg)
		return NULL;
	if (msg_len > 0 &&
	    fread(wpabuf_put(msg, msg_len), msg_len, 1, f) != 1) {
		wpa_printf(MSG_ERROR, "TEST-ERROR: Truncated msg (msg_len=%u)",
			   msg_len);
		wpabuf_free(msg);
		return NULL;
	}
	wpa_hexdump_buf(MSG_DEBUG, "TEST: Read message from file", msg);

	return msg;
}

int main(int argc, char *argv[])
{
	struct tls_config conf;
	void *tls_server, *tls_client;
	struct tls_connection_params params;
	struct tls_connection *conn_server = NULL, *conn_client = NULL;
	int ret = -1;
	struct wpabuf *in = NULL, *out = NULL, *appl;
	enum { SERVER, CLIENT } test_peer;
	enum { READ, WRITE } test_oper;
	const char *file;
	FILE *f;

	wpa_debug_level = 0;
	wpa_debug_show_keys = 1;

	if (argc < 4)
		usage();

	if (os_strcmp(argv[1], "server") == 0)
		test_peer = SERVER;
	else if (os_strcmp(argv[1], "client") == 0)
		test_peer = CLIENT;
	else
		usage();

	if (os_strcmp(argv[2], "read") == 0)
		test_oper = READ;
	else if (os_strcmp(argv[2], "write") == 0)
		test_oper = WRITE;
	else
		usage();

	file = argv[3];

	f = fopen(file, test_oper == READ ? "r" : "w");
	if (!f)
		return -1;

	os_memset(&conf, 0, sizeof(conf));
	tls_server = tls_init(&conf);
	tls_client = tls_init(&conf);
	if (!tls_server || !tls_client)
		goto fail;

	os_memset(&params, 0, sizeof(params));
	params.ca_cert = "hwsim/auth_serv/ca.pem";
	params.client_cert = "hwsim/auth_serv/server.pem";
	params.private_key = "hwsim/auth_serv/server.key";
	params.dh_file = "hwsim/auth_serv/dh.conf";

	if (tls_global_set_params(tls_server, &params)) {
		wpa_printf(MSG_ERROR, "Failed to set TLS parameters");
		goto fail;
	}

	conn_server = tls_connection_init(tls_server);
	conn_client = tls_connection_init(tls_client);
	if (!conn_server || !conn_client)
		goto fail;

	in = NULL;
	for (;;) {
		appl = NULL;
		if (test_peer == CLIENT && test_oper == READ)
			out = read_msg(f);
		else
			out = tls_connection_handshake(tls_client, conn_client,
						       in, &appl);
		wpabuf_free(in);
		in = NULL;
		if (!out)
			goto fail;
		if (test_peer == CLIENT && test_oper == WRITE &&
		    wpabuf_len(out) > 0)
			write_msg(f, out);
		if (!(test_peer == CLIENT && test_oper == READ) &&
		    tls_connection_get_failed(tls_client, conn_client)) {
			wpa_printf(MSG_ERROR, "TLS handshake failed");
			goto fail;
		}
		if (((test_peer == CLIENT && test_oper == READ) ||
		     tls_connection_established(tls_client, conn_client)) &&
		    ((test_peer == SERVER && test_oper == READ) ||
		     tls_connection_established(tls_server, conn_server)))
			break;

		appl = NULL;
		if (test_peer == SERVER && test_oper == READ)
			in = read_msg(f);
		else
			in = tls_connection_server_handshake(tls_server,
							     conn_server,
							     out, &appl);
		wpabuf_free(out);
		out = NULL;
		if (!in)
			goto fail;
		if (test_peer == SERVER && test_oper == WRITE)
			write_msg(f, in);
		if (!(test_peer == SERVER && test_oper == READ) &&
		    tls_connection_get_failed(tls_server, conn_server)) {
			wpa_printf(MSG_ERROR, "TLS handshake failed");
			goto fail;
		}
		if (((test_peer == CLIENT && test_oper == READ) ||
		     tls_connection_established(tls_client, conn_client)) &&
		    ((test_peer == SERVER && test_oper == READ) ||
		     tls_connection_established(tls_server, conn_server)))
			break;
	}

	wpabuf_free(in);
	in = wpabuf_alloc(100);
	if (!in)
		goto fail;
	wpabuf_put_str(in, "PING");
	wpabuf_free(out);
	if (test_peer == CLIENT && test_oper == READ)
		out = read_msg(f);
	else
		out = tls_connection_encrypt(tls_client, conn_client, in);
	wpabuf_free(in);
	in = NULL;
	if (!out)
		goto fail;
	if (test_peer == CLIENT && test_oper == WRITE)
		write_msg(f, out);

	if (!(test_peer == SERVER && test_oper == READ)) {
		in = tls_connection_decrypt(tls_server, conn_server, out);
		wpabuf_free(out);
		out = NULL;
		if (!in)
			goto fail;
		wpa_hexdump_buf(MSG_DEBUG, "Server decrypted ApplData", in);
	}

	wpabuf_free(in);
	in = wpabuf_alloc(100);
	if (!in)
		goto fail;
	wpabuf_put_str(in, "PONG");
	wpabuf_free(out);
	if (test_peer == SERVER && test_oper == READ)
		out = read_msg(f);
	else
		out = tls_connection_encrypt(tls_server, conn_server, in);
	wpabuf_free(in);
	in = NULL;
	if (!out)
		goto fail;
	if (test_peer == SERVER && test_oper == WRITE)
		write_msg(f, out);

	if (!(test_peer == CLIENT && test_oper == READ)) {
		in = tls_connection_decrypt(tls_client, conn_client, out);
		wpabuf_free(out);
		out = NULL;
		if (!in)
			goto fail;
		wpa_hexdump_buf(MSG_DEBUG, "Client decrypted ApplData", in);
	}

	ret = 0;
fail:
	if (tls_server) {
		if (conn_server)
			tls_connection_deinit(tls_server, conn_server);
		tls_deinit(tls_server);
	}
	if (tls_client) {
		if (conn_client)
			tls_connection_deinit(tls_server, conn_client);
		tls_deinit(tls_client);
	}
	wpabuf_free(in);
	wpabuf_free(out);
	fclose(f);

	return ret;
}
