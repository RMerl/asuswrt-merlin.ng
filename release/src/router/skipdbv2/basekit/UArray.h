/*
	copyright: Steve Dekorte, 2006. All rights reserved.
	license: See _BSDLicense.txt.
	description: A mutable array of same-sized values.
*/


#ifndef UARRAY_DEFINED
#define UARRAY_DEFINED 1

#include "Common.h"
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(float32_t)
	typedef float  float32_t;
	typedef double float64_t;
#endif

typedef size_t PID_TYPE;

typedef enum
{
	CTYPE_uint8_t,
	CTYPE_uint16_t,
	CTYPE_uint32_t,
	CTYPE_uint64_t,

	CTYPE_int8_t,
	CTYPE_int16_t,
	CTYPE_int32_t,
	CTYPE_int64_t,

	CTYPE_float32_t,
	CTYPE_float64_t,

	CTYPE_uintptr_t
} CTYPE;

typedef enum
{
	CENCODING_ASCII,
	CENCODING_UTF8,
	CENCODING_UTF16,
	CENCODING_UTF32,
	CENCODING_NUMBER
} CENCODING;

typedef struct
{
	uint8_t   uint8;
	uint16_t  uint16;
	uint32_t  uint32;
	uint64_t  uint64;
	int8_t    int8;
	int16_t   int16;
	int32_t   int32;
	int64_t   int64;
	float32_t float32;
	float64_t float64;
	uintptr_t uintptr;
} UArrayValueUnion;

#define UARRAY_DEBUG 1

typedef struct
{
	uint8_t *data;  // memory for items
	size_t size;    // number of items
	CTYPE itemType;
	size_t itemSize;
	uintptr_t hash;
	uint8_t encoding;
	#ifdef UARRAY_DEBUG
	int stackAllocated;
	#endif
} UArray;

typedef  UArray CharUArray;
typedef  UArray PtrUArray;
typedef  UArray FloatUArray;

BASEKIT_API const void *UArray_data(const UArray *self);
BASEKIT_API const uint8_t *UArray_bytes(const UArray *self);
BASEKIT_API uint8_t *UArray_mutableBytes(UArray *self);
BASEKIT_API const char *UArray_asCString(const UArray *self);

BASEKIT_API size_t UArray_SizeOfCtype(CTYPE type);

BASEKIT_API const char *CTYPE_name(CTYPE type);
BASEKIT_API int CTYPE_forName(const char *name);

BASEKIT_API const char *CENCODING_name(CENCODING encoding);
BASEKIT_API int CENCODING_forName(const char *name);

BASEKIT_API void UArray_unsupported_with_(const UArray *self, const char *methodName, const UArray *other);
BASEKIT_API void UArray_error_(const UArray *self, char *e);

BASEKIT_API UArray *UArray_new(void);
BASEKIT_API UArray *UArray_newWithData_type_size_copy_(void *data, CTYPE type, size_t size, int copy);
BASEKIT_API UArray *UArray_newWithData_type_encoding_size_copy_(void *bytes, CTYPE type, CENCODING encoding, size_t size, int copy);
BASEKIT_API UArray *UArray_newWithCString_copy_(char *s, int copy);
BASEKIT_API UArray *UArray_newWithCString_(const char *s);
BASEKIT_API void UArray_setCString_(UArray *self, const char *s);
BASEKIT_API void UArray_setData_type_size_copy_(UArray *self, void *data, CTYPE type, size_t size, int copy);
BASEKIT_API UArray *UArray_clone(const UArray *self);
BASEKIT_API void UArray_show(const UArray *self);
BASEKIT_API void UArray_print(const UArray *self);

BASEKIT_API UArray UArray_stackAllocedWithData_type_size_(void *data, CTYPE type, size_t size);
BASEKIT_API UArray UArray_stackAllocedWithCString_(char *s);
BASEKIT_API UArray UArray_stackAllocedEmptyUArray(void);

BASEKIT_API void UArray_stackFree(UArray *self);
BASEKIT_API void UArray_free(UArray *self);

