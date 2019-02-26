/*
 * Copyright (C) 2010-2014 Martin Willi
 * Copyright (C) 2010-2014 revosec AG
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
 * @defgroup forecast_forwarder forecast_forwarder
 * @{ @ingroup forecast
 */

#ifndef FORECAST_FORWARDER_H_
#define FORECAST_FORWARDER_H_

#include "forecast_listener.h"

typedef struct forecast_forwarder_t forecast_forwarder_t;

/**
 * Broadcast/Multicast sniffer and forwarder.
 */
struct forecast_forwarder_t {

	/**
	 * Destroy a forecast_forwarder_t.
	 */
	void (*destroy)(forecast_forwarder_t *this);
};

/**
 * Create a forecast_forwarder instance.
 *
 * @param listener		listener to check for addresses to forward to
 * @return				forwarder instance
 */
forecast_forwarder_t *forecast_forwarder_create(forecast_listener_t *listener);

#endif /** FORECAST_FORWARDER_H_ @}*/
