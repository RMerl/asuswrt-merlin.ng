/*
 * Copyright (C) 2015 Andreas Steffen
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
 * @defgroup imv_hcd imv_hcd
 * @ingroup libimcv_plugins
 *
 * @defgroup imv_hcd_state_t imv_hcd_state
 * @{ @ingroup imv_hcd
 */

#ifndef IMV_HCD_STATE_H_
#define IMV_HCD_STATE_H_

#include <imv/imv_state.h>
#include <library.h>

#include <tncif_pa_subtypes.h>

typedef struct imv_hcd_state_t imv_hcd_state_t;
typedef enum imv_hcd_attr_t imv_hcd_attr_t;
typedef enum imv_hcd_handshake_state_t imv_hcd_handshake_state_t;
typedef enum os_settings_t os_settings_t;

/**
 * Flag set when corresponding attribute has been received
 */
enum imv_hcd_attr_t {
	IMV_HCD_ATTR_NONE =                          0,
	IMV_HCD_ATTR_DEFAULT_PWD_ENABLED =       (1<<0),
	IMV_HCD_ATTR_FIREWALL_SETTING =          (1<<1),
	IMV_HCD_ATTR_FORWARDING_ENABLED =        (1<<2),
	IMV_HCD_ATTR_MACHINE_TYPE_MODEL =        (1<<3),
	IMV_HCD_ATTR_PSTN_FAX_ENABLED =          (1<<4),
	IMV_HCD_ATTR_TIME_SOURCE =               (1<<5),
	IMV_HCD_ATTR_USER_APP_ENABLED =          (1<<6),
	IMV_HCD_ATTR_USER_APP_PERSIST_ENABLED =  (1<<7),
	IMV_HCD_ATTR_VENDOR_NAME =               (1<<8),
	IMV_HCD_ATTR_VENDOR_SMI_CODE =           (1<<9),
	IMV_HCD_ATTR_CERTIFICATION_STATE =       (1<<10),
	IMV_HCD_ATTR_CONFIGURATION_STATE =       (1<<11),

	IMV_HCD_ATTR_SYSTEM_ONLY =               (1<<12)-1,

	IMV_HCD_ATTR_NATURAL_LANG =              (1<<12),
	IMV_HCD_ATTR_FIRMWARE_NAME =             (1<<13),
	IMV_HCD_ATTR_RESIDENT_APP_NAME =         (1<<14),
	IMV_HCD_ATTR_USER_APP_NAME =             (1<<15),

	IMV_HCD_ATTR_MUST =                      (1<<16)-1
};

/**
 * IMV OS Handshake States (state machine)
 */
enum imv_hcd_handshake_state_t {
	IMV_HCD_STATE_INIT,
	IMV_HCD_STATE_ATTR_REQ,
	IMV_HCD_STATE_END
};

/**
 * Internal state of an imv_hcd_t connection instance
 */
struct imv_hcd_state_t {

	/**
	 * imv_state_t interface
	 */
	imv_state_t interface;

	/**
	 * Set state of the handshake
	 *
	 * @param new_state			the handshake state of IMV
	 */
	void (*set_handshake_state)(imv_hcd_state_t *this,
								imv_hcd_handshake_state_t new_state);

	/**
	 * Get state of the handshake
	 *
	 * @return					the handshake state of IMV
	 */
	imv_hcd_handshake_state_t (*get_handshake_state)(imv_hcd_state_t *this);

	/**
	 * Set the PWG HCD PA subtype currently being handled
	 *
	 * @param subtype			PWG HCD PA subtype
	 */
	void (*set_subtype)(imv_hcd_state_t *this, pa_subtype_pwg_t subtype);

	/**
	 * Set User Application Disabled
	 */
	void (*set_user_app_disabled)(imv_hcd_state_t *this);

};

/**
 * Create an imv_hcd_state_t instance
 *
 * @param id			connection ID
 */
imv_state_t* imv_hcd_state_create(TNC_ConnectionID id);

#endif /** IMV_HCD_STATE_H_ @}*/
