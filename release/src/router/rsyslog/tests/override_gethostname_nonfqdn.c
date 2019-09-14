#include <stdio.h>

int gethostname(char *name, size_t __attribute__((unused)) len)
{
	*name++  = 'n';
	*name++  = 'o';
	*name++  = 'n';
	*name++  = 'f';
	*name++  = 'q';
	*name++  = 'd';
	*name++  = 'n';
	*name++  = '\0';
	return 0;
}
