#ifndef _dll_t_
#define _dll_t_
/*
<:copyright-BRCM:2014:DUAL/GPL:standard 

   Copyright (c) 2014 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#if !defined(_envelope_of)
/* derived from container_of, without "const", for gcc -Wcast-qual compile */
#define _envelope_of(ptr, type, member) \
({ \
	typeof(((type *)0)->member) *__mptr = (ptr); \
	(type *)((char *)__mptr - offsetof(type, member)); \
})
#endif /* _envelope_of */


typedef struct dll_t dll_t; /* common to wlan bcmutils.h, pktHdr.h */
typedef struct dll_t {
	dll_t * next_p;
	dll_t * prev_p;
} Dll_t, * PDll_t;

#define dll dll_t

#define DLL_STRUCT_INITIALIZER(struct_name, dll_name) \
	{ .next_p = &(struct_name).dll_name, .prev_p = &(struct_name).dll_name }

#define dll_init(node_p)        ((node_p)->next_p = (node_p)->prev_p = (node_p))

/* dll macros returing a "dll_t *" */
#define dll_head_p(list_p)      ((list_p)->next_p)
#define dll_tail_p(list_p)      ((list_p)->prev_p)

#define dll_next_p(node_p)      ((node_p)->next_p)
#define dll_prev_p(node_p)      ((node_p)->prev_p)

#define dll_empty(list_p)       ((list_p)->next_p == (list_p))
#define dll_end(list_p, node_p) ((list_p) == (node_p))

/* inserts the node new_p "after" the node at_p */
#define dll_insert(new_p, at_p) \
({ \
	(new_p)->next_p = (at_p)->next_p; \
	(new_p)->prev_p = (at_p); \
	(at_p)->next_p = (new_p); \
	(new_p)->next_p->prev_p = (new_p); \
})

#define dll_append(list_p, node_p)      dll_insert((node_p), dll_tail_p(list_p))
#define dll_prepend(list_p, node_p)     dll_insert((node_p), (list_p))

/* deletes a node from any list that it "may" be in, if at all. */
#define dll_delete(node_p) \
({ \
	(node_p)->prev_p->next_p = (node_p)->next_p; \
	(node_p)->next_p->prev_p = (node_p)->prev_p; \
})

/**
 * dll_for_each -   iterate over a list
 * @pos:    the &struct list_head to use as a loop cursor.
 * @head:   the head for your list.
 *
 * Iterator "pos" may not be moved.
 *
 * If you need to delete the iterator, then use the below sample.
 *   dll_t * iter_p, * next_p;
 *   for (iter_p = dll_head_p(&someList); ! dll_end(&someList, iter_p);
 *        iter_p = next_p)
 *   {
 *       next_p = dll_next_p(iter_p);
 *       ... use iter_p at will, including removing it from list ...
 *   }
 *
 */
#define dll_for_each(pos, head) \
	for (pos = (head)->next_p; pos != (head); pos = pos->next_p)

/**
 * Take all elements of list A and join them to the tail of list B.
 * List A must not be empty and list A will be returned as an empty list.
 */
#define dll_join(listA_p, listB_p) \
({ \
	dll_t *_listB_p = (listB_p); \
	dll_t *headA_p  = dll_head_p(listA_p); \
	dll_t *tailA_p  = dll_tail_p(listA_p); \
	dll_t *tailB_p  = dll_tail_p(listB_p); \
	/* Link up list B's tail to list A's head */ \
	headA_p->prev_p = tailB_p; \
	tailB_p->next_p = headA_p; \
	/* Make list A's tail to be list B's new tail */ \
	tailA_p->next_p = (listB_p); \
	_listB_p->prev_p = tailA_p; \
	dll_init(listA_p); \
})

#endif  /* ! defined(_dll_t_) */
