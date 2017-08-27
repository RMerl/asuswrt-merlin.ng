/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup ldap_fetcher ldap_fetcher
 * @{ @ingroup ldap_p
 */

#ifndef LDAP_FETCHER_H_
#define LDAP_FETCHER_H_

typedef struct ldap_fetcher_t ldap_fetcher_t;

/**
 * Fetcher implementation using OpenLDAP.
 */
struct ldap_fetcher_t {

	/**
	 * Implements fetcher interface
	 */
	fetcher_t interface;
};

/**
 * Create a ldap_fetcher instance.
 */
ldap_fetcher_t *ldap_fetcher_create();

#endif /** LDAP_FETCHER_H_ @}*/
