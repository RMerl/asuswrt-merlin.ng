/*
 * Copyright (C) 2013-2015 Tobias Brunner
 * Copyright (C) 2016 Andreas Steffen
 * Copyright (C) 2009 Martin Willi
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

#include <utils/identification.h>

/*******************************************************************************
 * create (_from_encoding, _from_data, _from_string, _from_sockaddr)
 */

START_TEST(test_from_encoding)
{
	identification_t *a;
	chunk_t expected, encoding;

	/* only ID_ANY is handled differently, for all other types the following
	 * applies.  should we perhaps test that this is in fact the case? */
	expected = chunk_from_str("moon@strongswan.org");
	a = identification_create_from_encoding(ID_RFC822_ADDR, expected);
	ck_assert(ID_RFC822_ADDR == a->get_type(a));
	encoding = a->get_encoding(a);
	ck_assert(expected.ptr != encoding.ptr);
	ck_assert(chunk_equals(expected, encoding));
	a->destroy(a);

	a = identification_create_from_encoding(ID_ANY, expected);
	ck_assert(ID_ANY == a->get_type(a));
	encoding = a->get_encoding(a);
	ck_assert(encoding.ptr == NULL);
	ck_assert(encoding.len == 0);
	a->destroy(a);
}
END_TEST

START_TEST(test_from_data)
{
	identification_t *a;
	chunk_t expected, encoding;

	/* this uses the DN parser (C=CH) */
	expected = chunk_from_chars(0x30, 0x0d, 0x31, 0x0b, 0x30, 0x09, 0x06,
								0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x43, 0x48);
	a = identification_create_from_data(expected);
	ck_assert(ID_DER_ASN1_DN == a->get_type(a));
	encoding = a->get_encoding(a);
	ck_assert(expected.ptr != encoding.ptr);
	ck_assert(chunk_equals(expected, encoding));
	a->destroy(a);

	/* everything else is handled by the string parser */
	expected = chunk_from_str("moon@strongswan.org");
	a = identification_create_from_data(expected);
	ck_assert(ID_RFC822_ADDR == a->get_type(a));
	encoding = a->get_encoding(a);
	ck_assert(expected.ptr != encoding.ptr);
	ck_assert(chunk_equals(expected, encoding));
	a->destroy(a);
}
END_TEST

START_TEST(test_from_sockaddr)
{
	identification_t *a;
	chunk_t expected, encoding;
	struct sockaddr_in in = {
		.sin_family = AF_INET,
	};
	struct sockaddr_in6 in6 = {
		.sin6_family = AF_INET6,
	};

	expected = chunk_from_chars(0xc0, 0xa8, 0x01, 0x01);
	memcpy(&in.sin_addr, expected.ptr, sizeof(in.sin_addr));
	a = identification_create_from_sockaddr((sockaddr_t*)&in);
	ck_assert(ID_IPV4_ADDR == a->get_type(a));
	encoding = a->get_encoding(a);
	ck_assert(chunk_equals(expected, encoding));
	a->destroy(a);

	expected = chunk_from_chars(0xfe, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
								0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01);
	memcpy(&in6.sin6_addr, expected.ptr, sizeof(in6.sin6_addr));
	a = identification_create_from_sockaddr((sockaddr_t*)&in6);
	ck_assert(ID_IPV6_ADDR == a->get_type(a));
	encoding = a->get_encoding(a);
	ck_assert(chunk_equals(expected, encoding));
	a->destroy(a);

	in6.sin6_family = AF_UNSPEC;
	a = identification_create_from_sockaddr((sockaddr_t*)&in6);
	ck_assert(ID_ANY == a->get_type(a));
	a->destroy(a);
}
END_TEST

