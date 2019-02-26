/*
 * Copyright (C) 2011-2017 Andreas Steffen
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

#include "ita_comp_ima.h"
#include "ita_comp_func_name.h"

#include "imcv.h"
#include "pts/pts_pcr.h"
#include "pts/pts_ima_bios_list.h"
#include "pts/pts_ima_event_list.h"
#include "pts/components/pts_component.h"

#include <utils/debug.h>
#include <crypto/hashers/hasher.h>
#include <pen/pen.h>

#define SECURITY_DIR				"/sys/kernel/security/"
#define IMA_BIOS_MEASUREMENTS		SECURITY_DIR "tpm0/binary_bios_measurements"
#define IMA_RUNTIME_MEASUREMENTS	SECURITY_DIR "ima/binary_runtime_measurements"
#define IMA_FILENAME_LEN_MAX		255

typedef struct pts_ita_comp_ima_t pts_ita_comp_ima_t;
typedef enum ima_state_t ima_state_t;

enum ima_state_t {
	IMA_STATE_INIT,
	IMA_STATE_BIOS,
	IMA_STATE_BOOT_AGGREGATE,
	IMA_STATE_RUNTIME,
	IMA_STATE_END
};

/**
 * Private data of a pts_ita_comp_ima_t object.
 *
 */
struct pts_ita_comp_ima_t {

	/**
	 * Public pts_component_t interface.
	 */
	pts_component_t public;

	/**
	 * Component Functional Name
	 */
	pts_comp_func_name_t *name;

	/**
	 * Sub-component depth
	 */
	uint32_t depth;

	/**
	 * PTS measurement database
	 */
	pts_database_t *pts_db;

	/**
	 * Primary key for AIK database entry
	 */
	int aik_id;

	/**
	 * Primary key for IMA BIOS Component Functional Name database entry
	 */
	int bios_cid;

	/**
	 * Primary key for IMA Runtime Component Functional Name database entry
	 */
	int ima_cid;

	/**
	 * Component is registering IMA BIOS measurements
	 */
	bool is_bios_registering;

	/**
	 * Component is registering IMA boot aggregate measurement
	 */
	bool is_ima_registering;

	/**
	 * Measurement sequence number
	 */
	int seq_no;

	/**
	 * Expected IMA BIOS measurement count
	 */
	int bios_count;

	/**
     * IMA BIOS measurements
	 */
	pts_ima_bios_list_t *bios_list;

	/**
     * IMA runtime file measurements
	 */
	pts_ima_event_list_t *ima_list;

	/**
	 * Whether to send pcr_before and pcr_after info
	 */
	bool pcr_info;

	/**
	 * Creation time of measurement
	 */
	time_t creation_time;

	/**
	 * IMA state machine
	 */
	ima_state_t state;

	/**
	 * Total number of component measurements
	 */
	int count;

	/**
	 * Number of successful component measurements
	 */
	int count_ok;

	/**
	 * Number of unknown component measurements
	 */
	int count_unknown;

	/**
	 * Number of differing component measurements
	 */
	int count_differ;

	/**
	 * Number of failed component measurements
	 */
	int count_failed;

	/**
	 * Reference count
	 */
	refcount_t ref;

};

/**
 * Extend measurement into PCR and create evidence
 */
static pts_comp_evidence_t* extend_pcr(pts_ita_comp_ima_t* this,
									   uint8_t qualifier, pts_pcr_t *pcrs,
									   uint32_t pcr, chunk_t measurement)
{
	size_t pcr_len;
	pts_pcr_transform_t pcr_transform;
	pts_meas_algorithms_t hash_algo;
	pts_comp_func_name_t *name;
	pts_comp_evidence_t *evidence;
	chunk_t pcr_before = chunk_empty, pcr_after = chunk_empty;

	hash_algo = PTS_MEAS_ALGO_SHA1;
	pcr_len = HASH_SIZE_SHA1;
	pcr_transform = pts_meas_algo_to_pcr_transform(hash_algo, pcr_len);

	if (this->pcr_info)
	{
		pcr_before = chunk_clone(pcrs->get(pcrs, pcr));
	}
	pcr_after = pcrs->extend(pcrs, pcr, measurement);
	if (!pcr_after.ptr)
	{
		free(pcr_before.ptr);
		return NULL;
	}
	name = this->name->clone(this->name);
	name->set_qualifier(name, qualifier);
	evidence = pts_comp_evidence_create(name, this->depth, pcr, hash_algo,
						pcr_transform, this->creation_time, measurement);
	if (this->pcr_info)
	{
		pcr_after =chunk_clone(pcrs->get(pcrs, pcr));
		evidence->set_pcr_info(evidence, pcr_before, pcr_after);
	}
	return evidence;
}

