/*
 * Copyright (C) 2017-2018 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "test_suite.h"

#include "swima/swima_record.h"
#include "swima/swima_data_model.h"
#include "swima/swima_inventory.h"
#include "swima/swima_event.h"
#include "swima/swima_events.h"
#include "swima/swima_collector.h"
#include "ietf/swima/ietf_swima_attr_req.h"
#include "ietf/swima/ietf_swima_attr_sw_inv.h"
#include "ietf/swima/ietf_swima_attr_sw_ev.h"

static pen_type_t ita_data_model = { PEN_ITA, 0x19 };

static char* sw_id_str[] = {
	"strongswan.org_strongSwan_5.3.3",
	"strongswan.org_62251aa6-1a01-479b-aea6-f3dcf0ab1f1a"
};
static char sw_locator_str[] = "/usr/share/strongswan";

static char* sw_record_str[] = {
	"<SoftwareIdentity tagId=\"abc\"></SoftwareIdentity>",
	"<SoftwareIdentity tagId=\"def\"></SoftwareIdentity>"
};

START_TEST(test_imcv_swima_record)
{
	chunk_t sw_id, sw_locator, locator;
	swima_record_t *sw_record, *sw_record_cp;
	uint32_t record_id = 1;
	uint8_t source_id = 2;
	chunk_t record = chunk_from_str(sw_record_str[0]);

	sw_id = chunk_from_str(sw_id_str[0]);
	sw_locator = chunk_from_str(sw_locator_str);

	/* Software Identity with Software Locator */
	sw_record = swima_record_create(record_id, sw_id, sw_locator),
	ck_assert(sw_record);
	sw_record_cp = sw_record->get_ref(sw_record);

	ck_assert(record_id == sw_record->get_record_id(sw_record));
	ck_assert_chunk_eq(sw_id, sw_record->get_sw_id(sw_record, NULL));
	ck_assert_chunk_eq(sw_id, sw_record->get_sw_id(sw_record, &locator));
	ck_assert_chunk_eq(locator, sw_locator);

	sw_record->set_data_model(sw_record, ita_data_model);
	ck_assert(pen_type_equals(sw_record->get_data_model(sw_record),
							  ita_data_model));

	sw_record->set_source_id(sw_record, source_id);
	ck_assert(source_id == sw_record->get_source_id(sw_record));

	sw_record->set_record(sw_record, record);
	ck_assert_chunk_eq(record, sw_record->get_record(sw_record));

	sw_record->destroy(sw_record);
	sw_record_cp->destroy(sw_record);

	/* Software Identity without Software Locator */
	sw_record = swima_record_create(record_id, sw_id, chunk_empty),
	ck_assert(sw_record);
	ck_assert_chunk_eq(sw_id, sw_record->get_sw_id(sw_record, &locator));
	ck_assert(locator.ptr == NULL && locator.len == 0);

	ck_assert(pen_type_equals(swima_data_model_iso_2015_swid_xml,
							  sw_record->get_data_model(sw_record)));

	sw_record->destroy(sw_record);
}
END_TEST

typedef struct req_data_t req_data_t;

struct req_data_t {
	uint8_t flags;
	uint32_t request_id;
	uint32_t earliest_eid;
	uint32_t sw_id_count;
	chunk_t  value;
};

static req_data_t req_data[] = {
	{ IETF_SWIMA_ATTR_REQ_FLAG_NONE, 1,   0, 0, chunk_from_chars(
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
		0x00, 0x00)
	},
	{ IETF_SWIMA_ATTR_REQ_FLAG_R,    2,  15, 1, chunk_from_chars(
		0x20, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
		0x00, 0x0F, 0x00, 0x1f, 0x73, 0x74, 0x72, 0x6f, 0x6e, 0x67,
		0x73, 0x77, 0x61, 0x6e, 0x2e, 0x6f, 0x72, 0x67, 0x5f, 0x73,
		0x74, 0x72, 0x6f, 0x6e, 0x67, 0x53, 0x77, 0x61, 0x6e, 0x5f,
		0x35, 0x2e, 0x33, 0x2e, 0x33)
	},
	{ IETF_SWIMA_ATTR_REQ_FLAG_S,    3, 256, 2, chunk_from_chars(
		0x40, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x1f, 0x73, 0x74, 0x72, 0x6f, 0x6e, 0x67,
		0x73, 0x77, 0x61, 0x6e, 0x2e, 0x6f, 0x72, 0x67, 0x5f, 0x73,
		0x74, 0x72, 0x6f, 0x6e, 0x67, 0x53, 0x77, 0x61, 0x6e, 0x5f,
		0x35, 0x2e, 0x33, 0x2e, 0x33, 0x00, 0x33, 0x73, 0x74, 0x72,
		0x6f, 0x6e, 0x67, 0x73, 0x77, 0x61, 0x6e, 0x2e, 0x6f, 0x72,
		0x67, 0x5f, 0x36, 0x32, 0x32, 0x35, 0x31, 0x61, 0x61, 0x36,
		0x2d, 0x31, 0x61, 0x30, 0x31, 0x2d, 0x34, 0x37, 0x39, 0x62,
		0x2d, 0x61, 0x65, 0x61, 0x36, 0x2d, 0x66, 0x33, 0x64, 0x63,
		0x66, 0x30, 0x61, 0x62, 0x31, 0x66, 0x31, 0x61)
	},
};

