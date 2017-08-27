/*
 * Copyright (C) 2011-2012 Reto Guadagnini
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

#include <stdio.h>

#include <library.h>
#include <utils/debug.h>

/**
 * Define debug level
 */
static level_t dbg_level = 1;

static void dbg_dnssec(debug_t group, level_t level, char *fmt, ...)
{
	if ((level <= dbg_level) || level <= 1)
	{
		va_list args;

		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		fprintf(stderr, "\n");
		va_end(args);
	}
}

int main(int argc, char *argv[])
{
	resolver_t *resolver;
	resolver_response_t *response;
	enumerator_t *enumerator;
	chunk_t rdata;
	rr_set_t *rrset;
	rr_t *rr;

	library_init(NULL, "dnssec");
	atexit(library_deinit);

	dbg = dbg_dnssec;

	if (!lib->plugins->load(lib->plugins, PLUGINS))
	{
		return 1;
	}
	if (argc != 2)
	{
		fprintf(stderr, "usage: dnssec <name>\n");
		return 1;
	}

	resolver = lib->resolver->create(lib->resolver);
	if (!resolver)
	{
		printf("failed to create a resolver!\n");
		return 1;
	}

	response = resolver->query(resolver, argv[1], RR_CLASS_IN, RR_TYPE_A);
	if (!response)
	{
		printf("no response received!\n");
		resolver->destroy(resolver);
		return 1;
	}

	printf("DNS response:\n");
	if (!response->has_data(response) || !response->query_name_exist(response))
	{
		if (!response->has_data(response))
		{
			printf("  no data in the response\n");
		}
		if (!response->query_name_exist(response))
		{
			printf("  query name does not exist\n");
		}
		response->destroy(response);
		resolver->destroy(resolver);
		return 1;
	}

	printf("  RRs in the response:\n");
	rrset = response->get_rr_set(response);
	if (!rrset)
	{
		printf("    response contains no RRset!\n");
		response->destroy(response);
		resolver->destroy(resolver);
		return 1;
	}

	enumerator = rrset->create_rr_enumerator(rrset);
	while (enumerator->enumerate(enumerator, &rr))
	{
		printf("    name: %s\n", rr->get_name(rr));
	}

	enumerator = rrset->create_rrsig_enumerator(rrset);
	if (enumerator)
	{
		printf("  RRSIGs for the RRset:\n");
		while (enumerator->enumerate(enumerator, &rr))
		{
			rdata = rr->get_rdata(rr);

			printf("    name: %s\n", rr->get_name(rr));
			printf("    RDATA: %#B\n", &rdata);
		}
	}

	printf("  security status of the response: ");
	switch (response->get_security_state(response))
	{
		case SECURE:
			printf("SECURE\n\n");
			break;
		case INSECURE:
			printf("INSECURE\n\n");
			break;
		case BOGUS:
			printf("BOGUS\n\n");
			break;
		case INDETERMINATE:
			printf("INDETERMINATE\n\n");
			break;
	}
	response->destroy(response);
	resolver->destroy(resolver);
	return 0;
}
