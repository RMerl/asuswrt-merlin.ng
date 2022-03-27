/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013  Intel Corporation. All rights reserved.
 *  Copyright (C) 2013  Instituto Nokia de Tecnologia - INdT
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <stdlib.h>
#include <errno.h>

#include "src/shared/util.h"
#include "src/shared/tester.h"

#include "lib/sdp.h"
#include "lib/sdp_lib.h"

static void test_ntoh64(const void *data)
{
	uint64_t test = 0x123456789abcdef;

	g_assert(ntoh64(test) == be64toh(test));
	g_assert(ntoh64(test) == be64_to_cpu(test));
	tester_test_passed();
}

static void test_hton64(const void *data)
{
	uint64_t test = 0x123456789abcdef;

	g_assert(hton64(test) == htobe64(test));
	g_assert(hton64(test) == cpu_to_be64(test));
	tester_test_passed();
}

static void test_sdp_get_access_protos_valid(const void *data)
{
	sdp_record_t *rec;
	sdp_list_t *aproto, *apseq, *proto[2];
	const uint8_t u8 = 1;
	uuid_t l2cap, rfcomm;
	sdp_data_t *channel;
	int err;

	rec = sdp_record_alloc();
	sdp_uuid16_create(&l2cap, L2CAP_UUID);
	proto[0] = sdp_list_append(NULL, &l2cap);
	apseq = sdp_list_append(NULL, proto[0]);

	sdp_uuid16_create(&rfcomm, RFCOMM_UUID);
	proto[1] = sdp_list_append(NULL, &rfcomm);
	channel = sdp_data_alloc(SDP_UINT8, &u8);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(NULL, apseq);
	sdp_set_access_protos(rec, aproto);
	sdp_set_add_access_protos(rec, aproto);
	sdp_data_free(channel);
	sdp_list_free(proto[0], NULL);
	sdp_list_free(proto[1], NULL);
	sdp_list_free(apseq, NULL);
	sdp_list_free(aproto, NULL);

	err = sdp_get_access_protos(rec, &aproto);
	g_assert(err == 0);
	sdp_list_foreach(aproto, (sdp_list_func_t) sdp_list_free, NULL);
	sdp_list_free(aproto, NULL);

	err = sdp_get_add_access_protos(rec, &aproto);
	g_assert(err == 0);
	sdp_list_foreach(aproto, (sdp_list_func_t) sdp_list_free, NULL);
	sdp_list_free(aproto, NULL);

	sdp_record_free(rec);
	tester_test_passed();
}

static void test_sdp_get_access_protos_nodata(const void *data)
{
	sdp_record_t *rec;
	sdp_list_t *aproto;
	int err;

	rec = sdp_record_alloc();

	err = sdp_get_access_protos(rec, &aproto);
	g_assert(err == -1 && errno == ENODATA);

	err = sdp_get_add_access_protos(rec, &aproto);
	g_assert(err == -1 && errno == ENODATA);

	sdp_record_free(rec);
	tester_test_passed();
}

static void test_sdp_get_access_protos_invalid_dtd1(const void *tdata)
{
	const uint32_t u32 = 0xdeadbeeb;
	sdp_record_t *rec;
	sdp_list_t *aproto;
	sdp_data_t *data;
	int err;

	rec = sdp_record_alloc();

	data = sdp_data_alloc(SDP_UINT32, &u32);
	g_assert(data != NULL);
	sdp_attr_replace(rec, SDP_ATTR_PROTO_DESC_LIST, data);

	err = sdp_get_access_protos(rec, &aproto);
	g_assert(err == -1 && errno == EINVAL);

	data = sdp_data_alloc(SDP_UINT32, &u32);
	g_assert(data != NULL);
	sdp_attr_replace(rec, SDP_ATTR_ADD_PROTO_DESC_LIST, data);

	err = sdp_get_add_access_protos(rec, &aproto);
	g_assert(err == -1 && errno == EINVAL);

	sdp_record_free(rec);
	tester_test_passed();
}

static void test_sdp_get_access_protos_invalid_dtd2(const void *tdata)
{
	uint8_t dtd = SDP_UINT8, u8 = 0xff;
	void *dtds = &dtd, *values = &u8;
	sdp_record_t *rec;
	sdp_list_t *aproto;
	sdp_data_t *data;
	int err;

	rec = sdp_record_alloc();

	data = sdp_seq_alloc(&dtds, &values, 1);
	g_assert(data != NULL);
	sdp_attr_replace(rec, SDP_ATTR_PROTO_DESC_LIST, data);

	err = sdp_get_access_protos(rec, &aproto);
	g_assert(err == -1 && errno == EINVAL);

	data = sdp_seq_alloc(&dtds, &values, 1);
	g_assert(data != NULL);
	sdp_attr_replace(rec, SDP_ATTR_ADD_PROTO_DESC_LIST, data);

	err = sdp_get_add_access_protos(rec, &aproto);
	g_assert(err == -1 && errno == EINVAL);

	sdp_record_free(rec);
	tester_test_passed();
}

