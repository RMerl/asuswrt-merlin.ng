/*
 * Copyright (C) 2008-2015 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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

#include "stroke_ca.h"
#include "stroke_cred.h"

#include <threading/rwlock.h>
#include <collections/linked_list.h>
#include <crypto/hashers/hasher.h>

#include <daemon.h>

typedef struct private_stroke_ca_t private_stroke_ca_t;
typedef struct ca_section_t ca_section_t;
typedef struct ca_cert_t ca_cert_t;

/**
 * Provided by stroke_cred.c
 */
certificate_t *stroke_load_ca_cert(char *filename);

/**
 * private data of stroke_ca
 */
struct private_stroke_ca_t {

	/**
	 * public functions
	 */
	stroke_ca_t public;

	/**
	 * read-write lock to lists
	 */
	rwlock_t *lock;

	/**
	 * list of CA sections and their certificates (ca_section_t)
	 */
	linked_list_t *sections;

	/**
	 * list of all loaded CA certificates (ca_cert_t)
	 */
	linked_list_t *certs;
};


/**
 * loaded ipsec.conf CA sections
 */
struct ca_section_t {

	/**
	 * name of the CA section
	 */
	char *name;

	/**
	 * path/name of the certificate
	 */
	char *path;

	/**
	 * reference to cert
	 */
	certificate_t *cert;

	/**
	 * CRL URIs
	 */
	linked_list_t *crl;

	/**
	 * OCSP URIs
	 */
	linked_list_t *ocsp;

	/**
	 * Hashes of certificates issued by this CA
	 */
	linked_list_t *hashes;

	/**
	 * Base URI used for certificates from this CA
	 */
	char *certuribase;
};

/**
 * loaded CA certificate
 */
struct ca_cert_t {

	/**
	 * reference to cert
	 */
	certificate_t *cert;

	/**
	 * The number of CA sections referring to this certificate
	 */
	u_int count;

	/**
	 * TRUE if this certificate was automatically loaded
	 */
	bool automatic;
};

/**
 * create a new CA section
 */
static ca_section_t *ca_section_create(char *name, char *path)
{
	ca_section_t *ca = malloc_thing(ca_section_t);

	ca->name = strdup(name);
	ca->path = strdup(path);
	ca->crl = linked_list_create();
	ca->ocsp = linked_list_create();
	ca->hashes = linked_list_create();
	ca->certuribase = NULL;
	return ca;
}

/**
 * destroy a ca section entry
 */
static void ca_section_destroy(ca_section_t *this)
{
	this->crl->destroy_function(this->crl, free);
	this->ocsp->destroy_function(this->ocsp, free);
	this->hashes->destroy_offset(this->hashes, offsetof(identification_t, destroy));
	this->cert->destroy(this->cert);
	free(this->certuribase);
	free(this->path);
	free(this->name);
	free(this);
}

/**
 * Destroy a ca cert entry
 */
static void ca_cert_destroy(ca_cert_t *this)
{
	this->cert->destroy(this->cert);
	free(this);
}

/**
 * Data for the certificate enumerator
 */
typedef struct {
	private_stroke_ca_t *this;
	certificate_type_t cert;
	key_type_t key;
	identification_t *id;
} cert_data_t;

CALLBACK(cert_data_destroy, void,
	cert_data_t *data)
{
	data->this->lock->unlock(data->this->lock);
	free(data);
}

CALLBACK(certs_filter, bool,
	cert_data_t *data, enumerator_t *orig, va_list args)
{
	ca_cert_t *cacert;
	public_key_t *public;
	certificate_t **out;

	VA_ARGS_VGET(args, out);

	while (orig->enumerate(orig, &cacert))
	{
		certificate_t *cert = cacert->cert;

		if (data->cert != CERT_ANY && data->cert != cert->get_type(cert))
		{
			continue;
		}
		public = cert->get_public_key(cert);
		if (public)
		{
			if (data->key == KEY_ANY || data->key == public->get_type(public))
			{
				if (data->id && public->has_fingerprint(public,
											data->id->get_encoding(data->id)))
				{
					public->destroy(public);
					*out = cert;
					return TRUE;
				}
			}
			public->destroy(public);
		}
		else if (data->key != KEY_ANY)
		{
			continue;
		}
		if (!data->id || cert->has_subject(cert, data->id))
		{
			*out = cert;
			return TRUE;
		}
	}
	return FALSE;
}

METHOD(credential_set_t, create_cert_enumerator, enumerator_t*,
	private_stroke_ca_t *this, certificate_type_t cert, key_type_t key,
	identification_t *id, bool trusted)
{
	enumerator_t *enumerator;
	cert_data_t *data;

	INIT(data,
		.this = this,
		.cert = cert,
		.key = key,
		.id = id,
	);

	this->lock->read_lock(this->lock);
	enumerator = this->certs->create_enumerator(this->certs);
	return enumerator_create_filter(enumerator, certs_filter, data,
									cert_data_destroy);
}

