/*
 * Copyright (C) 2010-2013 Andreas Steffen,
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

#include "tnc_imv.h"

#ifndef WIN32
#include <dlfcn.h>
#endif

#include <tncif_pa_subtypes.h>

#include <utils/debug.h>
#include <library.h>
#include <collections/linked_list.h>
#include <threading/mutex.h>

typedef struct private_tnc_imv_t private_tnc_imv_t;

/**
 * Private data of an imv_t object.
 */
struct private_tnc_imv_t {

	/**
	 * Public members of imv_t.
	 */
	imv_t public;

	/**
	 * Name of loaded IMV
	 */
	char *name;

	/**
	 * Handle of loaded IMV
	 */
	void *handle;

	/**
	 * ID of loaded IMV
	 */
	TNC_IMVID id;

	/**
	 * List of additional IMV IDs
	 */
	linked_list_t *additional_ids;

	/**
	 * List of message types supported by IMV - Vendor ID part
	 */
	TNC_VendorIDList supported_vids;

	/**
	 * List of message types supported by IMV - Subtype part
	 */
	TNC_MessageSubtypeList supported_subtypes;

	/**
	 * Number of supported message types
	 */
	TNC_UInt32 type_count;

	/**
	 * mutex to lock the imv_t object
	 */
	mutex_t *mutex;
};

METHOD(imv_t, set_id, void,
	private_tnc_imv_t *this, TNC_IMVID id)
{
	this->id = id;
}

METHOD(imv_t, get_id, TNC_IMVID,
	private_tnc_imv_t *this)
{
	return this->id;
}

METHOD(imv_t, add_id, void,
	private_tnc_imv_t *this, TNC_IMVID id)
{
	TNC_IMVID *new_id;

	new_id = malloc_thing(TNC_IMVID);
	*new_id = id;
	this->additional_ids->insert_last(this->additional_ids, new_id);
}

