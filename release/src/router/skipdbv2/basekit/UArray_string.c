/*
	copyright: Steve Dekorte, 2006. All rights reserved.
	license: See _BSDLicense.txt.
*/

#include "UArray.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>

void UArray_append_(UArray *self, const UArray *other)
{
	UArray_at_putAll_(self, self->size, other);
}

void UArray_appendCString_(UArray *self, const char *s)
{
	while (*s)
	{
		UArray_appendLong_(self, *s);
		s ++;
	}
}

void UArray_prepend_(UArray *self, const UArray *other)
{
	UArray_at_putAll_(self, 0, other);
}

int UArray_equalsAnyCase_(const UArray *self, const UArray *other)
{
	if (self->size == other->size)
	{
		return UArray_findAnyCase_(self, other) == 0;
	}

	return 0;
}

void UArray_replace_with_(UArray *self, const UArray *a1, const UArray *a2)
{
	long i;
	size_t start = 0;
	UArray visible = UArray_stackRange(self, start, self->size);

	while ((i = UArray_find_(&visible, a1)) != -1)
	{
		size_t index = start + i;
		UArray_removeRange(self, index, a1->size);
		UArray_at_putAll_(self, index, a2);
		start = index + a2->size;
		visible = UArray_stackRange(self, start, self->size - start);
	}
	UArray_changed(self);
}

BASEKIT_API void UArray_replaceCString_withCString_(UArray *self, const char *s1, const char *s2)
{
	UArray a1 = UArray_stackAllocedWithCString_((char *)s1);
	UArray a2 = UArray_stackAllocedWithCString_((char *)s2);
	UArray_replace_with_(self, &a1, &a2);
}

void UArray_replaceAnyCase_with_(UArray *self, const UArray *a1, const UArray *a2)
{
	long i;
	size_t start = 0;
	UArray visible = UArray_stackRange(self, start, self->size);

	while ((i = UArray_findAnyCase_(&visible, a1)) != -1)
	{
		size_t index = start + i;
		UArray_removeRange(self, index, a1->size);
		UArray_at_putAll_(self, index, a2);
		start = index + a2->size;
		visible = UArray_stackRange(self, start, self->size - start);
	}

	UArray_changed(self);
}

BASEKIT_API void UArray_remove_(UArray *self, const UArray *a1)
{
	UArray blank = UArray_stackAllocedEmptyUArray();
	UArray_replace_with_(self, a1, &blank);
}

BASEKIT_API void UArray_removeAnyCase_(UArray *self, const UArray *a1)
{
	UArray blank = UArray_stackAllocedEmptyUArray();
	UArray_replaceAnyCase_with_(self, a1, &blank);
}

// clipping

BASEKIT_API int UArray_clipBefore_(UArray *self, const UArray *other)
{
	long index = UArray_find_(self, other);

	if (index > -1)
	{
		UArray_removeRange(self, 0, index);
		return 1;
	}

	return 0;
}

BASEKIT_API int UArray_clipBeforeEndOf_(UArray *self, const UArray *other)
{
	long index = UArray_find_(self, other);

	if (index > -1)
	{
		UArray_removeRange(self, 0, index + other->size);
		return 1;
	}

	return 0;
}

BASEKIT_API int UArray_clipAfter_(UArray *self, const UArray *other)
{
	long index = UArray_find_(self, other);

	if (index > -1)
	{
		UArray_removeRange(self, index + other->size, self->size);
		return 1;
	}

	return 0;
}

BASEKIT_API int UArray_clipAfterStartOf_(UArray *self, const UArray *other)
{
	long index = UArray_find_(self, other);

	if (index > -1)
	{
		UArray_removeRange(self, index, self->size);
		return 1;
	}

	return 0;
}

// strip

void UArray_lstrip_(UArray *self, const UArray *other)
{
	size_t amount = 0;

	if (UArray_isFloatType(self))
	{
		UARRAY_FOREACH(self, i, v,
			amount = i+1;
			if (!UArray_containsDouble_(other, v))
			{
				amount --;
				break;
			}
		)
	}
	else
	{
		UARRAY_FOREACH(self, i, v,
			amount = i+1;
			if (!UArray_containsLong_(other, v))
			{
				amount --;
				break;
			}
		)
	}

	UArray_removeRange(self, 0, amount);
}

