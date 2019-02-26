/*
 * Copyright (C) 2013 Ruslan Marchenko
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

#include "dnscert.h"

#include <library.h>
#include <utils/debug.h>
#include <bio/bio_reader.h>

typedef struct private_dnscert_t private_dnscert_t;

/**
* private data of the dnscert
*/
struct private_dnscert_t {

	/**
	 * public functions
	 */
	dnscert_t public;

	/**
	 * Certificate type
	 */
	uint16_t cert_type;

	/**
	 * Key tag
	 */
	uint16_t key_tag;

	/**
	 * Algorithm
	 */
	uint8_t algorithm;

	/**
	 * Certificate
	 */
	chunk_t certificate;
};

METHOD(dnscert_t, get_cert_type, dnscert_type_t,
	private_dnscert_t *this)
{
	return this->cert_type;
}

METHOD(dnscert_t, get_key_tag, uint16_t,
	private_dnscert_t *this)
{
	return this->key_tag;
}

METHOD(dnscert_t, get_algorithm, dnscert_algorithm_t,
	private_dnscert_t *this)
{
	return this->algorithm;
}

METHOD(dnscert_t, get_certificate, chunk_t,
	private_dnscert_t *this)
{
	return this->certificate;
}

METHOD(dnscert_t, destroy, void,
	private_dnscert_t *this)
{
	chunk_free(&this->certificate);
	free(this);
}

dnscert_t *dnscert_create_frm_rr(rr_t *rr)
{
	private_dnscert_t *this;
	bio_reader_t *reader = NULL;

	INIT(this,
			.public = {
				.get_cert_type = _get_cert_type,
				.get_key_tag  = _get_key_tag,
				.get_algorithm = _get_algorithm,
				.get_certificate = _get_certificate,
				.destroy = _destroy,
			},
	);

	if (rr->get_type(rr) != RR_TYPE_CERT)
	{
		DBG1(DBG_CFG, "unable to create a dnscert out of an RR "
					  "whose type is not CERT");
		free(this);
		return NULL;
	}

	/**
	 * Parse the content (RDATA field) of the RR
	 * First - type/tag/algo fields and then cert body
	 */
	reader = bio_reader_create(rr->get_rdata(rr));
	if (!reader->read_uint16(reader, &this->cert_type) ||
		!reader->read_uint16(reader, &this->key_tag) ||
		!reader->read_uint8(reader, &this->algorithm) )
	{
		DBG1(DBG_CFG, "CERT RR has a wrong format");
		reader->destroy(reader);
		free(this);
		return NULL;
	}

	if (!reader->read_data(reader, reader->remaining(reader),
						   &this->certificate))
	{
		DBG1(DBG_CFG, "failed to read DNS certificate field");
		reader->destroy(reader);
		free(this);
		return NULL;
	}
	this->certificate = chunk_clone(this->certificate);
	reader->destroy(reader);
	return &this->public;
}
