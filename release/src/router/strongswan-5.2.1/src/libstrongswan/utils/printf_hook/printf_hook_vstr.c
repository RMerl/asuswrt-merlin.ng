/*
 * Copyright (C) 2009-2013 Tobias Brunner
 * Copyright (C) 2006-2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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
#include <threading/thread_value.h>

#include <vstr.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

typedef struct private_printf_hook_t private_printf_hook_t;
typedef struct printf_hook_handler_t printf_hook_handler_t;

#define PRINTF_BUF_LEN 8192
#define ARGS_MAX 3

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
	 * types of the arguments, VSTR_TYPE_FMT_*
	 */
	int argtypes[ARGS_MAX];

	/**
	 * name required for Vstr
	 */
	char *name;
};

/**
 * Data to pass to a printf hook.
 */
struct printf_hook_data_t {

	/**
	 * Base to append printf to
	 */
	Vstr_base *base;

	/**
	 * Position in base to write to
	 */
	size_t pos;
};

/* A-Z | 6 other chars | a-z */
static printf_hook_handler_t *printf_hooks[58];

#define SPEC_TO_INDEX(spec) ((int)(spec) - (int)'A')

/**
 * These are used below, whenever the public wrapper functions are called before
 * initialization or after destruction.
 */
#undef vprintf
#undef vfprintf
#undef vsnprintf

/**
 * Vstr variant of print_in_hook()
 */
size_t print_in_hook(printf_hook_data_t *data, char *fmt, ...)
{
	size_t written;
	va_list args;

	va_start(args, fmt);
	written = vstr_add_vfmt(data->base, data->pos, fmt, args);
	va_end(args);

	data->pos += written;
	return written;
}

/**
 * Vstr custom format specifier callback function.
 */
static int custom_fmt_cb(Vstr_base *base, size_t pos, Vstr_fmt_spec *fmt_spec)
{
	int i;
	const void *args[ARGS_MAX];
	printf_hook_spec_t spec;
	printf_hook_handler_t *handler;
	printf_hook_data_t data = {
		.base = base,
		.pos = pos,
	};

	handler = printf_hooks[SPEC_TO_INDEX(fmt_spec->name[0])];
	for (i = 0; i < handler->numargs; i++)
	{
		switch (handler->argtypes[i])
		{
			case VSTR_TYPE_FMT_INT:
				args[i] = VSTR_FMT_CB_ARG_PTR(fmt_spec, i);
				break;
			case VSTR_TYPE_FMT_PTR_VOID:
				args[i] = &VSTR_FMT_CB_ARG_PTR(fmt_spec, i);
				break;
		}
	}

	spec.hash = fmt_spec->fmt_hash;
	spec.plus = fmt_spec->fmt_plus;
	spec.minus = fmt_spec->fmt_minus;
	spec.width = fmt_spec->obj_field_width;

	handler->hook(&data, &spec, args);

	return 1;
}

/**
 * Add a custom format handler to the given Vstr_conf object
 */
static void vstr_fmt_add_handler(Vstr_conf *conf, printf_hook_handler_t *handler)
{
	int *at;

	at = handler->argtypes;
	switch (handler->numargs)
	{
		case 1:
			vstr_fmt_add(conf, handler->name, custom_fmt_cb, at[0],
						 VSTR_TYPE_FMT_END);
			break;
		case 2:
			vstr_fmt_add(conf, handler->name, custom_fmt_cb, at[0],
						 at[1], VSTR_TYPE_FMT_END);
			break;
		case 3:
			vstr_fmt_add(conf, handler->name, custom_fmt_cb, at[0],
						 at[1], at[2], VSTR_TYPE_FMT_END);
			break;
	}
}

/**
 * Thread specific vstr config
 */
static thread_value_t *vstr_conf = NULL;

/**
 * Create vstr config for current thread
 */
static Vstr_conf *create_vstr_conf()
{
	Vstr_conf *conf;
	int i;

	conf = vstr_make_conf();
	vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '%');
	vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE,
						 VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR);
	vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_NUM_BUF_SZ, PRINTF_BUF_LEN);

	for (i = 0; i < countof(printf_hooks); i++)
	{
		if (printf_hooks[i])
		{
			vstr_fmt_add_handler(conf, printf_hooks[i]);
		}
	}
	return conf;
}

/**
 * Get vstr config of current thread
 */
static inline Vstr_conf *get_vstr_conf()
{
	Vstr_conf *conf = NULL;

	if (vstr_conf)
	{
		conf = (Vstr_conf*)vstr_conf->get(vstr_conf);
		if (!conf)
		{
			conf = create_vstr_conf();
			vstr_conf->set(vstr_conf, conf);
		}
	}
	return conf;
}

/**
 * Wrapper functions for printf and alike
 */
int vstr_wrapper_printf(const char *format, ...)
{
	int written;
	va_list args;
	va_start(args, format);
	written = vstr_wrapper_vprintf(format, args);
	va_end(args);
	return written;
}

int vstr_wrapper_fprintf(FILE *stream, const char *format, ...)
{
	int written;
	va_list args;
	va_start(args, format);
	written = vstr_wrapper_vfprintf(stream, format, args);
	va_end(args);
	return written;
}

