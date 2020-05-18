/*
	copyright: Steve Dekorte, 2006. All rights reserved.
	license: See _BSDLicense.txt.
*/

// internally, Io always uses a forward slash "/" for path separators,
// but on Windows, back slashes are also tolerated as path separators.
#if defined(DOS) || defined(ON_WINDOWS)
#define OS_PATH_SEPARATOR     "\\"
#define IO_PATH_SEPARATORS    "\\/"
#else
#define OS_PATH_SEPARATOR     "/"
#define IO_PATH_SEPARATORS    "/"
#endif

#define IO_PATH_SEPARATOR     "/"
#define IO_PATH_SEPARATOR_DOT "."


#ifdef ON_WINDOWS
#define IS_PATH_SEPERATOR(ch) ((ch == '/') || (ch == '\\'))
#else
#define IS_PATH_SEPERATOR(ch) (ch == '/')
#endif


BASEKIT_API void UArray_appendPath_(UArray *self, const UArray *path);

// last component

BASEKIT_API void UArray_removeLastPathComponent(UArray *self);
BASEKIT_API void UArray_clipBeforeLastPathComponent(UArray *self);
BASEKIT_API long UArray_findLastPathComponent(const UArray *self);
BASEKIT_API UArray *UArray_lastPathComponent(const UArray *self);

// extension

BASEKIT_API long UArray_findPathExtension(UArray *self);
BASEKIT_API void UArray_removePathExtension(UArray *self);
BASEKIT_API UArray *UArray_pathExtension(UArray *self);

// fileName

BASEKIT_API UArray *UArray_fileName(UArray *self);

// to/from os path - always returns a copy

BASEKIT_API int UArray_OSPathSeparatorIsUnixSeparator(void);
BASEKIT_API UArray *UArray_asOSPath(UArray *self);
BASEKIT_API UArray *UArray_asUnixPath(UArray *self);