static struct {
	char *id;
	id_type_t type;
	struct {
		enum {
			ENC_CHUNK,
			ENC_STRING,
			ENC_SIMPLE,
		} type;
		union {
			chunk_t c;
			char *s;
		} data;
	} result;
} string_data[] = {
	{NULL,						ID_ANY,					{ .type = ENC_CHUNK  }},
	{"",						ID_ANY,					{ .type = ENC_CHUNK  }},
	{"%any",					ID_ANY,					{ .type = ENC_CHUNK  }},
	{"%any6",					ID_ANY,					{ .type = ENC_CHUNK  }},
	{"0.0.0.0",					ID_ANY,					{ .type = ENC_CHUNK  }},
	{"0::0",					ID_ANY,					{ .type = ENC_CHUNK  }},
	{"::",						ID_ANY,					{ .type = ENC_CHUNK  }},
	{"*",						ID_ANY,					{ .type = ENC_CHUNK  }},
	{"any",						ID_FQDN,				{ .type = ENC_SIMPLE }},
	{"any6",					ID_FQDN,				{ .type = ENC_SIMPLE }},
	{"0",						ID_FQDN,				{ .type = ENC_SIMPLE }},
	{"**",						ID_FQDN,				{ .type = ENC_SIMPLE }},
	{"192.168.1.1",				ID_IPV4_ADDR,			{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xc0,0xa8,0x01,0x01) }},
	{"192.168.",				ID_FQDN,				{ .type = ENC_SIMPLE }},
	{".",						ID_FQDN,				{ .type = ENC_SIMPLE }},
	{"192.168.1.1/33",			ID_FQDN,				{ .type = ENC_SIMPLE }},
	{"192.168.1.1/32",			ID_IPV4_ADDR_SUBNET,	{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xc0,0xa8,0x01,0x01,0xff,0xff,0xff,0xff)  }},
	{"192.168.1.1/31",			ID_IPV4_ADDR_SUBNET,	{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xc0,0xa8,0x01,0x00,0xff,0xff,0xff,0xfe)  }},
	{"192.168.1.8/30",			ID_IPV4_ADDR_SUBNET,	{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xc0,0xa8,0x01,0x08,0xff,0xff,0xff,0xfc)  }},
	{"192.168.1.128/25",		ID_IPV4_ADDR_SUBNET,	{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xc0,0xa8,0x01,0x80,0xff,0xff,0xff,0x80)  }},
	{"192.168.1.0/24",			ID_IPV4_ADDR_SUBNET,	{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xc0,0xa8,0x01,0x00,0xff,0xff,0xff,0x00)  }},
	{"192.168.1.0/23",			ID_IPV4_ADDR_SUBNET,	{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xc0,0xa8,0x00,0x00,0xff,0xff,0xfe,0x00)  }},
	{"192.168.4.0/22",			ID_IPV4_ADDR_SUBNET,	{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xc0,0xa8,0x04,0x00,0xff,0xff,0xfc,0x00)  }},
	{"0.0.0.0/0",				ID_IPV4_ADDR_SUBNET,	{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00)  }},
	{"192.168.1.0-192.168.1.40",ID_IPV4_ADDR_RANGE,		{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xc0,0xa8,0x01,0x00,0xc0,0xa8,0x01,0x28)  }},
	{"0.0.0.0-255.255.255.255",	ID_IPV4_ADDR_RANGE,		{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff)  }},
	{"192.168.1.40-192.168.1.0",ID_FQDN,				{ .type = ENC_SIMPLE }},
	{"fec0::1",					ID_IPV6_ADDR,			{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xfe,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,
								   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01)  }},
	{"fec0::",					ID_IPV6_ADDR,			{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xfe,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,
								   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00)  }},
	{"fec0:",					ID_KEY_ID,				{ .type = ENC_SIMPLE }},
	{":",						ID_KEY_ID,				{ .type = ENC_SIMPLE }},
	{"fec0::1/129",				ID_KEY_ID,				{ .type = ENC_SIMPLE }},
	{"fec0::1/128",				ID_IPV6_ADDR_SUBNET,	{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xfe,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,
								   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
								   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
								   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff ) }},
	{"fec0::1/127",				ID_IPV6_ADDR_SUBNET,	{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xfe,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,
								   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
								   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
								   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe ) }},
	{"fec0::4/126",				ID_IPV6_ADDR_SUBNET,	{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xfe,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,
								   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,
								   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
								   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfc ) }},
	{"fec0::100/120",			ID_IPV6_ADDR_SUBNET,	{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xfe,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,
								   0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,
								   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
								   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00 ) }},
	{"::/0",					ID_IPV6_ADDR_SUBNET,	{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
								   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
								   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
								   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 ) }},
	{"fec0::1-fec0::4fff",		ID_IPV6_ADDR_RANGE,		{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xfe,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,
								   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
								   0xfe,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,
								   0x00,0x00,0x00,0x00,0x00,0x00,0x4f,0xff ) }},
	{"fec0::4fff-fec0::1",		ID_KEY_ID,				{ .type = ENC_SIMPLE }},
	{"fec0::1-",				ID_KEY_ID,				{ .type = ENC_SIMPLE }},
	{"alice@strongswan.org",	ID_RFC822_ADDR,			{ .type = ENC_SIMPLE }},
	{"alice@strongswan",		ID_RFC822_ADDR,			{ .type = ENC_SIMPLE }},
	{"alice@",					ID_RFC822_ADDR,			{ .type = ENC_SIMPLE }},
	{"alice",					ID_FQDN,				{ .type = ENC_SIMPLE }},
	{"@",						ID_FQDN,				{ .type = ENC_CHUNK }},
	{" @",						ID_RFC822_ADDR,			{ .type = ENC_SIMPLE }},
	{"@strongswan.org",			ID_FQDN,				{ .type = ENC_STRING,
		.data.s = "strongswan.org" }},
	{"@#deadbeef",				ID_KEY_ID,				{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xde,0xad,0xbe,0xef) }},
	{"@#deadbee",				ID_KEY_ID,				{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0x0d,0xea,0xdb,0xee) }},
	{"foo=bar",					ID_KEY_ID,				{ .type = ENC_SIMPLE }},
	{"foo=",					ID_KEY_ID,				{ .type = ENC_SIMPLE }},
	{"=bar",					ID_KEY_ID,				{ .type = ENC_SIMPLE }},
	{"C=",						ID_DER_ASN1_DN,			{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0x30,0x0b,0x31,0x09,0x30,0x07,0x06,
								   0x03,0x55,0x04,0x06,0x13,0x00) }},
	{"C=CH",					ID_DER_ASN1_DN,			{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0x30,0x0d,0x31,0x0b,0x30,0x09,0x06,
								   0x03,0x55,0x04,0x06,0x13,0x02,0x43,0x48) }},
	{"C=CH,",					ID_DER_ASN1_DN,			{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0x30,0x0d,0x31,0x0b,0x30,0x09,0x06,
								   0x03,0x55,0x04,0x06,0x13,0x02,0x43,0x48) }},
	{"C=CH, ",					ID_DER_ASN1_DN,			{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0x30,0x0d,0x31,0x0b,0x30,0x09,0x06,
								   0x03,0x55,0x04,0x06,0x13,0x02,0x43,0x48) }},
	{"C=CH, O",					ID_KEY_ID,				{ .type = ENC_SIMPLE }},
	{"IPv4:#c0a80101",			ID_IPV4_ADDR,			{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xc0,0xa8,0x01,0x01) }},
	{ "email:tester",			ID_RFC822_ADDR,			{ .type = ENC_STRING,
		.data.s = "tester" }},
	{"xmppaddr:bob@strongswan.org",	ID_DER_ASN1_GN,		{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xa0,0x20,0x06,0x08,0x2b,0x06,0x01,0x05,
								   0x05,0x07,0x08,0x05,0xa0,0x14,0x0c,0x12,
								   0x62,0x6f,0x62,0x40,0x73,0x74,0x72,0x6f,
								   0x6e,0x67,0x73,0x77,0x61,0x6e,0x2e,0x6f,
								   0x72,0x67) }},
	{ "{1}:#c0a80101",			ID_IPV4_ADDR,			{ .type = ENC_CHUNK,
		.data.c = chunk_from_chars(0xc0,0xa8,0x01,0x01) }},
	{ "{0x02}:tester",			ID_FQDN,				{ .type = ENC_STRING,
		.data.s = "tester" }},
	{ "{99}:somedata",			99,						{ .type = ENC_STRING,
		.data.s = "somedata" }},
};

