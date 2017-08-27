/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup tkm-id-manager id manager
 * @{ @ingroup tkm
 */

#ifndef TKM_ID_MANAGER_H_
#define TKM_ID_MANAGER_H_

#include <library.h>

typedef struct tkm_id_manager_t tkm_id_manager_t;
typedef enum tkm_context_kind_t tkm_context_kind_t;

/**
 * Trusted key manager context kinds.
 */
enum tkm_context_kind_t {
	/** Nonce context */
	TKM_CTX_NONCE,
	/** Diffie-Hellman context */
	TKM_CTX_DH,
	/** Certificate chain context */
	TKM_CTX_CC,
	/** IKE SA context */
	TKM_CTX_ISA,
	/** Authenticated Endpoint context */
	TKM_CTX_AE,
	/** ESP SA context */
	TKM_CTX_ESA,

	/** helper to determine the number of elements in this enum */
	TKM_CTX_MAX,
};

/**
 * enum name for context_kind_t.
 */
extern enum_name_t *tkm_context_kind_names;

/**
 * TKM context limits.
 */
typedef uint64_t tkm_limits_t[TKM_CTX_MAX];

/**
 * The tkm id manager hands out context ids for all context kinds (e.g. nonce).
 */
struct tkm_id_manager_t {

	/**
	 * Acquire new context id for a specific context kind.
	 *
	 * @param kind			kind of context id to acquire
	 * @return				context id of given kind,
	 *						0 if no id of given kind could be acquired
	 */
	int (*acquire_id)(tkm_id_manager_t * const this,
					  const tkm_context_kind_t kind);

	/**
	 * Release a previously acquired context id.
	 *
	 * @param kind			kind of context id to release
	 * @param id			id to release
	 * @return				TRUE if id was released, FALSE otherwise
	 */
	bool (*release_id)(tkm_id_manager_t * const this,
					   const tkm_context_kind_t kind,
					   const int id);

	/**
	 * Destroy a tkm_id_manager instance.
	 */
	void (*destroy)(tkm_id_manager_t *this);

};

/**
 * Create a tkm id manager instance using the given context limits.
 */
tkm_id_manager_t *tkm_id_manager_create(const tkm_limits_t limits);

#endif /** TKM_ID_MANAGER_H_ @}*/
