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
#ifndef _check_getdns_hostname_h_
#define _check_getdns_hostname_h_

    /*
     **************************************************************************
     *                                                                        *
     *  T E S T S  F O R  G E T D N S _ H O S T N A M E                       *
     *                                                                        *
     **************************************************************************
    */

     START_TEST (getdns_hostname_1)
     {
      /*
       *  context = NULL
       *  expect: GETDNS_RETURN_INVALID_PARAMETER
       */
       struct getdns_context *context = NULL;
       struct getdns_dict *address = NULL;
       struct getdns_bindata address_type = { 5, (void *)"IPv4" };
       struct getdns_bindata address_data = { 4, (void *)"\x08\x08\x08\x08" };
       getdns_transaction_t transaction_id = 0;

       DICT_CREATE(address);
       ASSERT_RC(getdns_dict_set_bindata(address, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");
       ASSERT_RC(getdns_dict_set_bindata(address, "address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

       ASSERT_RC(getdns_hostname(context, address, NULL,
         NULL, &transaction_id, callbackfn),
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_hostname()");

       DICT_DESTROY(address);
     }
     END_TEST

     START_TEST (getdns_hostname_2)
     {
      /*
       *  address = NULL
       *  expect: GETDNS_RETURN_INVALID_PARAMETER
       */
       struct getdns_context *context = NULL;
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_hostname(context, NULL, NULL,
         NULL, &transaction_id, callbackfn),
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_hostname()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;
     }
     END_TEST

     START_TEST (getdns_hostname_3)
     {
      /*
       *  dict in address does not contain getdns_bindata
       *  expect:  GETDNS_RETURN_NO_SUCH_DICT_NAME
       */
       struct getdns_context *context = NULL;
       struct getdns_dict *address = NULL;
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);
       DICT_CREATE(address);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_hostname(context, address, NULL,
         NULL, &transaction_id, callbackfn),
         GETDNS_RETURN_NO_SUCH_DICT_NAME, "Return code from getdns_hostname()");

       RUN_EVENT_LOOP;
       DICT_DESTROY(address);
       CONTEXT_DESTROY;
     }
     END_TEST

     START_TEST (getdns_hostname_4)
     {
      /*
       *  dict in address does not contain two names
       *  expect: GETDNS_RETURN_NO_SUCH_DICT_NAME
       */
       struct getdns_context *context = NULL;
       struct getdns_dict *address = NULL;
       struct getdns_bindata address_type = { 5, (void *) "IPv4" };
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);

       DICT_CREATE(address);
       ASSERT_RC(getdns_dict_set_bindata(address, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_hostname(context, address, NULL,
         NULL, &transaction_id, callbackfn),
         GETDNS_RETURN_NO_SUCH_DICT_NAME, "Return code from getdns_hostname()");

       RUN_EVENT_LOOP;

       DICT_DESTROY(address);
       CONTEXT_DESTROY;
     }
     END_TEST

     START_TEST (getdns_hostname_5)
     {
      /*
       *  dict in address contains names other than adddress_type
       *  and address_data.
       *  expect:  GETDNS_RETURN_NO_SUCH_DICT_NAME
       */
       struct getdns_context *context = NULL;
       struct getdns_dict *address = NULL;
       struct getdns_bindata address_data = { 4, (void *)"\x08\x08\x08\x08" };
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);

       DICT_CREATE(address);
       ASSERT_RC(getdns_dict_set_int(address, "not_address_type", 100),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");
       ASSERT_RC(getdns_dict_set_bindata(address, "not_address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_hostname(context, address, NULL,
         NULL, &transaction_id, callbackfn),
         GETDNS_RETURN_NO_SUCH_DICT_NAME, "Return code from getdns_hostname()");

       RUN_EVENT_LOOP;

       DICT_DESTROY(address);
       CONTEXT_DESTROY;
     }
     END_TEST

     START_TEST (getdns_hostname_6)
     {
      /*
       *  dict in address contains names address_type
       *  and address_data but data type is not bindata
       *  expect:  GETDNS_RETURN_WRONG_TYPE_REQUESTED
       */
       struct getdns_context *context = NULL;
       struct getdns_dict *address = NULL;
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);

       DICT_CREATE(address);
       ASSERT_RC(getdns_dict_set_int(address, "address_type", 100),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");
       ASSERT_RC(getdns_dict_set_int(address, "address_data", 200),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");

       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_hostname(context, address, NULL,
         NULL, &transaction_id, callbackfn),
         GETDNS_RETURN_WRONG_TYPE_REQUESTED, "Return code from getdns_hostname()");

       RUN_EVENT_LOOP;

       DICT_DESTROY(address);
       CONTEXT_DESTROY;
     }
     END_TEST

     START_TEST (getdns_hostname_7)
     {
      /*
       *  dict in address contains invalid address_type
       *  expect:  GETDNS_RETURN_INVALID_PARAMETER
       */
       struct getdns_context *context = NULL;
       struct getdns_dict *address = NULL;
       struct getdns_bindata address_type = { 5, (void *)"IPv5" };
       struct getdns_bindata address_data = { 4, (void *)"\x08\x08\x08\x08" };
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);

       DICT_CREATE(address);
       ASSERT_RC(getdns_dict_set_bindata(address, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");
       ASSERT_RC(getdns_dict_set_bindata(address, "address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_hostname(context, address, NULL,
         NULL, &transaction_id, callbackfn),
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_hostname()");

       RUN_EVENT_LOOP;

       DICT_DESTROY(address);
       CONTEXT_DESTROY;
     }
     END_TEST

     START_TEST (getdns_hostname_8)
     {
      /*
       *  dict in address contains invalid address_data
       *  expect:  GETDNS_RETURN_INVALID_PARAMETER
       */
       struct getdns_context *context = NULL;
       struct getdns_dict *address = NULL;
       struct getdns_bindata address_type = { 5, (void *)"IPv4" };
       struct getdns_bindata address_data = { 5, (void *)"\x08\x08\x08\x08\x08" };
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);

       DICT_CREATE(address);
       ASSERT_RC(getdns_dict_set_bindata(address, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");
       ASSERT_RC(getdns_dict_set_bindata(address, "address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_hostname(context, address, NULL,
         NULL, &transaction_id, callbackfn),
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_hostname()");

       RUN_EVENT_LOOP;

       DICT_DESTROY(address);
       CONTEXT_DESTROY;
     }
     END_TEST

     START_TEST (getdns_hostname_9)
     {
      /*
       *  callbackfn = NULL
       *  expect:  GETDNS_RETURN_INVALID_PARAMETER
       */
       struct getdns_context *context = NULL;
       struct getdns_dict *address = NULL;
       struct getdns_bindata address_type = { 5, (void *)"IPv4" };
       struct getdns_bindata address_data = { 4, (void *)"\x08\x08\x08\x08" };
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);

       DICT_CREATE(address);
       ASSERT_RC(getdns_dict_set_bindata(address, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");
       ASSERT_RC(getdns_dict_set_bindata(address, "address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_hostname(context, address, NULL,
         NULL, &transaction_id, NULL),
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_hostname()");

       RUN_EVENT_LOOP;

       DICT_DESTROY(address);
       CONTEXT_DESTROY;
     }
     END_TEST

     START_TEST (getdns_hostname_10)
     {
      /*
       *  dict in address has resolvable IPv4 address
       *  expect:  response with correct hostname
       */
       void verify_getdns_hostname_10(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_hostname_10 };
       struct getdns_context *context = NULL;
       struct getdns_dict *address = NULL;
       struct getdns_bindata address_type = { 5, (void *)"IPv4" };
       struct getdns_bindata address_data = { 4, (void *)"\x08\x08\x08\x08" };
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);

       DICT_CREATE(address);
       ASSERT_RC(getdns_dict_set_bindata(address, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");
       ASSERT_RC(getdns_dict_set_bindata(address, "address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_hostname(context, address, NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_hostname()");

       RUN_EVENT_LOOP;

       DICT_DESTROY(address);
       CONTEXT_DESTROY;
     }
     END_TEST

     void verify_getdns_hostname_10(struct extracted_response *ex_response)
     {
       assert_noerror(ex_response);
       assert_ptr_in_answer(ex_response);
     }

     START_TEST (getdns_hostname_11)
     {
      /*
       *  dict in address has unresolvable IPv4 address
       *  expect:  response with no hostname
       */
       void verify_getdns_hostname_11(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_hostname_11 };
       struct getdns_context *context = NULL;
       struct getdns_dict *address = NULL;
       struct getdns_bindata address_type = { 5, (void *)"IPv4" };
       struct getdns_bindata address_data = { 4, (void *)"\x08\x08\x08\x00" };
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);

       DICT_CREATE(address);
       ASSERT_RC(getdns_dict_set_bindata(address, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");
       ASSERT_RC(getdns_dict_set_bindata(address, "address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_hostname(context, address, NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_hostname()");

       RUN_EVENT_LOOP;

       DICT_DESTROY(address);
       CONTEXT_DESTROY;
     }
     END_TEST

     void verify_getdns_hostname_11(struct extracted_response *ex_response)
     {
       assert_nxdomain(ex_response);
       assert_nodata(ex_response);
       // Ubuntu 18.04 system resolver does not return an SOA
       //assert_soa_in_authority(ex_response);
     }

     START_TEST (getdns_hostname_12)
     {
      /*
       *  dict in address has resolvable IPv6 address
       *  expect:  response with correct hostname
       */
       void verify_getdns_hostname_12(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_hostname_12 };
       struct getdns_context *context = NULL;
       struct getdns_dict *address = NULL;
       struct getdns_bindata address_type = { 5, (void *)"IPv6" };
       struct getdns_bindata address_data = { 16, (void *)"\x20\x01\x48\x60\x48\x60\x00\x00\x00\x00\x00\x00\x00\x00\x88\x88" };
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_context_set_timeout(context, 10000),
         GETDNS_RETURN_GOOD, "Return code from getdns_context_set_timeout()");

       DICT_CREATE(address);
       ASSERT_RC(getdns_dict_set_bindata(address, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");
       ASSERT_RC(getdns_dict_set_bindata(address, "address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_hostname(context, address, NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_hostname()");

       RUN_EVENT_LOOP;

       DICT_DESTROY(address);
       CONTEXT_DESTROY;
     }
     END_TEST

     void verify_getdns_hostname_12(struct extracted_response *ex_response)
     {
       assert_noerror(ex_response);
       assert_ptr_in_answer(ex_response);
     }

     START_TEST (getdns_hostname_13)
     {
      /*
       *  dict in address has unresolvable IPv4 address
       *  expect:  response with no hostname
       */
       void verify_getdns_hostname_13(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_hostname_13 };
       struct getdns_context *context = NULL;
       struct getdns_dict *address = NULL;
       struct getdns_bindata address_type = { 5, (void *)"IPv6" };
       struct getdns_bindata address_data = { 16, (void *)"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" };
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       CONTEXT_CREATE(TRUE);

       DICT_CREATE(address);
       ASSERT_RC(getdns_dict_set_bindata(address, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");
       ASSERT_RC(getdns_dict_set_bindata(address, "address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_hostname(context, address, NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_hostname()");

       RUN_EVENT_LOOP;

       DICT_DESTROY(address);
       CONTEXT_DESTROY;
     }
     END_TEST

     void verify_getdns_hostname_13(struct extracted_response *ex_response)
     {
       assert_nxdomain(ex_response);
       assert_nodata(ex_response);
       // Ubuntu 18.04 system resolver does not return an SOA
       //assert_soa_in_authority(ex_response);
     }

     Suite *
     getdns_hostname_suite (void)
     {
       Suite *s = suite_create ("getdns_hostname()");

       /* Negative test caseis */
       TCase *tc_neg = tcase_create("Negative");
       tcase_add_test(tc_neg, getdns_hostname_1);
       tcase_add_test(tc_neg, getdns_hostname_2);
       tcase_add_test(tc_neg, getdns_hostname_3);
       tcase_add_test(tc_neg, getdns_hostname_4);
       tcase_add_test(tc_neg, getdns_hostname_5);
       tcase_add_test(tc_neg, getdns_hostname_6);
       tcase_add_test(tc_neg, getdns_hostname_7);
       tcase_add_test(tc_neg, getdns_hostname_8);
       tcase_add_test(tc_neg, getdns_hostname_9);
       suite_add_tcase(s, tc_neg);

       /* Positive test cases */
       TCase *tc_pos = tcase_create("Positive");
       tcase_set_timeout(tc_pos, 10.0);
       tcase_add_test(tc_pos, getdns_hostname_10);
       tcase_add_test(tc_pos, getdns_hostname_11);
       tcase_add_test(tc_pos, getdns_hostname_12);
       tcase_add_test(tc_pos, getdns_hostname_13);
       suite_add_tcase(s, tc_pos);

       return s;
     }

#endif
