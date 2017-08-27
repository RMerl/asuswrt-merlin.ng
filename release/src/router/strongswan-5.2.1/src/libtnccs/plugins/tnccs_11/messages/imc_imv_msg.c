/*
 * Copyright (C) 2006 Mike McCauley (mikem@open.com.au)
 * Copyright (C) 2010 Andreas Steffen, HSR Hochschule fuer Technik Rapperswil
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

#include "imc_imv_msg.h"

#include <tnc/tnccs/tnccs.h>

#include <utils/lexparser.h>
#include <utils/debug.h>

typedef struct private_imc_imv_msg_t private_imc_imv_msg_t;

#define BYTES_PER_LINE	57

/**
 * Private data of a imc_imv_msg_t object.
 *
 */
struct private_imc_imv_msg_t {
	/**
	 * Public imc_imv_msg_t interface.
	 */
	imc_imv_msg_t public;

	/**
	 * TNCCS message type
	 */
	tnccs_msg_type_t type;

	/**
	 * XML-encoded message node
	 */
	xmlNodePtr node;

	/**
	 * IMC-IMV message type
	 */
	TNC_MessageType msg_type;

	/**
	 * IMC-IMV message body
	 */
	chunk_t msg_body;

};

/**
 * Encodes message data into multiple base64-encoded lines
 */
static chunk_t encode_base64(chunk_t data)
{
	chunk_t encoding;
	u_char *pos;
	size_t b64_chars, b64_lines;

	/* handle empty message data object */
	if (data.len == 0)
	{
		encoding = chunk_alloc(1);
		*encoding.ptr = '\0';
		return encoding;
	}

	/* compute and allocate maximum size of base64 object */
	b64_chars = 4 * ((data.len + 2) / 3);
	b64_lines = (data.len + BYTES_PER_LINE - 1) / BYTES_PER_LINE;
	encoding = chunk_alloc(b64_chars + b64_lines);
	pos = encoding.ptr;

	/* encode lines */
	while (b64_lines--)
	{
		chunk_t data_line, b64_line;

		data_line = chunk_create(data.ptr, min(data.len, BYTES_PER_LINE));
		data.ptr += data_line.len;
		data.len -= data_line.len;
		b64_line = chunk_to_base64(data_line, pos);
		pos += b64_line.len;
		*pos = '\n';
		pos++;
	}
	/* terminate last line with NULL character instead of newline */
	*(pos-1) = '\0';

	return encoding;
}

/**
 * Decodes message data from multiple base64-encoded lines
 */
static chunk_t decode_base64(chunk_t data)
{
	chunk_t decoding, data_line, b64_line;
	u_char *pos;

	/* compute and allocate maximum size of decoded message data */
	decoding = chunk_alloc(3 * ((data.len + 3) / 4));
	pos = decoding.ptr;
	decoding.len = 0;

	while (fetchline(&data, &b64_line))
	{
		data_line = chunk_from_base64(b64_line, pos);
		pos += data_line.len;
		decoding.len += data_line.len;
	}

	return decoding;
}

METHOD(tnccs_msg_t, get_type, tnccs_msg_type_t,
	private_imc_imv_msg_t *this)
{
	return this->type;
}

METHOD(tnccs_msg_t, get_node, xmlNodePtr,
	private_imc_imv_msg_t *this)
{
	return this->node;
}

METHOD(tnccs_msg_t, destroy, void,
	private_imc_imv_msg_t *this)
{
	free(this->msg_body.ptr);
	free(this);
}

METHOD(imc_imv_msg_t, get_msg_type, TNC_MessageType,
	private_imc_imv_msg_t *this)
{
	return this->msg_type;
}

METHOD(imc_imv_msg_t, get_msg_body, chunk_t,
	private_imc_imv_msg_t *this)
{
	return this->msg_body;
}

/**
 * See header
 */
tnccs_msg_t *imc_imv_msg_create_from_node(xmlNodePtr node, linked_list_t *errors)
{
	private_imc_imv_msg_t *this;
	xmlNsPtr ns;
	xmlNodePtr cur;
	xmlChar *content;
	chunk_t b64_body;

	INIT(this,
		.public = {
			.tnccs_msg_interface = {
				.get_type = _get_type,
				.get_node = _get_node,
				.destroy = _destroy,
			},
			.get_msg_type = _get_msg_type,
			.get_msg_body = _get_msg_body,
		},
		.type = IMC_IMV_MSG,
		.node = node,
	);

	ns =  node->ns;
	cur = node->xmlChildrenNode;
	while (cur)
	{
		if (streq(cur->name, "Type") && cur->ns == ns)
		{
			content = xmlNodeGetContent(cur);
			this->msg_type = strtoul(content, NULL, 16);
			xmlFree(content);
		}
		else if (streq(cur->name, "Base64") && cur->ns == ns)
		{
			content = xmlNodeGetContent(cur);
			b64_body = chunk_create(content, strlen(content));
			this->msg_body = decode_base64(b64_body);
			xmlFree(content);
		}
		cur = cur->next;
	}

	return &this->public.tnccs_msg_interface;
}

/**
 * See header
 */
tnccs_msg_t *imc_imv_msg_create(TNC_MessageType msg_type, chunk_t msg_body)
{
	private_imc_imv_msg_t *this;
	chunk_t b64_body;
	char buf[10];		/* big enough for hex-encoded message type */
	xmlNodePtr n;

	INIT(this,
		.public = {
			.tnccs_msg_interface = {
				.get_type = _get_type,
				.get_node = _get_node,
				.destroy = _destroy,
			},
			.get_msg_type = _get_msg_type,
			.get_msg_body = _get_msg_body,
		},
		.type = IMC_IMV_MSG,
		.node = xmlNewNode(NULL, "IMC-IMV-Message"),
		.msg_type = msg_type,
		.msg_body = chunk_clone(msg_body),
	);

	/* add the message type number in hex */
	n = xmlNewNode(NULL, "Type");
	snprintf(buf, 10, "%08x", this->msg_type);
	xmlNodeSetContent(n, buf);
	xmlAddChild(this->node, n);

	/* encode the message as a Base64 node */
	n = xmlNewNode(NULL, "Base64");
	b64_body = encode_base64(this->msg_body);
	xmlNodeSetContent(n, b64_body.ptr);
	xmlAddChild(this->node, n);
	free(b64_body.ptr);

	return &this->public.tnccs_msg_interface;
}
