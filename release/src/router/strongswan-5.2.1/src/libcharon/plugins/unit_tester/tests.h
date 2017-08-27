/*
 * Copyright (C) 2007 Martin Willi
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

/**
 * @defgroup tests tests
 * @{ @ingroup unit_tester
 */

DEFINE_TEST("auth cfg", test_auth_cfg, FALSE)
DEFINE_TEST("CURL get", test_curl_get, FALSE)
DEFINE_TEST("MySQL operations", test_mysql, FALSE)
DEFINE_TEST("SQLite operations", test_sqlite, FALSE)
DEFINE_TEST("X509 certificate", test_cert_x509, FALSE)
DEFINE_TEST("Mediation database key fetch", test_med_db, FALSE)
DEFINE_TEST("IP pool", test_pool, FALSE)
DEFINE_TEST("SSH agent", test_agent, FALSE)

/** @}*/