START_TEST(test_imcv_swima_sw_req)
{
	pen_type_t type;
	pen_type_t pen_type = { PEN_IETF, IETF_ATTR_SWIMA_REQUEST };
	pa_tnc_attr_t *attr, *attr1, *attr2;
	ietf_swima_attr_req_t *c_attr;
	swima_record_t *target;
	swima_inventory_t *targets;
	chunk_t sw_id, value;
	enumerator_t *enumerator;
	uint32_t offset;
	int n;

	attr = ietf_swima_attr_req_create(req_data[_i].flags,
									  req_data[_i].request_id);
	ck_assert(attr);

	type = attr->get_type(attr);
	ck_assert(pen_type_equals(type, pen_type));

	ck_assert(attr->get_noskip_flag(attr) == FALSE);
	attr->set_noskip_flag(attr, TRUE);
	ck_assert(attr->get_noskip_flag(attr) == TRUE);

	targets = swima_inventory_create();
	targets->set_eid(targets, req_data[_i].earliest_eid, 0);

	for (n = 0; n < req_data[_i].sw_id_count; n++)
	{
		sw_id = chunk_from_str(sw_id_str[n]);
		target = swima_record_create(0, sw_id, chunk_empty);
		targets->add(targets, target);
	}
	c_attr = (ietf_swima_attr_req_t*)attr;
	c_attr->set_targets(c_attr, targets);
	c_attr->set_targets(c_attr, targets);
	targets->destroy(targets);

	attr->build(attr);
	attr->build(attr);
	value = attr->get_value(attr);
	ck_assert_chunk_eq(value, req_data[_i].value);

	attr1 = attr->get_ref(attr);
	attr->destroy(attr);

	attr2 = ietf_swima_attr_req_create_from_data(value.len, value);
	ck_assert(attr2);

	attr1->destroy(attr1);
	ck_assert(attr2->process(attr2, &offset) == SUCCESS);

	type = attr2->get_type(attr2);
	ck_assert(pen_type_equals(type, pen_type));

	c_attr = (ietf_swima_attr_req_t*)attr2;
	ck_assert(c_attr->get_flags(c_attr) == req_data[_i].flags);
	ck_assert(c_attr->get_request_id(c_attr) == req_data[_i].request_id);

	targets = c_attr->get_targets(c_attr);
	ck_assert(targets->get_eid(targets, NULL) == req_data[_i].earliest_eid);

	enumerator = targets->create_enumerator(targets);
	ck_assert(enumerator);
	n = 0;
	while (enumerator->enumerate(enumerator, &target))
	{
		sw_id = target->get_sw_id(target, NULL);
		ck_assert(chunk_equals(sw_id, chunk_from_str(sw_id_str[n++])));
	}
	enumerator->destroy(enumerator);

	attr2->destroy(attr2);
}
END_TEST

START_TEST(test_imcv_swima_sw_req_trunc)
{
	pa_tnc_attr_t *attr;
	chunk_t data;
	uint32_t offset = 100;

	/* Data smaller than minimum size */
	attr = ietf_swima_attr_req_create_from_data(0, chunk_empty);
	ck_assert(attr);
	ck_assert(attr->process(attr, &offset) == FAILED && offset == 0);
	attr->destroy(attr);

	/* Truncate first SW ID */
	data = req_data[2].value;
	data.len = 14;
	attr = ietf_swima_attr_req_create_from_data(data.len, data);
	ck_assert(attr);
	ck_assert(attr->process(attr, &offset) == FAILED && offset == 12);
	attr->destroy(attr);

	/* Truncate second SW ID */
	data = req_data[2].value;
	data.len = 47;
	attr = ietf_swima_attr_req_create_from_data(data.len, data);
	ck_assert(attr);
	ck_assert(attr->process(attr, &offset) == FAILED && offset == 45);
	attr->destroy(attr);

	/* Segmentation */
	data = req_data[2].value;
	data.len = 50;
	attr = ietf_swima_attr_req_create_from_data(req_data[2].value.len, data);
	ck_assert(attr);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);
	data = chunk_skip(req_data[2].value, 50);
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == SUCCESS);
	attr->destroy(attr);
}
END_TEST

static pen_type_t sw_inv_types[] = {
	{ PEN_IETF, IETF_ATTR_SW_INVENTORY },
	{ PEN_IETF, IETF_ATTR_SW_ID_INVENTORY }
};

typedef struct sw_inv_data_t sw_inv_data_t;

struct sw_inv_data_t {
	uint8_t flags;
	uint32_t request_id;
	uint32_t eid_epoch;
	uint32_t last_eid;
	chunk_t  value;
};