int vstr_wrapper_sprintf(char *str, const char *format, ...)
{
	int written;
	va_list args;
	va_start(args, format);
	written = vstr_wrapper_vsprintf(str, format, args);
	va_end(args);
	return written;
}

int vstr_wrapper_snprintf(char *str, size_t size, const char *format, ...)
{
	int written;
	va_list args;
	va_start(args, format);
	written = vstr_wrapper_vsnprintf(str, size, format, args);
	va_end(args);
	return written;
}

int vstr_wrapper_asprintf(char **str, const char *format, ...)
{
	int written;
	va_list args;
	va_start(args, format);
	written = vstr_wrapper_vasprintf(str, format, args);
	va_end(args);
	return written;
}

static inline int vstr_wrapper_vprintf_internal(Vstr_conf *conf, FILE *stream,
												const char *format,
												va_list args)
{
	struct iovec *iov;
	int iovcnt, written = 0;
	Vstr_base *s;

	s = vstr_make_base(conf);
	vstr_add_vfmt(s, 0, format, args);
	if (vstr_export_iovec_ptr_all(s, &iov, &iovcnt))
	{
		while (iovcnt--)
		{
			if (iov->iov_base)
			{
				written += fwrite(iov->iov_base, 1, iov->iov_len, stream);
			}
			iov++;
		}
	}
	vstr_free_base(s);
	return written;
}

int vstr_wrapper_vprintf(const char *format, va_list args)
{
	Vstr_conf *conf;

	conf = get_vstr_conf();
	if (conf)
	{
		return vstr_wrapper_vprintf_internal(conf, stdout, format, args);
	}
	return vprintf(format, args);
}

int vstr_wrapper_vfprintf(FILE *stream, const char *format, va_list args)
{
	Vstr_conf *conf;

	conf = get_vstr_conf();
	if (conf)
	{
		return vstr_wrapper_vprintf_internal(conf, stream, format, args);
	}
	return vfprintf(stream, format, args);
}

static inline int vstr_wrapper_vsnprintf_internal(char *str, size_t size,
												  const char *format,
												  va_list args)
{
	Vstr_conf *conf;
	Vstr_base *s;
	int written;

	conf = get_vstr_conf();
	if (conf)
	{
		s = vstr_make_base(conf);
		vstr_add_vfmt(s, 0, format, args);
		written = s->len;
		vstr_export_cstr_buf(s, 1, s->len, str, (size > 0) ? size : s->len + 1);
		vstr_free_base(s);
		return written;
	}
	return vsnprintf(str, size, format, args);
}

int vstr_wrapper_vsprintf(char *str, const char *format, va_list args)
{
	return vstr_wrapper_vsnprintf_internal(str, 0, format, args);
}

int vstr_wrapper_vsnprintf(char *str, size_t size, const char *format,
						   va_list args)
{
	if (size > 0)
	{
		return vstr_wrapper_vsnprintf_internal(str, size, format, args);
	}
	return 0;
}

int vstr_wrapper_vasprintf(char **str, const char *format, va_list args)
{
	size_t len = 100;
	int written;

	*str = malloc(len);
	while (TRUE)
	{
		va_list ac;
		va_copy(ac, args);
		written = vstr_wrapper_vsnprintf_internal(*str, len, format, ac);
		va_end(ac);
		if (written < len)
		{
			break;
		}
		len = written + 1;
		*str = realloc(*str, len);
	}
	return written;
}

METHOD(printf_hook_t, add_handler, void,
	private_printf_hook_t *this, char spec, printf_hook_function_t hook, ...)
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
		if (++i >= ARGS_MAX)
		{
			DBG1(DBG_LIB, "Too many arguments for printf hook with "
				 "specifier '%c', not registered!", spec);
			failed = TRUE;
			break;
		}
		switch (argtype)
		{
			case PRINTF_HOOK_ARGTYPE_INT:
				handler->argtypes[i] = VSTR_TYPE_FMT_INT;
				break;
			case PRINTF_HOOK_ARGTYPE_POINTER:
				handler->argtypes[i] = VSTR_TYPE_FMT_PTR_VOID;
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
		Vstr_conf *conf = get_vstr_conf();
		handler->name = malloc(2);
		handler->name[0] = spec;
		handler->name[1] = '\0';
		vstr_fmt_add_handler(conf, handler);
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
	Vstr_conf *conf;
	printf_hook_handler_t *handler;

	conf = get_vstr_conf();
	for (i = 0; i < countof(printf_hooks); ++i)
	{
		handler = printf_hooks[i];
		if (handler)
		{
			vstr_fmt_del(conf, handler->name);
			free(handler->name);
			free(handler);
		}
	}

	/* freeing the Vstr_conf of the main thread */
	vstr_conf->destroy(vstr_conf);
	vstr_conf = NULL;
	vstr_exit();
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

	if (!vstr_init())
	{
		DBG1(DBG_LIB, "failed to initialize Vstr library!");
		free(this);
		return NULL;
	}
	vstr_conf = thread_value_create((thread_cleanup_t)vstr_free_conf);

	return &this->public;
}
