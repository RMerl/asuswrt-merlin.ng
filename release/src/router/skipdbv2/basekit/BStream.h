/*
copyright 
	Steve Dekorte, 2004
license
	BSD revised
description
	A Binary Stream that supports tagged items.
*/

#ifndef BSTREAM_DEFINED
#define BSTREAM_DEFINED 1

#include "Common.h"
#include "UArray.h"
#include "BStreamTag.h"

#ifdef __cplusplus
extern "C" {
#endif

//typedef struct UArray UArray;

typedef struct
{
	UArray *ba;
	size_t index;
	unsigned char ownsUArray;
	UArray *tmp;
	UArray *errorBa;
	int flipEndian;
	unsigned char *typeBuf;
} BStream;

/*
#define BStream_ba_(self, v) self->ba = v;
#define BStream_ba(self) (self->ba)

#define BStream_index_(self, v) self->index = v;
#define BStream_index(self) (self->index)

#define BStream_ownsUArray_(self, v) self->ownsUArray = v;
#define BStream_ownsUArray(self) (self->ownsUArray)

#define BStream_tmp_(self, v) self->tmp = v;
#define BStream_tmp(self) (self->tmp)

#define BStream_errorBa_(self, v) self->errorBa = v;
#define BStream_errorBa(self) (self->errorBa)

#define BStream_flipEndian_(self, v) self->flipEndian = v;
#define BStream_flipEndian(self) (self->flipEndian)

#define BStream_typeBuf_(self, v) self->typeBuf = v;
#define BStream_typeBufs(self) (self->typeBuf)
*/

BASEKIT_API BStream *BStream_new(void);
BASEKIT_API BStream *BStream_clone(BStream *self);
BASEKIT_API void BStream_free(BStream *self);

BASEKIT_API char *BStream_errorString(BStream *self);
BASEKIT_API void BStream_setUArray_(BStream *self, UArray *ba);
BASEKIT_API void BStream_setData_length_(BStream *self, unsigned char *data, size_t length);
BASEKIT_API UArray *BStream_byteArray(BStream *self);
BASEKIT_API void BStream_empty(BStream *self);
BASEKIT_API int BStream_isEmpty(BStream *self);

// writing --------------------------------------

BASEKIT_API void BStream_writeByte_(BStream *self, unsigned char v);

BASEKIT_API void BStream_writeUint8_(BStream *self, uint8_t v);
BASEKIT_API void BStream_writeUint32_(BStream *self, uint32_t v);
BASEKIT_API void BStream_writeInt32_(BStream *self, int32_t v);
#if !defined(__SYMBIAN32__)
BASEKIT_API void BStream_writeInt64_(BStream *self, int64_t v);
#endif
BASEKIT_API void BStream_writeDouble_(BStream *self, double v);
BASEKIT_API void BStream_writeData_length_(BStream *self, const unsigned char *data, size_t length);
BASEKIT_API void BStream_writeCString_(BStream *self, const char *s);
BASEKIT_API void BStream_writeUArray_(BStream *self, UArray *ba);

// reading --------------------------------------

BASEKIT_API unsigned char BStream_readByte(BStream *self);

BASEKIT_API uint8_t BStream_readUint8(BStream *self);
BASEKIT_API uint32_t BStream_readUint32(BStream *self);
BASEKIT_API int32_t BStream_readInt32(BStream *self);
#if !defined(__SYMBIAN32__)
BASEKIT_API int64_t BStream_readInt64(BStream *self);
#endif
BASEKIT_API double BStream_readDouble(BStream *self);
BASEKIT_API uint8_t *BStream_readDataOfLength_(BStream *self, size_t length);
BASEKIT_API void BStream_readUArray_(BStream *self, UArray *b);
BASEKIT_API UArray *BStream_readUArray(BStream *self);
BASEKIT_API const char *BStream_readCString(BStream *self);

// tagged writing --------------------------------------

BASEKIT_API void BStream_writeTaggedUint8_(BStream *self, uint8_t v);
BASEKIT_API void BStream_writeTaggedUint32_(BStream *self, uint32_t v);
BASEKIT_API void BStream_writeTaggedInt32_(BStream *self, int32_t v);
#if !defined(__SYMBIAN32__)
BASEKIT_API void BStream_writeTaggedInt64_(BStream *self, int64_t v);
#endif
BASEKIT_API void BStream_writeTaggedDouble_(BStream *self, double v);
BASEKIT_API void BStream_writeTaggedData_length_(BStream *self, const unsigned char *data, size_t length);
BASEKIT_API void BStream_writeTaggedCString_(BStream *self, const char *s);
BASEKIT_API void BStream_writeTaggedUArray_(BStream *self, UArray *ba);

// tagged reading --------------------------------------

BASEKIT_API uint8_t BStream_readTaggedUint8(BStream *self);
BASEKIT_API uint32_t BStream_readTaggedUint32(BStream *self);
BASEKIT_API int32_t BStream_readTaggedInt32(BStream *self);
#if !defined(__SYMBIAN32__)
BASEKIT_API int64_t BStream_readTaggedInt64(BStream *self);
#endif
BASEKIT_API double BStream_readTaggedDouble(BStream *self);
BASEKIT_API void BStream_readTaggedUArray_(BStream *self, UArray *b);
BASEKIT_API UArray *BStream_readTaggedUArray(BStream *self);
BASEKIT_API const char *BStream_readTaggedCString(BStream *self);

BASEKIT_API void BStream_show(BStream *self);

#ifdef __cplusplus
}
#endif
#endif

