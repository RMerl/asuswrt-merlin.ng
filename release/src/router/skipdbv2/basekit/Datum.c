/*#io
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("Datum")    
*/

#include "Datum.h"
#include "List.h"
#include "UArray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PID_TYPE Datum_size(Datum *self)
{
	return self->size;
}

unsigned char *Datum_data(Datum *self)
{
	return self->data;
}

void *Datum_asUArray(Datum *self)
{
	return UArray_newWithData_type_size_copy_(self->data, CENCODING_UTF8, self->size, 0);
}

Datum Datum_FromData_length_(unsigned char *data, PID_TYPE size)
{
	Datum d;
	d.data = data;
	d.size = size;
	return d;
}

Datum Datum_FromCString_(const char *s)
{
	Datum d;
	d.data = (unsigned char *)s;
	d.size = strlen(s) + 1;
	return d;
}

/*
Datum Datum_FromPid_(PID_TYPE pid)
{
	Datum d;
	d.data = (unsigned char *)&pid;
	d.size = sizeof(PID_TYPE);
	return d;
}
*/

Datum Datum_FromUArray_(UArray *ba)
{
	Datum d;
	d.data = (unsigned char *)UArray_bytes(ba);
	d.size = UArray_sizeInBytes(ba);
	return d;
}

Datum Datum_Empty(void)
{
	Datum d;
	d.data = NULL;
	d.size = 0;
	return d;
}

Datum Datum_datumAt_(Datum *self, size_t i)
{
	Datum d;
	
	if (i < self->size)
	{
		d.data = self->data + i;
		d.size = self->size - i;
	}
	else
	{
		d.data = NULL;
		d.size = 0;
	}
	
	return d;
}


int Datum_compare_(Datum *self, Datum *other)
{
	size_t l1 = self->size;
	size_t l2 = other->size;
	size_t min = l1 < l2 ? l1 : l2;
	int cmp = memcmp(self->data, other->data, min);
	
	if (cmp == 0)
	{
		if (l1 == l2) 
		{
			return 0;
		}
		
		if (l1 < l2)
		{
			return -1;
		}
		
		return 1;
	}
	
	return cmp;
}

int Datum_compare_length_(Datum *self, Datum *other, size_t limit)
{
	size_t l1 = self->size;
	size_t l2 = other->size;
	size_t min = l1 < l2 ? l1 : l2;
	
	min = min < limit ? min : limit;
	
	return memcmp(self->data, other->data, min);
}

int Datum_beginsWith_(Datum *self, Datum *other)
{
	if (self->size >= other->size)
	{
		return (0 == Datum_compare_length_(self, other, other->size));
	}
	
	return 0;
}

int Datum_endsWith_(Datum *self, Datum *other)
{
	if (self->size >= other->size)
	{
		Datum d = Datum_datumAt_(self, self->size - other->size); 
		return (0 == Datum_compare_(&d, other));
	}
	
	return 0;
}

int Datum_compareCString_(Datum *self, const char *s)
{
	Datum sd = Datum_FromCString_(s);
	return Datum_compare_(self, &sd);
}

size_t Datum_matchingPrefixSizeWith_(Datum *self, Datum *other)
{
	unsigned int l1 = self->size;
	unsigned int l2 = other->size;
	unsigned int min = l1 < l2 ? l1 : l2;
	unsigned char *b1 = self->data;
	unsigned char *b2 = other->data;
	size_t i;
	
	for (i = 0; i < min; i ++)
	{
		if (*b1 != *b2) break;
		*b1 ++;
		*b2 ++;
	}
	
	return i;
}

Datum *Datum_newFrom_to_(Datum *self, size_t start, size_t end)
{
	Datum *d = (Datum *)malloc(sizeof(Datum));
	
	if ((start < end) && (end <= (size_t)self->size))
	{
		d->data = self->data + start;
		d->size = end - start;
		return d;
	}
	
	d->data = NULL;
	d->size = 0;
	return d;
}

long Datum_find_(Datum *self, void *delimsList, size_t startIndex) 
{
	List *delims = (List *)delimsList;
	List *results = List_new();
	size_t i, last = 0;
	
	if (startIndex > self->size) return -1;
	
	for (i = startIndex; i < self->size; i ++)
	{
		Datum d = Datum_datumAt_(self, i);
		size_t j; 
		
		for (j = 0; j < (size_t)List_size(delims); j ++)
		{
			Datum *delim = (Datum *)List_at_(delims, j);
			
			if (Datum_beginsWith_(&d, delim))
			{
				return i;
			}
		}
	}
	
	return -1;
}


void *Datum_split_(Datum *self, void *delimsList) /* returns a List */
{
	List *delims = (List *)delimsList;
	List *results = List_new();
	size_t i, last = 0;
	
	for (i = 0; i < self->size; i ++)
	{
		Datum d = Datum_datumAt_(self, i);
		size_t j; 
		
		for (j = 0; j < (size_t)List_size(delims); j ++)
		{
			Datum *delim = (Datum *)List_at_(delims, j);
			
			if (Datum_beginsWith_(&d, delim))
			{
                                List_append_(results, Datum_newFrom_to_(self, last, i));
				
				last = i + delim->size;
				i = last - 1; /* since for() will increment it */
				break;
			}
		}
	}
	
	if (last != self->size)
	{
		List_append_(results, Datum_newFrom_to_(self, last, self->size));
	}
	
	return results;
}


// enumeration 

/*
 int Datum_next(Datum *self)
 {
	 if (self->size)
	 {
		 self->data ++;
		 self->size --;
		 return 1;
	 }
	 return 0;
 }
 */

unsigned int Datum_hash(Datum *self)
{
	PID_TYPE length = self->size;
	unsigned char *key = self->data;
	unsigned int h = 5381;
	
	while (length-- > 0)
	{
		h += (h << 5); /* h(i) = (h(i-1) * 33) ^ key(i) */
		h ^= *key ++;
	}
	
	return h;
}

/*
 typedef int (DatumDetectWithFunc)(void *, Datum *); // 1 = match, -1 = break 
 
 int Datum_detect_withTarget_(Datum *self, DatumDetectWithFunc *func, void *target)
 {
	 unsigned char *s   = self->data;
	 unsigned char *end = self->data + self->size;
	 
	 Datum d;
	 int result;
	 
	 while (s < end)
	 {
		 d.data = s;
		 d.size = end - s;
		 result = (*func)(target, &d);
		 
		 if (result == 1) 
		 {
			 return s - self->bytes;
		 } 
		 else if (result == -1)
		 {
			 return -1;
		 }
		 
		 s ++;
	 }
	 
	 return -1;
 }
 */

/*
 Datum Datum_datumUntil_withTarget_(Datum *self, DatumDetectWithFunc *func, void *target)
 {
	 int index = Datum_detect_withTarget_(self, func, target);
	 Datum chunk;
	 
	 if (index == -1) 
	 {
		 index = length; 
	 }
	 
	 d.data = self->data + index;
	 d.size = self->size - index;
	 return d;
 }
 */
