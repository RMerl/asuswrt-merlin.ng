/*
 *  Bag of pointers
 *  Copyright (c) 2001 by Abramo Bagnara <abramo@alsa-project.org>
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include "mixer_local.h"

int bag_new(bag_t **bag)
{
	bag_t *b = malloc(sizeof(*b));
	if (!b)
		return -ENOMEM;
	INIT_LIST_HEAD(b);
	*bag = b;
	return 0;
}

void bag_free(bag_t *bag)
{
	assert(list_empty(bag));
	free(bag);
}

int bag_empty(bag_t *bag)
{
	return list_empty(bag);
}

int bag_add(bag_t *bag, void *ptr)
{
	bag1_t *b = malloc(sizeof(*b));
	if (!b)
		return -ENOMEM;
	b->ptr = ptr;
	list_add_tail(&b->list, bag);
	return 0;
}

int bag_del(bag_t *bag, void *ptr)
{
	struct list_head *pos;
	list_for_each(pos, bag) {
		bag1_t *b = list_entry(pos, bag1_t, list);
		if (b->ptr == ptr) {
			list_del(&b->list);
			free(b);
			return 0;
		}
	}
	return -ENOENT;
}

void bag_del_all(bag_t *bag)
{
	while (!list_empty(bag))
		list_del(bag->next);
}