BASEKIT_API CTYPE UArray_itemType(const UArray *self);
BASEKIT_API size_t UArray_itemSize(const UArray *self);
BASEKIT_API void UArray_setItemType_(UArray *self, CTYPE type);
BASEKIT_API CENCODING UArray_encoding(const UArray *self);
BASEKIT_API void UArray_setEncoding_(UArray *self, CENCODING encoding);
BASEKIT_API void UArray_convertToEncoding_(UArray *self, CENCODING encoding);

// copy

BASEKIT_API void UArray_copyItems_(UArray *self, const UArray *other);
BASEKIT_API void UArray_copy_(UArray *self, const UArray *other);
BASEKIT_API void UArray_copyData_(UArray *self, const UArray *other);
BASEKIT_API void UArray_convertToItemType_(UArray *self, CTYPE newItemType);

// size

#define UArray_minSizeWith_(self, other) self->size < other->size ? self->size : other->size
#define UArray_minSizeInBytesWith_(self, other) self->size * self->itemSize < other->size  * other->itemSize ? self->size * self->itemSize : other->size  * other->itemSize

BASEKIT_API void UArray_setSize_(UArray *self, size_t size);
BASEKIT_API size_t UArray_size(const UArray *self);
BASEKIT_API size_t UArray_sizeInBytes(const UArray *self);

BASEKIT_API void UArray_sizeTo_(UArray *self, size_t size);

// slice

BASEKIT_API UArray UArray_stackRange(const UArray *self, size_t start, size_t size);
BASEKIT_API UArray *UArray_range(const UArray *self, size_t start, size_t size);
BASEKIT_API UArray UArray_stackSlice(const UArray *self, long start, long end);
BASEKIT_API UArray *UArray_slice(const UArray *self, long start, long end);

// compare

BASEKIT_API int UArray_compare_(const UArray *self, const UArray *other);
BASEKIT_API int UArray_equals_(const UArray *self, const UArray *other);
BASEKIT_API int UArray_greaterThan_(const UArray *self, const UArray *other);
BASEKIT_API int UArray_lessThan_(const UArray *self, const UArray *other);
BASEKIT_API int UArray_greaterThanOrEqualTo_(const UArray *self, const UArray *other);
BASEKIT_API int UArray_lessThanOrEqualTo_(const UArray *self, const UArray *other);
BASEKIT_API int UArray_isZero(const UArray *self);

// contains

BASEKIT_API int UArray_contains_(const UArray *self, const UArray *other);
BASEKIT_API int UArray_containsAnyCase_(const UArray *self, const UArray *other);

// find

BASEKIT_API long UArray_find_(const UArray *self, const UArray *other);
BASEKIT_API long UArray_find_from_(const UArray *self, const UArray *other, size_t from);
BASEKIT_API long UArray_rFind_from_(const UArray *self, const UArray *other, size_t from);
BASEKIT_API long UArray_rFind_(const UArray *self, const UArray *other);
BASEKIT_API long UArray_rFindAnyCase_(const UArray *self, const UArray *other);
BASEKIT_API long UArray_rFindAnyValue_(const UArray *self, const UArray *other);

// insert

BASEKIT_API void UArray_at_putLong_(UArray *self, size_t pos, long v);
BASEKIT_API void UArray_at_putDouble_(UArray *self, size_t pos, double v);
BASEKIT_API void UArray_at_putPointer_(UArray *self, size_t pos, void *v);
BASEKIT_API void UArray_at_putAll_(UArray *self, size_t pos, const UArray *other);

BASEKIT_API void UArray_appendLong_(UArray *self, long v);
BASEKIT_API void UArray_appendDouble_(UArray *self, double v);
BASEKIT_API void UArray_appendPointer_(UArray *self, void *v);

BASEKIT_API void UArray_appendBytes_size_(UArray *self, const uint8_t *bytes, size_t size);
//BASEKIT_API void UArray_appendByte_(UArray *self, uint8_t byte);

BASEKIT_API void UArray_insert_every_(UArray *self, UArray *other, size_t itemCount);

// remove

BASEKIT_API void UArray_removeRange(UArray *self, size_t start, size_t size);
BASEKIT_API void UArray_leave_thenRemove_(UArray *self, size_t itemsToLeave, size_t itemsToRemove);
BASEKIT_API void UArray_removeFirst(UArray *self);
BASEKIT_API void UArray_removeLast(UArray *self);

// at

