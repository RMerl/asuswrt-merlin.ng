#include <ccan/json/json.c>
#include <ccan/tap/tap.h>

#include <errno.h>
#include <string.h>

static char *chomp(char *s)
{
	char *e;
	
	if (s == NULL || *s == 0)
		return s;
	
	e = strchr(s, 0);
	if (e[-1] == '\n')
		*--e = 0;
	return s;
}
