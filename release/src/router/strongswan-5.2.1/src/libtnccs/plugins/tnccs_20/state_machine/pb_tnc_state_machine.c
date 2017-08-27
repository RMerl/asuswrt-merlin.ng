/*
 * Copyright (C) 2010 Andreas Steffen
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

#include "pb_tnc_state_machine.h"

#include <utils/debug.h>

ENUM(pb_tnc_state_names, PB_STATE_INIT, PB_STATE_END,
	"Init",
	"Server Working",
	"Client Working",
	"Decided",
	"End"
);

/**
 *   PB-TNC State Machine (see section 3.2 of RFC 5793)
 *
 *              Receive CRETRY        SRETRY
 *                   or SRETRY   +----------------+
 *                        +--+   |                |
 *                        v  |   v                |
 *                       +---------+  CRETRY  +---------+
 *             CDATA     | Server  |<---------| Decided | CLOSE
 *          +----------->| Working |--------->|         |-------+
 *          |            +---------+  RESULT  +---------+       |
 *          |                ^ |  |                             v
 *          |                | |  +---------------------->=======
 *        ========           | |              CLOSE       " End "
 *        " Init "      CDATA| |SDATA                     =======
 *        ========           | |                          ^    ^
 *          |  |             | v                          |    |
 *          |  | SDATA   +---------+          CLOSE       |    |
 *          |  +-------->| Client  |----------------------+    |
 *          |            | Working |                           |
 *          |            +---------+                           |
 *          |                |  ^                              |
 *          |                +--+                              |
 *          |            Receive CRETRY                        |
 *          |   CLOSE                                          |
 *          +--------------------------------------------------+
 */

typedef struct private_pb_tnc_state_machine_t private_pb_tnc_state_machine_t;

/**
 * Private data of a pb_tnc_state_machine_t object.
 *
 */
struct private_pb_tnc_state_machine_t {
	/**
	 * Public pb_pa_message_t interface.
	 */
	pb_tnc_state_machine_t public;

	/**
	 * PB-TNC Server if TRUE, PB-TNC Client if FALSE
	 */
	bool is_server;

	/**
	 * Informs whether last received PB-TNC CDATA Batch was empty
	 */
	bool empty_cdata;

	/**
	 * Current PB-TNC state
	 */
	pb_tnc_state_t state;
};

METHOD(pb_tnc_state_machine_t, get_state, pb_tnc_state_t,
	private_pb_tnc_state_machine_t *this)
{
	return this->state;
}

METHOD(pb_tnc_state_machine_t, receive_batch, bool,
	private_pb_tnc_state_machine_t *this, pb_tnc_batch_type_t type)
{
	pb_tnc_state_t old_state = this->state;

	switch (this->state)
	{
		case PB_STATE_INIT:
			if (this->is_server && type == PB_BATCH_CDATA)
			{
				this->state = PB_STATE_SERVER_WORKING;
				break;
			}
			if (!this->is_server && type == PB_BATCH_SDATA)
			{
				this->state = PB_STATE_CLIENT_WORKING;
				break;
			}
			if (type == PB_BATCH_CLOSE)
			{
				this->state = PB_STATE_END;
				break;
			}
			return FALSE;
		case PB_STATE_SERVER_WORKING:
			if (!this->is_server && (type == PB_BATCH_SDATA ||
									 type == PB_BATCH_SRETRY))
			{
				this->state = PB_STATE_CLIENT_WORKING;
				break;
			}
			if (!this->is_server && type == PB_BATCH_RESULT)
			{
				this->state = PB_STATE_DECIDED;
				break;
			}
			if (this->is_server && type == PB_BATCH_CRETRY)
			{
				break;
			}
			if (type == PB_BATCH_CLOSE)
			{
				this->state = PB_STATE_END;
				break;
			}
			return FALSE;
		case PB_STATE_CLIENT_WORKING:
			if (this->is_server && type == PB_BATCH_CDATA)
			{
				this->state = PB_STATE_SERVER_WORKING;
				break;
			}
			if (this->is_server && type == PB_BATCH_CRETRY)
			{
				break;
			}
			if (type == PB_BATCH_CLOSE)
			{
				this->state = PB_STATE_END;
				break;
			}
			return FALSE;
		case PB_STATE_DECIDED:
			if ((this->is_server && type == PB_BATCH_CRETRY) ||
			   (!this->is_server && type == PB_BATCH_SRETRY))
			{
				this->state = PB_STATE_SERVER_WORKING;
				break;
			}
			if (type == PB_BATCH_CLOSE)
			{
				this->state = PB_STATE_END;
				break;
			}
			return FALSE;
		case PB_STATE_END:
			if (type == PB_BATCH_CLOSE)
			{
				break;
			}
			return FALSE;
	}

	if (this->state != old_state)
	{
		DBG2(DBG_TNC, "PB-TNC state transition from '%N' to '%N'",
			 pb_tnc_state_names, old_state, pb_tnc_state_names, this->state);
	}
	return TRUE;
}

