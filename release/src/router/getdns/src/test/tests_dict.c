/**
 * \file
 * unit tests for getdns_dict helper routines, these should be used to
 * perform regression tests, output must be unchanged from canonical output
 * stored with the sources
 */

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "testmessages.h"
#include "getdns/getdns.h"

#define TSTMSGBUF 80

/*---------------------------------------- tst_bindatasetget */
/**
 * test the bindata get and set routines 
 */
void
tst_bindatasetget(void)
{
	char msg[TSTMSGBUF];
	char key[20];
	getdns_return_t retval;
	struct getdns_dict *dict = NULL;
	struct getdns_bindata *ans_bdata;
	struct getdns_bindata *bindata;

	tstmsg_case_begin("tst_bindatasetget");

	dict = getdns_dict_create();

	/* test int get function against empty dict and with bogus params */

	strcpy(key, "foo");

	tstmsg_case_msg("getdns_dict_get_bindata() empty dict");
	retval = getdns_dict_get_bindata(NULL, key, &ans_bdata);
	snprintf(msg, sizeof(msg),
	    "test 1: getdns_dict_get_bindata(NULL, key, &ans_bdata),retval = %d",
	    retval);
	tstmsg_case_msg(msg);

	retval = getdns_dict_get_bindata(dict, key, NULL);
	snprintf(msg, sizeof(msg),
	    "test 2: getdns_dict_get_bindata(dict, key, NULL),retval = %d",
	    retval);
	tstmsg_case_msg(msg);

	tstmsg_case_msg("getdns_dict_get_bindata(dict, NULL, &ans_bindata)");
	retval = getdns_dict_get_bindata(dict, NULL, &ans_bdata);
	snprintf(msg, sizeof(msg), "test 3: getdns_dict_get_bindata,retval = %d", retval);
	tstmsg_case_msg(msg);

	tstmsg_case_msg("getdns_dict_get_bindata(dict, key, &ans_bdata)");
	retval = getdns_dict_get_bindata(dict, key, &ans_bdata);
	snprintf(msg, sizeof(msg), "test 4: getdns_list_get_bindata,retval = %d", retval);
	tstmsg_case_msg(msg);

	getdns_dict_destroy(dict);

	/* TODO: test getdns_dict_set functions with bogus params */

	/* test set and get legitimate use case */

	dict = getdns_dict_create();

	strcpy(key, "foo");
	bindata =
	    (struct getdns_bindata *) malloc(sizeof(struct getdns_bindata));
	bindata->size = strlen("foobar") + 1;
	bindata->data = (void *) strdup("foobar");

	tstmsg_case_msg("getdns_dict_set_bindata(dict, key, bindata)");
	retval = getdns_dict_set_bindata(dict, key, bindata);
	snprintf(msg, sizeof(msg), "test 5: getdns_dict_set_bindata,retval=%d,key=%s",
	    retval, key);
	tstmsg_case_msg(msg);

	tstmsg_case_msg("getdns_dict_get_bindata(dict, key, &ans_bdata)");
	retval = getdns_dict_get_bindata(dict, key, &ans_bdata);
	snprintf(msg, sizeof(msg),
	    "test 6: getdns_dict_get_bindata,retval=%d,key=%s,data=%s",
	    retval, key, ans_bdata->data);
	tstmsg_case_msg(msg);

	getdns_dict_destroy(dict);
	free(bindata->data);
	free(bindata);

	tstmsg_case_end();

	return;
}				/* tst_bindatasetget */

/*---------------------------------------- tst_dictsetget */
/**
 * test the dict get and set routines 
 */
