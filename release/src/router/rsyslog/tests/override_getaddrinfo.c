// we need this for dlsym(): #include <dlfcn.h>
#include "config.h"
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *node __attribute__((unused)),
	const char *service __attribute__((unused)),
	const struct addrinfo *hints __attribute__((unused)),
	struct addrinfo **res __attribute__((unused)))
{
	return EAI_MEMORY;
}

static void __attribute__((constructor))
my_init(void)
{
	/* we currently do not need this entry point, but keep it as
	 * a "template". It can be used, e.g. to emit some diagnostic
	 * information:
	printf("loaded\n");
	 * or - more importantly - obtain a pointer to the overriden
	 * API:
	orig_etry = dlsym(RTLD_NEXT, "original_entry_point");
	*/
}
