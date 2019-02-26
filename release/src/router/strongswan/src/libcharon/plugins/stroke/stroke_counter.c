/*
 * Copyright (C) 2017 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include <inttypes.h>

#include "stroke_counter.h"

#include <counters_query.h>

ENUM(stroke_counter_type_names,
	COUNTER_INIT_IKE_SA_REKEY, COUNTER_OUT_INFORMATIONAL_RSP,
	"ikeInitRekey",
	"ikeRspRekey",
	"ikeChildSaRekey",
	"ikeInInvalid",
	"ikeInInvalidSpi",
	"ikeInInitReq",
	"ikeInInitRsp",
	"ikeOutInitReq",
	"ikeOutInitRsp",
	"ikeInAuthReq",
	"ikeInAuthRsp",
	"ikeOutAuthReq",
	"ikeOutAuthRsp",
	"ikeInCrChildReq",
	"ikeInCrChildRsp",
	"ikeOutCrChildReq",
	"ikeOutCrChildRsp",
	"ikeInInfoReq",
	"ikeInInfoRsp",
	"ikeOutInfoReq",
	"ikeOutInfoRsp",
);

typedef struct private_stroke_counter_t private_stroke_counter_t;

/**
 * Private data of an stroke_counter_t object.
 */
struct private_stroke_counter_t {

	/**
	 * Public stroke_counter_t interface.
	 */
	stroke_counter_t public;

	/**
	 * Reference to query interface
	 */
	counters_query_t *query;
};

/**
 * Make sure we have the query interface
 */
static inline bool ensure_query(private_stroke_counter_t *this)
{
	if (this->query)
	{
		return TRUE;
	}
	return (this->query = lib->get(lib, "counters")) != NULL;
}

/**
 * Print global or connection-specific IKE counters
 */
static void print_one(private_stroke_counter_t *this, FILE *out, char *name)
{
	uint64_t *counters;
	counter_type_t i;

	counters = this->query->get_all(this->query, name);
	if (!counters)
	{
		fprintf(out, "No IKE counters found for '%s'\n", name);
		return;
	}
	if (name)
	{
		fprintf(out, "\nList of IKE counters for '%s':\n\n", name);
	}
	else
	{
		fprintf(out, "\nList of IKE counters:\n\n");
	}
	for (i = 0; i < COUNTER_MAX; i++)
	{
		fprintf(out, "%-18N %12"PRIu64"\n", stroke_counter_type_names, i,
				counters[i]);
	}
	free(counters);
}

/**
 * Print counters for all connections
 */
static void print_all(private_stroke_counter_t *this, FILE *out)
{
	enumerator_t *enumerator;
	char *name;

	enumerator = this->query->get_names(this->query);
	while (enumerator->enumerate(enumerator, &name))
	{
		print_one(this, out, name);
	}
	enumerator->destroy(enumerator);
}

METHOD(stroke_counter_t, print, void,
	private_stroke_counter_t *this, FILE *out, char *name)
{
	if (!ensure_query(this))
	{
		fprintf(out, "\nNo counters available (plugin missing?)\n\n");
		return;
	}
	if (name && streq(name, "all"))
	{
		return print_all(this, out);
	}
	return print_one(this, out, name);
}

METHOD(stroke_counter_t, reset, void,
	private_stroke_counter_t *this, char *name)
{
	if (!ensure_query(this))
	{
		return;
	}
	this->query->reset(this->query, name);
}

METHOD(stroke_counter_t, destroy, void,
	private_stroke_counter_t *this)
{
	free(this);
}

/**
 * See header
 */
stroke_counter_t *stroke_counter_create()
{
	private_stroke_counter_t *this;

	INIT(this,
		.public = {
			.print = _print,
			.reset = _reset,
			.destroy = _destroy,
		},
	);

	return &this->public;
}