METHOD(pb_tnc_state_machine_t, send_batch, bool,
	private_pb_tnc_state_machine_t *this, pb_tnc_batch_type_t type)
{
	pb_tnc_state_t old_state = this->state;

	switch (this->state)
	{
		case PB_STATE_INIT:
			if (!this->is_server && type == PB_BATCH_CDATA)
			{
				this->state = PB_STATE_SERVER_WORKING;
				break;
			}
			if (this->is_server && type == PB_BATCH_SDATA)
			{
				this->state = PB_STATE_CLIENT_WORKING;
				break;
			}
			if (type == PB_BATCH_CLOSE)
			{
				this->state = PB_STATE_END;
				break;
			}
			return FALSE;
		case PB_STATE_SERVER_WORKING:
			if (this->is_server && (type == PB_BATCH_SDATA ||
									type == PB_BATCH_SRETRY))
			{
				this->state = PB_STATE_CLIENT_WORKING;
				break;
			}
			if (this->is_server && type == PB_BATCH_RESULT)
			{
				this->state = PB_STATE_DECIDED;
				break;
			}
			if (!this->is_server && type == PB_BATCH_CRETRY)
			{
				break;
			}
			if (type == PB_BATCH_CLOSE)
			{
				this->state = PB_STATE_END;
				break;
			}
			return FALSE;
		case PB_STATE_CLIENT_WORKING:
			if (!this->is_server && (type == PB_BATCH_CDATA ||
									 type == PB_BATCH_CRETRY))
			{
				this->state = PB_STATE_SERVER_WORKING;
				break;
			}
			if (this->is_server && type == PB_BATCH_SRETRY)
			{
				break;
			}
			if (type == PB_BATCH_CLOSE)
			{
				this->state = PB_STATE_END;
				break;
			}
			return FALSE;
		case PB_STATE_DECIDED:
			if ((this->is_server && type == PB_BATCH_SRETRY) ||
			   (!this->is_server && type == PB_BATCH_CRETRY))
			{
				this->state = PB_STATE_SERVER_WORKING;
				break;
			}
			if (type == PB_BATCH_CLOSE)
			{
				this->state = PB_STATE_END;
				break;
			}
			return FALSE;
		case PB_STATE_END:
			if (type == PB_BATCH_CLOSE)
			{
				break;
			}
			return FALSE;
	}

	if (this->state != old_state)
	{
		DBG2(DBG_TNC, "PB-TNC state transition from '%N' to '%N'",
			 pb_tnc_state_names, old_state, pb_tnc_state_names, this->state);
	}
	return TRUE;
}

METHOD(pb_tnc_state_machine_t, get_empty_cdata, bool,
	private_pb_tnc_state_machine_t *this)
{
	return this->empty_cdata;
}

METHOD(pb_tnc_state_machine_t, set_empty_cdata, void,
	private_pb_tnc_state_machine_t *this, bool empty)
{
	if (empty)
	{
		DBG2(DBG_TNC, "received empty PB-TNC CDATA batch");
	}
	this->empty_cdata = empty;
}

METHOD(pb_tnc_state_machine_t, destroy, void,
	private_pb_tnc_state_machine_t *this)
{
	free(this);
}

/**
 * See header
 */
pb_tnc_state_machine_t* pb_tnc_state_machine_create(bool is_server)
{
	private_pb_tnc_state_machine_t *this;

	INIT(this,
		.public = {
			.get_state = _get_state,
			.receive_batch = _receive_batch,
			.send_batch = _send_batch,
			.get_empty_cdata = _get_empty_cdata,
			.set_empty_cdata = _set_empty_cdata,
			.destroy = _destroy,
		},
		.is_server = is_server,
		.state = PB_STATE_INIT,
	);

	return &this->public;
}
