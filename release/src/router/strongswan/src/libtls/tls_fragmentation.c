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

#include "tls_fragmentation.h"

#include <bio/bio_reader.h>
#include <utils/debug.h>

/**
 * Maximum size of a TLS handshake message we accept
 */
#define TLS_MAX_HANDSHAKE_LEN	65536

typedef struct private_tls_fragmentation_t private_tls_fragmentation_t;

/**
 * Alert state
 */
typedef enum {
	/* no alert received/sent */
	ALERT_NONE,
	/* currently sending an alert */
	ALERT_SENDING,
	/* alert sent and out */
	ALERT_SENT,
} alert_state_t;

/**
 * Private data of an tls_fragmentation_t object.
 */
struct private_tls_fragmentation_t {

	/**
	 * Public tls_fragmentation_t interface.
	 */
	tls_fragmentation_t public;

	/**
	 * Upper layer handshake protocol
	 */
	tls_handshake_t *handshake;

	/**
	 * TLS alert handler
	 */
	tls_alert_t *alert;

	/**
	 * State of alert handling
	 */
	alert_state_t state;

	/**
	 * Did the application layer complete successfully?
	 */
	bool application_finished;

	/**
	 * Handshake input buffer
	 */
	chunk_t input;

	/**
	 * Position in input buffer
	 */
	size_t inpos;

	/**
	 * Currently processed handshake message type
	 */
	tls_handshake_type_t type;

	/**
	 * Handshake output buffer
	 */
	chunk_t output;

	/**
	 * Type of data in output buffer
	 */
	tls_content_type_t output_type;

	/**
	 * Upper layer application data protocol
	 */
	tls_application_t *application;

	/**
	 * Type of context this TLS instance runs in
	 */
	tls_purpose_t purpose;
};

/**
 * Check if we should send a close notify once the application finishes
 */
static bool send_close_notify(private_tls_fragmentation_t *this)
{
	switch (this->purpose)
	{
		case TLS_PURPOSE_EAP_TLS:
		case TLS_PURPOSE_EAP_TTLS:
		case TLS_PURPOSE_EAP_PEAP:
			/* not for TLS-in-EAP, as we indicate completion with EAP-SUCCCESS.
			 * Windows does not like close notifies, and hangs/disconnects. */
			return FALSE;
		default:
			return TRUE;
	}
}

/**
 * Process a TLS alert
 */
static status_t process_alert(private_tls_fragmentation_t *this,
							  bio_reader_t *reader)
{
	uint8_t level, description;

	if (!reader->read_uint8(reader, &level) ||
		!reader->read_uint8(reader, &description))
	{
		this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
		return NEED_MORE;
	}
	return this->alert->process(this->alert, level, description);
}

/**
 * Process TLS handshake protocol data
 */
static status_t process_handshake(private_tls_fragmentation_t *this,
								  bio_reader_t *reader)
{
	while (reader->remaining(reader))
	{
		bio_reader_t *msg;
		uint8_t type;
		uint32_t len;
		status_t status;
		chunk_t data;

		if (reader->remaining(reader) > TLS_MAX_FRAGMENT_LEN)
		{
			DBG1(DBG_TLS, "TLS fragment has invalid length");
			this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
			return NEED_MORE;
		}

		if (this->input.len == 0)
		{	/* new handshake message */
			if (!reader->read_uint8(reader, &type) ||
				!reader->read_uint24(reader, &len))
			{
				DBG1(DBG_TLS, "TLS handshake header invalid");
				this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
				return NEED_MORE;
			}
			this->type = type;
			if (len > TLS_MAX_HANDSHAKE_LEN)
			{
				DBG1(DBG_TLS, "TLS handshake exceeds maximum length");
				this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
				return NEED_MORE;
			}
			chunk_free(&this->input);
			this->inpos = 0;
			if (len)
			{
				this->input = chunk_alloc(len);
			}
		}

		len = min(this->input.len - this->inpos, reader->remaining(reader));
		if (!reader->read_data(reader, len, &data))
		{
			DBG1(DBG_TLS, "TLS fragment has invalid length");
			this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
			return NEED_MORE;
		}
		memcpy(this->input.ptr + this->inpos, data.ptr, len);
		this->inpos += len;

		if (this->input.len == this->inpos)
		{	/* message completely defragmented, process */
			msg = bio_reader_create(this->input);
			DBG2(DBG_TLS, "received TLS %N handshake (%u bytes)",
				 tls_handshake_type_names, this->type, this->input.len);
			status = this->handshake->process(this->handshake, this->type, msg);
			msg->destroy(msg);
			chunk_free(&this->input);
			if (status != NEED_MORE)
			{
				return status;
			}
		}
		if (this->alert->fatal(this->alert))
		{
			break;
		}
	}
	return NEED_MORE;
}