START_TEST(test_from_string)
{
	identification_t *a;
	chunk_t encoding, expected = chunk_empty;
	char *id;

	id = string_data[_i].id;
	a = identification_create_from_string(id);
	fail_unless(a->get_type(a) == string_data[_i].type,
				"type of id '%s' is %N, %N expected", id,
				id_type_names, a->get_type(a),
				id_type_names, string_data[_i].type);

	encoding = a->get_encoding(a);
	switch (string_data[_i].result.type)
	{
		case ENC_SIMPLE:
			expected = chunk_from_str(string_data[_i].id);
			break;
		case ENC_STRING:
			expected = chunk_from_str(string_data[_i].result.data.s);
			break;
		case ENC_CHUNK:
			expected = string_data[_i].result.data.c;
			break;
		default:
			fail("unexpected result type");
	}

	ck_assert(!id || (char*)encoding.ptr != id);
	if (expected.ptr)
	{
		fail_unless(chunk_equals(encoding, expected),
					"parsing '%s' failed\nencoding %B\nexpected %B\n",
					id, &encoding, &expected);
	}
	else
	{
		ck_assert(encoding.ptr == NULL);
		ck_assert(encoding.len == 0);
	}
	a->destroy(a);
}
END_TEST

/*******************************************************************************
 * printf_hook
 */

static void string_equals(char *a_str, char *b_str)
{
	identification_t *b;
	char buf[128];

	b = b_str ? identification_create_from_string(b_str) : NULL;
	snprintf(buf, sizeof(buf), "%Y", b);
	DESTROY_IF(b);
	ck_assert_str_eq(a_str, buf);
}

static void string_equals_id(char *a_str, identification_t *b)
{
	char buf[128];

	snprintf(buf, sizeof(buf), "%Y", b);
	DESTROY_IF(b);
	ck_assert_str_eq(a_str, buf);
}

START_TEST(test_printf_hook)
{
	string_equals("(null)", NULL);
	string_equals("%any", "");
	string_equals("%any", "%any");
	string_equals("%any", "*");

	string_equals("192.168.1.1", "192.168.1.1");
	string_equals_id("(invalid ID_IPV4_ADDR)",
			identification_create_from_encoding(ID_IPV4_ADDR, chunk_empty));
	string_equals("192.168.1.1/32", "192.168.1.1/32");
	string_equals("192.168.1.2/31", "192.168.1.2/31");
	string_equals("192.168.1.0/24", "192.168.1.0/24");
	string_equals("192.168.2.0/23", "192.168.2.0/23");
	string_equals("0.0.0.0/0", "0.0.0.0/0");
	string_equals_id("(invalid ID_IPV4_ADDR_SUBNET)",
			identification_create_from_encoding(ID_IPV4_ADDR_SUBNET, chunk_empty));
	string_equals("192.168.1.1-192.168.1.254", "192.168.1.1-192.168.1.254");
	string_equals("0.0.0.0-255.255.255.255", "0.0.0.0-255.255.255.255");
	string_equals_id("(invalid ID_IPV4_ADDR_RANGE)",
			identification_create_from_encoding(ID_IPV4_ADDR_RANGE, chunk_empty));
	string_equals("fec0::1", "fec0::1");
	string_equals("fec0::1", "fec0:0:0::1");
	string_equals_id("(invalid ID_IPV6_ADDR)",
			identification_create_from_encoding(ID_IPV6_ADDR, chunk_empty));
	string_equals("fec0::1/128", "fec0::1/128");
	string_equals("fec0::2/127", "fec0::2/127");
	string_equals("fec0::100/120", "fec0::100/120");
	string_equals("::/0", "::/0");
	string_equals_id("(invalid ID_IPV6_ADDR_SUBNET)",
			identification_create_from_encoding(ID_IPV6_ADDR_SUBNET, chunk_empty));
	string_equals("fec0::1-fec0::4fff", "fec0::1-fec0::4fff");
	string_equals_id("(invalid ID_IPV6_ADDR_RANGE)",
			identification_create_from_encoding(ID_IPV6_ADDR_RANGE, chunk_empty));
	string_equals_id("(unknown ID type: 255)",
			identification_create_from_encoding(255, chunk_empty));

	string_equals("moon@strongswan.org", "moon@strongswan.org");
	string_equals("MOON@STRONGSWAN.ORG", "MOON@STRONGSWAN.ORG");
	/* non-printable characters */
	string_equals_id("????@strongswan.org", identification_create_from_encoding(ID_RFC822_ADDR,
			chunk_from_chars(0xfa, 0xfb, 0xfc, 0xfd, 0x40, 0x73, 0x74, 0x72,
							 0x6f, 0x6e, 0x67, 0x73, 0x77, 0x61, 0x6e, 0x2e,
							 0x6f, 0x72, 0x67)));

	/* not a DN => ID_KEY_ID => no normalization */
	string_equals("C=CH, AsdF=asdf", "C=CH, AsdF=asdf");
	string_equals_id("moon@strongswan.org", identification_create_from_encoding(ID_KEY_ID,
			chunk_from_str("moon@strongswan.org")));
	/* non-printable characters */
	string_equals_id("de:ad:be:ef", identification_create_from_encoding(ID_KEY_ID,
			chunk_from_chars(0xde, 0xad, 0xbe, 0xef)));
	/* printable characters */
	string_equals_id("ABCDEFGHIJKLMNOPQRS",
		identification_create_from_encoding(ID_KEY_ID,
			chunk_from_chars(0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
							 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
							 0x51, 0x52, 0x53)));
	/* ABCDEFGHIJKLMNOPQRST is printable but has the length of a SHA1 hash */
	string_equals_id("41:42:43:44:45:46:47:48:49:4a:4b:4c:4d:4e:4f:50:51:52:53:54",
		identification_create_from_encoding(ID_KEY_ID,
			chunk_from_chars(0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
							 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
							 0x51, 0x52, 0x53, 0x54)));

	string_equals_id("", identification_create_from_encoding(ID_DER_ASN1_DN, chunk_empty));
	string_equals("C=", "C=");
	string_equals("C=", "C=,");
	string_equals("C=", "C=, ");
	string_equals("C=", "C= , ");
	string_equals("C=, O=strongSwan", "C=, O=strongSwan");
	string_equals("C=CH, O=", "C=CH, O=");
	string_equals("C=CH, O=strongSwan, CN=strongswan.org",
				  "C=CH, O=strongSwan, CN=strongswan.org");
	string_equals("CN=strongswan.org, O=strongSwan, C=CH",
				  "cn=strongswan.org, o=strongSwan, c=CH");
	string_equals("C=CH, O=strongSwan, CN=strongswan.org",
				  "C=CH,O=strongSwan,CN=strongswan.org");
	string_equals("C=CH, O=strongSwan, CN=strongswan.org",
				  "/C=CH/O=strongSwan/CN=strongswan.org");
	string_equals("CN=strongswan.org, O=strongSwan, C=CH",
				  "CN=strongswan.org,O=strongSwan,C=CH");

	string_equals("C=CH, E=moon@strongswan.org, CN=moon",
				  "C=CH, email=moon@strongswan.org, CN=moon");
	string_equals("C=CH, E=moon@strongswan.org, CN=moon",
				  "C=CH, emailAddress=moon@strongswan.org, CN=moon");

	/* C=CH, telexNumber=123 (telexNumber is currently not recognized) */
	string_equals_id("C=CH, 55:04:15=123", identification_create_from_encoding(ID_DER_ASN1_DN,
		chunk_from_chars(0x30, 0x19, 0x31, 0x17, 0x30, 0x09, 0x06, 0x03, 0x55,
						 0x04, 0x06, 0x13, 0x02, 0x43, 0x48, 0x30, 0x0a, 0x06,
						 0x03, 0x55, 0x04, 0x15, 0x13, 0x03, 0x31, 0x32, 0x33)));
	/* C=CH, O=strongSwan (but instead of a 2nd OID -0x06- we got NULL -0x05) */
	string_equals_id("C=CH, (invalid ID_DER_ASN1_DN)", identification_create_from_encoding(ID_DER_ASN1_DN,
		chunk_from_chars(0x30, 0x20, 0x31, 0x1e, 0x30, 0x09, 0x06, 0x03, 0x55,
						 0x04, 0x06, 0x13, 0x02, 0x43, 0x48, 0x30, 0x11, 0x05,
						 0x03, 0x55, 0x04, 0x0a, 0x13, 0x0a, 0x73, 0x74, 0x72,
						 0x6f, 0x6e, 0x67, 0x53, 0x77, 0x61, 0x6e)));
	/* moon@strongswan.org as GN */
	string_equals_id("(ASN.1 general name)", identification_create_from_encoding(ID_DER_ASN1_GN,
		chunk_from_chars(0x81, 0x14, 0x6d, 0x6f, 0x6f, 0x6e, 0x40, 0x73, 0x74,
						 0x72, 0x6f, 0x6e, 0x67, 0x73, 0x77, 0x61, 0x6e, 0x2e,
						 0x6f, 0x72, 0x67)));
}
END_TEST

