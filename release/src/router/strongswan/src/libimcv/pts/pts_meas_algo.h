/*
 * Copyright (C) 2011 Sansar Choinyambuu
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

/**
 * @defgroup pts_meas_algo pts_meas_algo
 * @{ @ingroup pts
 */

#ifndef PTS_MEAS_ALGO_H_
#define PTS_MEAS_ALGO_H_

#include <library.h>
#include <crypto/hashers/hasher.h>

typedef enum pts_meas_algorithms_t pts_meas_algorithms_t;

/**
 * PTS Measurement Algorithms
 */
enum pts_meas_algorithms_t {
	PTS_MEAS_ALGO_NONE     =      0,
	PTS_MEAS_ALGO_SHA384   = (1<<13),
	PTS_MEAS_ALGO_SHA256   = (1<<14),
	PTS_MEAS_ALGO_SHA1     = (1<<15)
};

/**
 * enum name for pts_meas_algorithms_t.
 */
extern enum_name_t *pts_meas_algorithm_names;

/**
 * Diffie-Hellman Hash Algorithm Values
 * see section 3.8.5 of PTS Protocol: Binding to TNC IF-M Specification
 *
 *					   1
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |1|2|3|R|R|R|R|R|R|R|R|R|R|R|R|R|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  
 */

/**
 * Probe available PTS measurement algorithms
 *
 * @param algorithms	set of available algorithms
 * @return				TRUE if mandatory algorithms are available
 */
bool pts_meas_algo_probe(pts_meas_algorithms_t *algorithms);

/**
 * Update supported PTS measurement algorithms according to configuration
 *
 * sha1 :  PTS_MEAS_ALGO_SHA1
 * sha256: PTS_MEAS_ALGO_SHA1 | PTS_MEAS_ALGO_SHA256
 * sha384: PTS_MEAS_ALGO_SHA1 | PTS_MEAS_ALGO_SHA256 | PTS_MEAS_ALGO_SHA384
 *
 * The PTS-IMC is expected to select the strongest supported algorithm
 *
 * @param hash_alg		configured hash algorithm
 * @param algorithms	returns set of available PTS measurement algorithms
 */
bool pts_meas_algo_update(char *hash_alg, pts_meas_algorithms_t *algorithms);

/**
 * Select the strongest PTS measurement algorithm
 * among a set of offered PTS measurement algorithms
 *
 * @param supported_algos	set of supported PTS measurement algorithms
 * @param offered_algos		set of offered PTS measurements algorithms
 * @return					selected algorithm
 */
pts_meas_algorithms_t pts_meas_algo_select(pts_meas_algorithms_t supported_algos,
										   pts_meas_algorithms_t offered_algos);

/**
 * Convert pts_meas_algorithms_t to hash_algorithm_t
 *
 * @param algorithm		PTS measurement algorithm type
 * @return				libstrongswan hash algorithm type
 */
hash_algorithm_t pts_meas_algo_to_hash(pts_meas_algorithms_t algorithm);

/**
 * Convert hash_algorithm_t to pts_meas_algorithms_t
 *
 * @param algorithm		PTS measurement algorithm type
 * @return				libstrongswan hash algorithm type
 */
pts_meas_algorithms_t pts_meas_algo_from_hash(hash_algorithm_t algorithm);

/**
 * Return the hash size of a pts_meas_algorithm
 *
 * @param algorithm		PTS measurement algorithm type
 * @return				hash size in bytes
 */
size_t pts_meas_algo_hash_size(pts_meas_algorithms_t algorithm);

#endif /** PTS_MEAS_ALGO_H_ @}*/