void UArray_rstrip_(UArray *self, const UArray *other)
{
	size_t index = 0; // initial value is only needed when FOREACHes don't work

	if (UArray_isFloatType(self))
	{
		UARRAY_RFOREACH(self, i, v,
			index = i;
			if (!UArray_containsDouble_(other, v)) { index++; break; }
		)
	}
	else
	{
		UARRAY_RFOREACH(self, i, v,
			index = i;
			if (!UArray_containsLong_(other, v)) { index++; break; }
		)
	}

	UArray_removeRange(self, index, self->size);
}

BASEKIT_API void UArray_strip_(UArray *self, const UArray *other)
{
	UArray_lstrip_(self, other);
	UArray_rstrip_(self, other);
}

// swap

BASEKIT_API void UArray_swapIndex_withIndex_(UArray *self, size_t i, size_t j)
{
	int itemSize = self->itemSize;
	uint8_t *data = self->data;
	void *ip = data + i * itemSize;
	void *jp = data + j * itemSize;
	UArrayValueUnion b;

	memcpy(&b, ip, sizeof(UArray));
	memcpy(ip, jp, sizeof(UArray));
	memcpy(jp, &b, sizeof(UArray));
	UArray_changed(self);
}

// reverse

BASEKIT_API void UArray_reverse(UArray *self)
{
	long i = 0;
	long j = self->size - 1;
	UArrayValueUnion b;
	int itemSize = self->itemSize;
	uint8_t *data = self->data;

	while (j > i)
	{
		void *ip = data + i * itemSize;
		void *jp = data + j * itemSize;

		memcpy(&b, ip, itemSize);
		memcpy(ip, jp, itemSize);
		memcpy(jp, &b, itemSize);

		j --;
		i ++;
	}

	UArray_changed(self);
}

/*
#define UARRAY_MATCHPREFIXLENGTH_TYPES(OP2, TYPE1, self, TYPE2, other)\
{\
	long i, minSize = (self->size < other->size) ? self->size : other->size;\
	for(i = 0; i < minSize; i ++)\
	{\
		TYPE1 v1 = ((TYPE1 *)self->data)[i];\
		TYPE2 v2 = ((TYPE2 *)other->data)[i];\
		if (v1 != v2) break;\
	}\
	return i;\
}

BASEKIT_API size_t UArray_matchingPrefixSizeWith_(const UArray *self, const UArray *other)
{
	DUARRAY_OP(UARRAY_MATCHPREFIXLENGTH_TYPES, NULL, self, other);
}
*/

// split

PtrUArray *UArray_split_(const UArray *self, const PtrUArray *delims)
{
	PtrUArray *results = UArray_new();
	size_t i, last = 0;
	UArray_setItemType_(results, CTYPE_uintptr_t);

	for (i = 0; i < self->size; i ++)
	{
		UArray slice = UArray_stackRange(self, i, self->size - i);
		size_t j;

		for (j = 0; j < delims->size; j ++)
		{
			UArray *delim = UArray_rawPointerAt_(delims, j);

			if (UArray_beginsWith_(&slice, delim))
			{
				UArray_appendPointer_(results, UArray_range(self, last, i - last));

				last = i + delim->size;
				i = last - 1; // since for() will increment it
				break;
			}
		}
	}

	if (last != self->size)
	{
		UArray_appendPointer_(results, UArray_range(self, last, self->size - last));
	}

	return results;
}

size_t UArray_splitCount_(const UArray *self, const PtrUArray *delims)
{
	PtrUArray *r = UArray_split_(self, delims);
	size_t count = UArray_size(r);
	UArray_free(r);
	return count;
}

// find

BASEKIT_API int UArray_beginsWith_(UArray *self, const UArray *other)
{
	if (self->size >= other->size)
	{
		UArray tmp = UArray_stackRange(self, 0, other->size);
		return UArray_find_(&tmp, other) != -1;
	}

	return 0;
}

BASEKIT_API int UArray_endsWith_(UArray *self, const UArray *other)
{
	if (self->size >= other->size)
	{
		UArray tmp = UArray_stackRange(self, self->size - other->size, other->size);
		return UArray_find_(&tmp, other) != -1;
	}

	return 0;
}

// escape and quote

void UArray_swapWith_(UArray *self, UArray *other)
{
	UArray b;
	memcpy(&b, other, sizeof(UArray));
	memcpy(other, self, sizeof(UArray));
	memcpy(self, &b, sizeof(UArray));
	UArray_changed(self);
}

