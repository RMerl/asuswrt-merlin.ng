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
#ifndef _check_getdns_list_get_list_h_
#define _check_getdns_list_get_list_h_

    /*
     **************************************************************************
     *                                                                        *
     *  T E S T S  F O R  G E T D N S _ L I S T _ G E T _ L I S T   *
     *                                                                        *
     **************************************************************************
    */

    START_TEST (getdns_list_get_list_1)
    {
     /*
      *  list = NULL
      *  expect: GETDNS_RETURN_INVALID_PARAMETER
      */
      struct getdns_list *list = NULL;
      size_t index = 0;
      struct getdns_list *answer = NULL;

      ASSERT_RC(getdns_list_get_list(list, index, &answer),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_list_get_list()");
    }
    END_TEST

    START_TEST (getdns_list_get_list_2)
    {
     /* index is out of range
      * create a list and set index 0 to an int with a value of 100
      * Call getdns_get_list() for index 1 
      * expect: GETDNS_RETURN_INVALID_PARAMETER
     */
      struct getdns_list *list = NULL;
      size_t index = 0;

      LIST_CREATE(list);
      
      ASSERT_RC(getdns_list_set_int(list, index, 1),
        GETDNS_RETURN_GOOD, "Return code from getdns_list_set_int()");

      index++;
      ASSERT_RC(getdns_list_get_list(list,index,NULL),
      	GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_list_get_list()");

      LIST_DESTROY(list);
    }
    END_TEST


    START_TEST (getdns_list_get_list_3)
    {
     /* data type at index is not a list
      * create a list and set index 0 to an int with a value of 100
      * expect: GETDNS_RETURN_WRONG_TYPE_REQUESTED
      */
      struct getdns_list *list = NULL;
      size_t index = 0;
      struct getdns_list *answer = NULL;

      LIST_CREATE(list);

      ASSERT_RC(getdns_list_set_int(list, index, 1),
        GETDNS_RETURN_GOOD, "Return code from getdns_list_set_int()");

      ASSERT_RC(getdns_list_get_list(list, index, &answer),
        GETDNS_RETURN_WRONG_TYPE_REQUESTED, "Return code from getdns_list_get_list()");

      LIST_DESTROY(list);
    }
    END_TEST

    START_TEST (getdns_list_get_list_4)
    {
     /* answer == NULL
      * expect: GETDNS_RETURN_INVALID_PARAMETER
      */
      struct getdns_list *list = NULL;
      size_t index = 0;

      ASSERT_RC(getdns_list_get_list(list, index, NULL),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_list_get_list()");
    }
    END_TEST

    START_TEST (getdns_list_get_list_5)
    {
     /* create a list. Also create a second list
      * Set list value at index 0 in the second list via getdns_list_set_int() to 45
      * Set list value at index 0 in the first list via getdns_list_set_list() to the second list
      * Call getdns_list_get_list() for index 0 against the first list
      * Call getdns_list_get_int() for index 0 against retrieved list
      * expect: GETDNS_RETURN_GOOD
      */
      struct getdns_list *list = NULL;
      struct getdns_list *second_list = NULL;
      size_t index = 0;
      struct getdns_list *answer = NULL;
      uint32_t value;

      LIST_CREATE(list);
      LIST_CREATE(second_list);

      ASSERT_RC(getdns_list_set_int(second_list, index, 45),
        GETDNS_RETURN_GOOD, "Return code from getdns_list_set_int()");

      ASSERT_RC(getdns_list_set_list(list, index, second_list),
        GETDNS_RETURN_GOOD, "Return code from getdns_list_set_list()");

      ASSERT_RC(getdns_list_get_list(list, index, &answer),
        GETDNS_RETURN_GOOD, "Return code from getdns_list_get_list()");

      ASSERT_RC(getdns_list_get_int(answer, index, &value),
        GETDNS_RETURN_GOOD, "Return code from getdns_list_get_int()");

      ck_assert_msg(value == 45, "Expected retrieved int == 45, got: %d",
        value);

      LIST_DESTROY(list);
      LIST_DESTROY(second_list);
    }
    END_TEST
    
    

    

    
    Suite *
    getdns_list_get_list_suite (void)
    {
      Suite *s = suite_create ("getdns_list_get_list()");

      /* Negative test caseis */
      TCase *tc_neg = tcase_create("Negative");
      tcase_add_test(tc_neg, getdns_list_get_list_1);
      tcase_add_test(tc_neg, getdns_list_get_list_2);
      tcase_add_test(tc_neg, getdns_list_get_list_3);
      tcase_add_test(tc_neg, getdns_list_get_list_4);
      suite_add_tcase(s, tc_neg);

      /* Positive test cases */
       TCase *tc_pos = tcase_create("Positive");
       tcase_add_test(tc_pos, getdns_list_get_list_5);
       suite_add_tcase(s, tc_pos);

      return s;
    }

#endif
