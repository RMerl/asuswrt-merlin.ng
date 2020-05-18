/* based on DynLib.c contributed by Daniel A. Koepke
 * Reorg, Steve Dekorte, 2003-08-30
 * See _BSDLicense.txt
 */

#ifndef DYNLIB_DEFINED
#define DYNLIB_DEFINED 1

#include "Common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void DynLibNoArgFunction(void);
typedef void DynLibOneArgFunction(void *arg);

typedef struct
{
	char *path;
	char *initFuncName;
	void *initArg;
	char *freeFuncName;
	void *freeArg;
	char *error;
	void *handle;
	int refCount;
} DynLib;

BASEKIT_API DynLib *DynLib_new(void);
BASEKIT_API void DynLib_free(DynLib *self);

BASEKIT_API void DynLib_setPath_(DynLib *self, const char *path);
BASEKIT_API char *DynLib_path(DynLib *self);

BASEKIT_API void DynLib_setInitFuncName_(DynLib *self, const char *name);
BASEKIT_API char *DynLib_initFuncName(DynLib *self);
BASEKIT_API void DynLib_setInitArg_(DynLib *self, void *arg);

BASEKIT_API void DynLib_setFreeFuncName_(DynLib *self, const char *name);
BASEKIT_API char *DynLib_freeFuncName(DynLib *self);
BASEKIT_API void DynLib_setFreeArg_(DynLib *self, void *arg);

BASEKIT_API void DynLib_setError_(DynLib *self, const char *path);
BASEKIT_API char *DynLib_error(DynLib *self);

BASEKIT_API void DynLib_open(DynLib *self);
BASEKIT_API unsigned char DynLib_isOpen(DynLib *self);
BASEKIT_API void DynLib_close(DynLib *self);
BASEKIT_API void *DynLib_pointerForSymbolName_(DynLib *self, const char *symbolName);

#ifdef __cplusplus
}
#endif
#endif
