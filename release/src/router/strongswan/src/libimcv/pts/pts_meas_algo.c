/*
 * Copyright (C) 2011-2014 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
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

#include "pts_meas_algo.h"

#include <utils/debug.h>

ENUM_BEGIN(pts_meas_algorithm_names, PTS_MEAS_ALGO_NONE, PTS_MEAS_ALGO_NONE,
	"None");
ENUM_NEXT(pts_meas_algorithm_names,	PTS_MEAS_ALGO_SHA512, PTS_MEAS_ALGO_SHA512,
									PTS_MEAS_ALGO_NONE,
	"SHA512");
ENUM_NEXT(pts_meas_algorithm_names,	PTS_MEAS_ALGO_SHA384, PTS_MEAS_ALGO_SHA384,
									PTS_MEAS_ALGO_SHA512,
	"SHA384");
ENUM_NEXT(pts_meas_algorithm_names,	PTS_MEAS_ALGO_SHA256, PTS_MEAS_ALGO_SHA256,
									PTS_MEAS_ALGO_SHA384,
	"SHA256");
ENUM_NEXT(pts_meas_algorithm_names,	PTS_MEAS_ALGO_SHA1, PTS_MEAS_ALGO_SHA1,
									PTS_MEAS_ALGO_SHA256,
	"SHA1");
ENUM_END(pts_meas_algorithm_names,  PTS_MEAS_ALGO_SHA1);

#define ALG_UNAVAIL "  %s PTS measurement algorithm %N not available"

/**
 * Described in header.
 */
bool pts_meas_algo_probe(pts_meas_algorithms_t *algorithms)
{
	enumerator_t *enumerator;
	hash_algorithm_t hash_alg;
	const char *plugin_name;

	*algorithms = 0;

	enumerator = lib->crypto->create_hasher_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &hash_alg, &plugin_name))
	{
		pts_meas_algorithms_t mapped = 0;
		bool mandatory DBG_UNUSED = FALSE;

		switch (hash_alg)
		{
			case HASH_SHA1:
				mapped = PTS_MEAS_ALGO_SHA1;
				mandatory = TRUE;
				break;
			case HASH_SHA256:
				mapped = PTS_MEAS_ALGO_SHA256;
				mandatory = TRUE;
				break;
			case HASH_SHA384:
				mapped = PTS_MEAS_ALGO_SHA384;
				break;
			case HASH_SHA512:
				mapped = PTS_MEAS_ALGO_SHA512;
				break;
			default:
				break;
		}
		if (mapped)
		{
			*algorithms |= mapped;
			DBG2(DBG_PTS, "  %s PTS measurement algorithm %N[%s] available",
				 mandatory ? "mandatory" : "optional ", hash_algorithm_names,
				 hash_alg, plugin_name);
		}
	}
	enumerator->destroy(enumerator);

	if (!(*algorithms & PTS_MEAS_ALGO_SHA512))
	{
		DBG1(DBG_PTS, ALG_UNAVAIL, "optional ", hash_algorithm_names, HASH_SHA512);
	}
	if (!(*algorithms & PTS_MEAS_ALGO_SHA384))
	{
		DBG1(DBG_PTS, ALG_UNAVAIL, "optional ", hash_algorithm_names, HASH_SHA384);
	}
	if ((*algorithms & PTS_MEAS_ALGO_SHA1) &&
		(*algorithms & PTS_MEAS_ALGO_SHA256))
	{
		return TRUE;
	}
	if (!(*algorithms & PTS_MEAS_ALGO_SHA256))
	{
		DBG1(DBG_PTS, ALG_UNAVAIL, "mandatory", hash_algorithm_names, HASH_SHA256);
	}
	if (!(*algorithms & PTS_MEAS_ALGO_SHA1))
	{
		DBG1(DBG_PTS, ALG_UNAVAIL, "mandatory", hash_algorithm_names, HASH_SHA1);
	}
	return FALSE;
}

/**
 * Described in header.
 */
