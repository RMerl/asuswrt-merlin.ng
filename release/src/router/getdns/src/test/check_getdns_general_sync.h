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
#ifndef _check_getdns_general_sync_h_
#define _check_getdns_general_sync_h_

    /*
     **************************************************************************
     *                                                                        *
     *  T E S T S  F O R  G E T D N S _ G E N E R A L _ S Y N C               *
     *                                                                        *
     **************************************************************************
    */

     START_TEST (getdns_general_sync_1)
     {
      /*
       *  context = NULL
       *  expect: GETDNS_RETURN_INVALID_PARAMETER
       */
       struct getdns_context *context = NULL;
       struct getdns_dict *response = NULL;

       ASSERT_RC(getdns_general_sync(context, "google.com", GETDNS_RRTYPE_A, NULL, &response), 
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_general_sync()");
     }
     END_TEST
     
     START_TEST (getdns_general_sync_2)
     {
      /*
       *  name = NULL
       *  expect: GETDNS_RETURN_INVALID_PARAMETER
       */
       struct getdns_context *context = NULL;
       struct getdns_dict *response = NULL;

       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_general_sync(context, NULL, GETDNS_RRTYPE_A, NULL, &response), 
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_general_sync()");

       CONTEXT_DESTROY;
     }
     END_TEST
     
     START_TEST (getdns_general_sync_3)
     {
      /*
       *  name = invalid domain (too many octets)
       *  expect:  GETDNS_RETURN_BAD_DOMAIN_NAME
       */
       struct getdns_context *context = NULL;   
       struct getdns_dict *response = NULL;
       const char *name = "oh.my.gosh.and.for.petes.sake.are.you.fricking.crazy.man.because.this.spectacular.and.elaborately.thought.out.domain.name.of.very.significant.length.is.just.too.darn.long.because.you.know.the rfc.states.that.two.hundred.fifty.five.characters.is.the.max.com";

       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_general_sync(context, name, GETDNS_RRTYPE_A, NULL, &response), 
         GETDNS_RETURN_BAD_DOMAIN_NAME, "Return code from getdns_general_sync()");

       CONTEXT_DESTROY;
     }
     END_TEST
     
     START_TEST (getdns_general_sync_4)
     {
      /*
       *  name = invalid domain (label too long)
       *  expect: GETDNS_RETURN_BAD_DOMAIN_NAME
       */
       struct getdns_context *context = NULL;   
       struct getdns_dict *response = NULL;
       const char *name = "this.domain.hasalabelwhichexceedsthemaximumdnslabelsizeofsixtythreecharacters.com";

       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_general_sync(context, name, GETDNS_RRTYPE_A, NULL, &response), 
         GETDNS_RETURN_BAD_DOMAIN_NAME, "Return code from getdns_general_sync()");

       CONTEXT_DESTROY;
     }
     END_TEST
     
     START_TEST (getdns_general_sync_5)
     {
      /*
       *  response = NULL
       *  expect:  GETDNS_RETURN_INVALID_PARAMETER
       */
       struct getdns_context *context = NULL;   

       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_general_sync(context, "google.com", GETDNS_RRTYPE_A, NULL, NULL), 
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_general_sync()");

       CONTEXT_DESTROY;
     }
     END_TEST
     
     // START_TEST (getdns_general_sync_6)
     // {
     //  /*
     //   *  name = "google.com"
     //   *  request_type = 0 (minimum valid RRTYPE)
     //   *  expect: NOERROR/NODATA response:
     //   *    status = GETDNS_RESPSTATUS_NO_NAME
     //   *    rcode = 0
     //   *    ancount = 0 (number of records in ANSWER section)
     //   */
     //   struct getdns_context *context = NULL;   
     //   struct getdns_dict *response = NULL;
     // 
     //   CONTEXT_CREATE(TRUE);
     // 
     //   ASSERT_RC(getdns_general_sync(context, "google.com", 0, NULL, &response), 
     //     GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");
     // 
     //   EXTRACT_RESPONSE;
     // 
     //   assert_noerror(&ex_response);
     //   assert_nodata(&ex_response);
     // 
     //   CONTEXT_DESTROY;
     // }
     // END_TEST
     
     START_TEST (getdns_general_sync_7)
     {
      /*
       *  name = "nlnetlabs.nl"
       *  request_type = 65279 (maximum unassigned RRTYPE)
       *  expect: NOERROR/NODATA response:
       *    status = GETDNS_RESPSTATUS_NO_NAME
       *    rcode = 0
       *    ancount = 0 (number of records in ANSWER section)
       */
       struct getdns_context *context = NULL;   
       struct getdns_dict *response = NULL;
     
       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_general_sync(context, "nlnetlabs.nl", 65279, NULL, &response), 
         GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");

       EXTRACT_RESPONSE;

       assert_noerror(&ex_response);
       assert_nodata(&ex_response);

       CONTEXT_DESTROY;
     }
     END_TEST
     
     START_TEST (getdns_general_sync_8)
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
       struct getdns_context *context = NULL;   
       struct getdns_dict *response = NULL;
     
       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_general_sync(context, "google.com", GETDNS_RRTYPE_A, NULL, &response), 
         GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");

       EXTRACT_RESPONSE;

       assert_noerror(&ex_response);
       assert_address_in_answer(&ex_response, TRUE, FALSE);

       CONTEXT_DESTROY;
     }
     END_TEST
     
     START_TEST (getdns_general_sync_9)
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
       struct getdns_context *context = NULL;   
       struct getdns_dict *response = NULL;
     
       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_general_sync(context, "google.com", GETDNS_RRTYPE_AAAA, NULL, &response), 
         GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");

       EXTRACT_RESPONSE;

       assert_noerror(&ex_response);
       assert_address_in_answer(&ex_response, FALSE, TRUE);

       CONTEXT_DESTROY;
     }
     END_TEST
     
     START_TEST (getdns_general_sync_10)
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
       struct getdns_context *context = NULL;   
       struct getdns_dict *response = NULL;
       const char *name = "thisdomainsurelydoesntexist.com";
     
       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_general_sync(context, name, GETDNS_RRTYPE_TXT, NULL, &response), 
         GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");

       EXTRACT_RESPONSE;

       assert_nxdomain(&ex_response);
       assert_nodata(&ex_response);
       // Ubuntu 18.04 system resolver does not return an SOA
       //assert_soa_in_authority(&ex_response);

       CONTEXT_DESTROY;
     }
     END_TEST
     
     START_TEST (getdns_general_sync_11)
     {
      /*
       *  name = "d2a8n3.rootcanary.net" an unbound zone (as in no MX)
       *  request_type = GETDNS_RRTYPE_MX
       *  expect: NOERROR/NODATA response:
       *    status = GETDNS_RESPSTATUS_NO_NAME
       *    rcode = 0
       *    ancount = 0 (number of records in ANSWER section)
       */
       struct getdns_context *context = NULL;   
       struct getdns_dict *response = NULL;
     
       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_general_sync(context, "d2a8n3.rootcanary.net", GETDNS_RRTYPE_MX, NULL, &response), 
         GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");

       EXTRACT_RESPONSE;

       assert_noerror(&ex_response);
       assert_nodata(&ex_response);

       CONTEXT_DESTROY;
     }
     END_TEST
     
     START_TEST (getdns_general_sync_12)
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
       struct getdns_context *context = NULL;   
       struct getdns_dict *response = NULL;
     
       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_general_sync(context, "google.com", GETDNS_RRTYPE_A, NULL, &response), 
         GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");

       EXTRACT_RESPONSE;

       assert_noerror(&ex_response);
       assert_address_in_answer(&ex_response, TRUE, FALSE);

       CONTEXT_DESTROY;
     }
     END_TEST
     
     Suite *
     getdns_general_sync_suite (void)
     {
       Suite *s = suite_create ("getdns_general_sync()");
     
       /* Negative test caseis */
       TCase *tc_neg = tcase_create("Negative");
       tcase_add_test(tc_neg, getdns_general_sync_1);
       tcase_add_test(tc_neg, getdns_general_sync_2);
       tcase_add_test(tc_neg, getdns_general_sync_3);
       tcase_add_test(tc_neg, getdns_general_sync_4);
       tcase_add_test(tc_neg, getdns_general_sync_5);
       suite_add_tcase(s, tc_neg);
       /* Positive test cases */

       TCase *tc_pos = tcase_create("Positive");
       // Ubuntu 18.04 system resolver returns FORMERR for this query
       //tcase_add_test(tc_pos, getdns_general_sync_6);
       tcase_add_test(tc_pos, getdns_general_sync_7);
       tcase_add_test(tc_pos, getdns_general_sync_8);
       tcase_add_test(tc_pos, getdns_general_sync_9);
       tcase_add_test(tc_pos, getdns_general_sync_10);
       tcase_add_test(tc_pos, getdns_general_sync_11);
       tcase_add_test(tc_pos, getdns_general_sync_12);
       suite_add_tcase(s, tc_pos);
     
       return s;
     }

#endif
