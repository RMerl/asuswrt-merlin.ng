#include <unistd.h>
#include "selinux_internal.h"
#include <stdlib.h>
#include <errno.h>

void freecon(security_context_t con)
{
	free(con);
}

hidden_def(freecon)
