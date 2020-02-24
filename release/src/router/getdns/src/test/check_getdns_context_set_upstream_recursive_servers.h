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
#ifndef _check_getdns_context_set_upstream_recursive_servers_h_
#define _check_getdns_context_set_upstream_recursive_servers_h_

    /*
     ******************************************************************************************
     *                                                                                        *
     *  T E S T S  F O R  G E T D N S _ C O N T E X T _ S E T _ S T U B _ R E S O L U T I O N *
     *                                                                                        *
     ******************************************************************************************
    */

    START_TEST (getdns_context_set_upstream_recursive_servers_1)
    {
     /*
      *  context is NULL
      *  expect:  GETDNS_RETURN_INVALID_PARAMETER
      */

      struct getdns_context *context = NULL;
      struct getdns_list    *upstream_list = NULL;
     
      LIST_CREATE(upstream_list);
      
      ASSERT_RC(getdns_context_set_upstream_recursive_servers(context, upstream_list),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_context_set_upstream_recursive_servers()");

            LIST_DESTROY(upstream_list);
    }
    END_TEST

    START_TEST (getdns_context_set_upstream_recursive_servers_2)
    {
     /*
      *  upstream_list  is NULL
      *  expect: GETDNS_RETURN_INVALID_PARAMETER
      */

      struct getdns_context *context = NULL;
      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_upstream_recursive_servers(context, NULL),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_upstream_recursive_servers()");


      CONTEXT_DESTROY;        
    }
    END_TEST

    START_TEST (getdns_context_set_upstream_recursive_servers_3)
    {
     /*
      *  create upstream_list 
      *  create context
      * a dict in upstream_list does not contain getdns_bindata
      */

      struct getdns_context *context = NULL;
      struct getdns_list    *upstream_list = NULL;
      struct getdns_dict *dict = NULL;
      size_t index = 0;

      CONTEXT_CREATE(TRUE);
      LIST_CREATE(upstream_list);
      DICT_CREATE(dict);

      ASSERT_RC(getdns_list_set_dict(upstream_list, index, dict), GETDNS_RETURN_GOOD,
        "Return code from getdns_list_set_dict()");


      ASSERT_RC(getdns_context_set_upstream_recursive_servers(context, upstream_list),
        GETDNS_RETURN_CONTEXT_UPDATE_FAIL, "Return code from getdns_context_set_upstream_recursive_servers()");

      CONTEXT_DESTROY;  
      LIST_DESTROY(upstream_list);
      DICT_DESTROY(dict);      
    }
    END_TEST

    START_TEST (getdns_context_set_upstream_recursive_servers_4)
    {
     /*
      *  create upstream_list 
      *  create context
      * a dict in upstream_list does not contain two names
      */

      struct getdns_context *context = NULL;
      struct getdns_list    *upstream_list = NULL;
      struct getdns_dict *dict = NULL;
      struct getdns_bindata address_type = { 5, (void *) "IPv4" };
      size_t index = 0;

      CONTEXT_CREATE(TRUE);
      LIST_CREATE(upstream_list);
      DICT_CREATE(dict);

      ASSERT_RC(getdns_list_set_dict(upstream_list, index, dict), GETDNS_RETURN_GOOD,
        "Return code from getdns_list_set_dict()");

      ASSERT_RC(getdns_dict_set_bindata(dict, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");


      ASSERT_RC(getdns_context_set_upstream_recursive_servers(context, upstream_list),
        GETDNS_RETURN_CONTEXT_UPDATE_FAIL, "Return code from getdns_context_set_upstream_recursive_servers()");

      CONTEXT_DESTROY;  
      LIST_DESTROY(upstream_list);
      DICT_DESTROY(dict);      
    }
    END_TEST

    START_TEST (getdns_context_set_upstream_recursive_servers_5)
    {
     /*
      *  create upstream_list 
      *  create context
      * a dict in upstream_list contains names other than address_type , 
         address_data, and port
      */

      struct getdns_context *context = NULL;
      struct getdns_list    *upstream_list = NULL;
      struct getdns_dict *dict = NULL;
      struct getdns_bindata address_data = { 4, (void *)"\x08\x08\x08\x08" };
      size_t index = 0;

      CONTEXT_CREATE(TRUE);
      LIST_CREATE(upstream_list);
      DICT_CREATE(dict);

      ASSERT_RC(getdns_list_set_dict(upstream_list, index, dict), GETDNS_RETURN_GOOD,
        "Return code from getdns_list_set_dict()");

     ASSERT_RC(getdns_dict_set_int(dict, "not_address_type", 100),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");

       ASSERT_RC(getdns_dict_set_bindata(dict, "not_address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

      ASSERT_RC(getdns_context_set_upstream_recursive_servers(context, upstream_list),
        GETDNS_RETURN_CONTEXT_UPDATE_FAIL, "Return code from getdns_context_set_upstream_recursive_servers()");

      CONTEXT_DESTROY;  
      LIST_DESTROY(upstream_list);
      DICT_DESTROY(dict);      
    }
    END_TEST

    START_TEST (getdns_context_set_upstream_recursive_servers_6)
    {
     /*
      *  create upstream_list 
      *  create context
      * a dict in upstream_list contains invalid address_ type (not “IPv4” or “IPv6”)
      */

      struct getdns_context *context = NULL;
      struct getdns_list    *upstream_list = NULL;
      struct getdns_dict *dict = NULL;
      struct getdns_bindata address_type = { 5, (void *)"IPv5" };
       struct getdns_bindata address_data = { 4, (void *)"\x08\x08\x08\x08" };
      size_t index = 0;

      CONTEXT_CREATE(TRUE);
      LIST_CREATE(upstream_list);
      DICT_CREATE(dict);

      ASSERT_RC(getdns_list_set_dict(upstream_list, index, dict), GETDNS_RETURN_GOOD,
        "Return code from getdns_list_set_dict()");

     ASSERT_RC(getdns_dict_set_bindata(dict, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");
       ASSERT_RC(getdns_dict_set_bindata(dict, "address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

      ASSERT_RC(getdns_context_set_upstream_recursive_servers(context, upstream_list),
        GETDNS_RETURN_CONTEXT_UPDATE_FAIL, "Return code from getdns_context_set_upstream_recursive_servers()");

      CONTEXT_DESTROY;  
      LIST_DESTROY(upstream_list);
      DICT_DESTROY(dict);      
    }
    END_TEST

    START_TEST (getdns_context_set_upstream_recursive_servers_7)
    {
     /*
      *  create upstream_list 
      *  create context
      * a dict in upstream_list contains named address_type and 
         address_data but the data type isn’t bindata
      */

      struct getdns_context *context = NULL;
      struct getdns_list    *upstream_list = NULL;
      struct getdns_dict *dict = NULL;
      size_t index = 0;

      CONTEXT_CREATE(TRUE);
      LIST_CREATE(upstream_list);
      DICT_CREATE(dict);

     ASSERT_RC(getdns_list_set_dict(upstream_list, index, dict), GETDNS_RETURN_GOOD,
        "Return code from getdns_list_set_dict()"); 

     ASSERT_RC(getdns_dict_set_int(dict, "address_type", 100),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");

       ASSERT_RC(getdns_dict_set_int(dict, "address_data", 200),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");

      ASSERT_RC(getdns_context_set_upstream_recursive_servers(context, upstream_list),
        GETDNS_RETURN_CONTEXT_UPDATE_FAIL, "Return code from getdns_context_set_upstream_recursive_servers()");

      CONTEXT_DESTROY;  
      LIST_DESTROY(upstream_list);
      DICT_DESTROY(dict);      
    }
    END_TEST

    START_TEST (getdns_context_set_upstream_recursive_servers_8)
    {
     /*
      *  create upstream_list 
      *  create context
      *  a dict in upstream_list contains invalid address_data
      */

      struct getdns_context *context = NULL;
      struct getdns_list    *upstream_list = NULL;
      struct getdns_dict *dict = NULL;
      struct getdns_bindata address_type = { 5, (void *)"IPv5" };
      struct getdns_bindata address_data = { 4, (void *)"\x08\x08\x08\x08" };
      size_t index = 0;

      CONTEXT_CREATE(TRUE);
      LIST_CREATE(upstream_list);
      DICT_CREATE(dict);

     ASSERT_RC(getdns_list_set_dict(upstream_list, index, dict), GETDNS_RETURN_GOOD,
        "Return code from getdns_list_set_dict()"); 

     ASSERT_RC(getdns_dict_set_bindata(dict, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");
       ASSERT_RC(getdns_dict_set_bindata(dict, "address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

      ASSERT_RC(getdns_context_set_upstream_recursive_servers(context, upstream_list),
        GETDNS_RETURN_CONTEXT_UPDATE_FAIL, "Return code from getdns_context_set_upstream_recursive_servers()");

      CONTEXT_DESTROY;  
      LIST_DESTROY(upstream_list);
      DICT_DESTROY(dict);      
    }
    END_TEST

    START_TEST (getdns_context_set_upstream_recursive_servers_9)
    {
     /*
      *  create context
      *  Call getdns_list_create() to create a list
      *  Call getdns_dict_create() to create a list
      *  Create bindata containing “IPv4” 
      */

      struct getdns_context *context = NULL;
      struct getdns_list    *upstream_list = NULL;
      struct getdns_dict *dict = NULL;
      struct getdns_dict *response = NULL;
      struct getdns_bindata address_type = { 4, (void *)"IPv4" };
      struct getdns_bindata address_data = { 4, (void *)"\x08\x08\x08\x08" };
      size_t index = 0;

      CONTEXT_CREATE(TRUE);
      LIST_CREATE(upstream_list);
      DICT_CREATE(dict);

      ASSERT_RC(getdns_dict_set_bindata(dict, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

      ASSERT_RC(getdns_dict_set_bindata(dict, "address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

      ASSERT_RC(getdns_list_set_dict(upstream_list, index, dict), GETDNS_RETURN_GOOD,
        "Return code from getdns_list_set_dict()"); 


      ASSERT_RC(getdns_context_set_upstream_recursive_servers(context, upstream_list),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_upstream_recursive_servers()");

      ASSERT_RC(getdns_general_sync(context, "google.com", GETDNS_RRTYPE_A, NULL, &response), 
         GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");


      EXTRACT_RESPONSE;

       assert_noerror(&ex_response);
       assert_address_in_answer(&ex_response, TRUE, FALSE);

      CONTEXT_DESTROY;  
      LIST_DESTROY(upstream_list);
      DICT_DESTROY(dict);
      DICT_DESTROY(response);
    }
    END_TEST

/* This test disabled because travis does not support IPv6 in their
 * container based infrastructure!
 */
#if 0
    START_TEST (getdns_context_set_upstream_recursive_servers_10)
    {
     /*
      *  create context
      *  Call getdns_list_create() to create a list
      * Call getdns_dict_create() to create a list
      * Create bindata containing “IPv6” 
      */

      struct getdns_context *context = NULL;
      struct getdns_list    *upstream_list = NULL;
      struct getdns_dict *dict = NULL;
      struct getdns_dict *response = NULL;
      struct getdns_bindata address_type = { 5, (void *)"IPv6" };
      struct getdns_bindata address_data = { 16, (void *)"\x26\x20\x00\x74\x00\x1b\x00\x00\x00\x00\x00\x00\x00\x01\x00\x01" };
      size_t index = 0;

      CONTEXT_CREATE(TRUE);
      LIST_CREATE(upstream_list);
      DICT_CREATE(dict);

      ASSERT_RC(getdns_dict_set_bindata(dict, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

      ASSERT_RC(getdns_dict_set_bindata(dict, "address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

      ASSERT_RC(getdns_list_set_dict(upstream_list, index, dict), GETDNS_RETURN_GOOD,
        "Return code from getdns_list_set_dict()"); 


      ASSERT_RC(getdns_context_set_upstream_recursive_servers(context, upstream_list),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_upstream_recursive_servers()");

      ASSERT_RC(getdns_general_sync(context, "google.com", GETDNS_RRTYPE_A, NULL, &response), 
         GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");

      EXTRACT_RESPONSE;

       assert_noerror(&ex_response);
       assert_address_in_answer(&ex_response, TRUE, FALSE);

      CONTEXT_DESTROY;  
      LIST_DESTROY(upstream_list);
      DICT_DESTROY(dict);
      DICT_DESTROY(response);
    }
    END_TEST
#endif
    START_TEST (getdns_context_set_upstream_recursive_servers_11)
    {
     /*
      *  create context
      *  Call getdns_list_create() to create a list
      *  Call getdns_dict_create() to create a list
      *  Create bindata containing “IPv4” 
      */

      struct getdns_context *context = NULL;
      struct getdns_list    *upstream_list = NULL;
      struct getdns_dict *dict = NULL;
      struct getdns_dict *response = NULL;
      struct getdns_bindata address_type = { 4, (void *)"IPv4" };
      struct getdns_bindata address_data = { 4, (void *)"\x08\x08\x08\x08" };
      struct getdns_bindata port = { 3, (void *)"53" };
      size_t index = 0;

      CONTEXT_CREATE(TRUE);
      LIST_CREATE(upstream_list);
      DICT_CREATE(dict);

      ASSERT_RC(getdns_dict_set_bindata(dict, "address_type", &address_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

      ASSERT_RC(getdns_dict_set_bindata(dict, "address_data", &address_data),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

      ASSERT_RC(getdns_dict_set_bindata(dict, "53", &port),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

      ASSERT_RC(getdns_list_set_dict(upstream_list, index, dict), GETDNS_RETURN_GOOD,
        "Return code from getdns_list_set_dict()"); 


      ASSERT_RC(getdns_context_set_upstream_recursive_servers(context, upstream_list),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_upstream_recursive_servers()");

      ASSERT_RC(getdns_general_sync(context, "google.com", GETDNS_RRTYPE_A, NULL, &response), 
         GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");


      EXTRACT_RESPONSE;
      //printf("the resp is %s\n", getdns_pretty_print_dict(response));


       assert_noerror(&ex_response);
       assert_address_in_answer(&ex_response, TRUE, FALSE);

      CONTEXT_DESTROY;  
      LIST_DESTROY(upstream_list);
      DICT_DESTROY(dict);
      DICT_DESTROY(response);
    }
    END_TEST

    
    
    
    Suite *
    getdns_context_set_upstream_recursive_servers_suite (void)
    {
      Suite *s = suite_create ("getdns_context_set_upstream_recursive_servers()");
    
      /* Negative test caseis */
      TCase *tc_neg = tcase_create("Negative");
      tcase_add_test(tc_neg, getdns_context_set_upstream_recursive_servers_1);
      tcase_add_test(tc_neg, getdns_context_set_upstream_recursive_servers_2);
      tcase_add_test(tc_neg, getdns_context_set_upstream_recursive_servers_3);
      tcase_add_test(tc_neg, getdns_context_set_upstream_recursive_servers_4);
      tcase_add_test(tc_neg, getdns_context_set_upstream_recursive_servers_5);
      tcase_add_test(tc_neg, getdns_context_set_upstream_recursive_servers_6);
      tcase_add_test(tc_neg, getdns_context_set_upstream_recursive_servers_7);
      tcase_add_test(tc_neg, getdns_context_set_upstream_recursive_servers_8);

      suite_add_tcase(s, tc_neg);
    
      /* Positive test cases */
       TCase *tc_pos = tcase_create("Positive");
       tcase_add_test(tc_pos, getdns_context_set_upstream_recursive_servers_9);
/***** tcase_add_test(tc_pos, getdns_context_set_upstream_recursive_servers_10); *****/
       tcase_add_test(tc_pos, getdns_context_set_upstream_recursive_servers_11);
      
       suite_add_tcase(s, tc_pos);

       return s;

    }

#endif
