// SPDX-License-Identifier: MIT
/*
 * This file is copyright 2001 Simon Tatham.
 * Rewritten from original source 2006 by Dan Merillat for use in u-boot.
 *
 * Original code can be found at:
 * http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html
 */

#include <common.h>
#include "jffs2_private.h"

int sort_list(struct b_list *list)
{
	struct b_node *p, *q, *e, **tail;
	int k, psize, qsize;

	if (!list->listHead)
		return 0;

	for (k = 1; k < list->listCount; k *= 2) {
		tail = &list->listHead;
		for (p = q = list->listHead; p; p = q) {
			/* step 'k' places from p; */
			for (psize = 0; q && psize < k; psize++)
				q = q->next;
			qsize = k;

			/* two lists, merge them. */
			while (psize || (qsize && q)) {
				/* merge the next element */
				if (psize == 0 ||
				    ((qsize && q) &&
				     list->listCompare(p, q))) {
					/* p is empty, or p > q, so q next */
					e = q;
					q = q->next;
					qsize--;
				} else {
					e = p;
					p = p->next;
					psize--;
				}
				e->next = NULL; /* break accidental loops. */
				*tail = e;
				tail = &e->next;
			}
		}
	}
	return 0;
}