METHOD(imv_t, has_id, bool,
	private_tnc_imv_t *this, TNC_IMVID id)
{
	enumerator_t *enumerator;
	TNC_IMVID *additional_id;
	bool found = FALSE;

	/* check primary IMV ID */
	if (id == this->id)
	{
		return TRUE;
	}

	/* return if there are no additional IMV IDs */
	if (this->additional_ids->get_count(this->additional_ids) == 0)
	{
		return FALSE;
	}

	/* check additional IMV IDs */
	enumerator = this->additional_ids->create_enumerator(this->additional_ids);
	while (enumerator->enumerate(enumerator, &additional_id))
	{
		if (id == *additional_id)
		{
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	return found;
}

METHOD(imv_t, get_name, char*,
	private_tnc_imv_t *this)
{
	return this->name;
}

METHOD(imv_t, set_message_types, void,
	private_tnc_imv_t *this, TNC_MessageTypeList supported_types,
							 TNC_UInt32 type_count)
{
	char buf[BUF_LEN];
	char *pos = buf;
	int len = sizeof(buf);
	int i, written;
	size_t size;
	TNC_VendorID vid;
	TNC_MessageSubtype subtype;
	enum_name_t *pa_subtype_names;

	/* lock the imv_t instance */
	this->mutex->lock(this->mutex);

	/* Free existing VendorID and MessageSubtype lists */
	free(this->supported_vids);
	this->supported_vids = NULL;
	free(this->supported_subtypes);
	this->supported_subtypes = NULL;

	/* Store the new MessageType list */
	this->type_count = type_count;
	if (type_count && supported_types)
	{
		size = type_count * sizeof(TNC_VendorID);
		this->supported_vids = malloc(size);
		size = type_count * sizeof(TNC_MessageSubtype);
		this->supported_subtypes = malloc(size);

		for (i = 0; i < type_count; i++)
		{
			vid = (supported_types[i] >> 8) & TNC_VENDORID_ANY;
			subtype = supported_types[i] & TNC_SUBTYPE_ANY;

			pa_subtype_names = get_pa_subtype_names(vid);
			if (pa_subtype_names)
			{
				written = snprintf(pos, len," '%N/%N' 0x%06x/0x%02x",
								   pen_names, vid, pa_subtype_names, subtype,
								   vid, subtype);
			}
			else
			{
				written = snprintf(pos, len," '%N' 0x%06x/0x%02x",
								   pen_names, vid, vid, subtype);
			}
			if (written >= len)
			{
				break;
			}
			pos += written;
			len -= written;

			this->supported_vids[i] = vid;
			this->supported_subtypes[i] = subtype;
		}
	}
	*pos = '\0';
	DBG2(DBG_TNC, "IMV %u supports %u message type%s:%s",
				  this->id, type_count, (type_count == 1) ? "":"s", buf);

	/* unlock the imv_t instance */
	this->mutex->unlock(this->mutex);
}

METHOD(imv_t, set_message_types_long, void,
	private_tnc_imv_t *this, TNC_VendorIDList supported_vids,
	TNC_MessageSubtypeList supported_subtypes, TNC_UInt32 type_count)
{
	char buf[BUF_LEN];
	char *pos = buf;
	int len = sizeof(buf);
	int i, written;
	size_t size;
	TNC_VendorID vid;
	TNC_MessageSubtype subtype;
	enum_name_t *pa_subtype_names;

	/* lock the imv_t instance */
	this->mutex->lock(this->mutex);

	/* Free existing VendorID and MessageSubtype lists */
	free(this->supported_vids);
	this->supported_vids = NULL;
	free(this->supported_subtypes);
	this->supported_subtypes = NULL;

	/* Store the new MessageType list */
	this->type_count = type_count;
	if (type_count && supported_vids && supported_subtypes)
	{
		size = type_count * sizeof(TNC_VendorID);
		this->supported_vids = malloc(size);
		memcpy(this->supported_vids, supported_vids, size);
		size = type_count * sizeof(TNC_MessageSubtype);
		this->supported_subtypes = malloc(size);
		memcpy(this->supported_subtypes, supported_subtypes, size);

		for (i = 0; i < type_count; i++)
		{
			vid = supported_vids[i];
			subtype = supported_subtypes[i];

			pa_subtype_names = get_pa_subtype_names(vid);
			if (pa_subtype_names)
			{
				written = snprintf(pos, len," '%N/%N' 0x%06x/0x%08x",
								   pen_names, vid, pa_subtype_names, subtype,
								   vid, subtype);
			}
			else
			{
				written = snprintf(pos, len," '%N' 0x%06x/0x%08x",
								   pen_names, vid, vid, subtype);
			}
			if (written >= len)
			{
				break;
			}
			pos += written;
			len -= written;
		}
	}
	*pos = '\0';
	DBG2(DBG_TNC, "IMV %u supports %u message type%s:%s",
				  this->id, type_count, (type_count == 1) ? "":"s", buf);

	/* unlock the imv_t instance */
	this->mutex->unlock(this->mutex);
}

METHOD(imv_t, type_supported, bool,
	private_tnc_imv_t *this, TNC_VendorID msg_vid, TNC_MessageSubtype msg_subtype)
{
	TNC_VendorID vid;
	TNC_MessageSubtype subtype;
	int i;

	for (i = 0; i < this->type_count; i++)
	{
		vid = this->supported_vids[i];
		subtype = this->supported_subtypes[i];

		if ((vid == TNC_VENDORID_ANY && subtype == TNC_SUBTYPE_ANY) ||
			(vid == msg_vid && (subtype == TNC_SUBTYPE_ANY ||
			 subtype == msg_subtype)))
		{
			return TRUE;
		}
	}
	return FALSE;
}

METHOD(imv_t, destroy, void,
	private_tnc_imv_t *this)
{
	if (this->handle && lib->settings->get_bool(lib->settings,
				"%s.plugins.tnc-imv.dlclose", TRUE, lib->ns))
	{
		dlclose(this->handle);
	}
	this->mutex->destroy(this->mutex);
	this->additional_ids->destroy_function(this->additional_ids, free);
	free(this->supported_vids);
	free(this->supported_subtypes);
	free(this->name);
	free(this);
}

/**
 * Generic constructor.
 */
static private_tnc_imv_t* tnc_imv_create_empty(char *name)
{
	private_tnc_imv_t *this;

	INIT(this,
		.public = {
			.set_id = _set_id,
			.get_id = _get_id,
			.add_id = _add_id,
			.has_id = _has_id,
			.get_name = _get_name,
			.set_message_types = _set_message_types,
			.set_message_types_long = _set_message_types_long,
			.type_supported = _type_supported,
			.destroy = _destroy,
		},
		.name = strdup(name),
		.additional_ids = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);

	return this;
}

/**
 * Described in header.
 */
imv_t* tnc_imv_create(char *name, char *path)
{
	private_tnc_imv_t *this;
	int flag = RTLD_LAZY;

	this = tnc_imv_create_empty(name);

	if (lib->settings->get_bool(lib->settings, "%s.dlopen_use_rtld_now",
								FALSE, lib->ns))
	{
		flag = RTLD_NOW;
	}
	this->handle = dlopen(path, flag);
	if (!this->handle)
	{
		DBG1(DBG_TNC, "IMV \"%s\" failed to load: %s", name, dlerror());
		destroy(this);
		return NULL;
	}

	this->public.initialize = dlsym(this->handle, "TNC_IMV_Initialize");
	if (!this->public.initialize)
	{
		DBG1(DBG_TNC, "could not resolve TNC_IMV_Initialize in %s: %s\n",
					   path, dlerror());
		destroy(this);
		return NULL;
	}
	this->public.notify_connection_change =
						dlsym(this->handle, "TNC_IMV_NotifyConnectionChange");
	this->public.solicit_recommendation =
						dlsym(this->handle, "TNC_IMV_SolicitRecommendation");
	if (!this->public.solicit_recommendation)
	{
		DBG1(DBG_TNC, "could not resolve TNC_IMV_SolicitRecommendation in %s: %s\n",
					   path, dlerror());
		destroy(this);
		return NULL;
	}
	this->public.receive_message =
						dlsym(this->handle, "TNC_IMV_ReceiveMessage");
	this->public.receive_message_long =
						dlsym(this->handle, "TNC_IMV_ReceiveMessageLong");
	this->public.batch_ending =
						dlsym(this->handle, "TNC_IMV_BatchEnding");
	this->public.terminate =
						dlsym(this->handle, "TNC_IMV_Terminate");
	this->public.provide_bind_function =
						dlsym(this->handle, "TNC_IMV_ProvideBindFunction");
	if (!this->public.provide_bind_function)
	{
		DBG1(DBG_TNC, "could not resolve TNC_IMV_ProvideBindFunction in %s: %s\n",
					  path, dlerror());
		destroy(this);
		return NULL;
	}

	return &this->public;
}

/**
 * Described in header.
 */
imv_t* tnc_imv_create_from_functions(char *name,
				TNC_IMV_InitializePointer initialize,
				TNC_IMV_NotifyConnectionChangePointer notify_connection_change,
				TNC_IMV_ReceiveMessagePointer receive_message,
				TNC_IMV_ReceiveMessageLongPointer receive_message_long,
				TNC_IMV_SolicitRecommendationPointer solicit_recommendation,
				TNC_IMV_BatchEndingPointer batch_ending,
				TNC_IMV_TerminatePointer terminate,
				TNC_IMV_ProvideBindFunctionPointer provide_bind_function)
{
	private_tnc_imv_t *this;

	this = tnc_imv_create_empty(name);

	this->public.initialize = initialize;
	this->public.notify_connection_change = notify_connection_change;
	this->public.receive_message = receive_message;
	this->public.receive_message_long = receive_message_long;
	this->public.solicit_recommendation = solicit_recommendation;
	this->public.batch_ending = batch_ending;
	this->public.terminate = terminate;
	this->public.provide_bind_function = provide_bind_function;

	return &this->public;
}
