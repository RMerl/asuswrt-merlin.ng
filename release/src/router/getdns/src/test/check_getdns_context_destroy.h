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
#ifndef _check_getdns_context_destroy_h_
#define _check_getdns_context_destroy_h_

#include <signal.h>

    /*
     **************************************************************************
     *                                                                        *
     *  T E S T S  F O R  G E T D N S _ C O N T E X T _ D E S T R O Y         *
     *                                                                        *
     **************************************************************************
    */

     START_TEST (getdns_context_destroy_1)
     {
      /*
       *  context = NULL
       *  expect: nothing, no segmentation fault
       */

       getdns_context_destroy(NULL);
     }
     END_TEST

     START_TEST (getdns_context_destroy_2)
     {
      /*
       *  destroy called with valid context and no outstanding transactions
       *  expect: nothing, context is freed
       */
       struct getdns_context *context = NULL;

       CONTEXT_CREATE(TRUE);
       CONTEXT_DESTROY;
     }
     END_TEST

     START_TEST (getdns_context_destroy_3)
     {
      /*
       *  destroy called immediately following getdns_general
       *  expect: callback should be called before getdns_context_destroy() returns
       */
       void verify_getdns_context_destroy(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_context_destroy };
       struct getdns_context *context = NULL;
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       callback_called = 0;	/* Initialize counter */

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_general(context, "google.com", GETDNS_RRTYPE_A, NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_general()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;

       ck_assert_msg(callback_called == 1, "callback_called should == 1, got %d", callback_called);
     }
     END_TEST

     START_TEST (getdns_context_destroy_4)
     {
      /*
       *  destroy called immediately following getdns_address
       *  expect: callback should be called before getdns_context_destroy() returns
       */
       void verify_getdns_context_destroy(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_context_destroy };
       struct getdns_context *context = NULL;
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       callback_called = 0;     /* Initialize counter */

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_address(context, "google.com", NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_address()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;

       ck_assert_msg(callback_called == 1, "callback_called should == 1, got %d", callback_called);
     }
     END_TEST

     START_TEST (getdns_context_destroy_5)
     {
      /*
       *  destroy called immediately following getdns_address
       *  expect: callback should be called before getdns_context_destroy() returns
       */
       void verify_getdns_context_destroy(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_context_destroy };
       struct getdns_context *context = NULL;
       void* eventloop = NULL;
       struct getdns_bindata address_type = { 5, (void *)"IPv4" };
       struct getdns_bindata address_data = { 4, (void *)"\x08\x08\x08\x08" };
       struct getdns_dict *address = NULL;
       getdns_transaction_t transaction_id = 0;

       callback_called = 0;     /* Initialize counter */

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       DICT_CREATE(address);
       ASSERT_RC(getdns_dict_set_bindata(address, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata");
       ASSERT_RC(getdns_dict_set_bindata(address, "address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata");

       ASSERT_RC(getdns_hostname(context, address, NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_address()");

       RUN_EVENT_LOOP;
       DICT_DESTROY(address);
       CONTEXT_DESTROY;

       ck_assert_msg(callback_called == 1, "callback_called should == 1, got %d", callback_called);
     }
     END_TEST

     START_TEST (getdns_context_destroy_6)
     {
      /*
       *  destroy called immediately following getdns_address
       *  expect: callback should be called before getdns_context_destroy() returns
       */
       void verify_getdns_context_destroy(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_context_destroy };
       struct getdns_context *context = NULL;
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       callback_called = 0;     /* Initialize counter */

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_service(context, "google.com", NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_service()");

       RUN_EVENT_LOOP;
       CONTEXT_DESTROY;

       ck_assert_msg(callback_called == 1, "callback_called should == 1, got %d", callback_called);
     }
     END_TEST

#if 0
     START_TEST (getdns_context_destroy_7)
     {
      /*
       *  destroy called immediately following getdns_address
       *  expect: callback should be called before getdns_context_destroy() returns
       */
       struct getdns_context *context = NULL;
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       int flag = 0;     /* Initialize flag */

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_address(context, "google.com", NULL,
         &flag, &transaction_id, destroy_callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_address()");

       RUN_EVENT_LOOP;

       CONTEXT_DESTROY;
       ck_assert_msg(flag == 1, "flag should == 1, got %d", flag);
     }
     END_TEST

     START_TEST (getdns_context_destroy_8)
     {
      /*
       *  destroy called immediately following getdns_address
       *  expect: callback should be called before getdns_context_destroy() returns
       */
       struct getdns_context *context = NULL;
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       int flag = 0;     /* Initialize flag */

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_address(context, "google.com", NULL,
         &flag, &transaction_id, destroy_callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_address()");
       getdns_cancel_callback(context, transaction_id);

       RUN_EVENT_LOOP;

       CONTEXT_DESTROY;
       ck_assert_msg(flag == 1, "flag should == 1, got %d", flag);
     }
     END_TEST

     START_TEST (getdns_context_destroy_9)
     {
      /*
       *  destroy called immediately following getdns_address
       *  expect: callback should be called before getdns_context_destroy() returns
       */
       struct getdns_context *context = NULL;
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       int flag = 0;     /* Initialize flag */

       CONTEXT_CREATE(TRUE);
       // set timeout to something unreasonable
       getdns_context_set_timeout(context, 1);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_address(context, "google.com", NULL,
         &flag, &transaction_id, destroy_callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_address()");
       ASSERT_RC(getdns_address(context, "getdnsapi.net", NULL,
         &flag, &transaction_id, destroy_callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_address()");

       RUN_EVENT_LOOP;

       CONTEXT_DESTROY;
       ck_assert_msg(flag == 1, "flag should == 1, got %d", flag);
     }
     END_TEST
#endif

     void verify_getdns_context_destroy(struct extracted_response *ex_response)
     {
       (void)ex_response; /* unused parameter */
       /*
        * Sleep for a second to make getdns_context_destroy() wait.
        */
       sleep(1);

       /*
        *  callback_called is a global and we increment it
        *  here to show that the callback was called.
        */
       callback_called++;
     }

     Suite *
     getdns_context_destroy_suite (void)
     {
       Suite *s = suite_create ("getdns_context_destroy()");

       /* Negative test caseis */
       TCase *tc_neg = tcase_create("Negative");
       tcase_add_test(tc_neg, getdns_context_destroy_1);
       suite_add_tcase(s, tc_neg);

       /* Positive test cases */
       TCase *tc_pos = tcase_create("Positive");
       tcase_add_test(tc_pos, getdns_context_destroy_2);
       tcase_add_test(tc_pos, getdns_context_destroy_3);
       tcase_add_test(tc_pos, getdns_context_destroy_4);
       tcase_add_test(tc_pos, getdns_context_destroy_5);
       tcase_add_test(tc_pos, getdns_context_destroy_6);
#if 0
       tcase_add_test_raise_signal(tc_pos, getdns_context_destroy_7, SIGABRT);
       tcase_add_test_raise_signal(tc_pos, getdns_context_destroy_8, SIGABRT);
       tcase_add_test_raise_signal(tc_pos, getdns_context_destroy_9, SIGABRT);
#endif
       suite_add_tcase(s, tc_pos);

       return s;
     }

#endif
