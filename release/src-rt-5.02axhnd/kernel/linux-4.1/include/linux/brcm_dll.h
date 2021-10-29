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

typedef struct dll_t {
    struct dll_t * next_p;
    struct dll_t * prev_p;
} Dll_t, * PDll_t;

#define dll_init(node_p)        ((node_p)->next_p = (node_p)->prev_p = (node_p))

/* dll macros returing a PDll_t */
#define dll_head_p(list_p)      ((list_p)->next_p)
#define dll_tail_p(list_p)      ((list_p)->prev_p)

#define dll_next_p(node_p)      ((node_p)->next_p)
#define dll_prev_p(node_p)      ((node_p)->prev_p)

#define dll_empty(list_p)       ((list_p)->next_p == (list_p))
#define dll_end(list_p, node_p) ((list_p) == (node_p))

/* inserts the node new_p "after" the node at_p */
#define dll_insert(new_p, at_p) ((new_p)->next_p = (at_p)->next_p,      \
                                 (new_p)->prev_p = (at_p),              \
                                 (at_p)->next_p = (new_p),              \
                                 (new_p)->next_p->prev_p = (new_p))

#define dll_append(list_p, node_p)      dll_insert((node_p), dll_tail_p(list_p))
#define dll_prepend(list_p, node_p)     dll_insert((node_p), (list_p))

/* deletes a node from any list that it "may" be in, if at all. */
#define dll_delete(node_p)      ((node_p)->prev_p->next_p = (node_p)->next_p, \
                                 (node_p)->next_p->prev_p = (node_p)->prev_p)
/**
 * dll_for_each -   iterate over a list
 * @pos:    the &struct list_head to use as a loop cursor.
 * @head:   the head for your list.
 */
#define dll_for_each(pos, head) \
    for (pos = (head)->next_p; pos != (head); pos = pos->next_p)

#endif  /* ! defined(_dll_t_) */
