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
#ifndef _check_getdns_convert_ulabel_to_alabel_h_
#define _check_getdns_convert_ulabel_to_alabel_h_

    /*
     *************************************************************************************
     *                                                                                   *
     *  T E S T S  F O R  G E T D N S _ C O N V E R T _ U L A B E L _ T O _ A L A B E L  *
     *                                                                                   *
     *************************************************************************************
    */
     
     START_TEST (getdns_convert_ulabel_to_alabel_1)
     {
      /*
       *  ulabel = NULL
       *  expect: GETDNS_RETURN_GENERIC_ERROR
       */
      char *ulabel = NULL;


      ck_assert_msg(( getdns_convert_ulabel_to_alabel( ulabel ) == 0 ),
               "Was not expecting %p from getdns_convert_ulabel_to_alabel()",
		(void *)getdns_convert_ulabel_to_alabel( ulabel ) );
     }
     END_TEST

     START_TEST (getdns_convert_ulabel_to_alabel_2)
     {
      /*
       *  ulabel = invalid characters
       *  expect: GETDNS_RETURN_GENERIC_ERROR
       */
      char *ulabel = "#$%_";

      ck_assert_msg(strcmp( getdns_convert_ulabel_to_alabel( ulabel ), "#$%_" ) == 0,
                "Was not expecting %s from getdns_convert_ulabel_to_alabel()", getdns_convert_ulabel_to_alabel( ulabel ) );
     }
     END_TEST

     START_TEST (getdns_convert_ulabel_to_alabel_3)
     {
      /*
       *  ulabel = valid characters  ( _abc, -abc, -abc-, abc- and limited to 63 octets )
       *  expect: GETDNS_RETURN_GOOD
       */
      char *ulabel = "café";

      ck_assert_msg(strcmp( getdns_convert_ulabel_to_alabel( ulabel ), "xn--caf-dma" ) == 0,
                "Was not expecting %s from getdns_convert_ulabel_to_alabel()", getdns_convert_ulabel_to_alabel( ulabel ) );

     }
     END_TEST
     
     Suite *
     getdns_convert_ulabel_to_alabel_suite (void)
     {
       Suite *s = suite_create ("getdns_convert_ulabel_to_alabel()");
     
       /* Negative test caseis */
       TCase *tc_neg = tcase_create("Negative");
       tcase_add_test(tc_neg, getdns_convert_ulabel_to_alabel_1);
       tcase_add_test(tc_neg, getdns_convert_ulabel_to_alabel_2);
       suite_add_tcase(s, tc_neg);
     
       /* Positive test cases */
       TCase *tc_pos = tcase_create("Positive");
       tcase_add_test(tc_pos, getdns_convert_ulabel_to_alabel_3);
       suite_add_tcase(s, tc_pos);
     
       return s;
     }
 
#endif
