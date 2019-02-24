/*
 * Copyright (C) 2010-2015 Andreas Steffen
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

#define _GNU_SOURCE /* for asprintf() */

#include "tnc_tnccs_manager.h"

#include <tnc/tnc.h>
#include <tnc/imv/imv_manager.h>
#include <tnc/imc/imc_manager.h>
#include <tnc/imv/imv_manager.h>

#include <tncif_identity.h>

#include <tls.h>

#include <utils/debug.h>
#include <pen/pen.h>
#include <bio/bio_writer.h>
#include <collections/linked_list.h>
#include <threading/rwlock.h>

#include <stdio.h>

typedef struct private_tnc_tnccs_manager_t private_tnc_tnccs_manager_t;
typedef struct tnccs_entry_t tnccs_entry_t;
typedef struct tnccs_connection_entry_t tnccs_connection_entry_t;

/**
 * TNCCS constructor entry
 */
struct tnccs_entry_t {

	/**
	 * TNCCS protocol type
	 */
	tnccs_type_t type;

	/**
	 * constructor function to create instance
	 */
	tnccs_constructor_t constructor;
};

/**
 * TNCCS connection entry
 */
struct tnccs_connection_entry_t {

	/**
	 * TNCCS connection ID
	 */
	TNC_ConnectionID id;

	/**
	 * TNCCS protocol type
	 */
	tnccs_type_t type;

	/**
	 * TNCCS instance
	 */
	tnccs_t *tnccs;

	/**
	 * TNCCS send message function
	 */
	tnccs_send_message_t send_message;

	/**
	 * TNCCS request handshake retry flag
	 */
	bool *request_handshake_retry;

	/**
	 * Maximum size of a PA-TNC message
	 */
	uint32_t max_msg_len;

	/**
	 * collection of IMV recommendations
	 */
	recommendations_t *recs;
};

/**
 * private data of tnc_tnccs_manager
 */
struct private_tnc_tnccs_manager_t {

	/**
	 * public functions
	 */
	tnccs_manager_t public;

	/**
	 * list of TNCCS protocol entries
	 */
	linked_list_t *protocols;

	/**
	 * rwlock to lock the TNCCS protocol entries
	 */
	rwlock_t *protocol_lock;

	/**
	 * connection ID counter
	 */
	TNC_ConnectionID connection_id;

	/**
	 * list of TNCCS connection entries
	 */
	linked_list_t *connections;

	/**
	 * rwlock to lock TNCCS connection entries
	 */
	rwlock_t *connection_lock;

};

METHOD(tnccs_manager_t, add_method, void,
	private_tnc_tnccs_manager_t *this, tnccs_type_t type,
	tnccs_constructor_t constructor)
{
	tnccs_entry_t *entry;

	entry = malloc_thing(tnccs_entry_t);
	entry->type = type;
	entry->constructor = constructor;

	this->protocol_lock->write_lock(this->protocol_lock);
	this->protocols->insert_last(this->protocols, entry);
	this->protocol_lock->unlock(this->protocol_lock);
}

METHOD(tnccs_manager_t, remove_method, void,
	private_tnc_tnccs_manager_t *this, tnccs_constructor_t constructor)
{
	enumerator_t *enumerator;
	tnccs_entry_t *entry;

	this->protocol_lock->write_lock(this->protocol_lock);
	enumerator = this->protocols->create_enumerator(this->protocols);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (constructor == entry->constructor)
		{
			this->protocols->remove_at(this->protocols, enumerator);
			free(entry);
		}
	}
	enumerator->destroy(enumerator);
	this->protocol_lock->unlock(this->protocol_lock);
}

