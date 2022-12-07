// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2019  Intel Corporation. All rights reserved.
 *
 *
 */

#include <ell/ell.h>

#include "timeout.h"

static struct l_queue *timeout_q;

struct timeout_data {
	timeout_func_t func;
	timeout_destroy_func_t destroy;
	void *user_data;
	unsigned int timeout;
};

static bool match_id(const void *a, const void *b)
{
	unsigned int to_id = L_PTR_TO_UINT(a);
	unsigned int id = L_PTR_TO_UINT(b);

	return (to_id == id);
}

static void timeout_callback(struct l_timeout *timeout, void *user_data)
{
	struct timeout_data *data = user_data;

	if (data->func)
		data->func(data->user_data);

	l_timeout_modify(timeout, data->timeout);
}

static void timeout_destroy(void *user_data)
{
	struct timeout_data *data = user_data;

	if (data->destroy)
		data->destroy(data->user_data);

	l_free(data);
}

unsigned int timeout_add(unsigned int timeout, timeout_func_t func,
			void *user_data, timeout_destroy_func_t destroy)
{
	struct timeout_data *data;
	unsigned int id = 0;
	struct l_timeout *to;
	int tries = 0;

	if (!timeout_q)
		timeout_q = l_queue_new();

	data = l_new(struct timeout_data, 1);

	data->func = func;
	data->destroy = destroy;
	data->user_data = user_data;
	data->timeout = timeout;

	while (id == 0 && tries < 3) {
		to = l_timeout_create(timeout, timeout_callback,
							data, timeout_destroy);
		if (!to)
			break;

		tries++;
		id = L_PTR_TO_UINT(to);

		if (id == 0 ||
			l_queue_find(timeout_q, match_id, L_UINT_TO_PTR(id))) {

			l_timeout_remove(to);
			continue;
		}

		l_queue_push_tail(timeout_q, to);
	}

	if (id == 0)
		l_free(data);

	return id;
}

void timeout_remove(unsigned int id)
{
	struct l_timeout *to;

	to = l_queue_remove_if(timeout_q, match_id, L_UINT_TO_PTR(id));

	if (to)
		l_timeout_remove(to);
}