static sw_inv_data_t sw_inv_data[] = {
	{ IETF_SWIMA_ATTR_SW_INV_FLAG_NONE, 0xaabbccd0, 0x87654321, 0x00000007,
	  chunk_from_chars(
		0x00, 0x00, 0x00, 0x00, 0xAA, 0xBB, 0xCC, 0xD0, 0x87, 0x65,
		0x43, 0x21, 0x00, 0x00, 0x00, 0x07)
	},
	{ IETF_SWIMA_ATTR_SW_INV_FLAG_NONE, 0xaabbccd1, 0x87654321, 0x00000007,
	  chunk_from_chars(
		0x00, 0x00, 0x00, 0x00, 0xAA, 0xBB, 0xCC, 0xD1, 0x87, 0x65,
		0x43, 0x21, 0x00, 0x00, 0x00, 0x07)
	},
	{ IETF_SWIMA_ATTR_SW_INV_FLAG_NONE, 0xaabbccd2, 0x12345678, 0x00000030,
	  chunk_from_chars(
		0x00, 0x00, 0x00, 0x01, 0xAA, 0xBB, 0xCC, 0xD2, 0x12, 0x34,
		0x56, 0x78, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x73, 0x74,
		0x72, 0x6F, 0x6E, 0x67, 0x73, 0x77, 0x61, 0x6E, 0x2E, 0x6F,
		0x72, 0x67, 0x5F, 0x73, 0x74, 0x72, 0x6F, 0x6E, 0x67, 0x53,
		0x77, 0x61, 0x6E, 0x5F, 0x35, 0x2E, 0x33, 0x2E, 0x33, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x31, 0x3C, 0x53, 0x6F, 0x66, 0x74,
		0x77, 0x61, 0x72, 0x65, 0x49, 0x64, 0x65, 0x6E, 0x74, 0x69,
		0x74, 0x79, 0x20, 0x74, 0x61, 0x67, 0x49, 0x64, 0x3D, 0x22,
		0x61, 0x62, 0x63, 0x22, 0x3E, 0x3C, 0x2F, 0x53, 0x6F, 0x66,
		0x74, 0x77, 0x61, 0x72, 0x65, 0x49, 0x64, 0x65, 0x6E, 0x74,
		0x69, 0x74, 0x79, 0x3E)
	},
	{ IETF_SWIMA_ATTR_SW_INV_FLAG_NONE, 0xaabbccd3, 0x12345678, 0x00000030,
	  chunk_from_chars(
		0x00, 0x00, 0x00, 0x01, 0xAA, 0xBB, 0xCC, 0xD3, 0x12, 0x34,
		0x56, 0x78, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x73, 0x74,
		0x72, 0x6F, 0x6E, 0x67, 0x73, 0x77, 0x61, 0x6E, 0x2E, 0x6F,
		0x72, 0x67, 0x5F, 0x73, 0x74, 0x72, 0x6F, 0x6E, 0x67, 0x53,
		0x77, 0x61, 0x6E, 0x5F, 0x35, 0x2E, 0x33, 0x2E, 0x33, 0x00,
		0x00)
	},
	{ IETF_SWIMA_ATTR_SW_INV_FLAG_S_F, 0xaabbccd4, 0x12345678, 0x00000034,
	  chunk_from_chars(
		0x80, 0x00, 0x00, 0x02, 0xAA, 0xBB, 0xCC, 0xD4, 0x12, 0x34,
		0x56, 0x78, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x73, 0x74,
		0x72, 0x6F, 0x6E, 0x67, 0x73, 0x77, 0x61, 0x6E, 0x2E, 0x6F,
		0x72, 0x67, 0x5F, 0x73, 0x74, 0x72, 0x6F, 0x6E, 0x67, 0x53,
		0x77, 0x61, 0x6E, 0x5F, 0x35, 0x2E, 0x33, 0x2E, 0x33, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x31, 0x3C, 0x53, 0x6F, 0x66, 0x74,
		0x77, 0x61, 0x72, 0x65, 0x49, 0x64, 0x65, 0x6E, 0x74, 0x69,
		0x74, 0x79, 0x20, 0x74, 0x61, 0x67, 0x49, 0x64, 0x3D, 0x22,
		0x61, 0x62, 0x63, 0x22, 0x3E, 0x3C, 0x2F, 0x53, 0x6F, 0x66,
		0x74, 0x77, 0x61, 0x72, 0x65, 0x49, 0x64, 0x65, 0x6E, 0x74,
		0x69, 0x74, 0x79, 0x3E, 0x00, 0x00, 0x00, 0x01, 0x00, 0x90,
		0x2A, 0x19, 0x11, 0x00, 0x00, 0x33, 0x73, 0x74, 0x72, 0x6F,
		0x6E, 0x67, 0x73, 0x77, 0x61, 0x6E, 0x2E, 0x6F, 0x72, 0x67,
		0x5F, 0x36, 0x32, 0x32, 0x35, 0x31, 0x61, 0x61, 0x36, 0x2D,
		0x31, 0x61, 0x30, 0x31, 0x2D, 0x34, 0x37, 0x39, 0x62, 0x2D,
		0x61, 0x65, 0x61, 0x36, 0x2D, 0x66, 0x33, 0x64, 0x63, 0x66,
		0x30, 0x61, 0x62, 0x31, 0x66, 0x31, 0x61, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x31, 0x3C, 0x53, 0x6F, 0x66, 0x74, 0x77, 0x61,
		0x72, 0x65, 0x49, 0x64, 0x65, 0x6E, 0x74, 0x69, 0x74, 0x79,
		0x20, 0x74, 0x61, 0x67, 0x49, 0x64, 0x3D, 0x22, 0x64, 0x65,
		0x66, 0x22, 0x3E, 0x3C, 0x2F, 0x53, 0x6F, 0x66, 0x74, 0x77,
		0x61, 0x72, 0x65, 0x49, 0x64, 0x65, 0x6E, 0x74, 0x69, 0x74,
		0x79, 0x3E)
	},
	{ IETF_SWIMA_ATTR_SW_INV_FLAG_S_F, 0xaabbccd5, 0x12345678, 0x00000034,
	  chunk_from_chars(
		0x80, 0x00, 0x00, 0x02, 0xAA, 0xBB, 0xCC, 0xD5, 0x12, 0x34,
		0x56, 0x78, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x73, 0x74,
		0x72, 0x6F, 0x6E, 0x67, 0x73, 0x77, 0x61, 0x6E, 0x2E, 0x6F,
		0x72, 0x67, 0x5F, 0x73, 0x74, 0x72, 0x6F, 0x6E, 0x67, 0x53,
		0x77, 0x61, 0x6E, 0x5F, 0x35, 0x2E, 0x33, 0x2E, 0x33, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x90, 0x2A, 0x19, 0x11,
		0x00, 0x00, 0x33, 0x73, 0x74, 0x72, 0x6F, 0x6E, 0x67, 0x73,
		0x77, 0x61, 0x6E, 0x2E, 0x6F, 0x72, 0x67, 0x5F, 0x36, 0x32,
		0x32, 0x35, 0x31, 0x61, 0x61, 0x36, 0x2D, 0x31, 0x61, 0x30,
		0x31, 0x2D, 0x34, 0x37, 0x39, 0x62, 0x2D, 0x61, 0x65, 0x61,
		0x36, 0x2D, 0x66, 0x33, 0x64, 0x63, 0x66, 0x30, 0x61, 0x62,
		0x31, 0x66, 0x31, 0x61, 0x00, 0x00)
	}
};

