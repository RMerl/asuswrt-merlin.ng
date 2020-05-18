/*
copyright
	Steve Dekorte, 2004
license
	BSD revised
*/

#ifndef BSTREAMTAG_DEFINED
#define BSTREAMTAG_DEFINED 1

#ifdef __cplusplus
extern "C" {
#endif

#define BSTREAM_UNSIGNED_INT 0
#define BSTREAM_SIGNED_INT   1
#define BSTREAM_FLOAT        2
#define BSTREAM_POINTER      3

typedef struct
{
	unsigned int isArray   : 1;
	unsigned int type      : 2; // 0 = unsigned int, 1 = signed int, 2 = float, 3 = pointer
	unsigned int byteCount : 5; // number of bytes in data value(s)
} BStreamTag;

// values in network byte order / big endian

BStreamTag BStreamTag_FromUnsignedChar(unsigned char c);
unsigned char BStreamTag_asUnsignedChar(BStreamTag *self);
BStreamTag BStreamTag_TagArray_type_byteCount_(unsigned int a, unsigned int t, unsigned int b);
int BStreamTag_isEqual_(BStreamTag *self, BStreamTag *other);
void BStreamTag_print(BStreamTag *self);

char *BStreamTag_typeName(BStreamTag *self);

#ifdef __cplusplus
}
#endif
#endif

