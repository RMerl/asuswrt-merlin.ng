/*
	copyright: Steve Dekorte, 2006. All rights reserved.
	license: See _BSDLicense.txt.
*/

// these return item read/written count or -1 on error

// read

BASEKIT_API size_t UArray_fread_(UArray *self, FILE *fp);
BASEKIT_API long UArray_readFromCStream_(UArray *self, FILE *stream);
BASEKIT_API long UArray_readFromFilePath_(UArray *self, const UArray *path);
BASEKIT_API long UArray_readNumberOfItems_fromCStream_(UArray *self, size_t size, FILE *stream);
BASEKIT_API int UArray_readLineFromCStream_(UArray *self, FILE *stream);

// write

BASEKIT_API size_t UArray_fwrite_(const UArray *self, size_t size, FILE *stream);
BASEKIT_API long UArray_writeToCStream_(const UArray *self, FILE *stream);
BASEKIT_API long UArray_writeToFilePath_(const UArray *self, const UArray *path);
