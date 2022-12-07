/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2016  Intel Corporation. All rights reserved.
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
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "queue.h"
#include "hashmap.h"
#include "string.h"
#include "dbus.h"
#include "dbus-private.h"
#include "gvariant-private.h"
#include "private.h"

#define NODE_TYPE_CALLBACK	L_DBUS_MATCH_NONE

struct filter_node {
	enum l_dbus_match_type type;
	union {
		struct {
			char *value;
			struct filter_node *children;
			bool remote_rule;
		} match;
		struct {
			l_dbus_message_func_t func;
			void *user_data;
		} callback;
	};
	unsigned int id;
	struct filter_node *next;
};

struct _dbus_filter {
	struct l_dbus *dbus;
	struct filter_node *root;
	unsigned int signal_id;
	unsigned int last_id;
	const struct _dbus_filter_ops *driver;
	struct _dbus_name_cache *name_cache;
};

static void filter_subtree_free(struct filter_node *node)
{
	struct filter_node *child, *next;

	if (node->type == NODE_TYPE_CALLBACK) {
		l_free(node);
		return;
	}

	next = node->match.children;

	l_free(node->match.value);
	l_free(node);

	while (next) {
		child = next;
		next = child->next;

		filter_subtree_free(child);
	}
}

static void dbus_filter_destroy(void *data)
{
	struct _dbus_filter *filter = data;

	if (filter->root)
		filter_subtree_free(filter->root);

	l_free(filter);
}

static void filter_dispatch_match_recurse(struct _dbus_filter *filter,
						struct filter_node *node,
						struct l_dbus_message *message)
{
	const char *value = NULL;
	const char *alt_value = NULL;
	struct filter_node *child;

	switch ((int) node->type) {
	case NODE_TYPE_CALLBACK:
		node->callback.func(message, node->callback.user_data);
		return;

	case L_DBUS_MATCH_SENDER:
		value = l_dbus_message_get_sender(message);
		break;

	case L_DBUS_MATCH_TYPE:
		value = _dbus_message_get_type_as_string(message);
		break;

	case L_DBUS_MATCH_PATH:
		value = l_dbus_message_get_path(message);
		break;

	case L_DBUS_MATCH_INTERFACE:
		value = l_dbus_message_get_interface(message);
		break;

	case L_DBUS_MATCH_MEMBER:
		value = l_dbus_message_get_member(message);
		break;

	case L_DBUS_MATCH_ARG0...(L_DBUS_MATCH_ARG0 + 63):
		value = _dbus_message_get_nth_string_argument(message,
						node->type - L_DBUS_MATCH_ARG0);
		break;
	}

	if (!value)
		return;

	if (node->type == L_DBUS_MATCH_SENDER && filter->name_cache)
		alt_value = _dbus_name_cache_lookup(filter->name_cache,
							node->match.value);

	if (strcmp(value, node->match.value) &&
			(!alt_value || strcmp(value, alt_value)))
		return;

	for (child = node->match.children; child; child = child->next)
		filter_dispatch_match_recurse(filter, child, message);
}

void _dbus_filter_dispatch(struct l_dbus_message *message, void *user_data)
{
	struct _dbus_filter *filter = user_data;

	filter_dispatch_match_recurse(filter, filter->root, message);
}

struct _dbus_filter *_dbus_filter_new(struct l_dbus *dbus,
					const struct _dbus_filter_ops *driver,
					struct _dbus_name_cache *name_cache)
{
	struct _dbus_filter *filter;

	filter = l_new(struct _dbus_filter, 1);

	filter->dbus = dbus;
	filter->driver = driver;
	filter->name_cache = name_cache;

	if (!filter->driver->skip_register)
		filter->signal_id = l_dbus_register(dbus, _dbus_filter_dispatch,
							filter,
							dbus_filter_destroy);

	return filter;
}

void _dbus_filter_free(struct _dbus_filter *filter)
{
	if (!filter)
		return;

	if (!filter->driver->skip_register)
		l_dbus_unregister(filter->dbus, filter->signal_id);
	else
		dbus_filter_destroy(filter);
}

static int condition_compare(const void *a, const void *b)
{
	const struct _dbus_filter_condition *condition_a = a, *condition_b = b;

	return condition_a->type - condition_b->type;
}

static bool remove_recurse(struct _dbus_filter *filter,
				struct filter_node **node, unsigned int id)
{
	struct filter_node *tmp;

	for (; *node; node = &(*node)->next) {
		if ((*node)->type == NODE_TYPE_CALLBACK && (*node)->id == id)
			break;

		if ((*node)->type != NODE_TYPE_CALLBACK &&
				remove_recurse(filter, &(*node)->match.children,
						id))
			break;
	}

	if (!*node)
		return false;

	if ((*node)->type == NODE_TYPE_CALLBACK || !(*node)->match.children) {
		tmp = *node;
		*node = tmp->next;

		if (tmp->match.remote_rule)
			filter->driver->remove_match(filter->dbus, tmp->id);

		if (tmp->type == L_DBUS_MATCH_SENDER && filter->name_cache &&
				!_dbus_parse_unique_name(tmp->match.value,
								NULL))
			_dbus_name_cache_remove(filter->name_cache,
						tmp->match.value);

		filter_subtree_free(tmp);
	}

