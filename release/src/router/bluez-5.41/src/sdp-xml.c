/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2005-2010  Marcel Holtmann <marcel@holtmann.org>
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
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#include <glib.h>

#include "lib/sdp.h"
#include "lib/sdp_lib.h"

#include "sdp-xml.h"

#define DBG(...) (void)(0)
#define error(...) (void)(0)

#define SDP_XML_ENCODING_NORMAL	0
#define SDP_XML_ENCODING_HEX	1

#define STRBUFSIZE 1024
#define MAXINDENT 64

struct sdp_xml_data {
	char *text;			/* Pointer to the current buffer */
	int size;			/* Size of the current buffer */
	sdp_data_t *data;		/* The current item being built */
	struct sdp_xml_data *next;	/* Next item on the stack */
	char type;			/* 0 = Text or Hexadecimal */
	char *name;			/* Name, optional in the dtd */
	/* TODO: What is it used for? */
};

struct context_data {
	sdp_record_t *record;
	sdp_data_t attr_data;
	struct sdp_xml_data *stack_head;
	uint16_t attr_id;
};

static int compute_seq_size(sdp_data_t *data)
{
	int unit_size = data->unitSize;
	sdp_data_t *seq = data->val.dataseq;

	for (; seq; seq = seq->next)
		unit_size += seq->unitSize;

	return unit_size;
}

#define DEFAULT_XML_DATA_SIZE 1024

static struct sdp_xml_data *sdp_xml_data_alloc(void)
{
	struct sdp_xml_data *elem;

	elem = malloc(sizeof(struct sdp_xml_data));
	if (!elem)
		return NULL;

	memset(elem, 0, sizeof(struct sdp_xml_data));

	/* Null terminate the text */
	elem->size = DEFAULT_XML_DATA_SIZE;
	elem->text = malloc(DEFAULT_XML_DATA_SIZE);
	if (!elem->text) {
		free(elem);
		return NULL;
	}
	elem->text[0] = '\0';

	return elem;
}

static struct sdp_xml_data *sdp_xml_data_expand(struct sdp_xml_data *elem)
{
	char *newbuf;

	newbuf = malloc(elem->size * 2);
	if (!newbuf)
		return NULL;

	memcpy(newbuf, elem->text, elem->size);
	elem->size *= 2;
	free(elem->text);

	elem->text = newbuf;

	return elem;
}

static sdp_data_t *sdp_xml_parse_uuid128(const char *data)
{
	uint128_t val;
	unsigned int i, j;

	char buf[3];

	memset(&val, 0, sizeof(val));

	buf[2] = '\0';

	for (j = 0, i = 0; i < strlen(data);) {
		if (data[i] == '-') {
			i++;
			continue;
		}

		buf[0] = data[i];
		buf[1] = data[i + 1];

		val.data[j++] = strtoul(buf, 0, 16);
		i += 2;
	}

	return sdp_data_alloc(SDP_UUID128, &val);
}

static sdp_data_t *sdp_xml_parse_uuid(const char *data, sdp_record_t *record)
{
	sdp_data_t *ret;
	char *endptr;
	uint32_t val;
	uint16_t val2;
	int len;

	len = strlen(data);

	if (len == 36) {
		ret = sdp_xml_parse_uuid128(data);
		goto result;
	}

	val = strtoll(data, &endptr, 16);

	/* Couldn't parse */
	if (*endptr != '\0')
		return NULL;

	if (val > USHRT_MAX) {
		ret = sdp_data_alloc(SDP_UUID32, &val);
		goto result;
	}

	val2 = val;

	ret = sdp_data_alloc(SDP_UUID16, &val2);

result:
	if (record && ret)
		sdp_pattern_add_uuid(record, &ret->val.uuid);

	return ret;
}

