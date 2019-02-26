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

#include "certexpire_export.h"

#include "certexpire_cron.h"

#include <time.h>
#include <limits.h>
#include <errno.h>

#include <utils/debug.h>
#include <daemon.h>
#include <collections/hashtable.h>
#include <threading/mutex.h>
#include <credentials/certificates/x509.h>

typedef struct private_certexpire_export_t private_certexpire_export_t;

/**
 * Private data of an certexpire_export_t object.
 */
struct private_certexpire_export_t {

	/**
	 * Public certexpire_export_t interface.
	 */
	certexpire_export_t public;

	/**
	 * hashtable caching local trustchains, mapping entry_t => entry_t
	 */
	hashtable_t *local;

	/**
	 * hashtable caching remote trustchains, mapping entry_t => entry_t
	 */
	hashtable_t *remote;

	/**
	 * Mutex to lock hashtables
	 */
	mutex_t *mutex;

	/**
	 * Cronjob for export
	 */
	certexpire_cron_t *cron;

	/**
	 * strftime() format to generate local CSV file
	 */
	char *local_path;

	/**
	 * strftime() format to generate remote CSV file
	 */
	char *remote_path;

	/**
	 * stftime() format of the exported expiration date
	 */
	char *format;

	/**
	 * CSV field separator
	 */
	char *separator;

	/**
	 * TRUE to use fixed field count, CA at end
	 */
	bool fixed_fields;

	/**
	 * String to use in empty fields, if using fixed_fields
	 */
	char *empty_string;

	/**
	 * Force export of all trustchains we have a private key for
	 */
	bool force;
};

/**
 * Maximum number of expiration dates we store (for subject + IM CAs + CA)
 */
#define MAX_TRUSTCHAIN_LENGTH 7

/**
 * Hashtable entry
 */
typedef struct {
	/** certificate subject as subjectAltName or CN of a DN */
	char id[128];
	/** list of expiration dates, 0 if no certificate */
	time_t expire[MAX_TRUSTCHAIN_LENGTH];
} entry_t;

/**
 * Hashtable hash function
 */
static u_int hash(entry_t *key)
{
	return chunk_hash(chunk_create(key->id, strlen(key->id)));
}

/**
 * Hashtable equals function
 */
static bool equals(entry_t *a, entry_t *b)
{
	return streq(a->id, b->id);
}

/**
 * Export a single trustchain to a path
 */
static void export_csv(private_certexpire_export_t *this, char *path,
					   hashtable_t *chains)
{
	enumerator_t *enumerator;
	char buf[PATH_MAX];
	entry_t *entry;
	FILE *file;
	struct tm tm;
	time_t t;
	int i;

	t = time(NULL);
	localtime_r(&t, &tm);

	strftime(buf, sizeof(buf), path, &tm);
	file = fopen(buf, "a");
	if (file)
	{
		DBG1(DBG_CFG, "exporting expiration dates of %d trustchain%s to '%s'",
			 chains->get_count(chains),
			 chains->get_count(chains) == 1 ? "" : "s", buf);
		this->mutex->lock(this->mutex);
		enumerator = chains->create_enumerator(chains);
		while (enumerator->enumerate(enumerator, NULL, &entry))
		{
			fprintf(file, "%s%s", entry->id, this->separator);
			for (i = 0; i < MAX_TRUSTCHAIN_LENGTH; i++)
			{
				if (entry->expire[i])
				{
					localtime_r(&entry->expire[i], &tm);
					strftime(buf, sizeof(buf), this->format, &tm);
					fprintf(file, "%s", buf);
				}
				if (i == MAX_TRUSTCHAIN_LENGTH - 1)
				{
					fprintf(file, "\n");
				}
				else if (entry->expire[i])
				{
					fprintf(file, "%s", this->separator);
				}
				else if (this->fixed_fields)
				{
					fprintf(file, "%s%s", this->empty_string, this->separator);
				}
			}
			chains->remove_at(chains, enumerator);
			free(entry);
		}
		enumerator->destroy(enumerator);
		this->mutex->unlock(this->mutex);
		fclose(file);
	}
	else
	{
		DBG1(DBG_CFG, "opening CSV file '%s' failed: %s", buf, strerror(errno));
	}
}

