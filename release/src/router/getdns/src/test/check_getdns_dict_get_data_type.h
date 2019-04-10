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
#ifndef _check_getdns_dict_get_data_type_h_
#define _check_getdns_dict_get_data_type_h_

    /*
     **************************************************************************
     *                                                                        *
     *  T E S T S  F O R  G E T D N S _ D I C T _ G E T _ D A T A _ T Y P E   *
     *                                                                        *
     **************************************************************************
    */

    START_TEST (getdns_dict_get_data_type_1)
    {
     /*
      *  this_dict = NULL
      *  expect:  GETDNS_RETURN_INVALID_PARAMETER
      */
      struct getdns_dict *this_dict = NULL;
      getdns_data_type answer;

      ASSERT_RC(getdns_dict_get_data_type(this_dict, "key", &answer),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_dict_get_data_type()");

    }
    END_TEST

    START_TEST (getdns_dict_get_data_type_2)
    {
     /*
      *  name = NULL
      *  expect:  GETDNS_RETURN_INVALID_PARAMETER
      */
      struct getdns_dict *this_dict = NULL;
      getdns_data_type answer;

      DICT_CREATE(this_dict);
      ASSERT_RC(getdns_dict_set_int(this_dict, "ten", 10),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");

      ASSERT_RC(getdns_dict_get_data_type(this_dict, NULL, &answer),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_dict_get_data_type()");

      DICT_DESTROY(this_dict);

    }
    END_TEST
    
    START_TEST (getdns_dict_get_data_type_3)
    {
     /*
      *  name does not exist in dict
      *  Create a dict with three keys ("ten" = 10, "eleven" = 11, "twelve" = 12)
      *  Call getdns_dict_get_data_type() with name = "nine"
      *  expect:  GETDNS_RETURN_NO_SUCH_DICT_NAME
      */
      struct getdns_dict *this_dict = NULL;
      char *keys[3] = { "ten", "eleven", "twelve" };
      uint32_t values[3] = { 10, 11, 12 };
      int i;
      getdns_data_type answer;

      DICT_CREATE(this_dict);

      for(i = 0; i < 3; i++)
      {
        ASSERT_RC(getdns_dict_set_int(this_dict, keys[i], values[i]), 
          GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");
      }

      ASSERT_RC(getdns_dict_get_data_type(this_dict, "nine", &answer),
        GETDNS_RETURN_NO_SUCH_DICT_NAME, "Return code from getdns_dict_get_names()");

      DICT_DESTROY(this_dict);
    }
    END_TEST
    
    START_TEST (getdns_dict_get_data_type_4)
    {
     /*
      *  answer = NULL
      *  expect: GETDNS_RETURN_INVALID_PARAMETER
      */
      struct getdns_dict *this_dict = NULL;

      DICT_CREATE(this_dict);
      ASSERT_RC(getdns_dict_set_int(this_dict, "ten", 10),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");

      ASSERT_RC(getdns_dict_get_data_type(this_dict, "ten", NULL),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_dict_get_names()");

      DICT_DESTROY(this_dict);
    }
    END_TEST
    
    START_TEST (getdns_dict_get_data_type_5)
    {
     /*
      *  data type is dict
      *  Create a dict
      *  Create a second dict and add it to the first as name = "dict"
      *  Call getdns_dict_get_data_type() for name = "dict"
      *  expect:  GETDNS_RETURN_GOOD
      *           retrieved answer should = t_dict 
      */
      struct getdns_dict *this_dict = NULL;
      struct getdns_dict *second_dict = NULL;
      getdns_data_type answer;

      DICT_CREATE(this_dict);
      DICT_CREATE(second_dict);
      ASSERT_RC(getdns_dict_set_dict(this_dict, "dict", second_dict),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_dict()");

      ASSERT_RC(getdns_dict_get_data_type(this_dict, "dict", &answer),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_get_data_type()");

      ck_assert_msg(answer == t_dict, "Expected answer = t_dict (%d), got: %d", t_dict, answer);

      DICT_DESTROY(this_dict);
      DICT_DESTROY(second_dict);
    }
    END_TEST
    
    START_TEST (getdns_dict_get_data_type_6)
    {
     /*
      *  data type is list
      *  Create a dict
      *  Create a list and add it to the dict as name = "list"
      *  Call getdns_dict_get_data_type() for name = "list"
      *  expect:  GETDNS_RETURN_GOOD
      *           retrieved answer should = t_list
      */
      struct getdns_dict *this_dict = NULL;
      struct getdns_list *list = NULL;
      getdns_data_type answer;

      DICT_CREATE(this_dict);
      LIST_CREATE(list);
      ASSERT_RC(getdns_dict_set_list(this_dict, "list", list),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_list()");

      ASSERT_RC(getdns_dict_get_data_type(this_dict, "list", &answer),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_get_data_type()");

      ck_assert_msg(answer == t_list, "Expected answer = t_list (%d), got: %d", t_list, answer);

      DICT_DESTROY(this_dict);
      LIST_DESTROY(list);
    }
    END_TEST
    
    START_TEST (getdns_dict_get_data_type_7)
    {
     /*
      *  data type is bindata
      *  Create a dict
      *  Create some bindata and add it to the dict as name = "bindata"
      *  Call getdns_dict_get_data_type() for name = "bindata"
      *  expect:  GETDNS_RETURN_GOOD
      *           retrieved answer should = t_bindata
      */
      struct getdns_dict *this_dict = NULL;
      struct getdns_bindata bindata = { 8, (void *)"bindata" };
      getdns_data_type answer;

      DICT_CREATE(this_dict);
      ASSERT_RC(getdns_dict_set_bindata(this_dict, "bindata", &bindata),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

      ASSERT_RC(getdns_dict_get_data_type(this_dict, "bindata", &answer),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_get_data_type()");

      ck_assert_msg(answer == t_bindata, "Expected answer = t_bindata (%d), got: %d", t_bindata, answer);

      DICT_DESTROY(this_dict);
    }
    END_TEST
    
    START_TEST (getdns_dict_get_data_type_8)
    {
     /*
      *  data type is int
      *  Create a dict
      *  Add an int to the dict as name = "int"
      *  Call getdns_dict_get_data_type() for name = "int"
      *  expect:  GETDNS_RETURN_GOOD
      *           retrieved answer should = t_int
      */
      struct getdns_dict *this_dict = NULL;
      getdns_data_type answer;

      DICT_CREATE(this_dict);
      ASSERT_RC(getdns_dict_set_int(this_dict, "int", 100),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");

      ASSERT_RC(getdns_dict_get_data_type(this_dict, "int", &answer),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_get_data_type()");

      ck_assert_msg(answer == t_int, "Expected answer = t_int (%d), got: %d", t_int, answer);

      DICT_DESTROY(this_dict);
    }
    END_TEST
    
    Suite *
    getdns_dict_get_data_type_suite (void)
    {
      Suite *s = suite_create ("getdns_dict_get_data_type()");
    
      /* Negative test caseis */
      TCase *tc_neg = tcase_create("Negative");
      tcase_add_test(tc_neg, getdns_dict_get_data_type_1);
      tcase_add_test(tc_neg, getdns_dict_get_data_type_2);
      tcase_add_test(tc_neg, getdns_dict_get_data_type_3);
      tcase_add_test(tc_neg, getdns_dict_get_data_type_4);
      suite_add_tcase(s, tc_neg);
    
      /* Positive test cases */
      TCase *tc_pos = tcase_create("Positive");
      tcase_add_test(tc_pos, getdns_dict_get_data_type_5);
      tcase_add_test(tc_pos, getdns_dict_get_data_type_6);
      tcase_add_test(tc_pos, getdns_dict_get_data_type_7);
      tcase_add_test(tc_pos, getdns_dict_get_data_type_8);
      suite_add_tcase(s, tc_pos);
    
      return s;
    }

#endif
