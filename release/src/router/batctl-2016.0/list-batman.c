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



#include "list-batman.h"



/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the next entries already!
 */
static void __list_add( struct list_head *new, struct list_head *prev, struct list_head *next ) {

	new->next = next;
	prev->next = new;

}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
void list_add( struct list_head *new, struct list_head_first *head ) {

	__list_add( new, (struct list_head *)head, head->next );

	if ( head->prev == (struct list_head *)head )
		head->prev = new;

}

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
void list_add_tail( struct list_head *new, struct list_head_first *head ) {

	__list_add( new, head->prev, (struct list_head *)head );

	head->prev = new;

}

void list_add_before( struct list_head *prev_node, struct list_head *next_node, struct list_head *new_node ) {

	prev_node->next = new_node;
	new_node->next = next_node;

}



/*
 * Delete a list entry by making the next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the next entries already!
 */
static void __list_del( struct list_head *prev, struct list_head *next ) {

	prev->next = next;

}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty on entry does not return true after this, the entry is in an undefined state.
 */
void list_del( struct list_head *prev_entry, struct list_head *entry, struct list_head_first *head ) {

	if ( head->prev == entry )
		head->prev = prev_entry;

	__list_del( prev_entry, entry->next );

	entry->next = (void *) 0;

}



/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
int list_empty( struct list_head_first *head ) {

	return head->next == (struct list_head *)head;

}

