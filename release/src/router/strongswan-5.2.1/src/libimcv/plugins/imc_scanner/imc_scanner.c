/*
 * Copyright (C) 2011-2014 Andreas Steffen
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

#include "imc_scanner_state.h"

#include <imc/imc_agent.h>
#include <imc/imc_msg.h>
#include <ietf/ietf_attr.h>
#include <ietf/ietf_attr_attr_request.h>
#include <ietf/ietf_attr_port_filter.h>

#include <tncif_pa_subtypes.h>

#include <pen/pen.h>
#include <utils/lexparser.h>
#include <utils/debug.h>

#include <stdio.h>

/* IMC definitions */

static const char imc_name[] = "Scanner";

static pen_type_t msg_types[] = {
	{ PEN_IETF, PA_SUBTYPE_IETF_VPN }
};

static imc_agent_t *imc_scanner;

/**
 * see section 3.8.1 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_Initialize(TNC_IMCID imc_id,
							  TNC_Version min_version,
							  TNC_Version max_version,
							  TNC_Version *actual_version)
{
	if (imc_scanner)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has already been initialized", imc_name);
		return TNC_RESULT_ALREADY_INITIALIZED;
	}
	imc_scanner = imc_agent_create(imc_name, msg_types, countof(msg_types),
								   imc_id, actual_version);
	if (!imc_scanner)
	{
		return TNC_RESULT_FATAL;
	}
	if (min_version > TNC_IFIMC_VERSION_1 || max_version < TNC_IFIMC_VERSION_1)
	{
		DBG1(DBG_IMC, "no common IF-IMC version");
		return TNC_RESULT_NO_COMMON_VERSION;
	}
	return TNC_RESULT_SUCCESS;
}

/**
 * see section 3.8.2 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_NotifyConnectionChange(TNC_IMCID imc_id,
										  TNC_ConnectionID connection_id,
										  TNC_ConnectionState new_state)
{
	imc_state_t *state;

	if (!imc_scanner)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	switch (new_state)
	{
		case TNC_CONNECTION_STATE_CREATE:
			state = imc_scanner_state_create(connection_id);
			return imc_scanner->create_state(imc_scanner, state);
		case TNC_CONNECTION_STATE_HANDSHAKE:
			if (imc_scanner->change_state(imc_scanner, connection_id, new_state,
				&state) != TNC_RESULT_SUCCESS)
			{
				return TNC_RESULT_FATAL;
			}
			state->set_result(state, imc_id,
							  TNC_IMV_EVALUATION_RESULT_DONT_KNOW);
			return TNC_RESULT_SUCCESS;
		case TNC_CONNECTION_STATE_DELETE:
			return imc_scanner->delete_state(imc_scanner, connection_id);
		default:
			return imc_scanner->change_state(imc_scanner, connection_id,
											 new_state, NULL);
	}
}

/**
 * Determine all TCP and UDP server sockets listening on physical interfaces
 */
