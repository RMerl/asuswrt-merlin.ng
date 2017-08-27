/*
 * Copyright (C) 2012-2013 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "tnc_pdp.h"
#include "tnc_pdp_connections.h"

#include <errno.h>
#include <unistd.h>
#include <time.h>

#include <radius_message.h>
#include <radius_mppe.h>

#include <pt_tls_server.h>

#include <tnc/tnc.h>

#include <tncifimv.h>
#include <tncif_names.h>

#include <daemon.h>
#include <utils/debug.h>
#include <pen/pen.h>
#include <threading/thread.h>
#include <processing/jobs/callback_job.h>
#include <sa/eap/eap_method.h>

typedef struct private_tnc_pdp_t private_tnc_pdp_t;
typedef struct client_entry_t client_entry_t;
/**
 * Default RADIUS port, when not configured
 */
#define RADIUS_PORT 1812

/**
 * Maximum size of a RADIUS IP packet
 */
#define MAX_PACKET 4096

#define RADIUS_RETRANSMIT_TIMEOUT	30 /* seconds */

/**
 * private data of tnc_pdp_t
 */
struct private_tnc_pdp_t {

	/**
	 * implements tnc_pdp_t interface
	 */
	tnc_pdp_t public;

	/**
	 * ID of the server
	 */
	identification_t *server;

	/**
	 * EAP method type to be used
	 */
	eap_type_t type;

	/**
	 * PT-TLS port of the server
	 */
	u_int16_t pt_tls_port;

	/**
	 * PT-TLS IPv4 socket
	 */
	int pt_tls_ipv4;

	/**
	 * PT-TLS IPv6 socket
	 */
	int pt_tls_ipv6;

	/**
	 * RADIUS IPv4 socket
	 */
	int radius_ipv4;

	/**
	 * RADIUS IPv6 socket
	 */
	int radius_ipv6;

	/**
	 * RADIUS shared secret
	 */
	chunk_t secret;

	/**
	 * RADIUS clients
	 */
	linked_list_t *clients;

	/**
	 * MD5 hasher
	 */
	hasher_t *hasher;

	/**
	 * HMAC MD5 signer, with secret set
	 */
	signer_t *signer;

	/**
	 * Nonce generator for MS-MPPE salt values
	 */
	nonce_gen_t *ng;

	/**
	 * List of registered TNC-PDP connections
	 */
	tnc_pdp_connections_t *connections;

};

/**
 * Client entry helping to detect RADIUS packet retransmissions
 */
struct client_entry_t {

	/**
	 * IP host address and port of client
	 */
	host_t *host;

	/**
	 * Time of last RADIUS Access-Request received from client
	 */
	time_t last_time;

	/**
	 * Identifier of last RADIUS Access-Request received from client
	 */
	uint8_t last_id;
};

static void free_client_entry(client_entry_t *this)
{
	this->host->destroy(this->host);
	free(this);
}

/**
 * Open IPv4 or IPv6 UDP socket
 */
static int open_udp_socket(int family, u_int16_t port)
{
	int on = TRUE;
	struct sockaddr_storage addr;
	socklen_t addrlen;
	int skt;

	memset(&addr, 0, sizeof(addr));
	addr.ss_family = family;

	/* precalculate constants depending on address family */
	switch (family)
	{
		case AF_INET:
		{
			struct sockaddr_in *sin = (struct sockaddr_in *)&addr;

			htoun32(&sin->sin_addr.s_addr, INADDR_ANY);
			htoun16(&sin->sin_port, port);
			addrlen = sizeof(struct sockaddr_in);
			break;
		}
		case AF_INET6:
		{
			struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&addr;

			memcpy(&sin6->sin6_addr, &in6addr_any, sizeof(in6addr_any));
			htoun16(&sin6->sin6_port, port);
			addrlen = sizeof(struct sockaddr_in6);
			break;
		}
		default:
			return 0;
	}

	/* open the socket */
	skt = socket(family, SOCK_DGRAM, IPPROTO_UDP);
	if (skt < 0)
	{
		DBG1(DBG_CFG, "opening UDP socket failed: %s", strerror(errno));
		return 0;
	}
	if (setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) < 0)
	{
		DBG1(DBG_CFG, "unable to set SO_REUSEADDR on socket: %s",
					   strerror(errno));
		close(skt);
		return 0;
	}
	if (family == AF_INET6)
	{
		if (setsockopt(skt, IPPROTO_IPV6, IPV6_V6ONLY,
							(void *)&on, sizeof(on)) < 0)
		{
			DBG1(DBG_CFG, "unable to set IPV6_V6ONLY on socket: %s",
						   strerror(errno));
			close(skt);
			return 0;
		}
	}

	/* bind the socket */
	if (bind(skt, (struct sockaddr *)&addr, addrlen) < 0)
	{
		DBG1(DBG_CFG, "unable to bind UDP socket: %s", strerror(errno));
		close(skt);
		return 0;
	}

	return skt;
}

