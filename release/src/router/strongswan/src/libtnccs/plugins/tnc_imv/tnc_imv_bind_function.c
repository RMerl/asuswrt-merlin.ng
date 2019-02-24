/*
 * Copyright (C) 2006 Mike McCauley
 * Copyright (C) 2010-2011 Andreas Steffen
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

#include <tnc/tnc.h>
#include <tnc/imv/imv_manager.h>
#include <tnc/tnccs/tnccs_manager.h>

#include <utils/debug.h>

/**
 * Called by the IMV to inform a TNCS about the set of message types the IMV
 * is able to receive
 */
TNC_Result TNC_TNCS_ReportMessageTypes(TNC_IMVID imv_id,
									   TNC_MessageTypeList supported_types,
									   TNC_UInt32 type_count)
{
	if (!tnc->imvs->is_registered(tnc->imvs, imv_id))
	{
		DBG1(DBG_TNC, "ignoring ReportMessageTypes() from unregistered IMV %u",
					   imv_id);
		return TNC_RESULT_INVALID_PARAMETER;
	}
	return tnc->imvs->set_message_types(tnc->imvs, imv_id, supported_types,
										type_count);
}

/**
 * Called by the IMV to inform a TNCS about the set of message types the IMV
 * is able to receive. This function supports long message types.
 */
TNC_Result TNC_TNCS_ReportMessageTypesLong(TNC_IMVID imv_id,
									   TNC_VendorIDList supported_vids,
									   TNC_MessageSubtypeList supported_subtypes,
									   TNC_UInt32 type_count)
{
	if (!tnc->imvs->is_registered(tnc->imvs, imv_id))
	{
		DBG1(DBG_TNC, "ignoring ReportMessageTypesLong() from unregistered IMV %u",
					   imv_id);
		return TNC_RESULT_INVALID_PARAMETER;
	}
	return tnc->imvs->set_message_types_long(tnc->imvs, imv_id, supported_vids,
											 supported_subtypes, type_count);
}

/**
 * Called by the IMV to ask a TNCS to retry an Integrity Check Handshake
 */
TNC_Result TNC_TNCS_RequestHandshakeRetry(TNC_IMVID imv_id,
										  TNC_ConnectionID connection_id,
										  TNC_RetryReason reason)
{
	if (!tnc->imvs->is_registered(tnc->imvs, imv_id))
	{
		DBG1(DBG_TNC, "ignoring RequestHandshakeRetry() from unregistered IMV %u",
					   imv_id);
		return TNC_RESULT_INVALID_PARAMETER;
	}
	return tnc->tnccs->request_handshake_retry(tnc->tnccs, FALSE, imv_id,
											   connection_id, reason);
}

/**
 * Called by the IMV when an IMV-IMC message is to be sent
 */
TNC_Result TNC_TNCS_SendMessage(TNC_IMVID imv_id,
								TNC_ConnectionID connection_id,
								TNC_BufferReference msg,
								TNC_UInt32 msg_len,
								TNC_MessageType msg_type)
{
	TNC_VendorID msg_vid;
	TNC_MessageSubtype msg_subtype;

	if (!tnc->imvs->is_registered(tnc->imvs, imv_id))
	{
		DBG1(DBG_TNC, "ignoring SendMessage() from unregistered IMV %u",
					   imv_id);
		return TNC_RESULT_INVALID_PARAMETER;
	}
	msg_vid = (msg_type >> 8) & TNC_VENDORID_ANY;
	msg_subtype = msg_type & TNC_SUBTYPE_ANY;

	return tnc->tnccs->send_message(tnc->tnccs, TNC_IMCID_ANY, imv_id,
						connection_id, 0, msg, msg_len, msg_vid, msg_subtype);
}

/**
 * Called by the IMV when an IMV-IMC message is to be sent over IF-TNCCS 2.0
 */
TNC_Result TNC_TNCS_SendMessageLong(TNC_IMVID imv_id,
									TNC_ConnectionID connection_id,
									TNC_UInt32 msg_flags,
									TNC_BufferReference msg,
									TNC_UInt32 msg_len,
									TNC_VendorID msg_vid,
									TNC_MessageSubtype msg_subtype,
	 						 		TNC_UInt32 imc_id)
{
	if (!tnc->imvs->is_registered(tnc->imvs, imv_id))
	{
		DBG1(DBG_TNC, "ignoring SendMessageLong() from unregistered IMV %u",
					   imv_id);
		return TNC_RESULT_INVALID_PARAMETER;
	}
	return tnc->tnccs->send_message(tnc->tnccs, imc_id, imv_id, connection_id,
								msg_flags, msg, msg_len, msg_vid, msg_subtype);
}

/**
 * Called by the IMV to deliver its IMV Action Recommendation and IMV Evaluation
 * Result to the TNCS
 */
