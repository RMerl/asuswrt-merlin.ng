//metadoc MainArgs copyright Steve Dekorte 2002
//metadoc MainArgs license BSD revised
/*metadoc MainArgs description
Structure for copying and storing command line arguments.")
*/

#ifndef MAINARGS_DEFINED
#define MAINARGS_DEFINED 1

#include "Common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	int argc;
	const char **argv;
} MainArgs;

BASEKIT_API MainArgs *MainArgs_new(void);
BASEKIT_API void MainArgs_free(MainArgs *self);

BASEKIT_API void MainArgs_argc_argv_(MainArgs *self, int argc, const char **argv);
#define MainArgs_argCount(self) self->argc
#define MainArgs_argAt_(self, index) self->argv[index]

#ifdef __cplusplus
}
#endif
#endif



