#ifndef _STACK_H_
#define _STACK_H_

#include "linux_list.h"

struct stack {
	struct list_head	list;
	int			items;
};

static inline void stack_init(struct stack *s)
{
	INIT_LIST_HEAD(&s->list);
}

struct stack_item {
	struct list_head	head;
	int			type;
	int			data_len;
	char			data[0];
};

struct stack_item *stack_item_alloc(int type, size_t data_len);
void stack_item_free(struct stack_item *e);
void stack_item_push(struct stack *s, struct stack_item *e);
struct stack_item *stack_item_pop(struct stack *s, int type);

#endif
