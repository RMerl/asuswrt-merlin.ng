/*
copyright: Steve Dekorte, 2006. All rights reserved.
license: See _BSDLicense.txt.
*/

#include "Base.h"

#define UArray_C
#include "UArray.h"
#undef UArray_C

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>

size_t CTYPE_size(CTYPE type)
{
	switch (type)
	{
		case CTYPE_uint8_t:   return sizeof(uint8_t);
		case CTYPE_uint16_t:  return sizeof(uint16_t);
		case CTYPE_uint32_t:  return sizeof(uint32_t);
		case CTYPE_uint64_t:  return sizeof(uint64_t);

		case CTYPE_int8_t:    return sizeof(int8_t);
		case CTYPE_int16_t:   return sizeof(int16_t);
		case CTYPE_int32_t:   return sizeof(int32_t);
		case CTYPE_int64_t:   return sizeof(int64_t);

		case CTYPE_float32_t: return sizeof(float32_t);
		case CTYPE_float64_t: return sizeof(float64_t);

		case CTYPE_uintptr_t: return sizeof(uintptr_t);
	}
	return 0;
}

const char *CTYPE_name(CTYPE type)
{
	switch (type)
	{
		case CTYPE_uint8_t:   return "uint8";
		case CTYPE_uint16_t:  return "uint16";
		case CTYPE_uint32_t:  return "uint32";
		case CTYPE_uint64_t:  return "uint64";

		case CTYPE_int8_t:    return "int8";
		case CTYPE_int16_t:   return "int16";
		case CTYPE_int32_t:   return "int32";
		case CTYPE_int64_t:   return "int64";

		case CTYPE_float32_t: return "float32";
		case CTYPE_float64_t: return "float64";

		case CTYPE_uintptr_t: return "intptr";
	}
	return "unknown";
}

int CTYPE_forName(const char *name)
{
	if(!strcmp(name, "uint8"))   return CTYPE_uint8_t;
	if(!strcmp(name, "uint16"))  return CTYPE_uint16_t;
	if(!strcmp(name, "uint32"))  return CTYPE_uint32_t;
	if(!strcmp(name, "uint64"))  return CTYPE_uint64_t;
	if(!strcmp(name, "int8"))    return CTYPE_int8_t;
	if(!strcmp(name, "int16"))   return CTYPE_int16_t;
	if(!strcmp(name, "int32"))   return CTYPE_int32_t;
	if(!strcmp(name, "int64"))   return CTYPE_int64_t;
	if(!strcmp(name, "float32")) return CTYPE_float32_t;
	if(!strcmp(name, "float64")) return CTYPE_float64_t;
	return -1;
}

int CENCODING_forName(const char *name)
{
	if(!strcmp(name, "ascii"))  return CENCODING_ASCII;
	if(!strcmp(name, "utf8"))   return CENCODING_UTF8;
	if(!strcmp(name, "utf16"))  return CENCODING_UTF16;
	if(!strcmp(name, "utf32"))  return CENCODING_UTF32;
	if(!strcmp(name, "number")) return CENCODING_NUMBER;
	return -1;
}

const char *CENCODING_name(CENCODING encoding)
{
	switch (encoding)
	{
		case CENCODING_ASCII:  return "ascii";
		case CENCODING_UTF8:   return "utf8";
		case CENCODING_UTF16:  return "utf16";
		case CENCODING_UTF32:  return "utf32";
		case CENCODING_NUMBER: return "number";
	}
	return "unknown";
}

// error

void UArray_unsupported_with_(const UArray *self, const char *methodName, const UArray *other)
{
	//UArray_error_(self, "Error: '%s' not supported between '%s' and '%s'\n");
	printf("Error: '%s' not supported between '%s' and '%s'\n",
		   methodName, CTYPE_name(self->itemType), CTYPE_name(other->itemType));
	exit(-1);
}

void UArray_error_(const UArray *self, char *e)
{
	printf("%s\n", e);
	exit(-1);
}

// new

CTYPE UArray_itemType(const UArray *self)
{
	return self->itemType;
}

size_t UArray_itemSize(const UArray *self)
{
	return self->itemSize;
}

//inline 
size_t UArray_sizeRequiredToContain_(const UArray *self, const UArray *other)
{
	return (UArray_sizeInBytes(other)  + self->itemSize - 1) / self->itemSize;
}

void UArray_rawSetItemType_(UArray *self, CTYPE type)
{
	size_t itemSize = CTYPE_size(type);
	self->itemType = type;
	self->itemSize = itemSize;
}

void UArray_setItemType_(UArray *self, CTYPE type)
{
	size_t itemSize = CTYPE_size(type);
	div_t q = div(UArray_sizeInBytes(self), itemSize);

	if (q.rem != 0)
	{
		q.quot += 1;
		UArray_setSize_(self, (q.quot * itemSize) / self->itemSize);
	}

	self->itemType = type;

	self->itemSize = itemSize;
	self->size = q.quot;

	// ensure encoding is sane for type

	if (UArray_isFloatType(self))
	{
		self->encoding = CENCODING_NUMBER;
	}
	else if (self->encoding == CENCODING_ASCII)
	{
		switch(self->itemSize)
		{
			case 2: self->encoding = CENCODING_UTF16; break;
			case 4: self->encoding = CENCODING_UTF32; break;
			case 8: self->encoding = CENCODING_NUMBER; break;
		}
	}
}

