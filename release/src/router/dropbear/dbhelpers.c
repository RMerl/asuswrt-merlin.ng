#include "dbhelpers.h"
#include "includes.h"

/* Erase data */
void m_burn(void *data, unsigned int len) {

#if defined(HAVE_MEMSET_S)
	memset_s(data, len, 0x0, len);
#elif defined(HAVE_EXPLICIT_BZERO)
	explicit_bzero(data, len);
#else
	/* This must be volatile to avoid compiler optimisation */
	volatile void *p = data;
	memset((void*)p, 0x0, len);
#endif
}