METHOD(tnccs_manager_t, create_instance, tnccs_t*,
	private_tnc_tnccs_manager_t *this, tnccs_type_t type, bool is_server,
	identification_t *server_id, identification_t *peer_id, host_t *server_ip,
	host_t *peer_ip, tnc_ift_type_t transport, tnccs_cb_t cb)
{
	enumerator_t *enumerator;
	tnccs_entry_t *entry;
	tnccs_t *protocol = NULL;

	this->protocol_lock->read_lock(this->protocol_lock);
	enumerator = this->protocols->create_enumerator(this->protocols);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (type == entry->type)
		{
			protocol = entry->constructor(is_server, server_id, peer_id,
										  server_ip, peer_ip, transport, cb);
			if (protocol)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->protocol_lock->unlock(this->protocol_lock);

	return protocol;
}

METHOD(tnccs_manager_t, create_connection, TNC_ConnectionID,
	private_tnc_tnccs_manager_t *this, tnccs_type_t type, tnccs_t *tnccs,
	tnccs_send_message_t send_message, bool* request_handshake_retry,
	uint32_t max_msg_len, recommendations_t **recs)
{
	tnccs_connection_entry_t *entry;

	entry = malloc_thing(tnccs_connection_entry_t);
	entry->type = type;
	entry->tnccs = tnccs;
	entry->send_message = send_message;
	entry->request_handshake_retry = request_handshake_retry;
	entry->max_msg_len = max_msg_len;
	if (recs)
	{
		/* we assume a TNC Server needing recommendations from IMVs */
		if (!tnc->imvs)
		{
			DBG1(DBG_TNC, "no IMV manager available!");
			free(entry);
			return 0;
		}
		entry->recs = tnc->imvs->create_recommendations(tnc->imvs);
		*recs = entry->recs;
	}
	else
	{
		/* we assume a TNC Client */
		if (!tnc->imcs)
		{
			DBG1(DBG_TNC, "no IMC manager available!");
			free(entry);
			return 0;
		}
		entry->recs = NULL;
	}
	this->connection_lock->write_lock(this->connection_lock);
	entry->id = ++this->connection_id;
	this->connections->insert_last(this->connections, entry);
	this->connection_lock->unlock(this->connection_lock);

	DBG1(DBG_TNC, "assigned TNCCS Connection ID %u", entry->id);
	return entry->id;
}

METHOD(tnccs_manager_t, remove_connection, void,
	private_tnc_tnccs_manager_t *this, TNC_ConnectionID id, bool is_server)
{
	enumerator_t *enumerator;
	tnccs_connection_entry_t *entry;

	if (is_server)
	{
		if (tnc->imvs)
		{
			tnc->imvs->notify_connection_change(tnc->imvs, id,
										TNC_CONNECTION_STATE_DELETE);
		}
	}
	else
	{
		if (tnc->imcs)
		{
			tnc->imcs->notify_connection_change(tnc->imcs, id,
										TNC_CONNECTION_STATE_DELETE);
		}
	}

	this->connection_lock->write_lock(this->connection_lock);
	enumerator = this->connections->create_enumerator(this->connections);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (id == entry->id)
		{
			this->connections->remove_at(this->connections, enumerator);
			if (entry->recs)
			{
				entry->recs->destroy(entry->recs);
			}
			free(entry);
			DBG1(DBG_TNC, "removed TNCCS Connection ID %u", id);
		}
	}
	enumerator->destroy(enumerator);
	this->connection_lock->unlock(this->connection_lock);
}

