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
#ifndef _check_getdns_service_sync_h_
#define _check_getdns_service_sync_h_

    /*
     *************************************************************
     *                                                           *
     *  T E S T S  F O R  G E T D N S _ S E R V I C E _ S Y N C  * 
     *                                                           *
     *************************************************************
    */

     START_TEST (getdns_service_sync_1)
     {
      /*
       *  context = NULL
       *  expect: GETDNS_RETURN_INVALID_PARAMETER
       */
       struct getdns_context *context = NULL;   
       struct getdns_dict *response = NULL;

       ASSERT_RC(getdns_service_sync(context, "google.com", NULL, &response), 
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_service_sync()");
     }
     END_TEST

     START_TEST (getdns_service_sync_2)
     {
      /*
       *  name = NULL
       *  expect: GETDNS_RETURN_INVALID_PARAMETER
       */
       struct getdns_context *context = NULL;   
       struct getdns_dict *response = NULL;

       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_service_sync(context, NULL, NULL, &response), 
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_service_sync()");
     }
     END_TEST

     START_TEST (getdns_service_sync_3)
     {
      /*
       *  name is invalid (domain name length > 255)
       *  expect: GETDNS_RETURN_BAD_DOMAIN_NAME
       */
       struct getdns_context *context = NULL;   
       struct getdns_dict *response = NULL;
       const char *name = "oh.my.gosh.and.for.petes.sake.are.you.fricking.crazy.man.because.this.spectacular.and.elaborately.thought.out.domain.name.of.very.significant.length.is.just.too.darn.long.because.you.know.the rfc.states.that.two.hundred.fifty.five.characters.is.the.max.com";

       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_service_sync(context, name, NULL, &response), 
         GETDNS_RETURN_BAD_DOMAIN_NAME, "Return code from getdns_service_sync()");
     }
     END_TEST

     START_TEST (getdns_service_sync_4)
     {
      /*
       *  name is invalid (domain name label length > 63)
       *  expect: GETDNS_RETURN_BAD_DOMAIN_NAME
       */
       struct getdns_context *context = NULL;   
       struct getdns_dict *response = NULL;
       const char *name = "this.domain.hasalabelwhichexceedsthemaximumdnslabelsizeofsixtythreecharacters.com";

       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_service_sync(context, name, NULL, &response), 
         GETDNS_RETURN_BAD_DOMAIN_NAME, "Return code from getdns_service_sync()");
     }
     END_TEST

     START_TEST (getdns_service_sync_5)
     {
      /*
       *  response is NULL
       *  expect: GETDNS_RETURN_INVALID_PARAMETER
       */
       struct getdns_context *context = NULL;   

       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_service_sync(context, "google.com", NULL, NULL), 
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_service_sync()");
     }
     END_TEST

     START_TEST (getdns_service_sync_7)
     {
      /*
       *  rname is <non-existent domain name> (NXDOMAIN)
       *  no extensions
       *  expected: NXDOMAIN response (with SOA record)

       */
       struct getdns_context *context = NULL;   
       struct getdns_dict *response = NULL;
       const char *name = "labelsizeofsixtythreecharacterscom.";

       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_service_sync(context, name, NULL, &response), 
         GETDNS_RETURN_GOOD, "Return code from getdns_service_sync()");

       EXTRACT_RESPONSE;

       assert_nxdomain(&ex_response);
       assert_nodata(&ex_response);
       // Ubuntu 18.04 system resolver does not return an SOA
       //assert_soa_in_authority(&ex_response);
     }
     END_TEST



     
     Suite *
     getdns_service_sync_suite (void)
     {
       Suite *s = suite_create ("getdns_service_sync()");
     
       /* Negative test caseis */
       TCase *tc_neg = tcase_create("Negative");
       tcase_add_test(tc_neg, getdns_service_sync_1);
       tcase_add_test(tc_neg, getdns_service_sync_2);
       tcase_add_test(tc_neg, getdns_service_sync_3);
       tcase_add_test(tc_neg, getdns_service_sync_4);
       tcase_add_test(tc_neg, getdns_service_sync_5);
       
       suite_add_tcase(s, tc_neg);
     
       /* Positive test cases */
       TCase *tc_pos = tcase_create("Positive");
       tcase_add_test(tc_pos, getdns_service_sync_7);
       suite_add_tcase(s, tc_pos);
     
       return s;
     }

#endif