#define UARRAY_RAWAT_(self, i) \
	switch (self->itemType)\
	{\
		case CTYPE_uint8_t:   return ((uint8_t   *)self->data)[i];\
		case CTYPE_uint16_t:  return ((uint16_t  *)self->data)[i];\
		case CTYPE_uint32_t:  return ((uint32_t  *)self->data)[i];\
		case CTYPE_uint64_t:  return ((uint64_t  *)self->data)[i];\
		case CTYPE_int8_t:    return ((int8_t    *)self->data)[i];\
		case CTYPE_int16_t:   return ((int16_t   *)self->data)[i];\
		case CTYPE_int32_t:   return ((int32_t   *)self->data)[i];\
		case CTYPE_int64_t:   return ((int64_t   *)self->data)[i];\
		case CTYPE_float32_t: return ((float32_t *)self->data)[i];\
		case CTYPE_float64_t: return ((float64_t *)self->data)[i];\
		case CTYPE_uintptr_t: return ((uintptr_t *)self->data)[i];\
	}

// at, without bounds check

BASEKIT_API void *UArray_rawPointerAt_(const UArray *self, size_t i);
BASEKIT_API long UArray_rawLongAt_(const UArray *self, size_t i);
BASEKIT_API double UArray_rawDoubleAt_(const UArray *self, size_t i);

// at, with bounds check

BASEKIT_API void *UArray_pointerAt_(const UArray *self, size_t i);
BASEKIT_API long UArray_longAt_(const UArray *self, size_t i);
BASEKIT_API double UArray_doubleAt_(const UArray *self, size_t i);

// at, extras

BASEKIT_API long UArray_lastLong(const UArray *self);
BASEKIT_API long UArray_firstLong(const UArray *self);

// types

BASEKIT_API int UArray_isFloatType(const UArray *self);
BASEKIT_API int UArray_isSignedType(const UArray *self);

BASEKIT_API size_t UArray_wrapPos_(const UArray *self, long pos);

// sort

BASEKIT_API void UArray_sort(UArray *self);

typedef int (UArraySortCallback)(const void *, const void *);

BASEKIT_API void UArray_sortBy_(UArray *self, UArraySortCallback *cmp);

// accessing

#define UARRAY_BYTEPOSAT_(self, n) (self->itemSize * n)
#define UARRAY_BYTESAT_(self, n) (self->data + (self->itemSize * n))

// macros

#define DUARRAY_INTOTHER(MACRO, OP, TYPE1, self, other) \
	switch (other->itemType)\
	{\
		case CTYPE_uint8_t:  MACRO(OP, TYPE1, self, uint8_t,  other); break;\
		case CTYPE_uint16_t: MACRO(OP, TYPE1, self, uint16_t, other); break;\
		case CTYPE_uint32_t: MACRO(OP, TYPE1, self, uint32_t, other); break;\
		case CTYPE_int8_t:   MACRO(OP, TYPE1, self, int8_t,   other); break;\
		case CTYPE_int16_t:  MACRO(OP, TYPE1, self, int16_t,  other); break;\
		case CTYPE_int32_t:  MACRO(OP, TYPE1, self, int32_t,  other); break;\
		case CTYPE_uintptr_t: MACRO(OP, TYPE1, self, uintptr_t, other); break;\
	}

#define DUARRAY_OTHER(MACRO, OP, TYPE1, self, other) \
	switch (other->itemType)\
	{\
		case CTYPE_uint8_t:   MACRO(OP, TYPE1, self, uint8_t,   other); break;\
		case CTYPE_uint16_t:  MACRO(OP, TYPE1, self, uint16_t,  other); break;\
		case CTYPE_uint32_t:  MACRO(OP, TYPE1, self, uint32_t,  other); break;\
		case CTYPE_uint64_t:  MACRO(OP, TYPE1, self, uint64_t,  other); break;\
		case CTYPE_int8_t:    MACRO(OP, TYPE1, self, int8_t,    other); break;\
		case CTYPE_int16_t:   MACRO(OP, TYPE1, self, int16_t,   other); break;\
		case CTYPE_int32_t:   MACRO(OP, TYPE1, self, int32_t,   other); break;\
		case CTYPE_int64_t:   MACRO(OP, TYPE1, self, int64_t,   other); break;\
		case CTYPE_float32_t: MACRO(OP, TYPE1, self, float32_t, other); break;\
		case CTYPE_float64_t: MACRO(OP, TYPE1, self, float64_t, other); break;\
		case CTYPE_uintptr_t: MACRO(OP, TYPE1, self, uintptr_t, other); break;\
	}

