//metadoc List copyright Steve Dekorte 2002
//metadoc List license BSD revised

#define LIST_C
#include "List.h"
#undef LIST_C
#include <stdlib.h>

List *List_new(void)
{
	List *self = (List *)io_calloc(1, sizeof(List));
	self->size = 0;
	self->memSize = sizeof(void *)*LIST_START_SIZE;
	self->items = (void **)io_calloc(1, self->memSize);
	return self;
}

List *List_clone(const List *self)
{
	List *child = List_new();
	List_copy_(child, self);
	/*
	 List *child = cpalloc(self, sizeof(List));
	 child->items = cpalloc(self->items, self->memSize);
	 */
	return child;
}

static size_t indexWrap(long index, size_t size)
{
	if (index < 0)
	{
		index = size - index;

		if (index < 0)
		{
			index = 0;
		}
	}

	if (index > (int)size)
	{
		index = size;
	}

	return (size_t)index;
}

void List_sliceInPlace(List* self, long startIndex, long endIndex)
{
	size_t i, size = List_size(self);
	List *tmp = List_new();
	size_t start = indexWrap(startIndex, size);
	size_t end   = indexWrap(endIndex, size);

	for (i = start; i < size && i < end + 1; i ++)
	{
		List_append_(tmp, List_at_(self, i));
	}

	List_copy_(self, tmp);
	List_free(tmp);
}

List *List_cloneSlice(const List *self, long startIndex, long endIndex)
{
	List *child = List_clone(self);
	List_sliceInPlace(child, startIndex, endIndex);
	return child;
}

void List_free(List *self)
{
	//printf("List_free(%p)\n", (void *)self);
	io_free(self->items);
	io_free(self);
}

UArray List_asStackAllocatedUArray(List *self)
{
	UArray a = UArray_stackAllocedEmptyUArray();
	a.itemType = CTYPE_uintptr_t;
	a.itemSize = sizeof(uintptr_t);
	a.size = self->size;
	a.data = (uint8_t *)(self->items);
	return a;
}

size_t List_memorySize(const List *self)
{
	return sizeof(List) + (self->size * sizeof(void *));
}

void List_removeAll(List *self)
{
	self->size = 0;
	List_compactIfNeeded(self);
}

void List_copy_(List *self, const List *otherList)
{
	if (self == otherList || (!otherList->size && !self->size))
	{
		return;
	}

	List_preallocateToSize_(self, otherList->size);
	memmove(self->items, otherList->items, sizeof(void *) * (otherList->size));
	self->size = otherList->size;
}

int List_equals_(const List *self, const List *otherList)
{
	return (self->size == otherList->size &&
		   memcmp(self->items, otherList->items, sizeof(void *) * self->size) == 0);
}

/* --- sizing ------------------------------------------------ */

void List_setSize_(List *self, size_t index)
{
	List_ifNeededSizeTo_(self, index);
	self->size = index;
}

void List_preallocateToSize_(List *self, size_t index)
{
	size_t s = index * sizeof(void *);

	if (s >= self->memSize)
	{
		size_t newSize = self->memSize * LIST_RESIZE_FACTOR;

		if (s > newSize)
		{
			newSize = s;
		}

		self->items = (void **)io_realloc(self->items, newSize);
		memset(self->items + self->size, 0, (newSize - (self->size*sizeof(void *))));
		self->memSize = newSize;
	}
}

void List_compact(List *self)
{
	self->memSize = self->size * sizeof(void *);
	self->items = (void **)io_realloc(self->items, self->memSize);
}

// -----------------------------------------------------------

void List_print(const List *self)
{
	size_t i;

	printf("List <%p> [%i bytes]\n", (void *)self, (int)self->memSize);

	for (i = 0; i < self->size; i ++)
	{
		printf("%i: %p\n", i, (void *)self->items[i]);
	}

	printf("\n");
}

// enumeration -----------------------------------------

void List_do_(List *self, ListDoCallback *callback)
{
	LIST_FOREACH(self, i, v, if (v) (*callback)(v));
}

void List_do_with_(List *self, ListDoWithCallback *callback, void *arg)
{
	LIST_FOREACH(self, i, v, if (v) (*callback)(v, arg));
}

void List_mapInPlace_(List *self, ListCollectCallback *callback)
{
	void **items = self->items;
	LIST_FOREACH(self, i, v, items[i] = (*callback)(v));
}

List *List_map_(List *self, ListCollectCallback *callback)
{
	List *r = List_new();
	LIST_FOREACH(self, i, v, List_append_(r, (*callback)(v)););
	return r;
}

List *List_select_(List *self, ListSelectCallback *callback)
{
	List *r = List_new();
	LIST_FOREACH(self, i, v, if ((*callback)(v)) List_append_(r, v));
	return r;
}

void *List_detect_(List *self, ListDetectCallback *callback)
{
	LIST_FOREACH(self, i, v, if (v && (*callback)(v)) return v; );
	return NULL;
}

void *List_anyOne(const List *self)
{
	size_t i;

	if (self->size == 0)
	{
		return (void *)NULL;
	}

	if (self->size == 1)
	{
		return LIST_AT_(self, 0);
	}

	i = (rand() >> 4) % (self->size); // without the shift, just get a sequence!

	return LIST_AT_(self, i);
}

void List_shuffle(List *self)
{
	size_t i, j;

	for (i = 0; i < self->size - 1; i ++)
	{
		j = i + rand() % (self->size - i);
		List_swap_with_(self, i, j);
	}
}

void *List_removeLast(List *self)
{
	void *item = List_at_(self, self->size - 1);

	if (item)
	{
		self->size --;
		List_compactIfNeeded(self);
	}

	return item;
}

