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
#ifndef _check_getdns_list_get_length_h_
#define _check_getdns_list_get_length_h_

    /*
     **************************************************************************
     *                                                                        *
     *  T E S T S  F O R  G E T D N S _ L I S T _ G E T _ L E N G T H         *
     *                                                                        *
     **************************************************************************
    */

    START_TEST (getdns_list_get_length_1)
    {
     /*
      *  list = NULL
      *  expect = GETDNS_RETURN_INVALID_PARAMETER
      */
      struct getdns_list *list = NULL;
      size_t length;

      ASSERT_RC(getdns_list_get_length(list, &length),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_list_get_length()");

    }
    END_TEST
    
    START_TEST (getdns_list_get_length_2)
    {
     /*
      *  answer = NULL
      *  expect: GETDNS_RETURN_INVALID_PARAMETER
      */
      struct getdns_list *list = NULL;

      LIST_CREATE(list);

      ASSERT_RC(getdns_list_get_length(list, NULL),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_list_get_length()");

      LIST_DESTROY(list);
    }
    END_TEST
    
    START_TEST (getdns_list_get_length_3)
    {
     /*
      *  Create a list, add 3 ints to it, get the length.
      *  expect: GETDNS_RETURN_GOOD
      *          length = 3
      */
      struct getdns_list *list = NULL;
      size_t i;
      size_t length;

      LIST_CREATE(list);

      for(i = 0; i < 3; i++)
      {
        ASSERT_RC(getdns_list_set_int(list, i, i), GETDNS_RETURN_GOOD,
          "Return code from getdns_list_set_int()");
      }

      ASSERT_RC(getdns_list_get_length(list, &length),
        GETDNS_RETURN_GOOD, "Return code from getdns_list_get_length()");

      ck_assert_msg(length == 3, "Expected length == 3, got %d", (int)length);

      LIST_DESTROY(list);
    }
    END_TEST
    
    START_TEST (getdns_list_get_length_4)
    {
     /*
      *  Create a list (empty) and get the length
      *  expect: GETDNS_RETURN_GOOD
      *          length = 3
      */
      struct getdns_list *list = NULL;
      size_t length;

      LIST_CREATE(list);

      ASSERT_RC(getdns_list_get_length(list, &length),
        GETDNS_RETURN_GOOD, "Return code from getdns_list_get_length()");

      ck_assert_msg(length == 0, "Expected length == 3, got %d", (int)length);

      LIST_DESTROY(list);    
    }
    END_TEST
    
    Suite *
    getdns_list_get_length_suite (void)
    {
      Suite *s = suite_create ("getdns_list_get_length()");
    
      /* Negative test caseis */
      TCase *tc_neg = tcase_create("Negative");
      tcase_add_test(tc_neg, getdns_list_get_length_1);
      tcase_add_test(tc_neg, getdns_list_get_length_2);
      suite_add_tcase(s, tc_neg);
    
      /* Positive test cases */
      TCase *tc_pos = tcase_create("Positive");
      tcase_add_test(tc_pos, getdns_list_get_length_3);
      tcase_add_test(tc_pos, getdns_list_get_length_4);
      suite_add_tcase(s, tc_pos);
    
      return s;
    }

#endif
