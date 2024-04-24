/*
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2022 Andreas Steffen
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

TEST_SUITE(bio_reader_suite_create)
TEST_SUITE(bio_writer_suite_create)
TEST_SUITE(chunk_suite_create)
TEST_SUITE(enum_suite_create)
TEST_SUITE(enumerator_suite_create)
TEST_SUITE(linked_list_suite_create)
TEST_SUITE(linked_list_enumerator_suite_create)
TEST_SUITE(hashtable_suite_create)
TEST_SUITE(array_suite_create)
TEST_SUITE(identification_suite_create)
TEST_SUITE(traffic_selector_suite_create)
TEST_SUITE(threading_suite_create)
TEST_SUITE(process_suite_create)
TEST_SUITE(watcher_suite_create)
TEST_SUITE(stream_suite_create)
TEST_SUITE(utils_suite_create)
TEST_SUITE(settings_suite_create)
TEST_SUITE(vectors_suite_create)
TEST_SUITE_DEPEND(ecdsa_suite_create, PRIVKEY_GEN, KEY_ECDSA)
TEST_SUITE_DEPEND(rsa_suite_create, PRIVKEY_GEN, KEY_RSA)
TEST_SUITE_DEPEND(rsa_pkcs1_suite_create,       PRIVKEY_DECRYPT, ENCRYPT_RSA_PKCS1)
TEST_SUITE_DEPEND(rsa_oaep_sha1_suite_create,   PRIVKEY_DECRYPT, ENCRYPT_RSA_OAEP_SHA1)
TEST_SUITE_DEPEND(rsa_oaep_sha224_suite_create, PRIVKEY_DECRYPT, ENCRYPT_RSA_OAEP_SHA224)
TEST_SUITE_DEPEND(rsa_oaep_sha256_suite_create, PRIVKEY_DECRYPT, ENCRYPT_RSA_OAEP_SHA256)
TEST_SUITE_DEPEND(rsa_oaep_sha384_suite_create, PRIVKEY_DECRYPT, ENCRYPT_RSA_OAEP_SHA384)
TEST_SUITE_DEPEND(rsa_oaep_sha512_suite_create, PRIVKEY_DECRYPT, ENCRYPT_RSA_OAEP_SHA512)
TEST_SUITE_DEPEND(certpolicy_suite_create, CERT_ENCODE, CERT_X509)
TEST_SUITE_DEPEND(certnames_suite_create, CERT_ENCODE, CERT_X509)
TEST_SUITE_DEPEND(serial_gen_suite_create, CERT_ENCODE, CERT_X509)
TEST_SUITE(serial_parse_suite_create)
TEST_SUITE(host_suite_create)
TEST_SUITE(printf_suite_create)
TEST_SUITE(auth_cfg_suite_create)
TEST_SUITE(hasher_suite_create)
TEST_SUITE(crypter_suite_create)
TEST_SUITE(proposal_suite_create)
TEST_SUITE(crypto_factory_suite_create)
TEST_SUITE_DEPEND(iv_gen_suite_create, RNG, RNG_STRONG)
TEST_SUITE(pen_suite_create)
TEST_SUITE(asn1_suite_create)
TEST_SUITE(asn1_parser_suite_create)
TEST_SUITE(rng_tester_suite_create)
TEST_SUITE_DEPEND(mgf1_sha1_suite_create, XOF, XOF_MGF1_SHA1)
TEST_SUITE_DEPEND(mgf1_sha256_suite_create, XOF, XOF_MGF1_SHA256)
TEST_SUITE_DEPEND(prf_plus_suite_create, KDF, KDF_PRF_PLUS)
TEST_SUITE_DEPEND(ntru_suite_create, KE, NTRU_112_BIT)
TEST_SUITE_DEPEND(fetch_http_suite_create, FETCHER, "http://")
TEST_SUITE_DEPEND(ed25519_suite_create, PRIVKEY_GEN, KEY_ED25519)
TEST_SUITE_DEPEND(ed448_suite_create, PRIVKEY_GEN, KEY_ED448)
TEST_SUITE(signature_params_suite_create)
TEST_SUITE(metadata_suite_create)
TEST_SUITE(metadata_set_suite_create)