#define DUARRAY_INTSELF(MACRO, OP, self, other) \
	switch (self->itemType)\
	{\
		case CTYPE_uint8_t:  DUARRAY_INTOTHER(MACRO, OP, uint8_t,  self, other);\
		case CTYPE_uint16_t: DUARRAY_INTOTHER(MACRO, OP, uint16_t, self, other);\
		case CTYPE_uint32_t: DUARRAY_INTOTHER(MACRO, OP, uint32_t, self, other);\
		case CTYPE_int8_t:   DUARRAY_INTOTHER(MACRO, OP, int8_t,   self, other);\
		case CTYPE_int16_t:  DUARRAY_INTOTHER(MACRO, OP, int16_t,  self, other);\
		case CTYPE_int32_t:  DUARRAY_INTOTHER(MACRO, OP, uint32_t, self, other);\
	}

#define DUARRAY_SELF(MACRO, OP, self, other) \
	switch (self->itemType)\
	{\
		case CTYPE_uint8_t:   DUARRAY_OTHER(MACRO, OP, uint8_t,   self, other);\
		case CTYPE_uint16_t:  DUARRAY_OTHER(MACRO, OP, uint16_t,  self, other);\
		case CTYPE_uint32_t:  DUARRAY_OTHER(MACRO, OP, uint32_t,  self, other);\
		case CTYPE_uint64_t:  DUARRAY_OTHER(MACRO, OP, uint64_t,  self, other);\
		case CTYPE_int8_t:    DUARRAY_OTHER(MACRO, OP, int8_t,    self, other);\
		case CTYPE_int16_t:   DUARRAY_OTHER(MACRO, OP, int16_t,   self, other);\
		case CTYPE_int32_t:   DUARRAY_OTHER(MACRO, OP, uint32_t,  self, other);\
		case CTYPE_int64_t:   DUARRAY_OTHER(MACRO, OP, uint64_t,  self, other);\
		case CTYPE_float32_t: DUARRAY_OTHER(MACRO, OP, float32_t, self, other);\
		case CTYPE_float64_t: DUARRAY_OTHER(MACRO, OP, float64_t, self, other);\
		case CTYPE_uintptr_t: DUARRAY_OTHER(MACRO, OP, uintptr_t, self, other);\
	}

#define DUARRAY_OP(MACRO, OP, self, other)\
	DUARRAY_SELF(MACRO, OP, self, other);\
	UArray_unsupported_with_(self, #OP, other);

#define DUARRAY_INTOP(MACRO, OP, self, other)\
	DUARRAY_INTSELF(MACRO, OP, self, other);\
	UArray_unsupported_with_(self, #OP, other);

// two array primitive ops

#define UARRAY_BASICOP_TYPES(OP2, TYPE1, self, TYPE2, other)\
{\
	size_t i, minSize = self->size < other->size ? self->size : other->size;\
	for(i = 0; i < minSize; i ++)\
	{\
		((TYPE1 *)self->data)[i] OP2 ((TYPE2 *)other->data)[i];\
	}\
	return; \
}

//printf("%i: " #TYPE1 " %f " #OP2 " " #TYPE2 " %i\n", i, ((TYPE1 *)self->data)[i], ((TYPE2 *)other->data)[i]);

// single array ops

// foreach --------------------------

#define UARRAY_FOREACHTYPE(self, i, v, code, TYPE)\
	{\
		size_t i;\
		for(i = 0; i < self->size; i ++)\
		{\
			TYPE v = ((TYPE *)self->data)[i];\
			code;\
		}\
	}

#define UARRAY_FOREACH_CASETYPE_(self, i, v, code, TYPE)\
		case CTYPE_ ## TYPE: UARRAY_FOREACHTYPE(self, i, v, code, TYPE); break;

#define UARRAY_FOREACH(self, i, v, code)\
	switch(self->itemType)\
	{\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, uint8_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, uint16_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, uint32_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, uint64_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, int8_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, int16_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, int32_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, int64_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, float32_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, float64_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, uintptr_t);\
	}

#define UARRAY_INTFOREACH(self, i, v, code)\
	switch(self->itemType)\
	{\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, uint8_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, uint16_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, uint32_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, uint64_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, int8_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, int16_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, int32_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, int64_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, float32_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, float64_t);\
		UARRAY_FOREACH_CASETYPE_(self, i, v, code, uintptr_t);\
	}

