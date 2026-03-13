/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
 *
 * Copyright (C) secunet Security Networks AG
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
#include "tkm_key_exchange.h"
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

	tkm_key_exchange_t *ke = tkm_key_exchange_create(MODP_4096_BIT);
	fail_if(!ke, "Unable to create KE");

	/* Use the same pubvalue for both sides */
	chunk_t pubvalue;
	ck_assert(ke->ke.get_public_key(&ke->ke, &pubvalue));
	ck_assert(ke->ke.set_public_key(&ke->ke, pubvalue));

	array_t *kes = NULL;
	array_insert_create(&kes, ARRAY_TAIL, ke);
	fail_unless(keymat->keymat_v2.derive_ike_keys(&keymat->keymat_v2, proposal,
				kes, nonce, nonce, ike_sa_id, PRF_UNDEFINED, chunk_empty),
				"Key derivation failed");
	array_destroy(kes);
	chunk_free(&nonce);

	aead_t * const aead = keymat->keymat_v2.keymat.get_aead(&keymat->keymat_v2.keymat, TRUE);
	fail_if(!aead, "AEAD is NULL");

	fail_if(aead->get_key_size(aead) != 1, "Key size mismatch %d",
			aead->get_key_size(aead));
	fail_if(aead->get_block_size(aead) != 16, "Block size mismatch %d",
			aead->get_block_size(aead));

	ng->nonce_gen.destroy(&ng->nonce_gen);
	proposal->destroy(proposal);
	ke->ke.destroy(&ke->ke);
	ike_sa_id->destroy(ike_sa_id);
	keymat->keymat_v2.keymat.destroy(&keymat->keymat_v2.keymat);
	chunk_free(&pubvalue);
}
END_TEST

START_TEST(test_derive_ike_keys_multi_ke)
{
	proposal_t *proposal = proposal_create_from_string(PROTO_IKE,
			"aes256-sha512-modp3072-ke1_modp4096");
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

	tkm_key_exchange_t *ke = tkm_key_exchange_create(MODP_3072_BIT);
	fail_if(!ke, "Unable to create first KE");

	/* Use the same pubvalue for both sides */
	chunk_t pubvalue;
	ck_assert(ke->ke.get_public_key(&ke->ke, &pubvalue));
	ck_assert(ke->ke.set_public_key(&ke->ke, pubvalue));
	chunk_free(&pubvalue);

	array_t *kes = NULL;
	array_insert_create(&kes, ARRAY_TAIL, ke);
	fail_unless(keymat->keymat_v2.derive_ike_keys(&keymat->keymat_v2, proposal,
				kes, nonce, nonce, ike_sa_id, PRF_UNDEFINED, chunk_empty),
				"Key derivation failed");
	array_destroy(kes);
	ke->ke.destroy(&ke->ke);

	const aead_t *aead = keymat->keymat_v2.keymat.get_aead(&keymat->keymat_v2.keymat, TRUE);
	fail_if(!aead, "AEAD is NULL");

	/* single KE during IKE_INTERMEDIATE on the same keymat with same nonces */
	pseudo_random_function_t prf;
	chunk_t skd;
	prf = keymat->keymat_v2.get_skd(&keymat->keymat_v2, &skd);
	fail_if(prf != PRF_HMAC_SHA2_512, "PRF incorrect");

	ke = tkm_key_exchange_create(MODP_4096_BIT);
	fail_if(!ke, "Unable to create second KE");
	ck_assert(ke->ke.get_public_key(&ke->ke, &pubvalue));
	ck_assert(ke->ke.set_public_key(&ke->ke, pubvalue));
	chunk_free(&pubvalue);

	kes = NULL;
	array_insert_create(&kes, ARRAY_TAIL, ke);
	fail_unless(keymat->keymat_v2.derive_ike_keys(&keymat->keymat_v2, proposal,
				kes, nonce, nonce, ike_sa_id, prf, skd),
				"Second key derivation failed");
	array_destroy(kes);
	ke->ke.destroy(&ke->ke);
	chunk_free(&nonce);

	aead = keymat->keymat_v2.keymat.get_aead(&keymat->keymat_v2.keymat, TRUE);
	fail_if(!aead, "AEAD is NULL");
	ng->nonce_gen.destroy(&ng->nonce_gen);
	ike_sa_id->destroy(ike_sa_id);

	/* rekeying uses a new keymat/SA/nonce and multiple KEs */
	ike_sa_id = ike_sa_id_create(IKEV2_MAJOR_VERSION,
			34912312312, 612313122, TRUE);
	fail_if(!ike_sa_id, "Unable to create IKE SA ID");

	tkm_keymat_t *keymat2 = tkm_keymat_create(TRUE);
	fail_if(!keymat2, "Unable to create keymat");
	fail_if(!keymat2->get_isa_id(keymat2), "Invalid ISA context id (0)");

	ng = tkm_nonceg_create();
	fail_if(!ng, "Unable to create nonce generator");
	fail_unless(ng->nonce_gen.allocate_nonce(&ng->nonce_gen, 32, &nonce),
			"Unable to allocate nonce");

	tkm_key_exchange_t *ke1 = tkm_key_exchange_create(MODP_3072_BIT);
	fail_if(!ke1, "Unable to create first KE");
	ck_assert(ke1->ke.get_public_key(&ke1->ke, &pubvalue));
	ck_assert(ke1->ke.set_public_key(&ke1->ke, pubvalue));
	chunk_free(&pubvalue);
	tkm_key_exchange_t *ke2 = tkm_key_exchange_create(MODP_4096_BIT);
	fail_if(!ke2, "Unable to create second KE");
	ck_assert(ke2->ke.get_public_key(&ke2->ke, &pubvalue));
	ck_assert(ke2->ke.set_public_key(&ke2->ke, pubvalue));
	chunk_free(&pubvalue);

	prf = keymat->keymat_v2.get_skd(&keymat->keymat_v2, &skd);
	fail_if(prf != PRF_HMAC_SHA2_512, "PRF incorrect");

	kes = NULL;
	array_insert_create(&kes, ARRAY_TAIL, ke1);
	array_insert_create(&kes, ARRAY_TAIL, ke2);
	fail_unless(keymat2->keymat_v2.derive_ike_keys(&keymat2->keymat_v2, proposal,
				kes, nonce, nonce, ike_sa_id, prf, skd),
				"Rekey key derivation failed");
	array_destroy(kes);
	ke1->ke.destroy(&ke1->ke);
	ke2->ke.destroy(&ke2->ke);
	chunk_free(&nonce);

	aead = keymat2->keymat_v2.keymat.get_aead(&keymat2->keymat_v2.keymat, TRUE);
	fail_if(!aead, "AEAD is NULL");

	ng->nonce_gen.destroy(&ng->nonce_gen);
	proposal->destroy(proposal);
	ike_sa_id->destroy(ike_sa_id);
	keymat->keymat_v2.keymat.destroy(&keymat->keymat_v2.keymat);
	keymat2->keymat_v2.keymat.destroy(&keymat2->keymat_v2.keymat);
}
END_TEST

