/* SPDX-License-Identifier: GPL-2.0 */
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "ip_common.h"

static void nlmon_print_help(struct link_util *lu,
			    int argc, char **argv, FILE *f)
{
	fprintf(f, "Usage: ... nlmon\n");
}

struct link_util nlmon_link_util = {
	.id		= "nlmon",
	.print_help	= nlmon_print_help,
};
