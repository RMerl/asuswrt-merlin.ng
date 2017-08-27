/*
 * Copyright (C) 2007 Martin Willi
 * Copyright (C) 2006-2007 Fabian Hartmann, Noah Heusser
 * Hochschule fuer Technik Rapperswil
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

#include "ike_config.h"

#include <daemon.h>
#include <hydra.h>
#include <encoding/payloads/cp_payload.h>

typedef struct private_ike_config_t private_ike_config_t;

/**
 * Private members of a ike_config_t task.
 */
struct private_ike_config_t {

	/**
	 * Public methods and task_t interface.
	 */
	ike_config_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;

	/**
	 * Received list of virtual IPs, host_t*
	 */
	linked_list_t *vips;

	/**
	 * list of attributes requested and its handler, entry_t
	 */
	linked_list_t *requested;
};

/**
 * Entry for a requested attribute and the requesting handler
 */
typedef struct {
	/** attribute requested */
	configuration_attribute_type_t type;
	/** handler requesting this attribute */
	attribute_handler_t *handler;
} entry_t;

/**
 * build INTERNAL_IPV4/6_ADDRESS attribute from virtual ip
 */
static configuration_attribute_t *build_vip(host_t *vip)
{
	configuration_attribute_type_t type;
	chunk_t chunk, prefix;

	if (vip->get_family(vip) == AF_INET)
	{
		type = INTERNAL_IP4_ADDRESS;
		if (vip->is_anyaddr(vip))
		{
			chunk = chunk_empty;
		}
		else
		{
			chunk = vip->get_address(vip);
		}
	}
	else
	{
		type = INTERNAL_IP6_ADDRESS;
		if (vip->is_anyaddr(vip))
		{
			chunk = chunk_empty;
		}
		else
		{
			prefix = chunk_alloca(1);
			*prefix.ptr = 64;
			chunk = vip->get_address(vip);
			chunk = chunk_cata("cc", chunk, prefix);
		}
	}
	return configuration_attribute_create_chunk(PLV2_CONFIGURATION_ATTRIBUTE,
												type, chunk);
}

/**
 * Handle a received attribute as initiator
 */
static void handle_attribute(private_ike_config_t *this,
							 configuration_attribute_t *ca)
{
	attribute_handler_t *handler = NULL;
	enumerator_t *enumerator;
	entry_t *entry;

	/* find the handler which requested this attribute */
	enumerator = this->requested->create_enumerator(this->requested);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->type == ca->get_type(ca))
		{
			handler = entry->handler;
			this->requested->remove_at(this->requested, enumerator);
			free(entry);
			break;
		}
	}
	enumerator->destroy(enumerator);

	/* and pass it to the handle function */
	handler = hydra->attributes->handle(hydra->attributes,
							this->ike_sa->get_other_id(this->ike_sa), handler,
							ca->get_type(ca), ca->get_chunk(ca));
	this->ike_sa->add_configuration_attribute(this->ike_sa,
							handler, ca->get_type(ca), ca->get_chunk(ca));
}

/**
 * process a single configuration attribute
 */
static void process_attribute(private_ike_config_t *this,
							  configuration_attribute_t *ca)
{
	host_t *ip;
	chunk_t addr;
	int family = AF_INET6;

	switch (ca->get_type(ca))
	{
		case INTERNAL_IP4_ADDRESS:
			family = AF_INET;
			/* fall */
		case INTERNAL_IP6_ADDRESS:
		{
			addr = ca->get_chunk(ca);
			if (addr.len == 0)
			{
				ip = host_create_any(family);
			}
			else
			{
				/* skip prefix byte in IPv6 payload*/
				if (family == AF_INET6)
				{
					addr.len--;
				}
				ip = host_create_from_chunk(family, addr, 0);
			}
			if (ip)
			{
				this->vips->insert_last(this->vips, ip);
			}
			break;
		}
		case INTERNAL_IP4_SERVER:
		case INTERNAL_IP6_SERVER:
			/* assume it's a Windows client if we see proprietary attributes */
			this->ike_sa->enable_extension(this->ike_sa, EXT_MS_WINDOWS);
			/* fall */
		default:
		{
			if (this->initiator)
			{
				handle_attribute(this, ca);
			}
		}
	}
}

/**
 * Scan for configuration payloads and attributes
 */
