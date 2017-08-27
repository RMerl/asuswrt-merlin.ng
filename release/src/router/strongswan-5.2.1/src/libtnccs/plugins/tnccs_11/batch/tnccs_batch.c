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

#include "tnccs_batch.h"
#include "messages/tnccs_error_msg.h"

#include <tnc/tnccs/tnccs.h>

#include <collections/linked_list.h>
#include <utils/debug.h>

#include <libxml/parser.h>

#define TNCCS_NS	"http://www.trustedcomputinggroup.org/IWG/TNC/1_0/IF_TNCCS#"
#define SCHEMA_NS	"http://www.w3.org/2001/XMLSchema-instance"
#define TNCCS_XSD	"https://www.trustedcomputinggroup.org/XML/SCHEMA/TNCCS_1.0.xsd"

typedef struct private_tnccs_batch_t private_tnccs_batch_t;

/**
 * Private data of a tnccs_batch_t object.
 *
 */
struct private_tnccs_batch_t {
	/**
	 * Public tnccs_batch_t interface.
	 */
	tnccs_batch_t public;

	/**
	 * Batch ID
	 */
	int batch_id;

	/**
	 * TNCC if TRUE, TNCS if FALSE
	 */
	bool is_server;

	/**
	 * linked list of TNCCS messages
	 */
	linked_list_t *messages;

	/**
	 * linked list of TNCCS error messages
	 */
	linked_list_t *errors;

	/**
	 * XML document
	 */
	xmlDocPtr doc;

	/**
	 * Encoded message
	 */
	chunk_t encoding;
};

METHOD(tnccs_batch_t, get_encoding, chunk_t,
	private_tnccs_batch_t *this)
{
	return this->encoding;
}

METHOD(tnccs_batch_t, add_msg, void,
	private_tnccs_batch_t *this, tnccs_msg_t* msg)
{
	xmlNodePtr root;

	DBG2(DBG_TNC, "adding %N message", tnccs_msg_type_names,
									   msg->get_type(msg));
	this->messages->insert_last(this->messages, msg);
	root = xmlDocGetRootElement(this->doc);
	xmlAddChild(root, msg->get_node(msg));
}

METHOD(tnccs_batch_t, build, void,
	private_tnccs_batch_t *this)
{
	xmlChar *xmlbuf;
	int buf_size;

	xmlDocDumpFormatMemory(this->doc, &xmlbuf, &buf_size, 1);
	this->encoding = chunk_create(xmlbuf, buf_size);
	this->encoding = chunk_clone(this->encoding);
	xmlFree(xmlbuf);
}

METHOD(tnccs_batch_t, process, status_t,
	private_tnccs_batch_t *this)
{
	tnccs_msg_t *tnccs_msg, *msg;
	tnccs_error_type_t error_type = TNCCS_ERROR_OTHER;
	char *error_msg, buf[BUF_LEN];
	xmlNodePtr cur;
	xmlNsPtr ns;
	xmlChar *batchid, *recipient;
	int batch_id;

	this->doc = xmlParseMemory(this->encoding.ptr, this->encoding.len);
	if (!this->doc)
	{
		error_type = TNCCS_ERROR_MALFORMED_BATCH;
		error_msg = "failed to parse XML message";
		goto fatal;
	}

	/* check out the XML document */
	cur = xmlDocGetRootElement(this->doc);
	if (!cur)
	{
		error_type = TNCCS_ERROR_MALFORMED_BATCH;
		error_msg = "empty XML document";
		goto fatal;
	}

	/* check TNCCS namespace */
	ns = xmlSearchNsByHref(this->doc, cur, TNCCS_NS);
	if (!ns)
	{
		error_type = TNCCS_ERROR_MALFORMED_BATCH;
		error_msg = "TNCCS namespace not found";
		goto fatal;
	}

	/* check XML document type */
	if (xmlStrcmp(cur->name, "TNCCS-Batch"))
	{
		error_type = TNCCS_ERROR_MALFORMED_BATCH;
		error_msg = buf;
		snprintf(buf, BUF_LEN, "wrong XML document type '%s', expected TNCCS-Batch",
								cur->name);
		goto fatal;
	}

	/* check presence of BatchID property */
	batchid = xmlGetProp(cur, "BatchId");
	if (!batchid)
	{
		error_type = TNCCS_ERROR_INVALID_BATCH_ID;
		error_msg = "BatchId is missing";
		goto fatal;
	}

	/* check BatchID */
	batch_id = atoi((char*)batchid);
	xmlFree(batchid);
	if (batch_id != this->batch_id)
	{
		error_type = TNCCS_ERROR_INVALID_BATCH_ID;
		error_msg = buf;
		snprintf(buf, BUF_LEN, "BatchId %d expected, got %d", this->batch_id,
															  batch_id);
		goto fatal;
	}

	/* check presence of Recipient property */
	recipient = xmlGetProp(cur, "Recipient");
	if (!recipient)
	{
		error_type = TNCCS_ERROR_INVALID_RECIPIENT_TYPE;
		error_msg = "Recipient is missing";
		goto fatal;
	}

	/* check recipient */
	if (!streq(recipient, this->is_server ? "TNCS" : "TNCC"))
	{
		error_type = TNCCS_ERROR_INVALID_RECIPIENT_TYPE;
		error_msg =	buf;
		snprintf(buf, BUF_LEN, "message recipient expected '%s', got '%s'",
				 this->is_server ? "TNCS" : "TNCC", recipient);
		xmlFree(recipient);
		goto fatal;
	}
	xmlFree(recipient);

	DBG2(DBG_TNC, "processing TNCCS Batch #%d", batch_id);

	/* Now walk the tree, handling message nodes as we go */
	for (cur = cur->xmlChildrenNode; cur != NULL; cur = cur->next)
	{
		/* ignore empty or blank nodes */
		if (xmlIsBlankNode(cur))
		{
			continue;
		}

		/* ignore nodes with wrong namespace */
		if (cur->ns != ns)
		{
			DBG1(DBG_TNC, "ignoring message node '%s' having wrong namespace",
						  cur->name);
			continue;
		}

		tnccs_msg = tnccs_msg_create_from_node(cur, this->errors);

		/* exit if a message parsing error occurred */
		if (this->errors->get_count(this->errors) > 0)
		{
			return FAILED;
		}

		/* ignore unrecognized messages */
		if (!tnccs_msg)
		{
			continue;
		}

		this->messages->insert_last(this->messages, tnccs_msg);
	}
	return SUCCESS;

fatal:
	msg = tnccs_error_msg_create(error_type, error_msg);
	this->errors->insert_last(this->errors, msg);
	return FAILED;
}

