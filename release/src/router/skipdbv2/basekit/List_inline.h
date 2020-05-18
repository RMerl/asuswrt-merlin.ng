//metadoc List copyright Steve Dekorte 2002
//metadoc List license BSD revised
/*metadoc List description
List is an array of void pointers.
The List is not responsible for io_freeing it's elements.
*/

#ifdef LIST_C
#define IO_IN_C_FILE
#endif
#include "Common_inline.h"
#ifdef IO_DECLARE_INLINES

#define LIST_FOREACH(list, index, value, code) \
{ \
	const List *foreachList = list; \
	size_t index, foreachMax = foreachList->size; \
	\
	for (index = 0; index < foreachMax; index ++) \
	{ \
		void *value = foreachList->items[index]; \
		code; \
	} \
}

#define LIST_SAFEFOREACH(list, index, value, code) \
{ \
	const List *foreachList = list; \
	size_t index; \
	\
	for (index = 0; index < List_size(foreachList); index ++) \
	{ \
		void *value = List_at_(foreachList, index); \
		code; \
	} \
}

#define LIST_REVERSEFOREACH(list, index, value, code) \
{ \
	const List *foreachList = list; \
	size_t index = List_size(foreachList); \
	\
	while (index) \
	{ \
		void *value = List_at_(foreachList, (index --)); \
		code; \
	} \
}

#define LIST_SAFEREVERSEFOREACH(list, index, value, code) \
{ \
	const List *foreachList = list; \
	size_t index = List_size(foreachList); \
	\
	for (index = List_size(foreachList) - 1; index > 0; index --) \
	{ \
		void *value = List_at_(foreachList, index); \
		code; \
		if (index > List_size(foreachList) - 1) { index = List_size(foreachList) - 1; } \
	} \
}

#define LIST_DO_(list, func) \
{ \
	const List *foreachList = list; \
	size_t index, foreachMax = List_size(foreachList); \
	\
	for (index = 0; index < foreachMax; index ++) \
	{ \
		func(List_at_(foreachList, index)); \
	} \
}

#define List_size(self) (self->size)
/*
IOINLINE size_t List_size(const List *self)
{
	return self->size;
}
*/

IOINLINE void List_ifNeededSizeTo_(List *self, size_t newSize)
{
	if (newSize * sizeof(void *) >= self->memSize)
	{
		List_preallocateToSize_(self, newSize);
	}
}

IOINLINE void *List_rawAt_(List *self, size_t index)
{
	return self->items[index];
}


IOINLINE void *List_at_(const List *self, size_t index)
{
	if (index < self->size)
	{
		return self->items[index];
	}

	return (void *)NULL;
}

// --------------------------------------------

IOINLINE long List_indexOf_(List *self, void *item)
{
	LIST_FOREACH(self, i, v, if(v == item) return i);
	return -1;
}

IOINLINE int List_contains_(List *self, void *item)
{
	LIST_FOREACH(self, i, v, if(v == item) return 1);
	return 0;
}

IOINLINE void *List_append_(List *self, void *item)
{
	List_ifNeededSizeTo_(self, self->size + 1);
	self->items[self->size] = item;
	self->size ++;
	return item;
}

IOINLINE void List_appendSeq_(List *self, const List *otherList)
{
	LIST_FOREACH(otherList, i, v, List_append_(self, v));
}

IOINLINE void List_compactIfNeeded(List *self)
{
	if(self->memSize > 1024 && self->size * sizeof(void *) * 4 < self->memSize)
	{
		List_compact(self);
	}
}

IOINLINE void List_removeIndex_(List *self, size_t index)
{
	if (index >= 0 && index < self->size)
	{
		if ( index != self->size - 1)
		{
			memmove(&self->items[index], &self->items[index + 1],
				   (self->size - 1 - index) * sizeof(void *));
		}

		self->size --;

		List_compactIfNeeded(self);
	}
}

IOINLINE void List_removeIndex_toIndex_(List *self, size_t index1, size_t index2)
{
	long length;

	if (index1 < 0)
	{
		index1 = 0;
	}

	if (index1 > self->size - 1)
	{
		index1 = self->size - 1;
	}

	if (index2 < 0)
	{
		index2 = 0;
	}

	if (index2 > self->size - 1)
	{
		index2 = self->size - 1;
	}

	length = index2 - index1;

	if (length <= 0)
	{
		return;
	}

	memmove(&self->items[index1], &self->items[index2],
		   (self->size - index2) * sizeof(void *));

	self->size -= length;

	List_compactIfNeeded(self);
}

