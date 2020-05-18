
#ifdef __cplusplus
extern "C" {
#endif


#ifdef _WIN32
#include "Common.h"

BASEKIT_API    int truncate(const char *path, long length);

#else

	#include <unistd.h>

#endif


#ifdef __cplusplus
}
#endif
