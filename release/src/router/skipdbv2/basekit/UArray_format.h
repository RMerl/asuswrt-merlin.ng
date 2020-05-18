/*
	copyright: Steve Dekorte, 2006. All rights reserved.
	license: See _BSDLicense.txt.
*/

BASEKIT_API UArray *UArray_newWithFormat_(const char *format, ...);
BASEKIT_API UArray *UArray_newWithVargs_(const char *format, va_list ap);
BASEKIT_API UArray *UArray_fromFormat_(UArray *self, const char *format, ...);
BASEKIT_API void UArray_fromVargs_(UArray *self, const char *format, va_list ap);

BASEKIT_API UArray *UArray_asNewHexStringUArray(UArray *self);
