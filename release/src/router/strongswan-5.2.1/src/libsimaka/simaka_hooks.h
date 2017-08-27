/*
 * Copyright (C) 2008-2011 Martin Willi
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
 * @defgroup simaka_hooks simaka_hooks
 * @{ @ingroup libsimaka
 */

#ifndef SIMAKA_HOOKS_H_
#define SIMAKA_HOOKS_H_

typedef struct simaka_hooks_t simaka_hooks_t;

#include "simaka_message.h"

/**
 * Additional hooks invoked during EAP-SIM/AKA message processing.
 */
struct simaka_hooks_t {

	/**
	 * SIM/AKA message parsing.
	 *
	 * As a SIM/AKA optionally contains encrypted attributes, the hook
	 * might get invoked twice, once before and once after decryption.
	 *
	 * @param message	SIM/AKA message
	 * @param inbound	TRUE for incoming messages, FALSE for outgoing
	 * @param decrypted	TRUE if AT_ENCR_DATA has been decrypted
	 */
	void (*message)(simaka_hooks_t *this, simaka_message_t *message,
					bool inbound, bool decrypted);

	/**
	 * SIM/AKA encryption/authentication key hooks.
	 *
	 * @param k_encr	derived SIM/AKA encryption key k_encr
	 * @param k_auth	derived SIM/AKA authentication key k_auth
	 */
	void (*keys)(simaka_hooks_t *this, chunk_t k_encr, chunk_t k_auth);
};

#endif /** SIMAKA_HOOKS_H_ @}*/
