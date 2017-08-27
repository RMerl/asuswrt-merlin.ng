/*
 * tls.c
 *
 * Version:     $Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2001  hereUare Communications, Inc. <raghud@hereuare.com>
 * Copyright 2003  Alan DeKok <aland@freeradius.org>
 * Copyright 2006  The FreeRADIUS server project
 */

RCSID("$Id$")
USES_APPLE_DEPRECATED_API	/* OpenSSL API has been deprecated by Apple */

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/process.h>
#include <freeradius-devel/rad_assert.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef WITH_TLS
#ifdef HAVE_OPENSSL_RAND_H
#include <openssl/rand.h>
#endif

#ifdef HAVE_OPENSSL_OCSP_H
#include <openssl/ocsp.h>
#endif

#ifdef HAVE_PTHREAD_H
#define PTHREAD_MUTEX_LOCK pthread_mutex_lock
#define PTHREAD_MUTEX_UNLOCK pthread_mutex_unlock
#else
#define PTHREAD_MUTEX_LOCK(_x)
#define PTHREAD_MUTEX_UNLOCK(_x)
#endif

static void dump_hex(char const *msg, uint8_t const *data, size_t data_len)
{
	size_t i;

	if (debug_flag < 3) return;

	printf("%s %d\n", msg, (int) data_len);
	if (data_len > 256) data_len = 256;

	for (i = 0; i < data_len; i++) {
		if ((i & 0x0f) == 0x00) printf ("%02x: ", (unsigned int) i);
		printf("%02x ", data[i]);
		if ((i & 0x0f) == 0x0f) printf ("\n");
	}
	printf("\n");
	fflush(stdout);
}

static void tls_socket_close(rad_listen_t *listener)
{
	listen_socket_t *sock = listener->data;

	listener->status = RAD_LISTEN_STATUS_REMOVE_NOW;
	listener->tls = NULL; /* parent owns this! */

	if (sock->parent) {
		/*
		 *	Decrement the number of connections.
		 */
		if (sock->parent->limit.num_connections > 0) {
			sock->parent->limit.num_connections--;
		}
		if (sock->client->limit.num_connections > 0) {
			sock->client->limit.num_connections--;
		}
	}

	/*
	 *	Tell the event handler that an FD has disappeared.
	 */
	DEBUG("Client has closed connection");
	event_new_fd(listener);

	/*
	 *	Do NOT free the listener here.  It's in use by
	 *	a request, and will need to hang around until
	 *	all of the requests are done.
	 *
	 *	It is instead free'd in remove_from_request_hash()
	 */
}

static int tls_socket_write(rad_listen_t *listener, REQUEST *request)
{
	uint8_t *p;
	ssize_t rcode;
	listen_socket_t *sock = listener->data;

	p = sock->ssn->dirty_out.data;

	while (p < (sock->ssn->dirty_out.data + sock->ssn->dirty_out.used)) {
		RDEBUG3("Writing to socket %d", request->packet->sockfd);
		rcode = write(request->packet->sockfd, p,
			      (sock->ssn->dirty_out.data + sock->ssn->dirty_out.used) - p);
		if (rcode <= 0) {
			RDEBUG("Error writing to TLS socket: %s", strerror(errno));

			tls_socket_close(listener);
			return 0;
		}
		p += rcode;
	}

	sock->ssn->dirty_out.used = 0;

	return 1;
}


