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
#ifndef _check_getdns_dict_set_list_h_
#define _check_getdns_dict_set_list_h_

    /*
     **************************************************************************
     *                                                                        *
     *  T E S T S  F O R  G E T D N S _ D I C T _ S E T _ L I S T             *
     *                                                                        *
     **************************************************************************
    */

    START_TEST (getdns_dict_set_list_1)
    {
     /*
      *  this_dict = NULL
      *  expect:  GETDNS_RETURN_INVALID_PARAMETER
      */
      struct getdns_dict *this_dict = NULL;
      struct getdns_list *list = NULL;

      DICT_CREATE(this_dict);
      ASSERT_RC(getdns_dict_set_list(this_dict, "list", list),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_dict_set_list()");

    }
    END_TEST

    START_TEST (getdns_dict_set_list_2)
    {
     /*
      *  name = NULL
      *  expect:  GETDNS_RETURN_INVALID_PARAMETER
      */
      struct getdns_dict *this_dict = NULL;
      struct getdns_list *list = NULL;

      DICT_CREATE(this_dict);
      LIST_CREATE(list);
      ASSERT_RC(getdns_dict_set_list(this_dict, NULL, list),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_dict_set_list()");

      DICT_DESTROY(this_dict);
      LIST_DESTROY(list);
    }
    END_TEST

    START_TEST (getdns_dict_set_list_3)
    {
     /*
      *  child_list = NULL
      *  expect:  GETDNS_RETURN_INVALID_PARAMETER
      */
      struct getdns_dict *this_dict = NULL;

      DICT_CREATE(this_dict);
      ASSERT_RC(getdns_dict_set_list(this_dict, "list", NULL),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_dict_set_list()");

      DICT_DESTROY(this_dict);
    }
    END_TEST
    
    START_TEST (getdns_dict_set_list_4)
    {
     /*
      *  name already exists in dict
      *  Create a dict
      *  Create a list
      *  Set list value at index 0 via getdns_list_set_int() to 100
      *  Add the list to the dict as name "list"
      *  Create a second list
      *  Set list value at index 0 in the second list to 101
      *  Add the second list to the dict using name "list" again
      *  Call getdns_dict_get_list() against the dict with name = "list"
      *  Call getdns_list_get_int() for index 0 against the retrieved list
      *  expect:  GETDNS_RETURN_GOOD (all functions)
      *           retrieved int should = 101
      */
      struct getdns_dict *this_dict = NULL;
      struct getdns_list *first_list = NULL;
      struct getdns_list *second_list = NULL;
      struct getdns_list *retrieved_list = NULL;
      uint32_t value;

      DICT_CREATE(this_dict);

      LIST_CREATE(first_list);
      ASSERT_RC(getdns_list_set_int(first_list, 0, 100),
        GETDNS_RETURN_GOOD, "Return from getdns_list_set_int()");

      ASSERT_RC(getdns_dict_set_list(this_dict, "list", first_list),
        GETDNS_RETURN_GOOD, "Return from getdns_list_set_list()");

      LIST_CREATE(second_list);
      ASSERT_RC(getdns_list_set_int(second_list, 0, 101),
        GETDNS_RETURN_GOOD, "Return from getdns_list_set_int()");

      ASSERT_RC(getdns_dict_set_list(this_dict, "list", second_list),
        GETDNS_RETURN_GOOD, "Return from getdns_list_set_list()");

      ASSERT_RC(getdns_dict_get_list(this_dict, "list", &retrieved_list),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_get_list()");

      ASSERT_RC(getdns_list_get_int(retrieved_list, 0, &value),
        GETDNS_RETURN_GOOD, "Return code from getdns_list_get_int()");

      ck_assert_msg(value == 101, "Expected int retrieved == 101, got: %d",
        value);

      DICT_DESTROY(this_dict);
      LIST_DESTROY(first_list);
      LIST_DESTROY(second_list);
    }
    END_TEST
    
    START_TEST (getdns_dict_set_list_5)
    {
     /*
      *  name already exists in dict, changing data type
      *  Create a dict
      *  Create a second dict
      *  Add an int to the second dict (name = "int", value = 100)
      *  Add the second dict to the first dict as name "list"
      *  Create a list
      *  Set list value at index 0 in the list to 101
      *  Add the list to the first dict using name "list" again
      *  Call getdns_dict_get_list() against the dict with name = "list"
      *  Call getdns_list_get_int() for index 0 against the retrieved list
      *  expect:  GETDNS_RETURN_GOOD (all functions)
      *           retrieved int should = 101
      */
      struct getdns_dict *first_dict = NULL;
      struct getdns_dict *second_dict = NULL;
      struct getdns_list *list = NULL;
      struct getdns_list *retrieved_list = NULL;
      uint32_t value;

      DICT_CREATE(first_dict);

      DICT_CREATE(second_dict);
      ASSERT_RC(getdns_dict_set_int(second_dict, "int", 100),
        GETDNS_RETURN_GOOD, "Return from getdns_dict_set_int()");

      ASSERT_RC(getdns_dict_set_dict(first_dict, "list", second_dict),
        GETDNS_RETURN_GOOD, "Return from getdns_dict_set_dict()");

      LIST_CREATE(list);
      ASSERT_RC(getdns_list_set_int(list, 0, 101),
        GETDNS_RETURN_GOOD, "Return from getdns_list_set_int()");

      ASSERT_RC(getdns_dict_set_list(first_dict, "list", list),
        GETDNS_RETURN_GOOD, "Return from getdns_dict_set_list()");

      ASSERT_RC(getdns_dict_get_list(first_dict, "list", &retrieved_list),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_get_list()");

      ASSERT_RC(getdns_list_get_int(retrieved_list, 0, &value),
        GETDNS_RETURN_GOOD, "Return code from getdns_list_get_int()");

      ck_assert_msg(value == 101, "Expected int retrieved == 101, got: %d",
        value);

      DICT_DESTROY(first_dict);
      DICT_DESTROY(second_dict);
      LIST_DESTROY(list);
    }
    END_TEST
    
    Suite *
    getdns_dict_set_list_suite (void)
    {
      Suite *s = suite_create ("getdns_dict_set_list()");
    
      /* Negative test caseis */
      TCase *tc_neg = tcase_create("Negative");
      tcase_add_test(tc_neg, getdns_dict_set_list_1);
      tcase_add_test(tc_neg, getdns_dict_set_list_2);
      tcase_add_test(tc_neg, getdns_dict_set_list_3);
      suite_add_tcase(s, tc_neg);
    
      /* Positive test cases */
      TCase *tc_pos = tcase_create("Positive");
      tcase_add_test(tc_pos, getdns_dict_set_list_4);
      tcase_add_test(tc_pos, getdns_dict_set_list_5);
      suite_add_tcase(s, tc_pos);
    
      return s;
    }

#endif