START_TEST(test_imcv_swima_inv)
{
	pen_type_t type, data_model;
	chunk_t sw_id, record, value;
	ietf_swima_attr_sw_inv_t *c_attr;
	pa_tnc_attr_t *attr, *attr1, *attr2;
	swima_record_t *sw_record;
	swima_inventory_t *sw_inv;
	enumerator_t *enumerator;
	uint32_t offset, epoch;
	uint8_t source_id;
	bool sw_id_only = _i % 2;
	int n;

	attr = ietf_swima_attr_sw_inv_create(sw_inv_data[_i].flags,
										 sw_inv_data[_i].request_id,
										 sw_id_only);

	sw_inv = swima_inventory_create();
	sw_inv->set_eid(sw_inv, sw_inv_data[_i].last_eid, sw_inv_data[_i].eid_epoch);
	for (n = 0; n < _i/2; n++)
	{
		sw_id = chunk_from_str(sw_id_str[n]);
		sw_record = swima_record_create(n, sw_id, chunk_empty);

		if (n == 1)
		{
			sw_record->set_data_model(sw_record, ita_data_model);
			sw_record->set_source_id(sw_record, 0x11);
		}
		if (!sw_id_only)
		{
			record = chunk_from_str(sw_record_str[n]);
			sw_record->set_record(sw_record, record);
		}
		sw_inv->add(sw_inv, sw_record);
	}
	c_attr = (ietf_swima_attr_sw_inv_t*)attr;
	c_attr->set_inventory(c_attr, sw_inv);
	c_attr->set_inventory(c_attr, sw_inv);

	attr->build(attr);
	attr->build(attr);
	sw_inv->destroy(sw_inv);

	type = attr->get_type(attr);
	ck_assert(pen_type_equals(type, sw_inv_types[sw_id_only]));

	ck_assert(attr->get_noskip_flag(attr) == FALSE);
	attr->set_noskip_flag(attr, TRUE);
	ck_assert(attr->get_noskip_flag(attr) == TRUE);

	value = attr->get_value(attr);
	ck_assert_chunk_eq(value, sw_inv_data[_i].value);

	attr1 = attr->get_ref(attr);
	attr->destroy(attr);

	attr2 = ietf_swima_attr_sw_inv_create_from_data(value.len, value,
													sw_id_only);
	ck_assert(attr2);
	attr1->destroy(attr1);
	ck_assert(attr2->process(attr2, &offset) == SUCCESS);

	type = attr2->get_type(attr2);
	ck_assert(pen_type_equals(type, sw_inv_types[sw_id_only]));

	c_attr = (ietf_swima_attr_sw_inv_t*)attr2;
	ck_assert(c_attr->get_flags(c_attr) == sw_inv_data[_i].flags);
	ck_assert(c_attr->get_record_count(c_attr) == 0);
	ck_assert(c_attr->get_request_id(c_attr) == sw_inv_data[_i].request_id);

	sw_inv =  c_attr->get_inventory(c_attr);
	ck_assert(sw_inv->get_eid(sw_inv, NULL) == sw_inv_data[_i].last_eid);
	ck_assert(sw_inv->get_eid(sw_inv, &epoch) == sw_inv_data[_i].last_eid);
	ck_assert(epoch == sw_inv_data[_i].eid_epoch);
	ck_assert(sw_inv);
	ck_assert(sw_inv->get_count(sw_inv) == _i/2);

	enumerator = sw_inv->create_enumerator(sw_inv);
	ck_assert(enumerator);

	n = 0;
	while (enumerator->enumerate(enumerator, &sw_record))
	{
		ck_assert(sw_record->get_record_id(sw_record) == n);
		data_model = sw_record->get_data_model(sw_record);
		ck_assert(pen_type_equals(data_model, (n == 1) ? ita_data_model :
								  swima_data_model_iso_2015_swid_xml));
		source_id = sw_record->get_source_id(sw_record);
		ck_assert(source_id == (n == 1 ? 0x11 : 0x00));
		n++;
	}
	enumerator->destroy(enumerator);
	ck_assert(n == _i/2);

	attr2->destroy(attr2);
}
END_TEST

/**
 * Offsets in sw_inv_data[4].value
 *
 *   0 constant header
 *  12   segment  1  -  12 octets
 *  16 record_id
 *  18   segment  2  -   6 octets
 *  20 data_model_pen
 *  22   segment  3  -   4 octets
 *  23   segment  4  -   1 octet
 *  23 data_model_type
 *  24   segment  5  -   1 octet
 *  24 source_id
 *  25   segment  6  -   1 octet
 *  25 reserved
 *  26 sw_id
 *  27   segment  7  -   2 octets
 *  59 sw_locator
 *  60   segment  8  -  33 octets
 *  61 record
 *  63   segment  9  -   3 octets
 * 114 sw record 2
 * 115   segment 10  -  52 octets
 * 231   segment 11  - 117 octets
 */

START_TEST(test_imcv_swima_sw_inv_trunc)
{
	pa_tnc_attr_t *attr;
	ietf_swima_attr_sw_inv_t *c_attr;
	chunk_t data;
	swima_inventory_t *sw_inv;
	size_t len = sw_inv_data[4].value.len;
	uint32_t offset = 100;

	/* Data smaller than minimum size */
	attr = ietf_swima_attr_sw_inv_create_from_data(0, chunk_empty, FALSE);
	ck_assert(attr);
	ck_assert(attr->process(attr, &offset) == FAILED && offset == 0);
	attr->destroy(attr);

	/* Length larger than data */
	data = sw_inv_data[4].value;
	attr = ietf_swima_attr_sw_inv_create_from_data(len + 2, data, FALSE);
	ck_assert(attr);
	ck_assert(attr->process(attr, &offset) == FAILED && offset == len);
	attr->destroy(attr);

	/* Segment 1 truncates minimum size */
	data = sw_inv_data[4].value;
	data.len = 12;
	attr = ietf_swima_attr_sw_inv_create_from_data(len, data, FALSE);
	ck_assert(attr);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 2 truncates record_id */
	data = chunk_skip(sw_inv_data[4].value, 12);
	data.len = 6;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 3 truncates data_model_pen */
	data = chunk_skip(sw_inv_data[4].value, 18);
	data.len = 4;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 4 truncates data_model_type */
	data = chunk_skip(sw_inv_data[4].value, 22);
	data.len = 1;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 5 truncates source_id */
	data = chunk_skip(sw_inv_data[4].value, 23);
	data.len = 1;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 6 truncates reserved */
	data = chunk_skip(sw_inv_data[4].value, 24);
	data.len = 1;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 7 truncates sw_id */
	data = chunk_skip(sw_inv_data[4].value, 25);
	data.len = 2;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 8 truncates sw_locator */
	data = chunk_skip(sw_inv_data[4].value, 27);
	data.len = 33;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 9 truncates record */
	data = chunk_skip(sw_inv_data[4].value, 60);
	data.len = 3;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 10 truncates second sw_record */
	data = chunk_skip(sw_inv_data[4].value, 63);
	data.len = 52;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == SUCCESS);

	/* Process first inventory entry */
	c_attr = (ietf_swima_attr_sw_inv_t*)attr;
	sw_inv = c_attr->get_inventory(c_attr);
	ck_assert(sw_inv->get_count(sw_inv) == 1);
	c_attr->clear_inventory(c_attr);

	/* Segment 11 truncates second sw_record */
	data = chunk_skip(sw_inv_data[4].value, 115);
	data.len = 117;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == SUCCESS);

	/* Process second inventory entry */
	sw_inv = c_attr->get_inventory(c_attr);
	ck_assert(sw_inv->get_count(sw_inv) == 1);
	c_attr->clear_inventory(c_attr);

	attr->destroy(attr);
}
END_TEST

static char* sw_ev_timestamp_str[] = {
	"2017-05-30T18:09:25Z",
	"2017-06-14T15:38:00Z"
};