/**
 * Generate an IMA or IMA-NG hash from an event digest and event name
 *
 * @param digest		event digest
 * @param ima_algo		hash algorithm string ("sha1:", "sha256:", etc.)
 * @param ima_name		event name
 * @param little_endian	endianness of client platform
 * @param algo			hash algorithm used by TPM
 * @param hash_buf		hash value to be compared with TPM measurement
 */
static bool ima_hash(chunk_t digest, char *ima_algo, char *ima_name,
					 bool little_endian, pts_meas_algorithms_t algo,
					 char *hash_buf)
{
	hash_algorithm_t hash_alg;
	hasher_t *hasher;
	bool success;

	hash_alg = pts_meas_algo_to_hash(algo);
	hasher = lib->crypto->create_hasher(lib->crypto, hash_alg);
	if (!hasher)
	{
		DBG1(DBG_PTS, "%N hasher could not be created",
			 hash_algorithm_short_names, hash_alg);
		return FALSE;
	}

	if (ima_algo)
	{
		uint32_t d_len, n_len;
		chunk_t algo_name, event_name, digest_len, name_len;

		/* IMA-NG hash */
		algo_name  = chunk_create(ima_algo, strlen(ima_algo) + 1);
		event_name = chunk_create(ima_name, strlen(ima_name) + 1);

		d_len = algo_name.len + digest.len;
		digest_len = chunk_create((uint8_t*)&d_len, sizeof(d_len));
		/* TODO handle endianness of both client and server platforms */

		n_len = event_name.len;
		name_len = chunk_create((uint8_t*)&n_len, sizeof(n_len));
		/* TODO handle endianness of both client and server platforms */

		success = hasher->get_hash(hasher, digest_len, NULL) &&
				  hasher->get_hash(hasher, algo_name, NULL) &&
				  hasher->get_hash(hasher, digest, NULL) &&
				  hasher->get_hash(hasher, name_len, NULL) &&
				  hasher->get_hash(hasher, event_name, hash_buf);
	}
	else
	{
		u_char filename_buffer[IMA_FILENAME_LEN_MAX + 1];
		chunk_t file_name;

		/* IMA legacy hash */
		memset(filename_buffer, 0, sizeof(filename_buffer));
		strncpy(filename_buffer, ima_name, IMA_FILENAME_LEN_MAX);
		file_name = chunk_create (filename_buffer, sizeof(filename_buffer));

		success = hasher->get_hash(hasher, digest, NULL) &&
				  hasher->get_hash(hasher, file_name, hash_buf);
	}
	hasher->destroy(hasher);

	return success;
}

/**
 * Compute and check boot aggregate value by hashing PCR0 to PCR7
 */
static bool check_boot_aggregate(pts_pcr_t *pcrs, chunk_t measurement,
								 char *algo)
{
	u_char pcr_buffer[HASH_SIZE_SHA1];
	chunk_t boot_aggregate;
	hasher_t *hasher;
	uint32_t i;
	bool success, pcr_ok = TRUE;

	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (!hasher)
	{
		DBG1(DBG_PTS, "%N hasher could not be created",
			 hash_algorithm_short_names, HASH_SHA1);
		return FALSE;
	}
	for (i = 0; i < 8 && pcr_ok; i++)
	{
		pcr_ok = hasher->get_hash(hasher, pcrs->get(pcrs, i), NULL);
	}
	if (pcr_ok)
	{
		pcr_ok = hasher->get_hash(hasher, chunk_empty, pcr_buffer);
	}
	hasher->destroy(hasher);

	if (pcr_ok)
	{
		boot_aggregate = chunk_create(pcr_buffer, sizeof(pcr_buffer));

		/* TODO handle endianness of client platform */
		pcr_ok = ima_hash(boot_aggregate, algo, "boot_aggregate",
						  TRUE, PTS_MEAS_ALGO_SHA1, pcr_buffer);
	}
	if (pcr_ok)
	{
		success = chunk_equals_const(boot_aggregate, measurement);
		DBG1(DBG_PTS, "boot aggregate value is %scorrect",
					   success ? "":"in");
		return success;
	}
	else
	{
		DBG1(DBG_PTS, "failed to compute boot aggregate value");
		return FALSE;
	}
}