START_TEST(test_printf_hook_width)
{
	identification_t *a;
	char buf[128];

	a = identification_create_from_string("moon@strongswan.org");
	snprintf(buf, sizeof(buf), "%25Y", a);
	ck_assert_str_eq("      moon@strongswan.org", buf);
	snprintf(buf, sizeof(buf), "%-*Y", 25, a);
	ck_assert_str_eq("moon@strongswan.org      ", buf);
	snprintf(buf, sizeof(buf), "%5Y", a);
	ck_assert_str_eq("moon@strongswan.org", buf);
	DESTROY_IF(a);
}
END_TEST

/*******************************************************************************
 * equals
 */

static bool id_equals(identification_t *a, char *b_str)
{
	identification_t *b;
	bool equals;

	b = identification_create_from_string(b_str);
	equals = a->equals(a, b);
	ck_assert_int_eq(equals, b->equals(b, a));
	b->destroy(b);
	return equals;
}

START_TEST(test_equals)
{
	identification_t *a;
	chunk_t encoding, fuzzed;
	int i;

	/* this test also tests identification_create_from_string with DNs */
	a = identification_create_from_string(
							 "C=CH, E=moon@strongswan.org, CN=moon");

	ck_assert(id_equals(a, "C=CH, E=moon@strongswan.org, CN=moon"));
	ck_assert(id_equals(a, "C==CH , E==moon@strongswan.org , CN==moon"));
	ck_assert(id_equals(a, "  C=CH, E=moon@strongswan.org, CN=moon  "));
	ck_assert(id_equals(a, "C=ch, E=moon@STRONGSWAN.ORG, CN=Moon"));
	ck_assert(id_equals(a, "/C=CH/E=moon@strongswan.org/CN=moon"));
	ck_assert(id_equals(a, " / C=CH / E=moon@strongswan.org / CN=moon"));

	ck_assert(!id_equals(a, "C=CH/E=moon@strongswan.org/CN=moon"));
	ck_assert(!id_equals(a, "C=CH/E=moon@strongswan.org,CN=moon"));
	ck_assert(!id_equals(a, "C=CH E=moon@strongswan.org CN=moon"));
	ck_assert(!id_equals(a, "C=CN, E=moon@strongswan.org, CN=moon"));
	ck_assert(!id_equals(a, "E=moon@strongswan.org, C=CH, CN=moon"));
	ck_assert(!id_equals(a, "E=moon@strongswan.org, C=CH, CN=moon"));

	encoding = chunk_clone(a->get_encoding(a));
	a->destroy(a);

	/* simple fuzzing, increment each byte of encoding */
	for (i = 0; i < encoding.len; i++)
	{
		if (i == 11 || i == 30 || i == 60)
		{	/* skip ASN.1 type fields, as equals() handles them graceful */
			continue;
		}
		fuzzed = chunk_clone(encoding);
		fuzzed.ptr[i]++;
		a = identification_create_from_encoding(ID_DER_ASN1_DN, fuzzed);
		if (id_equals(a, "C=CH, E=moon@strongswan.org, CN=moon"))
		{
			printf("%d %B\n%B\n", i, &fuzzed, &encoding);
		}
		ck_assert(!id_equals(a, "C=CH, E=moon@strongswan.org, CN=moon"));
		a->destroy(a);
		free(fuzzed.ptr);
	}

	/* and decrement each byte of encoding */
	for (i = 0; i < encoding.len; i++)
	{
		if (i == 11 || i == 30 || i == 60)
		{
			continue;
		}
		fuzzed = chunk_clone(encoding);
		fuzzed.ptr[i]--;
		a = identification_create_from_encoding(ID_DER_ASN1_DN, fuzzed);
		ck_assert(!id_equals(a, "C=CH, E=moon@strongswan.org, CN=moon"));
		a->destroy(a);
		free(fuzzed.ptr);
	}
	free(encoding.ptr);
}
END_TEST