CENCODING UArray_encoding(const UArray *self)
{
	return self->encoding;
}

void UArray_setEncoding_(UArray *self, CENCODING encoding)
{
	// ensure that size matches new encoding

	switch(encoding)
	{
		case CENCODING_ASCII:
		case CENCODING_UTF8:
			UArray_setItemType_(self, CTYPE_uint8_t);
			break;
		case CENCODING_UTF16:
			UArray_setItemType_(self, CTYPE_uint16_t);
			break;
		case CENCODING_UTF32:
			UArray_setItemType_(self, CTYPE_uint32_t);
			break;
		case CENCODING_NUMBER:
			// Don't change itemType when setting raw encoding. Raw encoding
			// used for vectors and numbers and the item type may have been set
			// before this call.
			break;
	}

	self->encoding = encoding;
}

void UArray_convertToEncoding_(UArray *self, CENCODING encoding)
{
	switch(encoding)
	{
		case CENCODING_ASCII:
		case CENCODING_UTF8:
			UArray_convertToUTF8(self);
			break;
		case CENCODING_UTF16:
			UArray_convertToUTF16(self);
			break;
		case CENCODING_UTF32:
			UArray_convertToUTF32(self);
			break;
		case CENCODING_NUMBER:
			UArray_setItemType_(self, CTYPE_uint8_t);
			break;
	}

	self->encoding = encoding;
	UArray_changed(self);
}

UArray *UArray_newWithData_type_encoding_size_copy_(void *bytes, CTYPE type, CENCODING encoding, size_t size, int copy)
{
	UArray *self = (UArray *)io_calloc(1, sizeof(UArray));
	UArray_setData_type_size_copy_(self, bytes, type, size, copy);
	self->encoding = encoding;
	return self;
}

UArray *UArray_newWithData_type_size_copy_(void *bytes, CTYPE type, size_t size, int copy)
{
	UArray *self = (UArray *)io_calloc(1, sizeof(UArray));
	UArray_setData_type_size_copy_(self, bytes, type, size, copy);
	self->encoding = CENCODING_ASCII;
	return self;
}

UArray *UArray_new(void)
{
	return UArray_newWithData_type_size_copy_("", CTYPE_uint8_t, 0, 1);
}

UArray *UArray_clone(const UArray *self)
{
	UArray *out = UArray_new();
	UArray_copy_(out, self);
	return out;
}

void UArray_show(const UArray *self)
{
	printf("UArray_%p %s\t", (void *)self, CTYPE_name(self->itemType));
	printf("size: %i ", self->size);
	printf("itemSize: %i ", self->itemSize);
	printf("data: ");
	UArray_print(self);
	printf("\n");
}

void UArray_print(const UArray *self)
{
	if(self->encoding == CENCODING_ASCII || self->encoding == CENCODING_UTF8)
	{
		printf("%s", (char *)self->data);
	}
	else if(self->encoding != CENCODING_NUMBER)
	{
		UARRAY_FOREACH(self, i, v, printf("%c", (int)v); );
	}
	else if(UArray_isFloatType(self))
	{
		printf("[");
		UARRAY_FOREACH(self, i, v,
					printf("%f", (float)v);
					if(i != self->size - 1) printf(", ");
					);
		printf("]");
	}
	else
	{
		printf("[");
		UARRAY_FOREACH(self, i, v,
					printf("%i", (int)v);
					if(i != self->size - 1) printf(", ");
					);
		printf("]");
	}
}

UArray UArray_stackAllocedWithData_type_size_(void *data, CTYPE type, size_t size)
{
	UArray self;
	memset(&self, 0, sizeof(UArray));

#ifdef UARRAY_DEBUG
	self.stackAllocated = 1;
#endif

	self.itemType = type;
	self.itemSize = CTYPE_size(type);
	self.size = size;
	self.data = data;
	return self;
}

BASEKIT_API UArray UArray_stackAllocedEmptyUArray(void)
{
	UArray self;
	memset(&self, 0, sizeof(UArray));

#ifdef UARRAY_DEBUG
	self.stackAllocated = 1;
#endif

	self.itemType = CTYPE_int32_t;
	self.itemSize = 4;
	self.size = 0;
	self.data = 0x0;
	return self;
}

UArray *UArray_newWithCString_copy_(char *s, int copy)
{
	return UArray_newWithData_type_size_copy_(s, CTYPE_uint8_t, strlen(s), copy);
}

UArray *UArray_newWithCString_(const char *s)
{
	return UArray_newWithData_type_size_copy_((uint8_t *)s, CTYPE_uint8_t, strlen(s), 1);
}

void UArray_empty(UArray *self)
{
	UArray_setSize_(self, 0);
}

void UArray_setCString_(UArray *self, const char *s)
{
	UArray_empty(self);
	UArray_setItemType_(self, CTYPE_uint8_t);
	UArray_appendCString_(self, s);
}

