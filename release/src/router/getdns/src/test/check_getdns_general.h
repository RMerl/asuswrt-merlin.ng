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
#ifndef _check_getdns_general_h_
#define _check_getdns_general_h_

    /*
     **************************************************************************
     *                                                                        *
     *  T E S T S  F O R  G E T D N S _ G E M E R A L                         *
     *                                                                        *
     **************************************************************************
    */

     START_TEST (getdns_general_1)
     {
      /*
       *  context = NULL
       *  expect: GETDNS_RETURN_INVALID_PARAMETER
       */
       struct getdns_context *context = NULL;
       getdns_transaction_t transaction_id = 0;

       ASSERT_RC(getdns_general(context, "google.com", GETDNS_RRTYPE_A, NULL,
         NULL, &transaction_id, callbackfn),
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_general()");
     }
     END_TEST

     START_TEST (getdns_general_2)
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

       ASSERT_RC(getdns_general(context, NULL, GETDNS_RRTYPE_A, NULL,
         NULL, &transaction_id, callbackfn),
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_general()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

     START_TEST (getdns_general_3)
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

       ASSERT_RC(getdns_general(context, name, GETDNS_RRTYPE_A, NULL,
         NULL, &transaction_id, callbackfn),
         GETDNS_RETURN_BAD_DOMAIN_NAME, "Return code from getdns_general()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

     START_TEST (getdns_general_4)
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

       ASSERT_RC(getdns_general(context, name, GETDNS_RRTYPE_A, NULL,
         NULL, &transaction_id, callbackfn),
         GETDNS_RETURN_BAD_DOMAIN_NAME, "Return code from getdns_general()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

     START_TEST (getdns_general_5)
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

       ASSERT_RC(getdns_general(context, "google.com", GETDNS_RRTYPE_A, NULL,
         NULL, &transaction_id, NULL),
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_general()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

#if 0
     START_TEST (getdns_general_6)
     {
      /*
       *  name = "google.com"
       *  request_type = 0 (minimum valid RRTYPE)
       *  expect: NOERROR/NODATA response:
       *    status = GETDNS_RESPSTATUS_NO_NAME
       *    rcode = 0
       *    ancount = 0 (number of records in ANSWER section)
       */
       void verify_getdns_general_6(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_general_6 };
       struct getdns_context *context = NULL;   \
       void* eventloop = NULL;    \
       getdns_transaction_t transaction_id = 0;
     
       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;
     
       ASSERT_RC(getdns_general(context, "google.com", 0, NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_general()");
     
       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST
     
     void verify_getdns_general_6(struct extracted_response *ex_response)
     {
       assert_noerror(ex_response);
       assert_nodata(ex_response);
     }
#endif

     START_TEST (getdns_general_7)
     {
      /*
       *  name = "nlnetlabs.nl"
       *  request_type = 65279 (maximum unassigned RRTYPE)
       *  expect: NOERROR/NODATA response:
       *    status = GETDNS_RESPSTATUS_NO_NAME
       *    rcode = 0
       *    ancount = 0 (number of records in ANSWER section)
       */
       void verify_getdns_general_7(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_general_7 };
       struct getdns_context *context = NULL;   \
       void* eventloop = NULL;    \
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_general(context, "nlnetlabs.nl", 65279, NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_general()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

     void verify_getdns_general_7(struct extracted_response *ex_response)
     {
       assert_noerror(ex_response);
       assert_nodata(ex_response);
     }

     START_TEST (getdns_general_8)
     {
      /*
       *  name = "google.com"
       *  request_type = GETDNS_RRTYPE_A
       *  expect: NOERROR response with A records
       *    status = GETDNS_RESPSTATUS_GOOD
       *    rcode = 0
       *    ancount >= 1 (number of records in ANSWER section)
       *      and equals number of A records ("type": 1) in "answer" list
       */
       void verify_getdns_general_8(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_general_8 };
       struct getdns_context *context = NULL;   \
       void* eventloop = NULL;    \
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_general(context, "google.com", GETDNS_RRTYPE_A, NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_general()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

     void verify_getdns_general_8(struct extracted_response *ex_response)
     {
       assert_noerror(ex_response);
       assert_address_in_answer(ex_response, TRUE, FALSE);
     }

     START_TEST (getdns_general_9)
     {
      /*
       *  name = "google.com"
       *  request_type = GETDNS_RRTYPE_AAAA
       *  expect: NOERROR response with AAAA records
       *    status = GETDNS_RESPSTATUS_GOOD
       *    rcode = 0
       *    ancount >= 1 (number of records in ANSWER section)
       *      and equals number of AAAA records ("type": 28) in "answer" list
       */
       void verify_getdns_general_9(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_general_9 };
       struct getdns_context *context = NULL;   \
       void* eventloop = NULL;    \
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_general(context, "google.com", GETDNS_RRTYPE_AAAA, NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_general()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

     void verify_getdns_general_9(struct extracted_response *ex_response)
     {
       assert_noerror(ex_response);
       assert_address_in_answer(ex_response, FALSE, TRUE);
     }

     START_TEST (getdns_general_10)
     {
      /*
       *  name = "thisdomainsurelydoesntexist.com"
       *  request_type = GETDNS_RRTYPE_TXT`
       *  expect: NXDOMAIN response with SOA record
       *    status = GETDNS_RESPSTATUS_GOOD
       *    rcode = 3
       *    ancount = 0 (number of records in ANSWER section)
       *    nscount = 1 (number of records in AUTHORITY section)
       *      and SOA record ("type": 6) present in "authority" list
       */
       void verify_getdns_general_10(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_general_10 };
       struct getdns_context *context = NULL;   \
       void* eventloop = NULL;    \
       getdns_transaction_t transaction_id = 0;
       const char *name = "thisdomainsurelydoesntexist.com";

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_general(context, name, GETDNS_RRTYPE_TXT, NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_general()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

     void verify_getdns_general_10(struct extracted_response *ex_response)
     {
       assert_nxdomain(ex_response);
       assert_nodata(ex_response);
       // Ubuntu 18.04 system resolver does not return an SOA
       //assert_soa_in_authority(ex_response);
     }

     START_TEST (getdns_general_11)
     {
      /*
       *  name = "d2a8n3.rootcanary.net"  and unbound zone
       *  request_type = GETDNS_RRTYPE_MX
       *  expect: NOERROR/NODATA response:
       *    status = GETDNS_RESPSTATUS_NO_NAME
       *    rcode = 0
       *    ancount = 0 (number of records in ANSWER section)
       */
       void verify_getdns_general_11(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_general_11 };
       struct getdns_context *context = NULL;   \
       void* eventloop = NULL;    \
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_general(context, "d2a8n3.rootcanary.net", GETDNS_RRTYPE_MX, NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_general()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

     void verify_getdns_general_11(struct extracted_response *ex_response)
     {
       assert_noerror(ex_response);
       assert_nodata(ex_response);
     }

     START_TEST (getdns_general_12)
     {
      /*
       *  name = "google.com"  need to swap this out for max domain name length with max lable length`
       *  request_type = GETDNS_RRTYPE_A
       *  expect: NOERROR response with A records
       *    status = GETDNS_RESPSTATUS_GOOD
       *    rcode = 0
       *    ancount >= 1 (number of records in ANSWER section)
       *      and equals number of A records ("type": 1) in "answer" list
       */
       void verify_getdns_general_12(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_general_12 };
       struct getdns_context *context = NULL;   \
       void* eventloop = NULL;    \
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_general(context, "google.com", GETDNS_RRTYPE_A, NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_general()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

     void verify_getdns_general_12(struct extracted_response *ex_response)
     {
       assert_noerror(ex_response);
       assert_address_in_answer(ex_response, TRUE, FALSE);
     }

     Suite *
     getdns_general_suite (void)
     {
       Suite *s = suite_create ("getdns_general()");

       /* Negative test caseis */
       TCase *tc_neg = tcase_create("Negative");
       tcase_add_test(tc_neg, getdns_general_1);
       tcase_add_test(tc_neg, getdns_general_2);
       tcase_add_test(tc_neg, getdns_general_3);
       tcase_add_test(tc_neg, getdns_general_4);
       tcase_add_test(tc_neg, getdns_general_5);
       suite_add_tcase(s, tc_neg);

       /* Positive test cases */
       TCase *tc_pos = tcase_create("Positive");
       // Ubuntu 18.04 system resolver returns FORMERR for this query
       //tcase_add_test(tc_pos, getdns_general_6);
       tcase_add_test(tc_pos, getdns_general_7);
       tcase_add_test(tc_pos, getdns_general_8);
       tcase_add_test(tc_pos, getdns_general_9);
       tcase_add_test(tc_pos, getdns_general_10);
       tcase_add_test(tc_pos, getdns_general_11);
       tcase_add_test(tc_pos, getdns_general_12);
       suite_add_tcase(s, tc_pos);

       return s;
     }

#endif
