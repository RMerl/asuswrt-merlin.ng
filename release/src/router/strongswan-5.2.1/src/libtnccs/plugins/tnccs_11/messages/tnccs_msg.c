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

#include "tnccs_msg.h"
#include "imc_imv_msg.h"
#include "tnccs_error_msg.h"
#include "tnccs_preferred_language_msg.h"
#include "tnccs_reason_strings_msg.h"
#include "tnccs_recommendation_msg.h"
#include "tnccs_tncs_contact_info_msg.h"

#include <library.h>
#include <utils/debug.h>

ENUM(tnccs_msg_type_names, IMC_IMV_MSG, TNCCS_MSG_ROOF,
	"IMC-IMV",
	"TNCCS-Recommendation",
	"TNCCS-Error",
	"TNCCS-PreferredLanguage",
	"TNCCS-ReasonStrings",
	"TNCCS-TNCSContactInfo"
);

/**
 * See header
 */
tnccs_msg_t* tnccs_msg_create_from_node(xmlNodePtr node, linked_list_t *errors)
{
	char *error_msg, buf[BUF_LEN];
	tnccs_error_type_t error_type = TNCCS_ERROR_MALFORMED_BATCH;
	tnccs_msg_t *msg;
	tnccs_msg_type_t type = IMC_IMV_MSG, nametype;

	if (streq((char*)node->name, "IMC-IMV-Message"))
	{
		DBG2(DBG_TNC, "processing %N message", tnccs_msg_type_names, type);
		return imc_imv_msg_create_from_node(node, errors);
	}
	else if (streq((char*)node->name, "TNCC-TNCS-Message"))
	{
		bool found = FALSE;
		xmlNsPtr ns = node->ns;
		xmlNodePtr cur = node->xmlChildrenNode;
		xmlNodePtr xml_msg_node = NULL;

		while (cur)
		{
			if (streq(cur->name, "Type") && cur->ns == ns)
			{
				xmlChar *content = xmlNodeGetContent(cur);

				type = strtol(content, NULL, 16);
				xmlFree(content);
				found = TRUE;
			}
			else if (streq(cur->name, "XML") && cur->ns == ns)
			{
				xml_msg_node = cur->xmlChildrenNode;
			}
			cur = cur->next;
		}
		if (!found)
		{
			error_msg = "Type is missing in TNCC-TNCS-Message";
			goto fatal;
		}
		if (!xml_msg_node)
		{
			error_msg = "XML node is missing in TNCC-TNCS-Message";
			goto fatal;
		}
		cur = xml_msg_node;

		/* skip empty and blank nodes */
		while (cur && xmlIsBlankNode(cur))
		{
			cur = cur->next;
		}
		if (!cur)
		{
			error_msg = "XML node is empty";
			goto fatal;
		}

		/* check if TNCCS message type and node name agree */
		if (type >= TNCCS_MSG_RECOMMENDATION && type <= TNCCS_MSG_ROOF)
		{
			DBG2(DBG_TNC, "processing %N message", tnccs_msg_type_names, type);
			if (cur->ns != ns)
			{
				error_msg = "node is not in the TNCCS message namespace";
				goto fatal;
			}
			if (!enum_from_name(tnccs_msg_type_names, cur->name, &nametype) ||
				type != nametype)
			{
				error_msg = buf;
				snprintf(buf, BUF_LEN, "expected '%N' node but was '%s'",
						 tnccs_msg_type_names, type, (char*)cur->name);
				goto fatal;
			}
		}

		switch (type)
		{
			case TNCCS_MSG_RECOMMENDATION:
				return tnccs_recommendation_msg_create_from_node(cur, errors);
			case TNCCS_MSG_ERROR:
				return tnccs_error_msg_create_from_node(cur);
			case TNCCS_MSG_PREFERRED_LANGUAGE:
				return tnccs_preferred_language_msg_create_from_node(cur, errors);
			case TNCCS_MSG_REASON_STRINGS:
				return tnccs_reason_strings_msg_create_from_node(cur, errors);
			case TNCCS_MSG_TNCS_CONTACT_INFO:
				return tnccs_tncs_contact_info_msg_create_from_node(cur, errors);
			default:
				DBG1(DBG_TNC, "ignoring TNCC-TNCS-Message with type %d", type);
				return NULL;
		}
	}
	DBG1(DBG_TNC, "ignoring unknown message node '%s'", (char*)node->name);
	return NULL;

fatal:
	msg = tnccs_error_msg_create(error_type, error_msg);
	errors->insert_last(errors, msg);
	return NULL;
}