METHOD(pts_component_t, get_comp_func_name, pts_comp_func_name_t*,
	pts_ita_comp_ima_t *this)
{
	return this->name;
}

METHOD(pts_component_t, get_evidence_flags, uint8_t,
	pts_ita_comp_ima_t *this)
{
	return PTS_REQ_FUNC_COMP_EVID_PCR;
}

METHOD(pts_component_t, get_depth, uint32_t,
	pts_ita_comp_ima_t *this)
{
	return this->depth;
}

METHOD(pts_component_t, measure, status_t,
	pts_ita_comp_ima_t *this, uint8_t qualifier, pts_t *pts,
	pts_comp_evidence_t **evidence)
{
	pts_pcr_t *pcrs;
	pts_comp_evidence_t *evid = NULL;
	size_t algo_len, name_len;
	chunk_t measurement;
	char *uri, *algo, *name;
	uint32_t pcr;
	status_t status;

	pcrs = pts->get_pcrs(pts);

	if (qualifier == (PTS_ITA_QUALIFIER_FLAG_KERNEL |
					  PTS_ITA_QUALIFIER_TYPE_TRUSTED))
	{
		switch (this->state)
		{
			case IMA_STATE_INIT:
				this->bios_list = pts_ima_bios_list_create(
												IMA_BIOS_MEASUREMENTS);
				if (!this->bios_list)
				{
					return FAILED;
				}
				this->creation_time = this->bios_list->get_time(this->bios_list);
				this->bios_count = this->bios_list->get_count(this->bios_list);
				this->state = IMA_STATE_BIOS;
				/* fall through to next state */
			case IMA_STATE_BIOS:
				status = this->bios_list->get_next(this->bios_list, &pcr,
											       &measurement);
				if (status != SUCCESS)
				{
					DBG1(DBG_PTS, "could not retrieve bios measurement entry");
					return status;
				}
				evid = extend_pcr(this, qualifier, pcrs, pcr, measurement);

				this->state = this->bios_list->get_count(this->bios_list) ?
										IMA_STATE_BIOS : IMA_STATE_INIT;
				break;
			default:
				return FAILED;
		}
	}
	else if (qualifier == (PTS_ITA_QUALIFIER_FLAG_KERNEL |
						   PTS_ITA_QUALIFIER_TYPE_OS))
	{
		switch (this->state)
		{
			case IMA_STATE_INIT:
				this->ima_list = pts_ima_event_list_create(
												IMA_RUNTIME_MEASUREMENTS);
				if (!this->ima_list)
				{
					return FAILED;
				}
				this->creation_time = this->ima_list->get_time(this->ima_list);
				this->count = this->ima_list->get_count(this->ima_list);
				this->state = IMA_STATE_BOOT_AGGREGATE;
				/* fall through to next state */
			case IMA_STATE_BOOT_AGGREGATE:
			case IMA_STATE_RUNTIME:
				status = this->ima_list->get_next(this->ima_list, &measurement,
												  &algo, &name);
				if (status != SUCCESS)
				{
					DBG1(DBG_PTS, "could not retrieve ima measurement entry");
					return status;
				}
				if (this->state == IMA_STATE_BOOT_AGGREGATE && this->bios_count)
				{
					if (!check_boot_aggregate(pcrs, measurement, algo))
					{
						return FAILED;
					}
				}
				evid = extend_pcr(this, qualifier, pcrs, IMA_PCR,
								  measurement);
				if (evid)
				{
					if (algo)
					{
						algo_len = strlen(algo);
						name_len = strlen(name);
						uri = malloc(algo_len + name_len + 1);
						memcpy(uri, algo, algo_len);
						strcpy(uri + algo_len, name);
					}
					else
					{
						uri = strdup(name);
					}
					evid->set_validation(evid, PTS_COMP_EVID_VALIDATION_PASSED,
											   uri);
					free(uri);
				}
				free(name);
				free(algo);

				this->state = this->ima_list->get_count(this->ima_list) ?
									IMA_STATE_RUNTIME : IMA_STATE_END;
				break;
			default:
				return FAILED;
		}
	}
	else
	{
		DBG1(DBG_PTS, "unsupported functional component name qualifier");
		return FAILED;
	}

	*evidence = evid;
	if (!evid)
	{
		return FAILED;
	}

	return (this->state == IMA_STATE_INIT || this->state == IMA_STATE_END) ?
			SUCCESS : NEED_MORE;
}