#ifdef UARRAY_DEBUG
void UArray_checkIfOkToRelloc(UArray *self)
{
	if(self->stackAllocated)
	{
		printf("UArray debug error: attempt to io_realloc UArray data that this UArray does not own");
		exit(-1);
	}
}
#endif

void UArray_setData_type_size_copy_(UArray *self, void *data, CTYPE type, size_t size, int copy)
{
	size_t sizeInBytes;

	UArray_rawSetItemType_(self, type);
	self->size = size;

	sizeInBytes = self->size * self->itemSize;

	if (copy)
	{	
		#ifdef UARRAY_DEBUG
		UArray_checkIfOkToRelloc(self);
		#endif

		self->data = io_realloc(self->data, sizeInBytes + 1);
		memmove(self->data, data, sizeInBytes);
		self->data[sizeInBytes] = 0x0;
	}
	else
	{
		if(self->data) free(self->data);
		self->data = data;
	}

}

UArray UArray_stackAllocedWithCString_(char *s)
{
	return UArray_stackAllocedWithData_type_size_(s, CTYPE_uint8_t, strlen(s));
}

const void *UArray_data(const UArray *self)
{
	return self->data;
}

const uint8_t *UArray_bytes(const UArray *self)
{
	return self->data;
}

uint8_t *UArray_mutableBytes(UArray *self)
{
	UArray_changed(self);
	return self->data;
}

const char *UArray_asCString(const UArray *self)
{
	return (const char *)(self->data);
}

void UArray_stackFree(UArray *self)
{
	if(self->data) io_free(self->data);
}

void UArray_free(UArray *self)
{
	if(self->data) io_free(self->data);
	io_free(self);
}

// size

void UArray_setSize_(UArray *self, size_t size)
{
	if (size != self->size)
	{
		size_t oldSizeInBytes = UArray_sizeInBytes(self);
		size_t newSizeInBytes = self->itemSize * size;

#ifdef UARRAY_DEBUG
			UArray_checkIfOkToRelloc(self);
#endif
		self->data = io_realloc(self->data, newSizeInBytes + 1);


		self->data[newSizeInBytes] = 0x0;
		self->size = size;

		if (newSizeInBytes > oldSizeInBytes)
		{
			memset(self->data + oldSizeInBytes, 0, newSizeInBytes - oldSizeInBytes);
		}

		UArray_changed(self);
	}
}

size_t UArray_size(const UArray *self)
{
	return self->size;
}

size_t UArray_sizeInBytes(const UArray *self)
{
	return self->size * self->itemSize;
}

void UArray_sizeTo_(UArray *self, size_t size)
{
	UArray_setSize_(self, size);
}


// copy

void UArray_copy_(UArray *self, const UArray *other)
{
	UArray_setItemType_(self, UArray_itemType(other));
	UArray_setEncoding_(self, UArray_encoding(other));
	UArray_setSize_(self, UArray_size(other));
	UArray_copyItems_(self, other);
}

void UArray_copyItems_(UArray *self, const UArray *other)
{
	if(self->size != other->size)
	{
		printf("UArray_copyItems_ error - arrays not of same size\n");
		exit(-1);
	}

	if(self->itemType == other->itemType)
	{
		UArray_copyData_(self, other);
	}
	else
	{
		DUARRAY_OP(UARRAY_BASICOP_TYPES, =, self, other);
	}
	UArray_changed(self);
}

void UArray_copyData_(UArray *self, const UArray *other)
{
	UArray_setSize_(self, UArray_sizeRequiredToContain_(self, other));
	memmove(self->data, other->data, UArray_sizeInBytes(other));
}

void UArray_convertToItemType_(UArray *self, CTYPE newItemType)
{
	if (self->itemType != newItemType)
	{
		UArray *tmp = UArray_new();
		UArray_setItemType_(tmp, newItemType);
		UArray_setEncoding_(tmp, UArray_encoding(self));
		UArray_setSize_(tmp, self->size);
		UArray_copyItems_(tmp, self);
		UArray_copy_(self, tmp);
		UArray_free(tmp);
		UArray_changed(self);
	}
}

// slice

UArray UArray_stackRange(const UArray *self, size_t start, size_t size)
{
	UArray s;

	memcpy(&s, self, sizeof(UArray));
	s.hash = 0;

#ifdef UARRAY_DEBUG
	s.stackAllocated = 1;
#endif

	if(start < self->size || start == 0)
	{
		s.data = self->data + self->itemSize * start;
	}
	else
	{
		s.data = 0x0;
	}

	if(start + size <= self->size)
	{
		s.size = size;
	}
	else
	{
		s.size = 0;
	}

	return s;
}

UArray *UArray_range(const UArray *self, size_t start, size_t size)
{
	UArray out = UArray_stackRange(self, start, size);
	return UArray_clone(&out);
}

UArray UArray_stackSlice(const UArray *self, long start, long end)
{
	start = UArray_wrapPos_(self, start);
	end   = UArray_wrapPos_(self, end);
	if (end < start) end = start;
	return UArray_stackRange(self, start, end - start);
}

BASEKIT_API UArray *UArray_slice(const UArray *self, long start, long end)
{
	UArray out = UArray_stackSlice(self, start, end);
	return UArray_clone(&out);
}

