/*
 * OBEX Server
 *
 * Copyright (C) 2008-2010 Intel Corporation.  All rights reserved.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
