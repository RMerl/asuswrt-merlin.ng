/*
	copyright: Steve Dekorte, 2006. All rights reserved.
	license: See _BSDLicense.txt.
*/

BASEKIT_API int UArray_convertToFixedSizeType(UArray *self);

BASEKIT_API int UArray_maxCharSize(const UArray *self);
BASEKIT_API int UArray_isMultibyte(const UArray *self);
BASEKIT_API int UArray_isLegalUTF8(const UArray *self);

BASEKIT_API UArray *UArray_asUTF8(const UArray *self);
BASEKIT_API UArray *UArray_asUTF16(const UArray *self);
BASEKIT_API UArray *UArray_asUTF32(const UArray *self);

BASEKIT_API void UArray_convertToUTF8(UArray *self);
BASEKIT_API void UArray_convertToUTF16(UArray *self);
BASEKIT_API void UArray_convertToUTF32(UArray *self);
