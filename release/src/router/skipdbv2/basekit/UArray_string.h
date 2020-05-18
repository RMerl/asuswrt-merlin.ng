/*
	copyright: Steve Dekorte, 2006. All rights reserved.
	license: See _BSDLicense.txt.
*/

/*
	copyright: Steve Dekorte, 2006. All rights reserved.
	license: See _BSDLicense.txt.
*/

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>

BASEKIT_API void UArray_append_(UArray *self, const UArray *other);
BASEKIT_API void UArray_appendCString_(UArray *self, const char *s);
BASEKIT_API void UArray_prepend_(UArray *self, const UArray *other);

BASEKIT_API int UArray_equalsAnyCase_(const UArray *self, const UArray *other);
BASEKIT_API void UArray_replace_with_(UArray *self, const UArray *a1, const UArray *a2);
BASEKIT_API void UArray_replaceAnyCase_with_(UArray *self, const UArray *a1, const UArray *a2);
BASEKIT_API void UArray_replaceCString_withCString_(UArray *self, const char *s1, const char *s2);
BASEKIT_API void UArray_remove_(UArray *self, const UArray *a1);
BASEKIT_API void UArray_removeAnyCase_(UArray *self, const UArray *a1);

// clipping

BASEKIT_API int UArray_clipBefore_(UArray *self, const UArray *other);
BASEKIT_API int UArray_clipBeforeEndOf_(UArray *self, const UArray *other);
BASEKIT_API int UArray_clipAfter_(UArray *self, const UArray *other);
BASEKIT_API int UArray_clipAfterStartOf_(UArray *self, const UArray *other);

// strip

BASEKIT_API void UArray_lstrip_(UArray *self, const UArray *other);
BASEKIT_API void UArray_rstrip_(UArray *self, const UArray *other);
BASEKIT_API void UArray_strip_(UArray *self, const UArray *other);

// swap

BASEKIT_API void UArray_swapIndex_withIndex_(UArray *self, size_t i, size_t j);

// reverse

BASEKIT_API void UArray_reverse(UArray *self);

//BASEKIT_API size_t UArray_matchingPrefixSizeWith_(const UArray *self, const UArray *other);

// split

BASEKIT_API PtrUArray *UArray_split_(const UArray *self, const PtrUArray *delims);
BASEKIT_API size_t UArray_splitCount_(const UArray *self, const PtrUArray *delims);

// find

BASEKIT_API int UArray_beginsWith_(UArray *self, const UArray *other);
BASEKIT_API int UArray_endsWith_(UArray *self, const UArray *other);

// escape and quote

BASEKIT_API void UArray_swapWith_(UArray *self, UArray *other);

BASEKIT_API void UArray_escape(UArray *self);
BASEKIT_API void UArray_unescape(UArray *self);

BASEKIT_API void UArray_quote(UArray *self);
BASEKIT_API void UArray_unquote(UArray *self);

BASEKIT_API void UArray_translate(UArray *self, UArray *fromChars, UArray *toChars);
BASEKIT_API size_t UArray_count_(const UArray *self, const UArray *other);

