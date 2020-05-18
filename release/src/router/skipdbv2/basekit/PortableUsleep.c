int PortableUsleep_justHereToAvoidRanlibWarning(void) { return 0; }

#ifdef WIN32
#include <windows.h>

int usleep(unsigned int us)
{
	static LARGE_INTEGER freq;
	static int initted = 0;
	LARGE_INTEGER s, e, d;

	if (!initted)
	{
		QueryPerformanceFrequency(&freq);
		initted = 1;
	}

	QueryPerformanceCounter(&s);
	d.QuadPart = freq.QuadPart * ((double)us / 1000000.0);

	do
	{
		QueryPerformanceCounter(&e);
	} while (e.QuadPart - s.QuadPart < d.QuadPart);

	return 0;

}

#endif


