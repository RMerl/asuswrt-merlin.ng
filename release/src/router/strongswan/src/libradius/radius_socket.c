/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

/*
 * Copyright (C) 2015 Thom Troy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "radius_socket.h"
#include "radius_mppe.h"

#include <errno.h>
#include <unistd.h>
#include <math.h>

#include <pen/pen.h>
#include <utils/debug.h>

typedef struct private_radius_socket_t private_radius_socket_t;

/**
 * Private data of an radius_socket_t object.
 */
struct private_radius_socket_t {

	/**
	 * Public radius_socket_t interface.
	 */
	radius_socket_t public;

	/**
	 * Server port for authentication
	 */
	uint16_t auth_port;

	/**
	 * socket file descriptor for authentication
	 */
	int auth_fd;

	/**
	 * Server port for accounting
	 */
	uint16_t acct_port;

	/**
	 * socket file descriptor for accounting
	 */
	int acct_fd;

	/**
	 * Server address
	 */
	char *address;

	/**
	 * current RADIUS identifier
	 */
	uint8_t identifier;

	/**
	 * hasher to use for response verification
	 */
	hasher_t *hasher;

	/**
	 * HMAC-MD5 signer to build Message-Authenticator attribute
	 */
	signer_t *signer;

	/**
	 * random number generator for RADIUS request authenticator
	 */
	rng_t *rng;

	/**
	 * RADIUS secret
	 */
	chunk_t secret;

	/**
	 * Number of times we retransmit messages before giving up
	 */
	u_int retransmit_tries;

	/**
	 * Retransmission timeout
	 */
	double retransmit_timeout;

	/**
	 * Base to calculate retransmission timeout
	 */
	double retransmit_base;
};

/**
 * Check or establish RADIUS connection
 */
static bool check_connection(private_radius_socket_t *this,
							 int *fd, uint16_t port)
{
	if (*fd == -1)
	{
		host_t *server;

		server = host_create_from_dns(this->address, AF_UNSPEC, port);
		if (!server)
		{
			DBG1(DBG_CFG, "resolving RADIUS server address '%s' failed",
				 this->address);
			return FALSE;
		}
		*fd = socket(server->get_family(server), SOCK_DGRAM, IPPROTO_UDP);
		if (*fd == -1)
		{
			DBG1(DBG_CFG, "opening RADIUS socket for %#H failed: %s",
				 server, strerror(errno));
			server->destroy(server);
			return FALSE;
		}
		if (connect(*fd, server->get_sockaddr(server),
					*server->get_sockaddr_len(server)) < 0)
		{
			DBG1(DBG_CFG, "connecting RADIUS socket to %#H failed: %s",
				 server, strerror(errno));
			server->destroy(server);
			close(*fd);
			*fd = -1;
			return FALSE;
		}
		server->destroy(server);
	}
	return TRUE;
}

/**
 * Receive the response to the message with the given ID
 */
static status_t receive_response(int fd, int timeout, uint8_t id,
								 radius_message_t **response)
{
	radius_message_t *msg;
	char buf[4096];
	int res;
	struct pollfd pfd = {
		.fd = fd,
		.events = POLLIN,
	};

	while (TRUE)
	{
		res = poll(&pfd, 1, timeout);
		if (res < 0)
		{
			DBG1(DBG_CFG, "waiting for RADIUS message failed: %s",
				 strerror(errno));
			return FAILED;
		}
		if (res == 0)
		{	/* timeout */
			return OUT_OF_RES;
		}
		res = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);
		if (res <= 0)
		{
			DBG1(DBG_CFG, "receiving RADIUS message failed: %s",
				 strerror(errno));
			return FAILED;
		}
		msg = radius_message_parse(chunk_create(buf, res));
		if (!msg)
		{
			DBG1(DBG_CFG, "received invalid RADIUS message, ignored");
			return FAILED;
		}
		if (id != msg->get_identifier(msg))
		{
			/* we haven't received the response to our current request, but
			 * perhaps one for an earlier request for which we didn't wait
			 * long enough */
			DBG1(DBG_CFG, "received RADIUS message with unexpected ID %d "
				 "[%d expected], ignored", msg->get_identifier(msg), id);
			msg->destroy(msg);
			continue;
		}
		*response = msg;
		return SUCCESS;
	}
}

