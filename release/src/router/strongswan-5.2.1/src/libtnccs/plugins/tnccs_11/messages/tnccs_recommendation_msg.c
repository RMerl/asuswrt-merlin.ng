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

#include "tnccs_recommendation_msg.h"
#include "tnccs_error_msg.h"

#include <utils/debug.h>

typedef struct private_tnccs_recommendation_msg_t private_tnccs_recommendation_msg_t;

/**
 * Private data of a tnccs_recommendation_msg_t object.
 *
 */
struct private_tnccs_recommendation_msg_t {
	/**
	 * Public tnccs_recommendation_msg_t interface.
	 */
	tnccs_recommendation_msg_t public;

	/**
	 * TNCCS message type
	 */
	tnccs_msg_type_t type;

	/**
	 * XML-encoded message node
	 */
	xmlNodePtr node;

	/**
	 * Action Recommendation
	 */
	TNC_IMV_Action_Recommendation rec;
};

METHOD(tnccs_msg_t, get_type, tnccs_msg_type_t,
	private_tnccs_recommendation_msg_t *this)
{
	return this->type;
}

METHOD(tnccs_msg_t, get_node, xmlNodePtr,
	private_tnccs_recommendation_msg_t *this)
{
	return this->node;
}

METHOD(tnccs_msg_t, destroy, void,
	private_tnccs_recommendation_msg_t *this)
{
	free(this);
}

METHOD(tnccs_recommendation_msg_t, get_recommendation, TNC_IMV_Action_Recommendation,
	private_tnccs_recommendation_msg_t *this)
{
	return this->rec;
}

/**
 * See header
 */
tnccs_msg_t *tnccs_recommendation_msg_create_from_node(xmlNodePtr node,
													   linked_list_t *errors)
{
	private_tnccs_recommendation_msg_t *this;
	xmlChar *rec_string;
	char *error_msg, buf[BUF_LEN];
	tnccs_error_type_t error_type = TNCCS_ERROR_MALFORMED_BATCH;
	tnccs_msg_t *msg;

	INIT(this,
		.public = {
			.tnccs_msg_interface = {
				.get_type = _get_type,
				.get_node = _get_node,
				.destroy = _destroy,
			},
			.get_recommendation = _get_recommendation,
		},
		.type = TNCCS_MSG_RECOMMENDATION,
		.node = node,
	);

	rec_string = xmlGetProp(node, "type");
	if (!rec_string)
	{
		error_msg = "type property in TNCCS-Recommendation is missing";
		goto fatal;
	}
	else if (streq(rec_string, "allow"))
	{
		this->rec = TNC_IMV_ACTION_RECOMMENDATION_ALLOW;
	}
	else if (streq(rec_string, "isolate"))
	{
		this->rec = TNC_IMV_ACTION_RECOMMENDATION_ISOLATE;
	}
	else if (streq(rec_string, "none"))
	{
		this->rec = TNC_IMV_ACTION_RECOMMENDATION_NO_ACCESS;
	}
	else
	{
		error_msg = buf;
		snprintf(buf, BUF_LEN, "unsupported type property value '%s' "
							   "in TNCCS-Recommendation", rec_string);
		xmlFree(rec_string);
		goto fatal;
	}
	xmlFree(rec_string);

	return &this->public.tnccs_msg_interface;

fatal:
	msg = tnccs_error_msg_create(error_type, error_msg);
	errors->insert_last(errors, msg);
	destroy(this);
	return NULL;
}

/**
 * See header
 */
tnccs_msg_t *tnccs_recommendation_msg_create(TNC_IMV_Action_Recommendation rec)
{
	private_tnccs_recommendation_msg_t *this;
	xmlNodePtr n, n2;
	char *rec_string;

	INIT(this,
		.public = {
			.tnccs_msg_interface = {
				.get_type = _get_type,
				.get_node = _get_node,
				.destroy = _destroy,
			},
			.get_recommendation = _get_recommendation,
		},
		.type = TNCCS_MSG_RECOMMENDATION,
		.node =  xmlNewNode(NULL, "TNCC-TNCS-Message"),
		.rec = rec,
	);

	/* add the message type number in hex */
	n = xmlNewNode(NULL, "Type");
	xmlNodeSetContent(n, "00000001");
	xmlAddChild(this->node, n);

	n = xmlNewNode(NULL, "XML");
	xmlAddChild(this->node, n);

	switch (rec)
	{
		case TNC_IMV_ACTION_RECOMMENDATION_ALLOW:
			rec_string = "allow";
			break;
		case TNC_IMV_ACTION_RECOMMENDATION_ISOLATE:
			rec_string = "isolate";
			break;
		case TNC_IMV_ACTION_RECOMMENDATION_NO_ACCESS:
		case TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION:
		default:
			rec_string = "none";
	}

	n2 = xmlNewNode(NULL, enum_to_name(tnccs_msg_type_names, this->type));
	xmlNewProp(n2, BAD_CAST "type", rec_string);
	xmlNodeSetContent(n2, "");
	xmlAddChild(n, n2);

	return &this->public.tnccs_msg_interface;
}