START_TEST(test_derive_child_keys)
{
	tkm_key_exchange_t *ke = tkm_key_exchange_create(MODP_4096_BIT);
	fail_if(!ke, "Unable to create DH object");
	proposal_t *proposal = proposal_create_from_string(PROTO_ESP,
			"aes256-sha512-modp4096");
	fail_if(!proposal, "Unable to create proposal");
	proposal->set_spi(proposal, 42);

	tkm_keymat_t *keymat = tkm_keymat_create(TRUE);
	fail_if(!keymat, "Unable to create keymat");

	chunk_t encr_i, encr_r, integ_i, integ_r;
	chunk_t nonce_i = chunk_from_chars("test chunk 1"),
			nonce_r = chunk_from_chars("test chunk 2");

	array_t *kes = NULL;
	array_insert_create(&kes, ARRAY_TAIL, ke);
	fail_unless(keymat->keymat_v2.derive_child_keys(&keymat->keymat_v2, proposal,
													kes, nonce_i, nonce_r, &encr_i,
													&integ_i, &encr_r, &integ_r),
				"Child key derivation failed");
	array_destroy(kes);

	esa_info_t *info = (esa_info_t *)encr_i.ptr;
	fail_if(!info, "encr_i does not contain esa information");
	fail_if(info->isa_id != keymat->get_isa_id(keymat),
			"Isa context id mismatch (encr_i)");
	fail_if(info->spi_l != 42,
			"SPI mismatch (encr_i)");
	fail_unless(chunk_equals(info->nonce_i, nonce_i),
				"nonce_i mismatch (encr_i)");
	fail_unless(chunk_equals(info->nonce_r, nonce_r),
				"nonce_r mismatch (encr_i)");
	fail_if(info->is_encr_r,
			"Flag is_encr_r set for encr_i");
	fail_if(info->ke_ids.size != 1,
			"KE context number mismatch (encr_i)");
	fail_if(info->ke_ids.data[0] != ke->get_id(ke),
			"KE context id mismatch (encr_i)");
	chunk_free(&info->nonce_i);
	chunk_free(&info->nonce_r);

	info = (esa_info_t *)encr_r.ptr;
	fail_if(!info, "encr_r does not contain esa information");
	fail_if(info->isa_id != keymat->get_isa_id(keymat),
			"Isa context id mismatch (encr_r)");
	fail_if(info->spi_l != 42,
			"SPI mismatch (encr_r)");
	fail_unless(chunk_equals(info->nonce_i, nonce_i),
				"nonce_i mismatch (encr_r)");
	fail_unless(chunk_equals(info->nonce_r, nonce_r),
				"nonce_r mismatch (encr_r)");
	fail_unless(info->is_encr_r,
				"Flag is_encr_r set for encr_r");
	fail_if(info->ke_ids.size != 1,
			"KE context number mismatch (encr_i)");
	fail_if(info->ke_ids.data[0] != ke->get_id(ke),
			"KE context id mismatch (encr_i)");
	chunk_free(&info->nonce_i);
	chunk_free(&info->nonce_r);

	proposal->destroy(proposal);
	ke->ke.destroy(&ke->ke);
	keymat->keymat_v2.keymat.destroy(&keymat->keymat_v2.keymat);
	chunk_free(&encr_i);
	chunk_free(&encr_r);
}
END_TEST