/**
 * Parse a validation URI of the form <hash algorithm>:<event name>
 * into its components
 */
static pts_meas_algorithms_t parse_validation_uri(pts_comp_evidence_t *evidence,
								char **ima_name, char **ima_algo, char *algo_buf)
{
    pts_meas_algorithms_t hash_algo;
	char *uri, *pos, *algo, *name;

	evidence->get_validation(evidence, &uri);

	/* IMA-NG format? */
	pos = strchr(uri, ':');
	if (pos && (pos - uri + 1) < IMA_ALGO_LEN_MAX)
	{
		memset(algo_buf, '\0', IMA_ALGO_LEN_MAX);
		memcpy(algo_buf, uri, pos - uri + 1);
		algo = algo_buf;
		name = pos + 1;

		if (streq(algo, "sha1:") || streq(algo, ":"))
		{
			hash_algo = PTS_MEAS_ALGO_SHA1;
		}
		else if (streq(algo, "sha256:"))
		{
			hash_algo = PTS_MEAS_ALGO_SHA256;
		}
		else if (streq(algo, "sha384:"))
		{
			hash_algo = PTS_MEAS_ALGO_SHA384;
		}
		else
		{
			hash_algo = PTS_MEAS_ALGO_NONE;
		}
	}
	else
	{
		algo = NULL;
		name = uri;
		hash_algo = PTS_MEAS_ALGO_SHA1;
	}

	if (ima_name)
	{
		*ima_name = name;
	}
	if (ima_algo)
	{
		*ima_algo = algo;
	}

	return hash_algo;
}

