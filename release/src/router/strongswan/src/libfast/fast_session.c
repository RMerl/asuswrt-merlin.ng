/*
 * Copyright (C) 2007 Martin Willi
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

#define _GNU_SOURCE

#include "fast_session.h"

#include <string.h>
#include <fcgiapp.h>
#include <stdio.h>

#include <collections/linked_list.h>

#define COOKIE_LEN 16

typedef struct private_fast_session_t private_fast_session_t;

/**
 * private data of the task manager
 */
struct private_fast_session_t {

	/**
	 * public functions
	 */
	fast_session_t public;

	/**
	 * session ID
	 */
	char sid[COOKIE_LEN * 2 + 1];

	/**
	 * have we sent the session cookie?
	 */
	bool cookie_sent;

	/**
	 * list of controller instances controller_t
	 */
	linked_list_t *controllers;

	/**
	 * list of filter instances filter_t
	 */
	linked_list_t *filters;

	/**
	 * user defined session context
	 */
	fast_context_t *context;
};

METHOD(fast_session_t, add_controller, void,
	private_fast_session_t *this, fast_controller_t *controller)
{
	this->controllers->insert_last(this->controllers, controller);
}

METHOD(fast_session_t, add_filter, void,
	private_fast_session_t *this, fast_filter_t *filter)
{
	this->filters->insert_last(this->filters, filter);
}

/**
 * Create a session ID and a cookie
 */
static bool create_sid(private_fast_session_t *this)
{
	char buf[COOKIE_LEN];
	rng_t *rng;

	rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (!rng)
	{
		return FALSE;
	}
	if (!rng->get_bytes(rng, sizeof(buf), buf))
	{
		rng->destroy(rng);
		return FALSE;
	}
	rng->destroy(rng);
	chunk_to_hex(chunk_create(buf, sizeof(buf)), this->sid, FALSE);
	return TRUE;
}

/**
 * run all registered filters
 */
static bool run_filter(private_fast_session_t *this, fast_request_t *request,
					char *p0, char *p1, char *p2, char *p3, char *p4, char *p5)
{
	enumerator_t *enumerator;
	fast_filter_t *filter;

	enumerator = this->filters->create_enumerator(this->filters);
	while (enumerator->enumerate(enumerator, &filter))
	{
		if (!filter->run(filter, request, p0, p1, p2, p3, p4, p5))
		{
			enumerator->destroy(enumerator);
			return FALSE;
		}
	}
	enumerator->destroy(enumerator);
	return TRUE;
}

METHOD(fast_session_t, process, void,
	private_fast_session_t *this, fast_request_t *request)
{
	char *pos, *start, *param[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
	enumerator_t *enumerator;
	bool handled = FALSE;
	fast_controller_t *current;
	int i = 0;

	if (!this->cookie_sent)
	{
		request->add_cookie(request, "SID", this->sid);
		this->cookie_sent = TRUE;
	}

	start = request->get_path(request);
	if (start)
	{
		if (*start == '/')
		{
			start++;
		}
		while ((pos = strchr(start, '/')) != NULL && i < 5)
		{
			param[i++] = strndupa(start, pos - start);
			start = pos + 1;
		}
		param[i] = strdupa(start);

		if (run_filter(this, request, param[0], param[1], param[2], param[3],
						param[4], param[5]))
		{
			enumerator = this->controllers->create_enumerator(this->controllers);
			while (enumerator->enumerate(enumerator, &current))
			{
				if (streq(current->get_name(current), param[0]))
				{
					current->handle(current, request, param[1], param[2],
									param[3], param[4], param[5]);
					handled = TRUE;
					break;
				}
			}
			enumerator->destroy(enumerator);
		}
		else
		{
			handled = TRUE;
		}
	}
	if (!handled)
	{
		if (this->controllers->get_first(this->controllers,
										 (void**)&current) == SUCCESS)
		{
			request->streamf(request,
				"Status: 301 Moved permanently\nLocation: %s/%s\n\n",
				request->get_base(request), current->get_name(current));
		}
	}
}

METHOD(fast_session_t, get_sid, char*,
	private_fast_session_t *this)
{
	return this->sid;
}

METHOD(fast_session_t, destroy, void,
	private_fast_session_t *this)
{
	this->controllers->destroy_offset(this->controllers,
									  offsetof(fast_controller_t, destroy));
	this->filters->destroy_offset(this->filters,
									  offsetof(fast_filter_t, destroy));
	DESTROY_IF(this->context);
	free(this);
}

/*
 * see header file
 */
fast_session_t *fast_session_create(fast_context_t *context)
{
	private_fast_session_t *this;

	INIT(this,
		.public = {
			.add_controller = _add_controller,
			.add_filter = _add_filter,
			.process = _process,
			.get_sid = _get_sid,
			.destroy = _destroy,
		},
		.controllers = linked_list_create(),
		.filters = linked_list_create(),
		.context = context,
	);
	if (!create_sid(this))
	{
		destroy(this);
		return NULL;
	}

	return &this->public;
}