START_TEST(test_derive_child_keys_multi_ke)
{
	tkm_key_exchange_t *ke1 = tkm_key_exchange_create(MODP_3072_BIT);
	fail_if(!ke1, "Unable to create DH object");
	tkm_key_exchange_t *ke2 = tkm_key_exchange_create(MODP_4096_BIT);
	fail_if(!ke2, "Unable to create DH object");
	proposal_t *proposal = proposal_create_from_string(PROTO_ESP,
			"aes256-sha512-modp4096");
	fail_if(!proposal, "Unable to create proposal");
	proposal->set_spi(proposal, 42);

	tkm_keymat_t *keymat = tkm_keymat_create(TRUE);
	fail_if(!keymat, "Unable to create keymat");

	chunk_t encr_i, encr_r, integ_i, integ_r;
	chunk_t nonce_i = chunk_from_chars("test chunk 1"),
			nonce_r = chunk_from_chars("test chunk 2");

	array_t *kes = NULL;
	array_insert_create(&kes, ARRAY_TAIL, ke1);
	array_insert_create(&kes, ARRAY_TAIL, ke2);
	fail_unless(keymat->keymat_v2.derive_child_keys(&keymat->keymat_v2, proposal,
													kes, nonce_i, nonce_r, &encr_i,
													&integ_i, &encr_r, &integ_r),
				"Child key derivation failed");
	array_destroy(kes);

	esa_info_t *info = (esa_info_t *)encr_i.ptr;
	fail_if(!info, "encr_i does not contain esa information");
	fail_if(info->isa_id != keymat->get_isa_id(keymat),
			"Isa context id mismatch (encr_i)");
	fail_if(info->spi_l != 42,
			"SPI mismatch (encr_i)");
	fail_unless(chunk_equals(info->nonce_i, nonce_i),
				"nonce_i mismatch (encr_i)");
	fail_unless(chunk_equals(info->nonce_r, nonce_r),
				"nonce_r mismatch (encr_i)");
	fail_if(info->is_encr_r,
			"Flag is_encr_r set for encr_i");
	fail_if(info->ke_ids.size != 2,
			"KE context number mismatch (encr_i)");
	fail_if(info->ke_ids.data[0] != ke1->get_id(ke1),
			"KE context id mismatch (encr_i)");
	fail_if(info->ke_ids.data[1] != ke2->get_id(ke2),
			"KE context id mismatch (encr_i)");
	chunk_free(&info->nonce_i);
	chunk_free(&info->nonce_r);

	info = (esa_info_t *)encr_r.ptr;
	fail_if(!info, "encr_r does not contain esa information");
	fail_if(info->isa_id != keymat->get_isa_id(keymat),
			"Isa context id mismatch (encr_r)");
	fail_if(info->spi_l != 42,
			"SPI mismatch (encr_r)");
	fail_unless(chunk_equals(info->nonce_i, nonce_i),
				"nonce_i mismatch (encr_r)");
	fail_unless(chunk_equals(info->nonce_r, nonce_r),
				"nonce_r mismatch (encr_r)");
	fail_unless(info->is_encr_r,
				"Flag is_encr_r set for encr_r");
	fail_if(info->ke_ids.size != 2,
			"KE context number mismatch (encr_i)");
	fail_if(info->ke_ids.data[0] != ke1->get_id(ke1),
			"KE context id mismatch (encr_i)");
	fail_if(info->ke_ids.data[1] != ke2->get_id(ke2),
			"KE context id mismatch (encr_i)");
	chunk_free(&info->nonce_i);
	chunk_free(&info->nonce_r);

	proposal->destroy(proposal);
	ke1->ke.destroy(&ke1->ke);
	ke2->ke.destroy(&ke2->ke);
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
	tcase_add_test(tc, test_derive_ike_keys_multi_ke);
	suite_add_tcase(s, tc);

	tc = tcase_create("derive CHILD keys");
	tcase_add_test(tc, test_derive_child_keys);
	tcase_add_test(tc, test_derive_child_keys_multi_ke);
	suite_add_tcase(s, tc);

	return s;
}
