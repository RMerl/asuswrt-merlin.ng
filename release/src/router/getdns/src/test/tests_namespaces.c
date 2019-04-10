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
main()
{

    /* First up, use the default settings on a general and non-general call */

    /* The namespaces used are per query depending on the type of method called.
       But note that namespaces can only be changed before the first query.*/

    /* Create the DNS context for this call */
    struct getdns_context *this_context = NULL;
    getdns_return_t context_create_return =
        getdns_context_create(&this_context, 1);
    if (context_create_return != GETDNS_RETURN_GOOD) {
        fprintf(stderr, "Trying to create the context failed: %d",
            context_create_return);
        return (GETDNS_RETURN_GENERIC_ERROR);
    }
    getdns_context_set_resolution_type(this_context, GETDNS_RESOLUTION_STUB);

    /* This will return a response with only the just_address_answers part
       as the current implementaiton uses [LOCALNAMES, DNS]*/
    struct getdns_dict *response = NULL;
    getdns_return_t ret =
        getdns_address_sync(this_context, "localhost", NULL, &response);

    if (ret != GETDNS_RETURN_GOOD || response == NULL) {
        fprintf(stderr, "Address sync returned error.\n");
        exit(EXIT_FAILURE);
    }
    print_response(response);
    getdns_dict_destroy(response);

    /* This should fall back to a full DNS lookup*/
    ret = getdns_address_sync(this_context, "www.google.com", NULL, &response);

    if (ret != GETDNS_RETURN_GOOD || response == NULL) {
        fprintf(stderr, "Address sync returned error.\n");
        exit(EXIT_FAILURE);
    }
    print_response(response);
    getdns_dict_destroy(response);

    /* This should return a full DNS reply as the general lookups don't use
       the namespaces, they just do pure DNS*/
    ret = getdns_general_sync(this_context, "localhost", GETDNS_RRTYPE_A,
                              NULL, &response);
    if (ret != GETDNS_RETURN_GOOD || response == NULL) {
        fprintf(stderr, "General sync over TCP returned error.\n");
        exit(EXIT_FAILURE);
    }
    print_response(response);
    getdns_dict_destroy(response);
    
    /* Clean up */
    getdns_context_destroy(this_context);


    /* Secondly, specify the namespace and see what happens*/
    
    /* Create the DNS context for this call */
    struct getdns_context *next_context = NULL;
    context_create_return = getdns_context_create(&next_context, 1);
    if (context_create_return != GETDNS_RETURN_GOOD) {
        fprintf(stderr, "Trying to create the context failed: %d",
            context_create_return);
        return (GETDNS_RETURN_GENERIC_ERROR);
    }
    getdns_context_set_resolution_type(next_context, GETDNS_RESOLUTION_STUB);

    getdns_namespace_t namespace_arr[2] = {GETDNS_NAMESPACE_DNS, GETDNS_NAMESPACE_LOCALNAMES};
    getdns_context_set_namespaces(next_context, 2,namespace_arr);

    /* This will return a full DNS reply*/
    ret = getdns_address_sync(next_context, "localhost", NULL, &response);

    if (ret != GETDNS_RETURN_GOOD || response == NULL) {
        fprintf(stderr, "Address sync returned error.\n");
        exit(EXIT_FAILURE);
    }
    print_response(response);
    getdns_dict_destroy(response);

    /* Clean up */
    getdns_context_destroy(next_context);
    
    /* Assuming we get here, leave gracefully */
    exit(EXIT_SUCCESS);
}                /* main */

/* tests_stub_sync.c */

