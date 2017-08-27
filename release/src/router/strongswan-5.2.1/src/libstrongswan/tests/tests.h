/*
 * Copyright (C) 2013 Tobias Brunner
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
TEST_SUITE(threading_suite_create)
TEST_SUITE(process_suite_create)
TEST_SUITE(watcher_suite_create)
TEST_SUITE(stream_suite_create)
TEST_SUITE(utils_suite_create)
TEST_SUITE(settings_suite_create)
TEST_SUITE(vectors_suite_create)
TEST_SUITE_DEPEND(ecdsa_suite_create, PRIVKEY_GEN, KEY_ECDSA)
TEST_SUITE_DEPEND(rsa_suite_create, PRIVKEY_GEN, KEY_RSA)
TEST_SUITE(host_suite_create)
TEST_SUITE(printf_suite_create)
TEST_SUITE(hasher_suite_create)
TEST_SUITE(crypter_suite_create)
TEST_SUITE(crypto_factory_suite_create)
TEST_SUITE(pen_suite_create)
TEST_SUITE(asn1_suite_create)
TEST_SUITE(asn1_parser_suite_create)
TEST_SUITE(test_rng_suite_create)
TEST_SUITE_DEPEND(ntru_suite_create, DH, NTRU_112_BIT)
TEST_SUITE_DEPEND(fetch_http_suite_create, FETCHER, "http://")
