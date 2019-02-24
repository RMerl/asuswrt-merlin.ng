/*
 * Copyright (C) 2013-2016 Andreas Steffen
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

#include "ntru_trits.h"
#include "ntru_convert.h"

#include <crypto/xofs/xof_bitspender.h>
#include <utils/debug.h>
#include <utils/test.h>

typedef struct private_ntru_trits_t private_ntru_trits_t;

/**
 * Private data of an ntru_trits_t object.
 */
struct private_ntru_trits_t {

	/**
	 * Public ntru_trits_t interface.
	 */
	ntru_trits_t public;

	/**
	 * Size of the trits array
	 */
	size_t trits_len;

	/**
	 * Array containing a trit per octet
	 */
	uint8_t *trits;

};

METHOD(ntru_trits_t, get_size, size_t,
	private_ntru_trits_t *this)
{
	return this->trits_len;
}

METHOD(ntru_trits_t, get_trits, uint8_t*,
	private_ntru_trits_t *this)
{
	return this->trits;
}

METHOD(ntru_trits_t, destroy, void,
	private_ntru_trits_t *this)
{
	memwipe(this->trits, this->trits_len);
	free(this->trits);
	free(this);
}

/*
 * Described in header.
 */
ntru_trits_t *ntru_trits_create(size_t len, ext_out_function_t alg,
								chunk_t seed)
{
	private_ntru_trits_t *this;
	uint8_t octet, buf[5], *trits;
	size_t trits_needed;
	xof_bitspender_t *bitspender;

	bitspender = xof_bitspender_create(alg, seed, TRUE);
	if (!bitspender)
	{
	    return NULL;
	}

	INIT(this,
		.public = {
			.get_size = _get_size,
			.get_trits = _get_trits,
			.destroy = _destroy,
		},
		.trits_len = len,
		.trits = malloc(len),
	);

	trits = this->trits;
	trits_needed = this->trits_len;

	while (trits_needed > 0)
	{
		if (!bitspender->get_byte(bitspender, &octet))
		{
			bitspender->destroy(bitspender);
			destroy(this);
			return NULL;
		}
		if (octet < 243)  /* 243 = 3^5 */
		{		
			ntru_octet_2_trits(octet, (trits_needed < 5) ? buf : trits);
			if (trits_needed < 5)
			{
				memcpy(trits, buf, trits_needed);
				break;
			}
			trits += 5;
			trits_needed -= 5;
		}
	}
	bitspender->destroy(bitspender);

	return &this->public;
}

EXPORT_FUNCTION_FOR_TESTS(ntru, ntru_trits_create);
