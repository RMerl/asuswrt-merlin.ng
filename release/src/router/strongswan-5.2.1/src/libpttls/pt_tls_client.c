/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include "pt_tls_client.h"
#include "pt_tls.h"

#include <sasl/sasl_mechanism.h>

#include <tls_socket.h>
#include <utils/debug.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

typedef struct private_pt_tls_client_t private_pt_tls_client_t;

/**
 * Private data of an pt_tls_client_t object.
 */
struct private_pt_tls_client_t {

	/**
	 * Public pt_tls_client_t interface.
	 */
	pt_tls_client_t public;

	/**
	 * TLS secured socket used by PT-TLS
	 */
	tls_socket_t *tls;

	/**
	 * Server address/port
	 */
	host_t *address;

	/**
	 * Server identity
	 */
	identification_t *server;

	/**
	 * Client authentication identity
	 */
	identification_t *client;

	/**
	 * Current PT-TLS message identifier
	 */
	u_int32_t identifier;
};

/**
 * Establish TLS secured TCP connection to TNC server
 */
static bool make_connection(private_pt_tls_client_t *this)
{
	int fd;

	fd = socket(this->address->get_family(this->address), SOCK_STREAM, 0);
	if (fd == -1)
	{
		DBG1(DBG_TNC, "opening PT-TLS socket failed: %s", strerror(errno));
		return FALSE;
	}
	if (connect(fd, this->address->get_sockaddr(this->address),
				*this->address->get_sockaddr_len(this->address)) == -1)
	{
		DBG1(DBG_TNC, "connecting to PT-TLS server failed: %s", strerror(errno));
		close(fd);
		return FALSE;
	}

	this->tls = tls_socket_create(FALSE, this->server, this->client, fd,
								  NULL, TLS_1_2, FALSE);
	if (!this->tls)
	{
		close(fd);
		return FALSE;
	}
	return TRUE;
}

/**
 * Negotiate PT-TLS version
 */
static bool negotiate_version(private_pt_tls_client_t *this)
{
	bio_writer_t *writer;
	bio_reader_t *reader;
	u_int32_t type, vendor, identifier, reserved;
	u_int8_t version;
	bool res;

	DBG1(DBG_TNC, "sending offer for PT-TLS version %d", PT_TLS_VERSION);

	writer = bio_writer_create(4);
	writer->write_uint8(writer, 0);
	writer->write_uint8(writer, PT_TLS_VERSION);
	writer->write_uint8(writer, PT_TLS_VERSION);
	writer->write_uint8(writer, PT_TLS_VERSION);
	res = pt_tls_write(this->tls, PT_TLS_VERSION_REQUEST, this->identifier++,
					   writer->get_buf(writer));
	writer->destroy(writer);
	if (!res)
	{
		return FALSE;
	}

	reader = pt_tls_read(this->tls, &vendor, &type, &identifier);
	if (!reader)
	{
		return FALSE;
	}
	if (vendor != 0 || type != PT_TLS_VERSION_RESPONSE ||
		!reader->read_uint24(reader, &reserved) ||
		!reader->read_uint8(reader, &version) ||
		version != PT_TLS_VERSION)
	{
		DBG1(DBG_TNC, "PT-TLS version negotiation failed");
		reader->destroy(reader);
		return FALSE;
	}
	reader->destroy(reader);
	return TRUE;
}

/**
 * Run a SASL mechanism
 */
