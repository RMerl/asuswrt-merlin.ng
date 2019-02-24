/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#include "sasl_mechanism.h"

#include "sasl_plain/sasl_plain.h"

/**
 * Available SASL mechanisms.
 */
static struct {
	char *name;
	bool server;
	sasl_mechanism_constructor_t create;
} mechs[] = {
	{ "PLAIN",		TRUE,	(sasl_mechanism_constructor_t)sasl_plain_create	},
	{ "PLAIN",		FALSE,	(sasl_mechanism_constructor_t)sasl_plain_create	},
};

/**
 * See header.
 */
sasl_mechanism_t *sasl_mechanism_create(char *name, identification_t *client)
{
	int i;

	for (i = 0; i < countof(mechs); i++)
	{
		if (streq(mechs[i].name, name) && mechs[i].server == (client == NULL))
		{
			return mechs[i].create(name, client);
		}
	}
	return NULL;
}

/**
 * SASL mechanism enumerator
 */
typedef struct {
	/** implements enumerator_t */
	enumerator_t public;
	/** looking for client or server? */
	bool server;
	/** position in mechs[] */
	int i;
} mech_enumerator_t;

METHOD(enumerator_t, mech_enumerate, bool,
	mech_enumerator_t *this, va_list args)
{
	char **name;

	VA_ARGS_VGET(args, name);
	while (this->i < countof(mechs))
	{
		if (mechs[this->i].server == this->server)
		{
			*name = mechs[this->i].name;
			this->i++;
			return TRUE;
		}
		this->i++;
	}
	return FALSE;
}

/**
 * See header.
 */
enumerator_t* sasl_mechanism_create_enumerator(bool server)
{
	mech_enumerator_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _mech_enumerate,
			.destroy = (void*)free,
		},
		.server = server,
	);
	return &enumerator->public;
}
