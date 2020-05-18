/*
	copyright: Steve Dekorte, 2006. All rights reserved.
	license: See _BSDLicense.txt.
*/

#include "UArray.h"
#include "ConvertUTF.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>

int UArray_MachineIsLittleEndian(void)
{
	unsigned int i = 0x1;
	return ((unsigned char *)(&i))[0] == 1;
}

static int UArray_SizeOfUTFChar(const uint8_t *s)
{
	uint8_t c = *s;

	if (c & 0x80)
	{
		if((c & 0xE0) == 0xC0) return 2;
		if((c & 0xF0) == 0xE0) return 3;
		if((c & 0xF8) == 0xF0) return 4;
		if((c & 0xFC) == 0xF8) return 5;
		if((c & 0xFE) == 0xFC) return 6;
		return -1;
	}

	return 1;
}

int UArray_maxCharSize(const UArray *self)
{
	if (self->encoding == CENCODING_UTF8)
	{
		int maxCharSize = 1;
		size_t i = 0;

		while (i < self->size)
		{
			int charSize = UArray_SizeOfUTFChar(self->data + i);
			if (charSize > maxCharSize)  maxCharSize = charSize;
			if (charSize == -1) return -1;
			i += charSize;
		}

		return maxCharSize;
	}

	return self->itemSize;
}

void UArray_truncateAfterConvertToEncoding_(UArray *self){
	if(self->encoding == CENCODING_NUMBER)
	{
		return;
	}

	{
		UArray tmp = UArray_stackAllocedWithData_type_size_("\0", CTYPE_uint8_t, 1);
		long newSize = UArray_find_(self, &tmp);
		if(newSize != -1)
		{
			UArray_setSize_(self, newSize);
		}
	}
}

int UArray_convertToFixedSizeType(UArray *self)
{
	if (self->encoding == CENCODING_UTF8)
	{
		int maxCharSize = UArray_maxCharSize(self);

		if(maxCharSize == 1)
		{
			self->encoding = CENCODING_ASCII;
		}
		else if(maxCharSize == 2)
		{
			UArray_convertToUTF16(self);
		}
		else
		{
			UArray_convertToUTF32(self);
		}

		return 1;
	}

	return 0;
}

int UArray_isMultibyte(const UArray *self)
{
	if (self->encoding == CENCODING_UTF8)
	{
		UARRAY_INTFOREACH(self, i, v, if (ismbchar((int)v)) return 1; );
	}

	return 0;
}

int UArray_isLegalUTF8(const UArray *self)
{
	void *sourceStart = self->data;
	void *sourceEnd   = self->data + self->size * self->itemSize;

	return isLegalUTF8Sequence(sourceStart, sourceEnd);
}

UArray *UArray_asNumberArrayString(const UArray *self)
{
	UArray *out = UArray_new();
	UArray_setEncoding_(out, CENCODING_ASCII);

	UARRAY_INTFOREACH(self, i, v,
		char s[128];

		if(UArray_isFloatType(self))
		{
			sprintf(s, "%f", v);
		}
		else
		{
			sprintf(s, "%i", v);
		}

		if(i != UArray_size(self) -1 ) strcat(s, ", ");
		UArray_appendBytes_size_(out, (unsigned char *)s, strlen(s));
	);

	return out;
}


UArray *UArray_asUTF8(const UArray *self)
{
	UArray *out = UArray_new();
	UArray_setItemType_(out, CTYPE_uint8_t);
	UArray_setEncoding_(out, CENCODING_UTF8);
	UArray_setSize_(out, self->size * 4);

	{
		ConversionResult r = conversionOK;
		ConversionFlags options = lenientConversion;
		void *sourceStart = self->data;
		void *sourceEnd   = self->data + self->size * self->itemSize;
		UTF8 *targetStart = out->data;
		UTF8 *targetEnd   = out->data + out->size * out->itemSize;
		size_t outSize;

		switch(self->encoding)
		{
			case CENCODING_ASCII:
				UArray_copy_(out, self);
				break;
			case CENCODING_UTF8:
				UArray_copy_(out, self);
				break;
			case CENCODING_UTF16:
				r = ConvertUTF16toUTF8((const UTF16 **)&sourceStart, (const UTF16 *)sourceEnd, &targetStart, targetEnd, options);
				//outSize = (targetStart - out->data) / out->itemSize;
				break;
			case CENCODING_UTF32:
				r = ConvertUTF32toUTF8((const UTF32 **)&sourceStart, (const UTF32 *)sourceEnd, &targetStart, targetEnd, options);
				//outSize = (targetStart - out->data) / out->itemSize;
				break;
			case CENCODING_NUMBER:
				{
					UArray *nas = UArray_asNumberArrayString(self);
					UArray_free(out);
					out = UArray_asUTF8(nas);
					UArray_free(nas);
					break;
				}
			default:
				printf("UArray_asUTF8 - unknown source encoding\n");
		}
	}


	UArray_setSize_(out, strlen((char *)out->data));

	return out;
}

