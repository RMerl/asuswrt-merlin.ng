/* $Id: tsx_basic_test.c 3553 2011-05-05 06:14:19Z nanang $ */
/* 
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#include "test.h"
#include <pjsip.h>
#include <pjlib.h>

#define THIS_FILE   "tsx_basic_test.c"

static char TARGET_URI[PJSIP_MAX_URL_SIZE];
static char FROM_URI[PJSIP_MAX_URL_SIZE];


/* Test transaction layer. */
static int tsx_layer_test(void)
{
    pj_str_t target, from, tsx_key;
    pjsip_tx_data *tdata;
    pjsip_transaction *tsx, *found;
    pj_status_t status;

    PJ_LOG(3,(THIS_FILE, "  transaction layer test"));

    target = pj_str(TARGET_URI);
    from = pj_str(FROM_URI);

    status = pjsip_endpt_create_request(endpt, &pjsip_invite_method, &target,
					&from, &target, NULL, NULL, -1, NULL,
					&tdata);
    if (status != PJ_SUCCESS) {
	app_perror("  error: unable to create request", status);
	return -110;
    }

    status = pjsip_tsx_create_uac(0, NULL, tdata, &tsx);
    if (status != PJ_SUCCESS) {
	app_perror("   error: unable to create transaction", status);
	return -120;
    }

    pj_strdup(tdata->pool, &tsx_key, &tsx->transaction_key);

    found = pjsip_tsx_layer_find_tsx(0, &tsx_key, PJ_FALSE);
    if (found != tsx) {
	return -130;
    }

    pjsip_tsx_terminate(tsx, PJSIP_SC_REQUEST_TERMINATED);
    flush_events(500);

    if (pjsip_tx_data_dec_ref(tdata) != PJSIP_EBUFDESTROYED) {
	return -140;
    }

    return 0;
}

/* Double terminate test. */
static int double_terminate(void)
{
    pj_str_t target, from, tsx_key;
    pjsip_tx_data *tdata;
    pjsip_transaction *tsx;
    pj_status_t status;

    PJ_LOG(3,(THIS_FILE, "  double terminate test"));

    target = pj_str(TARGET_URI);
    from = pj_str(FROM_URI);

    /* Create request. */
    status = pjsip_endpt_create_request(endpt, &pjsip_invite_method, &target,
					&from, &target, NULL, NULL, -1, NULL,
					&tdata);
    if (status != PJ_SUCCESS) {
	app_perror("  error: unable to create request", status);
	return -10;
    }

    /* Create transaction. */
    status = pjsip_tsx_create_uac(0, NULL, tdata, &tsx);
    if (status != PJ_SUCCESS) {
	app_perror("   error: unable to create transaction", status);
	return -20;
    }

    /* Save transaction key for later. */
    pj_strdup_with_null(tdata->pool, &tsx_key, &tsx->transaction_key);

    /* Add reference to transmit buffer (tsx_send_msg() will dec txdata). */
    pjsip_tx_data_add_ref(tdata);

    /* Send message to start timeout timer. */
    status = pjsip_tsx_send_msg(tsx, NULL);

    /* Terminate transaction. */
    status = pjsip_tsx_terminate(tsx, PJSIP_SC_REQUEST_TERMINATED);
    if (status != PJ_SUCCESS) {
	app_perror("   error: unable to terminate transaction", status);
	return -30;
    }

    tsx = pjsip_tsx_layer_find_tsx(0, &tsx_key, PJ_TRUE);
    if (tsx) {
	/* Terminate transaction again. */
	pjsip_tsx_terminate(tsx, PJSIP_SC_REQUEST_TERMINATED);
	if (status != PJ_SUCCESS) {
	    app_perror("   error: unable to terminate transaction", status);
	    return -40;
	}
	pj_grp_lock_release(tsx->grp_lock);
    }

    flush_events(500);
    if (pjsip_tx_data_dec_ref(tdata) != PJSIP_EBUFDESTROYED) {
	return -50;
    }

    return PJ_SUCCESS;
}

int tsx_basic_test(struct tsx_test_param *param)
{
    int status;

    pj_ansi_sprintf(TARGET_URI, "sip:bob@127.0.0.1:%d;transport=%s", 
		    param->port, param->tp_type);
    pj_ansi_sprintf(FROM_URI, "sip:alice@127.0.0.1:%d;transport=%s", 
		    param->port, param->tp_type);

    status = tsx_layer_test();
    if (status != 0)
	return status;

    status = double_terminate();
    if (status != 0)
	return status;

    return 0;
}

/**************************************************************************/

struct tsx_test_state
{
    int pool_cnt;
};

static void save_tsx_test_state(struct tsx_test_state *st)
{
    st->pool_cnt = caching_pool.used_count;
}

