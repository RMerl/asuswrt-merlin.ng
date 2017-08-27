/*
 * Copyright (C) 2014 Andreas Steffen
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
 * @defgroup imv_session_manager_t imv_session_manager
 * @{ @ingroup libimcv_imv
 */

#ifndef IMV_SESSION_MANAGER_H_
#define IMV_SESSION_MANAGER_H_

#include "imv_session.h"

#include <tncifimv.h>

#include <library.h>

typedef struct imv_session_manager_t imv_session_manager_t;

/**
 * IMV session manager interface 
 */
struct imv_session_manager_t {

	/**
	 * Create or get a session associated with a TNCCS connection
	 *
	 * @param conn_id		TNCCS Connection ID
	 * @param ar_id_type	Access Requestor identity type
	 * @param ar_id_value	Access Requestor identity value
	 * @return				Session associated with TNCCS Connection
	 */
	 imv_session_t* (*add_session)(imv_session_manager_t *this,
								   TNC_ConnectionID conn_id,
								   uint32_t ar_id_type, chunk_t ar_id_value);

	/**
	 * Remove a session
	 *
	 * @param session		Session
	 */
	 void (*remove_session)(imv_session_manager_t *this, imv_session_t *session);


	/**
	 * Destroys an imv_session_manager_t object
	 */
	void (*destroy)(imv_session_manager_t *this);
};

/**
 * Create an imv_session_manager_t instance
 */
imv_session_manager_t* imv_session_manager_create();

#endif /** IMV_SESSION_MANAGER_H_ @}*/
