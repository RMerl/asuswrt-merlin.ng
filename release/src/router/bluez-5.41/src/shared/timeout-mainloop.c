/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 */

#include <stdlib.h>

#include "mainloop.h"
#include "util.h"
#include "timeout.h"

struct timeout_data {
	int id;
	timeout_func_t func;
	timeout_destroy_func_t destroy;
	unsigned int timeout;
	void *user_data;
};

static void timeout_callback(int id, void *user_data)
{
	struct timeout_data *data = user_data;

	if (data->func(data->user_data) &&
			!mainloop_modify_timeout(data->id, data->timeout))
		return;

	mainloop_remove_timeout(data->id);
}

static void timeout_destroy(void *user_data)
{
	struct timeout_data *data = user_data;

	if (data->destroy)
		data->destroy(data->user_data);

	free(data);
}

unsigned int timeout_add(unsigned int timeout, timeout_func_t func,
			void *user_data, timeout_destroy_func_t destroy)
{
	struct timeout_data *data;

	data = new0(struct timeout_data, 1);
	data->func = func;
	data->user_data = user_data;
	data->timeout = timeout;
	data->destroy = destroy;

	data->id = mainloop_add_timeout(timeout, timeout_callback, data,
							timeout_destroy);
	if (data->id < 0) {
		free(data);
		return 0;
	}

	return (unsigned int) data->id;
}

void timeout_remove(unsigned int id)
{
	if (!id)
		return;

	mainloop_remove_timeout((int) id);
}
