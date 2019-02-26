/*
 * Copyright (C) 2012-2013 Andreas Steffen
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

/**
 * Define the following two static constants externally:
 * static const char imv_name[] = "xx";
 * static const imv_agent_create_t imv_agent_create = imv_xx_agent_create;
 */

#include <utils/debug.h>

static imv_agent_if_t *imv_agent;

/*
 * see section 3.8.1 of TCG TNC IF-IMV Specification 1.3
 */
TNC_Result TNC_IMV_API TNC_IMV_Initialize(TNC_IMVID imv_id,
										  TNC_Version min_version,
										  TNC_Version max_version,
										  TNC_Version *actual_version)
{
	if (imv_agent)
	{
		DBG1(DBG_IMV, "IMV \"%s\" has already been initialized", imv_name);
		return TNC_RESULT_ALREADY_INITIALIZED;
	}

	imv_agent = imv_agent_create(imv_name, imv_id, actual_version);

	if (!imv_agent)
	{
		return TNC_RESULT_FATAL;
	}
	if (min_version > TNC_IFIMV_VERSION_1 || max_version < TNC_IFIMV_VERSION_1)
	{
		DBG1(DBG_IMV, "no common IF-IMV version");
		return TNC_RESULT_NO_COMMON_VERSION;
	}
	return TNC_RESULT_SUCCESS;
}

/**
 * see section 3.8.2 of TCG TNC IF-IMV Specification 1.3
 */
TNC_Result TNC_IMV_API TNC_IMV_NotifyConnectionChange(TNC_IMVID imv_id,
												TNC_ConnectionID connection_id,
												TNC_ConnectionState new_state)
{
	if (!imv_agent)
	{
		DBG1(DBG_IMV, "IMV \"%s\" has not been initialized", imv_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return imv_agent->notify_connection_change(imv_agent, connection_id,
											   new_state);
}

/**
 * see section 3.8.4 of TCG TNC IF-IMV Specification 1.3
 */
TNC_Result TNC_IMV_API TNC_IMV_ReceiveMessage(TNC_IMVID imv_id,
											  TNC_ConnectionID connection_id,
											  TNC_BufferReference msg,
											  TNC_UInt32 msg_len,
											  TNC_MessageType msg_type)
{
	if (!imv_agent)
	{
		DBG1(DBG_IMV, "IMV \"%s\" has not been initialized", imv_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return imv_agent->receive_message(imv_agent, connection_id, msg_type,
									  chunk_create(msg, msg_len));
}

/**
 * see section 3.8.6 of TCG TNC IF-IMV Specification 1.3
 */
TNC_Result TNC_IMV_API TNC_IMV_ReceiveMessageLong(TNC_IMVID imv_id,
												  TNC_ConnectionID connection_id,
												  TNC_UInt32 msg_flags,
												  TNC_BufferReference msg,
												  TNC_UInt32 msg_len,
												  TNC_VendorID msg_vid,
												  TNC_MessageSubtype msg_subtype,
												  TNC_UInt32 src_imc_id,
												  TNC_UInt32 dst_imv_id)
{
	if (!imv_agent)
	{
		DBG1(DBG_IMV, "IMV \"%s\" has not been initialized", imv_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return imv_agent->receive_message_long(imv_agent, connection_id,
							src_imc_id, dst_imv_id,
							msg_vid, msg_subtype, chunk_create(msg, msg_len));
}

/**
 * see section 3.8.7 of TCG TNC IF-IMV Specification 1.3
 */
TNC_Result TNC_IMV_API TNC_IMV_SolicitRecommendation(TNC_IMVID imv_id,
												TNC_ConnectionID connection_id)
{

	if (!imv_agent)
	{
		DBG1(DBG_IMV, "IMV \"%s\" has not been initialized", imv_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return imv_agent->solicit_recommendation(imv_agent, connection_id);
}

/**
 * see section 3.8.8 of TCG TNC IF-IMV Specification 1.3
 */
TNC_Result TNC_IMV_API TNC_IMV_BatchEnding(TNC_IMVID imv_id,
										   TNC_ConnectionID connection_id)
{
	if (!imv_agent)
	{
		DBG1(DBG_IMV, "IMV \"%s\" has not been initialized", imv_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return imv_agent->batch_ending(imv_agent, connection_id);
}

/**
 * see section 3.8.9 of TCG TNC IF-IMV Specification 1.3
 */
TNC_Result TNC_IMV_API TNC_IMV_Terminate(TNC_IMVID imv_id)
{
	if (!imv_agent)
	{
		DBG1(DBG_IMV, "IMV \"%s\" has not been initialized", imv_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	imv_agent->destroy(imv_agent);
	imv_agent = NULL;

	return TNC_RESULT_SUCCESS;
}

/**
 * see section 4.2.8.1 of TCG TNC IF-IMV Specification 1.3
 */
TNC_Result TNC_IMV_API TNC_IMV_ProvideBindFunction(TNC_IMVID imv_id,
								TNC_TNCS_BindFunctionPointer bind_function)
{
	if (!imv_agent)
	{
		DBG1(DBG_IMV, "IMV \"%s\" has not been initialized", imv_name);
		return TNC_RESULT_NOT_INITIALIZED;
	}
	return imv_agent->bind_functions(imv_agent, bind_function);
}
