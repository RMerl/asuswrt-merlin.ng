#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "ip_common.h"

static void netdevsim_print_help(struct link_util *lu,
				 int argc, char **argv, FILE *f)
{
	fprintf(f, "Usage: ... netdevsim\n");
}

struct link_util netdevsim_link_util = {
	.id		= "netdevsim",
	.print_help	= netdevsim_print_help,
};
