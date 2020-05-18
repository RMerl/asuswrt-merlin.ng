/*
	copyright: Steve Dekorte, 2006. All rights reserved.
	license: See _BSDLicense.txt.
*/

#include "UArray.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>

// read ------------------------------------------------------

size_t UArray_fread_(UArray *self, FILE *fp)
{
	size_t itemsRead = fread(self->data, self->itemSize, self->size, fp);
	UArray_setSize_(self, itemsRead);
	return itemsRead;
}

long UArray_readFromCStream_(UArray *self, FILE *fp)
{
	long totalItemsRead = 0;
	long itemsPerBuffer =  4096 / self->itemSize;
	UArray *buffer = UArray_new();
	UArray_setItemType_(buffer, self->itemType);
	UArray_setSize_(buffer, itemsPerBuffer);

	if (!fp) { perror("UArray_readFromCStream_"); return -1; }

	while(!feof(fp) && !ferror(fp))
	{
		size_t itemsRead;
		UArray_setSize_(buffer, itemsPerBuffer);
		itemsRead = UArray_fread_(buffer, fp);

		totalItemsRead += itemsRead;
		UArray_append_(self, buffer);
		if (itemsRead != itemsPerBuffer) break;
	}

	if (ferror(fp)) { perror("UArray_readFromCStream_"); return -1; }

	UArray_free(buffer);
	return totalItemsRead;
}

long UArray_readNumberOfItems_fromCStream_(UArray *self, size_t size, FILE *stream)
{
	size_t itemsRead;
	UArray *buffer = UArray_new();
	UArray_setItemType_(buffer, self->itemType);
	UArray_setSize_(buffer, size);

	itemsRead = UArray_fread_(buffer, stream);
	UArray_append_(self, buffer);

	UArray_free(buffer);
	return itemsRead;
}

long UArray_readFromFilePath_(UArray *self, const UArray *path)
{
	FILE *stream;
	long itemsRead;
	UArray *sysPath = (UArray_itemSize(path) == 1) ? (UArray *)path : UArray_asUTF8(path);
	const char *p = UArray_asCString(sysPath);

	//printf("UArray_readFromFilePath_(\"%s\")\n", p);

	stream = fopen(p, "rb");
	if (!stream) return -1;
	itemsRead = UArray_readFromCStream_(self, stream);
	fclose(stream);

	if(sysPath != path) UArray_free(sysPath);
	return itemsRead;
}


#define CHUNK_SIZE 4096

int UArray_readLineFromCStream_(UArray *self, FILE *stream)
{
	int readSomething = 0;

	if(self->itemSize == 1)
	{
		char *s = (char *)io_calloc(1, CHUNK_SIZE);

		while (fgets(s, CHUNK_SIZE, stream) != NULL)
		{
			char *eol1 = strchr(s, '\n');
			char *eol2 = strchr(s, '\r');

			readSomething = 1;

			if (eol1) { *eol1 = 0; } // remove the \n return character
			if (eol2) { *eol2 = 0; } // remove the \r return character

			if (*s)
			{
				UArray_appendCString_(self, s);
			}

			if (eol1 || eol2)
			{
				break;
			}
		}

		io_free(s);
	}

	return readSomething;
}

// write ------------------------------------------------------

size_t UArray_fwrite_(const UArray *self, size_t size, FILE *fp)
{
	return fwrite(self->data, 1, self->itemSize * size, fp);
}

long UArray_writeToCStream_(const UArray *self, FILE *stream)
{
	size_t totalItemsRead = UArray_fwrite_(self, self->size, stream);
	if (ferror(stream)) { perror("UArray_readFromCStream_"); return -1; }
	return totalItemsRead;
}

long UArray_writeToFilePath_(const UArray *self, const UArray *path)
{
	UArray *sysPath = (UArray_itemSize(path) == 1) ? (UArray *)path : UArray_asUTF8(path);
	FILE *fp = fopen(UArray_asCString(sysPath), "w");
	long itemsWritten = -1;

	if (fp)
	{
		itemsWritten = UArray_writeToCStream_(self, fp);
		fclose(fp);
	}

	return itemsWritten;
}