/**
 * Open IPv4 or IPv6 TCP socket
 */
static int open_tcp_socket(int family, u_int16_t port)
{
	int on = TRUE;
	struct sockaddr_storage addr;
	socklen_t addrlen;
	int skt;

	memset(&addr, 0, sizeof(addr));
	addr.ss_family = family;

	/* precalculate constants depending on address family */
	switch (family)
	{
		case AF_INET:
		{
			struct sockaddr_in *sin = (struct sockaddr_in *)&addr;

			htoun32(&sin->sin_addr.s_addr, INADDR_ANY);
			htoun16(&sin->sin_port, port);
			addrlen = sizeof(struct sockaddr_in);
			break;
		}
		case AF_INET6:
		{
			struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&addr;

			memcpy(&sin6->sin6_addr, &in6addr_any, sizeof(in6addr_any));
			htoun16(&sin6->sin6_port, port);
			addrlen = sizeof(struct sockaddr_in6);
			break;
		}
		default:
			return 0;
	}

	/* open the socket */
	skt = socket(family, SOCK_STREAM, IPPROTO_TCP);
	if (skt < 0)
	{
		DBG1(DBG_CFG, "opening TCP socket failed: %s", strerror(errno));
		return 0;
	}
	if (setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) < 0)
	{
		DBG1(DBG_CFG, "unable to set SO_REUSEADDR on socket: %s",
					   strerror(errno));
		close(skt);
		return 0;
	}
	if (family == AF_INET6)
	{
	if (setsockopt(skt, IPPROTO_IPV6, IPV6_V6ONLY,
							(void *)&on, sizeof(on)) < 0)
		{
			DBG1(DBG_CFG, "unable to set IPV6_V6ONLY on socket: %s",
						   strerror(errno));
			close(skt);
			return 0;
		}
	}

	/* bind the socket */
	if (bind(skt, (struct sockaddr *)&addr, addrlen) < 0)
	{
		DBG1(DBG_CFG, "unable to bind TCP socket: %s", strerror(errno));
		close(skt);
		return 0;
	}

	/* start listening on socket */
	if (listen(skt, 5) == -1)
	{
		DBG1(DBG_TNC, "listen on TCP socket failed: %s", strerror(errno));
		close(skt);
		return 0;
	}

	return skt;
}

/**
 * Send a RADIUS message to client
 */
static void send_message(private_tnc_pdp_t *this, radius_message_t *message,
						 host_t *client)
{
	int fd;
	chunk_t data;

	fd = (client->get_family(client) == AF_INET) ?
			this->radius_ipv4 : this->radius_ipv6;
	data = message->get_encoding(message);

	DBG2(DBG_CFG, "sending RADIUS packet to %#H", client);
	DBG3(DBG_CFG, "%B", &data);

	if (sendto(fd, data.ptr, data.len, 0, client->get_sockaddr(client),
			   *client->get_sockaddr_len(client)) != data.len)
	{
		DBG1(DBG_CFG, "sending RADIUS message failed: %s", strerror(errno));
	}
}

/**
 * Encrypt a MS-MPPE-Send/Recv-Key
 */
