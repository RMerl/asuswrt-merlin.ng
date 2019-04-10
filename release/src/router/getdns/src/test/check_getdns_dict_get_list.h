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
#ifndef _check_getdns_dict_get_list_h_
#define _check_getdns_dict_get_list_h_

    /*
     **************************************************************************
     *                                                                        *
     *  T E S T S  F O R  G E T D N S _ D I C T _ G E T _ L I S T             *
     *                                                                        *
     **************************************************************************
    */

    START_TEST (getdns_dict_get_list_1)
    {
     /*
      *  this_dict = NULL
      *  expect:  GETDNS_RETURN_INVALID_PARAMETER
      */
      struct getdns_dict *this_dict = NULL;
      struct getdns_list *answer = NULL;

      ASSERT_RC(getdns_dict_get_list(this_dict, "key", &answer),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_dict_get_list()");

    }
    END_TEST

    START_TEST (getdns_dict_get_list_2)
    {
     /*
      *  name = NULL
      *  expect:  GETDNS_RETURN_INVALID_PARAMETER
      */
      struct getdns_dict *this_dict = NULL;
      struct getdns_list *list = NULL;
      struct getdns_list *answer = NULL;

      DICT_CREATE(this_dict);
      LIST_CREATE(list);
      ASSERT_RC(getdns_dict_set_list(this_dict, "list", list),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_list()");

      ASSERT_RC(getdns_dict_get_list(this_dict, NULL, &answer),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_dict_get_list()");

      DICT_DESTROY(this_dict);
      LIST_DESTROY(list);
    }
    END_TEST
    
    START_TEST (getdns_dict_get_list_3)
    {
     /*
      *  name does not exist in dict
      *  Create a dict with three ints ("ten" = 10, "eleven" = 11, "twelve" = 12)
      *  Call getdns_dict_get_list() against the first dict with name = "nine"
      *  expect:  GETDNS_RETURN_NO_SUCH_DICT_NAME
      */
      struct getdns_dict *this_dict = NULL;
      char *keys[3] = { "ten", "eleven", "twelve" };
      uint32_t values[3] = { 10, 11, 12 };
      int i;
      struct getdns_list *answer = NULL;

      DICT_CREATE(this_dict);

      for(i = 0; i < 3; i++)
      {
        ASSERT_RC(getdns_dict_set_int(this_dict, keys[i], values[i]), 
          GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");
      }

      ASSERT_RC(getdns_dict_get_list(this_dict, "nine", &answer),
        GETDNS_RETURN_NO_SUCH_DICT_NAME, "Return code from getdns_dict_get_list()");

      DICT_DESTROY(this_dict);
    }
    END_TEST
    
    START_TEST (getdns_dict_get_list_4)
    {
     /*
      *  data type at name is not a list
      *  Create a dict with one int (name = "ten", value = 10)
      *  Call getdns_dict_get_list() with name = "ten"
      *  expect:  GETDNS_RETURN_WRONG_TYPE_REQUESTED
      */
      struct getdns_dict *this_dict = NULL;
      struct getdns_list *answer = NULL;

      DICT_CREATE(this_dict);

      ASSERT_RC(getdns_dict_set_int(this_dict, "ten", 10), 
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");

      ASSERT_RC(getdns_dict_get_list(this_dict, "ten", &answer),
        GETDNS_RETURN_WRONG_TYPE_REQUESTED, "Return code from getdns_dict_get_list()");

      DICT_DESTROY(this_dict);
    }
    END_TEST
    
    START_TEST (getdns_dict_get_list_5)
    {
     /*
      *  answer = NULL
      *  expect: GETDNS_RETURN_INVALID_PARAMETER
      */
      struct getdns_dict *this_dict = NULL;
      struct getdns_list *list = NULL;

      DICT_CREATE(this_dict);
      LIST_CREATE(list);

      ASSERT_RC(getdns_dict_set_list(this_dict, "list", list),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_list()");

      ASSERT_RC(getdns_dict_get_list(this_dict, "list", NULL),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_dict_get_list()");

      DICT_DESTROY(this_dict);
      LIST_DESTROY(list);
    }
    END_TEST
    
    START_TEST (getdns_dict_get_list_6)
    {
     /*
      *  successful get list
      *  Create a dict
      *  Create a list and set index 0 to an int with a value of 100
      *  Add the list to the dict as name "list"
      *  Call getdns_dict_get_list() against the dict for name = "list"
      *  Call getdns_list_set_int() for index 0
      *  expect:  int retrieved = 100
      */
      struct getdns_dict *this_dict = NULL;
      struct getdns_list *list = NULL;
      struct getdns_list *answer = NULL;
      uint32_t value;

      DICT_CREATE(this_dict);
      LIST_CREATE(list);

      ASSERT_RC(getdns_list_set_int(list, 0, 100),
        GETDNS_RETURN_GOOD, "Return code from getdns_list_set_int()");

      ASSERT_RC(getdns_dict_set_list(this_dict, "list", list),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_list()");

      ASSERT_RC(getdns_dict_get_list(this_dict, "list", &answer),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_get_list()");

      ASSERT_RC(getdns_list_get_int(answer, 0, &value),
        GETDNS_RETURN_GOOD, "Return code from getdns_list_get_int()");

      ck_assert_msg(value == 100, "Expected int retrieved == 100, got: %d", value);

      DICT_DESTROY(this_dict);
      LIST_DESTROY(list);
    }
    END_TEST
    
    Suite *
    getdns_dict_get_list_suite (void)
    {
      Suite *s = suite_create ("getdns_dict_get_list()");
    
      /* Negative test caseis */
      TCase *tc_neg = tcase_create("Negative");
      tcase_add_test(tc_neg, getdns_dict_get_list_1);
      tcase_add_test(tc_neg, getdns_dict_get_list_2);
      tcase_add_test(tc_neg, getdns_dict_get_list_3);
      tcase_add_test(tc_neg, getdns_dict_get_list_4);
      tcase_add_test(tc_neg, getdns_dict_get_list_5);
      suite_add_tcase(s, tc_neg);
    
      /* Positive test cases */
      TCase *tc_pos = tcase_create("Positive");
      tcase_add_test(tc_pos, getdns_dict_get_list_6);
      suite_add_tcase(s, tc_pos);
    
      return s;
    }

#endif
