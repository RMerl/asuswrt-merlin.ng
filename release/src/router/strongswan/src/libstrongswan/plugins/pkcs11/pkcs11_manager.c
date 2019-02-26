/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "pkcs11_manager.h"

#include <utils/debug.h>
#include <collections/linked_list.h>
#include <threading/thread.h>

#include "pkcs11_library.h"

#include <processing/jobs/callback_job.h>

typedef struct private_pkcs11_manager_t private_pkcs11_manager_t;

/**
 * Private data of an pkcs11_manager_t object.
 */
struct private_pkcs11_manager_t {

	/**
	 * Public pkcs11_manager_t interface.
	 */
	pkcs11_manager_t public;

	/**
	 * List of loaded libraries, as lib_entry_t
	 */
	linked_list_t *libs;

	/**
	 * Slot event callback function
	 */
	pkcs11_manager_token_event_t cb;

	/**
	 * Slot event user data
	 */
	void *data;
};

/**
 * Entry for a loaded library
 */
typedef struct {
	/* back reference to this */
	private_pkcs11_manager_t *this;
	/* associated library path */
	char *path;
	/* loaded library */
	pkcs11_library_t *lib;
} lib_entry_t;

/**
 * Destroy a lib_entry_t
 */
static void lib_entry_destroy(lib_entry_t *entry)
{
	entry->lib->destroy(entry->lib);
	free(entry);
}

/**
 * Print supported mechanisms of a token in a slot
 */
static void print_mechs(lib_entry_t *entry, CK_SLOT_ID slot)
{
	enumerator_t *enumerator;
	CK_MECHANISM_TYPE type;
	CK_MECHANISM_INFO info;

	enumerator = entry->lib->create_mechanism_enumerator(entry->lib, slot);
	while (enumerator->enumerate(enumerator, &type, &info))
	{
		DBG2(DBG_CFG, "      %N %lu-%lu [ %s%s%s%s%s%s%s%s%s%s%s%s%s]",
			ck_mech_names, type,
			info.ulMinKeySize, info.ulMaxKeySize,
			info.flags & CKF_HW ? "HW " : "",
			info.flags & CKF_ENCRYPT ? "ENCR " : "",
			info.flags & CKF_DECRYPT ? "DECR " : "",
			info.flags & CKF_DIGEST ? "DGST " : "",
			info.flags & CKF_SIGN ? "SIGN " : "",
			info.flags & CKF_SIGN_RECOVER ? "SIGN_RCVR " : "",
			info.flags & CKF_VERIFY ? "VRFY " : "",
			info.flags & CKF_VERIFY_RECOVER ? "VRFY_RCVR " : "",
			info.flags & CKF_GENERATE ? "GEN " : "",
			info.flags & CKF_GENERATE_KEY_PAIR ? "GEN_KEY_PAIR " : "",
			info.flags & CKF_WRAP ? "WRAP " : "",
			info.flags & CKF_UNWRAP ? "UNWRAP " : "",
			info.flags & CKF_DERIVE ? "DERIVE " : "");
	}
	enumerator->destroy(enumerator);
}

/**
 * Handle a token
 */
static void handle_token(lib_entry_t *entry, CK_SLOT_ID slot)
{
	CK_TOKEN_INFO info;
	CK_RV rv;

	rv = entry->lib->f->C_GetTokenInfo(slot, &info);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_GetTokenInfo failed: %N", ck_rv_names, rv);
		return;
	}
	pkcs11_library_trim(info.label, sizeof(info.label));
	pkcs11_library_trim(info.manufacturerID, sizeof(info.manufacturerID));
	pkcs11_library_trim(info.model, sizeof(info.model));
	DBG1(DBG_CFG, "    %s (%s: %s)",
		 info.label, info.manufacturerID, info.model);

	print_mechs(entry, slot);
}

/**
 * Handle slot changes
 */
static void handle_slot(lib_entry_t *entry, CK_SLOT_ID slot, bool hot)
{
	CK_SLOT_INFO info;
	CK_RV rv;

	rv = entry->lib->f->C_GetSlotInfo(slot, &info);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_GetSlotInfo failed: %N", ck_rv_names, rv);
		return;
	}

	pkcs11_library_trim(info.slotDescription, sizeof(info.slotDescription));
	if (info.flags & CKF_TOKEN_PRESENT)
	{
		DBG1(DBG_CFG, "  found token in slot '%s':%lu (%s)",
			 entry->lib->get_name(entry->lib), slot, info.slotDescription);
		handle_token(entry, slot);
		if (hot)
		{
			entry->this->cb(entry->this->data, entry->lib, slot, TRUE);
		}
	}
	else
	{
		DBG1(DBG_CFG, "token removed from slot '%s':%lu (%s)",
			 entry->lib->get_name(entry->lib), slot, info.slotDescription);
		if (hot)
		{
			entry->this->cb(entry->this->data, entry->lib, slot, FALSE);
		}
	}
}

CALLBACK(dispatch_slot_events, job_requeue_t,
	lib_entry_t *entry)
{
	CK_SLOT_ID slot;
	CK_RV rv;

	rv = entry->lib->f->C_WaitForSlotEvent(0, &slot, NULL);
	if (rv == CKR_FUNCTION_NOT_SUPPORTED || rv == CKR_NO_EVENT)
	{
		DBG1(DBG_CFG, "module '%s' does not support hot-plugging, cancelled",
			 entry->lib->get_name(entry->lib));
		return JOB_REQUEUE_NONE;
	}
	if (rv == CKR_CRYPTOKI_NOT_INITIALIZED)
	{	/* C_Finalize called, abort */
		return JOB_REQUEUE_NONE;
	}
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "error in C_WaitForSlotEvent: %N", ck_rv_names, rv);
	}
	handle_slot(entry, slot, TRUE);

	return JOB_REQUEUE_DIRECT;
}