static int tls_socket_recv(rad_listen_t *listener)
{
	int doing_init = false;
	ssize_t rcode;
	RADIUS_PACKET *packet;
	REQUEST *request;
	listen_socket_t *sock = listener->data;
	fr_tls_status_t status;
	RADCLIENT *client = sock->client;

	if (!sock->packet) {
		sock->packet = rad_alloc(NULL, 0);
		if (!sock->packet) return 0;

		sock->packet->sockfd = listener->fd;
		sock->packet->src_ipaddr = sock->other_ipaddr;
		sock->packet->src_port = sock->other_port;
		sock->packet->dst_ipaddr = sock->my_ipaddr;
		sock->packet->dst_port = sock->my_port;

		if (sock->request) {
			(void) talloc_steal(sock->request, sock->packet);
			sock->request->packet = sock->packet;
		}
	}

	/*
	 *	Allocate a REQUEST for debugging.
	 */
	if (!sock->request) {
		sock->request = request = request_alloc(NULL);
		if (!sock->request) {
			ERROR("Out of memory");
			return 0;
		}

		rad_assert(request->packet == NULL);
		rad_assert(sock->packet != NULL);
		request->packet = sock->packet;

		request->component = "<core>";
		request->component = "<tls-connect>";

		/*
		 *	Not sure if we should do this on every packet...
		 */
		request->reply = rad_alloc(request, 0);
		if (!request->reply) return 0;

		request->options = RAD_REQUEST_OPTION_DEBUG2;

		rad_assert(sock->ssn == NULL);

		sock->ssn = tls_new_session(listener->tls, sock->request,
					    listener->tls->require_client_cert);
		if (!sock->ssn) {
			request_free(&sock->request);
			sock->packet = NULL;
			return 0;
		}

		(void) talloc_steal(sock, sock->ssn);
		SSL_set_ex_data(sock->ssn->ssl, FR_TLS_EX_INDEX_REQUEST, (void *)request);
		SSL_set_ex_data(sock->ssn->ssl, FR_TLS_EX_INDEX_CERTS, (void *)&request->packet->vps);

		doing_init = true;
	}

	rad_assert(sock->request != NULL);
	rad_assert(sock->request->packet != NULL);
	rad_assert(sock->packet != NULL);
	rad_assert(sock->ssn != NULL);

	request = sock->request;

	RDEBUG3("Reading from socket %d", request->packet->sockfd);
	PTHREAD_MUTEX_LOCK(&sock->mutex);
	rcode = read(request->packet->sockfd,
		     sock->ssn->dirty_in.data,
		     sizeof(sock->ssn->dirty_in.data));
	if ((rcode < 0) && (errno == ECONNRESET)) {
	do_close:
		PTHREAD_MUTEX_UNLOCK(&sock->mutex);
		tls_socket_close(listener);
		return 0;
	}

	if (rcode < 0) {
		RDEBUG("Error reading TLS socket: %s", strerror(errno));
		goto do_close;
	}

	/*
	 *	Normal socket close.
	 */
	if (rcode == 0) goto do_close;

	sock->ssn->dirty_in.used = rcode;

	dump_hex("READ FROM SSL", sock->ssn->dirty_in.data, sock->ssn->dirty_in.used);

	/*
	 *	Catch attempts to use non-SSL.
	 */
	if (doing_init && (sock->ssn->dirty_in.data[0] != handshake)) {
		RDEBUG("Non-TLS data sent to TLS socket: closing");
		goto do_close;
	}

	/*
	 *	Skip ahead to reading application data.
	 */
	if (SSL_is_init_finished(sock->ssn->ssl)) goto app;

	if (!tls_handshake_recv(request, sock->ssn)) {
		RDEBUG("FAILED in TLS handshake receive");
		goto do_close;
	}

	if (sock->ssn->dirty_out.used > 0) {
		tls_socket_write(listener, request);
		PTHREAD_MUTEX_UNLOCK(&sock->mutex);
		return 0;
	}

app:
	/*
	 *	FIXME: Run the packet through a virtual server in
	 *	order to see if we like the certificate presented by
	 *	the client.
	 */

	status = tls_application_data(sock->ssn, request);
	RDEBUG("Application data status %d", status);

	if (status == FR_TLS_MORE_FRAGMENTS) {
		PTHREAD_MUTEX_UNLOCK(&sock->mutex);
		return 0;
	}

	if (sock->ssn->clean_out.used == 0) {
		PTHREAD_MUTEX_UNLOCK(&sock->mutex);
		return 0;
	}

	dump_hex("TUNNELED DATA", sock->ssn->clean_out.data, sock->ssn->clean_out.used);

	/*
	 *	If the packet is a complete RADIUS packet, return it to
	 *	the caller.  Otherwise...
	 */
	if ((sock->ssn->clean_out.used < 20) ||
	    (((sock->ssn->clean_out.data[2] << 8) | sock->ssn->clean_out.data[3]) != (int) sock->ssn->clean_out.used)) {
		RDEBUG("Received bad packet: Length %d contents %d",
		       sock->ssn->clean_out.used,
		       (sock->ssn->clean_out.data[2] << 8) | sock->ssn->clean_out.data[3]);
		goto do_close;
	}

	packet = sock->packet;
	packet->data = talloc_array(packet, uint8_t, sock->ssn->clean_out.used);
	packet->data_len = sock->ssn->clean_out.used;
	sock->ssn->record_minus(&sock->ssn->clean_out, packet->data, packet->data_len);
	packet->vps = NULL;
	PTHREAD_MUTEX_UNLOCK(&sock->mutex);

	if (!rad_packet_ok(packet, 0)) {
		RDEBUG("Received bad packet: %s", fr_strerror());
		tls_socket_close(listener);
		return 0;	/* do_close unlocks the mutex */
	}

	/*
	 *	Copied from src/lib/radius.c, rad_recv();
	 */
	if (fr_debug_flag) {
		char host_ipaddr[128];

		if ((packet->code > 0) && (packet->code < FR_MAX_PACKET_CODE)) {
			RDEBUG("tls_recv: %s packet from host %s port %d, id=%d, length=%d",
			       fr_packet_codes[packet->code],
			       inet_ntop(packet->src_ipaddr.af,
					 &packet->src_ipaddr.ipaddr,
					 host_ipaddr, sizeof(host_ipaddr)),
			       packet->src_port,
			       packet->id, (int) packet->data_len);
		} else {
			RDEBUG("tls_recv: Packet from host %s port %d code=%d, id=%d, length=%d",
			       inet_ntop(packet->src_ipaddr.af,
					 &packet->src_ipaddr.ipaddr,
					 host_ipaddr, sizeof(host_ipaddr)),
			       packet->src_port,
			       packet->code,
			       packet->id, (int) packet->data_len);
		}
	}

	FR_STATS_INC(auth, total_requests);

	return 1;
}