TNC_Result TNC_TNCS_ProvideRecommendation(TNC_IMVID imv_id,
								TNC_ConnectionID connection_id,
								TNC_IMV_Action_Recommendation recommendation,
								TNC_IMV_Evaluation_Result evaluation)
{
	if (!tnc->imvs->is_registered(tnc->imvs, imv_id))
	{
		DBG1(DBG_TNC, "ignoring ProvideRecommendation() from unregistered IMV %u",
					   imv_id);
		return TNC_RESULT_INVALID_PARAMETER;
	}
	return tnc->tnccs->provide_recommendation(tnc->tnccs, imv_id, connection_id,
											  recommendation, evaluation);
}

/**
 * Called by the IMV to get the value of an attribute associated with a
 * connection or with the TNCS as a whole.
 */
TNC_Result TNC_TNCS_GetAttribute(TNC_IMVID imv_id,
								 TNC_ConnectionID connection_id,
								 TNC_AttributeID attribute_id,
								 TNC_UInt32 buffer_len,
								 TNC_BufferReference buffer,
								 TNC_UInt32 *out_value_len)
{
	if (!tnc->imvs->is_registered(tnc->imvs, imv_id))
	{
		DBG1(DBG_TNC, "ignoring GetAttribute() from unregistered IMV %u",
					   imv_id);
		return TNC_RESULT_INVALID_PARAMETER;
	}
	return tnc->tnccs->get_attribute(tnc->tnccs, FALSE, imv_id, connection_id,
							attribute_id, buffer_len, buffer, out_value_len);
}

/**
 * Called by the IMV to set the value of an attribute associated with a
 * connection or with the TNCS as a whole.
 */
TNC_Result TNC_TNCS_SetAttribute(TNC_IMVID imv_id,
								 TNC_ConnectionID connection_id,
								 TNC_AttributeID attribute_id,
								 TNC_UInt32 buffer_len,
								 TNC_BufferReference buffer)
{
	if (!tnc->imvs->is_registered(tnc->imvs, imv_id))
	{
		DBG1(DBG_TNC, "ignoring SetAttribute() from unregistered IMV %u",
					   imv_id);
		return TNC_RESULT_INVALID_PARAMETER;
	}
	return tnc->tnccs->set_attribute(tnc->tnccs, FALSE, imv_id, connection_id,
									 attribute_id, buffer_len, buffer);
}

/**
 * Called by the IMV when it wants to reserve an additional IMV ID for itself
 */
TNC_Result TNC_TNCS_ReserveAdditionalIMVID(TNC_IMVID imv_id, TNC_UInt32 *new_id)
{
	if (tnc->imvs->reserve_id(tnc->imvs, imv_id, new_id))
	{
		return TNC_RESULT_SUCCESS;
	}
	DBG1(DBG_TNC, "ignoring ReserveAdditionalIMVID() from unregistered IMV %u",
				   imv_id);
	return TNC_RESULT_INVALID_PARAMETER;
}

/**
 * Called by the IMV when it needs a function pointer
 */
TNC_Result TNC_TNCS_BindFunction(TNC_IMVID id,
								 char *function_name,
								 void **function_pointer)
{
	if (streq(function_name, "TNC_TNCS_ReportMessageTypes"))
	{
		*function_pointer = (void*)TNC_TNCS_ReportMessageTypes;
	}
	else if (streq(function_name, "TNC_TNCS_ReportMessageTypesLong"))
	{
		*function_pointer = (void*)TNC_TNCS_ReportMessageTypesLong;
	}
	else if (streq(function_name, "TNC_TNCS_RequestHandshakeRetry"))
	{
		*function_pointer = (void*)TNC_TNCS_RequestHandshakeRetry;
	}
	else if (streq(function_name, "TNC_TNCS_SendMessage"))
	{
		*function_pointer = (void*)TNC_TNCS_SendMessage;
	}
	else if (streq(function_name, "TNC_TNCS_SendMessageLong"))
	{
		*function_pointer = (void*)TNC_TNCS_SendMessageLong;
	}
	else if (streq(function_name, "TNC_TNCS_ProvideRecommendation"))
	{
		*function_pointer = (void*)TNC_TNCS_ProvideRecommendation;
	}
	else if (streq(function_name, "TNC_TNCS_GetAttribute"))
	{
		*function_pointer = (void*)TNC_TNCS_GetAttribute;
	}
	else if (streq(function_name, "TNC_TNCS_SetAttribute"))
	{
		*function_pointer = (void*)TNC_TNCS_SetAttribute;
	}
    else if (streq(function_name, "TNC_TNCS_ReserveAdditionalIMVID"))
	{
		*function_pointer = (void*)TNC_TNCS_ReserveAdditionalIMVID;
	}
	else
	{
		return TNC_RESULT_INVALID_PARAMETER;
	}
	return TNC_RESULT_SUCCESS;
}