START_TEST(test_equals_any)
{
	identification_t *a, *b;

	a = identification_create_from_string("%any");
	b = identification_create_from_encoding(ID_ANY, chunk_empty);
	ck_assert(a->equals(a, b));
	ck_assert(b->equals(b, a));
	b->destroy(b);

	b = identification_create_from_string("C=CH, O=strongSwan, CN=strongswan.org");
	ck_assert(!a->equals(a, b));
	ck_assert(!b->equals(b, a));
	a->destroy(a);
	b->destroy(b);
}
END_TEST

START_TEST(test_equals_binary)
{
	identification_t *a, *b;
	chunk_t encoding;

	encoding = chunk_from_str("foobar=");
	/* strings containing = are parsed as KEY_ID if they aren't valid ASN.1 DNs */
	a = identification_create_from_string("foobar=");
	ck_assert(a->get_type(a) == ID_KEY_ID);
	b = identification_create_from_encoding(ID_KEY_ID, encoding);
	ck_assert(a->equals(a, b));
	a->destroy(a);
	b->destroy(b);
}
END_TEST

START_TEST(test_equals_fqdn)
{
	identification_t *a;

	a = identification_create_from_string("ipsec.strongswan.org");
	ck_assert(id_equals(a, "IPSEC.strongswan.org"));
	ck_assert(id_equals(a, "ipsec.strongSwan.org"));
	ck_assert(id_equals(a, "ipsec.strongSwan.ORG"));
	ck_assert(!id_equals(a, "strongswan.org"));
	a->destroy(a);
}
END_TEST

START_TEST(test_equals_empty)
{
	identification_t *a;

	a = identification_create_from_encoding(_i, chunk_empty);

	switch (_i)
	{
		case ID_ANY:
			ck_assert(id_equals(a, "%any"));
			break;
		case ID_IPV4_ADDR:
			ck_assert(!id_equals(a, "192.168.1.1"));
			break;
		case ID_FQDN:
			ck_assert(!id_equals(a, "moon.strongswan.org"));
			break;
		case ID_USER_FQDN:
			ck_assert(!id_equals(a, "moon@strongswan.org"));
			break;
		case ID_IPV6_ADDR:
			ck_assert(!id_equals(a, "fec0::1"));
			break;
		case ID_DER_ASN1_DN:
			ck_assert(!id_equals(a, "C=CH, E=moon@strongswan.org, CN=moon"));
			break;
		case ID_KEY_ID:
			ck_assert(!id_equals(a, "@#12345678"));
			break;
		case ID_DER_ASN1_GN:
		case ID_IPV4_ADDR_SUBNET:
		case ID_IPV6_ADDR_SUBNET:
		case ID_IPV4_ADDR_RANGE:
		case ID_IPV6_ADDR_RANGE:
			/* currently not tested */
			break;
	}

	a->destroy(a);
}
END_TEST

/*******************************************************************************
 * matches
 */

static bool id_matches(identification_t *a, char *b_str, id_match_t expected)
{
	identification_t *b;
	id_match_t match;

	b = identification_create_from_string(b_str);
	match = a->matches(a, b);
	b->destroy(b);
	return match == expected;
}

START_TEST(test_matches)
{
	identification_t *a;

	a = identification_create_from_string("C=CH, E=moon@strongswan.org, CN=moon");

	ck_assert(id_matches(a, "C=CH, E=moon@strongswan.org, CN=moon", ID_MATCH_PERFECT));
	ck_assert(id_matches(a, "C=CH, E=*@strongswan.org, CN=moon", ID_MATCH_NONE));
	ck_assert(id_matches(a, "C=CH, E=*, CN=moon", ID_MATCH_ONE_WILDCARD));
	ck_assert(id_matches(a, "C=CH, E=*, CN=*", ID_MATCH_ONE_WILDCARD - 1));
	ck_assert(id_matches(a, "C=*, E=*, CN=*", ID_MATCH_ONE_WILDCARD - 2));
	ck_assert(id_matches(a, "C=*, E=*, CN=*, O=BADInc", ID_MATCH_NONE));
	ck_assert(id_matches(a, "C=*, E=*", ID_MATCH_NONE));
	ck_assert(id_matches(a, "C=*, E=a@b.c, CN=*", ID_MATCH_NONE));
	ck_assert(id_matches(a, "%any", ID_MATCH_ANY));

	a->destroy(a);
}
END_TEST

START_TEST(test_matches_any)
{
	identification_t *a;

	a = identification_create_from_string("%any");

	ck_assert(id_matches(a, "%any", ID_MATCH_ANY));
	ck_assert(id_matches(a, "", ID_MATCH_ANY));
	ck_assert(id_matches(a, "*", ID_MATCH_ANY));
	ck_assert(id_matches(a, "moon@strongswan.org", ID_MATCH_NONE));
	ck_assert(id_matches(a, "vpn.strongswan.org", ID_MATCH_NONE));
	a->destroy(a);
}
END_TEST

START_TEST(test_matches_binary)
{
	identification_t *a;

	/* strings containing = are parsed as KEY_ID if they aren't valid ASN.1 DNs */
	a = identification_create_from_string("foo=bar");
	ck_assert(a->get_type(a) == ID_KEY_ID);
	ck_assert(id_matches(a, "%any", ID_MATCH_ANY));
	ck_assert(id_matches(a, "foo=bar", ID_MATCH_PERFECT));
	ck_assert(id_matches(a, "bar=foo", ID_MATCH_NONE));
	ck_assert(id_matches(a, "*=bar", ID_MATCH_NONE));
	ck_assert(id_matches(a, "foo=*", ID_MATCH_NONE));
	ck_assert(id_matches(a, "foo@bar", ID_MATCH_NONE));
	a->destroy(a);
}
END_TEST

