/*
 * Copyright (C) 2016 secunet Security Networks AG
 * Copyright (C) 2016 Thomas Egerer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @defgroup custom_logger custom_logger
 * @{ @ingroup listeners
 */

#ifndef CUSTOM_LOGGER_H_
#define CUSTOM_LOGGER_H_

#include <bus/listeners/logger.h>

typedef struct custom_logger_t custom_logger_t;

/**
 * Custom logger which implements listener_t.
 */
struct custom_logger_t {

	/**
	 * Implements the logger_t interface.
	 */
	logger_t logger;

	/**
	 * Set the loglevel for a debug group.
	 *
	 * @param group		debug group to set
	 * @param level		max level to log (0..4)
	 */
	void (*set_level)(custom_logger_t *this, debug_t group, level_t level);

	/**
	 * Reload custom logger configuration.
	 */
	void (*reload)(custom_logger_t *this);

	/**
	 * Destroy the custom_logger_t object.
	 */
	void (*destroy)(custom_logger_t *this);
};

/**
 * Prototype for custom logger construction function pointer.
 */
typedef custom_logger_t *(*custom_logger_constructor_t)(const char *name);

#endif /** CUSTOM_LOGGER_H_ @}*/
