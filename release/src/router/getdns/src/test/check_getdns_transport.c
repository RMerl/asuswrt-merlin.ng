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


#include "check_getdns_transport.h"
#include "check_getdns_common.h"
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/param.h>
#include <stdlib.h>
#include <sys/time.h>


#define GETDNS_STR_IPV4 "IPv4"
#define GETDNS_STR_IPV6 "IPv6"
#define GETDNS_STR_ADDRESS_TYPE "address_type"
#define GETDNS_STR_ADDRESS_DATA "address_data"
#define GETDNS_STR_PORT "port"
#define TEST_PORT 42100

static uint16_t get_test_port(void)
{
	char    *test_port_str;
	uint16_t test_port;
	struct   timeval tv;

	if (!(test_port_str = getenv("GETDNS_TEST_PORT")) ||
	    !(test_port = (uint16_t)atoi(test_port_str)))
		test_port = TEST_PORT;

	(void)gettimeofday(&tv, NULL);
	srandom((int)getpid() + (int)tv.tv_usec);
	test_port += random() % 1000;
	return test_port;
}

/* utilities to start a junk listener */
typedef struct transport_thread_data {
  uint16_t port;
  volatile int running;
  int udp_count;
  int tcp_count;
} transport_thread_data;

void* run_transport_server(void* data) {
  transport_thread_data* tdata = (transport_thread_data*) data;
  int udp, tcp, conn = 0;
  struct sockaddr_in serv_addr;
  uint8_t mesg[65536], tcplength[2];
  fd_set read_fds;
  size_t pkt_len;
  getdns_context *ctxt;
  getdns_return_t r;
  getdns_dict *dns_msg;
  getdns_bindata *qname;
  char *qname_str;
  uint32_t qtype, qid;
  int udp_count = 0;
  int tcp_count = 0;
  if ((r = getdns_context_create(&ctxt, 1))) {
    fprintf( stderr, "Could not create a getdns context: \"%s\"\n"
           , getdns_get_errorstr_by_id(r));
    return NULL;
  }
  if ((r = getdns_context_set_resolution_type(ctxt, GETDNS_RESOLUTION_STUB))) {
    fprintf( stderr, "Could not set context in stub mode resolution: \"%s\"\n"
           , getdns_get_errorstr_by_id(r));
    return NULL;
  }

  udp = socket(AF_INET, SOCK_DGRAM, 0);
  tcp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  memset(&serv_addr, 0, sizeof (serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(tdata->port);
  bind(udp, (struct sockaddr *) &serv_addr, sizeof (serv_addr));
  bind(tcp, (struct sockaddr *) &serv_addr, sizeof (serv_addr));
  listen(tcp, 5);
  /* signal that it's listening */
  /* dirty timing hack to yield */
  sleep(1);
  tdata->running = 1;
  while (tdata->running) {
    struct sockaddr_in client_addr;
    FD_ZERO(&read_fds);
    FD_SET(udp, &read_fds);
    FD_SET(tcp, &read_fds);
    int n = 0;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    int maxfdp1 = MAX(udp, tcp) + 1;
    int r = select(maxfdp1, &read_fds, NULL, NULL, &tv);
    if (r > 0) {
      socklen_t len = sizeof (client_addr);
      if (FD_ISSET(udp, &read_fds)) {
        n = recvfrom(udp, mesg, 65536, 0, (struct sockaddr *) &client_addr, &len);
        udp_count++;
      } else if (FD_ISSET(tcp, &read_fds)) {
        conn = accept(tcp, (struct sockaddr *) &client_addr, &len);
        /* throw away the length */
        n = read(conn, tcplength, 2);
	assert(n == 2);
        n = read(conn, mesg, 65536);
	assert(n == ((tcplength[0] << 8) | tcplength[1]));
        tcp_count++;
      } else {
        fprintf(stderr, "Timeout in run_transport_server\n");
	break;
      }

      if ((r = getdns_wire2msg_dict(mesg, n, &dns_msg))) {
        fprintf( stderr, "Could not convert wireformat to DNS message dict: \"%s\"\n"
               , getdns_get_errorstr_by_id(r));
        continue;
      }
      if ((r = getdns_dict_get_bindata(dns_msg, "/question/qname", &qname))) {
        fprintf( stderr, "Could not get query name from request dict: \"%s\"\n"
               , getdns_get_errorstr_by_id(r));
        continue;
      }
      if ((r = getdns_convert_dns_name_to_fqdn(qname, &qname_str))) {
        fprintf( stderr, "Could not convert qname to fqdn: \"%s\"\n"
               , getdns_get_errorstr_by_id(r));
        continue;
      }
      if ((r = getdns_dict_get_int(dns_msg, "/question/qtype", &qtype))) {
        fprintf( stderr, "Could not get query type from request dict: \"%s\"\n"
               , getdns_get_errorstr_by_id(r));
        continue;
      }
      if ((r = getdns_dict_get_int(dns_msg, "/header/id", &qid))) {
        fprintf( stderr, "Could not get message ID from request dict: \"%s\"\n"
               , getdns_get_errorstr_by_id(r));
        continue;
      }
      getdns_dict_destroy(dns_msg);
      r = getdns_general_sync(ctxt, qname_str, qtype, NULL, &dns_msg);
      if (r) {
        fprintf( stderr, "Could query for \"%s\" %d: \"%s\"\n", qname_str, (int)qtype
               , getdns_get_errorstr_by_id(r));
        free(qname_str);
        continue;
      }
      free(qname_str);
      if ((r = getdns_dict_set_int(dns_msg, "/replies_tree/0/header/id", qid))) {
        fprintf( stderr, "Could not set message ID on reply dict: \"%s\"\n"
               , getdns_get_errorstr_by_id(r));
        continue;
      }
      pkt_len = sizeof(mesg) - 2;
      if ((r = getdns_msg_dict2wire_buf(dns_msg, mesg + 2, &pkt_len))) {
        fprintf( stderr, "Could not convert DNS message dict to wireformat: \"%s\"\n"
               , getdns_get_errorstr_by_id(r));
      }
      getdns_dict_destroy(dns_msg);

      if (udp_count > 0) {
        sendto(udp, mesg + 2, pkt_len, 
                0, (struct sockaddr *) &client_addr, sizeof (client_addr));
      } else if (conn > 0) {
        /* add length of packet */
        mesg[0] = (uint8_t) ((pkt_len >> 8) & 0xff);
        mesg[1] = (uint8_t) (pkt_len & 0xff);
        if (write(conn, mesg, pkt_len + 2) == -1) return 0;
      }
    } /* End of if */
  } /* end of while loop */
  close(udp);
  close(tcp);
  if (conn > 0) close(conn);
  getdns_context_destroy(ctxt);
  tdata->udp_count = udp_count;
  tdata->tcp_count = tcp_count;
  return NULL;

}

void transport_cb(struct getdns_context *context,
  getdns_callback_type_t callback_type,
  struct getdns_dict * response,
  void *userarg, getdns_transaction_t transaction_id) {
  /* Don't really care about the answer*/
  (void)context; (void)callback_type; (void)response;
  (void)userarg; (void)transaction_id;
  return;
}

START_TEST(getdns_transport_udp_sync) {
  /*
   *  Create a context by calling getdns_context_create()
   *  Create listener thread
   *  Set upstream to localhost:port
   *
   *  getdns_context_set_resolution_type() to GETDNS_RESOLUTION_STUB
   *  expect:  GETDNS_CONTEXT_CODE_RESOLUTION_TYPE
   */

  struct getdns_context *context = NULL;
  struct getdns_dict* server_dict;
  getdns_dict* response = NULL;
  struct getdns_list* upstream_list;
  struct getdns_bindata bindata;
  uint32_t local_addr = htonl(0x7F000001);
  pthread_t thread;
  transport_thread_data t_data;
  t_data.running = 0;
  t_data.udp_count = 0;
  t_data.tcp_count = 0;
  t_data.port = get_test_port();

  pthread_create(&thread, NULL, run_transport_server, (void *) &t_data);

  while (!t_data.running) {
    sleep(1);
  }

  /* set up */
  CONTEXT_CREATE(TRUE);
  server_dict = getdns_dict_create_with_context(context);
  ck_assert_msg(server_dict != NULL, "Allocate IP dictionary failed");
  bindata.size = strlen(GETDNS_STR_IPV4) + 1;
  bindata.data = (uint8_t*) GETDNS_STR_IPV4;
  ASSERT_RC(getdns_dict_set_bindata(server_dict, GETDNS_STR_ADDRESS_TYPE, &bindata),
    GETDNS_RETURN_GOOD, "set ip bindata");
  bindata.size = 4;
  bindata.data = (uint8_t*) & local_addr;
  ASSERT_RC(getdns_dict_set_bindata(server_dict, GETDNS_STR_ADDRESS_DATA, &bindata),
    GETDNS_RETURN_GOOD, "set addr bindata");
  ASSERT_RC(getdns_dict_set_int(server_dict, GETDNS_STR_PORT, t_data.port),
    GETDNS_RETURN_GOOD, "set addr port");

  upstream_list = getdns_list_create_with_context(context);
  ck_assert_msg(upstream_list != NULL, "Allocate lists");

  ASSERT_RC(getdns_list_set_dict(upstream_list, 0, server_dict),
    GETDNS_RETURN_GOOD, "set upstream");

  ASSERT_RC(getdns_context_set_upstream_recursive_servers(context, upstream_list),
    GETDNS_RETURN_GOOD, "set rec servers");

  ASSERT_RC(getdns_context_set_dns_transport(context, GETDNS_TRANSPORT_UDP_ONLY),
    GETDNS_RETURN_GOOD, "Bad return code from setting udp transport");

  /* stub */
  ASSERT_RC(getdns_context_set_resolution_type(context, GETDNS_RESOLUTION_STUB),
    GETDNS_RETURN_GOOD, "Return code from getdns_context_set_resolution_type()");

  ASSERT_RC(getdns_general_sync(context, "getdnsapi.net", GETDNS_RRTYPE_A, NULL, &response),
    GETDNS_RETURN_GOOD, "Bad return code from getdns_general_sync");

  CONTEXT_DESTROY;

  t_data.running = 0;
  pthread_join(thread, NULL);
  ck_assert_msg(t_data.udp_count >= 1, "udp_count !>= 1");
  ck_assert_msg(t_data.tcp_count == 0, "tcp_count != 0");

}

END_TEST

START_TEST(getdns_transport_tcp_sync) {
  /*
   *  Create a context by calling getdns_context_create()
   *  Create listener thread
   *  Set upstream to localhost:port
   *
   *  getdns_context_set_resolution_type() to GETDNS_RESOLUTION_STUB
   *  expect:  GETDNS_CONTEXT_CODE_RESOLUTION_TYPE
   */

  struct getdns_context *context = NULL;
  struct getdns_dict* server_dict;
  getdns_dict* response = NULL;
  struct getdns_list* upstream_list;
  struct getdns_bindata bindata;
  uint32_t local_addr = htonl(0x7F000001);
  pthread_t thread;
  transport_thread_data t_data;
  t_data.running = 0;
  t_data.udp_count = 0;
  t_data.tcp_count = 0;
  t_data.port = get_test_port();

  pthread_create(&thread, NULL, run_transport_server, (void *) &t_data);

  while (!t_data.running) {
    sleep(1);
  }

  /* set up */
  CONTEXT_CREATE(TRUE);
  server_dict = getdns_dict_create_with_context(context);
  ck_assert_msg(server_dict != NULL, "Allocate IP dictionary failed");
  bindata.size = strlen(GETDNS_STR_IPV4) + 1;
  bindata.data = (uint8_t*) GETDNS_STR_IPV4;
  ASSERT_RC(getdns_dict_set_bindata(server_dict, GETDNS_STR_ADDRESS_TYPE, &bindata),
    GETDNS_RETURN_GOOD, "set ip bindata");
  bindata.size = 4;
  bindata.data = (uint8_t*) & local_addr;
  ASSERT_RC(getdns_dict_set_bindata(server_dict, GETDNS_STR_ADDRESS_DATA, &bindata),
    GETDNS_RETURN_GOOD, "set addr bindata");
  ASSERT_RC(getdns_dict_set_int(server_dict, GETDNS_STR_PORT, t_data.port),
    GETDNS_RETURN_GOOD, "set addr port");

  upstream_list = getdns_list_create_with_context(context);
  ck_assert_msg(upstream_list != NULL, "Allocate lists");

  ASSERT_RC(getdns_list_set_dict(upstream_list, 0, server_dict),
    GETDNS_RETURN_GOOD, "set upstream");

  ASSERT_RC(getdns_context_set_upstream_recursive_servers(context, upstream_list),
    GETDNS_RETURN_GOOD, "set rec servers");

  ASSERT_RC(getdns_context_set_dns_transport(context, GETDNS_TRANSPORT_TCP_ONLY),
    GETDNS_RETURN_GOOD, "Bad return code from setting tcp transport");

  /* stub */
  ASSERT_RC(getdns_context_set_resolution_type(context, GETDNS_RESOLUTION_STUB),
    GETDNS_RETURN_GOOD, "Return code from getdns_context_set_resolution_type()");

  ASSERT_RC(getdns_general_sync(context, "getdnsapi.net", GETDNS_RRTYPE_A, NULL, &response),
    GETDNS_RETURN_GOOD, "Bad return code from getdns_general_sync");

  CONTEXT_DESTROY;

  t_data.running = 0;
  pthread_join(thread, NULL);
  ck_assert_msg(t_data.udp_count == 0, "udp_count != 0");
  ck_assert_msg(t_data.tcp_count >= 1, "tcp_count !>= 1");

}

END_TEST

START_TEST(getdns_transport_udp_async) {
  /*
   *  Create a context by calling getdns_context_create()
   *  Create listener thread
   *  Set upstream to localhost:port
   *
   *  getdns_context_set_resolution_type() to GETDNS_RESOLUTION_STUB
   *  expect:  GETDNS_CONTEXT_CODE_RESOLUTION_TYPE
   */

  struct getdns_context *context = NULL;
  void* eventloop = NULL;
  struct getdns_dict* server_dict;
  struct getdns_list* upstream_list;
  struct getdns_bindata bindata;
  uint32_t local_addr = htonl(0x7F000001);
  pthread_t thread;
  transport_thread_data t_data;
  t_data.running = 0;
  t_data.udp_count = 0;
  t_data.tcp_count = 0;
  t_data.port = get_test_port();

  pthread_create(&thread, NULL, run_transport_server, (void *) &t_data);

  while (!t_data.running) {
    sleep(1);
  }

  /* set up */
  CONTEXT_CREATE(TRUE);
  server_dict = getdns_dict_create_with_context(context);
  ck_assert_msg(server_dict != NULL, "Allocate IP dictionary failed");
  bindata.size = strlen(GETDNS_STR_IPV4) + 1;
  bindata.data = (uint8_t*) GETDNS_STR_IPV4;
  ASSERT_RC(getdns_dict_set_bindata(server_dict, GETDNS_STR_ADDRESS_TYPE, &bindata),
    GETDNS_RETURN_GOOD, "set ip bindata");
  bindata.size = 4;
  bindata.data = (uint8_t*) & local_addr;
  ASSERT_RC(getdns_dict_set_bindata(server_dict, GETDNS_STR_ADDRESS_DATA, &bindata),
    GETDNS_RETURN_GOOD, "set addr bindata");
  ASSERT_RC(getdns_dict_set_int(server_dict, GETDNS_STR_PORT, t_data.port),
    GETDNS_RETURN_GOOD, "set addr port");

  upstream_list = getdns_list_create_with_context(context);
  ck_assert_msg(upstream_list != NULL, "Allocate lists");

  ASSERT_RC(getdns_list_set_dict(upstream_list, 0, server_dict),
    GETDNS_RETURN_GOOD, "set upstream");

  ASSERT_RC(getdns_context_set_upstream_recursive_servers(context, upstream_list),
    GETDNS_RETURN_GOOD, "set rec servers");

  ASSERT_RC(getdns_context_set_dns_transport(context, GETDNS_TRANSPORT_UDP_ONLY),
    GETDNS_RETURN_GOOD, "Bad return code from setting udp transport");

  /* stub */
  ASSERT_RC(getdns_context_set_resolution_type(context, GETDNS_RESOLUTION_STUB),
    GETDNS_RETURN_GOOD, "Return code from getdns_context_set_resolution_type()");

  EVENT_BASE_CREATE;

  ASSERT_RC(getdns_general(context, "getdnsapi.net", GETDNS_RRTYPE_A, NULL, &t_data, NULL, transport_cb),
    GETDNS_RETURN_GOOD, "Bad return code from getdns_general_sync");

  RUN_EVENT_LOOP;

  CONTEXT_DESTROY;

  t_data.running = 0;
  pthread_join(thread, NULL);
  ck_assert_msg(t_data.udp_count >= 1, "udp_count !>= 1");
  ck_assert_msg(t_data.tcp_count == 0, "tcp_count != 0");

}

END_TEST

START_TEST(getdns_transport_tcp_async) {
  /*
   *  Create a context by calling getdns_context_create()
   *  Create listener thread
   *  Set upstream to localhost:port
   *
   *  getdns_context_set_resolution_type() to GETDNS_RESOLUTION_STUB
   *  expect:  GETDNS_CONTEXT_CODE_RESOLUTION_TYPE
   */

  struct getdns_context *context = NULL;
  void* eventloop = NULL;
  struct getdns_dict* server_dict;
  struct getdns_list* upstream_list;
  struct getdns_bindata bindata;
  uint32_t local_addr = htonl(0x7F000001);
  pthread_t thread;
  transport_thread_data t_data;
  t_data.running = 0;
  t_data.udp_count = 0;
  t_data.tcp_count = 0;
  t_data.port = get_test_port();

  pthread_create(&thread, NULL, run_transport_server, (void *) &t_data);

  while (!t_data.running) {
    sleep(1);
  }

  /* set up */
  CONTEXT_CREATE(TRUE);
  server_dict = getdns_dict_create_with_context(context);
  ck_assert_msg(server_dict != NULL, "Allocate IP dictionary failed");
  bindata.size = strlen(GETDNS_STR_IPV4) + 1;
  bindata.data = (uint8_t*) GETDNS_STR_IPV4;
  ASSERT_RC(getdns_dict_set_bindata(server_dict, GETDNS_STR_ADDRESS_TYPE, &bindata),
    GETDNS_RETURN_GOOD, "set ip bindata");
  bindata.size = 4;
  bindata.data = (uint8_t*) & local_addr;
  ASSERT_RC(getdns_dict_set_bindata(server_dict, GETDNS_STR_ADDRESS_DATA, &bindata),
    GETDNS_RETURN_GOOD, "set addr bindata");
  ASSERT_RC(getdns_dict_set_int(server_dict, GETDNS_STR_PORT, t_data.port),
    GETDNS_RETURN_GOOD, "set addr port");

  upstream_list = getdns_list_create_with_context(context);
  ck_assert_msg(upstream_list != NULL, "Allocate lists");

  ASSERT_RC(getdns_list_set_dict(upstream_list, 0, server_dict),
    GETDNS_RETURN_GOOD, "set upstream");

  ASSERT_RC(getdns_context_set_upstream_recursive_servers(context, upstream_list),
    GETDNS_RETURN_GOOD, "set rec servers");

  ASSERT_RC(getdns_context_set_dns_transport(context, GETDNS_TRANSPORT_TCP_ONLY),
    GETDNS_RETURN_GOOD, "Bad return code from setting tcp transport");

  /* stub */
  ASSERT_RC(getdns_context_set_resolution_type(context, GETDNS_RESOLUTION_STUB),
    GETDNS_RETURN_GOOD, "Return code from getdns_context_set_resolution_type()");

  EVENT_BASE_CREATE;

  ASSERT_RC(getdns_general(context, "getdnsapi.net", GETDNS_RRTYPE_A, NULL, &t_data, NULL, transport_cb),
    GETDNS_RETURN_GOOD, "Bad return code from getdns_general_sync");

  RUN_EVENT_LOOP;

  CONTEXT_DESTROY;

  t_data.running = 0;
  pthread_join(thread, NULL);
  ck_assert_msg(t_data.udp_count == 0, "udp_count != 0");
  ck_assert_msg(t_data.tcp_count >= 1, "tcp_count !>= 1");

}

END_TEST




Suite *
getdns_transport_suite(void) {
  Suite *s = suite_create("getdns_transport()");

  /* Note that the exact number of messages received depends on if a trust
   * anchor is configured so these tests just check that no messages are
   * received on the wrong transport and at least one is received on the
   * expected transport */

  /* Positive test cases */
  TCase *tc_pos = tcase_create("Positive");
  tcase_set_timeout(tc_pos, 15.0);
  tcase_add_test(tc_pos, getdns_transport_udp_sync);
  tcase_add_test(tc_pos, getdns_transport_tcp_sync);
  tcase_add_test(tc_pos, getdns_transport_udp_async);
  tcase_add_test(tc_pos, getdns_transport_tcp_async);
  suite_add_tcase(s, tc_pos);

  return s;

}