static void test_sdp_get_lang_attr_valid(const void *data)
{
	sdp_record_t *rec;
	sdp_list_t *list;
	int err;

	rec = sdp_record_alloc();
	sdp_add_lang_attr(rec);

	err = sdp_get_lang_attr(rec, &list);
	g_assert(err == 0);

	sdp_list_free(list, free);
	sdp_record_free(rec);
	tester_test_passed();
}

static void test_sdp_get_lang_attr_nodata(const void *data)
{
	sdp_record_t *rec;
	sdp_list_t *list;
	int err;

	rec = sdp_record_alloc();

	err = sdp_get_lang_attr(rec, &list);
	g_assert(err == -1 && errno == ENODATA);

	sdp_record_free(rec);
	tester_test_passed();
}

static void test_sdp_get_lang_attr_invalid_dtd(const void *tdata)
{
	uint8_t dtd1 = SDP_UINT16, dtd2 = SDP_UINT32;
	uint32_t u32 = 0xdeadbeeb;
	uint16_t u16 = 0x1234;
	void *dtds1[] = { &dtd1, &dtd2, &dtd2 };
	void *values1[] = { &u16, &u32, &u32 };
	void *dtds2[] = { &dtd1, &dtd1, &dtd2 };
	void *values2[] = { &u16, &u16, &u32 };
	sdp_record_t *rec;
	sdp_data_t *data;
	sdp_list_t *list;
	int err;

	rec = sdp_record_alloc();

	/* UINT32 */
	data = sdp_data_alloc(SDP_UINT32, &u32);
	g_assert(data != NULL);
	sdp_attr_add(rec, SDP_ATTR_LANG_BASE_ATTR_ID_LIST, data);
	err = sdp_get_lang_attr(rec, &list);
	g_assert(err == -1 && errno == EINVAL);

	/* SEQ8(UINT32) */
	data = sdp_seq_alloc(&dtds1[1], &values1[1], 1);
	sdp_attr_replace(rec, SDP_ATTR_LANG_BASE_ATTR_ID_LIST, data);
	err = sdp_get_lang_attr(rec, &list);
	g_assert(err == -1 && errno == EINVAL);

	/* SEQ8(UINT16, UINT16) */
	data = sdp_seq_alloc(dtds2, values2, 2);
	sdp_attr_replace(rec, SDP_ATTR_LANG_BASE_ATTR_ID_LIST, data);
	err = sdp_get_lang_attr(rec, &list);
	g_assert(err == -1 && errno == EINVAL);

	/* SEQ8(UINT16, UINT32, UINT32) */
	data = sdp_seq_alloc(dtds1, values1, 3);
	sdp_attr_replace(rec, SDP_ATTR_LANG_BASE_ATTR_ID_LIST, data);
	err = sdp_get_lang_attr(rec, &list);
	g_assert(err == -1 && errno == EINVAL);

	/* SEQ8(UINT16, UINT16, UINT32) */
	data = sdp_seq_alloc(dtds2, values2, 3);
	sdp_attr_replace(rec, SDP_ATTR_LANG_BASE_ATTR_ID_LIST, data);
	err = sdp_get_lang_attr(rec, &list);
	g_assert(err == -1 && errno == EINVAL);

	sdp_record_free(rec);
	tester_test_passed();
}

static void test_sdp_get_profile_descs_valid(const void *data)
{
	sdp_profile_desc_t profile;
	sdp_record_t *rec;
	sdp_list_t *list;
	int err;

	rec = sdp_record_alloc();

	sdp_uuid16_create(&profile.uuid, NAP_PROFILE_ID);
	profile.version = 0x0100;
	list = sdp_list_append(NULL, &profile);
	err = sdp_set_profile_descs(rec, list);
	sdp_list_free(list, NULL);
	g_assert(err == 0);

	list = NULL;
	err = sdp_get_profile_descs(rec, &list);
	sdp_list_free(list, free);
	g_assert(err == 0 && list != NULL);

	sdp_record_free(rec);
	tester_test_passed();
}

static void test_sdp_get_profile_descs_nodata(const void *data)
{
	sdp_record_t *rec;
	sdp_list_t *list;
	int err;

	rec = sdp_record_alloc();

	err = sdp_get_profile_descs(rec, &list);
	g_assert(err == -1 && errno == ENODATA);

	sdp_record_free(rec);
	tester_test_passed();
}

