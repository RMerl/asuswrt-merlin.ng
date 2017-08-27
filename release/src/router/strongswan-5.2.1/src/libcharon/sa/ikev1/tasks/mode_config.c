/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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

#include "mode_config.h"

#include <daemon.h>
#include <hydra.h>
#include <encoding/payloads/cp_payload.h>

typedef struct private_mode_config_t private_mode_config_t;

/**
 * Private members of a mode_config_t task.
 */
struct private_mode_config_t {

	/**
	 * Public methods and task_t interface.
	 */
	mode_config_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;

	/**
	 * Use pull (CFG_REQUEST/RESPONSE) or push (CFG_SET/ACK)?
	 */
	bool pull;

	/**
	 * Received list of virtual IPs, host_t*
	 */
	linked_list_t *vips;

	/**
	 * Requested/received list of attributes, entry_t
	 */
	linked_list_t *attributes;

	/**
	 * Identifier to include in response
	 */
	u_int16_t identifier;
};

/**
 * Entry for a attribute and associated handler
 */
typedef struct {
	/** attribute type */
	configuration_attribute_type_t type;
	/** handler for this attribute */
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
	return configuration_attribute_create_chunk(PLV1_CONFIGURATION_ATTRIBUTE,
												type, chunk);
}

/**
 * Handle a received attribute as initiator
 */
static void handle_attribute(private_mode_config_t *this,
							 configuration_attribute_t *ca)
{
	attribute_handler_t *handler = NULL;
	enumerator_t *enumerator;
	entry_t *entry;

	/* find the handler which requested this attribute */
	enumerator = this->attributes->create_enumerator(this->attributes);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->type == ca->get_type(ca))
		{
			handler = entry->handler;
			this->attributes->remove_at(this->attributes, enumerator);
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
static void process_attribute(private_mode_config_t *this,
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
		default:
		{
			if (this->initiator == this->pull)
			{
				handle_attribute(this, ca);
			}
		}
	}
}

/**
 * Check if config allows push mode when acting as task responder
 */
static bool accept_push(private_mode_config_t *this)
{
	enumerator_t *enumerator;
	peer_cfg_t *config;
	bool vip;
	host_t *host;

	config = this->ike_sa->get_peer_cfg(this->ike_sa);
	enumerator = config->create_virtual_ip_enumerator(config);
	vip = enumerator->enumerate(enumerator, &host);
	enumerator->destroy(enumerator);

	return vip && !config->use_pull_mode(config);
}

/**
 * Scan for configuration payloads and attributes
 */
static void process_payloads(private_mode_config_t *this, message_t *message)
{
	enumerator_t *enumerator, *attributes;
	payload_t *payload;

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) == PLV1_CONFIGURATION)
		{
			cp_payload_t *cp = (cp_payload_t*)payload;
			configuration_attribute_t *ca;

			switch (cp->get_type(cp))
			{
				case CFG_SET:
					/* when acting as a responder, we detect the mode using
					 * the type of configuration payload. But we should double
					 * check the peer is allowed to use push mode on us. */
					if (!this->initiator && accept_push(this))
					{
						this->pull = FALSE;
					}
					/* FALL */
				case CFG_REQUEST:
					this->identifier = cp->get_identifier(cp);
					/* FALL */
				case CFG_REPLY:
					attributes = cp->create_attribute_enumerator(cp);
					while (attributes->enumerate(attributes, &ca))
					{
						DBG2(DBG_IKE, "processing %N attribute",
							 configuration_attribute_type_names, ca->get_type(ca));
						process_attribute(this, ca);
					}
					attributes->destroy(attributes);
					break;
				case CFG_ACK:
					break;
				default:
					DBG1(DBG_IKE, "ignoring %N config payload",
						 config_type_names, cp->get_type(cp));
					break;
			}
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Add an attribute to a configuration payload, and store it in task
 */
static void add_attribute(private_mode_config_t *this, cp_payload_t *cp,
						  configuration_attribute_type_t type, chunk_t data,
						  attribute_handler_t *handler)
{
	entry_t *entry;

	cp->add_attribute(cp,
			configuration_attribute_create_chunk(PLV1_CONFIGURATION_ATTRIBUTE,
												 type, data));
	INIT(entry,
		.type = type,
		.handler = handler,
	);
	this->attributes->insert_last(this->attributes, entry);
}

/**
 * Build a CFG_REQUEST as initiator
 */
static status_t build_request(private_mode_config_t *this, message_t *message)
{
	cp_payload_t *cp;
	enumerator_t *enumerator;
	attribute_handler_t *handler;
	peer_cfg_t *config;
	configuration_attribute_type_t type;
	chunk_t data;
	linked_list_t *vips;
	host_t *host;

	cp = cp_payload_create_type(PLV1_CONFIGURATION, CFG_REQUEST);

	vips = linked_list_create();

	/* reuse virtual IP if we already have one */
	enumerator = this->ike_sa->create_virtual_ip_enumerator(this->ike_sa, TRUE);
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
		add_attribute(this, cp, type, data, handler);
	}
	enumerator->destroy(enumerator);

	vips->destroy(vips);

	message->add_payload(message, (payload_t*)cp);

	return NEED_MORE;
}

