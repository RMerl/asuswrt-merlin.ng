/*
 * Copyright (C) 2015 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
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

#include "iv_gen.h"
#include "iv_gen_rand.h"
#include "iv_gen_seq.h"
#include "iv_gen_null.h"

/**
 * See header.
 */
iv_gen_t* iv_gen_create_for_alg(encryption_algorithm_t alg)
{
	switch (alg)
	{
		case ENCR_DES:
		case ENCR_3DES:
		case ENCR_RC5:
		case ENCR_IDEA:
		case ENCR_CAST:
		case ENCR_BLOWFISH:
		case ENCR_3IDEA:
		case ENCR_AES_CBC:
		case ENCR_CAMELLIA_CBC:
		case ENCR_SERPENT_CBC:
		case ENCR_TWOFISH_CBC:
		case ENCR_RC2_CBC:
			return iv_gen_rand_create();
		case ENCR_AES_CTR:
		case ENCR_AES_CCM_ICV8:
		case ENCR_AES_CCM_ICV12:
		case ENCR_AES_CCM_ICV16:
		case ENCR_AES_GCM_ICV8:
		case ENCR_AES_GCM_ICV12:
		case ENCR_AES_GCM_ICV16:
		case ENCR_CAMELLIA_CTR:
		case ENCR_CAMELLIA_CCM_ICV8:
		case ENCR_CAMELLIA_CCM_ICV12:
		case ENCR_CAMELLIA_CCM_ICV16:
		case ENCR_CHACHA20_POLY1305:
		case ENCR_NULL_AUTH_AES_GMAC:
			return iv_gen_seq_create();
		case ENCR_NULL:
			return iv_gen_null_create();
		case ENCR_UNDEFINED:
		case ENCR_DES_ECB:
		case ENCR_DES_IV32:
		case ENCR_DES_IV64:
			break;
	}
	return NULL;
}