METHOD(pts_component_t, verify, status_t,
	pts_ita_comp_ima_t *this, uint8_t qualifier, pts_t *pts,
	pts_comp_evidence_t *evidence)
{
	bool has_pcr_info;
	uint32_t pcr;
	pts_meas_algorithms_t algo;
	pts_pcr_transform_t transform;
	pts_pcr_t *pcrs;
	time_t creation_time;
	chunk_t measurement, pcr_before, pcr_after;
	status_t status = NOT_FOUND;

	this->aik_id = pts->get_aik_id(pts);
	pcrs = pts->get_pcrs(pts);
	measurement = evidence->get_measurement(evidence, &pcr,	&algo, &transform,
											&creation_time);

	if (qualifier == (PTS_ITA_QUALIFIER_FLAG_KERNEL |
					  PTS_ITA_QUALIFIER_TYPE_TRUSTED))
	{
		switch (this->state)
		{
			case IMA_STATE_INIT:
				this->name->set_qualifier(this->name, qualifier);
				status = this->pts_db->get_comp_measurement_count(this->pts_db,
											this->name, this->aik_id, algo,
											&this->bios_cid, &this->bios_count);
				this->name->set_qualifier(this->name, PTS_QUALIFIER_UNKNOWN);
				if (status != SUCCESS)
				{
					return status;
				}

				if (this->bios_count)
				{
					DBG1(DBG_PTS, "checking %d BIOS evidence measurements",
								   this->bios_count);
				}
				else
				{
					DBG1(DBG_PTS, "registering BIOS evidence measurements");
					this->is_bios_registering = TRUE;
				}

				this->state = IMA_STATE_BIOS;
				/* fall through to next state */
			case IMA_STATE_BIOS:
				if (this->is_bios_registering)
				{
					status = this->pts_db->insert_comp_measurement(this->pts_db,
									measurement, this->bios_cid, this->aik_id,
									++this->seq_no,	pcr, algo);
					if (status != SUCCESS)
					{
						return status;
					}
					this->bios_count = this->seq_no + 1;
				}
				else
				{
					status = this->pts_db->check_comp_measurement(this->pts_db,
									measurement, this->bios_cid, this->aik_id,
									++this->seq_no,	pcr, algo);
					if (status == FAILED)
					{
						return status;
					}
				}
				break;
			default:
				return FAILED;
		}
	}
	else if (qualifier == (PTS_ITA_QUALIFIER_FLAG_KERNEL |
						   PTS_ITA_QUALIFIER_TYPE_OS))
	{
		int ima_count;
		char *ima_algo, *ima_name;
		char algo_buf[IMA_ALGO_LEN_MAX];
		pts_meas_algorithms_t hash_algo;

		hash_algo = parse_validation_uri(evidence, &ima_name, &ima_algo,
										 algo_buf);

		switch (this->state)
		{
			case IMA_STATE_BIOS:
				this->state = IMA_STATE_RUNTIME;

				if (!streq(ima_name, "boot_aggregate"))
				{
					DBG1(DBG_PTS, "ima: name must be 'boot_aggregate' "
								  "but is '%s'", ima_name);
					return FAILED;
				}
				if (hash_algo != PTS_MEAS_ALGO_SHA1)
				{
					DBG1(DBG_PTS, "ima: boot_aggregate algorithm must be %N "
								  "but is %N",
								   pts_meas_algorithm_names, PTS_MEAS_ALGO_SHA1,
								   pts_meas_algorithm_names, hash_algo);
					return FAILED;
				}
				if (!check_boot_aggregate(pcrs, measurement, ima_algo))
				{
					return FAILED;
				}
				this->state = IMA_STATE_INIT;
				/* fall through to next state */
			case IMA_STATE_INIT:
				this->name->set_qualifier(this->name, qualifier);
				status = this->pts_db->get_comp_measurement_count(this->pts_db,
												this->name, this->aik_id, algo,
												&this->ima_cid,	&ima_count);
				this->name->set_qualifier(this->name, PTS_QUALIFIER_UNKNOWN);
				if (status != SUCCESS)
				{
					return status;
				}

				if (ima_count)
				{
					DBG1(DBG_PTS, "checking boot aggregate evidence "
								  "measurement");
					status = this->pts_db->check_comp_measurement(this->pts_db,
													measurement, this->ima_cid,
													this->aik_id, 1, pcr, algo);
				}
				else
				{
					DBG1(DBG_PTS, "registering boot aggregate evidence "
								  "measurement");
					this->is_ima_registering = TRUE;
					status = this->pts_db->insert_comp_measurement(this->pts_db,
													measurement, this->ima_cid,
													this->aik_id, 1, pcr, algo);
				}
				this->state = IMA_STATE_RUNTIME;

				if (status != SUCCESS)
				{
					return status;
				}
				break;
			case IMA_STATE_RUNTIME:
			{
				uint8_t hash_buf[HASH_SIZE_SHA512];
				uint8_t digest_buf[HASH_SIZE_SHA512], *hex_digest_buf;
				chunk_t hex_digest, digest, hash;
				enumerator_t *e;

				this->count++;
				if (evidence->get_validation(evidence, NULL) !=
					PTS_COMP_EVID_VALIDATION_PASSED)
				{
					DBG1(DBG_PTS, "evidence validation failed");
					this->count_failed++;
					return FAILED;
				}
				hash = chunk_create(hash_buf, pts_meas_algo_hash_size(algo));

				e = this->pts_db->create_file_meas_enumerator(this->pts_db,
												pts->get_platform_id(pts),
												hash_algo, ima_name);
				if (e)
				{
					while (e->enumerate(e, &hex_digest_buf))
					{
						hex_digest = chunk_from_str(hex_digest_buf);
						digest = chunk_from_hex(hex_digest, digest_buf);
						if (!ima_hash(digest, ima_algo, ima_name,
									  FALSE, algo, hash_buf))
						{
							status = FAILED;
							break;
						}
						if (chunk_equals_const(measurement, hash))
						{
							status = SUCCESS;
							break;
						}
						else
						{
							status = VERIFY_ERROR;
						}
					}
					e->destroy(e);
				}
				else
				{
					status = FAILED;
				}

				switch (status)
				{
					case SUCCESS:
						DBG3(DBG_PTS, "%#B for '%s' is ok",
									   &measurement, ima_name);
						this->count_ok++;
						break;
					case NOT_FOUND:
						DBG2(DBG_PTS, "%#B for '%s' not found",
									   &measurement, ima_name);
						this->count_unknown++;
						break;
					case VERIFY_ERROR:
						DBG1(DBG_PTS, "%#B for '%s' differs",
									   &measurement, ima_name);
						this->count_differ++;
						break;
					case FAILED:
					default:
						DBG1(DBG_PTS, "%#B for '%s' failed",
									   &measurement, ima_name);
						this->count_failed++;
				}
				break;
			}
			default:
				return FAILED;
		}
	}
	else
	{
		DBG1(DBG_PTS, "unsupported functional component name qualifier");
		return FAILED;
	}

	has_pcr_info = evidence->get_pcr_info(evidence, &pcr_before, &pcr_after);
	if (has_pcr_info)
	{
		if (!chunk_equals_const(pcr_before, pcrs->get(pcrs, pcr)))
		{
			DBG1(DBG_PTS, "PCR %2u: pcr_before is not equal to register value",
						   pcr);
		}
		if (pcrs->set(pcrs, pcr, pcr_after))
		{
			return status;
		}
	}
	else
	{
		pcr_after = pcrs->extend(pcrs, pcr, measurement);
		if (pcr_after.ptr)
		{
			return status;
		}
	}
	return FAILED;
}

