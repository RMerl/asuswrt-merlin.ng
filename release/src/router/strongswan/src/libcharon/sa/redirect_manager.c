/*
 * Copyright (C) 2015 Tobias Brunner
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

#include "redirect_manager.h"

#include <collections/linked_list.h>
#include <threading/rwlock.h>
#include <bio/bio_reader.h>
#include <bio/bio_writer.h>

typedef struct private_redirect_manager_t private_redirect_manager_t;

/**
 * Private data
 */
struct private_redirect_manager_t {

	/**
	 * Public interface
	 */
	redirect_manager_t public;

	/**
	 * Registered providers
	 */
	linked_list_t *providers;

	/**
	 * Lock to access list of providers
	 */
	rwlock_t *lock;
};


/**
 * Gateway identify types
 *
 * The encoding is the same as that for corresponding ID payloads.
 */
typedef enum {
	/** IPv4 address of the VPN gateway */
	GATEWAY_ID_TYPE_IPV4 = 1,
	/** IPv6 address of the VPN gateway */
	GATEWAY_ID_TYPE_IPV6 = 2,
	/** FQDN of the VPN gateway */
	GATEWAY_ID_TYPE_FQDN = 3,
} gateway_id_type_t;

/**
 * Mapping of gateway identity types to identity types
 */
static id_type_t gateway_to_id_type(gateway_id_type_t type)
{
	switch (type)
	{
		case GATEWAY_ID_TYPE_IPV4:
			return ID_IPV4_ADDR;
		case GATEWAY_ID_TYPE_IPV6:
			return ID_IPV6_ADDR;
		case GATEWAY_ID_TYPE_FQDN:
			return ID_FQDN;
		default:
			return 0;
	}
}

/**
 * Mapping of identity types to gateway identity types
 */
static gateway_id_type_t id_type_to_gateway(id_type_t type)
{
	switch (type)
	{
		case ID_IPV4_ADDR:
			return GATEWAY_ID_TYPE_IPV4;
		case ID_IPV6_ADDR:
			return GATEWAY_ID_TYPE_IPV6;
		case ID_FQDN:
			return GATEWAY_ID_TYPE_FQDN;
		default:
			return 0;
	}
}

METHOD(redirect_manager_t, add_provider, void,
	private_redirect_manager_t *this, redirect_provider_t *provider)
{
	this->lock->write_lock(this->lock);
	this->providers->insert_last(this->providers, provider);
	this->lock->unlock(this->lock);
}

METHOD(redirect_manager_t, remove_provider, void,
	private_redirect_manager_t *this, redirect_provider_t *provider)
{
	this->lock->write_lock(this->lock);
	this->providers->remove(this->providers, provider, NULL);
	this->lock->unlock(this->lock);
}

/**
 * Determine whether a client should be redirected using the callback with the
 * given offset into the redirect_provider_t interface.
 */
static bool should_redirect(private_redirect_manager_t *this, ike_sa_t *ike_sa,
							identification_t **gateway, size_t offset)
{
	enumerator_t *enumerator;
	void *provider;
	bool redirect = FALSE;

	this->lock->read_lock(this->lock);
	enumerator = this->providers->create_enumerator(this->providers);
	while (enumerator->enumerate(enumerator, &provider))
	{
		bool (**method)(void*,ike_sa_t*,identification_t**) = provider + offset;
		if (*method && (*method)(provider, ike_sa, gateway))
		{
			if (*gateway && id_type_to_gateway((*gateway)->get_type(*gateway)))
			{
				redirect = TRUE;
				break;
			}
			else
			{
				DBG1(DBG_CFG, "redirect provider returned invalid gateway ID");
				DESTROY_IF(*gateway);
			}
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return redirect;
}

METHOD(redirect_manager_t, redirect_on_init, bool,
	private_redirect_manager_t *this, ike_sa_t *ike_sa,
	identification_t **gateway)
{
	return should_redirect(this, ike_sa, gateway,
						   offsetof(redirect_provider_t, redirect_on_init));
}

METHOD(redirect_manager_t, redirect_on_auth, bool,
	private_redirect_manager_t *this, ike_sa_t *ike_sa,
	identification_t **gateway)
{
	return should_redirect(this, ike_sa, gateway,
						   offsetof(redirect_provider_t, redirect_on_auth));
}

METHOD(redirect_manager_t, destroy, void,
	private_redirect_manager_t *this)
{
	this->providers->destroy(this->providers);
	this->lock->destroy(this->lock);
	free(this);
}

/*
 * Described in header
 */
redirect_manager_t *redirect_manager_create()
{
	private_redirect_manager_t *this;

	INIT(this,
		.public = {
			.add_provider = _add_provider,
			.remove_provider = _remove_provider,
			.redirect_on_init = _redirect_on_init,
			.redirect_on_auth = _redirect_on_auth,
			.destroy = _destroy,
		},
		.providers = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}

/*
 * Encoding of a REDIRECT or REDIRECTED_FROM notify
 *
                         1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    | Next Payload  |C|  RESERVED   |         Payload Length        |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |Protocol ID(=0)| SPI Size (=0) |      Notify Message Type      |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    | GW Ident Type |  GW Ident Len |                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               ~
    ~                   New Responder GW Identity                   ~
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    ~                        Nonce Data                             ~
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

/*
 * Described in header
 */
chunk_t redirect_data_create(identification_t *gw, chunk_t nonce)
{
	gateway_id_type_t type;
	bio_writer_t *writer;
	chunk_t data;

	type = id_type_to_gateway(gw->get_type(gw));
	if (!type)
	{
		return chunk_empty;
	}

	writer = bio_writer_create(0);
	writer->write_uint8(writer, type);
	writer->write_data8(writer, gw->get_encoding(gw));
	if (nonce.ptr)
	{
		writer->write_data(writer, nonce);
	}

	data = writer->extract_buf(writer);
	writer->destroy(writer);
	return data;
}

/*
 * Described in header
 */
identification_t *redirect_data_parse(chunk_t data, chunk_t *nonce)
{
	bio_reader_t *reader;
	id_type_t id_type;
	chunk_t gateway;
	uint8_t type;

	reader = bio_reader_create(data);
	if (!reader->read_uint8(reader, &type) ||
		!reader->read_data8(reader, &gateway))
	{
		DBG1(DBG_ENC, "invalid REDIRECT notify data");
		reader->destroy(reader);
		return NULL;
	}
	id_type = gateway_to_id_type(type);
	if (!id_type)
	{
		DBG1(DBG_ENC, "invalid gateway ID type (%d) in REDIRECT notify", type);
		reader->destroy(reader);
		return NULL;
	}
	if (nonce)
	{
		*nonce = chunk_clone(reader->peek(reader));
	}
	reader->destroy(reader);
	return identification_create_from_encoding(id_type, gateway);
}
