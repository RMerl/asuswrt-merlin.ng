
#ifdef ON_WINDOWS
	#include <windows.h>
#ifndef WIN32
	int usleep (unsigned int us);
#endif
#else
	#include <unistd.h>
#endif

