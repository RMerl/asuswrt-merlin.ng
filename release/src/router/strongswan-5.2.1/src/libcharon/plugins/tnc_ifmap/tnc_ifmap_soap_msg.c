/*
 * Copyright (C) 2013 Andreas Steffen
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

#include "tnc_ifmap_soap_msg.h"
#include "tnc_ifmap_http.h"

#include <utils/debug.h>

#define SOAP_NS		"http://www.w3.org/2003/05/soap-envelope"

typedef struct private_tnc_ifmap_soap_msg_t private_tnc_ifmap_soap_msg_t;

/**
 * Private data of an tnc_ifmap_soap_msg_t object.
 */
struct private_tnc_ifmap_soap_msg_t {

	/**
	 * Public tnc_ifmap_soap_msg_t interface.
	 */
	tnc_ifmap_soap_msg_t public;

	/**
	 * HTTP POST request builder and response processing
	 */
	tnc_ifmap_http_t *http;

	/**
	 * TLS socket
	 */
	tls_socket_t *tls;

	/**
	 * XML Document
	 */
	xmlDocPtr doc;

};

/**
 * Find a child node with a given name
 */
static xmlNodePtr find_child(xmlNodePtr parent, const xmlChar* name)
{
	xmlNodePtr child;
	
	child = parent->xmlChildrenNode;
	while (child)
	{
		if (xmlStrcmp(child->name, name) == 0)
		{
			return child;
		}
		child = child->next;
	}

	DBG1(DBG_TNC, "child node \"%s\" not found", name);
	return NULL;
}

METHOD(tnc_ifmap_soap_msg_t, post, bool,
	private_tnc_ifmap_soap_msg_t *this, xmlNodePtr request, char *result_name,
	xmlNodePtr *result)
{
	xmlDocPtr doc;
	xmlNodePtr env, body, cur, response;
	xmlNsPtr ns;
	xmlChar *xml_str, *errorCode, *errorString;
	int xml_len, len, written;
	chunk_t xml, http;
	char buf[4096];
	status_t status;

	DBG2(DBG_TNC, "sending ifmap %s", request->name);

	/* Generate XML Document containing SOAP Envelope */
	doc = xmlNewDoc("1.0");
	env =xmlNewNode(NULL, "Envelope");
	ns = xmlNewNs(env, SOAP_NS, "env");
	xmlSetNs(env, ns);
	xmlDocSetRootElement(doc, env);

	/* Add SOAP Body containing IF-MAP request */
	body = xmlNewNode(ns, "Body");
	xmlAddChild(body, request);
	xmlAddChild(env, body);

	/* Convert XML Document into a character string */
	xmlDocDumpFormatMemory(doc, &xml_str, &xml_len, 1);
	xmlFreeDoc(doc);
	DBG3(DBG_TNC, "%.*s", xml_len, xml_str);
	xml = chunk_create(xml_str, xml_len);

	/* Send SOAP-XML request via HTTPS POST */
	do
	{
		status = this->http->build(this->http, &xml, &http);
		if (status == FAILED)
		{
			break;
		}
		written = this->tls->write(this->tls, http.ptr, http.len);
		free(http.ptr);
		if (written != http.len)
		{
			status = FAILED;
			break;
		}
	}
	while (status == NEED_MORE);

	xmlFree(xml_str);
	if (status != SUCCESS)
	{
		return FALSE;
	}

	/* Receive SOAP-XML response via [chunked] HTTPS */
	xml = chunk_empty;
	do
	{
		len = this->tls->read(this->tls, buf, sizeof(buf), TRUE);
		if (len <= 0)
		{
			return FALSE;
		}
		http = chunk_create(buf, len);

		status = this->http->process(this->http, &http, &xml);
		if (status == FAILED)
		{
			free(xml.ptr);
			return FALSE;
		}
	}
	while (status == NEED_MORE);

	DBG3(DBG_TNC, "parsing XML message %B", &xml);
	this->doc = xmlParseMemory(xml.ptr, xml.len);
	free(xml.ptr);
	
	if (!this->doc)
	{
		DBG1(DBG_TNC, "failed to parse XML message");
		return FALSE;
	}

	/* check out XML document */
	cur = xmlDocGetRootElement(this->doc);
	if (!cur)
	{
		DBG1(DBG_TNC, "empty XML message");
		return FALSE;
	}

	/* get XML Document type is a SOAP Envelope */
	if (xmlStrcmp(cur->name, "Envelope"))
	{
		DBG1(DBG_TNC, "XML message does not contain a SOAP Envelope");
		return FALSE;
	}

	/* get SOAP Body */
	cur = find_child(cur, "Body");
	if (!cur)
	{
		return FALSE;
	}

	/* get IF-MAP response */
	response = find_child(cur, "response");
	if (!response)
	{
		return FALSE;
	}

	/* get IF-MAP result */
	cur = find_child(response, result_name);
	if (!cur)
	{
		cur = find_child(response, "errorResult");
		if (cur)
		{
			DBG1(DBG_TNC, "received errorResult");

			errorCode = xmlGetProp(cur, "errorCode");
			if (errorCode)
			{
				DBG1(DBG_TNC, "  %s", errorCode);
				xmlFree(errorCode);
			}

			cur = find_child(cur, "errorString");
			if (cur)
			{
				errorString = xmlNodeGetContent(cur);
				if (errorString)
				{
					DBG1(DBG_TNC, "  %s", errorString);
					xmlFree(errorString);
				}
			}
		}
		return FALSE;
	}

	if (result)
	{
		*result = cur;
	}
	return TRUE;
}

METHOD(tnc_ifmap_soap_msg_t, destroy, void,
	private_tnc_ifmap_soap_msg_t *this)
{
	this->http->destroy(this->http);
	if (this->doc)
	{
		xmlFreeDoc(this->doc);
	}
	free(this);
}

/**
 * See header
 */
tnc_ifmap_soap_msg_t *tnc_ifmap_soap_msg_create(char *uri, chunk_t user_pass,
												tls_socket_t *tls)
{
	private_tnc_ifmap_soap_msg_t *this;

	INIT(this,
		.public = {
			.post = _post,
			.destroy = _destroy,
		},
		.http = tnc_ifmap_http_create(uri, user_pass),
		.tls = tls,
	);

	return &this->public;
}