/**
 * Build a CFG_SET as initiator
 */
static status_t build_set(private_mode_config_t *this, message_t *message)
{
	enumerator_t *enumerator;
	configuration_attribute_type_t type;
	chunk_t value;
	cp_payload_t *cp;
	peer_cfg_t *config;
	identification_t *id;
	linked_list_t *pools;
	host_t *any4, *any6, *found;
	char *name;

	cp = cp_payload_create_type(PLV1_CONFIGURATION, CFG_SET);

	id = this->ike_sa->get_other_eap_id(this->ike_sa);
	config = this->ike_sa->get_peer_cfg(this->ike_sa);
	any4 = host_create_any(AF_INET);
	any6 = host_create_any(AF_INET6);

	this->ike_sa->clear_virtual_ips(this->ike_sa, FALSE);

	/* in push mode, we ask each configured pool for an address */
	enumerator = config->create_pool_enumerator(config);
	while (enumerator->enumerate(enumerator, &name))
	{
		pools = linked_list_create_with_items(name, NULL);
		/* try IPv4, then IPv6 */
		found = hydra->attributes->acquire_address(hydra->attributes,
												   pools, id, any4);
		if (!found)
		{
			found = hydra->attributes->acquire_address(hydra->attributes,
													   pools, id, any6);
		}
		pools->destroy(pools);
		if (found)
		{
			DBG1(DBG_IKE, "assigning virtual IP %H to peer '%Y'", found, id);
			this->ike_sa->add_virtual_ip(this->ike_sa, FALSE, found);
			cp->add_attribute(cp, build_vip(found));
			this->vips->insert_last(this->vips, found);
		}
	}
	enumerator->destroy(enumerator);

	any4->destroy(any4);
	any6->destroy(any6);

	charon->bus->assign_vips(charon->bus, this->ike_sa, TRUE);

	/* query registered providers for additional attributes to include */
	pools = linked_list_create_from_enumerator(
									config->create_pool_enumerator(config));
	enumerator = hydra->attributes->create_responder_enumerator(
									hydra->attributes, pools, id, this->vips);
	while (enumerator->enumerate(enumerator, &type, &value))
	{
		add_attribute(this, cp, type, value, NULL);
	}
	enumerator->destroy(enumerator);
	pools->destroy(pools);

	message->add_payload(message, (payload_t*)cp);

	return SUCCESS;
}

METHOD(task_t, build_i, status_t,
	private_mode_config_t *this, message_t *message)
{
	if (this->pull)
	{
		return build_request(this, message);
	}
	return build_set(this, message);
}

/**
 * Store received virtual IPs to the IKE_SA, install them
 */
static void install_vips(private_mode_config_t *this)
{
	enumerator_t *enumerator;
	host_t *host;

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
}

METHOD(task_t, process_r, status_t,
	private_mode_config_t *this, message_t *message)
{
	process_payloads(this, message);

	if (!this->pull)
	{
		install_vips(this);
	}
	return NEED_MORE;
}

/**
 * Build CFG_REPLY message after receiving CFG_REQUEST
 */
