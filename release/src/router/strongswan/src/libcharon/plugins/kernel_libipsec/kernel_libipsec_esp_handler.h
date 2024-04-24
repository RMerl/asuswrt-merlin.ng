/*
 * Copyright (C) 2023 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup kernel_libipsec_esp_handler kernel_libipsec_esp_handler
 * @{ @ingroup kernel_libipsec
 */

#ifndef KERNEL_LIBIPSEC_ESP_HANDLER_H_
#define KERNEL_LIBIPSEC_ESP_HANDLER_H_

#include <esp_packet.h>

typedef struct kernel_libipsec_esp_handler_t kernel_libipsec_esp_handler_t;

/**
 * Class that sends and receives raw ESP packets.
 */
struct kernel_libipsec_esp_handler_t {

	/**
	 * Send the given ESP packet without UDP encapsulation.
	 *
	 * @param packet	ESP packet to send
	 */
	void (*send)(kernel_libipsec_esp_handler_t *this, esp_packet_t *packet);

	/**
	 * Destroy the given instance.
	 */
	void (*destroy)(kernel_libipsec_esp_handler_t *this);
};

/**
 * Create a kernel_libipsec_esp_handler_t instance.
 *
 * @return				created instance, NULL if not supported
 */
kernel_libipsec_esp_handler_t *kernel_libipsec_esp_handler_create();

#endif /** KERNEL_LIBIPSEC_ESP_HANDLER_H_ @}*/
