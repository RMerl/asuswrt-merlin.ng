#ifndef LIBMISSING_H
#define LIBMISSING_H

#include "config.h"

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#ifndef HAVE_EXECINFO_H
int backtrace(void **buffer, int size);
char **backtrace_symbols(void *const *buffer, int size);
void backtrace_symbols_fd(void *const *buffer, int size, int fd);
#endif

#endif /* LIBMISSING_H */