static status_t build_reply(private_mode_config_t *this, message_t *message)
{
	enumerator_t *enumerator;
	configuration_attribute_type_t type;
	chunk_t value;
	cp_payload_t *cp;
	peer_cfg_t *config;
	identification_t *id;
	linked_list_t *vips, *pools;
	host_t *requested;

	cp = cp_payload_create_type(PLV1_CONFIGURATION, CFG_REPLY);

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

	charon->bus->assign_vips(charon->bus, this->ike_sa, TRUE);

	/* query registered providers for additional attributes to include */
	enumerator = hydra->attributes->create_responder_enumerator(
											hydra->attributes, pools, id, vips);
	while (enumerator->enumerate(enumerator, &type, &value))
	{
		cp->add_attribute(cp,
			configuration_attribute_create_chunk(PLV1_CONFIGURATION_ATTRIBUTE,
												 type, value));
	}
	enumerator->destroy(enumerator);
	vips->destroy_offset(vips, offsetof(host_t, destroy));
	pools->destroy(pools);

	cp->set_identifier(cp, this->identifier);
	message->add_payload(message, (payload_t*)cp);

	return SUCCESS;
}

/**
 * Build CFG_ACK for a received CFG_SET
 */
static status_t build_ack(private_mode_config_t *this, message_t *message)
{
	cp_payload_t *cp;
	enumerator_t *enumerator;
	host_t *host;
	configuration_attribute_type_t type;
	entry_t *entry;

	cp = cp_payload_create_type(PLV1_CONFIGURATION, CFG_ACK);

	/* return empty attributes for installed IPs */

	enumerator = this->vips->create_enumerator(this->vips);
	while (enumerator->enumerate(enumerator, &host))
	{
		type = INTERNAL_IP6_ADDRESS;
		if (host->get_family(host) == AF_INET6)
		{
			type = INTERNAL_IP6_ADDRESS;
		}
		else
		{
			type = INTERNAL_IP4_ADDRESS;
		}
		cp->add_attribute(cp, configuration_attribute_create_chunk(
								PLV1_CONFIGURATION_ATTRIBUTE, type, chunk_empty));
	}
	enumerator->destroy(enumerator);

	enumerator = this->attributes->create_enumerator(this->attributes);
	while (enumerator->enumerate(enumerator, &entry))
	{
		cp->add_attribute(cp,
			configuration_attribute_create_chunk(PLV1_CONFIGURATION_ATTRIBUTE,
												 entry->type, chunk_empty));
	}
	enumerator->destroy(enumerator);

	cp->set_identifier(cp, this->identifier);
	message->add_payload(message, (payload_t*)cp);

	return SUCCESS;
}

METHOD(task_t, build_r, status_t,
	private_mode_config_t *this, message_t *message)
{
	if (this->pull)
	{
		return build_reply(this, message);
	}
	return build_ack(this, message);
}

METHOD(task_t, process_i, status_t,
	private_mode_config_t *this, message_t *message)
{
	process_payloads(this, message);

	if (this->pull)
	{
		install_vips(this);
	}
	return SUCCESS;
}

METHOD(task_t, get_type, task_type_t,
	private_mode_config_t *this)
{
	return TASK_MODE_CONFIG;
}

METHOD(task_t, migrate, void,
	private_mode_config_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
	this->vips->destroy_offset(this->vips, offsetof(host_t, destroy));
	this->vips = linked_list_create();
	this->attributes->destroy_function(this->attributes, free);
	this->attributes = linked_list_create();
}

METHOD(task_t, destroy, void,
	private_mode_config_t *this)
{
	this->vips->destroy_offset(this->vips, offsetof(host_t, destroy));
	this->attributes->destroy_function(this->attributes, free);
	free(this);
}

/*
 * Described in header.
 */
mode_config_t *mode_config_create(ike_sa_t *ike_sa, bool initiator, bool pull)
{
	private_mode_config_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
		},
		.initiator = initiator,
		.pull = initiator ? pull : TRUE,
		.ike_sa = ike_sa,
		.attributes = linked_list_create(),
		.vips = linked_list_create(),
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
