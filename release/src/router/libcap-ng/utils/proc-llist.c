/*
* proc-llist.c - Minimal linked list library
* Copyright (c) 2009, 2020 Red Hat Inc.
* All Rights Reserved.
*
* This software may be freely redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free
* Software Foundation; either version 2, or (at your option) any
* later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; see the file COPYING. If not, write to the
* Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor
* Boston, MA 02110-1335, USA.
*
* Authors:
*   Steve Grubb <sgrubb@redhat.com>
*/

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include "proc-llist.h"

void list_create(llist *l)
{
	l->head = NULL;
	l->cur = NULL;
	l->cnt = 0;
}

void list_append(llist *l, lnode *node)
{
	lnode* newnode;

	if (node == NULL || l == NULL)
		return;

	newnode = malloc(sizeof(lnode));
	if (newnode == NULL)
		return;

	newnode->ppid = node->ppid;
	newnode->pid = node->pid;
	newnode->uid = node->uid;
	newnode->inode = node->inode;
	// Take custody of the memory
	newnode->cmd = node->cmd;
	newnode->capabilities = node->capabilities;
	newnode->bounds = node->bounds;
	newnode->ambient = node->ambient;
	newnode->next = NULL;

	// if we are at top, fix this up
	if (l->head == NULL)
		l->head = newnode;
	else	// Otherwise add pointer to newnode
		l->cur->next = newnode;

	// make newnode current
	l->cur = newnode;
	l->cnt++;
}

void list_clear(llist* l)
{
	lnode* nextnode;
	register lnode* cur;

	cur = l->head;
	while (cur) {
		nextnode=cur->next;
		free(cur->cmd);
		free(cur->capabilities);
		free(cur->bounds);
		free(cur->ambient);
		free(cur);
		cur=nextnode;
	}
	l->head = NULL;
	l->cur = NULL;
	l->cnt = 0;
}

lnode *list_find_inode(llist *l, unsigned long i)
{
        register lnode* cur;

	cur = l->head;	/* start at the beginning */
	while (cur) {
		if (cur->inode == i) {
			l->cur = cur;
			return cur;
		} else
			cur = cur->next;
	}
	return NULL;
}

