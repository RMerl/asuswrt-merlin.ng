/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * OBEX Server
 *
 * Copyright (C) 2008-2010 Intel Corporation.  All rights reserved.
 *
 */

enum phonebook_number_type {
	TEL_TYPE_HOME,
	TEL_TYPE_MOBILE,
	TEL_TYPE_FAX,
	TEL_TYPE_WORK,
	TEL_TYPE_OTHER,
};

enum phonebook_field_type {
	FIELD_TYPE_HOME,
	FIELD_TYPE_WORK,
	FIELD_TYPE_OTHER,
};

enum phonebook_call_type {
	CALL_TYPE_NOT_A_CALL,
	CALL_TYPE_MISSED,
	CALL_TYPE_INCOMING,
	CALL_TYPE_OUTGOING,
};

struct phonebook_field {
	char *text;
	int type;
};

struct phonebook_addr {
	GSList *fields;
	int type;
};

struct phonebook_contact {
	char *uid;
	char *fullname;
	char *given;
	char *family;
	char *additional;
	GSList *numbers;
	GSList *emails;
	char *prefix;
	char *suffix;
	GSList *addresses;
	char *birthday;
	char *nickname;
	GSList *urls;
	char *photo;
	char *company;
	char *department;
	char *role;
	char *title;
	char *datetime;
	int calltype;
};

void phonebook_add_contact(GString *vcards, struct phonebook_contact *contact,
					uint64_t filter, uint8_t format);

void phonebook_contact_free(struct phonebook_contact *contact);

void phonebook_addr_free(gpointer addr);
