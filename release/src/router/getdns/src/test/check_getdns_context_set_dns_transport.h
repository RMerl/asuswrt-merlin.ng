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
#ifndef _check_getdns_context_set_dns_transport_h_
#define _check_getdns_context_set_dns_transport_h_

    /*
     **************************************************************************
     *                                                                        *
     *  T E S T S  F O R  G E T D N S _ C O N T E X T _ S E T _ D N S _ T R A N S P O R T *
     *                                                                        *
     **************************************************************************
    */

    START_TEST (getdns_context_set_dns_transport_1)
    {
     /*
      *  context is NULL
      *  expect:  GETDNS_RETURN_INVALID_PARAMETER
      */

      struct getdns_context *context = NULL;
      uint16_t value = 302;

      ASSERT_RC(getdns_context_set_dns_transport(context, value),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_context_set_dns_transport()");

      ASSERT_RC(getdns_context_set_edns_maximum_udp_payload_size(context, 512),
       GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_context_set_edns_maximum_udp_payload_size()");

      ASSERT_RC(getdns_context_unset_edns_maximum_udp_payload_size(context),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_context_unset_edns_maximum_udp_payload_size()");
    }
    END_TEST

    START_TEST (getdns_context_set_dns_transport_2)
    {
     /*
      *  value is an undefined transport value
      *  do_bit is not correct
      *  expect: GETDNS_RETURN_CONTEXT_UPDATE_FAIL
      */

      struct getdns_context *context = NULL;
      //uint16_t value = 233;
      CONTEXT_CREATE(TRUE);


      ASSERT_RC(getdns_context_set_dns_transport(context, 233),
        GETDNS_RETURN_CONTEXT_UPDATE_FAIL, "Return code from getdns_context_set_dns_transport()");

      ASSERT_RC(getdns_context_set_edns_do_bit(context, 5),
        GETDNS_RETURN_CONTEXT_UPDATE_FAIL, "Return code from getdns_context_set_edns_do_bit()");

      CONTEXT_DESTROY;
        
    }
    END_TEST

    START_TEST (getdns_context_set_dns_transport_list_3)
    {
     /*
      *  context is NULL
      *  expect:  GETDNS_RETURN_INVALID_PARAMETER
      */

      struct getdns_context *context = NULL;
      getdns_transport_list_t transports[1];
      transports[0] = GETDNS_TRANSPORT_UDP;
      size_t transport_count = sizeof(transports);

      ASSERT_RC(getdns_context_set_dns_transport_list(context, transport_count, transports),
        GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_context_set_dns_transport()");

    }
    END_TEST

    START_TEST (getdns_context_set_dns_transport_list_4)
    {
    /*
      *  list is invalid
      *  expect: GETDNS_RETURN_CONTEXT_UPDATE_FAIL
      */

      struct getdns_context *context = NULL;
      getdns_transport_list_t transports[1];
      transports[0] = GETDNS_TRANSPORT_UDP;
      CONTEXT_CREATE(TRUE);

      ASSERT_RC(getdns_context_set_dns_transport_list(context, 0, NULL),
        GETDNS_RETURN_CONTEXT_UPDATE_FAIL, "Return code from getdns_context_set_dns_transport()");
      ASSERT_RC(getdns_context_set_dns_transport_list(context, 1, NULL),
        GETDNS_RETURN_CONTEXT_UPDATE_FAIL, "Return code from getdns_context_set_dns_transport()");
      ASSERT_RC(getdns_context_set_dns_transport_list(context, 0, transports),
        GETDNS_RETURN_CONTEXT_UPDATE_FAIL, "Return code from getdns_context_set_dns_transport()");
      ASSERT_RC(getdns_context_set_dns_transport_list(context, 2, transports),
        GETDNS_RETURN_CONTEXT_UPDATE_FAIL, "Return code from getdns_context_set_dns_transport()");

      CONTEXT_DESTROY;

    }
    END_TEST

     START_TEST (getdns_context_set_dns_transport_stub_5)
     {
       /*
       *  Request answer larger then 512 bytes but set UDP payload to that
       *  Call getdns_context_set_dns_transport() with value = GETDNS_TRANSPORT_UDP_ONLY
       *  expect: Message uses UDP but is truncated
       *  Call getdns_context_set_dns_transport() with value = GETDNS_TRANSPORT_TCP_ONLY
       *  expect: Message uses TCP and is not truncated
       *  Call getdns_context_set_dns_transport() with value = GETDNS_TRANSPORT_UDP_FIRST_AND_FALL_BACK_TO_TCP
       *  expect: Message uses TCP and is not truncated
       */
       struct getdns_context *context = NULL;
       struct getdns_dict *response = NULL;
       struct getdns_dict *extensions = getdns_dict_create();
       uint32_t tc;
       uint32_t transport;
       uint32_t type;
       uint16_t payload_size;
       uint8_t do_bit;
       getdns_transport_t trans;
       int upstream_is_dnsmasq = 0;
       getdns_bindata *version_str = NULL;

       /* Note that stricly this test just establishes that the requested transport
          and the reported transport are consistent, it does not guarantee which
          transport is used on the wire...*/

       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_context_set_resolution_type(context, GETDNS_RESOLUTION_STUB),
         GETDNS_RETURN_GOOD, "Return code from getdns_context_set_resolution_type()");
       ASSERT_RC(getdns_dict_set_int(extensions,"return_call_reporting", GETDNS_EXTENSION_TRUE),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");


       /* Request a response that should be truncated over UDP */
       ASSERT_RC(getdns_context_set_dns_transport(context, GETDNS_TRANSPORT_UDP_ONLY),
         GETDNS_RETURN_GOOD, "Return code from getdns_context_set_dns_transport()");
       ASSERT_RC(getdns_context_get_dns_transport(context, &trans),
         GETDNS_RETURN_GOOD, "Return code from getdns_context_get_dns_transport()");
       ck_assert_msg(trans == 541, "dns_transport should be 541(GETDNS_TRANSPORT_UDP_ONLY) but got %d", (int)trans);

       ASSERT_RC(getdns_dict_set_int(extensions,"specify_class", GETDNS_RRCLASS_CH),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");
       ASSERT_RC(getdns_general_sync(context, "version.bind.", GETDNS_RRTYPE_TXT, extensions, &response), 
         GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");
       (void) getdns_dict_get_bindata(response, "/replies_tree/0/answer/0/rdata/txt_strings/0", &version_str);
       upstream_is_dnsmasq = version_str && version_str->size > 7 &&
                             strncmp((char *)version_str->data, "dnsmasq", 7) == 0;
       ASSERT_RC(getdns_dict_set_int(extensions,"specify_class", GETDNS_RRCLASS_IN),
         GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");

       ASSERT_RC(getdns_context_set_edns_maximum_udp_payload_size(context, 512),
           GETDNS_RETURN_GOOD, "Return code from getdns_context_set_edns_maximum_udp_payload_size()");
       ASSERT_RC(getdns_context_get_edns_maximum_udp_payload_size(context, &payload_size),
           GETDNS_RETURN_GOOD, "Return code from getdns_context_get_edns_maximum_udp_payload_size()");
       ck_assert_msg(payload_size == 512, "payload_size should be 512, got %d", (int)payload_size);

       ASSERT_RC(getdns_context_set_edns_do_bit(context, 1),
           GETDNS_RETURN_GOOD, "Return code from getdns_context_set_edns_do_bit()");
       ASSERT_RC(getdns_context_get_edns_do_bit(context, &do_bit),
           GETDNS_RETURN_GOOD, "Return code from getdns_context_get_edns_do_bit()");
       ck_assert_msg(do_bit == 1, "do_bit should be 1, got %d", (int)do_bit);

       ASSERT_RC(getdns_general_sync(context, "large.getdnsapi.net", GETDNS_RRTYPE_TXT, extensions, &response), 
         GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");

       ASSERT_RC(getdns_dict_get_int(response, "/call_reporting/0/transport", &transport),
         GETDNS_RETURN_GOOD, "Failed to extract \"transport\"");
       ASSERT_RC(transport, GETDNS_TRANSPORT_UDP, "Query did not go over UDP");
       ASSERT_RC(getdns_dict_get_int(response, "/call_reporting/0/resolution_type", &type),
         GETDNS_RETURN_GOOD, "Failed to extract \"resolution_type\"");
       ASSERT_RC(type, GETDNS_RESOLUTION_STUB, "Query did not use stub mode");
       ASSERT_RC(getdns_dict_get_int(response, "/replies_tree/0/header/tc", &tc),
         GETDNS_RETURN_GOOD, "Failed to extract \"tc\"");
       if (!upstream_is_dnsmasq) {
         ASSERT_RC(tc, 1, "Packet not truncated as expected");

         /* Re-do over TCP */
         ASSERT_RC(getdns_context_set_dns_transport(context, GETDNS_TRANSPORT_TCP_ONLY),
           GETDNS_RETURN_GOOD, "Return code from getdns_context_set_dns_transport()");   
  
         ASSERT_RC(getdns_general_sync(context, "large.getdnsapi.net", GETDNS_RRTYPE_TXT, extensions, &response), 
           GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");

         ASSERT_RC(getdns_dict_get_int(response, "/call_reporting/0/transport", &transport),
           GETDNS_RETURN_GOOD, "Failed to extract \"transport\"");
         ASSERT_RC(transport, GETDNS_TRANSPORT_TCP, "Query did not go over TCP");
         ASSERT_RC(getdns_dict_get_int(response, "/replies_tree/0/header/tc", &tc),
           GETDNS_RETURN_GOOD, "Failed to extract \"tc\"");
         ASSERT_RC(tc, 0, "Packet trucated - not as expected");

         /* Now let it fall back to TCP */
         ASSERT_RC(getdns_context_set_dns_transport(context, GETDNS_TRANSPORT_UDP_FIRST_AND_FALL_BACK_TO_TCP),
           GETDNS_RETURN_GOOD, "Return code from getdns_context_set_dns_transport()");   
         ASSERT_RC(getdns_general_sync(context, "large.getdnsapi.net", GETDNS_RRTYPE_TXT, extensions, &response), 
           GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");

         ASSERT_RC(getdns_dict_get_int(response, "/call_reporting/0/transport", &transport),
           GETDNS_RETURN_GOOD, "Failed to extract \"transport\"");
         ASSERT_RC(transport, GETDNS_TRANSPORT_TCP, "Query did not go over TCP");
         ASSERT_RC(getdns_dict_get_int(response, "/replies_tree/0/header/tc", &tc),
           GETDNS_RETURN_GOOD, "Failed to extract \"tc\"");
         ASSERT_RC(tc, 0, "Packet trucated - not as expected");
       }
       ASSERT_RC(getdns_context_unset_edns_maximum_udp_payload_size(context),
         GETDNS_RETURN_GOOD, "Return code from getdns_context_unset_edns_maximum_udp_payload_size()");

      CONTEXT_DESTROY;

     }
     END_TEST

     START_TEST (getdns_context_set_dns_transport_recursing_6)
     {
       /*
       *  Request answer larger then 512 bytes but set UDP payload to that
       *  Call getdns_context_set_dns_transport() with value = GETDNS_TRANSPORT_UDP_ONLY
       *  expect: No response returned
       */
       struct getdns_context *context = NULL;
       struct getdns_dict *response = NULL;
       struct getdns_dict *extensions = getdns_dict_create();
       uint32_t status;

       /* Recursive mode does not report the transport used and does not answer
          if the response is trucated. Also, transport can't be changed on a ub ctx.*/

       CONTEXT_CREATE(TRUE);
       /* Need to explicit check as we may be compiled stub-only*/
       getdns_resolution_t resolution_type;
       ASSERT_RC(getdns_context_get_resolution_type(context, &resolution_type),
            GETDNS_RETURN_GOOD, "Return code from getdns_context_get_resolution_type()"); 
       if (resolution_type == GETDNS_RESOLUTION_RECURSING) {

           /* Request a response that should be truncated over UDP */
           ASSERT_RC(getdns_context_set_dns_transport(context, GETDNS_TRANSPORT_UDP_ONLY),
             GETDNS_RETURN_GOOD, "Return code from getdns_context_set_dns_transport()");
           ASSERT_RC(getdns_context_set_edns_maximum_udp_payload_size(context, 512),
               GETDNS_RETURN_GOOD, "Return code from getdns_context_set_edns_maximum_udp_payload_size()");
           ASSERT_RC(getdns_general_sync(context, "large.getdnsapi.net", GETDNS_RRTYPE_TXT, extensions, &response), 
             GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");

           ASSERT_RC(getdns_dict_get_int(response, "status", &status),
             GETDNS_RETURN_GOOD, "Failed to extract \"status\"");
         
             /*  TODO: INVESTIGATE THIS AS IT SHOULDN'T BE A TIMEOUT...*/
           ASSERT_RC(status, GETDNS_RESPSTATUS_ALL_TIMEOUT, "Status not as expected");
      }

      CONTEXT_DESTROY;

     }
     END_TEST

     START_TEST (getdns_context_set_dns_transport_recursing_7)
     {
       /*
        *  Call getdns_context_set_dns_transport() with value = GETDNS_TRANSPORT_TCP_ONLY
        *  expect: Response returned
        */
       struct getdns_context *context = NULL;
       struct getdns_dict *response = NULL;
       struct getdns_dict *extensions = getdns_dict_create();

       /*
        * Not all root servers seem to support TCP reliably. To mitigate,
	* we put our faith in k.root-servers.net.
        */
       struct getdns_list *root_servers = getdns_list_create();
       struct getdns_list *root_servers2 = getdns_list_create();
       struct getdns_bindata k4_root = {  4, (void *)"\xC1\x00\x0E\x81" };
       struct getdns_bindata k6_root = { 16, (void *)
         "\x20\x01\x07\xFD\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01" };
       struct getdns_bindata *answer = NULL;

       uint32_t status;
       uint32_t type;
       uint32_t tc;

       CONTEXT_CREATE(TRUE);
       /* Need to explicit check as we may be compiled stub-only*/
       getdns_resolution_t resolution_type;
       ASSERT_RC(getdns_context_get_resolution_type(context, &resolution_type),
            GETDNS_RETURN_GOOD, "Return code from getdns_context_get_resolution_type()"); 
       if (resolution_type == GETDNS_RESOLUTION_RECURSING) {

           /* Re-do over TCP */
           ASSERT_RC(getdns_dict_set_int(extensions,"return_call_reporting", GETDNS_EXTENSION_TRUE),
             GETDNS_RETURN_GOOD, "Return code from getdns_dict_set_int()");
           ASSERT_RC(getdns_list_set_bindata(root_servers, 0, &k6_root),
             GETDNS_RETURN_GOOD, "Return code from getdns_list_set_bindata()");
           ASSERT_RC(getdns_list_set_bindata(root_servers, 1, &k4_root),
             GETDNS_RETURN_GOOD, "Return code from getdns_list_set_bindata()");

           ASSERT_RC(getdns_context_set_dns_root_servers(context, root_servers),
             GETDNS_RETURN_GOOD, "Return code from getdns_context_set_dns_root_servers()");
           ASSERT_RC(getdns_context_get_dns_root_servers(context, &root_servers2),
             GETDNS_RETURN_GOOD, "Return code from getdns_context_get_dns_root_servers()");
           ASSERT_RC(getdns_list_get_bindata(root_servers2, 0, &answer),
             GETDNS_RETURN_GOOD, "Return code from getdns_list_get_bindata()");
           ck_assert_msg(strncmp((char *)answer->data, (char *)k6_root.data, 16) == 0,
             "Expected answer data to be 2001:7fd::1");
           ASSERT_RC(getdns_list_get_bindata(root_servers2, 1, &answer),
             GETDNS_RETURN_GOOD, "Return code from getdns_list_get_bindata()");
           ck_assert_msg(strncmp((char *)answer->data, (char *)k4_root.data, 4) == 0,
             "Expected answer data to be 193.0.14.129");

           ASSERT_RC(getdns_context_set_dns_transport(context, GETDNS_TRANSPORT_TCP_ONLY),
             GETDNS_RETURN_GOOD, "Return code from getdns_context_set_dns_transport()");
           ASSERT_RC(getdns_context_set_edns_maximum_udp_payload_size(context, 512),
             GETDNS_RETURN_GOOD, "Return code from getdns_context_set_edns_maximum_udp_payload_size()");
           ASSERT_RC(getdns_general_sync(context, "large.getdnsapi.net", GETDNS_RRTYPE_TXT, extensions, &response), 
             GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");

           ASSERT_RC(getdns_dict_get_int(response, "status", &status),
             GETDNS_RETURN_GOOD, "Failed to extract \"status\"");
           ASSERT_RC(status, GETDNS_RESPSTATUS_GOOD, "Status not as expected");
           ASSERT_RC(getdns_dict_get_int(response, "/call_reporting/0/resolution_type", &type),
             GETDNS_RETURN_GOOD, "Failed to extract \"resolution_type\"");
           ASSERT_RC(type, GETDNS_RESOLUTION_RECURSING, "Query did not use Recursive mode");
           ASSERT_RC(getdns_dict_get_int(response, "/replies_tree/0/header/tc", &tc),
             GETDNS_RETURN_GOOD, "Failed to extract \"tc\"");
           ASSERT_RC(tc, 0, "Packet trucated - not as expected");
      }

      CONTEXT_DESTROY;

      getdns_dict_destroy(extensions);
      getdns_list_destroy(root_servers);

     }
     END_TEST

     START_TEST (getdns_context_set_dns_transport_recursing_8)
     {
       /*
       *  Call getdns_context_set_dns_transport() with value = GETDNS_TRANSPORT_UDP_FIRST_AND_FALL_BACK_TO_TCP
       *  expect: Response returned
       */
       struct getdns_context *context = NULL;
       struct getdns_dict *response = NULL;
       struct getdns_dict *extensions = getdns_dict_create();
       uint32_t status;
       uint32_t tc;

       CONTEXT_CREATE(TRUE);
       /* Need to explicit check as we may be compiled stub-only*/
       getdns_resolution_t resolution_type;
       ASSERT_RC(getdns_context_get_resolution_type(context, &resolution_type),
            GETDNS_RETURN_GOOD, "Return code from getdns_context_get_resolution_type()"); 
       if (resolution_type == GETDNS_RESOLUTION_RECURSING) {

           /* Now let it fall back to TCP */
           ASSERT_RC(getdns_context_set_dns_transport(context, GETDNS_TRANSPORT_UDP_FIRST_AND_FALL_BACK_TO_TCP),
             GETDNS_RETURN_GOOD, "Return code from getdns_context_set_dns_transport()");
           ASSERT_RC(getdns_context_set_edns_maximum_udp_payload_size(context, 512),
             GETDNS_RETURN_GOOD, "Return code from getdns_context_set_edns_maximum_udp_payload_size()");
           ASSERT_RC(getdns_general_sync(context, "large.getdnsapi.net", GETDNS_RRTYPE_TXT, extensions, &response), 
             GETDNS_RETURN_GOOD, "Return code from getdns_general_sync()");

           ASSERT_RC(getdns_dict_get_int(response, "status", &status),
             GETDNS_RETURN_GOOD, "Failed to extract \"status\"");
           ASSERT_RC(status, GETDNS_RESPSTATUS_GOOD, "Status not as expected");

           ASSERT_RC(getdns_dict_get_int(response, "/replies_tree/0/header/tc", &tc),
             GETDNS_RETURN_GOOD, "Failed to extract \"tc\"");
           ASSERT_RC(tc, 0, "Packet trucated - not as expected");
      }

      CONTEXT_DESTROY;

     }
     END_TEST

     START_TEST (getdns_context_set_dns_transport_recursing_9)
     {
       /*
        * Check TLS
       */
       struct getdns_context *context = NULL;
       getdns_resolution_t resolution_type;
       getdns_transport_list_t transport_list[1];
       getdns_transport_list_t *transport_list2;
       size_t count;
       getdns_tls_authentication_t auth;
       uint16_t backoff;
       uint16_t retries;

       transport_list[0] = GETDNS_TRANSPORT_TLS;

       CONTEXT_CREATE(TRUE);

       ASSERT_RC(getdns_context_set_resolution_type(context, GETDNS_RESOLUTION_STUB),
         GETDNS_RETURN_GOOD, "Return code from getdns_context_set_resolution_type()");
       ASSERT_RC(getdns_context_get_resolution_type(context, &resolution_type),
         GETDNS_RETURN_GOOD, "Return code from getdns_context_get_resolution_type()");
       ck_assert_msg(resolution_type == GETDNS_RESOLUTION_STUB, "resolution_type should be stub (520), got %d", (int)resolution_type);

       ASSERT_RC(getdns_context_set_dns_transport_list(context, 1, transport_list),
	 GETDNS_RETURN_GOOD, "Return code from getdns_context_set_dns_transport_list()");
       ASSERT_RC(getdns_context_get_dns_transport_list(context, &count, &transport_list2),
         GETDNS_RETURN_GOOD, "Return code from getdns_context_get_dns_transport_list()");
       ck_assert_msg(transport_list2[0] == GETDNS_TRANSPORT_TLS, "transport_list should be 1202 but got %d", (int) transport_list2[0]);

       ASSERT_RC(getdns_context_set_tls_authentication(context, GETDNS_AUTHENTICATION_REQUIRED),
	 GETDNS_RETURN_GOOD, "Return cond from getdns_context_set_tls_authentication()");
       ASSERT_RC(getdns_context_get_tls_authentication(context, &auth),
         GETDNS_RETURN_GOOD, "Return code from getdns_context_get_tls_authentication()");
       ck_assert_msg(auth == 1301, "tls_authentication should be 1301, but got %d", (int) auth);

       ASSERT_RC(getdns_context_set_tls_backoff_time(context, 1000),
         GETDNS_RETURN_GOOD, "Return code from getdns_context_set_tls_backoff_time()");
       ASSERT_RC(getdns_context_get_tls_backoff_time(context, &backoff),
         GETDNS_RETURN_GOOD, "Return code from getdns_context_get_tls_backoff_time()");
       ck_assert_msg(backoff == 1000, "backoff should be 1000, but got %d", (int) backoff);

       ASSERT_RC(getdns_context_set_tls_connection_retries(context, 5),
         GETDNS_RETURN_GOOD, "Return code from getdns_context_set_tls_connection_retries()");
       ASSERT_RC(getdns_context_get_tls_connection_retries(context, &retries),
         GETDNS_RETURN_GOOD, "Return code from getdns_context_get_tls_connection_retries()");
       ck_assert_msg(retries == 5, "retries should be 5 but got %d", (int) retries);

      /*TODO: should extend the test */
      CONTEXT_DESTROY;

     }
     END_TEST




    Suite *
    getdns_context_set_dns_transport_suite (void)
    {
      Suite *s = suite_create ("getdns_context_set_dns_transport()");

      /* Negative test cases */
      TCase *tc_neg = tcase_create("Negative");
      tcase_add_test(tc_neg, getdns_context_set_dns_transport_1);
      tcase_add_test(tc_neg, getdns_context_set_dns_transport_2);
      tcase_add_test(tc_neg, getdns_context_set_dns_transport_list_3);
      tcase_add_test(tc_neg, getdns_context_set_dns_transport_list_4);
      /* TODO: Test which specific lists are not supported */
      suite_add_tcase(s, tc_neg);

      /* Positive test cases */
       TCase *tc_pos = tcase_create("Positive");
       /* TODO: Test which specific lists are supported */
       tcase_add_test(tc_pos, getdns_context_set_dns_transport_stub_5);
       tcase_set_timeout(tc_pos, 6.0); /* test 6 times out after 5 secs */
       tcase_add_test(tc_pos, getdns_context_set_dns_transport_recursing_6);     
       tcase_add_test(tc_pos, getdns_context_set_dns_transport_recursing_7);
       tcase_add_test(tc_pos, getdns_context_set_dns_transport_recursing_8);
       /*  TLS */
       tcase_add_test(tc_pos, getdns_context_set_dns_transport_recursing_9);

       suite_add_tcase(s, tc_pos);

       return s;

    }

#endif