int dual_tls_recv(rad_listen_t *listener)
{
	RADIUS_PACKET *packet;
	REQUEST *request;
	RAD_REQUEST_FUNP fun = NULL;
	listen_socket_t *sock = listener->data;
	RADCLIENT	*client = sock->client;

	if (!tls_socket_recv(listener)) {
		return 0;
	}

	rad_assert(sock->request != NULL);
	rad_assert(sock->request->packet != NULL);
	rad_assert(sock->packet != NULL);
	rad_assert(sock->ssn != NULL);

	request = sock->request;
	packet = sock->packet;

	/*
	 *	Some sanity checks, based on the packet code.
	 */
	switch(packet->code) {
	case PW_AUTHENTICATION_REQUEST:
		if (listener->type != RAD_LISTEN_AUTH) goto bad_packet;
		FR_STATS_INC(auth, total_requests);
		fun = rad_authenticate;
		break;

	case PW_ACCOUNTING_REQUEST:
		if (listener->type != RAD_LISTEN_ACCT) goto bad_packet;
		FR_STATS_INC(acct, total_requests);
		fun = rad_accounting;
		break;

	case PW_STATUS_SERVER:
		if (!mainconfig.status_server) {
			FR_STATS_INC(auth, total_unknown_types);
			WDEBUG("Ignoring Status-Server request due to security configuration");
			rad_free(&sock->packet);
			request->packet = NULL;
			return 0;
		}
		fun = rad_status_server;
		break;

	default:
	bad_packet:
		FR_STATS_INC(auth, total_unknown_types);

		DEBUG("Invalid packet code %d sent from client %s port %d : IGNORED",
		      packet->code, client->shortname, packet->src_port);
		rad_free(&sock->packet);
		request->packet = NULL;
		return 0;
	} /* switch over packet types */

	if (!request_receive(listener, packet, client, fun)) {
		FR_STATS_INC(auth, total_packets_dropped);
		rad_free(&sock->packet);
		request->packet = NULL;
		return 0;
	}

	sock->packet = NULL;	/* we have no need for more partial reads */
	request->packet = NULL;

	return 1;
}


/*
 *	Send a response packet
 */
int dual_tls_send(rad_listen_t *listener, REQUEST *request)
{
	listen_socket_t *sock = listener->data;

	rad_assert(request->listener == listener);
	rad_assert(listener->send == dual_tls_send);

	/*
	 *	Accounting reject's are silently dropped.
	 *
	 *	We do it here to avoid polluting the rest of the
	 *	code with this knowledge
	 */
	if (request->reply->code == 0) return 0;

	/*
	 *	Pack the VPs
	 */
	if (rad_encode(request->reply, request->packet,
		       request->client->secret) < 0) {
		RDEBUG("Failed encoding packet: %s", fr_strerror());
		return 0;
	}

	/*
	 *	Sign the packet.
	 */
	if (rad_sign(request->reply, request->packet,
		       request->client->secret) < 0) {
		RDEBUG("Failed signing packet: %s", fr_strerror());
		return 0;
	}

	PTHREAD_MUTEX_LOCK(&sock->mutex);
	/*
	 *	Write the packet to the SSL buffers.
	 */
	sock->ssn->record_plus(&sock->ssn->clean_in,
			       request->reply->data, request->reply->data_len);

	/*
	 *	Do SSL magic to get encrypted data.
	 */
	tls_handshake_send(request, sock->ssn);

	/*
	 *	And finally write the data to the socket.
	 */
	if (sock->ssn->dirty_out.used > 0) {
		dump_hex("WRITE TO SSL", sock->ssn->dirty_out.data, sock->ssn->dirty_out.used);

		tls_socket_write(listener, request);
	}
	PTHREAD_MUTEX_UNLOCK(&sock->mutex);

	return 0;
}


