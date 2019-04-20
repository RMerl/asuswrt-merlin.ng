/*
 * (C) 2005-2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "stack.h"

struct stack_item *
stack_item_alloc(int type, size_t data_len)
{
	struct stack_item *e;

	e = calloc(1, sizeof(struct stack_item) + data_len);
	if (e == NULL)
		return NULL;

	e->data_len = data_len;
	e->type = type;

	return e;
}

void stack_item_free(struct stack_item *e)
{
	free(e);
}

void stack_item_push(struct stack *s, struct stack_item *e)
{
	list_add(&e->head, &s->list);
}

struct stack_item *stack_item_pop(struct stack *s, int type)
{
	struct stack_item *cur, *tmp, *found = NULL;

	list_for_each_entry_safe(cur, tmp, &s->list, head) {
		if (cur->type != type && type != -1)
			continue;

		list_del(&cur->head);
		found = cur;
		break;
	}

	return found;
}
