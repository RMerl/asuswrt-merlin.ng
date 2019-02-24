/*
 * Copyright (C) 2013 Andreas Steffen
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
 * @defgroup imv_database_t imv_database
 * @{ @ingroup libimcv_imv
 */

#ifndef IMV_DATABASE_H_
#define IMV_DATABASE_H_

#include "imv_session.h"
#include "imv_workitem.h"

#include <tncifimv.h>

#include <library.h>

typedef struct imv_database_t imv_database_t;

/**
 * IMV database interface 
 */
struct imv_database_t {

	/**
	 * Create or get a session associated with a TNCCS connection
	 *
	 * @param conn_id		TNCCS Connection ID
	 * @param ar_id_type	Access Requestor identity type
	 * @param ar_id_value	Access Requestor identity value
	 * @return				Session associated with TNCCS Connection
	 */
	 imv_session_t* (*add_session)(imv_database_t *this,
								   TNC_ConnectionID conn_id,
								   uint32_t ar_id_type, chunk_t ar_id_value);

	/**
	 * Remove and delete a session
	 *
	 * @param session		Session
	 */
	 void (*remove_session)(imv_database_t *this, imv_session_t *session);

	/**
	 * Add final recommendation to a session database entry
	 *
	 * @param session		Session
	 * @param rec			Final recommendation
	 */
	 void (*add_recommendation)(imv_database_t *this, imv_session_t *session,
								TNC_IMV_Action_Recommendation rec);

	/**
	 * Announce session start/stop to policy script
	 *
	 * @param session		Session
	 * @param start			TRUE if session start, FALSE if session stop
	 * @return				TRUE if command successful, FALSE otherwise
	 */
	 bool (*policy_script)(imv_database_t *this, imv_session_t *session,
						   bool start);

	/**
	 * Finalize a workitem
	 *
	 * @param workitem		Workitem to be finalized
	 */
	bool (*finalize_workitem)(imv_database_t *this, imv_workitem_t *workitem);

	/**
	 * Get database handle
	 *
	 * @return				Database handle
	 */
	 database_t* (*get_database)(imv_database_t *this);

	/**
	 * Destroys an imv_database_t object
	 */
	void (*destroy)(imv_database_t *this);
};

/**
 * Create an imv_database_t instance
 *
 * @param uri				Database uri
 * @param script			Policy Manager script
 */
imv_database_t* imv_database_create(char *uri, char *script);

#endif /** IMV_DATABASE_H_ @}*/