START_TEST(test_imcv_swima_event)
{
	chunk_t sw_id, sw_timestamp, timestamp;
	swima_event_t *sw_event, *sw_event_cp;
	swima_record_t *sw_record;
	uint32_t record_id = 1, eid = 7;
	uint8_t action = SWIMA_EVENT_ACTION_CREATION;

	sw_id = chunk_from_str(sw_id_str[0]);
	sw_timestamp = chunk_from_str(sw_ev_timestamp_str[0]);

	/* Software Identity without Software Locator */
	sw_record = swima_record_create(record_id, sw_id, chunk_empty),
	ck_assert(sw_record);

	sw_event = swima_event_create(eid, sw_timestamp, action, sw_record);
	ck_assert(sw_event);
	sw_event_cp = sw_event->get_ref(sw_event);

	ck_assert(sw_event->get_eid(sw_event, NULL) == eid);
	ck_assert(sw_event->get_eid(sw_event, &timestamp) == eid);
	ck_assert_chunk_eq(sw_timestamp, timestamp);
	ck_assert(sw_event->get_action(sw_event) == action);
	sw_event->destroy(sw_event);

	sw_record = sw_event_cp->get_sw_record(sw_event_cp);
	ck_assert(sw_record);
	ck_assert(sw_record->get_record_id(sw_record) == record_id);
	ck_assert_chunk_eq(sw_record->get_sw_id(sw_record, NULL), sw_id);
	sw_event_cp->destroy(sw_event_cp);
}
END_TEST

static pen_type_t sw_ev_types[] = {
	{ PEN_IETF, IETF_ATTR_SW_EVENTS },
	{ PEN_IETF, IETF_ATTR_SW_ID_EVENTS }
};

typedef struct sw_ev_data_t sw_ev_data_t;

struct sw_ev_data_t {
	uint8_t flags;
	uint32_t request_id;
	uint32_t eid_epoch;
	uint32_t last_eid;
	uint32_t last_consulted_eid;
	chunk_t  value;
};

