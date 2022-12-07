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

TEST_SUITE(make_id_manager_tests)
TEST_SUITE(make_chunk_map_tests)
TEST_SUITE(make_utility_tests)
TEST_SUITE_DEPEND(make_nonceg_tests, CUSTOM, "tkm")
TEST_SUITE_DEPEND(make_diffie_hellman_tests, CUSTOM, "tkm")
TEST_SUITE_DEPEND(make_keymat_tests, CUSTOM, "tkm")
TEST_SUITE(make_kernel_sad_tests)
