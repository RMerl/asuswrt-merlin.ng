/*
 * Copyright (C) 2009-2016  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */

#ifndef _BATCTL_BISECT_IV_H
#define _BATCTL_BISECT_IV_H

#include "list-batman.h"

#define NAME_LEN 18
#define MAX_LINE 256
#define LOOP_MAGIC_LEN ((2 * NAME_LEN) + (2 * sizeof(int)) - 2)

#define RT_FLAG_ADD 1
#define RT_FLAG_UPDATE 2
#define RT_FLAG_DELETE 3

int bisect_iv(int argc, char **argv);



struct bat_node {
	char name[NAME_LEN];
	struct list_head_first orig_event_list;
	struct list_head_first rt_table_list;
	char loop_magic[LOOP_MAGIC_LEN];
	char loop_magic2[LOOP_MAGIC_LEN];
};

struct orig_event {
	struct list_head list;
	struct bat_node *orig_node;
	struct list_head_first event_list;
	struct list_head_first rt_hist_list;
};

struct rt_table {
	struct list_head list;
	int num_entries;
	struct rt_entry *entries;
	struct rt_hist *rt_hist;
};

struct rt_hist {
	struct list_head list;
	struct rt_table *rt_table;
	struct rt_hist *prev_rt_hist;
	struct seqno_event *seqno_event;
	struct bat_node *next_hop;
	char flags;
	char loop_magic[LOOP_MAGIC_LEN];
};

struct rt_entry {
	char orig[NAME_LEN];
	struct bat_node *next_hop;
	char flags;
};

struct seqno_event {
	struct list_head list;
	struct bat_node *orig;
	struct bat_node *neigh;
	struct bat_node *prev_sender;
	long long seqno;
	int tq;
	int ttl;
	struct rt_hist *rt_hist;
};

struct seqno_trace_neigh {
	struct bat_node *bat_node;
	struct seqno_event *seqno_event;
	int num_neighbors;
	struct seqno_trace_neigh **seqno_trace_neigh;
};

struct seqno_trace {
	struct list_head list;
	long long seqno;
	char print;
	struct seqno_trace_neigh seqno_trace_neigh;
};

#endif
