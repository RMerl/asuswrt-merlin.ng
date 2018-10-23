#ifndef _STACK_H_
#define _STACK_H_

struct stack;

struct stack *stack_create(size_t elem_size, int max_elems);
void stack_destroy(struct stack *s);
int stack_push(struct stack *s, void *data);
int stack_pop(struct stack *s, void *data);

#endif