METHOD(pts_component_t, finalize, bool,
	pts_ita_comp_ima_t *this, uint8_t qualifier, bio_writer_t *result)
{
	char result_buf[BUF_LEN];
	char *pos = result_buf;
	size_t len = BUF_LEN;
	int written;
	bool success = TRUE;

	this->name->set_qualifier(this->name, qualifier);

	if (qualifier == (PTS_ITA_QUALIFIER_FLAG_KERNEL |
					  PTS_ITA_QUALIFIER_TYPE_TRUSTED))
	{
		/* finalize BIOS measurements */
		if (this->is_bios_registering)
		{
			/* close registration */
			this->is_bios_registering = FALSE;

			snprintf(pos, len, "registered %d BIOS evidence measurements",
					 this->seq_no);
		}
		else if (this->seq_no < this->bios_count)
		{
			snprintf(pos, len, "%d of %d BIOS evidence measurements missing",
					 this->bios_count - this->seq_no, this->bios_count);
			success = FALSE;
		}
		else
		{
			snprintf(pos, len, "%d BIOS evidence measurements are ok",
					 this->bios_count);
		}
	}
	else if (qualifier == (PTS_ITA_QUALIFIER_FLAG_KERNEL |
						   PTS_ITA_QUALIFIER_TYPE_OS))
	{
		/* finalize IMA file measurements */
		if (this->is_ima_registering)
		{
			/* close registration */
			this->is_ima_registering = FALSE;

			written = snprintf(pos, len, "registered IMA boot aggregate "
							   "evidence measurement; ");
			pos += written;
			len -= written;
		}
		if (this->count)
		{
			snprintf(pos, len, "processed %d IMA file evidence measurements: "
					 "%d ok, %d unknown, %d differ, %d failed",
					 this->count, this->count_ok, this->count_unknown,
					 this->count_differ, this->count_failed);
		}
		else
		{
			snprintf(pos, len, "no IMA file evidence measurements");
            success = FALSE;
		}
	}
	else
	{
		snprintf(pos, len, "unsupported functional component name qualifier");
		success = FALSE;
	}
	this->name->set_qualifier(this->name, PTS_QUALIFIER_UNKNOWN);

	DBG1(DBG_PTS, "%s", result_buf);
	result->write_data(result, chunk_from_str(result_buf));

	return success;
}

METHOD(pts_component_t, get_ref, pts_component_t*,
	pts_ita_comp_ima_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

METHOD(pts_component_t, destroy, void,
	pts_ita_comp_ima_t *this)
{
	int count;

	if (ref_put(&this->ref))
	{

		if (this->is_bios_registering)
		{
			count = this->pts_db->delete_comp_measurements(this->pts_db,
												this->bios_cid, this->aik_id);
			DBG1(DBG_PTS, "deleted %d registered BIOS evidence measurements",
						   count);
		}
		if (this->is_ima_registering)
		{
			count = this->pts_db->delete_comp_measurements(this->pts_db,
												this->ima_cid, this->aik_id);
			DBG1(DBG_PTS, "deleted registered boot aggregate evidence "
						  "measurement");
		}
		DESTROY_IF(this->bios_list);
		DESTROY_IF(this->ima_list);
		this->name->destroy(this->name);

		free(this);
	}
}

/**
 * See header
 */
pts_component_t *pts_ita_comp_ima_create(uint32_t depth,
										 pts_database_t *pts_db)
{
	pts_ita_comp_ima_t *this;

	INIT(this,
		.public = {
			.get_comp_func_name = _get_comp_func_name,
			.get_evidence_flags = _get_evidence_flags,
			.get_depth = _get_depth,
			.measure = _measure,
			.verify = _verify,
			.finalize = _finalize,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.name = pts_comp_func_name_create(PEN_ITA, PTS_ITA_COMP_FUNC_NAME_IMA,
										  PTS_QUALIFIER_UNKNOWN),
		.depth = depth,
		.pts_db = pts_db,
		.pcr_info = lib->settings->get_bool(lib->settings,
						"%s.plugins.imc-attestation.pcr_info", FALSE, lib->ns),
		.ref = 1,
	);

	return &this->public;
}