static sdp_data_t *sdp_xml_parse_int(const char *data, uint8_t dtd)
{
	char *endptr;
	sdp_data_t *ret = NULL;

	switch (dtd) {
	case SDP_BOOL:
	{
		uint8_t val = 0;

		if (!strcmp("true", data))
			val = 1;
		else if (!strcmp("false", data))
			val = 0;
		else
			return NULL;

		ret = sdp_data_alloc(dtd, &val);
		break;
	}

	case SDP_INT8:
	{
		int8_t val = strtoul(data, &endptr, 0);

		/* Failed to parse */
		if ((endptr != data) && (*endptr != '\0'))
			return NULL;

		ret = sdp_data_alloc(dtd, &val);
		break;
	}

	case SDP_UINT8:
	{
		uint8_t val = strtoul(data, &endptr, 0);

		/* Failed to parse */
		if ((endptr != data) && (*endptr != '\0'))
			return NULL;

		ret = sdp_data_alloc(dtd, &val);
		break;
	}

	case SDP_INT16:
	{
		int16_t val = strtoul(data, &endptr, 0);

		/* Failed to parse */
		if ((endptr != data) && (*endptr != '\0'))
			return NULL;

		ret = sdp_data_alloc(dtd, &val);
		break;
	}

	case SDP_UINT16:
	{
		uint16_t val = strtoul(data, &endptr, 0);

		/* Failed to parse */
		if ((endptr != data) && (*endptr != '\0'))
			return NULL;

		ret = sdp_data_alloc(dtd, &val);
		break;
	}

	case SDP_INT32:
	{
		int32_t val = strtoul(data, &endptr, 0);

		/* Failed to parse */
		if ((endptr != data) && (*endptr != '\0'))
			return NULL;

		ret = sdp_data_alloc(dtd, &val);
		break;
	}

	case SDP_UINT32:
	{
		uint32_t val = strtoul(data, &endptr, 0);

		/* Failed to parse */
		if ((endptr != data) && (*endptr != '\0'))
			return NULL;

		ret = sdp_data_alloc(dtd, &val);
		break;
	}

	case SDP_INT64:
	{
		int64_t val = strtoull(data, &endptr, 0);

		/* Failed to parse */
		if ((endptr != data) && (*endptr != '\0'))
			return NULL;

		ret = sdp_data_alloc(dtd, &val);
		break;
	}

	case SDP_UINT64:
	{
		uint64_t val = strtoull(data, &endptr, 0);

		/* Failed to parse */
		if ((endptr != data) && (*endptr != '\0'))
			return NULL;

		ret = sdp_data_alloc(dtd, &val);
		break;
	}

	case SDP_INT128:
	case SDP_UINT128:
	{
		uint128_t val;
		int i = 0;
		char buf[3];

		buf[2] = '\0';

		for (; i < 32; i += 2) {
			buf[0] = data[i];
			buf[1] = data[i + 1];

			val.data[i >> 1] = strtoul(buf, 0, 16);
		}

		ret = sdp_data_alloc(dtd, &val);
		break;
	}

	};

	return ret;
}

static char *sdp_xml_parse_string_decode(const char *data, char encoding,
							uint32_t *length)
{
	int len = strlen(data);
	char *text;

	if (encoding == SDP_XML_ENCODING_NORMAL) {
		text = strdup(data);
		*length = len;
	} else {
		char buf[3], *decoded;
		int i;

		decoded = malloc((len >> 1) + 1);
		if (!decoded)
			return NULL;

		/* Ensure the string is a power of 2 */
		len = (len >> 1) << 1;

		buf[2] = '\0';

		for (i = 0; i < len; i += 2) {
			buf[0] = data[i];
			buf[1] = data[i + 1];

			decoded[i >> 1] = strtoul(buf, 0, 16);
		}

		decoded[len >> 1] = '\0';
		text = decoded;
		*length = len >> 1;
	}

	return text;
}

static sdp_data_t *sdp_xml_parse_url(const char *data)
{
	uint8_t dtd = SDP_URL_STR8;
	char *url;
	uint32_t length;
	sdp_data_t *ret;

	url = sdp_xml_parse_string_decode(data,
				SDP_XML_ENCODING_NORMAL, &length);
	if (!url)
		return NULL;

	if (length > UCHAR_MAX)
		dtd = SDP_URL_STR16;

	ret = sdp_data_alloc_with_length(dtd, url, length);

	free(url);

	return ret;
}