static chunk_t encrypt_mppe_key(private_tnc_pdp_t *this, u_int8_t type,
								chunk_t key, u_int16_t *salt,
								radius_message_t *request)
{
	chunk_t a, r, seed, data;
	u_char b[HASH_SIZE_MD5], *c;
	mppe_key_t *mppe_key;

	/**
	 * From RFC2548 (encryption):
	 * b(1) = MD5(S + R + A)    c(1) = p(1) xor b(1)   C = c(1)
	 * b(2) = MD5(S + c(1))     c(2) = p(2) xor b(2)   C = C + c(2)
	 *      . . .
	 * b(i) = MD5(S + c(i-1))   c(i) = p(i) xor b(i)   C = C + c(i)
	 */

	data = chunk_alloc(sizeof(mppe_key_t) +
					   HASH_SIZE_MD5 * (1 + key.len / HASH_SIZE_MD5));
	memset(data.ptr, 0x00, data.len);

	mppe_key = (mppe_key_t*)data.ptr;
	mppe_key->id = htonl(PEN_MICROSOFT);
	mppe_key->type = type;
	mppe_key->length = data.len - sizeof(mppe_key->id);
	mppe_key->key[0] = key.len;

	memcpy(&mppe_key->key[1], key.ptr, key.len);

	/**
	 * generate a 16 bit unique random salt value for the MPPE stream cipher
	 * the MSB of the salt MUST be set to 1
	 */
	a = chunk_create((u_char*)&(mppe_key->salt), sizeof(mppe_key->salt));
	do
	{
		if (!this->ng->get_nonce(this->ng, a.len, a.ptr))
		{
			free(data.ptr);
			return chunk_empty;
		}
		*a.ptr |= 0x80;
	}
	while (mppe_key->salt == *salt);

	/* update the salt value */
	*salt = mppe_key->salt;

	r = chunk_create(request->get_authenticator(request), HASH_SIZE_MD5);
	seed = chunk_cata("cc", r, a);

	c = mppe_key->key;
	while (c < data.ptr + data.len)
	{
		/* b(i) = MD5(S + c(i-1)) */
		if (!this->hasher->get_hash(this->hasher, this->secret, NULL) ||
			!this->hasher->get_hash(this->hasher, seed, b))
		{
			free(data.ptr);
			return chunk_empty;
		}

		/* c(i) = b(i) xor p(1) */
		memxor(c, b, HASH_SIZE_MD5);

		/* prepare next round */
		seed = chunk_create(c, HASH_SIZE_MD5);
		c += HASH_SIZE_MD5;
	}

	return data;
}

/**
 * Send a RADIUS response for a request
 */
static void send_response(private_tnc_pdp_t *this, radius_message_t *request,
						  radius_message_code_t code, eap_payload_t *eap,
						  identification_t *group, chunk_t msk, host_t *client)
{
	radius_message_t *response;
	chunk_t data, recv, send;
	u_int32_t tunnel_type;
	u_int16_t salt = 0;

	response = radius_message_create(code);
	data = eap->get_data(eap);
	DBG3(DBG_CFG, "%N payload %B", eap_type_names, this->type, &data);

	/* fragment data suitable for RADIUS */
	while (data.len > MAX_RADIUS_ATTRIBUTE_SIZE)
	{
		response->add(response, RAT_EAP_MESSAGE,
					  chunk_create(data.ptr, MAX_RADIUS_ATTRIBUTE_SIZE));
		data = chunk_skip(data, MAX_RADIUS_ATTRIBUTE_SIZE);
	}
	response->add(response, RAT_EAP_MESSAGE, data);

	if (group)
	{
		tunnel_type = RADIUS_TUNNEL_TYPE_ESP;
		htoun32(data.ptr, tunnel_type);
		data.len = sizeof(tunnel_type);
		response->add(response, RAT_TUNNEL_TYPE, data);
		response->add(response, RAT_FILTER_ID, group->get_encoding(group));
	}
	if (msk.len)
	{
		recv = chunk_create(msk.ptr, msk.len / 2);
		data = encrypt_mppe_key(this, MS_MPPE_RECV_KEY, recv, &salt, request);
		response->add(response, RAT_VENDOR_SPECIFIC, data);
		chunk_free(&data);

		send = chunk_create(msk.ptr + recv.len, msk.len - recv.len);
		data = encrypt_mppe_key(this, MS_MPPE_SEND_KEY, send, &salt, request);
		response->add(response, RAT_VENDOR_SPECIFIC, data);
		chunk_free(&data);
	}
	response->set_identifier(response, request->get_identifier(request));
	if (response->sign(response, request->get_authenticator(request),
					   this->secret, this->hasher, this->signer, NULL, TRUE))
	{
		DBG1(DBG_CFG, "sending RADIUS %N to client '%H'",
			 radius_message_code_names, code, client);
		send_message(this, response, client);
	}
	response->destroy(response);
}

