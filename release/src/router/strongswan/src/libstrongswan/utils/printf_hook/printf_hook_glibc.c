/*
 * Copyright (C) 2009-2013 Tobias Brunner
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

#include "printf_hook.h"

#include <utils/utils.h>
#include <utils/debug.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <printf.h>

typedef struct private_printf_hook_t private_printf_hook_t;
typedef struct printf_hook_handler_t printf_hook_handler_t;

/**
 * private data of printf_hook
 */
struct private_printf_hook_t {

	/**
	 * public functions
	 */
	printf_hook_t public;
};

/**
 * struct with information about a registered handler
 */
struct printf_hook_handler_t {

	/**
	 * callback function
	 */
	printf_hook_function_t hook;

	/**
	 * number of arguments
	 */
	int numargs;

	/**
	 * types of the arguments, PA_*
	 */
	int argtypes[3];
};

/**
 * Data to pass to a printf hook.
 */
struct printf_hook_data_t {

	/**
	 * Output FILE stream
	 */
	FILE *stream;;
};

/* A-Z | 6 other chars | a-z */
static printf_hook_handler_t *printf_hooks[58];

#define SPEC_TO_INDEX(spec) ((int)(spec) - (int)'A')

/**
 * Glibc variant of print_in_hook()
 */
size_t print_in_hook(printf_hook_data_t *data, char *fmt, ...)
{
	ssize_t written;
	va_list args;

	va_start(args, fmt);
	written = vfprintf(data->stream, fmt, args);
	va_end(args);

	if (written < 0)
	{
		written = 0;
	}
	return written;
}

/**
 * Printf hook print function. This is actually of type "printf_function",
 * however glibc does it typedef to function, but uclibc to a pointer.
 * So we redefine it here.
 */
static int custom_print(FILE *stream, const struct printf_info *info,
						const void *const *args)
{
	printf_hook_spec_t spec;
	printf_hook_handler_t *handler;
	printf_hook_data_t data = {
		.stream = stream,
	};

	handler =  printf_hooks[SPEC_TO_INDEX(info->spec)];
	spec.hash = info->alt;
	spec.plus = info->showsign;
	spec.minus = info->left;
	spec.width = info->width;

	return handler->hook(&data, &spec, args);
}

/**
 * Printf hook arginfo function, which is actually of type
 * "printf_arginfo_[size_]function".
 */
static int custom_arginfo(const struct printf_info *info, size_t n, int *argtypes
#ifdef HAVE_PRINTF_SPECIFIER
						  , int *size
#endif
						  )
{
	int i;
	printf_hook_handler_t *handler;

	handler = printf_hooks[SPEC_TO_INDEX(info->spec)];
	if (handler->numargs <= n)
	{
		for (i = 0; i < handler->numargs; ++i)
		{
			argtypes[i] = handler->argtypes[i];
		}
	}
	/* we never set "size", as we have no user defined types */
	return handler->numargs;
}

METHOD(printf_hook_t, add_handler, void,
	private_printf_hook_t *this, char spec,
						printf_hook_function_t hook, ...)
{
	int i = -1;
	bool failed = FALSE;
	printf_hook_handler_t *handler;
	printf_hook_argtype_t argtype;
	va_list args;

	if (SPEC_TO_INDEX(spec) <= -1 ||
		SPEC_TO_INDEX(spec) >= countof(printf_hooks))
	{
		DBG1(DBG_LIB, "'%c' is not a valid printf hook specifier, "
			 "not registered!", spec);
		return;
	}

	INIT(handler,
		.hook = hook,
	);

	va_start(args, hook);
	while (!failed)
	{
		argtype = va_arg(args, printf_hook_argtype_t);

		if (argtype == PRINTF_HOOK_ARGTYPE_END)
		{
			break;
		}
		if (++i >= countof(handler->argtypes))
		{
			DBG1(DBG_LIB, "Too many arguments for printf hook with "
				 "specifier '%c', not registered!", spec);
			failed = TRUE;
			break;
		}
		switch (argtype)
		{
			case PRINTF_HOOK_ARGTYPE_INT:
				handler->argtypes[i] = PA_INT;
				break;
			case PRINTF_HOOK_ARGTYPE_POINTER:
				handler->argtypes[i] = PA_POINTER;
				break;
			default:
				DBG1(DBG_LIB, "Invalid printf hook arg type for '%c'", spec);
				failed = TRUE;
				break;
		}
	}
	va_end(args);

	handler->numargs = i + 1;
	if (!failed && handler->numargs > 0)
	{
#	ifdef HAVE_PRINTF_SPECIFIER
		register_printf_specifier(spec, custom_print, custom_arginfo);
#	else
		register_printf_function(spec, custom_print, custom_arginfo);
#	endif
		printf_hooks[SPEC_TO_INDEX(spec)] = handler;
	}
	else
	{
		free(handler);
	}
}

METHOD(printf_hook_t, destroy, void,
	private_printf_hook_t *this)
{
	int i;

	for (i = 0; i < countof(printf_hooks); i++)
	{
		free(printf_hooks[i]);
	}
	free(this);
}

/*
 * see header file
 */
printf_hook_t *printf_hook_create()
{
	private_printf_hook_t *this;

	INIT(this,
		.public = {
			.add_handler = _add_handler,
			.destroy = _destroy,
		},
	);

	memset(printf_hooks, 0, sizeof(printf_hooks));

	return &this->public;
}