// at, without bounds check

void *UArray_rawPointerAt_(const UArray *self, size_t i)
{
	if (self->itemType == CTYPE_uintptr_t)
	{
		return ((void **)self->data)[i];
	}

	UArray_error_(self, "UArray_rawPointerAt_ not supported on this type");
	return NULL;
}

long UArray_rawLongAt_(const UArray *self, size_t i)
{
	UARRAY_RAWAT_(self, i);
	UArray_error_(self, "UArray_rawLongAt_ not supported on this type");
	return 0;
}

double UArray_rawDoubleAt_(const UArray *self, size_t i)
{
	UARRAY_RAWAT_(self, i);
	UArray_error_(self, "UArray_doubleAt_ not supported on this type");
	return 0;
}

// at, with bounds check

void *UArray_pointerAt_(const UArray *self, size_t i)
{
	if (i >= self->size) { return NULL; }
	return UArray_rawPointerAt_(self, i);
}

long UArray_longAt_(const UArray *self, size_t i)
{
	if (i >= self->size) { return 0; }
	return UArray_rawLongAt_(self, i);
}

double UArray_doubleAt_(const UArray *self, size_t i)
{
	if (i >= self->size) { return 0.0; }
	return UArray_rawDoubleAt_(self, i);
}

// at, extras

long UArray_firstLong(const UArray *self)
{
	return UArray_rawLongAt_(self, 0);
}

long UArray_lastLong(const UArray *self)
{
	if (!self->size)
	{
		return 0;
	}

	return UArray_rawLongAt_(self, self->size - 1);
}

// remove

void UArray_removeRange(UArray *self, size_t start, size_t removeSize)
{
	if (start < self->size)
	{
		if (start + removeSize > self->size)
		{
			removeSize = self->size - start;
		}
		else if (start + removeSize < self->size)
		{
			// need to copy end
			size_t remainder = start + removeSize;
			size_t remainderSize = self->size - remainder;
			memmove(UARRAY_BYTESAT_(self, start), UARRAY_BYTESAT_(self, remainder), self -> itemSize * remainderSize);
		}

		UArray_setSize_(self, self->size - removeSize);
	}
	UArray_changed(self);
}

void UArray_leave_thenRemove_(UArray *self, size_t itemsToLeave, size_t itemsToRemove)
{
	if (itemsToLeave <= 0)
	{
		UArray_clear(self);
		UArray_setSize_(self, 0);
		return;
	}
	
	if (itemsToRemove <= 0)
	{
		return;
	}
	
	{
		size_t tailChunkSizeInBytes;
		
		size_t period = itemsToLeave + itemsToRemove;
		size_t tailItemCount = UArray_size(self) % period;
		size_t itemSize = self->itemSize;
		size_t chunkSizeInBytes = itemSize * itemsToLeave;
		
		if (tailItemCount == 0)
		{
			tailChunkSizeInBytes = 0;
		}
		else if (tailItemCount <= itemsToLeave)
		{
			tailChunkSizeInBytes = tailItemCount * itemSize;
		}
		else
		{
			tailChunkSizeInBytes = chunkSizeInBytes;
		}
		
		{
			size_t chunkCount = UArray_size(self) / period;
			size_t newItemCount = chunkCount * itemsToLeave + tailChunkSizeInBytes / itemSize;
			uint8_t *newData = malloc(newItemCount * itemSize);
			
			{
				size_t chunkPos;
				
				for (chunkPos = 0; chunkPos < chunkCount; chunkPos++)
				{
					memmove(newData + chunkPos * chunkSizeInBytes, UARRAY_BYTESAT_(self, chunkPos * period), chunkSizeInBytes);
				}

				if (tailChunkSizeInBytes)
				{
					memmove(newData + chunkPos * chunkSizeInBytes, UARRAY_BYTESAT_(self, chunkPos * period), tailChunkSizeInBytes);
				}
				
				UArray_setData_type_size_copy_(self, newData, UArray_itemType(self), newItemCount, 0);
				UArray_changed(self);
			}
		}
	}
}

BASEKIT_API void UArray_removeFirst(UArray *self)
{
	UArray_removeRange(self, 0, 1);
}

BASEKIT_API void UArray_removeLast(UArray *self)
{
	if (self->size > 0)
	{
		UArray_setSize_(self, self->size - 1);
	}
}

// insert

#define UARRAY_RAWAT_PUT_(self, pos, v) \
switch (self->itemType)\
{\
	case CTYPE_uint8_t:   ((uint8_t   *)self->data)[pos] = v; return;\
	case CTYPE_uint16_t:  ((uint16_t  *)self->data)[pos] = v; return;\
	case CTYPE_uint32_t:  ((uint32_t  *)self->data)[pos] = v; return;\
	case CTYPE_uint64_t:  ((uint64_t  *)self->data)[pos] = v; return;\
	case CTYPE_int8_t:    ((int8_t    *)self->data)[pos] = v; return;\
	case CTYPE_int16_t:   ((int16_t   *)self->data)[pos] = v; return;\
	case CTYPE_int32_t:   ((int32_t   *)self->data)[pos] = v; return;\
	case CTYPE_int64_t:   ((int64_t   *)self->data)[pos] = v; return;\
	case CTYPE_float32_t: ((float32_t *)self->data)[pos] = v; return;\
	case CTYPE_float64_t: ((float64_t *)self->data)[pos] = v; return;\
	case CTYPE_uintptr_t: ((uintptr_t *)self->data)[pos] = v; return;\
}