METHOD(certexpire_export_t, add, void,
	private_certexpire_export_t *this, linked_list_t *trustchain, bool local)
{
	enumerator_t *enumerator;
	certificate_t *cert;
	int count;

	/* don't store expiration dates if no path configured */
	if (local)
	{
		if (!this->local_path)
		{
			return;
		}
	}
	else
	{
		if (!this->remote_path)
		{
			return;
		}
	}

	count = min(trustchain->get_count(trustchain), MAX_TRUSTCHAIN_LENGTH) - 1;

	enumerator = trustchain->create_enumerator(trustchain);
	/* get subject cert */
	if (enumerator->enumerate(enumerator, &cert))
	{
		identification_t *id;
		entry_t *entry;
		int i;

		INIT(entry);

		/* prefer FQDN subjectAltName... */
		if (cert->get_type(cert) == CERT_X509)
		{
			x509_t *x509 = (x509_t*)cert;
			enumerator_t *sans;

			sans = x509->create_subjectAltName_enumerator(x509);
			while (sans->enumerate(sans, &id))
			{
				if (id->get_type(id) == ID_FQDN)
				{
					snprintf(entry->id, sizeof(entry->id), "%Y", id);
					break;
				}
			}
			sans->destroy(sans);
		}
		/* fallback to CN of DN */
		if (!entry->id[0])
		{
			enumerator_t *parts;
			id_part_t part;
			chunk_t data;

			id = cert->get_subject(cert);
			parts = id->create_part_enumerator(id);
			while (parts->enumerate(parts, &part, &data))
			{
				if (part == ID_PART_RDN_CN)
				{
					snprintf(entry->id, sizeof(entry->id), "%.*s",
							 (int)data.len, data.ptr);
					break;
				}
			}
			parts->destroy(parts);
		}
		/* no usable identity? skip */
		if (!entry->id[0])
		{
			enumerator->destroy(enumerator);
			free(entry);
			return;
		}

		/* get intermediate CA expiration dates */
		cert->get_validity(cert, NULL, NULL, &entry->expire[0]);
		for (i = 1; i < count && enumerator->enumerate(enumerator, &cert); i++)
		{
			cert->get_validity(cert, NULL, NULL, &entry->expire[i]);
		}
		/* get CA expiration date, as last array entry */
		if (enumerator->enumerate(enumerator, &cert))
		{
			cert->get_validity(cert, NULL, NULL,
							   &entry->expire[MAX_TRUSTCHAIN_LENGTH - 1]);
		}
		this->mutex->lock(this->mutex);
		if (local)
		{
			entry = this->local->put(this->local, entry, entry);
		}
		else
		{
			entry = this->remote->put(this->remote, entry, entry);
		}
		this->mutex->unlock(this->mutex);
		if (entry)
		{
			free(entry);
		}
		if (!this->cron)
		{	/* export directly if no cron job defined */
			if (local)
			{
				export_csv(this, this->local_path, this->local);
			}
			else
			{
				export_csv(this, this->remote_path, this->remote);
			}
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Add trustchains we have a private key for to the list
 */
static void add_local_certs(private_certexpire_export_t *this)
{
	enumerator_t *enumerator;
	certificate_t *cert;

	enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr,
											CERT_X509, KEY_ANY, NULL, FALSE);
	while (enumerator->enumerate(enumerator, &cert))
	{
		linked_list_t *trustchain;
		private_key_t *private;
		public_key_t *public;
		identification_t *keyid;
		chunk_t chunk;
		x509_t *x509 = (x509_t*)cert;

		trustchain = linked_list_create();

		public = cert->get_public_key(cert);
		if (public)
		{
			if (public->get_fingerprint(public, KEYID_PUBKEY_INFO_SHA1, &chunk))
			{
				keyid = identification_create_from_encoding(ID_KEY_ID, chunk);
				private = lib->credmgr->get_private(lib->credmgr,
										public->get_type(public), keyid, NULL);
				keyid->destroy(keyid);
				if (private)
				{
					trustchain->insert_last(trustchain, cert->get_ref(cert));

					while (!(x509->get_flags(x509) & X509_SELF_SIGNED))
					{
						cert = lib->credmgr->get_cert(lib->credmgr, CERT_X509,
										KEY_ANY, cert->get_issuer(cert), FALSE);
						if (!cert)
						{
							break;
						}
						x509 = (x509_t*)cert;
						trustchain->insert_last(trustchain, cert);
					}
					private->destroy(private);
				}
			}
			public->destroy(public);
		}
		add(this, trustchain, TRUE);
		trustchain->destroy_offset(trustchain, offsetof(certificate_t, destroy));
	}
	enumerator->destroy(enumerator);
}

/**
 * Export cached trustchain expiration dates to CSV files
 */
static void cron_export(private_certexpire_export_t *this)
{
	if (this->local_path)
	{
		if (this->force)
		{
			add_local_certs(this);
		}
		export_csv(this, this->local_path, this->local);
	}
	if (this->remote_path)
	{
		export_csv(this, this->remote_path, this->remote);
	}
}

METHOD(certexpire_export_t, destroy, void,
	private_certexpire_export_t *this)
{
	entry_t *key, *value;
	enumerator_t *enumerator;

	enumerator = this->local->create_enumerator(this->local);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		free(value);
	}
	enumerator->destroy(enumerator);
	enumerator = this->remote->create_enumerator(this->remote);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		free(value);
	}
	enumerator->destroy(enumerator);

	this->local->destroy(this->local);
	this->remote->destroy(this->remote);
	DESTROY_IF(this->cron);
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * See header
 */
certexpire_export_t *certexpire_export_create()
{
	private_certexpire_export_t *this;
	char *cron;

	INIT(this,
		.public = {
			.add = _add,
			.destroy = _destroy,
		},
		.local = hashtable_create((hashtable_hash_t)hash,
								  (hashtable_equals_t)equals, 4),
		.remote = hashtable_create((hashtable_hash_t)hash,
								   (hashtable_equals_t)equals, 32),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.local_path = lib->settings->get_str(lib->settings,
									"%s.plugins.certexpire.csv.local",
									NULL, lib->ns),
		.remote_path = lib->settings->get_str(lib->settings,
									"%s.plugins.certexpire.csv.remote",
									NULL, lib->ns),
		.separator = lib->settings->get_str(lib->settings,
									"%s.plugins.certexpire.csv.separator",
									",", lib->ns),
		.format = lib->settings->get_str(lib->settings,
									"%s.plugins.certexpire.csv.format",
									"%d:%m:%Y", lib->ns),
		.fixed_fields = lib->settings->get_bool(lib->settings,
									"%s.plugins.certexpire.csv.fixed_fields",
									TRUE, lib->ns),
		.empty_string = lib->settings->get_str(lib->settings,
									"%s.plugins.certexpire.csv.empty_string",
									"", lib->ns),
		.force = lib->settings->get_bool(lib->settings,
									"%s.plugins.certexpire.csv.force",
									TRUE, lib->ns),
	);

	cron = lib->settings->get_str(lib->settings,
								  "%s.plugins.certexpire.csv.cron",
								  NULL, lib->ns);
	if (cron)
	{
		this->cron = certexpire_cron_create(cron,
									(certexpire_cron_job_t)cron_export, this);
	}
	return &this->public;
}
