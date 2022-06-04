#include "libmissing.h"

#ifndef HAVE_EXECINFO_H
#define PROGRAM_NAME "libmissing"
#include "common.h"

int backtrace(void **buffer, int size)
{
	void *addr = __builtin_return_address(0);
	(void)buffer; (void)size;

	errmsg("backtrace() is not implemented. Called from %p", addr);
	return 0;
}

char **backtrace_symbols(void *const *buffer, int size)
{
	(void)buffer; (void)size;
	errmsg("backtrace_symbols() is not implemented");
	return NULL;
}

void backtrace_symbols_fd(void *const *buffer, int size, int fd)
{
	(void)buffer; (void)size; (void)fd;
	errmsg("backtrace_symbols_fd() is not implemented");
}
#endif /* !HAVE_EXECINFO_H */