static pj_status_t check_tsx_test_state(struct tsx_test_state *st)
{
    if (caching_pool.used_count > st->pool_cnt)
	return -1;

    return 0;
}

static void destroy_endpt()
{
    pjsip_endpt_destroy(endpt);
    endpt = NULL;
}

static pj_status_t init_endpt()
{
    pj_str_t ns = { "10.187.27.172", 13};	/* just a random, unreachable IP */
    pj_dns_resolver *resolver;
    pj_status_t rc;

    rc = pjsip_endpt_create(&caching_pool.factory, "endpt", &endpt);
    if (rc != PJ_SUCCESS) {
	app_perror("pjsip_endpt_create", rc);
	return rc;
    }

    /* Start transaction layer module. */
    rc = pjsip_tsx_layer_init_module(endpt);
    if (rc != PJ_SUCCESS) {
	app_perror("tsx_layer_init", rc);
	return rc;
    }

    rc = pjsip_udp_transport_start(endpt, NULL, NULL, 1,  NULL);
    if (rc != PJ_SUCCESS) {
	app_perror("udp init", rc);
	return rc;
    }

    rc = pjsip_tcp_transport_start(endpt, NULL, 1, NULL);
    if (rc != PJ_SUCCESS) {
	app_perror("tcp init", rc);
	return rc;
    }

    rc = pjsip_endpt_create_resolver(endpt, &resolver);
    if (rc != PJ_SUCCESS) {
	app_perror("create resolver", rc);
	return rc;
    }

    pj_dns_resolver_set_ns(resolver, 1, &ns, NULL);

    rc = pjsip_endpt_set_resolver(endpt, resolver);
    if (rc != PJ_SUCCESS) {
	app_perror("set resolver", rc);
	return rc;
    }

    return PJ_SUCCESS;
}

static int tsx_create_and_send_req(void *arg)
{
    pj_str_t dst_uri = pj_str((char*)arg);
    pj_str_t from_uri = pj_str((char*)"<sip:user@host>");
    pjsip_tx_data *tdata;
    pj_status_t status;

    status = pjsip_endpt_create_request(endpt, &pjsip_options_method,
                                        &dst_uri, &from_uri, &dst_uri,
                                        NULL, NULL, -1, NULL,
                                        &tdata);
    if (status != PJ_SUCCESS)
	return status;

    status = pjsip_endpt_send_request(endpt, tdata, -1, NULL, NULL);
    if (status != PJ_SUCCESS)
	return status;

    return PJ_SUCCESS;
}

int tsx_destroy_test()
{
    struct tsx_test_state state;
    struct test_desc
    {
	const char *title;
	int (*func)(void*);
	void *arg;
	int sleep_before_unload;
	int sleep_after_unload;
    } test_entries[] =
    {
	{
	    "normal unable to resolve",
	    &tsx_create_and_send_req,
	    "sip:user@somehost",
	    10000,
	    1
	},
	{
	    "resolve and destroy, wait",
	    &tsx_create_and_send_req,
	    "sip:user@somehost",
	    1,
	    10000
	},
	{
	    "tcp connect and destroy",
	    &tsx_create_and_send_req,
	    "sip:user@10.125.36.63:58517;transport=tcp",
	    60000,
	    1000
	},
	{
	    "tcp connect and destroy",
	    &tsx_create_and_send_req,
	    "sip:user@10.125.36.63:58517;transport=tcp",
	    1,
	    60000
	},

    };
    int rc;
    unsigned i;
    //const int INDENT = 2;

    //pj_log_add_indent(INDENT);
    destroy_endpt();

    for (i=0; i<PJ_ARRAY_SIZE(test_entries); ++i) {
	struct test_desc *td = &test_entries[i];

	PJ_LOG(3,(THIS_FILE, "%s", td->title));

	//pj_log_add_indent(INDENT);
	save_tsx_test_state(&state);

	rc = init_endpt();
	if (rc != PJ_SUCCESS) {
	    //pj_log_add_indent(-INDENT*2);
	    return -10;
	}

	rc = td->func(td->arg);
	if (rc != PJ_SUCCESS) {
	    //pj_log_add_indent(-INDENT*2);
	    return -20;
	}

	flush_events(td->sleep_before_unload);
	//pjsip_tsx_layer_destroy();
	flush_events(td->sleep_after_unload);
	destroy_endpt();

	rc = check_tsx_test_state(&state);
	if (rc != PJ_SUCCESS) {
	    init_endpt();
	    //pj_log_add_indent(-INDENT*2);
	    return -30;
	}

	//pj_log_add_indent(-INDENT);
    }

    init_endpt();

    //pj_log_add_indent(-INDENT);
    return 0;
}

