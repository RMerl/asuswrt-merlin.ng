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
#ifndef _check_getdns_convert_alabel_to_ulabel_h_
#define _check_getdns_convert_alabel_to_ulabel_h_

    /*
     *************************************************************************************
     *                                                                                   *
     *  T E S T S  F O R  G E T D N S _ C O N V E R T _ A L A B E L _ T O _ U L A B E L  *
     *                                                                                   *
     *************************************************************************************
    */
     
     START_TEST (getdns_convert_alabel_to_ulabel_1)
     {
      /*
       *  alabel = NULL
       *  expect: GETDNS_RETURN_GENERIC_ERROR
       */
      char *alabel = NULL;

      ck_assert_msg( getdns_convert_alabel_to_ulabel( alabel ) == 0,
               "Was not expecting %p from getdns_convert_alabel_to_ulabel()",
		(void *)getdns_convert_alabel_to_ulabel( alabel ) );

     }
     END_TEST

     START_TEST (getdns_convert_alabel_to_ulabel_2)
     {
      /*
       *  alabel = invalid characters
       *  expect: GETDNS_RETURN_GENERIC_ERROR
       */
      char *alabel = "#$%_";

      ck_assert_msg(strcmp( getdns_convert_alabel_to_ulabel( alabel ), "#$%_" ) == 0,
                "Was not expecting %s from getdns_convert_alabel_to_ulabel()", getdns_convert_alabel_to_ulabel( alabel ) );


     }
     END_TEST

     START_TEST (getdns_convert_alabel_to_ulabel_3)
     {
      /*
       *  alabel = valid characters  (ace must begin with prefix "xn--" and be followed by a valid puny algorithm output; length limited to 59 chars)
       *  expect: GETDNS_RETURN_GOOD
       */
      char *alabel = "xn--caf-dma";

      ck_assert_msg(strcmp( getdns_convert_alabel_to_ulabel( alabel ), "café" ) == 0,
                "Was not expecting %s from getdns_convert_alabel_to_ulabel()", getdns_convert_alabel_to_ulabel( alabel ) );

     }
     END_TEST
     
     Suite *
     getdns_convert_alabel_to_ulabel_suite (void)
     {
       Suite *s = suite_create ("getdns_convert_alabel_to_ulabel()");
     
       /* Negative test caseis */
       TCase *tc_neg = tcase_create("Negative");
       tcase_add_test(tc_neg, getdns_convert_alabel_to_ulabel_1);
       tcase_add_test(tc_neg, getdns_convert_alabel_to_ulabel_2);
       suite_add_tcase(s, tc_neg);
     
       /* Positive test cases */
       TCase *tc_pos = tcase_create("Positive");
       tcase_add_test(tc_pos, getdns_convert_alabel_to_ulabel_3);
       suite_add_tcase(s, tc_pos);
     
       return s;
     }
 
#endif
