/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup vici_logger vici_logger
 * @{ @ingroup vici
 */

#ifndef VICI_LOGGER_H_
#define VICI_LOGGER_H_

#include "vici_dispatcher.h"

#include <bus/listeners/logger.h>

typedef struct vici_logger_t vici_logger_t;

/**
 * Generic debugging logger over vici.
 */
struct vici_logger_t {

	/**
	 * Implements logger interface.
	 */
	logger_t logger;

	/**
	 * Destroy a vici_logger_t.
	 */
	void (*destroy)(vici_logger_t *this);
};

/**
 * Create a vici_logger instance.
 *
 * @param dispatcher		dispatcher to receive requests from
 * @return					loggerential backend
 */
vici_logger_t *vici_logger_create(vici_dispatcher_t *dispatcher);

#endif /** VICI_LOGGER_H_ @}*/
