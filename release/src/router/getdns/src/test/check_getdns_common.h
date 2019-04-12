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
#ifndef _check_getdns_common_h_
#define _check_getdns_common_h_

#include "getdns/getdns.h"
#include "getdns/getdns_extra.h"

     #define TRUE 1
     #define FALSE 0
     #define MAXLEN 200

     extern int callback_called;
     extern int callback_completed;
     extern int callback_canceled;
     extern uint16_t expected_changed_item;

     struct extracted_response {
       uint32_t top_answer_type;
       struct getdns_dict *response;
       struct getdns_bindata *top_canonical_name;
       struct getdns_list *just_address_answers;
       struct getdns_list *replies_full;
       struct getdns_list *replies_tree;
       struct getdns_dict *replies_tree_sub_dict;
       struct getdns_list *additional;
       struct getdns_list *answer;
       uint32_t answer_type;
       struct getdns_list *authority;
       struct getdns_bindata *canonical_name;
       struct getdns_dict *header;
       struct getdns_dict *question;
       uint32_t status;
     };

     /*
      *  The ASSERT_RC macro is used to assert
      *  whether the return code from the last
      *  getdns API call is what was expected.
      */
     #define ASSERT_RC(rc, expected_rc, prefix)					\
     {                                          				\
       uint32_t evaluated_rc = rc;						\
       ck_assert_msg((uint32_t) evaluated_rc == (uint32_t) expected_rc,		\
         "%s: expecting %s: %d, but received: %d: %s",				\
         prefix, #expected_rc, expected_rc, evaluated_rc, getdns_get_errorstr_by_id((uint16_t)evaluated_rc));	\
     }

     /*
      *  The CONTEXT_CREATE macro is used to
      *  create a context and assert the proper
      *  return code is returned.
      */
     #define CONTEXT_CREATE(set_from_os)			\
       ASSERT_RC(getdns_context_create(&context, set_from_os),	\
         GETDNS_RETURN_GOOD, 					\
         "Return code from getdns_context_create()");

     /*
      *  The CONTEXT_FREE macro is used to
      *  destroy the current context.
      */
     #define CONTEXT_DESTROY \
       getdns_context_destroy(context);

     /*
      *  The EVENT_BASE_CREATE macro is used to
      *  create an event base and put it in the
      *  context.
      */
     #define EVENT_BASE_CREATE	eventloop = create_event_base(context);

     /*
      *   The RUN_EVENT_LOOP macro calls the event loop.
      */
     #define RUN_EVENT_LOOP run_event_loop(context, eventloop);

     /*
      *  The LIST_CREATE macro simply creates a
      *  list and verifies the returned pointer
      *  is not NULL.
      */
    #define LIST_CREATE(list)		\
      list = getdns_list_create();	\
      ck_assert_msg(list != NULL, 	\
        "NULL pointer returned by getdns_list_create()");

     /*
      *  The LIST_DESTROY macro destroys a list.
      */
     #define LIST_DESTROY(list) getdns_list_destroy(list);

     /*
      *  The DICT_CREATE macro simply creates a
      *  dict and verifies the returned pointer
      *  is not NULL.
      */
    #define DICT_CREATE(dict)		\
      dict = getdns_dict_create();	\
      ck_assert_msg(dict != NULL, 	\
        "NULL pointer returned by getdns_dict_create()");

     /*
      *  The DICT_DESTROY macro destroys a dict.
      */
     #define DICT_DESTROY(dict) getdns_dict_destroy(dict);

     /*
      *  The process_response macro declares the
      *  variables needed to house the response and
      *  calls the function that extracts it.
      */
     #define EXTRACT_RESPONSE			\
       struct extracted_response ex_response;	\
       extract_response(response, &ex_response);

     //
     //  FUNCTION DECLARATIONS
     //
     
     #define EXTRACT_LOCAL_RESPONSE			\
       struct extracted_response ex_response;	\
       extract_local_response(response, &ex_response);

     //
     //  FUNCTION DECLARATIONS
     //

     /*
      *    extract_response extracts all of the various information
      *    a test may want to look at from the response.
      */
     void extract_response(struct getdns_dict *response, struct extracted_response *ex_response);

     /*
      *    extract__local_response extracts all of the various information
      *    a test may want to look at from the response for a minimal, local.
      */
     void extract_local_response(struct getdns_dict *response, struct extracted_response *ex_response);

     /*
      *    assert_noerror asserts that the rcode is 0.
      */
     void assert_noerror(struct extracted_response *ex_response);

    /*
     *     assert_nodata asserts that ancount in the header and the
     *     of the answer section (list) are both zero.
     */
     void assert_nodata(struct extracted_response *ex_response);

     /*
      *    assert_address_records_in_answer asserts that ancount in
      *    the header *    is >= 1, ancount is equal to the length
      *    of "answer", and that all of *    the records in the
      *    answer section are A and/or AAAA resource records based
      *    on the value of the a/aaaa arguments.
      */
     void assert_address_in_answer(struct extracted_response *ex_response, int a, int aaaa);

     /*
      *    assert_address_in_just_address_answers asserts that 
      *    just_address_answers contains at least one address.
      */
     void assert_address_in_just_address_answers(struct extracted_response *ex_response);

     /*
      *    assert_nxdomain asserts that an NXDOMAIN response was
      *    was returned for the DNS query meaning rcode == 3.
      */
     void assert_nxdomain(struct extracted_response *ex_response);

     /*
      *    assert_soa_in_authority asserts that a SOA record was
      *    returned in the authority sections.
      */
     void assert_soa_in_authority(struct extracted_response *ex_response);

     /*
      *    assert_ptr_in_answer asserts that a PTR record was
      *    returned in the answer sections.
      */
     void assert_ptr_in_answer(struct extracted_response *ex_response);


     void destroy_callbackfn(struct getdns_context *context,
                     getdns_callback_type_t callback_type,
                     struct getdns_dict *response,
                     void *userarg,
                     getdns_transaction_t transaction_id);

     typedef struct fn_cont {
          void (*fn)(struct extracted_response *ex_response);
     } fn_cont;
     /*
      *    callbackfn is the callback function given to all
      *    asynchronous query tests.  It is expected to only
      *    be called for positive tests and will verify the
      *    response that is returned.
      */
     void callbackfn(struct getdns_context *context,
                     getdns_callback_type_t callback_type,
                     struct getdns_dict *response,
                     void *userarg,
                     getdns_transaction_t transaction_id);

     /*
      *    update_callbackfn is the callback function given to
      *    getdns_context_set_context_update_callback tests.
      */
     void update_callbackfn(struct getdns_context *context,
                     getdns_context_code_t changed_item);

     /* run the event loop */
     void run_event_loop(struct getdns_context *context, void* eventloop);

     void* create_event_base(struct getdns_context* context);

#endif
