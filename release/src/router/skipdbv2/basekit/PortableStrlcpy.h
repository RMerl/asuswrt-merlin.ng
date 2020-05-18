#ifdef _WIN32
#include <stddef.h> // size_t on windows
#else
#include <sys/types.h> // size_t
#endif

#define strlcpy(d, s, l) PortableStrlcpy((d), (s), (l))

size_t PortableStrlcpy(char*, const char*, size_t);