bool pts_meas_algo_update(char *hash_alg, pts_meas_algorithms_t *algorithms)
{
	if (strcaseeq(hash_alg, "sha512") || strcaseeq(hash_alg, "sha2_512"))
	{
		/* nothing to update, all algorithms are supported */
		return TRUE;
	}
	if (strcaseeq(hash_alg, "sha384") || strcaseeq(hash_alg, "sha2_384"))
	{
		/* remove SHA512 algorithm */
		*algorithms &= ~PTS_MEAS_ALGO_SHA512;
		return TRUE;
	}
	if (strcaseeq(hash_alg, "sha256") || strcaseeq(hash_alg, "sha2_256"))
	{
		/* remove SHA512 and SHA384 algorithms */
		*algorithms &= ~(PTS_MEAS_ALGO_SHA512 | PTS_MEAS_ALGO_SHA384);
		return TRUE;
	}
	if (strcaseeq(hash_alg, "sha1"))
	{
		/* remove SHA512, SHA384 and SHA256 algorithms */
		*algorithms &= ~(PTS_MEAS_ALGO_SHA512 | PTS_MEAS_ALGO_SHA384 |
						 PTS_MEAS_ALGO_SHA256);
		return TRUE;
	}
	DBG1(DBG_PTS, "unknown hash algorithm '%s' configured", hash_alg);
	return FALSE;
}

/**
 * Described in header.
 */
void pts_meas_algo_with_pcr(tpm_tss_t *tpm, pts_meas_algorithms_t *algorithms)
{
	pts_meas_algorithms_t algo_set[] = { PTS_MEAS_ALGO_SHA1,
										PTS_MEAS_ALGO_SHA256,
										PTS_MEAS_ALGO_SHA384,
										PTS_MEAS_ALGO_SHA512
									  };
	int i;

	for (i = 0; i < countof(algo_set); i++)
	{
		if (!tpm->has_pcr_bank(tpm, pts_meas_algo_to_hash(algo_set[i])))
		{
			/* remove algorithm */
			*algorithms &= ~algo_set[i];
		}
	}
}

/**
 * Described in header.
 */
pts_meas_algorithms_t pts_meas_algo_select(pts_meas_algorithms_t supported_algos,
										   pts_meas_algorithms_t offered_algos)
{
	if ((supported_algos & PTS_MEAS_ALGO_SHA512) &&
		(offered_algos   & PTS_MEAS_ALGO_SHA512))
	{
		return PTS_MEAS_ALGO_SHA512;
	}
	if ((supported_algos & PTS_MEAS_ALGO_SHA384) &&
		(offered_algos   & PTS_MEAS_ALGO_SHA384))
	{
		return PTS_MEAS_ALGO_SHA384;
	}
	if ((supported_algos & PTS_MEAS_ALGO_SHA256) &&
		(offered_algos   & PTS_MEAS_ALGO_SHA256))
	{
		return PTS_MEAS_ALGO_SHA256;
	}
	if ((supported_algos & PTS_MEAS_ALGO_SHA1) &&
		(offered_algos   & PTS_MEAS_ALGO_SHA1))
	{
		return PTS_MEAS_ALGO_SHA1;
	}
	return PTS_MEAS_ALGO_NONE;
}

/**
 * Described in header.
 */
hash_algorithm_t pts_meas_algo_to_hash(pts_meas_algorithms_t algorithm)
{
	switch (algorithm)
	{
		case PTS_MEAS_ALGO_SHA1:
			return HASH_SHA1;
		case PTS_MEAS_ALGO_SHA256:
			return HASH_SHA256;
		case PTS_MEAS_ALGO_SHA384:
			return HASH_SHA384;
		case PTS_MEAS_ALGO_SHA512:
			return HASH_SHA512;
		default:
			return HASH_UNKNOWN;
	}
}

/**
 * Described in header.
 */
pts_meas_algorithms_t pts_meas_algo_from_hash(hash_algorithm_t algorithm)
{
	switch (algorithm)
	{
		case HASH_SHA1:
			return PTS_MEAS_ALGO_SHA1;
		case HASH_SHA256:
			return PTS_MEAS_ALGO_SHA256;
		case HASH_SHA384:
			return PTS_MEAS_ALGO_SHA384;
		case HASH_SHA512:
			return PTS_MEAS_ALGO_SHA512;
		default:
			return PTS_MEAS_ALGO_NONE;
	}
}

/**
 * Described in header.
 */
size_t pts_meas_algo_hash_size(pts_meas_algorithms_t algorithm)
{
	switch (algorithm)
	{
		case PTS_MEAS_ALGO_SHA1:
			return HASH_SIZE_SHA1;
		case PTS_MEAS_ALGO_SHA256:
			return HASH_SIZE_SHA256;
		case PTS_MEAS_ALGO_SHA384:
			return HASH_SIZE_SHA384;
		case PTS_MEAS_ALGO_SHA512:
			return HASH_SIZE_SHA512;
		case PTS_MEAS_ALGO_NONE:
		default:
			return 0;
	}
}