	return true;
}

unsigned int _dbus_filter_add_rule(struct _dbus_filter *filter,
				const struct _dbus_filter_condition *rule,
				int rule_len,
				l_dbus_message_func_t signal_func,
				void *user_data)
{
	struct filter_node **node_ptr = &filter->root;
	struct filter_node *uninitialized_var(node);
	struct filter_node *parent = filter->root;
	bool remote_rule = false;
	struct _dbus_filter_condition sorted[rule_len];
	struct _dbus_filter_condition *unused;
	struct _dbus_filter_condition *uninitialized_var(condition);
	struct _dbus_filter_condition *end = sorted + rule_len;

	memcpy(sorted, rule, sizeof(sorted));
	qsort(sorted, rule_len, sizeof(*condition), condition_compare);

	/*
	 * Find or create a path in the tree with a node for each
	 * condition in the rule, loop until all conditions have been
	 * used.
	 */
	unused = sorted;
	while (unused < end) {
		/*
		 * Find a child of the node that matches any unused
		 * condition.  Note there could be multiple matches, we're
		 * happy with the first we can find.
		 */
		while (*node_ptr) {
			node = *node_ptr;

			for (condition = unused; condition < end; condition++) {
				if (condition->type > node->type) {
					condition = end;
					break;
				}

				if (condition->type < node->type ||
						condition->type ==
						L_DBUS_MATCH_NONE)
					continue;

				if (!strcmp(node->match.value,
						condition->value))
					break;
			}

			if (condition < end)
				break;

			node_ptr = &node->next;
		}

		/* Add a node */
		if (!*node_ptr) {
			condition = unused;

			node = l_new(struct filter_node, 1);
			node->type = condition->type;
			node->match.value = l_strdup(condition->value);

			*node_ptr = node;

			if (node->type == L_DBUS_MATCH_SENDER &&
					filter->name_cache &&
					!_dbus_parse_unique_name(
							node->match.value,
							NULL))
				_dbus_name_cache_add(filter->name_cache,
							node->match.value);

		}

		/*
		 * Mark the condition used.  We do this by setting
		 * condition->type to an invalid value unless it is the
		 * first condition left in which case we can push the
		 * rule start.  Another option is to always push the rule
		 * start and memmove the still unused conditions by one
		 * if necessary.
		 */
		condition->type = L_DBUS_MATCH_NONE;
		while (unused < end && unused[0].type == L_DBUS_MATCH_NONE)
			unused++;

		node_ptr = &node->match.children;

		parent = node;

		/*
		 * Only have to call AddMatch if none of the parent nodes
		 * have yet created an AddMatch rule on the server.
		 */
		remote_rule |= node->match.remote_rule;
	}

	node = l_new(struct filter_node, 1);
	node->type = NODE_TYPE_CALLBACK;
	node->callback.func = signal_func;
	node->callback.user_data = user_data;
	node->id = ++filter->last_id;
	node->next = *node_ptr;

	*node_ptr = node;

	if (!remote_rule) {
		if (!filter->driver->add_match(filter->dbus, node->id,
						rule, rule_len))
			goto err;

		parent->id = node->id;
		parent->match.remote_rule = true;
	}

	return node->id;

err:
	/* Remove all the nodes we may have added */
	node->id = (unsigned int) -1;
	remove_recurse(filter, &filter->root, node->id);

	return 0;
}

bool _dbus_filter_remove_rule(struct _dbus_filter *filter, unsigned int id)
{
	return remove_recurse(filter, &filter->root, id);
}

char *_dbus_filter_rule_to_str(const struct _dbus_filter_condition *rule,
				int rule_len)
{
	struct l_string *str = l_string_new(63);
	char *key, arg_buf[6];
	const char *value, *endp;

	for (; rule_len; rule++, rule_len--) {
		switch ((int) rule->type) {
		case L_DBUS_MATCH_SENDER:
			key = "sender";
			break;
		case L_DBUS_MATCH_TYPE:
			key = "type";
			break;
		case L_DBUS_MATCH_PATH:
			key = "path";
			break;
		case L_DBUS_MATCH_INTERFACE:
			key = "interface";
			break;
		case L_DBUS_MATCH_MEMBER:
			key = "member";
			break;
		case L_DBUS_MATCH_ARG0...(L_DBUS_MATCH_ARG0 + 63):
			key = arg_buf;
			snprintf(arg_buf, sizeof(arg_buf), "arg%i",
					rule->type - L_DBUS_MATCH_ARG0);
			break;
		default:
			l_string_free(str);
			return NULL;
		}

		l_string_append(str, key);
		l_string_append(str, "='");

		/* We only need to escape single-quotes in the values */
		value = rule->value;

		while ((endp = strchr(value, '\''))) {
			l_string_append_fixed(str, value, endp - value);
			l_string_append(str, "'\\''");

			value = endp + 1;
		}

		l_string_append(str, value);
		l_string_append_c(str, '\'');

		if (rule_len > 1)
			l_string_append_c(str, ',');
	}

	return l_string_unwrap(str);
}
