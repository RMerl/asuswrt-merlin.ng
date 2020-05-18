/*
	copyright: Steve Dekorte, 2006. All rights reserved.
	license: See _BSDLicense.txt.
*/

#include "UArray.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>

UArray *UArray_newWithFormat_(const char *format, ...)
{
	UArray *self;
	va_list ap;
	va_start(ap, format);
	self = UArray_newWithVargs_(format, ap);
	va_end(ap);
	return self;
}

UArray *UArray_newWithVargs_(const char *format, va_list ap)
{
	UArray *self = UArray_new();
	UArray_fromVargs_(self, format,ap);
	return self;
}

UArray *UArray_fromFormat_(UArray *self, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	UArray_fromVargs_(self, format, ap);
	va_end(ap);
	return self;
}

void UArray_fromVargs_(UArray *self, const char *format, va_list ap)
{
	while (*format)
	{
		if (*format == '%')
		{
			format ++;

			if (*format == 's')
			{
				char *s = va_arg(ap, char *);
				if (!s) { printf("UArray_fromVargs_ missing param"); return; }
				UArray_appendCString_(self, s);
			}
			else if (*format == 'i' || *format == 'd')
			{
				int i = va_arg(ap, int);
				char s[100];

				snprintf(s, 100, "%i", i);
				UArray_appendCString_(self, s);
			}
			else if (*format == 'f')
			{
				double d = va_arg(ap, double);
				char s[100];

				snprintf(s, 100, "%f", d);
				UArray_appendCString_(self, s);
			}
			else if (*format == 'p')
			{
				void *p = va_arg(ap, void *);
				char s[100];

				snprintf(s, 100, "%p", p);
				UArray_appendCString_(self, s);
			}
			// new format command for a given number adding spaces
			else if (*format == '#')
			{
				int n, i = va_arg(ap, int);
				char *s = " ";

				for (n = 0; n < i; n ++)
				{
					UArray_appendCString_(self, s);
				}
			}
		}
		else
		{
			char s[2];

			snprintf(s, 2, "%c", *format);
			UArray_appendCString_(self, s);
		}

		format ++;
	}
}

UArray *UArray_asNewHexStringUArray(UArray *self)
{
	size_t i, newSize = self->size * 2;
	UArray *ba = UArray_new();
	UArray_setSize_(ba, newSize);

	for(i = 0; i < self->size; i ++)
	{
		int v = UArray_longAt_(self, i);
		char *s = (char *)(ba->data + i * 2);

		if (v < 16)
		{
			snprintf(s, newSize, "0%x", (int)v);
		}
		else
		{
			snprintf(s, newSize, "%x", (int)v);
		}
	}

	return ba;
}