static sw_ev_data_t sw_ev_data[] = {
	{ IETF_SWIMA_ATTR_SW_EV_FLAG_NONE, 0xaabbccd0, 0x87654321, 0x00000007,
	  0x00000007, chunk_from_chars(
		0x00, 0x00, 0x00, 0x00, 0xAA, 0xBB, 0xCC, 0xD0, 0x87, 0x65,
		0x43, 0x21, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x07)
	},
	{ IETF_SWIMA_ATTR_SW_EV_FLAG_NONE, 0xaabbccd1, 0x87654321, 0x00000007,
	  0x00000007, chunk_from_chars(
		0x00, 0x00, 0x00, 0x00, 0xAA, 0xBB, 0xCC, 0xD1, 0x87, 0x65,
		0x43, 0x21, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x07)
	},
	{ IETF_SWIMA_ATTR_SW_EV_FLAG_NONE, 0xaabbccd2, 0x12345678, 0x00000030,
	  0x00000030, chunk_from_chars(
		0x00, 0x00, 0x00, 0x01, 0xAA, 0xBB, 0xCC, 0xD2, 0x12, 0x34,
		0x56, 0x78, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x30,
		0x00, 0x00, 0x00, 0x30,  '2',  '0',  '1',  '7',  '-',  '0',
		 '5',  '-',  '3',  '0',  'T',  '1',  '8',  ':',  '0',  '9',
		 ':',  '2',  '5',  'Z', 0x00, 0x00, 0x00, 0x00,	0x00, 0x00,
		0x00, 0x00, 0x00, 0x01, 0x00, 0x1F, 0x73, 0x74,	0x72, 0x6F,
		0x6E, 0x67, 0x73, 0x77, 0x61, 0x6E, 0x2E, 0x6F,	0x72, 0x67,
		0x5F, 0x73, 0x74, 0x72, 0x6F, 0x6E, 0x67, 0x53,	0x77, 0x61,
		0x6E, 0x5F, 0x35, 0x2E, 0x33, 0x2E, 0x33, 0x00,	0x00, 0x00,
		0x00, 0x00, 0x31, 0x3C, 0x53, 0x6F, 0x66, 0x74,	0x77, 0x61,
		0x72, 0x65, 0x49, 0x64, 0x65, 0x6E, 0x74, 0x69,	0x74, 0x79,
		0x20, 0x74, 0x61, 0x67, 0x49, 0x64, 0x3D, 0x22,	0x61, 0x62,
		0x63, 0x22, 0x3E, 0x3C, 0x2F, 0x53, 0x6F, 0x66,	0x74, 0x77,
		0x61, 0x72, 0x65, 0x49, 0x64, 0x65, 0x6E, 0x74,	0x69, 0x74,
		0x79, 0x3E)
	},
	{ IETF_SWIMA_ATTR_SW_EV_FLAG_NONE, 0xaabbccd3, 0x12345678, 0x00000030,
	  0x00000030, chunk_from_chars(
		0x00, 0x00, 0x00, 0x01, 0xAA, 0xBB, 0xCC, 0xD3, 0x12, 0x34,
		0x56, 0x78, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x30,
		0x00, 0x00, 0x00, 0x30,  '2',  '0',  '1',  '7',  '-',  '0',
		 '5',  '-',  '3',  '0',  'T',  '1',  '8',  ':',  '0',  '9',
		 ':',  '2',  '5',  'Z', 0x00, 0x00, 0x00, 0x00,	0x00, 0x00,
		0x00, 0x00, 0x00, 0x01, 0x00, 0x1F, 0x73, 0x74,	0x72, 0x6F,
		0x6E, 0x67, 0x73, 0x77, 0x61, 0x6E, 0x2E, 0x6F,	0x72, 0x67,
		0x5F, 0x73, 0x74, 0x72, 0x6F, 0x6E, 0x67, 0x53,	0x77, 0x61,
		0x6E, 0x5F, 0x35, 0x2E, 0x33, 0x2E, 0x33, 0x00,	0x00)
	},
	{ IETF_SWIMA_ATTR_SW_EV_FLAG_S_F, 0xaabbccd4, 0x12345678, 0x00000050,
	  0x00000034, chunk_from_chars(
		0x80, 0x00, 0x00, 0x02, 0xAA, 0xBB, 0xCC, 0xD4, 0x12, 0x34,
		0x56, 0x78, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x34,
		0x00, 0x00, 0x00, 0x30,  '2',  '0',  '1',  '7',  '-',  '0',
		 '5',  '-',  '3',  '0',  'T',  '1',  '8',  ':',  '0',  '9',
		 ':',  '2',  '5',  'Z', 0x00, 0x00, 0x00, 0x00,	0x00, 0x00,
		0x00, 0x00, 0x00, 0x01, 0x00, 0x1F, 0x73, 0x74,	0x72, 0x6F,
		0x6E, 0x67, 0x73, 0x77, 0x61, 0x6E, 0x2E, 0x6F,	0x72, 0x67,
		0x5F, 0x73, 0x74, 0x72, 0x6F, 0x6E, 0x67, 0x53,	0x77, 0x61,
		0x6E, 0x5F, 0x35, 0x2E, 0x33, 0x2E, 0x33, 0x00,	0x00, 0x00,
		0x00, 0x00, 0x31, 0x3C, 0x53, 0x6F, 0x66, 0x74,	0x77, 0x61,
		0x72, 0x65, 0x49, 0x64, 0x65, 0x6E, 0x74, 0x69,	0x74, 0x79,
		0x20, 0x74, 0x61, 0x67, 0x49, 0x64, 0x3D, 0x22,	0x61, 0x62,
		0x63, 0x22, 0x3E, 0x3C, 0x2F, 0x53, 0x6F, 0x66,	0x74, 0x77,
		0x61, 0x72, 0x65, 0x49, 0x64, 0x65, 0x6E, 0x74,	0x69, 0x74,
		0x79, 0x3E, 0x00, 0x00, 0x00, 0x34,  '2',  '0',  '1',  '7',
		 '-',  '0',  '6',  '-',  '1',  '4',  'T',  '1',  '5',  ':',
		 '3',  '8',  ':',  '0',  '0',  'Z', 0x00, 0x00, 0x00, 0x01,
		0x00, 0x90, 0x2A, 0x19, 0x11, 0x02, 0x00, 0x33, 0x73, 0x74,
		0x72, 0x6F, 0x6E, 0x67, 0x73, 0x77, 0x61, 0x6E, 0x2E, 0x6F,
		0x72, 0x67, 0x5F, 0x36, 0x32, 0x32, 0x35, 0x31, 0x61, 0x61,
		0x36, 0x2D, 0x31, 0x61, 0x30, 0x31, 0x2D, 0x34, 0x37, 0x39,
		0x62, 0x2D, 0x61, 0x65, 0x61, 0x36, 0x2D, 0x66, 0x33, 0x64,
		0x63, 0x66, 0x30, 0x61, 0x62, 0x31, 0x66, 0x31, 0x61, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x31, 0x3C, 0x53, 0x6F, 0x66, 0x74,
		0x77, 0x61, 0x72, 0x65, 0x49, 0x64, 0x65, 0x6E, 0x74, 0x69,
		0x74, 0x79, 0x20, 0x74, 0x61, 0x67, 0x49, 0x64, 0x3D, 0x22,
		0x64, 0x65, 0x66, 0x22, 0x3E, 0x3C, 0x2F, 0x53, 0x6F, 0x66,
		0x74, 0x77, 0x61, 0x72, 0x65, 0x49, 0x64, 0x65, 0x6E, 0x74,
		0x69, 0x74, 0x79, 0x3E)
	},
	{ IETF_SWIMA_ATTR_SW_EV_FLAG_S_F, 0xaabbccd5, 0x12345678, 0x00000050,
	  0x00000034, chunk_from_chars(
		0x80, 0x00, 0x00, 0x02, 0xAA, 0xBB, 0xCC, 0xD5, 0x12, 0x34,
		0x56, 0x78, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x34,
		0x00, 0x00, 0x00, 0x30,  '2',  '0',  '1',  '7',  '-',  '0',
		 '5',  '-',  '3',  '0',  'T',  '1',  '8',  ':',  '0',  '9',
		 ':',  '2',  '5',  'Z', 0x00, 0x00, 0x00, 0x00,	0x00, 0x00,
		0x00, 0x00, 0x00, 0x01, 0x00, 0x1F, 0x73, 0x74,	0x72, 0x6F,
		0x6E, 0x67, 0x73, 0x77, 0x61, 0x6E, 0x2E, 0x6F,	0x72, 0x67,
		0x5F, 0x73, 0x74, 0x72, 0x6F, 0x6E, 0x67, 0x53,	0x77, 0x61,
		0x6E, 0x5F, 0x35, 0x2E, 0x33, 0x2E, 0x33, 0x00,	0x00, 0x00,
		0x00, 0x00, 0x34,  '2',  '0',  '1',  '7', '-',   '0',  '6',
		 '-',  '1',  '4',  'T',  '1',  '5',  ':', '3',  '8',  ':',
		 '0',  '0',  'Z', 0x00, 0x00, 0x00, 0x01, 0x00, 0x90, 0x2A,
		0x19, 0x11, 0x02, 0x00, 0x33, 0x73, 0x74, 0x72, 0x6F, 0x6E,
		0x67, 0x73, 0x77, 0x61, 0x6E, 0x2E, 0x6F, 0x72, 0x67, 0x5F,
		0x36, 0x32, 0x32, 0x35, 0x31, 0x61, 0x61, 0x36, 0x2D, 0x31,
		0x61, 0x30, 0x31, 0x2D, 0x34, 0x37, 0x39, 0x62, 0x2D, 0x61,
		0x65, 0x61, 0x36, 0x2D, 0x66, 0x33, 0x64, 0x63, 0x66, 0x30,
		0x61, 0x62, 0x31, 0x66, 0x31, 0x61, 0x00, 0x00)
	}
};

