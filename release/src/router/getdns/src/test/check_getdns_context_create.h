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
#ifndef _check_getdns_context_create_h_
#define _check_getdns_context_create_h_

    /*
     **************************************************************************
     *                                                                        *
     *  T E S T S  F O R  G E T D N S _ C O N T E X T _ C R E A T E           *
     *                                                                        *
     **************************************************************************
    */

     START_TEST (getdns_context_create_1)
     {
      /*
       *  context = NULL
       *  expect: GETDNS_RETURN_INVALID_PARAMETER
       */

       ASSERT_RC(getdns_context_create(NULL, TRUE),
         GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_context_create()");
     }
     END_TEST
     
     START_TEST (getdns_context_create_2)
     {
      /*
       *  set_from_os = TRUE
       *  expect: context initialized with operating system info
       *          GETDNS_RETURN_GOOD
       */
       struct getdns_context *context = NULL;
       getdns_append_name_t append_name;

       CONTEXT_CREATE(TRUE);
       //  TODO:  Do something here to verify set_from_os = TRUE 
       getdns_context_set_append_name(context, GETDNS_APPEND_NAME_TO_SINGLE_LABEL_FIRST);
       getdns_context_get_append_name(context, &append_name);
       ck_assert_msg(append_name == GETDNS_APPEND_NAME_TO_SINGLE_LABEL_FIRST, "suffix is not correct");
       CONTEXT_DESTROY;
     }
     END_TEST
     
     START_TEST (getdns_context_create_3)
     {
      /*
       *  set_from_os = FALSE
       *  expect: context is not initialized with operating system info
       *          GETDNS_RETURN_GOOD
       */
       struct getdns_context *context = NULL;

       CONTEXT_CREATE(FALSE);
       //  TODO:  Do something here to verify set_from_os = TRUE 
       CONTEXT_DESTROY;
     }
     END_TEST

     Suite *
     getdns_context_create_suite (void)
     {
       Suite *s = suite_create ("getdns_context_create()");
     
       /* Negative test caseis */
       TCase *tc_neg = tcase_create("Negative");
       tcase_add_test(tc_neg, getdns_context_create_1);
       suite_add_tcase(s, tc_neg);

       /* Positive test cases */
       TCase *tc_pos = tcase_create("Positive");
       tcase_add_test(tc_pos, getdns_context_create_2);
       tcase_add_test(tc_pos, getdns_context_create_3);
       suite_add_tcase(s, tc_pos);
     
       return s;
     }

#endif
