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


#include "check_getdns_context_set_timeout.h"
#include "check_getdns_common.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>


/*
 **************************************************************************
 *                                                                        *
 *  T E S T S  F O R  G E T D N S _ C O N T E X T _ S E T _ TIMEOUT       *
 *                                                                        *
 **************************************************************************
*/

START_TEST (getdns_context_set_timeout_1)
{
 /*
  *  context is NULL
  *  expect:  GETDNS_RETURN_INVALID_PARAMETER
  */

  struct getdns_context *context = NULL;

  ASSERT_RC(getdns_context_set_timeout(context, 1000),
    GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_context_set_timeout()");

}
END_TEST

START_TEST (getdns_context_set_timeout_2)
{
 /*
  *  timeout is 0
  *  expect: GETDNS_RETURN_INVALID_PARAMETER
  */

  struct getdns_context *context = NULL;
  CONTEXT_CREATE(TRUE);

  ASSERT_RC(getdns_context_set_timeout(context, 0),
    GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_context_set_timeout()");

  CONTEXT_DESTROY;

}
END_TEST

START_TEST (getdns_context_set_idle_timeout_1)
{
 /*
  *  context is NULL
  *  expect:  GETDNS_RETURN_INVALID_PARAMETER
  */

  struct getdns_context *context = NULL;

  ASSERT_RC(getdns_context_set_idle_timeout(context, 1000),
    GETDNS_RETURN_INVALID_PARAMETER, "Return code from getdns_context_set_timeout()");

}
END_TEST

START_TEST (getdns_context_set_idle_timeout_2)
{
 /*
  *  timeout is 0 and then 100
  *  expect: GETDNS_RETURN_GOOD
  */

  struct getdns_context *context = NULL;
  uint64_t time;
  CONTEXT_CREATE(TRUE);

  ASSERT_RC(getdns_context_set_idle_timeout(context, 0),
    GETDNS_RETURN_GOOD, "Return code from getdns_context_set_timeout()");
  ASSERT_RC(getdns_context_set_idle_timeout(context, 100),
    GETDNS_RETURN_GOOD, "Return code from getdns_context_set_timeout()");
  ASSERT_RC(getdns_context_get_idle_timeout(context, &time),
    GETDNS_RETURN_GOOD, "Return code from getdns_context_set_timeout()");
  ck_assert_msg(time == 100, "idle_timeout should be 100, got %d", (int)time);

  CONTEXT_DESTROY;

}
END_TEST

#define GETDNS_STR_IPV4 "IPv4"
#define GETDNS_STR_IPV6 "IPv6"
#define GETDNS_STR_ADDRESS_TYPE "address_type"
#define GETDNS_STR_ADDRESS_DATA "address_data"
#define GETDNS_STR_PORT "port"
#define TEST_PORT 43210

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

/* utilities to start a junk udp listener */
typedef struct timeout_thread_data {
    uint16_t port;
    volatile int running;
    int num_callbacks;
    int num_timeouts;
} timeout_thread_data;

typedef struct queued_response {
    struct sockaddr_in client_addr;
    getdns_dict *reply;
} queued_response;

void* run_server(void* data) {
    timeout_thread_data* tdata = (timeout_thread_data*)data;
    int fd;
    struct sockaddr_in serv_addr;
    uint8_t mesg[65536];
    fd_set read_fds;
    getdns_context *ctxt;
    getdns_return_t r;
    getdns_dict *dns_msg;
    getdns_bindata *qname;
    char *qname_str;
    uint32_t qtype, qid;
    int num_received = 0;
    queued_response responses[10];

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

    fd=socket(AF_INET,SOCK_DGRAM,0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(tdata->port);
    bind(fd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));

    /* signal that it's listening */
    /* dirty timing hack to yield */
    sleep(1);
    tdata->running = 1;
    /* queue up query responses to send out, and delay sending them
     * for a second */
    while (tdata->running) {
        struct sockaddr_in client_addr;
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int r = select(fd + 1, &read_fds, NULL, NULL, &tv);
        if (r > 0 && num_received < 10) {
            socklen_t len = sizeof(client_addr);
            int n = recvfrom(fd,mesg,65536,0,(struct sockaddr *)&(responses[num_received].client_addr),&len);
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
            r = getdns_general_sync(ctxt, qname_str, qtype, NULL, &responses[num_received].reply);
            if (r) {
                fprintf( stderr, "Could query for \"%s\" %d: \"%s\"\n", qname_str, (int)qtype
                       , getdns_get_errorstr_by_id(r));
                free(qname_str);
                continue;
            }
            free(qname_str);
            if ((r = getdns_dict_set_int(responses[num_received].reply, "/replies_tree/0/header/id", qid))) {
                fprintf( stderr, "Could not set message ID on reply dict: \"%s\"\n"
                       , getdns_get_errorstr_by_id(r));
                continue;
            }
            ++num_received;
        } else if (r == 0 && num_received > 0) {
            int i = 0;
            /* timeout - see if we have anything to send */
            for (i = 0; i < num_received; ++i) {
                size_t pkt_len = sizeof(mesg);
                if ((r = getdns_msg_dict2wire_buf(responses[i].reply, mesg, &pkt_len))) {
                    fprintf( stderr, "Could not convert DNS message dict to wireformat: \"%s\"\n"
                           , getdns_get_errorstr_by_id(r));
                }
                sendto(fd,mesg,pkt_len,0,(struct sockaddr *)&(responses[i].client_addr),sizeof(client_addr));
                getdns_dict_destroy(responses[i].reply);
            }
            num_received = 0;
        }
    }
    getdns_context_destroy(ctxt);
    return NULL;

}

void timeout_3_cb(struct getdns_context *context,
                  getdns_callback_type_t callback_type,
                  struct getdns_dict * response,
                  void *userarg, getdns_transaction_t transaction_id) {
    (void)response; (void)transaction_id;
    timeout_thread_data *tdata = (timeout_thread_data*)userarg;
    tdata->num_callbacks++;
    if (callback_type == GETDNS_CALLBACK_TIMEOUT) {
        tdata->num_timeouts++;
    }
    if (tdata->num_callbacks == 1) {
        /* set timeout to 2 seconds and then issue request */
        getdns_context_set_timeout(context, 500);
        getdns_general(context, "getdnsapi.org", GETDNS_RRTYPE_A, NULL,
                       tdata, NULL, timeout_3_cb);
    }
}

/* Temporarily disabled because of unpredictable results */
START_TEST (getdns_context_set_timeout_3)
{
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

  timeout_thread_data t_data;
  t_data.running = 0;
  t_data.num_callbacks = 0;
  t_data.num_timeouts = 0;
  uint64_t timeout;
  t_data.port = get_test_port();

  pthread_create(&thread, NULL, run_server, (void *)&t_data);

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
  bindata.data = (uint8_t*) &local_addr;
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

  /* stub */
  ASSERT_RC(getdns_context_set_resolution_type(context, GETDNS_RESOLUTION_STUB),
    GETDNS_RETURN_GOOD, "Return code from getdns_context_set_resolution_type()");

  EVENT_BASE_CREATE;

  getdns_general(context, "getdnsapi.net", GETDNS_RRTYPE_A, NULL,
                 &t_data, NULL, timeout_3_cb);

  RUN_EVENT_LOOP;

  ASSERT_RC(getdns_context_get_timeout(context, &timeout),
    GETDNS_RETURN_GOOD, "Return code from getdns_context_get_timeout()");
  ck_assert_msg(timeout == 500, "timeout should be 500, got %d", (int)timeout);

  CONTEXT_DESTROY;

  t_data.running = 0;
  pthread_join(thread, NULL);
  ck_assert_msg(t_data.num_callbacks == 2, "callbacks != 2");
  ck_assert_msg(t_data.num_timeouts == 1, "timeouts != 1");

}
END_TEST



Suite *
getdns_context_set_timeout_suite (void)
{
  Suite *s = suite_create ("getdns_context_set_timeout()");

  /* Negative test caseis */
  TCase *tc_neg = tcase_create("Negative");
  tcase_add_test(tc_neg, getdns_context_set_timeout_1);
  tcase_add_test(tc_neg, getdns_context_set_timeout_2);
  tcase_add_test(tc_neg, getdns_context_set_idle_timeout_1);
  tcase_add_test(tc_neg, getdns_context_set_idle_timeout_2);
  suite_add_tcase(s, tc_neg);

  /* Positive test cases */
  /* Temporarily disabled because of unpredictable results */
  TCase *tc_pos = tcase_create("Positive");
  tcase_set_timeout(tc_pos, 15.0);
  tcase_add_test(tc_pos, getdns_context_set_timeout_3);
  suite_add_tcase(s, tc_pos);

   return s;

}

