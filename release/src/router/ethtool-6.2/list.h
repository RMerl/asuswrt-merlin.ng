#ifndef ETHTOOL_LIST_H__
#define ETHTOOL_LIST_H__

#include <unistd.h>

/* Generic list utilities */

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

static inline void list_add(struct list_head *new, struct list_head *head)
{
	head->next->prev = new;
	new->next = head->next;
	new->prev = head;
	head->next = new;
}

static inline void list_del(struct list_head *entry)
{
	entry->next->prev = entry->prev;
	entry->prev->next = entry->next;
	entry->next = NULL;
	entry->prev = NULL;
}

#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

#endif
