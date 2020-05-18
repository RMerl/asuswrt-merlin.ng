/*
	copyright: Steve Dekorte, 2006. All rights reserved.
	license: See _BSDLicense.txt.
*/

// set

BASEKIT_API void UArray_clear(UArray *self);
BASEKIT_API void UArray_setItemsToLong_(UArray *self, long x);
BASEKIT_API void UArray_setItemsToDouble_(UArray *self, double x);
BASEKIT_API void UArray_rangeFill(UArray *self);
BASEKIT_API void UArray_negate(const UArray *self);

// basic vector math

BASEKIT_API void UArray_add_(UArray *self, const UArray *other);
BASEKIT_API void UArray_subtract_(UArray *self, const UArray *other);
BASEKIT_API void UArray_multiply_(UArray *self, const UArray *other);
BASEKIT_API void UArray_divide_(UArray *self, const UArray *other);
BASEKIT_API double UArray_dotProduct_(const UArray *self, const UArray *other);

// basic scalar math

BASEKIT_API void UArray_addScalarDouble_(UArray *self, double v);
BASEKIT_API void UArray_subtractScalarDouble_(UArray *self, double v);
BASEKIT_API void UArray_multiplyScalarDouble_(UArray *self, double v);
BASEKIT_API void UArray_divideScalarDouble_(UArray *self, double v);

// bitwise logic

BASEKIT_API void UArray_bitwiseOr_(UArray *self, const UArray *other);
BASEKIT_API void UArray_bitwiseAnd_(UArray *self, const UArray *other);
BASEKIT_API void UArray_bitwiseXor_(UArray *self, const UArray *other);
BASEKIT_API void UArray_bitwiseNot(UArray *self);

// bitwise ops

BASEKIT_API void UArray_setAllBitsTo_(UArray *self, uint8_t aBool);
BASEKIT_API uint8_t UArray_byteAt_(UArray *self, size_t i);
BASEKIT_API int UArray_bitAt_(UArray *self, size_t i);
BASEKIT_API void UArray_setBit_at_(UArray *self, int b, size_t i);
BASEKIT_API UArray * UArray_asBits(const UArray *self);
BASEKIT_API size_t UArray_bitCount(UArray *self);

// boolean logic

BASEKIT_API void UArray_logicalOr_(UArray *self, const UArray *other);
BASEKIT_API void UArray_logicalAnd_(UArray *self, const UArray *other);

// trigonometry

BASEKIT_API void UArray_sin(UArray *self);
BASEKIT_API void UArray_cos(UArray *self);
BASEKIT_API void UArray_tan(UArray *self);

BASEKIT_API void UArray_asin(UArray *self);
BASEKIT_API void UArray_acos(UArray *self);
BASEKIT_API void UArray_atan(UArray *self);

//void UArray_atan2(UArray *self, const UArray *other);

BASEKIT_API void UArray_sinh(UArray *self);
BASEKIT_API void UArray_cosh(UArray *self);
BASEKIT_API void UArray_tanh(UArray *self);

BASEKIT_API void UArray_exp(UArray *self);
BASEKIT_API void UArray_log(UArray *self);
BASEKIT_API void UArray_log10(UArray *self);

//void UArray_pow(UArray *self, const UArray *other);

BASEKIT_API void UArray_sqrt(UArray *self);
BASEKIT_API void UArray_ceil(UArray *self);
BASEKIT_API void UArray_floor(UArray *self);
BASEKIT_API void UArray_abs(UArray *self);
BASEKIT_API void UArray_round(UArray *self);

//void UArray_ldexp(UArray *self, const UArray *other);
//void UArray_fmod(UArray *self, const UArray *other);

BASEKIT_API void UArray_square(UArray *self);
BASEKIT_API void UArray_normalize(UArray *self);

BASEKIT_API void UArray_crossProduct_(UArray *self, const UArray *other);
BASEKIT_API double UArray_distanceTo_(const UArray *self, const UArray *other);

// extras

BASEKIT_API double UArray_sumAsDouble(const UArray *self);
BASEKIT_API double UArray_productAsDouble(const UArray *self);
BASEKIT_API double UArray_arithmeticMeanAsDouble(const UArray *self);
BASEKIT_API double UArray_arithmeticMeanSquareAsDouble(const UArray *self);
BASEKIT_API double UArray_maxAsDouble(const UArray *self);
BASEKIT_API double UArray_minAsDouble(const UArray *self);
BASEKIT_API void UArray_Max(UArray *self, const UArray *other);
BASEKIT_API void UArray_Min(UArray *self, const UArray *other);

// hash

BASEKIT_API void UArray_changed(UArray *self);
BASEKIT_API uintptr_t UArray_calcHash(UArray *self);
BASEKIT_API uintptr_t UArray_hash(UArray *self);
BASEKIT_API int UArray_equalsWithHashCheck_(UArray *self, UArray *other);

// indexes

BASEKIT_API void UArray_duplicateIndexes(UArray *self);
BASEKIT_API void UArray_removeOddIndexes(UArray *self);
BASEKIT_API void UArray_removeEvenIndexes(UArray *self);

BASEKIT_API void UArray_reverseItemByteOrders(UArray *self);

