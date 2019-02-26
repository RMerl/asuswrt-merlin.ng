/*
 * Copyright (C) 2009 Tobias Brunner
 * Copyright (C) 2006-2008 Martin Willi
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
 * @defgroup printf_hook printf_hook
 * @{ @ingroup utils
 */

#ifndef PRINTF_HOOK_H_
#define PRINTF_HOOK_H_

#include <stdlib.h>

typedef struct printf_hook_t printf_hook_t;
typedef struct printf_hook_spec_t printf_hook_spec_t;
typedef struct printf_hook_data_t printf_hook_data_t;
typedef enum printf_hook_argtype_t printf_hook_argtype_t;

#if defined(USE_VSTR)
# include "printf_hook_vstr.h"
#elif defined(USE_BUILTIN_PRINTF)
# include "printf_hook_builtin.h"
#endif

/**
 * Argument types to pass to printf hook.
 */
enum printf_hook_argtype_t {
	PRINTF_HOOK_ARGTYPE_END,
	PRINTF_HOOK_ARGTYPE_INT,
	PRINTF_HOOK_ARGTYPE_POINTER,
};

/**
 * Callback function type for printf hooks.
 *
 * @param data		hook data, to pass to print_in_hook()
 * @param spec		format specifier
 * @param args		arguments array
 * @return			number of characters written
 */
typedef int (*printf_hook_function_t)(printf_hook_data_t *data,
									  printf_hook_spec_t *spec,
									  const void *const *args);

/**
 * Properties of the format specifier
 */
struct printf_hook_spec_t {

	/**
	 * TRUE if a '#' was used in the format specifier
	 */
	int hash;

	/**
	 * TRUE if a '-' was used in the format specifier
	 */
	int minus;

	/**
	 * TRUE if a '+' was used in the format specifier
	 */
	int plus;

	/**
	 * The width as given in the format specifier.
	 */
	int width;
};

/**
 * Printf handler management.
 */
struct printf_hook_t {

	/**
	 * Register a printf handler.
	 *
	 * @param spec		printf hook format character
	 * @param hook		hook function
	 * @param ...		list of PRINTF_HOOK_ARGTYPE_*, MUST end with PRINTF_HOOK_ARGTYPE_END
	 */
	void (*add_handler)(printf_hook_t *this, char spec,
						printf_hook_function_t hook, ...);

	/**
	 * Destroy a printf_hook instance.
	 */
	void (*destroy)(printf_hook_t *this);
};

/**
 * Create a printf_hook instance.
 */
printf_hook_t *printf_hook_create();

/**
 * Print with format string within a printf hook.
 *
 * @param data		hook data, as passed to printf hook
 * @param fmt		printf format string
 * @param ...		arguments to format string
 * @return			number of characters written
 */
size_t print_in_hook(printf_hook_data_t *data, char *fmt, ...);

#endif /** PRINTF_HOOK_H_ @}*/