void UArray_escape(UArray *self)
{
	UArray *out = UArray_new();
	out->itemType = self->itemType;

	UARRAY_FOREACH(self, i, v,
		switch ((int)v)
		{
			case '"':  UArray_appendCString_(out, "\\\""); break;
			case '\a': UArray_appendCString_(out, "\\a"); break;
			case '\b': UArray_appendCString_(out, "\\b"); break;
			case '\f': UArray_appendCString_(out, "\\f"); break;
			case '\n': UArray_appendCString_(out, "\\n"); break;
			case '\r': UArray_appendCString_(out, "\\r"); break;
			case '\t': UArray_appendCString_(out, "\\t"); break;
			case '\v': UArray_appendCString_(out, "\\v"); break;
			case '\\': UArray_appendCString_(out, "\\\\"); break;
			default:   UArray_appendLong_(out, v);
		}
	);

	UArray_swapWith_(self, out);
	UArray_free(out);
	UArray_changed(self);
}

void UArray_unescape(UArray *self)
{
	size_t getIndex = 0;
	size_t putIndex = 0;

	while (getIndex < self->size)
	{
		long c = UArray_longAt_(self, getIndex);
		long nextChar = UArray_longAt_(self, getIndex + 1);

		if (c != '\\')
		{
			if (getIndex != putIndex)
			{
				UArray_at_putLong_(self, putIndex, c);
			}

			putIndex ++;
		}
		else
		{
			c = nextChar;

			switch (c)
			{
				case  'a': c = '\a'; break;
				case  'b': c = '\b'; break;
				case  'f': c = '\f'; break;
				case  'n': c = '\n'; break;
				case  'r': c = '\r'; break;
				case  't': c = '\t'; break;
				case  'v': c = '\v'; break;
				case '\0': c = '\\'; break;
				default:
					if (isdigit(c))
					{
						c -= 48;
					}
			}

			UArray_at_putLong_(self, putIndex, c);
			getIndex ++;
			putIndex ++;
		}

		getIndex ++;
	}

	UArray_setSize_(self, putIndex);
	UArray_changed(self);
}

void UArray_quote(UArray *self)
{
	UArray q = UArray_stackAllocedWithCString_("\"");
	UArray_prepend_(self, &q);
	UArray_append_(self, &q);
	UArray_changed(self);
}

void UArray_unquote(UArray *self)
{
	UArray q = UArray_stackAllocedWithCString_("\"");

	if(UArray_beginsWith_(self, &q) && UArray_endsWith_(self, &q))
	{
		UArray_removeFirst(self);
		UArray_removeLast(self);
		UArray_changed(self);
	}
}

void UArray_translate(UArray *self, UArray *fromChars, UArray *toChars)
{
	size_t max = 4096;
	long fromMax = UArray_maxAsDouble(fromChars);
	long toMax   = UArray_maxAsDouble(toChars);

	if (UArray_size(fromChars) != UArray_size(toChars))
	{
		printf("UArray_translate: translation strings must be of the same length");
		return;
	}

	if ((0 < fromMax && fromMax < max) && (0 < toMax && toMax < 256))
	{
		size_t i;
		uint8_t *map = io_calloc(1, fromMax);
		memset(map, 0x0, fromMax);

		for(i = 0; i < UArray_size(fromChars); i ++)
		{
			map[UArray_longAt_(fromChars, i)] = UArray_longAt_(toChars, i);
		}

		for(i = 0; i < UArray_size(self); i ++)
		{
			self->data[i] = map[self->data[i]];
		}

		io_free(map);
		return;
	}

	/*
	 UARRAY_FOREACH(self, i, currChar,
				 UARRAY_FOREACH(fromChars, j, fromChar,
							 if(currChar == fromChar)
							 {
								 UARRAY_RAWAT_PUT_(self, i, UARRAY_RAWAT_(toChars, j));
								 break;
							 }
							 );
				 );
	 */

	UArray_error_(self, "UArray_translate unimplemented for this type");
}

size_t UArray_count_(const UArray *self, const UArray *other)
{
	long i = 0;
	size_t count = 0;

	while ((i = UArray_find_from_(self, other, i)) != -1)
	{
		i += UArray_size(other);
		count ++;
	}

	return count;
}
