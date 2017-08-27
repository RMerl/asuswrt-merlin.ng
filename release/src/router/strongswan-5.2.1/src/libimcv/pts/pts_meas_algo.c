/*
 * Copyright (C) 2011-2014 Andreas Steffen
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

#include "pts_meas_algo.h"

#include <utils/debug.h>

ENUM_BEGIN(pts_meas_algorithm_names, PTS_MEAS_ALGO_NONE, PTS_MEAS_ALGO_NONE,
	"None");
ENUM_NEXT(pts_meas_algorithm_names,	PTS_MEAS_ALGO_SHA384, PTS_MEAS_ALGO_SHA384,
									PTS_MEAS_ALGO_NONE,
	"SHA384");
ENUM_NEXT(pts_meas_algorithm_names,	PTS_MEAS_ALGO_SHA256, PTS_MEAS_ALGO_SHA256,
									PTS_MEAS_ALGO_SHA384,
	"SHA256");
ENUM_NEXT(pts_meas_algorithm_names,	PTS_MEAS_ALGO_SHA1, PTS_MEAS_ALGO_SHA1,
									PTS_MEAS_ALGO_SHA256,
	"SHA1");
ENUM_END(pts_meas_algorithm_names,  PTS_MEAS_ALGO_SHA1);

/**
 * Described in header.
 */
bool pts_meas_algo_probe(pts_meas_algorithms_t *algorithms)
{
	enumerator_t *enumerator;
	hash_algorithm_t hash_alg;
	const char *plugin_name;
	char format1[] = "  %s PTS measurement algorithm %N[%s] available";
	char format2[] = "  %s PTS measurement algorithm %N not available";

	*algorithms = 0;

	enumerator = lib->crypto->create_hasher_enumerator(lib->crypto);
	while (enumerator->enumerate(enumerator, &hash_alg, &plugin_name))
	{
		if (hash_alg == HASH_SHA1)
		{
			*algorithms |= PTS_MEAS_ALGO_SHA1;
			DBG2(DBG_PTS, format1, "mandatory", hash_algorithm_names, hash_alg,
								  plugin_name);
		}
		else if (hash_alg == HASH_SHA256)
		{
			*algorithms |= PTS_MEAS_ALGO_SHA256;
			DBG2(DBG_PTS, format1, "mandatory", hash_algorithm_names, hash_alg,
								  plugin_name);
		}
		else if (hash_alg == HASH_SHA384)
		{
			*algorithms |= PTS_MEAS_ALGO_SHA384;
			DBG2(DBG_PTS, format1, "optional ", hash_algorithm_names, hash_alg,
								  plugin_name);
		}
	}
	enumerator->destroy(enumerator);

	if (!(*algorithms & PTS_MEAS_ALGO_SHA384))
	{
		DBG1(DBG_PTS, format2, "optional ", hash_algorithm_names, HASH_SHA384);
	}
	if ((*algorithms & PTS_MEAS_ALGO_SHA1) &&
		(*algorithms & PTS_MEAS_ALGO_SHA256))
	{
		return TRUE;
	}
	if (!(*algorithms & PTS_MEAS_ALGO_SHA1))
	{
		DBG1(DBG_PTS, format2, "mandatory", hash_algorithm_names, HASH_SHA1);
	}
	if (!(*algorithms & PTS_MEAS_ALGO_SHA256))
	{
		DBG1(DBG_PTS, format2, "mandatory", hash_algorithm_names, HASH_SHA256);
	}
	return FALSE;
}

/**
 * Described in header.
 */
bool pts_meas_algo_update(char *hash_alg, pts_meas_algorithms_t *algorithms)
{
	if (strcaseeq(hash_alg, "sha384") || strcaseeq(hash_alg, "sha2_384"))
	{
		/* nothing to update, all algorithms are supported */
		return TRUE;
	}
	if (strcaseeq(hash_alg, "sha256") || strcaseeq(hash_alg, "sha2_256"))
	{
		/* remove SHA384algorithm */
		*algorithms &= ~PTS_MEAS_ALGO_SHA384;
		return TRUE;
	}
	if (strcaseeq(hash_alg, "sha1"))
	{
		/* remove SHA384 and SHA256 algorithms */
		*algorithms &= ~(PTS_MEAS_ALGO_SHA384 | PTS_MEAS_ALGO_SHA256);
		return TRUE;
	}
	DBG1(DBG_PTS, "unknown hash algorithm '%s' configured", hash_alg);
	return FALSE;
}

/**
 * Described in header.
 */
pts_meas_algorithms_t pts_meas_algo_select(pts_meas_algorithms_t supported_algos,
										   pts_meas_algorithms_t offered_algos)
{
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
		default:
			return HASH_UNKNOWN;
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
		case PTS_MEAS_ALGO_NONE:
		default:
			return 0;
	}
}