UArray *UArray_asUTF16(const UArray *self)
{
	UArray *out = UArray_new();
	UArray_setItemType_(out, CTYPE_uint16_t);
	UArray_setEncoding_(out, CENCODING_UTF16);
	UArray_setSize_(out, self->size);

	{
		ConversionResult r = conversionOK;
		ConversionFlags options = lenientConversion;
		void *sourceStart = self->data;
		void *sourceEnd   = self->data + self->size * self->itemSize;
		UTF16 *targetStart = (UTF16 *)out->data;
		UTF16 *targetEnd   = (UTF16 *)(out->data + out->size * out->itemSize);

		switch(self->encoding)
		{
			case CENCODING_ASCII:
				r = ConvertUTF8toUTF16((const UTF8 **)&sourceStart, (const UTF8 *)sourceEnd, &targetStart, targetEnd, options);
				break;
			case CENCODING_UTF8:
				r = ConvertUTF8toUTF16((const UTF8 **)&sourceStart, (const UTF8 *)sourceEnd, &targetStart, targetEnd, options);
				break;
			case CENCODING_UTF16:
				UArray_copy_(out, self);
				break;
			case CENCODING_UTF32:
				r = ConvertUTF32toUTF16((const UTF32 **)&sourceStart, (const UTF32 *)sourceEnd, &targetStart, targetEnd, options);
				break;
			case CENCODING_NUMBER:
				{
					UArray *nas = UArray_asNumberArrayString(self);
					UArray_free(out);
					out = UArray_asUTF16(nas);
					UArray_free(nas);
					break;
				}
			default:
				printf("UArray_asUTF16 - unknown source encoding\n");
		}
	}

	UArray_truncateAfterConvertToEncoding_(out);

	return out;
}

UArray *UArray_asUTF32(const UArray *self)
{
	UArray *out = UArray_new();
	UArray_setItemType_(out, CTYPE_uint32_t);
	UArray_setEncoding_(out, CENCODING_UTF32);
	UArray_setSize_(out, self->size);

	{
		ConversionResult r = conversionOK;
		ConversionFlags options = lenientConversion;
		void *sourceStart = self->data;
		void *sourceEnd   = self->data + self->size * self->itemSize;
		UTF32 *targetStart = (UTF32 *)out->data;
		UTF32 *targetEnd   = (UTF32 *)(out->data + out->size * out->itemSize);

		switch(self->encoding)
		{
			case CENCODING_ASCII:
				r = ConvertUTF8toUTF32((const UTF8 **)&sourceStart, (const UTF8 *)sourceEnd, &targetStart, targetEnd, options);
				break;
			case CENCODING_UTF8:
				r = ConvertUTF8toUTF32((const UTF8 **)&sourceStart, (const UTF8 *)sourceEnd, &targetStart, targetEnd, options);
				break;
			case CENCODING_UTF16:
				r = ConvertUTF16toUTF32((const UTF16 **)&sourceStart, (const UTF16 *)sourceEnd, &targetStart, targetEnd, options);
				break;
			case CENCODING_UTF32:
				UArray_copy_(out, self);
				break;
			case CENCODING_NUMBER:
				{
					UArray *nas = UArray_asNumberArrayString(self);
					UArray_free(out);
					out = UArray_asUTF32(nas);
					UArray_free(nas);
					break;
				}
			default:
				printf("UArray_asUTF32 - unknown source encoding\n");
		}
	}

	UArray_truncateAfterConvertToEncoding_(out);

	return out;
}

void UArray_convertToUTF8(UArray *self)
{
	UArray *a = UArray_asUTF8(self);
	UArray_swapWith_(self, a);
	UArray_free(a);
}

void UArray_convertToUTF16(UArray *self)
{
	UArray *a = UArray_asUTF16(self);
	UArray_swapWith_(self, a);
	UArray_free(a);
}

void UArray_convertToUTF32(UArray *self)
{
	UArray *a = UArray_asUTF32(self);
	UArray_swapWith_(self, a);
	UArray_free(a);
}


// ----------------------------------------------------


