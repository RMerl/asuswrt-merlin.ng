/*
 * Copyright (C) 2007 Martin Willi
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
 * @defgroup ike_mobike ike_mobike
 * @{ @ingroup tasks_v2
 */

#ifndef IKE_MOBIKE_H_
#define IKE_MOBIKE_H_

typedef struct ike_mobike_t ike_mobike_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/task.h>
#include <networking/packet.h>

/**
 * Task of type ike_mobike, detects and handles MOBIKE extension.
 *
 * The MOBIKE extension is defined in RFC4555. It allows to update IKE
 * and IPsec tunnel addresses.
 * This tasks handles the MOBIKE_SUPPORTED notify exchange to detect MOBIKE
 * support, allows the exchange of ADDITIONAL_*_ADDRESS to exchange additional
 * endpoints and handles the UPDATE_SA_ADDRESS notify to finally update
 * endpoints.
 */
struct ike_mobike_t {

	/**
	 * Implements the task_t interface
	 */
	task_t task;

	/**
	 * Use the task to update the list of additional addresses.
	 */
	void (*addresses)(ike_mobike_t *this);

	/**
	 * Use the task to roam to other addresses.
	 *
	 * @param address		TRUE to include address list update
	 */
	void (*roam)(ike_mobike_t *this, bool address);

	/**
	 * Use the task for a DPD check which detects changes in NAT mappings.
	 */
	void (*dpd)(ike_mobike_t *this);

	/**
	 * Transmission hook, called by task manager.
	 *
	 * The task manager calls this hook whenever it transmits a packet. It
	 * allows the mobike task to send the packet on multiple paths to do path
	 * probing.
	 *
	 * @param packet		the packet to transmit
	 * @return				TRUE if transmitted, FALSE if no path found
	 */
	bool (*transmit)(ike_mobike_t *this, packet_t *packet);

	/**
	 * Check if this task is probing for routability.
	 *
	 * @return				TRUE if task is probing
	 */
	bool (*is_probing)(ike_mobike_t *this);

	/**
	 * Enable probing for routability.
	 */
	void (*enable_probing)(ike_mobike_t *this);
};

/**
 * Create a new ike_mobike task.
 *
 * @param ike_sa		IKE_SA this task works for
 * @param initiator		TRUE if task is initiated by us
 * @return				ike_mobike task to handle by the task_manager
 */
ike_mobike_t *ike_mobike_create(ike_sa_t *ike_sa, bool initiator);

#endif /** IKE_MOBIKE_H_ @}*/