METHOD(tnccs_manager_t,	request_handshake_retry, TNC_Result,
	private_tnc_tnccs_manager_t *this, bool is_imc, TNC_UInt32 imcv_id,
													TNC_ConnectionID id,
													TNC_RetryReason reason)
{
	enumerator_t *enumerator;
	tnccs_connection_entry_t *entry;

	if (id == TNC_CONNECTIONID_ANY)
	{
		DBG2(DBG_TNC, "%s %u requests handshake retry for all connections "
					  "(reason: %u)", is_imc ? "IMC":"IMV", reason);
	}
	else
	{
		DBG2(DBG_TNC, "%s %u requests handshake retry for Connection ID %u "
					  "(reason: %u)", is_imc ? "IMC":"IMV", imcv_id, id, reason);
	}
	this->connection_lock->read_lock(this->connection_lock);
	enumerator = this->connections->create_enumerator(this->connections);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (id == TNC_CONNECTIONID_ANY || id == entry->id)
		{
			*entry->request_handshake_retry = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->connection_lock->unlock(this->connection_lock);

	return TNC_RESULT_SUCCESS;
}

METHOD(tnccs_manager_t, send_message, TNC_Result,
	private_tnc_tnccs_manager_t *this, TNC_IMCID imc_id, TNC_IMVID imv_id,
									   TNC_ConnectionID id,
									   TNC_UInt32 msg_flags,
									   TNC_BufferReference msg,
									   TNC_UInt32 msg_len,
									   TNC_VendorID msg_vid,
									   TNC_MessageSubtype msg_subtype)

{
	enumerator_t *enumerator;
	tnccs_connection_entry_t *entry;
	tnccs_send_message_t send_message = NULL;
	tnccs_t *tnccs = NULL;

	if (msg_vid == TNC_VENDORID_ANY || msg_subtype == TNC_SUBTYPE_ANY)
	{
		DBG1(DBG_TNC, "not sending message of invalid type 0x%02x/0x%08x",
					   msg_vid, msg_subtype);
		return TNC_RESULT_INVALID_PARAMETER;
	}

	this->connection_lock->read_lock(this->connection_lock);
	enumerator = this->connections->create_enumerator(this->connections);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (id == entry->id)
		{
			tnccs = entry->tnccs;
			send_message = entry->send_message;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->connection_lock->unlock(this->connection_lock);

	if (tnccs && send_message)
	{
		return send_message(tnccs, imc_id, imv_id, msg_flags, msg, msg_len,
							msg_vid, msg_subtype);
	}
	return TNC_RESULT_FATAL;
}

METHOD(tnccs_manager_t, provide_recommendation, TNC_Result,
	private_tnc_tnccs_manager_t *this, TNC_IMVID imv_id,
									   TNC_ConnectionID id,
									   TNC_IMV_Action_Recommendation rec,
									   TNC_IMV_Evaluation_Result eval)
{
	enumerator_t *enumerator;
	tnccs_connection_entry_t *entry;
	recommendations_t *recs = NULL;

	this->connection_lock->read_lock(this->connection_lock);
	enumerator = this->connections->create_enumerator(this->connections);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (id == entry->id)
		{
			recs = entry->recs;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->connection_lock->unlock(this->connection_lock);

	if (recs)
	{
		recs->provide_recommendation(recs, imv_id, rec, eval);
		return TNC_RESULT_SUCCESS;
	 }
	return TNC_RESULT_FATAL;
}

/**
 * Write the value of a boolean attribute into the buffer
 */
static TNC_Result bool_attribute(TNC_UInt32 buffer_len,
								 TNC_BufferReference buffer,
								 TNC_UInt32 *value_len,
								 bool value)
{
	*value_len = 1;

	if (buffer && buffer_len > 0)
	{
		*buffer = value ? 0x01 : 0x00;
		return TNC_RESULT_SUCCESS;
	}
	else
	{
		return TNC_RESULT_INVALID_PARAMETER;
	}
}

/**
 * Write the value of an uint32_t attribute into the buffer
 */
static TNC_Result uint_attribute(TNC_UInt32 buffer_len,
								 TNC_BufferReference buffer,
								 TNC_UInt32 *value_len,
								 uint32_t value)
{
	*value_len = sizeof(uint32_t);

	if (buffer && buffer_len >= *value_len)
	{
		htoun32(buffer, value);
		return TNC_RESULT_SUCCESS;
	}
	else
	{
		return TNC_RESULT_INVALID_PARAMETER;
	}
}

/**
 * Write the value of string attribute into the buffer
 */
static TNC_Result str_attribute(TNC_UInt32 buffer_len,
								 TNC_BufferReference buffer,
								 TNC_UInt32 *value_len,
								 char *value)
{
	*value_len = 1 + strlen(value);

	if (buffer && buffer_len >= *value_len)
	{
		snprintf(buffer, buffer_len, "%s", value);
		return TNC_RESULT_SUCCESS;
	}
	else
	{
		return TNC_RESULT_INVALID_PARAMETER;
	}
}

/**
 * Write the value of a TNC identity list into the buffer
 */
static TNC_Result identity_attribute(TNC_UInt32 buffer_len,
									 TNC_BufferReference buffer,
									 TNC_UInt32 *value_len,
									 linked_list_t *list)
{
	bio_writer_t *writer;
	enumerator_t *enumerator;
	uint32_t count;
	chunk_t value;
	tncif_identity_t *tnc_id;
	TNC_Result result = TNC_RESULT_INVALID_PARAMETER;

	count = list->get_count(list);
	writer = bio_writer_create(4 + TNCIF_IDENTITY_MIN_SIZE * count);
	writer->write_uint32(writer, count);

	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &tnc_id))
	{
		tnc_id->build(tnc_id, writer);
	}
	enumerator->destroy(enumerator);

	value = writer->get_buf(writer);
	*value_len = value.len;
	if (buffer && buffer_len >= value.len)
	{
		memcpy(buffer, value.ptr, value.len);
		result = TNC_RESULT_SUCCESS;
	}
	writer->destroy(writer);

	return result;
}

METHOD(tnccs_manager_t, get_attribute, TNC_Result,
	private_tnc_tnccs_manager_t *this, bool is_imc,
									   TNC_UInt32 imcv_id,
									   TNC_ConnectionID id,
									   TNC_AttributeID attribute_id,
									   TNC_UInt32 buffer_len,
									   TNC_BufferReference buffer,
									   TNC_UInt32 *value_len)
{
	enumerator_t *enumerator;
	tnccs_connection_entry_t *entry;
	bool attribute_match = FALSE, entry_found = FALSE;

	if (is_imc)
	{
		switch (attribute_id)
		{
			/* these attributes are unsupported */
			case TNC_ATTRIBUTEID_SOHR:
			case TNC_ATTRIBUTEID_SSOHR:
				return TNC_RESULT_INVALID_PARAMETER;

			/* these attributes are supported */
			case TNC_ATTRIBUTEID_PRIMARY_IMC_ID:
				attribute_match = TRUE;
				break;

			/* these attributes are yet to be matched */
			default:
				break;
		}
	}
	else
	{
		switch (attribute_id)
		{
			/* these attributes are unsupported or invalid */
			case TNC_ATTRIBUTEID_REASON_STRING:
			case TNC_ATTRIBUTEID_REASON_LANGUAGE:
			case TNC_ATTRIBUTEID_SOH:
			case TNC_ATTRIBUTEID_SSOH:
				return TNC_RESULT_INVALID_PARAMETER;

			/* these attributes are supported */
			case TNC_ATTRIBUTEID_PRIMARY_IMV_ID:
			case TNC_ATTRIBUTEID_AR_IDENTITIES:
				attribute_match = TRUE;
				break;

			/* these attributes are yet to be matched */
			default:
				break;
		}
	}

	if (!attribute_match)
	{
		switch (attribute_id)
		{
			/* these attributes are supported */
			case TNC_ATTRIBUTEID_PREFERRED_LANGUAGE:
			case TNC_ATTRIBUTEID_MAX_ROUND_TRIPS:
			case TNC_ATTRIBUTEID_MAX_MESSAGE_SIZE:
			case TNC_ATTRIBUTEID_HAS_LONG_TYPES:
			case TNC_ATTRIBUTEID_HAS_EXCLUSIVE:
			case TNC_ATTRIBUTEID_HAS_SOH:
			case TNC_ATTRIBUTEID_IFTNCCS_PROTOCOL:
			case TNC_ATTRIBUTEID_IFTNCCS_VERSION:
			case TNC_ATTRIBUTEID_IFT_PROTOCOL:
			case TNC_ATTRIBUTEID_IFT_VERSION:
				break;

			/* these attributes are unsupported or unknown */
			case TNC_ATTRIBUTEID_DHPN:
			case TNC_ATTRIBUTEID_TLS_UNIQUE:
			default:
				return TNC_RESULT_INVALID_PARAMETER;
		}
	}

	/* attributes specific to the TNCC or TNCS are unsupported */
	if (id == TNC_CONNECTIONID_ANY)
	{
		return TNC_RESULT_INVALID_PARAMETER;
	}

	this->connection_lock->read_lock(this->connection_lock);
	enumerator = this->connections->create_enumerator(this->connections);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (id == entry->id)
		{
			entry_found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->connection_lock->unlock(this->connection_lock);

	if (!entry_found)
	{
		return TNC_RESULT_INVALID_PARAMETER;
	}

	switch (attribute_id)
	{
		case TNC_ATTRIBUTEID_PREFERRED_LANGUAGE:
		{
			recommendations_t *recs;
			chunk_t pref_lang;

			recs = entry->recs;
			if (!recs)
			{
				return TNC_RESULT_INVALID_PARAMETER;
			}
			pref_lang = recs->get_preferred_language(recs);
			if (pref_lang.len == 0)
			{
				return TNC_RESULT_INVALID_PARAMETER;
			}
			*value_len = pref_lang.len;
			if (buffer && buffer_len >= pref_lang.len)
			{
				memcpy(buffer, pref_lang.ptr, pref_lang.len);
			}
			return TNC_RESULT_SUCCESS;
		}
		case TNC_ATTRIBUTEID_MAX_ROUND_TRIPS:
			return uint_attribute(buffer_len, buffer, value_len,
								  0xffffffff);
		case TNC_ATTRIBUTEID_MAX_MESSAGE_SIZE:
			return uint_attribute(buffer_len, buffer, value_len,
								  entry->max_msg_len);
		case TNC_ATTRIBUTEID_HAS_LONG_TYPES:
		case TNC_ATTRIBUTEID_HAS_EXCLUSIVE:
			return bool_attribute(buffer_len, buffer, value_len,
								  entry->type == TNCCS_2_0);
		case TNC_ATTRIBUTEID_HAS_SOH:
			return bool_attribute(buffer_len, buffer, value_len,
								  entry->type == TNCCS_SOH);
		case TNC_ATTRIBUTEID_IFTNCCS_PROTOCOL:
		{
			char *protocol;

			switch (entry->type)
			{
				case TNCCS_1_1:
				case TNCCS_2_0:
					protocol = "IF-TNCCS";
					break;
				case TNCCS_SOH:
					protocol = "IF-TNCCS-SOH";
					break;
				default:
				return TNC_RESULT_INVALID_PARAMETER;
			}
			return str_attribute(buffer_len, buffer, value_len, protocol);
		}
		case TNC_ATTRIBUTEID_IFTNCCS_VERSION:
		{
			char *version;

			switch (entry->type)
			{
				case TNCCS_1_1:
					version = "1.1";
					break;
				case TNCCS_2_0:
					version = "2.0";
					break;
				case TNCCS_SOH:
					version = "1.0";
					break;
				default:
					return TNC_RESULT_INVALID_PARAMETER;
			}
			return str_attribute(buffer_len, buffer, value_len, version);
		}
		case TNC_ATTRIBUTEID_IFT_PROTOCOL:
		{
			char *protocol;

			switch (entry->tnccs->get_transport(entry->tnccs))
			{
				case TNC_IFT_EAP_1_0:
				case TNC_IFT_EAP_1_1:
				case TNC_IFT_EAP_2_0:
					protocol = "IF-T for Tunneled EAP";
					break;
				case TNC_IFT_TLS_1_0:
				case TNC_IFT_TLS_2_0:
					protocol = "IF-T for TLS";
					break;
				default:
					return TNC_RESULT_INVALID_PARAMETER;
			}
			return str_attribute(buffer_len, buffer, value_len, protocol);
		}
 		case TNC_ATTRIBUTEID_IFT_VERSION:
		{
			char *version;

			switch (entry->tnccs->get_transport(entry->tnccs))
			{
				case TNC_IFT_EAP_1_0:
				case TNC_IFT_TLS_1_0:
					version = "1.0";
					break;
				case TNC_IFT_EAP_1_1:
					version = "1.1";
					break;
				case TNC_IFT_EAP_2_0:
				case TNC_IFT_TLS_2_0:
					version = "2.0";
					break;
				default:
					return TNC_RESULT_INVALID_PARAMETER;
			}
			return str_attribute(buffer_len, buffer, value_len, version);
		}
		case TNC_ATTRIBUTEID_AR_IDENTITIES:
		{
			linked_list_t *list;
			identification_t *peer_id;
			host_t *peer_ip;
			tnccs_t *tnccs;
			tncif_identity_t *tnc_id;
			uint32_t id_type, subject_type;
			chunk_t id_value;
			char *id_str;
			TNC_Result result;

			list = linked_list_create();
			tnccs = entry->tnccs;

			peer_id = tnccs->tls.is_server(&tnccs->tls) ?
					tnccs->tls.get_peer_id(&tnccs->tls) :
					tnccs->tls.get_server_id(&tnccs->tls);
			if (peer_id)
			{
				switch (peer_id->get_type(peer_id))
				{
					case ID_IPV4_ADDR:
						id_type = TNC_ID_IPV4_ADDR;
						subject_type = TNC_SUBJECT_MACHINE;
						break;
					case ID_IPV6_ADDR:
						id_type = TNC_ID_IPV6_ADDR;
						subject_type = TNC_SUBJECT_MACHINE;
						break;
					case ID_FQDN:
						id_type = TNC_ID_USERNAME;
						subject_type = TNC_SUBJECT_USER;
						break;
					case ID_RFC822_ADDR:
						id_type = TNC_ID_EMAIL_ADDR;
						subject_type = TNC_SUBJECT_USER;
						break;
					case ID_DER_ASN1_DN:
						id_type = TNC_ID_X500_DN;
						subject_type = TNC_SUBJECT_USER;
						break;
					default:
						id_type = TNC_ID_UNKNOWN;
						subject_type = TNC_SUBJECT_UNKNOWN;
				}
				if (id_type != TNC_ID_UNKNOWN &&
					asprintf(&id_str, "%Y", peer_id) >= 0)
				{
					id_value = chunk_from_str(id_str);
					tnc_id = tncif_identity_create(
								pen_type_create(PEN_TCG, id_type), id_value,
								pen_type_create(PEN_TCG, subject_type),
								pen_type_create(PEN_TCG,
												tnccs->get_auth_type(tnccs)));
					list->insert_last(list, tnc_id);
				}
			}

			peer_ip = tnccs->tls.is_server(&tnccs->tls) ?
					tnccs->get_peer_ip(tnccs) :
					tnccs->get_server_ip(tnccs);
			if (peer_ip)
			{
				switch (peer_ip->get_family(peer_ip))
				{
					case AF_INET:
						id_type = TNC_ID_IPV4_ADDR;
						break;
					case AF_INET6:
						id_type = TNC_ID_IPV6_ADDR;
						break;
					default:
						id_type = TNC_ID_UNKNOWN;
				}

				if (id_type != TNC_ID_UNKNOWN &&
					asprintf(&id_str, "%H", peer_ip) >= 0)
				{
					id_value = chunk_from_str(id_str);
					tnc_id = tncif_identity_create(
								pen_type_create(PEN_TCG, id_type), id_value,
								pen_type_create(PEN_TCG, TNC_SUBJECT_MACHINE),
								pen_type_create(PEN_TCG, TNC_AUTH_UNKNOWN));
					list->insert_last(list, tnc_id);
				}
			}
			result = identity_attribute(buffer_len, buffer, value_len, list);
			list->destroy_offset(list, offsetof(tncif_identity_t, destroy));
			return result;
		}
		default:
			return TNC_RESULT_INVALID_PARAMETER;
	 }
}

METHOD(tnccs_manager_t, set_attribute, TNC_Result,
	private_tnc_tnccs_manager_t *this, bool is_imc,
									   TNC_UInt32 imcv_id,
									   TNC_ConnectionID id,
									   TNC_AttributeID attribute_id,
									   TNC_UInt32 buffer_len,
									   TNC_BufferReference buffer)
{
	enumerator_t *enumerator;
	tnccs_connection_entry_t *entry;
	recommendations_t *recs = NULL;

	if (is_imc || id == TNC_CONNECTIONID_ANY ||
		(attribute_id != TNC_ATTRIBUTEID_REASON_STRING &&
		 attribute_id != TNC_ATTRIBUTEID_REASON_LANGUAGE))
	{
		return TNC_RESULT_INVALID_PARAMETER;
	}

	this->connection_lock->read_lock(this->connection_lock);
	enumerator = this->connections->create_enumerator(this->connections);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (id == entry->id)
		{
			recs = entry->recs;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->connection_lock->unlock(this->connection_lock);

	if (recs)
	{
		chunk_t attribute = { buffer, buffer_len };

		if (attribute_id == TNC_ATTRIBUTEID_REASON_STRING)
		{
			return recs->set_reason_string(recs, imcv_id, attribute);
		}
		else
		{
			return recs->set_reason_language(recs, imcv_id, attribute);
		}
	}
	return TNC_RESULT_INVALID_PARAMETER;
}

METHOD(tnccs_manager_t, destroy, void,
	private_tnc_tnccs_manager_t *this)
{
	this->protocols->destroy_function(this->protocols, free);
	this->protocol_lock->destroy(this->protocol_lock);
	this->connections->destroy_function(this->connections, free);
	this->connection_lock->destroy(this->connection_lock);
	free(this);
}

/*
 * See header
 */
tnccs_manager_t *tnc_tnccs_manager_create()
{
	private_tnc_tnccs_manager_t *this;

	INIT(this,
			.public = {
				.add_method = _add_method,
				.remove_method = _remove_method,
				.create_instance = _create_instance,
				.create_connection = _create_connection,
				.remove_connection = _remove_connection,
				.request_handshake_retry = _request_handshake_retry,
				.send_message = _send_message,
				.provide_recommendation = _provide_recommendation,
				.get_attribute = _get_attribute,
				.set_attribute = _set_attribute,
				.destroy = _destroy,
			},
			.protocols = linked_list_create(),
			.connections = linked_list_create(),
			.protocol_lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
			.connection_lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}

