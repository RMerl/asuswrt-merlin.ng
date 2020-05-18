/*#io
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("Datum")    
*/

#ifndef Datum_DEFINED
#define Datum_DEFINED 1

#include "Common.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define PID_TYPE uint32_t
#define PID_TYPE size_t
#define PID_FORMAT "zi"

typedef struct
{
    PID_TYPE size;
    unsigned char *data;
} Datum;

BASEKIT_API PID_TYPE Datum_size(Datum *self);
BASEKIT_API unsigned char *Datum_data(Datum *self);

// return stack allocated datums 

BASEKIT_API Datum Datum_FromData_length_(unsigned char *data, PID_TYPE size);
BASEKIT_API Datum Datum_FromCString_(const char *s);
//Datum Datum_FromPid_(PID_TYPE pid);
BASEKIT_API Datum Datum_Empty(void);

BASEKIT_API Datum Datum_datumAt_(Datum *self, size_t i);
BASEKIT_API Datum *Datum_newFrom_to_(Datum *self, size_t start, size_t end);

// comparison

BASEKIT_API int Datum_compare_length_(Datum *self, Datum *other, size_t limit);
BASEKIT_API int Datum_compare_(Datum *self, Datum *other);
BASEKIT_API int Datum_compareCString_(Datum *self, const char *s);
BASEKIT_API int Datum_beginsWith_(Datum *self, Datum *other);
BASEKIT_API int Datum_endsWith_(Datum *self, Datum *other);
BASEKIT_API size_t Datum_matchingPrefixSizeWith_(Datum *self, Datum *other);

BASEKIT_API long Datum_find_(Datum *self, void *delimsList, size_t startIndex);
BASEKIT_API void *Datum_split_(Datum *self, void *delims); /* returns a List */

//int Datum_next(Datum *self);

BASEKIT_API unsigned int Datum_hash(Datum *self);

typedef int (DatumDetectWithFunc)(void *, Datum *); /* 1 = match, -1 = break */
BASEKIT_API int Datum_detect_with_(Datum *self, DatumDetectWithFunc *func, void *target);

#include "UArray.h"

BASEKIT_API Datum Datum_FromUArray_(UArray *ba);
BASEKIT_API void *Datum_asUArray(Datum *self);

#ifdef __cplusplus
}
#endif
#endif