START_TEST(test_matches_range)
{
	identification_t *a, *b;

	/* IPv4 addresses */
	a = identification_create_from_string("192.168.1.1");
	ck_assert(a->get_type(a) == ID_IPV4_ADDR);
	ck_assert(id_matches(a, "%any", ID_MATCH_ANY));
	ck_assert(id_matches(a, "0.0.0.0/0", ID_MATCH_MAX_WILDCARDS));
	ck_assert(id_matches(a, "192.168.1.1", ID_MATCH_PERFECT));
	ck_assert(id_matches(a, "192.168.1.2", ID_MATCH_NONE));
	ck_assert(id_matches(a, "192.168.1.1/32", ID_MATCH_PERFECT));
	ck_assert(id_matches(a, "192.168.1.0/32", ID_MATCH_NONE));
	ck_assert(id_matches(a, "192.168.1.0/24", ID_MATCH_ONE_WILDCARD));
	ck_assert(id_matches(a, "192.168.0.0/24", ID_MATCH_NONE));
	ck_assert(id_matches(a, "192.168.1.1-192.168.1.1", ID_MATCH_PERFECT));
	ck_assert(id_matches(a, "192.168.1.0-192.168.1.64", ID_MATCH_ONE_WILDCARD));
	ck_assert(id_matches(a, "192.168.1.2-192.168.1.64", ID_MATCH_NONE));
	ck_assert(id_matches(a, "192.168.0.240-192.168.1.0", ID_MATCH_NONE));
	ck_assert(id_matches(a, "foo@bar", ID_MATCH_NONE));

	/* Malformed IPv4 subnet and range encoding */
	b = identification_create_from_encoding(ID_IPV4_ADDR_SUBNET, chunk_empty);
	ck_assert(a->matches(a, b) == ID_MATCH_NONE);
	b->destroy(b);
	b = identification_create_from_encoding(ID_IPV4_ADDR_RANGE, chunk_empty);
	ck_assert(a->matches(a, b) == ID_MATCH_NONE);
	b->destroy(b);
	b = identification_create_from_encoding(ID_IPV4_ADDR_RANGE,
			chunk_from_chars(0xc0,0xa8,0x01,0x28,0xc0,0xa8,0x01,0x00));
	ck_assert(a->matches(a, b) == ID_MATCH_NONE);
	b->destroy(b);

	a->destroy(a);

	/* IPv6 addresses */
	a = identification_create_from_string("fec0::1");
	ck_assert(a->get_type(a) == ID_IPV6_ADDR);
	ck_assert(id_matches(a, "%any", ID_MATCH_ANY));
	ck_assert(id_matches(a, "::/0", ID_MATCH_MAX_WILDCARDS));
	ck_assert(id_matches(a, "fec0::1", ID_MATCH_PERFECT));
	ck_assert(id_matches(a, "fec0::2", ID_MATCH_NONE));
	ck_assert(id_matches(a, "fec0::1/128", ID_MATCH_PERFECT));
	ck_assert(id_matches(a, "fec0::/128", ID_MATCH_NONE));
	ck_assert(id_matches(a, "fec0::/120", ID_MATCH_ONE_WILDCARD));
	ck_assert(id_matches(a, "fec0::100/120", ID_MATCH_NONE));
	ck_assert(id_matches(a, "fec0::1-fec0::1", ID_MATCH_PERFECT));
	ck_assert(id_matches(a, "fec0::0-fec0::5", ID_MATCH_ONE_WILDCARD));
	ck_assert(id_matches(a, "fec0::4001-fec0::4ffe", ID_MATCH_NONE));
	ck_assert(id_matches(a, "feb0::1-fec0::0", ID_MATCH_NONE));
	ck_assert(id_matches(a, "foo@bar", ID_MATCH_NONE));

	/* Malformed IPv6 subnet and range encoding */
	b = identification_create_from_encoding(ID_IPV6_ADDR_SUBNET, chunk_empty);
	ck_assert(a->matches(a, b) == ID_MATCH_NONE);
	b->destroy(b);
	b = identification_create_from_encoding(ID_IPV6_ADDR_RANGE, chunk_empty);
	ck_assert(a->matches(a, b) == ID_MATCH_NONE);
	b->destroy(b);
	b = identification_create_from_encoding(ID_IPV6_ADDR_RANGE,
			chunk_from_chars(0xfe,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x4f,0xff,
							 0xfe,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01 ));
	ck_assert(a->matches(a, b) == ID_MATCH_NONE);
	b->destroy(b);

	a->destroy(a);

	/* Malformed IPv4 address encoding */
	a = identification_create_from_encoding(ID_IPV4_ADDR, chunk_empty);
	ck_assert(id_matches(a, "0.0.0.0/0", ID_MATCH_NONE));
	ck_assert(id_matches(a, "0.0.0.0-255.255.255.255", ID_MATCH_NONE));
	a->destroy(a);

	/* Malformed IPv6 address encoding */
	a = identification_create_from_encoding(ID_IPV6_ADDR, chunk_empty);
	ck_assert(id_matches(a, "::/0", ID_MATCH_NONE));
	ck_assert(id_matches(a, "::-ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", ID_MATCH_NONE));
	a->destroy(a);
}
END_TEST