static status_t do_sasl(private_pt_tls_client_t *this, sasl_mechanism_t *sasl)
{
	u_int32_t type, vendor, identifier;
	u_int8_t result;
	bio_reader_t *reader;
	bio_writer_t *writer;
	chunk_t data;
	bool res;

	writer = bio_writer_create(32);
	writer->write_data8(writer, chunk_from_str(sasl->get_name(sasl)));
	switch (sasl->build(sasl, &data))
	{
		case INVALID_STATE:
			break;
		case NEED_MORE:
			writer->write_data(writer, data);
			free(data.ptr);
			break;
		case SUCCESS:
			/* shouldn't happen */
			free(data.ptr);
			/* FALL */
		case FAILED:
		default:
			writer->destroy(writer);
			return FAILED;
	}
	res = pt_tls_write(this->tls, PT_TLS_SASL_MECH_SELECTION,
					   this->identifier++, writer->get_buf(writer));
	writer->destroy(writer);
	if (!res)
	{
		return FAILED;
	}
	while (TRUE)
	{
		reader = pt_tls_read(this->tls, &vendor, &type, &identifier);
		if (!reader)
		{
			return FAILED;
		}
		if (vendor != 0)
		{
			reader->destroy(reader);
			return FAILED;
		}
		switch (type)
		{
			case PT_TLS_SASL_AUTH_DATA:
				switch (sasl->process(sasl, reader->peek(reader)))
				{
					case NEED_MORE:
						reader->destroy(reader);
						break;
					case SUCCESS:
						/* should not happen, as it would come in a RESULT */
					case FAILED:
					default:
						reader->destroy(reader);
						return FAILED;
				}
				break;
			case PT_TLS_SASL_RESULT:
				if (!reader->read_uint8(reader, &result))
				{
					reader->destroy(reader);
					return FAILED;
				}
				DBG1(DBG_TNC, "received SASL %N result",
					 pt_tls_sasl_result_names, result);

				switch (result)
				{
					case PT_TLS_SASL_RESULT_ABORT:
						reader->destroy(reader);
						return FAILED;
					case PT_TLS_SASL_RESULT_SUCCESS:
						switch (sasl->process(sasl, reader->peek(reader)))
						{
							case SUCCESS:
								reader->destroy(reader);
								return SUCCESS;
							case NEED_MORE:
								/* inacceptable, it won't get more. FALL */
							case FAILED:
							default:
								reader->destroy(reader);
								return FAILED;
						}
						break;
					case PT_TLS_SASL_RESULT_MECH_FAILURE:
					case PT_TLS_SASL_RESULT_FAILURE:
						/* non-fatal failure, try again */
						reader->destroy(reader);
						return NEED_MORE;
				}
				/* fall-through */
			default:
				reader->destroy(reader);
				return FAILED;
		}

		writer = bio_writer_create(32);
		switch (sasl->build(sasl, &data))
		{
			case INVALID_STATE:
				break;
			case SUCCESS:
				/* shoudln't happen, continue until we get a result */
			case NEED_MORE:
				writer->write_data(writer, data);
				free(data.ptr);
				break;
			case FAILED:
			default:
				writer->destroy(writer);
				return FAILED;
		}
		res = pt_tls_write(this->tls, PT_TLS_SASL_AUTH_DATA,
						   this->identifier++, writer->get_buf(writer));
		writer->destroy(writer);
		if (!res)
		{
			return FAILED;
		}
	}
}

/**
 * Read SASL mechanism list, select and run mechanism
 */
static status_t select_and_do_sasl(private_pt_tls_client_t *this)
{
	bio_reader_t *reader;
	sasl_mechanism_t *sasl = NULL;
	u_int32_t type, vendor, identifier;
	u_int8_t len;
	chunk_t chunk;
	char buf[21];
	status_t status = NEED_MORE;

	reader = pt_tls_read(this->tls, &vendor, &type, &identifier);
	if (!reader)
	{
		return FAILED;
	}
	if (vendor != 0 || type != PT_TLS_SASL_MECHS)
	{
		reader->destroy(reader);
		return FAILED;
	}
	if (!reader->remaining(reader))
	{	/* mechanism list empty, SASL completed */
		DBG1(DBG_TNC, "PT-TLS authentication complete");
		reader->destroy(reader);
		return SUCCESS;
	}
	while (reader->remaining(reader))
	{
		if (!reader->read_uint8(reader, &len) ||
			!reader->read_data(reader, len & 0x1F, &chunk))
		{
			reader->destroy(reader);
			return FAILED;
		}
		snprintf(buf, sizeof(buf), "%.*s", (int)chunk.len, chunk.ptr);
		sasl = sasl_mechanism_create(buf, this->client);
		if (sasl)
		{
			break;
		}
	}
	reader->destroy(reader);

	if (!sasl)
	{
		/* TODO: send PT-TLS error (5) */
		return FAILED;
	}
	while (status == NEED_MORE)
	{
		status = do_sasl(this, sasl);
	}
	sasl->destroy(sasl);
	if (status == SUCCESS)
	{	/* continue until we receive empty SASL mechanism list */
		return NEED_MORE;
	}
	return FAILED;
}

