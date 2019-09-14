// we need this for dlsym(): #include <dlfcn.h>
#include <stdio.h>


int gethostname(char *name, size_t __attribute__((unused)) len)
{
	*name = '\0';
	return 0;
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
