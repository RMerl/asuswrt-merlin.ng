#include "config.h"

int main(int argc __attribute__((unused)), char *argv[]__attribute__((unused)))
{
#ifdef HAVE_RELPSRVSETOVERSIZEMODE
	return 0;
#else
	return 1;
#endif
}
