/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Internal definitions for events
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __event_internal_h
#define __event_internal_h

#include <event.h>
#include <linux/list.h>

/**
 * struct event_spy - a spy that watches for an event of a particular type
 *
 * @id: Spy ID
 * @type: Event type to subscribe to
 * @func: Function to call when the event is sent
 * @ctx: Context to pass to the function
 */
struct event_spy {
	struct list_head sibling_node;
	const char *id;
	enum event_t type;
	event_handler_t func;
	void *ctx;
};

struct event_state {
	struct list_head spy_head;
};

#endif
