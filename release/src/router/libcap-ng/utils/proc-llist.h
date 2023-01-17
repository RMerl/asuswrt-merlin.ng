/*
* proc-llist.h
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

#ifndef PROC_HEADER
#define PROC_HEADER

#include "config.h"
#include <sys/types.h>	/* Ensure types in _lnode are defined on all systems */


/* This is the node of the linked list. Any data elements that are per
 *  record goes here. */
typedef struct _lnode{
  pid_t ppid;           // parent process ID
  pid_t pid;            // process ID
  uid_t uid;            // user ID
  char *cmd;		// command run by user
  unsigned long inode;	// inode of socket
  char *capabilities;	// Text of partial capabilities
  char *bounds;		// Text for bounding set
  char *ambient;	// Text for ambient set
  struct _lnode* next;	// Next node pointer
} lnode;

/* This is the linked list head. Only data elements that are 1 per
 * event goes here. */
typedef struct {
  lnode *head;		// List head
  lnode *cur;		// Pointer to current node
  unsigned int cnt;	// How many items in this list
} llist;

void list_create(llist *l);
static inline lnode *list_get_cur(llist *l) { return l->cur; }
void list_append(llist *l, lnode *node);
void list_clear(llist* l);

/* Given a message type, find the matching node */
lnode *list_find_inode(llist *l, unsigned long i);

#endif