IOINLINE void List_remove_(List *self, void *item)
{
	size_t index;

	for (index = 0; index < self->size; index ++)
	{
		if (self->items[index] == item)
		{
			List_removeIndex_(self, index);
		}
	}
}

IOINLINE int List_removeFirst_(List *self, void *item)
{
	int i, max = self->size;

	for (i = 0; i < max; i ++)
	{
		if (self->items[i] == item)
		{
			List_removeIndex_(self, i);
			return 1;
		}
	}

	return 0;
}

IOINLINE void List_removeLast_(List *self, void *item)
{
	int index = self->size - 1;

	for (index = self->size - 1; index > -1; index --)
	{
		if (self->items[index] == item)
		{
			List_removeIndex_(self, index);
			break;
		}
	}
}

IOINLINE void List_removeItems_(List *self, List *other)
{
	LIST_FOREACH(other, i, v, List_remove_(self, v));
}

IOINLINE void List_at_insert_(List *self, size_t index, void *item)
{
	if (index < 0)
	{
		return;
	}

	if (index > self->size - 1)
	{
		List_preallocateToSize_(self, index + 1);
	}
	else
	{
		List_ifNeededSizeTo_(self, self->size + 1);
	}

	memmove(&self->items[index + 1], &self->items[index],
		   (self->size - index) * sizeof(void *));

	self->items[index] = item;
	self->size ++;
}

IOINLINE void List_at_put_(List *self, size_t index, void *item)
{
	if (index < 0)
	{
		return;
	}

	List_ifNeededSizeTo_(self, index);
	self->items[index] = item;

	if (index + 1 > self->size)
	{
		self->size = index + 1;
	}
}

IOINLINE void List_swap_with_(List *self, long index1, long index2)
{
	if (index1 < 0 || index2 < 0)
	{
		return;
	}

	if (index1 != index2)
	{
		void **items = self->items;
		register void *v1 = items[index1];

		items[index1] = items[index2];
		items[index2] = v1;
	}
}

IOINLINE void List_reverse(List *self)
{
	register void **i = self->items;
	register void **j = i + (self->size - 1);
	register void *iv;

	while (j > i)
	{
		iv = *i;
		*i = *j;
		*j = iv;
		j --;
		i ++;
	}
}

// stack --------------------------------------------------

IOINLINE void List_push_(List *self, void *item)
{
	List_ifNeededSizeTo_(self, self->size + 1);
	self->items[self->size] = item;
	self->size ++;
}

IOINLINE void *List_pop(List *self)
{
	void *item;

	if (!self->size)
	{
		return (void *)NULL;
	}

	self->size --;
	item = self->items[self->size];
	List_compactIfNeeded(self);
	return item;
}

IOINLINE void *List_top(const List *self)
{
	if (!self->size)
	{
		return (void *)NULL;
	}

	return self->items[self->size - 1];
}

/* --- perform -------------------------------------------------- */

IOINLINE int List_removeTrueFor_(List *self, ListCollectCallback* callback)
{
	size_t getIndex = 0;
	size_t putIndex = 0;
	size_t count = self->size;
	void **items = self->items;

	while (getIndex < count)
	{
		void *item = items[getIndex];

		if (item && !((*callback)(item)))
		{
			if (getIndex!=putIndex)
			{
				items[putIndex] = item;
			}

			putIndex ++;
		}

		getIndex ++;
	}

	self->size = putIndex;

	return getIndex - putIndex;
}

IOINLINE void List_qsort(List *self, ListSortCallback *callback)
{
	qsort(self->items, self->size, sizeof(void *), *callback);
}

IOINLINE void *List_bsearch(List *self, const void *key, ListSortCallback *callback)
{
	return bsearch(key, self->items, self->size, sizeof(void *), callback);
}

IOINLINE void *List_first(const List *self)
{
	return List_at_(self, 0);
}

IOINLINE void *List_last(List *self)
{
	return List_at_(self, List_size(self) - 1);
}

#undef IO_IN_C_FILE
#endif

