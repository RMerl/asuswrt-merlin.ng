/* SPDX-License-Identifier: GPL-2.0 */
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "ip_common.h"

static void dummy_print_help(struct link_util *lu,
			    int argc, char **argv, FILE *f)
{
	fprintf(f, "Usage: ... dummy\n");
}

struct link_util dummy_link_util = {
	.id		= "dummy",
	.print_help	= dummy_print_help,
};
