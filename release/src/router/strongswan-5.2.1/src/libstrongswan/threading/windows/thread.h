/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#ifndef WINDOWS_THREAD_H_
#define WINDOWS_THREAD_H_

/* for conditionVariables, Vista */
#define _WIN32_WINNT 0x0600
#include <library.h>

/**
 * @defgroup windowsthread windows
 * @ingroup threading
 *
 * @defgroup threadwindows thread
 * @{ @ingroup windowsthread
 */

/**
 * Set active condvar of a thread before waiting in it.
 *
 * @param condvar	active condition variable, NULL to unset
 */
void thread_set_active_condvar(CONDITION_VARIABLE *condvar);

/**
 * Set a thread specific value on the current thread.
 *
 * @param key		unique key specifying the TLS variable
 * @param value		value to set
 * @return			old value for key, if any
 */
void* thread_tls_put(void *key, void *value);

/**
 * Get a thread specific value from the current thread.
 *
 * @param key		unique key specifying the TLS variable
 * @return			value for key, if any
 */
void* thread_tls_get(void *key);

/**
 * Remove a thread specific value from the current thread.
 *
 * @param key		unique key specifying the TLS variable
 * @return			value for key, if any
 */
void* thread_tls_remove(void *key);

/**
 * Cleanup function for thread specific value.
 *
 * This is called whenever a thread exits to clean up thread specific data.
 *
 * This function is actually implemented in thread_value.c.
 *
 * @param value		value, as passed to thread_tls_put()
 */
void thread_tls_cleanup(void *value);

#endif /** WINDOWS_THREAD_H_ @}*/