void
tst_dictsetget(void)
{
	char msg[TSTMSGBUF];
	char key[20];
	uint32_t int1;
	uint32_t int2;
	getdns_return_t retval;
	struct getdns_dict *newdict;
	struct getdns_dict *ansdict;
	struct getdns_dict *dict = NULL;

	tstmsg_case_begin("tst_dictsetget");

	dict = getdns_dict_create();

	/* test get function against empty list and with bogus params */

	strcpy(key, "foo");

	tstmsg_case_msg("getdns_dict_get_dict() empty dict");
	retval = getdns_dict_get_dict(NULL, key, &ansdict);
	snprintf(msg, sizeof(msg),
	    "test 7: getdns_dict_get_dict(NULL, key, &ansdict),retval = %d",
	    retval);
	tstmsg_case_msg(msg);

	retval = getdns_dict_get_dict(dict, key, NULL);
	snprintf(msg, sizeof(msg),
	    "test 8: getdns_dict_get_dict(dict, key, NULL),retval = %d",
	    retval);
	tstmsg_case_msg(msg);

	tstmsg_case_msg("getdns_dict_get_dict(dict, NULL, &ansdict)");
	retval = getdns_dict_get_dict(dict, NULL, &ansdict);
	snprintf(msg, sizeof(msg), "test 9: getdns_dict_get_dict,retval = %d", retval);
	tstmsg_case_msg(msg);

	tstmsg_case_msg("getdns_dict_get_dict(dict, key, &ansdict)");
	retval = getdns_dict_get_dict(dict, key, &ansdict);
	snprintf(msg, sizeof(msg), "test 10: getdns_list_get_dict,retval = %d", retval);
	tstmsg_case_msg(msg);

	getdns_dict_destroy(dict);

	/* TODO: test getdns_dict_set functions with bogus params */

	/* test set and get legitimate use case */

	dict = getdns_dict_create();

	strcpy(key, "foo");
	newdict = getdns_dict_create();
	getdns_dict_set_int(newdict, "foo", 42);
	getdns_dict_set_int(newdict, "bar", 52);

	tstmsg_case_msg("getdns_dict_set_dict(dict, key, newdict)");
	retval = getdns_dict_set_dict(dict, key, newdict);
	snprintf(msg, sizeof(msg), "test 11: getdns_dict_set_dict,retval=%d,key=%s", retval,
		key);
	tstmsg_case_msg(msg);
	getdns_dict_destroy(newdict);

	tstmsg_case_msg("getdns_dict_get_dict(dict, key, &ansdict)");
	retval = getdns_dict_get_dict(dict, key, &ansdict);
	getdns_dict_get_int(ansdict, "foo", &int1);
	getdns_dict_get_int(ansdict, "bar", &int2);
	snprintf(msg, sizeof(msg),
	    "test 12: getdns_dict_get_dict,retval=%d,key=%s,int1=%d,int2=%d",
	    retval, key, int1, int2);
	tstmsg_case_msg(msg);

	getdns_dict_destroy(dict);

	tstmsg_case_end();

	return;
}				/* tst_dictsetget */

/*---------------------------------------- tst_getnames */
/**
 * exercise the getdns_dict_get_names function
 */
void
tst_getnames(void)
{
	size_t index;
	size_t llen;
	uint32_t ansint;
	size_t i;
	getdns_return_t result;
	getdns_data_type dtype;
	struct getdns_dict *dict = NULL;
	struct getdns_list *list = NULL;

	tstmsg_case_begin("tst_getnames");

	dict = getdns_dict_create();

	/* degenerative use cases */

	tstmsg_case_msg("getdns_dict_get_names(NULL, &list)");
	getdns_dict_get_names(NULL, &list);
	getdns_list_destroy(list);

	tstmsg_case_msg("getdns_dict_get_names(dict, NULL)");
	getdns_dict_get_names(dict, NULL);

	tstmsg_case_msg
	    ("getdns_dict_get_names(dict, &list), empty dictionary");
	getdns_dict_get_names(dict, &list);
	getdns_list_destroy(list);

	/* legit use case, add items out of order to exercise tree */
	/* TODO: add elements of type dict, bindata, list to the dict */

	i = 0;
	getdns_dict_set_int(dict, "foo", i++);
	getdns_dict_set_int(dict, "bar", i++);
	getdns_dict_set_int(dict, "quz", i++);
	getdns_dict_set_int(dict, "alpha", i++);

	getdns_dict_get_names(dict, &list);

	result = getdns_list_get_length(list, &llen);
	if (result != GETDNS_RETURN_GOOD) {
		tstmsg_case_msg
		    ("getdns_list_get_length failed, exiting");
		return;
	}
	if (llen != i) {
		tstmsg_case_msg
		    ("getdns_list_get_length returned unreasonable length, exiting");
		return;
	}

	for (index = 0; index < llen; index++) {
		getdns_list_get_data_type(list, index, &dtype);
		printf("    list item %d: ", (int) index);
		switch (dtype) {
		case t_bindata:
			printf("NOTIMPLEMENTED");
			break;

		case t_dict:
			printf("NOTIMPLEMENTED");
			break;

		case t_int:
			getdns_list_get_int(list, index, &ansint);
			printf("t_int, value=%d\n", ansint);
			break;

		case t_list:
			printf("NOTIMPLEMENTED");
			break;

		default:
			printf("data type invalid");
			break;
		}
	}

	getdns_dict_destroy(dict);
	getdns_list_destroy(list);

	tstmsg_case_end();
}				/* tst_getnames */