static sdp_data_t *sdp_xml_parse_text(const char *data, char encoding)
{
	uint8_t dtd = SDP_TEXT_STR8;
	char *text;
	uint32_t length;
	sdp_data_t *ret;

	text = sdp_xml_parse_string_decode(data, encoding, &length);
	if (!text)
		return NULL;

	if (length > UCHAR_MAX)
		dtd = SDP_TEXT_STR16;

	ret = sdp_data_alloc_with_length(dtd, text, length);

	free(text);

	return ret;
}

static sdp_data_t *sdp_xml_parse_nil(const char *data)
{
	return sdp_data_alloc(SDP_DATA_NIL, 0);
}

static sdp_data_t *sdp_xml_parse_datatype(const char *el,
						struct sdp_xml_data *elem,
						sdp_record_t *record)
{
	const char *data = elem->text;

	if (!strcmp(el, "boolean"))
		return sdp_xml_parse_int(data, SDP_BOOL);
	else if (!strcmp(el, "uint8"))
		return sdp_xml_parse_int(data, SDP_UINT8);
	else if (!strcmp(el, "uint16"))
		return sdp_xml_parse_int(data, SDP_UINT16);
	else if (!strcmp(el, "uint32"))
		return sdp_xml_parse_int(data, SDP_UINT32);
	else if (!strcmp(el, "uint64"))
		return sdp_xml_parse_int(data, SDP_UINT64);
	else if (!strcmp(el, "uint128"))
		return sdp_xml_parse_int(data, SDP_UINT128);
	else if (!strcmp(el, "int8"))
		return sdp_xml_parse_int(data, SDP_INT8);
	else if (!strcmp(el, "int16"))
		return sdp_xml_parse_int(data, SDP_INT16);
	else if (!strcmp(el, "int32"))
		return sdp_xml_parse_int(data, SDP_INT32);
	else if (!strcmp(el, "int64"))
		return sdp_xml_parse_int(data, SDP_INT64);
	else if (!strcmp(el, "int128"))
		return sdp_xml_parse_int(data, SDP_INT128);
	else if (!strcmp(el, "uuid"))
		return sdp_xml_parse_uuid(data, record);
	else if (!strcmp(el, "url"))
		return sdp_xml_parse_url(data);
	else if (!strcmp(el, "text"))
		return sdp_xml_parse_text(data, elem->type);
	else if (!strcmp(el, "nil"))
		return sdp_xml_parse_nil(data);

	return NULL;
}
static void element_start(GMarkupParseContext *context,
		const char *element_name, const char **attribute_names,
		const char **attribute_values, gpointer user_data, GError **err)
{
	struct context_data *ctx_data = user_data;

	if (!strcmp(element_name, "record"))
		return;

	if (!strcmp(element_name, "attribute")) {
		int i;
		for (i = 0; attribute_names[i]; i++) {
			if (!strcmp(attribute_names[i], "id")) {
				ctx_data->attr_id = strtol(attribute_values[i], 0, 0);
				break;
			}
		}
		DBG("New attribute 0x%04x", ctx_data->attr_id);
		return;
	}

	if (ctx_data->stack_head) {
		struct sdp_xml_data *newelem = sdp_xml_data_alloc();
		newelem->next = ctx_data->stack_head;
		ctx_data->stack_head = newelem;
	} else {
		ctx_data->stack_head = sdp_xml_data_alloc();
		ctx_data->stack_head->next = NULL;
	}

	if (!strcmp(element_name, "sequence"))
		ctx_data->stack_head->data = sdp_data_alloc(SDP_SEQ8, NULL);
	else if (!strcmp(element_name, "alternate"))
		ctx_data->stack_head->data = sdp_data_alloc(SDP_ALT8, NULL);
	else {
		int i;
		/* Parse value, name, encoding */
		for (i = 0; attribute_names[i]; i++) {
			if (!strcmp(attribute_names[i], "value")) {
				int curlen = strlen(ctx_data->stack_head->text);
				int attrlen = strlen(attribute_values[i]);

				/* Ensure we're big enough */
				while ((curlen + 1 + attrlen) > ctx_data->stack_head->size)
					sdp_xml_data_expand(ctx_data->stack_head);

				memcpy(ctx_data->stack_head->text + curlen,
						attribute_values[i], attrlen);
				ctx_data->stack_head->text[curlen + attrlen] = '\0';
			}

			if (!strcmp(attribute_names[i], "encoding")) {
				if (!strcmp(attribute_values[i], "hex"))
					ctx_data->stack_head->type = 1;
			}

			if (!strcmp(attribute_names[i], "name"))
				ctx_data->stack_head->name = strdup(attribute_values[i]);
		}

		ctx_data->stack_head->data = sdp_xml_parse_datatype(element_name,
				ctx_data->stack_head, ctx_data->record);

		if (ctx_data->stack_head->data == NULL)
			error("Can't parse element %s", element_name);
	}
}

