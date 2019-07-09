/*
 * Copyright (C) 2014 Tobias Brunner
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
 * @defgroup android_dns_proxy android_dns_proxy
 * @{ @ingroup android_backend
 */

#ifndef ANDROID_DNS_PROXY_H_
#define ANDROID_DNS_PROXY_H_

#include <ip_packet.h>

typedef struct android_dns_proxy_t android_dns_proxy_t;

/**
 * Callback called to deliver a DNS response packet.
 *
 * @param data			data supplied during registration of the callback
 * @param packet		DNS response packet (has to be destroyed)
 */
typedef void (*dns_proxy_response_cb_t)(void *data, ip_packet_t *packet);

/**
 * DNS proxy class
 */
struct android_dns_proxy_t {

	/**
	 * Handle an outbound DNS packet (if the packet is one)
	 *
	 * @param packet		packet to handle
	 * @return 				TRUE if handled, FALSE otherwise (no DNS)
	 */
	bool (*handle)(android_dns_proxy_t *this, ip_packet_t *packet);

	/**
	 * Register the callback used to deliver DNS response packets.
	 *
	 * @param cb			the callback function
	 * @param data			optional data provided to callback
	 */
	void (*register_cb)(android_dns_proxy_t *this, dns_proxy_response_cb_t cb,
						void *data);

	/**
	 * Unregister the callback used to deliver DNS response packets.
	 *
	 * @param cb			the callback function
	 * @param data			optional data provided to callback
	 */
	void (*unregister_cb)(android_dns_proxy_t *this, dns_proxy_response_cb_t cb);

	/**
	 * Add a hostname for which queries are proxied.  If at least one hostname
	 * is configured DNS queries for others will not be handled.
	 *
	 * @param hostname		hostname to add (gets cloned)
	 */
	void (*add_hostname)(android_dns_proxy_t *this, char *hostname);

	/**
	 * Destroy an instance.
	 */
	void (*destroy)(android_dns_proxy_t *this);
};

/**
 * Create an instance.
 */
android_dns_proxy_t *android_dns_proxy_create();

#endif /** ANDROID_DNS_PROXY_H_ @}*/

