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
#ifndef _check_getdns_address_h_
#define _check_getdns_address_h_

    /*
     ***************************************************
     *                                                 *
     *  T E S T S  F O R  G E T D N S _ A D D R E S S  *
     *                                                 *
     ***************************************************
    */

     START_TEST (getdns_address_1)
     {
      /*
       *  context = NULL
       *  expect: GETDNS_RETURN_INVALID_PARAMETER
       */
       struct getdns_context *context = NULL;
       getdns_transaction_t transaction_id = 0;

       ASSERT_RC(getdns_address(context, "google.com", NULL,
         NULL, &transaction_id, callbackfn),
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_address()");
     }
     END_TEST

     START_TEST (getdns_address_2)
     {
      /*
       *  name = NULL
       *  expect: GETDNS_RETURN_INVALID_PARAMETER
       */
       struct getdns_context *context = NULL;   \
       void* eventloop = NULL;    \
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_address(context, NULL, NULL,
         NULL, &transaction_id, callbackfn),
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_address()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

     START_TEST (getdns_address_3)
     {
      /*
       *  name = invalid domain (too many octets)
       *  expect:  GETDNS_RETURN_BAD_DOMAIN_NAME
       */
       struct getdns_context *context = NULL;   \
       void* eventloop = NULL;    \
       getdns_transaction_t transaction_id = 0;
       const char *name = "oh.my.gosh.and.for.petes.sake.are.you.fricking.crazy.man.because.this.spectacular.and.elaborately.thought.out.domain.name.of.very.significant.length.is.just.too.darn.long.because.you.know.the rfc.states.that.two.hundred.fifty.five.characters.is.the.max.com";

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_address(context, name, NULL,
         NULL, &transaction_id, callbackfn),
         GETDNS_RETURN_BAD_DOMAIN_NAME, "Return code from getdns_address()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

     START_TEST (getdns_address_4)
     {
      /*
       *  name = invalid domain (label too long)
       *  expect: GETDNS_RETURN_BAD_DOMAIN_NAME
       */
       struct getdns_context *context = NULL;   \
       void* eventloop = NULL;    \
       getdns_transaction_t transaction_id = 0;
       const char *name = "this.domain.hasalabelwhichexceedsthemaximumdnslabelsizeofsixtythreecharacters.com";

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_address(context, name, NULL,
         NULL, &transaction_id, callbackfn),
         GETDNS_RETURN_BAD_DOMAIN_NAME, "Return code from getdns_address()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

     START_TEST (getdns_address_5)
     {
      /*
       *  callbackfn = NULL
       *  expect:  GETDNS_RETURN_INVALID_PARAMETER
       */
       struct getdns_context *context = NULL;   \
       void* eventloop = NULL;    \
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_address(context, "google.com", NULL,
         NULL, &transaction_id, NULL),
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_address()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST


     START_TEST (getdns_address_6)
     {
      /*
       *  name = "google.com"
       *    status = GETDNS_RESPSTATUS_GOOD
       *    rcode = 0
       */
       void verify_getdns_address_6(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_address_6 };
       struct getdns_context *context = NULL;   \
       void* eventloop = NULL;    \
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_address(context, "google.com", NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_address()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

     void verify_getdns_address_6(struct extracted_response *ex_response)
     {
       assert_noerror(ex_response);
       //assert_soa_in_authority(ex_response);
       assert_address_in_answer(ex_response, TRUE, TRUE);
     }


     START_TEST (getdns_address_7)
     {
      /*
       *  name = "localhost"   name should be resolved from host file
       *  expect: NOERROR response:
       *    status = GETDNS_RESPSTATUS_GOOD
       *    rcode = 0
       *    ancount = 1 (number of records in ANSWER section)
       */
       void verify_getdns_address_7(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_address_7 };
       struct getdns_context *context = NULL;   \
       void* eventloop = NULL;    \
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_address(context, "localhost.", NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_address()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

     void verify_getdns_address_7(struct extracted_response *ex_response)
     {
       assert_address_in_just_address_answers(ex_response);
     }

     START_TEST (getdns_address_8)
     {
      /*
       *  name = "hostnamedoesntexist" (name should not be resolved)
       *  expect: NXDOMAIN response
       *    status = GETDNS_RESPSTATUS_GOOD
       *    rcode = 3 (NXDOMAIN)
       */
       void verify_getdns_address_8(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_address_8 };
       struct getdns_context *context = NULL;   \
       void* eventloop = NULL;    \
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);

       EVENT_BASE_CREATE;

       // Ubuntu 18.04 system resolver - does not foward single labels, returns 
       // SERVFAIL so this must be a multilabel name
       ASSERT_RC(getdns_address(context, "hostname.doesntexist.", NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_address()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

     void verify_getdns_address_8(struct extracted_response *ex_response)
     {
       assert_nxdomain(ex_response);
       assert_nodata(ex_response);
       // Ubuntu 18.04 system resolver does not return an SOA
       //assert_soa_in_authority(ex_response);
     }


     Suite *
     getdns_address_suite (void)
     {
       Suite *s = suite_create ("getdns_address()");

       /* Negative test caseis */
       TCase *tc_neg = tcase_create("Negative");
       tcase_add_test(tc_neg, getdns_address_1);
       tcase_add_test(tc_neg, getdns_address_2);
       tcase_add_test(tc_neg, getdns_address_3);
       tcase_add_test(tc_neg, getdns_address_4);
       tcase_add_test(tc_neg, getdns_address_5);
       suite_add_tcase(s, tc_neg);

       /* Positive test cases */
       TCase *tc_pos = tcase_create("Positive");
       tcase_add_test(tc_pos, getdns_address_6);
       tcase_add_test(tc_pos, getdns_address_7);
       tcase_add_test(tc_pos, getdns_address_8);
       suite_add_tcase(s, tc_pos);

       return s;
     }

#endif
