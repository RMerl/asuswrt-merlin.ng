/*
 * Copyright (C) 2014 Martin Willi
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

TEST_SUITE(hkdf_suite_create)
TEST_SUITE(socket_suite_create)
TEST_SUITE_DEPEND(socket_suite_ed25519_create, PRIVKEY_GEN, KEY_ED25519)
TEST_SUITE_DEPEND(socket_suite_ed448_create, PRIVKEY_GEN, KEY_ED448)
TEST_SUITE(suites_suite_create)
