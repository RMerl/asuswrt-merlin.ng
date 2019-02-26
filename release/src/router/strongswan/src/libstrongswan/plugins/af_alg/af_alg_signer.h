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
 * @defgroup af_alg_signer af_alg_signer
 * @{ @ingroup af_alg
 */

#ifndef AF_ALG_SIGNER_H_
#define AF_ALG_SIGNER_H_

typedef struct af_alg_signer_t af_alg_signer_t;

#include <plugins/plugin.h>
#include <crypto/signers/signer.h>

/** Number of signers */
#define AF_ALG_SIGNER 14

/**
 * Implementation of signers using AF_ALG.
 */
struct af_alg_signer_t {

	/**
	 * Implements signer_t interface.
	 */
	signer_t signer;
};

/**
 * Creates a new af_alg_signer_t.
 *
 * @param algo		algorithm to implement
 * @return			af_alg_signer_t, NULL if  not supported
 */
af_alg_signer_t *af_alg_signer_create(integrity_algorithm_t algo);

/**
 * Probe algorithms and return plugin features.
 *
 * @param features		plugin features to create
 * @param pos			current position in features
 */
void af_alg_signer_probe(plugin_feature_t *features, int *pos);

#endif /** AF_ALG_SIGNER_H_ @}*/