/*---------------------------------------- tst_listsetget */
/**
 * test the list get and set routines 
 */
void
tst_listsetget(void)
{
	char msg[TSTMSGBUF];
	char key[20];
	uint32_t int1;
	uint32_t int2;
	getdns_return_t retval;
	struct getdns_list *newlist;
	struct getdns_list *anslist;
	struct getdns_dict *dict = NULL;

	tstmsg_case_begin("tst_listsetget");

	dict = getdns_dict_create();

	/* test get function against empty list and with bogus params */

	strcpy(key, "foo");

	tstmsg_case_msg("getdns_dict_get_list() empty dict");
	retval = getdns_dict_get_list(NULL, key, &anslist);
	snprintf(msg, sizeof(msg),
	    "test 13: getdns_dict_get_list(NULL, key, &anslist),retval = %d",
	    retval);
	tstmsg_case_msg(msg);

	retval = getdns_dict_get_list(dict, key, NULL);
	snprintf(msg, sizeof(msg),
	    "test 14: getdns_dict_get_list(dict, key, NULL),retval = %d",
	    retval);
	tstmsg_case_msg(msg);

	tstmsg_case_msg("getdns_dict_get_list(dict, NULL, &anslist)");
	retval = getdns_dict_get_list(dict, NULL, &anslist);
	snprintf(msg, sizeof(msg), "test 15: getdns_dict_get_list,retval = %d", retval);
	tstmsg_case_msg(msg);

	tstmsg_case_msg("getdns_dict_get_list(dict, key, &anslist)");
	retval = getdns_dict_get_list(dict, key, &anslist);
	snprintf(msg, sizeof(msg), "test 16: getdns_list_get_list,retval = %d", retval);
	tstmsg_case_msg(msg);

	getdns_dict_destroy(dict);

	/* TODO: test getdns_dict_set functions with bogus params */

	/* test set and get legitimate use case */

	dict = getdns_dict_create();

	strcpy(key, "foo");
	newlist = getdns_list_create();
	getdns_list_set_int(newlist, 0, 42);
	getdns_list_set_int(newlist, 1, 52);

	tstmsg_case_msg("getdns_dict_set_list(dict, key, newlist)");
	retval = getdns_dict_set_list(dict, key, newlist);
	snprintf(msg, sizeof(msg), "test 17: getdns_dict_set_list,retval=%d,key=%s",
	    retval, key);
	tstmsg_case_msg(msg);
	getdns_list_destroy(newlist);

	tstmsg_case_msg("getdns_dict_get_list(dict, key, &anslist)");
	retval = getdns_dict_get_list(dict, key, &anslist);
	getdns_list_get_int(anslist, 0, &int1);
	getdns_list_get_int(anslist, 1, &int2);
	snprintf(msg, sizeof(msg),
	    "test 18: getdns_dict_get_list,retval=%d,key=%s,int1=%d,int2=%d",
	    retval, key, int1, int2);
	tstmsg_case_msg(msg);

	getdns_dict_destroy(dict);

	tstmsg_case_end();

	return;
}				/* tst_listsetget */

/*---------------------------------------- tst_intsetget */
/**
 * test the int get and set routines 
 */
