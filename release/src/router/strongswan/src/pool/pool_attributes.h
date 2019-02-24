/*
 * Copyright (C) 2009-2010 Andreas Steffen
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

#ifndef POOL_ATTRIBUTES_H_
#define POOL_ATTRIBUTES_H_

#include <attributes/attributes.h>

typedef enum value_type_t value_type_t;

enum value_type_t {
	VALUE_NONE,
	VALUE_HEX,
	VALUE_STRING,
	VALUE_ADDR,
	VALUE_SUBNET
};

/**
 * enum names for value_type_t.
 */
extern enum_name_t *value_type_names;

/**
 * lookup/insert an identity
 */
u_int get_identity(identification_t *id);

/**
 * ipsec pool --addattr <type>  - add attribute entry
 */
void add_attr(char *name, char *pool, char *identity,
			  char *value, value_type_t value_type);

/**
 * ipsec pool --delattr <type>  - delete attribute entry
 */
void del_attr(char *name, char *pool, char *identity,
			  char *value, value_type_t value_type);

/**
 * ipsec pool --statusattr      - show all attribute entries
 */
void status_attr(bool hexout);

/**
 * ipsec pool --showattr - show all supported attribute keywords
 */
void show_attr(void);

#endif /* POOL_ATTRIBUTES_H_ */
