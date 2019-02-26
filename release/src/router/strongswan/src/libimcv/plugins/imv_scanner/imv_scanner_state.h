/*
 * Copyright (C) 2011-2013 Andreas Steffen
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
 * @defgroup imv_scanner imv_scanner
 * @ingroup libimcv_plugins
 *
 * @defgroup imv_scanner_state_t imv_scanner_state
 * @{ @ingroup imv_scanner
 */

#ifndef IMV_SCANNER_STATE_H_
#define IMV_SCANNER_STATE_H_

#include <imv/imv_state.h>
#include <ietf/ietf_attr_port_filter.h>

#include <library.h>

typedef struct imv_scanner_state_t imv_scanner_state_t;
typedef enum imv_scanner_handshake_state_t imv_scanner_handshake_state_t;

/**
 * IMV Scanner Handshake States (state machine)
 */
enum imv_scanner_handshake_state_t {
	IMV_SCANNER_STATE_INIT,
	IMV_SCANNER_STATE_ATTR_REQ,
	IMV_SCANNER_STATE_WORKITEMS,
	IMV_SCANNER_STATE_END
};

/**
 * Internal state of an imv_scanner_t connection instance
 */
struct imv_scanner_state_t {

	/**
	 * imv_state_t interface
	 */
	imv_state_t interface;

	/**
	 * Set state of the handshake
	 *
	 * @param new_state			the handshake state of IMV
	 */
	void (*set_handshake_state)(imv_scanner_state_t *this,
								imv_scanner_handshake_state_t new_state);

	/**
	 * Get state of the handshake
	 *
	 * @return					the handshake state of IMV
	 */
	imv_scanner_handshake_state_t (*get_handshake_state)(imv_scanner_state_t *this);

	/**
	 * Store an IETF Port Filter attribute for later evaluation
	 *
	 * @param attr				IETF Port Filter attribute
	 */
	void (*set_port_filter_attr)(imv_scanner_state_t *this,
								 ietf_attr_port_filter_t *attr);

	/**
	 * Get the stored IETF Port Filter attribute
	 *
	 * @return					IETF Port Filter attribute
	 */
	ietf_attr_port_filter_t* (*get_port_filter_attr)(imv_scanner_state_t *this);

	/**
	 * add a violating TCP or UDP port
	 */
	void (*add_violating_port)(imv_scanner_state_t *this, char *port);
};

/**
 * Create an imv_scanner_state_t instance
 *
 * @param id			connection ID
 */
imv_state_t* imv_scanner_state_create(TNC_ConnectionID id);

#endif /** IMV_SCANNER_STATE_H_ @}*/