/**
 * Process TLS application data
 */
static status_t process_application(private_tls_fragmentation_t *this,
									bio_reader_t *reader)
{
	if (!this->handshake->finished(this->handshake))
	{
		DBG1(DBG_TLS, "received TLS application data, "
			 "but handshake not finished");
		return FAILED;
	}
	while (reader->remaining(reader))
	{
		status_t status;
		chunk_t data;

		if (reader->remaining(reader) > TLS_MAX_FRAGMENT_LEN)
		{
			DBG1(DBG_TLS, "TLS fragment has invalid length");
			this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
			return NEED_MORE;
		}
		data = reader->peek(reader);
		DBG3(DBG_TLS, "%B", &data);
		status = this->application->process(this->application, reader);
		switch (status)
		{
			case NEED_MORE:
				continue;
			case SUCCESS:
				this->application_finished = TRUE;
				if (!send_close_notify(this))
				{
					return SUCCESS;
				}
				/* FALL */
			case FAILED:
			default:
				this->alert->add(this->alert, TLS_FATAL, TLS_CLOSE_NOTIFY);
				return NEED_MORE;
		}
	}
	return NEED_MORE;
}

METHOD(tls_fragmentation_t, process, status_t,
	private_tls_fragmentation_t *this, tls_content_type_t type, chunk_t data)
{
	bio_reader_t *reader;
	status_t status;

	switch (this->state)
	{
		case ALERT_SENDING:
		case ALERT_SENT:
			/* don't accept more input, fatal error occurred */
			return NEED_MORE;
		case ALERT_NONE:
			break;
	}
	reader = bio_reader_create(data);
	switch (type)
	{
		case TLS_CHANGE_CIPHER_SPEC:
			if (this->handshake->cipherspec_changed(this->handshake, TRUE))
			{
				this->handshake->change_cipherspec(this->handshake, TRUE);
				status = NEED_MORE;
				break;
			}
			status = FAILED;
			break;
		case TLS_ALERT:
			status = process_alert(this, reader);
			break;
		case TLS_HANDSHAKE:
			status = process_handshake(this, reader);
			break;
		case TLS_APPLICATION_DATA:
			status = process_application(this, reader);
			break;
		default:
			DBG1(DBG_TLS, "received unknown TLS content type %d, ignored", type);
			status = NEED_MORE;
			break;
	}
	reader->destroy(reader);
	return status;
}

/**
 * Check if alerts are pending
 */
static bool check_alerts(private_tls_fragmentation_t *this, chunk_t *data)
{
	tls_alert_level_t level;
	tls_alert_desc_t desc;
	bio_writer_t *writer;

	if (this->alert->get(this->alert, &level, &desc))
	{
		writer = bio_writer_create(2);

		writer->write_uint8(writer, level);
		writer->write_uint8(writer, desc);

		*data = chunk_clone(writer->get_buf(writer));
		writer->destroy(writer);
		return TRUE;
	}
	return FALSE;
}

/**
 * Build handshake message
 */