START_TEST(test_matches_string)
{
	identification_t *a;

	a = identification_create_from_string("moon@strongswan.org");

	ck_assert(id_matches(a, "moon@strongswan.org", ID_MATCH_PERFECT));
	ck_assert(id_matches(a, "*@strongswan.org", ID_MATCH_ONE_WILDCARD));
	ck_assert(id_matches(a, "*@*.org", ID_MATCH_NONE));
	ck_assert(id_matches(a, "*@*", ID_MATCH_NONE));
	/* the following two are parsed as ID_FQDN, so no match */
	ck_assert(id_matches(a, "*strongswan.org", ID_MATCH_NONE));
	ck_assert(id_matches(a, "*.org", ID_MATCH_NONE));
	ck_assert(id_matches(a, "moon@*", ID_MATCH_NONE));
	ck_assert(id_matches(a, "**", ID_MATCH_NONE));
	ck_assert(id_matches(a, "*", ID_MATCH_ANY));
	ck_assert(id_matches(a, "%any", ID_MATCH_ANY));
	a->destroy(a);

	a = identification_create_from_string("vpn.strongswan.org");

	ck_assert(id_matches(a, "vpn.strongswan.org", ID_MATCH_PERFECT));
	ck_assert(id_matches(a, "*.strongswan.org", ID_MATCH_ONE_WILDCARD));
	ck_assert(id_matches(a, "*strongswan.org", ID_MATCH_ONE_WILDCARD));
	ck_assert(id_matches(a, "*.org", ID_MATCH_ONE_WILDCARD));
	ck_assert(id_matches(a, "*.strongswan.*", ID_MATCH_NONE));
	ck_assert(id_matches(a, "*vpn.strongswan.org", ID_MATCH_NONE));
	ck_assert(id_matches(a, "vpn.strongswan.*", ID_MATCH_NONE));
	ck_assert(id_matches(a, "**", ID_MATCH_NONE));
	ck_assert(id_matches(a, "*", ID_MATCH_ANY));
	ck_assert(id_matches(a, "%any", ID_MATCH_ANY));
	a->destroy(a);
}
END_TEST

START_TEST(test_matches_empty)
{
	identification_t *a;

	a = identification_create_from_encoding(_i, chunk_empty);

	switch (_i)
	{
		case ID_ANY:
			ck_assert(id_matches(a, "%any", ID_MATCH_ANY));
			break;
		case ID_IPV4_ADDR:
			ck_assert(id_matches(a, "192.168.1.1", ID_MATCH_NONE));
			break;
		case ID_FQDN:
			ck_assert(id_matches(a, "moon.strongswan.org", ID_MATCH_NONE));
			break;
		case ID_USER_FQDN:
			ck_assert(id_matches(a, "moon@strongswan.org", ID_MATCH_NONE));
			break;
		case ID_IPV6_ADDR:
			ck_assert(id_matches(a, "fec0::1", ID_MATCH_NONE));
			break;
		case ID_DER_ASN1_DN:
			ck_assert(id_matches(a, "C=CH, E=moon@strongswan.org, CN=moon",
								 ID_MATCH_NONE));
			break;
		case ID_KEY_ID:
			ck_assert(id_matches(a, "@#12345678", ID_MATCH_NONE));
			break;
		case ID_DER_ASN1_GN:
		case ID_IPV4_ADDR_SUBNET:
		case ID_IPV6_ADDR_SUBNET:
		case ID_IPV4_ADDR_RANGE:
		case ID_IPV6_ADDR_RANGE:
			/* currently not tested */
			break;
	}

	a->destroy(a);
}
END_TEST

static bool id_matches_rev(identification_t *a, char *b_str, id_match_t expected)
{
	identification_t *b;
	id_match_t match;

	b = identification_create_from_string(b_str);
	match = b->matches(b, a);
	b->destroy(b);
	return match == expected;
}

START_TEST(test_matches_empty_reverse)
{
	identification_t *a;

	a = identification_create_from_encoding(_i, chunk_empty);

	switch (_i)
	{
		case ID_ANY:
			ck_assert(id_matches_rev(a, "%any", ID_MATCH_ANY));
			break;
		case ID_IPV4_ADDR:
			ck_assert(id_matches_rev(a, "192.168.1.1", ID_MATCH_NONE));
			break;
		case ID_FQDN:
			ck_assert(id_matches_rev(a, "moon.strongswan.org", ID_MATCH_NONE));
			break;
		case ID_USER_FQDN:
			ck_assert(id_matches_rev(a, "moon@strongswan.org", ID_MATCH_NONE));
			break;
		case ID_IPV6_ADDR:
			ck_assert(id_matches_rev(a, "fec0::1", ID_MATCH_NONE));
			break;
		case ID_DER_ASN1_DN:
			ck_assert(id_matches_rev(a, "C=CH, E=moon@strongswan.org, CN=moon",
									 ID_MATCH_NONE));
			break;
		case ID_KEY_ID:
			ck_assert(id_matches_rev(a, "@#12345678", ID_MATCH_NONE));
			break;
		case ID_DER_ASN1_GN:
		case ID_IPV4_ADDR_SUBNET:
		case ID_IPV6_ADDR_SUBNET:
		case ID_IPV4_ADDR_RANGE:
		case ID_IPV6_ADDR_RANGE:
			/* currently not tested */
			break;
	}

	a->destroy(a);
}
END_TEST

/*******************************************************************************
 * identification hashing
 */

static bool id_hash_equals(char *str, char *b_str)
{
	identification_t *a, *b;
	bool success = FALSE;

	a = identification_create_from_string(str);
	b = identification_create_from_string(b_str ?: str);
	success = a->hash(a, 0) == b->hash(b, 0);
	a->destroy(a);
	b->destroy(b);
	return success;
}

START_TEST(test_hash)
{
	ck_assert(id_hash_equals("moon@strongswan.org", NULL));
	ck_assert(id_hash_equals("vpn.strongswan.org", NULL));
	ck_assert(id_hash_equals("192.168.1.1", NULL));
	ck_assert(id_hash_equals("C=CH", NULL));

	ck_assert(!id_hash_equals("moon@strongswan.org", "sun@strongswan.org"));
	ck_assert(!id_hash_equals("vpn.strongswan.org", "*.strongswan.org"));
	ck_assert(!id_hash_equals("192.168.1.1", "192.168.1.2"));
	ck_assert(!id_hash_equals("C=CH", "C=DE"));
	ck_assert(!id_hash_equals("fqdn:strongswan.org", "keyid:strongswan.org"));
}
END_TEST

START_TEST(test_hash_any)
{
	ck_assert(id_hash_equals("%any", NULL));
	ck_assert(id_hash_equals("%any", "0.0.0.0"));
	ck_assert(id_hash_equals("%any", "*"));
	ck_assert(id_hash_equals("%any", ""));

	ck_assert(!id_hash_equals("%any", "any"));
}
END_TEST

