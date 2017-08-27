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

#include "proposal_keywords.h"
#include "proposal_keywords_static.h"

#include <collections/linked_list.h>
#include <threading/rwlock.h>

typedef struct private_proposal_keywords_t private_proposal_keywords_t;

struct private_proposal_keywords_t {

	/**
	 * public interface
	 */
	proposal_keywords_t public;

	/**
	 * registered tokens, as proposal_token_t
	 */
	linked_list_t * tokens;

	/**
	 * registered algname parsers, as proposal_algname_parser_t
	 */
	linked_list_t *parsers;

	/**
	 * rwlock to lock access to modules
	 */
	rwlock_t *lock;
};

/**
 * Find the token object for the algorithm specified.
 */
static const proposal_token_t* find_token(private_proposal_keywords_t *this,
										  const char *str)
{
	proposal_token_t *token, *found = NULL;
	enumerator_t *enumerator;

	this->lock->read_lock(this->lock);
	enumerator = this->tokens->create_enumerator(this->tokens);
	while (enumerator->enumerate(enumerator, &token))
	{
		if (streq(token->name, str))
		{
			found = token;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return found;
}

/**
 * Parse the given algorithm into a token with user defined parser functions.
 */
static const proposal_token_t* parse_token(private_proposal_keywords_t *this,
										   const char *str)
{
	proposal_algname_parser_t parser;
	enumerator_t *enumerator;
	proposal_token_t *found = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->parsers->create_enumerator(this->parsers);
	while (enumerator->enumerate(enumerator, &parser))
	{
		found = parser(str);
		if (found)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return found;
}

METHOD(proposal_keywords_t, get_token, const proposal_token_t*,
	private_proposal_keywords_t *this, const char *str)
{
	const proposal_token_t *token;

	token = proposal_get_token_static(str, strlen(str));
	if (!token)
	{
		token = find_token(this, str);
	}
	if (!token)
	{
		token = parse_token(this, str);
	}
	return token;
}

METHOD(proposal_keywords_t, register_token, void,
	private_proposal_keywords_t *this, const char *name, transform_type_t type,
	u_int16_t algorithm, u_int16_t keysize)
{
	proposal_token_t *token;

	INIT(token,
		.name = strdup(name),
		.type = type,
		.algorithm = algorithm,
		.keysize = keysize,
	);

	this->lock->write_lock(this->lock);
	this->tokens->insert_first(this->tokens, token);
	this->lock->unlock(this->lock);
}

METHOD(proposal_keywords_t, register_algname_parser, void,
	private_proposal_keywords_t *this, proposal_algname_parser_t parser)
{
	this->lock->write_lock(this->lock);
	this->tokens->insert_first(this->parsers, parser);
	this->lock->unlock(this->lock);
}

METHOD(proposal_keywords_t, destroy, void,
	private_proposal_keywords_t *this)
{
	proposal_token_t *token;

	while (this->tokens->remove_first(this->tokens, (void**)&token) == SUCCESS)
	{
		free(token->name);
		free(token);
	}
	this->tokens->destroy(this->tokens);
	this->parsers->destroy(this->parsers);
	this->lock->destroy(this->lock);
	free(this);
}

/*
 * Described in header.
 */
proposal_keywords_t *proposal_keywords_create()
{
	private_proposal_keywords_t *this;

	INIT(this,
		.public = {
			.get_token = _get_token,
			.register_token = _register_token,
			.register_algname_parser = _register_algname_parser,
			.destroy = _destroy,
		},
		.tokens = linked_list_create(),
		.parsers = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}
