//metadoc Stack copyright Steve Dekorte 2002
//metadoc Stack license BSD revised
/*metadoc Stack description
 Stack - array of void pointers
 supports setting marks - when a mark is popped,
 all stack items above it are popped as well
 */

#ifdef STACK_C
#define IO_IN_C_FILE
#endif
#include "Common_inline.h"
#ifdef IO_DECLARE_INLINES

IOINLINE void Stack_do_(const Stack *self, StackDoCallback *callback)
{
	void **itemP = self->top;
	intptr_t mark = self->lastMark;

	while (itemP > self->items)
	{
		if (itemP - self->items == mark)
		{
			mark = (intptr_t)(*itemP);
		}
		else
		{
			(*callback)(*itemP);
		}

		itemP --;
	}
}

IOINLINE void Stack_doUntilMark_(const Stack *self, StackDoCallback *callback)
{
	void **itemP = self->top;
	intptr_t mark = self->lastMark;

	while (itemP > self->items)
	{
		if (itemP - self->items == mark)
		{
			return;
		}
		else
		{
			(*callback)(*itemP);
		}

		itemP --;
	}
}

IOINLINE void Stack_clear(Stack *self)
{
	self->top = self->items;
	self->lastMark = 0;
}

IOINLINE size_t Stack_totalSize(const Stack *self)
{
	return (self->top - self->items);
}

IOINLINE int Stack_count(const Stack *self)
{
	return (self->top - self->items);
}

IOINLINE void Stack_push_(Stack *self, void *item)
{
	self->top ++;

	if (self->top == self->memEnd)
	{
		Stack_resize(self);
	}

	*(self->top) = item;
}

IOINLINE void Stack_pushMark(Stack *self)
{
	Stack_push_(self, (void *)self->lastMark);
	self->lastMark = self->top - self->items;
}

IOINLINE intptr_t Stack_pushMarkPoint(Stack *self)
{
	Stack_push_(self, (void *)self->lastMark);
	self->lastMark = self->top - self->items;
	return self->lastMark;
}

IOINLINE void *Stack_pop(Stack *self)
{
	void *top = *(self->top);

	if (self->items != self->top)
	{
#ifdef STACK_POP_CALLBACK
		if(self->popCallback) self->popCallback(*(self->top));
#endif
		self->top --;
	}

	return top;
}

IOINLINE void Stack_popMark(Stack *self)
{
#ifdef STACK_POP_CALLBACK
	if(self->popCallback) Stack_doUntilMark_(self, self->popCallback);
#endif

	self->top = self->items + self->lastMark - 1;

	if (self->lastMark)
	{
		self->lastMark = (intptr_t)(self->items[self->lastMark]);
	}
}

IOINLINE int Stack_popMarkPoint_(Stack *self, intptr_t mark)
{
	while (self->lastMark && self->lastMark != mark)
	{
		Stack_popMark(self);
	}

	if (self->lastMark != mark)
	{
		return 0;
	}

	Stack_popMark(self);
	return 1;
}

IOINLINE void Stack_clearTop(Stack *self)
{
	Stack_popMark(self);
	Stack_pushMark(self);
	//self->top = self->items + self->lastMark;
}

IOINLINE void *Stack_top(const Stack *self)
{
	return *(self->top);
}

IOINLINE void *Stack_at_(const Stack *self, int i)
{
	return self->items[i + 1];
}



#undef IO_IN_C_FILE
#endif

