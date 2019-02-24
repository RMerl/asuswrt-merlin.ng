/*
 * Copyright (C) 2012-2014 Tobias Brunner
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
 * @defgroup strerror_i strerror
 * @{ @ingroup utils_i
 */

#ifndef STRERROR_H_
#define STRERROR_H_

/**
 * Thread-safe wrapper around strerror and strerror_r.
 *
 * This is required because the first is not thread-safe (on some platforms)
 * and the second uses two different signatures (POSIX/GNU) and is impractical
 * to use anyway.
 *
 * @param errnum	error code (i.e. errno)
 * @return			error message
 */
const char *strerror_safe(int errnum);

/**
 * Initialize strerror_safe()
 */
void strerror_init();

/**
 * Deinitialize strerror_safe()
 */
void strerror_deinit();

/**
 * Replace usages of strerror(3) with thread-safe variant.
 */
#define strerror(errnum) strerror_safe(errnum)

#endif /** STRERROR_H_ @}*/