START_TEST(test_imcv_swima_ev)
{
	pen_type_t type, data_model;
	chunk_t sw_id, record, timestamp, value;
	ietf_swima_attr_sw_ev_t *c_attr;
	pa_tnc_attr_t *attr, *attr1, *attr2;
	swima_record_t *sw_record;
	swima_event_t *sw_event;
	swima_events_t *sw_ev;
	enumerator_t *enumerator;
	uint32_t offset, epoch, eid, last_eid;
	uint8_t source_id, action;
	bool sw_id_only = _i % 2;
	int n;

	attr = ietf_swima_attr_sw_ev_create(sw_ev_data[_i].flags,
										sw_ev_data[_i].request_id,
										sw_id_only);
	sw_ev = swima_events_create();
	sw_ev->set_eid(sw_ev, sw_ev_data[_i].last_consulted_eid,
						  sw_ev_data[_i].eid_epoch);
	if (sw_ev_data[_i].last_consulted_eid < sw_ev_data[_i].last_eid)
	{
		sw_ev->set_last_eid(sw_ev, sw_ev_data[_i].last_eid);
	}

	for (n = 0; n < _i/2; n++)
	{
		sw_id = chunk_from_str(sw_id_str[n]);
		sw_record = swima_record_create(n, sw_id, chunk_empty);

		if (n == 1)
		{
			sw_record->set_data_model(sw_record, ita_data_model);
			sw_record->set_source_id(sw_record, 0x11);
		}
		if (!sw_id_only)
		{
			record = chunk_from_str(sw_record_str[n]);
			sw_record->set_record(sw_record, record);
		}
		eid = 0x30 + 4 * n;
		timestamp = chunk_from_str(sw_ev_timestamp_str[n]);
		action = n + 1;
		sw_event = swima_event_create(eid, timestamp, action, sw_record);
		sw_ev->add(sw_ev, sw_event);
	}
	c_attr = (ietf_swima_attr_sw_ev_t*)attr;
	c_attr->set_events(c_attr, sw_ev);
	c_attr->set_events(c_attr, sw_ev);

	attr->build(attr);
	attr->build(attr);
	sw_ev->destroy(sw_ev);

	type = attr->get_type(attr);
	ck_assert(pen_type_equals(type, sw_ev_types[sw_id_only]));

	ck_assert(attr->get_noskip_flag(attr) == FALSE);
	attr->set_noskip_flag(attr, TRUE);
	ck_assert(attr->get_noskip_flag(attr) == TRUE);

	value = attr->get_value(attr);
	ck_assert_chunk_eq(value, sw_ev_data[_i].value);

	attr1 = attr->get_ref(attr);
	attr->destroy(attr);

	attr2 = ietf_swima_attr_sw_ev_create_from_data(value.len, value,
												   sw_id_only);
	ck_assert(attr2);
	attr1->destroy(attr1);
	ck_assert(attr2->process(attr2, &offset) == SUCCESS);

	type = attr2->get_type(attr2);
	ck_assert(pen_type_equals(type, sw_ev_types[sw_id_only]));

	c_attr = (ietf_swima_attr_sw_ev_t*)attr2;
	ck_assert(c_attr->get_flags(c_attr) == sw_ev_data[_i].flags);
	ck_assert(c_attr->get_event_count(c_attr) == 0);
	ck_assert(c_attr->get_request_id(c_attr) == sw_ev_data[_i].request_id);

	sw_ev =  c_attr->get_events(c_attr);
	ck_assert(sw_ev);
	eid = sw_ev->get_eid(sw_ev, NULL, NULL);
	ck_assert(eid == sw_ev_data[_i].last_consulted_eid);
	eid = sw_ev->get_eid(sw_ev, &epoch, &last_eid);
	ck_assert(eid == sw_ev_data[_i].last_consulted_eid);
	ck_assert(epoch == sw_ev_data[_i].eid_epoch);
	ck_assert(last_eid == sw_ev_data[_i].last_eid);
	ck_assert(sw_ev->get_count(sw_ev) == _i/2);

	enumerator = sw_ev->create_enumerator(sw_ev);
	ck_assert(enumerator);

	n = 0;
	while (enumerator->enumerate(enumerator, &sw_event))
	{
		ck_assert(sw_event->get_eid(sw_event, &timestamp) == 0x30 + 4 * n);
		ck_assert_chunk_eq(timestamp, chunk_from_str(sw_ev_timestamp_str[n]));
		sw_record = sw_event->get_sw_record(sw_event);
		ck_assert(sw_record);
		ck_assert(sw_record->get_record_id(sw_record) == n);
		data_model = sw_record->get_data_model(sw_record);
		ck_assert(pen_type_equals(data_model, (n == 1) ? ita_data_model :
								  swima_data_model_iso_2015_swid_xml));
		source_id = sw_record->get_source_id(sw_record);
		ck_assert(source_id == (n == 1 ? 0x11 : 0x00));
		n++;
	}
	enumerator->destroy(enumerator);
	ck_assert(n == _i/2);

	attr2->destroy(attr2);
}
END_TEST


/**
 * Offsets in sw_ev_data[4].value
 *
 *   0 constant header
 *  16   segment  1  -  16 octets
 *  20 eid
 *  22   segment  2  -   6 octets
 *  24 timestamp
 *  26   segment  3  -   4 octets
 *  44 record_id
 *  46   segment  4  -  20 octets
 *  48 data_model_pen
 *  50   segment  5  -   4 octets
 *  51   segment  6  -   1 octet
 *  51 data_model_type
 *  52   segment  7  -   1 octet
 *  52 source_id
 *  53   segment  8  -   1 octet
 *  53 action
 *  54 sw_id
 *  55   segment  9  -   2 octets
 *  87 sw_locator
 *  88   segment 10  -  33 octets
 *  89 record
 *  91   segment 11  -   3 octets
 * 142 sw record 2
 * 143   segment 12  -  52 octets
 * 284   segment 13  - 141 octets
 */

