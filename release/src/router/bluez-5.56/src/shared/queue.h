/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 */

#include <stdbool.h>

typedef void (*queue_destroy_func_t)(void *data);

struct queue;

struct queue_entry {
	void *data;
	struct queue_entry *next;
};

struct queue *queue_new(void);
void queue_destroy(struct queue *queue, queue_destroy_func_t destroy);

bool queue_push_tail(struct queue *queue, void *data);
bool queue_push_head(struct queue *queue, void *data);
bool queue_push_after(struct queue *queue, void *entry, void *data);
void *queue_pop_head(struct queue *queue);
void *queue_peek_head(struct queue *queue);
void *queue_peek_tail(struct queue *queue);

typedef void (*queue_foreach_func_t)(void *data, void *user_data);

void queue_foreach(struct queue *queue, queue_foreach_func_t function,
							void *user_data);

typedef bool (*queue_match_func_t)(const void *data, const void *match_data);

void *queue_find(struct queue *queue, queue_match_func_t function,
							const void *match_data);

bool queue_remove(struct queue *queue, void *data);
void *queue_remove_if(struct queue *queue, queue_match_func_t function,
							void *user_data);
unsigned int queue_remove_all(struct queue *queue, queue_match_func_t function,
				void *user_data, queue_destroy_func_t destroy);

const struct queue_entry *queue_get_entries(struct queue *queue);

unsigned int queue_length(struct queue *queue);
bool queue_isempty(struct queue *queue);