/**
 * Process EAP message
 */
static void process_eap(private_tnc_pdp_t *this, radius_message_t *request,
						host_t *source)
{
	enumerator_t *enumerator;
	eap_payload_t *in, *out = NULL;
	eap_method_t *method;
	eap_type_t eap_type;
	u_int32_t eap_vendor;
	chunk_t data, message = chunk_empty, msk = chunk_empty;
	chunk_t user_name = chunk_empty, nas_id = chunk_empty;
	identification_t *group = NULL;
	radius_message_code_t code = RMC_ACCESS_CHALLENGE;
	int type;

	enumerator = request->create_enumerator(request);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		switch (type)
		{
			case RAT_USER_NAME:
				user_name = data;
				break;
			case RAT_NAS_IDENTIFIER:
				nas_id = data;
				break;
			case RAT_EAP_MESSAGE:
				if (data.len)
				{
					message = chunk_cat("mc", message, data);
				}
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (message.len)
	{
		in = eap_payload_create_data(message);

		/* apply EAP method selected by RADIUS server */
		eap_type = in->get_type(in, &eap_vendor);

		DBG3(DBG_CFG, "%N payload %B", eap_type_names, eap_type, &message);

		if (eap_type == EAP_IDENTITY)
		{
			identification_t *peer;
			chunk_t eap_identity;

			if (message.len < 5)
			{
				goto end;
			}
			eap_identity = chunk_create(message.ptr + 5, message.len - 5);
			peer = identification_create_from_data(eap_identity);
			method = charon->eap->create_instance(charon->eap, this->type,
										0, EAP_SERVER, this->server, peer);
			if (!method)
			{
				peer->destroy(peer);
				goto end;
			}
			this->connections->add(this->connections, nas_id, user_name, peer,
								   method);
			if (method->initiate(method, &out) == NEED_MORE)
			{
				send_response(this, request, code, out, group, msk, source);
			}
		}
		else
		{
			ike_sa_t *ike_sa;
			auth_cfg_t *auth;
			auth_rule_t type;
			identification_t *data;
			enumerator_t *e;

			method = this->connections->get_state(this->connections, nas_id,
												  user_name, &ike_sa);
			if (!method)
			{
				goto end;
			}
			charon->bus->set_sa(charon->bus, ike_sa);

			switch (method->process(method, in, &out))
			{
				case NEED_MORE:
					code = RMC_ACCESS_CHALLENGE;
					break;
				case SUCCESS:
					code = RMC_ACCESS_ACCEPT;
					method->get_msk(method, &msk);
					auth = ike_sa->get_auth_cfg(ike_sa, FALSE);
					e = auth->create_enumerator(auth);
					while (e->enumerate(e, &type, &data))
					{
						/* look for group memberships */
						if (type == AUTH_RULE_GROUP)
						{
							group = data;
						}
					}
					e->destroy(e);

					DESTROY_IF(out);
					out = eap_payload_create_code(EAP_SUCCESS,
												  in->get_identifier(in));
					break;
				case FAILED:
				default:
					code = RMC_ACCESS_REJECT;
					DESTROY_IF(out);
					out = eap_payload_create_code(EAP_FAILURE,
												  in->get_identifier(in));
			}
			charon->bus->set_sa(charon->bus, NULL);
			send_response(this, request, code, out, group, msk, source);
			this->connections->unlock(this->connections);
		}

		if (code == RMC_ACCESS_ACCEPT || code == RMC_ACCESS_REJECT)
		{
			this->connections->remove(this->connections, nas_id, user_name);
		}

		out->destroy(out);
end:
		free(message.ptr);
		in->destroy(in);
	}
}

/**
 * Callback function to get recommendation from TNCCS connection
 */
static bool get_recommendation(TNC_IMV_Action_Recommendation rec,
							   TNC_IMV_Evaluation_Result eval)
{
	DBG1(DBG_TNC, "final recommendation is '%N' and evaluation is '%N'",
		 TNC_IMV_Action_Recommendation_names, rec,
		 TNC_IMV_Evaluation_Result_names, eval);

	return TRUE;
}

/**
 * Get more data on a PT-TLS connection
 */
static bool pt_tls_receive_more(pt_tls_server_t *this, int fd,
								watcher_event_t event)
{
	switch (this->handle(this))
	{
		case NEED_MORE:
			return TRUE;
		case FAILED:
		case SUCCESS:
		default:
			DBG1(DBG_TNC, "PT-TLS connection terminates");
			this->destroy(this);
			close(fd);
			return FALSE;
	}
}

/**
 * Accept TCP connection received on the PT-TLS listening socket
 */
static bool pt_tls_receive(private_tnc_pdp_t *this, int fd, watcher_event_t event)
{
	int pt_tls_fd;
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);
	identification_t *peer;
	host_t *host;
	pt_tls_server_t *pt_tls;
	tnccs_t *tnccs;
	pt_tls_auth_t auth = PT_TLS_AUTH_TLS_OR_SASL;

	pt_tls_fd = accept(fd, (sockaddr_t*)&addr, &addrlen);
	if (pt_tls_fd == -1)
	{
		DBG1(DBG_TNC, "accepting PT-TLS stream failed: %s", strerror(errno));
		return FALSE;
	}
	host = host_create_from_sockaddr((sockaddr_t*)&addr);
	DBG1(DBG_TNC, "accepting PT-TLS stream from %H", host);
	host->destroy(host);

	/* At this moment the peer identity is not known yet */
	peer = identification_create_from_encoding(ID_ANY, chunk_empty),

	tnccs = tnc->tnccs->create_instance(tnc->tnccs, TNCCS_2_0, TRUE,
										this->server, peer, TNC_IFT_TLS_2_0,
										(tnccs_cb_t)get_recommendation);
	peer->destroy(peer);

	if (!tnccs)
	{
		DBG1(DBG_TNC, "could not create TNCCS 2.0 connection instance");
		close(pt_tls_fd);
		return FALSE;
	}

	pt_tls = pt_tls_server_create(this->server, pt_tls_fd, auth, tnccs);
	if (!pt_tls)
	{
		DBG1(DBG_TNC, "could not create PT-TLS connection instance");
		close(pt_tls_fd);
		return FALSE;
	}

	lib->watcher->add(lib->watcher, pt_tls_fd, WATCHER_READ,
							 (watcher_cb_t)pt_tls_receive_more, pt_tls);

	return TRUE;
}