static void sdp_xml_data_free(struct sdp_xml_data *elem)
{
	if (elem->data)
		sdp_data_free(elem->data);

	free(elem->name);
	free(elem->text);
	free(elem);
}

static void element_end(GMarkupParseContext *context,
		const char *element_name, gpointer user_data, GError **err)
{
	struct context_data *ctx_data = user_data;
	struct sdp_xml_data *elem;

	if (!strcmp(element_name, "record"))
		return;

	if (!strcmp(element_name, "attribute")) {
		if (ctx_data->stack_head && ctx_data->stack_head->data) {
			int ret = sdp_attr_add(ctx_data->record, ctx_data->attr_id,
							ctx_data->stack_head->data);
			if (ret == -1)
				DBG("Could not add attribute 0x%04x",
							ctx_data->attr_id);

			ctx_data->stack_head->data = NULL;
			sdp_xml_data_free(ctx_data->stack_head);
			ctx_data->stack_head = NULL;
		} else {
			DBG("No data for attribute 0x%04x", ctx_data->attr_id);
		}
		return;
	}

	if (!ctx_data->stack_head || !ctx_data->stack_head->data) {
		DBG("No data for %s", element_name);
		return;
	}

	if (!strcmp(element_name, "sequence")) {
		ctx_data->stack_head->data->unitSize = compute_seq_size(ctx_data->stack_head->data);

		if (ctx_data->stack_head->data->unitSize > USHRT_MAX) {
			ctx_data->stack_head->data->unitSize += sizeof(uint32_t);
			ctx_data->stack_head->data->dtd = SDP_SEQ32;
		} else if (ctx_data->stack_head->data->unitSize > UCHAR_MAX) {
			ctx_data->stack_head->data->unitSize += sizeof(uint16_t);
			ctx_data->stack_head->data->dtd = SDP_SEQ16;
		} else {
			ctx_data->stack_head->data->unitSize += sizeof(uint8_t);
		}
	} else if (!strcmp(element_name, "alternate")) {
		ctx_data->stack_head->data->unitSize = compute_seq_size(ctx_data->stack_head->data);

		if (ctx_data->stack_head->data->unitSize > USHRT_MAX) {
			ctx_data->stack_head->data->unitSize += sizeof(uint32_t);
			ctx_data->stack_head->data->dtd = SDP_ALT32;
		} else if (ctx_data->stack_head->data->unitSize > UCHAR_MAX) {
			ctx_data->stack_head->data->unitSize += sizeof(uint16_t);
			ctx_data->stack_head->data->dtd = SDP_ALT16;
		} else {
			ctx_data->stack_head->data->unitSize += sizeof(uint8_t);
		}
	}

	if (ctx_data->stack_head->next && ctx_data->stack_head->data &&
					ctx_data->stack_head->next->data) {
		switch (ctx_data->stack_head->next->data->dtd) {
		case SDP_SEQ8:
		case SDP_SEQ16:
		case SDP_SEQ32:
		case SDP_ALT8:
		case SDP_ALT16:
		case SDP_ALT32:
			ctx_data->stack_head->next->data->val.dataseq =
				sdp_seq_append(ctx_data->stack_head->next->data->val.dataseq,
								ctx_data->stack_head->data);
			ctx_data->stack_head->data = NULL;
			break;
		}

		elem = ctx_data->stack_head;
		ctx_data->stack_head = ctx_data->stack_head->next;

		sdp_xml_data_free(elem);
	}
}

