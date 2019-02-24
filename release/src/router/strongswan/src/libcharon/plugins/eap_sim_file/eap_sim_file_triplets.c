/*
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

#include "eap_sim_file_triplets.h"

#include <stdio.h>
#include <errno.h>

#include <daemon.h>
#include <collections/linked_list.h>
#include <threading/mutex.h>
#include <simaka_manager.h>

typedef struct private_eap_sim_file_triplets_t private_eap_sim_file_triplets_t;

/**
 * Private data of an eap_sim_file_triplets_t object.
 */
struct private_eap_sim_file_triplets_t {

	/**
	 * Public eap_sim_file_triplets_t interface.
	 */
	eap_sim_file_triplets_t public;

	/**
	 * List of triplets, as triplet_t
	 */
	linked_list_t *triplets;

	/**
	 * mutex to lock triplets list
	 */
	mutex_t *mutex;
};

/**
 * A single triplet
 */
typedef struct  {
	identification_t *imsi;
	char rand[SIM_RAND_LEN];
	char sres[SIM_SRES_LEN];
	char kc[SIM_KC_LEN];
} triplet_t;

/**
 * Destroy a triplet
 */
static void triplet_destroy(triplet_t *this)
{
	DESTROY_IF(this->imsi);
	free(this);
}

/**
 * triplet enumerator
 */
typedef struct {
	/** implements enumerator */
	enumerator_t public;
	/** inner enumerator */
	enumerator_t *inner;
	/** current enumerating triplet */
	triplet_t *current;
	/** back ptr */
	private_eap_sim_file_triplets_t *this;
} triplet_enumerator_t;

METHOD(enumerator_t, enumerator_destroy, void,
	triplet_enumerator_t *e)
{
	if (e->current)
	{
		/* We assume that the current element is used on invocation if destroy.
		 * We move that triplet to the end to avoid handout of the same triplet
		 * next time. */
		e->this->triplets->remove_at(e->this->triplets, e->inner);
		e->this->triplets->insert_last(e->this->triplets, e->current);
	}
	e->inner->destroy(e->inner);
	e->this->mutex->unlock(e->this->mutex);
	free(e);
}

METHOD(enumerator_t, enumerator_enumerate, bool,
	triplet_enumerator_t *e, va_list args)
{
	identification_t **imsi;
	triplet_t *triplet;
	char **rand, **sres, **kc;

	VA_ARGS_VGET(args, imsi, rand, sres, kc);

	if (e->inner->enumerate(e->inner, &triplet))
	{
		e->current = triplet;
		*imsi = triplet->imsi;
		*rand = triplet->rand;
		*sres = triplet->sres;
		*kc = triplet->kc;
		return TRUE;
	}
	e->current = NULL;
	return FALSE;
}

METHOD(eap_sim_file_triplets_t, create_enumerator, enumerator_t*,
	private_eap_sim_file_triplets_t *this)
{
	triplet_enumerator_t *enumerator;

	this->mutex->lock(this->mutex);
	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerator_enumerate,
			.destroy = _enumerator_destroy,
		},
		.inner = this->triplets->create_enumerator(this->triplets),
		.this = this,
	);
	return &enumerator->public;
}

/**
 * convert to token into the array
 */
static void parse_token(char *to, char *from, size_t len)
{
	chunk_t chunk;

	chunk = chunk_create(from, min(strlen(from), len * 2));
	chunk = chunk_from_hex(chunk, NULL);
	memset(to, 0, len);
	memcpy(to + len - chunk.len, chunk.ptr, chunk.len);
	free(chunk.ptr);
}

/**
 * Read the triplets from the file
 */
static bool read_triplets(private_eap_sim_file_triplets_t *this, char *path)
{
	char line[512];
	FILE *file;
	int i, nr = 0;

	file = fopen(path, "r");
	if (file == NULL)
	{
		DBG1(DBG_CFG, "opening triplet file %s failed: %s",
			 path, strerror(errno));
		return FALSE;
	}

	/* read line by line */
	while (fgets(line, sizeof(line), file))
	{
		triplet_t *triplet;
		enumerator_t *enumerator;
		char *token;

		nr++;
		/* skip comments, empty lines */
		switch (line[0])
		{
			case '\n':
			case '\r':
			case '#':
			case '\0':
				continue;
			default:
				break;
		}
		triplet = malloc_thing(triplet_t);
		memset(triplet, 0, sizeof(triplet_t));

		i = 0;
		enumerator = enumerator_create_token(line, ",", " \n\r#");
		while (enumerator->enumerate(enumerator, &token))
		{
			switch (i++)
			{
				case 0: /* IMSI */
					triplet->imsi = identification_create_from_string(token);
					continue;
				case 1: /* rand */
					parse_token(triplet->rand, token, SIM_RAND_LEN);
					continue;
				case 2: /* sres */
					parse_token(triplet->sres, token, SIM_SRES_LEN);
					continue;
				case 3: /* kc */
					parse_token(triplet->kc, token, SIM_KC_LEN);
					continue;
				default:
					break;;
			}
			break;
		}
		enumerator->destroy(enumerator);
		if (i < 4)
		{
			DBG1(DBG_CFG, "error in triplet file, line %d", nr);
			triplet_destroy(triplet);
			continue;
		}

		DBG2(DBG_CFG, "triplet: imsi %Y\nrand %b\nsres %b\nkc %b",
			 triplet->imsi, triplet->rand, SIM_RAND_LEN,
			 triplet->sres, SIM_SRES_LEN, triplet->kc, SIM_KC_LEN);

		this->triplets->insert_last(this->triplets, triplet);
	}
	fclose(file);

	DBG1(DBG_CFG, "read %d triplets from %s",
		 this->triplets->get_count(this->triplets), path);
	return TRUE;
}

METHOD(eap_sim_file_triplets_t, destroy, void,
	private_eap_sim_file_triplets_t *this)
{
	this->triplets->destroy_function(this->triplets, (void*)triplet_destroy);
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * See header
 */
eap_sim_file_triplets_t *eap_sim_file_triplets_create(char *file)
{
	private_eap_sim_file_triplets_t *this;

	INIT(this,
		.public = {
			.create_enumerator = _create_enumerator,
			.destroy = _destroy,
		},
		.triplets = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);

	if (!read_triplets(this, file))
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}