START_TEST(test_hash_dn)
{
	identification_t *a, *b;

	/* same DN (C=CH, O=strongSwan), different RDN type (PRINTABLESTRING vs.
	 * UTF8STRING) */
	a = identification_create_from_data(chunk_from_chars(
			0x30, 0x22, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03,
			0x55, 0x04, 0x06, 0x13, 0x02, 0x43, 0x48, 0x31,
			0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04, 0x0a,
			0x13, 0x0a, 0x73, 0x74, 0x72, 0x6f, 0x6e, 0x67,
			0x53, 0x77, 0x61, 0x6e));
	b = identification_create_from_data(chunk_from_chars(
			0x30, 0x22, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03,
			0x55, 0x04, 0x06, 0x0c, 0x02, 0x43, 0x48, 0x31,
			0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04, 0x0a,
			0x0c, 0x0a, 0x73, 0x74, 0x72, 0x6f, 0x6e, 0x67,
			0x53, 0x77, 0x61, 0x6e));
	ck_assert_int_eq(a->hash(a, 0), b->hash(b, 0));
	ck_assert(a->equals(a, b));
	a->destroy(a);
	b->destroy(b);
}
END_TEST

START_TEST(test_hash_inc)
{
	identification_t *a;

	a = identification_create_from_string("vpn.strongswan.org");
	ck_assert(a->hash(a, 0) != a->hash(a, 1));
	a->destroy(a);

	a = identification_create_from_string("C=CH, O=strongSwan");
	ck_assert(a->hash(a, 0) != a->hash(a, 1));
	a->destroy(a);
}
END_TEST

/*******************************************************************************
 * identification part enumeration
 */

START_TEST(test_parts)
{
	identification_t *id;
	enumerator_t *enumerator;
	id_part_t part;
	chunk_t data;
	int i = 0;

	id = identification_create_from_string("C=CH, O=strongSwan, CN=tester");

	enumerator = id->create_part_enumerator(id);
	while (enumerator->enumerate(enumerator, &part, &data))
	{
		switch (i++)
		{
			case 0:
				ck_assert(part == ID_PART_RDN_C &&
						  chunk_equals(data, chunk_create("CH", 2)));
				break;
			case 1:
				ck_assert(part == ID_PART_RDN_O &&
						  chunk_equals(data, chunk_from_str("strongSwan")));
				break;
			case 2:
				ck_assert(part == ID_PART_RDN_CN &&
						  chunk_equals(data, chunk_from_str("tester")));
				break;
			default:
				fail("unexpected identification part %d", part);
		}
	}
	ck_assert_int_eq(i, 3);
	enumerator->destroy(enumerator);
	id->destroy(id);
}
END_TEST

/*******************************************************************************
 * wildcards
 */

static bool id_contains_wildcards(char *string)
{
	identification_t *id;
	bool contains;

	id = identification_create_from_string(string);
	contains = id->contains_wildcards(id);
	id->destroy(id);
	return contains;
}

START_TEST(test_contains_wildcards)
{
	ck_assert(id_contains_wildcards("%any"));
	ck_assert(id_contains_wildcards("C=*, O=strongSwan, CN=gw"));
	ck_assert(id_contains_wildcards("C=CH, O=strongSwan, CN=*"));
	ck_assert(id_contains_wildcards("*@strongswan.org"));
	ck_assert(id_contains_wildcards("*.strongswan.org"));
	ck_assert(!id_contains_wildcards("C=**, O=a*, CN=*a"));
}
END_TEST

/*******************************************************************************
 * clone
 */

START_TEST(test_clone)
{
	identification_t *a, *b;
	chunk_t a_enc, b_enc;

	a = identification_create_from_string("moon@strongswan.org");
	a_enc = a->get_encoding(a);
	b = a->clone(a);
	ck_assert(b != NULL);
	ck_assert(a != b);
	b_enc = b->get_encoding(b);
	ck_assert(a_enc.ptr != b_enc.ptr);
	ck_assert(chunk_equals(a_enc, b_enc));
	a->destroy(a);
	b->destroy(b);
}
END_TEST

Suite *identification_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("identification");

	tc = tcase_create("create");
	tcase_add_test(tc, test_from_encoding);
	tcase_add_test(tc, test_from_data);
	tcase_add_test(tc, test_from_sockaddr);
	tcase_add_loop_test(tc, test_from_string, 0, countof(string_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("printf_hook");
	tcase_add_test(tc, test_printf_hook);
	tcase_add_test(tc, test_printf_hook_width);
	suite_add_tcase(s, tc);

	tc = tcase_create("equals");
	tcase_add_test(tc, test_equals);
	tcase_add_test(tc, test_equals_any);
	tcase_add_test(tc, test_equals_binary);
	tcase_add_test(tc, test_equals_fqdn);
	tcase_add_loop_test(tc, test_equals_empty, ID_ANY, ID_KEY_ID + 1);
	suite_add_tcase(s, tc);

	tc = tcase_create("matches");
	tcase_add_test(tc, test_matches);
	tcase_add_test(tc, test_matches_any);
	tcase_add_test(tc, test_matches_binary);
	tcase_add_test(tc, test_matches_range);
	tcase_add_test(tc, test_matches_string);
	tcase_add_loop_test(tc, test_matches_empty, ID_ANY, ID_KEY_ID + 1);
	tcase_add_loop_test(tc, test_matches_empty_reverse, ID_ANY, ID_KEY_ID + 1);
	suite_add_tcase(s, tc);

	tc = tcase_create("hash");
	tcase_add_test(tc, test_hash);
	tcase_add_test(tc, test_hash_any);
	tcase_add_test(tc, test_hash_dn);
	tcase_add_test(tc, test_hash_inc);
	suite_add_tcase(s, tc);

	tc = tcase_create("part enumeration");
	tcase_add_test(tc, test_parts);
	suite_add_tcase(s, tc);

	tc = tcase_create("wildcards");
	tcase_add_test(tc, test_contains_wildcards);
	suite_add_tcase(s, tc);

	tc = tcase_create("clone");
	tcase_add_test(tc, test_clone);
	suite_add_tcase(s, tc);

	return s;
}