static status_t build_handshake(private_tls_fragmentation_t *this)
{
	bio_writer_t *hs, *msg;
	tls_handshake_type_t type;
	status_t status;

	msg = bio_writer_create(64);
	while (TRUE)
	{
		hs = bio_writer_create(64);
		status = this->handshake->build(this->handshake, &type, hs);
		switch (status)
		{
			case NEED_MORE:
				if (this->alert->fatal(this->alert))
				{
					break;
				}
				msg->write_uint8(msg, type);
				msg->write_data24(msg, hs->get_buf(hs));
				DBG2(DBG_TLS, "sending TLS %N handshake (%u bytes)",
					 tls_handshake_type_names, type, hs->get_buf(hs).len);
				if (!this->handshake->cipherspec_changed(this->handshake, FALSE))
				{
					hs->destroy(hs);
					continue;
				}
				/* FALL */
			case INVALID_STATE:
				this->output_type = TLS_HANDSHAKE;
				this->output = chunk_clone(msg->get_buf(msg));
				break;
			default:
				break;
		}
		hs->destroy(hs);
		break;
	}
	msg->destroy(msg);
	return status;
}

/**
 * Build TLS application data
 */
static status_t build_application(private_tls_fragmentation_t *this)
{
	bio_writer_t *msg;
	status_t status;

	msg = bio_writer_create(64);
	while (TRUE)
	{
		status = this->application->build(this->application, msg);
		switch (status)
		{
			case NEED_MORE:
				continue;
			case INVALID_STATE:
				this->output_type = TLS_APPLICATION_DATA;
				this->output = chunk_clone(msg->get_buf(msg));
				break;
			case SUCCESS:
				this->application_finished = TRUE;
				if (!send_close_notify(this))
				{
					break;
				}
				/* FALL */
			case FAILED:
			default:
				this->alert->add(this->alert, TLS_FATAL, TLS_CLOSE_NOTIFY);
				break;
		}
		break;
	}
	msg->destroy(msg);
	return status;
}

METHOD(tls_fragmentation_t, build, status_t,
	private_tls_fragmentation_t *this, tls_content_type_t *type, chunk_t *data)
{
	status_t status = INVALID_STATE;

	switch (this->state)
	{
		case ALERT_SENDING:
			this->state = ALERT_SENT;
			return INVALID_STATE;
		case ALERT_SENT:
			if (this->application_finished)
			{
				return SUCCESS;
			}
			return FAILED;
		case ALERT_NONE:
			break;
	}
	if (check_alerts(this, data))
	{
		this->state = ALERT_SENDING;
		*type = TLS_ALERT;
		return NEED_MORE;
	}
	if (!this->output.len)
	{
		if (this->handshake->cipherspec_changed(this->handshake, FALSE))
		{
			this->handshake->change_cipherspec(this->handshake, FALSE);
			*type = TLS_CHANGE_CIPHER_SPEC;
			*data = chunk_clone(chunk_from_chars(0x01));
			return NEED_MORE;
		}
		if (!this->handshake->finished(this->handshake))
		{
			status = build_handshake(this);
		}
		else if (this->application)
		{
			status = build_application(this);
		}
		if (check_alerts(this, data))
		{
			this->state = ALERT_SENDING;
			*type = TLS_ALERT;
			return NEED_MORE;
		}
	}
	if (this->output.len)
	{
		*type = this->output_type;
		if (this->output.len <= TLS_MAX_FRAGMENT_LEN)
		{
			*data = this->output;
			this->output = chunk_empty;
			return NEED_MORE;
		}
		*data = chunk_create(this->output.ptr, TLS_MAX_FRAGMENT_LEN);
		this->output = chunk_clone(chunk_skip(this->output, TLS_MAX_FRAGMENT_LEN));
		return NEED_MORE;
	}
	return status;
}

METHOD(tls_fragmentation_t, application_finished, bool,
	private_tls_fragmentation_t *this)
{
	return this->application_finished;
}

METHOD(tls_fragmentation_t, destroy, void,
	private_tls_fragmentation_t *this)
{
	free(this->input.ptr);
	free(this->output.ptr);
	free(this);
}

/**
 * See header
 */
tls_fragmentation_t *tls_fragmentation_create(tls_handshake_t *handshake,
							tls_alert_t *alert, tls_application_t *application,
							tls_purpose_t purpose)
{
	private_tls_fragmentation_t *this;

	INIT(this,
		.public = {
			.process = _process,
			.build = _build,
			.application_finished = _application_finished,
			.destroy = _destroy,
		},
		.handshake = handshake,
		.alert = alert,
		.state = ALERT_NONE,
		.application = application,
		.purpose = purpose,
	);

	return &this->public;
}