// rforeach --------------------------

#define UARRAY_RFOREACHTYPE(self, i, v, code, TYPE)\
	{\
	long i;\
	for(i = self->size - 1; i > -1; i --)\
	{\
		TYPE v = ((TYPE *)self->data)[i];\
		code;\
	}\
	}

#define UARRAY_RFOREACH_CASETYPE_(self, i, v, code, TYPE)\
		case CTYPE_ ## TYPE: UARRAY_RFOREACHTYPE(self, i, v, code, TYPE); break;

#define UARRAY_RFOREACH(self, i, v, code)\
	switch(self->itemType)\
	{\
		UARRAY_RFOREACH_CASETYPE_(self, i, v, code, uint8_t);\
		UARRAY_RFOREACH_CASETYPE_(self, i, v, code, uint16_t);\
		UARRAY_RFOREACH_CASETYPE_(self, i, v, code, uint32_t);\
		UARRAY_RFOREACH_CASETYPE_(self, i, v, code, uint64_t);\
		UARRAY_RFOREACH_CASETYPE_(self, i, v, code, int8_t);\
		UARRAY_RFOREACH_CASETYPE_(self, i, v, code, int16_t);\
		UARRAY_RFOREACH_CASETYPE_(self, i, v, code, int32_t);\
		UARRAY_RFOREACH_CASETYPE_(self, i, v, code, int64_t);\
		UARRAY_RFOREACH_CASETYPE_(self, i, v, code, float32_t);\
		UARRAY_RFOREACH_CASETYPE_(self, i, v, code, float64_t);\
		UARRAY_RFOREACH_CASETYPE_(self, i, v, code, uintptr_t);\
	}

// foreach assign --------------------------

#define UARRAY_FOREACHTYPEASSIGN(self, i, v, code, TYPE)\
	{\
		size_t i;\
		for(i = 0; i < self->size; i ++)\
		{\
			TYPE v = ((TYPE *)self->data)[i];\
			((TYPE *)self->data)[i] = code;\
		}\
	}

#define UARRAY_FOREACHASSIGN(self, i, v, code)\
	switch(self->itemType)\
	{\
		case CTYPE_uint8_t:   UARRAY_FOREACHTYPEASSIGN(self, i, v, code, uint8_t);   break;\
		case CTYPE_uint16_t:  UARRAY_FOREACHTYPEASSIGN(self, i, v, code, uint16_t);  break;\
		case CTYPE_uint32_t:  UARRAY_FOREACHTYPEASSIGN(self, i, v, code, uint32_t);  break;\
		case CTYPE_uint64_t:  UARRAY_FOREACHTYPEASSIGN(self, i, v, code, uint64_t);  break;\
		case CTYPE_int8_t:    UARRAY_FOREACHTYPEASSIGN(self, i, v, code, int8_t);    break;\
		case CTYPE_int16_t:   UARRAY_FOREACHTYPEASSIGN(self, i, v, code, int16_t);   break;\
		case CTYPE_int32_t:   UARRAY_FOREACHTYPEASSIGN(self, i, v, code, int32_t);   break;\
		case CTYPE_int64_t:   UARRAY_FOREACHTYPEASSIGN(self, i, v, code, int64_t);   break;\
		case CTYPE_float32_t: UARRAY_FOREACHTYPEASSIGN(self, i, v, code, float32_t); break;\
		case CTYPE_float64_t: UARRAY_FOREACHTYPEASSIGN(self, i, v, code, float64_t); break;\
	}

// ----------------------------

#include "UArray_character.h"
#include "UArray_format.h"
#include "UArray_math.h"
#include "UArray_path.h"
#include "UArray_stream.h"
#include "UArray_string.h"
#include "UArray_utf.h"

#ifdef __cplusplus
}
#endif
#endif