void
tst_intsetget(void)
{
	char msg[TSTMSGBUF];
	char key[20];
	uint32_t ans_int;
	uint32_t newint;
	getdns_return_t retval;
	struct getdns_dict *dict = NULL;
	getdns_data_type dtype;

	tstmsg_case_begin("tst_intsetget");

	dict = getdns_dict_create();

	/* test int get function against empty list and with bogus params */

	strcpy(key, "foo");

	tstmsg_case_msg("getdns_dict_get_int() empty dict");
	retval = getdns_dict_get_int(NULL, key, &ans_int);
	snprintf(msg, sizeof(msg),
	    "test 19: getdns_dict_get_int(NULL, key, &ans_int),retval = %d",
	    retval);
	tstmsg_case_msg(msg);

	retval = getdns_dict_get_int(dict, key, NULL);
	snprintf(msg, sizeof(msg),
	    "test 20: getdns_dict_get_int(dict, key, NULL),retval = %d",
	    retval);
	tstmsg_case_msg(msg);

	tstmsg_case_msg("getdns_dict_get_int(dict, NULL, &ans_int)");
	retval = getdns_dict_get_int(dict, NULL, &ans_int);
	snprintf(msg, sizeof(msg), "test 21: getdns_dict_get_int,retval = %d", retval);
	tstmsg_case_msg(msg);

	tstmsg_case_msg("getdns_dict_get_int(dict, key, &ans_int)");
	retval = getdns_dict_get_int(dict, key, &ans_int);
	snprintf(msg, sizeof(msg), "test 22: getdns_list_get_int,retval = %d", retval);
	tstmsg_case_msg(msg);

	getdns_dict_destroy(dict);

	/* TODO: test getdns_dict_set functions with bogus params */

	/* test set and get legitimate use case */

	dict = getdns_dict_create();

	strcpy(key, "foo");
	newint = 42;

	tstmsg_case_msg("getdns_dict_set_int(dict, key, newint)");
	retval = getdns_dict_set_int(dict, key, newint);
	snprintf(msg, sizeof(msg), "test 23: getdns_dict_set_int,retval=%d,key=%s,int=%d",
	    retval, key, newint);
	tstmsg_case_msg(msg);

	tstmsg_case_msg("getdns_dict_get_int(dict, key, &ans_int)");
	retval = getdns_dict_get_int(dict, key, &ans_int);
	snprintf(msg, sizeof(msg), "test 24: getdns_dict_get_int,retval=%d,key=%s,int=%d",
	    retval, key, ans_int);
	tstmsg_case_msg(msg);

	strcpy(key, "bar");
	newint = 52;
	tstmsg_case_msg("getdns_dict_set_int(dict, key, newint)");
	retval = getdns_dict_set_int(dict, key, newint);
	snprintf(msg, sizeof(msg), "test 25: getdns_dict_set_int,retval=%d,key=%s,int=%d",
	    retval, key, newint);
	tstmsg_case_msg(msg);

	tstmsg_case_msg("getdns_dict_get_int(dict, key, &ans_int)");
	retval = getdns_dict_get_int(dict, key, &ans_int);
	snprintf(msg, sizeof(msg), "test 26: getdns_dict_get_int,retval=%d,key=%s,int=%d",
	    retval, key, ans_int);
	tstmsg_case_msg(msg);

	tstmsg_case_msg("getdns_dict_get_data_type(dict, key, &dtype)");
	retval = getdns_dict_get_data_type(dict, key, &dtype);
	snprintf(msg, sizeof(msg),
	    "test 27: getdns_dict_get_data_type,retval=%d,key=%s,dtype=%d",
	    retval, key, dtype);
	tstmsg_case_msg(msg);

	getdns_dict_destroy(dict);

	tstmsg_case_end();

	return;
}				/* tst_intsetget */

/*---------------------------------------- tst_create */
/**
 * test the create, destroy and allocation functions
 */
void
tst_create(void)
{
	struct getdns_dict *dict = NULL;

	/* make sure we can do a simple create/destroy first */

	tstmsg_case_begin("tst_create");

	tstmsg_case_msg("getdns_dict_create");
	dict = getdns_dict_create();

	if (dict != NULL) {
		tstmsg_case_msg("getdns_dict_destroy(dict)");
		getdns_dict_destroy(dict);
	}

	tstmsg_case_msg("getdns_dict_destroy(NULL)");
	getdns_dict_destroy(NULL);

	tstmsg_case_end();

	return;
}				/* tst_create */

/*---------------------------------------- main */
/**
 *  runs unit tests against list management routines
 */
int
main()
{
	tstmsg_prog_begin("tests_dict");

	tst_create();

	tst_bindatasetget();

	tst_dictsetget();

	tst_intsetget();

	tst_listsetget();

	tst_getnames();

	tstmsg_prog_end();

	return 0;
}				/* main */

/* tests_dict.c */