int proxy_tls_recv(rad_listen_t *listener)
{
	int rcode;
	size_t length;
	listen_socket_t *sock = listener->data;
	char buffer[256];
	RADIUS_PACKET *packet;
	uint8_t *data;

	/*
	 *	Get the maximum size of data to receive.
	 */
	if (!sock->data) sock->data = talloc_array(sock, uint8_t,
						   sock->ssn->offset);
	data = sock->data;

	DEBUG3("Proxy SSL socket has data to read");
	PTHREAD_MUTEX_LOCK(&sock->mutex);
redo:
	rcode = SSL_read(sock->ssn->ssl, data, 4);
	if (rcode <= 0) {
		int err = SSL_get_error(sock->ssn->ssl, rcode);
		switch (err) {
		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:
			goto redo;

		case SSL_ERROR_ZERO_RETURN:
			/* remote end sent close_notify, send one back */
			SSL_shutdown(sock->ssn->ssl);

		case SSL_ERROR_SYSCALL:
		do_close:
			PTHREAD_MUTEX_UNLOCK(&sock->mutex);
			tls_socket_close(listener);
			return 0;

		default:
			while ((err = ERR_get_error())) {
				DEBUG("proxy recv says %s",
				      ERR_error_string(err, NULL));
			}

			goto do_close;
		}
	}

	length = (data[2] << 8) | data[3];
	DEBUG3("Proxy received header saying we have a packet of %u bytes",
	       (unsigned int) length);

	if (length > sock->ssn->offset) {
		INFO("Received packet will be too large! Set \"fragment_size=%u\"",
		       (data[2] << 8) | data[3]);
		goto do_close;
	}

	rcode = SSL_read(sock->ssn->ssl, data + 4, length);
	if (rcode <= 0) {
		switch (SSL_get_error(sock->ssn->ssl, rcode)) {
		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:
			break;

		case SSL_ERROR_ZERO_RETURN:
			/* remote end sent close_notify, send one back */
			SSL_shutdown(sock->ssn->ssl);
			goto do_close;
		default:
			goto do_close;
		}
	}
	PTHREAD_MUTEX_UNLOCK(&sock->mutex);

	packet = rad_alloc(NULL, 0);
	packet->sockfd = listener->fd;
	packet->src_ipaddr = sock->other_ipaddr;
	packet->src_port = sock->other_port;
	packet->dst_ipaddr = sock->my_ipaddr;
	packet->dst_port = sock->my_port;
	packet->code = data[0];
	packet->id = data[1];
	packet->data_len = length;
	packet->data = talloc_array(packet, uint8_t, packet->data_len);
	memcpy(packet->data, data, packet->data_len);
	memcpy(packet->vector, packet->data + 4, 16);

	/*
	 *	FIXME: Client MIB updates?
	 */
	switch(packet->code) {
	case PW_AUTHENTICATION_ACK:
	case PW_ACCESS_CHALLENGE:
	case PW_AUTHENTICATION_REJECT:
		break;

#ifdef WITH_ACCOUNTING
	case PW_ACCOUNTING_RESPONSE:
		break;
#endif

	default:
		/*
		 *	FIXME: Update MIB for packet types?
		 */
		ERROR("Invalid packet code %d sent to a proxy port "
		       "from home server %s port %d - ID %d : IGNORED",
		       packet->code,
		       ip_ntoh(&packet->src_ipaddr, buffer, sizeof(buffer)),
		       packet->src_port, packet->id);
		rad_free(&packet);
		return 0;
	}

	if (!request_proxy_reply(packet)) {
		rad_free(&packet);
		return 0;
	}

	return 1;
}

int proxy_tls_send(rad_listen_t *listener, REQUEST *request)
{
	int rcode;
	listen_socket_t *sock = listener->data;

	/*
	 *	Normal proxying calls us with the data already
	 *	encoded.  The "ping home server" code does not.  So,
	 *	if there's no packet, encode it here.
	 */
	if (!request->proxy->data) {
		request->proxy_listener->encode(request->proxy_listener,
						request);
	}

	DEBUG3("Proxy is writing %u bytes to SSL",
	       (unsigned int) request->proxy->data_len);
	PTHREAD_MUTEX_LOCK(&sock->mutex);
	while ((rcode = SSL_write(sock->ssn->ssl, request->proxy->data,
				  request->proxy->data_len)) < 0) {
		int err;
		while ((err = ERR_get_error())) {
			DEBUG("proxy SSL_write says %s",
			      ERR_error_string(err, NULL));
		}
		PTHREAD_MUTEX_UNLOCK(&sock->mutex);
		tls_socket_close(listener);
		return 0;
	}
	PTHREAD_MUTEX_UNLOCK(&sock->mutex);

	return 1;
}

#endif	/* WITH_TLS */