static bool do_netstat(ietf_attr_port_filter_t *attr)
{
	FILE *file;
	char buf[BUF_LEN];
	chunk_t line, token;
	int n = 0;
	bool success = FALSE;
	const char system_v4[]   = "127.0.1.1";
	const char loopback_v4[] = "127.0.0.1";
	const char loopback_v6[] = "::1";

	/* Open a pipe stream for reading the output of the netstat commmand */
	file = popen("/bin/netstat -n -l -p -4 -6 --inet", "r");
	if (!file)
	{
		DBG1(DBG_IMC, "failed to run netstat command");
		return FALSE;
	}

	/* Read the output a line at a time */
	while (fgets(buf, sizeof(buf), file))
	{
		u_char *pos;
		u_int8_t new_protocol, protocol;
		u_int16_t new_port, port;
		int i;
		enumerator_t *enumerator;
		bool allowed, found = FALSE;

		DBG2(DBG_IMC, "%.*s", (int)(strlen(buf)-1), buf);

		if (n++ < 2)
		{
			/* skip the first two header lines */
			continue;
		}
		line = chunk_create(buf, strlen(buf));

		/* Extract the IP protocol type */
		if (!extract_token(&token, ' ', &line))
		{
			DBG1(DBG_IMC, "protocol field in netstat output not found");
			goto end;
		}
		if (match("tcp", &token) || match("tcp6", &token))
		{
			new_protocol = IPPROTO_TCP;
		}
		else if (match("udp", &token) || match("udp6", &token))
		{
			new_protocol = IPPROTO_UDP;
		}
		else
		{
			DBG1(DBG_IMC, "skipped unknown IP protocol in netstat output");
			continue;
		}

		/* Skip the Recv-Q and Send-Q fields */
		for (i = 0; i < 3; i++)
		{
			if (!eat_whitespace(&line) || !extract_token(&token, ' ', &line))
			{
				token = chunk_empty;
				break;
			}
		}
		if (token.len == 0)
		{
			DBG1(DBG_IMC, "local address field in netstat output not found");
			goto end;
		}

		/* Find the local port appended to the local address */
		pos = token.ptr + token.len;
		while (*--pos != ':' && --token.len);
		if (*pos != ':')
		{
			DBG1(DBG_IMC, "local port field in netstat output not found");
			goto end;
		}
		token.len--;

		/* ignore ports of IPv4 and IPv6 loopback interfaces
		   and the internal system IPv4 address */
		if ((token.len == strlen(system_v4) &&
				memeq(system_v4, token.ptr, token.len)) ||
			(token.len == strlen(loopback_v4) &&
				memeq(loopback_v4, token.ptr, token.len)) ||
			(token.len == strlen(loopback_v6) &&
				memeq(loopback_v6, token.ptr, token.len)))
		{
			continue;
		}

		/* convert the port string to an integer */
		new_port = atoi(pos+1);

		/* check if the there is already a port entry */
		enumerator = attr->create_port_enumerator(attr);
		while (enumerator->enumerate(enumerator, &allowed, &protocol, &port))
		{
			if (new_port == port && new_protocol == protocol)
			{
				found = TRUE;
			}
		}
		enumerator->destroy(enumerator);

		/* Skip the duplicate port entry */
		if (found)
		{
			continue;
		}

		/* Add new port entry */
		attr->add_port(attr, FALSE, new_protocol, new_port);
	}

	/* Successfully completed the parsing of the netstat output */
	success = TRUE;

end:
	/* Close the pipe stream */
	pclose(file);
	return success;
}

/**
 * Add IETF Port Filter attribute to the send queue
 */
static TNC_Result add_port_filter(imc_msg_t *msg)
{
	pa_tnc_attr_t *attr;
	ietf_attr_port_filter_t *attr_port_filter;

	attr = ietf_attr_port_filter_create();
	attr->set_noskip_flag(attr, TRUE);
	attr_port_filter = (ietf_attr_port_filter_t*)attr;
	if (!do_netstat(attr_port_filter))
	{
		attr->destroy(attr);
		return TNC_RESULT_FATAL;
	}
	msg->add_attribute(msg, attr);

	return TNC_RESULT_SUCCESS;
}

/**
 * see section 3.8.3 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_BeginHandshake(TNC_IMCID imc_id,
								  TNC_ConnectionID connection_id)
{
	imc_state_t *state;
	imc_msg_t *out_msg;
	TNC_Result result = TNC_RESULT_SUCCESS;

	if (!imc_scanner)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_scanner->get_state(imc_scanner, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	if (lib->settings->get_bool(lib->settings,
							"%s.plugins.imc-scanner.push_info", TRUE, lib->ns))
	{
		out_msg = imc_msg_create(imc_scanner, state, connection_id, imc_id,
								 TNC_IMVID_ANY, msg_types[0]);
		result = add_port_filter(out_msg);
		if (result == TNC_RESULT_SUCCESS)
		{
			/* send PA-TNC message with the excl flag not set */
			result = out_msg->send(out_msg, FALSE);
		}
		out_msg->destroy(out_msg);
	}

	return result;
}

