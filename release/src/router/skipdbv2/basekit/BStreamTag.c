/*
copyright
	Steve Dekorte, 2004
license
	BSD revised
*/

#include "BStreamTag.h"
#include <stdio.h>

/*
unsigned int isArray   : 1;
unsigned int type      : 2;
unsigned int byteCount : 5;
*/

BStreamTag BStreamTag_FromUnsignedChar(unsigned char c)
{
	// we need to do this because bit fields are compiler dependent
	BStreamTag t;
	t.isArray = c >> 7;
	t.type = ( c << 1) >> 6;
	t.byteCount = ( c << 3 ) >> 3;
	return t;
}

unsigned char BStreamTag_asUnsignedChar(BStreamTag *self)
{
	BStreamTag t = *self;
	unsigned char c = 0;
	c = c | t.isArray << 7;
	c = c | t.type << 5;
	c = c | t.byteCount;
	return c;
}

// -----------------------------------------------------

BStreamTag BStreamTag_TagArray_type_byteCount_(unsigned int a, unsigned int t, unsigned int b)
{
	BStreamTag self;
	self.isArray = a;
	self.type = t;
	self.byteCount = b;
	return self;
}

int BStreamTag_isEqual_(BStreamTag *self, BStreamTag *other)
{
	return (BStreamTag_asUnsignedChar(self) == BStreamTag_asUnsignedChar(other));
}

void BStreamTag_print(BStreamTag *self)
{
	printf("[Tag ");
	printf("isArray: %i ", self->isArray);
	printf("type: %i ", self->type);
	printf("byteCount: %i", self->byteCount);
	printf("]");
}

char *BStreamTag_typeName(BStreamTag *self)
{
	switch (self->type)
	{
		case BSTREAM_UNSIGNED_INT:
			return "uint";
		case BSTREAM_SIGNED_INT:
			return "int";
		case BSTREAM_FLOAT:
			return "float";
		case BSTREAM_POINTER:
			return "pointer";
	}

	return "UNKNOWN TYPE";
}