void UArray_at_putLong_(UArray *self, size_t pos, long v)
{
	if(pos >= self->size) UArray_setSize_(self, pos + 1);

	//if(UArray_longAt_(self, pos) != v)
	{
		UARRAY_RAWAT_PUT_(self, pos, v);
		UArray_changed(self);
	}
}

void UArray_at_putDouble_(UArray *self, size_t pos, double v)
{
	if(pos >= self->size) UArray_setSize_(self, pos + 1);

	//if(UArray_doubleAt_(self, pos) != v)
	{
		UARRAY_RAWAT_PUT_(self, pos, v);
		UArray_changed(self);
	}
}

void UArray_at_putPointer_(UArray *self, size_t pos, void *v)
{
	if (pos >= self->size) UArray_setSize_(self, pos + 1);

	switch (self->itemType)
	{
		case CTYPE_uintptr_t:
			if(((void **)self->data)[pos] != v)
			{
				((void **)self->data)[pos] = v;
				UArray_changed(self);
			}
			return;
	}

	UArray_error_(self, "UArray_at_putPointer_ not supported with this type");
}

void UArray_appendLong_(UArray *self, long v)
{
	UArray_at_putLong_(self, self->size, v);
}

void UArray_appendDouble_(UArray *self, double v)
{
	UArray_at_putDouble_(self, self->size, v);
}

void UArray_appendPointer_(UArray *self, void *v)
{
	UArray_at_putPointer_(self, self->size, v);
}

void UArray_appendBytes_size_(UArray *self, const uint8_t *bytes, size_t size)
{
	UArray a = UArray_stackAllocedWithData_type_size_((uint8_t *)bytes, CTYPE_uint8_t, size);
	UArray_append_(self, &a);
}

/*
void UArray_appendByte_(UArray *self, uint8_t byte)
{
	UArray a = UArray_stackAllocedWithData_type_size_(&byte, CTYPE_uint8_t, 1);
	UArray_append_(self, &a);	
}
*/

void UArray_insert_every_(UArray *self, UArray *other, size_t itemCount)
{
	UArray *out = UArray_new();
	UArray *convertedOther = other;
	
	if (itemCount == 0)
	{
		UArray_error_(self, "UArray_insert_every_: itemCount must be > 0");
		return;
	}
	
	if(UArray_itemType(self) != UArray_itemType(other))
	{
		UArray *convertedOther = UArray_clone(other);
		UArray_convertToItemType_(convertedOther, UArray_itemType(self));
	}
		
	{
		size_t selfSizeInBytes  = UArray_sizeInBytes(self);
		size_t otherSize = UArray_size(convertedOther);
		size_t chunkSize  = itemCount * UArray_itemSize(self);
		size_t i;
		
		for(i = 0; i < selfSizeInBytes; i += chunkSize)
		{
			if (i + chunkSize > selfSizeInBytes) 
			{ 
				UArray_appendBytes_size_(out, self->data + i, selfSizeInBytes - i);
			}
			else
			{
				UArray_appendBytes_size_(out, self->data + i, chunkSize);
				UArray_appendBytes_size_(out, convertedOther->data, otherSize);
			}
		}
	}
	
	if(UArray_itemType(self) != UArray_itemType(other))
	{
		UArray_free(convertedOther);
	}
	
	UArray_copy_(self, out);
	UArray_free(out);
}

void UArray_at_putAll_(UArray *self, size_t pos, const UArray *other)
{
	if (other->size == 0) return;

	if (pos > self->size)
	{
		UArray_setSize_(self, pos);
	}

	{
		size_t chunkSize = self->size - pos;
		size_t originalSelfSize = self->size;

		UArray_setSize_(self, self->size + other->size);

		{
			UArray oldChunk = UArray_stackRange(self, pos, chunkSize);
			UArray newChunk = UArray_stackRange(self, pos + other->size, chunkSize);
			UArray insertChunk = UArray_stackRange(self, pos, other->size);

			if (
				//(&newChunk)->data == 0x0 ||
				(&insertChunk)->data == 0x0)
			{
				printf("oldChunk.data     %p size %i\n", (void *)(&oldChunk)->data, oldChunk.size);
				printf("newChunk.data     %p size %i\n", (void *)(&newChunk)->data, newChunk.size);
				printf("insertChunk.data  %p size %i\n", (void *)(&insertChunk)->data, insertChunk.size);
				printf("originalSelfSize = %i\n", originalSelfSize);
				printf("self->size  = %i\n", self->size);
				printf("other->size = %i\n", other->size);
				printf("pos = %i\n", pos);
				//exit(-1);

				oldChunk = UArray_stackRange(self, pos, chunkSize);
				newChunk = UArray_stackRange(self, pos + other->size, chunkSize);
				insertChunk = UArray_stackRange(self, pos, other->size);
				return;
			}

			if (newChunk.size) //UArray_copy_(&newChunk, &oldChunk); // copy chunk to end
			UArray_copyItems_(&newChunk, &oldChunk);
			//UArray_copy_(&insertChunk, other); // insert other
			UArray_copyItems_(&insertChunk, other);
		}

		UArray_changed(self);
	}
}