CALLBACK(cancel_events, bool,
	lib_entry_t *entry)
{
	/* it's possible other threads still use the API after this call, but we
	 * have no other way to return from C_WaitForSlotEvent() if we can't cancel
	 * the thread because libraries hold locks they don't release */
	entry->lib->f->C_Finalize(NULL);
	return TRUE;
}

/**
 * Get the slot list of a library
 */
static CK_SLOT_ID_PTR get_slot_list(pkcs11_library_t *p11, CK_ULONG *out)
{
	CK_SLOT_ID_PTR slots;
	CK_ULONG count;
	CK_RV rv;

	rv = p11->f->C_GetSlotList(TRUE, NULL, &count);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_GetSlotList() failed: %N", ck_rv_names, rv);
		return NULL;
	}
	if (count == 0)
	{
		return NULL;
	}
	slots = malloc(sizeof(CK_SLOT_ID) * count);
	rv = p11->f->C_GetSlotList(TRUE, slots, &count);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_GetSlotList() failed: %N", ck_rv_names, rv);
		free(slots);
		return NULL;
	}
	*out = count;
	return slots;
}

/**
 * Query the slots for tokens
 */
static void query_slots(lib_entry_t *entry)
{
	CK_ULONG count;
	CK_SLOT_ID_PTR slots;
	int i;

	slots = get_slot_list(entry->lib, &count);
	if (slots)
	{
		for (i = 0; i < count; i++)
		{
			handle_slot(entry, slots[i], FALSE);
		}
		free(slots);
	}
}

/**
 * Token enumerator
 */
typedef struct {
	/* implements enumerator */
	enumerator_t public;
	/* inner enumerator over PKCS#11 libraries */
	enumerator_t *inner;
	/* active library entry */
	lib_entry_t *entry;
	/* slot list with tokens */
	CK_SLOT_ID_PTR slots;
	/* number of slots */
	CK_ULONG count;
	/* current slot */
	int current;
} token_enumerator_t;

METHOD(enumerator_t, enumerate_token, bool,
	token_enumerator_t *this, va_list args)
{
	pkcs11_library_t **out;
	CK_SLOT_ID *slot;

	VA_ARGS_VGET(args, out, slot);

	if (this->current >= this->count)
	{
		free(this->slots);
		this->slots = NULL;
		this->current = 0;
	}
	while (!this->slots)
	{
		if (!this->inner->enumerate(this->inner, &this->entry))
		{
			return FALSE;
		}
		this->slots = get_slot_list(this->entry->lib, &this->count);
	}
	*out = this->entry->lib;
	*slot = this->slots[this->current++];
	return TRUE;
}

METHOD(enumerator_t, destroy_token, void,
	token_enumerator_t *this)
{
	this->inner->destroy(this->inner);
	free(this->slots);
	free(this);
}

METHOD(pkcs11_manager_t, create_token_enumerator, enumerator_t*,
	private_pkcs11_manager_t *this)
{
	token_enumerator_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate_token,
			.destroy = _destroy_token,
		},
		.inner = this->libs->create_enumerator(this->libs),
	);
	return &enumerator->public;
}

METHOD(pkcs11_manager_t, destroy, void,
	private_pkcs11_manager_t *this)
{
	this->libs->destroy_function(this->libs, (void*)lib_entry_destroy);
	free(this);
}

/**
 * See header
 */
pkcs11_manager_t *pkcs11_manager_create(pkcs11_manager_token_event_t cb,
										void *data)
{
	private_pkcs11_manager_t *this;
	enumerator_t *enumerator;
	lib_entry_t *entry;
	char *module;

	INIT(this,
		.public = {
			.create_token_enumerator = _create_token_enumerator,
			.destroy = _destroy,
		},
		.libs = linked_list_create(),
		.cb = cb,
		.data = data,
	);

	enumerator = lib->settings->create_section_enumerator(lib->settings,
										"%s.plugins.pkcs11.modules", lib->ns);
	while (enumerator->enumerate(enumerator, &module))
	{
		INIT(entry,
			.this = this,
		);

		entry->path = lib->settings->get_str(lib->settings,
				"%s.plugins.pkcs11.modules.%s.path", NULL, lib->ns, module);
		if (!entry->path)
		{
			DBG1(DBG_CFG, "PKCS11 module '%s' lacks library path", module);
			free(entry);
			continue;
		}
		entry->lib = pkcs11_library_create(module, entry->path,
						lib->settings->get_bool(lib->settings,
							"%s.plugins.pkcs11.modules.%s.os_locking",
							FALSE, lib->ns, module));
		if (!entry->lib)
		{
			free(entry);
			continue;
		}
		this->libs->insert_last(this->libs, entry);
	}
	enumerator->destroy(enumerator);

	enumerator = this->libs->create_enumerator(this->libs);
	while (enumerator->enumerate(enumerator, &entry))
	{
		query_slots(entry);
		lib->processor->queue_job(lib->processor,
			(job_t*)callback_job_create_with_prio(dispatch_slot_events,
						entry, NULL, cancel_events, JOB_PRIO_CRITICAL));
	}
	enumerator->destroy(enumerator);

	return &this->public;
}

