/*
 * Copyright (C) 2009 Red Hat Inc, Steven Rostedt <srostedt@redhat.com>
 * Copyright (C) 2009 Johannes Berg <johannes@sipsolutions.net>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License (not later!)
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not,  see <http://www.gnu.org/licenses>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "event-parse.h"

static int timer_expire_handler(struct trace_seq *s,
				struct pevent_record *record,
				struct event_format *event, void *context)
{
	trace_seq_printf(s, "hrtimer=");

	if (pevent_print_num_field(s, "0x%llx", event, "timer",
				   record, 0) == -1)
		pevent_print_num_field(s, "0x%llx", event, "hrtimer",
				       record, 1);

	trace_seq_printf(s, " now=");

	pevent_print_num_field(s, "%llu", event, "now", record, 1);

	pevent_print_func_field(s, " function=%s", event, "function",
				record, 0);
	return 0;
}

static int timer_start_handler(struct trace_seq *s,
			       struct pevent_record *record,
			       struct event_format *event, void *context)
{
	trace_seq_printf(s, "hrtimer=");

	if (pevent_print_num_field(s, "0x%llx", event, "timer",
				   record, 0) == -1)
		pevent_print_num_field(s, "0x%llx", event, "hrtimer",
				       record, 1);

	pevent_print_func_field(s, " function=%s", event, "function",
				record, 0);

	trace_seq_printf(s, " expires=");
	pevent_print_num_field(s, "%llu", event, "expires", record, 1);

	trace_seq_printf(s, " softexpires=");
	pevent_print_num_field(s, "%llu", event, "softexpires", record, 1);
	return 0;
}

int PEVENT_PLUGIN_LOADER(struct pevent *pevent)
{
	pevent_register_event_handler(pevent, -1,
				      "timer", "hrtimer_expire_entry",
				      timer_expire_handler, NULL);

	pevent_register_event_handler(pevent, -1, "timer", "hrtimer_start",
				      timer_start_handler, NULL);
	return 0;
}

void PEVENT_PLUGIN_UNLOADER(struct pevent *pevent)
{
	pevent_unregister_event_handler(pevent, -1,
					"timer", "hrtimer_expire_entry",
					timer_expire_handler, NULL);

	pevent_unregister_event_handler(pevent, -1, "timer", "hrtimer_start",
					timer_start_handler, NULL);
}
