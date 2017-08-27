/*
 * Copyright (C) 2012 Tobias Brunner
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

#include "eap_dynamic.h"

#include <daemon.h>
#include <library.h>

typedef struct private_eap_dynamic_t private_eap_dynamic_t;

/**
 * Private data of an eap_dynamic_t object.
 */
struct private_eap_dynamic_t {

	/**
	 * Public authenticator_t interface.
	 */
	eap_dynamic_t public;

	/**
	 * ID of the server
	 */
	identification_t *server;

	/**
	 * ID of the peer
	 */
	identification_t *peer;

	/**
	 * Our supported EAP types (as eap_vendor_type_t*)
	 */
	linked_list_t *types;

	/**
	 * EAP types supported by peer, if any
	 */
	linked_list_t *other_types;

	/**
	 * Prefer types sent by peer
	 */
	bool prefer_peer;

	/**
	 * The proxied EAP method
	 */
	eap_method_t *method;
};

/**
 * Compare two eap_vendor_type_t objects
 */
static bool entry_matches(eap_vendor_type_t *item, eap_vendor_type_t *other)
{
	return item->type == other->type && item->vendor == other->vendor;
}

/**
 * Load the given EAP method
 */
static eap_method_t *load_method(private_eap_dynamic_t *this,
								 eap_type_t type, u_int32_t vendor)
{
	eap_method_t *method;

	method = charon->eap->create_instance(charon->eap, type, vendor, EAP_SERVER,
										  this->server, this->peer);
	if (!method)
	{
		if (vendor)
		{
			DBG1(DBG_IKE, "loading vendor specific EAP method %d-%d failed",
				 type, vendor);
		}
		else
		{
			DBG1(DBG_IKE, "loading %N method failed", eap_type_names, type);
		}
	}
	return method;
}

/**
 * Select the first method we can instantiate and is supported by both peers.
 */
static void select_method(private_eap_dynamic_t *this)
{
	eap_vendor_type_t *entry;
	linked_list_t *outer = this->types, *inner = this->other_types;
	char *who = "peer";

	if (this->other_types && this->prefer_peer)
	{
		outer = this->other_types;
		inner = this->types;
		who = "us";
	}

	while (outer->remove_first(outer, (void*)&entry) == SUCCESS)
	{
		if (inner)
		{
			if (inner->find_first(inner, (void*)entry_matches,
								  NULL, entry) != SUCCESS)
			{
				if (entry->vendor)
				{
					DBG2(DBG_IKE, "proposed vendor specific EAP method %d-%d "
						 "not supported by %s, skipped", entry->type,
						  entry->vendor, who);
				}
				else
				{
					DBG2(DBG_IKE, "proposed %N method not supported by %s, "
						 "skipped", eap_type_names, entry->type, who);
				}
				free(entry);
				continue;
			}
		}
		this->method = load_method(this, entry->type, entry->vendor);
		if (this->method)
		{
			if (entry->vendor)
			{
				DBG1(DBG_IKE, "vendor specific EAP method %d-%d selected",
					 entry->type, entry->vendor);
			}
			else
			{
				DBG1(DBG_IKE, "%N method selected", eap_type_names,
					 entry->type);
			}
			free(entry);
			break;
		}
		free(entry);
	}
}

METHOD(eap_method_t, initiate, status_t,
	private_eap_dynamic_t *this, eap_payload_t **out)
{
	if (!this->method)
	{
		select_method(this);
		if (!this->method)
		{
			DBG1(DBG_IKE, "no supported EAP method found");
			return FAILED;
		}
	}
	return this->method->initiate(this->method, out);
}

METHOD(eap_method_t, process, status_t,
	private_eap_dynamic_t *this, eap_payload_t *in, eap_payload_t **out)
{
	eap_type_t received_type, type;
	u_int32_t received_vendor, vendor;

	received_type = in->get_type(in, &received_vendor);
	if (received_vendor == 0 && received_type == EAP_NAK)
	{
		enumerator_t *enumerator;

		DBG1(DBG_IKE, "received %N, selecting a different EAP method",
			 eap_type_names, EAP_NAK);

		if (this->other_types)
		{	/* we already received a Nak or a proper response before */
			DBG1(DBG_IKE, "%N is not supported in this state", eap_type_names,
				 EAP_NAK);
			return FAILED;
		}

		this->other_types = linked_list_create();
		enumerator = in->get_types(in);
		while (enumerator->enumerate(enumerator, &type, &vendor))
		{
			eap_vendor_type_t *entry;

			if (!type)
			{
				DBG1(DBG_IKE, "peer does not support any other EAP methods");
				enumerator->destroy(enumerator);
				return FAILED;
			}
			INIT(entry,
				.type = type,
				.vendor = vendor,
			);
			this->other_types->insert_last(this->other_types, entry);
		}
		enumerator->destroy(enumerator);

		/* restart with a different method */
		this->method->destroy(this->method);
		this->method = NULL;
		return initiate(this, out);
	}
	if (!this->other_types)
	{	/* so we don't handle EAP-Naks later */
		this->other_types = linked_list_create();
	}
	if (this->method)
	{
		return this->method->process(this->method, in, out);
	}
	return FAILED;
}

