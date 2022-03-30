#ifndef _dll_t_
#define _dll_t_
/*
<:copyright-BRCM:2014:DUAL/GPL:standard 

   Copyright (c) 2014 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

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