static GMarkupParser parser = {
	element_start, element_end, NULL, NULL, NULL
};

sdp_record_t *sdp_xml_parse_record(const char *data, int size)
{
	GMarkupParseContext *ctx;
	struct context_data *ctx_data;
	sdp_record_t *record;

	ctx_data = malloc(sizeof(*ctx_data));
	if (!ctx_data)
		return NULL;

	record = sdp_record_alloc();
	if (!record) {
		free(ctx_data);
		return NULL;
	}

	memset(ctx_data, 0, sizeof(*ctx_data));
	ctx_data->record = record;

	ctx = g_markup_parse_context_new(&parser, 0, ctx_data, NULL);

	if (g_markup_parse_context_parse(ctx, data, size, NULL) == FALSE) {
		error("XML parsing error");
		g_markup_parse_context_free(ctx);
		sdp_record_free(record);
		free(ctx_data);
		return NULL;
	}

	g_markup_parse_context_free(ctx);

	free(ctx_data);

	return record;
}


static void convert_raw_data_to_xml(sdp_data_t *value, int indent_level,
		void *data, void (*appender)(void *, const char *))
{
	int i, hex;
	char buf[STRBUFSIZE];
	char indent[MAXINDENT];

	if (!value)
		return;

	if (indent_level >= MAXINDENT)
		indent_level = MAXINDENT - 2;

	for (i = 0; i < indent_level; i++)
		indent[i] = '\t';

	indent[i] = '\0';
	buf[STRBUFSIZE - 1] = '\0';

	switch (value->dtd) {
	case SDP_DATA_NIL:
		appender(data, indent);
		appender(data, "<nil/>\n");
		break;

	case SDP_BOOL:
		appender(data, indent);
		appender(data, "<boolean value=\"");
		appender(data, value->val.uint8 ? "true" : "false");
		appender(data, "\" />\n");
		break;

	case SDP_UINT8:
		appender(data, indent);
		appender(data, "<uint8 value=\"");
		snprintf(buf, STRBUFSIZE - 1, "0x%02x", value->val.uint8);
		appender(data, buf);
		appender(data, "\" />\n");
		break;

	case SDP_UINT16:
		appender(data, indent);
		appender(data, "<uint16 value=\"");
		snprintf(buf, STRBUFSIZE - 1, "0x%04x", value->val.uint16);
		appender(data, buf);
		appender(data, "\" />\n");
		break;

	case SDP_UINT32:
		appender(data, indent);
		appender(data, "<uint32 value=\"");
		snprintf(buf, STRBUFSIZE - 1, "0x%08x", value->val.uint32);
		appender(data, buf);
		appender(data, "\" />\n");
		break;

	case SDP_UINT64:
		appender(data, indent);
		appender(data, "<uint64 value=\"");
		snprintf(buf, STRBUFSIZE - 1, "0x%016jx", value->val.uint64);
		appender(data, buf);
		appender(data, "\" />\n");
		break;

	case SDP_UINT128:
		appender(data, indent);
		appender(data, "<uint128 value=\"");

		for (i = 0; i < 16; i++) {
			sprintf(&buf[i * 2], "%02x",
				(unsigned char) value->val.uint128.data[i]);
		}

		appender(data, buf);
		appender(data, "\" />\n");
		break;

	case SDP_INT8:
		appender(data, indent);
		appender(data, "<int8 value=\"");
		snprintf(buf, STRBUFSIZE - 1, "%d", value->val.int8);
		appender(data, buf);
		appender(data, "\" />\n");
		break;

	case SDP_INT16:
		appender(data, indent);
		appender(data, "<int16 value=\"");
		snprintf(buf, STRBUFSIZE - 1, "%d", value->val.int16);
		appender(data, buf);
		appender(data, "\" />\n");
		break;

	case SDP_INT32:
		appender(data, indent);
		appender(data, "<int32 value=\"");
		snprintf(buf, STRBUFSIZE - 1, "%d", value->val.int32);
		appender(data, buf);
		appender(data, "\" />\n");
		break;

	case SDP_INT64:
		appender(data, indent);
		appender(data, "<int64 value=\"");
		snprintf(buf, STRBUFSIZE - 1, "%jd", value->val.int64);
		appender(data, buf);
		appender(data, "\" />\n");
		break;

	case SDP_INT128:
		appender(data, indent);
		appender(data, "<int128 value=\"");

		for (i = 0; i < 16; i++) {
			sprintf(&buf[i * 2], "%02x",
				(unsigned char) value->val.int128.data[i]);
		}
		appender(data, buf);

		appender(data, "\" />\n");
		break;

	case SDP_UUID16:
		appender(data, indent);
		appender(data, "<uuid value=\"");
		snprintf(buf, STRBUFSIZE - 1, "0x%04x", value->val.uuid.value.uuid16);
		appender(data, buf);
		appender(data, "\" />\n");
		break;

	case SDP_UUID32:
		appender(data, indent);
		appender(data, "<uuid value=\"");
		snprintf(buf, STRBUFSIZE - 1, "0x%08x", value->val.uuid.value.uuid32);
		appender(data, buf);
		appender(data, "\" />\n");
		break;

	case SDP_UUID128:
		appender(data, indent);
		appender(data, "<uuid value=\"");

		snprintf(buf, STRBUFSIZE - 1,
			 "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			 (unsigned char) value->val.uuid.value.
			 uuid128.data[0],
			 (unsigned char) value->val.uuid.value.
			 uuid128.data[1],
			 (unsigned char) value->val.uuid.value.
			 uuid128.data[2],
			 (unsigned char) value->val.uuid.value.
			 uuid128.data[3],
			 (unsigned char) value->val.uuid.value.
			 uuid128.data[4],
			 (unsigned char) value->val.uuid.value.
			 uuid128.data[5],
			 (unsigned char) value->val.uuid.value.
			 uuid128.data[6],
			 (unsigned char) value->val.uuid.value.
			 uuid128.data[7],
			 (unsigned char) value->val.uuid.value.
			 uuid128.data[8],
			 (unsigned char) value->val.uuid.value.
			 uuid128.data[9],
			 (unsigned char) value->val.uuid.value.
			 uuid128.data[10],
			 (unsigned char) value->val.uuid.value.
			 uuid128.data[11],
			 (unsigned char) value->val.uuid.value.
			 uuid128.data[12],
			 (unsigned char) value->val.uuid.value.
			 uuid128.data[13],
			 (unsigned char) value->val.uuid.value.
			 uuid128.data[14],
			 (unsigned char) value->val.uuid.value.
			 uuid128.data[15]);

		appender(data, buf);
		appender(data, "\" />\n");
		break;

	case SDP_TEXT_STR8:
	case SDP_TEXT_STR16:
	case SDP_TEXT_STR32:
	{
		int num_chars_to_escape = 0;
		int length = value->unitSize - 1;
		char *strBuf;

		hex = 0;

		for (i = 0; i < length; i++) {
			if (!isprint(value->val.str[i]) &&
					value->val.str[i] != '\0') {
				hex = 1;
				break;
			}

			/* XML is evil, must do this... */
			if ((value->val.str[i] == '<') ||
					(value->val.str[i] == '>') ||
					(value->val.str[i] == '"') ||
					(value->val.str[i] == '&'))
				num_chars_to_escape++;
		}

		appender(data, indent);

		appender(data, "<text ");

		if (hex) {
			appender(data, "encoding=\"hex\" ");
			strBuf = malloc(sizeof(char)
						 * ((value->unitSize-1) * 2 + 1));
			if (!strBuf) {
				DBG("No memory to convert raw data to xml");
				return;
			}

			/* Unit Size seems to include the size for dtd
			   It is thus off by 1
			   This is safe for Normal strings, but not
			   hex encoded data */
			for (i = 0; i < (value->unitSize-1); i++)
				sprintf(&strBuf[i*sizeof(char)*2],
					"%02x",
					(unsigned char) value->val.str[i]);

			strBuf[(value->unitSize-1) * 2] = '\0';
		} else {
			int j;
			/* escape the XML disallowed chars */
			strBuf = malloc(sizeof(char) *
					(value->unitSize + 1 + num_chars_to_escape * 4));
			if (!strBuf) {
				DBG("No memory to convert raw data to xml");
				return;
			}
			for (i = 0, j = 0; i < length; i++) {
				if (value->val.str[i] == '&') {
					strBuf[j++] = '&';
					strBuf[j++] = 'a';
					strBuf[j++] = 'm';
					strBuf[j++] = 'p';
				} else if (value->val.str[i] == '<') {
					strBuf[j++] = '&';
					strBuf[j++] = 'l';
					strBuf[j++] = 't';
				} else if (value->val.str[i] == '>') {
					strBuf[j++] = '&';
					strBuf[j++] = 'g';
					strBuf[j++] = 't';
				} else if (value->val.str[i] == '"') {
					strBuf[j++] = '&';
					strBuf[j++] = 'q';
					strBuf[j++] = 'u';
					strBuf[j++] = 'o';
					strBuf[j++] = 't';
				} else if (value->val.str[i] == '\0') {
					strBuf[j++] = ' ';
				} else {
					strBuf[j++] = value->val.str[i];
				}
			}

			strBuf[j] = '\0';
		}

		appender(data, "value=\"");
		appender(data, strBuf);
		appender(data, "\" />\n");
		free(strBuf);
		break;
	}

	case SDP_URL_STR8:
	case SDP_URL_STR16:
	case SDP_URL_STR32:
	{
		char *strBuf;

		appender(data, indent);
		appender(data, "<url value=\"");
		strBuf = strndup(value->val.str, value->unitSize - 1);
		appender(data, strBuf);
		free(strBuf);
		appender(data, "\" />\n");
		break;
	}

	case SDP_SEQ8:
	case SDP_SEQ16:
	case SDP_SEQ32:
		appender(data, indent);
		appender(data, "<sequence>\n");

		convert_raw_data_to_xml(value->val.dataseq,
					indent_level + 1, data, appender);

		appender(data, indent);
		appender(data, "</sequence>\n");

		break;

	case SDP_ALT8:
	case SDP_ALT16:
	case SDP_ALT32:
		appender(data, indent);

		appender(data, "<alternate>\n");

		convert_raw_data_to_xml(value->val.dataseq,
					indent_level + 1, data, appender);
		appender(data, indent);

		appender(data, "</alternate>\n");

		break;
	}

	convert_raw_data_to_xml(value->next, indent_level, data, appender);
}

