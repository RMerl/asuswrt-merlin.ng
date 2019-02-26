/*
 * Copyright (C) 2013-2015 Andreas Steffen
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
 *
 * @defgroup imv_session_t imv_session
 * @{ @ingroup libimcv_imv
 */

#ifndef  IMV_SESSION_H_
#define  IMV_SESSION_H_

#include "imv_workitem.h"
#include "imv_os_info.h"

#include <tncifimv.h>
#include <library.h>

#include <time.h>

typedef struct imv_session_t imv_session_t;

/**
 * IMV session interface
 */
struct imv_session_t {

	/**
	 * Set unique session ID
	 *
	 * @param session_id	primary key into sessions table
	 * @param pid			primary key into products table
	 * @param did			Primary key into devices table
	 */
	void (*set_session_id)(imv_session_t *this, int session_id, int pid, int did);

	/**
	 * Get unique session ID
	 *
	 * @param pid			primary key into products table
	 * @param did			Primary key into devices table
	 * @return				primary key into sessions table
	 */
	int (*get_session_id)(imv_session_t *this, int *pid, int *did);

	/**
	 * Get TNCCS Connection ID
	 *
	 * @return				TNCCS Connection ID
	 */
	TNC_ConnectionID (*get_connection_id)(imv_session_t *this);

	/**
	 * Set session creation time
	 *
	 * @param created		Session creation time
	 */
	void (*set_creation_time)(imv_session_t *this, time_t created);

	/**
	 * Get session creation time
	 *
	 * @return				Session creation time
	 */
	time_t (*get_creation_time)(imv_session_t *this);

	/**
	 * Get list of Access Requestor identities
	 *
	 * @return				List of Access Requestor identities
	 */
	enumerator_t* (*create_ar_identities_enumerator)(imv_session_t *this);

	/**
	 * Get OS Information
	 *
	 * @return				OS info object
	 */
	imv_os_info_t* (*get_os_info)(imv_session_t *this);

	/**
	 * Set Device ID
	 *
	 * @param device_id		Device ID
	 */
	void (*set_device_id)(imv_session_t *this, chunk_t device_id);

	/**
	 * Get Device ID
	 *
	 * @param device_id		Device ID
	 * @return				TRUE if Device ID has already been set
	 */
	bool (*get_device_id)(imv_session_t *this, chunk_t *device_id);

	/**
	 * Set trust into Device ID
	 *
	 * @param trusted		TRUE if Device ID is trusted
	 */
	void (*set_device_trust)(imv_session_t *this, bool trusted);


	/**
	 * Get device ID trust (needed for TPM-based attestation)
	 *
	 * @return				TRUE if Device ID is trusted
	 */
	bool (*get_device_trust)(imv_session_t *this);

	/**
	 * Set policy_started status
	 *
	 * @param start			TRUE if policy started, FALSE if policy stopped
	 */
	void (*set_policy_started)(imv_session_t *this, bool start);

	/**
	 * Get policy_started status
	 *
	 * @return				TRUE if policy started, FALSE if policy stopped
	 */
	bool (*get_policy_started)(imv_session_t *this);

	/**
	 * Insert workitem into list
	 *
	 * @param workitem		Workitem to be inserted
	 */
	void (*insert_workitem)(imv_session_t *this, imv_workitem_t *workitem);

	/**
	 * Remove workitem from list
	 *
	 * @param enumerator	Enumerator pointing to workitem to be removed
	 */
	void (*remove_workitem)(imv_session_t *this, enumerator_t *enumerator);

	/**
	 * Create workitem enumerator
	 *
	 */
	 enumerator_t* (*create_workitem_enumerator)(imv_session_t *this);

	/**
	 * Get number of workitem allocated to a given IMV
	 *
	 * @param imv_id		IMV ID
	 * @return				Number of workitems assigned to given IMV
	 */
	 int (*get_workitem_count)(imv_session_t *this, TNC_IMVID imv_id);

	/**
	 * Get reference to session
	 */
	imv_session_t* (*get_ref)(imv_session_t*);

	/**
	 * Destroys an imv_session_t object
	 */
	void (*destroy)(imv_session_t *this);
};

/**
 * Create an imv_session_t instance
 *
 * @param id				Associated Connection ID
 * @param ar_identities		List of Access Requestor identities
 */
imv_session_t* imv_session_create(TNC_ConnectionID id,
								 linked_list_t *ar_identities);

#endif /**  IMV_SESSION_H_ @}*/
