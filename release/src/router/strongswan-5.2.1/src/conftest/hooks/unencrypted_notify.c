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

#include "hook.h"

typedef struct private_unencrypted_notify_t private_unencrypted_notify_t;

/**
 * Private data of an unencrypted_notify_t object.
 */
struct private_unencrypted_notify_t {

	/**
	 * Implements the hook_t interface.
	 */
	hook_t hook;

	/**
	 * ID of message send.
	 */
	int id;

	/**
	 * Notify type
	 */
	char *type;

	/**
	 * Notify data
	 */
	char *data;

	/**
	 * SPI of notify
	 */
	int spi;

	/**
	 * TRUE for a ESP protocol notify, FALSE for IKE
	 */
	bool esp;
};

METHOD(listener_t, ike_updown, bool,
	private_unencrypted_notify_t *this, ike_sa_t *ike_sa, bool up)
{
	if (up)
	{
		message_t *message;
		host_t *host;
		notify_type_t type;
		notify_payload_t *notify;
		chunk_t data = chunk_empty;
		packet_t *packet;

		type = atoi(this->type);
		if (!type)
		{
			if (!enum_from_name(notify_type_names, this->type, &type))
			{
				DBG1(DBG_CFG, "unknown notify: '%s', skipped", this->type);
				return TRUE;
			}
		}
		if (strncaseeq(this->data, "0x", 2))
		{
			data = chunk_skip(chunk_create(this->data, strlen(this->data)), 2);
			data = chunk_from_hex(data, NULL);
		}
		else if (strlen(this->data))
		{
			data = chunk_clone(chunk_create(this->data, strlen(this->data)));
		}
		notify = notify_payload_create_from_protocol_and_type(PLV2_NOTIFY,
									this->esp ? PROTO_ESP : PROTO_IKE, type);
		notify->set_spi(notify, this->spi);
		if (data.len)
		{
			notify->set_notification_data(notify, data);
			free(data.ptr);
		}

		DBG1(DBG_CFG, "injecting unencrypted INFORMATIONAL message");

		message = message_create(IKEV2_MAJOR_VERSION, IKEV2_MINOR_VERSION);
		message->set_message_id(message, this->id);
		message->set_ike_sa_id(message, ike_sa->get_id(ike_sa));
		message->set_exchange_type(message, INFORMATIONAL);
		message->set_request(message, TRUE);
		host = ike_sa->get_my_host(ike_sa);
		message->set_source(message, host->clone(host));
		host = ike_sa->get_other_host(ike_sa);
		message->set_destination(message, host->clone(host));
		message->add_payload(message, &notify->payload_interface);
		if (message->generate(message, NULL, &packet) != SUCCESS)
		{
			DBG1(DBG_CFG, "generating message failed");
			message->destroy(message);
			return TRUE;
		}
		message->destroy(message);
		charon->sender->send(charon->sender, packet);
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_unencrypted_notify_t *this)
{
	free(this);
}

/**
 * Create the IKE_AUTH fill hook
 */
hook_t *unencrypted_notify_hook_create(char *name)
{
	private_unencrypted_notify_t *this;

	INIT(this,
		.hook = {
			.listener = {
				.ike_updown = _ike_updown,
			},
			.destroy = _destroy,
		},
		.id = conftest->test->get_int(conftest->test,
										"hooks.%s.id", 2, name),
		.type = conftest->test->get_str(conftest->test,
										"hooks.%s.type", "", name),
		.data = conftest->test->get_str(conftest->test,
										"hooks.%s.data", "", name),
		.spi = conftest->test->get_int(conftest->test,
										"hooks.%s.spi", 0, name),
		.esp = conftest->test->get_bool(conftest->test,
										"hooks.%s.esp", FALSE, name),
	);

	return &this->hook;
}