/**
 * Process packets received on the RADIUS socket
 */
static bool radius_receive(private_tnc_pdp_t *this, int fd, watcher_event_t event)
{
	radius_message_t *request;
	char buffer[MAX_PACKET];
	client_entry_t *client;
	bool retransmission = FALSE, found = FALSE, stale;
	enumerator_t *enumerator;
	int bytes_read = 0;
	host_t *source;
	uint8_t id;
	time_t now;

	union {
		struct sockaddr_in in4;
		struct sockaddr_in6 in6;
	} src;

	struct iovec iov = {
		.iov_base = buffer,
		.iov_len = MAX_PACKET,
	};

	struct msghdr msg = {
		.msg_name = &src,
		.msg_namelen = sizeof(src),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};

	/* read received packet */
	bytes_read = recvmsg(fd, &msg, 0);
	if (bytes_read < 0)
	{
		DBG1(DBG_CFG, "error reading RADIUS socket: %s", strerror(errno));
		return FALSE;
	}
	if (msg.msg_flags & MSG_TRUNC)
	{
		DBG1(DBG_CFG, "receive buffer too small, RADIUS packet discarded");
		return FALSE;
	}
	source = host_create_from_sockaddr((sockaddr_t*)&src);
	DBG2(DBG_CFG, "received RADIUS packet from %#H", source);
	DBG3(DBG_CFG, "%b", buffer, bytes_read);
	request = radius_message_parse(chunk_create(buffer, bytes_read));
	if (request)
	{
		DBG1(DBG_CFG, "received RADIUS %N from client '%H'",
		radius_message_code_names, request->get_code(request), source);

		if (request->verify(request, NULL, this->secret, this->hasher,
										   this->signer))
		{
			id = request->get_identifier(request);
			now = time(NULL);

			enumerator = this->clients->create_enumerator(this->clients);
			while (enumerator->enumerate(enumerator, &client))
			{
				stale = client->last_time < now - RADIUS_RETRANSMIT_TIMEOUT;

				if (source->equals(source, client->host))
				{
					retransmission = !stale && client->last_id == id;
					client->last_id = id;
					client->last_time = now;
					found = TRUE;
				}
				else if (stale)
				{
					this->clients->remove_at(this->clients, enumerator);
					free_client_entry(client);
				}
			}
			enumerator->destroy(enumerator);

			if (!found)
			{
				client = malloc_thing(client_entry_t);
				client->host = source->clone(source);
				client->last_id = id;
				client->last_time = now;
				this->clients->insert_last(this->clients, client);
			}
			if (retransmission)
			{
				DBG1(DBG_CFG, "ignoring RADIUS Access-Request 0x%02x, "
							  "already processing", id);
			}
			else
			{
				process_eap(this, request, source);
			}
		}
		request->destroy(request);
	}
	else
	{
		DBG1(DBG_CFG, "received invalid RADIUS message, ignored");
	}
	source->destroy(source);
	return TRUE;
}

