/*
	copyright: Steve Dekorte, 2006. All rights reserved.
	license: See _BSDLicense.txt.
*/

#include "UArray.h"
#include <math.h>

// set

#define UARRAY_IOP(OP) \
void UArray_ ## OP (UArray *self) { UARRAY_FOREACHASSIGN(self, i, v, OP((int)v)); }

UARRAY_IOP(isalnum);
UARRAY_IOP(isalpha);
UARRAY_IOP(iscntrl);
UARRAY_IOP(isdigit);
UARRAY_IOP(isgraph);
UARRAY_IOP(islower);
UARRAY_IOP(isprint);
UARRAY_IOP(ispunct);
UARRAY_IOP(isspace);
UARRAY_IOP(isupper);
UARRAY_IOP(isxdigit);

UARRAY_IOP(tolower);
UARRAY_IOP(toupper);

BASEKIT_API int UArray_isLowercase(UArray *self)
{
	UARRAY_INTFOREACH(self, i, v, if(v != tolower(v)) return 0);
	return 1;
}

BASEKIT_API int UArray_isUppercase(UArray *self)
{
	UARRAY_INTFOREACH(self, i, v, if(v != toupper(v)) return 0);
	return 1;
}
