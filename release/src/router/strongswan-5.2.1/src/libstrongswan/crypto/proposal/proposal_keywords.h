/*
 * Copyright (C) 2012 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
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

/*
 * Copyright (c) 2012 Nanoteq Pty Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @defgroup proposal_keywords proposal_keywords
 * @{ @ingroup crypto
 */

#ifndef PROPOSAL_KEYWORDS_H_
#define PROPOSAL_KEYWORDS_H_

typedef struct proposal_token_t proposal_token_t;
typedef struct proposal_keywords_t proposal_keywords_t;

typedef proposal_token_t*(*proposal_algname_parser_t)(const char *algname);

#include <library.h>
#include <crypto/transform.h>

/**
 * Class representing a proposal token.
 */
struct proposal_token_t {

	/**
	 * The name of the token.
	 */
	char *name;

	/**
	 * The type of transform in the token.
	 */
	transform_type_t type;

	/**
	 * The IKE id of the algorithm.
	 */
	u_int16_t algorithm;

	/**
	 * The key size associated with the specific algorithm.
	 */
	u_int16_t keysize;
};

/**
 * Class to manage proposal keywords
 */
struct proposal_keywords_t {

	/**
	 * Returns the proposal token for the specified string if a token exists.
	 *
	 * @param str		the string containing the name of the token
	 * @return			proposal_token if found, NULL otherwise
	 */
	const proposal_token_t *(*get_token)(proposal_keywords_t *this,
										 const char *str);

	/**
	 * Register a new proposal token for an algorithm.
	 *
	 * @param name		the string containing the name of the token
	 * @param type		the transform_type_t for the token
	 * @param algorithm	the IKE id of the algorithm
	 * @param keysize	the key size associated with the specific algorithm
	 */
	void (*register_token)(proposal_keywords_t *this, const char *name,
						   transform_type_t type, u_int16_t algorithm,
						   u_int16_t keysize);

	/**
	 * Register an algorithm name parser.
	 *
	 * It is meant to parse an algorithm name into a proposal token in a
	 * generic, user defined way.
	 *
	 * @param parser	a pointer to the parser function
	 */
	void (*register_algname_parser)(proposal_keywords_t *this,
									proposal_algname_parser_t parser);

	/**
	 * Destroy a proposal_keywords_t instance.
	 */
	void (*destroy)(proposal_keywords_t *this);
};

/**
 * Create a proposal_keywords_t instance.
 */
proposal_keywords_t *proposal_keywords_create();

#endif /** PROPOSAL_KEYWORDS_H_ @}*/