METHOD(tnccs_batch_t, create_msg_enumerator, enumerator_t*,
	private_tnccs_batch_t *this)
{
	return this->messages->create_enumerator(this->messages);
}

METHOD(tnccs_batch_t, create_error_enumerator, enumerator_t*,
	private_tnccs_batch_t *this)
{
	return this->errors->create_enumerator(this->errors);
}

METHOD(tnccs_batch_t, destroy, void,
	private_tnccs_batch_t *this)
{
	this->messages->destroy_offset(this->messages,
								   offsetof(tnccs_msg_t, destroy));
	this->errors->destroy_offset(this->errors,
								   offsetof(tnccs_msg_t, destroy));
	xmlFreeDoc(this->doc);
	free(this->encoding.ptr);
	free(this);
}

/**
 * See header
 */
tnccs_batch_t* tnccs_batch_create(bool is_server, int batch_id)
{
	private_tnccs_batch_t *this;
	xmlNodePtr n;
	xmlNsPtr ns_xsi;
	char buf[12];

	INIT(this,
		.public = {
			.get_encoding = _get_encoding,
			.add_msg = _add_msg,
			.build = _build,
			.process = _process,
			.create_msg_enumerator = _create_msg_enumerator,
			.create_error_enumerator = _create_error_enumerator,
			.destroy = _destroy,
		},
		.is_server = is_server,
		.messages = linked_list_create(),
		.errors = linked_list_create(),
		.batch_id = batch_id,
		.doc = xmlNewDoc("1.0"),
	);

	DBG2(DBG_TNC, "creating TNCCS Batch #%d", this->batch_id);
	n = xmlNewNode(NULL, "TNCCS-Batch");
	xmlNewNs(n, TNCCS_NS, NULL);
	ns_xsi = xmlNewNs(n, SCHEMA_NS, "xsi");
	snprintf(buf, sizeof(buf), "%d", batch_id);
	xmlNewProp(n, "BatchId", buf);
	xmlNewProp(n, "Recipient", this->is_server ? "TNCC" : "TNCS");
	xmlNewNsProp(n, ns_xsi, "schemaLocation", TNCCS_NS " " TNCCS_XSD);
	xmlDocSetRootElement(this->doc, n);

	return &this->public;
}

/**
 * See header
 */
tnccs_batch_t* tnccs_batch_create_from_data(bool is_server, int batch_id, chunk_t data)
{
	private_tnccs_batch_t *this;

	INIT(this,
		.public = {
			.get_encoding = _get_encoding,
			.add_msg = _add_msg,
			.build = _build,
			.process = _process,
			.create_msg_enumerator = _create_msg_enumerator,
			.create_error_enumerator = _create_error_enumerator,
			.destroy = _destroy,
		},
		.is_server = is_server,
		.batch_id = batch_id,
		.messages = linked_list_create(),
		.errors = linked_list_create(),
		.encoding = chunk_clone(data),
	);

	return &this->public;
}