/**
 * data to pass to create_inner_cdp
 */
typedef struct {
	private_stroke_ca_t *this;
	certificate_type_t type;
	identification_t *id;
} cdp_data_t;

/**
 * destroy cdp enumerator data and unlock list
 */
static void cdp_data_destroy(cdp_data_t *data)
{
	data->this->lock->unlock(data->this->lock);
	free(data);
}

/**
 * inner enumerator constructor for CDP URIs
 */
static enumerator_t *create_inner_cdp(ca_section_t *section, cdp_data_t *data)
{
	public_key_t *public;
	enumerator_t *enumerator = NULL;
	linked_list_t *list;

	if (data->type == CERT_X509_OCSP_RESPONSE)
	{
		list = section->ocsp;
	}
	else
	{
		list = section->crl;
	}

	public = section->cert->get_public_key(section->cert);
	if (public)
	{
		if (!data->id)
		{
			enumerator = list->create_enumerator(list);
		}
		else
		{
			if (public->has_fingerprint(public, data->id->get_encoding(data->id)))
			{
				enumerator = list->create_enumerator(list);
			}
		}
		public->destroy(public);
	}
	return enumerator;
}

/**
 * inner enumerator constructor for "Hash and URL"
 */
static enumerator_t *create_inner_cdp_hashandurl(ca_section_t *section, cdp_data_t *data)
{
	enumerator_t *enumerator = NULL, *hash_enum;
	identification_t *current;

	if (!data->id || !section->certuribase)
	{
		return NULL;
	}

	hash_enum = section->hashes->create_enumerator(section->hashes);
	while (hash_enum->enumerate(hash_enum, &current))
	{
		if (current->matches(current, data->id))
		{
			char *url, *hash;

			url = malloc(strlen(section->certuribase) + 40 + 1);
			strcpy(url, section->certuribase);
			hash = chunk_to_hex(current->get_encoding(current), NULL, FALSE).ptr;
			strncat(url, hash, 40);
			free(hash);

			enumerator = enumerator_create_single(url, free);
			break;
		}
	}
	hash_enum->destroy(hash_enum);
	return enumerator;
}

METHOD(credential_set_t, create_cdp_enumerator, enumerator_t*,
	private_stroke_ca_t *this, certificate_type_t type, identification_t *id)
{
	cdp_data_t *data;

	switch (type)
	{	/* we serve CRLs, OCSP responders and URLs for "Hash and URL" */
		case CERT_X509:
		case CERT_X509_CRL:
		case CERT_X509_OCSP_RESPONSE:
		case CERT_ANY:
			break;
		default:
			return NULL;
	}
	data = malloc_thing(cdp_data_t);
	data->this = this;
	data->type = type;
	data->id = id;

	this->lock->read_lock(this->lock);
	return enumerator_create_nested(this->sections->create_enumerator(this->sections),
			(type == CERT_X509) ? (void*)create_inner_cdp_hashandurl : (void*)create_inner_cdp,
			data, (void*)cdp_data_destroy);
}

CALLBACK(match_cert, bool,
	ca_cert_t *item, va_list args)
{
	certificate_t *cert;

	VA_ARGS_VGET(args, cert);
	return cert->equals(cert, item->cert);
}

/**
 * Match automatically added certificates and remove/destroy them if they are
 * not referenced by CA sections.
 */