static void process_payloads(private_ike_config_t *this, message_t *message)
{
	enumerator_t *enumerator, *attributes;
	payload_t *payload;

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) == PLV2_CONFIGURATION)
		{
			cp_payload_t *cp = (cp_payload_t*)payload;
			configuration_attribute_t *ca;

			switch (cp->get_type(cp))
			{
				case CFG_REQUEST:
				case CFG_REPLY:
				{
					attributes = cp->create_attribute_enumerator(cp);
					while (attributes->enumerate(attributes, &ca))
					{
						DBG2(DBG_IKE, "processing %N attribute",
							 configuration_attribute_type_names, ca->get_type(ca));
						process_attribute(this, ca);
					}
					attributes->destroy(attributes);
					break;
				}
				default:
					DBG1(DBG_IKE, "ignoring %N config payload",
						 config_type_names, cp->get_type(cp));
					break;
			}
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(task_t, build_i, status_t,
	private_ike_config_t *this, message_t *message)
{
	if (message->get_message_id(message) == 1)
	{	/* in first IKE_AUTH only */
		cp_payload_t *cp = NULL;
		enumerator_t *enumerator;
		attribute_handler_t *handler;
		peer_cfg_t *config;
		configuration_attribute_type_t type;
		chunk_t data;
		linked_list_t *vips;
		host_t *host;

		vips = linked_list_create();

		/* reuse virtual IP if we already have one */
		enumerator = this->ike_sa->create_virtual_ip_enumerator(this->ike_sa,
																TRUE);
		while (enumerator->enumerate(enumerator, &host))
		{
			vips->insert_last(vips, host);
		}
		enumerator->destroy(enumerator);

		if (vips->get_count(vips) == 0)
		{
			config = this->ike_sa->get_peer_cfg(this->ike_sa);
			enumerator = config->create_virtual_ip_enumerator(config);
			while (enumerator->enumerate(enumerator, &host))
			{
				vips->insert_last(vips, host);
			}
			enumerator->destroy(enumerator);
		}

		if (vips->get_count(vips))
		{
			cp = cp_payload_create_type(PLV2_CONFIGURATION, CFG_REQUEST);
			enumerator = vips->create_enumerator(vips);
			while (enumerator->enumerate(enumerator, &host))
			{
				cp->add_attribute(cp, build_vip(host));
			}
			enumerator->destroy(enumerator);
		}

		enumerator = hydra->attributes->create_initiator_enumerator(
								hydra->attributes,
								this->ike_sa->get_other_id(this->ike_sa), vips);
		while (enumerator->enumerate(enumerator, &handler, &type, &data))
		{
			configuration_attribute_t *ca;
			entry_t *entry;

			/* create configuration attribute */
			DBG2(DBG_IKE, "building %N attribute",
				 configuration_attribute_type_names, type);
			ca = configuration_attribute_create_chunk(PLV2_CONFIGURATION_ATTRIBUTE,
													  type, data);
			if (!cp)
			{
				cp = cp_payload_create_type(PLV2_CONFIGURATION, CFG_REQUEST);
			}
			cp->add_attribute(cp, ca);

			/* save handler along with requested type */
			entry = malloc_thing(entry_t);
			entry->type = type;
			entry->handler = handler;

			this->requested->insert_last(this->requested, entry);
		}
		enumerator->destroy(enumerator);

		vips->destroy(vips);

		if (cp)
		{
			message->add_payload(message, (payload_t*)cp);
		}
	}
	return NEED_MORE;
}

METHOD(task_t, process_r, status_t,
	private_ike_config_t *this, message_t *message)
{
	if (message->get_message_id(message) == 1)
	{	/* in first IKE_AUTH only */
		process_payloads(this, message);
	}
	return NEED_MORE;
}

METHOD(task_t, build_r, status_t,
	private_ike_config_t *this, message_t *message)
{
	if (this->ike_sa->get_state(this->ike_sa) == IKE_ESTABLISHED)
	{	/* in last IKE_AUTH exchange */
		enumerator_t *enumerator;
		configuration_attribute_type_t type;
		chunk_t value;
		cp_payload_t *cp = NULL;
		peer_cfg_t *config;
		identification_t *id;
		linked_list_t *vips, *pools;
		host_t *requested;

		id = this->ike_sa->get_other_eap_id(this->ike_sa);
		config = this->ike_sa->get_peer_cfg(this->ike_sa);
		vips = linked_list_create();
		pools = linked_list_create_from_enumerator(
									config->create_pool_enumerator(config));

		this->ike_sa->clear_virtual_ips(this->ike_sa, FALSE);

		enumerator = this->vips->create_enumerator(this->vips);
		while (enumerator->enumerate(enumerator, &requested))
		{
			host_t *found = NULL;

			/* query all pools until we get an address */
			DBG1(DBG_IKE, "peer requested virtual IP %H", requested);

			found = hydra->attributes->acquire_address(hydra->attributes,
													   pools, id, requested);
			if (found)
			{
				DBG1(DBG_IKE, "assigning virtual IP %H to peer '%Y'", found, id);
				this->ike_sa->add_virtual_ip(this->ike_sa, FALSE, found);
				if (!cp)
				{
					cp = cp_payload_create_type(PLV2_CONFIGURATION, CFG_REPLY);
				}
				cp->add_attribute(cp, build_vip(found));
				vips->insert_last(vips, found);
			}
			else
			{
				DBG1(DBG_IKE, "no virtual IP found for %H requested by '%Y'",
					 requested, id);
			}
		}
		enumerator->destroy(enumerator);

		if (this->vips->get_count(this->vips) && !vips->get_count(vips))
		{
			DBG1(DBG_IKE, "no virtual IP found, sending %N",
				 notify_type_names, INTERNAL_ADDRESS_FAILURE);
			charon->bus->alert(charon->bus, ALERT_VIP_FAILURE, this->vips);
			message->add_notify(message, FALSE, INTERNAL_ADDRESS_FAILURE,
								chunk_empty);
			vips->destroy_offset(vips, offsetof(host_t, destroy));
			pools->destroy(pools);
			return SUCCESS;
		}
		charon->bus->assign_vips(charon->bus, this->ike_sa, TRUE);

		if (pools->get_count(pools) && !this->vips->get_count(this->vips))
		{
			DBG1(DBG_IKE, "expected a virtual IP request, sending %N",
				 notify_type_names, FAILED_CP_REQUIRED);
			charon->bus->alert(charon->bus, ALERT_VIP_FAILURE, this->vips);
			message->add_notify(message, FALSE, FAILED_CP_REQUIRED, chunk_empty);
			vips->destroy_offset(vips, offsetof(host_t, destroy));
			pools->destroy(pools);
			return SUCCESS;
		}

		/* query registered providers for additional attributes to include */
		enumerator = hydra->attributes->create_responder_enumerator(
											hydra->attributes, pools, id, vips);
		while (enumerator->enumerate(enumerator, &type, &value))
		{
			if (!cp)
			{
				cp = cp_payload_create_type(PLV2_CONFIGURATION, CFG_REPLY);
			}
			DBG2(DBG_IKE, "building %N attribute",
				 configuration_attribute_type_names, type);
			cp->add_attribute(cp,
				configuration_attribute_create_chunk(PLV2_CONFIGURATION_ATTRIBUTE,
													 type, value));
		}
		enumerator->destroy(enumerator);
		vips->destroy_offset(vips, offsetof(host_t, destroy));
		pools->destroy(pools);

		if (cp)
		{
			message->add_payload(message, (payload_t*)cp);
		}
		return SUCCESS;
	}
	return NEED_MORE;
}

METHOD(task_t, process_i, status_t,
	private_ike_config_t *this, message_t *message)
{
	if (this->ike_sa->get_state(this->ike_sa) == IKE_ESTABLISHED)
	{	/* in last IKE_AUTH exchange */
		enumerator_t *enumerator;
		host_t *host;

		process_payloads(this, message);

		this->ike_sa->clear_virtual_ips(this->ike_sa, TRUE);

		enumerator = this->vips->create_enumerator(this->vips);
		while (enumerator->enumerate(enumerator, &host))
		{
			if (!host->is_anyaddr(host))
			{
				this->ike_sa->add_virtual_ip(this->ike_sa, TRUE, host);
			}
		}
		enumerator->destroy(enumerator);

		charon->bus->handle_vips(charon->bus, this->ike_sa, TRUE);
		return SUCCESS;
	}
	return NEED_MORE;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_config_t *this)
{
	return TASK_IKE_CONFIG;
}

METHOD(task_t, migrate, void,
	private_ike_config_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
	this->vips->destroy_offset(this->vips, offsetof(host_t, destroy));
	this->vips = linked_list_create();
	this->requested->destroy_function(this->requested, free);
	this->requested = linked_list_create();
}

METHOD(task_t, destroy, void,
	private_ike_config_t *this)
{
	this->vips->destroy_offset(this->vips, offsetof(host_t, destroy));
	this->requested->destroy_function(this->requested, free);
	free(this);
}

/*
 * Described in header.
 */
ike_config_t *ike_config_create(ike_sa_t *ike_sa, bool initiator)
{
	private_ike_config_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
		},
		.initiator = initiator,
		.ike_sa = ike_sa,
		.vips = linked_list_create(),
		.requested = linked_list_create(),
	);

	if (initiator)
	{
		this->public.task.build = _build_i;
		this->public.task.process = _process_i;
	}
	else
	{
		this->public.task.build = _build_r;
		this->public.task.process = _process_r;
	}

	return &this->public;
}
