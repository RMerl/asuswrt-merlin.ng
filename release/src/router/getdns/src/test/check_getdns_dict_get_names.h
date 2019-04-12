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
#ifndef _check_getdns_dict_get_names_h_
#define _check_getdns_dict_get_names_h_

    /*
     **************************************************************************
     *                                                                        *
     *  T E S T S  F O R  G E T D N S _ D I C T _ G E T _ N A M E S           *
     *                                                                        *
     **************************************************************************
    */

    START_TEST (getdns_dict_get_names_1)
    {
     /*
      *  this_dict = NULL
      *  expect = GETDNS_RETURN_INVALID_PARAMETER
      */
      struct getdns_dict *this_dict = NULL;
      struct getdns_list *answer = NULL;

      ASSERT_RC(getdns_dict_get_names(this_dict, &answer),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_dict_get_names()");

    }
    END_TEST
    
    START_TEST (getdns_dict_get_names_2)
    {
     /*
      *  answer = NULL
      *  expect: GETDNS_RETURN_INVALID_PARAMETER
      */
      struct getdns_dict *this_dict = NULL;

      DICT_CREATE(this_dict);

      ASSERT_RC(getdns_dict_get_names(this_dict, NULL),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_dict_get_names()");

      DICT_DESTROY(this_dict);
    }
    END_TEST
    
    START_TEST (getdns_dict_get_names_3)
    {
     /*
      *  Create a dict with three keys ("ten" = 10, "eleven" = 11, "twelve" = 12)
      *  Call getdns_dict_get_names()
      *  Iterate through list and append names together in a single string
      *  expect: string == "teneleventwelve"
      */
      struct getdns_dict *this_dict = NULL;
      struct getdns_list *answer = NULL;
      char *keys[3] = { "ten", "eleven", "twelve" };
      uint32_t values[3] = { 10, 11, 12 };
      size_t i;
      size_t length;
      struct getdns_bindata *key = NULL;
      char string_buffer[20] = "";

      DICT_CREATE(this_dict);

      for(i = 0; i < 3; i++)
      {
        ASSERT_RC(getdns_dict_set_int(this_dict, keys[i], values[i]), 
          GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");
      }

      ASSERT_RC(getdns_dict_get_names(this_dict, &answer),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_get_names()");

      ASSERT_RC(getdns_list_get_length(answer, &length),
        GETDNS_RETURN_GOOD, "Return code from getdns_list_get_length()");
 
      ck_assert_msg(length == 3, "Expected length == 3, got %d", length);

      for(i = 0; i < length; i++)
      {
        ASSERT_RC(getdns_list_get_bindata(answer, i, &key),
          GETDNS_RETURN_GOOD, "Return code from getdns_list_get_bindata()");
	/* concatenate strings, if enough buffer space */
	if(strlen(string_buffer) + strlen((char*)key->data) + 1
		< sizeof(string_buffer)) {
		memmove(string_buffer+strlen(string_buffer), key->data,
			strlen((char*)key->data)+1);
	}
      }

      ck_assert_msg(strcmp(string_buffer, "elevententwelve") == 0, 
        "Expected concatenated names to be \"elevententwelve\", got \"%s\"", string_buffer);

      LIST_DESTROY(answer);
      DICT_DESTROY(this_dict);
    }
    END_TEST
    
    Suite *
    getdns_dict_get_names_suite (void)
    {
      Suite *s = suite_create ("getdns_dict_get_names()");
    
      /* Negative test caseis */
      TCase *tc_neg = tcase_create("Negative");
      tcase_add_test(tc_neg, getdns_dict_get_names_1);
      tcase_add_test(tc_neg, getdns_dict_get_names_2);
      suite_add_tcase(s, tc_neg);
    
      /* Positive test cases */
      TCase *tc_pos = tcase_create("Positive");
      tcase_add_test(tc_pos, getdns_dict_get_names_3);
      suite_add_tcase(s, tc_pos);
    
      return s;
    }

#endif