START_TEST(test_imcv_swima_sw_ev_trunc)
{
	pa_tnc_attr_t *attr;
	ietf_swima_attr_sw_ev_t *c_attr;
	chunk_t data;
	swima_events_t *sw_ev;
	size_t len = sw_ev_data[4].value.len;
	uint32_t offset = 100;

	/* Data smaller than minimum size */
	attr = ietf_swima_attr_sw_ev_create_from_data(0, chunk_empty, FALSE);
	ck_assert(attr);
	ck_assert(attr->process(attr, &offset) == FAILED && offset == 0);
	attr->destroy(attr);

	/* Length larger than data */
	data = sw_ev_data[4].value;
	attr = ietf_swima_attr_sw_ev_create_from_data(len + 2, data, FALSE);
	ck_assert(attr);
	ck_assert(attr->process(attr, &offset) == FAILED && offset == len);
	attr->destroy(attr);

	/* Segment 1 truncates minimum size */
	data = sw_ev_data[4].value;
	data.len = 16;
	attr = ietf_swima_attr_sw_ev_create_from_data(len, data, FALSE);
	ck_assert(attr);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 2 truncates eid */
	data = chunk_skip(sw_ev_data[4].value, 16);
	data.len = 6;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 3 truncates timestamp */
	data = chunk_skip(sw_ev_data[4].value, 22);
	data.len = 4;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 4 truncates record_id */
	data = chunk_skip(sw_ev_data[4].value, 26);
	data.len = 20;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 5 truncates data_model_pen */
	data = chunk_skip(sw_ev_data[4].value, 46);
	data.len = 4;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 6 truncates data_model_type */
	data = chunk_skip(sw_ev_data[4].value, 50);
	data.len = 1;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 7 truncates source_id */
	data = chunk_skip(sw_ev_data[4].value, 51);
	data.len = 1;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 8 truncates action */
	data = chunk_skip(sw_ev_data[4].value, 52);
	data.len = 1;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 9 truncates sw_id */
	data = chunk_skip(sw_ev_data[4].value, 53);
	data.len = 2;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 10 truncates sw_locator */
	data = chunk_skip(sw_ev_data[4].value, 55);
	data.len = 33;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 11 truncates record */
	data = chunk_skip(sw_ev_data[4].value, 88);
	data.len = 3;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == NEED_MORE);

	/* Segment 12 truncates second sw_entry */
	data = chunk_skip(sw_ev_data[4].value, 91);
	data.len = 52;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == SUCCESS);

	/* Process first event entry */
	c_attr = (ietf_swima_attr_sw_ev_t*)attr;
	sw_ev = c_attr->get_events(c_attr);
	ck_assert(sw_ev->get_count(sw_ev) == 1);
	c_attr->clear_events(c_attr);

	/* Segment 13 truncates second sw_record */
	data = chunk_skip(sw_ev_data[4].value, 143);
	data.len = 141;
	attr->add_segment(attr, data);
	ck_assert(attr->process(attr, &offset) == SUCCESS);

	/* Process second event entry */
	sw_ev = c_attr->get_events(c_attr);
	ck_assert(sw_ev->get_count(sw_ev) == 1);
	c_attr->clear_events(c_attr);
	attr->destroy(attr);

	/* Invalid Action values */
	data = chunk_clone(sw_ev_data[2].value);
	data.ptr[53] = 0;
	attr = ietf_swima_attr_sw_ev_create_from_data(data.len, data, FALSE);
	ck_assert(attr);
	ck_assert(attr->process(attr, &offset) == FAILED);
	attr->destroy(attr);

	data.ptr[53] = SWIMA_EVENT_ACTION_LAST + 1;
	attr = ietf_swima_attr_sw_ev_create_from_data(data.len, data, FALSE);
	ck_assert(attr);
	ck_assert(attr->process(attr, &offset) == FAILED && offset == 20);
	attr->destroy(attr);
	chunk_free(&data);
}
END_TEST

START_TEST(test_imcv_swima_sw_collector)
{
	swima_collector_t *collector;
	swima_inventory_t *targets, *inventory;
	swima_events_t *events;
	swima_record_t *sw_record;
	swima_event_t *sw_event;
	chunk_t sw_id, sw_locator, swid_tag;
	enumerator_t *enumerator;
	uint8_t source_id;
	int item = 0, items;

	targets = swima_inventory_create();
	collector = swima_collector_create();

	/* software identifier events only */
	events = collector->collect_events(collector, TRUE, targets);
	if (events)
	{
		items = events->get_count(events);
		DBG1(DBG_IMC, "%d software identifiers collected", items);

		enumerator = events->create_enumerator(events);
		while (enumerator->enumerate(enumerator, &sw_event))
		{
			item++;
			if ( item == 1 || item == items)
			{
				sw_record = sw_event->get_sw_record(sw_event);
				sw_id = sw_record->get_sw_id(sw_record, NULL);
				source_id =sw_record->get_source_id(sw_record);
				DBG1(DBG_IMC, "source %u: %.*s", source_id, sw_id.len, sw_id.ptr);
			}
		}
		enumerator->destroy(enumerator);
	}

	/* software identifier inventory only */
	inventory = collector->collect_inventory(collector, TRUE, targets);
	if (inventory)
	{
		items = inventory->get_count(inventory);
		DBG1(DBG_IMC, "%d software identifiers collected", items);

		enumerator = inventory->create_enumerator(inventory);
		while (enumerator->enumerate(enumerator, &sw_record))
		{
			item++;
			if ( item == 1 || item == items)
			{
				sw_id = sw_record->get_sw_id(sw_record, &sw_locator);
				source_id =sw_record->get_source_id(sw_record);
				DBG1(DBG_IMC, "source %u: %.*s", source_id, sw_id.len, sw_id.ptr);
				if (sw_locator.len > 0)
				{
					DBG1(DBG_IMC, "          locator: %.*s",
						 sw_locator.len, sw_locator.ptr);
				}
				targets->add(targets, sw_record->get_ref(sw_record));
			}
		}
		enumerator->destroy(enumerator);
	}

	/* targeted software inventory */
	inventory = collector->collect_inventory(collector, FALSE, targets);
	if (inventory)
	{
		items = inventory->get_count(inventory);
		DBG1(DBG_IMC, "%d SWID tags collected", items);

		enumerator = inventory->create_enumerator(inventory);
		while (enumerator->enumerate(enumerator, &sw_record))
		{
			sw_id = sw_record->get_sw_id(sw_record, NULL);
			source_id =sw_record->get_source_id(sw_record);
			swid_tag = sw_record->get_record(sw_record);
			DBG1(DBG_IMC, "source %u: %.*s", source_id, sw_id.len, sw_id.ptr);
			DBG2(DBG_IMC, "%B", &swid_tag);
		}
		enumerator->destroy(enumerator);
	}

	collector->destroy(collector);
	targets->destroy(targets);
}
END_TEST

Suite *imcv_swima_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("imcv_swima");

	tc = tcase_create("sw_record");
	tcase_add_test(tc, test_imcv_swima_record);
	suite_add_tcase(s, tc);

	tc = tcase_create("sw_req");
	tcase_add_loop_test(tc, test_imcv_swima_sw_req, 0, countof(req_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("sw_req_trunc");
	tcase_add_test(tc, test_imcv_swima_sw_req_trunc);
	suite_add_tcase(s, tc);

	tc = tcase_create("sw_inv");
	tcase_add_loop_test(tc, test_imcv_swima_inv, 0, 6);
	suite_add_tcase(s, tc);

	tc = tcase_create("sw_inv_trunc");
	tcase_add_test(tc, test_imcv_swima_sw_inv_trunc);
	suite_add_tcase(s, tc);

	tc = tcase_create("sw_event");
	tcase_add_test(tc, test_imcv_swima_event);
	suite_add_tcase(s, tc);

	tc = tcase_create("sw_ev");
	tcase_add_loop_test(tc, test_imcv_swima_ev, 0, 6);
	suite_add_tcase(s, tc);

	tc = tcase_create("sw_ev_trunc");
	tcase_add_test(tc, test_imcv_swima_sw_ev_trunc);
	suite_add_tcase(s, tc);

	tc = tcase_create("sw_collector");
	tcase_add_test(tc, test_imcv_swima_sw_collector);
	suite_add_tcase(s, tc);

	return s;
}
