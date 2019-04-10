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
#ifndef _check_getdns_context_set_context_update_callback_h_
#define _check_getdns_context_set_context_update_callback_h_

    /*
     **************************************************************************
     *                                                                        *
     *  T E S T S  F O R  G E T D N S _ C O N T E X T _ S E T _ C O N T E X T _ U P D A T E _ C A L L B A C K *
     *                                                                        *
     **************************************************************************
    */

    START_TEST (getdns_context_set_context_update_callback_1)
    {
     /*
      *  context is NULL
      *  expect:  GETDNS_RETURN_BAD_CONTEXT
      */

      struct getdns_context *context = NULL;

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_context_set_context_update_callback()");
        
    }
    END_TEST

    START_TEST (getdns_context_set_context_update_callback_2)
    {
     /*
      *  value is NULL
      *  expect: GETDNS_RETURN_INVALID_PARAMETER
      */

      struct getdns_context *context = NULL;
      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      expected_changed_item = GETDNS_CONTEXT_CODE_TIMEOUT;
       

      ASSERT_RC(getdns_context_set_timeout(context, 3),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_timeout()");

      ASSERT_RC(getdns_context_set_context_update_callback(context, NULL),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      CONTEXT_DESTROY;
        
    }
    END_TEST

    START_TEST (getdns_context_set_context_update_callback_5)
    {
      /*
      *  Create a context by calling getdns_context_create()
      *  Define a callback routine for context changes and call getdns_context_set_context_update_callback() so that it gets called when there are context changes
      *  getdns_context_set_resolution_type() to GETDNS_RESOLUTION_STUB
      *  expect:  GETDNS_CONTEXT_CODE_RESOLUTION_TYPE
      */
      struct getdns_context *context = NULL;
      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      expected_changed_item = GETDNS_CONTEXT_CODE_RESOLUTION_TYPE;

      ASSERT_RC(getdns_context_set_resolution_type(context, GETDNS_RESOLUTION_STUB),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_resolution_type()");

      CONTEXT_DESTROY;
  
    }
    END_TEST

    START_TEST (getdns_context_set_context_update_callback_6)
    {
      /*
      *  Create a context by calling getdns_context_create()
      *  Define a callback routine for context changes and call getdns_context_set_context_update_callback() so that it gets called when there are context changes
      *  Call getdns_context_set_namespaces() to change the order and/or number of namespaces to be queried
      *  expect:  GETDNS_CONTEXT_CODE_NAMESPACES
      */ 
      struct getdns_context *context = NULL;
      getdns_namespace_t namespace_arr[2] = {GETDNS_NAMESPACE_DNS, GETDNS_NAMESPACE_LOCALNAMES};
      size_t count;
      getdns_namespace_t *namespaces;
      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      expected_changed_item = GETDNS_CONTEXT_CODE_NAMESPACES;

      ASSERT_RC(getdns_context_set_namespaces(context, 2, namespace_arr),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_namespaces()");
      ASSERT_RC(getdns_context_get_namespaces(context, &count, &namespaces),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_get_namespaces()");
      ck_assert_msg(count == 2 && namespaces[0] == 500 && namespaces[1] == 501, "namespaces are not correctly set");


      CONTEXT_DESTROY;
       
    }
    END_TEST

    START_TEST (getdns_context_set_context_update_callback_7)
    {
      /*
      *  Create a context by calling getdns_context_create()
      *  Define a callback routine for context changes and call getdns_context_set_context_update_callback() so that it gets called when there are context changes
      *  Call getdns_context_set_dns_transport() to GETDNS_TRANSPORT_UDP_ONLY
      *  expect:  GETDNS_CONTEXT_CODE_DNS_TRANSPORT
      */ 
      struct getdns_context *context = NULL;
      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      expected_changed_item = GETDNS_CONTEXT_CODE_DNS_TRANSPORT;

      ASSERT_RC(getdns_context_set_dns_transport(context, GETDNS_TRANSPORT_UDP_ONLY),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_dns_transport()");

      CONTEXT_DESTROY;
       
    }
    END_TEST

    START_TEST (getdns_context_set_context_update_callback_8)
    {
      /*
      *  Create a context by calling getdns_context_create()
      *  Define a callback routine for context changes and call getdns_context_set_context_update_callback() so that it gets called when there are context changes
      *  Call getdns_context_set_limit_outstanding_queries() and set limit to 10
      *  expect:  GETDNS_CONTEXT_CODE_LIMIT_OUTSTANDING_QUERIES
      */ 
      struct getdns_context *context = NULL;
      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      expected_changed_item = GETDNS_CONTEXT_CODE_LIMIT_OUTSTANDING_QUERIES;

      ASSERT_RC(getdns_context_set_limit_outstanding_queries(context, 10),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_limit_outstanding_queries()");

      CONTEXT_DESTROY;
       
    }
    END_TEST

    START_TEST (getdns_context_set_context_update_callback_9)
    {
      /*
      *  Create a context by calling getdns_context_create()
      *  Define a callback routine for context changes and call getdns_context_set_context_update_callback() so that it gets called when there are context changes
      *  Call getdns_context_set_timeout() and set timeout to 3 seconds
      *  expect:  GETDNS_CONTEXT_CODE_TIMEOUT
      */ 
      struct getdns_context *context = NULL;
      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      expected_changed_item = GETDNS_CONTEXT_CODE_TIMEOUT;

      ASSERT_RC(getdns_context_set_timeout(context, 3),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_timeout()");

      CONTEXT_DESTROY;
       
    }
    END_TEST

    START_TEST (getdns_context_set_context_update_callback_10)
    {
      /*
      *  Create a context by calling getdns_context_create()
      *  Define a callback routine for context changes and call getdns_context_set_context_update_callback() so that it gets called when there are context changes
      *  Call getdns_context_set_follow_redirects() to GETDNS_REDIRECTS_DO_NOT_FOLLOW
      *  expect:  GETDNS_CONTEXT_CODE_FOLLOW_REDIRECTS
      */ 
      struct getdns_context *context = NULL;
      getdns_redirects_t redir;
      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      expected_changed_item = GETDNS_CONTEXT_CODE_FOLLOW_REDIRECTS;

      (void) getdns_context_set_follow_redirects(context, GETDNS_REDIRECTS_DO_NOT_FOLLOW);
      (void) getdns_context_get_follow_redirects(context, &redir);
      ck_assert_msg(redir == GETDNS_REDIRECTS_DO_NOT_FOLLOW, "getdns_context_get_follow_redirects failed");

      CONTEXT_DESTROY;
       
    }
    END_TEST

    START_TEST (getdns_context_set_context_update_callback_15)
    {
      /*
      *  Create a context by calling getdns_context_create()
      *  Define a callback routine for context changes and call getdns_context_set_context_update_callback() so that it gets called when there are context changes
      *  Call getdns_context_set_stub_resolution() providing where the API should send queries to
      *  expect:  GETDNS_CONTEXT_CODE_UPSTREAM_RECURSIVE_SERVERS
      */ 
      struct getdns_context *context = NULL;
      struct getdns_list *upstream_list = NULL;
      struct getdns_dict *dict = NULL;
      size_t index = 0;
      struct getdns_bindata address_type = { 5, (void *)"IPv4" };
      struct getdns_bindata address_data = { 4, (void *)"\x0A\x58\x1E\x52" };
      
      
      
      CONTEXT_CREATE(TRUE);
      LIST_CREATE(upstream_list);
      DICT_CREATE(dict);

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      expected_changed_item = GETDNS_CONTEXT_CODE_UPSTREAM_RECURSIVE_SERVERS;

      ASSERT_RC(getdns_dict_set_bindata(dict, "address_type", &address_type), 
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

      ASSERT_RC(getdns_dict_set_bindata(dict, "address_data", &address_data),
        GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_bindata()");

      ASSERT_RC(getdns_list_set_dict(upstream_list, index, dict), GETDNS_RETURN_GOOD,
        "Return code from getdns_list_set_dict()");



      ASSERT_RC(getdns_context_set_upstream_recursive_servers(context, upstream_list),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_upstream_recursive_servers()");

      CONTEXT_DESTROY;
      LIST_DESTROY(upstream_list);
      DICT_DESTROY(dict);
       
    }
    END_TEST

    START_TEST (getdns_context_set_context_update_callback_16)
    {
      /*
      *  Create a context by calling getdns_context_create()
      *  Define a callback routine for context changes and call getdns_context_set_context_update_callback() so that it gets called when there are context changes
      *  Call getdns_context_set_edns_maximum_udp_payload_size() setting max UDP payload to 512
      *  expect:  GETDNS_CONTEXT_CODE_EDNS_MAXIMUM_UDP_PAYLOAD_SIZE
      */ 
      struct getdns_context *context = NULL;
      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      expected_changed_item = GETDNS_CONTEXT_CODE_EDNS_MAXIMUM_UDP_PAYLOAD_SIZE;

      ASSERT_RC(getdns_context_set_edns_maximum_udp_payload_size(context, 512),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_edns_maximum_udp_payload_size()");

      CONTEXT_DESTROY;
       
    }
    END_TEST

    START_TEST (getdns_context_set_context_update_callback_17)
    {
      /*
      *  Create a context by calling getdns_context_create()
      *  Define a callback routine for context changes and call getdns_context_set_context_update_callback() so that it gets called when there are context changes
      *  Call getdns_context_set_edns_extended_rcode() setting  extended rcode to 1
      *  expect:  GETDNS_CONTEXT_CODE_EDNS_EXTENDED_RCODE
      */ 
      struct getdns_context *context = NULL;
      uint8_t extended_rcode;

      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      expected_changed_item = GETDNS_CONTEXT_CODE_EDNS_EXTENDED_RCODE;

      ASSERT_RC(getdns_context_set_edns_extended_rcode(context, 1),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_edns_extended_rcode()");
      ASSERT_RC(getdns_context_get_edns_extended_rcode(context, &extended_rcode),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_get_edns_extended_rcode()");
      ck_assert_msg(extended_rcode == 1, "extended_rcode should be 1, got %d", (int)extended_rcode);

      CONTEXT_DESTROY;
       
    }
    END_TEST

    START_TEST (getdns_context_set_context_update_callback_18)
    {
      /*
      *  Create a context by calling getdns_context_create()
      *  Define a callback routine for context changes and call getdns_context_set_context_update_callback() so that it gets called when there are context changes
      *  Call getdns_context_set_edns_version() setting  edns version to 1
      *  expect:  GETDNS_CONTEXT_CODE_EDNS_VERSION
      */ 
      struct getdns_context *context = NULL;
      uint8_t version;

      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      expected_changed_item = GETDNS_CONTEXT_CODE_EDNS_VERSION;

      ASSERT_RC(getdns_context_set_edns_version(context, 1),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_edns_version()");
      ASSERT_RC(getdns_context_get_edns_version(context, &version),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_get_edns_version()");
      ck_assert_msg(version == 1, "version should be 1, got %d", (int)version);

      CONTEXT_DESTROY;
       
    }
    END_TEST

    START_TEST (getdns_context_set_context_update_callback_19)
    {
      /*
      *  Create a context by calling getdns_context_create()
      *  Define a callback routine for context changes and call getdns_context_set_context_update_callback() so that it gets called when there are context changes
      *  Call getdns_context_set_edns_do_bit() setting  edns do bit to 1
      *  expect:  GETDNS_CONTEXT_CODE_EDNS_DO_BIT
      */ 
      struct getdns_context *context = NULL;
      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      expected_changed_item = GETDNS_CONTEXT_CODE_EDNS_DO_BIT;

      ASSERT_RC(getdns_context_set_edns_do_bit(context, 1),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_edns_do_bit()");

      CONTEXT_DESTROY;
       
    }
    END_TEST

    START_TEST (getdns_context_set_context_update_callback_20)
    {
      /*
      *  Create a context by calling getdns_context_create()
      *  Define a callback routine for context changes and call getdns_context_set_context_update_callback() so that it gets called when there are context changes
      *  Call getdns_context_set_edns_client_subnet_private() setting to 1
      *  expect:  GETDNS_CONTEXT_CODE_EDNS_CLIENT_SUBNET_PRIVATE
      */ 
      struct getdns_context *context = NULL;
      uint8_t client_subnet_private;

      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      expected_changed_item = GETDNS_CONTEXT_CODE_EDNS_CLIENT_SUBNET_PRIVATE;

      ASSERT_RC(getdns_context_set_edns_client_subnet_private(context, 1),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_edns_client_subnet_private()");
      ASSERT_RC(getdns_context_get_edns_client_subnet_private(context, &client_subnet_private),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_get_edns_client_subnet_private()");
      ck_assert_msg(client_subnet_private == 1, "client_subnet_private should be 1, got %d", (int)client_subnet_private);

      CONTEXT_DESTROY;
       
    }
    END_TEST

    START_TEST (getdns_context_set_context_update_callback_21)
    {
      /*
      *  Create a context by calling getdns_context_create()
      *  Define a callback routine for context changes and call getdns_context_set_context_update_callback() so that it gets called when there are context changes
      *  Call getdns_context_set_edns_client_subnet_private() setting to 1
      *  expect:  GETDNS_CONTEXT_CODE_TLS_QUERY_PADDING_BLOCKSIZE
      */ 
      struct getdns_context *context = NULL;
      uint16_t pad;

      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      expected_changed_item = GETDNS_CONTEXT_CODE_TLS_QUERY_PADDING_BLOCKSIZE;

      ASSERT_RC(getdns_context_set_tls_query_padding_blocksize(context, 1400),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_tls_query_padding_blocksize()");
      ASSERT_RC(getdns_context_get_tls_query_padding_blocksize(context, &pad),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_get_tls_query_padding_blocksize()");
      ck_assert_msg(pad == 1400, "padding_blocksize should be 1400 but got %d", (int) pad);

      CONTEXT_DESTROY;
       
    }
    END_TEST

    START_TEST (getdns_context_set_context_update_callback_22)
    {
     /*
      *  value is NULL
      *  expect: GETDNS_RETURN_INVALID_PARAMETER
      */

      struct getdns_context *context = NULL;
      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      expected_changed_item = GETDNS_CONTEXT_CODE_IDLE_TIMEOUT;

      ASSERT_RC(getdns_context_set_idle_timeout(context, 100),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_timeout()");

      CONTEXT_DESTROY;

    }
    END_TEST    

    START_TEST (getdns_context_set_context_update_callback_23)
    {
     /*
      *  expect: GETDNS_RETURN_GOOD
      */

      struct getdns_context *context = NULL;
      uint8_t round_robin;

      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_context_update_callback(context, update_callbackfn),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_context_update_callback()");

      expected_changed_item = GETDNS_CONTEXT_CODE_ROUND_ROBIN_UPSTREAMS;

      ASSERT_RC(getdns_context_set_round_robin_upstreams(context, 1),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_set_round_robin_upstream()");
      ASSERT_RC(getdns_context_get_round_robin_upstreams(context, &round_robin),
        GETDNS_RETURN_GOOD, "Return code from getdns_context_get_round_robin_upstream()");
      ck_assert_msg( round_robin == 1, "round_robin should be 1, got %d", (int)round_robin);

      CONTEXT_DESTROY;

    }
    END_TEST
    
    Suite *
    getdns_context_set_context_update_callback_suite (void)
    {
      Suite *s = suite_create ("getdns_context_set_context_update_callback()");
    
      /* Negative test caseis */
      TCase *tc_neg = tcase_create("Negative");
      tcase_add_test(tc_neg, getdns_context_set_context_update_callback_1);
      tcase_add_test(tc_neg, getdns_context_set_context_update_callback_2);
      suite_add_tcase(s, tc_neg);
    
      /* Positive test cases */
      TCase *tc_pos = tcase_create("Positive");
      tcase_add_test(tc_pos, getdns_context_set_context_update_callback_5);
      tcase_add_test(tc_pos, getdns_context_set_context_update_callback_6);
      tcase_add_test(tc_pos, getdns_context_set_context_update_callback_7);
      tcase_add_test(tc_pos, getdns_context_set_context_update_callback_8);
      tcase_add_test(tc_pos, getdns_context_set_context_update_callback_9);
      tcase_add_test(tc_pos, getdns_context_set_context_update_callback_10);
      tcase_add_test(tc_pos, getdns_context_set_context_update_callback_15);
      tcase_add_test(tc_pos, getdns_context_set_context_update_callback_16);
      tcase_add_test(tc_pos, getdns_context_set_context_update_callback_17);
      tcase_add_test(tc_pos, getdns_context_set_context_update_callback_18);
      tcase_add_test(tc_pos, getdns_context_set_context_update_callback_19);
      tcase_add_test(tc_pos, getdns_context_set_context_update_callback_20);
      tcase_add_test(tc_pos, getdns_context_set_context_update_callback_21);
      tcase_add_test(tc_pos, getdns_context_set_context_update_callback_22);
      tcase_add_test(tc_pos, getdns_context_set_context_update_callback_23);
      suite_add_tcase(s, tc_pos);

       return s;

    }

#endif
