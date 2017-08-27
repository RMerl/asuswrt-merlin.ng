/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup af_alg_crypter af_alg_crypter
 * @{ @ingroup af_alg
 */

#ifndef AF_ALG_CRYPTER_H_
#define AF_ALG_CRYPTER_H_

typedef struct af_alg_crypter_t af_alg_crypter_t;

#include <plugins/plugin.h>
#include <crypto/crypters/crypter.h>

/** Number of crypters */
#define AF_ALG_CRYPTER 25

/**
 * Implementation of signers using AF_ALG.
 */
struct af_alg_crypter_t {

	/**
	 * The crypter_t interface.
	 */
	crypter_t crypter;
};

/**
 * Constructor to create af_alg_crypter_t.
 *
 * @param algo			algorithm to implement
 * @param key_size		key size in bytes
 * @return				af_alg_crypter_t, NULL if not supported
 */
af_alg_crypter_t *af_alg_crypter_create(encryption_algorithm_t algo,
										size_t key_size);

/**
 * Probe algorithms and return plugin features.
 *
 * @param features		plugin features to create
 * @param pos			current position in features
 */
void af_alg_crypter_probe(plugin_feature_t *features, int *pos);

#endif /** AF_ALG_CRYPTER_H_ @}*/
