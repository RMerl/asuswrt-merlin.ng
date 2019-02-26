/*
 * Copyright (C) 2014-2016 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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

/**
 * @defgroup libcharon-tests tests
 * @ingroup libcharon
 *
 * @defgroup test_utils_c test_utils
 * @ingroup libcharon-tests
 */

TEST_SUITE(ike_cfg_suite_create)
TEST_SUITE(peer_cfg_suite_create)
TEST_SUITE(mem_pool_suite_create)
TEST_SUITE_DEPEND(message_chapoly_suite_create, AEAD, ENCR_CHACHA20_POLY1305, 32)
