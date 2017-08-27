/*
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2008 Thomas Kallenberg
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
 * @defgroup uci_creds_t uci_creds
 * @{ @ingroup uci
 */

#ifndef UCI_CREDS_H_
#define UCI_CREDS_H_

#include "uci_parser.h"

#include <credentials/credential_set.h>

typedef struct uci_creds_t uci_creds_t;

/**
 * OpenWRT UCI credential set implementation.
 */
struct uci_creds_t {

	/**
	 * Implements credential set interface.
	 */
	credential_set_t credential_set;

	/**
	 * Destroy the backend.
	 */
	void (*destroy)(uci_creds_t *this);
};

/**
 * Create a UCI based credential set.
 *
 * @param parser	UCI parser to use
 * @return			credential set
 */
uci_creds_t *uci_creds_create(uci_parser_t *parser);

#endif /** UCI_CREDS_H_ @}*/
