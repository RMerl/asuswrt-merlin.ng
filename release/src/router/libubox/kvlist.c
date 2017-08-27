/*
 * kvlist - simple key/value store
 *
 * Copyright (C) 2014 Felix Fietkau <nbd@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "avl-cmp.h"
#include "blob.h"

#include "kvlist.h"

int kvlist_strlen(struct kvlist *kv, const void *data)
{
	return strlen(data) + 1;
}

int kvlist_blob_len(struct kvlist *kv, const void *data)
{
	return blob_pad_len(data);
}

void kvlist_init(struct kvlist *kv, int (*get_len)(struct kvlist *kv, const void *data))
{
	avl_init(&kv->avl, avl_strcmp, false, NULL);
	kv->get_len = get_len;
}

static struct kvlist_node *__kvlist_get(struct kvlist *kv, const char *name)
{
	struct kvlist_node *node;

	return avl_find_element(&kv->avl, name, node, avl);
}

void *kvlist_get(struct kvlist *kv, const char *name)
{
	struct kvlist_node *node;

	node = __kvlist_get(kv, name);
	if (!node)
		return NULL;

	return node->data;
}

bool kvlist_delete(struct kvlist *kv, const char *name)
{
	struct kvlist_node *node;

	node = __kvlist_get(kv, name);
	if (node) {
		avl_delete(&kv->avl, &node->avl);
		free(node);
	}

	return !!node;
}

bool kvlist_set(struct kvlist *kv, const char *name, const void *data)
{
	struct kvlist_node *node;
	char *name_buf;
	int len = kv->get_len(kv, data);

	node = calloc_a(sizeof(struct kvlist_node) + len,
		&name_buf, strlen(name) + 1);
	if (!node)
		return false;

	kvlist_delete(kv, name);

	memcpy(node->data, data, len);

	node->avl.key = strcpy(name_buf, name);
	avl_insert(&kv->avl, &node->avl);

	return true;
}

void kvlist_free(struct kvlist *kv)
{
	struct kvlist_node *node, *tmp;

	avl_remove_all_elements(&kv->avl, node, avl, tmp)
		free(node);
}
