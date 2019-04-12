/**
 * \file
 * unit tests for getdns_dict helper routines, these should be used to
 * perform regression tests, output must be unchanged from canonical output
 * stored with the sources
 */

/*
 * Copyright (c) 2013, NLNet Labs, Verisign, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the names of the copyright holders nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Verisign, Inc. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "testmessages.h"
#include "getdns/getdns.h"
#include "getdns/getdns_extra.h"

#define TRANSPORT_UDP "udp"
#define TRANSPORT_TCP "tcp"
#define TRANSPORT_PIPELINE "pipeline"
#define TRANSPORT_TLS_KEEPOPEN "tls"
#define TRANSPORT_TLS_TCP_KEEPOPEN "dns-over-tls"
#define RESOLUTION_STUB "stub"
#define RESOLUTION_REC "rec"

static void
print_response(struct getdns_dict * response)
{
	char *dict_str = getdns_pretty_print_dict(response);
	if (dict_str) {
		fprintf(stdout, "The packet %s\n", dict_str);
		free(dict_str);
	}
}

int
main(int argc, char** argv)
{

	const char *transport = argc > 1 ? argv[1] : "udp";
	const char *resolution = argc > 2 ? argv[2] : "stub";

	/* Create the DNS context for this call */
	struct getdns_context *this_context = NULL;
	getdns_return_t context_create_return =
	    getdns_context_create(&this_context, 1);
	if (context_create_return != GETDNS_RETURN_GOOD) {
		fprintf(stderr, "Trying to create the context failed: %d",
		    context_create_return);
		return (GETDNS_RETURN_GENERIC_ERROR);
	}

	if (strncmp(resolution, RESOLUTION_STUB, 4) == 0)
		getdns_context_set_resolution_type(this_context, GETDNS_RESOLUTION_STUB);
	else if (strncmp(resolution, RESOLUTION_REC, 4) != 0) {
		fprintf(stderr, "Invalid resolution %s, must be one of stub or rec\n", transport);
		exit(EXIT_FAILURE);
	}

	if (strncmp(transport, TRANSPORT_TCP, 3) == 0)
		getdns_context_set_dns_transport(this_context, GETDNS_TRANSPORT_TCP_ONLY);
	else if (strncmp(transport, TRANSPORT_PIPELINE, 8) == 0)
		getdns_context_set_dns_transport(this_context, GETDNS_TRANSPORT_TCP_ONLY_KEEP_CONNECTIONS_OPEN);
	else if (strncmp(transport, TRANSPORT_TLS_KEEPOPEN, 3) == 0) 
		getdns_context_set_dns_transport(this_context, GETDNS_TRANSPORT_TLS_ONLY_KEEP_CONNECTIONS_OPEN);
	else if (strncmp(transport, TRANSPORT_TLS_TCP_KEEPOPEN, 12) == 0) 
		getdns_context_set_dns_transport(this_context, GETDNS_TRANSPORT_TLS_FIRST_AND_FALL_BACK_TO_TCP_KEEP_CONNECTIONS_OPEN);
	else if (strncmp(transport, TRANSPORT_UDP, 3) != 0) {
		fprintf(stderr, "Invalid transport %s, must be one of udp, tcp or pipeline\n", transport);
		exit(EXIT_FAILURE);
	}

	struct getdns_dict *response = NULL;
	getdns_return_t ret =
	    getdns_address_sync(this_context, "www.google.com", NULL, &response);

	if (ret != GETDNS_RETURN_GOOD || response == NULL) {
		fprintf(stderr, "Address sync returned error.\n");
		exit(EXIT_FAILURE);
	}
	print_response(response);
	getdns_dict_destroy(response);

	ret =
	    getdns_service_sync(this_context, "_xmpp-client._tcp.jabber.com.", NULL, &response);
	if (ret != GETDNS_RETURN_GOOD || response == NULL) {
		fprintf(stderr, "Service sync returned error.\n");
		exit(EXIT_FAILURE);
	}
	print_response(response);
	getdns_dict_destroy(response);

	ret = getdns_general_sync(this_context, "www.google.com", GETDNS_RRTYPE_A,
	                          NULL, &response);
	if (ret != GETDNS_RETURN_GOOD || response == NULL) {
		fprintf(stderr, "General sync over TCP returned error.\n");
		exit(EXIT_FAILURE);
	}
	print_response(response);
	getdns_dict_destroy(response);

	/* Clean up */
	getdns_context_destroy(this_context);
	/* Assuming we get here, leave gracefully */
	exit(EXIT_SUCCESS);
}				/* main */

/* tests_stub_sync.c */
