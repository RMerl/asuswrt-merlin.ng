/*
 * Copyright (C) 2010 Tobias Brunner
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
 * @defgroup android_log_logger android_log_logger
 * @{ @ingroup android_log
 */

#ifndef ANDROID_LOG_LOGGER_H_
#define ANDROID_LOG_LOGGER_H_

#include <bus/bus.h>

typedef struct android_log_logger_t android_log_logger_t;

/**
 * Android specific logger.
 */
struct android_log_logger_t {

	/**
	 * Implements logger_t interface
	 */
	logger_t logger;

	/**
	 * Destroy the logger.
	 */
	void (*destroy)(android_log_logger_t *this);

};

/**
 * Create an Android specific logger instance.
 *
 * @return			logger instance
 */
android_log_logger_t *android_log_logger_create();

#endif /** ANDROID_LOG_LOGGER_H_ @}*/
