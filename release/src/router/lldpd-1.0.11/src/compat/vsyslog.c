/* -*- mode: c; c-file-style: "openbsd" -*- */

#include <stdlib.h>
#include <syslog.h>
#include "compat.h"

/* vsyslog() doesn't exist on HP-UX */
void
vsyslog(int facility, const char *format, va_list ap) {
	char *msg = NULL;
	if (vasprintf(&msg, format, ap) == -1) {
		return;
	}
	syslog(facility, "%s", msg);
	free(msg);
}
