/*
 * Copyright (C) 2012 Tobias Brunner
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
 * @defgroup ipsec_processor ipsec_processor
 * @{ @ingroup libipsec
 */

#ifndef IPSEC_PROCESSOR_H_
#define IPSEC_PROCESSOR_H_

#include "ip_packet.h"
#include "esp_packet.h"

typedef struct ipsec_processor_t ipsec_processor_t;

/**
 * Callback called to deliver an inbound plaintext packet.
 *
 * @param data			data supplied during registration of the callback
 * @param packet		plaintext IP packet to deliver
 */
typedef void (*ipsec_inbound_cb_t)(void *data, ip_packet_t *packet);

/**
 * Callback called to send an ESP packet.
 *
 * @note The ESP packet currently comes without IP header (and without UDP
 * header in case of UDP encapsulation)
 *
 * @param data			data supplied during registration of the callback
 * @param packet		ESP packet to send
 */
typedef void (*ipsec_outbound_cb_t)(void *data, esp_packet_t *packet);

/**
 *  IPsec processor
 */
struct ipsec_processor_t {

	/**
	 * Queue an inbound ESP packet for processing.
	 *
	 * @param packet		the ESP packet to process
	 */
	void (*queue_inbound)(ipsec_processor_t *this, esp_packet_t *packet);

	/**
	 * Queue an outbound plaintext IP packet for processing.
	 *
	 * @param packet		the plaintext IP packet
	 */
	void (*queue_outbound)(ipsec_processor_t *this, ip_packet_t *packet);

	/**
	 * Register the callback used to deliver inbound plaintext packets.
	 *
	 * @param cb			the inbound callback function
	 * @param data			optional data provided to callback
	 */
	void (*register_inbound)(ipsec_processor_t *this, ipsec_inbound_cb_t cb,
							 void *data);

	/**
	 * Unregister a previously registered inbound callback.
	 *
	 * @param cb			previously registered callback function
	 */
	void (*unregister_inbound)(ipsec_processor_t *this,
							   ipsec_inbound_cb_t cb);

	/**
	 * Register the callback used to send outbound ESP packets.
	 *
	 * @param cb			the outbound callback function
	 * @param data			optional data provided to callback
	 */
	void (*register_outbound)(ipsec_processor_t *this, ipsec_outbound_cb_t cb,
							  void *data);

	/**
	 * Unregister a previously registered outbound callback.
	 *
	 * @param cb			previously registered callback function
	 */
	void (*unregister_outbound)(ipsec_processor_t *this,
								ipsec_outbound_cb_t cb);

	/**
	 * Destroy an ipsec_processor_t.
	 */
	void (*destroy)(ipsec_processor_t *this);

};

/**
 * Create an ipsec_processor_t instance
 *
 * @return					IPsec processor instance
 */
ipsec_processor_t *ipsec_processor_create();

#endif /** IPSEC_PROCESSOR_H_ @}*/