// compare

#define UARRAY_COMPARE_TYPES(OP2, TYPE1, self, TYPE2, other)\
{\
	size_t i, minSize = self->size < other->size ? self->size : other->size;\
	for(i = 0; i < minSize; i ++)\
	{\
		TYPE1 v1 = ((TYPE1 *)self->data)[i];\
		TYPE2 v2 = ((TYPE2 *)other->data)[i];\
		if (v1 > v2) return 1;\
		if (v1 < v2) return -1;\
	}\
	if(self->size != other->size)\
	{\
		return self->size < other->size ? -1 : 1;\
	}\
	return 0;\
}

#define UARRAY_EQ_TYPES(OP2, TYPE1, self, TYPE2, other)\
{\
	size_t i, minSize = self->size < other->size ? self->size : other->size;\
	for(i = 0; i < minSize; i ++)\
	{\
		TYPE1 v1 = ((TYPE1 *)self->data)[i];\
		TYPE2 v2 = ((TYPE2 *)other->data)[i];\
		if (v1 != v2) return 0;\
	}\
	return 1;\
}

#define UARRAY_GT_TYPES(OP2, TYPE1, self, TYPE2, other)\
{\
	size_t i, minSize = self->size < other->size ? self->size : other->size;\
	for(i = 0; i < minSize; i ++)\
	{\
		TYPE1 v1 = ((TYPE1 *)self->data)[i];\
		TYPE2 v2 = ((TYPE2 *)other->data)[i];\
		if (v1 < v2) return 0;\
	}\
	return 1;\
}

#define UARRAY_LT_TYPES(OP2, TYPE1, self, TYPE2, other)\
{\
	size_t i, minSize = self->size < other->size ? self->size : other->size;\
	for(i = 0; i < minSize; i ++)\
	{\
		TYPE1 v1 = ((TYPE1 *)self->data)[i];\
		TYPE2 v2 = ((TYPE2 *)other->data)[i];\
		if (v1 > v2) return 0;\
	}\
	return 1;\
}

int UArray_compare_(const UArray *self, const UArray *other)
{
	DUARRAY_OP(UARRAY_COMPARE_TYPES, NULL, self, other);
	return 0;
}

int UArray_equals_(const UArray *self, const UArray *other)
{
	if (self->size != other->size) return 0;
	DUARRAY_OP(UARRAY_EQ_TYPES, NULL, self, other);
	return 0;
}

int UArray_greaterThan_(const UArray *self, const UArray *other)
{
	if(self->encoding == CENCODING_NUMBER)
	{ DUARRAY_OP(UARRAY_GT_TYPES, NULL, self, other); }

	return UArray_compare_(self, other) > 0;
}

int UArray_lessThan_(const UArray *self, const UArray *other)
{
	if(self->encoding == CENCODING_NUMBER)
	{ DUARRAY_OP(UARRAY_LT_TYPES, NULL, self, other); }

	return UArray_compare_(self, other) < 0;
}

int UArray_greaterThanOrEqualTo_(const UArray *self, const UArray *other)
{
	if(self->encoding == CENCODING_NUMBER)
	{
		if (UArray_greaterThan_(self, other) | UArray_equals_(self, other))
		{ return 1; } else { return 0; }
	}

	return UArray_compare_(self, other) >= 0;
}

int UArray_lessThanOrEqualTo_(const UArray *self, const UArray *other)
{
	if(self->encoding == CENCODING_NUMBER)
	{
		if (UArray_lessThan_(self, other) | UArray_equals_(self, other))
		{ return 1; } else { return 0; }
	}

	return UArray_compare_(self, other) <= 0;
}

int UArray_isZero(const UArray *self)
{
	UARRAY_FOREACH(self, i, v, if (v) return 0;)
	return 1;
}

// find

// printf("i %i %c j %i %c\n", i, v1, j, v2);\
// printf("j%i == %i\n", i, other->size);\

#define UARRAY_FIND_TYPES(OP2, TYPE1, self, TYPE2, other)\
{\
	size_t i;\
			if(self->size < other->size || self->size == 0) return -1;\
			for(i = 0; i < self->size - other->size + 1; i ++)\
			{\
				size_t j;\
					int match = 1;\
					for(j = 0; j < other->size; j ++)\
					{\
						TYPE1 v1 = ((TYPE1 *)self->data)[i + j];\
							TYPE2 v2 = ((TYPE2 *)other->data)[j];\
								if (v1 != v2) { match = 0; break; }\
					}\
					if (match) return i;\
			}\
			return -1;\
}

long UArray_find_(const UArray *self, const UArray *other)
{
	DUARRAY_OP(UARRAY_FIND_TYPES, NULL, self, other);
	return -1;
}

