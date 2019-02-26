/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup led_listener led_listener
 * @{ @ingroup led
 */

#ifndef LED_LISTENER_H_
#define LED_LISTENER_H_

#include <bus/listeners/listener.h>

typedef struct led_listener_t led_listener_t;

/**
 * Listener that controls LEDs based on IKEv2 activity/state.
 */
struct led_listener_t {

	/**
	 * Implements listener_t interface.
	 */
	listener_t listener;

	/**
	 * Destroy a led_listener_t.
	 */
	void (*destroy)(led_listener_t *this);
};

/**
 * Create a led_listener instance.
 */
led_listener_t *led_listener_create();

#endif /** LED_LISTENER_H_ @}*/
