/*
 * Copyright (C) 2017 Andreas Steffen
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

#define _GNU_SOURCE
#include <stdio.h>

#include "sw_collector_dpkg.h"

typedef struct private_sw_collector_dpkg_t private_sw_collector_dpkg_t;

/**
 * Private data of an sw_collector_dpkg_t object.
 */
struct private_sw_collector_dpkg_t {

	/**
	 * Public members of sw_collector_dpkg_state_t
	 */
	sw_collector_dpkg_t public;

};

typedef struct {
	/** public enumerator interface */
	enumerator_t public;
	/** dpkg output stream */
	FILE *file;
	/** current dpkg output line */
	char line[BUF_LEN];
} dpkg_enumerator_t;

METHOD(enumerator_t, enumerate, bool,
	dpkg_enumerator_t *this, va_list args)
{
	char **package, **arch, **version, *state, *pos;

	VA_ARGS_VGET(args, package, arch, version);

	while (TRUE)
	{
		if (!fgets(this->line, BUF_LEN, this->file))
		{
			return FALSE;
		}

		*package = this->line;
		pos = strchr(this->line, '\t');
		if (!pos)
		{
			return FALSE;
		}
		*pos = '\0';

		*arch = ++pos;
		pos = strchr(pos, '\t');
		if (!pos)
		{
			return FALSE;
		}
		*pos = '\0';

		*version = ++pos;
		pos = strchr(pos, '\t');
		if (!pos)
		{
			return FALSE;
		}
		*pos = '\0';

		state = ++pos;
		pos = strchr(pos, '\n');
		if (!pos)
		{
			return FALSE;
		}
		*pos = '\0';

		if (streq(state, "install ok installed"))
		{
			return TRUE;
		}
	}
}

METHOD(enumerator_t, enumerator_destroy, void,
	dpkg_enumerator_t *this)
{
	pclose(this->file);
	free(this);	
}

METHOD(sw_collector_dpkg_t, create_sw_enumerator, enumerator_t*,
	private_sw_collector_dpkg_t *this)
{
	dpkg_enumerator_t *enumerator;
	char cmd[] = "dpkg-query -W -f="
				 "\'${Package}\t${Architecture}\t${Version}\t${Status}\n\'";
	FILE *file;

	file = popen(cmd, "r");
	if (!file)
	{
		DBG1(DBG_IMC, "failed to run dpgk-query command");
		return NULL;
	}

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate,
			.destroy = _enumerator_destroy,
		},
		.file = file,
	);

	return &enumerator->public;
}

METHOD(sw_collector_dpkg_t, destroy, void,
	private_sw_collector_dpkg_t *this)
{
	free(this);
}

/**
 * Described in header.
 */
sw_collector_dpkg_t *sw_collector_dpkg_create(void)
{
	private_sw_collector_dpkg_t *this;

	INIT(this,
		.public = {
			.create_sw_enumerator = _create_sw_enumerator,
			.destroy = _destroy,
		},
	);

	return &this->public;
}