/**
 * Authenticate session using SASL
 */
static bool authenticate(private_pt_tls_client_t *this)
{
	while (TRUE)
	{
		switch (select_and_do_sasl(this))
		{
			case NEED_MORE:
				continue;
			case SUCCESS:
				return TRUE;
			case FAILED:
			default:
				return FALSE;
		}
	}
}

/**
 * Perform assessment
 */
static bool assess(private_pt_tls_client_t *this, tls_t *tnccs)
{
	while (TRUE)
	{
		size_t msglen;
		size_t buflen = PT_TLS_MAX_MESSAGE_LEN;
		char buf[buflen];
		bio_reader_t *reader;
		u_int32_t vendor, type, identifier;
		chunk_t data;

		switch (tnccs->build(tnccs, buf, &buflen, &msglen))
		{
			case SUCCESS:
				return tnccs->is_complete(tnccs);
			case ALREADY_DONE:
				data = chunk_create(buf, buflen);
				if (!pt_tls_write(this->tls, PT_TLS_PB_TNC_BATCH,
								  this->identifier++, data))
				{
					return FALSE;
				}
				break;
			case INVALID_STATE:
				break;
			case FAILED:
			default:
				return FALSE;
		}

		reader = pt_tls_read(this->tls, &vendor, &type, &identifier);
		if (!reader)
		{
			return FALSE;
		}
		if (vendor == 0)
		{
			if (type == PT_TLS_ERROR)
			{
				DBG1(DBG_TNC, "received PT-TLS error");
				reader->destroy(reader);
				return FALSE;
			}
			if (type != PT_TLS_PB_TNC_BATCH)
			{
				DBG1(DBG_TNC, "unexpected PT-TLS message: %d", type);
				reader->destroy(reader);
				return FALSE;
			}
			data = reader->peek(reader);
			switch (tnccs->process(tnccs, data.ptr, data.len))
			{
				case SUCCESS:
					reader->destroy(reader);
					return tnccs->is_complete(tnccs);
				case FAILED:
				default:
					reader->destroy(reader);
					return FALSE;
				case NEED_MORE:
					break;
			}
		}
		else
		{
			DBG1(DBG_TNC, "ignoring vendor specific PT-TLS message");
		}
		reader->destroy(reader);
	}
}

METHOD(pt_tls_client_t, run_assessment, status_t,
	private_pt_tls_client_t *this, tnccs_t *tnccs)
{
	if (!this->tls)
	{
		DBG1(DBG_TNC, "entering PT-TLS setup phase");
		if (!make_connection(this))
		{
			return FAILED;
		}
	}

	DBG1(DBG_TNC, "entering PT-TLS negotiation phase");
	if (!negotiate_version(this))
	{
		return FAILED;
	}

	DBG1(DBG_TNC, "doing SASL client authentication");
	if (!authenticate(this))
	{
		return FAILED;
	}

	DBG1(DBG_TNC, "entering PT-TLS data transport phase");
	if (!assess(this, (tls_t*)tnccs))
	{
		return FAILED;
	}
	return SUCCESS;
}


METHOD(pt_tls_client_t, destroy, void,
	private_pt_tls_client_t *this)
{
	if (this->tls)
	{
		int fd;

		fd = this->tls->get_fd(this->tls);
		this->tls->destroy(this->tls);
		close(fd);
	}
	this->address->destroy(this->address);
	this->server->destroy(this->server);
	this->client->destroy(this->client);
	free(this);
}

/**
 * See header
 */
pt_tls_client_t *pt_tls_client_create(host_t *address, identification_t *server,
									  identification_t *client)
{
	private_pt_tls_client_t *this;

	INIT(this,
		.public = {
			.run_assessment = _run_assessment,
			.destroy = _destroy,
		},
		.address = address,
		.server = server,
		.client = client,
	);

	return &this->public;
}