static bool remove_auto_certs(ca_cert_t *item, void *not_used)
{
	if (item->automatic)
	{
		item->automatic = FALSE;
		if (!item->count)
		{
			ca_cert_destroy(item);
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Find the given certificate that was referenced by a section and remove it
 * unless it was also loaded automatically or is used by other CA sections.
 */
static bool remove_cert(ca_cert_t *item, certificate_t *cert)
{
	if (item->count && cert->equals(cert, item->cert))
	{
		if (--item->count == 0 && !item->automatic)
		{
			ca_cert_destroy(item);
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Adds a certificate to the certificate store
 */
static certificate_t *add_cert_internal(private_stroke_ca_t *this,
										certificate_t *cert, bool automatic)
{
	ca_cert_t *found;

	if (this->certs->find_first(this->certs, match_cert, (void**)&found, cert))
	{
		cert->destroy(cert);
		cert = found->cert->get_ref(found->cert);
	}
	else
	{
		INIT(found,
			.cert = cert->get_ref(cert)
		);
		this->certs->insert_first(this->certs, found);
	}
	if (automatic)
	{
		found->automatic = TRUE;
	}
	else
	{
		found->count++;
	}
	return cert;
}

METHOD(stroke_ca_t, add, void,
	private_stroke_ca_t *this, stroke_msg_t *msg)
{
	certificate_t *cert;
	ca_section_t *ca;

	if (msg->add_ca.cacert == NULL)
	{
		DBG1(DBG_CFG, "missing cacert parameter");
		return;
	}
	cert = stroke_load_ca_cert(msg->add_ca.cacert);
	if (cert)
	{
		ca = ca_section_create(msg->add_ca.name, msg->add_ca.cacert);
		if (msg->add_ca.crluri)
		{
			ca->crl->insert_last(ca->crl, strdup(msg->add_ca.crluri));
		}
		if (msg->add_ca.crluri2)
		{
			ca->crl->insert_last(ca->crl, strdup(msg->add_ca.crluri2));
		}
		if (msg->add_ca.ocspuri)
		{
			ca->ocsp->insert_last(ca->ocsp, strdup(msg->add_ca.ocspuri));
		}
		if (msg->add_ca.ocspuri2)
		{
			ca->ocsp->insert_last(ca->ocsp, strdup(msg->add_ca.ocspuri2));
		}
		if (msg->add_ca.certuribase)
		{
			ca->certuribase = strdup(msg->add_ca.certuribase);
		}
		this->lock->write_lock(this->lock);
		ca->cert = add_cert_internal(this, cert, FALSE);
		this->sections->insert_last(this->sections, ca);
		this->lock->unlock(this->lock);
		DBG1(DBG_CFG, "added ca '%s'", msg->add_ca.name);
	}
}

METHOD(stroke_ca_t, del, void,
	private_stroke_ca_t *this, stroke_msg_t *msg)
{
	enumerator_t *enumerator;
	ca_section_t *ca = NULL;

	this->lock->write_lock(this->lock);
	enumerator = this->sections->create_enumerator(this->sections);
	while (enumerator->enumerate(enumerator, &ca))
	{
		if (streq(ca->name, msg->del_ca.name))
		{
			this->sections->remove_at(this->sections, enumerator);
			break;
		}
		ca = NULL;
	}
	enumerator->destroy(enumerator);
	if (ca)
	{
		this->certs->remove(this->certs, ca->cert, (void*)remove_cert);
	}
	this->lock->unlock(this->lock);
	if (!ca)
	{
		DBG1(DBG_CFG, "no ca named '%s' found\n", msg->del_ca.name);
		return;
	}
	ca_section_destroy(ca);

	lib->credmgr->flush_cache(lib->credmgr, CERT_ANY);
}

METHOD(stroke_ca_t, get_cert_ref, certificate_t*,
	private_stroke_ca_t *this, certificate_t *cert)
{
	ca_cert_t *found;

	this->lock->read_lock(this->lock);
	if (this->certs->find_first(this->certs, match_cert, (void**)&found, cert))
	{
		cert->destroy(cert);
		cert = found->cert->get_ref(found->cert);
	}
	this->lock->unlock(this->lock);
	return cert;
}

METHOD(stroke_ca_t, reload_certs, void,
	private_stroke_ca_t *this)
{
	enumerator_t *enumerator;
	certificate_t *cert;
	ca_section_t *ca;
	certificate_type_t type = CERT_X509;

	/* holding the write lock while loading/parsing certificates is not optimal,
	 * however, there usually are not that many ca sections configured */
	this->lock->write_lock(this->lock);
	if (this->sections->get_count(this->sections))
	{
		DBG1(DBG_CFG, "rereading ca certificates in ca sections");
	}
	enumerator = this->sections->create_enumerator(this->sections);
	while (enumerator->enumerate(enumerator, &ca))
	{
		cert = stroke_load_ca_cert(ca->path);
		if (cert)
		{
			if (cert->equals(cert, ca->cert))
			{
				cert->destroy(cert);
			}
			else
			{
				this->certs->remove(this->certs, ca->cert, (void*)remove_cert);
				ca->cert->destroy(ca->cert);
				ca->cert = add_cert_internal(this, cert, FALSE);
			}
		}
		else
		{
			DBG1(DBG_CFG, "failed to reload certificate '%s', removing ca '%s'",
				 ca->path, ca->name);
			this->sections->remove_at(this->sections, enumerator);
			this->certs->remove(this->certs, ca->cert, (void*)remove_cert);
			ca_section_destroy(ca);
			type = CERT_ANY;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	lib->credmgr->flush_cache(lib->credmgr, type);
}

METHOD(stroke_ca_t, replace_certs, void,
	private_stroke_ca_t *this, mem_cred_t *certs)
{
	enumerator_t *enumerator;
	certificate_t *cert;

	enumerator = certs->set.create_cert_enumerator(&certs->set, CERT_X509,
												   KEY_ANY, NULL, TRUE);
	this->lock->write_lock(this->lock);
	this->certs->remove(this->certs, NULL, (void*)remove_auto_certs);
	while (enumerator->enumerate(enumerator, &cert))
	{
		cert = add_cert_internal(this, cert->get_ref(cert), TRUE);
		cert->destroy(cert);
	}
	this->lock->unlock(this->lock);
	enumerator->destroy(enumerator);
	lib->credmgr->flush_cache(lib->credmgr, CERT_X509);
}
/**
 * list crl or ocsp URIs
 */
static void list_uris(linked_list_t *list, char *label, FILE *out)
{
	bool first = TRUE;
	char *uri;
	enumerator_t *enumerator;

	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, (void**)&uri))
	{
		if (first)
		{
			fprintf(out, "%s", label);
			first = FALSE;
		}
		else
		{
			fprintf(out, "            ");
		}
		fprintf(out, "'%s'\n", uri);
	}
	enumerator->destroy(enumerator);
}

METHOD(stroke_ca_t, check_for_hash_and_url, void,
	private_stroke_ca_t *this, certificate_t* cert)
{
	ca_section_t *section;
	enumerator_t *enumerator;

	hasher_t *hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (hasher == NULL)
	{
		DBG1(DBG_IKE, "unable to use hash-and-url: sha1 not supported");
		return;
	}

	this->lock->write_lock(this->lock);
	enumerator = this->sections->create_enumerator(this->sections);
	while (enumerator->enumerate(enumerator, (void**)&section))
	{
		if (section->certuribase && cert->issued_by(cert, section->cert, NULL))
		{
			chunk_t hash, encoded;

			if (cert->get_encoding(cert, CERT_ASN1_DER, &encoded))
			{
				if (hasher->allocate_hash(hasher, encoded, &hash))
				{
					section->hashes->insert_last(section->hashes,
						identification_create_from_encoding(ID_KEY_ID, hash));
					chunk_free(&hash);
				}
				chunk_free(&encoded);
			}
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	hasher->destroy(hasher);
}

METHOD(stroke_ca_t, list, void,
	private_stroke_ca_t *this, stroke_msg_t *msg, FILE *out)
{
	bool first = TRUE;
	ca_section_t *section;
	enumerator_t *enumerator;

	this->lock->read_lock(this->lock);
	enumerator = this->sections->create_enumerator(this->sections);
	while (enumerator->enumerate(enumerator, (void**)&section))
	{
		certificate_t *cert = section->cert;
		public_key_t *public = cert->get_public_key(cert);
		chunk_t chunk;

		if (first)
		{
			fprintf(out, "\n");
			fprintf(out, "List of CA Information Sections:\n");
			first = FALSE;
		}
		fprintf(out, "\n");
		fprintf(out, "  authname:    \"%Y\"\n", cert->get_subject(cert));

		/* list authkey and keyid */
		if (public)
		{
			if (public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &chunk))
			{
				fprintf(out, "  authkey:      %#B\n", &chunk);
			}
			if (public->get_fingerprint(public, KEYID_PUBKEY_INFO_SHA1, &chunk))
			{
				fprintf(out, "  keyid:        %#B\n", &chunk);
			}
			public->destroy(public);
		}
		list_uris(section->crl, "  crluris:     ", out);
		list_uris(section->ocsp, "  ocspuris:    ", out);
		if (section->certuribase)
		{
			fprintf(out, "  certuribase: '%s'\n", section->certuribase);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

METHOD(stroke_ca_t, destroy, void,
	private_stroke_ca_t *this)
{
	this->sections->destroy_function(this->sections, (void*)ca_section_destroy);
	this->certs->destroy_function(this->certs, (void*)ca_cert_destroy);
	this->lock->destroy(this->lock);
	free(this);
}

/*
 * see header file
 */
stroke_ca_t *stroke_ca_create()
{
	private_stroke_ca_t *this;

	INIT(this,
		.public = {
			.set = {
				.create_private_enumerator = (void*)return_null,
				.create_cert_enumerator = _create_cert_enumerator,
				.create_shared_enumerator = (void*)return_null,
				.create_cdp_enumerator = _create_cdp_enumerator,
				.cache_cert = (void*)nop,
			},
			.add = _add,
			.del = _del,
			.list = _list,
			.get_cert_ref = _get_cert_ref,
			.reload_certs = _reload_certs,
			.replace_certs = _replace_certs,
			.check_for_hash_and_url = _check_for_hash_and_url,
			.destroy = _destroy,
		},
		.sections = linked_list_create(),
		.certs = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}