static void test_sdp_get_profile_descs_invalid_dtd(const void *tdata)
{
	uint8_t dtd1 = SDP_UUID16, dtd2 = SDP_UINT32;
	uint32_t u32 = 0xdeadbeeb;
	uint16_t u16 = 0x1234;
	void *dtds[1], *values[1];
	void *dtds2[] = { &dtd1, &dtd2 };
	void *values2[] = { &u16, &u32 };
	sdp_record_t *rec;
	sdp_data_t *data;
	sdp_list_t *list;
	int err;

	rec = sdp_record_alloc();

	/* UINT32 */
	data = sdp_data_alloc(SDP_UINT32, &u32);
	g_assert(data != NULL);
	sdp_attr_add(rec, SDP_ATTR_PFILE_DESC_LIST, data);
	err = sdp_get_profile_descs(rec, &list);
	g_assert(err == -1 && errno == EINVAL);

	/* SEQ8() */
	data = sdp_seq_alloc(NULL, NULL, 0);
	sdp_attr_replace(rec, SDP_ATTR_PFILE_DESC_LIST, data);
	err = sdp_get_profile_descs(rec, &list);
	g_assert(err == -1 && errno == EINVAL);

	/* SEQ8(UINT32) */
	data = sdp_seq_alloc(&dtds2[1], &values2[1], 1);
	sdp_attr_replace(rec, SDP_ATTR_PFILE_DESC_LIST, data);
	err = sdp_get_profile_descs(rec, &list);
	g_assert(err == -1 && errno == EINVAL);

	/* SEQ8(SEQ8()) */
	data = sdp_seq_alloc(NULL, NULL, 0);
	dtds[0] = &data->dtd;
	values[0] = data;
	data = sdp_seq_alloc(dtds, values, 1);
	sdp_attr_replace(rec, SDP_ATTR_PFILE_DESC_LIST, data);
	err = sdp_get_profile_descs(rec, &list);
	g_assert(err == -1 && errno == EINVAL);

	/* SEQ8(SEQ8(UINT32)) */
	data = sdp_seq_alloc(&dtds2[1], &values2[1], 1);
	dtds[0] = &data->dtd;
	values[0] = data;
	data = sdp_seq_alloc(dtds, values, 1);
	sdp_attr_replace(rec, SDP_ATTR_PFILE_DESC_LIST, data);
	err = sdp_get_profile_descs(rec, &list);
	g_assert(err == -1 && errno == EINVAL);

	/* SEQ8(SEQ8(UUID16)) */
	data = sdp_seq_alloc(dtds2, values2, 1);
	dtds[0] = &data->dtd;
	values[0] = data;
	data = sdp_seq_alloc(dtds, values, 1);
	sdp_attr_replace(rec, SDP_ATTR_PFILE_DESC_LIST, data);
	err = sdp_get_profile_descs(rec, &list);
	g_assert(err == -1 && errno == EINVAL);

	/* SEQ8(SEQ8(UUID16, UINT32)) */
	data = sdp_seq_alloc(dtds2, values2, 2);
	dtds[0] = &data->dtd;
	values[0] = data;
	data = sdp_seq_alloc(dtds, values, 1);
	sdp_attr_replace(rec, SDP_ATTR_PFILE_DESC_LIST, data);
	err = sdp_get_profile_descs(rec, &list);
	g_assert(err == -1 && errno == EINVAL);

	sdp_record_free(rec);
	tester_test_passed();
}

static void test_sdp_get_profile_descs_workaround(const void *tdata)
{
	uint8_t dtd1 = SDP_UUID16, dtd2 = SDP_UINT16, dtd3 = SDP_UINT32;
	uint16_t u16 = 0x1234;
	uint32_t u32 = 0xdeadbeeb;
	void *dtds[] = { &dtd1, &dtd2 };
	void *values[] = { &u16, &u16 };
	void *dtds2[] = { &dtd1, &dtd3 };
	void *values2[] = { &u16, &u32 };
	sdp_record_t *rec;
	sdp_data_t *data;
	sdp_list_t *list;
	int err;

	rec = sdp_record_alloc();

	/* SEQ8(UUID16) */
	data = sdp_seq_alloc(dtds, values, 1);
	sdp_attr_add(rec, SDP_ATTR_PFILE_DESC_LIST, data);
	list = NULL;
	err = sdp_get_profile_descs(rec, &list);
	sdp_list_free(list, free);
	g_assert(err == 0 && list != NULL);

	/* SEQ8(UUID16, UINT16) */
	data = sdp_seq_alloc(dtds, values, 2);
	sdp_attr_replace(rec, SDP_ATTR_PFILE_DESC_LIST, data);
	list = NULL;
	err = sdp_get_profile_descs(rec, &list);
	sdp_list_free(list, free);
	g_assert(err == 0 && list != NULL);

	/* SEQ8(UUID16) */
	data = sdp_seq_alloc(dtds, values, 1);
	sdp_attr_replace(rec, SDP_ATTR_PFILE_DESC_LIST, data);
	list = NULL;
	err = sdp_get_profile_descs(rec, &list);
	sdp_list_free(list, free);
	g_assert(err == 0 && list != NULL);

	/* SEQ8(UUID16, UINT32) */
	data = sdp_seq_alloc(dtds2, values2, 2);
	sdp_attr_replace(rec, SDP_ATTR_PFILE_DESC_LIST, data);
	list = NULL;
	err = sdp_get_profile_descs(rec, &list);
	g_assert(err == -1 && errno == EINVAL);

	sdp_record_free(rec);
	tester_test_passed();
}