int UArray_contains_(const UArray *self, const UArray *other)
{
	return UArray_find_(self, other) != -1;
}

long UArray_find_from_(const UArray *self, const UArray *other, size_t from)
{
	UArray s = UArray_stackRange(self, from, self->size - from);
	long i = UArray_find_(&s, other);

	return i == -1 ? -1 : from + i;
}

#define UARRAY_FINDANYCASE_TYPES(OP2, TYPE1, self, TYPE2, other)\
{\
	size_t i;\
		if(self->size < other->size) return -1;\
			for(i = 0; i < self->size - other->size + 1; i ++)\
			{\
				size_t j;\
				int match = 1;\
					for(j = 0; j < other->size; j ++)\
					{\
						TYPE1 v1 = ((TYPE1 *)self->data)[i + j];\
						TYPE2 v2 = ((TYPE2 *)other->data)[j];\
						if (tolower((int)v1) != tolower((int)v2)) { match = 0; break; }\
					}\
					if(match) { return i; }\
			}\
			return -1;\
}

long UArray_findAnyCase_(const UArray *self, const UArray *other)
{
	DUARRAY_OP(UARRAY_FINDANYCASE_TYPES, NULL, self, other);
	return -1;
}

int UArray_containsAnyCase_(const UArray *self, const UArray *other)
{
	long i = UArray_findAnyCase_(self, other);
	return i != -1;
}

long UArray_findLongValue_(const UArray *self, long value)
{
	UARRAY_FOREACH(self, i, v, if(v == value) return i);
	return -1;
}

int UArray_containsLong_(const UArray *self, long value)
{
	return UArray_findLongValue_(self, value) != -1;
}

long UArray_findDoubleValue_(const UArray *self, double value)
{
	UARRAY_FOREACH(self, i, v, if(v == value) return i);
	return -1;
}

int UArray_containsDouble_(const UArray *self, double value)
{
	return UArray_findDoubleValue_(self, value) != -1;
}

#define UARRAY_RFIND_TYPES(OP2, TYPE1, self, TYPE2, other)\
{\
	long i, j;\
		if(self->size < other->size) return -1;\
			for(i = self->size - other->size + 1; i > -1; i --)\
			{\
				int match = 1;\
				for(j = 0; j < other->size; j ++)\
				{\
					TYPE1 v1 = ((TYPE1 *)self->data)[i+j];\
						TYPE2 v2 = ((TYPE2 *)other->data)[j];\
							if (v1 != v2) { match = 0; break; }\
				}\
				if(match) { return i;}\
			}\
			return -1;\
}

long UArray_rFind_(const UArray *self, const UArray *other)
{
	DUARRAY_OP(UARRAY_RFIND_TYPES, NULL, self, other);
	return -1;
}

BASEKIT_API long UArray_rFind_from_(const UArray *self, const UArray *other, size_t from)
{
	UArray s = UArray_stackRange(self, 0, from);
	long i = UArray_rFind_(&s, other);
	return i;
}

#define UARRAY_RFINDANYCASE_TYPES(OP2, TYPE1, self, TYPE2, other)\
{\
	long i, j;\
		if(self->size < other->size) return -1;\
			for(i = self->size - other->size + 1; i > -1; i --)\
			{\
				int match = 1;\
				for(j = 0; j < other->size; j ++)\
				{\
					int v1 = ((TYPE1 *)self->data)[i+j];\
						int v2 = ((TYPE2 *)other->data)[j];\
							if (tolower(v1) != tolower(v2)) { match = 0; break; }\
				}\
				if(match) return i;\
			}\
			return -1;\
}

long UArray_rFindAnyCase_(const UArray *self, const UArray *other)
{
	DUARRAY_OP(UARRAY_RFINDANYCASE_TYPES, NULL, self, other);
	return -1;
}

#define UARRAY_RFINDANYVALUE_TYPES(OP2, TYPE1, self, TYPE2, other)\
{\
	long i, j;\
		if(self->size < other->size) return -1;\
			for(i = self->size - 1; i > -1; i --)\
			{\
				TYPE1 v1 = ((TYPE1 *)self->data)[i];\
				for(j = 0; j < other->size; j ++)\
				{\
					TYPE2 v2 = ((TYPE2 *)other->data)[j];\
					if (v1 == v2) { return i; }\
				}\
			}\
			return -1;\
}

long UArray_rFindAnyValue_(const UArray *self, const UArray *other)
{
	DUARRAY_OP(UARRAY_RFINDANYVALUE_TYPES, NULL, self, other);
	return -1;
}

// types

int UArray_isFloatType(const UArray *self)
{
	return self->itemType == CTYPE_float32_t || self->itemType == CTYPE_float64_t;
}

int UArray_isSignedType(const UArray *self)
{
	switch (self->itemType)
	{
		case CTYPE_uint8_t:   return 0;
		case CTYPE_uint16_t:  return 0;
		case CTYPE_uint32_t:  return 0;
		case CTYPE_uint64_t:  return 0;
		case CTYPE_int8_t:    return 1;
		case CTYPE_int16_t:   return 1;
		case CTYPE_int32_t:   return 1;
		case CTYPE_int64_t:   return 1;
		case CTYPE_float32_t: return 1;
		case CTYPE_float64_t: return 1;
	}
	return 0;
}