struct conversion_data {
	void *data;
	void (*appender)(void *data, const char *);
};

static void convert_raw_attr_to_xml_func(void *val, void *data)
{
	struct conversion_data *cd = data;
	sdp_data_t *value = val;
	char buf[STRBUFSIZE];

	buf[STRBUFSIZE - 1] = '\0';
	snprintf(buf, STRBUFSIZE - 1, "\t<attribute id=\"0x%04x\">\n",
		 value->attrId);
	cd->appender(cd->data, buf);

	convert_raw_data_to_xml(value, 2, cd->data, cd->appender);

	cd->appender(cd->data, "\t</attribute>\n");
}

/*
 * Will convert the sdp record to XML.  The appender and data can be used
 * to control where to output the record (e.g. file or a data buffer).  The
 * appender will be called repeatedly with data and the character buffer
 * (containing parts of the generated XML) to append.
 */
void convert_sdp_record_to_xml(sdp_record_t *rec,
			void *data, void (*appender)(void *, const char *))
{
	struct conversion_data cd;

	cd.data = data;
	cd.appender = appender;

	if (rec && rec->attrlist) {
		appender(data, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\n");
		appender(data, "<record>\n");
		sdp_list_foreach(rec->attrlist,
				 convert_raw_attr_to_xml_func, &cd);
		appender(data, "</record>\n");
	}
}