static void test_sdp_get_server_ver(const void *tdata)
{
	uint16_t u16 = 0x1234;
	uint32_t u32 = 0xdeadbeeb;
	uint8_t dtd1 = SDP_UINT16, dtd2 = SDP_UINT32;
	void *dtds1[] = { &dtd1 };
	void *values1[] = { &u16 };
	void *dtds2[] = { &dtd2 };
	void *values2[] = { &u32 };
	sdp_record_t *rec;
	sdp_data_t *data;
	sdp_list_t *list;
	int err;

	rec = sdp_record_alloc();

	err = sdp_get_server_ver(rec, &list);
	g_assert(err == -1 && errno == ENODATA);

	/* Valid DTD */
	data = sdp_seq_alloc(dtds1, values1, 1);
	sdp_attr_add(rec, SDP_ATTR_VERSION_NUM_LIST, data);
	err = sdp_get_server_ver(rec, &list);
	g_assert(err == 0 && list != NULL);
	sdp_list_free(list, NULL);

	/* Invalid: UINT32 */
	data = sdp_data_alloc(SDP_UINT32, &u32);
	sdp_attr_replace(rec, SDP_ATTR_VERSION_NUM_LIST, data);
	err = sdp_get_server_ver(rec, &list);
	g_assert(err == -1 && errno == EINVAL);

	/* Invalid: SEQ8() */
	data = sdp_seq_alloc(NULL, NULL, 0);
	sdp_attr_replace(rec, SDP_ATTR_VERSION_NUM_LIST, data);
	err = sdp_get_server_ver(rec, &list);
	g_assert(err == -1 && errno == EINVAL);

	/* Invalid: SEQ8(UINT32) */
	data = sdp_seq_alloc(dtds2, values2, 1);
	sdp_attr_replace(rec, SDP_ATTR_VERSION_NUM_LIST, data);
	err = sdp_get_server_ver(rec, &list);
	g_assert(err == -1 && errno == EINVAL);

	sdp_record_free(rec);
	tester_test_passed();
}

int main(int argc, char *argv[])
{
	tester_init(&argc, &argv);

	tester_add("/lib/ntoh64", NULL, NULL, test_ntoh64, NULL);
	tester_add("/lib/hton64", NULL, NULL, test_hton64, NULL);

	tester_add("/lib/sdp_get_access_protos/valid", NULL, NULL,
				test_sdp_get_access_protos_valid, NULL);
	tester_add("/lib/sdp_get_access_protos/nodata", NULL, NULL,
				test_sdp_get_access_protos_nodata, NULL);
	tester_add("/lib/sdp_get_access_protos/invalid_dtd1", NULL, NULL,
				test_sdp_get_access_protos_invalid_dtd1, NULL);
	tester_add("/lib/sdp_get_access_protos/invalid_dtd2", NULL, NULL,
				test_sdp_get_access_protos_invalid_dtd2, NULL);

	tester_add("/lib/sdp_get_lang_attr/valid", NULL, NULL,
					test_sdp_get_lang_attr_valid, NULL);
	tester_add("/lib/sdp_get_lang_attr/nodata", NULL, NULL,
					test_sdp_get_lang_attr_nodata, NULL);
	tester_add("/lib/sdp_get_lang_attr/invalid_dtd", NULL, NULL,
				test_sdp_get_lang_attr_invalid_dtd, NULL);

	tester_add("/lib/sdp_get_profile_descs/valid", NULL, NULL,
					test_sdp_get_profile_descs_valid, NULL);
	tester_add("/lib/sdp_get_profile_descs/nodata", NULL, NULL,
				test_sdp_get_profile_descs_nodata, NULL);
	tester_add("/lib/sdp_get_profile_descs/invalid_dtd", NULL, NULL,
				test_sdp_get_profile_descs_invalid_dtd, NULL);
	/* Test for workaround commented on sdp_get_profile_descs() */
	tester_add("/lib/sdp_get_profile_descs/workaround", NULL, NULL,
				test_sdp_get_profile_descs_workaround, NULL);

	tester_add("/lib/sdp_get_server_ver", NULL, NULL,
					test_sdp_get_server_ver, NULL);

	return tester_run();
}