METHOD(eap_method_t, get_type, eap_type_t,
	private_eap_dynamic_t *this, u_int32_t *vendor)
{
	if (this->method)
	{
		return this->method->get_type(this->method, vendor);
	}
	*vendor = 0;
	return EAP_DYNAMIC;
}

METHOD(eap_method_t, get_msk, status_t,
	private_eap_dynamic_t *this, chunk_t *msk)
{
	if (this->method)
	{
		return this->method->get_msk(this->method, msk);
	}
	return FAILED;
}

METHOD(eap_method_t, get_identifier, u_int8_t,
	private_eap_dynamic_t *this)
{
	if (this->method)
	{
		return this->method->get_identifier(this->method);
	}
	return 0;
}

METHOD(eap_method_t, set_identifier, void,
	private_eap_dynamic_t *this, u_int8_t identifier)
{
	if (this->method)
	{
		this->method->set_identifier(this->method, identifier);
	}
}

METHOD(eap_method_t, is_mutual, bool,
	private_eap_dynamic_t *this)
{
	if (this->method)
	{
		return this->method->is_mutual(this->method);
	}
	return FALSE;
}

METHOD(eap_method_t, destroy, void,
	private_eap_dynamic_t *this)
{
	DESTROY_IF(this->method);
	this->types->destroy_function(this->types, (void*)free);
	DESTROY_FUNCTION_IF(this->other_types, (void*)free);
	this->server->destroy(this->server);
	this->peer->destroy(this->peer);
	free(this);
}

/**
 * Parse preferred EAP types
 */
static void handle_preferred_eap_types(private_eap_dynamic_t *this,
									   char *methods)
{
	enumerator_t *enumerator;
	eap_vendor_type_t *type, *entry;
	linked_list_t *preferred;
	char *method;

	/* parse preferred EAP methods, format: type[-vendor], ... */
	preferred = linked_list_create();
	enumerator = enumerator_create_token(methods, ",", " ");
	while (enumerator->enumerate(enumerator, &method))
	{
		type = eap_vendor_type_from_string(method);
		if (type)
		{
			preferred->insert_last(preferred, type);
		}
	}
	enumerator->destroy(enumerator);

	enumerator = this->types->create_enumerator(this->types);
	while (preferred->remove_last(preferred, (void**)&type) == SUCCESS)
	{	/* move (supported) types to the front, maintain the preferred order */
		this->types->reset_enumerator(this->types, enumerator);
		while (enumerator->enumerate(enumerator, &entry))
		{
			if (entry_matches(entry, type))
			{
				this->types->remove_at(this->types, enumerator);
				this->types->insert_first(this->types, entry);
				break;
			}
		}
		free(type);
	}
	enumerator->destroy(enumerator);
	preferred->destroy(preferred);
}

/**
 * Get all supported EAP methods
 */
static void get_supported_eap_types(private_eap_dynamic_t *this)
{
	enumerator_t *enumerator;
	eap_type_t type;
	u_int32_t vendor;

	enumerator = charon->eap->create_enumerator(charon->eap, EAP_SERVER);
	while (enumerator->enumerate(enumerator, &type, &vendor))
	{
		eap_vendor_type_t *entry;

		INIT(entry,
			.type = type,
			.vendor = vendor,
		);
		this->types->insert_last(this->types, entry);
	}
	enumerator->destroy(enumerator);
}

/*
 * Defined in header
 */
eap_dynamic_t *eap_dynamic_create(identification_t *server,
								  identification_t *peer)
{
	private_eap_dynamic_t *this;
	char *preferred;

	INIT(this,
		.public = {
			.interface = {
				.initiate = _initiate,
				.process = _process,
				.get_type = _get_type,
				.is_mutual = _is_mutual,
				.get_msk = _get_msk,
				.get_identifier = _get_identifier,
				.set_identifier = _set_identifier,
				.destroy = _destroy,
			},
		},
		.peer = peer->clone(peer),
		.server = server->clone(server),
		.types = linked_list_create(),
		.prefer_peer = lib->settings->get_bool(lib->settings,
						"%s.plugins.eap-dynamic.prefer_peer", FALSE, lib->ns),
	);

	/* get all supported EAP methods */
	get_supported_eap_types(this);
	/* move preferred methods to the front */
	preferred = lib->settings->get_str(lib->settings,
						"%s.plugins.eap-dynamic.preferred", NULL, lib->ns);
	if (preferred)
	{
		handle_preferred_eap_types(this, preferred);
	}
	return &this->public;
}