static TNC_Result receive_message(imc_msg_t *in_msg)
{
	imc_msg_t *out_msg;
	enumerator_t *enumerator;
	pa_tnc_attr_t *attr;
	pen_type_t attr_type;
	TNC_Result result = TNC_RESULT_SUCCESS;
	bool fatal_error = FALSE;

	/* generate an outgoing PA-TNC message - we might need it */
	out_msg = imc_msg_create_as_reply(in_msg);

	/* parse received PA-TNC message and handle local and remote errors */
	result = in_msg->receive(in_msg, out_msg, &fatal_error);
	if (result != TNC_RESULT_SUCCESS)
	{
		out_msg->destroy(out_msg);
		return result;
	}

	/* analyze PA-TNC attributes */
	enumerator = in_msg->create_attribute_enumerator(in_msg);
	while (enumerator->enumerate(enumerator, &attr))
	{
		attr_type = attr->get_type(attr);

		if (attr_type.vendor_id != PEN_IETF)
		{
			continue;
		}
		if (attr_type.type == IETF_ATTR_ATTRIBUTE_REQUEST)
		{
			ietf_attr_attr_request_t *attr_cast;
			pen_type_t *entry;
			enumerator_t *e;

			attr_cast = (ietf_attr_attr_request_t*)attr;

			e = attr_cast->create_enumerator(attr_cast);
			while (e->enumerate(e, &entry))
			{
				if (entry->vendor_id != PEN_IETF)
				{
					continue;
				}
				switch (entry->type)
				{
					case IETF_ATTR_PORT_FILTER:
						result = add_port_filter(out_msg);
						break;
					default:
						break;
				}
			}
			e->destroy(e);
		}
	}
	enumerator->destroy(enumerator);

	if (fatal_error)
	{
		result = TNC_RESULT_FATAL;
	}
	else if (result == TNC_RESULT_SUCCESS)
	{
		/* send PA-TNC message with the EXCL flag set */
		result = out_msg->send(out_msg, TRUE);
	}
	out_msg->destroy(out_msg);

	return result;
}

/**
 * see section 3.8.4 of TCG TNC IF-IMC Specification 1.3

 */
TNC_Result TNC_IMC_ReceiveMessage(TNC_IMCID imc_id,
								  TNC_ConnectionID connection_id,
								  TNC_BufferReference msg,
								  TNC_UInt32 msg_len,
								  TNC_MessageType msg_type)
{
	imc_state_t *state;
	imc_msg_t *in_msg;
	TNC_Result result;

	if (!imc_scanner)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_scanner->get_state(imc_scanner, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}

	in_msg = imc_msg_create_from_data(imc_scanner, state, connection_id,
									  msg_type, chunk_create(msg, msg_len));
	result = receive_message(in_msg);
	in_msg->destroy(in_msg);

	return result;
}

/**
 * see section 3.8.6 of TCG TNC IF-IMV Specification 1.3
 */
TNC_Result TNC_IMC_ReceiveMessageLong(TNC_IMCID imc_id,
									  TNC_ConnectionID connection_id,
									  TNC_UInt32 msg_flags,
									  TNC_BufferReference msg,
									  TNC_UInt32 msg_len,
									  TNC_VendorID msg_vid,
									  TNC_MessageSubtype msg_subtype,
									  TNC_UInt32 src_imv_id,
									  TNC_UInt32 dst_imc_id)
{
	imc_state_t *state;
	imc_msg_t *in_msg;
	TNC_Result result;

	if (!imc_scanner)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	if (!imc_scanner->get_state(imc_scanner, connection_id, &state))
	{
		return TNC_RESULT_FATAL;
	}
	in_msg = imc_msg_create_from_long_data(imc_scanner, state, connection_id,
								src_imv_id, dst_imc_id, msg_vid, msg_subtype,
								chunk_create(msg, msg_len));
	result = receive_message(in_msg);
	in_msg->destroy(in_msg);

	return result;
}

/**
 * see section 3.8.7 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_BatchEnding(TNC_IMCID imc_id,
							   TNC_ConnectionID connection_id)
{
	if (!imc_scanner)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return TNC_RESULT_SUCCESS;
}

/**
 * see section 3.8.8 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_Terminate(TNC_IMCID imc_id)
{
	if (!imc_scanner)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	imc_scanner->destroy(imc_scanner);
	imc_scanner = NULL;

	return TNC_RESULT_SUCCESS;
}

/**
 * see section 4.2.8.1 of TCG TNC IF-IMC Specification 1.3
 */
TNC_Result TNC_IMC_ProvideBindFunction(TNC_IMCID imc_id,
									   TNC_TNCC_BindFunctionPointer bind_function)
{
	if (!imc_scanner)
	{
		DBG1(DBG_IMC, "IMC \"%s\" has not been initialized", imc_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return imc_scanner->bind_functions(imc_scanner, bind_function);
}
