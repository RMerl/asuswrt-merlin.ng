/*
 * Copyright (C) 2006-2016  B.A.T.M.A.N. contributors:
 *
 * Marek Lindner
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

#ifndef _LINUX_LIST_H
#define _LINUX_LIST_H

/*
 * XXX: Resolve conflict between this file and <sys/queue.h> on BSD systems.
 */
#ifdef LIST_HEAD
#undef LIST_HEAD
#endif

/*
 * Simple linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct list_head {
	struct list_head *next;
};

struct list_head_first {
	struct list_head *next, *prev;
};


void list_add( struct list_head *new, struct list_head_first *head );
void list_add_tail( struct list_head *new, struct list_head_first *head );
void list_add_before( struct list_head *prev_node, struct list_head *next_node, struct list_head *new_node );
void list_del( struct list_head *prev_entry, struct list_head *entry, struct list_head_first *head );
int list_empty( struct list_head_first *head );



#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); \
} while (0)

#define INIT_LIST_HEAD_FIRST(ptr) \
	ptr.next = (struct list_head *)&ptr; ptr.prev = (struct list_head *)&ptr; \


/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (struct list_head *)(head); \
        	pos = pos->next)

/**
 * list_for_each_safe	-	iterate over a list safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop counter.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (struct list_head *)(head); \
		pos = n, n = pos->next)

/**
 * list_for_each_entry  -       iterate over list of given type
 * @pos:        the type * to use as a loop counter.
 * @head:       the head for your list.
 * @member:     the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, member)                          \
        for (pos = list_entry((head)->next, typeof(*pos), member);      \
             &pos->member != (struct list_head *)(head);                \
             pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:        the type * to use as a loop counter.
 * @n:          another type * to use as temporary storage
 * @head:       the head for your list.
 * @member:     the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member)                  \
        for (pos = list_entry((head)->next, typeof(*pos), member),      \
                n = list_entry(pos->member.next, typeof(*pos), member); \
             &pos->member != (struct list_head *)(head);                \
             pos = n, n = list_entry(n->member.next, typeof(*n), member))


#endif