size_t UArray_wrapPos_(const UArray *self, long pos)
{
	long size = self->size;

	if (pos > size - 1)
	{
		return size;
	}

	if (pos < 0)
	{
		pos = size + pos;

		if (pos < 0)
		{
			pos = 0;
		}
	}

	return pos;
}

int cmp_uint8_t (const uint8_t  *a, const uint8_t *b)     { return *a == *b ? 0 : (*a < *b ? -1 : 1); }
int cmp_uint16_t(const uint16_t *a, const uint16_t *b)    { return *a == *b ? 0 : (*a < *b ? -1 : 1); }
int cmp_uint32_t(const uint32_t *a, const uint32_t *b)    { return *a == *b ? 0 : (*a < *b ? -1 : 1); }
int cmp_uint64_t(const uint64_t *a, const uint64_t *b)    { return *a == *b ? 0 : (*a < *b ? -1 : 1); }

int cmp_int8_t (const int8_t  *a, const int8_t  *b)       { return *a == *b ? 0 : (*a < *b ? -1 : 1); }
int cmp_int16_t(const int16_t *a, const int16_t *b)       { return *a == *b ? 0 : (*a < *b ? -1 : 1); }
int cmp_int32_t(const int32_t *a, const int32_t *b)       { return *a == *b ? 0 : (*a < *b ? -1 : 1); }
int cmp_int64_t(const int64_t *a, const int64_t *b)       { return *a == *b ? 0 : (*a < *b ? -1 : 1); }

int cmp_float32_t(const float32_t *a, const float32_t *b) { return *a == *b ? 0 : (*a < *b ? -1 : 1); }
int cmp_float64_t(const float64_t *a, const float64_t *b) { return *a == *b ? 0 : (*a < *b ? -1 : 1); }
int cmp_uintptr_t(const uintptr_t *a, const uintptr_t *b) { return *a == *b ? 0 : (*a < *b ? -1 : 1); }

void UArray_sort(UArray *self)
{
	void *base = self->data;
	size_t size = self->size;

	UArray_changed(self);

	switch(self->itemType)
	{
		case CTYPE_uint8_t:   qsort(base, size,  sizeof(uint8_t),  (UArraySortCallback *)cmp_uint8_t);   return;
		case CTYPE_uint16_t:  qsort(base, size, sizeof(uint16_t),  (UArraySortCallback *)cmp_uint16_t);  return;
		case CTYPE_uint32_t:  qsort(base, size, sizeof(uint32_t),  (UArraySortCallback *)cmp_uint32_t);  return;
		case CTYPE_uint64_t:  qsort(base, size, sizeof(uint64_t),  (UArraySortCallback *)cmp_uint64_t);  return;

		case CTYPE_int8_t:    qsort(base, size,  sizeof(int8_t),   (UArraySortCallback *)cmp_int8_t);    return;
		case CTYPE_int16_t:   qsort(base, size, sizeof(int16_t),   (UArraySortCallback *)cmp_int16_t);   return;
		case CTYPE_int32_t:   qsort(base, size, sizeof(int32_t),   (UArraySortCallback *)cmp_int32_t);   return;
		case CTYPE_int64_t:   qsort(base, size, sizeof(int64_t),   (UArraySortCallback *)cmp_int64_t);   return;

		case CTYPE_float32_t: qsort(base, size, sizeof(float32_t), (UArraySortCallback *)cmp_float32_t); return;
		case CTYPE_float64_t: qsort(base, size, sizeof(float64_t), (UArraySortCallback *)cmp_float64_t); return;
		case CTYPE_uintptr_t: qsort(base, size, sizeof(uintptr_t), (UArraySortCallback *)cmp_uintptr_t); return;
	}
}

void UArray_sortBy_(UArray *self, UArraySortCallback *cmp)
{
	void *base = self->data;
	size_t size = self->size;

	UArray_changed(self);

	switch(self->itemType)
	{
		case CTYPE_uint8_t:   qsort(base, size,  sizeof(uint8_t),  cmp); return;
		case CTYPE_uint16_t:  qsort(base, size, sizeof(uint16_t),  cmp); return;
		case CTYPE_uint32_t:  qsort(base, size, sizeof(uint32_t),  cmp); return;
		case CTYPE_uint64_t:  qsort(base, size, sizeof(uint64_t),  cmp); return;

		case CTYPE_int8_t:    qsort(base, size,  sizeof(int8_t),   cmp); return;
		case CTYPE_int16_t:   qsort(base, size, sizeof(int16_t),   cmp); return;
		case CTYPE_int32_t:   qsort(base, size, sizeof(int32_t),   cmp); return;
		case CTYPE_int64_t:   qsort(base, size, sizeof(int64_t),   cmp); return;

		case CTYPE_float32_t: qsort(base, size, sizeof(float32_t), cmp); return;
		case CTYPE_float64_t: qsort(base, size, sizeof(float64_t), cmp); return;
		case CTYPE_uintptr_t: qsort(base, size, sizeof(uintptr_t), cmp); return;
	}
}