METHOD(radius_socket_t, request, radius_message_t*,
	private_radius_socket_t *this, radius_message_t *request)
{
	radius_message_t *response;
	chunk_t data;
	int *fd, retransmit = 0, timeout;
	uint16_t port;
	rng_t *rng = NULL;

	if (request->get_code(request) == RMC_ACCOUNTING_REQUEST)
	{
		fd = &this->acct_fd;
		port = this->acct_port;
	}
	else
	{
		fd = &this->auth_fd;
		port = this->auth_port;
		rng = this->rng;
	}

	/* set Message Identifier */
	request->set_identifier(request, this->identifier++);
	/* sign the request */
	if (!request->sign(request, NULL, this->secret, this->hasher, this->signer,
					   rng, rng != NULL))
	{
		return NULL;
	}

	if (!check_connection(this, fd, port))
	{
		return NULL;
	}

	data = request->get_encoding(request);
	DBG3(DBG_CFG, "%B", &data);

	while (retransmit < this->retransmit_tries)
	{
		timeout = (int)(this->retransmit_timeout * 1000.0 *
						pow(this->retransmit_base, retransmit));
		if (retransmit)
		{
			DBG1(DBG_CFG, "retransmit %d of RADIUS %N (timeout: %.1fs)",
				 retransmit, radius_message_code_names,
				 request->get_code(request), timeout/1000.0);
		}
		if (send(*fd, data.ptr, data.len, 0) != data.len)
		{
			DBG1(DBG_CFG, "sending RADIUS message failed: %s", strerror(errno));
			return NULL;
		}
		switch (receive_response(*fd, timeout, request->get_identifier(request),
								 &response))
		{
			case SUCCESS:
				break;
			case OUT_OF_RES:
				retransmit++;
				continue;
			default:
				return NULL;
		}
		if (response->verify(response, request->get_authenticator(request),
							 this->secret, this->hasher, this->signer))
		{
			return response;
		}
		response->destroy(response);
		return NULL;
	}

	DBG1(DBG_CFG, "RADIUS %N timed out after %d attempts",
		 radius_message_code_names, request->get_code(request), retransmit);
	return NULL;
}

/**
 * Decrypt a MS-MPPE-Send/Recv-Key
 */
static chunk_t decrypt_mppe_key(private_radius_socket_t *this, uint16_t salt,
								chunk_t C, radius_message_t *request)
{
	chunk_t decrypted;

	decrypted = chunk_alloca(C.len);
	if (!request->crypt(request, chunk_from_thing(salt), C, decrypted,
						this->secret, this->hasher) ||
		decrypted.ptr[0] >= decrypted.len)
	{	/* decryption failed? */
		return chunk_empty;
	}
	/* remove truncation, first byte is key length */
	return chunk_clone(chunk_create(decrypted.ptr + 1, decrypted.ptr[0]));
}

METHOD(radius_socket_t, decrypt_msk, chunk_t,
	private_radius_socket_t *this, radius_message_t *request,
	radius_message_t *response)
{
	mppe_key_t *mppe_key;
	enumerator_t *enumerator;
	chunk_t data, send = chunk_empty, recv = chunk_empty;
	int type;

	enumerator = response->create_enumerator(response);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		if (type == RAT_VENDOR_SPECIFIC && data.len > sizeof(mppe_key_t))
		{
			mppe_key = (mppe_key_t*)data.ptr;
			if (ntohl(mppe_key->id) == PEN_MICROSOFT &&
				mppe_key->length == data.len - sizeof(mppe_key->id))
			{
				data = chunk_create(mppe_key->key, data.len - sizeof(mppe_key_t));
				if (mppe_key->type == MS_MPPE_SEND_KEY)
				{
					send = decrypt_mppe_key(this, mppe_key->salt, data, request);
				}
				if (mppe_key->type == MS_MPPE_RECV_KEY)
				{
					recv = decrypt_mppe_key(this, mppe_key->salt, data, request);
				}
			}
		}
	}
	enumerator->destroy(enumerator);
	if (send.ptr && recv.ptr)
	{
		chunk_t pad = chunk_empty;

		if ((send.len + recv.len) < 64)
		{	/* zero-pad MSK to at least 64 bytes */
			pad = chunk_alloca(64 - send.len - recv.len);
			memset(pad.ptr, 0, pad.len);
		}
		return chunk_cat("mmc", recv, send, pad);
	}
	chunk_clear(&send);
	chunk_clear(&recv);
	return chunk_empty;
}

METHOD(radius_socket_t, destroy, void,
	private_radius_socket_t *this)
{
	DESTROY_IF(this->hasher);
	DESTROY_IF(this->signer);
	DESTROY_IF(this->rng);
	if (this->auth_fd != -1)
	{
		close(this->auth_fd);
	};
	if (this->acct_fd != -1)
	{
		close(this->acct_fd);
	}
	free(this);
}

/**
 * See header
 */
radius_socket_t *radius_socket_create(char *address, uint16_t auth_port,
									  uint16_t acct_port, chunk_t secret,
									  u_int tries, double timeout, double base)
{
	private_radius_socket_t *this;

	INIT(this,
		.public = {
			.request = _request,
			.decrypt_msk = _decrypt_msk,
			.destroy = _destroy,
		},
		.address = address,
		.auth_port = auth_port,
		.auth_fd = -1,
		.acct_port = acct_port,
		.acct_fd = -1,
		.hasher = lib->crypto->create_hasher(lib->crypto, HASH_MD5),
		.signer = lib->crypto->create_signer(lib->crypto, AUTH_HMAC_MD5_128),
		.rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK),
		.retransmit_tries = tries,
		.retransmit_timeout = timeout,
		.retransmit_base = base,
	);

	if (!this->hasher || !this->signer || !this->rng ||
		!this->signer->set_key(this->signer, secret))
	{
		DBG1(DBG_CFG, "RADIUS initialization failed, HMAC/MD5/RNG required");
		destroy(this);
		return NULL;
	}
	this->secret = secret;
	/* we use a random identifier, helps if we restart often */
	this->identifier = random();

	return &this->public;
}
