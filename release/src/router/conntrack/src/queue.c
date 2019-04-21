/*
 * (C) 2006-2009 by Pablo Neira Ayuso <pablo@netfilter.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "queue.h"
#include "event.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

static LIST_HEAD(queue_list);	/* list of existing queues */
static uint32_t qobjects_num;	/* number of active queue objects */

struct queue *
queue_create(const char *name, int max_objects, unsigned int flags)
{
	struct queue *b;

	b = calloc(sizeof(struct queue), 1);
	if (b == NULL)
		return NULL;

	b->max_elems = max_objects;
	INIT_LIST_HEAD(&b->head);
	b->flags = flags;

	if (flags & QUEUE_F_EVFD) {
		b->evfd = create_evfd();
		if (b->evfd == NULL) {
			free(b);
			return NULL;
		}
	}
	strncpy(b->name, name, QUEUE_NAMELEN);
	b->name[QUEUE_NAMELEN-1]='\0';
	list_add(&b->list, &queue_list);

	return b;
}

void queue_destroy(struct queue *b)
{
	list_del(&b->list);
	if (b->flags & QUEUE_F_EVFD)
		destroy_evfd(b->evfd);
	free(b);
}

void queue_stats_show(int fd)
{
	struct queue *this;
	int size = 0;
	char buf[512];

	size += snprintf(buf+size, sizeof(buf),
			 "allocated queue nodes:\t\t%12u\n\n",
			 qobjects_num);

	list_for_each_entry(this, &queue_list, list) {
		size += snprintf(buf+size, sizeof(buf),
				 "queue %s:\n"
				 "current elements:\t\t%12u\n"
				 "maximum elements:\t\t%12u\n"
				 "not enough space errors:\t%12u\n\n",
				 this->name,
				 this->num_elems,
				 this->max_elems,
				 this->enospc_err);
	}
	send(fd, buf, size, 0);
}

void queue_node_init(struct queue_node *n, int type)
{
	INIT_LIST_HEAD(&n->head);
	n->type = type;
}

void *queue_node_data(struct queue_node *n)
{
	return ((char *)n) + sizeof(struct queue_node);
}

struct queue_object *queue_object_new(int type, size_t size)
{
	struct queue_object *obj;

	obj = calloc(sizeof(struct queue_object) + size, 1);
	if (obj == NULL)
		return NULL;

	obj->qnode.size = size;
	queue_node_init(&obj->qnode, type);
	qobjects_num++;

	return obj;
}

void queue_object_free(struct queue_object *obj)
{
	free(obj);
	qobjects_num--;
}

int queue_add(struct queue *b, struct queue_node *n)
{
	if (!list_empty(&n->head))
		return 0;

	if (b->num_elems >= b->max_elems) {
		b->enospc_err++;
		errno = ENOSPC;
		return -1;
	}
	n->owner = b;
	list_add_tail(&n->head, &b->head);
	b->num_elems++;
	if (b->evfd)
		write_evfd(b->evfd);
	return 1;
}

int queue_del(struct queue_node *n)
{
	if (list_empty(&n->head))
		return 0;

	list_del_init(&n->head);
	n->owner->num_elems--;
	if (n->owner->evfd)
		read_evfd(n->owner->evfd);
	n->owner = NULL;
	return 1;
}

struct queue_node *queue_del_head(struct queue *b)
{
	struct queue_node *n = (struct queue_node *) b->head.next;
	queue_del(n);
	return n;
}

int queue_in(struct queue *b, struct queue_node *n)
{
	return b == n->owner;
}

int queue_get_eventfd(struct queue *b)
{
	return get_read_evfd(b->evfd);
}

void queue_iterate(struct queue *b, 
		   const void *data, 
		   int (*iterate)(struct queue_node *n, const void *data2))
{
	struct list_head *i, *tmp;
	struct queue_node *n;

	list_for_each_safe(i, tmp, &b->head) {
		n = (struct queue_node *) i;
		if (iterate(n, data))
			break;
	}
}

unsigned int queue_len(const struct queue *b)
{
	return b->num_elems;
}
