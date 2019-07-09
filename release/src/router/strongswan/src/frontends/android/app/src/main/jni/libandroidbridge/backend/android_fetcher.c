/*
 * Copyright (C) 2017 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.  *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "android_fetcher.h"

#include "../android_jni.h"
#include "../charonservice.h"
#include <utils/debug.h>

typedef struct android_fetcher_t android_fetcher_t;

struct android_fetcher_t {

	/**
	 * Public interface
	 */
	fetcher_t public;

	/**
	 * Callback function
	 */
	fetcher_callback_t cb;

	/**
	 * Data to POST
	 */
	chunk_t data;

	/**
	 * Type of data to POST
	 */
	char *request_type;
};

METHOD(fetcher_t, fetch, status_t,
	android_fetcher_t *this, char *uri, void *userdata)
{
	JNIEnv *env;
	jmethodID method_id;
	jobjectArray jdata = NULL;
	jstring juri, jct = NULL;
	chunk_t data;
	status_t status = FAILED;

	if (this->cb == fetcher_default_callback)
	{
		*(chunk_t*)userdata = chunk_empty;
	}

	androidjni_attach_thread(&env);
	/* can't use FindClass here as this is not called by the main thread */
	method_id = (*env)->GetStaticMethodID(env, android_simple_fetcher_class,
						"fetch", "(Ljava/lang/String;[BLjava/lang/String;)[B");
	if (!method_id)
	{
		goto failed;
	}
	juri = (*env)->NewStringUTF(env, uri);
	if (!juri)
	{
		goto failed;
	}
	if (this->request_type)
	{
		jct = (*env)->NewStringUTF(env, this->request_type);
		if (!jct)
		{
			goto failed;
		}
	}
	if (this->data.ptr)
	{
		jdata = byte_array_from_chunk(env, this->data);
		if (!jdata)
		{
			goto failed;
		}
	}
	jdata = (*env)->CallStaticObjectMethod(env, android_simple_fetcher_class,
										   method_id, juri, jdata, jct);
	if (!jdata || androidjni_exception_occurred(env))
	{
		goto failed;
	}
	data = chunk_from_byte_array(env, jdata);
	if (this->cb(userdata, data))
	{
		status = SUCCESS;
	}
	chunk_free(&data);
	androidjni_detach_thread();
	return status;

failed:
	DBG1(DBG_LIB, "failed to fetch from '%s'", uri);
	androidjni_exception_occurred(env);
	androidjni_detach_thread();
	return FAILED;
}

METHOD(fetcher_t, set_option, bool,
	android_fetcher_t *this, fetcher_option_t option, ...)
{
	bool supported = TRUE;
	va_list args;

	va_start(args, option);
	switch (option)
	{
		case FETCH_CALLBACK:
		{
			this->cb = va_arg(args, fetcher_callback_t);
			break;
		}
		case FETCH_REQUEST_DATA:
		{
			this->data = chunk_clone(va_arg(args, chunk_t));
			break;
		}
		case FETCH_REQUEST_TYPE:
		{
			this->request_type = strdup(va_arg(args, char*));
			break;
		}
		default:
			supported = FALSE;
			break;
	}
	va_end(args);
	return supported;
}

METHOD(fetcher_t, destroy, void,
	android_fetcher_t *this)
{
	chunk_clear(&this->data);
	free(this->request_type);
	free(this);
}

/*
 * Described in header.
 */
fetcher_t *android_fetcher_create()
{
	android_fetcher_t *this;

	INIT(this,
		.public = {
			.fetch = _fetch,
			.set_option = _set_option,
			.destroy = _destroy,
		},
		.cb = fetcher_default_callback,
	);

	return &this->public;
}