METHOD(tnc_pdp_t, destroy, void,
	private_tnc_pdp_t *this)
{
	if (this->pt_tls_ipv4)
	{
		lib->watcher->remove(lib->watcher, this->pt_tls_ipv4);
		close(this->pt_tls_ipv4);
	}
	if (this->pt_tls_ipv6)
	{
		lib->watcher->remove(lib->watcher, this->pt_tls_ipv6);
		close(this->pt_tls_ipv6);
	}
	if (this->radius_ipv4)
	{
		lib->watcher->remove(lib->watcher, this->radius_ipv4);
		close(this->radius_ipv4);
	}
	if (this->radius_ipv6)
	{
		lib->watcher->remove(lib->watcher, this->radius_ipv6);
		close(this->radius_ipv6);
	}
	if (this->clients)
	{
		this->clients->destroy_function(this->clients, (void*)free_client_entry);
	}
	DESTROY_IF(this->server);
	DESTROY_IF(this->signer);
	DESTROY_IF(this->hasher);
	DESTROY_IF(this->ng);
	DESTROY_IF(this->connections);
	free(this);
}

/*
 * see header file
 */
tnc_pdp_t *tnc_pdp_create(void)
{
	private_tnc_pdp_t *this;
	char *secret, *server, *eap_type_str;
	int radius_port, pt_tls_port;
	bool radius_enable, pt_tls_enable;

	server = lib->settings->get_str(lib->settings,
						"%s.plugins.tnc-pdp.server", NULL, lib->ns);
	pt_tls_enable = lib->settings->get_bool(lib->settings,
						"%s.plugins.tnc-pdp.pt_tls.enable", TRUE, lib->ns);
	pt_tls_port = lib->settings->get_int(lib->settings,
						"%s.plugins.tnc-pdp.pt_tls.port", PT_TLS_PORT, lib->ns);
	radius_enable = lib->settings->get_bool(lib->settings,
						"%s.plugins.tnc-pdp.radius.enable", TRUE, lib->ns);
	radius_port = lib->settings->get_int(lib->settings,
						"%s.plugins.tnc-pdp.radius.port", RADIUS_PORT, lib->ns);
	secret = lib->settings->get_str(lib->settings,
						"%s.plugins.tnc-pdp.radius.secret", NULL, lib->ns);
	eap_type_str = lib->settings->get_str(lib->settings,
						"%s.plugins.tnc-pdp.radius.method", "ttls", lib->ns);

	if (!pt_tls_enable && !radius_enable)
	{
		DBG1(DBG_CFG, " neither PT-TLS and RADIUS protocols enabled, PDP disabled");
		return NULL;
	}
	if (!server)
	{
		DBG1(DBG_CFG, "missing PDP server name, PDP disabled");
		return NULL;
	}

	INIT(this,
		.public = {
			.destroy = _destroy,
		},
		.server = identification_create_from_string(server),
		.connections = tnc_pdp_connections_create(),
	);

	/* Create IPv4 and IPv6 PT-TLS listening sockets */
	if (pt_tls_enable)
	{
		this->pt_tls_ipv4 = open_tcp_socket(AF_INET,  pt_tls_port);
		this->pt_tls_ipv6 = open_tcp_socket(AF_INET6, pt_tls_port);

		if (!this->pt_tls_ipv4 && !this->pt_tls_ipv6)
		{
			DBG1(DBG_NET, "could not create any PT-TLS sockets");
			destroy(this);
			return NULL;
		}
		this->pt_tls_port = pt_tls_port;

		if (this->pt_tls_ipv4)
		{
			lib->watcher->add(lib->watcher, this->pt_tls_ipv4, WATCHER_READ,
							 (watcher_cb_t)pt_tls_receive, this);
		}
		else
		{
			DBG1(DBG_NET, "could not open IPv4 PT-TLS socket, IPv4 disabled");
		}

		if (this->pt_tls_ipv6)
		{
			lib->watcher->add(lib->watcher, this->pt_tls_ipv6, WATCHER_READ,
							 (watcher_cb_t)pt_tls_receive, this);
		}
		else
		{
			DBG1(DBG_NET, "could not open IPv6 PT-TLS socket, IPv6 disabled");
		}

		/* register PT-TLS service */
		lib->set(lib, "pt-tls-server", this->server);
		lib->set(lib, "pt-tls-port", &this->pt_tls_port);
	}

	/* Create IPv4 and IPv6 RADIUS listening sockets */
	if (radius_enable)
	{
		if (!secret)
		{
			DBG1(DBG_CFG, "missing RADIUS secret, PDP disabled");
			destroy(this);
			return NULL;
		}

		this->radius_ipv4 = open_udp_socket(AF_INET,  radius_port);
		this->radius_ipv6 = open_udp_socket(AF_INET6, radius_port);
		this->secret = chunk_from_str(secret);
		this->clients = linked_list_create();
		this->type = eap_type_from_string(eap_type_str);
		this->hasher = lib->crypto->create_hasher(lib->crypto, HASH_MD5);
		this->signer = lib->crypto->create_signer(lib->crypto, AUTH_HMAC_MD5_128);
		this->ng = lib->crypto->create_nonce_gen(lib->crypto);

		if (!this->hasher || !this->signer || !this->ng)
		{
			DBG1(DBG_CFG, "RADIUS initialization failed, HMAC/MD5/NG required");
			destroy(this);
			return NULL;
		}
		if (!this->radius_ipv4 && !this->radius_ipv6)
		{
			DBG1(DBG_NET, "could not create any RADIUS sockets");
			destroy(this);
			return NULL;
		}
		if (this->radius_ipv4)
		{
			lib->watcher->add(lib->watcher, this->radius_ipv4, WATCHER_READ,
							 (watcher_cb_t)radius_receive, this);
		}
		else
		{
			DBG1(DBG_NET, "could not open IPv4 RADIUS socket, IPv4 disabled");
		}
		if (this->radius_ipv6)
		{
			lib->watcher->add(lib->watcher, this->radius_ipv6, WATCHER_READ,
							 (watcher_cb_t)radius_receive, this);
		}
		else
		{
		DBG1(DBG_NET, "could not open IPv6 RADIUS socket, IPv6 disabled");
		}

		if (!this->signer->set_key(this->signer, this->secret))
		{
			DBG1(DBG_CFG, "could not set signer key");
			destroy(this);
			return NULL;
		}
		if (this->type == 0)
		{
			DBG1(DBG_CFG, "unrecognized eap method \"%s\"", eap_type_str);
			destroy(this);
			return NULL;
		}
		DBG1(DBG_IKE, "eap method %N selected", eap_type_names, this->type);
	}

	return &this->public;
}
