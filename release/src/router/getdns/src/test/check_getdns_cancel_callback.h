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
#ifndef _check_getdns_cancel_callback_h_
#define _check_getdns_cancel_callback_h_

    /*
     **************************************************************************
     *                                                                        *
     *  T E S T S  F O R  G E T D N S _ C A N C E L _ C A L L B A C K         *
     *                                                                        *
     **************************************************************************
    */

    START_TEST (getdns_cancel_callback_1)
    {
      /*
       *  context = NULL
       *  expect: GETDNS_RETURN_INVALID_PARAMETER
       */
       getdns_transaction_t transaction_id = 0;

       ASSERT_RC(getdns_cancel_callback(NULL, transaction_id),
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_cancel_callback()");
    }
    END_TEST

    START_TEST (getdns_cancel_callback_2)
    {
      /*
       *  transaction_id corresponds to callback that has already been called
       *  expect: GETDNS_RETURN_UNKNOWN_TRANSACTION
       */
       void verify_getdns_cancel_callback(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_cancel_callback };
       struct getdns_context *context = NULL;
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       callback_called = 0;     /* Initialize counter */

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_general(context, "google.com", GETDNS_RRTYPE_A, NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_general()");

       RUN_EVENT_LOOP;

       ck_assert_msg(callback_called == 1, "callback_called should == 1, got %d", callback_called);

       ASSERT_RC(getdns_cancel_callback(context, transaction_id),
         GETDNS_RETURN_UNKNOWN_TRANSACTION, "Return code from getdns_cancel_callback()");

       CONTEXT_DESTROY;
    }
    END_TEST

    START_TEST (getdns_cancel_callback_3)
    {
      /*
       *  transaction_id is unknown
       *  expect: GETDNS_RETURN_UNKNOWN_TRANSACTION
       */
       void verify_getdns_cancel_callback(struct extracted_response *ex_response);
       fn_cont fn_ref = { verify_getdns_cancel_callback };
       struct getdns_context *context = NULL;
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;

       callback_called = 0;     /* Initialize counter */

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       ASSERT_RC(getdns_general(context, "google.com", GETDNS_RRTYPE_A, NULL,
         &fn_ref, &transaction_id, callbackfn),
         GETDNS_RETURN_GOOD, "Return code from getdns_general()");

       RUN_EVENT_LOOP;

       ck_assert_msg(callback_called == 1, "callback_called should == 1, got %d", callback_called);

       transaction_id++;
       ASSERT_RC(getdns_cancel_callback(context, transaction_id),
         GETDNS_RETURN_UNKNOWN_TRANSACTION, "Return code from getdns_cancel_callback()");

       CONTEXT_DESTROY;
    }
    END_TEST

    START_TEST (getdns_cancel_callback_4)
    {
      /*
       *  getdns_cancel_callback() called with transaction_id returned from getdns_general()
       *
       *  if transaction_id is odd, callback is canceled before event loop
       *    expect:  GETDNS_RETURN_GOOD
       *  if transaction_id is even, callback is canceled after event loop
       *    expect:  GETDNS_RETURN_UNKNOWN_TRANSACTION
       *
       *  expect: callback to be called with GETDNS_CALLBACK_CANCELED (if canceled)
       *          or GETDNS_CALLBACK_COMPLETE (if not canceled).
       */
       void cancel_callbackfn(
               struct getdns_context *context,
               getdns_callback_type_t callback_type,
               struct getdns_dict *response,
               void *userarg,
               getdns_transaction_t transaction_id);
       struct getdns_context *context = NULL;
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;
       getdns_transaction_t transaction_id_array[10]
	       = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
       int i;
       int odd = 0;
       int even = 0;

      /*
       *  Initialize counters
       */
       callback_called = 0;
       callback_completed = 0;
       callback_canceled = 0;

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       for(i = 0; i < 10; i++)
       {
         ASSERT_RC(getdns_general(context, "google.com", GETDNS_RRTYPE_A, NULL,
           NULL, &transaction_id, cancel_callbackfn),
           GETDNS_RETURN_GOOD, "Return code from getdns_general()");

         transaction_id_array[i] = transaction_id;

         /*
          *  Cancel callback if transaction_id is odd which should be accepted
          */
         if(transaction_id % 2)
         {
           odd++;
           ASSERT_RC(getdns_cancel_callback(context, transaction_id),
             GETDNS_RETURN_GOOD, "Return code from getdns_cancel_callback()");
         }
       }

       RUN_EVENT_LOOP;

       /*
        *  Cancel the callback for even transaction_ids which should be complete
        */
       for(i = 0; i < 10; i++)
       {
         if((transaction_id_array[i] % 2) == 0)
         {
           even++;
           ASSERT_RC(getdns_cancel_callback(context, transaction_id_array[i]),
             GETDNS_RETURN_UNKNOWN_TRANSACTION, "Return code from getdns_cancel_callback()");
         }
       }

       ck_assert_msg(callback_called == 10, "callback_called should == 10, got: %d", callback_called);
       ck_assert_msg(callback_completed == even, "callback_completed should == %d, got: %d", even, callback_completed);
       ck_assert_msg(callback_canceled == odd, "callback_canceled should == %d, got: %d", odd, callback_canceled);

       CONTEXT_DESTROY;
    }
    END_TEST

    START_TEST (getdns_cancel_callback_5)
    {
      /*
       *  getdns_cancel_callback() called with transaction_id returned from getdns_address()
       *
       *  if transaction_id is odd, callback is canceled before event loop
       *    expect:  GETDNS_RETURN_GOOD
       *  if transaction_id is even, callback is canceled after event loop
       *    expect:  GETDNS_RETURN_UNKNOWN_TRANSACTION
       *
       *  expect: callback to be called with GETDNS_CALLBACK_CANCELED (if canceled)
       *          or GETDNS_CALLBACK_COMPLETE (if not canceled).
       */
       void cancel_callbackfn(
               struct getdns_context *context,
               getdns_callback_type_t callback_type,
               struct getdns_dict *response,
               void *userarg,
               getdns_transaction_t transaction_id);
       struct getdns_context *context = NULL;
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;
       getdns_transaction_t transaction_id_array[10]
	       = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
       int i;
       int odd = 0;
       int even = 0;

      /*
       *  Initialize counters
       */
       callback_called = 0;
       callback_completed = 0;
       callback_canceled = 0;


       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       for(i = 0; i < 10; i++)
       {
         ASSERT_RC(getdns_address(context, "google.com", NULL,
           NULL, &transaction_id, cancel_callbackfn),
           GETDNS_RETURN_GOOD, "Return code from getdns_address()");

         transaction_id_array[i] = transaction_id;

         /*
          *  Cancel callback if transaction_id is odd which should be accepted
          */
         if(transaction_id % 2)
         {
           odd++;
           ASSERT_RC(getdns_cancel_callback(context, transaction_id),
             GETDNS_RETURN_GOOD, "Return code from getdns_cancel_callback()");
         }
       }

       RUN_EVENT_LOOP;

       /*
        *  Cancel the callback for even transaction_ids which should be complete
        */
       for(i = 0; i < 10; i++)
       {
         if((transaction_id_array[i] % 2) == 0)
         {
           even++;
           ASSERT_RC(getdns_cancel_callback(context, transaction_id_array[i]),
             GETDNS_RETURN_UNKNOWN_TRANSACTION, "Return code from getdns_cancel_callback()");
         }
       }

       ck_assert_msg(callback_called == 10, "callback_called should == 10, got: %d", callback_called);
       ck_assert_msg(callback_completed == even, "callback_completed should == %d, got: %d", even, callback_completed);
       ck_assert_msg(callback_canceled == odd, "callback_canceled should == %d, got: %d", odd, callback_canceled);

       CONTEXT_DESTROY;
    }
    END_TEST

    START_TEST (getdns_cancel_callback_6)
    {
      /*
       *  getdns_cancel_callback() called with transaction_id returned from getdns_hostname()
       *
       *  if transaction_id is odd, callback is canceled before event loop
       *    expect:  GETDNS_RETURN_GOOD
       *  if transaction_id is even, callback is canceled after event loop
       *    expect:  GETDNS_RETURN_UNKNOWN_TRANSACTION
       *
       *  expect: callback to be called with GETDNS_CALLBACK_CANCELED (if canceled)
       *          or GETDNS_CALLBACK_COMPLETE (if not canceled).
       */
       void cancel_callbackfn(
               struct getdns_context *context,
               getdns_callback_type_t callback_type,
               struct getdns_dict *response,
               void *userarg,
               getdns_transaction_t transaction_id);
       struct getdns_context *context = NULL;
       void* eventloop = NULL;
       struct getdns_bindata address_type = { 5, (void *)"IPv4" };
       struct getdns_bindata address_data = { 4, (void *)"\x08\x08\x08\x08" };
       struct getdns_dict *address = NULL;
       getdns_transaction_t transaction_id = 0;
       getdns_transaction_t transaction_id_array[10]
	       = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
       int i;
       int odd = 0;
       int even = 0;

      /*
       *  Initialize counters
       */
       callback_called = 0;
       callback_completed = 0;
       callback_canceled = 0;

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       DICT_CREATE(address);
       ASSERT_RC(getdns_dict_set_bindata(address, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata");
       ASSERT_RC(getdns_dict_set_bindata(address, "address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata");

       for(i = 0; i < 10; i++)
       {
         ASSERT_RC(getdns_hostname(context, address, NULL,
           NULL, &transaction_id, cancel_callbackfn),
           GETDNS_RETURN_GOOD, "Return code from getdns_address()");

         transaction_id_array[i] = transaction_id;

         /*
          *  Cancel callback if transaction_id is odd which should be accepted
          */
         if(transaction_id % 2)
         {
           odd++;
           ASSERT_RC(getdns_cancel_callback(context, transaction_id),
             GETDNS_RETURN_GOOD, "Return code from getdns_cancel_callback()");
         }
       }

       RUN_EVENT_LOOP;

       /*
        *  Cancel the callback for even transaction_ids which should be complete
        */
       for(i = 0; i < 10; i++)
       {
         if((transaction_id_array[i] % 2) == 0)
         {
           even++;
           ASSERT_RC(getdns_cancel_callback(context, transaction_id_array[i]),
             GETDNS_RETURN_UNKNOWN_TRANSACTION, "Return code from getdns_cancel_callback()");
         }
       }

       ck_assert_msg(callback_called == 10, "callback_called should == 10, got: %d", callback_called);
       ck_assert_msg(callback_completed == even, "callback_completed should == %d, got: %d", even, callback_completed);
       ck_assert_msg(callback_canceled == odd, "callback_canceled should == %d, got: %d", odd, callback_canceled);

       DICT_DESTROY(address);
       CONTEXT_DESTROY;
    }
    END_TEST

    START_TEST (getdns_cancel_callback_7)
    {
      /*
       *  getdns_cancel_callback() called with transaction_id returned from getdns_service()
       *
       *  if transaction_id is odd, callback is canceled before event loop
       *    expect:  GETDNS_RETURN_GOOD
       *  if transaction_id is even, callback is canceled after event loop
       *    expect:  GETDNS_RETURN_UNKNOWN_TRANSACTION
       *
       *  expect: callback to be called with GETDNS_CALLBACK_CANCELED (if canceled)
       *          or GETDNS_CALLBACK_COMPLETE (if not canceled).
       */
       void cancel_callbackfn(
               struct getdns_context *context,
               getdns_callback_type_t callback_type,
               struct getdns_dict *response,
               void *userarg,
               getdns_transaction_t transaction_id);
       struct getdns_context *context = NULL;
       void* eventloop = NULL;
       getdns_transaction_t transaction_id = 0;
       getdns_transaction_t transaction_id_array[10]
	       = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
       int i;
       int odd = 0;
       int even = 0;

      /*
       *  Initialize counters
       */
       callback_called = 0;
       callback_completed = 0;
       callback_canceled = 0;

       CONTEXT_CREATE(TRUE);
       EVENT_BASE_CREATE;

       for(i = 0; i < 10; i++)
       {
         ASSERT_RC(getdns_service(context, "google.com", NULL,
           NULL, &transaction_id, cancel_callbackfn),
           GETDNS_RETURN_GOOD, "Return code from getdns_service()");

         transaction_id_array[i] = transaction_id;

         /*
          *  Cancel callback if transaction_id is odd which should be accepted
          */
         if(transaction_id % 2)
         {
           odd++;
           ASSERT_RC(getdns_cancel_callback(context, transaction_id),
             GETDNS_RETURN_GOOD, "Return code from getdns_cancel_callback()");
         }
       }

       RUN_EVENT_LOOP;

       /*
        *  Cancel the callback for even transaction_ids which should be complete
        */
       for(i = 0; i < 10; i++)
       {
         if((transaction_id_array[i] % 2) == 0)
         {
           even++;
           ASSERT_RC(getdns_cancel_callback(context, transaction_id_array[i]),
             GETDNS_RETURN_UNKNOWN_TRANSACTION, "Return code from getdns_cancel_callback()");
         }
       }

       ck_assert_msg(callback_called == 10, "callback_called should == 10, got: %d", callback_called);
       ck_assert_msg(callback_completed == even, "callback_completed should == %d, got: %d", even, callback_completed);
       ck_assert_msg(callback_canceled == odd, "callback_canceled should == %d, got: %d", odd, callback_canceled);

       CONTEXT_DESTROY;
    }
    END_TEST

    /*
     *  Callback function for getdns_cancel_callback() tests
     *
     *  callback_type should be GETDNS_CALLBACK_CANCEL for odd transaction_ids
     *                should be GETDNS_CALLBACK_COMPLETE for even transaction_ids
     */
     void cancel_callbackfn(
         struct getdns_context *context,
         getdns_callback_type_t callback_type,
         struct getdns_dict *response,
         void *userarg,
         getdns_transaction_t transaction_id)
     {
       (void)context; (void)response; (void)userarg; /* unused parameters */

       callback_called++;

       if(callback_type == GETDNS_CALLBACK_CANCEL)
       {
         callback_canceled++;
         ck_assert_msg(transaction_id % 2,
           "Only callbacks with odd transaction_ids were canceled, this one is even: %d",
           (int)transaction_id);
       }
       else if(callback_type == GETDNS_CALLBACK_COMPLETE)
       {
         callback_completed++;
         ck_assert_msg((transaction_id % 2) == 0,
           "One callbacks with even transaction_ids should complete, this one is odd: %d",
           (int)transaction_id);
       }
       else
       {
         if(transaction_id % 2)
           ck_abort_msg("callback_type should == GETDNS_CALLBACK_CANCEL for odd transaction_id (%d), got: %d",
             (int)transaction_id, (int)callback_type);
         else
           ck_abort_msg("callback_type should == GETDNS_CALLBACK_COMPLETE for even transaction_id (%d), got %d",
             (int)transaction_id, (int)callback_type);
       }
     }

    /*
     *  Function passed via userarg to async functions for cancel callback tests
     */
     void verify_getdns_cancel_callback(struct extracted_response *ex_response)
     {
       (void)ex_response; /* unused parameter */
       /*
        *  increment callback_called global to prove callback was called.
        */
       callback_called++;
     }

     Suite *
     getdns_cancel_callback_suite (void)
     {
       Suite *s = suite_create ("getdns_cancel_callback()");

       /* Negative test caseis */
       TCase *tc_neg = tcase_create("Negative");
       tcase_add_test(tc_neg, getdns_cancel_callback_1);
       tcase_add_test(tc_neg, getdns_cancel_callback_2);
       tcase_add_test(tc_neg, getdns_cancel_callback_3);
       suite_add_tcase(s, tc_neg);

       /* Positive test cases */
       TCase *tc_pos = tcase_create("Positive");
       tcase_add_test(tc_pos, getdns_cancel_callback_4);
       tcase_add_test(tc_pos, getdns_cancel_callback_5);
       tcase_add_test(tc_pos, getdns_cancel_callback_6);
       tcase_add_test(tc_pos, getdns_cancel_callback_7);

       suite_add_tcase(s, tc_pos);
       return s;
     }

#endif
