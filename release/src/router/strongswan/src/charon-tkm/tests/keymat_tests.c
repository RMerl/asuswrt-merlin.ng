/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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

#include <tests/test_suite.h>

#include <daemon.h>
#include <crypto/proposal/proposal.h>
#include <encoding/payloads/ike_header.h>
#include <tkm/client.h>

#include "tkm.h"
#include "tkm_nonceg.h"
#include "tkm_diffie_hellman.h"
#include "tkm_keymat.h"
#include "tkm_types.h"

START_TEST(test_derive_ike_keys)
{
	proposal_t *proposal = proposal_create_from_string(PROTO_IKE,
			"aes256-sha512-modp4096");
	fail_if(!proposal, "Unable to create proposal");
	ike_sa_id_t *ike_sa_id = ike_sa_id_create(IKEV2_MAJOR_VERSION,
			123912312312, 32312313122, TRUE);
	fail_if(!ike_sa_id, "Unable to create IKE SA ID");

	tkm_keymat_t *keymat = tkm_keymat_create(TRUE);
	fail_if(!keymat, "Unable to create keymat");
	fail_if(!keymat->get_isa_id(keymat), "Invalid ISA context id (0)");

	chunk_t nonce;
	tkm_nonceg_t *ng = tkm_nonceg_create();
	fail_if(!ng, "Unable to create nonce generator");
	fail_unless(ng->nonce_gen.allocate_nonce(&ng->nonce_gen, 32, &nonce),
			"Unable to allocate nonce");

	tkm_diffie_hellman_t *dh = tkm_diffie_hellman_create(MODP_4096_BIT);
	fail_if(!dh, "Unable to create DH");

	/* Use the same pubvalue for both sides */
	chunk_t pubvalue;
	ck_assert(dh->dh.get_my_public_value(&dh->dh, &pubvalue));
	ck_assert(dh->dh.set_other_public_value(&dh->dh, pubvalue));

	fail_unless(keymat->keymat_v2.derive_ike_keys(&keymat->keymat_v2, proposal,
				&dh->dh, nonce, nonce, ike_sa_id, PRF_UNDEFINED, chunk_empty),
				"Key derivation failed");
	chunk_free(&nonce);

	aead_t * const aead = keymat->keymat_v2.keymat.get_aead(&keymat->keymat_v2.keymat, TRUE);
	fail_if(!aead, "AEAD is NULL");

	fail_if(aead->get_key_size(aead) != 96, "Key size mismatch %d",
			aead->get_key_size(aead));
	fail_if(aead->get_block_size(aead) != 16, "Block size mismatch %d",
			aead->get_block_size(aead));

	ng->nonce_gen.destroy(&ng->nonce_gen);
	proposal->destroy(proposal);
	dh->dh.destroy(&dh->dh);
	ike_sa_id->destroy(ike_sa_id);
	keymat->keymat_v2.keymat.destroy(&keymat->keymat_v2.keymat);
	chunk_free(&pubvalue);
}
END_TEST

START_TEST(test_derive_child_keys)
{
	tkm_diffie_hellman_t *dh = tkm_diffie_hellman_create(MODP_4096_BIT);
	fail_if(!dh, "Unable to create DH object");
	proposal_t *proposal = proposal_create_from_string(PROTO_ESP,
			"aes256-sha512-modp4096");
	fail_if(!proposal, "Unable to create proposal");
	proposal->set_spi(proposal, 42);

	tkm_keymat_t *keymat = tkm_keymat_create(TRUE);
	fail_if(!keymat, "Unable to create keymat");

	chunk_t encr_i, encr_r, integ_i, integ_r;
	chunk_t nonce = chunk_from_chars("test chunk");

	fail_unless(keymat->keymat_v2.derive_child_keys(&keymat->keymat_v2, proposal,
													(diffie_hellman_t *)dh,
													nonce, nonce, &encr_i,
													&integ_i, &encr_r, &integ_r),
				"Child key derivation failed");

	esa_info_t *info = (esa_info_t *)encr_i.ptr;
	fail_if(!info, "encr_i does not contain esa information");
	fail_if(info->isa_id != keymat->get_isa_id(keymat),
			"Isa context id mismatch (encr_i)");
	fail_if(info->spi_r != 42,
			"SPI mismatch (encr_i)");
	fail_unless(chunk_equals(info->nonce_i, nonce),
				"nonce_i mismatch (encr_i)");
	fail_unless(chunk_equals(info->nonce_r, nonce),
				"nonce_r mismatch (encr_i)");
	fail_if(info->is_encr_r,
			"Flag is_encr_r set for encr_i");
	fail_if(info->dh_id != dh->get_id(dh),
			"DH context id mismatch (encr_i)");
	chunk_free(&info->nonce_i);
	chunk_free(&info->nonce_r);

	info = (esa_info_t *)encr_r.ptr;
	fail_if(!info, "encr_r does not contain esa information");
	fail_if(info->isa_id != keymat->get_isa_id(keymat),
			"Isa context id mismatch (encr_r)");
	fail_if(info->spi_r != 42,
			"SPI mismatch (encr_r)");
	fail_unless(chunk_equals(info->nonce_i, nonce),
				"nonce_i mismatch (encr_r)");
	fail_unless(chunk_equals(info->nonce_r, nonce),
				"nonce_r mismatch (encr_r)");
	fail_unless(info->is_encr_r,
				"Flag is_encr_r set for encr_r");
	fail_if(info->dh_id != dh->get_id(dh),
			"DH context id mismatch (encr_i)");
	chunk_free(&info->nonce_i);
	chunk_free(&info->nonce_r);

	proposal->destroy(proposal);
	dh->dh.destroy(&dh->dh);
	keymat->keymat_v2.keymat.destroy(&keymat->keymat_v2.keymat);
	chunk_free(&encr_i);
	chunk_free(&encr_r);
}
END_TEST

Suite *make_keymat_tests()
{
	Suite *s;
	TCase *tc;

	s = suite_create("keymat");

	tc = tcase_create("derive IKE keys");
	tcase_add_test(tc, test_derive_ike_keys);
	suite_add_tcase(s, tc);

	tc = tcase_create("derive CHILD keys");
	tcase_add_test(tc, test_derive_child_keys);
	suite_add_tcase(s, tc);

	return s;
}
