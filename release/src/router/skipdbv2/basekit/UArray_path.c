/*
	copyright: Steve Dekorte, 2006. All rights reserved.
	license: See _BSDLicense.txt.
*/

#include "UArray.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>

void UArray_appendPath_(UArray *self, const UArray *path)
{
	const UArray sep = UArray_stackAllocedWithCString_(IO_PATH_SEPARATOR);

	int selfEndsWithSep   = IS_PATH_SEPERATOR(UArray_lastLong(self));
	int pathStartsWithSep = IS_PATH_SEPERATOR(UArray_firstLong(path));

	if (!selfEndsWithSep && !pathStartsWithSep)
	{
		if(self->size != 0) UArray_append_(self, &sep);
		UArray_append_(self, path);
	}
	else if (selfEndsWithSep && pathStartsWithSep)
	{
		const UArray pathPart = UArray_stackRange(path, 1, path->size - 1);
		UArray_append_(self, &pathPart);
	}
	else
	{
		UArray_append_(self, path);
	}
}

void UArray_removeLastPathComponent(UArray *self)
{
	UArray seps = UArray_stackAllocedWithCString_(IO_PATH_SEPARATORS);
	long pos = UArray_findLastPathComponent(self);
	if (pos) pos --;
	UArray_setSize_(self, pos);
}

void UArray_clipBeforeLastPathComponent(UArray *self)
{
	long pos = UArray_findLastPathComponent(self);

	if (pos != -1)
	{
		UArray_removeRange(self, 0, pos);
	}
}

long UArray_findLastPathComponent(const UArray *self)
{
	if (self->size)
	{
		UArray seps = UArray_stackAllocedWithCString_(IO_PATH_SEPARATORS);
		UArray s = UArray_stackRange(self, 0, self->size);
		long pos = 0;

		while (s.size && (pos = UArray_rFindAnyValue_(&s, &seps)) == s.size - 1)
		{
			s.size = pos;
		}

		if (pos == -1) { pos = 0; } else { pos ++; }
		return pos;
	}

	return 0;
}

UArray *UArray_lastPathComponent(const UArray *self)
{
	long pos = UArray_findLastPathComponent(self);
	return UArray_range(self, pos, self->size - pos);
}

long UArray_findPathExtension(UArray *self)
{
	UArray dot = UArray_stackAllocedWithCString_(IO_PATH_SEPARATOR_DOT);
	return UArray_rFind_(self, &dot);
}

void UArray_removePathExtension(UArray *self)
{
	long pos = UArray_findPathExtension(self);

	if (pos != -1)
	{
		UArray_setSize_(self, pos);
	}
}

UArray *UArray_pathExtension(UArray *self)
{
	long pos = UArray_findPathExtension(self);

	if (pos == -1 || pos == self->size - 1)
	{
		return UArray_newWithCString_copy_("", 1);
	}

	return UArray_range(self, pos + 1, self->size - pos - 1);
}

UArray *UArray_fileName(UArray *self)
{
	long extPos = UArray_findLastPathComponent(self);
	long dotPos = UArray_findPathExtension(self);

	//if (extPos == -1) { extPos = 0; } else { extPos ++; }
	if (dotPos == -1) dotPos = self->size;

	return UArray_range(self, extPos, dotPos - extPos);
}

// to/from os path - always returns a copy

int UArray_OSPathSeparatorIsUnixSeparator(void)
{
	return strcmp(OS_PATH_SEPARATOR, "/") == 0;
}

UArray *UArray_asOSPath(UArray *self)
{
	UArray *a = UArray_clone(self);
	UArray_replaceCString_withCString_(a, IO_PATH_SEPARATOR, OS_PATH_SEPARATOR);
	return a;
}

UArray *UArray_asUnixPath(UArray *self)
{
	UArray *a = UArray_clone(self);
	UArray_replaceCString_withCString_(a, OS_PATH_SEPARATOR, IO_PATH_SEPARATOR);
	return a;
}

